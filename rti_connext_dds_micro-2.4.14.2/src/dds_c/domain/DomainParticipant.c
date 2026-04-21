/*
 * FILE: DomainParticipant.c - DomainParticipant implementation
 *
 * (c) Copyright, Real-Time Innovations, 2008-2020.
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
 * 31may2022,jh MICRO-3381/PR.30046
 * - Conditionally marked factory_lock in DDS_DomainParticipant_initialize()
 *   as unused when compiling with RTI_CERT
 * - Conditionally included assignment of shared_lock when compiling without
 *   RTI_CERT in DDS_DomainParticipant_initialize()
 * 16mar2022,am MICRO-3519/PR.30311
 * - Excluded netio_udp.h when builtin UDP is excluded from the build. 
 * 01mar2022,jh MICRO-3381/PR.30046
 * - Protected database access in DDS_DomainParticipant_enable
 *   with the database lock
 * - Use database lock in get_qos and set_qos function (not related to cert)
 * - Move the check for qos immutability for set_qos after taking database lock
     (not related to cert).
 * 16feb2022,am MICRO-3455
 * - Changes to support excluding UDP on Deos platform.
 *   - Set DDS_PARTICIPANT_DEFAULT_TRANSPORT_LENGTH to 1.
 *   - Removed automatic registration of UDP transport.
 *   - Removed UDP from default discovery and user traffic transports.
 * 15feb2022,tk MICRO-3455
 * - Added OSAPI_CC_STRINGIFY_DEFINE macro to workaround incorrect
 *   macro expansion on Deos
 * 13dec2021,tk MICRO-3368/PR.30028
 * - Removed redundant test on (participant != NULL) at the end of
 *   DDS_DomainParticipant_initialize when checking for successful
 *   initialization since participant must be != NULL.
 * 17sep2021,tk MICRO-3276/PR.29664
 * - Added transport sequence in call to NETIO_BindResolver_reserve_addresses
 * 13sep2021,tk MICRO-3198/PR.29540
 * - Use Preprocessor conditions instead of C conditions when testing for
 *   range for RTIME_DDS_VERSION_REVISION and RTIME_DDS_VERSION_RELEASE in
 *   DDS_DomainParticipant_initialize to avoid unreachable code.
 * - Check that the individual version numbers are in legal range:
 *   RTIME_DDS_VERSION_MAJOR - must be [0,9]
 *   RTIME_DDS_VERSION_MINOR - must be [0,9]
 *   RTIME_DDS_VERSION_REVISION - must [0,99]
 *   RTIME_DDS_VERSION_RELEASE - must [0,99]
 *  If any of the ranges are exceeded, DDS_DomainParticipant_initialize must
 *  be updated to support the increased range.
 * 06jun2021,tk MICRO-3054/PR.28950
 * - Removed "_udp://239.255.0.1" and "_udp://127.0.0.1" from the defaults
 *   for discovery in DDS_DomainParticipant_initialize()
 * - Removed "_udp://127.0.0.1" from the defaults
 *   for user_traffic in DDS_DomainParticipant_initialize()
 * 16apr2021,tk MICRO-2845/PR.28682
 *  - Removed empty statements (LOG statements ending in ;)
 * 13apr2021,tk MICRO-3028/PR.28960
 * - Removed inclusion of "TopicQos.h" for CERT
 * 04apr2021,tk MICRO-2845/PR.28682
 *  - Removed empty blocks in DDS_DomainParticipant_initialize()
 * 07apr2021.tk MICRO-3017/PR.29043
 * - Corrected function comment for DDS_DomainParticipant_get_next_objectid
 * 06apr2021,tk MICRO-3003/PR.29038
 * - Removed redundant #ifdef RTI_CERT in DDS_DomainParticipant_finalize()
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
 * 31jul2014,tk MICRO-842/PR#9683  Removed superfluous paramaters in DB
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
#if !UDP_EXCLUDE_BUILTIN
#ifndef netio_udp_h
#include "netio/netio_udp.h"
#endif
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
#ifndef RTI_CERT
#ifndef dds_c_string_manager_h
#include "dds_c/dds_c_string_manager.h"
#endif
#endif

#include "Entity.h"
#include "Conditions.h"
#include "QosPolicy.h"
#include "TopicDescription.h"
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

#include "DomainParticipant.h"
#include "DomainParticipantChecksum.h"

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
RTI_PRIVATE const char *const DDS_TOPIC_TABLE_NAME = "topic";

/*ci
 * \brief The DB table used for managing types
 */
RTI_PRIVATE const char *const DDS_TYPE_TABLE_NAME = "type";

/*ci
 * \brief The DB table used for managing datareaders
 */
RTI_PRIVATE const char *const DDS_DATAREADER_TABLE_NAME = "datareader";

/*ci
 * \brief The DB table used for managing datawriters
 */
RTI_PRIVATE const char *const DDS_DATAWRITER_TABLE_NAME = "datawriter";

/*ci
 * \brief The DB table used for managing publications
 */
RTI_PRIVATE const char *const DDS_PUBLICATION_TABLE_NAME = "publication";

/*ci
 * \brief The DB table used for managing subscriptions
 */
RTI_PRIVATE const char *const DDS_SUBSCRIPTION_TABLE_NAME = "subscription";

/*ci
 * \brief The DB table used for managing remote participants
 */
RTI_PRIVATE const char *const DDS_PARTICIPANT_TABLE_NAME = "participant";

/*ci
 * \brief The DB table used for managing publishers
 */
RTI_PRIVATE const char *const DDS_PUBLISHER_TABLE_NAME = "publisher";

/*ci
 * \brief The DB table used for managing subscribers
 */
RTI_PRIVATE const char *const DDS_SUBSCRIBER_TABLE_NAME = "subscriber";

#ifndef RTI_CERT
/*ci
 * \brief The default participant name if the participant Qos policy does not
 *        contain one.
 */
RTI_PRIVATE const char *const DDS_DEFAULT_PARTICIPANT_NAME = "_participant_";
#endif

/*ci
 * \brief The number of tables needed by the participant
 */
#define DDS_PARTICIPANT_TABLES              (9)

#if UDP_EXCLUDE_BUILTIN
/*ci
 * \brief The number of default transports registered when the builtin UDP transport is excluded. 
 */
#define DDS_PARTICIPANT_DEFAULT_TRANSPORT_LENGTH (1)
#else
/*ci
 * \brief The number of default transports registered when the builtin UDP transport is included. 
 */
#define DDS_PARTICIPANT_DEFAULT_TRANSPORT_LENGTH (2)
#endif

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

