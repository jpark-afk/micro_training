/*
 * FILE: DomainParticipant.c - DomainParticipant implementation
 *
 * Copyright (c) 2008-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 03oct2016,tk MICRO-1569 Improved type registration/unregistration API
 * 27jun2016,tk MICRO-1545 delete_topic() now returns OK if referenced
 *                         but not inuse
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 * 31jul2015,tk MICRO-1479/PR#15696 Set maximum participant_index for the RTPS
 *                                  port resolver
 * 28jul2015,tk MICRO-1470/PR#15623 Do not enable contained entities unless
 *                                  auto_enable is true (in enable())
 * 14may2015,tk MICRO-1425/PR#15349 Check if entity > DDS_ENTITYNAME_QOS_NAME_MAX,
 *                                  not >= DDS_ENTITYNAME_QOS_NAME_MAX
 * 16may2015,tk MICRO-1215/PR#14760 Initialize get_parent_handle in create_topic
 * 12mar2015,tk MICRO-1102/PR#14163 Removed magic constants
 * 20feb2015,eh MICRO-813/PR#9172  Fix Lint warnings
 * 10dec2014,tk MICRO-978/PR#12914 Consistently return records on failure
 * 10dec2014,tk MICRO-957/PR#12363 Removed used of reserved_address sequence
 * 03dec2014,tk MICRO-984/PR#12966 Initialize is_valid
 * 03dec2014,tk MICRO-994/PR#12351 Removed redundant code
 * 03dec2014,tk MICRO-982/PR#12960 Initialize lease_duration to infinite
 *                                 (always set by the disc plugin though)
 * 03dec2014,tk MICRO-990/PR#12983 Removed redundant pre-condition check
 * 02dec2014,tk MICRO-960/PR#12369 Return error if disc plugin fails in
 *                                 on_after_local_participant_created
 * 01dec2014,tk MICRO-959/PR#12367 Fixed log message
 * 16sep2014,tk MICRO-873 Return DDS_RETCODE_PRECONDITION_NOT_MET instead of
 *                        DDS_RETCODE_ERROR is a type-name is registered
 *                        multiple times for different types
 * 16sep2014,as MICRO-903/PR#11236 Incorrect handling of status events and listeners
 * 31jul2014,tk MICRO-241/PR#1413  Removed superfluous DB fields
 * 31jul2014,tk MICRO-842/PR#9683  Removed superfluous parameters in DB
 *                                 compare function
 * 11jun2014,tk MICRO-803 Qos initialize locators to max size
 * 20may2014,tk MICRO-792 Qos consistency checks
 * 05may2014,as MICRO-270 Always enable precondition checks for public API
 *                        operations
 * 24apr2014,tk MICRO-669 Added initialize topic_qos
 * 19mar2014,tk MICRO-74  Support endpoint specific transport
 * 04mar2014,tk MICRO-212 Allow multiple registrations of the same type-name
 *                        as long as the plugin pointer is the same
 * 13feb2014,tk MICRO-719 add_peer() support
 * 08nov2013,as MICRO-681 Complete implementation of WaitSets
 *                        and support for StatusConditions
 * 01aug2013,tk MICRO-677 on_liveliness_callback on remote deletion
 * 19jul2013,as Added support for C++
 * 27jun2012,tk Major update
 * 25aug2011,yy Set interpreter storage to NULL upon deletion
 * 30apr2008,tk Written
 */
/*ci
 * \file
 *
 * \brief Implements the public DDS API.
 *
 * \details
 * This file implements the public DDS API as well as internal functions to
 * support it. In general this file should be limited to public APIs. The APIs
 * in this file is mostly related to the life-cycle and operation of a
 * participant, such as creating/deleting entities, set/get qos policies
 * and lookup entities.
 */
/*ci \addtogroup DDSDomainModule
 * @{
 */
#include "dds_c/dds_c_config.h"
#ifndef netio_address_h
#include "netio/netio_address.h"
#endif
#ifndef netio_route_h
#include "netio/netio_route.h"
#endif
#ifndef netio_loopback_h
#include "netio/netio_loopback.h"
#endif
#ifndef netio_rtps_h
#include "netio/netio_rtps.h"
#endif
#ifndef reda_sequence_h
#include "reda/reda_sequence.h"
#endif
#ifndef reda_string_h
#include "reda/reda_string.h"
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
#ifndef netio_notif_h
#include "netio/netio_notif.h"
#endif

#include "Entity.h"
#include "Conditions.h"
#include "QosPolicy.h"
#include "TopicDescription.h"
#include "TopicQos.h"
#include "Topic.h"
#include "Type.h"
#include "RtpsWellKnownPorts.h"
#include "DomainParticipantQos.h"
#include "DomainParticipantEvent.h"
#include "DataReaderImpl.h"
#include "SubscriberQos.h"
#include "SubscriberImpl.h"
#include "DataWriterImpl.h"
#include "PublisherQos.h"
#include "PublisherImpl.h"
#include "RemoteEntity.h"
#include "RemoteEndpoint.h"
#include "RemoteParticipant.h"
#include "RemotePublication.h"
#include "RemoteSubscription.h"
#include "BuiltinTopicData.h"
#include "UserDataQosPolicy.h"
#include "DomainFactory.h"
#include "PartitionQosPolicy.h"
#include "DomainParticipant.h"
#include "DomainParticipantChecksum.h"
#include "DomainParticipantTrust.h"

#if DDS_FILTERING_ENABLED
#include "DomainParticipantFilter.h"
#endif

/*ci
 * \brief The hostname property name sent as part of participant discovery
 */
RTI_PRIVATE char *const DDS_PROPERTY_HOSTNAME_NAME = "dds.sys_info.hostname";

/*ci
* \brief The process id property name sent as part of participant discovery
*/
RTI_PRIVATE char *const DDS_PROPERTY_PROCESSID_NAME = "dds.sys_info.process_id";

/*ci
* \brief The target-name property name sent as part of participant discovery
*/
RTI_PRIVATE char *const DDS_PROPERTY_TARGET_NAME = "dds.sys_info.target";

/*ci
* \brief The Micro version property name sent as part of discovery
*/
RTI_PRIVATE char *const DDS_PROPERTY_VERSION_NAME = "rti.service.version";

/*ci
 * \brief The DB table used for managing topics
 */
RTI_PRIVATE char *const DDS_TOPIC_TABLE_NAME = "topic";

/*ci
 * \brief The DB table used for managing types
 */
RTI_PRIVATE char *const DDS_TYPE_TABLE_NAME = "type";

/*ci
 * \brief The DB table used for managing datareaders
 */
RTI_PRIVATE char *const DDS_DATAREADER_TABLE_NAME = "datareader";

/*ci
 * \brief The DB table used for managing datawriters
 */
RTI_PRIVATE char *const DDS_DATAWRITER_TABLE_NAME = "datawriter";

/*ci
 * \brief The DB table used for managing publications
 */
RTI_PRIVATE char *const DDS_PUBLICATION_TABLE_NAME = "publication";

/*ci
 * \brief The DB table used for managing subscriptions
 */
RTI_PRIVATE char *const DDS_SUBSCRIPTION_TABLE_NAME = "subscription";

/*ci
 * \brief The DB table used for managing remote participants
 */
RTI_PRIVATE char *const DDS_PARTICIPANT_TABLE_NAME = "participant";

/*ci
 * \brief The DB table used for managing publishers
 */
RTI_PRIVATE char *const DDS_PUBLISHER_TABLE_NAME = "publisher";

/*ci
 * \brief The DB table used for managing subscribers
 */
RTI_PRIVATE char *const DDS_SUBSCRIBER_TABLE_NAME = "subscriber";

#ifndef RTI_CERT
/*ci
 * \brief The default participant name if the participant Qos policy does not
 *        contain one.
 */
RTI_PRIVATE char *const DDS_DEFAULT_PARTICIPANT_NAME = "_participant_";
#endif

/*ci
 * \brief The DB table used for managing topic string names
 */
RTI_PRIVATE const char *const DDS_MANAGEDSTRING_TABLE_NAME = "sm_table";

/*ci
 * \brief The DB table used for managing partition string names
 */
RTI_PRIVATE const char *const DDS_PARTITION_STRING_TABLE_NAME = "p_table";

/*ci
 * \brief The number of tables needed by the participant
 */
#define DDS_PARTICIPANT_TABLES              (9U)

/*ci
 * \brief The number of tables used by string manager for types/topic and partitions
 */
#define DDS_PARTICIPANT_STRING_MANAGER_TABLE (2U)

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Check if a DDS_DomainParticipantListener is consistent
 *
 * \param[in] l DDS_DomainParticipantListener to test for consistency
 * \param[in] m The current status mask to test
 *
 * \return evaluates to logical true on success, false on failure
 */
RTI_PRIVATE DDS_Boolean
DDS_DomainParticipantListener_is_consistent(
                        const struct DDS_DomainParticipantListener *l,
                        DDS_StatusMask m)
{
    return DDS_TopicListener_is_consistent(&l->as_topiclistener,m) &&
           DDS_PublisherListener_is_consistent(&l->as_publisherlistener,m) &&
           DDS_SubscriberListener_is_consistent(&l->as_subscriberlistener,m);
}

#if DDS_LIVELINESS_CHANNEL_ENABLED
/*ci
 * \brief Compare entries in the table of matched dw. The function is
 *        compatible with \ref DB_IndexCompare_T
 *
 * \param[in] flags Used to distinguish between op2 being a key or record
 * \param[in] op1   A DDS_RemotePublicationImpl already in the database
 * \param[in] op2   Either a DDS_RemotePublicationImpl being added or a key
 *                  being searched for
 *
 * \return positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2,
 *         zero if op1 is equal to op2
 */
RTI_INT32
DDS_DomainParticipant_matched_dw_compare(RTI_INT32 flags,
                                         const DB_Record_T op1,
                                         void *op2)
{
    const struct RemoteWriterMatchEntry_t *key_left =
                                      (const struct RemoteWriterMatchEntry_t*)op1;
    const struct RemoteWriterMatchEntry_t *key_right =
                                      (const struct RemoteWriterMatchEntry_t*)op2;

    UNUSED_ARG(flags);

    if (key_left->local_reader_oid > key_right->local_reader_oid)
    {
        return 1;
    }
    else if (key_left->local_reader_oid < key_right->local_reader_oid)
    {
        return -1;
    }
    else
    {
        return DDS_BuiltinTopicKey_compare(&key_left->remote_writer_key,
                                           &key_right->remote_writer_key);
    }
}
#endif

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DomainParticipant_remote_subscriber_entry_initialize(
    void *initialize_param, void *buffer)
{
    RTI_BOOL result = RTI_FALSE;
    struct DDS_RemoteSubscriptionImpl *rem_sub = NULL;
    DDS_DomainParticipant *participant = (DDS_DomainParticipant*)initialize_param;

    OSAPI_Memory_zero(buffer,sizeof(struct DDS_RemoteSubscriptionImpl));
    rem_sub = (struct DDS_RemoteSubscriptionImpl*)buffer;

    if (!DDS_SubscriptionBuiltinTopicData_initialize_shallow(&rem_sub->data, participant))
    {
        goto done;
    }

    result = RTI_TRUE;
done:
    return result;
}

#ifndef RTI_CERT
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DomainParticipant_remote_subscriber_entry_finalize(
    void *finalize_param, void *buffer)
{
    RTI_BOOL result = RTI_FALSE;
    struct DDS_RemoteSubscriptionImpl *rem_sub = (struct DDS_RemoteSubscriptionImpl*)buffer;

    UNUSED_ARG(finalize_param);

    if (!DDS_SubscriptionBuiltinTopicData_finalize(&rem_sub->data))
    {
        goto done;
    }
    result = RTI_TRUE;
done:
    return result;
}
#endif

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DomainParticipant_remote_publisher_entry_initialize(
    void *initialize_param, void *buffer)
{
    RTI_BOOL result = RTI_FALSE;
    struct DDS_RemotePublicationImpl *rem_pub = NULL;
    DDS_DomainParticipant *participant = (DDS_DomainParticipant*)initialize_param;

    OSAPI_Memory_zero(buffer,sizeof(struct DDS_RemotePublicationImpl));
    rem_pub = (struct DDS_RemotePublicationImpl*)buffer;

    if (!DDS_PublicationBuiltinTopicData_initialize_shallow(
            &rem_pub->data, &participant->qos))
    {
        goto done;
    }

    result = RTI_TRUE;
done:
    return result;
}

#ifndef RTI_CERT
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DomainParticipant_remote_publisher_entry_finalize(
    void *finalize_param, void *buffer)
{
    RTI_BOOL result = RTI_FALSE;
    struct DDS_RemotePublicationImpl *rem_pub = (struct DDS_RemotePublicationImpl*)buffer;

    UNUSED_ARG(finalize_param);

    if (!DDS_PublicationBuiltinTopicData_finalize(&rem_pub->data))
    {
        goto done;
    }

    result = RTI_TRUE;
done:
    return result;
}
#endif

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DomainParticipant_remote_participant_entry_initialize(
                    void *initialize_param, void *buffer)
{
    DDS_Boolean retval = RTI_FALSE;
    struct DDS_RemoteParticipantImpl *record = NULL;
    DDS_DomainParticipant *participant = (DDS_DomainParticipant*)initialize_param;

    OSAPI_Memory_zero(buffer,sizeof(struct DDS_RemoteParticipantImpl));
    record = (struct DDS_RemoteParticipantImpl*)buffer;

    if (!DDS_ParticipantBuiltinTopicData_initialize_shallow(
            &record->data, &participant->qos))
    {
        goto done;
    }

    retval = RTI_TRUE;
done:
    return retval;
}

#ifndef RTI_CERT
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DomainParticipant_remote_participant_entry_finalize(
                    void *initialize_param, void *buffer)
{
    RTI_BOOL retval = RTI_FALSE;
    struct DDS_RemoteParticipantImpl *record = (struct DDS_RemoteParticipantImpl*)buffer;

    UNUSED_ARG(initialize_param);

    if (!DDS_ParticipantBuiltinTopicData_finalize(&record->data))
    {
        goto done;
    }

    retval = RTI_TRUE;
done:
    return retval;
}
#endif

/*ci
 * \brief Initialize the local participant database
 *
 * \param[in] participant DDS DomainParticipant
 *
 * \return DDS_RETCODE_OK on success, one of the DDS_ReturnCode_t codes
 *         on failure
 */
RTI_PRIVATE DDS_ReturnCode_t
DDS_DomainParticipant_db_init(struct DDS_DomainParticipantImpl *participant)
{
    DB_ReturnCode_T dbrc;
    struct DB_TableProperty tbl_prop = DB_TableProperty_INITIALIZER;
    struct DB_DatabaseProperty db_property = DB_DatabaseProperty_INITIALIZER;
    DDS_ReturnCode_t ddsrc = DDS_RETCODE_ERROR;
    RTI_SIZE_T t_length;

    /* The maximum number of tables are calculated based on detailed knowledge
     * on the implementation. NOTE: The NETIO_RESOURCE_TABLES_PER_INTERFACE
     * is for the external RTPS interface which is not specified in the
     * enabled transport list.
     */
    t_length = (RTI_SIZE_T)REDA_StringSeq_get_length(
                            &participant->qos.transports.enabled_transports);

    /* The database is initialized before the default transports are added
     * when no are specified by the user.
     */
    if (t_length == 0)
    {
        struct RT_ComponentFactory *udp_factory = NULL;

        udp_factory = RT_Registry_lookup(participant->config.registry,
                                         NETIO_DEFAULT_UDP_NAME);
        if (udp_factory != NULL)
        {
            t_length = 2;
        }
        else
        {
            t_length = 1;
        }
    }


    db_property.lock_mode = DB_LOCK_LEVEL_SHARED;
    participant->database = NULL;

    dbrc = DB_Database_create(&participant->database,"domain",
                              &db_property,participant->db_lock);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_DATABASE_CREATE(OSAPI_LOGKIND_ERROR,dbrc)
        ddsrc = DDS_RETCODE_ERROR;
        goto done;
    }

    /*
     * Local entities
     */
    tbl_prop.max_records = (RTI_SIZE_T)participant->qos.resource_limits.local_topic_allocation;
    dbrc = DB_Database_create_table(&participant->topic_table,
                                    participant->database,DDS_TOPIC_TABLE_NAME,
                                    sizeof(struct DDS_TopicImpl),
                                    DDS_TopicImpl_compare,&tbl_prop);

    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_CREATE(OSAPI_LOGKIND_ERROR,DDS_TOPIC_TABLE_NAME,dbrc)
        ddsrc = DDS_RETCODE_ERROR;
        goto done;
    }

    tbl_prop.max_records = (RTI_SIZE_T)participant->qos.resource_limits.local_type_allocation;
    dbrc = DB_Database_create_table(&participant->type_table,
                                    participant->database,DDS_TYPE_TABLE_NAME,
                                    sizeof(struct DDS_TypeImpl),
                                    DDS_TypeImpl_compare,&tbl_prop);

    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_CREATE(OSAPI_LOGKIND_ERROR,DDS_TYPE_TABLE_NAME,dbrc)
        ddsrc = DDS_RETCODE_ERROR;
        goto done;
    }

    tbl_prop.max_records = (RTI_SIZE_T)participant->qos.resource_limits.local_publisher_allocation;
    dbrc = DB_Database_create_table(&participant->local_publisher_table,
                                    participant->database,
                                    DDS_PUBLISHER_TABLE_NAME,
                                    sizeof(struct DDS_PublisherImpl),
                                    DDS_PublisherImpl_compare,&tbl_prop);

    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_CREATE(OSAPI_LOGKIND_ERROR,DDS_PUBLISHER_TABLE_NAME,dbrc)
        ddsrc = DDS_RETCODE_ERROR;
        goto done;
    }

    tbl_prop.max_records = (RTI_SIZE_T)participant->qos.resource_limits.local_subscriber_allocation;
    dbrc = DB_Database_create_table(&participant->local_subscriber_table,
                                    participant->database,
                                    DDS_SUBSCRIBER_TABLE_NAME,
                                    sizeof(struct DDS_SubscriberImpl),
                                    DDS_SubscriberImpl_compare,&tbl_prop);

    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_CREATE(OSAPI_LOGKIND_ERROR,DDS_SUBSCRIBER_TABLE_NAME,dbrc)
        ddsrc = DDS_RETCODE_ERROR;
        goto done;
    }

    tbl_prop.max_records = (RTI_SIZE_T)participant->qos.resource_limits.local_reader_allocation;
    dbrc = DB_Database_create_table(&participant->local_reader_table,
                                    participant->database,
                                    DDS_DATAREADER_TABLE_NAME,
                                    sizeof(struct DDS_DataReaderImpl),
                                    DDS_DataReaderImpl_compare,&tbl_prop);

    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_CREATE(OSAPI_LOGKIND_ERROR,DDS_DATAREADER_TABLE_NAME,dbrc)
        ddsrc = DDS_RETCODE_ERROR;
        goto done;
    }

    tbl_prop.max_records = (RTI_SIZE_T)participant->qos.resource_limits.local_writer_allocation;
#if DDS_LIVELINESS_CHANNEL_ENABLED
    tbl_prop.max_indices = 3;
    tbl_prop.max_cursors = 2;
#endif

    dbrc = DB_Database_create_table(&participant->local_writer_table,
                                    participant->database,
                                    DDS_DATAWRITER_TABLE_NAME,
                                    sizeof(struct DDS_DataWriterImpl),
                                    DDS_DataWriterImpl_compare,
                                    &tbl_prop);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_CREATE(OSAPI_LOGKIND_ERROR,DDS_DATAWRITER_TABLE_NAME,dbrc)
        ddsrc = DDS_RETCODE_ERROR;
        goto done;
    }


    /*
     * Remote entities
     */
    tbl_prop.max_records =
            (RTI_SIZE_T)participant->qos.resource_limits.remote_participant_allocation;

#ifdef RTI_CERT
#define  TABLE_FINALIZER NULL
#else
#define  TABLE_FINALIZER DDS_DomainParticipant_remote_participant_entry_finalize
#endif

    dbrc = DB_Database_create_table_w_ctor_and_dtor(
                            &participant->remote_participant_table,
                            participant->database,
                            DDS_PARTICIPANT_TABLE_NAME,
                            (RTI_SIZE_T)sizeof(struct DDS_RemoteParticipantImpl),
                            DDS_RemoteParticipantImpl_compare,
                            &tbl_prop,
                            DDS_DomainParticipant_remote_participant_entry_initialize,
                            participant,
                            TABLE_FINALIZER,
                            NULL);
#undef TABLE_FINALIZER
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_CREATE(OSAPI_LOGKIND_ERROR,DDS_PARTICIPANT_TABLE_NAME,dbrc)
        ddsrc = DDS_RETCODE_ERROR;
        goto done;
    }

#ifdef RTI_CERT
#define  TABLE_FINALIZER NULL
#else
#define  TABLE_FINALIZER DDS_DomainParticipant_remote_subscriber_entry_finalize
#endif

    tbl_prop.max_records = (RTI_SIZE_T)participant->qos.resource_limits.remote_reader_allocation;
    dbrc = DB_Database_create_table_w_ctor_and_dtor(
                            &participant->remote_subscriber_table,
                            participant->database,
                            DDS_SUBSCRIPTION_TABLE_NAME,
                            (RTI_SIZE_T)sizeof(struct DDS_RemoteSubscriptionImpl),
                            DDS_RemoteSubscriptionImpl_compare,
                            &tbl_prop,
                            DDS_DomainParticipant_remote_subscriber_entry_initialize,
                            participant,
                            TABLE_FINALIZER,
                            NULL);

#undef TABLE_FINALIZER

    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_CREATE(OSAPI_LOGKIND_ERROR,DDS_SUBSCRIPTION_TABLE_NAME,dbrc)
        ddsrc = DDS_RETCODE_ERROR;
        goto done;
    }

#ifdef RTI_CERT
#define  TABLE_FINALIZER NULL
#else
#define  TABLE_FINALIZER DDS_DomainParticipant_remote_publisher_entry_finalize
#endif

    tbl_prop.max_records = (RTI_SIZE_T)participant->qos.resource_limits.remote_writer_allocation;
    dbrc = DB_Database_create_table_w_ctor_and_dtor(
                            &participant->remote_publisher_table,
                            participant->database,
                            DDS_PUBLICATION_TABLE_NAME,
                            (RTI_SIZE_T)sizeof(struct DDS_RemotePublicationImpl),
                            DDS_RemotePublicationImpl_compare,
                            &tbl_prop,
                            DDS_DomainParticipant_remote_publisher_entry_initialize,
                            participant,
                            TABLE_FINALIZER,
                            NULL);

#undef TABLE_FINALIZER

    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_CREATE(OSAPI_LOGKIND_ERROR,DDS_PUBLICATION_TABLE_NAME,dbrc)
        ddsrc = DDS_RETCODE_ERROR;
        goto done;
    }

    ddsrc = DDS_RETCODE_OK;

done:
    return ddsrc;
}

/*ci
 * \brief Return the participant key
 *
 * \param[in] self DDS DomainParticipant
 *
 * \return participant key
 */
MUST_CHECK_RETURN RTI_PRIVATE struct DDS_BuiltinTopicKey_t
NDDS_DomainParticipant_get_key(const DDS_DomainParticipant *const self)
{
    struct DDS_BuiltinTopicKey_t key = DDS_BuiltinTopicKey_t_INITIALIZER;
    DDS_InstanceHandle_t ih;

    ih = DDS_Entity_get_instance_handle(DDS_DomainParticipant_as_entity(self));
    OSAPI_Memory_copy(&key,ih.octet, 16);
#ifdef RTI_ENDIAN_LITTLE
     key.value[0] = NETIO_ntohl(key.value[0]);
     key.value[1] = NETIO_ntohl(key.value[1]);
     key.value[2] = NETIO_ntohl(key.value[2]);
     key.value[3] = NETIO_ntohl(key.value[3]);
#endif

    return key;
}

