/*
 * FILE: DataWriterImpl.c - DataWriter implementation
 *
 * (c) Copyright 2012-2021 Real-Time Innovations
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 08aug2022,tk MICRO-3367/PR.30121
 * - Removed UNUSED_ARG(buffer) in cdr_initialize
 * 17may2022, am MICRO-3570/PR.30488
 * - Updated DDS_DataWriter_write_untyped to return sample->payload buffer even 
 *   if NETIO_Interface_send() fails. 
 * 10dec2021,tk MICRO-3226/PR.29479
 * - Removed UNUSED_ARG(timestamp) in register_key
 * 17sep2021,tk MICRO-3276/PR.29664
 * - Added transport sequence in call to NETIO_BindResolver_reserve_addresses
 * 13sep2021,tk MICRO-3196/PR.29517
 * - Always send 0 as the PID length for the PID_SENTINEL in
 *   DDS_DataWriter_serialize_sample() (even if the OMG standard says the
 *   length for the PID_SENTINEL should be ignored by the receiver).
 * 15apr2021,tk MICRO-3034/PR.29077
 * - Set retcode to DDS_RETCODE_ERROR in DDS_DataWriter_write_untyped() when
 *   DDSHST_Writer_commit_entry() fails.
 * - Removed redundant tests for NULL for sample and sample_entry on failure in
 *   DDS_DataWriter_write_untyped().
 * - Free sample if DDS_DataWriter_serialize_sample() fails.
 * 15apr2021,tk MICRO-2979/PR.27549
 * - Removed unused functions from CERT:
 *   DDS_DataWriter_get_objectid
 * 06apr2021,tk MICRO-3007/PR.29021
 *  - Correct function header comment for DDS_DataWriter_serialize_sample.
 *  - Replace 16 with RTPS_KEY_HASH_MAX_LENGTH in DDS_DataWriter_serialize_sample
 * 21sep2016,tk  MICRO-1546 Removed special handling of best-effort/no deadline
 *                          treat it as any other Qos
 * 04nov2015,tk  MICRO-1505 Reduce memory footprint
 * 17jul2015,tk  MICRO-1442/PR#15468 Pass plugin_data to get_key_kind and
 *                                   get_serialized_key_max_size
 * 09jul2015,tk  MICRO-1369          Properly handle auto-registration of
 *                                   dispose and unregister
 * 30jun2015,tk  MICRO-1376/PR#15191 Fixed comment for is_consistent()
 * 29mar2015,tk  MICRO-1117/PR#14253 Removed invalid comment
 * 11jun2015,tk  MICRO-1119/PR#14255 Improved and simplified key-handling code
 * 16mar2015,tk  MICRO-1117/PR#14253 Removed magic constants and commented on
 *                                   how the maximum sample buffer is created
 * 16mar2015,tk  MICRO-1118/PR#14254 Removed redundant code
 * 16mar2015,tk  MICRO-1120/PR#14256 Use NETIO_RTPS_FLAGS_UNREGISTER instead
 *                                   of RTPS_UNREGISTER_STATUS_INFO (same value)
 * 16mar2015,tk  MICRO-1121/PR#14257 Return resources in all cases of failure
 * 19sep2014,tk  MICRO-875 Added checks for writer enabled where applicable
 * 16sep2014,as  MICRO-903/PR#11236 - Incorrect handling of status events and listeners
 * 31jul2014,tk  MICRO-842/PR#9683 - Removed superfluous parameters in DB
 *                                  compare function
 * 19mar2014,tk  MICRO-74: Support endpoint specific transport
 * 11nov2013,as  MICRO-718 Add get_X_status() API to DataReader and DataWriter
 * 08nov2013,as  MICRO-681 Complete implementation of WaitSets
 *               and support for StatusConditions
 * 06jun2013,kaj MICRO-183: CDR stream alignment reset moved to (de)ser_header
 * 23mar2013,tk  Updated logging, robustness checks
 * 06feb2013,eh  Refix MICRO-180: md5Stream only for USER_KEY
 * 06feb2013,eh  Fix sending keyHash for GUID_KEY
 * 06feb2013,eh  MICRO-262: assign RTPS resource limits
 * 14dec2012,kaj MICRO-180: DDS_DataWriterImpl_initialize was not
 *                         allocating md5Stream with serialized_key_max_size
 * 06jun2012,tk Written
 */
/*ce
 * \file
 * \brief DataWriter implementation
 *
 * \details
 * This file implements internal functions needed to support the public
 * DDS datawriter API, mainly related to the life-cycle of a DDS datawriter
 * object.
 */
/*ci \addtogroup DDSPublicationModule
 * @{
 */
#ifndef dds_c_config_h
#include "dds_c/dds_c_config.h"
#endif
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif
#ifndef reda_bufferpool_h
#include "reda/reda_bufferpool.h"
#endif
#ifndef netio_address_h
#include "netio/netio_address.h"
#endif
#ifndef netio_route_h
#include "netio/netio_route.h"
#endif
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_discovery_plugin_h
#include "dds_c/dds_c_discovery_plugin.h"
#endif

#include "InstanceHandle.h"
#include "QosPolicy.h"
#include "Entity.h"
#include "TopicDescription.h"
#include "Topic.h"
#include "Type.h"
#include "DataWriterQos.h"
#include "DataWriterEvent.h"
#include "DataWriterInterface.h"

#include "DataWriterImpl.h"

const char* const DDSHST_WRITER_DEFAULT_HISTORY_NAME = "wh";

const char* const DDS_DEFAULT_DATAWRITER_NETIO_NAME = "wi";

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Compare entries in the table of datawriter entries. The function is
 *        compatible with \ref DB_IndexCompare_T
 *
 * \param[in] flags Used to distinguish between op2 being a key or record
 * \param[in] op1   A DDS_DataWriterImpl already in the database
 * \param[in] op2   Either a DDS_DataWriterImpl being added or a key
 *                  being searched for
 *
 * \return positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2,
 *         zero if op1 is equal to op2
 */
RTI_INT32
DDS_DataWriterImpl_compare(RTI_INT32 flags,const DB_Record_T op1, void *op2)
{
    struct DDS_DataWriterImpl *record_left = (struct DDS_DataWriterImpl*)op1;
    const DDS_UnsignedLong *id_right;

    if (DB_SELECT_OP2_IS_KEY(flags))
    {
        id_right = (const DDS_UnsignedLong*)op2;
    }
    else
    {
        id_right = &((struct DDS_DataWriterImpl*)op2)->as_entity.entity_id;
    }

    return ((record_left->as_entity.entity_id == *id_right) ? 0 :
                 (record_left->as_entity.entity_id > *id_right ? 1 : -1));
}

#ifndef RTI_CERT
/*ci
 * \brief Return the object-id for the datawriter
 *
 * \param[in] self Datawriter to return object-id for
 *
 * \return The datawriter object-id
 */
DDS_UnsignedLong
DDS_DataWriter_get_objectid(DDS_DataWriter *self)
{
    return self->as_entity.entity_id;
}
#endif

