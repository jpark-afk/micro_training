/*
 * FILE: disc_dpse_log.h - DPSE Log definitions
 *
 * (c) Copyright 2011-2015 Real-Time Innovations,
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
 * 05jan2014,tk Updated log-codes
 * 30apr2011,tk Created
 */

/*e
 * \file
 * \defgroup DPSELogCodesClass DPSE 
 * \brief DPSE. ModuleID = 10
 * \ingroup LoggingModule
 */

#ifndef disc_dpse_log_h
#define disc_dpse_log_h

#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif

/*e
 * \brief A peer address string specifies an invalid address 
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_INVALID_PEER_ADDRESS_EC                     (DPSE_LOG_BASE + 1)
#define DPSE_LOG_INVALID_PEER_ADDRESS(level_,address_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DPSE_LOG_INVALID_PEER_ADDRESS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"address",(address_))

/*e
 * \brief Failed to create the publisher for a participant discovery writer 
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_CREATE_DISCOVERY_PUBLISHER_EC               (DPSE_LOG_BASE + 2)
#define DPSE_LOG_CREATE_DISCOVERY_PUBLISHER(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_CREATE_DISCOVERY_PUBLISHER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create the subscriber for a participant discovery datareader
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_CREATE_DISCOVERY_SUBSCRIBER_EC              (DPSE_LOG_BASE + 3)
#define DPSE_LOG_CREATE_DISCOVERY_SUBSCRIBER(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_CREATE_DISCOVERY_SUBSCRIBER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to register a built-in participant discovery type
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_REGISTER_TYPE_EC                            (DPSE_LOG_BASE + 4)
#define DPSE_LOG_REGISTER_TYPE(level_,type_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DPSE_LOG_REGISTER_TYPE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"type",(type_))

/*e
 * \brief Failed to create a topic for the built-in participant discovery topic
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_CREATE_TOPIC_EC                             (DPSE_LOG_BASE + 5)
#define DPSE_LOG_CREATE_TOPIC(level_,topic_,type_) \
OSAPI_LOG_ENTRY_ADD_2STRING((level_),DPSE_LOG_CREATE_TOPIC_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"topic",(topic_),"type",(type_))

/*e
 * \brief Failed to create a DataWriter for participant discovery  
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_CREATE_WRITER_EC                            (DPSE_LOG_BASE + 6)
#define DPSE_LOG_CREATE_WRITER(level_,topic_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DPSE_LOG_CREATE_WRITER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"topic",(topic_))

/*e
 * \brief Failed to create a DataReader for participant discovery
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_CREATE_READER_EC                            (DPSE_LOG_BASE + 7)
#define DPSE_LOG_CREATE_READER(level_,topic_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DPSE_LOG_CREATE_READER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"topic",(topic_))

/*e
 * \brief Failed to dispose a participant discovery instance
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_DISPOSE_EC                                  (DPSE_LOG_BASE + 8)
#define DPSE_LOG_DISPOSE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_DISPOSE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to dispose a participant discovery instance upon deletion
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_ANNOUNCE_LOCAL_PARTICIPANT_DELETION_EC      (DPSE_LOG_BASE + 9)
#define DPSE_LOG_ANNOUNCE_LOCAL_PARTICIPANT_DELETION(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_ANNOUNCE_LOCAL_PARTICIPANT_DELETION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to delete a participant discovery DataWriter
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_DELETE_WRITER_EC                           (DPSE_LOG_BASE + 10)
#define DPSE_LOG_DELETE_WRITER(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_DELETE_WRITER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to delete a participant discovery Publisher
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_DELETE_PUBLISHER_EC                        (DPSE_LOG_BASE + 11)
#define DPSE_LOG_DELETE_PUBLISHER(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_DELETE_PUBLISHER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to delete a participant discovery DataReader
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_DELETE_READER_EC                           (DPSE_LOG_BASE + 12)
#define DPSE_LOG_DELETE_READER(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_DELETE_READER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to delete a participant discovery Topic
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_DELETE_TOPIC_EC                            (DPSE_LOG_BASE + 13)
#define DPSE_LOG_DELETE_TOPIC(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_DELETE_TOPIC_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)
/*e
 * \brief Failed to delete a participant discovery Subscriber
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_DELETE_SUBSCRIBER_EC                       (DPSE_LOG_BASE + 14)
#define DPSE_LOG_DELETE_SUBSCRIBER(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_DELETE_SUBSCRIBER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief The list keeping track of asserted participant keys
 */
#define DPSE_KEYLIST_OBJECT                                   (1)

/*e
 * \brief The list keeping track of asserted participant names
 */