/*ci
 * \brief Initialize the participant built in topic data
 *
 * \param[in] self The participant
 * \param[in] data participant built-in topic data to initialize
 *
 * \return DDS_RETCODE_OK on success, one of the error return codes
 *         on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE DDS_ReturnCode_t
DDS_DomainParticipant_initialize_builtin_data(
                                struct DDS_DomainParticipantImpl *self,
                                struct DDS_ParticipantBuiltinTopicData *data)
{
    DDS_ProtocolVersion_t protocol_version = DDS_PROTOCOLVERSION;
    RTI_SIZE_T len;

    if (!DDS_ParticipantBuiltinTopicData_initialize_shallow(data, &self->qos))
    {
        return DDS_RETCODE_ERROR;
    }

    data->key = NDDS_DomainParticipant_get_key(self);

    len = OSAPI_String_length(self->qos.participant_name.name);
    if (len > DDS_ENTITYNAME_QOS_NAME_MAX)
    {
        DDSC_LOG_SET_ENTITY_NAME(OSAPI_LOGKIND_ERROR,
                                 data->participant_name.name,
                                 len,DDS_ENTITYNAME_QOS_NAME_MAX)
        return DDS_RETCODE_ERROR;
    }

    OSAPI_Memory_copy(data->participant_name.name,
                      self->qos.participant_name.name,len+1);

    data->rtps_protocol_version = protocol_version;
    data->rtps_vendor_id.vendorId[0] = RTI_CONNEXT_MICRO_VENDOR_ID_MAJOR;
    data->rtps_vendor_id.vendorId[1] = RTI_CONNEXT_MICRO_VENDOR_ID_MINOR;

#if DDS_LIVELINESS_CHANNEL_ENABLED
    data->dds_builtin_endpoints = DDS_BUILTIN_ENDPOINT_MESSAGE_DATA_ANNOUNCER |
                                  DDS_BUILTIN_ENDPOINT_MESSAGE_DATA_DETECTOR;
#else
    data->dds_builtin_endpoints = 0;
#endif

    data->product_version.major    = RTIME_DDS_VERSION_MAJOR;
    data->product_version.minor    = RTIME_DDS_VERSION_MINOR;
    data->product_version.revision = RTIME_DDS_VERSION_REVISION;
    data->product_version.release  = RTIME_DDS_VERSION_RELEASE;

    data->liveliness_lease_duration = DDS_DURATION_INFINITE;

    return DDS_RETCODE_OK;
}

/*ci
 * \brief Return the instance handle of the participant
 *
 * \details
 *
 * This function is the concrete implementation of the abstract method
 * DDS_Entity_get_instance_handle (participants are derived from Entities).
 * This method should not be called directly.
 *
 * \param[in] entity The participant as entity
 *
 * \return The participant instance handle
 */
RTI_PRIVATE DDS_InstanceHandle_t
DDS_DomainParticipantImpl_get_instance_handle(DDS_Entity *entity)
{
    struct DDS_DomainParticipantImpl *participant =
                                    (struct DDS_DomainParticipantImpl*)entity;
    DDS_InstanceHandle_t retval = DDS_HANDLE_NIL;

    OSAPI_Memory_copy(retval.octet,&participant->config.guid,16);
    retval.is_valid = DDS_BOOLEAN_TRUE;

    return retval;
}

/*ci
 * \brief Enable a participant and all the contained entities
 *
 * \details
 *
 * DDS defines the enable() function for a DDS entity and this function
 * implements the function for the concrete DDS_DomainParticipant
 * class. It is not exposed as a public API, it is called via the abstract
 * method DDS_Entity_enable().
 *
 * \param[in] self The participant
 *
 * \return DDS_RETCODE_OK on success, one of the error return codes on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE DDS_ReturnCode_t
DDS_DomainParticipant_enable(DDS_Entity *self)
{
    DB_ReturnCode_T dbrc;
    struct DDS_DomainParticipantImpl *participant = NULL;
    struct DDS_PublisherImpl *local_publisher;
    struct DDS_SubscriberImpl *local_subscriber;
    struct DDS_TopicImpl *topic;
    DB_Cursor_T cursor = NULL;
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    participant = (struct DDS_DomainParticipantImpl*)self;

    if (DB_Database_lock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (self->state == RTIDDS_ENTITY_STATE_ENABLED)
    {
        if (DB_Database_unlock(participant->database) != DB_RETCODE_OK)
        {
            return DDS_RETCODE_ERROR;
        }
        return DDS_RETCODE_OK;
    }

    OSAPI_TRACE_DDS("enable participant",RTI_FALSE)
    OSAPI_TRACE_INT32("ID",participant->as_entity.entity_id,RTI_TRUE)

    self->state = RTIDDS_ENTITY_STATE_ENABLED;

    if (DDS_Entity_enable(
            DDS_Publisher_as_entity(participant->builtin_publisher)) !=
            DDS_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (DDS_Entity_enable(
            DDS_Subscriber_as_entity(participant->builtin_subscriber)) !=
            DDS_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

#if DDS_LIVELINESS_CHANNEL_ENABLED
    /* Enable builtin channels before enabling Discovery */

    if (DDS_DomainParticipant_is_ipc_liveliness_enabled(participant) &&
        !DDS_IpcLiveliness_enable(participant->ipc_liveliness))
    {
        goto done;
    }
#endif

    if (participant->disc_plugin != NULL)
    {
        if (!NDDS_Discovery_Plugin_on_after_local_participant_enabled(
                participant->disc_plugin,
                participant,
                &participant->builtin_data))
        {
            DDSC_LOG_DISC_LOCAL_PARTICIPANT_ENABLED(OSAPI_LOGKIND_ERROR,
                            RT_ComponentFactoryId_get_name(
                                &participant->qos.discovery.discovery.name));
            goto done;
        }
    }

    /* Check if contained entities shall be enabled */
    if (!participant->qos.entity_factory.autoenable_created_entities)
    {
        retval = DDS_RETCODE_OK;
        goto done;
    }

    cursor = NULL;
    dbrc = DB_Table_select_all_default(participant->topic_table,
                                      &cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,participant->topic_table,dbrc)
        goto done;
    }

    do
    {
        dbrc = DB_Cursor_get_next(cursor, (DB_Record_T*)&topic);
        if ((dbrc == DB_RETCODE_OK) && !DDS_TopicImpl_is_hidden(topic))
        {
            if (DDS_Entity_enable(&topic->as_entity) != DDS_RETCODE_OK)
            {
                DDSC_LOG_ENTITY_ENABLE(OSAPI_LOGKIND_ERROR,DDSC_LOG_TOPIC_ENTITY)
                goto done;
            }
        }
    } while (dbrc == DB_RETCODE_OK);

    DB_Cursor_finish(participant->topic_table,cursor);

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    cursor = NULL;
    dbrc = DB_Table_select_all_default(participant->local_subscriber_table,
                                      &cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,participant->local_subscriber_table,dbrc)
        goto done;
    }

    do
    {
        dbrc = DB_Cursor_get_next(cursor, (DB_Record_T*)&local_subscriber);
        if ((dbrc == DB_RETCODE_OK) && !DDS_SubscriberImpl_is_hidden(local_subscriber))
        {
            if (DDS_Entity_enable(&local_subscriber->as_entity) != DDS_RETCODE_OK)
            {
                DDSC_LOG_ENTITY_ENABLE(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIBER_ENTITY)
                goto done;
            }
        }
    } while (dbrc == DB_RETCODE_OK);

    DB_Cursor_finish(participant->local_subscriber_table,cursor);

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    cursor = NULL;
    dbrc = DB_Table_select_all_default(participant->local_publisher_table,
                                       &cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,participant->local_publisher_table,dbrc)
        goto done;
    }

    do
    {
        dbrc = DB_Cursor_get_next(cursor, (DB_Record_T*)&local_publisher);
        if ((dbrc == DB_RETCODE_OK) && !DDS_PublisherImpl_is_hidden(local_publisher))
        {
            if (DDS_Entity_enable(&local_publisher->as_entity) != DDS_RETCODE_OK)
            {
                DDSC_LOG_ENTITY_ENABLE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLISHER_ENTITY)
                goto done;
            }
        }
    } while (dbrc == DB_RETCODE_OK);

    DB_Cursor_finish(participant->local_publisher_table,cursor);

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    retval = DDS_RETCODE_OK;

done:

    if (DB_Database_unlock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retval;
}

#ifndef RTI_CERT
RTI_PRIVATE RTI_BOOL
DDS_DomainParticipant_delete_user_data_manager(struct DDS_DomainParticipantImpl *participant)
{
    RTI_BOOL result = RTI_FALSE;

    DDS_UserDataManager_unregister_type(participant->user_data_manager,
                                        DDS_USER_DATA_PARTICIPANT_TYPE);
    DDS_UserDataManager_unregister_type(participant->user_data_manager,
                                        DDS_USER_DATA_TOPIC_TYPE);
    DDS_UserDataManager_unregister_type(participant->user_data_manager,
                                        DDS_USER_DATA_PUBLISHER_TYPE);
    DDS_UserDataManager_unregister_type(participant->user_data_manager,
                                        DDS_USER_DATA_SUBSCRIBER_TYPE);
    DDS_UserDataManager_unregister_type(participant->user_data_manager,
                                        DDS_USER_DATA_DATAWRITER_TYPE);
    DDS_UserDataManager_unregister_type(participant->user_data_manager,
                                        DDS_USER_DATA_DATAREADER_TYPE);

    if (!DDS_UserDataManager_delete(participant->user_data_manager))
    {
        goto done;
    }
    participant->user_data_manager = NULL;

    result = RTI_TRUE;

done:
    return result;
}
#endif

RTI_PRIVATE RTI_BOOL
DDS_DomainParticipant_create_user_data_manager(
        struct DDS_DomainParticipantImpl *participant,
        const DDS_DomainParticipantFactory *factory,
        const struct DDS_DomainParticipantQos *qos)
{
    RTI_BOOL result = RTI_FALSE;

#if INCLUDE_API_QOS
    if (qos == &DDS_PARTICIPANT_QOS_DEFAULT)
    {
        qos = &factory->default_participant_qos;
    }
#else
    UNUSED_ARG(factory);
#endif

    DDS_UserDataManager_T *user_data_manager = NULL;
    RTI_INT32 participant_user_data_max_count = qos->resource_limits.participant_user_data_max_count;
    RTI_INT32 topic_user_data_max_count = qos->resource_limits.topic_data_max_count;
    RTI_INT32 publisher_user_data_max_count = qos->resource_limits.publisher_group_data_max_count;
    RTI_INT32 subscriber_user_data_max_count = qos->resource_limits.subscriber_group_data_max_count;
    RTI_INT32 writer_user_data_max_count = qos->resource_limits.writer_user_data_max_count;
    RTI_INT32 reader_user_data_max_count = qos->resource_limits.reader_user_data_max_count;

    if (!DDS_UserDataManager_create(&user_data_manager, participant->database))
    {
        goto done;
    }
    participant->user_data_manager = user_data_manager;

    /* DDS_USER_DATA_PARTICIPANT_TYPE */
    if (qos->resource_limits.participant_user_data_max_length > 0)
    {
        if (participant_user_data_max_count == DDS_SIZE_AUTO)
        {
            participant_user_data_max_count = qos->resource_limits.remote_participant_allocation + 1;
        }
        if (!DDS_UserDataManager_register_type(
                    user_data_manager, DDS_USER_DATA_PARTICIPANT_TYPE,
                    qos->resource_limits.participant_user_data_max_length,
                    participant_user_data_max_count))
        {
            goto done;
        }
    }

    /* DDS_USER_DATA_TOPIC_TYPE */
    if (qos->resource_limits.topic_data_max_length > 0)
    {
        if (topic_user_data_max_count == DDS_SIZE_AUTO)
        {
            topic_user_data_max_count = qos->resource_limits.local_topic_allocation +
                                        qos->resource_limits.remote_writer_allocation +
                                        qos->resource_limits.remote_reader_allocation;
        }
        if (!DDS_UserDataManager_register_type(
                    user_data_manager, DDS_USER_DATA_TOPIC_TYPE,
                    qos->resource_limits.topic_data_max_length,
                    topic_user_data_max_count))
        {
            goto done;
        }
    }

    /* DDS_USER_DATA_PUBLISHER_TYPE */
    if (qos->resource_limits.publisher_group_data_max_length > 0)
    {
        if (publisher_user_data_max_count == DDS_SIZE_AUTO)
        {
            publisher_user_data_max_count = qos->resource_limits.local_publisher_allocation +
                                            qos->resource_limits.remote_writer_allocation;
        }
        if (!DDS_UserDataManager_register_type(
                    user_data_manager, DDS_USER_DATA_PUBLISHER_TYPE,
                    qos->resource_limits.publisher_group_data_max_length,
                    publisher_user_data_max_count))
        {
            goto done;
        }
    }

    /* DDS_USER_DATA_SUBSCRIBER_TYPE */
    if (qos->resource_limits.subscriber_group_data_max_length > 0)
    {
        if (subscriber_user_data_max_count == DDS_SIZE_AUTO)
        {
            subscriber_user_data_max_count = qos->resource_limits.local_subscriber_allocation +
                                             qos->resource_limits.remote_reader_allocation;
        }
        if (!DDS_UserDataManager_register_type(
                    user_data_manager, DDS_USER_DATA_SUBSCRIBER_TYPE,
                    qos->resource_limits.subscriber_group_data_max_length,
                    subscriber_user_data_max_count))
        {
            goto done;
        }
    }

    /* DDS_USER_DATA_DATAWRITER_TYPE */
    if (qos->resource_limits.writer_user_data_max_length > 0)
    {
        if (writer_user_data_max_count == DDS_SIZE_AUTO)
        {
            writer_user_data_max_count = qos->resource_limits.local_writer_allocation +
                                         qos->resource_limits.remote_writer_allocation;
        }
        if (!DDS_UserDataManager_register_type(
                    user_data_manager, DDS_USER_DATA_DATAWRITER_TYPE,
                    qos->resource_limits.writer_user_data_max_length,
                    writer_user_data_max_count))
        {
            goto done;
        }
    }

    /* DDS_USER_DATA_DATAREADER_TYPE */
    if (qos->resource_limits.reader_user_data_max_length > 0)
    {
        if (reader_user_data_max_count == DDS_SIZE_AUTO)
        {
            reader_user_data_max_count = qos->resource_limits.local_reader_allocation +
                                         qos->resource_limits.remote_reader_allocation;
        }
        if (!DDS_UserDataManager_register_type(
                    user_data_manager, DDS_USER_DATA_DATAREADER_TYPE,
                    qos->resource_limits.reader_user_data_max_length,
                    reader_user_data_max_count))
        {
            goto done;
        }
    }

    result = RTI_TRUE;

done:

#ifndef RTI_CERT
    if (!result)
    {
        if (user_data_manager != NULL)
        {
            DDS_DomainParticipant_delete_user_data_manager(participant);
        }
    }
#endif

    return result;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize a participant
 *
 * \details
 *
 * Finalize all resources used by a participant. Note that this function does
 * not delete contained entities, those must be deleted before this function
 * is called. Because the memory allocated for the participant is managed
 * by the participant factory, this function does not free the memory for the
 * participant (self) object.
 *
 * \param[in] self The participant to initialize
 *
 * \return DDS_RETCODE_OK on success, one of the error return codes on failure
 *
 * \sa \ref DDS_DomainParticipant_initialize
 */
DDS_ReturnCode_t
DDS_DomainParticipant_finalize(DDS_DomainParticipant *self)
{
   struct DDS_DomainParticipantImpl *participant =
                                        (struct DDS_DomainParticipantImpl*)self;
   DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
   DDS_BuiltinTopicKey_t key;
   DDS_Long count;
   DB_ReturnCode_T dbrc = DB_RETCODE_ERROR;
   struct RT_ComponentFactory *c_factory;
   DB_Cursor_T cursor = NULL;
   struct DDS_RemoteParticipantImpl *rem_participant;
   struct DDS_RemotePublicationImpl *rem_pub;
   struct DDS_RemoteSubscriptionImpl *rem_sub;
   DDS_Long i,j;
   NETIO_Interface_T *netio_intf = NULL;

   if (participant->database != NULL)
   {
       if (DB_Database_lock(participant->database) != DB_RETCODE_OK)
       {
           return DDS_RETCODE_ERROR;
       }
   }

   key = NDDS_DomainParticipant_get_key(self);

   /* Inform the discovery plugin that the participant is no longer available.
    * This is done independently of deleting remote participants since deleting
    * remote participants may remove locators and a dispose message would
    * not reach the destination.
    */
   if (self->disc_plugin != NULL)
   {
       if (!NDDS_Discovery_Plugin_on_dispose_local_participant(
                                               self->disc_plugin,self,&key))

       {
           DDSC_LOG_COMPONENT_DELETE(OSAPI_LOGKIND_ERROR,
                               RT_ComponentFactoryId_get_name(
                                       &self->qos.discovery.discovery.name),
                                       DDSC_LOG_DISCOVERY_COMPONENT)
           goto done;
       }
   }



   /* Deleting a DP follows the following rules
    * 1. All local publisher must be deleted
    * 2. All local subscribers must be deleted
    * 3. Remote participant liveliness timers must be deleted
    * 4. disc_plugin::on_before_local_participant_deleted must be called
    * 5. The built-in inter-participant channels must be deleted
    * 3. The built-in discovery readers must be deleted
    * 4. All threads (Sender/Receiver) must be deleted
    * 5. The Event thread must be deleted
    * 6. Generator and Interpreter must be deleted
    * 7. The OSAPI must be deleted
    */
   if (participant->remote_participant_table)
   {
       cursor = NULL;
       dbrc = DB_Table_select_all_default(
                              participant->remote_participant_table,&cursor);
       if (dbrc != DB_RETCODE_OK)
       {
           DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,participant->remote_participant_table,dbrc)
           goto done;
       }

       do
       {
           dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&rem_participant);
           if ((dbrc == DB_RETCODE_OK) &&
               (rem_participant->as_entity.state == RTIDDS_ENTITY_STATE_ENABLED))
           {
               if (!OSAPI_Timer_delete_timeout(rem_participant->timer,
                                       &rem_participant->lease_duration_event))
               {
                   DB_Cursor_finish(participant->remote_participant_table,cursor);
                   goto done;
               }

               /* Let the discovery plugin delete any information it added
                * as part of discovery of a remote endpoint.
                */
               if (participant->disc_plugin != NULL)
               {
                   if (!NDDS_Discovery_Plugin_on_before_remote_participant_deleted(
                                                           participant->disc_plugin,
                                                           participant,
                                                           &rem_participant->data,
                                                           rem_participant->status))
                   {
                       DDSC_LOG_DISC_BEFORE_REMOTE_PARTICIPANT_DELETED(OSAPI_LOGKIND_WARNING,
                                            RT_ComponentFactoryId_get_name(
                                               &participant->qos.discovery.discovery.name))
                       DB_Cursor_finish(participant->remote_participant_table,cursor);
                       goto done;
                   }
               }
           }
       } while (dbrc == DB_RETCODE_OK);

       DB_Cursor_finish(participant->remote_participant_table,cursor);

       if (dbrc != DB_RETCODE_NO_DATA)
       {
           DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
           goto done;
       }
   }

    /* Free any pending resources queued up, this must happen before the
     * discovery plugin is deleted
     */
    DDS_DiscoveryQueue_purge_by_prefix(participant,
                                       &participant->discovery_queue,NULL);

   if (self->disc_plugin != NULL)
   {
       if (!NDDS_Discovery_Plugin_on_before_local_participant_deleted(
                                   self->disc_plugin,self,&key))
       {
           DDSC_LOG_COMPONENT_DELETE(OSAPI_LOGKIND_ERROR,
                               RT_ComponentFactoryId_get_name(
                                       &self->qos.discovery.discovery.name),
                                       DDSC_LOG_DISCOVERY_COMPONENT)
           goto done;
       }
   }

   if (participant->remote_publisher_table)
    {
        cursor = NULL;
        dbrc = DB_Table_select_all_default(
                              participant->remote_publisher_table,&cursor);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,participant->remote_publisher_table,dbrc)
            goto done;
        }

        do
        {
            dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&rem_pub);
            if (dbrc == DB_RETCODE_OK)
            {
                if (!DDS_PublicationBuiltinTopicData_finalize_no_dealloc(
                                                        &rem_pub->data, self))
                {
                    DDSC_LOG_OBJECT_FINALIZE(OSAPI_LOGKIND_WARNING,
                                            DDSC_LOG_PUBLICATIONDATA_OBJECT)
                }
                dbrc = DB_Table_delete_record(
                                   participant->remote_publisher_table,rem_pub);
                if (dbrc != DB_RETCODE_OK)
                {
                    DDSC_LOG_RECORD_DELETE(OSAPI_LOGKIND_WARNING,
                                          DDSC_LOG_PUBLICATION_RECORD,dbrc)
                }
            }
        } while (dbrc == DB_RETCODE_OK);
        DB_Cursor_finish(participant->remote_publisher_table,cursor);

        if (dbrc != DB_RETCODE_NO_DATA)
        {
            DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
            goto done;
        }
    }

    if (participant->remote_subscriber_table)
    {
        cursor = NULL;
        dbrc = DB_Table_select_all_default(
                              participant->remote_subscriber_table,&cursor);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,
                                   participant->remote_subscriber_table,dbrc)
            goto done;
        }

        do
        {
            dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&rem_sub);
            if (dbrc == DB_RETCODE_OK)
            {
                if (!DDS_SubscriptionBuiltinTopicData_finalize_no_dealloc(
                                                        &rem_sub->data, self))
                {
                    DDSC_LOG_OBJECT_FINALIZE(OSAPI_LOGKIND_WARNING,
                                          DDSC_LOG_SUBSCRIPTIONDATA_OBJECT)
                }
                dbrc = DB_Table_delete_record(
                       participant->remote_subscriber_table,rem_sub);
                if (dbrc != DB_RETCODE_OK)
                {
                    DDSC_LOG_RECORD_DELETE(OSAPI_LOGKIND_WARNING,
                                          DDSC_LOG_SUBSCRIPTION_RECORD,dbrc)
                }
            }
        } while (dbrc == DB_RETCODE_OK);

        DB_Cursor_finish(participant->remote_subscriber_table,cursor);

        if (dbrc != DB_RETCODE_NO_DATA)
        {
            DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
            goto done;
        }
    }


    if (participant->remote_participant_table)
    {
        cursor = NULL;
        dbrc = DB_Table_select_all_default(
                              participant->remote_participant_table,&cursor);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,participant->remote_participant_table,dbrc)
            goto done;
        }
        do
        {
            dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&rem_participant);
            if (dbrc == DB_RETCODE_OK)
            {
#ifndef RTI_CERT
                if (!NDDS_DomainParticipant_delete_remote_participant_routes(participant,rem_participant))
                {
                    DDSC_LOG_OBJECT_FINALIZE(OSAPI_LOGKIND_WARNING,
                                          DDSC_LOG_PARTICIPANTDATA_OBJECT)
                }
#endif
                if (!NDDS_RemoteParticipantRecord_finalize(
                       rem_participant,participant))
                {
                    DDSC_LOG_OBJECT_FINALIZE(OSAPI_LOGKIND_WARNING,
                                          DDSC_LOG_PARTICIPANTDATA_OBJECT)
                }
                dbrc = DB_Table_delete_record(
                       participant->remote_participant_table,rem_participant);
                if (dbrc != DB_RETCODE_OK)
                {
                    DDSC_LOG_RECORD_DELETE(OSAPI_LOGKIND_WARNING,
                                          DDSC_LOG_PARTICIPANT_RECORD,dbrc)
                }
            }
        } while (dbrc == DB_RETCODE_OK);

        DB_Cursor_finish(participant->remote_participant_table,cursor);
        if (dbrc != DB_RETCODE_NO_DATA)
        {
            DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
            goto done;
        }
    }

#if DDS_LIVELINESS_CHANNEL_ENABLED
    if (DDS_DomainParticipant_is_ipc_liveliness_enabled(participant) &&
        !DDS_IpcLiveliness_delete(participant->ipc_liveliness))
    {
        DDSC_LOG_IPC_FINALIZE_FAILED(OSAPI_LOGKIND_ERROR);
        goto done;
    }
    DDS_IpcLiveliness_unreserve(participant);