#ifndef RTI_CERT
/*ci
 * \brief Initialize a DDS_SubscriptionBuiltinTopicData record
 *
 * \param[in] initialize_param User defined parameter
 * \param[in] buffer Record to initialize
 *
 * \return evaluates to logical true on success, false on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DomainParticipant_remote_subscriber_entry_initialize(
    void *initialize_param, void *buffer)
{
    RTI_BOOL result = RTI_FALSE;
    struct DDS_SubscriptionBuiltinTopicData init_val =
                        DDS_SubscriptionBuiltinTopicData_INITIALIZER;
    struct DDS_RemoteSubscriptionImpl *rem_pub = NULL;
    struct DDS_SubscriptionBuiltinTopicData *data = NULL;
    UNUSED_ARG(initialize_param);
    
    if (buffer == NULL)
    {
        goto done;
    }
    OSAPI_Memory_zero(buffer,sizeof(struct DDS_RemoteSubscriptionImpl));
    rem_pub = (struct DDS_RemoteSubscriptionImpl*)buffer;
    rem_pub->data = init_val;
    data = &rem_pub->data;
    
    if (!DDS_LocatorSeq_initialize(&data->unicast_locator))
    {
        goto done;
    }
    if (!DDS_LocatorSeq_set_maximum(&data->unicast_locator,
                                    RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX))
    {
        DDSC_LOG_SEQ_SETMAX(OSAPI_LOGKIND_ERROR,
                            DDSC_LOG_USERUNICAST_SEQUENCE,
                            RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX)
        goto done;
    }
    
    if (!DDS_LocatorSeq_initialize(&data->multicast_locator))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_set_maximum(&data->multicast_locator,
                                    RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX))
    {
        DDSC_LOG_SEQ_SETMAX(OSAPI_LOGKIND_ERROR,
                            DDSC_LOG_USERMULTICAST_SEQUENCE,
                            RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX)
        goto done;
    }

    result = RTI_TRUE;
done:
    return result;
    
}

/*ci
 * \brief Finalize a DDS_SubscriptionBuiltinTopicData record
 *
 * \param[in] initialize_param User defined parameter
 * \param[in] buffer Record to finalize
 *
 * \return evaluates to logical true on success, false on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DomainParticipant_remote_subscriber_entry_finalize(
    void *initialize_param, void *buffer)
{
    RTI_BOOL result = RTI_FALSE;
    struct DDS_RemoteSubscriptionImpl *rem_sub = NULL;
    struct DDS_SubscriptionBuiltinTopicData *data = NULL;
    UNUSED_ARG(initialize_param);
    if (buffer == NULL)
    {
        goto done;
    }
    rem_sub = (struct DDS_RemoteSubscriptionImpl*)buffer;
    data = &rem_sub->data;
    if (!DDS_LocatorSeq_finalize(&data->unicast_locator))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_finalize(&data->multicast_locator))
    {
        goto done;
    }

    result = RTI_TRUE;
done:
    return result;
}

/*ci
 * \brief Initialize a DDS_PublicationBuiltinTopicData record
 *
 * \param[in] initialize_param User defined parameter
 * \param[in] buffer Record to initialize
 *
 * \return evaluates to logical true on success, false on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DomainParticipant_remote_publisher_entry_initialize(
    void *initialize_param, void *buffer)
{
    RTI_BOOL result = RTI_FALSE;
    struct DDS_PublicationBuiltinTopicData init_val =
                        DDS_PublicationBuiltinTopicData_INITIALIZER;
    struct DDS_RemotePublicationImpl *rem_pub = NULL;
    struct DDS_PublicationBuiltinTopicData *data = NULL;
    UNUSED_ARG(initialize_param);
    
    if (buffer == NULL)
    {
        goto done;
    }
    OSAPI_Memory_zero(buffer,sizeof(struct DDS_RemotePublicationImpl));
    rem_pub = (struct DDS_RemotePublicationImpl*)buffer;
    rem_pub->data = init_val;
    data = &rem_pub->data;
    
    if (!DDS_LocatorSeq_initialize(&data->unicast_locator))
    {
        goto done;
    }
    if (!DDS_LocatorSeq_set_maximum(&data->unicast_locator,
                                    RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX))
    {
        DDSC_LOG_SEQ_SETMAX(OSAPI_LOGKIND_ERROR,
                            DDSC_LOG_USERUNICAST_SEQUENCE,
                            RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX)
        goto done;
    }
        
    result = RTI_TRUE;
done:
    return result;
    
}

/*ci
 * \brief Finalize a DDS_SubscriptionBuiltinTopicData record
 *
 * \param[in] initialize_param User defined parameter
 * \param[in] buffer Record to finalize
 *
 * \return evaluates to logical true on success, false on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DomainParticipant_remote_publisher_entry_finalize(
    void *initialize_param, void *buffer)
{
    RTI_BOOL result = RTI_FALSE;
    struct DDS_RemotePublicationImpl *rem_pub = NULL;
    struct DDS_PublicationBuiltinTopicData *data = NULL;
    UNUSED_ARG(initialize_param);
    
    if (buffer == NULL)
    {
        goto done;
    }
    rem_pub = (struct DDS_RemotePublicationImpl*)buffer;
    data = &rem_pub->data;
    if (!DDS_LocatorSeq_finalize(&data->unicast_locator))
    {
        goto done;
    }

    result = RTI_TRUE;
done:
return result;
}

/*ci
 * \brief Initialize a DDS_ParticipantBuiltinTopicData record
 *
 * \param[in] initialize_param User defined parameter
 * \param[in] buffer Record to initialize
 *
 * \return evaluates to logical true on success, false on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DomainParticipant_remote_participant_entry_initialize(
                    void *initialize_param, void *buffer)
{
    DDS_Boolean retval = RTI_FALSE;
    struct DDS_RemoteParticipantImpl *record = NULL;
                    
    struct DDS_ParticipantBuiltinTopicData init_val =
                        DDS_ParticipantBuiltinTopicData_INITIALIZER;
    UNUSED_ARG(initialize_param);
    if (buffer == NULL)
    {
        goto done;
    }
    OSAPI_Memory_zero(buffer,sizeof(struct DDS_RemoteParticipantImpl));
    record = (struct DDS_RemoteParticipantImpl*)buffer;
    
    record->data = init_val;

    if (!DDS_LocatorSeq_initialize(&record->data.default_unicast_locators))
    {
        goto done;
    }
    
    if (!DDS_LocatorSeq_set_maximum(
               &record->data.default_unicast_locators,
               RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX))
    {
        DDSC_LOG_SEQ_SETMAX(OSAPI_LOGKIND_ERROR,
                            DDSC_LOG_METAUNICAST_SEQUENCE,
                            RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX)
        goto done;  
    }

    if (!DDS_LocatorSeq_initialize(&record->data.default_multicast_locators))
    {
        goto done;
    }
    
    if (!DDS_LocatorSeq_set_maximum(
               &record->data.default_multicast_locators,
               RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX))
    {
        DDSC_LOG_SEQ_SETMAX(OSAPI_LOGKIND_ERROR,
                            DDSC_LOG_METAMULTICAST_SEQUENCE,
                            RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX)
        goto done;
    }

    if (!DDS_LocatorSeq_initialize(&record->data.metatraffic_unicast_locators))
    {
        goto done;
    }
    
    if (!DDS_LocatorSeq_set_maximum(
               &record->data.metatraffic_unicast_locators,
               RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX))
    {
        DDSC_LOG_SEQ_SETMAX(OSAPI_LOGKIND_ERROR,
                            DDSC_LOG_METAUNICAST_SEQUENCE,
                            RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX)
        goto done;
    }

    if (!DDS_LocatorSeq_initialize(&record->data.metatraffic_multicast_locators))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_set_maximum(
               &record->data.metatraffic_multicast_locators,
               RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX))
    {
        DDSC_LOG_SEQ_SETMAX(OSAPI_LOGKIND_ERROR,
                            DDSC_LOG_METAMULTICAST_SEQUENCE,
                            RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX)
        goto done;
    }

    retval = RTI_TRUE;  
done:
    return retval;
}

/*ci
 * \brief Finalize a DDS_RemoteParticipantImpl record
 *
 * \param[in] initialize_param User defined parameter
 * \param[in] buffer Record to finalize
 *
 * \return evaluates to logical true on success, false on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DomainParticipant_remote_participant_entry_finalize(
                    void *initialize_param, void *buffer)
{
    RTI_BOOL retval = RTI_FALSE;
    struct DDS_RemoteParticipantImpl *record = NULL;
    UNUSED_ARG(initialize_param);
    if (buffer == NULL)
    {
        goto done;
    }
    
    record = (struct DDS_RemoteParticipantImpl*)buffer;

    if (!DDS_LocatorSeq_finalize(&record->data.default_unicast_locators))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_finalize(&record->data.metatraffic_unicast_locators))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_finalize(&record->data.metatraffic_multicast_locators))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_finalize(&record->data.default_multicast_locators))
    {
        goto done;
    }
    
    retval = RTI_TRUE;
done:
    return retval;    
}
#endif /* !RTI_CERT */

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
    RTI_INT32 t_length;

    /* The maximum number of tables are calculated based on detailed knowledge
     * on the implementation. NOTE: The NETIO_RESOURCE_TABLES_PER_INTERFACE
     * is for the external RTPS interface which is not specified in the
     * enabled transport list.
     */
    t_length = REDA_StringSeq_get_length(
                            &participant->qos.transports.enabled_transports);

    /* The database is initialized before the default transports are added
     * when no are specified by the user.
     */
    if (t_length == 0)
    {
        t_length = DDS_PARTICIPANT_DEFAULT_TRANSPORT_LENGTH;
    }

    db_property.max_tables =
     DDS_PARTICIPANT_TABLES +
     RT_RESOURCE_TABLES_PER_INSTANCE +
     NETIO_RESOURCE_TABLES_PER_INTERFACE +
     (RTI_SIZE_T)NETIO_RESOURCE_COMMON_TABLES +
     ((RTI_SIZE_T)NETIO_RESOURCE_EXTERNAL_TABLE * (RTI_SIZE_T)t_length) +
         ((RTI_SIZE_T)participant->qos.resource_limits.local_reader_allocation *
          (RTI_SIZE_T)(DDS_MAX_INTERFACES_PER_DATAREADER * NETIO_RESOURCE_TABLES_PER_INTERFACE)) +
        ((RTI_SIZE_T)participant->qos.resource_limits.local_writer_allocation *
           ((RTI_SIZE_T)DDS_MAX_INTERFACES_PER_DATAWRITER * (RTI_SIZE_T)NETIO_RESOURCE_TABLES_PER_INTERFACE));

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
    dbrc = DB_Database_create_table(&participant->local_writer_table,
                                    participant->database,
                                    DDS_DATAWRITER_TABLE_NAME,
                                    sizeof(struct DDS_DataWriterImpl),
                                    DDS_DataWriterImpl_compare,&tbl_prop);
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
#ifndef RTI_CERT
    dbrc = DB_Database_create_table_w_ctor_and_dtor(
                            &participant->remote_participant_table,
                            participant->database,
                            DDS_PARTICIPANT_TABLE_NAME,
                            (RTI_SIZE_T)sizeof(struct DDS_RemoteParticipantImpl),
                            DDS_RemoteParticipantImpl_compare,
                            &tbl_prop,
                            DDS_DomainParticipant_remote_participant_entry_initialize,
                            NULL,
                            DDS_DomainParticipant_remote_participant_entry_finalize,
                            NULL);