/*ci
 * \brief Initialize A CDR buffer
 *
 * \details
 *
 * This function is called by a REDA_BufferPool to initialize each CDR buffer.
 * The memory layout for a CDR buffer is illustrated below
 *
 * \verbatim
 *
 *  RTI_TransformCDR_Sample
 *  +-----------+ sample
 *  |           |
 *  |   Header  |
 *  |           |
 *  +-----------+ sample->payload
 *  |           |
 *  |           |
 *  |  Payload  |
 *  |           |
 *  +-----------+
 *
 * \endverbatim
 *
 * \param[in] init_config Parameter passed to REDA_BufferPool_new()
 * \param[in] buffer      Buffer from buffer pool to initialize
 *
 * \return This function always returns RTI_TRUE.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DataWriter_cdr_initialize(void *init_config, void *buffer)
{
    struct RTI_TransformCDR_Sample *sample =
                                     (struct RTI_TransformCDR_Sample *)buffer;
    UNUSED_ARG(init_config);

    sample->payload = (char*)sample + sizeof(struct RTI_TransformCDR_Sample);

    return RTI_TRUE;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize a datawriter
 *
 * \details
 * Free up all resources used by a datawriter. Note that this function does
 * not free the memory used to hold the datawriter itself, this memory is
 * freed by the publisher because the publisher is the factory.
 *
 * \param[in] self Datawriter to finalize
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DDS_DataWriterImpl_finalize(DDS_DataWriter *self)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    struct RT_ComponentFactory *factory;
    struct DDS_DataWriterImpl *datawriter = (struct DDS_DataWriterImpl *)self;

    if (DDS_LocatorSeq_get_length(&datawriter->uc_locator_seq) > 0)
    {
        if (!NETIO_BindResolver_release_addresses(
                        datawriter->config->bind_resolver,
                        datawriter->config->enabled_transports,
                        NETIO_ROUTEKIND_USER,
                        (struct NETIO_AddressSeq*)&datawriter->uc_locator_seq))
        {
            goto done;
        }

        if (!DDS_LocatorSeq_finalize(&datawriter->uc_locator_seq))
        {
            goto done;
        }
    }

    if (datawriter->wh != NULL)
    {
        factory = RT_Registry_lookup(datawriter->config->registry,
                                     DDSHST_WRITER_DEFAULT_HISTORY_NAME);
        if (factory == NULL)
        {
            DDSC_LOG_COMPONENT_LOOKUP(OSAPI_LOGKIND_ERROR,
                                      DDSHST_WRITER_DEFAULT_HISTORY_NAME)
            goto done;
        }
        DDSHST_WriterFactory_delete_component(factory, datawriter->wh);
        datawriter->wh = NULL;
    }


    if (datawriter->dw_intf != NULL)
    {
        factory = RT_Registry_lookup(datawriter->config->registry,
                                     DDS_DEFAULT_DATAWRITER_NETIO_NAME);
        if (factory == NULL)
        {
            DDSC_LOG_COMPONENT_LOOKUP(OSAPI_LOGKIND_ERROR,
                                      DDS_DEFAULT_DATAWRITER_NETIO_NAME)
            goto done;
        }
        NETIO_InterfaceFactory_delete_component(factory,datawriter->dw_intf);
        datawriter->dw_intf = NULL;
    }

    if (datawriter->rtps_intf != NULL)
    {
        factory = RT_Registry_lookup(datawriter->config->registry,
                                     NETIO_DEFAULT_RTPS_NAME);
        if (factory == NULL)
        {
            DDSC_LOG_COMPONENT_LOOKUP(OSAPI_LOGKIND_ERROR,
                                     NETIO_DEFAULT_RTPS_NAME)
            goto done;
        }

        NETIO_InterfaceFactory_delete_component(factory,datawriter->rtps_intf);
        datawriter->rtps_intf = NULL;
    }

    if (DDS_Entity_is_enabled(DDS_DataWriter_as_entity(self)) &&
        !DDS_Duration_is_infinite(&datawriter->qos.deadline.period))
    {
        if (!OSAPI_Timer_delete_timeout(datawriter->config->timer,
                                        &datawriter->deadline_event))
        {
            goto done;
        }
    }

    if (DDS_Entity_is_enabled(DDS_DataWriter_as_entity(self)) &&
        !DDS_Duration_is_infinite(&datawriter->qos.liveliness.lease_duration))
    {
        if (!OSAPI_Timer_delete_timeout(datawriter->config->timer,
                                        &datawriter->liveliness_event))
        {
            goto done;
        }
    }

    if ((datawriter->cdr_samples != NULL) &&
        !REDA_BufferPool_delete(datawriter->cdr_samples))
    {
        DDSC_LOG_CDR_POOL_DELETE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_CDR)
        goto done;
    }

    if ((datawriter->cdr_payloads != NULL) &&
        !REDA_BufferPool_delete(datawriter->cdr_payloads))
    {
        DDSC_LOG_CDR_POOL_DELETE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_CDR)
        goto done;
    }

    if (self->md5_stream != NULL)
    {
        CDR_Stream_free(self->md5_stream);
    }

    if (DDS_DataWriterQos_finalize(&self->qos) != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_QOS)
        goto done;
    }

    if (!DDS_EntityImpl_finalize(&self->as_entity))
    {
        DDSC_LOG_ENTITY_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_ENTITY)
        goto done;
    }

    retval = DDS_BOOLEAN_TRUE;

done:

    return retval;
}
#endif /* !RTI_CERT */

/*ci
 * \brief Return the instance handle of a datawriter
 *
 * \details
 * This function overrides the DDS entity get_instance_handle function and
 * is called via DDS_Entity_get_instance_handle.
 *
 * \param[in] entity The base-class for the datawriter
 *
 * \return The instance handle
 */
MUST_CHECK_RETURN RTI_PRIVATE DDS_InstanceHandle_t
DDS_DataWriterImpl_get_instance_handle(DDS_Entity *entity)
{
    struct DDS_DataWriterImpl *dw = (struct DDS_DataWriterImpl*)entity;
    DDS_InstanceHandle_t retval;

    retval = dw->config->get_parent_handle((DDS_Entity*)dw->publisher);

    DDS_InstanceHandle_set_suffix(&retval,dw->as_entity.entity_id);

    return retval;
}