#endif

    if (participant->builtin_subscriber != NULL)
    {
        retcode = DDS_DomainParticipant_delete_subscriber(
                    participant, participant->builtin_subscriber);
        if (retcode != DDS_RETCODE_OK)
        {
            DDSC_LOG_RECORD_DELETE(OSAPI_LOGKIND_WARNING,
                                   DDSC_LOG_SUBSCRIBER_RECORD,
                                   retcode)
            goto done;
        }
        participant->builtin_subscriber = NULL;
    }

    if (participant->builtin_publisher != NULL)
    {
        retcode = DDS_DomainParticipant_delete_publisher(
                    participant, participant->builtin_publisher);
        if (retcode != DDS_RETCODE_OK)
        {
            DDSC_LOG_RECORD_DELETE(OSAPI_LOGKIND_WARNING,
                                   DDSC_LOG_PUBLISHER_RECORD,
                                   retcode)
            goto done;
        }
        participant->builtin_publisher = NULL;
    }

    retcode = DDS_RETCODE_ERROR;

    if (participant->local_publisher_table)
    {
        cursor = NULL;
        dbrc = DB_Table_select_all_default(
                                    participant->local_publisher_table,&cursor);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,participant->local_publisher_table,dbrc)
            goto done;
        }

        dbrc = DB_Cursor_get_count(cursor,&count);
        DB_Cursor_finish(participant->local_publisher_table,cursor);

        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,participant->local_publisher_table,dbrc)
            goto done;
        }

        if (count > 0)
        {
            DDSC_LOG_TABLE_INUSE(OSAPI_LOGKIND_ERROR,participant->local_publisher_table,count)
            retcode = DDS_RETCODE_ERROR;
            goto done;
        }
    }

    if (participant->local_subscriber_table)
    {
        cursor = NULL;
        dbrc = DB_Table_select_all_default(
                        participant->local_subscriber_table,&cursor);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,participant->local_subscriber_table,dbrc)
            goto done;
        }

        dbrc = DB_Cursor_get_count(cursor,&count);
        DB_Cursor_finish(participant->local_subscriber_table,cursor);

        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,participant->local_subscriber_table,dbrc)
            goto done;
        }

        if (count > 0)
        {
            DDSC_LOG_TABLE_INUSE(OSAPI_LOGKIND_ERROR,participant->local_subscriber_table,count)
            retcode = DDS_RETCODE_ERROR;
            goto done;
        }
    }

    if(participant->string_manager != NULL)
    {
        if (!DDS_StringManager_delete(&participant->string_manager))
        {
            DDSC_LOG_OBJECT_DELETE(OSAPI_LOGKIND_ERROR,DDSC_LOG_STRINGMANAGER_OBJECT)
            goto done;
        }
    }


    /* All of our resources */
    if (participant->database)
    {
        if (participant->local_reader_table)
        {
            dbrc = DB_Database_delete_table(participant->database,
                                            participant->local_reader_table);
            if (dbrc != DB_RETCODE_OK)
            {
                DDSC_LOG_TABLE_DELETE(OSAPI_LOGKIND_ERROR,participant->local_reader_table,dbrc)
                retcode = DDS_RETCODE_ERROR;
                goto done;
            }
        }



        if (participant->local_writer_table)
        {
            dbrc = DB_Database_delete_table(participant->database,
                                          participant->local_writer_table);
            if (dbrc != DB_RETCODE_OK)
            {
                DDSC_LOG_TABLE_DELETE(OSAPI_LOGKIND_ERROR,participant->local_writer_table,dbrc)
                retcode = DDS_RETCODE_ERROR;
                goto done;
            }
        }

        if (participant->local_publisher_table)
        {
            dbrc = DB_Database_delete_table(participant->database,
                                          participant->local_publisher_table);
            if (dbrc != DB_RETCODE_OK)
            {
                DDSC_LOG_TABLE_DELETE(OSAPI_LOGKIND_ERROR,participant->local_publisher_table,dbrc)
                retcode = DDS_RETCODE_ERROR;
                goto done;
            }
        }

        if (participant->local_subscriber_table)
        {
            dbrc = DB_Database_delete_table(participant->database,
                                          participant->local_subscriber_table);
            if (dbrc != DB_RETCODE_OK)
            {
                DDSC_LOG_TABLE_DELETE(OSAPI_LOGKIND_ERROR,participant->local_subscriber_table,dbrc)
                retcode = DDS_RETCODE_ERROR;
                goto done;
            }
        }

        if (participant->remote_participant_table)
        {
            dbrc = DB_Database_delete_table(participant->database,
                                          participant->remote_participant_table);
            if (dbrc != DB_RETCODE_OK)
            {
                DDSC_LOG_TABLE_DELETE(OSAPI_LOGKIND_ERROR,participant->remote_participant_table,dbrc)
                retcode = DDS_RETCODE_ERROR;
                goto done;
            }
        }

        if (participant->remote_publisher_table)
        {
            dbrc = DB_Database_delete_table(participant->database,
                                          participant->remote_publisher_table);
            if (dbrc != DB_RETCODE_OK)
            {
                DDSC_LOG_TABLE_DELETE(OSAPI_LOGKIND_ERROR,participant->remote_publisher_table,dbrc)
                retcode = DDS_RETCODE_ERROR;
                goto done;
            }
        }

        if (participant->remote_subscriber_table)
        {
            dbrc = DB_Database_delete_table(participant->database,
                                          participant->remote_subscriber_table);
            if (dbrc != DB_RETCODE_OK)
            {
                DDSC_LOG_TABLE_DELETE(OSAPI_LOGKIND_ERROR,participant->remote_subscriber_table,dbrc)
                retcode = DDS_RETCODE_ERROR;
                goto done;
            }
        }

        if (participant->topic_table)
        {
            dbrc = DB_Database_delete_table(participant->database,
                                          participant->topic_table);
            if (dbrc != DB_RETCODE_OK)
            {
                DDSC_LOG_TABLE_DELETE(OSAPI_LOGKIND_ERROR,participant->topic_table,dbrc)
                retcode = DDS_RETCODE_ERROR;
                goto done;
            }
        }

        if (participant->type_table)
        {
            dbrc = DB_Database_delete_table(participant->database,
                                            participant->type_table);
            if (dbrc != DB_RETCODE_OK)
            {
                DDSC_LOG_TABLE_DELETE(OSAPI_LOGKIND_ERROR,participant->type_table,dbrc)
                retcode = DDS_RETCODE_ERROR;
                goto done;
            }
        }
    }

    if (!DDS_DiscoveryQueue_finalize(&participant->discovery_queue))
    {
        DDSC_LOG_QUEUE_FINALIZE(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    /* It is safe to unlock the data base here. There are no shared resources
     * between the DDS and the transports, except the database. Since the
     * database is shared, avoid a deadlock as DDS sends messages to itself to
     * allow receive threads to unblock.
     */
    if (participant->database != NULL)
    {
        if (DB_Database_unlock(participant->database) != DB_RETCODE_OK)
        {
            return DDS_RETCODE_ERROR;
        }
    }

#if DDS_FLOW_CONTROLLER_ENABLED
    /* The flow-controller uses the database and runs its own thread. To
     * prevent deadlock it must be finalized _after_ the participant
     * release the DB lock. The flow-controllers are empty and does
     * not access any resources that have been deleted.
     */
    if (!DDS_FlowControl_finalize(&participant->flow_control,
                                  RTI_FALSE))
    {
        goto done;
    }
#endif

    if (!NETIO_BindResolver_release_addresses(
            participant->bind_resolver,
            (struct REDA_StringSeq*)&participant->qos.transports.enabled_transports,
            NETIO_ROUTEKIND_META,
            (struct NETIO_AddressSeq*)&participant->builtin_data.metatraffic_multicast_locators))
    {
        DDSC_LOG_RELEASE_META_MC(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    if (!NETIO_BindResolver_release_addresses(
            participant->bind_resolver,
            (struct REDA_StringSeq*)&participant->qos.transports.enabled_transports,
            NETIO_ROUTEKIND_META,
            (struct NETIO_AddressSeq*)&participant->builtin_data.metatraffic_unicast_locators))
    {
        DDSC_LOG_RELEASE_META_UC(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    if (!NETIO_BindResolver_release_addresses(
            participant->bind_resolver,
            (struct REDA_StringSeq*)&participant->qos.transports.enabled_transports,
            NETIO_ROUTEKIND_USER,
            (struct NETIO_AddressSeq*)&participant->builtin_data.default_multicast_locators))
    {
        DDSC_LOG_RELEASE_USER_MC(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    if (!NETIO_BindResolver_release_addresses(
            participant->bind_resolver,
            (struct REDA_StringSeq*)&participant->qos.transports.enabled_transports,
            NETIO_ROUTEKIND_USER,
            (struct NETIO_AddressSeq*)&participant->builtin_data.default_unicast_locators))
    {
        DDSC_LOG_RELEASE_USER_UC(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    if (participant->database != NULL)
    {
        if (DB_Database_lock(participant->database) != DB_RETCODE_OK)
        {
            return DDS_RETCODE_ERROR;
        }
    }

    if (participant->address_resolver != NULL)
    {
        for (i = 0;
             i < DDS_StringSeq_get_length(&participant->qos.transports.enabled_transports);
             ++i)
        {
            RT_ComponentFactoryId_T id;
            const char *id_name;
            netio_intf = NULL;

            id_name = *DDS_StringSeq_get_reference(
                            &participant->qos.transports.enabled_transports,i);
            if (!RT_ComponentFactoryId_set_name(&id,id_name))
            {
                goto done;
            }

            if (!NETIO_AddressResolver_delete_interface(participant->address_resolver,
                                                        id_name,NULL,&netio_intf))
            {
                goto done;
            }

            /* There is nothing to do if the interface is NULL*/
            if (netio_intf == NULL)
            {
                continue;
            }

            if (!NETIO_AddressSeq_set_length(&participant->routes,0))
            {
                DDSC_LOG_SEQ_SETLENGTH(OSAPI_LOGKIND_ERROR,DDSC_LOG_ROUTE_SEQUENCE,0)
                goto done;
            }

            if (!NETIO_NetmaskSeq_set_length(&participant->netmasks,0))
            {
                DDSC_LOG_SEQ_SETLENGTH(OSAPI_LOGKIND_ERROR,DDSC_LOG_NETMASK_SEQUENCE,0)
                goto done;
            }

            /* Rely on error code set by get route table function */
            if (!NETIO_Interface_get_route_table(netio_intf,
                                    &participant->routes,&participant->netmasks))
            {
                DDSC_LOG_NETIO_GET_ROUTE_TABLE_FAILED(OSAPI_LOGKIND_ERROR,id_name)
                goto done;
            }

            for (j = 0; j < NETIO_AddressSeq_get_length(&participant->routes); ++j)
            {
                struct NETIO_Address *routes =
                        NETIO_AddressSeq_get_reference(&participant->routes,j);
                struct NETIO_Netmask *netmasks =
                        NETIO_NetmaskSeq_get_reference(&participant->netmasks,j);

                if ((routes == NULL) || (netmasks == NULL))
                {
                    goto done;
                }

                /* Rely on error code set by delete_interface */
                if (!NETIO_RouteResolver_delete_interface(participant->route_resolver,
                        netio_intf,
                        routes,
                        netmasks,
                        NULL))
                {
                    goto done;
                }
            }

            c_factory = RT_Registry_lookup(participant->config.registry,
                    *DDS_StringSeq_get_reference(
                            &participant->qos.transports.enabled_transports,i));
            if (c_factory == NULL)
            {
                DDSC_LOG_COMPONENT_LOOKUP(OSAPI_LOGKIND_ERROR,
                    *DDS_StringSeq_get_reference(
                    &participant->qos.transports.enabled_transports,i))
                goto done;
            }

            NETIO_InterfaceFactory_delete_component(c_factory,netio_intf);
        }

        if (!NETIO_AddressResolver_delete(participant->address_resolver))
        {
            DDSC_LOG_OBJECT_DELETE(OSAPI_LOGKIND_ERROR,
                                  DDSC_LOG_ADDRESSRESOLVER_OBJECT)
            goto done;
        }
        participant->address_resolver = NULL;
    }

    if (participant->rtps_intf != NULL)
    {
        c_factory = RT_Registry_lookup(participant->config.registry,NETIO_DEFAULT_RTPS_NAME);
        if (c_factory != NULL)
        {
            NETIO_InterfaceFactory_delete_component(c_factory,
                                                   participant->rtps_intf);
        }
        else
        {
            DDSC_LOG_COMPONENT_LOOKUP(OSAPI_LOGKIND_ERROR,NETIO_DEFAULT_RTPS_NAME)
        }
        participant->rtps_intf = NULL;
    }
    if ((participant->network_lock != NULL) &&
        !OSAPI_Mutex_delete(participant->network_lock))
    {
        DDSC_LOG_OBJECT_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_MUTEX_OBJECT)
        goto done;
    }

    if (!NETIO_AddressSeq_finalize(&participant->routes))
    {
        DDSC_LOG_SEQ_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_ROUTE_SEQUENCE)
        goto done;
    }

    if (!NETIO_NetmaskSeq_finalize(&participant->netmasks))
    {
        DDSC_LOG_SEQ_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_NETMASK_SEQUENCE)
        goto done;
    }

    if (participant->bind_resolver != NULL)
    {
        if (!NETIO_BindResolver_delete(participant->bind_resolver))
        {
            DDSC_LOG_OBJECT_DELETE(OSAPI_LOGKIND_ERROR,
                                  DDSC_LOG_BINDRESOLVER_OBJECT)
            goto done;
        }
        participant->bind_resolver = NULL;
    }

    if (participant->route_resolver != NULL)
    {
        if (!NETIO_RouteResolver_delete(participant->route_resolver))
        {
            DDSC_LOG_OBJECT_DELETE(OSAPI_LOGKIND_ERROR,
                                  DDSC_LOG_ROUTERESOLVER_OBJECT)
            goto done;
        }
        participant->route_resolver = NULL;
    }

    if (self->disc_plugin != NULL)
    {
        if (self->qos.discovery.discovery.name._name._name[0] != 0)
        {
            c_factory = RT_Registry_lookup(self->config.registry,
                           RT_ComponentFactoryId_get_name(
                                   &participant->qos.discovery.discovery.name));
            if (c_factory == NULL)
            {
                DDSC_LOG_COMPONENT_LOOKUP(OSAPI_LOGKIND_ERROR,
                                   RT_ComponentFactoryId_get_name(
                                       &self->qos.discovery.discovery.name));
                goto done;
            }
            DiscoveryComponentFactory_delete_component(c_factory,
                                                      self->disc_plugin);
            self->disc_plugin = NULL;
        }
    }

#if DDS_FILTERING_ENABLED
    if (!DDS_DomainParticipant_finalize_filter_plugin(self))
    {
        DDSC_LOG_FINALIZE_FILTER_PLUGIN(OSAPI_LOGKIND_ERROR)
        goto done;
    }
#endif

    if (DDS_DomainParticipantQos_finalize_managed(&self->qos, self) != DDS_RETCODE_OK)
    {
        DDSC_LOG_OBJECT_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANTQOS_OBJECT)
        goto done;
    }

    if (self->_default_publisher_qos != &DDS_PUBLISHER_QOS_DEFAULT)
    {
        if (DDS_PublisherQos_finalize_managed(self->_default_publisher_qos, self) != DDS_RETCODE_OK)
        {
            DDSC_LOG_OBJECT_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLISHERQOS_OBJECT)
            goto done;
        }
        OSAPI_Heap_free_struct(self->_default_publisher_qos);
    }

    if (self->_default_subscriber_qos != &DDS_SUBSCRIBER_QOS_DEFAULT)
    {

        if (DDS_SubscriberQos_finalize_managed(self->_default_subscriber_qos, self) != DDS_RETCODE_OK)
        {
            DDSC_LOG_OBJECT_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIBERQOS_OBJECT)
            goto done;
        }
         OSAPI_Heap_free_struct(self->_default_subscriber_qos);
    }

    if (self->_default_topic_qos != &DDS_TOPIC_QOS_DEFAULT)
    {
        if (DDS_TopicQos_finalize_managed(self->_default_topic_qos, self) != DDS_RETCODE_OK)
        {
            DDSC_LOG_OBJECT_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_TOPICQOS_OBJECT)
            goto done;
        }
        OSAPI_Heap_free(self->_default_topic_qos);
    }

    if (!DDS_ParticipantBuiltinTopicData_finalize_no_dealloc(&self->builtin_data, self))
    {
        DDSC_LOG_OBJECT_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANTDATA_OBJECT)
        goto done;
    }
    if (!DDS_ParticipantBuiltinTopicData_finalize(&self->builtin_data))
    {
        DDSC_LOG_OBJECT_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANTDATA_OBJECT)
        goto done;
    }

    if(participant->partition_string_manager != NULL)
    {
        if (!DDS_StringManager_delete(&participant->partition_string_manager))
        {
            DDSC_LOG_OBJECT_DELETE(OSAPI_LOGKIND_ERROR,DDSC_LOG_STRINGMANAGER_OBJECT)
            goto done;
        }
    }

#ifndef RTI_CERT
    if (participant->user_data_manager != NULL)
    {
        if (!DDS_DomainParticipant_delete_user_data_manager(participant))
        {
            goto done;
        }
    }
#endif

    if (participant->packet_pool != NULL)
    {
        if (!REDA_BufferPool_delete(participant->packet_pool))
        {
            DDSC_LOG_NETIO_PACKETPOOL_DELETE(OSAPI_LOGKIND_ERROR)
            goto done;
        }
        participant->packet_pool = NULL;
    }

    if (!DDS_LocatorSeq_unloan(&participant->resolved_unicast_seq))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_unloan(&participant->resolved_multicast_seq))
    {
        goto done;
    }

#ifndef RTI_CERT
    if (!DDS_LocatorSeq_finalize(&participant->resolved_unicast_seq))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_finalize(&participant->resolved_multicast_seq))
    {
        goto done;
    }
#endif

    if (self->database != NULL)
    {
        if (DB_Database_unlock(participant->database) != DB_RETCODE_OK)
        {
            goto done;
        }
        dbrc = DB_Database_delete(participant->database);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_DATABASE_DELETE(OSAPI_LOGKIND_ERROR,dbrc)
            retcode = DDS_RETCODE_ERROR;
            goto done;
        }
        participant->database  = NULL;

        if (self->timer)
        {
            if (!OSAPI_Timer_delete(self->timer))
            {
                goto done;
            }
        }

        if (!OSAPI_Mutex_delete(self->db_lock))
        {
            goto done;
        }
    }

    if (!DDS_EntityImpl_finalize(&self->as_entity))
    {
        DDSC_LOG_ENTITY_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_ENTITY)
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:

    if ((retcode != DDS_RETCODE_OK) && (participant->database != NULL))
    {
        if (DB_Database_unlock(participant->database) != DB_RETCODE_OK)
        {
            return DDS_RETCODE_ERROR;
        }
    }

    return retcode;

}
#endif /* !RTI_CERT */

/*ci
 * \brief Verify that the discovery enabled transports contains only allowed transports
 *
 * \details Checks that the discovery enabled transports does not contain any
 *          any locators which correspond to a notification interface factory.
 *
 * \param[in] participant The participant
 * \param[in] qos         The participant's QoS
 *
 * \return RTI_TRUE if the discovery enabled transports are valid, RTI_FALSE otherwise
 */
RTI_PRIVATE RTI_BOOL
DDS_DomainParticipant_verify_discovery_transports(
        struct DDS_DomainParticipantImpl *participant,
        const struct DDS_DomainParticipantQos *qos)
{
    RTI_BOOL success = RTI_FALSE;
    RTI_INT32 i;
    RTI_UINT32 base_port;
    RTI_INT32 low_index;
    RTI_INT32 high_index;
    RT_ComponentFactoryId_T id;
    char address_string[NETIO_ADDRESS_TOKEN_MAX_SIZE];
    char *empty_string = "";

    struct RT_ComponentFactory *factory;

    for (i = 0; i < DDS_StringSeq_get_length(&qos->discovery.enabled_transports); ++i)
    {
        char *transport = NULL;
        transport = *DDS_StringSeq_get_reference(&qos->discovery.enabled_transports, i);
        if (transport == NULL)
        {
            goto done;
        }

        /* Only process transports wich are different from an empty string */
        if (OSAPI_String_cmp(empty_string, transport) == 0)
        {
            continue;
        }

        if (!NETIO_Address_parse(
                    transport,
                    &base_port,
                    &low_index,
                    &high_index,
                    &id,
                    address_string,
                    NETIO_ADDRESS_TOKEN_MAX_SIZE))
        {
            goto done;
        }

        factory = RT_Registry_lookup(
                participant->config.registry,
                RT_ComponentFactoryId_get_name(&id));
        if (factory == NULL)
        {
            DDSC_LOG_COMPONENT_LOOKUP(
                    OSAPI_LOGKIND_ERROR,
                    RT_ComponentFactoryId_get_name(&id))
            goto done;
        }

        /* R-100.3.4: Notification cannot be used for discovery */
        if (RT_INTERFACE_INSTANCE(RT_ComponentFactory_get_id(factory)) ==
            RT_COMPONENT_INSTANCE_NOTIF)
        {
            DDSC_LOG_NETIO_DISCOVERY_ENABLED_TRANSPORT(
                    OSAPI_LOGKIND_ERROR,
                    RT_ComponentFactoryId_get_name(&id))
            goto done;
        }
    }

    success = RTI_TRUE;
done:
    return success;
}

/*ci
 * \brief Initialize the discovery queue if configured
 *
 * \details
 *
 * The discovery queue is only 4 pointers, optional allocation would save
 * at most 24 bytes. The queue itself is always in a valid state, but
 * the publication and subscription queue may be not enabled after successful
 * initialization.
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
RTI_PRIVATE DDS_Boolean
DDS_DomainParticipant_initialize_discovery_queue(
                          struct DDS_DomainParticipantImpl *participant,
                          const struct DDS_DomainParticipantQos *qos)
{
    struct DDS_DiscoveryQueueProperty dq_prop =
                                        DDS_DiscoveryQueueProperty_INITIALIZER;
    DDS_DataReader *disc_reader;
    UNUSED_ARG(qos);

    if (qos->discovery.enable_endpoint_discovery_queue
        && (participant->disc_plugin != NULL)
        && NDDS_Discovery_Plugin_has_on_publication_data_return_loan(participant->disc_plugin)
        && NDDS_Discovery_Plugin_has_on_subscription_data_return_loan(participant->disc_plugin))
    {
        /* If the disocvery plugin has return_loan functions, allocate the
         * queue to be able to handle pending resources. In order to do that
         * it is necessary to get the max_samples for the builtin publication
         * and subscription reader.
         */
        disc_reader = DDS_Subscriber_lookup_datareader(
                                    participant->builtin_subscriber,
                                    DDS_PUBLICATION_BUILTIN_TOPIC_NAME);

        if (disc_reader != NULL)
        {
            dq_prop.max_publication_queue =
                                    DDS_DataReader_get_max_samples(disc_reader);
        }

        disc_reader = DDS_Subscriber_lookup_datareader(
                                    participant->builtin_subscriber,
                                    DDS_SUBSCRIPTION_BUILTIN_TOPIC_NAME);

        if (disc_reader != NULL)
        {
            dq_prop.max_subscription_queue =
                                    DDS_DataReader_get_max_samples(disc_reader);
        }
    }

    if (!DDS_DiscoveryQueue_initialize(&participant->discovery_queue,&dq_prop))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Initialize a participant
 *
 * \details
 *
 * Initialize and allocate all resources used by a participant. Note that
 * this function is passed the memory to use for the participant itself from
 * the participant factory.
 *
 * \param[in] participant     The participant to initialize
 * \param[in] factory         The participant factory that created this participant
 * \param[in] domain_id       The domain to create the participant in
 * \param[in] qos             The qos policies the participant was created with
 * \param[in] listener        The listener the participant was created with
 * \param[in] mask            The listener mask the participant was created with
 * \param[in] config          The participant configuration from the factory
 * \param[in] factory_lock    The shared lock to ensure thread-safety for
 *                            participant APIs
 *
 * \return Pointer to initialized participant on success, NULL on failure
 *
 * \sa \ref DDS_DomainParticipant_finalize
 */
DDS_DomainParticipant*
DDS_DomainParticipant_initialize(
                          struct DDS_DomainParticipantImpl *participant,
                          DDS_DomainParticipantFactory *factory,
                          DDS_DomainId_t domain_id,
                          const struct DDS_DomainParticipantQos *qos,
                          const struct DDS_DomainParticipantListener *listener,
                          DDS_StatusMask mask,
                          struct NDDS_ParticipantConfig *config,
                          struct OSAPI_Mutex *factory_lock)
{
    DDS_Boolean success = DDS_BOOLEAN_FALSE;
    DDS_ReturnCode_t rc;
    DDS_Long i,j;
    struct OSAPI_TimerProperty timer_property = OSAPI_TimerProperty_INITIALIZER;
    struct RT_ComponentFactory *c_factory = NULL;
    struct RT_ComponentFactory *udp_factory = NULL;
    struct NDDS_Discovery_Property disc_property =
                                        NDDS_Discovery_Property_INITIALIZER;
    struct NDDS_Discovery_Listener disc_listener =
                                        NDDS_Discovery_Listener_INITIALIZE;
    struct LOOP_InterfaceProperty lo_property =
                                        LOOP_InterfaceProperty_INITIALIZER;
    struct NETIO_InterfaceProperty intf_property = NETIO_InterfaceProperty_INITIALIZER;
    struct NETIO_BindResolverProperty rc_prop = NETIO_BindResolverProperty_INITIALIZER;
    struct NETIO_RouteResolverProperty rte_prop = NETIO_RouteResolverProperty_INITIALIZER;
    NETIO_Interface_T *netio_intf = NULL;
    struct NETIO_Address src_address;
    RTI_INT32 try_id,max_id;
    struct RTPS_InterfaceProperty rtps_property =
                                            RTPS_InterfaceProperty_INITIALIZER;
    struct NETIO_AddressResolverProperty nar_property =
                                     NETIO_AddressResolverProperty_INITIALIZER;
    struct ZCOPY_NotifInterfaceProperty notif_property =
                                     ZCOPY_NotifInterfaceProperty_INITIALIZER;
    struct DDS_StringManagerProperty str_man_prop =
                                        DDS_StringManagerProperty_INITIALIZER;
    struct DDS_StringManagerProperty partition_str_man_prop =
                                        DDS_StringManagerProperty_INITIALIZER;
    DDS_InstanceHandle_t instance_handle;
    struct DDS_DomainParticipantListener nil_listener =
        DDS_DomainParticipantListener_INITIALIZER;
#ifndef RTI_CERT
    RTI_SIZE_T def_dp_name_len = OSAPI_String_length(DDS_DEFAULT_PARTICIPANT_NAME);
#endif
    RTI_UINT32 v_idx = 0;
    RTI_INT32 min_mtu;