#else
    dbrc = DB_Database_create_table(&participant->remote_participant_table,
                                    participant->database,
                                    DDS_PARTICIPANT_TABLE_NAME,
                                    sizeof(struct DDS_RemoteParticipantImpl),
                                    DDS_RemoteParticipantImpl_compare,&tbl_prop);
#endif

    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_CREATE(OSAPI_LOGKIND_ERROR,DDS_PARTICIPANT_TABLE_NAME,dbrc)
        ddsrc = DDS_RETCODE_ERROR;
        goto done;
    }

    tbl_prop.max_records = (RTI_SIZE_T)participant->qos.resource_limits.remote_reader_allocation;
#ifndef RTI_CERT
    dbrc = DB_Database_create_table_w_ctor_and_dtor(
                            &participant->remote_subscriber_table,
                            participant->database,
                            DDS_SUBSCRIPTION_TABLE_NAME,
                            (RTI_SIZE_T)sizeof(struct DDS_RemoteSubscriptionImpl),
                            DDS_RemoteSubscriptionImpl_compare,
                            &tbl_prop,
                            DDS_DomainParticipant_remote_subscriber_entry_initialize,
                            NULL,
                            DDS_DomainParticipant_remote_subscriber_entry_finalize,
                            NULL);
#else
    dbrc = DB_Database_create_table(&participant->remote_subscriber_table,
                                    participant->database,
                                    DDS_SUBSCRIPTION_TABLE_NAME,
                                    sizeof(struct DDS_RemoteSubscriptionImpl),
                                    DDS_RemoteSubscriptionImpl_compare,&tbl_prop);
#endif

    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_CREATE(OSAPI_LOGKIND_ERROR,DDS_SUBSCRIPTION_TABLE_NAME,dbrc)
        ddsrc = DDS_RETCODE_ERROR;
        goto done;
    }

    tbl_prop.max_records = (RTI_SIZE_T)participant->qos.resource_limits.remote_writer_allocation;
#ifndef RTI_CERT
    dbrc = DB_Database_create_table_w_ctor_and_dtor(
                            &participant->remote_publisher_table,
                            participant->database,
                            DDS_PUBLICATION_TABLE_NAME,
                            (RTI_SIZE_T)sizeof(struct DDS_RemotePublicationImpl),
                            DDS_RemotePublicationImpl_compare,
                            &tbl_prop,
                            DDS_DomainParticipant_remote_publisher_entry_initialize,
                            NULL,
                            DDS_DomainParticipant_remote_publisher_entry_finalize,
                            NULL);
#else
    dbrc = DB_Database_create_table(&participant->remote_publisher_table,
                                    participant->database,
                                    DDS_PUBLICATION_TABLE_NAME,
                                    sizeof(struct DDS_RemotePublicationImpl),
                                    DDS_RemotePublicationImpl_compare,
                                    &tbl_prop);