/*ci
 * \brief Initialize a datawriter
 *
 * \details
 *
 * The publisher allocates memory to store the datawriter data and passes
 * it to the datawriter for initialization. The datawriter allocates all its
 * internal resources. The datawriter is passed shared resources in the
 * config structure, such as database, timers resolvers etc. These resources
 * are typically managed by the domain participant.
 *
 * \param[in] datawriter A datawriter structure to initialize
 * \param[in] publisher  The publisher creating the datawriter
 * \param[in] topic      The topic the datawriter is publishing
 * \param[in] qos        The datawriter qos policy
 * \param[in] listener   The datawriter listener
 * \param[in] mask       Mask with enabled statuses on the datawriter
 * \param[in] object_id  The datawriter object id generated by the factory
 * \param[in] config     General datawriter configuration
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DDS_DataWriterImpl_initialize(struct DDS_DataWriterImpl *datawriter,
                              DDS_Publisher *publisher,
                              DDS_Topic *topic,
                              const struct DDS_DataWriterQos *qos,
                              const struct DDS_DataWriterListener *listener,
                              DDS_StatusMask mask,
                              DDS_UnsignedLong object_id,
                              struct NDDS_DataWriterConfig *config)
{
#if !(INCLUDE_API_QOS)
    struct DDS_DataWriterQos init_val = DDS_DataWriterQos_INITIALIZER;
#endif
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    struct REDA_BufferPoolProperty cdr_property =
                                            REDA_BufferPoolProperty_INITIALIZER;
    struct DDSHST_WriterProperty wh_property =
                                DDSHST_WriterProperty_INITIALIZER;
    struct DDSHST_WriterListener wh_listener =
                                DDSHST_WriterListener_INITIALIZE;
    struct RT_ComponentFactory *factory;
    struct DDS_DataWriterInterfaceProperty dwintf_property =
                                DDS_DataWriterInterfaceProperty_INITIALIZER;
    DDS_InstanceHandle_t instance_handle;
    struct RTPS_InterfaceProperty rtps_property =
                                            RTPS_InterfaceProperty_INITIALIZER;
    DDS_UnsignedLong serialized_key_size;
    struct DDS_DataWriterListener nil_listener =
                                            DDS_DataWriterListener_INITIALIZER;
    NDDS_TypePluginKeyKind key_kind;

    OSAPI_Memory_zero(datawriter,sizeof(struct DDS_DataWriterImpl));

    if ((qos != &DDS_DATAWRITER_QOS_DEFAULT) &&
        !DDS_DataWriterQos_is_consistent(qos))
    {
        DDSC_LOG_QOS_INCONSISTENT(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_QOS)
        goto done;
    }

    if ((listener != NULL) &&
        !DDS_DataWriterListener_is_consistent(listener, mask))
    {
        DDSC_LOG_LISTENER_INCONSISTENT(OSAPI_LOGKIND_ERROR,
                                       DDSC_LOG_DATAWRITER_LISTENER,mask)
        goto done;
    }

    /* Initialize QoS */
    retcode = DDS_DataWriterQos_initialize(&datawriter->qos);
    if (retcode != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_INITIALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_QOS,retcode)
        goto done;
    }

    if (qos == &DDS_DATAWRITER_QOS_DEFAULT)
    {
#if INCLUDE_API_QOS
        DDS_Publisher_get_default_datawriter_qos(publisher, &datawriter->qos);
#else
        datawriter->qos = init_val;
#endif
    }
    else
    {
        if (DDS_DataWriterQos_copy(&datawriter->qos, qos) != DDS_RETCODE_OK)
        {
            DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_QOS)
            goto done;
        }
    }

    datawriter->topic = topic;
    datawriter->type_plugin = DDS_TypeImpl_get_plugin(
                                    DDS_Topic_get_type(datawriter->topic));

    if (datawriter->type_plugin == NULL)
    {
        DDSC_LOG_LOOKUP_TYPE_PLUGIN(OSAPI_LOGKIND_ERROR,
                                    DDS_Topic_get_type(datawriter->topic))
        goto done;
    }

    DDS_LocatorSeq_initialize(&datawriter->uc_locator_seq);
    DDS_LocatorSeq_initialize(&datawriter->mc_locator_seq);

    datawriter->writer_data.unicast_locator = &datawriter->uc_locator_seq;
    datawriter->qos.data = &datawriter->writer_data;

    if (DDS_StringSeq_get_length(&datawriter->qos.transport.enabled_transports) > 0)
    {
        /* NOTE: No support for multicast writer locators */
        if (!DDS_LocatorSeq_set_maximum(&datawriter->uc_locator_seq,
                                   RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX))
        {
            goto done;
        }

        if (!DDS_LocatorSeq_set_length(&datawriter->uc_locator_seq,0))
        {
            goto done;
        }

        if (!NETIO_BindResolver_reserve_addresses(
             config->bind_resolver,*config->participant_id,
             config->enabled_transports,
             (struct REDA_StringSeq*)&datawriter->qos.transport.enabled_transports,
             NETIO_ROUTEKIND_USER,
             (struct NETIO_AddressSeq*)&datawriter->mc_locator_seq,
             (struct NETIO_AddressSeq*)&datawriter->uc_locator_seq))
        {
            DDSC_LOG_RESERVE_LOCATORS(OSAPI_LOGKIND_ERROR)
            goto done;
        }
    }

    key_kind = datawriter->type_plugin->get_key_kind(
                                datawriter->type_plugin,
                                datawriter->qos.type_support.plugin_data);

    datawriter->md5_stream = NULL;
    if (key_kind != NDDS_TYPEPLUGIN_NO_KEY)
    {
        serialized_key_size =
            datawriter->type_plugin->get_serialized_key_max_size(
                                    datawriter->type_plugin, 0,
                                    datawriter->qos.type_support.plugin_data);

        datawriter->md5_stream = CDR_Stream_alloc(serialized_key_size);
        if (datawriter->md5_stream == NULL)
        {
            DDSC_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_MD5STREAM_OBJECT)
            goto done;
        }
#ifdef RTI_ENDIAN_LITTLE
        CDR_Stream_byteswap_set(datawriter->md5_stream, RTI_TRUE);
#else
        CDR_Stream_byteswap_set(datawriter->md5_stream, RTI_FALSE);
#endif
    }

    if (!DDS_EntityImpl_initialize(&datawriter->as_entity,
                                   DDS_DATAWRITER_ENTITY_KIND,
                                   object_id,DDS_DataWriter_enable,
                                   DDS_DataWriterImpl_get_instance_handle))
    {
        DDSC_LOG_ENTITY_INITIALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_ENTITY)
        goto done;
    }

    datawriter->config = config;
    datawriter->publisher = publisher;

    if (listener == NULL)
    {
        datawriter->listener = nil_listener;
    }
    else
    {
        datawriter->listener = *listener;
    }
    datawriter->mask = mask;

    /* REDA and DDS sequence numbers are identical in layout. Use copy
     * to avoid aliasing warning.
     */
    OSAPI_Memory_copy((void*)&datawriter->last_sn,
                      (void*)&datawriter->
                      qos.protocol.rtps_reliable_writer.first_write_sequence_number,
                      sizeof(struct REDA_SequenceNumber));

    REDA_SequenceNumber_minusminus(&datawriter->last_sn);

    datawriter->max_cdr_serialized_length =
            datawriter->type_plugin->get_serialized_sample_max_size(
                    datawriter->type_plugin, 4,
                    datawriter->qos.type_support.plugin_data);

    /* The data-path in Micro 2.4 and under is hard-coded to either:
     *
     * DDS<->RTPS<->UDP (UDP transport)
     * DDS<->DDS (intra transport)
     *
     * The intra transport does not require any additional header/trailer
     * in the packet since the packet is not serialized/deserialized into
     * the packet payload.
     *
     * For the UDP transport the maximum overhead on a serialized sample is
     * calculated as the space needed to send a serialized sample encapsulated
     * in RTPS and then encapsulated in UDP. The constants are defined in the
     * netio_common.h header-file until this information can be retrieved by
     * calling an interface method on each interface.
     */
    datawriter->max_packet_length = RTPS_PACKET_HEADER_MAX_LENGTH +
                                    UDP_PACKET_HEADER_MAX_LENGTH  +
                                    datawriter->max_cdr_serialized_length +
                                    RTPS_PACKET_TRAILER_MAX_LENGTH +
                                    UDP_PACKET_TRAILER_MAX_LENGTH;

    cdr_property.buffer_size = sizeof(struct RTI_TransformCDR_Sample);
    cdr_property.max_buffers = (RTI_SIZE_T)datawriter->qos.resource_limits.max_samples;
    cdr_property.flags = 0;

    datawriter->cdr_samples = REDA_BufferPool_new("cdr_samples",
       &cdr_property, DDS_DataWriter_cdr_initialize, datawriter, NULL, NULL);

    if (datawriter->cdr_samples == NULL)
    {
        DDSC_LOG_CDR_POOL_ALLOC(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_CDR,
                            cdr_property.buffer_size,cdr_property.max_buffers)
        goto done;
    }

    /* If a copy of the serialized data is needed, allocate sufficient
     * resources, otherwise allocate one buffer. A buffer-pool with 1
     * entry may seem redundant, but it keeps the logic elsewhere clean
     * and unified and enabled a shared buffer-pool too.
     */
    cdr_property.buffer_size = datawriter->max_packet_length;
    if ((datawriter->qos.reliability.kind == DDS_RELIABLE_RELIABILITY_QOS) &&
         datawriter->qos.protocol.serialize_on_write)
    {
        cdr_property.max_buffers = (RTI_SIZE_T)datawriter->qos.resource_limits.max_samples;
    }
    else
    {
        cdr_property.max_buffers = 1;
    }

    datawriter->cdr_payloads = REDA_BufferPool_new("cdr_payloads",
       &cdr_property, DDS_DataWriter_cdr_initialize, datawriter, NULL, NULL);

    if (datawriter->cdr_payloads == NULL)
    {
        DDSC_LOG_CDR_POOL_ALLOC(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_CDR,
                            cdr_property.buffer_size,cdr_property.max_buffers)
        goto done;
    }