#define DPSE_PARTICIPANTMAMES_OBJECT                          (2)

/*e
 * \brief The index keeping track of asserted participant names
 */
#define DPSE_PARTICIPANTNAMEINDEX_OBJECT                      (3)

/*e
 * \brief The participant announcement timeout object
 */
#define DPSE_PARTICIPANTANNOUCEMENT_OBJECT                    (4)

/*e
 * \brief A discovery plugin object
 */
#define DPSE_DISCOVERYPLUGIN_OBJECT                           (5)

/*e
 * \brief ParticipantBuiltinTopicData
 */
#define DPSE_PARTICIPANTBUILTINTOPICDATA_OBJECT               (6)

/*e
 * \brief ParticipantBuiltinTopicData entity-name object
 */
#define DPSE_PARTICIPANTENTITYNAME_OBJECT                     (7)

/*e
 * \brief The index keeping track of enabled participant names
 */
#define DPSE_ENABLEDPARTICIPANTINDEX_OBJECT                   (8)

/*e
 * \brief The pool of asserted participants
 */
#define DPSE_PARTICIPANTPOOL_OBJECT                          (10)

/*e
 * \brief Failed to allocate an object of the specified kind
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_OBJECT_ALLOCATE_EC                         (DPSE_LOG_BASE + 15)
#define DPSE_LOG_OBJECT_ALLOCATE(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPSE_LOG_OBJECT_ALLOCATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief Failed to initialize an object of the specified kind
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_OBJECT_INITIALIZE_EC                       (DPSE_LOG_BASE + 16)
#define DPSE_LOG_OBJECT_INITIALIZE(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPSE_LOG_OBJECT_INITIALIZE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief Failed to finalize an object of the specified kind
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_OBJECT_FINALIZE_EC                         (DPSE_LOG_BASE + 17)
#define DPSE_LOG_OBJECT_FINALIZE(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPSE_LOG_OBJECT_FINALIZE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief Failed to delete an object of the specified kind
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_OBJECT_DELETE_EC                           (DPSE_LOG_BASE + 18)
#define DPSE_LOG_OBJECT_DELETE(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPSE_LOG_OBJECT_DELETE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief The object was invalid in the context it was used
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_OBJECT_INVALID_EC                          (DPSE_LOG_BASE + 19)
#define DPSE_LOG_OBJECT_INVALID(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPSE_LOG_OBJECT_INVALID_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief Failed to set the CDR stream position
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_CDR_SET_POSITION_EC                        (DPSE_LOG_BASE + 20)
#define DPSE_LOG_CDR_SET_POSITION(level_,pos_) \
OSAPI_LOG_ENTRY_ADD_1UINT((level_),DPSE_LOG_CDR_SET_POSITION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"pos",(pos_))

/*e
 * \brief CDR Long kind
 */
#define DPSE_LOG_CDR_LONG_KIND                                       1

/*e
 * \brief CDR PID kind, the pid argument specifies which pid
 */
#define DPSE_LOG_CDR_PID_KIND                                       2

