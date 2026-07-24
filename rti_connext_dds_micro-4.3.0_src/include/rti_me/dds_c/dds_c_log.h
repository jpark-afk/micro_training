/*
 * FILE: dds_c_log.h - DDS log definitions
 *
 * Copyright (c) 2012-2026 Real-Time Innovations,Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 27jun2016,tk MICRO-1545 Added log message for topic in-use
 * 17dec2015,eh MICRO-1511 Add DDSC_LOG_CDR_DESERIALIZE_KEY
 * 01dec2014,tk MICRO-991: Fixed missing %s in format string
 * 09feb2014,eh MICRO-712: complete HTML doc descriptions
 * 30may2013,eh MICRO-415: resource limit msgs
 * 14may2013,eh MICRO-639, MICRO-394: add DDSC_LOG_DR_DESERIALIZE_BAD_PID_LENGTH
 * 30apr2012,tk Written
 */
/*e
 * \file
 * \brief DDS_C module log codes
 */

#ifndef dds_c_log_h
#define dds_c_log_h

#include "dds_c/dds_c_config.h"

#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif

/*e
 * \defgroup DDSCLogCodesClass DDS_C
 * \brief DDS C. ModuleID = 7
 * \ingroup LoggingModule
 */

/*******************************************************************************
 *                       INFRASTRUCTURE LOG-CODES
 ******************************************************************************/
/*e
 * \brief The specified duration is not valid.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_INVALID_DURATION_EC                         (DDSC_LOG_BASE + 1)
#define DDSC_LOG_INVALID_DURATION(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_INVALID_DURATION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief An invalid participant name was specified with an unknown GUID.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_INVALID_PARTICIPANT_NAME_EC                 (DDSC_LOG_BASE + 2)
#define DDSC_LOG_INVALID_PARTICIPANT_NAME(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_INVALID_PARTICIPANT_NAME_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief An invalid participant GUID prefix was specified, typically the
 *        GUID prefix does not match an already detected participant.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_INVALID_PARTICIPANT_GUID_PREFIX_EC          (DDSC_LOG_BASE + 3)
#define DDSC_LOG_INVALID_PARTICIPANT_GUID_PREFIX(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_INVALID_PARTICIPANT_GUID_PREFIX_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief A call to OSAPI_System_get_time failed.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_SYS_GETTIME_EC                              (DDSC_LOG_BASE + 4)
#define DDSC_LOG_SYS_GETTIME(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_SYS_GETTIME_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to get the next automatically generated object ID for an
 *        entity's GUID. This typically means the object id pool has been
 *        exhausted.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_GET_NEXT_OBJECT_ID_EC                       (DDSC_LOG_BASE + 5)
#define DDSC_LOG_GET_NEXT_OBJECT_ID(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_GET_NEXT_OBJECT_ID_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to set name string for a DDS entity in the DomainParticipantQos
 *        entity_name policy.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_SET_ENTITY_NAME_EC                          (DDSC_LOG_BASE + 6)
#define DDSC_LOG_SET_ENTITY_NAME(level_,name_,len_,max_) \
OSAPI_LOG_ENTRY_CREATE((level_),DDSC_LOG_SET_ENTITY_NAME_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_STRING("name",(name_),RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_UINT("len",(len_),RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_INT("max",(max_),RTI_TRUE)

/*e
 * \brief A call to OSAPI_System_get_hostname failed.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_SYS_GET_HOSTNAME_EC                         (DDSC_LOG_BASE + 7)
#define DDSC_LOG_SYS_GET_HOSTNAME(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_SYS_GET_HOSTNAME_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief A call to OSAPI_Stdio_snprintf failed. Typically this means the
 *        destination buffer was too small.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_IO_SNPRINTF_FAILED_EC                       (DDSC_LOG_BASE + 8)
#define DDSC_LOG_IO_SNPRINTF_FAILED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_IO_SNPRINTF_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to find a topic created by a DomainParticipant.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TOPIC_FIND_EC                               (DDSC_LOG_BASE + 9)
#define DDSC_LOG_TOPIC_FIND(level_,topic_,dbrc_) \
OSAPI_LOG_ENTRY_CREATE((level_),DDSC_LOG_TOPIC_FIND_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_STRING("topic",(topic_),RTI_FALSE)\
OSAPI_LOG_ENTRY_ADD_INT("dbrc",(dbrc_),RTI_TRUE)

/*e
 * \brief Endpoint discovery failed because the name of the remote participant
 *        parent for an endpoint was not found.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_UNKNOWN_REMOTE_PARTICIPANT_NAME_EC         (DDSC_LOG_BASE + 10)
#define DDSC_LOG_UNKNOWN_REMOTE_PARTICIPANT_NAME(level_,name_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDSC_LOG_UNKNOWN_REMOTE_PARTICIPANT_NAME_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"name",(name_))

/*e
 * \brief Endpoint discovery failed because the key of the remote participant
 *        parent for an endpoint was not found. Note that the key is logged
 *        as 4 integers in host endianess format.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_UNKNOWN_REMOTE_PARTICIPANT_KEY_EC          (DDSC_LOG_BASE + 11)
#define DDSC_LOG_UNKNOWN_REMOTE_PARTICIPANT_KEY(level_,key0_,key1_,key2_,key3_) \
OSAPI_LOG_ENTRY_CREATE((level_),DDSC_LOG_UNKNOWN_REMOTE_PARTICIPANT_KEY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_UINT("key0",(key0_),RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_UINT("key1",(key1_),RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_UINT("key2",(key2_),RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_UINT("key3",(key3_),RTI_TRUE)

/*e
 * \brief Failed endpoint discovery when key does not match the remote
 *        participant.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_REMOTE_PARTICIPANT_KEY_NOT_EQUAL_EC        (DDSC_LOG_BASE + 12)
#define DDSC_LOG_REMOTE_PARTICIPANT_KEY_NOT_EQUAL(level_,name_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDSC_LOG_REMOTE_PARTICIPANT_KEY_NOT_EQUAL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"name",(name_))

/*e
 * \brief Failed endpoint discovery due to an invalid or unknown endpoint GUID.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_INVALID_ENDPOINT_GUID_EC                   (DDSC_LOG_BASE + 13)
#define DDSC_LOG_INVALID_ENDPOINT_GUID(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_INVALID_ENDPOINT_GUID_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed endpoint discovery when an endpoint is determined to belong to
 *        a different remote participant.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_ENDPOINT_NOT_CHILD_OF_PARTICIPANT_EC       (DDSC_LOG_BASE + 14)
#define DDSC_LOG_ENDPOINT_NOT_CHILD_OF_PARTICIPANT(level_,participant_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDSC_LOG_ENDPOINT_NOT_CHILD_OF_PARTICIPANT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"participant",(participant_))

/*e
 * \brief Failed participant discovery because a remote participant that should
 *        have been already asserted locally was not found.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PARTICIPANT_DOES_NOT_EXIST_EC              (DDSC_LOG_BASE + 15)
#define DDSC_LOG_PARTICIPANT_DOES_NOT_EXIST(level_,participant_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDSC_LOG_PARTICIPANT_DOES_NOT_EXIST_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"participant",(participant_))

/*e
 * \brief Did not find a remote participant when asserting participant
 *        liveliness for it. Note that the key is logged as 4 integers in
 *        host endianess format.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_REFRESH_REM_PARTICIPANT_EC                 (DDSC_LOG_BASE + 16)
#define DDSC_LOG_REFRESH_REM_PARTICIPANT(level_,dbrc_,key0_,key1_,key2_,key3_) \
OSAPI_LOG_ENTRY_CREATE((level_),DDSC_LOG_REFRESH_REM_PARTICIPANT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_UINT("dbrc",(dbrc_),RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_UINT("key0",(key0_),RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_UINT("key1",(key1_),RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_UINT("key2",(key2_),RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_UINT("key3",(key3_),RTI_TRUE)

/*e
 * \brief Failed to assert participant liveliness to a remote participant.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_REFRESH_REM_PARTICIPANT_TIMEOUT_EC         (DDSC_LOG_BASE + 17)
#define DDSC_LOG_REFRESH_REM_PARTICIPANT_TIMEOUT(level_,name_,sec_,ns_) \
OSAPI_LOG_ENTRY_ADD_1STRING_2INT((level_),DDSC_LOG_REFRESH_REM_PARTICIPANT_TIMEOUT_EC,\
    OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"name",(name_),"sec",(sec_),"nanosec",(ns_))

/*e
 * \brief Failed to find a remote participant as previously discovered.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PARTICIPANT_LOOKUP_EC                      (DDSC_LOG_BASE + 18)
#define DDSC_LOG_PARTICIPANT_LOOKUP(level_,name_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDSC_LOG_PARTICIPANT_LOOKUP_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"name",(name_))

/*e
 * \brief Failed to remove resources for a remote publication.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_REMOVE_PUBLICATION_EC                      (DDSC_LOG_BASE + 19)
#define DDSC_LOG_REMOVE_PUBLICATION(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_REMOVE_PUBLICATION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to remove resources for a remote subscription.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_REMOVE_SUBSCRIPTION_EC                     (DDSC_LOG_BASE + 20)
#define DDSC_LOG_REMOVE_SUBSCRIPTION(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_REMOVE_SUBSCRIPTION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Cannot determine the participant of a remote publication.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_FIND_PUBLICATION_PARENT_EC                 (DDSC_LOG_BASE + 21)
#define DDSC_LOG_FIND_PUBLICATION_PARENT(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_FIND_PUBLICATION_PARENT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Cannot determine the participant of a remote subscription.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_FIND_SUBSCRIPTION_PARENT_EC                (DDSC_LOG_BASE + 22)
#define DDSC_LOG_FIND_SUBSCRIPTION_PARENT(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_FIND_SUBSCRIPTION_PARENT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create DomainParticipant due to running out of participant
 *        IDs. This is typically caused by all UDP ports being used.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_MAX_PARTICIPANT_ID_REACHED_EC              (DDSC_LOG_BASE + 23)
#define DDSC_LOG_MAX_PARTICIPANT_ID_REACHED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_MAX_PARTICIPANT_ID_REACHED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)


/*e
 * \brief Failed to reserve endpoint locators.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_RESERVE_LOCATORS_EC                        (DDSC_LOG_BASE + 24)
#define DDSC_LOG_RESERVE_LOCATORS(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_RESERVE_LOCATORS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)


/*e
 * \brief Failed to create a timeout.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TIMER_CREATE_TIMEOUT_EC                    (DDSC_LOG_BASE + 25)
#define DDSC_LOG_TIMER_CREATE_TIMEOUT(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_TIMER_CREATE_TIMEOUT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Illegal object id specified.
 *
 * \details
 *
 * Manually specifed object id must be in the range [0,0xffffff]
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_ILLEGAL_OBJECTID_EC                        (DDSC_LOG_BASE + 26)
#define DDSC_LOG_ILLEGAL_OBJECTID(level_,oid_) \
OSAPI_LOG_ENTRY_ADD_1UINT((level_),DDSC_LOG_ILLEGAL_OBJECTID_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"oid",(oid_))

/*e
 * \brief Failed to narrow a TopicDescription to the named Topic.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TOPIC_NARROW_EC                            (DDSC_LOG_BASE + 27)
#define DDSC_LOG_TOPIC_NARROW(level_,topic_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDSC_LOG_TOPIC_NARROW_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"topic",(topic_))

/*e
 * \brief Failed to update state of StatusCondition.
 *
 * \details
 *
 * An error occurred when trying to update the state (i.e. the trigger value) of
 * a StatusCondition.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_FAILED_UPDATE_STATUS_CONDITION_EC          (DDSC_LOG_BASE + 28)
#define DDSC_LOG_FAILED_UPDATE_STATUS_CONDITION(level_,entity_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDSC_LOG_FAILED_UPDATE_STATUS_CONDITION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"entity",#entity_)

/*e
 * \brief Failed to remove a condition reference from a waitset.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_WS_REMOVE_COND_REFERENCE_EC                (DDSC_LOG_BASE + 29)
#define DDSC_LOG_WS_REMOVE_COND_REFERENCE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_WS_REMOVE_COND_REFERENCE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to add a condition reference to a waitset.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_WS_ADD_COND_REFERENCE_EC                   (DDSC_LOG_BASE + 30)
#define DDSC_LOG_WS_ADD_COND_REFERENCE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_WS_ADD_COND_REFERENCE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to release resources for multicast discovery locators.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_RELEASE_META_MC_EC                         (DDSC_LOG_BASE + 31)
#define DDSC_LOG_RELEASE_META_MC(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_RELEASE_META_MC_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to release resources for unicast discovery locators.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_RELEASE_META_UC_EC                         (DDSC_LOG_BASE + 32)
#define DDSC_LOG_RELEASE_META_UC(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_RELEASE_META_UC_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to release resources for multicast user locators.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_RELEASE_USER_MC_EC                         (DDSC_LOG_BASE + 33)
#define DDSC_LOG_RELEASE_USER_MC(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_RELEASE_USER_MC_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to release resources for unicast user locators.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_RELEASE_USER_UC_EC                         (DDSC_LOG_BASE + 34)
#define DDSC_LOG_RELEASE_USER_UC(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_RELEASE_USER_UC_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief The domain ID specified exceeds what is allowed based on the
 *        parameters specified in dp_qos.protocol.rtps_well_known_ports
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_INVALID_DOMAINID_EC                        (DDSC_LOG_BASE + 35)
#define DDSC_LOG_INVALID_DOMAINID(level_,domainid_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_INVALID_DOMAINID_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"domain_id",(domainid_))

/*e
 * \brief Error occurred during the on_type_registered call back
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_ON_TYPE_REGISTERED_FAILURE_EC              (DDSC_LOG_BASE + 36)
#define DDSC_LOG_ON_TYPE_REGISTERED_FAILURE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_ON_TYPE_REGISTERED_FAILURE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Error occurred during the on_type_unregistered call back
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_ON_TYPE_UNREGISTERED_FAILURE_EC            (DDSC_LOG_BASE + 37)
#define DDSC_LOG_ON_TYPE_UNREGISTERED_FAILURE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_ON_TYPE_UNREGISTERED_FAILURE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Error occurred during the on_type_unregistered call back
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_GET_SERIALIZED_KEY_SIZE_EC                 (DDSC_LOG_BASE + 38)
#define DDSC_LOG_GET_SERIALIZED_KEY_SIZE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_GET_SERIALIZED_KEY_SIZE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)


/*e
 * \brief Minimum MTU across all Discovery transports must be greater or equal
          to a packet containing serialized ParticipantBuiltinTopicData.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_MIN_DISCOVERY_MTU_EC                (DDSC_LOG_BASE + 39)
#define DDSC_LOG_MIN_DISCOVERY_MTU(level_, size_, mtu_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_MIN_DISCOVERY_MTU_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"size",(size_), "min_mtu", (mtu_) )
/*e
 * \brief MTU for a transport set lower or equal to Protocol Overhead of 448 bytes.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_MIN_MTU_SIZE_EC                               (DDSC_LOG_BASE + 40)
#define DDSC_LOG_MIN_MTU_SIZE(level_, size_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_MIN_MTU_SIZE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"size",(size_))

/*e
 * \brief MTU could not be found because of no routes
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_GET_MTU_NO_ROUTES_EC                       (DDSC_LOG_BASE + 41)
#define DDSC_LOG_GET_MTU_NO_ROUTES(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_GET_MTU_NO_ROUTES_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)


/*e
 * \brief Failed to resolve locators based on priority.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_RESOLVE_LOCATORS_PRIORITY_EC               (DDSC_LOG_BASE + 42)
#define DDSC_LOG_RESOLVE_LOCATORS_PRIORITY(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_RESOLVE_LOCATORS_PRIORITY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief The publication builtin topic data for a remote publication changed
 *        after the remote publication was asserted.
 *
 * \details
 * No dynamic changes are supported to the publication builtin topic data after
 * the remote publication is asserted. The previous data will continue to be used.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_REMOTE_PUBLICATION_DATA_CHANGED_EC         (DDSC_LOG_BASE + 43)
#define DDSC_LOG_REMOTE_PUBLICATION_DATA_CHANGED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_REMOTE_PUBLICATION_DATA_CHANGED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Fields in subscription builtin topic data for a remote subscription
 *        which are not supported to dynamically change were updated after the
 *        remote subscription was asserted.
 *
 * \details
 * The only supported dynamic changes to the subscription builtin topic data
 * are the configured content filter on the remote subscription. Any other
 * changes are not supported and will be ignored.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_REMOTE_SUBSCRIPTION_DATA_CHANGED_EC        (DDSC_LOG_BASE + 44)
#define DDSC_LOG_REMOTE_SUBSCRIPTION_DATA_CHANGED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_REMOTE_SUBSCRIPTION_DATA_CHANGED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Error getting the mtu from the route resolver
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_GET_MTU_EC                               (DDSC_LOG_BASE + 45)
#define DDSC_LOG_GET_MTU(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_GET_MTU_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)
/*******************************************************************************
 *                            DATABASE RELATED
 ******************************************************************************/
/*e
 * \brief Failed to create database.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DATABASE_CREATE_EC                        (DDSC_LOG_BASE + 100)
#define DDSC_LOG_DATABASE_CREATE(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_DATABASE_CREATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to delete database.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DATABASE_DELETE_EC                        (DDSC_LOG_BASE + 101)
#define DDSC_LOG_DATABASE_DELETE(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_DATABASE_DELETE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to create database table of the specified name.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TABLE_CREATE_EC                           (DDSC_LOG_BASE + 102)
#define DDSC_LOG_TABLE_CREATE(level_,name_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1STRING_1INT((level_),DDSC_LOG_TABLE_CREATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "name",(name_),"dbrc",(dbrc_))

/*e
 * \brief Failed to delete a database table because it is not empty.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TABLE_INUSE_EC                            (DDSC_LOG_BASE + 103)
#define DDSC_LOG_TABLE_INUSE(level_,table_,count_) \
OSAPI_LOG_ENTRY_ADD_1STRING_1INT((level_),DDSC_LOG_TABLE_INUSE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "table",DB_Table_get_name((table_)),"count",(count_))

/*e
 * \brief Failed to delete a database table.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TABLE_DELETE_EC                           (DDSC_LOG_BASE + 104)
#define DDSC_LOG_TABLE_DELETE(level_,table_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1STRING_1INT((level_),DDSC_LOG_TABLE_DELETE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "table",DB_Table_get_name((table_)),"dbrc",(dbrc_))

/*e
 * \brief A selection operation failed on the specified database table.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TABLE_SELECT_EC                           (DDSC_LOG_BASE + 105)
#define DDSC_LOG_TABLE_SELECT(level_,table_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1STRING_1INT((level_),DDSC_LOG_TABLE_SELECT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "table",DB_Table_get_name((table_)),"dbrc",(dbrc_))

/*e
 * \brief Failed to create an index on a database table.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_CREATE_INDEX_EC                           (DDSC_LOG_BASE + 106)
#define DDSC_LOG_CREATE_INDEX(level_,table_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1STRING_1INT((level_),DDSC_LOG_CREATE_INDEX_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "table",DB_Table_get_name((table_)),"dbrc",(dbrc_))

/*e
 * \brief Failed to delete an index on a database table.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DELETE_INDEX_EC                           (DDSC_LOG_BASE + 107)
#define DDSC_LOG_DELETE_INDEX(level_,table_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1STRING_1INT((level_),DDSC_LOG_DELETE_INDEX_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "table",DB_Table_get_name((table_)),"dbrc",(dbrc_))


/*e
 * \brief A database table cursor was invalidated while in use.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DB_CURSOR_INVALIDATED_EC                  (DDSC_LOG_BASE + 108)
#define DDSC_LOG_DB_CURSOR_INVALIDATED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_DB_CURSOR_INVALIDATED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)


/*e
 * \brief Database record for a Subscription.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_SUBSCRIPTION_RECORD              1

/*e
 * \brief Database record for a Publication.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PUBLICATION_RECORD               2

/*e
 * \brief Database record for a Remote Participant.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PARTICIPANT_RECORD               3

/*e
 * \brief Database record for a Topic.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TOPIC_RECORD                     4

/*e
 * \brief Database record for a DataWriter.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DATAWRITER_RECORD                5

/*e
 * \brief Database record for a DataReader.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DATAREADER_RECORD                6

/*e
 * \brief Database record for a Publisher.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PUBLISHER_RECORD                 7

/*e
 * \brief Database record for a Subscriber.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_SUBSCRIBER_RECORD                8

/*e
 * \brief Database record for a route record.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_ROUTE_RECORD                     9

/*e
 * \brief Database record for a bind record.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_BIND_RECORD                      10

/*e
 * \brief Database record for a type plugin.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TYPE_RECORD                      11

/*e
 * \brief Database record for a remote trust endpoint.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_REMOTE_ENDPOINT_TRUST_STATE_RECORD                12

/*e
 * \brief Database record for a matched remote data writer.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_MATCHED_DW_RECORD                13

/*e
 * \brief Database record for a string property.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_STRING_RECORD                     14

/*e
 * \brief Failed to create a database record of the specified kind.
 *
 * \details
 * The creation of an internal database record failed The failure may
 * have been caused by insufficient resources based on the record kind.
 * The following resource-limits may apply:
 *
 *
 * DomainParticipantQos.resource_limit.local_publisher_allocation
 *          limits the number of DDS Publishers.
 *
 * DomainParticipantQos.resource_limit.local_subscriber_allocation
 *          limits the number of DDS Subscribers
 *
 * DomainParticipantQos.resource_limit.local_topic_allocation
 *          limits the number of DDS Topics.
 *
 * DomainParticipantQos.resource_limits.local_reader_allocation
 *          limits the number of DDS DataReader records.
 *
 * DomainParticipantQos.resource_limits.local_writer_allocation
 *          limits the number of DDS DataWriter records.
 *
 * DomainParticipantQos.resource_limits.remote_writer_allocation
 *          limits the number of remote publication records
 *
 * DomainParticipantQos.resource_limits.remote_reader_allocation
 *          limits the number of remote subscription records.
 *
 * DomainParticipantQos.resource_limits.remote_participant_allocation
 *          limits the number of remote participant records.
 *
 * DataWriterQos.writer_resource_limits.max_remote_readers
 *          limits of the number of DDS DataReaders a DDS DataWriter can
 *          communicate with.
 *
 * DataReaderQos.reader_resource_limits.max_remote_writers
 *          limits of the number of DDS DataWriter a DDS DataReader can
 *          communicate with.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_RECORD_CREATE_EC                          (DDSC_LOG_BASE + 109)
#define DDSC_LOG_RECORD_CREATE(level_,kind_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_RECORD_CREATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_),"dbrc",(dbrc_))

/*e
 * \brief Failed to delete a database record of the specified kind.
 *
 * - kind is the kind of record that failed to be deleted. The following kinds
 *   are defined:
 *   - \ref DDSC_LOG_BIND_RECORD
 *   - \ref DDSC_LOG_PUBLICATION_RECORD
 *   - \ref DDSC_LOG_SUBSCRIPTION_RECORD
 *   - \ref DDSC_LOG_SUBSCRIBER_RECORD
 *   - \ref DDSC_LOG_PUBLISHER_RECORD
 *   - \ref DDSC_LOG_PARTICIPANT_RECORD
 *   - \ref DDSC_LOG_TOPIC_RECORD
 *   - \ref DDSC_LOG_TYPE_RECORD
 *   - \ref DDSC_LOG_DATAWRITER_RECORD
 *   - \ref DDSC_LOG_DATAREADER_RECORD
 *   - \ref DDSC_LOG_STRING_RECORD
 *   - \ref DDSC_LOG_MATCHED_DW_RECORD
 *   - \ref DDSC_LOG_ROUTE_RECORD
 *   - \ref DDSC_LOG_DATAWRITER_RECORD
 *   - \ref DDSC_LOG_DATAREADER_RECORD
 *
 * - dbrc is the database return code from the delete operation. This
 *   information is internal, but helpful to RTI support.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_RECORD_DELETE_EC                          (DDSC_LOG_BASE + 110)
#define DDSC_LOG_RECORD_DELETE(level_,kind_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_RECORD_DELETE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_),"dbrc",(dbrc_))


/*e
 * \brief Failed to insert a database record of the specified kind.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_RECORD_INSERT_EC                          (DDSC_LOG_BASE + 111)
#define DDSC_LOG_RECORD_INSERT(level_,kind_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_RECORD_INSERT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_),"dbrc",(dbrc_))

/*e
 * \brief Unknown error for database record of the specified kind.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_RECORD_ERROR_EC                           (DDSC_LOG_BASE + 112)
#define DDSC_LOG_RECORD_ERROR(level_,kind_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_RECORD_ERROR_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_),"dbrc",(dbrc_))

/*e
 * \brief A database record of the specified kind already exists.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_RECORD_EXISTS_EC                          (DDSC_LOG_BASE + 113)
#define DDSC_LOG_RECORD_EXISTS(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_RECORD_EXISTS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief A lookup of a database record of the specified kind failed.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_RECORD_LOOKUP_EC                          (DDSC_LOG_BASE + 114)
#define DDSC_LOG_RECORD_LOOKUP(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_RECORD_LOOKUP_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief A database record of the specified kind does not exist.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_RECORD_NOT_EXISTS_EC                      (DDSC_LOG_BASE + 115)
#define DDSC_LOG_RECORD_NOT_EXISTS(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_RECORD_NOT_EXISTS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief A database select on the specified record kind failed.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_RECORD_SELECT_EC                          (DDSC_LOG_BASE + 116)
#define DDSC_LOG_RECORD_SELECT(level_,kind_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_RECORD_SELECT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_),"dbrc",(dbrc_))

/*e
 * \brief Removal a database record of the specified kind failed.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_RECORD_REMOVE_EC                          (DDSC_LOG_BASE + 117)
#define DDSC_LOG_RECORD_REMOVE(level_,kind_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_RECORD_REMOVE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_),"dbrc",(dbrc_))

/*e
 * \brief A database record of the specified kind could not be initialized
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_RECORD_INITIALIZE_EC                      (DDSC_LOG_BASE + 118)
#define DDSC_LOG_RECORD_INITIALIZE(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_RECORD_INITIALIZE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief A database record of the specified kind could not be finalized
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_RECORD_FINALIZE_EC                        (DDSC_LOG_BASE + 119)
#define DDSC_LOG_RECORD_FINALIZE(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_RECORD_FINALIZE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief A database record of the specified kind could not be reset
 *
 * A database record is "reset" when all its attributes are reverted to a
 * previous state (typically the one it had at creation/insertion),
 * without removing it from the database.
 *
 * The actions performed by the "reset" depend on the type of record.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_RECORD_RESET_EC                           (DDSC_LOG_BASE + 120)
#define DDSC_LOG_RECORD_RESET(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_RECORD_RESET_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*******************************************************************************
 *                              OBJECT RELATED
 ******************************************************************************/