    OSAPI_Memory_zero(participant,sizeof(struct DDS_DomainParticipantImpl));

    /* A precondition check has already been performed by the calling routine */
    if (factory == NULL || qos == NULL)
    {
        goto done;
    }

#if INCLUDE_API_QOS
    /* The consts are casted away on purpose. We never write to these variables
     * unless they have been allocated from the heap.
     */
    participant->_default_publisher_qos = (struct DDS_PublisherQos*)&DDS_PUBLISHER_QOS_DEFAULT;
    participant->_default_subscriber_qos = (struct DDS_SubscriberQos*)&DDS_SUBSCRIBER_QOS_DEFAULT;
    participant->_default_topic_qos = (struct DDS_TopicQos*)&DDS_TOPIC_QOS_DEFAULT;
#endif

    /* Save information which can be retrieved later. Need to validate it */
    if ((qos != &DDS_PARTICIPANT_QOS_DEFAULT) &&
        !DDS_DomainParticipantQos_is_consistent(qos))
    {
        DDSC_LOG_QOS_INCONSISTENT(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_QOS)
        goto done;
    }

    if ((listener != NULL) &&
            !DDS_DomainParticipantListener_is_consistent(listener,mask))
    {
        DDSC_LOG_LISTENER_INCONSISTENT(OSAPI_LOGKIND_ERROR,
                                       DDSC_LOG_PARTICIPANT_LISTENER,mask)
        goto done;
    }

    if (!DDS_PropertyQosPolicy_is_valid(&qos->property,
                                        DDS_PARTICIPANT_ENTITY_KIND))
    {
        DDSC_LOG_ENTITY_INVALID_PROPERTY(OSAPI_LOGKIND_ERROR,
                                         DDS_PARTICIPANT_ENTITY_KIND)
        goto done;
    }

    participant->config = *config;

    if ((qos->protocol.rtps_well_known_ports.participant_id_gain >=
         qos->protocol.rtps_well_known_ports.domain_id_gain) &&
        (domain_id >=
                (qos->protocol.rtps_well_known_ports.participant_id_gain
                 / qos->protocol.rtps_well_known_ports.domain_id_gain)))
    {
        DDSC_LOG_INVALID_DOMAINID(OSAPI_LOGKIND_ERROR,domain_id)
        goto done;
    }

    if (!DDS_LocatorSeq_initialize(&participant->resolved_unicast_seq))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_loan_contiguous(&participant->resolved_unicast_seq,
                                        &participant->resolved_unicast_arr,
                                        0,
                                        RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_initialize(&participant->resolved_multicast_seq))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_loan_contiguous(&participant->resolved_multicast_seq,
                                        &participant->resolved_multicast_arr,
                                        0,
                                        RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX))
    {
        goto done;
    }

    participant->domain_id = domain_id;

    DDS_GUID_set_suffix(&participant->config.guid,RTPS_OBJECT_ID_PARTICIPANT);

    /*Automatically assigned object-ids must be >=
     * OSAPI_SYSTEM_OBJECTID_START. This is because DDS entities, which is assigned
     * an object-id using this function, can specify their own
     * object-id < OSAPI_SYSTEM_OBJECTID_START.
     */
    participant->current_object_id = (OSAPI_SYSTEM_OBJECTID_START - 1);

    if (!DDS_EntityImpl_initialize(&participant->as_entity,
            DDS_PARTICIPANT_ENTITY_KIND,
            RTPS_OBJECT_ID_PARTICIPANT,
            DDS_DomainParticipant_enable,
            DDS_DomainParticipantImpl_get_instance_handle))
    {
        DDSC_LOG_ENTITY_INITIALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_ENTITY)
        goto done;
    }

    participant->db_lock = OSAPI_Mutex_new();
    if (participant->db_lock == NULL)
    {
        DDSC_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_MUTEX_OBJECT)
        goto done;
    }

    for (i = 0; i < DDS_PARTICIPANT_USER_DATA_MAX_ELEMENT; i++)
    {
        participant->user_data[i] = NULL;
    }

    participant->database = NULL;
    participant->shared_lock = factory_lock;

    rc = DDS_DomainParticipantQos_initialize(&participant->qos);
    if (rc != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_INITIALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_QOS,rc)
        goto done;
    }

#if INCLUDE_API_QOS
    if (qos == &DDS_PARTICIPANT_QOS_DEFAULT)
    {
        if (!DDS_DomainParticipantQos_copy_unmanaged_fields(
                &participant->qos, &factory->default_participant_qos))
        {
            DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_QOS)
            goto done;
        }
    }
    else
#endif
    {
        if (!DDS_DomainParticipantQos_copy_unmanaged_fields(&participant->qos, qos))
        {
            DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_QOS)
            goto done;
        }
    }

#if DDS_LIVELINESS_CHANNEL_ENABLED
    participant->qos.resource_limits.local_type_allocation += 1;
    participant->qos.resource_limits.local_topic_allocation += 1;
    participant->qos.resource_limits.local_writer_allocation += 1;
    participant->qos.resource_limits.local_reader_allocation += 1;
    participant->qos.resource_limits.matching_writer_reader_pair_allocation +=
             participant->qos.resource_limits.remote_participant_allocation * 2;
#endif

    /* Initialize memory pool to allocate buffer where to deserialize
     * incoming unbound data.
     */
    if (participant->qos.resource_limits.unbound_data_buffer_size < 0)
    {
        goto done;
    }

    /* NOTE:
     * Need to copy the RTPS parameters so that NETIO can use the info
     * to calculate RTPS ports to use.
     */
    participant->rtps_port_data.port_param = participant->qos.protocol.rtps_well_known_ports;
    participant->rtps_port_data.domain_id = participant->domain_id;


    participant->rtps_port_data.max_participant_id =
            DDS_RtpsWellKnownPorts_get_max_participant_index(
                            &participant->qos.protocol.rtps_well_known_ports);

    if (listener == NULL)
    {
        participant->listener = nil_listener;
    }
    else
    {
        participant->listener = *listener;
    }
    participant->mask = mask;

    {
        struct REDA_BufferPoolProperty pp_prop = REDA_BufferPoolProperty_INITIALIZER;

        /* It is sufficient number to allocate 1 packet because the Participant
         * is single threaded, only 1 datawriter or datareader can be active at
         * a time.
         */
        pp_prop.max_buffers = 1;
        pp_prop.buffer_size = (RTI_SIZE_T)sizeof(struct NETIO_Packet);
        participant->packet_pool = REDA_BufferPool_new("packet_pool",&pp_prop,
                                                       NULL,NULL,
                                                       NULL,NULL);

        if (participant->packet_pool == NULL)
        {
            DDSC_LOG_NETIO_PACKETPOOL_CREATE(OSAPI_LOGKIND_ERROR)
            goto done;
        }
    }

    /* Only one discovery plug-in is allowed, and it is okay with none
     */
    participant->disc_plugin = NULL;
    if (participant->qos.discovery.discovery.name._name._name[0] != 0)
    {
        c_factory = RT_Registry_lookup(participant->config.registry,
                                RT_ComponentFactoryId_get_name(
                                   &participant->qos.discovery.discovery.name));
        if (c_factory == NULL)
        {
            DDSC_LOG_COMPONENT_LOOKUP(OSAPI_LOGKIND_ERROR,
                                RT_ComponentFactoryId_get_name(
                                &participant->qos.discovery.discovery.name));
            goto done;
        }

        participant->disc_plugin = DiscoveryComponentFactory_create_component(
                      c_factory,&disc_property._parent,&disc_listener._parent);

        if (participant->disc_plugin == NULL)
        {
            DDSC_LOG_COMPONENT_CREATE(OSAPI_LOGKIND_ERROR,
                      DDSC_LOG_DISCOVERY_COMPONENT,
                          RT_ComponentFactoryId_get_name(
                                  &participant->qos.discovery.discovery.name));
            goto done;
        }
    }

    /* Initialize participant announcement data
     * The participant announcement relies on data collected from
     * the discovery plug-in(s)
     */
    rc = DDS_DomainParticipant_initialize_builtin_data(
                                    participant,&participant->builtin_data);
    if (rc != DDS_RETCODE_OK)
    {
        DDSC_LOG_OBJECT_INITIALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANTDATA_OBJECT);
        goto done;
    }

    /* Call the discovery plug-in before initialization, so the plug-in
     * can make any necessary changes to the plugin's QoS policies
     * (So the plug-in can allow for more resources, for example)
     * A discovery plug-in may alter the built-in topic-data, typically the
     * discovery locators are updated.
     */
    if (participant->disc_plugin != NULL)
    {
        if (!NDDS_Discovery_Plugin_on_before_local_participant_created(
                                                participant->disc_plugin,
                                                participant,
                                                &participant->qos,
                                                &participant->builtin_data))
        {
            DDSC_LOG_DISC_BEFORE_LOCAL_PARTICIPANT_CREATED(
                                OSAPI_LOGKIND_WARNING,
                                  RT_ComponentFactoryId_get_name(
                                   &participant->qos.discovery.discovery.name));
            goto done;
        }
    }

    /* Update QoS to take into account builtin Publisher and Subscriber,
     * used by all builtin endpoints (discovery, and inter-participant
     * channels). */
    participant->qos.resource_limits.local_publisher_allocation += 1;
    participant->qos.resource_limits.local_subscriber_allocation += 1;

    /* This calculation is based on detailed knowledge about the implementation:
     * 1 per remote participant (liveliness)
     * 4 per datawriter (deadline, liveliness, HB (RTPS), FRAG)
     * 3 per datareader (deadline, liveliness, ACKNACK (RTPS))
     * 1 for the discovery plugin
     */
    timer_property.max_entries =
            (participant->qos.resource_limits.remote_participant_allocation) +
            (4*participant->qos.resource_limits.local_writer_allocation) +
            (3*participant->qos.resource_limits.local_reader_allocation) +
            1;

    /* Make timer_property.max_entries an even number */
    timer_property.max_entries = (timer_property.max_entries + (timer_property.max_entries & 1));
    timer_property.max_slots = timer_property.max_entries / 2;

    participant->timer = OSAPI_Timer_new(&timer_property,participant->db_lock);
    if (participant->timer == NULL)
    {
        goto done;
    }


    /* Initialize the database */
    if (DDS_DomainParticipant_db_init(participant) != DDS_RETCODE_OK)
    {
        /* result code from from db_init() */
        goto done;
    }

    str_man_prop.database = participant->database;
    if (!DDS_StringManager_create(&participant->string_manager,
                                    &str_man_prop,
                                    DDS_MANAGEDSTRING_TABLE_NAME))
    {
        DDSC_LOG_OBJECT_CREATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_STRINGMANAGER_OBJECT)
        goto done;
    }

    if (participant->qos.resource_limits.max_partitions > 0)
    {
        partition_str_man_prop.database = participant->database;
        partition_str_man_prop.max_string_size =
            (RTI_INT32)participant->qos.resource_limits.max_partition_string_size;
        partition_str_man_prop.memory_allocation =
            (RTI_INT32)participant->qos.resource_limits.max_partition_string_allocation;

        if (!DDS_StringManager_create(&participant->partition_string_manager,
                                        &partition_str_man_prop,
                                        DDS_PARTITION_STRING_TABLE_NAME))
        {
            DDSC_LOG_OBJECT_CREATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_STRINGMANAGER_OBJECT)
            goto done;
        }
    }

    /* It is necessary to create the user_data_manager using qos instead of
     * participant->qos, because it contains the user specified resource limits
     * without any changes made by the discovery plugin.
     */
    if (!DDS_DomainParticipant_create_user_data_manager(participant, factory, qos))
    {
        goto done;
    }

    /* After database and managers are initialized, copy remaining fields of QoS */
#if INCLUDE_API_QOS
    if (qos == &DDS_PARTICIPANT_QOS_DEFAULT)
    {
        if (!DDS_DomainParticipantQos_copy_managed_fields(
                &participant->qos, &factory->default_participant_qos, participant))
        {
            DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_QOS)
            goto done;
        }
    }
    else