/*e
 * \brief Failed to serialize the specified kind
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_CDR_SERIALIZE_EC                           (DPSE_LOG_BASE + 21)
#define DPSE_LOG_CDR_SERIALIZE(level_,kind_,pid_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DPSE_LOG_CDR_SERIALIZE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "kind",(kind_),"pid",(pid_))

/*e
 * \brief Failed to deserialize the specified kind
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_CDR_DESERIALIZE_EC                         (DPSE_LOG_BASE + 22)
#define DPSE_LOG_CDR_DESERIALIZE(level_,kind_,pid_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DPSE_LOG_CDR_DESERIALIZE_EC,\
              OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
              "kind",(kind_),"pid",(pid_))

/*e
 * \brief Failed to serialize a GUID key parameter
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_SERIALIZE_GUID_EC                          (DPSE_LOG_BASE + 23)
#define DPSE_LOG_SERIALIZE_GUID(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_SERIALIZE_GUID_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to serialize a Builtin Endpoint Mask parameter
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_SERIALIZE_BUILTIN_ENDPOINTS_EC             (DPSE_LOG_BASE + 24)
#define DPSE_LOG_SERIALIZE_BUILTIN_ENDPOINTS(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_SERIALIZE_BUILTIN_ENDPOINTS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to serialize a Protocol Version parameter
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_SERIALIZE_PROTOCOL_VERSION_EC              (DPSE_LOG_BASE + 25)
#define DPSE_LOG_SERIALIZE_PROTOCOL_VERSION(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_SERIALIZE_PROTOCOL_VERSION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to serialize a Vendor ID parameter
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_SERIALIZE_VENDOR_ID_EC                     (DPSE_LOG_BASE + 26)
#define DPSE_LOG_SERIALIZE_VENDOR_ID(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_SERIALIZE_VENDOR_ID_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to serialize a Default Unicast Locator parameter
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_SERIALIZE_DEFAULT_UNICAST_EC               (DPSE_LOG_BASE + 27)
#define DPSE_LOG_SERIALIZE_DEFAULT_UNICAST(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_SERIALIZE_DEFAULT_UNICAST_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to serialize a Meta Unicast Locator parameter
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_SERIALIZE_META_UNICAST_EC                  (DPSE_LOG_BASE + 28)
#define DPSE_LOG_SERIALIZE_META_UNICAST(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_SERIALIZE_META_UNICAST_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to serialize a Meta Multicast Locator parameter
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_SERIALIZE_META_MULTICAST_EC                (DPSE_LOG_BASE + 29)
#define DPSE_LOG_SERIALIZE_META_MULTICAST(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_SERIALIZE_META_MULTICAST_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to serialize a Lease Duration parameter
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_SERIALIZE_LEASE_DURATION_EC                (DPSE_LOG_BASE + 30)
#define DPSE_LOG_SERIALIZE_LEASE_DURATION(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_SERIALIZE_LEASE_DURATION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to serialize a Product Version parameter
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_SERIALIZE_PRODUCT_VERSION_EC               (DPSE_LOG_BASE + 31)
#define DPSE_LOG_SERIALIZE_PRODUCT_VERSION(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_SERIALIZE_PRODUCT_VERSION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to serialize a Product Version parameter
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_DESERIALIZE_PRODUCT_VERSION_EC             (DPSE_LOG_BASE + 32)
#define DPSE_LOG_DESERIALIZE_PRODUCT_VERSION(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_DESERIALIZE_PRODUCT_VERSION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to serialize a Participant Name parameter
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_SERIALIZE_PARTICIPANT_NAME_EC              (DPSE_LOG_BASE + 33)
#define DPSE_LOG_SERIALIZE_PARTICIPANT_NAME(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_SERIALIZE_PARTICIPANT_NAME_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to deserialize a GUID key parameter
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_DESERIALIZE_GUID_EC                        (DPSE_LOG_BASE + 34)
#define DPSE_LOG_DESERIALIZE_GUID(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_DESERIALIZE_GUID_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to deserialize a Builtin Endpoint Mask parameter
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_DESERIALIZE_BUILTIN_ENDPOINTS_EC           (DPSE_LOG_BASE + 35)
#define DPSE_LOG_DESERIALIZE_BUILTIN_ENDPOINTS(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_DESERIALIZE_BUILTIN_ENDPOINTS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to deserialize a Protocol Version parameter
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_DESERIALIZE_PROTOCOL_VERSION_EC            (DPSE_LOG_BASE + 36)
#define DPSE_LOG_DESERIALIZE_PROTOCOL_VERSION(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_DESERIALIZE_PROTOCOL_VERSION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to deserialize a Vendor ID parameter
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_DESERIALIZE_VENDOR_ID_EC                   (DPSE_LOG_BASE + 37)
#define DPSE_LOG_DESERIALIZE_VENDOR_ID(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_DESERIALIZE_VENDOR_ID_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Cannot deserialize another locator parameter, having reached maximum 
 *        of 4 unicast or 4 multicast locators
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_DESERIALIZE_TOO_MANY_LOCATORS_EC           (DPSE_LOG_BASE + 38)
#define DPSE_LOG_DESERIALIZE_TOO_MANY_LOCATORS(level_,length_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPSE_LOG_DESERIALIZE_TOO_MANY_LOCATORS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"length",(length_))

/*e
 * \brief Failed to deserialize a Participant Name parameter
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_DESERIALIZE_PARTICIPANT_NAME_EC            (DPSE_LOG_BASE + 39)
#define DPSE_LOG_DESERIALIZE_PARTICIPANT_NAME(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_DESERIALIZE_PARTICIPANT_NAME_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to deserialize a parameter with an unknown ID
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_DESERIALIZE_UNKNOWN_PID_EC                 (DPSE_LOG_BASE + 40)
#define DPSE_LOG_DESERIALIZE_UNKNOWN_PID(level_,pid_,length_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DPSE_LOG_DESERIALIZE_UNKNOWN_PID_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"pid",(pid_),"length",(length_))

/*e
 * \brief Failed to send a participant discovery announcement
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_ANNOUNCEMENT_EC                            (DPSE_LOG_BASE + 41)
#define DPSE_LOG_ANNOUNCEMENT(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_ANNOUNCEMENT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to schedule an event to send the next participant discovery 
 * announcement 
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_UPDATE_PARTICIPANT_ASSERT_PERIOD_EC        (DPSE_LOG_BASE + 42)
#define DPSE_LOG_UPDATE_PARTICIPANT_ASSERT_PERIOD(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_UPDATE_PARTICIPANT_ASSERT_PERIOD_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to advance the sequence number of a participant discovery 
 * announcement 
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_ADVANCE_SN_EC                              (DPSE_LOG_BASE + 43)
#define DPSE_LOG_ADVANCE_SN(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_ADVANCE_SN_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to write a participant discovery announcement message
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_ANNOUNCE_WRITE_EC                          (DPSE_LOG_BASE + 44)
#define DPSE_LOG_ANNOUNCE_WRITE(level_,ddsrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPSE_LOG_ANNOUNCE_WRITE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"ddsrc",(ddsrc_))

/*e
 * \brief Failed to schedule an event to send the next participant discovery 
 * announcment 
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_SCHEDULE_FAST_ASSERTION_EC                 (DPSE_LOG_BASE + 45)
#define DPSE_LOG_SCHEDULE_FAST_ASSERTION(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_SCHEDULE_FAST_ASSERTION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to take a sample from a participant discovery DataReader
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_PARTICIPANT_TAKE_EC                        (DPSE_LOG_BASE + 46)
#define DPSE_LOG_PARTICIPANT_TAKE(level_,ddsrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPSE_LOG_PARTICIPANT_TAKE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"ddsrc",(ddsrc_))

/*e
 * \brief Failed to assert remote participant
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_ASSERT_REMOTE_PARTICIPANT_EC               (DPSE_LOG_BASE + 47)
#define DPSE_LOG_ASSERT_REMOTE_PARTICIPANT(level_,ddsrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPSE_LOG_ASSERT_REMOTE_PARTICIPANT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"ddsrc",(ddsrc_))

/*e
 * \brief Failed to assert and complete discovery of a remote participant
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_ON_ASSERT_REMOTE_PARTICIPANT_EC            (DPSE_LOG_BASE + 48)
#define DPSE_LOG_ON_ASSERT_REMOTE_PARTICIPANT(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_ON_ASSERT_REMOTE_PARTICIPANT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to add an anonymous route 
 * \details Results in a DomainParticipant not having a destination to which it 
 *          should have sent participant discovery announcements
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_ADD_ANONYMOUS_ROUTE_EC                     (DPSE_LOG_BASE + 49)
#define DPSE_LOG_ADD_ANONYMOUS_ROUTE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_ADD_ANONYMOUS_ROUTE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to delete an anonymous route
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_DELETE_ANONYMOUS_ROUTE_EC                  (DPSE_LOG_BASE + 50)
#define DPSE_LOG_DELETE_ANONYMOUS_ROUTE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_DELETE_ANONYMOUS_ROUTE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Exceeded resource limit, remote_participant_allocation
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_MAX_REMOTE_PARTICIPANT_EC                  (DPSE_LOG_BASE + 51)
#define DPSE_LOG_MAX_REMOTE_PARTICIPANT(level_,length_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPSE_LOG_MAX_REMOTE_PARTICIPANT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"length",(length_))

/*e
 * \brief Failed to refresh liveliness for a remote participant
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_REFRESH_REMOTE_PARTICIPANT_EC              (DPSE_LOG_BASE + 52)
#define DPSE_LOG_REFRESH_REMOTE_PARTICIPANT(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_REFRESH_REMOTE_PARTICIPANT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to reset liveliness for a remote participant
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_RESET_REMOTE_PARTICIPANT_EC                (DPSE_LOG_BASE + 53)
#define DPSE_LOG_RESET_REMOTE_PARTICIPANT(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_RESET_REMOTE_PARTICIPANT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Received a participant discovery announcement with invalid state
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_INVALID_DISCOVERY_SAMPLE_EC                (DPSE_LOG_BASE + 54)
#define DPSE_LOG_INVALID_DISCOVERY_SAMPLE(level_,val_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPSE_LOG_INVALID_DISCOVERY_SAMPLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"state",(val_))

/*e
 * \brief Failed to return a loan on a participant discovery announcement
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_RETURN_DISCOVERY_SAMPLE_EC                 (DPSE_LOG_BASE + 55)
#define DPSE_LOG_RETURN_DISCOVERY_SAMPLE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_RETURN_DISCOVERY_SAMPLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to serialize a Multicast Locator parameter
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_SERIALIZE_MULTICAST_EC                     (DPSE_LOG_BASE + 56)
#define DPSE_LOG_SERIALIZE_MULTICAST(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_SERIALIZE_MULTICAST_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to serialize a Multicast Locator parameter
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_GET_DDS_PROPERTIES_EC                      (DPSE_LOG_BASE + 57)
#define DPSE_LOG_GET_DDS_PROPERTIES(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_GET_DDS_PROPERTIES_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief A DDS Topic entity
 */
