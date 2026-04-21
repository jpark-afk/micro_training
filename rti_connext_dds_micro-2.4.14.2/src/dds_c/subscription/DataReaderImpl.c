/*
 * FILE: DataReaderImpl.c - DDS DataReader implementation
 *
 * (c) Copyright 2008-2015 Real-Time Innovations,
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
 * 17sep2021,tk MICRO-3276/PR.29664
 * - Added transport sequence in call to NETIO_BindResolver_reserve_addresses
 * 15apr2021,tk MICRO-2979/PR.27549
 * - Removed unused functions from CERT:
 *   DDS_DataReader_get_objectid
 * 06apr2021,tk MICRO-2916/PR.28866
 * - Added comment in _initalize() for why init_val is used instead of
 *   the value of DDS_DATAREADER_QOS_DEFAULT.
 * 04nov2015,tk  MICRO-1505 Reduce memory footprint
 * 17jul2015,tk  MICRO-1442/PR#15468 Pass plugin_data to
 *                                   get_serialized_key_max_size
 * 16mar2015,tk  MICRO-1123/PR#14259 Removed redundant if test
 * 19sep2014,tk  MICRO-877 Added checks for writer enabled where applicable
 * 16sep2014,as  MICRO-903/PR#11236 - Incorrect handling of status events and listeners
 * 08aug2014,eh  MICRO-867: max_window_size limit now set by RTPS
 * 31jul2014,tk  MICRO-842/PR#9683 - Removed superfluous paramaters in DB
 *                                  compare function
 * 19mar2014,tk  MICRO-74: Support endpoint specific transport
 * 11nov2013,as  MICRO-718 Add get_X_status() API to DataReader and DataWriter
 * 08nov2013,as  MICRO-681 Complete implementation of WaitSets
 *               and support for StatusConditions
 * 07feb2013,eh  MICRO-238: removed debug msg causing segfault
 * 06feb2013,eh  MICRO-262: assign RTPS resource limits
 * 08jun2012,tk  Refactored from DataReader.c
 */
/*ci
 * \file
 * \brief DDS DataReader implementation
 *
 * \details
 * This file implements internal functions needed to support the public
 * DDS datareader API, mainly related to the life-cycle of a DDS datareader
 * object.
 */
/*ci \addtogroup DDSSubscriptionModule
 * @{
 */
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef reda_bufferpool_h
#include "reda/reda_bufferpool.h"
#endif
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_discovery_plugin_h
#include "dds_c/dds_c_discovery_plugin.h"
#endif

#include "InstanceHandle.h"
#include "Conditions.h"
#include "QosPolicy.h"
#include "Entity.h"
#include "TopicDescription.h"
#include "Topic.h"
#include "Type.h"
#include "DataReaderQos.h"
#include "DataReaderEvent.h"
#include "DataReaderInterface.h"
#include "Conditions.h"
#include "DataReaderImpl.h"