#endif
    {
        if (!DDS_DomainParticipantQos_copy_managed_fields(&participant->qos, qos, participant))
        {
            DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_QOS)
            goto done;
        }
    }

    /* Copy user data from QoS into builtin_data */
   if (!DDS_UserDataManager_assert_user_data(
            participant->user_data_manager,
            DDS_USER_DATA_PARTICIPANT_TYPE,
            &participant->qos.user_data.value,
            &participant->builtin_data.user_data.value))
    {
        goto done;
    }

    participant->network_lock = OSAPI_Mutex_new();
    if (participant->network_lock == NULL)
    {
        DDSC_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_MUTEX_OBJECT)
        goto done;
    }

    /* Add the default transports, unless the user has already specified some
     */
    if (DDS_StringSeq_get_length(&participant->qos.transports.enabled_transports) == 0)
    {
        RTI_INT32 max_length = 0;

        udp_factory = RT_Registry_lookup(participant->config.registry,
                                         NETIO_DEFAULT_UDP_NAME);
        if (udp_factory != NULL)
        {
            max_length = 2;
        }
        else
        {
            max_length = 1;
        }

        if (!DDS_StringSeq_set_maximum(&participant->qos.transports.enabled_transports,
                                       max_length))
        {
            DDSC_LOG_SEQ_SETMAX(OSAPI_LOGKIND_ERROR,
                                DDSC_LOG_ENABLED_TRANSPORT_SEQUENCE,max_length)
            goto done;
        }
        if (!DDS_StringSeq_set_length(&participant->qos.transports.enabled_transports,
                                      max_length))
        {
            DDSC_LOG_SEQ_SETLENGTH(OSAPI_LOGKIND_ERROR,
                                   DDSC_LOG_ENABLED_TRANSPORT_SEQUENCE,max_length)
            goto done;
        }

        if (udp_factory != NULL)
        {
            *DDS_StringSeq_get_reference(
                    &participant->qos.transports.enabled_transports,0) =
                                           DDS_String_dup(NETIO_DEFAULT_UDP_NAME);
            *DDS_StringSeq_get_reference(
                    &participant->qos.transports.enabled_transports,1) =
                                       DDS_String_dup(NETIO_DEFAULT_INTRA_NAME);
        }
        else
        {
            *DDS_StringSeq_get_reference(
                    &participant->qos.transports.enabled_transports,0) =
                                       DDS_String_dup(NETIO_DEFAULT_INTRA_NAME);
        }
    }

    /*
     * Resource tables
     */
    nar_property.default_base_port =
                    (RTI_UINT32)qos->protocol.rtps_well_known_ports.port_base;

    nar_property.max_interfaces = (RTI_UINT32)DDS_StringSeq_get_length(
                              &participant->qos.transports.enabled_transports);
    participant->address_resolver = NETIO_AddressResolver_new("address_r",
                                          participant->database,&nar_property);
    if (participant->address_resolver == NULL)
    {
        DDSC_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_ADDRESSRESOLVER_OBJECT)
        goto done;
    }

    rc_prop.max_routes = (RTI_SIZE_T)participant->qos.resource_limits.max_receive_ports +
                         (RTI_SIZE_T)participant->qos.resource_limits.local_writer_allocation;

    participant->bind_resolver = NETIO_BindResolver_new(participant->database,
                                                participant->address_resolver,
                                                "bind_r",&rc_prop);
    if (participant->bind_resolver == NULL)
    {
        DDSC_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_BINDRESOLVER_OBJECT)
        goto done;
    }

    rte_prop.max_routes = (RTI_SIZE_T)participant->qos.resource_limits.max_destination_ports;
    participant->route_resolver = NETIO_RouteResolver_new(participant->database,
                            participant->address_resolver,"route_r",&rte_prop);
    if (participant->route_resolver == NULL)
    {
        DDSC_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_ROUTERESOLVER_OBJECT)
        goto done;
    }

    if (!NETIO_AddressSeq_initialize(&participant->routes))
    {
        goto done;
    }

    /* Register each transport */
    if (!NETIO_AddressSeq_set_maximum(&participant->routes,(RTI_INT32)rte_prop.max_routes))
    {
        DDSC_LOG_SEQ_SETMAX(OSAPI_LOGKIND_ERROR,
                            DDSC_LOG_ROUTE_SEQUENCE,
                            rte_prop.max_routes)
        goto done;
    }

    if (!NETIO_NetmaskSeq_initialize(&participant->netmasks))
    {
        goto done;
    }

    if (!NETIO_NetmaskSeq_set_maximum(&participant->netmasks,(RTI_INT32)rte_prop.max_routes))
    {
        DDSC_LOG_SEQ_SETMAX(OSAPI_LOGKIND_ERROR,
                            DDSC_LOG_NETMASK_SEQUENCE,rte_prop.max_routes)
        goto done;
    }

    instance_handle = DDS_Entity_get_instance_handle(DDS_DomainParticipant_as_entity(participant));
    NETIO_Address_set_guid(&src_address,0,(struct NETIO_Guid*)&instance_handle.octet);

    for (i = 0; i < DDS_StringSeq_get_length(&participant->qos.transports.enabled_transports); ++i)
    {
        RT_ComponentFactoryId_T id;

        if (!RT_ComponentFactoryId_set_name(&id,*DDS_StringSeq_get_reference(
                           &participant->qos.transports.enabled_transports,i)))
        {
            goto done;
        }

        c_factory = RT_Registry_lookup(participant->config.registry,
                *DDS_StringSeq_get_reference(
                           &participant->qos.transports.enabled_transports,i));

        if (c_factory == NULL)
        {
            DDSC_LOG_COMPONENT_LOOKUP(OSAPI_LOGKIND_ERROR,
                        *DDS_StringSeq_get_reference(
                           &participant->qos.transports.enabled_transports,i))
            goto done;
        }

        if (RT_INTERFACE_INSTANCE(RT_ComponentFactory_get_id(c_factory)) == RT_COMPONENT_INSTANCE_INTRA)
        {
            lo_property._parent._parent.db = participant->database;
            lo_property._parent.max_routes = (RTI_SIZE_T)participant->qos.resource_limits.local_reader_allocation;
            lo_property._parent.max_binds = (RTI_SIZE_T)participant->qos.resource_limits.local_reader_allocation;
            netio_intf = NETIO_InterfaceFactory_create_component(c_factory,&lo_property._parent._parent,NULL);
        }
        else if (RT_INTERFACE_INSTANCE(RT_ComponentFactory_get_id(c_factory)) == RT_COMPONENT_INSTANCE_NOTIF)
        {
            notif_property._parent._parent.db = participant->database;
            notif_property._parent.max_routes = (RTI_SIZE_T)participant->qos.resource_limits.matching_writer_reader_pair_allocation;
            notif_property._parent.max_binds = (RTI_SIZE_T)participant->qos.resource_limits.matching_writer_reader_pair_allocation;

            notif_property.domain_id = domain_id;
            notif_property.num_remote_writers = participant->qos.resource_limits.remote_writer_allocation;
            notif_property.max_receive_ports = 4;
            notif_property.lock = participant->db_lock;
            netio_intf = NETIO_InterfaceFactory_create_component(c_factory,
                    &notif_property._parent._parent,NULL);
        }
        else
        {
            intf_property._parent.db = participant->database;
            intf_property.max_binds = (RTI_SIZE_T)participant->qos.resource_limits.max_receive_ports;
            intf_property.max_routes = (RTI_SIZE_T)participant->qos.resource_limits.max_destination_ports;
            intf_property.network_lock = participant->network_lock;
            NETIO_Address_set_guid(
                    &intf_property.intf_addr,
                    0,
                    (struct NETIO_Guid*) &instance_handle.octet);
            netio_intf = NETIO_InterfaceFactory_create_component(c_factory,&intf_property._parent,NULL);
        }

        if (netio_intf == NULL)
        {
            DDSC_LOG_COMPONENT_CREATE(OSAPI_LOGKIND_ERROR,
                    DDSC_LOG_NETIO_NETIO_KIND,
                         *DDS_StringSeq_get_reference(
                            &participant->qos.transports.enabled_transports,i))
            goto done;
        }

        /* Add the interface to the address resolution table */
        if (!NETIO_AddressResolver_add_interface(participant->address_resolver,
                                    RT_ComponentFactoryId_get_name(&id),
                                    NETIO_rtps_calculate_port,
                                    &participant->rtps_port_data,netio_intf))
        {
            /* Rely on error code from add_route() */
            goto done;
        }

        if (!NETIO_AddressSeq_set_length(&participant->routes,0))
        {
            DDSC_LOG_SEQ_SETLENGTH(OSAPI_LOGKIND_ERROR,DDSC_LOG_ROUTE_SEQUENCE,0)
            goto done;
        }

        if (!NETIO_NetmaskSeq_set_length(&participant->netmasks,0))
        {
            DDSC_LOG_SEQ_SETLENGTH(OSAPI_LOGKIND_ERROR,DDSC_LOG_NETMASK_SEQUENCE,0)
            goto done;
        }

        if (!NETIO_Interface_get_route_table(netio_intf,
                            &participant->routes,&participant->netmasks))
        {
            DDSC_LOG_NETIO_GET_ROUTE_TABLE_FAILED(OSAPI_LOGKIND_ERROR,
                                                  id._name._name)
            goto done;
        }

        for (j = 0; j < NETIO_AddressSeq_get_length(&participant->routes); ++j)
        {
            struct NETIO_Address *routes =
                    NETIO_AddressSeq_get_reference(&participant->routes,j);
            struct NETIO_Netmask *netmasks =
                    NETIO_NetmaskSeq_get_reference(&participant->netmasks,j);

            if ((routes == NULL) || (netmasks == NULL))
            {
                goto done;
            }

            if (!NETIO_RouteResolver_add_interface(participant->route_resolver,
                    RT_ComponentFactoryId_get_name(&id),
                    netio_intf,
                    routes,
                    netmasks,
                    NULL))
            {
                goto done;
            }
        }
    }

    /* this sequences are used later for finalize, here we just set the length to zero
     * to indicate that they have no use outside of the scope of init and finalize functions
     */
    if (!NETIO_AddressSeq_set_length(&participant->routes,0))
    {
        DDSC_LOG_SEQ_SETLENGTH(OSAPI_LOGKIND_ERROR,DDSC_LOG_ROUTE_SEQUENCE,0)
        goto done;
    }

    if (!NETIO_NetmaskSeq_set_length(&participant->netmasks,0))
    {
        DDSC_LOG_SEQ_SETLENGTH(OSAPI_LOGKIND_ERROR,DDSC_LOG_NETMASK_SEQUENCE,0)
        goto done;
    }

    if (udp_factory != NULL)
    {
        /* Add the default discovery transports unless the user has specified a list */
        if (DDS_StringSeq_get_maximum(&qos->discovery.enabled_transports) == 0)
        {
            if (!DDS_StringSeq_set_maximum(&participant->qos.discovery.enabled_transports,3))
            {
                DDSC_LOG_SEQ_SETMAX(OSAPI_LOGKIND_ERROR,
                                  DDSC_LOG_ENABLED_DISCVOERY_TRANSPORT_SEQUENCE,3)
                goto done;
            }

            if (!DDS_StringSeq_set_length(&participant->qos.discovery.enabled_transports,3))
            {
                DDSC_LOG_SEQ_SETLENGTH(OSAPI_LOGKIND_ERROR,
                                 DDSC_LOG_ENABLED_DISCVOERY_TRANSPORT_SEQUENCE,3)
                goto done;
            }

            *DDS_StringSeq_get_reference(
                 &participant->qos.discovery.enabled_transports,0) =
                                             DDS_String_dup("_udp://239.255.0.1");
            *DDS_StringSeq_get_reference(
                    &participant->qos.discovery.enabled_transports,1) =
                                             DDS_String_dup("_udp://");
            *DDS_StringSeq_get_reference(
                 &participant->qos.discovery.enabled_transports,2) =
                                             DDS_String_dup("_udp://127.0.0.1");
        }
    }

    if (!DDS_DomainParticipant_verify_discovery_transports(participant, qos))
    {
        goto done;
    }

    /* Add the default user-traffic transports unless the user has specified a list */
    if (DDS_StringSeq_get_maximum(&qos->user_traffic.enabled_transports) == 0)
    {
        RTI_INT32 max_length = 0;

        if (udp_factory != NULL)
        {
            max_length = 2;
        }
        else
        {
            max_length = 1;
        }

        if (!DDS_StringSeq_set_maximum(&participant->qos.user_traffic.enabled_transports,
                                       max_length))
        {
            DDSC_LOG_SEQ_SETMAX(OSAPI_LOGKIND_ERROR,
                                DDSC_LOG_ENABLED_USER_TRANSPORT_SEQUENCE,
                                max_length)
            goto done;
        }

        if (!DDS_StringSeq_set_length(&participant->qos.user_traffic.enabled_transports,
                                      max_length))
        {
            DDSC_LOG_SEQ_SETLENGTH(OSAPI_LOGKIND_ERROR,
                                   DDSC_LOG_ENABLED_USER_TRANSPORT_SEQUENCE,
                                   max_length)
            goto done;
        }

        if (udp_factory != NULL)
        {
            *DDS_StringSeq_get_reference(
                        &participant->qos.user_traffic.enabled_transports,0) =
                                DDS_String_dup("_udp://");
            *DDS_StringSeq_get_reference(
                        &participant->qos.user_traffic.enabled_transports,1) =
                                DDS_String_dup("_udp://127.0.0.1");
        }
        else
        {
            *DDS_StringSeq_get_reference(
                        &participant->qos.user_traffic.enabled_transports,0) =
                                DDS_String_dup("_intra://");
        }
    }

    if (participant->qos.protocol.participant_id >= 0)
    {
        participant->participant_id = participant->qos.protocol.participant_id;
        try_id = participant->participant_id;
        max_id = try_id;
    }
    else
    {
        max_id = DDS_RtpsWellKnownPorts_get_max_participant_index(
                            &participant->qos.protocol.rtps_well_known_ports);
        try_id = 0;
    }

    for (i = try_id; (i <= max_id); ++i)
    {
        if (!DDS_LocatorSeq_set_length(&participant->builtin_data.metatraffic_multicast_locators,0))
        {
            DDSC_LOG_SEQ_SETLENGTH(OSAPI_LOGKIND_ERROR,DDSC_LOG_METAMULTICAST_SEQUENCE,0)
            goto done;
        }

        if (!DDS_LocatorSeq_set_length(&participant->builtin_data.metatraffic_unicast_locators,0))
        {
            DDSC_LOG_SEQ_SETLENGTH(OSAPI_LOGKIND_ERROR,DDSC_LOG_METAUNICAST_SEQUENCE,0)
            goto done;
        }

        if (!DDS_LocatorSeq_set_length(&participant->builtin_data.default_multicast_locators,0))
        {
            DDSC_LOG_SEQ_SETLENGTH(OSAPI_LOGKIND_ERROR,DDSC_LOG_USERMULTICAST_SEQUENCE,0)
            goto done;
        }

        if (!DDS_LocatorSeq_set_length(&participant->builtin_data.default_unicast_locators,0))
        {
            DDSC_LOG_SEQ_SETLENGTH(OSAPI_LOGKIND_ERROR,DDSC_LOG_USERUNICAST_SEQUENCE,0)
            goto done;
        }

        if (!NETIO_BindResolver_reserve_addresses(
             participant->bind_resolver,i,
             &participant->qos.transports.enabled_transports,
             (struct REDA_StringSeq*)&participant->qos.discovery.enabled_transports,
             NETIO_ROUTEKIND_META,
             (struct NETIO_AddressSeq*)&participant->builtin_data.metatraffic_multicast_locators,
             (struct NETIO_AddressSeq*)&participant->builtin_data.metatraffic_unicast_locators))
        {
            continue;
        }

        if (!NETIO_BindResolver_reserve_addresses(
             participant->bind_resolver,i,
             &participant->qos.transports.enabled_transports,
             (struct REDA_StringSeq*)&participant->qos.user_traffic.enabled_transports,NETIO_ROUTEKIND_USER,
             (struct NETIO_AddressSeq*)&participant->builtin_data.default_multicast_locators,
             (struct NETIO_AddressSeq*)&participant->builtin_data.default_unicast_locators))
        {
            if (!NETIO_BindResolver_release_addresses(
                    participant->bind_resolver,
                    (struct REDA_StringSeq*)&participant->qos.transports.enabled_transports,
                    NETIO_ROUTEKIND_META,
                    (struct NETIO_AddressSeq*)&participant->builtin_data.metatraffic_multicast_locators))
            {
                DDSC_LOG_RELEASE_META_MC(OSAPI_LOGKIND_ERROR)
                goto done;
            }

            if (!NETIO_BindResolver_release_addresses(
                    participant->bind_resolver,
                    (struct REDA_StringSeq*)&participant->qos.transports.enabled_transports,
                    NETIO_ROUTEKIND_META,
                    (struct NETIO_AddressSeq*)&participant->builtin_data.metatraffic_unicast_locators))
            {
                DDSC_LOG_RELEASE_META_UC(OSAPI_LOGKIND_ERROR)
                goto done;
            }

            continue;
        }

        /* This means that reservations succeeded */
        break;
    }

    if (i > max_id)
    {
        DDSC_LOG_MAX_PARTICIPANT_ID_REACHED(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    c_factory = RT_Registry_lookup(participant->config.registry,NETIO_DEFAULT_RTPS_NAME);
    if (c_factory == NULL)
    {
        DDSC_LOG_COMPONENT_LOOKUP(OSAPI_LOGKIND_ERROR,NETIO_DEFAULT_RTPS_NAME)
        goto done;
    }

    if (!DDS_DomainParticipant_checksum_configure(participant,c_factory,&rtps_property))
    {
        DDSC_LOG_COMPONENT_LOOKUP(OSAPI_LOGKIND_ERROR,NETIO_DEFAULT_RTPS_NAME)
        goto done;
    }

    instance_handle = DDS_Entity_get_instance_handle(DDS_DomainParticipant_as_entity(participant));
    NETIO_Address_set_guid(&src_address,
                           (RTI_UINT32)participant->domain_id,
                           (struct NETIO_Guid*)&instance_handle.octet);

    rtps_property.intf_address =  src_address;
    rtps_property._parent._parent.db = participant->database;

    /* This resource limit is needed because there is one entry for each match
     * and a local endpoint may match with more than 1 remote endpoint. A
     * decent automatic value is (local_writer_allocation + local_reader_allocation)
     * * remote_participant_allocation (fully meshed, 1 DR/1 DW of each topic in
     * each participant).
     */
    rtps_property._parent.max_binds =
            (RTI_SIZE_T)participant->qos.resource_limits.matching_writer_reader_pair_allocation;

    rtps_property.mode = RTPS_INTERFACEMODE_EXTERNAL_RECEIVER;
    min_mtu = NETIO_RouteResolver_get_minimum_mtu(participant->route_resolver);

    if (min_mtu < 0)
    {
        DDSC_LOG_GET_MTU(OSAPI_LOGKIND_ERROR)
        goto done;
    }
    if (min_mtu == 0)
    {
        DDSC_LOG_GET_MTU_NO_ROUTES(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    if (min_mtu <= (RTI_INT32)RTPS_PROTOCOL_OVERHEAD)
    {
        DDSC_LOG_MIN_MTU_SIZE(OSAPI_LOGKIND_ERROR, min_mtu)
        goto done;
    }

    if (min_mtu > NETIO_IP_PACKET_MAX_SIZE)
    {
        rtps_property.fragment_size_bytes = NETIO_IP_PACKET_MAX_SIZE;
    }
    else
    {
        rtps_property.fragment_size_bytes = (RTI_UINT16)min_mtu;
    }

    rtps_property._parent.network_lock = participant->network_lock;


    /* Add trust-related configuration to the RTPS property, if needed */
    if (!DDS_DomainParticipant_initialize_rtps_trust_property(participant,
                                                    &rtps_property,c_factory))
    {
        goto done;
    }

    /* Each participant creates an external interface, but the first participant
     * also initializes the RTPS factory. max_routes is not used by the external
     * interface and is instead used to initialize the RTPS factory max
     * external interfaces index.
     */
    rtps_property._parent.max_routes = (RTI_UINT32)factory->qos.resource_limits.max_participants;

    participant->rtps_intf = NETIO_InterfaceFactory_create_component(c_factory,
                                    &rtps_property._parent._parent,NULL);

    if (participant->rtps_intf == NULL)
    {
        DDSC_LOG_COMPONENT_CREATE(OSAPI_LOGKIND_ERROR,
                                  DDSC_LOG_RTPS_COMPONENT,NETIO_DEFAULT_RTPS_NAME)
        goto done;
    }

    participant->participant_id = i;

#ifndef RTI_CERT
    /* Create a default participant name if none is specified. The default name
     * is in the format:
     *
     * DDS_DEFAULT_PARTICIPANT_NAME<participant_id>
     */
    if (participant->qos.participant_name.name[0] == 0)
    {
        /* The participant name is 255 bytes (excluding NUL) */
        OSAPI_Memory_copy(participant->builtin_data.participant_name.name,
                          DDS_DEFAULT_PARTICIPANT_NAME,
                          def_dp_name_len+1);

        if (OSAPI_Log_itoa(
                participant->builtin_data.participant_name.name + def_dp_name_len,
                DDS_ENTITYNAME_QOS_NAME_MAX - def_dp_name_len,
                participant->participant_id) >=
                DDS_ENTITYNAME_QOS_NAME_MAX - def_dp_name_len)
        {
            goto done;
        }
    }
#endif

#if !DDS_DISABLE_PARTICIPANT_INFO
    /* Add the hostname property */
    if (!OSAPI_System_get_hostname(participant->hostname_property))
    {
        DDSC_LOG_SYS_GET_HOSTNAME(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    /* We will set a builtin_data.property 4 times. If we don't set the
     * maximum, the property will be reallocated each time we set it.
     * This is a problem for CERT since we cannot reallocate in CERT.
     */
    if (!DDS_PropertySeq_set_maximum(&participant->builtin_data.property.value,
                                     4))
    {
        goto done;
    }

    if (!DDS_PropertySeq_assert_property(
            &participant->builtin_data.property.value,
            DDS_PROPERTY_HOSTNAME_NAME,
            participant->hostname_property,
            CDR_BOOLEAN_TRUE))
    {
        goto done;
    }

    if (OSAPI_Process_pid_as_string(
            participant->process_property,
            OSAPI_SYSTEM_MAX_HOSTNAME,
            OSAPI_Process_getpid()) >= OSAPI_SYSTEM_MAX_HOSTNAME)
    {
        DDSC_LOG_IO_SNPRINTF_FAILED(OSAPI_LOGKIND_ERROR)
        goto done;
    }
    if (!DDS_PropertySeq_assert_property(
            &participant->builtin_data.property.value,
            DDS_PROPERTY_PROCESSID_NAME,
            participant->process_property,
            CDR_BOOLEAN_TRUE))
    {
        goto done;
    }

    /* Ensure that RTIME_TARGET_NAME is bounded */
    {
        char rime_target_name_truncated[OSAPI_SYSTEM_MAX_HOSTNAME + 1];
        RTI_SIZE_T len = OSAPI_String_length(RTIME_TARGET_NAME);

        if (len > OSAPI_SYSTEM_MAX_HOSTNAME)
        {
            len = OSAPI_SYSTEM_MAX_HOSTNAME;
        }
        OSAPI_Memory_copy(rime_target_name_truncated,
                          RTIME_TARGET_NAME,
                          len);
        rime_target_name_truncated[len] = 0;

        /* Target name */
        if (!DDS_PropertySeq_assert_property(
                &participant->builtin_data.property.value,
                DDS_PROPERTY_TARGET_NAME,
                rime_target_name_truncated,
                CDR_BOOLEAN_TRUE))
        {
            goto done;
        }
    }

    /* Version info: */

    v_idx = 0;
    participant->version_property[v_idx++] = '0' + RTIME_DDS_VERSION_MAJOR;
    participant->version_property[v_idx++] = '.';
    participant->version_property[v_idx++] = '0' + RTIME_DDS_VERSION_MINOR;
    participant->version_property[v_idx++] = '.';

    if (RTIME_DDS_VERSION_REVISION <= 9)
    {
        participant->version_property[v_idx++] = '0' + RTIME_DDS_VERSION_REVISION;
    }
    else
    {
        participant->version_property[v_idx++] = '0' + RTIME_DDS_VERSION_REVISION / 10;
        participant->version_property[v_idx++] = '0' + RTIME_DDS_VERSION_REVISION % 10;
    }

    participant->version_property[v_idx++] = '.';

    if (RTIME_DDS_VERSION_RELEASE <= 9)
    {
        participant->version_property[v_idx++] = '0' + RTIME_DDS_VERSION_RELEASE;
    }
    else
    {
        participant->version_property[v_idx++] = '0' + RTIME_DDS_VERSION_RELEASE / 10;
        participant->version_property[v_idx++] = '0' + RTIME_DDS_VERSION_RELEASE % 10;
    }

    participant->version_property[v_idx] = 0;

    if (!DDS_PropertySeq_assert_property(
            &participant->builtin_data.property.value,
            DDS_PROPERTY_VERSION_NAME,
            participant->version_property,
            CDR_BOOLEAN_TRUE))
    {
        goto done;
    }
#endif

#if DDS_FILTERING_ENABLED
    /* Filtering must be initialized before any entities are created by the participant */
    if (!DDS_DomainParticipant_create_filter_plugin(participant))
    {
        DDSC_LOG_CREATE_FILTER_PLUGIN(OSAPI_LOGKIND_ERROR)
        goto done;
    }
#endif

    /* Create builtin Publisher and Subscriber */
    {
        struct DDS_PublisherQos builtin_pub_qos = DDS_PublisherQos_INITIALIZER;
        struct DDS_SubscriberQos builtin_sub_qos = DDS_SubscriberQos_INITIALIZER;

        builtin_pub_qos.entity_factory.autoenable_created_entities =
                DDS_BOOLEAN_FALSE;
        builtin_pub_qos.management.is_hidden = DDS_BOOLEAN_TRUE;

        participant->builtin_publisher =
                DDS_DomainParticipant_create_publisher(
                        participant,
                        &builtin_pub_qos,
                        NULL,
                        DDS_STATUS_MASK_ALL);
        if (participant->builtin_publisher == NULL)
        {
            DDSC_LOG_OBJECT_INITIALIZE(OSAPI_LOGKIND_ERROR,
                                       DDSC_LOG_PARTICIPANT_BUILTIN_PUBLISHER)
            goto done;
        }

        builtin_sub_qos.entity_factory.autoenable_created_entities =
                DDS_BOOLEAN_FALSE;
        builtin_sub_qos.management.is_hidden = DDS_BOOLEAN_TRUE;

        participant->builtin_subscriber =
                DDS_DomainParticipant_create_subscriber(
                        participant,
                        &builtin_sub_qos,
                        NULL,
                        DDS_STATUS_MASK_ALL);
        if (participant->builtin_subscriber == NULL)
        {
            DDSC_LOG_OBJECT_INITIALIZE(OSAPI_LOGKIND_ERROR,
                                       DDSC_LOG_PARTICIPANT_BUILTIN_SUBSCRIBER)
            goto done;
        }
    }

#if DDS_LIVELINESS_CHANNEL_ENABLED
    if (!DDS_IpcLiveliness_reserve(participant))
    {
        goto done;
    }
#endif
#if DDS_FLOW_CONTROLLER_ENABLED
    DDS_FlowControl_initialize(&participant->flow_control,participant);
#endif

    if (participant->disc_plugin != NULL)
    {

        if (!NDDS_Discovery_Plugin_on_after_local_participant_created(
                        participant->disc_plugin,
                        participant,
                        &participant->builtin_data))
        {
            DDSC_LOG_DISC_AFTER_LOCAL_PARTICIPANT_CREATED(
                                    OSAPI_LOGKIND_ERROR,
                                    RT_ComponentFactoryId_get_name(
                                   &participant->qos.discovery.discovery.name));
            goto done;
        }
    }

    if (!DDS_DomainParticipant_initialize_discovery_queue(participant,qos))
    {
        DDSC_LOG_INITIALIZE_DISCOVERY_QUEUE(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    j = DDS_StringSeq_get_length(&participant->qos.discovery.initial_peers);
    for (i = 0; i < j; ++i)
    {
        if (DDS_DomainParticipant_add_peer(participant,
                *DDS_StringSeq_get_reference(
                                &participant->qos.discovery.initial_peers,i))
                                != DDS_RETCODE_OK)
        {
            DDSC_LOG_DISC_ADD_PEER(OSAPI_LOGKIND_ERROR,
                    RT_ComponentFactoryId_get_name(
                               &participant->qos.discovery.discovery.name),
                       *DDS_StringSeq_get_reference(
                                  &participant->qos.discovery.initial_peers,i))
            goto done;
        }
    }

    participant->as_entity.state = RTIDDS_ENTITY_STATE_CREATED;

    success = DDS_BOOLEAN_TRUE;

done:

    if (!success && (participant != NULL))
    {
#ifndef RTI_CERT
        rc = DDS_DomainParticipant_finalize(participant);
        IGNORE_RETVAL(rc);
#endif
        participant = NULL;
    }

    return participant;
}

DDS_ReturnCode_t
DDS_DomainParticipant_register_type(DDS_DomainParticipant *participant,
                                    const char *type_name,
                                    struct DDS_TypePluginI *plugin)
{
    struct DDS_DomainParticipantImpl *self =
                            (struct DDS_DomainParticipantImpl *)participant;
    struct DDS_TypeImpl *a_type = NULL;
    DB_ReturnCode_T dbrc;
    DDS_ReturnCode_t rc = DDS_RETCODE_ERROR;
    char *managed_type = NULL;
    RTI_BOOL rtn = RTI_FALSE;

    OSAPI_PRECONDITION_ALWAYS((participant == NULL) || (type_name == NULL) ||
                               (plugin == NULL),
               return DDS_RETCODE_BAD_PARAMETER,
               OSAPI_Log_entry_add_pointer("participant",participant,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("type_name",type_name,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("plugin",plugin,RTI_TRUE);)

    OSAPI_PRECONDITION_ALWAYS(DDS_String_length(type_name) > RTPS_PATHNAME_LEN_MAX,
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_uint("length",
                                   DDS_String_length(type_name),RTI_TRUE);)

    /*
     * The IPC registers during writer creation but does not require a 
     * call to on_type_registered. The factory lock protects the type plugin
     * values accessed through on_type_registered.
     * 
     * This prevents a lock-order inversion with the database lock that
     * would occur if registration were to maintain the database lock during
     * the call to on_type_registered for DDS_IpcLiveliness_create.
     */
#if DDS_LIVELINESS_CHANNEL_ENABLED
    if (REDA_String_compare(type_name, DDS_PARTICIPANT_MESSAGE_DATA_TYPE_NAME) != 0)
#endif
    {
        /* on_type_registered accesses a shared singleton. Lock access here. */
        if (!OSAPI_Mutex_take(self->shared_lock))
        {
            return DDS_RETCODE_ERROR;
        }
    }

    
    if (DB_Database_lock(self->database) != DB_RETCODE_OK)
    {
        /* We are already returning an error so there is no need to check the
         * return value.
         */
        rtn = OSAPI_Mutex_give(self->shared_lock);
        UNUSED_ARG(rtn);

        return DDS_RETCODE_ERROR;
    }

    dbrc = DB_Table_select_match(self->type_table,DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&a_type,(DB_Key_T)type_name);

    if (dbrc == DB_RETCODE_OK)
    {
        /* All type-plugins are static interfaces, thus allow multiple
         * registrations as long as the plugin pointer is the same
         */
        if (a_type->plugin == plugin)
        {
            /* Increase the number of times the type has been registered. A
             * type must be unregistered the same number of times, except as
             * part of delete_contatined_entities() in which case it is
             * deleted regardless of the number of times it was registered.
             */
            DDS_TypeImpl_reference(a_type);
            rc = DDS_RETCODE_OK;
        }
        else
        {
            DDSC_LOG_RECORD_EXISTS(OSAPI_LOGKIND_ERROR,DDSC_LOG_TYPE_RECORD)
            rc = DDS_RETCODE_PRECONDITION_NOT_MET;
        }
        goto done;
    }

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        rc = DDS_RETCODE_ERROR;
        goto done;
    }

    managed_type = DDS_StringManager_assert_string(participant->string_manager,type_name);
    if (managed_type == NULL)
    {
        DDSC_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR, DDSC_LOG_STRING_OBJECT)
        goto done;
    }

    rc = DDS_RETCODE_ERROR;
    dbrc = DB_Table_create_record(self->type_table,(DB_Record_T*)&a_type);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_CREATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_TYPE_RECORD,dbrc)
        if (dbrc == DB_RETCODE_OUT_OF_RESOURCES)
        {
            rc = DDS_RETCODE_OUT_OF_RESOURCES;
        }
        (void)DDS_StringManager_delete_string(participant->string_manager,managed_type);
        goto done;
    }

    if (!DDS_TypeImpl_initalize(self,a_type,managed_type,plugin))
    {
        DDSC_LOG_RECORD_INITIALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_TYPE_RECORD)
        (void)DB_Table_delete_record(self->type_table,a_type);
        (void)DDS_StringManager_delete_string(participant->string_manager, managed_type);
        goto done;
    }

    if (plugin->on_type_registered != NULL)
    {
        if (!plugin->on_type_registered(a_type))
        {
            DDSC_LOG_ON_TYPE_REGISTERED_FAILURE(OSAPI_LOGKIND_ERROR);
            goto done;
        }
    }

    dbrc = DB_Table_insert_record(self->type_table,(DB_Record_T)a_type);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_INSERT(OSAPI_LOGKIND_ERROR,DDSC_LOG_TYPE_RECORD,dbrc)
        /* Besides OK, insert_record returns EXISTS */
        (void)DB_Table_delete_record(self->type_table,a_type);
        (void)DDS_StringManager_delete_string(participant->string_manager, managed_type);
        goto done;
    }

    rc = DDS_RETCODE_OK;

done:

    if (DB_Database_unlock(self->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

#if DDS_LIVELINESS_CHANNEL_ENABLED
    if (REDA_String_compare(type_name, DDS_PARTICIPANT_MESSAGE_DATA_TYPE_NAME) != 0)
#endif
    {
        if (!OSAPI_Mutex_give(self->shared_lock))
        {
            return DDS_RETCODE_ERROR;
        }
    }
    
    return rc;
}

#ifndef RTI_CERT
/*ci
 * \brief Unregister a previously found type record
 *
 * \details
 *
 * This function unregisters a previously found type-record. If there are no
 * references and no topics attached, all resources are released. If the
 * type is referenced it is dereferenced and the type plugin is returned. A
 * referenced type record cannot be released. It is an error to delete a
 * non-referenced, but attached type record.
 *
 * \param[in] participant The participant the type record belongs to
 * \param[in] type        The type record to unregister
 *
 * \return The type-plugin interface on success, NULL on failure.
 */
RTI_PRIVATE struct DDS_TypePluginI*
DDS_DomainParticipant_unregister_type_record(DDS_DomainParticipant *participant,
                                             struct DDS_TypeImpl *type)
{
    struct DDS_DomainParticipantImpl *self =
                            (struct DDS_DomainParticipantImpl *)participant;
    DB_ReturnCode_T dbrc;
    struct DDS_TypePluginI *type_plugin = NULL;
    struct DDS_TypeImpl *typerec;
    char *managed_type = NULL;

    if (!DDS_TypeImpl_is_referenced(type))
    {
        return NULL;
    }

    DDS_TypeImpl_dereference(type);
    if (DDS_TypeImpl_is_referenced(type))
    {
        /* multiple registrations, this is allowed */
        type_plugin = type->plugin;
        goto done;
    }

    if (DDS_TypeImpl_is_attached(type))
    {
        /* This is not allowed, increment the reference to one again so it can
         * be unregistered again when no topics are attached.
         */
        DDS_TypeImpl_reference(type);
        DDSC_LOG_OBJECT_REFCOUNT(OSAPI_LOGKIND_ERROR,DDSC_LOG_TYPE_OBJECT)
        goto done;
    }

    typerec = NULL;
    dbrc = DB_Table_remove_record(self->type_table,
                                 (DB_Record_T*)&typerec,(DB_Key_T)type->name);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_REMOVE(OSAPI_LOGKIND_ERROR,DDSC_LOG_TYPE_RECORD,dbrc)
        goto done;
    }

    type_plugin = typerec->plugin;

    if (type_plugin->on_type_unregistered != NULL)
    {
        if (!type_plugin->on_type_unregistered(typerec))
        {
            DDSC_LOG_ON_TYPE_UNREGISTERED_FAILURE(OSAPI_LOGKIND_ERROR);
            goto done;
        }
    }

    managed_type = typerec->name;

    dbrc = DB_Table_delete_record_w_dtor(self->type_table,
                                         (DB_Record_T)typerec,DDS_Type_dtor);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_DELETE(OSAPI_LOGKIND_ERROR,DDSC_LOG_TYPE_RECORD,dbrc)
        type_plugin = NULL;
    }

    if (!DDS_StringManager_delete_string(participant->string_manager,managed_type))
    {
        type_plugin = NULL;
        goto done;
    }



done:
    return type_plugin;
}

struct DDS_TypePluginI*
DDS_DomainParticipant_unregister_type(DDS_DomainParticipant * participant,
                                      const char *type_name)
{
    struct DDS_DomainParticipantImpl *self =
                            (struct DDS_DomainParticipantImpl *)participant;
    struct DDS_TypeImpl *a_type = NULL;
    DB_ReturnCode_T dbrc;
    struct DDS_TypePluginI *type_plugin = NULL;

    if (DB_Database_lock(self->database) != DB_RETCODE_OK)
    {
        return NULL;
    }

    dbrc = DB_Table_select_match(self->type_table,DB_TABLE_DEFAULT_INDEX,
                                (DB_Record_T*)&a_type,(DB_Key_T)type_name);

    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_SELECT(OSAPI_LOGKIND_ERROR,DDSC_LOG_TYPE_RECORD,dbrc);
        goto done;
    }

    type_plugin = DDS_DomainParticipant_unregister_type_record(participant,a_type);

done:

    if (DB_Database_unlock(self->database) != DB_RETCODE_OK)
    {
        return NULL;
    }

    return type_plugin;
}
#endif /* !RTI_CERT */

#ifndef RTI_CERT
/*ci
 * \brief Delete all contained entities
 *
 * \details
 *
 * This internal function deletes all contained entities and also calls the
 * specified finalizer functions. The finalize functions typically cleans
 * up resources used by wrapper classes around the core DDS C entities, such
 * as the C++ API.
 *
 * \param[in] self      The participant to delete contained entities in
 * \param[in] finalizer The finalizer to call for each contained entity
 *
 * \return DDS_RETCODE_OK on success, one of the error return codes on failure
 */
DDS_ReturnCode_t
DDS_DomainParticipant_delete_contained_entities_w_finalizerI(
        DDS_DomainParticipant *self,
        struct DDS_DomainParticipant_EntityFinalizer *finalizer)
{
    struct DDS_DomainParticipantImpl *participant =
            (struct DDS_DomainParticipantImpl *)self;
    DB_ReturnCode_T dbrc;
    DDS_ReturnCode_t ddsrc = DDS_RETCODE_OK;
    DB_Cursor_T cursor;
    DDS_Topic *topic;
    DDS_Publisher *publisher;
    DDS_Subscriber *subscriber;
    DDS_Type *type;

    OSAPI_PRECONDITION(self == NULL,
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (DB_Database_lock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    dbrc = DB_Table_select_all_default(participant->local_publisher_table,&cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,participant->local_publisher_table,dbrc)
        goto done;
    }

    do
    {
        dbrc = DB_Cursor_get_next(cursor, (DB_Record_T*)&publisher);
        if (dbrc == DB_RETCODE_OK)
        {
            ddsrc = DDS_Publisher_delete_contained_entities_no_lock(publisher);
            if ((ddsrc == DDS_RETCODE_OK) &&
                    !DDS_PublisherImpl_is_hidden(publisher))
            {
                if ((finalizer != NULL) &&
                    (finalizer->finalize_publisher != NULL))
                {
                    finalizer->finalize_publisher(publisher);
                }
                ddsrc = DDS_DomainParticipant_delete_publisher(participant,
                                                               publisher);
            }
        }
    } while ((dbrc == DB_RETCODE_OK) && (ddsrc == DDS_RETCODE_OK));
    DB_Cursor_finish(participant->local_publisher_table,cursor);

    if ((dbrc != DB_RETCODE_NO_DATA) || (ddsrc != DDS_RETCODE_OK))
    {
        /* Rely on underlying error-code */
        goto done;
    }

    /* Delete subscriber */
    cursor = NULL;
    dbrc = DB_Table_select_all_default(participant->local_subscriber_table,&cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,participant->local_subscriber_table,dbrc)
        goto done;
    }

    do
    {
        dbrc = DB_Cursor_get_next(cursor, (DB_Record_T*)&subscriber);
        if (dbrc == DB_RETCODE_OK)
        {
            ddsrc = DDS_Subscriber_delete_contained_entities_no_lock(subscriber);
            if ((ddsrc == DDS_RETCODE_OK) &&
                 !DDS_SubscriberImpl_is_hidden(subscriber))
            {
                if ((finalizer != NULL) &&
                    (finalizer->finalize_subscriber != NULL))
                {
                    finalizer->finalize_subscriber(subscriber);
                }
                ddsrc = DDS_DomainParticipant_delete_subscriber(participant,
                                                                subscriber);
            }
        }
    } while ((dbrc == DB_RETCODE_OK) && (ddsrc == DDS_RETCODE_OK));

    DB_Cursor_finish(participant->local_subscriber_table,cursor);

    if ((dbrc != DB_RETCODE_NO_DATA) || (ddsrc != DDS_RETCODE_OK))
    {
        /* Rely on underlying error-code */
        goto done;
    }

    /* Delete topics */
    cursor = NULL;
    dbrc = DB_Table_select_all_default(participant->topic_table,&cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,participant->topic_table,dbrc)
        goto done;
    }

    do
    {
        dbrc = DB_Cursor_get_next(cursor, (DB_Record_T*)&topic);
        if (dbrc == DB_RETCODE_OK)
        {
            if (!DDS_TopicImpl_is_hidden(topic))
            {
                if ((finalizer != NULL) && (finalizer->finalize_topic != NULL))
                {
                    finalizer->finalize_topic(topic);
                }
                /* it does not matter how many times the topic is referenced,
                 * delete_contained_entities deletes the topic anyway. The
                 * specification is not clear on this, but this is what
                 * Connext Core does.
                 */
                DDS_TopicImpl_reset_reference(topic);
                ddsrc = DDS_DomainParticipant_delete_topic(self, topic);
            }
        }
    } while ((dbrc == DB_RETCODE_OK) && (ddsrc == DDS_RETCODE_OK));

    DB_Cursor_finish(participant->topic_table,cursor);

    if ((dbrc != DB_RETCODE_NO_DATA) || (ddsrc != DDS_RETCODE_OK))
    {
        /* Rely on underlying error-code */
        goto done;
    }

    /* Delete all the types */
    cursor = NULL;
    dbrc = DB_Table_select_all_default(participant->type_table,&cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,participant->type_table,dbrc)
        goto done;
    }

    do
    {
        dbrc = DB_Cursor_get_next(cursor, (DB_Record_T*)&type);
        if (dbrc == DB_RETCODE_OK)
        {
            /* Do not delete built-in types, must be done manually by the
             * discovery plugins.
             */
            if (REDA_String_compare(type->name,
                    DDS_PUBLICATION_BUILTIN_TOPIC_TYPE_NAME) &&
                REDA_String_compare(type->name,
                    DDS_SUBSCRIPTION_BUILTIN_TOPIC_TYPE_NAME) &&
                REDA_String_compare(type->name,
                    DDS_PARTICIPANT_BUILTIN_TOPIC_TYPE_NAME)
#if DDS_LIVELINESS_CHANNEL_ENABLED
                &&
                REDA_String_compare(type->name,
                    DDS_PARTICIPANT_MESSAGE_DATA_TYPE_NAME)
#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */
                    )
            {
                /* It does not matter how many times the type is referenced,
                 * delete_contained_entities deletes the types anyway.
                 */
                DDS_TypeImpl_reset_reference(type);
                if (DDS_DomainParticipant_unregister_type_record(self,type) == NULL)
                {
                    ddsrc = DDS_RETCODE_ERROR;
                }
            }
        }
    } while ((dbrc == DB_RETCODE_OK) && (ddsrc == DDS_RETCODE_OK));

    DB_Cursor_finish(participant->type_table,cursor);

    if ((dbrc != DB_RETCODE_NO_DATA) || (ddsrc != DDS_RETCODE_OK))
    {
        /* Rely on underlying error-code */
        goto done;
    }

    ddsrc = DDS_RETCODE_OK;

done:

    if (DB_Database_unlock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if ((dbrc != DB_RETCODE_NO_DATA) && (dbrc != DB_RETCODE_OK))
    {
        return DDS_RETCODE_ERROR;
    }

    /* in case no db error, return the dds error code,
     * which might be DDS_RETCODE_OK or not
     */
    return ddsrc;
}
#endif /* !RTI_CERT */

/*******************************************************************************
 *                                    Public API
 ******************************************************************************/

#ifndef RTI_CERT
DDS_ReturnCode_t
DDS_DomainParticipant_delete_contained_entities(DDS_DomainParticipant *self)
{
     OSAPI_PRECONDITION_ALWAYS((self == NULL),
                            return DDS_RETCODE_BAD_PARAMETER,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)
    return DDS_DomainParticipant_delete_contained_entities_w_finalizerI(self, NULL);
}
#endif

DDS_Publisher*
DDS_DomainParticipant_create_publisher(DDS_DomainParticipant *self,
                                const struct DDS_PublisherQos *qos,
                                const struct DDS_PublisherListener *listener,
                                DDS_StatusMask mask)
{
    DDS_Publisher *retval = NULL;
    DB_ReturnCode_T dbrc;
    struct DDS_DomainParticipantImpl *participant =
            (struct DDS_DomainParticipantImpl *)self;
    struct DDS_PublisherImpl *publisher = NULL;
    DDS_UnsignedLong object_id;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (qos == NULL),
                           return NULL,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE);)

    if (DB_Database_lock(participant->database) != DB_RETCODE_OK)
    {
        return NULL;
    }

    object_id = DDS_DomainParticipant_get_next_objectid(self);
    if (object_id == 0)
    {
        DDSC_LOG_GET_NEXT_OBJECT_ID(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    dbrc = DB_Table_create_record(participant->local_publisher_table,
                                  (DB_Record_T*)&publisher);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_CREATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLISHER_RECORD,dbrc)
        goto done;
    }

    self->pub_config.db = participant->database;
    self->pub_config.timer = participant->timer;
    self->pub_config.registry = participant->config.registry;
    self->pub_config.local_writer_table = participant->local_writer_table;
    self->pub_config.object_id_generator = DDS_DomainParticipant_get_next_objectid;
    self->pub_config.get_parent_handle = DDS_DomainParticipantImpl_get_instance_handle;
    self->pub_config.on_before_datawriter_created = DomainParticipantEvent_on_before_datawriter_created;
    self->pub_config.on_after_datawriter_enabled = DomainParticipantEvent_on_after_datawriter_enabled;