#define DPSE_TOPIC_ENTITY                                               1

/*e
 * \brief A DDS Publisher entity
 */
#define DPSE_PUBLISHER_ENTITY                                           2

/*e
 * \brief A DDS Subscriber entity
 */
#define DPSE_SUBSCRIBER_ENTITY                                          3

/*e
 * \brief Failed to enable the specified entity kind
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_ENTITY_ENABLE_EC                           (DPSE_LOG_BASE + 58)
#define DPSE_LOG_ENTITY_ENABLE(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPSE_LOG_ENTITY_ENABLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"kind",(kind_))


/*e
 * \brief The default unicast locator sequence
 */
#define DPSE_LOG_DEFAULT_UNICAST_LOCATOR_SEQUENCE                 1

/*e
 * \brief The default multicast locator sequence
 */
#define DPSE_LOG_DEFAULT_MULTICAST_LOCATOR_SEQUENCE               2

/*e
 * \brief The meta unicast locator sequence
 */
#define DPSE_LOG_META_UNICAST_LOCATOR_SEQUENCE                    3

/*e
 * \brief The meta multicast locator sequence
 */
#define DPSE_LOG_META_MULTICAST_LOCATOR_SEQUENCE                  4

/*e
 * \brief Failed to set the maximum length of a sequence of the specified kind.
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_SEQUENCE_SETMAX_EC                         (DPSE_LOG_BASE + 59)
#define DPSE_LOG_SEQ_SETMAX(level_,kind_,length_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DPSE_LOG_SEQUENCE_SETMAX_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "kind",(kind_),"length",(length_))

/*e
 * \brief Failed to set the length of a sequence of the specified kind.
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_SEQUENCE_SETLENGTH_EC                      (DPSE_LOG_BASE + 60)
#define DPSE_LOG_SEQ_SETLENGTH(level_,kind_,length_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DPSE_LOG_SEQUENCE_SETLENGTH_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
        "kind",(kind_),"length",(length_))

/*e
 * \brief Failed to get a reference at the specified index for a sequence of
 *        the specified kind.
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_SEQUENCE_GETREF_EC                         (DPSE_LOG_BASE + 61)
#define DPSE_LOG_SEQ_GETREF(level_,kind_,index_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DPSE_LOG_SEQUENCE_GETREF_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
        "kind",(kind_),"index",(index_))

/*e
 * \brief Failed to initialize a sequence of the specified kind.
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_SEQUENCE_INITIALIZE_EC                     (DPSE_LOG_BASE + 62)
#define DPSE_LOG_SEQ_INITIALIZE(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPSE_LOG_SEQUENCE_INITIALIZE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief Failed to finalize a sequence of the specified kind.
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_SEQUENCE_FINALIZE_EC                       (DPSE_LOG_BASE + 63)
#define DPSE_LOG_SEQ_FINALIZE(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPSE_LOG_SEQUENCE_FINALIZE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief Failed to copy a sequence of the specified kind.
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_SEQUENCE_COPY_EC                           (DPSE_LOG_BASE + 64)
#define DPSE_LOG_SEQ_COPY(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPSE_LOG_SEQUENCE_COPY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief Failed to get timer.
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_GET_TIMER_EC                               (DPSE_LOG_BASE + 65)
#define DPSE_LOG_GET_TIMER(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_GET_TIMER_EC,\
                    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief The locator kind is not supported by any transport registered with the
 *        domain participant and is dropped
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_UNSUPPORTED_LOCATOR_EC                     (DPSE_LOG_BASE + 66)
#define DPSE_LOG_UNSUPPORTED_LOCATOR(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPSE_LOG_UNSUPPORTED_LOCATOR_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"kind",(kind_))

/*e
 * \brief A participant with incompatible checksum was discovered and ignored
 * \ingroup DPSELogCodesClass
 */
#define DPSE_LOG_CHECKSUM_INCOMPATIBLE_PARTICIPANT_IGNORED_EC (DPDE_LOG_BASE + 67)
#define DPSE_LOG_CHECKSUM_INCOMPATIBLE_PARTICIPANT_IGNORED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPSE_LOG_CHECKSUM_INCOMPATIBLE_PARTICIPANT_IGNORED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

#endif