#endif

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
    struct DDS_BuiltinTopicKey_t key;
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

    if (!DDS_LocatorSeq_initialize(&data->metatraffic_unicast_locators))
    {
        DDSC_LOG_SEQ_INITIALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_METAUNICAST_SEQUENCE)
        return DDS_RETCODE_ERROR;
    }

    if (!DDS_LocatorSeq_initialize(&data->metatraffic_multicast_locators))
    {
        DDSC_LOG_SEQ_INITIALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_METAMULTICAST_SEQUENCE)
        return DDS_RETCODE_ERROR;
    }

    if (!DDS_LocatorSeq_initialize(&data->default_unicast_locators))
    {
        DDSC_LOG_SEQ_INITIALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_USERUNICAST_SEQUENCE)
        return DDS_RETCODE_ERROR;
    }

    if (!DDS_LocatorSeq_initialize(&data->default_multicast_locators))
    {
        DDSC_LOG_SEQ_INITIALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_USERMULTICAST_SEQUENCE)
        return DDS_RETCODE_ERROR;
    }

    if (!DDS_LocatorSeq_set_maximum(&data->metatraffic_multicast_locators,
                                    RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX))
    {
        DDSC_LOG_SEQ_SETMAX(OSAPI_LOGKIND_ERROR,
                            DDSC_LOG_METAMULTICAST_SEQUENCE,
                            RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX)
        return DDS_RETCODE_ERROR;
    }

    if (!DDS_LocatorSeq_set_maximum(&data->metatraffic_unicast_locators,
                                    RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX))
    {
        DDSC_LOG_SEQ_SETMAX(OSAPI_LOGKIND_ERROR,
                            DDSC_LOG_METAUNICAST_SEQUENCE,
                            RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX)
        return DDS_RETCODE_ERROR;
    }

    if (!DDS_LocatorSeq_set_maximum(&data->default_multicast_locators,
                                    RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX))
    {
        DDSC_LOG_SEQ_SETMAX(OSAPI_LOGKIND_ERROR,
                            DDSC_LOG_USERMULTICAST_SEQUENCE,
                            RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX)
        return DDS_RETCODE_ERROR;
    }

    if (!DDS_LocatorSeq_set_maximum(&data->default_unicast_locators,
                                    RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX))
    {
        DDSC_LOG_SEQ_SETMAX(OSAPI_LOGKIND_ERROR,
                            DDSC_LOG_USERUNICAST_SEQUENCE,
                            RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX)
        return DDS_RETCODE_ERROR;
    }

    data->dds_builtin_endpoints = 0;

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
    DDS_InstanceHandle_t retval;

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
                           return DDS_RETCODE_PRECONDITION_NOT_MET,
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

    if (participant->disc_plugin != NULL)
    {
        if (!NDDS_Discovery_Plugin_on_after_local_participant_enabled(
                participant->disc_plugin,
                participant,
                &participant->builtin_data))
        {
            DDSC_LOG_DISC_LOCAL_PARTICIPANT_ENABLED(OSAPI_LOGKIND_ERROR,
                            RT_ComponentFactoryId_get_name(
                                &participant->qos.discovery.discovery.name))
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
            topic->as_entity.enable(&topic->as_entity);
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
            local_subscriber->as_entity.enable(&local_subscriber->as_entity);
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
            local_publisher->as_entity.enable(&local_publisher->as_entity);
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
   NETIO_Interface_T *netio_intf;

   if (participant->database != NULL)
   {
       if (DB_Database_lock(participant->database) != DB_RETCODE_OK)
       {
           return DDS_RETCODE_ERROR;
       }
   }

   key = NDDS_DomainParticipant_get_key(self);

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
           }
       } while (dbrc == DB_RETCODE_OK);

       DB_Cursor_finish(participant->remote_participant_table,cursor);

       if (dbrc != DB_RETCODE_NO_DATA)
       {
           DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
           goto done;
       }
   }

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


   /* Deleting a DP follows the following rules
    * 1. All local publisher must be deleted
    * 2. All local subscribers must be deleted
    * 3. The built-in readers must be deleted
    * 4. All threads (Sender/Receiver) must be deleted
    * 5. The Event thread must be deleted
    * 6. Generator and Interpreter must be deleted
    * 7. The OSAPI must be deleted
    */
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
                                                       self,&rem_pub->data))
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
                                                       self,&rem_sub->data))
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
               if (!NDDS_RemoteParticipantRecord_finalize(rem_participant,NULL))
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

    if(participant->string_manager != NULL)
    {
        if (!DDS_StringManager_delete(participant->string_manager))
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

   /* It is safe to unlock the data base here. There are no shared resources
    * between the DDS and the transports, except the database. Since the
    * database is shared, avoid a deadlock as DDS sends messages to itself to
    * allow receive threads to unblock.
    */
   if ((participant->database != NULL) &&
       DB_Database_unlock(participant->database) != DB_RETCODE_OK)
   {
       return DDS_RETCODE_ERROR;
   }

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

   if ((participant->database != NULL) &&
        DB_Database_lock(participant->database) != DB_RETCODE_OK)
   {
       return DDS_RETCODE_ERROR;
   }

   for (i = 0;
        i < DDS_StringSeq_get_length(&participant->qos.transports.enabled_transports);
        ++i)
   {
       RT_ComponentFactoryId_T id;
       const char *id_name;

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

       if (!NETIO_AddressSeq_set_length(&participant->routes, 0))
       {
           DDSC_LOG_SEQ_SETLENGTH(OSAPI_LOGKIND_ERROR,DDSC_LOG_ROUTE_SEQUENCE,0)
           goto done;
       }

       if (!NETIO_NetmaskSeq_set_length(&participant->netmasks, 0))
       {
           DDSC_LOG_SEQ_SETLENGTH(OSAPI_LOGKIND_ERROR,DDSC_LOG_NETMASK_SEQUENCE,0)
           goto done;
       }

       /* Rely on error code set by get route table function */
       if (!NETIO_Interface_get_route_table(netio_intf,
                &participant->routes, &participant->netmasks))
       {
           DDSC_LOG_NETIO_GET_ROUTE_TABLE_FAILED(OSAPI_LOGKIND_ERROR,id_name)
           goto done;
       }

       for (j = 0; j < NETIO_AddressSeq_get_length(&participant->routes); ++j)
       {
            struct NETIO_Address *addr = NULL;
            struct NETIO_Netmask *mask = NULL;

            addr = NETIO_AddressSeq_get_reference(&participant->routes, j);
            if (addr == NULL)
            {
                DDSC_LOG_SEQ_GETREF(OSAPI_LOGKIND_ERROR,DDSC_LOG_ROUTE_SEQUENCE,j)
                goto done;
            }

            mask = NETIO_NetmaskSeq_get_reference(&participant->netmasks, j);
            if (mask == NULL)
            {
                DDSC_LOG_SEQ_GETREF(OSAPI_LOGKIND_ERROR,DDSC_LOG_NETMASK_SEQUENCE,j)
                goto done;
            }

            /* Rely on error code set by delete_interface */
            if (!NETIO_RouteResolver_delete_interface(participant->route_resolver,
                    netio_intf,
                    addr,
                    mask,
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

   if (participant->address_resolver != NULL)
   {
       if (!NETIO_AddressResolver_delete(participant->address_resolver))
       {
           DDSC_LOG_OBJECT_DELETE(OSAPI_LOGKIND_ERROR,
                                  DDSC_LOG_ADDRESSRESOLVER_OBJECT)
           goto done;
       }
       participant->address_resolver = NULL;
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

   if (DDS_DomainParticipantQos_finalize(&self->qos) != DDS_RETCODE_OK)
   {
       DDSC_LOG_OBJECT_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANTQOS_OBJECT)
       goto done;
   }

   if (DDS_PublisherQos_finalize(&self->_default_publisher_qos) != DDS_RETCODE_OK)
   {
       DDSC_LOG_OBJECT_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLISHERQOS_OBJECT)
       goto done;
   }

   if (DDS_SubscriberQos_finalize(&self->_default_subscriber_qos) != DDS_RETCODE_OK)
   {
       DDSC_LOG_OBJECT_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIBERQOS_OBJECT)
       goto done;
   }

   if (DDS_TopicQos_finalize(&self->_default_topic_qos) != DDS_RETCODE_OK)
   {
       DDSC_LOG_OBJECT_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_TOPICQOS_OBJECT)
       goto done;
   }

   if (!DDS_ParticipantBuiltinTopicData_finalize(&self->builtin_data))
   {
       DDSC_LOG_OBJECT_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANTDATA_OBJECT)
       goto done;
   }

    if (participant->database != NULL)
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

#ifndef RTI_CERT
/*ci
 * \brief Return the maximum number of strings required by the participant
 * 
 * \return The maximum number of strings.
 */
RTI_PRIVATE RTI_SIZE_T
DDS_DomainParticipant_get_max_string(DDS_DomainParticipant *participant)
{
    return (RTI_SIZE_T)(participant->qos.resource_limits.local_topic_allocation +
                        participant->qos.resource_limits.local_type_allocation +
                        participant->qos.resource_limits.remote_reader_allocation +
                        participant->qos.resource_limits.remote_writer_allocation);
}
#endif

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
#if !INCLUDE_API_QOS
    struct DDS_PublisherQos PUB_QOS_INIT = DDS_PublisherQos_INITIALIZER;
    struct DDS_SubscriberQos SUB_QOS_INIT = DDS_SubscriberQos_INITIALIZER;
    struct DDS_TopicQos TOPIC_QOS_INIT = DDS_TopicQos_INITIALIZER;
#endif
    struct OSAPI_TimerProperty timer_property = OSAPI_TimerProperty_INITIALIZER;
    struct RT_ComponentFactory *c_factory;
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
    struct RTPS_InterfaceProperty rtps_property = RTPS_InterfaceProperty_INITIALIZER;
    struct NETIO_AddressResolverProperty nar_property =
                                     NETIO_AddressResolverProperty_INITIALIZER;
#ifndef RTI_CERT
    struct DDS_StringManagerProperty str_man_prop = DDS_StringManagerProperty_INITIALIZER;
#endif
#if !DDS_DISABLE_PARTICIPANT_INFO
    struct DDS_Property *a_property_qos;
#endif
    DDS_InstanceHandle_t instance_handle;
    struct DDS_DomainParticipantListener nil_listener =
        DDS_DomainParticipantListener_INITIALIZER;
#ifndef RTI_CERT
    RTI_SIZE_T def_dp_name_len = OSAPI_String_length(DDS_DEFAULT_PARTICIPANT_NAME);
#endif
    RTI_UINT32 v_idx = 0;

#ifdef RTI_CERT
    UNUSED_ARG(factory_lock);
#endif

    OSAPI_Memory_zero(participant,sizeof(struct DDS_DomainParticipantImpl));

    /* A precondition check has already been performed by the calling routine */
    if (factory == NULL || qos == NULL)
    {
        goto done;
    }

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
    participant->database = NULL;
    participant->qos = DDS_PARTICIPANT_QOS_DEFAULT;
#ifndef RTI_CERT
    participant->shared_lock = factory_lock;
#endif

#if INCLUDE_API_QOS
    if (qos == &DDS_PARTICIPANT_QOS_DEFAULT)
    {
        if (DDS_DomainParticipantFactory_get_default_participant_qos(
                                factory,&participant->qos) != DDS_RETCODE_OK)
        {
            DDSC_LOG_QOS_GET(OSAPI_LOGKIND_ERROR,DDSC_LOG_DEFAULTPARTICIPANT_QOS)
            goto done;
        }
    }
    else
#endif
    {
        if (DDS_DomainParticipantQos_copy(&participant->qos,qos) != DDS_RETCODE_OK)
        {
            DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_QOS)
            goto done;
        }
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

#if INCLUDE_API_QOS
    rc = DDS_PublisherQos_initialize(&participant->_default_publisher_qos);
    if (rc != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_INITIALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLISHER_QOS,rc)
        goto done;
    }

    rc = DDS_SubscriberQos_initialize(&participant->_default_subscriber_qos);
    if (rc != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_INITIALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIBER_QOS,rc)
        goto done;
    }

    rc = DDS_TopicQos_initialize(&participant->_default_topic_qos);
    if (rc != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_INITIALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_TOPIC_QOS,rc)
        goto done;
    }