/*e
 * \brief DDS PublicationBuiltinTopicData object
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PUBLICATIONDATA_OBJECT                     1

/*e
 * \brief DDS SubscriptionBuiltinTopicData object
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_SUBSCRIPTIONDATA_OBJECT                    2

/*e
 * \brief DDS ParticipantBuiltinTopicData object
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PARTICIPANTDATA_OBJECT                     3

/*e
 * \brief DDS DataReaderQos object
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DATAREADERQOS_OBJECT                       4

/*e
 * \brief DDS DataWriterQos object
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DATAWRITERQOS_OBJECT                       5

/*e
 * \brief DDS TopicQos object
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TOPICQOS_OBJECT                            6

/*e
 * \brief DDS PublisherQos object
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PUBLISHERQOS_OBJECT                        7

/*e
 * \brief DDS SubscriberQos object
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_SUBSCRIBERQOS_OBJECT                       8

/*e
 * \brief DDS DomainParticipantQos object
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PARTICIPANTQOS_OBJECT                      9

/*e
 * \brief DDS Entity object
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_ENTITY_OBJECT                             10

/*e
 * \brief DDS DomainParticipantFactoryQos object
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PARTICIPANTFACTORYQOS_OBJECT              11

/*e
 * \brief A DDS Type object
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TYPE_OBJECT                               12

/*e
 * \brief A NETIO BindResolver object
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_BINDRESOLVER_OBJECT                       13

/*e
 * \brief A NETIO RouteResolver object
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_ROUTERESOLVER_OBJECT                      14

/*e
 * \brief A NETIO AddressResolver object
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_ADDRESSRESOLVER_OBJECT                    15

/*e
 * \brief A NETIO DataWriterInterface object
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DATAWRITERIO_OBJECT                       16

/*e
 * \brief A NETIO DataReaderInterface object
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DATAREADERIO_OBJECT                       17

/*e
 * \brief A OSAPI Log object
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_LOG_OBJECT                                18

/*e
 * \brief A OSAPI system object
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_SYSTEM_OBJECT                             19

/*e
 * \brief A OSAPI mutex object
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_MUTEX_OBJECT                              20

/*e
 * \brief A DDS Waitset condition reference object
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_CONDREF_OBJECT                            21

/*e
 * \brief A DDS Condition waitset reference object
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_WSREF_OBJECT                              22

/*e
 * \brief A MD5 stream object
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_MD5STREAM_OBJECT                          23

/*e
 * \brief A DDS Condition object
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_CONDITION_OBJECT                          24

/*e
 * \brief A RT Condition object
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_RT_OBJECT                                 25

/*e
 * \brief A Participant pool object
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PARTICIPANT_POOL_OBJECT                   26

/*e
 * \brief A OSAPI_Timer object
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TIMER_OBJECT                              27

/*e
 * \brief A OSAPI_Timer timeout object
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TIMEROUT_OBJECT                           28

/*e
 * \brief A DDS Topic object
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TOPIC_OBJECT                              29

/*e
 * \brief A DDS WaitSet object
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_WAITSET_OBJECT                            30

/*e
 * \brief A UUID object
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_UUID_OBJECT                               31

/*e
 * \brief A REDA_MemPool object
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_MEMPOOL_OBJECT                            32

/*e
 * \brief A DomainParticipant's string properties sequence
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PARTICIPANT_PROPERTIES                    33

/*e
 * \brief A DomainParticipant's binary properties sequence
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PARTICIPANT_BINARY_PROPERTIES             34

/*e
 * \brief A DomainParticipant's built-in Publisher
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PARTICIPANT_BUILTIN_PUBLISHER             35

/*e
 * \brief A DomainParticipant's built-in Subscriber
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PARTICIPANT_BUILTIN_SUBSCRIBER            36

/*e
 * \brief A built-in Inter-Participant Channel data type object.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PARTICIPANT_GENERIC_MESSAGE_OBJECT        37

/*e
 * \brief A Trust data holder sequence object.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DATA_HOLDER_SEQ_OBJECT                    38

/*e
 * \brief A Property QoS policy object.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PROPERTYQOSPOLICY_OBJECT                  39

/*e
 * \brief A String object.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_STRING_OBJECT                             40

/*e
 * \brief An authentication pool object.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_AUTHPOOL_OBJECT                           41

/*e
 * \brief A Log buffer object.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_BUFFER_OBJECT                             42

/*e
 * \brief A String Manager object.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_STRINGMANAGER_OBJECT                      43

/*e
 * \brief Out of resources to initialize object of the specified kind.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_OBJECT_INITIALIZE_EC                      (DDSC_LOG_BASE + 200)
#define DDSC_LOG_OBJECT_INITIALIZE(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_OBJECT_INITIALIZE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief Out of resources to allocate an object of the specified kind.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_OBJECT_ALLOCATE_EC                        (DDSC_LOG_BASE + 201)
#define DDSC_LOG_OBJECT_ALLOCATE(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_OBJECT_ALLOCATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief Failed to finalize object of specified kind.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_OBJECT_FINALIZE_EC                        (DDSC_LOG_BASE + 202)
#define DDSC_LOG_OBJECT_FINALIZE(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_OBJECT_FINALIZE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief Failed to delete object of specified kind.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_OBJECT_DELETE_EC                          (DDSC_LOG_BASE + 203)
#define DDSC_LOG_OBJECT_DELETE(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_OBJECT_DELETE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief Failed to copy object of specified kind.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_OBJECT_COPY_EC                            (DDSC_LOG_BASE + 204)
#define DDSC_LOG_OBJECT_COPY(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_OBJECT_COPY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief Failed to delete/finalize an object because other objects are
 *        referencing it.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_OBJECT_REFCOUNT_EC                        (DDSC_LOG_BASE + 205)
#define DDSC_LOG_OBJECT_REFCOUNT(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_OBJECT_REFCOUNT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief Failed to get the object properties.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_OBJECT_GET_PROPERTY_EC                    (DDSC_LOG_BASE + 206)
#define DDSC_LOG_OBJECT_GET_PROPERTY(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_OBJECT_GET_PROPERTY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief Failed to set the object properties.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_OBJECT_SET_PROPERTY_EC                    (DDSC_LOG_BASE + 207)
#define DDSC_LOG_OBJECT_SET_PROPERTY(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_OBJECT_SET_PROPERTY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief An object is empty, typically applies only to buffer-pool objects
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_OBJECT_EMPTY_EC                           (DDSC_LOG_BASE + 208)
#define DDSC_LOG_OBJECT_EMPTY(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_OBJECT_EMPTY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief Failed to delete/finalize an object because other objects are
 *        using it.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_OBJECT_INUSECOUNT_EC                      (DDSC_LOG_BASE + 209)
#define DDSC_LOG_OBJECT_INUSECOUNT(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_OBJECT_INUSECOUNT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief Failed to create an object
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_OBJECT_CREATE_EC                          (DDSC_LOG_BASE + 210)
#define DDSC_LOG_OBJECT_CREATE(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_OBJECT_CREATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*******************************************************************************
 *                              SEQUENCE RELATED
 ******************************************************************************/

/*e
 * \brief A NETIO Netmask sequence
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_NETMASK_SEQUENCE                                         1

/*e
 * \brief A NETIO Route sequence
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_ROUTE_SEQUENCE                                           2

/*e
 * \brief A NETIO Reserved address sequence
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_RESERVED_SEQUENCE                                        3

/*e
 * \brief A DDS sequence of enabled user NETIO transports
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_ENABLED_USER_TRANSPORT_SEQUENCE                          4

/*e
 * \brief A DDS sequence of enabled transports
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_ENABLED_TRANSPORT_SEQUENCE                               5

/*e
 * \brief A DDS sequence of enabled discovery transports
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_ENABLED_DISCVOERY_TRANSPORT_SEQUENCE                     6

/*e
 * \brief A DDS sequence of initial peers
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_INITIAL_PEER_SEQUENCE                                    7

/*e
 * \brief A NETIO sequence of destinations to send to
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DESTINATION_SEQUENCE                                     8

/*e
 * \brief A DDS sequence of meta-traffic unicast locators
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_METAUNICAST_SEQUENCE                                     9

/*e
 * \brief A DDS sequence of meta-traffic multicast locators
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_METAMULTICAST_SEQUENCE                                  10

/*e
 * \brief A DDS sequence of user-traffic unicast locators
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_USERUNICAST_SEQUENCE                                    11

/*e
 * \brief A DDS sequence of user-traffic multicast locators
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_USERMULTICAST_SEQUENCE                                  12

/*e
 * \brief A DDS sequence used in a read/take call
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_READTAKE_SEQUENCE                                       13

/*e
 * \brief A DDS sequence of DataHolder objects
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DATAHOLDER_SEQUENCE                                     14

/*e
 * \brief A cooke octet sequence
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_COOKIE_PAYLOAD_SEQUENCE                                 15

/*e
 *  \brief a sequence for data representation IDs
 *
 *  \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_REPRESENTATION_ID_SEQUENCE                              16

/*e
 * \brief Failed to set the maximum length of a sequence of the specified kind.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_SEQUENCE_SETMAX_EC                        (DDSC_LOG_BASE + 300)
#define DDSC_LOG_SEQ_SETMAX(level_,kind_,length_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_SEQUENCE_SETMAX_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "kind",(kind_),"length",(RTI_INT32)(length_))

/*e
 * \brief Failed to set the length of a sequence of the specified kind.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_SEQUENCE_SETLENGTH_EC                     (DDSC_LOG_BASE + 301)
#define DDSC_LOG_SEQ_SETLENGTH(level_,kind_,length_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_SEQUENCE_SETLENGTH_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
        "kind",(kind_),"length",(length_))

/*e
 * \brief Failed to get a reference at the specified index for a sequence of
 *        the specified kind.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_SEQUENCE_GETREF_EC                        (DDSC_LOG_BASE + 302)
#define DDSC_LOG_SEQ_GETREF(level_,kind_,index_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_SEQUENCE_GETREF_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
        "kind",(kind_),"index",(index_))

/*e
 * \brief Failed to initialize a sequence of the specified kind.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_SEQUENCE_INITIALIZE_EC                    (DDSC_LOG_BASE + 303)
#define DDSC_LOG_SEQ_INITIALIZE(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_SEQUENCE_INITIALIZE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief Failed to finalize a sequence of the specified kind.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_SEQUENCE_FINALIZE_EC                      (DDSC_LOG_BASE + 304)
#define DDSC_LOG_SEQ_FINALIZE(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_SEQUENCE_FINALIZE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief Failed to copy a sequence of the specified kind.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_SEQUENCE_COPY_EC                          (DDSC_LOG_BASE + 305)
#define DDSC_LOG_SEQ_COPY(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_SEQUENCE_COPY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief The sequence of the specified kind was invalid in the context it is
 *        used.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_SEQUENCE_INVALID_EC                       (DDSC_LOG_BASE + 306)
#define DDSC_LOG_SEQ_INVALID(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_SEQUENCE_INVALID_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*******************************************************************************
 *                              COMPONENT RELATED
 ******************************************************************************/
/*e
 * \brief A component of discovery kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DISCOVERY_COMPONENT                                        1

/*e
 * \brief A component of RTPS kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_RTPS_COMPONENT                                             2

/*e
 * \brief A component of reader-history kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_READERHISTORY_COMPONENT                                    3

/*e
 * \brief A component of writer-history kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_WRITERHISTORY_COMPONENT                                    4

/*e
 * \brief A component of DataReaderInterface kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DATAREADERIO_COMPONENT                                     5

/*e
 * \brief A component of DataWriterInterface kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DATAWRITERIO_COMPONENT                                     6

/*e
 * \brief A component of NETIO kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_NETIO_COMPONENT                                            7

/*e
 * \brief A component of transport kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRANSPORT_COMPONENT                                        8

/*e
 * \brief A component of application generation kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_APPGEN_COMPONENT                                           9

/*e
 * \brief Did not find a component factory with the given name in the registry
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_COMPONENT_LOOKUP_EC                       (DDSC_LOG_BASE + 400)
#define DDSC_LOG_COMPONENT_LOOKUP(level_,factory_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDSC_LOG_COMPONENT_LOOKUP_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"factory",(factory_))

/*e
 * \brief Could not create a component of the specified kind using the specified
 *        factory
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_COMPONENT_CREATE_EC                       (DDSC_LOG_BASE + 401)
#define DDSC_LOG_COMPONENT_CREATE(level_,kind_,factory_) \
OSAPI_LOG_ENTRY_ADD_1STRING_1INT((level_),DDSC_LOG_COMPONENT_CREATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"factory",(factory_),"kind",(kind_))

/*e
 * \brief Could not delete a component of the specified kind using the specified
 *        factory
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_COMPONENT_DELETE_EC                       (DDSC_LOG_BASE + 402)
#define DDSC_LOG_COMPONENT_DELETE(level_,factory_,kind_) \
OSAPI_LOG_ENTRY_ADD_1STRING_1INT((level_),DDSC_LOG_COMPONENT_DELETE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
        "factory",(factory_),"kind",(kind_))


/*******************************************************************************
 *                              QOS & LISTENER RELATED
 ******************************************************************************/
/*e
 * \brief Exceeded resource limits for writer history queue.
 *
 * \details
 *  A DataWriter failed to get a queue entry for a new sample it is
 *  attempting to send, due to exceeding a limit:
 *      - When the sample is for new instance:
 *          DataWriterQos.resource_limits.max_instances may have been exceeded
 *      - When the sample is for an existing instance, and History kind is
 *        KEEP_ALL:
 *          DataWriterQos.resource_limits.max_samples_per_instance
 *          may have been exceeded.
 *      - The limit on the total number of samples in the queue:
 *          DataWriterQos.resource_limits.max_samples, may have been exceeded.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_HISTORY_RESOURCE        1

/*e
 * \brief Failed to get resource for new sample due to resource limit.
 *
 * \details
 *  A DataWriter failed to get a buffer for a new sample being written
 *  because the limit DataWriterQos.resource_limits.max_samples was
 *  exceeded.
 *
 *  A DataReader failed in getting a buffer for a newly received sample
 *  because DataReaderQos.resource_limits.max_samples was exceeded.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_SAMPLE_RESOURCES        2

/*e
 * \brief Failed to allocate a new participant
 *
 * \details
 * The DomainParticipantFactory failed to get a buffer for a new
 * DomainParticipant because
 * DomainParticipantFactoryQos.system_resource.max_participants
 * because was exceeded.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PARTICIPANT_RESOURCES   3

/*e
 * \brief Could not allocate a resource of the specified kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_RESOURCE_EXCEEDED_EC                      (DDSC_LOG_BASE + 500)
#define DDSC_LOG_RESOURCE_EXCEEDED(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_RESOURCE_EXCEEDED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

#if DOXYGEN_DOCUMENTATION_ONLY
/*e \brief Kinds of QoS policies and QoS objects
 */
typedef enum
{
    /*e
     * \brief The TopicQos kind (1)
     * \ingroup DDSCLogCodesClass
     */
    DDSC_LOG_TOPIC_QOS = 1,

    /*e
     * \brief The DataReaderQos kind (2)
     * \ingroup DDSCLogCodesClass
     */
    DDSC_LOG_DATAREADER_QOS = 2,

    /*e
     * \brief The DataWriterQos kind (3)
     * \ingroup DDSCLogCodesClass
     */
    DDSC_LOG_DATAWRITER_QOS = 3,

    /*e
     * \brief The SubscriberQos kind (4)
     * \ingroup DDSCLogCodesClass
     */
    DDSC_LOG_SUBSCRIBER_QOS = 4,

    /*e
     * \brief The PublisherQos kind (5)
     * \ingroup DDSCLogCodesClass
     */
    DDSC_LOG_PUBLISHER_QOS = 5,

    /*e
     * \brief The DomainParticipantQos kind (6)
     * \ingroup DDSCLogCodesClass
     */
    DDSC_LOG_PARTICIPANT_QOS = 6,

    /*e
     * \brief The DomainParticipantFactoryQos kind (7)
     * \ingroup DDSCLogCodesClass
     */
    DDSC_LOG_PARTICIPANTFACTORY_QOS = 7,

    /*e
     * \brief The default TopicQos kind (8)
     * \ingroup DDSCLogCodesClass
     */
    DDSC_LOG_DEFAULTTOPIC_QOS = 8,

    /*e
     * \brief The default DataReaderQos kind (9)
     * \ingroup DDSCLogCodesClass
     */
    DDSC_LOG_DEFAULTDATAREADER_QOS = 9,

    /*e
     * \brief The default DataWriterQos kind (10)
     * \ingroup DDSCLogCodesClass
     */
    DDSC_LOG_DEFAULTDATAWRITER_QOS = 10,

    /*e
     * \brief The default SubscriberQos kind (11)
     * \ingroup DDSCLogCodesClass
     */
    DDSC_LOG_DEFAULTSUBSCRIBER_QOS = 11,

    /*e
     * \brief The default PublisherQos kind (12)
     * \ingroup DDSCLogCodesClass
     */
    DDSC_LOG_DEFAULTPUBLISHER_QOS = 12,

    /*e
     * \brief The default ParticipantQos kind (13)
     * \ingroup DDSCLogCodesClass
     */
    DDSC_LOG_DEFAULTPARTICIPANT_QOS = 13,

    /*e
     * \brief The deadline qos policy kind (14)
     * \ingroup DDSCLogCodesClass
     */
    DDSC_LOG_DEADLINE_QOS_POLICY = 14,

    /*e
     * \brief The liveliness qos policy kind (15)
     * \ingroup DDSCLogCodesClass
     */
    DDSC_LOG_LIVELINESS_QOS_POLICY = 15,

    /*e
     * \brief The history qos policy kind (16)
     * \ingroup DDSCLogCodesClass
     */
    DDSC_LOG_HISTORY_QOS_POLICY = 16,

    /*e
     * \brief The resource limits qos policy kind (17)
     * \ingroup DDSCLogCodesClass
     */
    DDSC_LOG_RESOURCE_LIMIT_QOS_POLICY = 17,

    /*e
     * \brief The protocol qos policy kind (18)
     * \ingroup DDSCLogCodesClass
     */
    DDSC_LOG_PROTOCOL_QOS_POLICY = 18,

    /*e
     * \brief The type-support qos policy kind (19)
     * \ingroup DDSCLogCodesClass
     */
    DDSC_LOG_TYPE_SUPPORT_QOS_POLICY = 19,

    /*e
     * \brief The reliability qos policy kind (20)
     * \ingroup DDSCLogCodesClass
     */
    DDSC_LOG_RELIABILITY_QOS_POLICY = 20,

    /*e
     * \brief The durability qos policy kind (21)
     * \ingroup DDSCLogCodesClass
     */
    DDSC_LOG_DURABILITY_QOS_POLICY = 21,

    /*e
     * \brief The ownership qos policy kind (22)
     * \ingroup DDSCLogCodesClass
     */
    DDSC_LOG_OWNERSHIP_QOS_POLICY = 22,

    /*e
     * \brief The ownership-strength qos policy kind (23)
     * \ingroup DDSCLogCodesClass
     */
    DDSC_LOG_OWNERSHIP_STRENGTH_QOS_POLICY = 23,

    /*e
     * \brief The transport qos policy kind (24)
     * \ingroup DDSCLogCodesClass
     */
    DDSC_LOG_TRANSPORT_QOS_POLICY = 24,

    /*e
     * \brief The participant id qos policy kind (25)
     * \ingroup DDSCLogCodesClass
     */
    DDSC_LOG_PARTICIPANT_ID_QOS_POLICY = 25,

    /*e
     * \brief The heartbeat qos policy kind (26)
     * \ingroup DDSCLogCodesClass
     *
     * \details When configured for reliable communication,
     * heartbeats_per_max_samples must be fit within max_samples
     */
    DDSC_LOG_HEARTBEATS_QOS_POLICY = 26,

    /*e
     * \brief The DataWriterQos writer_resource_limits policy (27)
     *
     * \details An invalid value has been set for a limit of
     *          DataWriterQos.writer_resource_limits.  Each value must be positive
     *          and finite.
     *          May be logged by Discovery writers with no initial_peers set.
     *
     * \ingroup DDSCLogCodesClass
     */
    DDSC_LOG_DATAWRITER_RESOURCE_QOS_POLICY = 27,

    /*e
     * \brief The DataReaderQos reader_resource_limits policy (28)
     *
     * \ingroup DDSCLogCodesClass
     */
    DDSC_LOG_DATAREADER_RESOURCE_QOS_POLICY = 28,

    /*e
     * \brief The DestinationOrderQos destination_order policy (29)
     *
     * \ingroup DDSCLogCodesClass
     */
    DDSC_LOG_DESTINATION_ORDER_POLICY = 29,

    /*e
     * \brief The PropertyQos policy (30)
     *
     * \ingroup DDSCLogCodesClass
     */
    DDSC_LOG_PROPERTY_QOS_POLICY     = 30,

    /*e
     * \brief The PublishModeQos policy (31)
     *
     * \ingroup DDSCLogCodesClass
     */
    DDSC_LOG_PUBLISH_MODE_QOS_POLICY  = 31,

    /*
     * \brief The DataTagQos policy (32)
     *
     * \ingroup DDSCLogCodesClass
     */
    DDSC_LOG_DATA_TAG_QOS_POLICY      = 32,

    /*e
     * \brief The DataRepresentationQos policy (33)
     *
     * \ingroup DDSCLogCodesClass
     */
    DDSC_LOG_DATA_REPRESENTATION_QOS_POLICY   = 33,

    /*e
     * \brief The LatencyBudgetQos policy (34)
     *
     * \ingroup DDSCLogCodesClass
     */
    DDSC_LOG_LATENCY_BUDGET_QOS_POLICY      = 34

    /*e
     * \brief The ContentFilterQos policy (35)
     *
     * \ingroup DDSCLogCodesClass
     */
    #define DDSC_LOG_CONTENT_FILTER_QOS_POLICY = 35
} QosPolicyKind;
#else
/*e
 * \brief The TopicQos kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TOPIC_QOS                                    1

/*e
 * \brief The DataReaderQos kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DATAREADER_QOS                               2

/*e
 * \brief The DataWriterQos kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DATAWRITER_QOS                               3

/*e
 * \brief The SubscriberQos kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_SUBSCRIBER_QOS                               4

/*e
 * \brief The PublisherQos kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PUBLISHER_QOS                                5

/*e
 * \brief The DomainParticipantQos kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PARTICIPANT_QOS                              6

/*e
 * \brief The DomainParticipantFactoryQos kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PARTICIPANTFACTORY_QOS                       7

/*e
 * \brief The default TopicQos kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DEFAULTTOPIC_QOS                             8

/*e
 * \brief The default DataReaderQos kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DEFAULTDATAREADER_QOS                        9

/*e
 * \brief The default DataWriterQos kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DEFAULTDATAWRITER_QOS                       10

/*e
 * \brief The default SubscriberQos kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DEFAULTSUBSCRIBER_QOS                       11

/*e
 * \brief The default PublisherQos kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DEFAULTPUBLISHER_QOS                        12

/*e
 * \brief The default PublisherQos kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DEFAULTPARTICIPANT_QOS                      13

/*e
 * \brief The deadline qos policy kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DEADLINE_QOS_POLICY                         14

/*e
 * \brief The liveliness qos policy kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_LIVELINESS_QOS_POLICY                       15

/*e
 * \brief The history qos policy kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_HISTORY_QOS_POLICY                          16

/*e
 * \brief The resource limits qos policy kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_RESOURCE_LIMIT_QOS_POLICY                   17

/*e
 * \brief The protocol qos policy kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PROTOCOL_QOS_POLICY                         18

/*e
 * \brief The type-support qos policy kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TYPE_SUPPORT_QOS_POLICY                     19

/*e
 * \brief The reliability qos policy kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_RELIABILITY_QOS_POLICY                      20

/*e
 * \brief The durability qos policy kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DURABILITY_QOS_POLICY                       21

/*e
 * \brief The ownership qos policy kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_OWNERSHIP_QOS_POLICY                        22

/*e
 * \brief The ownership-strength qos policy kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_OWNERSHIP_STRENGTH_QOS_POLICY               23

/*e
 * \brief The transport qos policy kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRANSPORT_QOS_POLICY                        24

/*e
 * \brief The participant id qos policy kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PARTICIPANT_ID_QOS_POLICY                   25

/*e
 * \brief The heartbeat qos policy kind
 * \ingroup DDSCLogCodesClass
 *
 * \details When configured for reliable communication,
 * heartbeats_per_max_samples must be fit within max_samples
 */