const char* const DDS_DEFAULT_DATAREADER_NETIO_NAME = "ri";

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Initialize a CDR sample
 *
 * \details
 * This method is called by the bufferpool when the CDR bufferpool is created.
 * This function allocates a CDR sample by calling the type-plugin for the
 * datareader. A pointer to the sample is stored in the reader sample.
 *
 * \param[in] init_config Parameter that was passed to REDA_BufferPool_new()
 * \param[in] buffer      The buffer element to initialize
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DataReader_cdr_initialize(void *init_config, void *buffer)
{
    struct DDS_DataReaderImpl *dr = (struct DDS_DataReaderImpl *)init_config;
    struct RTI_DataReaderSample *sample = (struct RTI_DataReaderSample*)buffer;

    if (!dr->type_plugin->create_sample(dr->type_plugin,
                                        &sample->hst_sample._user_data,
                                        dr->qos.type_support.plugin_data))
    {
        DDSC_LOG_CDR_INITIALIZE_SAMPLE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_CDR)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalized a CDR sample
 *
 * \details
 * This method is called by the bufferpool when the CDR pool is deleted. This
 * function deletes the CDR sample by calling the type-plugin since the sample
 * was allocated by the type-plugin.
 *
 * \param[in] finalize_config Parameter that was passed to REDA_BufferPool_new()
 * \param[in] buffer          The buffer element to finalize
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DataReader_cdr_finalize(void *finalize_config, void *buffer)
{
    struct DDS_DataReaderImpl *dr = (struct DDS_DataReaderImpl *)finalize_config;
    struct RTI_DataReaderSample *sample = (struct RTI_DataReaderSample*)buffer;

    if (!dr->type_plugin->delete_sample(dr->type_plugin,
                                        sample->hst_sample._user_data,
                                        dr->qos.type_support.plugin_data))
    {
        DDSC_LOG_CDR_FINALIZE_SAMPLE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_CDR)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}
#endif /* !RTI_CERT */

/*ci
 * \brief Compare two datareader records
 *
 * \details
 * This function is used as a compare function for a database table and
 * confirms to the semantics for \ref DB_IndexCompare_T
 *
 * \param[in] flags Used to distinguish between op2 being a key or record
 * \param[in] op1   A DDS_DataReaderImpl already in the database
 * \param[in] op2   Either a DDS_DataReaderImpl being added or a key
 *                  being searched for
 *
 * \return positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2,
 *         zero if op1 is equal to op2
 */
RTI_INT32
DDS_DataReaderImpl_compare(RTI_INT32 flags,const DB_Record_T op1, void *op2)
{
    struct DDS_DataReaderImpl *record_left = (struct DDS_DataReaderImpl*)op1;
    const DDS_UnsignedLong *id_right;

    if (DB_SELECT_OP2_IS_KEY(flags))
    {
        id_right = (const DDS_UnsignedLong*)op2;
    }
    else
    {
        id_right = &((struct DDS_DataReaderImpl*)op2)->as_entity.entity_id;
    }

    return ((record_left->as_entity.entity_id == *id_right) ? 0 :
                 (record_left->as_entity.entity_id > *id_right ? 1 : -1));
}

#ifndef RTI_CERT
/*ci
 * \brief Return the object-id for a datareader
 *
 * \param[in] self The datareader to return the object id for
 *
 * \return The object id for the datareader
 */
DDS_UnsignedLong
DDS_DataReader_get_objectid(DDS_DataReader *self)
{
    return self->as_entity.entity_id;
}
#endif

#ifndef RTI_CERT
/*ci
 * \brief Finalize a datareader
 *
 * \details
 * Free up all resources used by a datareader. Note that this function does
 * not free the memory used to hold the datareader itself, this memory is
 * freed by the subscriber because the subscriber is the factory.
 *
 * \param[in] self Datareader to finalize
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DDS_DataReaderImpl_finalize(DDS_DataReader *self)
{
    struct DDS_DataReaderImpl *datareader = (struct DDS_DataReaderImpl *)self;
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    struct RT_ComponentFactory *factory;

#ifdef ENABLE_QOS_DEADLINE
    if ((datareader->config != NULL) && (datareader->config->timer != NULL) &&
         DDS_Entity_is_enabled(DDS_DataReader_as_entity(self)) &&
        !DDS_Duration_is_infinite(&datareader->qos.deadline.period))
    {
        if (!OSAPI_Timer_delete_timeout(datareader->config->timer,
                        &datareader->deadline_event))
        {
            goto done;
        }
    }
#endif

    if (DDS_LocatorSeq_get_length(&datareader->mc_locator_seq) > 0)
    {
        if (!NETIO_BindResolver_release_addresses(
                datareader->config->bind_resolver,
                datareader->config->enabled_transports,
                NETIO_ROUTEKIND_USER,
                (struct NETIO_AddressSeq*)&datareader->mc_locator_seq))
        {
            goto done;
        }
    }
    if (!DDS_LocatorSeq_finalize(&datareader->mc_locator_seq))
    {
        goto done;
    }

    if (DDS_LocatorSeq_get_length(&datareader->uc_locator_seq) > 0)
    {
        if (!NETIO_BindResolver_release_addresses(
                datareader->config->bind_resolver,
                datareader->config->enabled_transports,
                NETIO_ROUTEKIND_USER,
                (struct NETIO_AddressSeq*)&datareader->uc_locator_seq))
        {
            goto done;
        }
    }
    if (!DDS_LocatorSeq_finalize(&datareader->uc_locator_seq))
    {
        goto done;
    }

    if (datareader->md5_stream != NULL)
    {
        CDR_Stream_free(datareader->md5_stream);
    }

    if (self->_rh != NULL)
    {
        factory = RT_Registry_lookup(datareader->config->registry,
                                     DDSHST_READER_DEFAULT_HISTORY_NAME);
        if (factory == NULL)
        {
            DDSC_LOG_COMPONENT_LOOKUP(OSAPI_LOGKIND_ERROR,
                                      DDSHST_READER_DEFAULT_HISTORY_NAME)
            goto done;
        }
        DDSHST_ReaderFactory_delete_component(factory, self->_rh);
        self->_rh = NULL;
    }

    if (datareader->dr_intf != NULL)
    {
        factory = RT_Registry_lookup(datareader->config->registry,
                                     DDS_DEFAULT_DATAREADER_NETIO_NAME);
        if (factory == NULL)
        {
            DDSC_LOG_COMPONENT_LOOKUP(OSAPI_LOGKIND_ERROR,DDS_DEFAULT_DATAREADER_NETIO_NAME)
            goto done;
        }
        NETIO_InterfaceFactory_delete_component(factory,datareader->dr_intf);
        datareader->dr_intf = NULL;
    }

    if (datareader->rtps_intf != NULL)
    {
        factory = RT_Registry_lookup(datareader->config->registry,NETIO_DEFAULT_RTPS_NAME);
        if (factory == NULL)
        {
            DDSC_LOG_COMPONENT_LOOKUP(OSAPI_LOGKIND_ERROR,NETIO_DEFAULT_RTPS_NAME)
            goto done;
        }

        NETIO_InterfaceFactory_delete_component(factory,datareader->rtps_intf);
        datareader->rtps_intf = NULL;
    }

    if ((datareader->cdr_samples != NULL)
         && !REDA_BufferPool_delete(datareader->cdr_samples))
    {
        DDSC_LOG_CDR_POOL_DELETE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_CDR)
        goto done;
    }

    if (DDS_DataReaderQos_finalize(&datareader->qos) != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_QOS)
        goto done;
    }

    if (!DDS_EntityImpl_finalize(&self->as_entity))
    {
        DDSC_LOG_ENTITY_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_ENTITY)
        goto done;
    }

    retval = DDS_BOOLEAN_TRUE;

done:

    return retval;
}
#endif /* !RTI_CERT */

/*ci
 * \brief Return the instance handle of a datareader
 *
 * \details
 * This function overrides the DDS entity get_instance_handle function and
 * is called via DDS_Entity_get_instance_handle.
 *
 * \param[in] entity The base-class for the datareader
 *
 * \return The instance handle
 */
MUST_CHECK_RETURN RTI_PRIVATE DDS_InstanceHandle_t
DDS_DataReaderImpl_get_instance_handle(DDS_Entity *entity)
{
    struct DDS_DataReaderImpl *dr = (struct DDS_DataReaderImpl*)entity;
    DDS_InstanceHandle_t retval;

    retval = dr->config->get_parent_handle((DDS_Entity*)dr->subscriber);

    DDS_InstanceHandle_set_suffix(&retval,dr->as_entity.entity_id);

    return retval;
}

/*ci
 * \brief Initialize a datareader
 *
 * \details
 *
 * The subscriber allocates memory to store the datareader data and passes
 * it to the datareader for initialization. The datareader allocates all its
 * internal resources. The datareader is passed shared resources in the
 * config structure, such as database, timers resolvers etc. These resources
 * are typically managed by the domain participant.
 *
 * \param[in] datareader A datareader structure to initialize
 * \param[in] subscriber The subscriber creating the datareader
 * \param[in] topic_description The topic the datareader is subscribing to
 * \param[in] qos        The datareader qos policy
 * \param[in] listener   The datareader listener
 * \param[in] mask       Mask with enabled statuses on the datareader
 * \param[in] object_id  The datareader object id generated by the factory
 * \param[in] config     General datareader configuration
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DDS_DataReaderImpl_initialize(struct DDS_DataReaderImpl *datareader,
                       DDS_Subscriber *subscriber,
                       DDS_TopicDescription *topic_description,
                       const struct DDS_DataReaderQos *qos,
                       const struct DDS_DataReaderListener *listener,
                       DDS_StatusMask mask,
                       DDS_UnsignedLong object_id,
                       struct NDDS_DataReaderConfig *config)
{
#if !(INCLUDE_API_QOS)
    struct DDS_DataReaderQos init_val = DDS_DataReaderQos_INITIALIZER;
#endif
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    DDS_Topic *topic;
    struct DDS_DataReaderInterfaceProperty drintf_property =
                                DDS_DataReaderInterfaceProperty_INITIALIZER;
    struct DDSHST_ReaderProperty rh_property =
                                        DDSHST_ReaderProperty_INITIALIZER;
    struct DDSHST_ReaderListener rh_listener =
                                        DDSHST_ReaderListener_INITIALIZE;
    DDS_InstanceHandle_t instance_handle;
    struct RT_ComponentFactory *factory;
    struct REDA_BufferPoolProperty pool_property =
                                            REDA_BufferPoolProperty_INITIALIZER;
    struct RTPS_InterfaceProperty rtps_property = RTPS_InterfaceProperty_INITIALIZER;
    DDS_UnsignedLong serialized_key_size;
    DDS_ReturnCode_t ddsrc;
    struct DDS_DataReaderListener nil_listener =
        DDS_DataReaderListener_INITIALIZER;

    OSAPI_Memory_zero(datareader,sizeof(struct DDS_DataReaderImpl));

    if ((qos != &DDS_DATAREADER_QOS_DEFAULT) &&
        !DDS_DataReaderQos_is_consistent(qos))
    {
        DDSC_LOG_QOS_INCONSISTENT(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_QOS)
        goto done;
    }

    if ((listener != NULL) &&
        !DDS_DataReaderListener_is_consistent(listener,mask))
    {
        DDSC_LOG_LISTENER_INCONSISTENT(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_LISTENER,mask)
        goto done;
    }

    /* Initialize QoS */
    ddsrc = DDS_DataReaderQos_initialize(&datareader->qos);
    if (ddsrc != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_INITIALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_QOS,ddsrc)
        goto done;
    }

    /* Initialize the DataReader */
    if (qos == &DDS_DATAREADER_QOS_DEFAULT)
    {
#if INCLUDE_API_QOS
        ddsrc = DDS_Subscriber_get_default_datareader_qos(subscriber, &datareader->qos);
        if (ddsrc != DDS_RETCODE_OK)
        {
            DDSC_LOG_QOS_GET(OSAPI_LOGKIND_ERROR,DDSC_LOG_DEFAULTDATAREADER_QOS)
            goto done;
        }

#else
        /* DDS_DATAREADER_QOS_DEFAULT is a special constant defined by DDS
         * that says "use the default Qos". The current RCC implementation
         * happens to define the DDS_DATAREADER_QOS_DEFAULT equal to the
         * DataReaderQos_INITIALIZER. However, this is a coupling that is
         * not guaranteed. Thus, we separate between the constant to use
         * the default value and the initial value.
         */
        datareader->qos = init_val;
#endif
    }
    else
    {
        if (DDS_DataReaderQos_copy(&datareader->qos, qos) != DDS_RETCODE_OK)
        {
            DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_QOS)
            goto done;
        }
    }

    /* pool for loaned sample and info sequence buffers */
    if (datareader->qos.history.kind == DDS_KEEP_ALL_HISTORY_QOS)
    {
        /* NOTE: assumes finite max_samples */
        datareader->read_seq_max =
            datareader->qos.resource_limits.max_samples;
    }
    else /* KEEP_LAST */
    {
        /* NOTE: assumes finite max_instances */
        datareader->read_seq_max =
            (datareader->qos.resource_limits.max_instances *
             datareader->qos.history.depth);
    }

    topic = DDS_Topic_narrow(topic_description);
    if (topic == NULL)
    {
        DDSC_LOG_TOPIC_NARROW(OSAPI_LOGKIND_ERROR,
                            DDS_TopicDescription_get_name(topic_description))
        goto done;
    }

    datareader->topic = topic;
    datareader->type_plugin =
                DDS_TypeImpl_get_plugin(DDS_Topic_get_type(datareader->topic));

    if (datareader->type_plugin == NULL)
    {
        DDSC_LOG_LOOKUP_TYPE_PLUGIN(OSAPI_LOGKIND_ERROR,
                                        DDS_Topic_get_type(datareader->topic))
        goto done;
    }

    DDS_LocatorSeq_initialize(&datareader->uc_locator_seq);
    DDS_LocatorSeq_initialize(&datareader->mc_locator_seq);

    datareader->reader_data.unicast_locator = &datareader->uc_locator_seq;
    datareader->reader_data.multicast_locator = &datareader->mc_locator_seq;
    datareader->qos.data = &datareader->reader_data;

    if (DDS_StringSeq_get_length(&datareader->qos.transport.enabled_transports) > 0)
    {
        if (!DDS_LocatorSeq_set_maximum(&datareader->uc_locator_seq,
                                   RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX))
        {
            DDSC_LOG_SEQ_SETMAX(OSAPI_LOGKIND_ERROR,
                                DDSC_LOG_USERUNICAST_SEQUENCE,
                                RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX)
            goto done;
        }

        if (!DDS_LocatorSeq_set_length(&datareader->uc_locator_seq,0))
        {
            DDSC_LOG_SEQ_SETLENGTH(OSAPI_LOGKIND_ERROR,
                                DDSC_LOG_USERUNICAST_SEQUENCE,0)
            goto done;
        }

        if (!DDS_LocatorSeq_set_maximum(&datareader->mc_locator_seq,
                                        RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX))
        {
            DDSC_LOG_SEQ_SETMAX(OSAPI_LOGKIND_ERROR,
                                DDSC_LOG_USERMULTICAST_SEQUENCE,
                                RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX)
            goto done;
        }

        if (!DDS_LocatorSeq_set_length(&datareader->mc_locator_seq,0))
        {
            DDSC_LOG_SEQ_SETLENGTH(OSAPI_LOGKIND_ERROR,
                                   DDSC_LOG_USERMULTICAST_SEQUENCE,0)
            goto done;
        }

        if (!NETIO_BindResolver_reserve_addresses(
             config->bind_resolver,*config->participant_id,
             config->enabled_transports,
             (struct REDA_StringSeq*)&datareader->qos.transport.enabled_transports,
             NETIO_ROUTEKIND_USER,
             (struct NETIO_AddressSeq*)&datareader->mc_locator_seq,
             (struct NETIO_AddressSeq*)&datareader->uc_locator_seq))
        {
            goto done;
        }
    }

    datareader->key_kind = datareader->type_plugin->get_key_kind(
                                 datareader->type_plugin,
                                 datareader->qos.type_support.plugin_data);

    if (!DDS_EntityImpl_initialize(&datareader->as_entity,
                                   DDS_DATAREADER_ENTITY_KIND,
                                   object_id,DDS_DataReader_enable,
                                   DDS_DataReaderImpl_get_instance_handle))
    {
        DDSC_LOG_ENTITY_INITIALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_ENTITY)
        goto done;
    }

    datareader->config = config;
    datareader->subscriber = subscriber;

    if (listener == NULL)
    {
        datareader->listener = nil_listener;
    }
    else
    {
        datareader->listener = *listener;
    }

    datareader->mask = mask;
    datareader->md5_stream = NULL;

    pool_property.buffer_size = sizeof(struct RTI_DataReaderSample);
    pool_property.max_buffers = (RTI_SIZE_T)datareader->qos.resource_limits.max_samples;

    /* Add one for keep last purposes */
    pool_property.max_buffers++;

    pool_property.flags |= REDA_BUFFERPOOL_FLAG_ARRAY_ALLOC;