#else
    participant->_default_publisher_qos = PUB_QOS_INIT;
    participant->_default_subscriber_qos = SUB_QOS_INIT;
    participant->_default_topic_qos = TOPIC_QOS_INIT;
#endif

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
                                &participant->qos.discovery.discovery.name))
            goto done;
        }

        participant->disc_plugin = DiscoveryComponentFactory_create_component(
                      c_factory,&disc_property._parent,&disc_listener._parent);

        if (participant->disc_plugin == NULL)
        {
            DDSC_LOG_COMPONENT_CREATE(OSAPI_LOGKIND_ERROR,
                      DDSC_LOG_DISCOVERY_COMPONENT,
                          RT_ComponentFactoryId_get_name(
                                  &participant->qos.discovery.discovery.name))
            goto done;
        }
    }

    /* Initialize participant announcement data
     * The participant announcement relies on data collected from
     * the discovery plug-in(s)
     */
    if (!DDS_ParticipantBuiltinTopicData_initialize(&participant->builtin_data))
    {
        DDSC_LOG_OBJECT_INITIALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANTDATA_OBJECT)
        goto done;
    }

    rc = DDS_DomainParticipant_initialize_builtin_data(
                                    participant,&participant->builtin_data);
    if (rc != DDS_RETCODE_OK)
    {
        DDSC_LOG_OBJECT_INITIALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANTDATA_OBJECT)
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
                                   &participant->qos.discovery.discovery.name))
            goto done;
        }
    }

    /* This calculation is based on detailed knowledge about the implementation:
     * 1 per remote participant
     * 3 per datawriter (deadline, liveliness, HB (RTPS))
     * 3 per datareader (deadline, liveliness, ACKNACK (RTPS))
     * 1 for the discovery plugin
     */
    timer_property.max_entries = participant->qos.resource_limits.remote_participant_allocation +
                (3*participant->qos.resource_limits.local_writer_allocation) +
                (3*participant->qos.resource_limits.local_reader_allocation) +
                1;

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

#ifndef RTI_CERT
    str_man_prop.database = participant->database;
    str_man_prop.max_strings = DDS_DomainParticipant_get_max_string(participant);
    
    if (!DDS_StringManager_create(&participant->string_manager,&str_man_prop))
    {
        DDSC_LOG_OBJECT_CREATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_STRINGMANAGER_OBJECT)
        goto done;
    }