#define DDSC_LOG_HEARTBEATS_QOS_POLICY                       26

/*e
 * \brief The DataWriterQos writer_resource_limits policy
 *
 * \details An invalid value has been set for a limit of
 *          DataWriterQos.writer_resource_limits.  Each value must be positive
 *          and finite.
 *          May be logged by Discovery writers with no initial_peers set.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DATAWRITER_RESOURCE_QOS_POLICY              27

/*e
 * \brief The DataReaderQos reader_resource_limits policy
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DATAREADER_RESOURCE_QOS_POLICY              28


/*e
 * \brief The DestinationOrderQos destination_order policy
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DESTINATION_ORDER_POLICY                    29

/*e
 * \brief The PropertyQos policy
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PROPERTY_QOS_POLICY                         30

/*e
 * \brief The PublishModeQos policy
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PUBLISH_MODE_QOS_POLICY                     31

/*
 * \brief The DataTagQos policy
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DATA_TAG_QOS_POLICY                         32

/*e
 * \brief The DataRepresentationQos policy
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DATA_REPRESENTATION_QOS_POLICY              33

/*e
 * \brief The LatencyBudgetQos policy
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_LATENCY_BUDGET_QOS_POLICY                   34

#if DDS_FILTERING_ENABLED
/*e
 * \brief The ContentFilterQos policy
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_CONTENT_FILTER_QOS_POLICY                  35
#endif /* DDS_FILTERING_ENABLED */

#endif /* DOXYGEN_DOCUMENTATION_ONLY */

/*e
 * \brief An inconsistent Qos policy for the specified
          \ref QosPolicyKind "kind" was found.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_QOS_INCONSISTENT_POLICY_EC                (DDSC_LOG_BASE + 501)
#define DDSC_LOG_QOS_INCONSISTENT_POLICY(level_,kind_,policy_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_QOS_INCONSISTENT_POLICY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "kind",(kind_),"policy",(policy_))

/*e
 * \brief An inconsistency between two Qos policies for the specified
 *        \ref QosPolicyKind "kind" was found.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_QOS_INCONSISTENT_POLICIES_EC              (DDSC_LOG_BASE + 502)
#define DDSC_LOG_QOS_INCONSISTENT_POLICIES(level_,kind_,policy1_,policy2_) \
OSAPI_LOG_ENTRY_ADD_3INT((level_),DDSC_LOG_QOS_INCONSISTENT_POLICIES_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "kind",(kind_),"policy1",(policy1_),"policy2",(policy2_))
/*e
 * \brief Failed to create an entity or set a qos due to inconsistent policy
 *        for \ref QosPolicyKind "kind".
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_QOS_INCONSISTENT_EC                       (DDSC_LOG_BASE + 503)
#define DDSC_LOG_QOS_INCONSISTENT(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_QOS_INCONSISTENT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief Failed to copy a Qos of the specified \ref QosPolicyKind "kind".
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_QOS_COPY_EC                               (DDSC_LOG_BASE + 504)
#define DDSC_LOG_QOS_COPY(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_QOS_COPY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"kind",(kind_))

/*e
 * \brief Failed to initialize a Qos of the specified \ref QosPolicyKind "kind".
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_QOS_INITIALIZE_EC                         (DDSC_LOG_BASE + 505)
#define DDSC_LOG_QOS_INITIALIZE(level_,kind_,ddsrc_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_QOS_INITIALIZE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "kind",(kind_),"ddsrc",(ddsrc_))

/*e
 * \brief Failed to finalize a Qos of the specified \ref QosPolicyKind "kind".
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_QOS_FINALIZE_EC                           (DDSC_LOG_BASE + 506)
#define DDSC_LOG_QOS_FINALIZE(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_QOS_FINALIZE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"kind",(kind_))

/*e
 * \brief Failed to set a Qos of the specified \ref QosPolicyKind "kind".
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_QOS_SET_EC                                (DDSC_LOG_BASE + 507)
#define DDSC_LOG_QOS_SET(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_QOS_SET_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"kind",(kind_))

/*e
 * \brief Failed to set a Qos of the specified \ref QosPolicyKind "kind"
 *        because the entity is already enabled.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_QOS_SET_ON_ENABLED_EC                     (DDSC_LOG_BASE + 508)
#define DDSC_LOG_QOS_SET_ON_ENABLED(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_QOS_SET_ON_ENABLED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief Failed to set a Qos of the specified for \ref QosPolicyKind "kind",
          because the immutable Qos policies have been changed.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_QOS_IMMUTABLE_EC                          (DDSC_LOG_BASE + 509)
#define DDSC_LOG_QOS_IMMUTABLE(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_QOS_IMMUTABLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief A discovered Qos of the specified \ref QosPolicyKind "kind"
 *  changed (the entity already existed).
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_QOS_CHANGED_EC                            (DDSC_LOG_BASE + 510)
#define DDSC_LOG_QOS_CHANGED(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_QOS_CHANGED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief Failed to get a Qos of the specified \ref QosPolicyKind "kind".
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_QOS_GET_EC                                (DDSC_LOG_BASE + 511)
#define DDSC_LOG_QOS_GET(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_QOS_GET_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief The topic listener kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TOPIC_LISTENER                                        1

/*e
 * \brief The datareader listener kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DATAREADER_LISTENER                                   2

/*e
 * \brief The datawriter listener kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DATAWRITER_LISTENER                                   3

/*e
 * \brief The subscriber listener kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_SUBSCRIBER_LISTENER                                   4

/*e
 * \brief The publisher listener kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PUBLISHER_LISTENER                                    5

/*e
 * \brief The participant listener kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PARTICIPANT_LISTENER                                  6

/*e
 * \brief The participant-factory listener kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PARTICIPANTFACTORY_LISTENER                           7

/*e
 * \brief Failed to create an entity due to inconsistent listener and
 *        status mask.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_LISTENER_INCONSISTENT_EC                  (DDSC_LOG_BASE + 512)
#define DDSC_LOG_LISTENER_INCONSISTENT(level_,kind_,mask_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_LISTENER_INCONSISTENT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "kind",(kind_),"mask",(RTI_INT32)(mask_))

/*e
 * \brief Failed to set the listener of the specified kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_LISTENER_SET_EC                           (DDSC_LOG_BASE + 513)
#define DDSC_LOG_LISTENER_SET(level_,kind_,mask_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_LISTENER_SET_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
        "kind",(kind_),"mask",(mask_))

/*e
 * \brief Failed to get the listener of the specified kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_LISTENER_GET_EC                           (DDSC_LOG_BASE + 514)
#define DDSC_LOG_LISTENER_GET(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_LISTENER_GET_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "kind",(kind_))

/*e
 * \brief Illegal combination of NULL listener and non-NONE status mask when
 *        setting a listener for an Entity
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_LISTENER_SET_ILLEGAL_NULL_EC              (DDSC_LOG_BASE + 515)
#define DDSC_LOG_SET_LISTENER_ILLEGAL_NULL(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_LISTENER_SET_ILLEGAL_NULL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*******************************************************************************
 *                              ENTITY RELATED
 ******************************************************************************/

/*e
 * \brief The topic entity kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TOPIC_ENTITY                                        1

/*e
 * \brief The datawriter entity kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DATAWRITER_ENTITY                                   2

/*e
 * \brief The datareader entity kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DATAREADER_ENTITY                                   3

/*e
 * \brief The publisher entity kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PUBLISHER_ENTITY                                    4

/*e
 * \brief The subscriber entity kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_SUBSCRIBER_ENTITY                                   5

/*e
 * \brief The participant entity kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PARTICIPANT_ENTITY                                  6

/*e
 * \brief The publication entity kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PUBLICATION_ENTITY                                  7

/*e
 * \brief The subscription entity kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_SUBSCRIPTION_ENTITY                                 8

/*e
 * \brief Failed to enable an entity of the specified kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_ENTITY_ENABLE_EC                          (DDSC_LOG_BASE + 600)
#define DDSC_LOG_ENTITY_ENABLE(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_ENTITY_ENABLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief Failed to an delete/finalize an entity of the specified kind because
 *        it is not empty.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_ENTITY_NOT_EMPTY_EC                       (DDSC_LOG_BASE + 601)
#define DDSC_LOG_ENTITY_NOT_EMPTY(level_,kind_,count_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_ENTITY_NOT_EMPTY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_),"count",(count_))

/*e
 * \brief Failed to finalize an entity of the specified kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_ENTITY_FINALIZE_EC                        (DDSC_LOG_BASE + 602)
#define DDSC_LOG_ENTITY_FINALIZE(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_ENTITY_FINALIZE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"kind",(kind_))

/*e
 * \brief Failed to initialize an entity of the specified kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_ENTITY_INITIALIZE_EC                      (DDSC_LOG_BASE + 603)
#define DDSC_LOG_ENTITY_INITIALIZE(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_ENTITY_INITIALIZE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"kind",(kind_))

/*e
 * \brief An operation was attempted on an entity that is not enabled
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_ENTITY_NOT_ENABLED_EC                     (DDSC_LOG_BASE + 604)
#define DDSC_LOG_ENTITY_NOT_ENABLED(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_ENTITY_NOT_ENABLED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief Entities are in different factories
 *
 * \details
 *
 * A entity tried to use an entity created by a different factory.
 * For example, it is illegal to create a datawriter using a topic
 * from a different participant.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_ENTITY_DIFFERENT_FACTORY_EC               (DDSC_LOG_BASE + 605)
#define DDSC_LOG_ENTITY_DIFFERENT_FACTORY(level_,factory_kind_,entity_kind_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_ENTITY_DIFFERENT_FACTORY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
        "factory_kind",(factory_kind_),"entity_kind",(entity_kind_))

/*e
 * \brief An invalid property was specified for the entity.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_ENTITY_INVALID_PROPERTY_EC               (DDSC_LOG_BASE + 606)
#define DDSC_LOG_ENTITY_INVALID_PROPERTY(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_ENTITY_INVALID_PROPERTY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))
/*******************************************************************************
 *                              CDR RELATED
 ******************************************************************************/
/*e
 * \brief The datawriter CDR kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DATAWRITER_CDR                                      1

/*e
 * \brief The datareader CDR kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DATAREADER_CDR                                      2

/*e
 * \brief The datawriter inline kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DATAWRITER_INLINE                                   3


/*e
 * \brief Failed to allocate a pool of the specified kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_CDR_POOL_ALLOC_EC                         (DDSC_LOG_BASE + 700)
#define DDSC_LOG_CDR_POOL_ALLOC(level_,kind_,size_,count_) \
OSAPI_LOG_ENTRY_ADD_3INT((level_),DDSC_LOG_CDR_POOL_ALLOC_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "kind",(kind_),"size",(RTI_INT32)(size_),"count",(RTI_INT32)(count_))

/*e
 * \brief Failed to set the CDR buffer for a packet
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_CDR_BUFFER_SET_EC                         (DDSC_LOG_BASE + 701)
#define DDSC_LOG_CDR_BUFFER_SET(level_,kind_,stream_,buf_,length_) \
OSAPI_LOG_ENTRY_CREATE((level_),DDSC_LOG_CDR_BUFFER_SET_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_INT("kind",(kind_),RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_POINTER("stream",(stream_),RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_POINTER("buf",(buf_),RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_INT("length",(RTI_INT32)(length_),RTI_TRUE)

/*e
 * \brief Failed to delete the CDR pool
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_CDR_POOL_DELETE_EC                        (DDSC_LOG_BASE + 702)
#define DDSC_LOG_CDR_POOL_DELETE(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_CDR_POOL_DELETE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief Failed to serialize a parameter ID
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_CDR_SERIALIZE_PID_EC                      (DDSC_LOG_BASE + 703)
#define DDSC_LOG_CDR_SERIALIZE_PID(level_,kind_,pid_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_CDR_SERIALIZE_PID_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_),"pid",(pid_))

/*e
 * \brief Failed to serialize a parameter length
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_CDR_SERIALIZE_PID_LENGTH_EC               (DDSC_LOG_BASE + 704)
#define DDSC_LOG_CDR_SERIALIZE_PID_LENGTH(level_,kind_,length_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_CDR_SERIALIZE_PID_LENGTH_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
        "kind",(kind_),"length",(length_))

/*e
 * \brief Failed to serialize a key-hash
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_CDR_SERIALIZE_KEYHASH_EC                  (DDSC_LOG_BASE + 705)
#define DDSC_LOG_CDR_SERIALIZE_KEYHASH(level_,kind_,length_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_CDR_SERIALIZE_KEYHASH_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
        "kind",(kind_),"length",(RTI_INT32)(length_))

/*e
 * \brief Failed to serialize payload data
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_CDR_SERIALIZE_DATA_EC                     (DDSC_LOG_BASE + 706)
#define DDSC_LOG_CDR_SERIALIZE_DATA(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_CDR_SERIALIZE_DATA_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief Deserialized an invalid parameter length for a specific parameter ID
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DESERIALIZE_BAD_PID_LENGTH_EC             (DDSC_LOG_BASE + 707)
#define DDSC_LOG_DESERIALIZE_BAD_PID_LENGTH(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_DESERIALIZE_BAD_PID_LENGTH_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief Failed to deserialize the ID of an inline parameter
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_CDR_DESERIALIZE_PID_EC                    (DDSC_LOG_BASE + 708)
#define DDSC_LOG_CDR_DESERIALIZE_PID(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_CDR_DESERIALIZE_PID_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief Failed to deserialize the length of an inline parameter
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_CDR_DESERIALIZE_PID_LENGTH_EC             (DDSC_LOG_BASE + 709)
#define DDSC_LOG_CDR_DESERIALIZE_PID_LENGTH(level_,kind_,pid_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_CDR_DESERIALIZE_PID_LENGTH_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "kind",(kind_),"pid",(pid_))

/*e
 * \brief Failed to increment to the position of the next inline parameter
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_CDR_INCREMENT_POS_EC                      (DDSC_LOG_BASE + 710)
#define DDSC_LOG_CDR_INCREMENT_POS(level_,kind_,adjust_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_CDR_INCREMENT_POS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
        "kind",(kind_),"adjust",(RTI_INT32)(adjust_))

/*e
 * \brief Failed to set the reception stream position
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_CDR_SET_POS_EC                            (DDSC_LOG_BASE + 711)
#define DDSC_LOG_CDR_SET_POS(level_,kind_,pos_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_CDR_SET_POS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
        "kind",(kind_),"pos",(RTI_INT32)(pos_))

/*e
 * \brief Failed to deserialize the encapsulation header
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_CDR_DESERIALIZE_HEADER_EC                 (DDSC_LOG_BASE + 712)
#define DDSC_LOG_CDR_DESERIALIZE_HEADER(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_CDR_DESERIALIZE_HEADER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief Failed to deserialize CDR payload data
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_CDR_DESERIALIZE_DATA_EC                   (DDSC_LOG_BASE + 713)
#define DDSC_LOG_CDR_DESERIALIZE_DATA(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_CDR_DESERIALIZE_DATA_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"kind",(kind_))

/*e
 * \brief Failed to initialize CDR sample
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_CDR_INITIALIZE_SAMPLE_EC                  (DDSC_LOG_BASE + 714)
#define DDSC_LOG_CDR_INITIALIZE_SAMPLE(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_CDR_INITIALIZE_SAMPLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"kind",(kind_))

/*e
 * \brief Failed to finalize sample
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_CDR_FINALIZE_SAMPLE_EC                    (DDSC_LOG_BASE + 715)
#define DDSC_LOG_CDR_FINALIZE_SAMPLE(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_CDR_FINALIZE_SAMPLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief Failed to serialize the status info parameter
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_CDR_SERIALIZE_STATUS_INFO_EC              (DDSC_LOG_BASE + 716)
#define DDSC_LOG_CDR_SERIALIZE_STATUS_INFO(level_,kind_,info_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_CDR_SERIALIZE_STATUS_INFO_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_),"status",(RTI_INT32)(info_))

/*e
 * \brief Failed to deserialize a key-hash
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_CDR_DESERIALIZE_KEYHASH_EC                (DDSC_LOG_BASE + 717)
#define DDSC_LOG_CDR_DESERIALIZE_KEYHASH(level_,kind_,length_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_CDR_DESERIALIZE_KEYHASH_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
        "kind",(kind_),"length",(length_))

/*e
 * \brief Failed to deserialize CDR payload key
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_CDR_DESERIALIZE_KEY_EC                    (DDSC_LOG_BASE + 718)
#define DDSC_LOG_CDR_DESERIALIZE_KEY(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_CDR_DESERIALIZE_KEY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"kind",(kind_))




/*******************************************************************************
 *                              NETIO RELATED
 ******************************************************************************/