#ifndef RTI_CERT
    datareader->cdr_samples = REDA_BufferPool_new("cdr_samples",&pool_property,
                                    DDS_DataReader_cdr_initialize,
                                    datareader,
                                    DDS_DataReader_cdr_finalize,
                                    datareader);
#else
    /* No finalize fn for RTI_CERT */
    datareader->cdr_samples = REDA_BufferPool_new("cdr_samples",&pool_property,
                                    DDS_DataReader_cdr_initialize,
                                    datareader,
                                    NULL,
                                    NULL);
#endif

    if (datareader->cdr_samples == NULL)
    {
        DDSC_LOG_CDR_POOL_ALLOC(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_CDR,
                                pool_property.buffer_size,
                                pool_property.max_buffers)
        goto done;
    }

    rh_listener.listener_data = (void *)datareader;
    rh_listener.on_sample_removed = DDS_DataReaderEvent_on_sample_removed;
    rh_listener.on_data_available = DDS_DataReaderEvent_on_data_available;
    rh_listener.on_deadline_missed = DDS_DataReaderEvent_on_deadline_missed;
    rh_listener.on_sample_rejected = DDS_DataReaderEvent_on_hst_sample_rejected;
    rh_listener.on_sample_lost = DDS_DataReaderEvent_on_hst_sample_lost;
    rh_listener.on_instance_replaced = DDS_DataReaderEvent_on_instance_replaced;
    rh_listener.on_sample_committed = DDS_DataReaderEvent_on_sample_committed;

    factory = RT_Registry_lookup(datareader->config->registry,
                                 DDSHST_READER_DEFAULT_HISTORY_NAME);
    if (factory == NULL)
    {
        DDSC_LOG_COMPONENT_LOOKUP(OSAPI_LOGKIND_ERROR,
                                  DDSHST_READER_DEFAULT_HISTORY_NAME)
        goto done;
    }

    rh_property._parent.db = datareader->config->db;
    rh_property.qos = &datareader->qos;
    rh_property.timer = datareader->config->timer;

    datareader->_rh = DDSHST_ReaderFactory_create_component(factory,
                                                        &rh_property._parent,
                                                        &rh_listener._parent);

    if (datareader->_rh == NULL)
    {
        DDSC_LOG_COMPONENT_CREATE(OSAPI_LOGKIND_ERROR,
                                  DDSC_LOG_DATAREADERIO_COMPONENT,
                                  DDSHST_READER_DEFAULT_HISTORY_NAME)
        goto done;
    }

    /* DataWriter Interface */
    factory = RT_Registry_lookup(datareader->config->registry,
                                 DDS_DEFAULT_DATAREADER_NETIO_NAME);
    if (factory == NULL)
    {
        DDSC_LOG_COMPONENT_LOOKUP(OSAPI_LOGKIND_ERROR,
                                  DDS_DEFAULT_DATAREADER_NETIO_NAME)
        goto done;
    }

    drintf_property._parent._parent.db = datareader->config->db;

    drintf_property._parent.max_binds =
            (RTI_SIZE_T)datareader->qos.reader_resource_limits.max_remote_writers;

    drintf_property.datareader = datareader;
    instance_handle = DDS_DataReaderImpl_get_instance_handle(
                                                    (DDS_Entity*)datareader);

    NETIO_Address_set_guid(&drintf_property.intf_address,
                          0,(struct NETIO_Guid*)&instance_handle.octet);

    datareader->dr_intf = NETIO_InterfaceFactory_create_component(factory,
                                        &drintf_property._parent._parent,NULL);
    if (datareader->dr_intf == NULL)
    {
        DDSC_LOG_COMPONENT_CREATE(OSAPI_LOGKIND_ERROR,
                                  DDSC_LOG_DATAREADERIO_COMPONENT,
                                  DDS_DEFAULT_DATAREADER_NETIO_NAME)
        goto done;
    }

    factory = RT_Registry_lookup(datareader->config->registry,NETIO_DEFAULT_RTPS_NAME);
    if (factory == NULL)
    {
        DDSC_LOG_COMPONENT_LOOKUP(OSAPI_LOGKIND_ERROR,NETIO_DEFAULT_RTPS_NAME)
        goto done;
    }

    NETIO_Address_set_guid(&rtps_property.intf_address,
        (RTI_UINT32)*datareader->config->domain_id,
        (struct NETIO_Guid*)&instance_handle.octet);
    rtps_property._parent._parent.db = datareader->config->db;
    rtps_property.anonymous = datareader->qos.management.is_anonymous;

    rtps_property._parent.max_routes = (RTI_SIZE_T)
                (datareader->qos.reader_resource_limits.max_remote_writers *
                 datareader->qos.reader_resource_limits.max_routes_per_writer);

    rtps_property.mode = RTPS_INTERFACEMODE_READER;
    rtps_property.reliable =
        (datareader->qos.reliability.kind == DDS_RELIABLE_RELIABILITY_QOS);
    rtps_property.max_peer_count =
        datareader->qos.reader_resource_limits.max_remote_writers;
    rtps_property._parent._parent.timer = datareader->config->timer;
    rtps_property.max_window_size =
            (RTI_SIZE_T)datareader->qos.reader_resource_limits.max_samples_per_remote_writer;
    rtps_property.max_samples =
        datareader->qos.resource_limits.max_samples;

    /* Note: for this "local" bind, max_binds could in theory just be set to the
     * max number of upstream interfaces.  However, in RTPS implementation,
     * the bind table is used to track src-peer mappings, and thus has a record
     * per src-peer pair and must be set to max peers (max_remote_writers).
     */
    rtps_property._parent.max_binds =
            (RTI_SIZE_T)datareader->qos.reader_resource_limits.max_remote_writers;