#endif

    /* Add the default transports, unless the user has already specified some
     */
    if (DDS_StringSeq_get_length(&participant->qos.transports.enabled_transports) == 0)
    {
        if (!DDS_StringSeq_set_maximum(&participant->qos.transports.enabled_transports,
                                       DDS_PARTICIPANT_DEFAULT_TRANSPORT_LENGTH))
        {
            DDSC_LOG_SEQ_SETMAX(OSAPI_LOGKIND_ERROR,
                                DDSC_LOG_ENABLED_TRANSPORT_SEQUENCE,DDS_PARTICIPANT_DEFAULT_TRANSPORT_LENGTH)
            goto done;
        }
        if (!DDS_StringSeq_set_length(&participant->qos.transports.enabled_transports,
                                      DDS_PARTICIPANT_DEFAULT_TRANSPORT_LENGTH))
        {
            DDSC_LOG_SEQ_SETLENGTH(OSAPI_LOGKIND_ERROR,
                                   DDSC_LOG_ENABLED_TRANSPORT_SEQUENCE,DDS_PARTICIPANT_DEFAULT_TRANSPORT_LENGTH)
            goto done;
        }
      
        *DDS_StringSeq_get_reference(
                &participant->qos.transports.enabled_transports,0) =
                                       DDS_String_dup(NETIO_DEFAULT_INTRA_NAME);
#if !UDP_EXCLUDE_BUILTIN  
        *DDS_StringSeq_get_reference(
                &participant->qos.transports.enabled_transports,1) =
                                       DDS_String_dup(NETIO_DEFAULT_UDP_NAME);
#endif                                       
    }

    /*
     * Resource tables
     */
    nar_property.max_interfaces = (RTI_SIZE_T)DDS_StringSeq_get_length(
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
    if (!NETIO_AddressSeq_set_maximum(&participant->routes, (RTI_INT32)rte_prop.max_routes))
    {
        DDSC_LOG_SEQ_SETMAX(OSAPI_LOGKIND_ERROR,
                            DDSC_LOG_ROUTE_SEQUENCE,rte_prop.max_routes)
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
        else
        {
            intf_property._parent.db = participant->database;
            intf_property.max_binds = (RTI_SIZE_T)participant->qos.resource_limits.max_receive_ports;
            intf_property.max_routes = (RTI_SIZE_T)participant->qos.resource_limits.max_destination_ports;
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

        if (!NETIO_AddressSeq_set_length(&participant->routes, 0))
        {
            DDSC_LOG_SEQ_SETLENGTH(OSAPI_LOGKIND_ERROR,DDSC_LOG_ROUTE_SEQUENCE,0)
            goto done;
        }

        if (!NETIO_NetmaskSeq_set_length(&participant->netmasks, 0))
        {
            DDSC_LOG_SEQ_SETLENGTH(OSAPI_LOGKIND_ERROR,DDSC_LOG_NETMASK_SEQUENCE,0)
            goto done;
        }

        if (!NETIO_Interface_get_route_table(netio_intf,
                &participant->routes, &participant->netmasks))
        {
            DDSC_LOG_NETIO_GET_ROUTE_TABLE_FAILED(OSAPI_LOGKIND_ERROR,
                                                  id._name._name)
            goto done;
        }

        for (j = 0; j < NETIO_AddressSeq_get_length(&participant->routes); ++j)
        {
            if (!NETIO_RouteResolver_add_interface(participant->route_resolver,
                    netio_intf,
                    NETIO_AddressSeq_get_reference(&participant->routes,j),
                    NETIO_NetmaskSeq_get_reference(&participant->netmasks,j),
                    NULL))
            {
                goto done;
            }
        }
    }

    /* These sequences are used later for finalize, here we just set the length to zero
     * to indicate that they have no use outside of the scope of init and finalize functions
     */
    if (!NETIO_AddressSeq_set_length(&participant->routes, 0))
    {
        DDSC_LOG_SEQ_SETLENGTH(OSAPI_LOGKIND_ERROR,DDSC_LOG_ROUTE_SEQUENCE, 0)
        goto done;
    }

    if (!NETIO_NetmaskSeq_set_length(&participant->netmasks, 0))
    {
        DDSC_LOG_SEQ_SETLENGTH(OSAPI_LOGKIND_ERROR,DDSC_LOG_NETMASK_SEQUENCE, 0)
        goto done;
    }

#if !UDP_EXCLUDE_BUILTIN
    /* Add the default discovery transports unless the user has specified a list */
    if (DDS_StringSeq_get_maximum(&qos->discovery.enabled_transports) == 0)
    {
        if (!DDS_StringSeq_set_maximum(&participant->qos.discovery.enabled_transports,1))
        {
            DDSC_LOG_SEQ_SETMAX(OSAPI_LOGKIND_ERROR,
                              DDSC_LOG_ENABLED_DISCVOERY_TRANSPORT_SEQUENCE,1)
            goto done;
        }

        if (!DDS_StringSeq_set_length(&participant->qos.discovery.enabled_transports,1))
        {
            DDSC_LOG_SEQ_SETLENGTH(OSAPI_LOGKIND_ERROR,
                             DDSC_LOG_ENABLED_DISCVOERY_TRANSPORT_SEQUENCE,1)
            goto done;
        }

        *DDS_StringSeq_get_reference(
                    &participant->qos.discovery.enabled_transports,0) =
                                         DDS_String_dup("_udp://");
    }
#endif

    /* Add the default user-traffic transports unless the user has specified a list */
    if (DDS_StringSeq_get_maximum(&qos->user_traffic.enabled_transports) == 0)
    {
        if (!DDS_StringSeq_set_maximum(&participant->qos.user_traffic.enabled_transports,
                                            DDS_PARTICIPANT_DEFAULT_TRANSPORT_LENGTH))
        {
            DDSC_LOG_SEQ_SETMAX(OSAPI_LOGKIND_ERROR,
                                DDSC_LOG_ENABLED_USER_TRANSPORT_SEQUENCE,
                                DDS_PARTICIPANT_DEFAULT_TRANSPORT_LENGTH)
            goto done;
        }

        if (!DDS_StringSeq_set_length(&participant->qos.user_traffic.enabled_transports,
                                        DDS_PARTICIPANT_DEFAULT_TRANSPORT_LENGTH))
        {
            DDSC_LOG_SEQ_SETLENGTH(OSAPI_LOGKIND_ERROR,
                                    DDSC_LOG_ENABLED_USER_TRANSPORT_SEQUENCE,
                                    DDS_PARTICIPANT_DEFAULT_TRANSPORT_LENGTH)
            goto done;
        }

         *DDS_StringSeq_get_reference(
                        &participant->qos.user_traffic.enabled_transports,0) =
                                DDS_String_dup("_intra://");
#if !UDP_EXCLUDE_BUILTIN
        *DDS_StringSeq_get_reference(
                        &participant->qos.user_traffic.enabled_transports,1) =
                                DDS_String_dup("_udp://");
#endif                                
       
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
             (struct REDA_StringSeq*)&participant->qos.transports.enabled_transports,
             (struct REDA_StringSeq*)&participant->qos.discovery.enabled_transports,
             NETIO_ROUTEKIND_META,
             (struct NETIO_AddressSeq*)&participant->builtin_data.metatraffic_multicast_locators,
             (struct NETIO_AddressSeq*)&participant->builtin_data.metatraffic_unicast_locators))
        {
            continue;
        }

        if (!NETIO_BindResolver_reserve_addresses(
             participant->bind_resolver,i,
             (struct REDA_StringSeq*)&participant->qos.transports.enabled_transports,
             (struct REDA_StringSeq*)&participant->qos.user_traffic.enabled_transports,
             NETIO_ROUTEKIND_USER,
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
    NETIO_Address_set_guid(&src_address,(RTI_UINT32)participant->domain_id,
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

    /* Create property sequence. This information is used by RTI tools
     */
    DDS_PropertySeq_initialize(&participant->dds_sys_property);

#if !DDS_DISABLE_PARTICIPANT_INFO
    if (!REDA_Sequence_loan_contiguous(
            (struct REDA_Sequence*)&participant->dds_sys_property,
            &participant->dds_sys_property_values,0,
            DDS_DOMAINPARTICIPANT_MAX_PROPERTIES))
    {
        goto done;
    }

    /* Add the hostname property */
    DDS_PropertySeq_set_length(&participant->dds_sys_property,1);
    a_property_qos = DDS_PropertySeq_get_reference(&participant->dds_sys_property,0);
    if (!OSAPI_System_get_hostname(participant->hostname_property))
    {
        DDSC_LOG_SYS_GET_HOSTNAME(OSAPI_LOGKIND_ERROR)
        goto done;
    }
    a_property_qos->name = DDS_PROPERTY_HOSTNAME_NAME;
    a_property_qos->value = participant->hostname_property;

    if (OSAPI_Process_pid_as_string(participant->process_property,
                       OSAPI_SYSTEM_MAX_HOSTNAME,
                       OSAPI_Process_getpid()) >= OSAPI_SYSTEM_MAX_HOSTNAME)
    {
        DDSC_LOG_IO_SNPRINTF_FAILED(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    DDS_PropertySeq_set_length(&participant->dds_sys_property,2);
    a_property_qos = DDS_PropertySeq_get_reference(&participant->dds_sys_property,1);
    a_property_qos->name = DDS_PROPERTY_PROCESSID_NAME;
    a_property_qos->value = participant->process_property;

    /* Target name */
    DDS_PropertySeq_set_length(&participant->dds_sys_property,3);
    a_property_qos = DDS_PropertySeq_get_reference(&participant->dds_sys_property,2);
    a_property_qos->name = DDS_PROPERTY_TARGET_NAME;

    a_property_qos->value = OSAPI_CC_STRINGIFY_DEFINE(RTIME_TARGET_NAME);

    /* Version info: */

    v_idx = 0;
#if (RTIME_DDS_VERSION_MAJOR >=0) && (RTIME_DDS_VERSION_MAJOR <= 9)
    participant->version_property[v_idx++] = '0' + RTIME_DDS_VERSION_MAJOR;
#else
#error "RTIME_DDS_VERSION_MAJOR must be in the range [0,9]"
#endif

    participant->version_property[v_idx++] = '.';

#if (RTIME_DDS_VERSION_MINOR >=0) && (RTIME_DDS_VERSION_MINOR <= 9)
    participant->version_property[v_idx++] = '0' + RTIME_DDS_VERSION_MINOR;
#else
#error "RTIME_DDS_VERSION_MINOR must be in the range [0,9]"
#endif

    participant->version_property[v_idx++] = '.';

#if (RTIME_DDS_VERSION_REVISION >= 0) && (RTIME_DDS_VERSION_REVISION <= 9)
    participant->version_property[v_idx++] = '0' + RTIME_DDS_VERSION_REVISION;
#elif (RTIME_DDS_VERSION_REVISION >= 0) && (RTIME_DDS_VERSION_REVISION <= 99)
    participant->version_property[v_idx++] = '0' + RTIME_DDS_VERSION_REVISION / 10;
    participant->version_property[v_idx++] = '0' + RTIME_DDS_VERSION_REVISION % 10;
#else
#error "RTIME_DDS_VERSION_REVISION must be in the range [0,99]"
#endif

    participant->version_property[v_idx++] = '.';

#if (RTIME_DDS_VERSION_RELEASE >= 0) && (RTIME_DDS_VERSION_RELEASE <= 9)
    participant->version_property[v_idx++] = '0' + RTIME_DDS_VERSION_RELEASE;
#elif (RTIME_DDS_VERSION_RELEASE >= 0) && (RTIME_DDS_VERSION_RELEASE <=99)
    participant->version_property[v_idx++] = '0' + RTIME_DDS_VERSION_RELEASE / 10;
    participant->version_property[v_idx++] = '0' + RTIME_DDS_VERSION_RELEASE % 10;
#else
#error "RTIME_DDS_VERSION_RELEASE must be in the range [0,99]"
#endif

    participant->version_property[v_idx] = 0;

    DDS_PropertySeq_set_length(&participant->dds_sys_property,4);
    a_property_qos = DDS_PropertySeq_get_reference(&participant->dds_sys_property,3);
    a_property_qos->name = DDS_PROPERTY_VERSION_NAME;
    a_property_qos->value = participant->version_property;
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
                                   &participant->qos.discovery.discovery.name))
            goto done;
        }
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

    if (!success)
    {
        participant = NULL;
    }

    return participant;
}

DDS_ReturnCode_t
DDS_DomainParticipant_register_type(DDS_DomainParticipant *participant,
                                    const char *type_name,
                                    struct NDDS_Type_Plugin *plugin)
{
    struct DDS_DomainParticipantImpl *self =
                            (struct DDS_DomainParticipantImpl *)participant;
    struct DDS_TypeImpl *a_type = NULL;
    DB_ReturnCode_T dbrc;
    DDS_ReturnCode_t rc = DDS_RETCODE_ERROR;
#ifndef RTI_CERT
    char *managed_type = NULL;
#endif

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

    if (DB_Database_lock(self->database) != DB_RETCODE_OK)
    {
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

#ifndef RTI_CERT
    managed_type = DDS_StringManager_assert_string(participant->string_manager,type_name);
    if (managed_type == NULL)
    {
        DDSC_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR, DDSC_LOG_STRING_OBJECT)
        goto done;
    }
#endif

    rc = DDS_RETCODE_ERROR;
    dbrc = DB_Table_create_record(self->type_table,(DB_Record_T*)&a_type);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_CREATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_TYPE_RECORD,dbrc)
        if (dbrc == DB_RETCODE_OUT_OF_RESOURCES)
        {
            rc = DDS_RETCODE_OUT_OF_RESOURCES;
        }
#ifndef RTI_CERT
        (void)DDS_StringManager_delete_string(participant->string_manager,managed_type);
#endif
        goto done;
    }

#ifndef RTI_CERT
    if (!DDS_TypeImpl_initalize(a_type,managed_type,plugin))
#else
    if (!DDS_TypeImpl_initalize(a_type,type_name,plugin))
#endif
    {
        DDSC_LOG_RECORD_INITIALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_TYPE_RECORD)
        (void)DB_Table_delete_record(self->type_table,a_type);
#ifndef RTI_CERT
        (void)DDS_StringManager_delete_string(participant->string_manager,managed_type);
#endif
        goto done;
    }

    dbrc = DB_Table_insert_record(self->type_table,(DB_Record_T)a_type);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_INSERT(OSAPI_LOGKIND_ERROR,DDSC_LOG_TYPE_RECORD,dbrc)
        /* Besides OK, insert_record returns EXISTS */
        (void)DB_Table_delete_record(self->type_table,a_type);