#ifndef RTI_CERT
    self->pub_config.on_before_datawriter_deleted = DomainParticipantEvent_on_before_datawriter_deleted;
    self->pub_config.on_after_datawriter_deleted = DomainParticipantEvent_on_after_datawriter_deleted;
#else
    self->pub_config.on_before_datawriter_deleted = NULL;
    self->pub_config.on_after_datawriter_deleted = NULL;
#endif /* !RTI_CERT */
    self->pub_config.on_liveliness_lost = NDDS_DomainParticipant_on_liveliness_lost;
    self->pub_config.on_offered_deadline_missed = NDDS_DomainParticipant_on_offered_deadline_missed;
    self->pub_config.on_offered_incompatible_qos = NDDS_DomainParticipant_on_offered_incompatible_qos;
    self->pub_config.on_publication_matched = NDDS_DomainParticipant_on_publication_matched;
    self->pub_config.on_reliable_reader_activity_changed = NDDS_DomainParticipant_on_reliable_reader_activity_changed;
    self->pub_config.default_unicast = &participant->builtin_data.default_unicast_locators;
    self->pub_config.default_multicast = &participant->builtin_data.default_multicast_locators;
    self->pub_config.default_meta_multicast = &participant->builtin_data.metatraffic_multicast_locators;
    self->pub_config.default_meta_unicast = &participant->builtin_data.metatraffic_unicast_locators;
    self->pub_config.bind_resolver = participant->bind_resolver;
    self->pub_config.enabled_transports = &participant->qos.transports.enabled_transports;
    self->pub_config.route_resolver = participant->route_resolver;
    self->pub_config.addr_resolver = participant->address_resolver;
    self->pub_config.participant_id = &participant->participant_id;
    self->pub_config.dw_config = &participant->dw_config;
    self->pub_config.domain_id = &participant->domain_id;
    self->pub_config.participant_property_qos_policy = &participant->qos.property;
    self->pub_config.ext_rtps_intf = participant->rtps_intf;
    self->pub_config.packet_pool = participant->packet_pool;
    self->pub_config.resolved_unicast_seq = &participant->resolved_unicast_seq;
    self->pub_config.resolved_multicast_seq = &participant->resolved_multicast_seq;

#if DDS_FLOW_CONTROLLER_ENABLED
    self->pub_config.dw_config->acquire_flowcontroller =
                            DDS_DomainParticipant_acquire_flowcontroller;
    self->pub_config.dw_config->release_flowcontroller =
                            DDS_DomainParticipant_release_flowcontroller;
#endif
#if DDS_FILTERING_ENABLED
    if (!self->qos.filter.disable_writer_filtering)
    {
        self->pub_config.filter_plugin = participant->filter_plugin;
    }
#endif /* DDS_FILTERING_ENABLED */

    if (!DDS_PublisherImpl_initialize(publisher,self,qos,listener,mask,
                                      object_id,&self->pub_config))
    {
        (void)DB_Table_delete_record(participant->local_publisher_table,
                                     (DB_Record_T)publisher);
        goto done;
    }

    dbrc = DB_Table_insert_record(participant->local_publisher_table,publisher);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_INSERT(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLISHER_RECORD,dbrc)
        (void)DB_Table_delete_record(participant->local_publisher_table,
                                     (DB_Record_T)publisher);
        goto done;
    }

    if ((self->as_entity.state == RTIDDS_ENTITY_STATE_ENABLED) &&
            self->qos.entity_factory.autoenable_created_entities)
    {
        if (DDS_Entity_enable(DDS_Publisher_as_entity(publisher)) !=
            DDS_RETCODE_OK)
        {
            DDSC_LOG_ENTITY_ENABLE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLISHER_ENTITY)
            goto done;
        }
    }

    retval = publisher;

done:

    if (DB_Database_unlock(participant->database) != DB_RETCODE_OK)
    {
        retval = NULL;
    }

    return retval;
}

#ifndef RTI_CERT

RTI_PRIVATE DDS_ReturnCode_t
DDS_DomainParticipant_delete_publisher_no_lock(DDS_DomainParticipant *self,
                                               DDS_Publisher *publisher)
{
    DDS_ReturnCode_t ddsrc = DDS_RETCODE_ERROR;
    DB_ReturnCode_T dbrc;
    struct DDS_PublisherImpl *pub_impl = (struct DDS_PublisherImpl*)publisher;

    if (!DDS_PublisherImpl_finalize(pub_impl))
    {
        goto done;
    }

    dbrc = DB_Table_delete_record(self->local_publisher_table,pub_impl);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_DELETE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLISHER_RECORD,dbrc)
        goto done;
    }

    ddsrc = DDS_RETCODE_OK;

done:

    return ddsrc;
}

DDS_ReturnCode_t
DDS_DomainParticipant_delete_publisher(DDS_DomainParticipant *self,
                                       DDS_Publisher *publisher)
{
    DDS_ReturnCode_t ddsrc = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (publisher == NULL),
                   return DDS_RETCODE_BAD_PARAMETER,
                   OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                   OSAPI_Log_entry_add_pointer("publisher",publisher,RTI_TRUE);)

    if (DB_Database_lock(self->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    ddsrc = DDS_DomainParticipant_delete_publisher_no_lock(self,publisher);

    if (DB_Database_unlock(self->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return ddsrc;
}
#endif /* !RTI_CERT */

DDS_Subscriber*
DDS_DomainParticipant_create_subscriber(DDS_DomainParticipant *self,
                                const struct DDS_SubscriberQos *qos,
                                const struct DDS_SubscriberListener *listener,
                                DDS_StatusMask mask)
{
    DDS_Subscriber *retval = NULL;
    DB_ReturnCode_T dbrc;
    struct DDS_DomainParticipantImpl *participant =
                                    (struct DDS_DomainParticipantImpl *)self;
    struct DDS_SubscriberImpl *subscriber = NULL;
    DDS_UnsignedLong object_id;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (qos == NULL),
                           return NULL,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE);)

    if (DB_Database_lock(participant->database) != DB_RETCODE_OK)
    {
        return NULL;
    }

    object_id = DDS_DomainParticipant_get_next_objectid(self);
    if (object_id == 0)
    {
        DDSC_LOG_GET_NEXT_OBJECT_ID(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    dbrc = DB_Table_create_record(participant->local_subscriber_table,
                                  (DB_Record_T*)&subscriber);

    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_CREATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIBER_RECORD,dbrc)
        goto done;
    }

    self->sub_config.db = participant->database;
    self->sub_config.timer = participant->timer;
    self->sub_config.registry = participant->config.registry;
    self->sub_config.local_reader_table = participant->local_reader_table;
    self->sub_config.object_id_generator = DDS_DomainParticipant_get_next_objectid;
    self->sub_config.get_parent_handle = DDS_DomainParticipantImpl_get_instance_handle;
    self->sub_config.on_before_datareader_created = DomainParticipantEvent_on_before_datareader_created;
    self->sub_config.on_after_datareader_enabled = DomainParticipantEvent_on_after_datareader_enabled;
#ifndef RTI_CERT
    self->sub_config.on_before_datareader_deleted = DomainParticipantEvent_on_before_datareader_deleted;
#else
    self->sub_config.on_before_datareader_deleted = NULL;
#endif

    self->sub_config.on_requested_deadline_missed = NDDS_DomainParticipant_on_requested_deadline_missed;
    self->sub_config.on_requested_incompatible_qos = NDDS_DomainParticipant_on_requested_incompatible_qos;
    self->sub_config.on_sample_rejected = NDDS_DomainParticipant_on_sample_rejected;
    self->sub_config.on_liveliness_changed = NDDS_DomainParticipant_on_liveliness_changed;
    self->sub_config.on_data_available = NDDS_DomainParticipant_on_data_available;
    self->sub_config.on_data_on_readers = NDDS_DomainParticipant_on_data_on_readers;
    self->sub_config.on_subscription_matched = NDDS_DomainParticipant_on_subscription_matched;
    self->sub_config.on_sample_lost = NDDS_DomainParticipant_on_sample_lost;
    self->sub_config.on_instance_replaced = NDDS_DomainParticipantListener_on_instance_replaced;
    self->sub_config.default_unicast = &participant->builtin_data.default_unicast_locators;
    self->sub_config.default_multicast = &participant->builtin_data.default_multicast_locators;
    self->sub_config.default_meta_multicast = &participant->builtin_data.metatraffic_multicast_locators;
    self->sub_config.default_meta_unicast = &participant->builtin_data.metatraffic_unicast_locators;
    self->sub_config.bind_resolver = participant->bind_resolver;
    self->sub_config.address_resolver = participant->address_resolver;
    self->sub_config.enabled_transports = &participant->qos.transports.enabled_transports;
    self->sub_config.route_resolver = participant->route_resolver;
    self->sub_config.participant_id = &participant->participant_id;
    self->sub_config.dr_config = &participant->dr_config;
    self->sub_config.domain_id = &participant->domain_id;
    self->sub_config.ext_rtps_intf = participant->rtps_intf;
    self->sub_config.packet_pool = participant->packet_pool;
    self->sub_config.resolved_unicast_seq = &participant->resolved_unicast_seq;
    self->sub_config.resolved_multicast_seq = &participant->resolved_multicast_seq;

#if DDS_FILTERING_ENABLED
    self->sub_config.filter_plugin = participant->filter_plugin;
#endif /* DDS_FILTERING_ENABLED */

    if (!DDS_SubscriberImpl_initialize(subscriber,self,qos,listener,mask,
                                       object_id,&self->sub_config))
    {
        (void)DB_Table_delete_record(participant->local_subscriber_table,
                                     (DB_Record_T)subscriber);
        goto done;
    }

    dbrc = DB_Table_insert_record(participant->local_subscriber_table,subscriber);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_INSERT(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIBER_RECORD,dbrc)
        (void)DB_Table_delete_record(participant->local_subscriber_table,
                                     (DB_Record_T)subscriber);
        goto done;
    }

    if ((self->as_entity.state == RTIDDS_ENTITY_STATE_ENABLED) &&
            self->qos.entity_factory.autoenable_created_entities)
    {
        if (DDS_Entity_enable(DDS_Subscriber_as_entity(subscriber)) !=
                    DDS_RETCODE_OK)
        {
            DDSC_LOG_ENTITY_ENABLE(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIBER_ENTITY)
            goto done;
        }
    }

    retval = subscriber;

done:

    if (DB_Database_unlock(participant->database) != DB_RETCODE_OK)
    {
        retval = NULL;
    }

    return retval;
}

#ifndef RTI_CERT
RTI_PRIVATE DDS_ReturnCode_t
DDS_DomainParticipant_delete_subscriber_no_lock(DDS_DomainParticipant *self,
                                                DDS_Subscriber *subscriber)
{
    struct DDS_SubscriberImpl *sub_impl = (struct DDS_SubscriberImpl*)subscriber;
    DB_ReturnCode_T dbrc;
    DDS_ReturnCode_t ddsrc = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (subscriber == NULL),
               return DDS_RETCODE_BAD_PARAMETER,
               OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("subscriber",subscriber,RTI_TRUE);)

    if (!DDS_SubscriberImpl_finalize(sub_impl))
    {
        goto done;
    }

    dbrc = DB_Table_delete_record(self->local_subscriber_table,sub_impl);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_DELETE(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIBER_RECORD,dbrc)
        goto done;
    }

    ddsrc = DDS_RETCODE_OK;

done:

    return ddsrc;
}

DDS_ReturnCode_t
DDS_DomainParticipant_delete_subscriber(DDS_DomainParticipant *self,
                                        DDS_Subscriber *subscriber)
{
    DDS_ReturnCode_t ddsrc = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (subscriber == NULL),
               return DDS_RETCODE_BAD_PARAMETER,
               OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("subscriber",subscriber,RTI_TRUE);)


    if (DB_Database_lock(self->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    ddsrc = DDS_DomainParticipant_delete_subscriber_no_lock(self,subscriber);

    if (DB_Database_unlock(self->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return ddsrc;
}
#endif

DDS_Topic*
DDS_DomainParticipant_create_topic(DDS_DomainParticipant *self,
                                   const char *topic_name,
                                   const char *type_name,
                                   const struct DDS_TopicQos *qos,
                                   const struct DDS_TopicListener *listener,
                                   DDS_StatusMask mask)
{
    DDS_Topic *retval = NULL;
    DB_ReturnCode_T dbrc;
    struct DDS_DomainParticipantImpl *participant =
                                    (struct DDS_DomainParticipantImpl *)self;
    struct DDS_TopicImpl *topic = NULL;
    struct DDS_TypeImpl *a_type = NULL;
    char* managed_topic = NULL;
    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (qos == NULL) ||
                           (topic_name == NULL) || (type_name == NULL),
                           return NULL,
                        OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("qos",qos,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("topic",
                                (topic_name != NULL ? topic_name : "<NULL>"),
                                                                    RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("type",
                                (type_name != NULL ? type_name : "<NULL>"),
                                                                    RTI_TRUE);)

    if (DB_Database_lock(participant->database) != DB_RETCODE_OK)
    {
        return NULL;
    }

    dbrc = DB_Table_select_match(self->type_table,DB_TABLE_DEFAULT_INDEX,
                                (DB_Record_T*)&a_type,(DB_Key_T)type_name);

    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_LOOKUP(OSAPI_LOGKIND_ERROR,DDSC_LOG_TYPE_RECORD)
        goto done;
    }

    managed_topic =
        DDS_StringManager_assert_string(self->string_manager,topic_name);
    if (managed_topic == NULL)
    {
        DDSC_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR, DDSC_LOG_STRING_OBJECT)
        goto done;
    }

    dbrc = DB_Table_create_record(self->topic_table,(DB_Record_T*)&topic);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_CREATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_TOPIC_RECORD,dbrc)
        (void)DDS_StringManager_delete_string(participant->string_manager,managed_topic);
        goto done;
    }

    self->topic_config.db = participant->database;
    self->topic_config.on_inconsistent_topic = NDDS_DomainParticipant_on_inconsistent_topic;
    self->topic_config.get_parent_handle = DDS_DomainParticipantImpl_get_instance_handle;


    if (!DDS_TopicImpl_initialize(topic,
                                self,managed_topic,a_type,
                                qos,listener,mask,
                                0,&self->topic_config))
    {
        (void)DB_Table_delete_record(participant->topic_table,
                                        (DB_Record_T)topic);
        (void)DDS_StringManager_delete_string(participant->string_manager,managed_topic);
        goto done;
    }


    dbrc = DB_Table_insert_record(participant->topic_table,topic);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_INSERT(OSAPI_LOGKIND_ERROR,DDSC_LOG_TOPIC_RECORD,dbrc)
        (void)DB_Table_delete_record(participant->topic_table,
                                     (DB_Record_T)topic);
        (void)DDS_StringManager_delete_string(participant->string_manager,managed_topic);
        goto done;
    }

    if ((self->as_entity.state == RTIDDS_ENTITY_STATE_ENABLED) &&
            self->qos.entity_factory.autoenable_created_entities)
    {
        if (DDS_Entity_enable(DDS_Topic_as_entity(topic)) != DDS_RETCODE_OK)
        {
            DDSC_LOG_ENTITY_ENABLE(OSAPI_LOGKIND_ERROR,DDSC_LOG_TOPIC_ENTITY)
            goto done;
        }
    }

    retval = topic;