#ifdef RTI_ENDIAN_LITTLE
    CDR_Stream_byteswap_set(&datawriter->stream, RTI_TRUE);
#else
    CDR_Stream_byteswap_set(&datawriter->stream, RTI_FALSE);
#endif

    wh_listener.on_sample_removed = DDS_DataWriterEvent_on_sample_removed;
#ifndef RTI_CERT
    wh_listener.on_key_removed = DDS_DataWriterEvent_on_key_removed;
#endif
    wh_listener.on_deadline_missed = DDS_DataWriterEvent_on_deadline_missed;
    wh_listener.listener_data = (void *)datawriter;

    factory = RT_Registry_lookup(datawriter->config->registry,
                                 DDSHST_WRITER_DEFAULT_HISTORY_NAME);
    if (factory == NULL)
    {
        DDSC_LOG_COMPONENT_LOOKUP(OSAPI_LOGKIND_ERROR,
                                  DDSHST_WRITER_DEFAULT_HISTORY_NAME)
        goto done;
    }

    wh_property.qos = &datawriter->qos;
    datawriter->wh = DDSHST_WriterFactory_create_component(factory,
                                                       &wh_property._parent,
                                                       &wh_listener._parent);
    if (datawriter->wh == NULL)
    {
        DDSC_LOG_COMPONENT_CREATE(OSAPI_LOGKIND_ERROR,
                                  DDSC_LOG_WRITERHISTORY_COMPONENT,
                                  DDSHST_WRITER_DEFAULT_HISTORY_NAME)
        goto done;
    }

    /* DataWriter Interface */
    factory = RT_Registry_lookup(datawriter->config->registry,
                                 DDS_DEFAULT_DATAWRITER_NETIO_NAME);
    if (factory == NULL)
    {
        DDSC_LOG_COMPONENT_LOOKUP(OSAPI_LOGKIND_ERROR,
                                  DDS_DEFAULT_DATAWRITER_NETIO_NAME)
        goto done;
    }

    dwintf_property._parent._parent.db = datawriter->config->db;

    dwintf_property._parent.max_routes =
                    (RTI_SIZE_T)(DDS_DATAWRITER_MAX_DESTINATION_PER_READER *
                    datawriter->qos.writer_resource_limits.max_remote_readers);
    dwintf_property.datawriter = datawriter;
    dwintf_property._parent.max_binds =
        (RTI_SIZE_T)datawriter->qos.writer_resource_limits.max_remote_readers;
    instance_handle = DDS_DataWriterImpl_get_instance_handle(
                                                    (DDS_Entity*)datawriter);

    NETIO_Address_set_guid(&dwintf_property.intf_address,
                          0,(struct NETIO_Guid*)instance_handle.octet);

    datawriter->dw_intf = NETIO_InterfaceFactory_create_component(factory,
                            &dwintf_property._parent._parent,NULL);
    if (datawriter->dw_intf == NULL)
    {
        DDSC_LOG_COMPONENT_CREATE(OSAPI_LOGKIND_ERROR,
                                  DDSC_LOG_DATAWRITERIO_COMPONENT,
                                  DDS_DEFAULT_DATAWRITER_NETIO_NAME)
        goto done;
    }

    factory = RT_Registry_lookup(datawriter->config->registry,
                                 NETIO_DEFAULT_RTPS_NAME);
    if (factory == NULL)
    {
        DDSC_LOG_COMPONENT_LOOKUP(OSAPI_LOGKIND_ERROR,
                                  NETIO_DEFAULT_RTPS_NAME)
        goto done;
    }

    NETIO_Address_set_guid(&rtps_property.intf_address,
                           (RTI_UINT32)*datawriter->config->domain_id,
                           (struct NETIO_Guid*)instance_handle.octet);
    rtps_property._parent._parent.db = datawriter->config->db;
    rtps_property.anonymous = datawriter->qos.management.is_anonymous;

    rtps_property._parent.max_routes =
              (RTI_SIZE_T)(datawriter->qos.writer_resource_limits.max_routes_per_reader *
                           datawriter->qos.writer_resource_limits.max_remote_readers);

    rtps_property.mode = RTPS_INTERFACEMODE_WRITER;
    rtps_property.reliable =
        (datawriter->qos.reliability.kind == DDS_RELIABLE_RELIABILITY_QOS);

    /* Default window size */
    if (DDS_RTPSRELIABLEWRITER_DEFAULT_SEND_WINDOW ==
        datawriter->qos.protocol.rtps_reliable_writer.max_send_window)
    {
        rtps_property.max_window_size = (RTI_UINT32)
            (datawriter->qos.history.depth *
             datawriter->qos.resource_limits.max_instances);
    }
    else
    {
        rtps_property.max_window_size = (RTI_UINT32)
            datawriter->qos.protocol.rtps_reliable_writer.max_send_window;
    }

    if (rtps_property.max_window_size > RTPS_RECEIVE_WINDOW_MAX_SIZE)
    {
        rtps_property.max_window_size = RTPS_RECEIVE_WINDOW_MAX_SIZE;
    }

    rtps_property.max_hb_retries =
        datawriter->qos.protocol.rtps_reliable_writer.max_heartbeat_retries;

    rtps_property.max_peer_count =
        datawriter->qos.writer_resource_limits.max_remote_readers;

    /* Note: for this "local" bind, max_binds could in theory just be set to the
     * max number of upstream interfaces.  However, in RTPS implementation,
     * the bind table is used to track src-peer mappings, and thus has a record
     * per src-peer pair and must be set to max peers (max_remote_readers).
     */
    rtps_property._parent.max_binds =
        (RTI_SIZE_T)datawriter->qos.writer_resource_limits.max_remote_readers;

    DDS_Duration_to_ntp_time(
       &datawriter->qos.protocol.rtps_reliable_writer.heartbeat_period,
       &rtps_property.hb_period);

    if (datawriter->qos.protocol.rtps_reliable_writer.
        heartbeats_per_max_samples == 0)
    {
        rtps_property.samples_per_hb = 0;
    }
    else
    {
        if (datawriter->qos.resource_limits.max_samples == DDS_LENGTH_UNLIMITED)
        {
            rtps_property.samples_per_hb =
                (RTI_SIZE_T)(DDS_INFINITE_MAX_SAMPLES /
                             datawriter->qos.protocol.rtps_reliable_writer.heartbeats_per_max_samples);
        }
        else
        {
            rtps_property.samples_per_hb =
                    (RTI_SIZE_T)(datawriter->qos.resource_limits.max_samples /
                                 datawriter->qos.protocol.rtps_reliable_writer.heartbeats_per_max_samples);
        }
    }

    rtps_property._parent._parent.timer = datawriter->config->timer;

#if OSAPI_ENABLE_TRACE
    rtps_property.session_name =
            DDS_TopicDescription_get_name(
                    DDS_Topic_as_topicdescription(datawriter->topic));
#endif

    datawriter->rtps_intf = NETIO_InterfaceFactory_create_component(factory,
                                    &rtps_property._parent._parent,NULL);
    if (datawriter->rtps_intf == NULL)
    {
        DDSC_LOG_COMPONENT_CREATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_RTPS_COMPONENT,
                                  NETIO_DEFAULT_RTPS_NAME)
        goto done;
    }

    datawriter->as_entity.state = RTIDDS_ENTITY_STATE_CREATED;

    retval = DDS_BOOLEAN_TRUE;