#ifndef RTI_CERT
        (void)DDS_StringManager_delete_string(participant->string_manager,managed_type);
#endif
        goto done;
    }

    rc = DDS_RETCODE_OK;

done:

    if (DB_Database_unlock(self->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
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
RTI_PRIVATE struct NDDS_Type_Plugin*
DDS_DomainParticipant_unregister_type_record(DDS_DomainParticipant *participant,
                                             struct DDS_TypeImpl *type)
{
    struct DDS_DomainParticipantImpl *self =
                            (struct DDS_DomainParticipantImpl *)participant;
    DB_ReturnCode_T dbrc;
    struct NDDS_Type_Plugin *type_plugin = NULL;
    struct DDS_TypeImpl *typerec;
#ifndef RTI_CERT
    char *managed_type = NULL;
#endif

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

#ifndef RTI_CERT
    managed_type = typerec->name;
#endif

    dbrc = DB_Table_delete_record_w_dtor(self->type_table,
                                         (DB_Record_T)typerec,DDS_Type_dtor);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_DELETE(OSAPI_LOGKIND_ERROR,DDSC_LOG_TYPE_RECORD,dbrc)
        type_plugin = NULL;
    }

#ifndef RTI_CERT
    if (!DDS_StringManager_delete_string(participant->string_manager,managed_type))
    {
        type_plugin = NULL;
        goto done;
    }
#endif

done:
    return type_plugin;
}

struct NDDS_Type_Plugin*
DDS_DomainParticipant_unregister_type(DDS_DomainParticipant * participant,
                                      const char *type_name)
{
    struct DDS_DomainParticipantImpl *self =
                            (struct DDS_DomainParticipantImpl *)participant;
    struct DDS_TypeImpl *a_type = NULL;
    DB_ReturnCode_T dbrc;
    struct NDDS_Type_Plugin *type_plugin = NULL;

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
            ddsrc = DDS_Publisher_delete_contained_entities(publisher);
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
            ddsrc = DDS_Subscriber_delete_contained_entities(subscriber);
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
                /* It does not matter how many times the topic is referenced,
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
                    DDS_PARTICIPANT_BUILTIN_TOPIC_TYPE_NAME))
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
        /* In case there was a database error return the generic
         * DDS_RETCODE_ERROR.
         */
        return DDS_RETCODE_ERROR;
    }

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
#else
    self->pub_config.on_before_datawriter_deleted = NULL;
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
    self->pub_config.participant_id = &participant->participant_id;
    self->pub_config.dw_config = &participant->dw_config;
    self->pub_config.domain_id = &participant->domain_id;

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
DDS_ReturnCode_t
DDS_DomainParticipant_delete_publisher(DDS_DomainParticipant *self,
                                       DDS_Publisher *publisher)
{
    struct DDS_PublisherImpl *pub_impl = (struct DDS_PublisherImpl*)publisher;
    DB_ReturnCode_T dbrc;
    DDS_ReturnCode_t ddsrc = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (publisher == NULL),
                   return DDS_RETCODE_BAD_PARAMETER,
                   OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                   OSAPI_Log_entry_add_pointer("publisher",publisher,RTI_TRUE);)

    if (DB_Database_lock(self->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

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
    self->sub_config.enabled_transports = &participant->qos.transports.enabled_transports;
    self->sub_config.route_resolver = participant->route_resolver;
    self->sub_config.participant_id = &participant->participant_id;
    self->sub_config.dr_config = &participant->dr_config;
    self->sub_config.domain_id = &participant->domain_id;

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
DDS_ReturnCode_t
DDS_DomainParticipant_delete_subscriber(DDS_DomainParticipant *self,
                                       DDS_Subscriber *subscriber)
{
    struct DDS_SubscriberImpl *sub_impl = (struct DDS_SubscriberImpl*)subscriber;
    DB_ReturnCode_T dbrc;
    DDS_ReturnCode_t ddsrc = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (subscriber == NULL),
               return DDS_RETCODE_BAD_PARAMETER,
               OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("subscriber",subscriber,RTI_TRUE);)


    if (DB_Database_lock(self->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

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
#ifndef RTI_CERT
    char* managed_topic = NULL;
#endif

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

#ifndef RTI_CERT
    managed_topic =  
        DDS_StringManager_assert_string(self->string_manager,topic_name);
    if (managed_topic == NULL)
    {
        DDSC_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR, DDSC_LOG_STRING_OBJECT)
        goto done;
    }
#endif

    dbrc = DB_Table_create_record(self->topic_table,(DB_Record_T*)&topic);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_CREATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_TOPIC_RECORD,dbrc)
#ifndef RTI_CERT
        (void)DDS_StringManager_delete_string(participant->string_manager,managed_topic);
#endif
        goto done;
    }

    self->topic_config.db = participant->database;
    self->topic_config.on_inconsistent_topic = NDDS_DomainParticipant_on_inconsistent_topic;
    self->topic_config.get_parent_handle = DDS_DomainParticipantImpl_get_instance_handle;

#ifndef RTI_CERT
    if (!DDS_TopicImpl_initialize(topic,
                                self,managed_topic,a_type,
                                qos,listener,mask,
                                0,&self->topic_config))
#else
    if (!DDS_TopicImpl_initialize(topic,
                                self,topic_name,a_type,
                                qos,listener,mask,
                                0,&self->topic_config))
#endif
    {
        (void)DB_Table_delete_record(participant->topic_table,
                                     (DB_Record_T)topic);
#ifndef RTI_CERT
        (void)DDS_StringManager_delete_string(participant->string_manager,managed_topic);
#endif
        goto done;
    }

    dbrc = DB_Table_insert_record(participant->topic_table,topic);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_INSERT(OSAPI_LOGKIND_ERROR,DDSC_LOG_TOPIC_RECORD,dbrc)
        (void)DB_Table_delete_record(participant->topic_table,
                                     (DB_Record_T)topic);
#ifndef RTI_CERT
        (void)DDS_StringManager_delete_string(participant->string_manager,managed_topic);
#endif
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
#ifndef RTI_CERT
    char *managed_topic = NULL;
#endif

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

#ifndef RTI_CERT
    managed_topic = (char*)DDS_TopicDescription_get_name(
                                    DDS_Topic_as_topicdescription(topic));
#endif

    dbrc = DB_Table_delete_record_w_dtor(self->topic_table,(DB_Record_T)topic,DDS_Topic_dtor);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_DELETE(OSAPI_LOGKIND_ERROR,DDSC_LOG_TOPIC_RECORD,dbrc)
        goto done;
    }

#ifndef RTI_CERT
    if (!DDS_StringManager_delete_string(self->string_manager,managed_topic))
    {
        goto done;
    }
#endif

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
                    peer)
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