#if OSAPI_ENABLE_TRACE
    rtps_property.session_name =
            DDS_TopicDescription_get_name(
                    DDS_Topic_as_topicdescription(datareader->topic));
#endif


    DDS_Duration_to_ntp_time(
                &datareader->qos.protocol.rtps_reliable_reader.nack_period,
                &rtps_property.nack_period);

    datareader->rtps_intf = NETIO_InterfaceFactory_create_component(factory,
                                        &rtps_property._parent._parent,NULL);
    if (datareader->rtps_intf == NULL)
    {
        DDSC_LOG_COMPONENT_CREATE(OSAPI_LOGKIND_ERROR,
                                  DDSC_LOG_RTPS_COMPONENT,NETIO_DEFAULT_RTPS_NAME)
        goto done;
    }

    if (datareader->key_kind == NDDS_TYPEPLUGIN_USER_KEY)
    {
        serialized_key_size =
                datareader->type_plugin->get_serialized_key_max_size(
                                    datareader->type_plugin, 0,
                                    datareader->qos.type_support.plugin_data);

        datareader->md5_stream = CDR_Stream_alloc(serialized_key_size);
        if (datareader->md5_stream == NULL)
        {
            DDSC_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_MD5STREAM_OBJECT)
            goto done;
        }
    }
    else if (datareader->key_kind == NDDS_TYPEPLUGIN_GUID_KEY)
    {
        datareader->md5_stream = CDR_Stream_alloc(16);
        if (datareader->md5_stream == NULL)
        {
            DDSC_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_MD5STREAM_OBJECT)
            goto done;
        }
    }

    if (datareader->md5_stream != NULL)
    {
#ifdef RTI_ENDIAN_LITTLE
        CDR_Stream_byteswap_set(datareader->md5_stream, RTI_TRUE);
#else
        CDR_Stream_byteswap_set(datareader->md5_stream, RTI_FALSE);
#endif
    }

    datareader->as_entity.state = RTIDDS_ENTITY_STATE_CREATED;

    retval = DDS_BOOLEAN_TRUE;