done:
#ifndef RTI_CERT
    if (!retval)
    {
        (void)DDS_DataWriterImpl_finalize(datawriter);
    }
#endif
    return retval;
}

/*ci
 * \brief Refresh the datawriter liveliness
 *
 * \details
 * If the datawriter offers finite liveliness a timer is started, and if
 * the timer expires the datawriter has failed to maintain its liveliness.
 * This function refreshes the liveliess timer and is called when the
 * datawriter does something considered to be an indication of being alive,
 * such as writing samples.
 *
 * \param[in] self Datawriter to update liveliness on
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
DDS_DataWriter_update_liveliness(DDS_DataWriter *self)
{
    struct DDS_DataWriterImpl *datawriter = (struct DDS_DataWriterImpl *)self;

    if (!DDS_Duration_is_infinite(&datawriter->qos.liveliness.lease_duration))
    {
        return OSAPI_Timer_update_timeout(self->config->timer,
                &datawriter->liveliness_event,
                datawriter->qos.liveliness.lease_duration.sec,
                (RTI_INT32)datawriter->qos.liveliness.lease_duration.nanosec);

    }

    return RTI_TRUE;
}

/*ci
 * \brief Serialize a sample
 *
 * \details
 * This function serializes a sample and readies it for transmission. The
 * function works in two ways. If the sample payload is NULL, it allocates the
 * payload and serializes the sample. If the sample payload is != NULL, it
 * assumes that the payload is already serialized and there is no further work
 * done.
 *
 * \param[in] self    Datawriter that is writing the sample
 * \param[in] sample  Sample to write, NULL if no data
 * \param[in] packet  The packet to seralize data to.
 * \param[in] sn      The sequence number of the packet.
 *
 * \return DDS_RETCODE_OK on success, one of the \ref DDS_ReturnCode_t on
 *         failure
 *
 * \sa \ref DDS_DataWriter_write, \ref DDS_DataWriter_write_w_timestamp,
 *     \ref DDS_DataWriter_unregister_instance,
 *     \ref DDS_DataWriter_unregister_instance_w_timestamp,
 *     \ref DDS_DataWriter_dispose, \ref DDS_DataWriter_dispose_w_timestamp
 */
DDS_ReturnCode_t
DDS_DataWriter_serialize_sample(struct DDS_DataWriterImpl *self,
                                struct RTI_TransformCDR_Sample *sample,
                                NETIO_Packet_T *packet,
                                struct REDA_SequenceNumber *sn)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    struct NETIO_PacketInfo *pkt_info;
    DDS_UnsignedShort pid_id;
    DDS_UnsignedShort pid_length;
    RTI_INT32 real_payload_length;
    DDS_InstanceHandle_t ih;

    pkt_info = NETIO_Packet_get_info(packet);

    if (sample->payload == NULL)
    {
        sample->payload = REDA_BufferPool_get_buffer(self->cdr_payloads);
        if (sample->payload == NULL)
        {
            DDSC_LOG_RESOURCE_EXCEEDED(OSAPI_LOGKIND_ERROR,DDSC_LOG_SAMPLE_RESOURCES)
            return DDS_RETCODE_OUT_OF_RESOURCES;
        }

        if (!NETIO_Packet_initialize(packet,sample->payload,
                self->max_packet_length,
                RTPS_PACKET_TRAILER_MAX_LENGTH +
                UDP_PACKET_TRAILER_MAX_LENGTH,NULL))
        {
            DDSC_LOG_PACKET_INIT(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_NETIO_KIND)
            goto done;
        }


        /* First serialize inline qos */
        if (!NETIO_Packet_set_head(packet,
                0 - ((RTI_INT32)self->max_cdr_serialized_length + DDS_INLINE_QOS_SIZE)))
        {
            DDSC_LOG_PACKET_SET_HEAD(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_NETIO_KIND,
                    0 - ((RTI_INT32)self->max_cdr_serialized_length + DDS_INLINE_QOS_SIZE))
            goto done;
        }

        CDR_Stream_reset(&self->stream);
        CDR_Stream_set_vendor(&self->stream,
                              CDR_VENDOR_ID_MAJOR_RTI,
                              CDR_VENDOR_ID_MINOR_MICRO);

        if (!CDR_Stream_set_buffer(&self->stream,
                NETIO_Packet_get_head(packet),
                (self->max_cdr_serialized_length + DDS_INLINE_QOS_SIZE)))
        {
            DDSC_LOG_CDR_BUFFER_SET(OSAPI_LOGKIND_ERROR,
                    DDSC_LOG_DATAWRITER_NETIO_KIND,&self->stream,
                    NETIO_Packet_get_head(packet),
                    self->max_cdr_serialized_length +
                    DDS_INLINE_QOS_SIZE)
                goto done;
        }

        if (sample->sample_info.send_key_hash)
        {
            pid_id = RTPS_PID_KEY_HASH;

            /* The RTPS specification says the length is always 16, regardless
             * of how many bytes are sent. However, at this point in the code
             * always use the actual length to make it correct in case the
             * specification changes.
             */
            pid_length = (DDS_UnsignedShort)sample->key_hash.length;

            if (!CDR_Stream_serialize_unsigned_short(&self->stream, &pid_id))
            {
                DDSC_LOG_CDR_SERIALIZE_PID(OSAPI_LOGKIND_ERROR,
                        DDSC_LOG_DATAWRITER_CDR,(RTI_INT32)pid_id)
                    goto done;
            }

            if (!CDR_Stream_serialize_unsigned_short(&self->stream, &pid_length))
            {
                DDSC_LOG_CDR_SERIALIZE_PID_LENGTH(OSAPI_LOGKIND_ERROR,
                        DDSC_LOG_DATAWRITER_CDR,(RTI_INT32)pid_length)
                    goto done;
            }

            if (!CDR_Stream_serialize_byte_array(&self->stream,
                    sample->key_hash.value, sample->key_hash.length))
            {
                DDSC_LOG_CDR_SERIALIZE_KEYHASH(OSAPI_LOGKIND_ERROR,
                        DDSC_LOG_DATAWRITER_CDR,sample->key_hash.length)
                    goto done;
            }
        }

        if (sample->sample_info.status_info &
                (RTPS_DISPOSE_STATUS_INFO | RTPS_UNREGISTER_STATUS_INFO))
        {
            pid_id = RTPS_PID_STATUS_INFO;
            pid_length = 4;

            if (!CDR_Stream_serialize_unsigned_short(&self->stream, &pid_id))
            {
                DDSC_LOG_CDR_SERIALIZE_PID(OSAPI_LOGKIND_ERROR,
                        DDSC_LOG_DATAWRITER_CDR,(RTI_INT32)pid_id)
                    goto done;
            }

            if (!CDR_Stream_serialize_unsigned_short(&self->stream, &pid_length))
            {
                DDSC_LOG_CDR_SERIALIZE_PID_LENGTH(OSAPI_LOGKIND_ERROR,
                        DDSC_LOG_DATAWRITER_CDR,(RTI_INT32)pid_length)
                    goto done;
            }

            if (!CDR_Stream_serialize_unsigned_long_to_big_endian(
                    &self->stream, &sample->sample_info.status_info))
            {
                DDSC_LOG_CDR_SERIALIZE_STATUS_INFO(OSAPI_LOGKIND_ERROR,
                        DDSC_LOG_DATAWRITER_CDR,
                        sample->sample_info.status_info)
                    goto done;
            }
        }

        if (CDR_Stream_get_current_position_offset(&self->stream) > 0)
        {
            pid_id = RTPS_PID_SENTINEL;
            pid_length = 0;

            if (!CDR_Stream_serialize_unsigned_short(&self->stream, &pid_id))
            {
                DDSC_LOG_CDR_SERIALIZE_PID(OSAPI_LOGKIND_ERROR,
                        DDSC_LOG_DATAWRITER_CDR,(RTI_INT32)pid_id)
                    goto done;
            }

            if (!CDR_Stream_serialize_unsigned_short(&self->stream, &pid_length))
            {
                DDSC_LOG_CDR_SERIALIZE_PID_LENGTH(OSAPI_LOGKIND_ERROR,
                        DDSC_LOG_DATAWRITER_CDR,(RTI_INT32)pid_length)
                    goto done;
            }

            pkt_info->rtps_flags |= NETIO_RTPS_FLAGS_INLINEQOS;
        }

        if ((sample->instance_data != NULL) &&
                (sample->sample_info.status_info == RTPS_NO_STATUS_INFO))
        {
            if (!CDR_Stream_serialize_header(&self->stream,
                    self->type_plugin->get_key_kind(self->type_plugin,
                            self->qos.type_support.plugin_data) == NDDS_TYPEPLUGIN_GUID_KEY))
            {
                DDSC_LOG_CDR_SERIALIZE_DATA(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_CDR)
                    goto done;
            }

            if (!self->type_plugin->serialize_data(&self->stream,
                    sample->instance_data,
                    self->qos.type_support.plugin_data))
            {
                DDSC_LOG_CDR_SERIALIZE_DATA(OSAPI_LOGKIND_ERROR,
                        DDSC_LOG_DATAWRITER_CDR)
                    goto done;
            }
            pkt_info->rtps_flags |= NETIO_RTPS_FLAGS_DATA;
        }

        if (sample->sample_info.status_info & RTPS_DISPOSE_STATUS_INFO)
        {
            pkt_info->rtps_flags |= NETIO_RTPS_FLAGS_DISPOSE;
        }

        if (sample->sample_info.status_info & RTPS_UNREGISTER_STATUS_INFO)
        {
            pkt_info->rtps_flags |= NETIO_RTPS_FLAGS_UNREGISTER;
        }

        real_payload_length = (((RTI_INT32)self->max_cdr_serialized_length +
                DDS_INLINE_QOS_SIZE) -
                (RTI_INT32)CDR_Stream_get_current_position_offset(&self->stream));

        /* NOTE: DDS does not guarantee that the tail is on an even boundary.
         * Although RTPS requires this, it may not be required by other protocols.
         */
        if (!NETIO_Packet_set_tail(packet, 0 - real_payload_length))
        {
            DDSC_LOG_PACKET_SET_TAIL(OSAPI_LOGKIND_ERROR,
                    DDSC_LOG_DATAWRITER_NETIO_KIND,real_payload_length)
                goto done;
        }

        NETIO_Packet_save_positions_to(packet,&sample->head,&sample->tail);
        sample->rtps_flags = pkt_info->rtps_flags;
    }
    else
    {
        if (!NETIO_Packet_initialize(packet,sample->payload,
                self->max_packet_length,
                RTPS_PACKET_TRAILER_MAX_LENGTH +
                UDP_PACKET_TRAILER_MAX_LENGTH,NULL))
        {
            DDSC_LOG_PACKET_INIT(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_NETIO_KIND)
                goto done;
        }
        NETIO_Packet_restore_positions_from(packet,sample->head,sample->tail);
    }

    /* General protocol data */
    packet->ref = sample;
    pkt_info->sn = *sn;
    pkt_info->rtps_flags = sample->rtps_flags;
    pkt_info->timestamp = sample->sample_info.timestamp;
    pkt_info->valid_data = ((sample->instance_data != NULL) &&
            (sample->sample_info.status_info == RTPS_NO_STATUS_INFO)) ? 1 : 0;

    /* General protocol data */
    OSAPI_Memory_copy(&pkt_info->instance, sample->key_hash.value,
                      RTPS_KEY_HASH_MAX_LENGTH);
    
    ih = DDS_Entity_get_instance_handle(DDS_DataWriter_as_entity(self));
    NETIO_Address_set_guid_from_array(&packet->source,0,ih.octet);

    /* Add intra-transport information. Since Micro does not support multi-
     * encapsulation this is the only option. Since intra-transport is
     * a synchronous call the data pointer is valid on the receiving end
     */
    pkt_info->protocol_id = NETIO_PROTOCOL_INTRA;
    pkt_info->protocol_data.intra_info.user_data = (void*)sample->instance_data;

    retcode = DDS_RETCODE_OK;