struct DDS_PropertySeq*
DDS_DomainParticipant_get_dds_properties(DDS_DomainParticipant *self)
{
    struct DDS_DomainParticipantImpl *participant =
                                    (struct DDS_DomainParticipantImpl *)self;

    OSAPI_PRECONDITION(self == NULL,
                           return NULL,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return &participant->dds_sys_property;
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

    retcode = DDS_PublisherQos_copy(qos, &self->_default_publisher_qos);

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
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (qos == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE);)

    if (!DDS_PublisherQos_is_consistent(qos))
    {
        DDSC_LOG_QOS_INCONSISTENT(OSAPI_LOGKIND_ERROR,DDSC_LOG_DEFAULTPUBLISHER_QOS)
        return DDS_RETCODE_INCONSISTENT_POLICY;
    }

    if (!OSAPI_Mutex_take(self->shared_lock))
    {
        return DDS_RETCODE_ERROR;
    }

    retcode = DDS_PublisherQos_copy(&self->_default_publisher_qos, qos);

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

    if (DDS_Entity_is_enabled(&self->as_entity))
    {
        DDSC_LOG_QOS_SET_ON_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_QOS)
        retcode = DDS_RETCODE_PRECONDITION_NOT_MET;
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

    retcode = DDS_SubscriberQos_copy(qos, &self->_default_subscriber_qos);

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
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (qos == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE);)

    if (!DDS_SubscriberQos_is_consistent(qos))
    {
        DDSC_LOG_QOS_INCONSISTENT(OSAPI_LOGKIND_ERROR,DDSC_LOG_DEFAULTSUBSCRIBER_QOS)
        return DDS_RETCODE_INCONSISTENT_POLICY;
    }

    if (!OSAPI_Mutex_take(self->shared_lock))
    {
        return DDS_RETCODE_ERROR;
    }

    retcode = DDS_SubscriberQos_copy(&self->_default_subscriber_qos, qos);

#if OSAPI_ENABLE_LOG
    if (retcode != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_DEFAULTSUBSCRIBER_QOS)
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
DDS_DomainParticipant_get_default_topic_qos(DDS_DomainParticipant *self,
                                            struct DDS_TopicQos *qos)
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

    retcode = DDS_TopicQos_copy(qos, &self->_default_topic_qos);

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
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (qos == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE);)

    if (!OSAPI_Mutex_take(self->shared_lock))
    {
        return DDS_RETCODE_ERROR;
    }

    retcode = DDS_TopicQos_copy(&self->_default_topic_qos, qos);

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


DDS_ReturnCode_t
DDS_DomainParticipant_get_current_time(DDS_DomainParticipant *self,
                                       struct DDS_Time_t *current_time)
{
    struct OSAPI_NtpTime now;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (current_time == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
           OSAPI_Log_entry_add_pointer("current_time",current_time,RTI_TRUE);)

    if (!OSAPI_System_get_time(&now))
    {
        DDSC_LOG_SYS_GETTIME(OSAPI_LOGKIND_ERROR)
        return DDS_RETCODE_ERROR;
    }

    OSAPI_NtpTime_to_nanosec((&(current_time->sec)),
                              (&(current_time->nanosec)), &now);

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
    PRECOND_ARG(participant_handles)

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (participant_handles == NULL),
                  return DDS_RETCODE_BAD_PARAMETER,
                  OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                  OSAPI_Log_entry_add_pointer("participant_handles",participant_handles,RTI_TRUE);)

    if (!DDS_Entity_is_enabled(&participant->as_entity))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_ENTITY)
        return DDS_RETCODE_NOT_ENABLED;
    }

    return DDS_RETCODE_UNSUPPORTED;
}
#endif

#if INCLUDE_API_LOOKUP
DDS_ReturnCode_t
DDS_DomainParticipant_get_discovered_participant_data(
            DDS_DomainParticipant *self,
            struct DDS_ParticipantBuiltinTopicData *participant_data,
            const DDS_InstanceHandle_t *participant_handle)
{
    struct DDS_DomainParticipantImpl *participant =
                                    (struct DDS_DomainParticipantImpl *)self;
    PRECOND_ARG(participant_data)
    PRECOND_ARG(participant_handle)

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

    return DDS_RETCODE_UNSUPPORTED;
}

#endif

/*ci \brief Return the next object id for an entity
 * 
 * \param[in] self Participant generating the object-id
 *
 * \return An object-id on success, 0 on failure.
 */
RTI_UINT32
DDS_DomainParticipant_get_next_objectid(void *self)
{
    struct DDS_DomainParticipantImpl *participant = (struct DDS_DomainParticipantImpl*)self;

    OSAPI_PRECONDITION(self == NULL,
                       return 0,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE))
    
     ++participant->current_object_id;

    /* Object-ids are not allowed to wrap around. A maximum of
     * (OSAPI_SYSTEM_OBJECTID_MAX - OSAPI_SYSTEM_OBJECTID_START) objects
     * can be created.
     */
    if (participant->current_object_id == OSAPI_SYSTEM_OBJECTID_MAX)
    {
        /* Make sure that if the limit has been reached always return 0
         * for every subsequent call (because object-ids are never returned)
         */
        --participant->current_object_id;

        DDSC_LOG_GET_NEXT_OBJECT_ID(OSAPI_LOGKIND_ERROR)
        return 0;
    }

    return participant->current_object_id;
}

/*ci @} */