/*e
 * \brief Failed to add a route to an anonymous participant discovery
 *        datawriter
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_NETIO_ADD_ANON_TOPIC_ROUTE_EC             (DDSC_LOG_BASE + 800)
#define DDSC_LOG_NETIO_ADD_ANON_TOPIC_ROUTE(level_,topic_,oid_) \
OSAPI_LOG_ENTRY_ADD_1STRING_1INT((level_),DDSC_LOG_NETIO_ADD_ANON_TOPIC_ROUTE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "topic",(topic_),"object_id",(RTI_INT32)(oid_))

/*e
 * \brief Failed to add a route to a topic from a datawriter
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_NETIO_ADD_TOPIC_ROUTE_EC                  (DDSC_LOG_BASE + 801)
#define DDSC_LOG_NETIO_ADD_TOPIC_ROUTE(level_,topic_,oid_) \
OSAPI_LOG_ENTRY_ADD_1STRING_1INT((level_),DDSC_LOG_NETIO_ADD_TOPIC_ROUTE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "topic",(topic_),"object_id",(RTI_INT32)(oid_))

/*e
 * \brief Failed to delete a route to a topic
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_NETIO_DELETE_TOPIC_ROUTE_EC               (DDSC_LOG_BASE + 802)
#define DDSC_LOG_NETIO_DELETE_TOPIC_ROUTE(level_,topic_,oid_) \
OSAPI_LOG_ENTRY_ADD_1STRING_1INT((level_),DDSC_LOG_NETIO_DELETE_TOPIC_ROUTE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "topic",(topic_),"object_id",(RTI_INT32)(oid_))

/*e
 * \brief Failed to forward a topic
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_NETIO_FORWARD_TOPIC_EC                    (DDSC_LOG_BASE + 803)
#define DDSC_LOG_NETIO_FORWARD_TOPIC(level_,topic_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDSC_LOG_NETIO_FORWARD_TOPIC_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"topic",(topic_))


/*e \brief Any NETIO interface kind, typically UDP
 *   \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_NETIO_NETIO_KIND                                    1

/*e \brief The intra interface kind
 *   \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_INTRA_NETIO_KIND                                    2

/*e \brief The RTPS interface kind
 *   \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_RTPS_NETIO_KIND                                     3

/*e \brief The DataReader interface kind
 *   \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DATAREADER_NETIO_KIND                               4

/*e \brief The DataWriter interface kind
 *   \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DATAWRITER_NETIO_KIND                               5

/*e
 * \brief Failed to bind two external interface of the specified kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_NETIO_BIND_EXTERNAL_EC                    (DDSC_LOG_BASE + 804)
#define DDSC_LOG_NETIO_BIND_EXTERNAL(level_,from_,to_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_NETIO_BIND_EXTERNAL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"from",(from_),"to",(to_))

/*e
 * \brief Failed to unbind two external interfaces of the specified kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_NETIO_UNBIND_EXTERNAL_EC                  (DDSC_LOG_BASE + 805)
#define DDSC_LOG_NETIO_UNBIND_EXTERNAL(level_,from_,to_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_NETIO_UNBIND_EXTERNAL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"from",(from_),"to",(to_))

/*e
 * \brief Failed to bind an interface to a peer interface
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_NETIO_BIND_EC                             (DDSC_LOG_BASE + 806)
#define DDSC_LOG_NETIO_BIND(level_,src_,dst_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_NETIO_BIND_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"src",(src_),"dst",(dst_))

/*e
 * \brief Failed to unbind an interface from a peer interface
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_NETIO_UNBIND_EC                           (DDSC_LOG_BASE + 807)
#define DDSC_LOG_NETIO_UNBIND(level_,src_,dst_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_NETIO_UNBIND_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"src",(src_),"dst",(dst_))

/*e
 * \brief Failed to add a route from an interface to a peer interface
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_NETIO_ADD_ROUTE_EC                        (DDSC_LOG_BASE + 808)
#define DDSC_LOG_NETIO_ADD_ROUTE(level_,src_,dst_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_NETIO_ADD_ROUTE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"src",(src_),"dst",(dst_))

/*e
 * \brief  Failed to delete a route from an interface to a peer interface
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_NETIO_DELETE_ROUTE_EC                     (DDSC_LOG_BASE + 809)
#define DDSC_LOG_NETIO_DELETE_ROUTE(level_,src_,dst_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_NETIO_DELETE_ROUTE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"src",(src_),"dst",(dst_))

/*e
 * \brief Failed to get an external interface for the specified interface kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_NETIO_GET_EXTERNAL_INTF_EC                (DDSC_LOG_BASE + 810)
#define DDSC_LOG_NETIO_GET_EXTERNAL_INTF(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_NETIO_GET_EXTERNAL_INTF_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief A DataReader failed a bind due to no existing route
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_NETIO_NO_ROUTE_EC                         (DDSC_LOG_BASE + 811)
#define DDSC_LOG_NETIO_NO_ROUTE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_NETIO_NO_ROUTE_EC,\
                        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Lookup a route to a destination failed
 *
 * \details
 *
 * A failure was encountered when trying to lookup a route to a destination,
 * this log message is preceded by a more specific message. Failure to lookup
 * a route does not mean the route does not exist, it means it failed to
 * determine if a route did exist.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_NETIO_ROUTE_LOOKUP_FAILED_EC              (DDSC_LOG_BASE + 812)
#define DDSC_LOG_NETIO_ROUTE_LOOKUP_FAILED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_NETIO_ROUTE_LOOKUP_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to get the route table for an interface
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_NETIO_GET_ROUTE_TABLE_FAILED_EC           (DDSC_LOG_BASE + 813)
#define DDSC_LOG_NETIO_GET_ROUTE_TABLE_FAILED(level_,name_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDSC_LOG_NETIO_GET_ROUTE_TABLE_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"name",(name_))

/*e
 * \brief Failure when sending on an interface
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_NETIO_SEND_FAILED_EC                      (DDSC_LOG_BASE + 814)
#define DDSC_LOG_NETIO_SEND_FAILED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_NETIO_SEND_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to set an interface state
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_NETIO_SET_STATE_EC                        (DDSC_LOG_BASE + 815)
#define DDSC_LOG_NETIO_SET_STATE(level_,kind_,state_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_NETIO_SET_STATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "kind",(kind_),"state",(state_))

/*e
 * \brief Datawriter did not find a peer
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_NETIO_PEER_LOOKUP_EC                      (DDSC_LOG_BASE + 816)
#define DDSC_LOG_NETIO_PEER_LOOKUP(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_NETIO_PEER_LOOKUP_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Forced removal of sample downstream failed
 *
 * \details
 *
 * The datawriter failed to force a sample removal downstream
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_NETIO_FORCED_REMOVE_EC                    (DDSC_LOG_BASE + 817)
#define DDSC_LOG_NETIO_FORCED_REMOVE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_NETIO_FORCED_REMOVE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to initialize a packet
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PACKET_INIT_EC                            (DDSC_LOG_BASE + 818)
#define DDSC_LOG_PACKET_INIT(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_PACKET_INIT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief Failed to set the head of a packet
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PACKET_SET_HEAD_EC                        (DDSC_LOG_BASE + 819)
#define DDSC_LOG_PACKET_SET_HEAD(level_,kind_,adjust_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_PACKET_SET_HEAD_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_),"adjust",(adjust_))

/*e
 * \brief Failed to set the tail of a packet
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PACKET_SET_TAIL_EC                        (DDSC_LOG_BASE + 820)
#define DDSC_LOG_PACKET_SET_TAIL(level_,kind_,adjust_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_PACKET_SET_TAIL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_),"adjust",(adjust_))

/*e
 * \brief Configured discovery enabled transport is not valid
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_NETIO_DISCOVERY_ENABLED_TRANSPORT_EC      (DDSC_LOG_BASE + 821)
#define DDSC_LOG_NETIO_DISCOVERY_ENABLED_TRANSPORT(level_,name_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDSC_LOG_NETIO_DISCOVERY_ENABLED_TRANSPORT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"name",(name_))

/*e
 * \brief Failed to return a packet to the packet pool
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_NETIO_PACKETPOOL_RETURN_EC                (DDSC_LOG_BASE + 822)
#define DDSC_LOG_NETIO_PACKETPOOL_RETURN(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_NETIO_PACKETPOOL_RETURN_EC,\
    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to get a packet from the packet pool
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_NETIO_PACKETPOOL_GET_EC                   (DDSC_LOG_BASE + 823)
#define DDSC_LOG_NETIO_PACKETPOOL_GET(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_NETIO_PACKETPOOL_GET_EC,\
    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create packet pool
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_NETIO_PACKETPOOL_CREATE_EC                (DDSC_LOG_BASE + 824)
#define DDSC_LOG_NETIO_PACKETPOOL_CREATE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_NETIO_PACKETPOOL_CREATE_EC,\
    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to delete packet pool
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_NETIO_PACKETPOOL_DELETE_EC                (DDSC_LOG_BASE + 825)
#define DDSC_LOG_NETIO_PACKETPOOL_DELETE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_NETIO_PACKETPOOL_DELETE_EC,\
    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*******************************************************************************
 *                              DATAWRITER RELATED
 ******************************************************************************/
/*e
 * \brief Failed to ACKNACK sample in the writer history
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DW_ACKNACK_FAILED_EC                      (DDSC_LOG_BASE + 900)
#define DDSC_LOG_DW_ACKNACK_FAILED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_DW_ACKNACK_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM )

/*e
 * \brief Failed to commit a sample to the writer queue
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DW_COMMIT_EC                              (DDSC_LOG_BASE + 901)
#define DDSC_LOG_DW_COMMIT(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_DW_COMMIT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create keyhash of instance handle
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DW_KEYHASH_CREATE_EC                      (DDSC_LOG_BASE + 902)
#define DDSC_LOG_DW_KEYHASH_CREATE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_DW_KEYHASH_CREATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed a write due to an invalid key kind for the type being written
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DW_ILLEGAL_KEY_KIND_EC                    (DDSC_LOG_BASE + 903)
#define DDSC_LOG_DW_ILLEGAL_KEY_KIND(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_DW_ILLEGAL_KEY_KIND_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))


/*e
 * \brief Failed to create a typed writer
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DW_CREATE_TYPED_WRITER_EC                 (DDSC_LOG_BASE + 904)
#define DDSC_LOG_DW_CREATE_TYPED_WRITER(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_DW_CREATE_TYPED_WRITER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to register the key of an instance
 *
 * \details A DataWriter failed to register the key of a new instance
 *          because DataWriterQos.resource_limits.max_instances was exceeded.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DW_HISTORY_REGISTER_KEY_EC                (DDSC_LOG_BASE + 905)
#define DDSC_LOG_DW_HISTORY_REGISTER_KEY(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_DW_HISTORY_REGISTER_KEY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create a flow-controller
 *
 * \details A DataWriter failed to create the flow-controller specified in the
 *          Qos policy.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DW_UNKNOWN_FLOW_CONTROLLER_EC             (DDSC_LOG_BASE + 906)
#define DDSC_LOG_DW_UNKNOWN_FLOW_CONTROLLER(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_DW_UNKNOWN_FLOW_CONTROLLER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief The data-sample is larger then supported by the transport, but the
 *        flow-controller has been compiled out.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DW_FLOW_CONTROLLER_REQUIRED_EC            (DDSC_LOG_BASE + 907)
#define DDSC_LOG_DW_FLOW_CONTROLLER_REQUIRED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_DW_FLOW_CONTROLLER_REQUIRED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to reserve an instance to publish discovery data for
 *        a user created data writer. This typically means
 *        DomainParticipantQos.resource_limits.remote_writer_allocation
 *        is too small.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DW_INSTANCE_ASSERTION_FAILED_EC           (DDSC_LOG_BASE + 908)
#define DDSC_LOG_DW_INSTANCE_ASSERTION_FAILED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_DW_INSTANCE_ASSERTION_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief A DataReader incompatible with padding bits in encapsulation header
 *  was discovered.
 *
 * \details
 * Discovered an incompatible Micro DataReader that
 * cannot parse the padding bits set in the encapsulation
 * options of a sample payload by the Micro DataWriter.
 * Resolve by configuring the Micro DataWriter to omit padding
 * bits or upgrade the Micro DataReader to a version that can
 * interpret them.
 *
 * Disable padding bits in the Micro DataWriter by setting the
 * property \idref_XTYPES_COMPLIANCE_MASK_PROPERTY to a value that removes the
 * encapsulation  option padding bit. See the \idref_XTypesComplianceMask
 * section in RTI Connext Micro documentation for more information.
 *
 * The version number of \rtime is printed as 4 hexadecimal digits:
 * - byte 3: Major
 * - byte 2: Minor
 * - byte 1: Release
 * - byte 0: Revision
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DW_INCOMPATIBLE_PADDING_EC                (DDSC_LOG_BASE + 909)
#define DDSC_LOG_DW_INCOMPATIBLE_PADDING(level_,maj_,min_,rel_,rev_) \
OSAPI_LOG_ENTRY_ADD_1INT_HEX((level_),DDSC_LOG_DW_INCOMPATIBLE_PADDING_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
    "version", (((maj_) << 24u) | ((min_) << 16u) | ((rel_) << 8u) | (rev_)))

/*******************************************************************************
 *                              DATAREADER RELATED
 ******************************************************************************/
/*e
 * \brief Failed to create a typed datareader
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DR_CREATE_TYPED_READER_EC                (DDSC_LOG_BASE + 1000)
#define DDSC_LOG_DR_CREATE_TYPED_READER(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_DR_CREATE_TYPED_READER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to copy a sample upon reception, read, or take
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DR_COPY_DATA_SAMPLE_EC                   (DDSC_LOG_BASE + 1001)
#define DDSC_LOG_DR_COPY_DATA_SAMPLE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_DR_COPY_DATA_SAMPLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to commit a sample to be made available to be read or taken
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DR_COMMIT_SAMPLE_EC                      (DDSC_LOG_BASE + 1002)
#define DDSC_LOG_DR_COMMIT_SAMPLE(level_,sn_high_,sn_low_,rc_) \
OSAPI_LOG_ENTRY_ADD_3INT((level_),DDSC_LOG_DR_COMMIT_SAMPLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "sn_high",(RTI_INT32)(sn_high_),"sn_low",(RTI_INT32)(sn_low_),\
        "rc=%d",(RTI_INT32)(rc_))

/*e
 * \brief A datareader filter function failed
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DR_FILTER_ERROR_EC                       (DDSC_LOG_BASE + 1003)
#define DDSC_LOG_DR_FILTER_ERROR(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_DR_FILTER_ERROR_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to deserialize a key-hash parameter
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DR_DESERIALIZE_KEYHASH_EC                (DDSC_LOG_BASE + 1004)
#define DDSC_LOG_DR_DESERIALIZE_KEYHASH(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_DR_DESERIALIZE_KEYHASH_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to get a Reader History entry for a received sample
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DR_GET_ENTRY_FAILED_EC                   (DDSC_LOG_BASE + 1005)
#define DDSC_LOG_DR_GET_ENTRY_FAILED(level_,reason_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_DR_GET_ENTRY_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"reason",(reason_))

/*e
 * \brief Failed to commit a receive sample to Reader History to be read or
 *        taken
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DR_COMMIT_ENTRY_EC                       (DDSC_LOG_BASE + 1006)
#define DDSC_LOG_DR_COMMIT_ENTRY(level_,sn_high_,sn_low_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_DR_COMMIT_ENTRY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "sn_high",(sn_high_),"sn_low",(sn_low_))

/*e
 * \brief A DataReader failed to unregister an instance
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DR_UNREGISTER_KEY_EC                     (DDSC_LOG_BASE + 1007)
#define DDSC_LOG_DR_UNREGISTER_KEY(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_DR_UNREGISTER_KEY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief A DataReader failed to dispose an instance
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DR_DISPOSE_KEY_EC                        (DDSC_LOG_BASE + 1008)
#define DDSC_LOG_DR_DISPOSE_KEY(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_DR_DISPOSE_KEY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief A call to a reader/take function failed
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DR_READ_TAKE_FAILURE_EC                  (DDSC_LOG_BASE + 1009)
#define DDSC_LOG_DR_READ_TAKE_FAILURE(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_DR_READ_TAKE_FAILURE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"retcode",(dbrc_))

/*e
 * \brief Failed to create keyhash of instance handle
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DR_INSTANCE_TO_KEYHASH_EC                (DDSC_LOG_BASE + 1010)
#define DDSC_LOG_DR_INSTANCE_TO_KEYHASH(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_DR_INSTANCE_TO_KEYHASH_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create keyhash of instance handle
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DR_INSTANCE_MAPPING_EXHAUSTED_EC         (DDSC_LOG_BASE + 1011)
#define DDSC_LOG_DR_INSTANCE_MAPPING_EXHAUSTED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_DR_INSTANCE_MAPPING_EXHAUSTED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)


/*e
 * \brief Failed to reserve an instance to publish discovery data for
 *        a user created data reader. This typically means
 *        DomainParticipantQos.resource_limits.remote_reader_allocation
 *        is too small.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DR_INSTANCE_ASSERTION_FAILED_EC          (DDSC_LOG_BASE + 1012)
#define DDSC_LOG_DR_INSTANCE_ASSERTION_FAILED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_DR_INSTANCE_ASSERTION_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to remove a sample when it was being pushed out from
 *        the Reader History.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DR_ON_SAMPLE_REMOVED_EC          (DDSC_LOG_BASE + 1013)
#define DDSC_LOG_DR_ON_SAMPLE_REMOVED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_DR_ON_SAMPLE_REMOVED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*******************************************************************************
 *                      TOPIC, TYPE & TYPE PLUGIN RELATED
 ******************************************************************************/
/*e
 * \brief Two type names are incompatible
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TYPE_NAME_CMP_EC                         (DDSC_LOG_BASE + 1100)
#define DDSC_LOG_TYPE_NAME_CMP(level_,left_,right_) \
OSAPI_LOG_ENTRY_ADD_2STRING((level_),DDSC_LOG_TYPE_NAME_CMP_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "left",(left_),"right",(right_))

/*e
 * \brief Two topic names are incompatible
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TOPIC_NAME_CMP_EC                        (DDSC_LOG_BASE + 1101)
#define DDSC_LOG_TOPIC_NAME_CMP(level_,left_,right_) \
OSAPI_LOG_ENTRY_ADD_2STRING((level_),DDSC_LOG_TOPIC_NAME_CMP_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "left",(left_),"right",(right_))

/*e
 * \brief The get_serialized_sample_max_size function kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TYPE_FUNCTION_GET_SERIALIZED_SAMPLE_MAX_SIZE        1

/*e
 * \brief The serialize_data function kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TYPE_FUNCTION_SERIALIZE_DATA                        2

/*e
 * \brief The deserialize_data function kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TYPE_FUNCTION_DESERIALIZE_DATA                      3

/*e
 * \brief The create_sample function kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TYPE_FUNCTION_CREATE_SAMPLE                         4

/*e
 * \brief The copy_sample function kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TYPE_FUNCTION_COPY_SAMPLE                           5

/*e
 * \brief The delete_sample function kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TYPE_FUNCTION_DELETE_SAMPLE                         6


/*e
 * \brief The get_key_kind function kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TYPE_FUNCTION_GET_KEY_KIND                          7


/*e
 * \brief The instance_to_keyhash function kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TYPE_FUNCTION_INSTANCE_TO_KEYHASH                   8

/*e
 * \brief Invalid type plugin, The specified function pointer is NULL
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TYPE_FUNCTION_NULL_EC                    (DDSC_LOG_BASE + 1102)
#define DDSC_LOG_type_function_null(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),\
        DDSC_LOG_TYPE_FUNCTION_NULL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief Failed to create a topic because the name exceeded the maximum
 *        length of 255 octets (excluding the terminating NUL)
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TOPIC_TOO_LONG_EC                        (DDSC_LOG_BASE + 1103)
#define DDSC_LOG_TOPIC_TOO_LONG(level_,length_) \
OSAPI_LOG_ENTRY_ADD_1UINT((level_),DDSC_LOG_TOPIC_TOO_LONG_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"length",(length_))

/*e
 * \brief Failed to create a type because the name exceeded the maximum
 *        length of 255 octets (excluding the terminating NUL)
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TYPE_TOO_LONG_EC                         (DDSC_LOG_BASE + 1104)
#define DDSC_LOG_TYPE_TOO_LONG(level_,length_) \
OSAPI_LOG_ENTRY_ADD_1UINT((level_),DDSC_LOG_TYPE_TOO_LONG_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"length",(length_))

/*e
 * \brief A type-plugin for the given type could not be found
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_LOOKUP_TYPE_PLUGIN_EC                    (DDSC_LOG_BASE + 1105)
#define DDSC_LOG_LOOKUP_TYPE_PLUGIN(level_,name_) \
OSAPI_LOG_ENTRY_ADD_1POINTER((level_),DDSC_LOG_LOOKUP_TYPE_PLUGIN_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM, "type",(name_))

/*e
 * \brief Two types have incompatible keys
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TYPE_KEY_TYPE_EC                         (DDSC_LOG_BASE + 1106)
#define DDSC_LOG_TYPE_KEY_TYPE(level_,topic_,type_) \
OSAPI_LOG_ENTRY_ADD_2STRING((level_),DDSC_LOG_TYPE_KEY_TYPE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"topic",(topic_),"type",(type_))

/*e
 * \brief Type-plugin cannot return a buffer.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TYPE_PLUGIN_GET_BUFFER_EC                (DDSC_LOG_BASE + 1107)
#define DDSC_LOG_TYPE_PLUGIN_GET_BUFFER(level_) \
OSAPI_LOG_ENTRY_ADD(level_, \
    DDSC_LOG_TYPE_PLUGIN_GET_BUFFER_EC,\
    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Inconsistent type-plugin DataReader.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TYPE_PLUGIN_INCONSISTENT_DR_EC           (DDSC_LOG_BASE + 1108)
#define DDSC_LOG_TYPE_PLUGIN_INCONSISTENT_DR(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),\
        DDSC_LOG_TYPE_FUNCTION_NULL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"using",(kind_))

/*e
 * \brief A DomainParticipant with the specified GUID already exists in the
 *        DomainParticipant factory.
 * \ingroup DDSCLogCodesClass
 *
 * \details
 * A DomainParticipant with the specified GUID already exists in the domain
 * in the DomainParticipant factory. The GUID is either generated (default)
 * or specified manually in the DomainParticipantQos.protocol policy.
 * This error typically indicates that the DomainParticipantQos.protocol
 * is manually specified and is a duplicate.
 */
#define DDSC_LOG_DUPLICATE_GUID_PREFIX_EC                 (DDSC_LOG_BASE + 1109)
#define DDSC_LOG_DUPLICATE_GUID_PREFIX(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_DUPLICATE_GUID_PREFIX_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Managed pool function failed.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TYPE_MANAGED_POOL_FAILURE_EC             (DDSC_LOG_BASE + 1110)
#define DDSC_LOG_TYPE_MANAGED_POOL_FAILURE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_TYPE_MANAGED_POOL_FAILURE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)


/*e
 * \brief Managed pool failed to get sample id.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TYPE_GET_SAMPLE_ID_EC                    (DDSC_LOG_BASE + 1111)
#define DDSC_LOG_TYPE_GET_SAMPLE_ID(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_TYPE_GET_SAMPLE_ID_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief A DomainParticipant with enable_participant_discovery_by_name set to
 *        to TRUE but an empty name is created.
 * \ingroup DDSCLogCodesClass
 *
 * \details
 *
 * If DomainParticipantQos.discovery.enable_participant_discovery_by_name is set
 * to TRUE, but DomainParticipantQos.participant_name.name is empty. This
 * is not allowed.
 *
 * If DomainParticipantQos.discovery.enable_participant_discovery_by_name is set
 * to TRUE, the requirement is that all DomainParticipant's in the domain are
 * uniquely named. If this is not true the system may experience undefined
 * behavior.
 *
 * Note that it is not possible For \rtime to verify that all
 * DomainParticipant's are uniquely named. This is the responsibility of the
 * applications.
 */
#define DDSC_LOG_PARTICIPANT_NAME_EMPTY_EC                (DDSC_LOG_BASE + 1112)
#define DDSC_LOG_PARTICIPANT_NAME_EMPTY(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_PARTICIPANT_NAME_EMPTY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief A DomainParticipant with enable_participant_discovery_by_name set to
 *        to TRUE, but a local participant with the same
 *        DomainParticipantQos.participant_name.name already exists.
 * \ingroup DDSCLogCodesClass
 *
 * \details
 *
 * If DomainParticipantQos.discovery.enable_participant_discovery_by_name is set
 * to TRUE, but an local DomainParticipant in the same
 * DomainParticipantQos.participant_name.name already exists in the same
 * process in the same domain. This is not allowed.
 *
 * If DomainParticipantQos.discovery.enable_participant_discovery_by_name is set
 * to TRUE, the requirement is that all DomainParticipant's in the domain are
 * uniquely named. If this is not true the system may experience undefined
 * behavior.
 *
 * Note that it is not possible For \rtime to verify that all
 * DomainParticipant's are uniquely named. This is the responsibility of the
 * applications.
 */
#define DDSC_LOG_DUPLICATE_PARTICIPANT_NAME_EC            (DDSC_LOG_BASE + 1113)
#define DDSC_LOG_DUPLICATE_PARTICIPANT_NAME(level_,name_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDSC_LOG_DUPLICATE_PARTICIPANT_NAME_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"name",name_)

/*e
 * \brief A remote participant is discovered with the same name as the a
 *        local participant name in the same domain ID.
 *
 * \ingroup DDSCLogCodesClass
 *
 * \details
 *
 * If DomainParticipantQos.discovery.enable_participant_discovery_by_name is set
 * to TRUE, but a remote participant is discovered with the same name as a
 *  local participant name in the same domain ID.
 *
 * If DomainParticipantQos.discovery.enable_participant_discovery_by_name is set
 * to TRUE, the requirement is that all DomainParticipant's in the domain are
 * uniquely named. If this is not true the system may experience undefined
 * behavior.
 *
 * Note that it is not possible For \rtime to verify that all
 * DomainParticipant's are uniquely named. This is the responsibility of the
 * applications.
 */
#define DDSC_LOG_DUPLICATE_REMOTE_PARTICIPANT_NAME_EC     (DDSC_LOG_BASE + 1114)
#define DDSC_LOG_DUPLICATE_REMOTE_PARTICIPANT_NAME(level_,name_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDSC_LOG_DUPLICATE_REMOTE_PARTICIPANT_NAME_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"name",name_)

/*e
 * \brief A remote participant without a name is discovered.
 * \ingroup DDSCLogCodesClass
 *
 * \details
 *
 * If DomainParticipantQos.discovery.enable_participant_discovery_by_name is set
 * to TRUE, but a discovered participants name is empty. This is not allowed.
 *
 * If DomainParticipantQos.discovery.enable_participant_discovery_by_name is set
 * to TRUE, the requirement is that all DomainParticipant's in the domain are
 * uniquely named. If this is not true the system may experience undefined
 * behavior.
 *
 * Note that it is not possible For \rtime to verify that all
 * DomainParticipant's are uniquely named. This is the responsibility of the
 * applications.
 */