done:

    if (retcode != DDS_RETCODE_OK)
    {
        REDA_BufferPool_return_buffer(self->cdr_payloads,sample->payload);
        sample->payload = NULL;
    }

    return retcode;
}

/*ci
 * \brief Write an untyped sample
 *
 * \details
 * This is a generic, untyped function that can write a sample of any type,
 * with or without an instance-handle and with or with data. Additional
 * meta-data about the sample is passed in, such as a timestamp.
 *
 * This function
 * \param[in] self           Datawriter that is writing the sample
 * \param[in] instance_data  Sample to write, NULL if no data
 * \param[in] handle         The instance handle for the sample, if any
 * \param[in] sample_info    Meta-data about the sample
 *
 * \return DDS_RETCODE_OK on success, one of the \ref DDS_ReturnCode_t on
 *         failure
 *
 * \sa \ref DDS_DataWriter_write, \ref DDS_DataWriter_write_w_timestamp,
 *     \ref DDS_DataWriter_unregister_instance,
 *     \ref DDS_DataWriter_unregister_instance_w_timestamp,
 *     \ref DDS_DataWriter_dispose, \ref DDS_DataWriter_dispose_w_timestamp
 */
DDS_ReturnCode_t
DDS_DataWriter_write_untyped(DDS_DataWriter *self,
                             const void *instance_data,
                             const DDS_InstanceHandle_t *handle,
                             struct NDDS_DataWriterSampleInfo *sample_info)
{
    struct DDS_DataWriterImpl *datawriter = (struct DDS_DataWriterImpl *)self;
    DDS_ReturnCode_t retcode = DDS_RETCODE_PRECONDITION_NOT_MET;
    struct RTI_TransformCDR_Sample *sample = NULL;
    DDSHST_WriterSampleEntryRef_T sample_entry = NULL;
    struct DDSHST_WriterState *wh_state;
    struct NETIO_Address destination = NETIO_Address_INITIALIZER;
    DDS_InstanceHandle_t actual_instance = DDS_HANDLE_NIL_NATIVE;
    struct NETIO_PacketInfo *pkt_info;
    DDS_KeyHash_t key_hash_buf = DDS_KEY_HASH_DEFAULT;
    NDDS_TypePluginKeyKind key_kind;
    DDSHST_WriterEntryKind_T entry_kind;
    RTI_BOOL assert_key = RTI_TRUE;
    DDS_InstanceHandle_t ih;
    struct DDSHST_InstanceState key_state;

    key_kind = datawriter->type_plugin->get_key_kind(self->type_plugin,
                                    datawriter->qos.type_support.plugin_data);

    if ((key_kind != NDDS_TYPEPLUGIN_NO_KEY) &&
        (sample_info->send_key_hash) &&
        (DDS_InstanceHandle_is_nil(handle) && (instance_data == NULL)))
    {
        return DDS_RETCODE_BAD_PARAMETER;
    }