done:

    if (DB_Database_unlock(participant->database) != DB_RETCODE_OK)
    {
        retval = NULL;
    }

    return retval;
}

#ifndef RTI_CERT
DDS_ReturnCode_t
DDS_DomainParticipant_delete_topic_w_finalizer(DDS_DomainParticipant *self,
                           DDS_Topic *topic,
                           DDS_DomainParticipant_finalize_topic_fn finalizer)
{
    DB_ReturnCode_T dbrc;
    DDS_ReturnCode_t ddsrc = DDS_RETCODE_ERROR;
    char *managed_topic = NULL;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (topic == NULL),
                            return DDS_RETCODE_BAD_PARAMETER,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("topic",topic,RTI_TRUE);)

    if (DDS_TopicDescription_get_participant(
                            DDS_Topic_as_topicdescription(topic)) != self)
    {
        DDSC_LOG_ENTITY_DIFFERENT_FACTORY(OSAPI_LOGKIND_ERROR,
                            DDSC_LOG_TOPIC_ENTITY,DDSC_LOG_PARTICIPANT_ENTITY)
        return DDS_RETCODE_PRECONDITION_NOT_MET;
    }

    if (DB_Database_lock(self->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (!DDS_TopicImpl_is_referenced(topic))
    {
        DDSC_LOG_OBJECT_REFCOUNT(OSAPI_LOGKIND_ERROR,DDSC_LOG_TOPIC_OBJECT)
        ddsrc = DDS_RETCODE_ERROR;
        goto done;
    }

    /* Require equal amount of deletes to (find + create) calls */
    DDS_TopicImpl_dereference(topic);
    if (DDS_TopicImpl_is_referenced(topic))
    {
        DDSC_LOG_OBJECT_REFCOUNT(OSAPI_LOGKIND_WARNING,DDSC_LOG_TOPIC_OBJECT)
        ddsrc = DDS_RETCODE_OK;
        goto done;
    }

    /* It is illegal to delete a topic that is used. If this point is reached
     * ref_count == 0. However, if the deletion fails because the topic is
     * in use, increment it to 1 to allow it to be deleted again.
     */
    if (DDS_TopicImpl_is_attached(topic))
    {
        DDS_TopicImpl_reference(topic);
        DDSC_LOG_OBJECT_INUSECOUNT(OSAPI_LOGKIND_ERROR,DDSC_LOG_TOPIC_OBJECT)
        ddsrc = DDS_RETCODE_PRECONDITION_NOT_MET;
        goto done;
    }

    if (finalizer != NULL)
    {
        finalizer(topic);
    }

    managed_topic = (char*)DDS_TopicDescription_get_name(
                                    DDS_Topic_as_topicdescription(topic));

    dbrc = DB_Table_delete_record_w_dtor(self->topic_table,(DB_Record_T)topic,DDS_Topic_dtor);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_DELETE(OSAPI_LOGKIND_ERROR,DDSC_LOG_TOPIC_RECORD,dbrc)
        goto done;
    }

    if (!DDS_StringManager_delete_string(self->string_manager,managed_topic))
    {
        goto done;
    }



    ddsrc = DDS_RETCODE_OK;

done:

    if (DB_Database_unlock(self->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return ddsrc;
}

DDS_ReturnCode_t
DDS_DomainParticipant_delete_topic(DDS_DomainParticipant *self,
                                   DDS_Topic *topic)
{
    return DDS_DomainParticipant_delete_topic_w_finalizer(self,topic,NULL);
}

#endif

/*ci
 * \brief Find a topic-name in a participant
 *
 * \details
 * This function is used to search for a topic in a participant. It is an
 * internal function which can either do a simple lookup or lookup and reference
 * count the topic. The latter case is used by find_topic and has the same
 * semantics as creating a topic. That is, a topic found with reference counting
 * set to TRUE must also be deleted.
 *
 * NOTE: Only a timeout of zero if supported, that is this function does not
 *       block.
 *
 * \param[in] self       The participant to search for the topic in
 * \param[in] topic_name The name of the topic to search for
 * \param[in] timeout    The maximum time to wait for the topic to be found
 * \param[in] ref_count  Whether to reference count the topic or not.
 *
 * \return DDS_BOOLEAN_TRUE if the immutable parts are equal,
 *         DDS_BOOLEAN_FALSE is not equal
 */
RTI_PRIVATE DDS_Topic*
DDS_DomainParticipant_find_topic_impl(DDS_DomainParticipant *self,
                                     const char *topic_name,
                                     const struct DDS_Duration_t *timeout,
                                     RTI_BOOL ref_count)
{
    DDS_Topic *topic = NULL;
    DB_ReturnCode_T dbrc;

    /* We do not support timeout. If the topic does not exist at call-time
     * we return NULL.
     */
    if (!DDS_Duration_is_zero(timeout))
    {
        DDSC_LOG_INVALID_DURATION(OSAPI_LOGKIND_ERROR)
        return NULL;
    }

    if (DB_Database_lock(self->database) != DB_RETCODE_OK)
    {
        return NULL;
    }

    dbrc = DB_Table_select_match(self->topic_table,DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&topic,(DB_Key_T)topic_name);

#if OSAPI_ENABLE_LOG
    if ((dbrc != DB_RETCODE_OK) && (dbrc != DB_RETCODE_NO_DATA))
    {
        DDSC_LOG_TOPIC_FIND(OSAPI_LOGKIND_ERROR,topic_name,dbrc)
    }
    else
#endif
    if ((dbrc == DB_RETCODE_OK) && ref_count)
    {
        /* If the topic already exists maintain correct number of references.
         * The topic must be deleted once for each reference.
         */
        DDS_TopicImpl_reference(topic);
    }

    if (DB_Database_unlock(self->database) != DB_RETCODE_OK)
    {
        return NULL;
    }

    return topic;
}

#if INCLUDE_API_LOOKUP
DDS_Topic*
DDS_DomainParticipant_find_topic(DDS_DomainParticipant *self,
                                 const char *topic_name,
                                 const struct DDS_Duration_t *timeout)
{
    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (topic_name == NULL) ||
                           (timeout == NULL),
                            return NULL,
                        OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("topic_name",topic_name,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("timeout",timeout,RTI_TRUE);)

    return DDS_DomainParticipant_find_topic_impl(self,topic_name,
                                                 timeout,RTI_TRUE);
}
#endif

DDS_TopicDescription*
DDS_DomainParticipant_lookup_topicdescription(DDS_DomainParticipant *self,
                                              const char *topic_name)
{
    DDS_TopicDescription *topic_description = NULL;
    DDS_Topic *topic = NULL;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (topic_name == NULL),
                            return NULL,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("topic_name",topic_name,RTI_TRUE);)

    topic = DDS_DomainParticipant_find_topic_impl(self,topic_name,
                                                  &DDS_DURATION_ZERO,RTI_FALSE);

    if (topic != NULL)
    {
        topic_description = DDS_Topic_as_topicdescription(topic);
    }

    return topic_description;
}

DDS_DomainId_t
DDS_DomainParticipant_get_domain_id(DDS_DomainParticipant * self)
{
    OSAPI_PRECONDITION_ALWAYS(self == NULL,
               return -1,
               OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return self->domain_id;
}

DDS_Long
DDS_DomainParticipant_get_participant_id(DDS_DomainParticipant *self)
{
    return self->participant_id;
}

#if DDS_LIVELINESS_CHANNEL_ENABLED
DDS_ReturnCode_t
DDS_DomainParticipant_assert_liveliness(DDS_DomainParticipant *self)
{
    DB_ReturnCode_T dbrc;
    DDS_ReturnCode_t ddsrc = DDS_RETCODE_OK;

    dbrc = DB_Database_lock(self->database);
    if (dbrc != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (self->ipc_liveliness == NULL)
    {
        /* If the liveliness channel has not been created, it is because
         * it is not require. Since it is not required, it has no effect
         * thus return ok.
         */
        ddsrc = DDS_RETCODE_OK;
    }
    else
    {
        DDS_IpcLiveliness_assert_liveliness(self->ipc_liveliness);
    }

    dbrc = DB_Database_unlock(self->database);
    if (dbrc != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return ddsrc;
}
#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */

OSAPI_Timer_T
DDS_DomainParticipant_get_timer(DDS_DomainParticipant * const self)
{
    return ((struct DDS_DomainParticipantImpl *)self)->timer;
}

/*
 * \brief Add a peer participant
 *
 * \details
 *
 * For each discovery plug-in registered with the participant add the
 * new peer to the discovery plug-in. It is up to the peer to determine
 * what to do with the address.
 */
DDS_ReturnCode_t
DDS_DomainParticipant_add_peer(DDS_DomainParticipant *self,
                               const char* peer)
{
    DB_ReturnCode_T dbrc;
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (peer == NULL),
                                  return DDS_RETCODE_BAD_PARAMETER,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("peer",peer,RTI_TRUE);)

    dbrc = DB_Database_lock(self->database);
    if (dbrc != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (self->disc_plugin)
    {
        if (!NDDS_Discovery_Plugin_add_peer(self->disc_plugin,self,peer))
        {
            DDSC_LOG_DISC_ADD_PEER(OSAPI_LOGKIND_WARNING,
                    RT_ComponentFactoryId_get_name(
                            &self->qos.discovery.discovery.name),
                    peer);
            retcode = DDS_RETCODE_ERROR;
        }
    }

    dbrc = DB_Database_unlock(self->database);

    if (dbrc != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}

DDS_ReturnCode_t
DDS_DomainParticipant_announce(DDS_DomainParticipant *self)
{
    DB_ReturnCode_T dbrc;
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;

    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                              return DDS_RETCODE_BAD_PARAMETER,
                              OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (!DDS_Entity_is_enabled(&self->as_entity))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_ENTITY)
        return DDS_RETCODE_NOT_ENABLED;
    }

    dbrc = DB_Database_lock(self->database);
    if (dbrc != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (self->disc_plugin == NULL)
    {
        retcode = DDS_RETCODE_UNSUPPORTED;
    }
    else if (NDDS_Discovery_Plugin_has_on_write_announcement(self->disc_plugin))
    {
        retcode = NDDS_Discovery_Plugin_on_write_announcement(
                                self->disc_plugin, self, &self->builtin_data);
    }
    else
    {
        retcode = DDS_RETCODE_UNSUPPORTED;
    }

    dbrc = DB_Database_unlock(self->database);
    if (dbrc != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}

DDS_ReturnCode_t
DDS_DomainParticipant_remove_discovered_participants(DDS_DomainParticipant *self)
{
    struct DDS_DomainParticipantImpl *participant;
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;
    DB_ReturnCode_T dbrc = DB_RETCODE_ERROR;
    DB_Cursor_T cursor = NULL;
    struct DDS_RemoteParticipantImpl *rem_participant;
    DDS_BuiltinTopicKey_t key = DDS_BuiltinTopicKey_t_INITIALIZER;

    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                              return DDS_RETCODE_BAD_PARAMETER,
                              OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    participant = (struct DDS_DomainParticipantImpl *)self;

    if (!DDS_Entity_is_enabled(&self->as_entity))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_ENTITY)
        return DDS_RETCODE_NOT_ENABLED;
    }

    if (DB_Database_lock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    dbrc = DB_Table_select_all_default(participant->remote_participant_table,
                                        &cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        retcode = DDS_RETCODE_ERROR;
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,
                              participant->remote_participant_table,dbrc)
        goto done;
    }
    
    do
    {
        dbrc = DB_Cursor_get_next(cursor, (DB_Record_T *)&rem_participant);

        if ((dbrc == DB_RETCODE_OK) && NDDS_RemoteEntity_is_enabled(&rem_participant->as_entity))
        {
            OSAPI_Memory_copy(&key, &rem_participant->data.key, sizeof(key));
            
#ifndef RTI_CERT
            if (DDS_RemoteParticipant_check_status(rem_participant,
                                        DDS_REMOTE_PARTICIPANT_STATUS_STATIC))
            {
#endif
                /* Reset inserts a record which invalidates the cursor.
                 * if DB_Table_select_all_default is performed then we
                 * get the first record again because static records are
                 * reinserted. Make sure we only process enabled records. */
                retcode = NDDS_DomainParticipant_reset_remote_participant(participant, &key);
                
                DB_Cursor_finish(participant->remote_participant_table, cursor);
                dbrc = DB_Table_select_all_default(participant->remote_participant_table,
                                        &cursor);
                if (dbrc != DB_RETCODE_OK)
                {
                    retcode = DDS_RETCODE_ERROR;
                    DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,
                                        participant->remote_participant_table,dbrc)
                    goto done;
                }
                
#ifndef RTI_CERT
            }
            else
            {
                retcode = NDDS_DomainParticipant_remove_remote_participant(participant, &key);
            }
#endif
            if (retcode != DDS_RETCODE_OK)
            {
                goto done;
            }
        }
    } while (dbrc == DB_RETCODE_OK);

done:

    if (cursor != NULL)
    {
        DB_Cursor_finish(participant->remote_participant_table, cursor);
    }

    if (DB_Database_unlock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}

struct DDS_PropertySeq*
DDS_DomainParticipant_get_dds_properties(DDS_DomainParticipant *self)
{
    struct DDS_DomainParticipantImpl *participant =
                                    (struct DDS_DomainParticipantImpl *)self;

    OSAPI_PRECONDITION(self == NULL,
                           return NULL,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return &participant->builtin_data.property.value;
}

DDS_Boolean
DDS_DomainParticipant_locator_is_supported(DDS_DomainParticipant *self,
                                           struct DDS_Locator *locator)
{
    struct NETIO_Address *address = (struct NETIO_Address*)locator;

    return (NETIO_RouteResolver_lookup_interface(self->route_resolver,address)
                                        ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE);
}

/*******************************************************************************
 *                             OPTIONAL APIs
 ******************************************************************************/

#ifndef RTI_CERT
DDS_ReturnCode_t
DDS_DomainParticipant_set_listener(DDS_DomainParticipant *self,
                                const struct DDS_DomainParticipantListener *l,
                                DDS_StatusMask mask)
{
    struct DDS_DomainParticipantListener nil_listener =
                DDS_DomainParticipantListener_INITIALIZER;

    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if ((l != NULL) && !DDS_DomainParticipantListener_is_consistent(l,mask))
    {
        DDSC_LOG_LISTENER_INCONSISTENT(OSAPI_LOGKIND_ERROR,
                                       DDSC_LOG_PARTICIPANT_LISTENER,mask)
        return DDS_RETCODE_BAD_PARAMETER;
    }

    if (!OSAPI_Mutex_take(self->shared_lock))
    {
        return DDS_RETCODE_ERROR;
    }

    if (l == NULL)
    {
        self->listener = nil_listener;
    }
    else
    {
        self->listener = *l;
    }

    self->mask = mask;

    if (!OSAPI_Mutex_give(self->shared_lock))
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}
#endif

#ifndef RTI_CERT
struct DDS_DomainParticipantListener
DDS_DomainParticipant_get_listener(DDS_DomainParticipant *self)
{
    struct DDS_DomainParticipantListener retval =
            DDS_DomainParticipantListener_INITIALIZER;
    struct DDS_DomainParticipantListener nil_retval =
            DDS_DomainParticipantListener_INITIALIZER;

    OSAPI_PRECONDITION(self == NULL,
                            return retval,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (!OSAPI_Mutex_take(self->shared_lock))
    {
        return nil_retval;
    }

    retval = self->listener;

    if (!OSAPI_Mutex_give(self->shared_lock))
    {
        return nil_retval;
    }

    return retval;
}
#endif

#if INCLUDE_API_QOS
DDS_ReturnCode_t
DDS_DomainParticipant_get_default_publisher_qos(DDS_DomainParticipant *self,
                                                struct DDS_PublisherQos *qos)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (qos == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE);)


    if (!OSAPI_Mutex_take(self->shared_lock))
    {
        return DDS_RETCODE_ERROR;
    }

    retcode = DDS_PublisherQos_copy(qos, self->_default_publisher_qos);

#if OSAPI_ENABLE_LOG
    if (retcode != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_DEFAULTPUBLISHER_QOS)
    }
#endif

    if (!OSAPI_Mutex_give(self->shared_lock))
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}
#endif

#if INCLUDE_API_QOS
DDS_ReturnCode_t
DDS_DomainParticipant_set_default_publisher_qos(DDS_DomainParticipant * self,
                                            const struct DDS_PublisherQos *qos)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;
    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (qos == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE);)

    if (!OSAPI_Mutex_take(self->shared_lock))
    {
        return DDS_RETCODE_ERROR;
    }

    if (!DDS_PartitionQosPolicy_is_consistent_w_limits(&qos->partition,
        &DDS_DomainParticipant_get_qos_ref(self)->resource_limits))
    {
        retcode = DDS_RETCODE_PRECONDITION_NOT_MET;
        goto done;
    }

    if (!DDS_PublisherQos_is_equal(self->_default_publisher_qos, qos))
    {
        if (self->_default_publisher_qos == &DDS_PUBLISHER_QOS_DEFAULT)
        {
            OSAPI_Heap_allocate_struct(&self->_default_publisher_qos,
                                       struct DDS_PublisherQos);
            if (self->_default_publisher_qos == NULL)
            {
                retcode = DDS_RETCODE_OUT_OF_RESOURCES;
                goto done;
            }
            DDS_PublisherQos_initialize(self->_default_publisher_qos);
        }
        retcode = DDS_PublisherQos_set_from(self->_default_publisher_qos, qos,
                                            DDS_BOOLEAN_TRUE, self);
#if OSAPI_ENABLE_LOG
        if (retcode != DDS_RETCODE_OK)
        {
            DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_DEFAULTPUBLISHER_QOS)
        }
#endif
    }

done:

    if (!OSAPI_Mutex_give(self->shared_lock))
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}
#endif

#if INCLUDE_API_QOS
DDS_ReturnCode_t
DDS_DomainParticipant_set_qos(DDS_DomainParticipant * self,
        const struct DDS_DomainParticipantQos * qos)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (qos == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE);)

    /* The database lock is used here to prevent a race condition with
     * DDS_DomainParticipant_enable modifying self->state.
     */
    if (DB_Database_lock(self->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (!DDS_DomainParticipantQos_immutable_is_equal(&self->qos, qos))
    {
        DDSC_LOG_QOS_IMMUTABLE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_QOS)
        retcode = DDS_RETCODE_IMMUTABLE_POLICY;
        goto done;
    }

    if (!DDS_DomainParticipant_set_trust_property(self, qos))
    {
        DDSC_LOG_QOS_SET(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_QOS)
        retcode = DDS_RETCODE_NOT_ALLOWED_BY_SEC;
        goto done;
    }

    retcode = DDS_DomainParticipantQos_copy(&self->qos, qos);

#if OSAPI_ENABLE_LOG
    if (retcode != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_QOS)
    }
#endif

done:

    if (DB_Database_unlock(self->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}
#endif /* !RTI_CERT */

#if INCLUDE_API_QOS
DDS_ReturnCode_t
DDS_DomainParticipant_get_qos(DDS_DomainParticipant * self,
                              struct DDS_DomainParticipantQos *qos)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (qos == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE);)

    if (DB_Database_lock(self->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    retcode = DDS_DomainParticipantQos_copy(qos,&self->qos);

#if OSAPI_ENABLE_LOG
    if (retcode != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_QOS)
    }
#endif

    if (DB_Database_unlock(self->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}
#endif /* !RTI_CERT */

struct DDS_DomainParticipantQos*
DDS_DomainParticipant_get_qos_ref(DDS_DomainParticipant *self)
{
    OSAPI_PRECONDITION(self == NULL,
                           return NULL,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return &self->qos;
}

DDS_UserDataManager_T*
DDS_DomainParticipant_get_user_data_manager(DDS_DomainParticipant *self)
{
    OSAPI_PRECONDITION(self == NULL,
                           return NULL,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return self->user_data_manager;

}

struct NDDS_ParticipantConfig*
DDS_DomainParticipant_get_cfg_ref(DDS_DomainParticipant *self)
{
    OSAPI_PRECONDITION(self == NULL,
                           return NULL,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return &self->config;
}

#if INCLUDE_API_QOS
DDS_ReturnCode_t
DDS_DomainParticipant_get_default_subscriber_qos(DDS_DomainParticipant *self,
                                                 struct DDS_SubscriberQos *qos)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (qos == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE);)

    if (!OSAPI_Mutex_take(self->shared_lock))
    {
        return DDS_RETCODE_ERROR;
    }

    retcode = DDS_SubscriberQos_copy(qos,self->_default_subscriber_qos);

#if OSAPI_ENABLE_LOG
    if (retcode != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIBER_QOS)
    }
#endif

    if (!OSAPI_Mutex_give(self->shared_lock))
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}
#endif

#if INCLUDE_API_QOS
DDS_ReturnCode_t
DDS_DomainParticipant_set_default_subscriber_qos(DDS_DomainParticipant * self,
                                        const struct DDS_SubscriberQos *qos)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;
    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (qos == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE);)

    if (!OSAPI_Mutex_take(self->shared_lock))
    {
        return DDS_RETCODE_ERROR;
    }

    if (!DDS_PartitionQosPolicy_is_consistent_w_limits(&qos->partition,
                    &DDS_DomainParticipant_get_qos_ref(self)->resource_limits))
    {
        retcode = DDS_RETCODE_PRECONDITION_NOT_MET;
        goto done;
    }

    if (!DDS_SubscriberQos_is_equal(self->_default_subscriber_qos, qos))
    {
        if (self->_default_subscriber_qos == &DDS_SUBSCRIBER_QOS_DEFAULT)
        {
            OSAPI_Heap_allocate_struct(&self->_default_subscriber_qos,
                                       struct DDS_SubscriberQos);
            if (self->_default_subscriber_qos == NULL)
            {
                retcode = DDS_RETCODE_OUT_OF_RESOURCES;
                goto done;
            }
            DDS_SubscriberQos_initialize(self->_default_subscriber_qos);
        }

        retcode = DDS_SubscriberQos_set_from(self->_default_subscriber_qos,qos,
                                             DDS_BOOLEAN_TRUE, self);

#if OSAPI_ENABLE_LOG
        if (retcode != DDS_RETCODE_OK)
        {
            DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_DEFAULTSUBSCRIBER_QOS)
        }
#endif
    }

done:

    if (!OSAPI_Mutex_give(self->shared_lock))
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}
#endif

#if INCLUDE_API_QOS
DDS_ReturnCode_t
DDS_DomainParticipant_get_default_topic_qos(DDS_DomainParticipant *self,
                                            struct DDS_TopicQos *qos)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;
    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (qos == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE);)

    if (!OSAPI_Mutex_take(self->shared_lock))
    {
        return DDS_RETCODE_ERROR;
    }

    retcode = DDS_TopicQos_copy(qos, self->_default_topic_qos);

#if OSAPI_ENABLE_LOG
    if (retcode != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_DEFAULTTOPIC_QOS)
    }
#endif

    if (!OSAPI_Mutex_give(self->shared_lock))
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}
#endif

#if INCLUDE_API_QOS
DDS_ReturnCode_t
DDS_DomainParticipant_set_default_topic_qos(DDS_DomainParticipant *self,
                                            const struct DDS_TopicQos *qos)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;
    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (qos == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE);)

    if (!OSAPI_Mutex_take(self->shared_lock))
    {
        return DDS_RETCODE_ERROR;
    }

    if (!DDS_TopicQos_is_equal(self->_default_topic_qos,qos))
    {
        if (self->_default_topic_qos ==  &DDS_TOPIC_QOS_DEFAULT)
        {
            OSAPI_Heap_allocate_struct(&self->_default_topic_qos,
                                       struct DDS_TopicQos);
            if (self->_default_topic_qos == NULL)
            {
                retcode = DDS_RETCODE_OUT_OF_RESOURCES;
                goto done;
            }
        }
        retcode = DDS_TopicQos_set_from(self->_default_topic_qos, qos,
                                        DDS_BOOLEAN_TRUE, self);
#if OSAPI_ENABLE_LOG
        if (retcode != DDS_RETCODE_OK)
        {
            DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_DEFAULTTOPIC_QOS)
        }
#endif
    }

done:

    if (!OSAPI_Mutex_give(self->shared_lock))
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}
#endif