#define DDSC_LOG_REMOTE_PARTICIPANT_NAME_EMPTY_EC         (DDSC_LOG_BASE + 1115)
#define DDSC_LOG_REMOTE_PARTICIPANT_NAME_EMPTY(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_REMOTE_PARTICIPANT_NAME_EMPTY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to reset a remote participant
 * \ingroup DDSCLogCodesClass
 *
 * \details
 *
 * If DomainParticipantQos.discovery.enable_participant_discovery_by_name is set
 * to TRUE, but a discovered participants name is empty. This is not allowed.
 *
 * If DomainParticipantQos.discovery.enable_participant_discovery_by_name is set
 * to TRUE, the requirement is that all DomainParticipant's in the domain are
 * uniquely named. If this is not true the system may experience undefined
 * behavior.
 *
 * Note that it is not possible For \rtime to verify that all
 * DomainParticipant's are uniquely named. This is the responsibility of the
 * applications.
 */
#define DDSC_LOG_REMOTE_PARTICIPANT_RESTART_EC            (DDSC_LOG_BASE + 1116)
#define DDSC_LOG_REMOTE_PARTICIPANT_RESTART(level_,name_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDSC_LOG_REMOTE_PARTICIPANT_RESTART_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"name",name_)

/*******************************************************************************
 *                     DISCOVERY PLUGIN RELATED
 ******************************************************************************/
/*e
 * \brief Discovery plugin failed its update after a local DomainParticipant
 *        was enabled
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DISC_LOCAL_PARTICIPANT_ENABLED_EC        (DDSC_LOG_BASE + 1200)
#define DDSC_LOG_DISC_LOCAL_PARTICIPANT_ENABLED(level_,plugin_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDSC_LOG_DISC_LOCAL_PARTICIPANT_ENABLED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"plugin",(plugin_))

/*e
 * \brief Discovery plugin failed its update before a local DomainParticipant
 *        was created
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DISC_BEFORE_LOCAL_PARTICIPANT_CREATED_EC (DDSC_LOG_BASE + 1201)
#define DDSC_LOG_DISC_BEFORE_LOCAL_PARTICIPANT_CREATED(level_,plugin_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDSC_LOG_DISC_BEFORE_LOCAL_PARTICIPANT_CREATED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"plugin",(plugin_))

/*e
 * \brief Discovery plugin failed its update after a local DomainParticipant
 *        was created
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DISC_AFTER_LOCAL_PARTICIPANT_CREATED_EC  (DDSC_LOG_BASE + 1202)
#define DDSC_LOG_DISC_AFTER_LOCAL_PARTICIPANT_CREATED(level_,plugin_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDSC_LOG_DISC_AFTER_LOCAL_PARTICIPANT_CREATED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"plugin",(plugin_))

/*e
 * \brief Discovery plugin failed its update after a local DataReader
 *        was enabled
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DISC_AFTER_LOCAL_DATAREADER_ENABLED_EC   (DDSC_LOG_BASE + 1203)
#define DDSC_LOG_DISC_AFTER_LOCAL_DATAREADER_ENABLED(level_,plugin_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDSC_LOG_DISC_AFTER_LOCAL_DATAREADER_ENABLED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"plugin",(plugin_))

/*e
 * \brief Discovery plugin failed its update after a local DataReader
 *        was deleted
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DISC_AFTER_LOCAL_DATAREADER_DELETED_EC   (DDSC_LOG_BASE + 1204)
#define DDSC_LOG_DISC_AFTER_LOCAL_DATAREADER_DELETED(level_,plugin_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDSC_LOG_DISC_AFTER_LOCAL_DATAREADER_DELETED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"plugin",(plugin_))

/*e
 * \brief Discovery plugin failed its update after a local DataWriter
 *        was enabled
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DISC_AFTER_LOCAL_DATAWRITER_ENABLED_EC   (DDSC_LOG_BASE + 1205)
#define DDSC_LOG_DISC_AFTER_LOCAL_DATAWRITER_ENABLED(level_,plugin_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDSC_LOG_DISC_AFTER_LOCAL_DATAWRITER_ENABLED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"plugin",(plugin_))

/*e
 * \brief Discovery plugin failed its update after a local DataWriter
 *        was deleted
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DISC_AFTER_LOCAL_DATAWRITER_DELETED_EC   (DDSC_LOG_BASE + 1206)
#define DDSC_LOG_DISC_AFTER_LOCAL_DATAWRITER_DELETED(level_,plugin_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDSC_LOG_DISC_AFTER_LOCAL_DATAWRITER_DELETED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"plugin",(plugin_))

/*e
 * \brief Discovery plugin failed its update after being notified that a remote
 *        DomainParticipant was about to be deleted.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DISC_BEFORE_REMOTE_PARTICIPANT_DELETED_EC       (DDSC_LOG_BASE + 1207)
#define DDSC_LOG_DISC_BEFORE_REMOTE_PARTICIPANT_DELETED(level_,plugin_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDSC_LOG_DISC_BEFORE_REMOTE_PARTICIPANT_DELETED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"plugin",(plugin_))

/*e
 * \brief Failed to add a peer with discovery plugin
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DISC_ADD_PEER_EC                         (DDSC_LOG_BASE + 1208)
#define DDSC_LOG_DISC_ADD_PEER(level_,plugin_,peer_) \
OSAPI_LOG_ENTRY_ADD_2STRING((level_),DDSC_LOG_DISC_ADD_PEER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"plugin",(plugin_),"peer",(peer_))

/*e
 * \brief Deserialized a parameter of unknown ID
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DESERIALIZE_UNKNOWN_PID_EC                 (DDSC_LOG_BASE + 1209)
#define DDSC_LOG_DESERIALIZE_UNKNOWN_PID(level_,pid_,length_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_DESERIALIZE_UNKNOWN_PID_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"pid",(pid_),"length",(length_))

/*e
 * \brief Failed to serialize Builtin Endpoint Mask parameter
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_SERIALIZE_BUILTIN_ENDPOINTS_EC             (DDSC_LOG_BASE + 1210)
#define DDSC_LOG_SERIALIZE_BUILTIN_ENDPOINTS(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_SERIALIZE_BUILTIN_ENDPOINTS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to deserialize Builtin Endpoint Mask parameter
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DESERIALIZE_BUILTIN_ENDPOINTS_EC           (DDSC_LOG_BASE + 1211)
#define DDSC_LOG_DESERIALIZE_BUILTIN_ENDPOINTS(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_DESERIALIZE_BUILTIN_ENDPOINTS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM )

/*e
 * \brief Failed to serialize Topic Name parameter
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_SERIALIZE_TOPIC_NAME_EC                     (DDSC_LOG_BASE + 1212)
#define DDSC_LOG_SERIALIZE_TOPIC_NAME(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_SERIALIZE_TOPIC_NAME_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to set current offset of stream
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_CDR_SET_OFFSET_EC                           (DDSC_LOG_BASE + 1213)
#define DDSC_LOG_CDR_SET_OFFSET(level_,offset_) \
OSAPI_LOG_ENTRY_ADD_1UINT((level_),DDSC_LOG_CDR_SET_OFFSET_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "offset",(offset_))

/*e
 * \brief Failed to serialize Entity Name parameter
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_SERIALIZE_ENTITY_NAME_EC                    (DDSC_LOG_BASE + 1214)
#define DDSC_LOG_SERIALIZE_ENTITY_NAME(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_SERIALIZE_ENTITY_NAME_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to serialize Type Name parameter
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_SERIALIZE_TYPE_NAME_EC                      (DDSC_LOG_BASE + 1215)
#define DDSC_LOG_SERIALIZE_TYPE_NAME(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_SERIALIZE_TYPE_NAME_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to serialize GUID key
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_SERIALIZE_GUID_EC                           (DDSC_LOG_BASE + 1216)
#define DDSC_LOG_SERIALIZE_GUID(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_SERIALIZE_GUID_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to serialize Default Unicast Locator parameter
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_SERIALIZE_DEFAULT_UNICAST_EC                (DDSC_LOG_BASE + 1217)
#define DDSC_LOG_SERIALIZE_DEFAULT_UNICAST(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_SERIALIZE_DEFAULT_UNICAST_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to deserialize a locator of the specified kind
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DESERIALIZE_LOCATOR_EC                     (DDSC_LOG_BASE + 1218)
#define DDSC_LOG_DESERIALIZE_LOCATOR(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_DESERIALIZE_LOCATOR_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"kind",(kind_))

/*e
 * \brief A locator sequence is full, the remaining locators of the specified
 *        kind are dropped
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_LOCATORS_FULL_EC                          (DDSC_LOG_BASE + 1219)
#define DDSC_LOG_LOCATORS_FULL(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_LOCATORS_FULL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"kind",(kind_))

/*e
 * \brief The locator kind is not supported by any transport registered with the
 *        domain participant and is dropped
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_UNSUPPORTED_LOCATOR_EC                     (DDSC_LOG_BASE + 1220)
#define DDSC_LOG_UNSUPPORTED_LOCATOR(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_UNSUPPORTED_LOCATOR_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"kind",(kind_))


/*e
 * \brief Failed to serialize Protocol Version parameter
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_SERIALIZE_PROTOCOL_VERSION_EC               (DDSC_LOG_BASE + 1221)
#define DDSC_LOG_SERIALIZE_PROTOCOL_VERSION(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_SERIALIZE_PROTOCOL_VERSION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to deserialize Protocol Version parameter
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DESERIALIZE_PROTOCOL_VERSION_EC             (DDSC_LOG_BASE + 1222)
#define DDSC_LOG_DESERIALIZE_PROTOCOL_VERSION(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_DESERIALIZE_PROTOCOL_VERSION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)


/*e
 * \brief Failed to deserialize Vendor ID parameter
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DESERIALIZE_VENDOR_ID_EC                    (DDSC_LOG_BASE + 1223)
#define DDSC_LOG_DESERIALIZE_VENDOR_ID(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_DESERIALIZE_VENDOR_ID_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to serialize Vendor ID parameter
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_SERIALIZE_VENDOR_ID_EC                     (DDSC_LOG_BASE + 1224)
#define DDSC_LOG_SERIALIZE_VENDOR_ID(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_SERIALIZE_VENDOR_ID_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to serialize Product Version parameter
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_SERIALIZE_PRODUCT_VERSION_EC               (DDSC_LOG_BASE + 1225)
#define DDSC_LOG_SERIALIZE_PRODUCT_VERSION(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_SERIALIZE_PRODUCT_VERSION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)


/*e
 * \brief Failed to serialize Lease Duration parameter
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_SERIALIZE_LEASE_DURATION_EC                (DDSC_LOG_BASE + 1226)
#define DDSC_LOG_SERIALIZE_LEASE_DURATION(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_SERIALIZE_LEASE_DURATION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)


/*e
 * \brief Failed to add a remote peer to a local DataWriter
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DATAWRITER_ADD_PEER_FAILED_EC              (DDSC_LOG_BASE + 1227)
#define DDSC_LOG_DATAWRITER_ADD_PEER_FAILED(level_,oid_) \
OSAPI_LOG_ENTRY_ADD_1UINT((level_),DDSC_LOG_DATAWRITER_ADD_PEER_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"oid",(oid_))


/*e
 * \brief Failed to add a remote peer to a local DataReader
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DATAREADER_ADD_PEER_FAILED_EC            (DDSC_LOG_BASE + 1228)
#define DDSC_LOG_DATAREADER_ADD_PEER_FAILED(level_,oid_) \
OSAPI_LOG_ENTRY_ADD_1INT_HEX((level_),DDSC_LOG_DATAREADER_ADD_PEER_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"oid",(RTI_INT32)(oid_))

/*e
 * \brief Failed to remote/reset remote participant after liveliness expired
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DISC_REMOTE_PARTICIPANT_REMOVE_EC        (DDSC_LOG_BASE + 1229)
#define DDSC_LOG_DISC_REMOTE_PARTICIPANT_REMOVE(level_,ddsrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_DISC_REMOTE_PARTICIPANT_REMOVE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"ddsrc",(ddsrc_))

/*e
 * \brief Failed to deserialize partition seq
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DESERIALIZE_PARTITION_SEQ_EC              (DDSC_LOG_BASE + 1230)
#define DDSC_LOG_DESERIALIZE_PARTITION_SEQ(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_DESERIALIZE_PARTITION_SEQ_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to initialize discovery queue
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_INITIALIZE_DISCOVERY_QUEUE_EC            (DDSC_LOG_BASE + 1231)
#define DDSC_LOG_INITIALIZE_DISCOVERY_QUEUE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_INITIALIZE_DISCOVERY_QUEUE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to queue a publication discovery message
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_QUEUE_PUBLICATION_DISCOVERY_EC           (DDSC_LOG_BASE + 1232)
#define DDSC_LOG_QUEUE_PUBLICATION_DISCOVERY(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_QUEUE_PUBLICATION_DISCOVERY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to queue a subscription discovery message
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_QUEUE_SUBSCRIPTION_DISCOVERY_EC          (DDSC_LOG_BASE + 1233)
#define DDSC_LOG_QUEUE_SUBSCRIPTION_DISCOVERY(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_QUEUE_SUBSCRIPTION_DISCOVERY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed finalize discovery queue
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_QUEUE_FINALIZE_EC                        (DDSC_LOG_BASE + 1234)
#define DDSC_LOG_QUEUE_FINALIZE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_QUEUE_FINALIZE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*******************************************************************************
 *                          BUILTIN IPC RELATED
 ******************************************************************************/

/*e
 * \brief Failed to initialize Builtin IPC entities
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_IPC_INIT_FAILED_EC                        (DDSC_LOG_BASE + 1300)
#define DDSC_LOG_IPC_INIT_FAILED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_IPC_INIT_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to allocate memory for one of the Builtin IPC entities
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_IPC_ALLOC_FAILED_EC                        (DDSC_LOG_BASE + 1301)
#define DDSC_LOG_IPC_ALLOC_FAILED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_IPC_ALLOC_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to finalize all resources
 * A potential memory leak might have been caused by unexpected
 * errors during finalization of acquired resources.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_IPC_FINALIZE_FAILED_EC                      (DDSC_LOG_BASE + 1302)
#define DDSC_LOG_IPC_FINALIZE_FAILED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_IPC_FINALIZE_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to register type for a Builtin IPC channel
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_IPC_CHANNEL_TYPE_REGISTER_EC                (DDSC_LOG_BASE + 1303)
#define DDSC_LOG_IPC_CHANNEL_TYPE_REGISTER(level_,channel_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_IPC_CHANNEL_TYPE_REGISTER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"channel",(channel_))

/*e
 * \brief Failed to create topic for a Builtin IPC channel
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_IPC_CHANNEL_TOPIC_CREATE_EC                 (DDSC_LOG_BASE + 1304)
#define DDSC_LOG_IPC_CHANNEL_TOPIC_CREATE(level_,channel_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_IPC_CHANNEL_TOPIC_CREATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"channel",(channel_))

/*e
 * \brief Failed to create reader for a Builtin IPC channel
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_IPC_CHANNEL_READER_CREATE_EC                (DDSC_LOG_BASE + 1305)
#define DDSC_LOG_IPC_CHANNEL_READER_CREATE(level_,channel_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_IPC_CHANNEL_READER_CREATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"channel",(channel_))

/*e
 * \brief Failed to create writer for a Builtin IPC channel
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_IPC_CHANNEL_WRITER_CREATE_EC                (DDSC_LOG_BASE + 1306)
#define DDSC_LOG_IPC_CHANNEL_WRITER_CREATE(level_,channel_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_IPC_CHANNEL_WRITER_CREATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"channel",(channel_))

/*e
 * \brief Failed to find type plugin for a Builtin IPC channel
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_IPC_CHANNEL_TYPE_PLUGIN_EC                 (DDSC_LOG_BASE + 1307)
#define DDSC_LOG_IPC_CHANNEL_TYPE_PLUGIN(level_,channel_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_IPC_CHANNEL_TYPE_PLUGIN_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"channel",(channel_))

/*e
 * \brief Unknown builtin IPC channel requested
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_IPC_CHANNEL_UNKNOWN_EC                     (DDSC_LOG_BASE + 1308)
#define DDSC_LOG_IPC_CHANNEL_UNKNOWN(level_,channel_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_IPC_CHANNEL_UNKNOWN_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"channel",(channel_))

/*e
 * \brief Failed to update DomainParticipantQos to account for
 *  built-in IPC endpoints.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_IPC_UPDATE_QOS_FAILED_EC                   (DDSC_LOG_BASE + 1309)
#define DDSC_LOG_IPC_UPDATE_QOS_FAILED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_IPC_UPDATE_QOS_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to add an anonymous route to a built-in DataWriter or DataReader
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_IPC_CHANNEL_ADD_ANON_ROUTE_EC               (DDSC_LOG_BASE + 1310)
#define DDSC_LOG_IPC_CHANNEL_ADD_ANON_ROUTE(level_,id_,kind_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_IPC_CHANNEL_ADD_ANON_ROUTE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"kind",kind_,"id",id_)

/*e
 * \brief Failed to delete an anonymous route to a built-in DataWriter or DataReader
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_IPC_CHANNEL_DELETE_ANON_ROUTE_EC            (DDSC_LOG_BASE + 1311)
#define DDSC_LOG_IPC_CHANNEL_DELETE_ANON_ROUTE(level_,id_,kind_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_IPC_CHANNEL_DELETE_ANON_ROUTE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"kind",kind_,"id",id_)

/*e
 * \brief Failed to add an inter-participant channel peer
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_IPC_CHANNEL_ADD_PEER_EC                    (DDSC_LOG_BASE + 1312)
#define DDSC_LOG_IPC_CHANNEL_ADD_PEER(level_,id_,kind_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_IPC_CHANNEL_ADD_PEER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"kind",kind_,"id",id_)

/*e
 * \brief Failed to remove an inter-participant channel peer
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_IPC_CHANNEL_REMOVE_PEER_EC                 (DDSC_LOG_BASE + 1313)
#define DDSC_LOG_IPC_CHANNEL_REMOVE_PEER(level_,id_,kind_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_IPC_CHANNEL_REMOVE_PEER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"kind",kind_,"id",id_)

/*e
 * \brief Failed to return a loaned inter-participant sample
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_IPC_CHANNEL_RETURN_LOAN_EC                 (DDSC_LOG_BASE + 1314)
#define DDSC_LOG_IPC_CHANNEL_RETURN_LOAN(level_,id_)\
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_IPC_CHANNEL_RETURN_LOAN_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM, "id", id_)

/*e
 * \brief Failed to process inter-participant message
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_IPC_CHANNEL_PROCESS_SAMPLE_EC              (DDSC_LOG_BASE + 1315)
#define DDSC_LOG_IPC_CHANNEL_PROCESS_SAMPLE(level_,id_)\
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_IPC_CHANNEL_PROCESS_SAMPLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM, "id",id_)

/*e
 * \brief Failed to assert inter-participant routes
 * \ingroup DDSCLogCodesClass
 */
#define DDS_LOG_IPC_ASSERT_ROUTES_EC                        (DDSC_LOG_BASE + 1316)
#define DDS_LOG_IPC_ASSERT_ROUTES(level_)\
OSAPI_LOG_ENTRY_ADD(level_,\
    DDS_LOG_IPC_ASSERT_ROUTES_EC,\
    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to write inter-participant message
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_IPC_CHANNEL_WRITE_SAMPLE_EC                (DDSC_LOG_BASE + 1317)
#define DDSC_LOG_IPC_CHANNEL_WRITE_SAMPLE(level_,id_)\
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_IPC_CHANNEL_WRITE_SAMPLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM, "id", id_)

/*e
 * \brief Failed to process handshake authentication
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_IPC_CHANNEL_FAILED_PROCESS_AUTH_HANDSHAKE_FAILED_EC \
                                                        (DDSC_LOG_BASE + 1318)
#define DDSC_LOG_IPC_CHANNEL_FAILED_PROCESS_AUTH_HANDSHAKE_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,\
                    DDSC_LOG_IPC_CHANNEL_FAILED_PROCESS_AUTH_HANDSHAKE_FAILED_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to process participant interceptor tokens
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_IPC_CHANNEL_FAILED_PROCESS_PARTICIPANT_INTERCEPTOR_STATE_FAILED_EC\
                                                        (DDSC_LOG_BASE + 1319)
#define DDSC_LOG_IPC_CHANNEL_FAILED_PROCESS_PARTICIPANT_INTERCEPTOR_STATE_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(\
    level_,\
    DDSC_LOG_IPC_CHANNEL_FAILED_PROCESS_PARTICIPANT_INTERCEPTOR_STATE_FAILED_EC,\
    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to process endpoint interceptor tokens
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_IPC_CHANNEL_FAILED_PROCESS_ENDPOINT_INTERCEPTOR_STATE_FAILED_EC \
                                                        (DDSC_LOG_BASE + 1320)
#define DDSC_LOG_IPC_CHANNEL_FAILED_PROCESS_ENDPOINT_INTERCEPTOR_STATE_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(\
    level_,\
    DDSC_LOG_IPC_CHANNEL_FAILED_PROCESS_ENDPOINT_INTERCEPTOR_STATE_FAILED_EC,\
    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to process authentication request
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_IPC_CHANNEL_FAILED_PROCESS_AUTH_REQUEST_FAILED_EC \
                                                        (DDSC_LOG_BASE + 1321)
#define DDSC_LOG_IPC_CHANNEL_FAILED_PROCESS_AUTH_REQUEST_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,\
                    DDSC_LOG_IPC_CHANNEL_FAILED_PROCESS_AUTH_REQUEST_FAILED_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to initialize inter-participant channel
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_IPC_CHANNEL_CONFIG_INITIALIZE_FAILED_EC \
                                                        (DDSC_LOG_BASE + 1322)
#define DDSC_LOG_IPC_CHANNEL_CONFIG_INITIALIZE_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,\
                    DDSC_LOG_IPC_CHANNEL_CONFIG_INITIALIZE_FAILED_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e Failed to finalize inter-participant channel
 * \brief
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_IPC_CHANNEL_CONFIG_FINALIZE_FAILED_EC \
                                                        (DDSC_LOG_BASE + 1323)
#define DDSC_LOG_IPC_CHANNEL_CONFIG_FINALIZE_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,\
                    DDSC_LOG_IPC_CHANNEL_CONFIG_FINALIZE_FAILED_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to get inter-participant channel configuration
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_IPC_CHANNEL_CONFIG_SET_FAILED_EC \
                                                        (DDSC_LOG_BASE + 1324)
#define DDSC_LOG_IPC_CHANNEL_CONFIG_SET_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,\
                    DDSC_LOG_IPC_CHANNEL_CONFIG_SET_FAILED_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to set inter-participant channel listeners
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_IPC_CHANNEL_SET_LISTENERS_FAILED_EC \
                                                        (DDSC_LOG_BASE + 1325)
#define DDSC_LOG_IPC_CHANNEL_SET_LISTENERS_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,\
                    DDSC_LOG_IPC_CHANNEL_SET_LISTENERS_FAILED_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to enable inter-participant channel entities
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_IPC_CHANNEL_ENABLE_FAILED_EC \
                                                        (DDSC_LOG_BASE + 1326)
#define DDSC_LOG_IPC_CHANNEL_ENABLE_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,\
                    DDSC_LOG_IPC_CHANNEL_ENABLE_FAILED_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create inter-participant channel
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_IPC_CHANNEL_CREATE_FAILED_EC \
                                                        (DDSC_LOG_BASE + 1327)
#define DDSC_LOG_IPC_CHANNEL_CREATE_FAILED(level_,channel_)\
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_IPC_CHANNEL_CREATE_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"channel",(channel_))

/*******************************************************************************
 *                            TRUST RELATED
 ******************************************************************************/