    if (!DDS_Entity_is_enabled(&self->as_entity))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_ENTITY)
        return DDS_RETCODE_NOT_ENABLED;
    }

    if (!DDS_DataWriter_update_liveliness(self))
    {
        return DDS_RETCODE_ERROR;
    }

    if ((key_kind == NDDS_TYPEPLUGIN_USER_KEY) ||
        (key_kind == NDDS_TYPEPLUGIN_GUID_KEY))
    {
        if (DDS_InstanceHandle_is_nil(handle))
        {
            key_hash_buf.length = RTPS_KEY_HASH_MAX_LENGTH;
            OSAPI_Memory_zero(key_hash_buf.value, key_hash_buf.length);
            OSAPI_Memory_zero(&actual_instance.octet, RTPS_KEY_HASH_MAX_LENGTH);
            if ((key_kind == NDDS_TYPEPLUGIN_USER_KEY) ||
                (self->as_entity.entity_id ==
                                        RTPS_OBJECT_ID_WRITER_SDP_PARTICIPANT))
            {
                CDR_Stream_reset(self->md5_stream);
                CDR_Stream_set_vendor(&self->stream,
                                      CDR_VENDOR_ID_MAJOR_RTI,
                                      CDR_VENDOR_ID_MINOR_MICRO);
                if (!datawriter->type_plugin->instance_to_keyhash(
                        datawriter->type_plugin,self->md5_stream,&key_hash_buf,
                        instance_data, datawriter->qos.type_support.plugin_data))
                {
                    DDSC_LOG_DW_KEYHASH_CREATE(OSAPI_LOGKIND_ERROR)
                        return DDS_RETCODE_PRECONDITION_NOT_MET;
                }
            }
            else
            {
                if (self->as_entity.entity_id == RTPS_OBJECT_ID_READER_SDP_PUBLICATION)
                {
                    ih = DDS_Entity_get_instance_handle(DDS_DataReader_as_entity(instance_data));
                }
                else
                {
                    ih = DDS_Entity_get_instance_handle(DDS_DataWriter_as_entity(instance_data));
                }
                OSAPI_Memory_copy(key_hash_buf.value, ih.octet, RTPS_KEY_HASH_MAX_LENGTH);
            }
            OSAPI_Memory_copy(&actual_instance.octet, &key_hash_buf.value,
                              key_hash_buf.length);
        }
        else
        {
            OSAPI_Memory_copy(key_hash_buf.value,handle,RTPS_KEY_HASH_MAX_LENGTH);
            key_hash_buf.length =
                datawriter->type_plugin->get_serialized_key_max_size(
                        datawriter->type_plugin,0,
                        datawriter->qos.type_support.plugin_data);

            /* The generated code gives the _max_ size which could be
             * larger than RTPS_KEY_HASH_MAX_LENGTH. In that case the length to
             * RTPS_KEY_HASH_MAX_LENGTH since that is the max key hash that
             * is sent. For keyhashes < RTPS_KEY_HASH_MAX_LENGTH the keyhash
             * is padded to 0s to always be of length RTPS_KEY_HASH_MAX_LENGTH;
             */
            key_hash_buf.length = RTPS_KEY_HASH_MAX_LENGTH;
            actual_instance = *handle;
        }
    }
    else if (key_kind != NDDS_TYPEPLUGIN_NO_KEY)
    {
        DDSC_LOG_DW_ILLEGAL_KEY_KIND(OSAPI_LOGKIND_ERROR,key_kind)
        return DDS_RETCODE_PRECONDITION_NOT_MET;
    }

    if ((sample_info->status_info & (RTPS_DISPOSE_STATUS_INFO |
                                    RTPS_UNREGISTER_STATUS_INFO)) ==
           (RTPS_DISPOSE_STATUS_INFO | RTPS_UNREGISTER_STATUS_INFO))
    {
        entry_kind = DDSHST_WRITER_ENTRY_UNREGISTER_DISPOSE;
    }
    else if (sample_info->status_info & RTPS_UNREGISTER_STATUS_INFO)
    {
        entry_kind = DDSHST_WRITER_ENTRY_UNREGISTER;
    }
    else if (sample_info->status_info & RTPS_DISPOSE_STATUS_INFO)
    {
        entry_kind = DDSHST_WRITER_ENTRY_DISPOSE;
    }
    else
    {
        entry_kind = DDSHST_WRITER_ENTRY_NORMAL;
    }

    /* If we need to save the sample, add it to the history queue. It would be
     * nice to store the same in case of best-effort and send it once also
     * for late joiners.
     */
    actual_instance.is_valid = DDS_BOOLEAN_TRUE;

    /* Allocate a queue entry. There are three cases:
     * 1. write() - If the handle is not NIL, use it. If it is NIL a key
     *    is calculated and it is auto-registered (if there are resources)
     *
     * 2. dispose() - If the handle is supplied then only allow
     *    the operation to complete if the key already exists. If the handle
     *    is NIL then auto-registration is done and a key may be reserved even
     *    if it does not exist.
     *
     * 2. unregister() - If the handle is supplied then only allow
     *    the operation to complete if the key already exists. If the handle
     *    is NIL then auto-registration is done, but an entry cannot be added
     *    unless it already exists.
     *
     * The default behavior is to allow a key to be added even if it does not
     * exist. The exception is if a non nil-handle is used or if an instance
     * is unregistered.
     */
    if (!DDS_InstanceHandle_is_nil(handle) ||
        (sample_info->status_info == RTPS_UNREGISTER_STATUS_INFO))
    {
        assert_key = RTI_FALSE;
    }

    /* If an instance has been registered/asserted but has not been published,
     * then check if a dispose/unregister should still be sent. This feature
     * is useful when instances resources are preallocated, but not actually
     * used.
     */
    if ((sample_info->status_info & (RTPS_DISPOSE_STATUS_INFO |
                                     RTPS_UNREGISTER_STATUS_INFO)) &&
         self->qos.management.disable_unregister_dispose_for_unpublished_instance)
    {
        if (DDS_InstanceHandle_is_nil(handle))
        {
            return DDS_RETCODE_BAD_PARAMETER;
        }

        if (DDSHST_Writer_get_instance_state(self->wh,
                      &actual_instance,&key_state) != DDSHST_RETCODE_SUCCESS)
        {
            return DDS_RETCODE_BAD_PARAMETER;
        }

        if (key_state.sample_count == 0)
        {
            /* Unregistering the key is a local operation and immediate */
            if (DDSHST_Writer_unregister_key(self->wh,&actual_instance) !=
                    DDSHST_RETCODE_SUCCESS)
            {
                return DDS_RETCODE_BAD_PARAMETER;
            }

            return DDS_RETCODE_OK;
        }

        /* Getting here means that at least one sample has been published
         * for the instance and unregister/dispose must proceed as normal
         * regardless of disable_unregister_dispose_for_unpublished_instance
         */
    }

    sample_entry = DDSHST_Writer_get_entry(self->wh, &actual_instance,
                                           entry_kind,assert_key,
                                           &sample_info->timestamp);
    if ((sample_entry == NULL) && assert_key)
    {
        DDSC_LOG_RESOURCE_EXCEEDED(OSAPI_LOGKIND_ERROR,DDSC_LOG_HISTORY_RESOURCE)
        return DDS_RETCODE_OUT_OF_RESOURCES;
    }

    if (sample_entry == NULL)
    {
        /* The key did not exist, return here */
        return DDS_RETCODE_BAD_PARAMETER;
    }

    sample = (struct RTI_TransformCDR_Sample*)
                                  REDA_BufferPool_get_buffer(self->cdr_samples);
    if (sample == NULL)
    {
        DDSC_LOG_RESOURCE_EXCEEDED(OSAPI_LOGKIND_ERROR,DDSC_LOG_SAMPLE_RESOURCES)
        DDSHST_Writer_return_entry(self->wh, sample_entry);
        return DDS_RETCODE_OUT_OF_RESOURCES;
    }


    /* Anonymous writers may re-publish the same sample, e.g for discovery */
    if (!self->qos.management.is_anonymous)
    {
        REDA_SequenceNumber_plusplus(&(self->last_sn));
    }

    sample->payload = NULL;
    sample->key_hash = key_hash_buf;
    sample->sample_info = *sample_info;
    sample->instance_data = instance_data;
    pkt_info = NETIO_Packet_get_info(&datawriter->packet);

    retcode = DDS_DataWriter_serialize_sample(datawriter,
                                            sample,
                                            &datawriter->packet,
                                            &self->last_sn);

    if (retcode != DDS_RETCODE_OK)
    {
        goto done;
    }

    if (DDSHST_Writer_commit_entry(self->wh,
            sample_entry, &sample->_sample,
            &self->last_sn,
            ((struct DDS_DataWriterInterface*)datawriter->dw_intf)->active_acking_readers) != DDSHST_RETCODE_SUCCESS)
    {
        retcode = DDS_RETCODE_ERROR;
        DDSC_LOG_DW_COMMIT(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    wh_state = DDSHST_Writer_get_state(datawriter->wh);
    pkt_info->committable_sn = wh_state->high_sn;
    pkt_info->first_available_sn = wh_state->low_sn;

    /* The protocol dictates that commitable is everything up to last SN + 1 */
    REDA_SequenceNumber_plusplus(&pkt_info->committable_sn);

    /* Note: destination currently is always unknown address, so sample is
     * always broadcast (at the DDS level, not at the transport level).
     */
    if (!NETIO_Interface_send(datawriter->dw_intf,
                              datawriter->dw_intf,&destination,
                              &datawriter->packet))
    {
        DDSC_LOG_NETIO_SEND_FAILED(OSAPI_LOGKIND_ERROR)
        retcode = DDS_RETCODE_ERROR;
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:

    /* Release the payload buffer after it has been sent unless if configured
     * not to. Note that this assumes the send call is synchronous.
     */
    if (!datawriter->qos.protocol.serialize_on_write)
    {
        if (sample->payload != NULL)
        {
            REDA_BufferPool_return_buffer(self->cdr_payloads, sample->payload);
            sample->payload = NULL;
        }
    }

    if (retcode != DDS_RETCODE_OK)
    {
        DDSHST_Writer_return_entry(self->wh, sample_entry);

        REDA_BufferPool_return_buffer(self->cdr_samples,sample);
    }

    return retcode;
}

/*ci
 * \brief Register a key in the datawriter
 *
 * \details
 * The key registration reserve an entry in the datawriter history-cache.
 * The function assumes that at least all the key fields are set to the
 * correct values, there is no attempt to determine whether a key field
 * is set or not.
 *
 * \param[in]  datawriter    Datawriter to register key in
 * \param[out] handle        Handle to the registered key
 * \param[in]  instance_data A sample with at least all the key fields set
 * \param[in]  timestamp     The timestamp when the key was registered,
 *                           not used
 * \return DDS_RETCODE_OK on success, one of the \ref DDS_ReturnCode_t
 *         on error
 */
DDS_ReturnCode_t
DDS_DataWriter_register_key(struct DDS_DataWriterImpl *datawriter,
                            DDS_InstanceHandle_t *handle,
                            const void *instance_data,
                            const struct OSAPI_NtpTime *timestamp)
{
    DDS_KeyHash_t keyHashBuffer = DDS_KEY_HASH_DEFAULT;
    DDSHST_ReturnCode_T whrc;

    if (datawriter->type_plugin->get_key_kind(datawriter->type_plugin,
          datawriter->qos.type_support.plugin_data) == NDDS_TYPEPLUGIN_GUID_KEY)
    {
        /* GUID keys */
        DDS_InstanceHandle_from_rtps(
           handle, (const struct RTPS_Guid *)instance_data);
    }
    else
    {
        /* user key */
        keyHashBuffer.length = RTPS_KEY_HASH_MAX_LENGTH;
        OSAPI_Memory_zero(keyHashBuffer.value, keyHashBuffer.length);

        if (!datawriter->type_plugin->instance_to_keyhash(
                datawriter->type_plugin, datawriter->md5_stream, &keyHashBuffer,
                instance_data, datawriter->qos.type_support.plugin_data))
        {
            DDSC_LOG_DW_KEYHASH_CREATE(OSAPI_LOGKIND_ERROR)
            return DDS_RETCODE_PRECONDITION_NOT_MET;
        }
        OSAPI_Memory_copy(&handle->octet,
                          &keyHashBuffer.value, keyHashBuffer.length);
        handle->is_valid = DDS_BOOLEAN_TRUE;
    }

    whrc = DDSHST_Writer_register_key(datawriter->wh, handle,timestamp);
    if (whrc != DDSHST_RETCODE_SUCCESS)
    {
        DDSC_LOG_DW_HISTORY_REGISTER_KEY(OSAPI_LOGKIND_ERROR)
        return DDS_RETCODE_OUT_OF_RESOURCES;
    }

    return DDS_RETCODE_OK;
}

/*ci
 * \brief Update the expected number of acknowledgments for a sample
 *
 * \details
 * The writer keeps track of how many reliable readers it is matched with
 * to determine how many acknowledgments are expected for a given
 * SN. This is again used to determine which samples can be purged if
 * resources need to be reclaimed.
 *
 * \param[in] self The datawriter to update the expected acknowledgment count
 *                 for
 */
void
DDS_DataWriter_update_historical_ackcount(DDS_DataWriter *self)
{
    struct DDS_DataWriterImpl *dw = (struct DDS_DataWriterImpl *)self;
    struct OSAPI_NtpTime now;
    struct DDSHST_WriterEvent event;
    RTI_BOOL bretval;

    event.kind = DDSHST_WRITEREVENT_KIND_HISTORICAL_DATA_REQUESTED;

    bretval = OSAPI_System_get_time(&now);

#if OSAPI_ENABLE_LOG
    /* Not important, can continue */
    if (!bretval)
    {
        DDSC_LOG_SYS_GETTIME(OSAPI_LOGKIND_ERROR)
    }
#else
    IGNORE_RETVAL(bretval);
#endif

    DDSHST_Writer_post_event(dw->wh,&event,&now);
}

/*ci
 * \brief Check if a DDS_DataWriterListener is consistent
 *
 * \param[in] l DDS_DataWriterListener to test for consistency
 * \param[in] m The current status mask to test
 *
 * \return DDS_BOOLEAN_TRUE if the listener is consistent, DDS_BOOLEAN_FALSE
 *         otherwise
 */
DDS_Boolean
DDS_DataWriterListener_is_consistent(const struct DDS_DataWriterListener *l,
                                     DDS_StatusMask m)
{
    return (((!((m) & DDS_OFFERED_DEADLINE_MISSED_STATUS)) ||
            (l->on_offered_deadline_missed != NULL)) &&
            ((!((m) & DDS_OFFERED_INCOMPATIBLE_QOS_STATUS)) ||
            (l->on_offered_incompatible_qos != NULL)) &&
            ((!((m) & DDS_LIVELINESS_LOST_STATUS)) ||
            (l->on_liveliness_lost != NULL)) &&
            ((!((m) & DDS_PUBLICATION_MATCHED_STATUS)) ||
            (l->on_publication_matched != NULL)) &&
            ((!((m) & DDS_RELIABLE_READER_ACTIVITY_CHANGED_STATUS)) ||
            (l->on_reliable_reader_activity_changed != NULL))) ?
            DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/*ci @} */