DDS_ReturnCode_t
DDS_DomainParticipant_set_user_data(DDS_DomainParticipant *self,
                                    DDS_DomainParticipantUserDataId user_data_id,
                                    void* data)
{
    struct DDS_DomainParticipantImpl *participant =
                                    (struct DDS_DomainParticipantImpl*)self;

    if (user_data_id >= DDS_PARTICIPANT_USER_DATA_MAX_ELEMENT)
    {
        return DDS_RETCODE_ERROR;
    }

    participant->user_data[user_data_id] = data;

    return DDS_RETCODE_OK;
}


DDS_ReturnCode_t
DDS_DomainParticipant_get_user_data(DDS_DomainParticipant *self,
                                    DDS_DomainParticipantUserDataId user_data_id,
                                    void **user_data_out)
{
    struct DDS_DomainParticipantImpl *participant =
                                (struct DDS_DomainParticipantImpl*)self;

    if (user_data_id >= DDS_PARTICIPANT_USER_DATA_MAX_ELEMENT)
    {
        return DDS_RETCODE_ERROR;
    }

    *user_data_out = participant->user_data[user_data_id];

    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t
DDS_DomainParticipant_get_current_time(DDS_DomainParticipant *self,
                                       struct DDS_Time_t *current_time)
{
    struct OSAPI_SystemTime now = OSAPI_TIME_ZERO;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (current_time == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
           OSAPI_Log_entry_add_pointer("current_time",current_time,RTI_TRUE);)

    if (!OSAPI_System_get_time(&now))
    {
        DDSC_LOG_SYS_GETTIME(OSAPI_LOGKIND_ERROR)
        return DDS_RETCODE_ERROR;
    }

    current_time->sec = now.sec;
    current_time->nanosec = now.nanosec;

    return DDS_RETCODE_OK;
}

struct DDS_RemoteParticipantImpl;

#if INCLUDE_API_LOOKUP
DDS_ReturnCode_t
DDS_DomainParticipant_get_discovered_participants(DDS_DomainParticipant *self,
                              struct DDS_InstanceHandleSeq *participant_handles)
{
    struct DDS_DomainParticipantImpl *participant =
                                     (struct DDS_DomainParticipantImpl *)self;
    DDS_ReturnCode_t result = DDS_RETCODE_ERROR;
    DB_ReturnCode_T dbrc = DB_RETCODE_ERROR;
    DB_Cursor_T cursor = NULL;
    struct DDS_RemoteParticipantImpl *rem_participant;
    DDS_InstanceHandle_t *handle;
    RTI_INT32 length = 0;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (participant_handles == NULL),
                  return DDS_RETCODE_BAD_PARAMETER,
                  OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                  OSAPI_Log_entry_add_pointer("participant_handles",participant_handles,RTI_TRUE);)

    if (!DDS_Entity_is_enabled(&participant->as_entity))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_ENTITY)
        return DDS_RETCODE_NOT_ENABLED;
    }

    dbrc = DB_Database_lock(participant->database);
    if (dbrc != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (!DDS_InstanceHandleSeq_set_length(participant_handles, 0))
    {
        goto done;
    }

    dbrc = DB_Table_select_all_default(
                              participant->remote_participant_table,&cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,participant->remote_participant_table,dbrc)
        goto done;
    }

    do
    {
        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&rem_participant);
        if ((dbrc == DB_RETCODE_OK) && NDDS_RemoteEntity_is_enabled(&rem_participant->as_entity))
        {
            if (!DDS_InstanceHandleSeq_ensure_length(participant_handles, length + 1,
                    participant->qos.resource_limits.remote_participant_allocation))
            {
                goto done;
            }

            handle = DDS_InstanceHandleSeq_get_reference(participant_handles, length);
            if (handle == NULL)
            {
                goto done;
            }

            DDS_InstanceHandle_from_rtps(handle, (struct RTPS_Guid *)&rem_participant->data.key);

            ++length;
        }
    } while (dbrc == DB_RETCODE_OK);

    DB_Cursor_finish(participant->remote_participant_table, cursor);
    cursor = NULL;

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    result = DDS_RETCODE_OK;

done:
    if (cursor != NULL)
    {
        DB_Cursor_finish(participant->remote_participant_table, cursor);
    }
    if (DB_Database_unlock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return result;
}
#endif

#if INCLUDE_API_LOOKUP
DDS_ReturnCode_t
DDS_DomainParticipant_get_discovered_participant_data(
            DDS_DomainParticipant *self,
            struct DDS_ParticipantBuiltinTopicData *participant_data,
            const DDS_InstanceHandle_t *participant_handle)
{
    DDS_ReturnCode_t result = DDS_RETCODE_ERROR;
    struct DDS_DomainParticipantImpl *participant =
                                    (struct DDS_DomainParticipantImpl *)self;
    DB_ReturnCode_T dbrc = DB_RETCODE_ERROR;
    struct DDS_BuiltinTopicKey_t key;
    struct DDS_RemoteParticipantImpl *remote_participant = NULL;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (participant_data == NULL) ||
                           (participant_handle == NULL),
           return DDS_RETCODE_BAD_PARAMETER,
           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
           OSAPI_Log_entry_add_pointer("participant_data",participant_data,RTI_FALSE);
           OSAPI_Log_entry_add_pointer("participant_handle",participant_handle,RTI_TRUE);)

    if (!DDS_Entity_is_enabled(&participant->as_entity))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_ENTITY)
        return DDS_RETCODE_NOT_ENABLED;
    }

    dbrc = DB_Database_lock(participant->database);
    if (dbrc != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    DDS_BuiltinTopicKey_from_instance_handle(&key, participant_handle);
    dbrc = DB_Table_select_match(participant->remote_participant_table,
                                DB_TABLE_DEFAULT_INDEX,
                                (DB_Record_T*)&remote_participant,
                                (DB_Key_T)&key);
    if (dbrc == DB_RETCODE_NO_DATA)
    {
        result = DDS_RETCODE_PRECONDITION_NOT_MET;
        goto done;
    }
    else if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,
                              participant->remote_participant_table,
                              dbrc)
        goto done;
    }

    /* RemoteParticipant must be enabled to be considered discovered */
    if (!NDDS_RemoteEntity_is_enabled(&remote_participant->as_entity))
    {
        result = DDS_RETCODE_PRECONDITION_NOT_MET;
        goto done;
    }

    if (!DDS_ParticipantBuiltinTopicData_copy(participant_data,
                                            &remote_participant->data))
    {
        goto done;
    }

    result = DDS_RETCODE_OK;

done:
    if (DB_Database_unlock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return result;
}
#endif

DDS_Publisher*
DDS_DomainParticipant_get_builtin_publisher(DDS_DomainParticipant *participant)
{
    struct DDS_DomainParticipantImpl *self =
            (struct DDS_DomainParticipantImpl*) participant;

    OSAPI_PRECONDITION(participant == NULL,
            return NULL,
            OSAPI_Log_entry_add_pointer("participant",participant,RTI_TRUE););

    return self->builtin_publisher;
}

DDS_Subscriber*
DDS_DomainParticipant_get_builtin_subscriber(DDS_DomainParticipant *participant)
{
    struct DDS_DomainParticipantImpl *self =
            (struct DDS_DomainParticipantImpl*) participant;

    OSAPI_PRECONDITION(participant == NULL,
            return NULL,
            OSAPI_Log_entry_add_pointer("participant",participant,RTI_TRUE););

    return self->builtin_subscriber;
}

struct DDS_ParticipantBuiltinTopicData*
DDS_DomainParticipant_get_builtin_data(DDS_DomainParticipant *participant)
{
    struct DDS_DomainParticipantImpl *self =
            (struct DDS_DomainParticipantImpl*) participant;

    OSAPI_PRECONDITION(participant == NULL,
            return NULL,
            OSAPI_Log_entry_add_pointer("participant",participant,RTI_TRUE););

    return &self->builtin_data;
}

#if DDS_ENABLE_APPGEN
DDS_Publisher*
DDS_DomainParticipant_lookup_publisher_by_name(
            DDS_DomainParticipant *self,
            const char *publisher_name)
{
    DB_ReturnCode_T dbrc;
    struct DDS_PublisherImpl *publisher = NULL;
    DB_Cursor_T cursor = NULL;
    struct DDS_DomainParticipantImpl *participant =
                                    (struct DDS_DomainParticipantImpl *)self;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (publisher_name == NULL),
           return NULL,
           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
           OSAPI_Log_entry_add_pointer("publisher_name",publisher_name,RTI_TRUE);)

    if (DB_Database_lock(participant->database) != DB_RETCODE_OK)
    {
        return NULL;
    }

    dbrc = DB_Table_select_all_default(participant->local_publisher_table, &cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,
                              participant->local_publisher_table,
                              dbrc)
        goto done;
    }

    do
    {
        dbrc = DB_Cursor_get_next(cursor, (DB_Record_T*)&publisher);
        if (dbrc == DB_RETCODE_OK)
        {
            if (!DDS_String_ncmp(publisher_name,
                                 publisher->publisher_name,
                                 DDS_ENTITYNAME_QOS_NAME_MAX))
            {
                break;
            }
            publisher = NULL;
        }
    } while (dbrc == DB_RETCODE_OK);

    DB_Cursor_finish(participant->local_publisher_table,cursor);

done:
    if (DB_Database_unlock(self->database) != DB_RETCODE_OK)
    {
        return NULL;
    }

    return publisher;
}

DDS_Subscriber*
DDS_DomainParticipant_lookup_subscriber_by_name(
            DDS_DomainParticipant *self,
            const char *subscriber_name)
{
    DB_ReturnCode_T dbrc;
    struct DDS_SubscriberImpl *subscriber = NULL;
    DB_Cursor_T cursor = NULL;
    struct DDS_DomainParticipantImpl *participant =
                                    (struct DDS_DomainParticipantImpl *)self;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (subscriber_name == NULL),
           return NULL,
           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
           OSAPI_Log_entry_add_pointer("subscriber_name",subscriber_name,RTI_TRUE);)

    if (DB_Database_lock(participant->database) != DB_RETCODE_OK)
    {
        return NULL;
    }

    dbrc = DB_Table_select_all_default(participant->local_subscriber_table, &cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,
                              participant->local_subscriber_table,
                              dbrc)
        goto done;
    }

    do
    {
        dbrc = DB_Cursor_get_next(cursor, (DB_Record_T*)&subscriber);
        if (dbrc == DB_RETCODE_OK)
        {
            if (!DDS_String_ncmp
                      (subscriber_name,
                       subscriber->subscriber_name,
                       DDS_ENTITYNAME_QOS_NAME_MAX))
            {
                break;
            }
            subscriber = NULL;
        }
    } while (dbrc == DB_RETCODE_OK);

    DB_Cursor_finish(participant->local_subscriber_table,cursor);

done:
    if (DB_Database_unlock(self->database) != DB_RETCODE_OK)
    {
        return NULL;
    }

    return subscriber;
}

/*ci
 * \brief Gets the publisher/subscriber names and the datawriter/datareader
 * names from a fully qualified name, e.g. "Publisher::DataWriter"
 *
 * \param[in]  qualified_name Pointer to fully qualified name
 * \param[out] Pointer to and array of length DDS_ENTITYNAME_QOS_NAME_MAX + 1
 *             The Publisher/Subscriber name will be copied here.
 * \param[out] Pointer to store the datawriter/datareader name start.
 *
 * \return RTI_TRUE on success. RTI_FALSE if error, in that case the output
 * parameter do not have any valid value and shall not be used.
 */
RTI_PRIVATE RTI_BOOL
DDS_DomainParticipant_split_qualified_names(
    const char *qualified_name,
    char *first_name,
    char **second_name_start)
{
    RTI_SIZE_T len = OSAPI_String_length(qualified_name);
    char *aux = NULL;
    RTI_SIZE_T first_name_len;
    RTI_BOOL ret_value = RTI_FALSE;

    *second_name_start = NULL;
    aux = OSAPI_Memory_fndchr((const void *)qualified_name,
                              (RTI_INT32)':', len);
    if (aux != NULL)
    {
        first_name_len = (RTI_SIZE_T)(aux - qualified_name);

        aux++;
        if ((*aux) == ':')
        {
            aux++;

            /* copy first name if not to long */
            if (first_name_len <= DDS_ENTITYNAME_QOS_NAME_MAX)
            {
                *second_name_start = aux;
                OSAPI_Memory_copy(first_name, qualified_name, first_name_len);
                first_name[first_name_len] = '\0';
                ret_value = RTI_TRUE;
            }
            else
            {
                DDSC_LOG_ENTITY_NAME_TOO_LONG(OSAPI_LOGKIND_ERROR,first_name_len);
            }
        }
        else
        {
            DDSC_LOG_NAME_LOOKUP(OSAPI_LOGKIND_ERROR,qualified_name);
        }
    }
    else
    {
        DDSC_LOG_NAME_LOOKUP(OSAPI_LOGKIND_ERROR,qualified_name);
    }

    return ret_value;
}

DDS_DataWriter*
DDS_DomainParticipant_lookup_datawriter_by_name(
            DDS_DomainParticipant *self,
            const char *datawriter_full_name)
{
    DDS_Publisher *publisher = NULL;
    DDS_DataWriter *datawriter = NULL;
    char pub_name[DDS_ENTITYNAME_QOS_NAME_MAX + 1];
    char *datawriter_name = NULL;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (datawriter_full_name == NULL),
           return NULL,
           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
           OSAPI_Log_entry_add_pointer("datawriter_full_name",datawriter_full_name,RTI_TRUE);)


    /* get publisher name */
    if (DDS_DomainParticipant_split_qualified_names(datawriter_full_name,
                                                    pub_name,
                                                    &datawriter_name))
    {
        publisher = DDS_DomainParticipant_lookup_publisher_by_name(
                        self, pub_name);

        if (publisher != NULL)
        {
            datawriter = DDS_Publisher_lookup_datawriter_by_name(
                             publisher, datawriter_name);
        }
    }

    return datawriter;
}

DDS_DataReader*
DDS_DomainParticipant_lookup_datareader_by_name(
            DDS_DomainParticipant *self,
            const char *datareader_full_name)
{
    DDS_Subscriber *subscriber = NULL;
    DDS_DataReader *datareader = NULL;
    char sub_name[DDS_ENTITYNAME_QOS_NAME_MAX + 1];
    char *datareader_name = NULL;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (datareader_full_name == NULL),
           return NULL,
           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
           OSAPI_Log_entry_add_pointer("datareader_full_name",datareader_full_name,RTI_TRUE);)

    /* get subscriber name */
    if (DDS_DomainParticipant_split_qualified_names(datareader_full_name,
                                                    sub_name,
                                                    &datareader_name))
    {
        subscriber = DDS_DomainParticipant_lookup_subscriber_by_name(
                         self, sub_name);

        if (subscriber != NULL)
        {
            datareader = DDS_Subscriber_lookup_datareader_by_name(
                             subscriber, datareader_name);
        }
    }

    return datareader;
}

#endif /* DDS_ENABLE_APPGEN */


DDS_Entity*
DDS_DomainParticipant_lookup_entity(DDS_DomainParticipant *self,
                                    const struct NETIO_Guid *const guid)
{
    DDS_UnsignedLong object_id = 0;
    DDS_UnsignedLong entity_kind;
    DB_ReturnCode_T dbrc;
    DDS_Entity *retval = NULL;

    OSAPI_Memory_copy(&object_id,&guid->entity.value,sizeof(RTI_UINT32));
    object_id = NETIO_ntohl(object_id);

    if (DB_Database_lock(self->database) != DB_RETCODE_OK)
    {
        return NULL;
    }

    entity_kind = guid->entity.value[3] & 0xf;

    if ((entity_kind == 2) || (entity_kind == 3))
    {
        struct DDS_DataWriterImpl *local_writer = NULL;

        dbrc = DB_Table_select_match(self->local_writer_table,
                                     DB_TABLE_DEFAULT_INDEX,
                                     (DB_Record_T)&local_writer,
                                     (DB_Key_T)&object_id);
        if (dbrc != DB_RETCODE_OK)
        {
            goto done;
        }

        retval = DDS_DataWriter_as_entity(local_writer);
    }
    else if ((entity_kind == 4) || (entity_kind == 7))
    {
        struct DDS_DataReaderImpl *local_reader = NULL;

        dbrc = DB_Table_select_match(self->local_reader_table,
                                     DB_TABLE_DEFAULT_INDEX,
                                     (DB_Record_T)&local_reader,
                                     (DB_Key_T)&object_id);
        if (dbrc != DB_RETCODE_OK)
        {
            goto done;
        }

        retval = DDS_DataReader_as_entity(local_reader);
    }

done:

    if (DB_Database_unlock(self->database) != DB_RETCODE_OK)
    {
    }

    return retval;
}

/*ci
 * \brief Scan existing DataWriters and count which should be announced
 * by discovery, and which shouldn't.
 *
 * \param[in] self              The DomainParticipant
 * \param[in] announced_out     The total number of DataWriters to be announced
 *                              by discovery.
 * \param[in] unannounced_out   The total number of DataWriters that should not
 *                              be announced by discovery.
 * \return RTI_TRUE if the output values were computed correctly and are thus
 * valid, RTI_FALSE if an error occur and the values should be ignored.
 */
RTI_BOOL
DDS_DomainParticipant_get_announced_datawriters(
        DDS_DomainParticipant *self,
        DDS_Long *announced_out,
        DDS_Long *unannounced_out)
{
    RTI_BOOL retval = RTI_FALSE;
    struct DDS_DataWriterImpl *writer = NULL;
    DB_Cursor_T dw_cursor = NULL;
    DB_ReturnCode_T dbrc = DB_RETCODE_ERROR;
    DDS_Long announced = 0,unannounced = 0;

    OSAPI_PRECONDITION(self == NULL,
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("announced_out",announced_out,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("unannounced_out",unannounced_out,RTI_TRUE);)

    if (announced_out != NULL)
    {
        *announced_out = 0;
    }
    if (unannounced_out != NULL)
    {
        *unannounced_out = 0;
    }

    dbrc = DB_Table_select_all(self->local_writer_table,
                               DB_TABLE_DEFAULT_INDEX,&dw_cursor);

    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,self->local_writer_table,dbrc)
        goto done;
    }

    do
    {
        dbrc = DB_Cursor_get_next(dw_cursor,(DB_Record_T*)&writer);

        if (dbrc == DB_RETCODE_OK)
        {
            if (DDS_DataWriter_is_announced(writer))
            {
                announced += 1;
            }
            else
            {
                unannounced += 1;
            }
        }
    } while (dbrc == DB_RETCODE_OK);
    DB_Cursor_finish(self->local_writer_table,dw_cursor);

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    if (announced_out != NULL)
    {
        *announced_out = announced;
    }
    if (unannounced_out != NULL)
    {
        *unannounced_out = unannounced;
    }

    retval = RTI_TRUE;

done:

    return retval;
}


/*ci
 * \brief Scan existing DataReaders and count which should be announced
 * by discovery, and which shouldn't.
 *
 * \param[in] self              The DomainParticipant
 * \param[in] announced_out     The total number of DataReaders to be announced
 *                              by discovery.
 * \param[in] unannounced_out   The total number of DataReaders that should not
 *                              be announced by discovery.
 * \return RTI_TRUE if the output values were computed correctly and are thus
 * valid, RTI_FALSE if an error occur and the values should be ignored.
 */
RTI_BOOL
DDS_DomainParticipant_get_announced_datareaders(
        DDS_DomainParticipant *self,
        DDS_Long *announced_out,
        DDS_Long *unannounced_out)
{
    RTI_BOOL retval = RTI_FALSE;
    struct DDS_DataReaderImpl *reader = NULL;
    DB_Cursor_T dr_cursor = NULL;
    DB_ReturnCode_T dbrc = DB_RETCODE_ERROR;
    DDS_Long announced = 0,unannounced = 0;

    OSAPI_PRECONDITION(self == NULL,
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("announced_out",announced_out,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("unannounced_out",unannounced_out,RTI_TRUE);)

    if (announced_out != NULL)
    {
        *announced_out = 0;
    }
    if (unannounced_out != NULL)
    {
        *unannounced_out = 0;
    }

    dbrc = DB_Table_select_all(self->local_reader_table,
                               DB_TABLE_DEFAULT_INDEX,&dr_cursor);

    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,self->local_reader_table,dbrc)
        goto done;
    }

    do
    {
        dbrc = DB_Cursor_get_next(dr_cursor,(DB_Record_T*)&reader);

        if (dbrc == DB_RETCODE_OK)
        {
            if (DDS_DataReader_is_announced(reader))
            {
                announced += 1;
            }
            else
            {
                unannounced += 1;
            }
        }
    } while (dbrc == DB_RETCODE_OK);
    DB_Cursor_finish(self->local_reader_table,dr_cursor);

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    if (announced_out != NULL)
    {
        *announced_out = announced;
    }
    if (unannounced_out != NULL)
    {
        *unannounced_out = unannounced;
    }

    retval = RTI_TRUE;

done:

    return retval;
}


RTI_UINT32
DDS_DomainParticipant_get_next_objectid(void *self)
{
    DDS_DomainParticipant *participant = (DDS_DomainParticipant*)self;

    OSAPI_PRECONDITION(self == NULL,
                       return 0,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE));

     ++participant->current_object_id;

    /* Object-ids are not allowed to wrap around. A maximum of
     * (OSAPI_SYSTEM_OBJECTID_MAX - OSAPI_SYSTEM_OBJECTID_START) objects
     * can be created.
     */
    if (participant->current_object_id == OSAPI_SYSTEM_OBJECTID_MAX)
    {
        /* Make sure that if the limit has been reached always return -1
         * for every subsequent call (because object-ids are never returned)
         */
        --participant->current_object_id;

        DDSC_LOG_GET_NEXT_OBJECT_ID(OSAPI_LOGKIND_ERROR)
        return 0;
    }

    return participant->current_object_id;
}

DDS_StringManager_T*
DDS_DomainParticipant_get_partition_string_manager(DDS_DomainParticipant *self)
{
    struct DDS_DomainParticipantImpl *dp =
            (struct DDS_DomainParticipantImpl *)self;

    OSAPI_PRECONDITION(self == NULL,
                        return NULL,
                        OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE););

    return dp->partition_string_manager;
}


RTI_BOOL
DDS_DomainParticipant_is_mtu_greater_than_builtindata(
        DDS_DomainParticipant *const participant,
        DDS_DataWriter *dw,
        struct DDS_ParticipantBuiltinTopicData *local_participant_data)
{
    struct CDR_Stream_t stream;
    RTI_UINT32 true_size = 0;
    struct DDS_TypePluginBuffer *tbuf = NULL;
    RTI_BOOL retval = RTI_FALSE;
    RTI_INT32 min_mtu;
    DDS_TypePlugin *dw_typeplugin = NULL;
    char *buf = NULL;
    RTI_UINT32 buf_len;

    CDR_Stream_initialize(&stream);
    dw_typeplugin = DDS_DataWriter_get_type_plugin(dw);
    if (dw_typeplugin == NULL)
    {
        return RTI_FALSE;
    }
    tbuf = DDS_TypePluginDefaultCdr_get_buffer(dw_typeplugin);
    if (tbuf == NULL)
    {
        return RTI_FALSE;
    }
    if (!DDS_TypePlugin_set_stream(dw_typeplugin, &stream, tbuf))
    {
        goto done;
    }

    /* Serialize empty encapsulation header before sample to match DWI */
    if (!CDR_Stream_serialize_header(&stream, 0, 0))
    {
        goto done;
    }
    if (!DDS_TypePlugin_serialize_sample(dw_typeplugin, &stream,
                                    local_participant_data, NULL))
    {
        goto done;
    }
    true_size = CDR_Stream_get_current_position_offset(&stream) +
                RTPS_PACKET_HEADER_MAX_LENGTH + UDP_PACKET_HEADER_MAX_LENGTH +
                RTPS_PACKET_TRAILER_MAX_LENGTH + UDP_PACKET_TRAILER_MAX_LENGTH;
    min_mtu = NETIO_BindResolver_get_min_mtu_by_kind(
                participant->bind_resolver, NETIO_ROUTEKIND_META);
    if (min_mtu < 0)
    {
        DDSC_LOG_GET_MTU(OSAPI_LOGKIND_ERROR)
        goto done;
    }
    if (true_size > (RTI_UINT32)min_mtu)
    {
        DDSC_LOG_MIN_DISCOVERY_MTU(OSAPI_LOGKIND_ERROR, true_size, min_mtu)
        goto done;
    }

    retval = RTI_TRUE;

done:
    buf = CDR_Stream_get_buffer(&stream, &buf_len);
    if (buf != NULL)
    {
        /* Re-initialize the buffer so that it is
         * in the state expected by the DWI.
         */
        OSAPI_Memory_zero(buf, buf_len);
    }
    DDS_TypePluginDefaultCdr_return_buffer(dw_typeplugin, tbuf);
    return retval;
}

DDS_Boolean
DDS_DomainParticipant_is_discovery_by_name_enabled(
                        const DDS_DomainParticipant *const participant)
{
    return participant->qos.discovery.enable_participant_discovery_by_name;
}

/*ci @} */