/*e
 * \brief Unknown generic message class id
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_UNKNOWN_GMCLASSID_EC                 (DDSC_LOG_BASE + 1401)
#define DDSC_LOG_TRUST_UNKNOWN_GMCLASSID(level_,classid_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDSC_LOG_TRUST_UNKNOWN_GMCLASSID_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"classid",(classid_))

/*e
 * \brief Unknown builtin service id
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_UNKNOWN_SERVICEID_EC                 (DDSC_LOG_BASE + 1402)
#define DDSC_LOG_TRUST_UNKNOWN_SERVICEID(level_,svcid_) \
OSAPI_LOG_ENTRY_ADD_1UINT((level_),DDSC_LOG_TRUST_UNKNOWN_SERVICEID_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"serviceid",(svcid_))

/*e
 * \brief Ignored remote participant
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_IGNORED_REMOTE_PARTICIPANT_EC        (DDSC_LOG_BASE + 1403)
#define DDSC_LOG_TRUST_IGNORED_REMOTE_PARTICIPANT(level_,k_) \
OSAPI_LOG_ENTRY_CREATE((level_),DDSC_LOG_TRUST_IGNORED_REMOTE_PARTICIPANT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k0",((RTI_INT8*)(k_))[0],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k1",((RTI_INT8*)(k_))[1],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k2",((RTI_INT8*)(k_))[2],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k3",((RTI_INT8*)(k_))[3],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k4",((RTI_INT8*)(k_))[4],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k5",((RTI_INT8*)(k_))[5],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k6",((RTI_INT8*)(k_))[6],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k7",((RTI_INT8*)(k_))[7],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k8",((RTI_INT8*)(k_))[8],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k9",((RTI_INT8*)(k_))[9],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k10",((RTI_INT8*)(k_))[10],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k11",((RTI_INT8*)(k_))[11],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k12",((RTI_INT8*)(k_))[12],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k13",((RTI_INT8*)(k_))[13],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k14",((RTI_INT8*)(k_))[14],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k15",((RTI_INT8*)(k_))[15],RTI_TRUE)

/*e
 * \brief Ignored handshake message from remote participant
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_IGNORED_HANDSHAKE_MESSAGE_EC         (DDSC_LOG_BASE + 1404)
#define DDSC_LOG_TRUST_IGNORED_HANDSHAKE_MESSAGE(level_,k_) \
OSAPI_LOG_ENTRY_CREATE((level_),DDSC_LOG_TRUST_IGNORED_HANDSHAKE_MESSAGE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k0",((RTI_INT8*)(k_))[0],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k1",((RTI_INT8*)(k_))[1],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k2",((RTI_INT8*)(k_))[2],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k3",((RTI_INT8*)(k_))[3],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k4",((RTI_INT8*)(k_))[4],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k5",((RTI_INT8*)(k_))[5],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k6",((RTI_INT8*)(k_))[6],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k7",((RTI_INT8*)(k_))[7],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k8",((RTI_INT8*)(k_))[8],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k9",((RTI_INT8*)(k_))[9],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k10",((RTI_INT8*)(k_))[10],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k11",((RTI_INT8*)(k_))[11],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k12",((RTI_INT8*)(k_))[12],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k13",((RTI_INT8*)(k_))[13],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k14",((RTI_INT8*)(k_))[14],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k15",((RTI_INT8*)(k_))[15],RTI_TRUE)

/*e
 * \brief Unexpected validation result
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_UNEXPECTED_VALIDATION_RESULT_EC      (DDSC_LOG_BASE + 1405)
#define DDSC_LOG_TRUST_UNEXPECTED_VALIDATION_RESULT(level_,res_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDSC_LOG_TRUST_UNEXPECTED_VALIDATION_RESULT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"val_res",(res_))

/*e
 * \brief A remote participant failed the authorization process
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_UNAUTHORIZED_REMOTE_PARTICIPANT_EC   (DDSC_LOG_BASE + 1406)
#define DDSC_LOG_TRUST_UNAUTHORIZED_REMOTE_PARTICIPANT(level_,k_) \
OSAPI_LOG_ENTRY_CREATE((level_),DDSC_LOG_TRUST_UNAUTHORIZED_REMOTE_PARTICIPANT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k0",((RTI_INT8*)(k_))[0],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k1",((RTI_INT8*)(k_))[1],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k2",((RTI_INT8*)(k_))[2],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k3",((RTI_INT8*)(k_))[3],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k4",((RTI_INT8*)(k_))[4],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k5",((RTI_INT8*)(k_))[5],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k6",((RTI_INT8*)(k_))[6],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k7",((RTI_INT8*)(k_))[7],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k8",((RTI_INT8*)(k_))[8],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k9",((RTI_INT8*)(k_))[9],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k10",((RTI_INT8*)(k_))[10],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k11",((RTI_INT8*)(k_))[11],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k12",((RTI_INT8*)(k_))[12],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k13",((RTI_INT8*)(k_))[13],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k14",((RTI_INT8*)(k_))[14],RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("k15",((RTI_INT8*)(k_))[15],RTI_TRUE)


#define DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,ec_) \
OSAPI_LOG_ENTRY_ADD_1STRING(level_,\
    ec_,\
    OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
    "exception message",\
    (ex_)->message)

/*e
 * \brief Failed to validate local identity
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_FAILED_VALIDATE_LOCAL_IDENTITY_EC     (DDSC_LOG_BASE + 1407)
#define DDSC_LOG_TRUST_FAILED_VALIDATE_LOCAL_IDENTITY(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
                DDSC_LOG_TRUST_FAILED_VALIDATE_LOCAL_IDENTITY_EC)

/*e
 * \brief Invalid local trust identity
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_INVALID_IDENTITY_HANDLE_EC           (DDSC_LOG_BASE + 1408)
#define DDSC_LOG_TRUST_INVALID_IDENTITY_HANDLE(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
                DDSC_LOG_TRUST_INVALID_IDENTITY_HANDLE_EC)

/*e
 * \brief Failed to get identity token
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_GET_IDENTITY_TOKEN_FAILED_EC         (DDSC_LOG_BASE + 1409)
#define DDSC_LOG_TRUST_GET_IDENTITY_TOKEN_FAILED(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
                DDSC_LOG_TRUST_GET_IDENTITY_TOKEN_FAILED_EC)

/*e
 * \brief Invalid local trust permissions
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_INVALID_PERMISSIONS_HANDLE_EC        (DDSC_LOG_BASE + 1410)
#define DDSC_LOG_TRUST_INVALID_PERMISSIONS_HANDLE(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
    DDSC_LOG_TRUST_INVALID_PERMISSIONS_HANDLE_EC)

/*e
 * \brief Invalid DomainParticipant GUID configuration
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_INVALID_PARTICIPANT_GUID_EC          (DDSC_LOG_BASE + 1411)
#define DDSC_LOG_TRUST_INVALID_PARTICIPANT_GUID(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
    DDSC_LOG_TRUST_INVALID_PARTICIPANT_GUID_EC)

/*e
 * \brief Failed to get DomainParticipant trust attributes
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_GET_PARTICIPANT_TRUST_ATTTRIBUTES_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1412)
#define DDSC_LOG_TRUST_GET_PARTICIPANT_TRUST_ATTTRIBUTES_FAILED(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
    DDSC_LOG_TRUST_GET_PARTICIPANT_TRUST_ATTTRIBUTES_FAILED_EC)

/*e
 * \brief Failed to create access control DomainParticipant
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_CHECK_CREATE_PARTICIPANT_FAILED_EC   (DDSC_LOG_BASE + 1413)
#define DDSC_LOG_TRUST_CHECK_CREATE_PARTICIPANT_FAILED(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
    DDSC_LOG_TRUST_CHECK_CREATE_PARTICIPANT_FAILED_EC)

/*e
 * \brief Failed to get permissions credential token
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_GET_PERMISSIONS_CREDENTIAL_TOKEN_FAILED_EC\
                                                            (DDSC_LOG_BASE + 1414)
#define DDSC_LOG_TRUST_GET_PERMISSIONS_CREDENTIAL_TOKEN_FAILED(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
    DDSC_LOG_TRUST_GET_PERMISSIONS_CREDENTIAL_TOKEN_FAILED_EC)

/*e
 * \brief Failed to get permissions token
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_GET_PERMISSIONS_TOKEN_FAILED_EC     (DDSC_LOG_BASE + 1415)
#define DDSC_LOG_TRUST_GET_PERMISSIONS_TOKEN_FAILED(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
                            DDSC_LOG_TRUST_GET_PERMISSIONS_TOKEN_FAILED_EC)

/*e
 * \brief Invalid interceptor handle
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_INVALID_INTERCEPTOR_HANDLE_EC       (DDSC_LOG_BASE + 1416)
#define DDSC_LOG_TRUST_INVALID_INTERCEPTOR_HANDLE(level_) \
OSAPI_LOG_ENTRY_ADD(level_,DDSC_LOG_TRUST_INVALID_INTERCEPTOR_HANDLE_EC,\
                                                OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to register local DomainParticipant
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_REGISTER_LOCAL_PARTICIPANT_FAILED_EC (DDSC_LOG_BASE + 1417)
#define DDSC_LOG_TRUST_REGISTER_LOCAL_PARTICIPANT_FAILED(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
    DDSC_LOG_TRUST_REGISTER_LOCAL_PARTICIPANT_FAILED_EC)

/*e
 * \brief Failed to return permissions credential token
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_RETURN_PERMISSIONS_CREDENTIAL_TOKEN_FAILED_EC\
                                                            (DDSC_LOG_BASE + 1418)
#define DDSC_LOG_TRUST_RETURN_PERMISSIONS_CREDENTIAL_TOKEN_FAILED(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
    DDSC_LOG_TRUST_RETURN_PERMISSIONS_CREDENTIAL_TOKEN_FAILED_EC)

/*e
 * \brief Failed to return permissions token
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_RETURN_PERMISSIONS_TOKEN_FAILED_EC   (DDSC_LOG_BASE + 1419)
#define DDSC_LOG_TRUST_RETURN_PERMISSIONS_TOKEN_FAILED(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
    DDSC_LOG_TRUST_RETURN_PERMISSIONS_TOKEN_FAILED_EC)

/*e
 * \brief Failed to return identity token
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_RETURN_IDENTITY_TOKEN_FAILED_EC      (DDSC_LOG_BASE + 1420)
#define DDSC_LOG_TRUST_RETURN_IDENTITY_TOKEN_FAILED(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
    DDSC_LOG_TRUST_RETURN_IDENTITY_TOKEN_FAILED_EC)

/*e
 * \brief Failed to unregister local participant
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_UNREGISTER_PARTICIPANT_FAILED_EC     (DDSC_LOG_BASE + 1421)
#define DDSC_LOG_TRUST_UNREGISTER_PARTICIPANT_FAILED(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
    DDSC_LOG_TRUST_UNREGISTER_PARTICIPANT_FAILED_EC)

/*e
 * \brief Failed to return permissions handle
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_RETURN_PERMISSIONS_HANDLE_FAILED_EC  (DDSC_LOG_BASE + 1422)
#define DDSC_LOG_TRUST_RETURN_PERMISSIONS_HANDLE_FAILED(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
    DDSC_LOG_TRUST_RETURN_PERMISSIONS_HANDLE_FAILED_EC)

/*e
 * \brief Failed to return identity handle
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_RETURN_IDENTITY_HANDLE_FAILED_EC     (DDSC_LOG_BASE + 1423)
#define DDSC_LOG_TRUST_RETURN_IDENTITY_HANDLE_FAILED(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
    DDSC_LOG_TRUST_RETURN_IDENTITY_HANDLE_FAILED_EC)

/*e
 * \brief Failed to return shared secret handle
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_RETURN_SHAREDSECRET_HANDLE_FAILED_EC (DDSC_LOG_BASE + 1424)
#define DDSC_LOG_TRUST_RETURN_SHAREDSECRET_HANDLE_FAILED(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
    DDSC_LOG_TRUST_RETURN_SHAREDSECRET_HANDLE_FAILED_EC)

/*e
 * \brief Failed to get authenticated peer credential token
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_GET_AUTHENTICATED_PEER_CREDENTIAL_TOKEN_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1425)
#define DDSC_LOG_TRUST_GET_AUTHENTICATED_PEER_CREDENTIAL_TOKEN_FAILED(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
    DDSC_LOG_TRUST_GET_AUTHENTICATED_PEER_CREDENTIAL_TOKEN_FAILED_EC)

/*e
 * \brief Failed to check access control remote DomainParticipant
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_CHECK_REMOTE_PARTICIPANT_FAILED_EC   (DDSC_LOG_BASE + 1426)
#define DDSC_LOG_TRUST_CHECK_REMOTE_PARTICIPANT_FAILED(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
    DDSC_LOG_TRUST_CHECK_REMOTE_PARTICIPANT_FAILED_EC)

/*e
 * \brief Failed to get shared secret
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_GET_SHARED_SECRET_FAILED_EC          (DDSC_LOG_BASE + 1427)
#define DDSC_LOG_TRUST_GET_SHARED_SECRET_FAILED(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
    DDSC_LOG_TRUST_GET_SHARED_SECRET_FAILED_EC)

/*e
 * \brief Failed to validate local DomainParticipant trust
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_PREPARE_LOCAL_PARTICIPANT_STATE_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1428)
#define DDSC_LOG_TRUST_PREPARE_LOCAL_PARTICIPANT_STATE_FAILED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_TRUST_PREPARE_LOCAL_PARTICIPANT_STATE_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to delete trust plugins
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_DELETE_TRUST_PLUGINS_EC              (DDSC_LOG_BASE + 1429)
#define DDSC_LOG_TRUST_DELETE_TRUST_PLUGINS(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_TRUST_DELETE_TRUST_PLUGINS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to set permissions credential and token
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_SET_PERMISSIONS_CREDENTIAL_TOKEN_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1430)
#define DDSC_LOG_TRUST_SET_PERMISSIONS_CREDENTIAL_TOKEN_FAILED(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
                DDSC_LOG_TRUST_SET_PERMISSIONS_CREDENTIAL_TOKEN_FAILED_EC)

/*e
 * \brief Failed to register matched remote DomainParticipant
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_REGISTER_MATCHED_REMOTE_PARTICIPANT_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1431)
#define DDSC_LOG_TRUST_REGISTER_MATCHED_REMOTE_PARTICIPANT_FAILED(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
    DDSC_LOG_TRUST_REGISTER_MATCHED_REMOTE_PARTICIPANT_FAILED_EC)

/*e
 * \brief Failed to get topic trust attributes
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_GET_TOPIC_TRUST_ATTRIBUTES_FAILED_EC (DDSC_LOG_BASE + 1432)
#define DDSC_LOG_TRUST_GET_TOPIC_TRUST_ATTRIBUTES_FAILED(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
                DDSC_LOG_TRUST_GET_TOPIC_TRUST_ATTRIBUTES_FAILED_EC)

/*e
 * \brief Failed to create trust check DataWriter
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_CHECK_CREATE_DATAWRITER_FAILED_EC    (DDSC_LOG_BASE + 1433)
#define DDSC_LOG_TRUST_CHECK_CREATE_DATAWRITER_FAILED(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
                DDSC_LOG_TRUST_CHECK_CREATE_DATAWRITER_FAILED_EC)

/*e
 * \brief Failed to return DataWriter trust attributes
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_RETURN_DATAWRITER_TRUST_ATTRIBUTES_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1434)
#define DDSC_LOG_TRUST_RETURN_DATAWRITER_TRUST_ATTRIBUTES_FAILED(\
                                                            level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
        DDSC_LOG_TRUST_RETURN_DATAWRITER_TRUST_ATTRIBUTES_FAILED_EC)

/*e
 * \brief Failed to get DataWriter trust attributes
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_GET_DATAWRITER_TRUST_ATTRIBUTES_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1435)
#define DDSC_LOG_TRUST_GET_DATAWRITER_TRUST_ATTRIBUTES_FAILED(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
                DDSC_LOG_TRUST_GET_DATAWRITER_TRUST_ATTRIBUTES_FAILED_EC)

/*e
 * \brief Failed to get DataReader trust attributes
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_GET_DATAREADER_TRUST_ATTRIBUTES_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1436)
#define DDSC_LOG_TRUST_GET_DATAREADER_TRUST_ATTRIBUTES_FAILED(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
                DDSC_LOG_TRUST_GET_DATAREADER_TRUST_ATTRIBUTES_FAILED_EC)

/*e
 * \brief Failed to create trust check DataReader
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_CHECK_CREATE_DATAREADER_FAILED_EC   (DDSC_LOG_BASE + 1437)
#define DDSC_LOG_TRUST_CHECK_CREATE_DATAREADER_FAILED(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
                DDSC_LOG_TRUST_CHECK_CREATE_DATAREADER_FAILED_EC)

/*e
 * \brief Handshake begin reply failed
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_BEGIN_HANDSHAKE_REPLY_EC           (DDSC_LOG_BASE + 1438)
#define DDSC_LOG_TRUST_BEGIN_HANDSHAKE_REPLY(level_,ex_,msg_)\
OSAPI_LOG_ENTRY_ADD_2STRING(level_,\
    DDSC_LOG_TRUST_BEGIN_HANDSHAKE_REPLY_EC, \
    OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
    "plugin-message",\
    (ex_)->message,\
    "message",msg_)

/*e
 * \brief Failed to create local DomainParticipant interceptor tokens
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_CREATE_LOCAL_PARTICIPANT_INTERCEPTOR_TOKENS_EC \
                                                            (DDSC_LOG_BASE + 1439)
#define DDSC_LOG_TRUST_CREATE_LOCAL_PARTICIPANT_INTERCEPTOR_TOKENS(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
                DDSC_LOG_TRUST_CREATE_LOCAL_PARTICIPANT_INTERCEPTOR_TOKENS_EC)

/*e
 * \brief Failed to create local DataWriter interceptor token
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_CREATE_LOCAL_DW_TOKEN_EC             (DDSC_LOG_BASE + 1440)
#define DDSC_LOG_TRUST_CREATE_LOCAL_DW_TOKEN(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
                DDSC_LOG_TRUST_CREATE_LOCAL_DW_TOKEN_EC)


/*e
 * \brief Failed to create local DataReader interceptor token
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_CREATE_LOCAL_DR_TOKEN_EC             (DDSC_LOG_BASE + 1441)
#define DDSC_LOG_TRUST_CREATE_LOCAL_DR_TOKEN(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
                DDSC_LOG_TRUST_CREATE_LOCAL_DR_TOKEN_EC)

/*e
 * \brief Failed to return handshake handle
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_RETURN_HANDSHAKE_HANDLE_FAILED_EC    (DDSC_LOG_BASE + 1442)
#define DDSC_LOG_TRUST_RETURN_HANDSHAKE_HANDLE_FAILED(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
                DDSC_LOG_TRUST_RETURN_HANDSHAKE_HANDLE_FAILED_EC)

/*e
 * \brief Failed to set remote DomainParticipant interceptor tokens
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_SET_REMOTE_PARTICIPANT_INTERCEPTOR_TOKENS_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1443)
#define DDSC_LOG_TRUST_SET_REMOTE_PARTICIPANT_INTERCEPTOR_TOKENS_FAILED(\
                                                                level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
                DDSC_LOG_TRUST_SET_REMOTE_PARTICIPANT_INTERCEPTOR_TOKENS_FAILED_EC)

/*e
 * \brief Failed to set remote DataReader interceptor tokens
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_SET_REMOTE_DATAREADER_INTERCEPTOR_TOKENS_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1444)
#define DDSC_LOG_TRUST_SET_REMOTE_DATAREADER_INTERCEPTOR_TOKENS_FAILED(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
                DDSC_LOG_TRUST_SET_REMOTE_DATAREADER_INTERCEPTOR_TOKENS_FAILED_EC)

/*e
 * \brief Failed to set remote DataWriter interceptor tokens
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_SET_REMOTE_DATAWRITER_INTERCEPTOR_TOKENS_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1445)
#define DDSC_LOG_TRUST_SET_REMOTE_DATAWRITER_INTERCEPTOR_TOKENS_FAILED(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
                DDSC_LOG_TRUST_SET_REMOTE_DATAWRITER_INTERCEPTOR_TOKENS_FAILED_EC)

/*e
 * \brief Failed to register local DataReader
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_REGISTER_LOCAL_DATAREADER_FAILED_EC  (DDSC_LOG_BASE + 1446)
#define DDSC_LOG_TRUST_REGISTER_LOCAL_DATAREADER_FAILED(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
                DDSC_LOG_TRUST_REGISTER_LOCAL_DATAREADER_FAILED_EC)

/*e
 * \brief Failed to register local DataWriter
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_REGISTER_LOCAL_DATAWRITER_FAILED_EC  (DDSC_LOG_BASE + 1447)
#define DDSC_LOG_TRUST_REGISTER_LOCAL_DATAWRITER_FAILED(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
                DDSC_LOG_TRUST_REGISTER_LOCAL_DATAWRITER_FAILED_EC)

/*e
 * \brief Failed to check remote DataWriter
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_CHECK_REMOTE_DATAWRITER_FAILED_EC    (DDSC_LOG_BASE + 1448)
#define DDSC_LOG_TRUST_CHECK_REMOTE_DATAWRITER_FAILED(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
                DDSC_LOG_TRUST_CHECK_REMOTE_DATAWRITER_FAILED_EC)

/*e
 * \brief Failed to check remote DataReader
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_CHECK_REMOTE_DATAREADER_FAILED_EC    (DDSC_LOG_BASE + 1449)
#define DDSC_LOG_TRUST_CHECK_REMOTE_DATAREADER_FAILED(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
                DDSC_LOG_TRUST_CHECK_REMOTE_DATAREADER_FAILED_EC)

/*e
 * \brief Failed to register matched remote DataWriter
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_REGISTER_MATCHED_REMOTE_DATAWRITER_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1450)
#define DDSC_LOG_TRUST_REGISTER_MATCHED_REMOTE_DATAWRITER_FAILED(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
                DDSC_LOG_TRUST_REGISTER_MATCHED_REMOTE_DATAWRITER_FAILED_EC)

/*e
 * \brief Failed to register matched remote DataReader
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_REGISTER_MATCHED_REMOTE_DATAREADER_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1451)
#define DDSC_LOG_TRUST_REGISTER_MATCHED_REMOTE_DATAREADER_FAILED(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
                DDSC_LOG_TRUST_REGISTER_MATCHED_REMOTE_DATAREADER_FAILED_EC)

/*e
 * \brief Failed to unregister DataReader
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_UNREGISTER_DATAREADER_FAILED_EC      (DDSC_LOG_BASE + 1452)
#define DDSC_LOG_TRUST_UNREGISTER_DATAREADER_FAILED(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
                DDSC_LOG_TRUST_UNREGISTER_DATAREADER_FAILED_EC)

/*e
 * \brief Failed to unregister DataWriter
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_UNREGISTER_DATAWRITER_FAILED_EC      (DDSC_LOG_BASE + 1453)
#define DDSC_LOG_TRUST_UNREGISTER_DATAWRITER_FAILED(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
                DDSC_LOG_TRUST_UNREGISTER_DATAWRITER_FAILED_EC)

/*e
 * \brief Failed to validate remote identity
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_VALIDATE_REMOTE_IDENTITY_EC          (DDSC_LOG_BASE + 1454)
#define DDSC_LOG_TRUST_VALIDATE_REMOTE_IDENTITY(level_,ex_,msg_)\
OSAPI_LOG_ENTRY_ADD_2STRING(level_,\
    DDSC_LOG_TRUST_VALIDATE_REMOTE_IDENTITY_EC, \
    OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
    "plugin-message",\
    (ex_)->message,\
    "message",msg_)

/*e
 * \brief Handshake begin request failed
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_BEGIN_HANDSHAKE_REQUEST_EC           (DDSC_LOG_BASE + 1455)
#define DDSC_LOG_TRUST_BEGIN_HANDSHAKE_REQUEST(level_,ex_,msg_)\
OSAPI_LOG_ENTRY_ADD_2STRING(level_,\
    DDSC_LOG_TRUST_BEGIN_HANDSHAKE_REQUEST_EC, \
    OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
    "plugin-message",\
    (ex_)->message,\
    "message",msg_)

/*e
 * \brief Handshake process failed
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_PROCESS_HANDSHAKE_EC                 (DDSC_LOG_BASE + 1456)
#define DDSC_LOG_TRUST_PROCESS_HANDSHAKE(level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
                DDSC_LOG_TRUST_PROCESS_HANDSHAKE_EC)

/*e
 * \brief Failed to create trust plugins
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_CREATE_TRUST_PLUGINS_FAILED_EC       (DDSC_LOG_BASE + 1457)
#define DDSC_LOG_TRUST_CREATE_TRUST_PLUGINS_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,\
                    DDSC_LOG_TRUST_CREATE_TRUST_PLUGINS_FAILED_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to parse trust property
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_PARSE_PROPERTY_FAILED_EC            (DDSC_LOG_BASE + 1458)
#define DDSC_LOG_TRUST_PARSE_PROPERTY_FAILED(level_,prop_)\
OSAPI_LOG_ENTRY_ADD_1STRING(level_,\
    DDSC_LOG_TRUST_PARSE_PROPERTY_FAILED_EC, \
    OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
    "property",\
    prop_)

/*e
 * \brief Error after remote trust DomainParticipant is ready
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_AFTER_REMOTE_PARTICIPANT_READY_FAILED_EC\
                                                            (DDSC_LOG_BASE + 1459)
#define DDSC_LOG_TRUST_AFTER_REMOTE_PARTICIPANT_READY_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,\
    DDSC_LOG_TRUST_AFTER_REMOTE_PARTICIPANT_READY_FAILED_EC,\
    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to remove remote DomainParticipant
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_REMOVE_REMOTE_PARTICIPANT_EC         (DDSC_LOG_BASE + 1460)
#define DDSC_LOG_TRUST_REMOVE_REMOTE_PARTICIPANT(level_)\
OSAPI_LOG_ENTRY_ADD(level_,\
    DDSC_LOG_TRUST_REMOVE_REMOTE_PARTICIPANT_EC,\
    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to assert DomainParticipant generic message
 * \ingroup DDSCLogCodesClass
 */