done:
#ifndef RTI_CERT
    if (!retval)
    {
        (void)DDS_DataReaderImpl_finalize(datareader);
    }
#endif
    return retval;
}

/*ci
 * \brief Check if a datareader is enabled
 *
 * \param[in] self Datareader to check
 *
 * \return DDS_BOOLEAN_TRUE if the datareader is enabled, DDS_BOOLEAN_FALSE
 *         otherwise
 */
DDS_Boolean
DDS_DataReader_is_enabled(DDS_DataReader *self)
{
    return (self->as_entity.state == RTIDDS_ENTITY_STATE_ENABLED) ?
                        DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/*ci
 * \brief Check if a DDS_DataReaderListener is consistent
 *
 * \param[in] l DDS_DataReaderListener to test for consistency
 * \param[in] m The current status mask to test
 *
 * \return evaluates to logical true on success, false on failure
 */
DDS_Boolean
DDS_DataReaderListener_is_consistent(const struct DDS_DataReaderListener *l,
                                     DDS_StatusMask m)
{
    return (((!((m) & DDS_REQUESTED_DEADLINE_MISSED_STATUS)) ||
            (l->on_requested_deadline_missed != NULL)) &&
        ((!((m) & DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS)) ||
            (l->on_requested_incompatible_qos != NULL)) &&
        ((!((m) & DDS_SAMPLE_REJECTED_STATUS)) ||
            (l->on_sample_rejected != NULL)) &&
        ((!((m) & DDS_SAMPLE_LOST_STATUS)) ||
            (l->on_sample_lost != NULL)) &&
        ((!((m) & DDS_LIVELINESS_CHANGED_STATUS)) ||
            (l->on_liveliness_changed != NULL)) &&
        ((!((m) & DDS_SUBSCRIPTION_MATCHED_STATUS)) ||
            (l->on_subscription_matched != NULL)) &&
        ((!((m) & DDS_INSTANCE_REPLACED_STATUS)) ||
            (l->on_instance_replaced != NULL)) &&
        ((!((m) & DDS_DATA_AVAILABLE_STATUS)) ||
            (l->on_data_available != NULL))) ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/*ci @} */