#define DDS_LOG_TRUST_ASSERT_PARTICIPANT_GENERIC_MESSAGE_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1461)
#define DDS_LOG_TRUST_ASSERT_PARTICIPANT_GENERIC_MESSAGE_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,\
    DDS_LOG_TRUST_ASSERT_PARTICIPANT_GENERIC_MESSAGE_FAILED_EC,\
    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to send handshake message
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_SEND_HANDSHAKE_EC                    (DDSC_LOG_BASE + 1462)
#define DDSC_LOG_TRUST_SEND_HANDSHAKE(level_,msg_)\
OSAPI_LOG_ENTRY_ADD_1STRING(level_,\
    DDSC_LOG_TRUST_SEND_HANDSHAKE_EC,\
    OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
    "error_message",\
    msg_)

/*e
 * \brief Failed to validate trust remote DomainParticipant
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_VALIDATE_REMOTE_PARTICIPANT_TRUST_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1463)
#define DDSC_LOG_VALIDATE_REMOTE_PARTICIPANT_TRUST_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,\
    DDSC_LOG_VALIDATE_REMOTE_PARTICIPANT_TRUST_FAILED_EC,\
    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to get remote DataReader bind properties
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_BIND_REMOTE_DATAREADER_FAILED_EC     (DDSC_LOG_BASE + 1464)
#define DDSC_LOG_TRUST_BIND_REMOTE_DATAREADER_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,DDSC_LOG_TRUST_BIND_REMOTE_DATAREADER_FAILED_EC,\
                                        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to register remote DataReader
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_REGISTER_REMOTE_DATAREADER_FAILED_EC  (DDSC_LOG_BASE + 1465)
#define DDSC_LOG_TRUST_REGISTER_REMOTE_DATAREADER_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,DDSC_LOG_TRUST_REGISTER_REMOTE_DATAREADER_FAILED_EC,\
                                            OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to get remote DataWriter bind properties
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_BIND_REMOTE_DATAWRITER_FAILED_EC      (DDSC_LOG_BASE + 1466)
#define DDSC_LOG_TRUST_BIND_REMOTE_DATAWRITER_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,DDSC_LOG_TRUST_BIND_REMOTE_DATAWRITER_FAILED_EC,\
                                            OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to register remote DataWriter
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_REGISTER_REMOTE_DATAWRITER_FAILED_EC  (DDSC_LOG_BASE + 1467)

#define DDSC_LOG_TRUST_REGISTER_REMOTE_DATAWRITER_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,DDSC_LOG_TRUST_REGISTER_REMOTE_DATAWRITER_FAILED_EC,\
                                            OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to validate remote DataWriter
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_VALIDATE_REMOTE_DATAWRITER_TRUST_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1468)
#define DDSC_LOG_TRUST_VALIDATE_REMOTE_DATAWRITER_TRUST_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,\
                    DDSC_LOG_TRUST_VALIDATE_REMOTE_DATAWRITER_TRUST_FAILED_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to validate remote DataReader
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_VALIDATE_REMOTE_DATAREADER_TRUST_FAILED_EC  (DDSC_LOG_BASE + 1469)
#define DDSC_LOG_TRUST_VALIDATE_REMOTE_DATAREADER_TRUST_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_, \
                    DDSC_LOG_TRUST_VALIDATE_REMOTE_DATAREADER_TRUST_FAILED_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to validate local DataWriter
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_VALIDATE_LOCAL_DATAWRITER_TRUST_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1470)
#define DDSC_LOG_TRUST_VALIDATE_LOCAL_DATAWRITER_TRUST_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_, \
                    DDSC_LOG_TRUST_VALIDATE_LOCAL_DATAWRITER_TRUST_FAILED_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Invalid DomainParticipant trust generic message
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_TRUST_INVALID_PARTICIPANT_GENERIC_MESSAGE_EC   (DDSC_LOG_BASE + 1471)
#define DDSC_TRUST_INVALID_PARTICIPANT_GENERIC_MESSAGE(level_,msg_)\
OSAPI_LOG_ENTRY_ADD_1STRING(level_,\
                            DDSC_TRUST_INVALID_PARTICIPANT_GENERIC_MESSAGE_EC,\
                            OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
                            "error message", msg_);

/*e
 * \brief Failed to prepare authentication request
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_PREPARE_AUTH_REQUEST_EC             (DDSC_LOG_BASE + 1472)
#define DDSC_LOG_TRUST_PREPARE_AUTH_REQUEST(level_)\
OSAPI_LOG_ENTRY_ADD(level_,\
    DDSC_LOG_TRUST_PREPARE_AUTH_REQUEST_EC,\
    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to prepare authentication handshake message
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_PREPARE_AUTH_HANDSHAKE_MESSAGE_EC    (DDSC_LOG_BASE + 1473)
#define DDSC_LOG_TRUST_PREPARE_AUTH_HANDSHAKE_MESSAGE(level_)\
OSAPI_LOG_ENTRY_ADD(level_,\
    DDSC_LOG_TRUST_PREPARE_AUTH_HANDSHAKE_MESSAGE_EC,\
    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to prepare endpoint interceptor message
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_PREPARE_ENDPOINT_INTERCEPTOR_MESSAGE_EC\
                                                            (DDSC_LOG_BASE + 1474)
#define DDSC_LOG_TRUST_PREPARE_ENDPOINT_INTERCEPTOR_MESSAGE(level_,kind_)\
OSAPI_LOG_ENTRY_ADD_1INT(level_,\
                         DDSC_LOG_TRUST_PREPARE_ENDPOINT_INTERCEPTOR_MESSAGE_EC,\
                         OSAPI_LOG_MSG_PN_X2_STD_PARAM,"kind",kind_)

/*e
 * \brief Invalid trust generic message reply
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_INVALID_GENERIC_MESSAGE_REPLY_EC    (DDSC_LOG_BASE + 1475)
#define DDSC_LOG_TRUST_INVALID_GENERIC_MESSAGE_REPLY(level_)\
OSAPI_LOG_ENTRY_ADD(level_,\
                    DDSC_LOG_TRUST_INVALID_GENERIC_MESSAGE_REPLY_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to finalize trust
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_FINALIZE_TRUST_FAILED_EC                  (DDSC_LOG_BASE + 1476)
#define DDSC_LOG_FINALIZE_TRUST_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,\
                    DDSC_LOG_FINALIZE_TRUST_FAILED_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to copy trust data holder
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_DATA_HOLDER_COPY_FAILED_EC         (DDSC_LOG_BASE + 1477)
#define DDSC_LOG_TRUST_DATA_HOLDER_COPY_FAILED(level_,type_)\
OSAPI_LOG_ENTRY_ADD_1STRING(level_,\
    DDSC_LOG_TRUST_DATA_HOLDER_COPY_FAILED_EC,\
    OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
    "type",type_)

/*e
 * \brief Invalid sample serialized size
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_INVALID_SAMPLE_MAX_SERIALIZED_SIZE_EC (DDSC_LOG_BASE + 1478)
#define DDSC_LOG_TRUST_INVALID_SAMPLE_MAX_SERIALIZED_SIZE(level_,size_)\
OSAPI_LOG_ENTRY_ADD_1INT(level_,\
                         DDSC_LOG_TRUST_INVALID_SAMPLE_MAX_SERIALIZED_SIZE_EC,\
                         OSAPI_LOG_MSG_PN_X2_STD_PARAM,"size",(RTI_INT32)size_)

/*e
 * \brief Failed to allocate memory
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_ALLOC_FAILED_EC                       (DDSC_LOG_BASE + 1479)
#define DDSC_LOG_TRUST_ALLOC_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,DDSC_LOG_TRUST_ALLOC_FAILED_EC, \
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to serialize trust data
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_SERIALIZE_DATA_FAILED_EC              (DDSC_LOG_BASE + 1480)
#define DDSC_LOG_TRUST_SERIALIZE_DATA_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,DDSC_LOG_TRUST_SERIALIZE_DATA_FAILED_EC, \
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to initialize trust
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_INITIALIZE_PARTICIPANT_TRUST_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1481)
#define DDSC_LOG_TRUST_INITIALIZE_PARTICIPANT_TRUST_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_, \
                    DDSC_LOG_TRUST_INITIALIZE_PARTICIPANT_TRUST_FAILED_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to initialize DomainParticipant trust data
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_INITIALIZE_PARTICIPANT_BUILTIN_DATA_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1482)
#define DDSC_LOG_TRUST_INITIALIZE_PARTICIPANT_BUILTIN_DATA_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_, \
                    DDSC_LOG_TRUST_INITIALIZE_PARTICIPANT_BUILTIN_DATA_FAILED_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Invalid DomainParticipant trust generic message
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_INVALID_PARTICIPANT_GENERIC_MESSAGE_EC \
                                                            (DDSC_LOG_BASE + 1483)
#define DDSC_LOG_TRUST_INVALID_PARTICIPANT_GENERIC_MESSAGE(level_)\
OSAPI_LOG_ENTRY_ADD(level_,DDSC_LOG_TRUST_INVALID_PARTICIPANT_GENERIC_MESSAGE_EC,\
                                                OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to advance trust sequence number
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_ADVANCE_SN_FAILED_EC                (DDSC_LOG_BASE + 1484)
#define DDSC_LOG_TRUST_ADVANCE_SN_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,DDSC_LOG_TRUST_ADVANCE_SN_FAILED_EC,\
                                                OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to write sample
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_WRITE_SAMPLE_FAILED_EC               (DDSC_LOG_BASE + 1485)
#define DDSC_LOG_TRUST_WRITE_SAMPLE_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,DDSC_LOG_TRUST_WRITE_SAMPLE_FAILED_EC,\
                                                OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to resend handshake because it is already in progress
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_RESEND_HANDSHAKE_IN_PROGRESS_EC      (DDSC_LOG_BASE + 1486)
#define DDSC_LOG_TRUST_RESEND_HANDSHAKE_IN_PROGRESS(level_)\
OSAPI_LOG_ENTRY_ADD(level_,DDSC_LOG_TRUST_RESEND_HANDSHAKE_IN_PROGRESS_EC,\
                                                OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to stop periodic handshake
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_RESEND_HANDSHAKE_STOP_FAILED_EC      (DDSC_LOG_BASE + 1487)
#define DDSC_LOG_TRUST_RESEND_HANDSHAKE_STOP_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,DDSC_LOG_TRUST_RESEND_HANDSHAKE_STOP_FAILED_EC,\
                                                OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to release DomainParticipant trust generic message
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_RELEASE_PARTICIPANT_GENERIC_MESSAGE_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1488)
#define DDSC_LOG_TRUST_RELEASE_PARTICIPANT_GENERIC_MESSAGE_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,\
                    DDSC_LOG_TRUST_RELEASE_PARTICIPANT_GENERIC_MESSAGE_FAILED_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to start periodic handshake
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_RESEND_HANDSHAKE_MESSAGE_START_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1489)
#define DDSC_LOG_TRUST_RESEND_HANDSHAKE_MESSAGE_START_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,\
                    DDSC_LOG_TRUST_RESEND_HANDSHAKE_MESSAGE_START_FAILED_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Invalid remote DomainParticipant trust status
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_UNEXPECTED_REMOTE_PARTICIPANT_STATUS_EC \
                                                            (DDSC_LOG_BASE + 1490)
#define DDSC_LOG_TRUST_UNEXPECTED_REMOTE_PARTICIPANT_STATUS(level_,status_)\
OSAPI_LOG_ENTRY_CREATE((level_), \
                        DDSC_LOG_TRUST_UNEXPECTED_REMOTE_PARTICIPANT_STATUS_EC,\
                        OSAPI_LOG_MSG_PN_X2_STD_PARAM,RTI_FALSE)\
    OSAPI_LOG_ENTRY_ADD_INT_HEX("status",(RTI_INT32)status_,RTI_TRUE)

/*e
 * \brief Failed to send DomainParticipant interceptor state
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_SEND_PARTICIPANT_INTERCEPTOR_STATE_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1491)
#define DDSC_LOG_TRUST_SEND_PARTICIPANT_INTERCEPTOR_STATE_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_, \
                    DDSC_LOG_TRUST_SEND_PARTICIPANT_INTERCEPTOR_STATE_FAILED_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief DomainParticipant authentication failed
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_PARTICIPANT_AUTHENTICATION_FAILED_EC  (DDSC_LOG_BASE + 1492)
#define DDSC_LOG_TRUST_PARTICIPANT_AUTHENTICATION_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,\
                    DDSC_LOG_TRUST_PARTICIPANT_AUTHENTICATION_FAILED_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to assert DomainParticipant generic message
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_ASSERT_PARTICIPANT_GENERIC_MESSAGE_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1493)
#define DDSC_LOG_TRUST_ASSERT_PARTICIPANT_GENERIC_MESSAGE_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_, \
                    DDSC_LOG_TRUST_ASSERT_PARTICIPANT_GENERIC_MESSAGE_FAILED_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to send handshake message
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_SEND_HANDSHAKE_MESSAGE_FAILED_EC     (DDSC_LOG_BASE + 1494)
#define DDSC_LOG_TRUST_SEND_HANDSHAKE_MESSAGE_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,DDSC_LOG_TRUST_SEND_HANDSHAKE_MESSAGE_FAILED_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Invalid interceptor tokens
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_INVALID_INTERCEPTOR_TOKENS_EC        (DDSC_LOG_BASE + 1495)
#define DDSC_LOG_TRUST_INVALID_INTERCEPTOR_TOKENS(\
                                        level_,loc_endp_,rem_endp_,svc_)\
OSAPI_LOG_ENTRY_ADD_3INT((level_),DDSC_LOG_TRUST_INVALID_INTERCEPTOR_TOKENS_EC,\
                        OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
                        "loc_endpoint",loc_endp_,\
                        "remote endpoint",rem_endp_,\
                        "svc id", svc_)

/*e
 * \brief Invalid interceptor token destination
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_INVALID_INTERCEPTOR_TOKENS_DESTINATION_EC \
                                                            (DDSC_LOG_BASE + 1496)
#define DDSC_LOG_TRUST_INVALID_INTERCEPTOR_TOKENS_DESTINATION(level_)\
OSAPI_LOG_ENTRY_ADD(level_,\
                    DDSC_LOG_TRUST_INVALID_INTERCEPTOR_TOKENS_DESTINATION_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to copy endpoint trust attributes
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_ENDPOINT_INTERNAL_ATTRIBUTES_CREATE_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1497)
#define DDSC_LOG_TRUST_ENDPOINT_INTERNAL_ATTRIBUTES_CREATE_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_, \
                    DDSC_LOG_TRUST_ENDPOINT_INTERNAL_ATTRIBUTES_CREATE_FAILED_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to send tokens to remote DomainParticipant
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_SEND_ENDPOINT_INTERCEPTOR_STATE_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1498)
#define DDSC_LOG_TRUST_SEND_ENDPOINT_INTERCEPTOR_STATE_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,\
                    DDSC_LOG_TRUST_SEND_ENDPOINT_INTERCEPTOR_STATE_FAILED_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create tokens for the remote DataReader
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_CREATE_LOCAL_DATAWRITER_INTERCEPTOR_TOKENS_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1499)
#define DDSC_LOG_TRUST_CREATE_LOCAL_DATAWRITER_INTERCEPTOR_TOKENS_FAILED(level_) \
OSAPI_LOG_ENTRY_ADD(level_,\
    DDSC_LOG_TRUST_CREATE_LOCAL_DATAWRITER_INTERCEPTOR_TOKENS_FAILED_EC,\
    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create tokens for the remote DataWriter
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_CREATE_LOCAL_DATAREADER_INTERCEPTOR_TOKENS_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1500)
#define DDSC_LOG_TRUST_CREATE_LOCAL_DATAREADER_INTERCEPTOR_TOKENS_FAILED(level_) \
OSAPI_LOG_ENTRY_ADD(level_,\
        DDSC_LOG_TRUST_CREATE_LOCAL_DATAREADER_INTERCEPTOR_TOKENS_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to return interceptor tokens
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_RETURN_INTERCEPTOR_TOKENS_FAILED_EC (DDSC_LOG_BASE + 1501)
#define DDSC_LOG_TRUST_RETURN_INTERCEPTOR_TOKENS_FAILED(level_) \
OSAPI_LOG_ENTRY_ADD(level_,DDSC_LOG_TRUST_RETURN_INTERCEPTOR_TOKENS_FAILED_EC,\
                                                OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to return authenticated peer credential token
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_RETURN_AUTHENTICATED_PEER_CREDENTIAL_TOKEN_FAILED_EC\
                                                            (DDSC_LOG_BASE + 1502)
#define DDSC_LOG_TRUST_RETURN_AUTHENTICATED_PEER_CREDENTIAL_TOKEN_FAILED(\
                                                                level_,ex_) \
DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(level_,ex_,\
    DDSC_LOG_TRUST_RETURN_AUTHENTICATED_PEER_CREDENTIAL_TOKEN_FAILED_EC)

/*e
 * \brief Failed to process interceptor tokens
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_PROCESS_REMOTE_INTERCEPTOR_TOKENS_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1503)
#define DDSC_LOG_TRUST_PROCESS_REMOTE_INTERCEPTOR_TOKENS_FAILED(level_) \
OSAPI_LOG_ENTRY_ADD(level_,\
    DDSC_LOG_TRUST_PROCESS_REMOTE_INTERCEPTOR_TOKENS_FAILED_EC,\
    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to unregister remote DataWriter
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_UNREGISTER_REMOTE_DATAWRITER_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1504)
#define DDSC_LOG_TRUST_UNREGISTER_REMOTE_DATAWRITER_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,DDSC_LOG_TRUST_UNREGISTER_REMOTE_DATAWRITER_FAILED_EC,\
                                            OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to unregister remote DataReader
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_UNREGISTER_REMOTE_DATAREADER_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1505)
#define DDSC_LOG_TRUST_UNREGISTER_REMOTE_DATAREADER_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,DDSC_LOG_TRUST_UNREGISTER_REMOTE_DATAREADER_FAILED_EC,\
                                            OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Invalid shared secret
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_INVALID_SHARED_SECRET_EC             (DDSC_LOG_BASE + 1506)
#define DDSC_LOG_TRUST_INVALID_SHARED_SECRET(level_)\
OSAPI_LOG_ENTRY_ADD(level_,DDSC_LOG_TRUST_INVALID_SHARED_SECRET_EC,\
                                                OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Unexpected DomainParticipant authentication error
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_PARTICIPANT_UNEXPECTED_AUTHENTICATION_ERROR_EC \
                                                            (DDSC_LOG_BASE + 1507)
#define DDSC_LOG_TRUST_PARTICIPANT_UNEXPECTED_AUTHENTICATION_ERROR(level_)\
OSAPI_LOG_ENTRY_ADD(level_, \
                    DDSC_LOG_TRUST_PARTICIPANT_UNEXPECTED_AUTHENTICATION_ERROR_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to prepare local DomainParticipant interceptor tokens
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_PREPARE_LOCAL_DP_TOKENS_EC            (DDSC_LOG_BASE + 1508)
#define DDSC_LOG_TRUST_PREPARE_LOCAL_DP_TOKENS(level_)\
OSAPI_LOG_ENTRY_ADD(level_,\
                    DDSC_LOG_TRUST_PREPARE_LOCAL_DP_TOKENS_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to assert trust property
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_ASSERT_PROPERTY_FAILED_EC              (DDSC_LOG_BASE + 1509)
#define DDSC_LOG_TRUST_ASSERT_PROPERTY_FAILED(level_,prop_)\
OSAPI_LOG_ENTRY_ADD_1STRING(level_,\
    DDSC_LOG_TRUST_ASSERT_PROPERTY_FAILED_EC,\
    OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
    "property",prop_)

/*e
 * \brief Failed to copy DomainParticipant trust attributes
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_PARTICIPANT_INTERNAL_ATTRIBUTES_CREATE_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1510)
#define DDSC_LOG_TRUST_PARTICIPANT_INTERNAL_ATTRIBUTES_CREATE_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_, \
    DDSC_LOG_TRUST_PARTICIPANT_INTERNAL_ATTRIBUTES_CREATE_FAILED_EC,\
    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Invalid DomainParticipant trust attributes
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_INVALID_PARTICIPANT_INTERNAL_ATTRIBUTES_EC \
                                                            (DDSC_LOG_BASE + 1511)
#define DDSC_LOG_TRUST_INVALID_PARTICIPANT_INTERNAL_ATTRIBUTES(level_)\
OSAPI_LOG_ENTRY_ADD(level_, \
    DDSC_LOG_TRUST_INVALID_PARTICIPANT_INTERNAL_ATTRIBUTES_EC,\
    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to encode outgoing serialized payload
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_TRANFORM_OUTGOING_SERIALIZED_PAYLOAD_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1512)
#define DDSC_LOG_TRUST_TRANFORM_OUTGOING_SERIALIZED_PAYLOAD_FAILED(level_,ex_) \
        DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(\
                            level_, \
                            ex_, \
                            DDSC_LOG_TRUST_TRANFORM_OUTGOING_SERIALIZED_PAYLOAD_FAILED_EC)

/*e
 * \brief Failed to decode incoming serialized payload
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_TRANFORM_INCOMING_SERIALIZED_PAYLOAD_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1513)
#define DDSC_LOG_TRUST_TRANFORM_INCOMING_SERIALIZED_PAYLOAD_FAILED(level_,ex_) \
        DDSC_LOG_ENTRY_ADD_TRUST_EXCEPTION(\
                            level_, \
                            ex_, \
                            DDSC_LOG_TRUST_TRANFORM_INCOMING_SERIALIZED_PAYLOAD_FAILED_EC)

/*e
 * \brief Failed to finalize trust properties
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_FINALIZE_AC_PLUGIN_PROPERTIES_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1514)
#define DDSC_LOG_TRUST_FINALIZE_AC_PLUGIN_PROPERTIES_FAILED(level_) \
        OSAPI_LOG_ENTRY_ADD(level_, \
                            DDSC_LOG_TRUST_FINALIZE_AC_PLUGIN_PROPERTIES_FAILED_EC,\
                            OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Error after remote trust DomainParticipant is authenticated
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_AFTER_REMOTE_PARTICIPANT_AUTHENTICATED_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1515)
#define DDSC_LOG_TRUST_AFTER_REMOTE_PARTICIPANT_AUTHENTICATED_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_, \
                    DDSC_LOG_TRUST_AFTER_REMOTE_PARTICIPANT_AUTHENTICATED_FAILED_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to copy remote DomainParticipant sample
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_CACHE_REMOTE_PARTICIPANT_SAMPLE_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1516)
#define DDSC_LOG_TRUST_CACHE_REMOTE_PARTICIPANT_SAMPLE_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_, \
                    DDSC_LOG_TRUST_CACHE_REMOTE_PARTICIPANT_SAMPLE_FAILED_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to finalize authentication handshake
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_FINALIZE_AUTH_HANDSHAKE_STATE_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1517)
#define DDSC_LOG_TRUST_FINALIZE_AUTH_HANDSHAKE_STATE_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_, \
                    DDSC_LOG_TRUST_FINALIZE_AUTH_HANDSHAKE_STATE_FAILED_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to finalize remote trust DomainParticipant record
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_FINALIZE_REMOTE_PARTICIPANT_TRUST_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1518)
#define DDSC_LOG_TRUST_FINALIZE_REMOTE_PARTICIPANT_TRUST_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_, \
                    DDSC_LOG_TRUST_FINALIZE_REMOTE_PARTICIPANT_TRUST_FAILED_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to finalize remote trust DomainParticipant
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_RESET_REMOTE_PARTICIPANT_TRUST_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1519)
#define DDSC_LOG_TRUST_RESET_REMOTE_PARTICIPANT_TRUST_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_, \
                    DDSC_LOG_TRUST_RESET_REMOTE_PARTICIPANT_TRUST_FAILED_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to assert authentication handshake state
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_ASSERT_AUTH_HANDSHAKE_STATE_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1520)
#define DDSC_LOG_TRUST_ASSERT_AUTH_HANDSHAKE_STATE_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_, \
                    DDSC_LOG_TRUST_ASSERT_AUTH_HANDSHAKE_STATE_FAILED_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Invalid authentication handshake state
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_INVALID_AUTH_HANDSHAKE_STATE_EC \
                                                            (DDSC_LOG_BASE + 1521)
#define DDSC_LOG_TRUST_INVALID_AUTH_HANDSHAKE_STATE(level_)\
OSAPI_LOG_ENTRY_ADD(level_, \
                    DDSC_LOG_TRUST_INVALID_AUTH_HANDSHAKE_STATE_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to send authentication request
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_SEND_AUTH_REQUEST_FAILED_EC          (DDSC_LOG_BASE + 1522)
#define DDSC_LOG_TRUST_SEND_AUTH_REQUEST_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,DDSC_LOG_TRUST_SEND_AUTH_REQUEST_FAILED_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to get authentication handshake state
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_GET_AUTH_HANDSHAKE_STATE_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1523)
#define DDSC_LOG_TRUST_GET_AUTH_HANDSHAKE_STATE_FAILED(level_,primary_)\
OSAPI_LOG_ENTRY_ADD_1INT(level_,\
                         DDSC_LOG_TRUST_GET_AUTH_HANDSHAKE_STATE_FAILED_EC,\
                         OSAPI_LOG_MSG_PN_X2_STD_PARAM,"primary",primary_)

/*e
 * \brief Failed to set authentication handshake state
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_SET_AUTH_HANDSHAKE_STATE_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1524)
#define DDSC_LOG_TRUST_SET_AUTH_HANDSHAKE_STATE_FAILED(level_,primary_)\
OSAPI_LOG_ENTRY_ADD_1INT(level_,\
                         DDSC_LOG_TRUST_SET_AUTH_HANDSHAKE_STATE_FAILED_EC,\
                         OSAPI_LOG_MSG_PN_X2_STD_PARAM,"primary",primary_)

/*e
 * \brief Failed to reauthenticate a remote DomainParticipant
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_REAUTH_REMOTE_PARTICIPANT_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1525)
#define DDSC_LOG_TRUST_REAUTH_REMOTE_PARTICIPANT_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,DDSC_LOG_TRUST_REAUTH_REMOTE_PARTICIPANT_FAILED_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to resend handshake message
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_RESEND_HANDSHAKE_MESSAGE_FAILED_EC \
                                                            (DDSC_LOG_BASE + 1526)
#define DDSC_LOG_TRUST_RESEND_HANDSHAKE_MESSAGE_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,DDSC_LOG_TRUST_RESEND_HANDSHAKE_MESSAGE_FAILED_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Maximum number of buffers exceeded
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_PBUFS_MAX_EXCEEDED_EC            (DDSC_LOG_BASE + 1527)
#define DDSC_LOG_TRUST_PBUFS_MAX_EXCEEDED(level_, bc_, max_pf_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_TRUST_PBUFS_MAX_EXCEEDED_EC,\
                            OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"buffer_count",(bc_),\
                            "max pbufs" , (max_pf_))


/*e
 * \brief Failed to create trust plugin
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_CREATE_TRUST_PLUGIN_EC           (DDSC_LOG_BASE + 1528)
#define DDSC_LOG_TRUST_CREATE_TRUST_PLUGIN(level_, name_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDSC_LOG_TRUST_CREATE_TRUST_PLUGIN_EC,\
                            OSAPI_LOG_MSG_PN_X2_STD_PARAM, "name", (name_))


/*e
 * \brief Failed to lookup trust plugin factory
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_LOOKUP_FACTORY_FAILED_EC           (DDSC_LOG_BASE + 1529)
#define DDSC_LOG_TRUST_LOOKUP_FACTORY_FAILED(level_, name_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDSC_LOG_TRUST_LOOKUP_FACTORY_FAILED_EC,\
                            OSAPI_LOG_MSG_PN_X2_STD_PARAM, "name", (name_))



/*******************************************************************************
 *                     LOOKUP FUNCTIONS RELATED
 ******************************************************************************/
/*e
 * \brief Fully qualified name does not contain substring "::"
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_NAME_LOOKUP_EC                           (DDSC_LOG_BASE + 1700)
#define DDSC_LOG_NAME_LOOKUP(level_,name_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDSC_LOG_NAME_LOOKUP_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"name",(name_))

/*e
 * \brief Failed to get an entity because the specified name exceeds the maximum
 *        length of 255 octets (excluding the terminating NUL)
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_ENTITY_NAME_TOO_LONG_EC                  (DDSC_LOG_BASE + 1701)
#define DDSC_LOG_ENTITY_NAME_TOO_LONG(level_,length_) \
OSAPI_LOG_ENTRY_ADD_1UINT((level_),DDSC_LOG_ENTITY_NAME_TOO_LONG_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"length",(length_))

/*******************************************************************************
 *                       P2P Participant Message Data
 ******************************************************************************/

/*e
 * \brief Unknown inter-participant message liveliness kind received
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_MESSAGE_DATA_UNKNOWN_LIVELINESS_KIND_EC  (DDSC_LOG_BASE + 1800)
#define DDSC_LOG_MESSAGE_DATA_UNKNOWN_LIVELINESS_KIND(level_,kind_) \
OSAPI_LOG_ENTRY_CREATE((level_),DDSC_LOG_MESSAGE_DATA_UNKNOWN_LIVELINESS_KIND_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_INT("kind",(kind_),RTI_TRUE)

/*e
 * \brief Wrong inter-participant message liveliness kind received
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_MESSAGE_DATA_WRONG_LIVELINESS_KIND_EC    (DDSC_LOG_BASE + 1801)
#define DDSC_LOG_MESSAGE_DATA_WRONG_LIVELINESS_KIND(level_,kind_) \
OSAPI_LOG_ENTRY_CREATE((level_),DDSC_LOG_MESSAGE_DATA_WRONG_LIVELINESS_KIND_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_INT("kind",(kind_),RTI_TRUE)

/*e
 * \brief Failed to update DataWriter lease duration
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_UPDATE_LEASE_DURATION_TIMER_EC           (DDSC_LOG_BASE + 1802)
#define DDSC_LOG_UPDATE_LEASE_DURATION_TIMER(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_UPDATE_LEASE_DURATION_TIMER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to write inter-participant liveliness message
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_PARTMESSAGE_WRITE_SAMPLE_FAILED_EC       (DDSC_LOG_BASE + 1803)
#define DDSC_LOG_PARTMESSAGE_WRITE_SAMPLE_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,DDSC_LOG_PARTMESSAGE_WRITE_SAMPLE_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to send inter-participant liveliness message
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_SEND_LIVELINESS_FAILED_EC                (DDSC_LOG_BASE + 1804)
#define DDSC_LOG_SEND_LIVELINESS_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,DDSC_LOG_SEND_LIVELINESS_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to update DataWriter liveliness
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_UPDATE_LIVELINESS_FAILED_EC               (DDSC_LOG_BASE + 1805)
#define DDSC_LOG_UPDATE_LIVELINESS_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,DDSC_LOG_UPDATE_LIVELINESS_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to refresh endpoint liveliness
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_REFRESH_LIVELINESS_FAILED_EC               (DDSC_LOG_BASE + 1806)
#define DDSC_LOG_REFRESH_LIVELINESS_FAILED(level_)\
OSAPI_LOG_ENTRY_ADD(level_,DDSC_LOG_REFRESH_LIVELINESS_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Two security endpoints are incompatible
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_TRUST_INCOMPATIBLE_ENDPOINT_TRUST_ATTRIBUTES_EC      (DDSC_LOG_BASE + 1807)
#define DDSC_LOG_TRUST_INCOMPATIBLE_ENDPOINT_TRUST_ATTRIBUTES(level_,tn_)\
OSAPI_LOG_ENTRY_ADD_1STRING(level_,DDSC_LOG_TRUST_INCOMPATIBLE_ENDPOINT_TRUST_ATTRIBUTES_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"topic name",tn_)

/*******************************************************************************
 *                            FLOW-CONTROLLER RELATED
 ******************************************************************************/
/*e
 * \brief Failed to create flow controller
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_FLOW_CONTROLLER_CREATE_EC               (DDSC_LOG_BASE + 1900)
#define DDSC_LOG_FLOW_CONTROLLER_CREATE(level_,name_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDSC_LOG_FLOW_CONTROLLER_CREATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"name",(name_))

/*e
 * \brief Failed to delete flow controller
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_FLOW_CONTROLLER_DELETE_EC               (DDSC_LOG_BASE + 1901)
#define DDSC_LOG_FLOW_CONTROLLER_DELETE(level_,name_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDSC_LOG_FLOW_CONTROLLER_DELETE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"name",(name_))

/*******************************************************************************
 *                            INTERPRETER RELATED
 ******************************************************************************/
/*e
 * \brief Extensible types sample state error
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_XTYPES_LOANED_SAMPLE_STATE_ERROR_EC              (DDSC_LOG_BASE + 2000)
#define DDSC_LOG_XTYPES_LOANED_SAMPLE_STATE_ERROR(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_XTYPES_LOANED_SAMPLE_STATE_ERROR_EC, OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Interpreter unexpected error
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_INTERPRETER_SUPPORT_UNEXPECTED_ERROR_EC          (DDSC_LOG_BASE + 2001)
#define DDSC_LOG_INTERPRETER_SUPPORT_UNEXPECTED_ERROR(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_INTERPRETER_SUPPORT_UNEXPECTED_ERROR_EC, OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Memory manager out of resources
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_MEMORY_MANAGER_OUT_OF_RESOURCES_EC               (DDSC_LOG_BASE + 2002)
#define DDSC_LOG_MEMORY_MANAGER_OUT_OF_RESOURCES(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_MEMORY_MANAGER_OUT_OF_RESOURCES_EC, OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Memory manager is not the owner of the data passed in
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_MEMORY_MANAGER_NOT_OWNER_EC               (DDSC_LOG_BASE + 2003)
#define DDSC_LOG_MEMORY_MANAGER_NOT_OWNER(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_MEMORY_MANAGER_NOT_OWNER_EC, OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief The wire manager was not able to delete a resource.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_MEMORY_MANAGER_DELETE_EC         (DDSC_LOG_BASE + 2004)
#define DDSC_LOG_MEMORY_MANAGER_DELETE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_MEMORY_MANAGER_DELETE_EC, \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)
/*******************************************************************************
 *                            DDS_BUFFER RELATED
 ******************************************************************************/
/*e
 * \brief Maximum size of buffer exceeded
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_BUFFER_MAX_EXCEEDED_EC            (DDSC_LOG_BASE + 2101)
#define DDSC_LOG_BUFFER_MAX_EXCEEDED(level_, size_) \
OSAPI_LOG_ENTRY_ADD_1UINT((level_),DDSC_LOG_BUFFER_MAX_EXCEEDED_EC,\
                            OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"size",(size_))

/*e
 * \brief Invalid buffer length
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_INVALID_BUFFER_LENGTH_EC          (DDSC_LOG_BASE + 2102)
#define DDSC_LOG_INVALID_BUFFER_LENGTH(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDSC_LOG_INVALID_BUFFER_LENGTH_EC,OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*******************************************************************************
 *                            INCOMPATIBLE QOS RELATED
 ******************************************************************************/
/*e
 * \brief Matching error because Presentation QoS is not compatible
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_INCOMPATIBLE_PRESENTATION_QOS_EC  (DDSC_LOG_BASE + 2201)
#define DDSC_LOG_INCOMPATIBLE_PRESENTATION_QOS(level_, scope_, coherent_, ordered_) \
OSAPI_LOG_ENTRY_ADD_3INT((level_),DDSC_LOG_INCOMPATIBLE_PRESENTATION_QOS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "access_scope",(RTI_INT32)(scope_),\
        "coherent_access",(RTI_INT32)(coherent_),\
        "ordered_access",(RTI_INT32)(ordered_))

/*e
 * \brief Two endpoints did not match because of incompatible partition QoS
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_INCOMPATIBLE_PARTITION_QOS_EC            (DDSC_LOG_BASE + 2202)
#define DDSC_LOG_INCOMPATIBLE_PARTITION_QOS(level_,topic_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDSC_LOG_INCOMPATIBLE_PARTITION_QOS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"topic",(topic_))

/*******************************************************************************
 *                     CHECKSUM RELATED ERRORS
 ******************************************************************************/
/*e
 * \brief A participant requires checksum, but the discovered participant
 *        does not send a checksum.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_CHECKSUM_REQUIRED_EC                     (DDSC_LOG_BASE + 2301)
#define DDSC_LOG_CHECKSUM_REQUIRED(level_,remote_,local_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_CHECKSUM_REQUIRED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "remote",(remote_),"local",(local_))

/*e
 * \brief A participant does not support checksum's sent by a discovered
 *        participant.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_CHECKSUM_INCOMPATIBLE_EC                 (DDSC_LOG_BASE + 2302)
#define DDSC_LOG_CHECKSUM_INCOMPATIBLE(level_,compute_,allow_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_CHECKSUM_INCOMPATIBLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "compute_crc",(compute_),"allow_",(allow_))

/*e
 * \brief The checksum class id for a custom checksum does match
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_CHECKSUM_INCONSISTENT_CLASS_EC           (DDSC_LOG_BASE + 2303)
#define DDSC_LOG_CHECKSUM_INCONSISTENT_CLASS(level_,class_,local_,remote_) \
OSAPI_LOG_ENTRY_ADD_3INT((level_),DDSC_LOG_CHECKSUM_INCONSISTENT_CLASS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "class",(class_),"local",(local_),"remote",(remote_))

/*e
 * \brief The computed_crc_kind is inconsistent with what is allowed or supported
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_CHECKSUM_INCONSISTENT_COMPUTE_EC         (DDSC_LOG_BASE + 2304)
#define DDSC_LOG_CHECKSUM_INCONSISTENT_COMPUTE(level_,compute_crc_,supported_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_CHECKSUM_INCONSISTENT_COMPUTE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "computed_crc",(compute_crc_),"supported",(supported_))

/*e
 * \brief The allowed_crc_mask is inconsistent with what is supported
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_CHECKSUM_INCONSISTENT_ALLOW_EC           (DDSC_LOG_BASE + 2305)
#define DDSC_LOG_CHECKSUM_INCONSISTENT_ALLOW(level_,allow_crc_,supported_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDSC_LOG_CHECKSUM_INCONSISTENT_ALLOW_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "allow_crc",(allow_crc_),"supported",(supported_))

/*e
 * \brief The token bucket properties for the flow controller are inconsistent.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_FLOWCONTROLLER_INCONSISTENT_PROP_EC           (DDSC_LOG_BASE + 2306)
#define DDSC_LOG_FLOWCONTROLLER_INCONSISTENT_PROP(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDSC_LOG_FLOWCONTROLLER_INCONSISTENT_PROP_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"token bucket property" ,kind_)

/*e
 * \brief Failed to delete the flow controller because it is still in use.
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DELETE_FLOWCONTROLLER_EC        (DDSC_LOG_BASE + 2307)
#define DDSC_LOG_DELETE_FLOWCONTROLLER(level_,name_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDSC_LOG_DELETE_FLOWCONTROLLER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"flow controller name" ,name_)
/*******************************************************************************
 *                            OSAPI RELATED
 ******************************************************************************/

/*e
 * \brief Failed to take a lock
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_LOCK_EC                               (DDSC_LOG_BASE + 2401)
#define DDSC_LOG_LOCK(level_, lock_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDSC_LOG_LOCK_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM, "lock", lock_)

/*e
 * \brief Failed to give a lock
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_UNLOCK_EC                               (DDSC_LOG_BASE + 2402)
#define DDSC_LOG_UNLOCK(level_, lock_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDSC_LOG_UNLOCK_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM, "unlock", lock_)

/*******************************************************************************
 *                          FILTERING RELATED
 ******************************************************************************/
#if DDS_FILTERING_ENABLED
/*e
 * \brief Failed to find the filter plugin factory specified in a DomainParticipant's QoS
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_FILTER_PLUGIN_FACTORY_NOT_FOUND_EC       (DDSC_LOG_BASE + 2501)
#define DDSC_LOG_FILTER_PLUGIN_FACTORY_NOT_FOUND(level_) \
OSAPI_LOG_ENTRY_ADD(level_,\
        DDSC_LOG_FILTER_PLUGIN_FACTORY_NOT_FOUND_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief A local writer failed to enable writer filtering for a reader
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_ENABLE_WRITER_FILTERING_EC               (DDSC_LOG_BASE + 2502)
#define DDSC_LOG_ENABLE_WRITER_FILTERING(level_) \
OSAPI_LOG_ENTRY_ADD(level_,\
        DDSC_LOG_ENABLE_WRITER_FILTERING_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief A local writer failed to create its writer filter
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_CREATE_WRITER_FILTER_EC                  (DDSC_LOG_BASE + 2503)
#define DDSC_LOG_CREATE_WRITER_FILTER(level_) \
OSAPI_LOG_ENTRY_ADD(level_,\
        DDSC_LOG_CREATE_WRITER_FILTER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief A local writer failed to evaluate a sample against the filters of its matched readers
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_EVALUATE_WRITER_FILTER_EC                (DDSC_LOG_BASE + 2504)
#define DDSC_LOG_EVALUATE_WRITER_FILTER(level_) \
OSAPI_LOG_ENTRY_ADD(level_,\
        DDSC_LOG_EVALUATE_WRITER_FILTER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief A local writer failed to check if a reader had filtered a sample
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_APPLY_READER_FILTER_EC                   (DDSC_LOG_BASE + 2505)
#define DDSC_LOG_APPLY_READER_FILTER(level_) \
OSAPI_LOG_ENTRY_ADD(level_,\
        DDSC_LOG_APPLY_READER_FILTER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to compile the content filter configured in a DataReader's QoS
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_COMPILE_CONTENT_FILTER_EC                (DDSC_LOG_BASE + 2506)
#define DDSC_LOG_COMPILE_CONTENT_FILTER(level_) \
OSAPI_LOG_ENTRY_ADD(level_,\
        DDSC_LOG_COMPILE_CONTENT_FILTER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Reader configured a content filter but content filtering is not enabled
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_CONTENT_FILTERING_NOT_ENABLED_EC         (DDSC_LOG_BASE + 2507)
#define DDSC_LOG_CONTENT_FILTERING_NOT_ENABLED(level_) \
OSAPI_LOG_ENTRY_ADD(level_,\
        DDSC_LOG_CONTENT_FILTERING_NOT_ENABLED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to evaluate a sample against a reader's content filter
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_EVALUATE_CONTENT_FILTER_EC               (DDSC_LOG_BASE + 2508)
#define DDSC_LOG_EVALUATE_CONTENT_FILTER(level_) \
OSAPI_LOG_ENTRY_ADD(level_,\
        DDSC_LOG_EVALUATE_CONTENT_FILTER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to deserialize the content filter info from the in-line QoS of a sample
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DESERIALIZE_CONTENT_FILTER_INFO_EC       (DDSC_LOG_BASE + 2509)
#define DDSC_LOG_DESERIALIZE_CONTENT_FILTER_INFO(level_) \
OSAPI_LOG_ENTRY_ADD(level_,\
        DDSC_LOG_DESERIALIZE_CONTENT_FILTER_INFO_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to change the configured content filter of a DataReader
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_CHANGE_CONTENT_FILTER_EC                 (DDSC_LOG_BASE + 2510)
#define DDSC_LOG_CHANGE_CONTENT_FILTER(level_) \
OSAPI_LOG_ENTRY_ADD(level_,\
        DDSC_LOG_CHANGE_CONTENT_FILTER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to update the content filter of a remote subscription
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_UPDATE_REMOTE_CONTENT_FILTER_EC          (DDSC_LOG_BASE + 2511)
#define DDSC_LOG_UPDATE_REMOTE_CONTENT_FILTER(level_) \
OSAPI_LOG_ENTRY_ADD(level_,\
        DDSC_LOG_UPDATE_REMOTE_CONTENT_FILTER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief DomainParticipant failed to create its filter plugin
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_CREATE_FILTER_PLUGIN_EC                  (DDSC_LOG_BASE + 2512)
#define DDSC_LOG_CREATE_FILTER_PLUGIN(level_) \
OSAPI_LOG_ENTRY_ADD(level_,\
        DDSC_LOG_CREATE_FILTER_PLUGIN_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief DomainParticipant failed to finalize its filter plugin
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_FINALIZE_FILTER_PLUGIN_EC                (DDSC_LOG_BASE + 2513)
#define DDSC_LOG_FINALIZE_FILTER_PLUGIN(level_) \
OSAPI_LOG_ENTRY_ADD(level_,\
        DDSC_LOG_FINALIZE_FILTER_PLUGIN_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to deserialize the content filter property in the subscription
 *        built-in topic data for a remote subscription.
 *
 * \details
 * This warning is expected if the remote subscription has configured a content
 * filter which exceeds the resource limits of the local participant. No writers
 * on this participant will be able perform writer filtering for this remote
 * subscription. The resource limits can be increased to allow the filter to be
 * stored locally if writer filtering is desired.
 *
 * \ingroup DDSCLogCodesClass
 */
#define DDSC_LOG_DESERIALIZE_REMOTE_CONTENT_FILTER_EC     (DDSC_LOG_BASE + 2514)
#define DDSC_LOG_DESERIALIZE_REMOTE_CONTENT_FILTER(level_) \
OSAPI_LOG_ENTRY_ADD(level_,\
        DDSC_LOG_DESERIALIZE_REMOTE_CONTENT_FILTER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)
#endif /* DDS_FILTERING_ENABLED */

#endif /* dds_c_log_h */
