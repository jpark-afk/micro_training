/*
 * FILE: disc_dpde_log.h - DPDE log definitions
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
 * 16aug2011,tk Created
 */

/*e
 * \file
 * \defgroup DPDELogCodesClass DPDE 
 * \brief DPDE. ModuleID = 11
 * \ingroup LoggingModule
 */
#ifndef disc_dpde_log_h
#define disc_dpde_log_h

#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif

/*e
 * \brief Failed to set current offset of stream
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_CDR_SET_OFFSET_EC                           (DPDE_LOG_BASE + 1)
#define DPDE_LOG_CDR_SET_OFFSET(level_,offset_) \
OSAPI_LOG_ENTRY_ADD_1UINT((level_),DPDE_LOG_CDR_SET_OFFSET_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "offset",(offset_))

/*e
 * \brief Failed to serialize Entity Name parameter
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_SERIALIZE_ENTITY_NAME_EC                    (DPDE_LOG_BASE + 2)
#define DPDE_LOG_SERIALIZE_ENTITY_NAME(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPDE_LOG_SERIALIZE_ENTITY_NAME_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to serialize Topic Name parameter
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_SERIALIZE_TOPIC_NAME_EC                     (DPDE_LOG_BASE + 3)
#define DPDE_LOG_SERIALIZE_TOPIC_NAME(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPDE_LOG_SERIALIZE_TOPIC_NAME_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to serialize Type Name parameter
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_SERIALIZE_TYPE_NAME_EC                      (DPDE_LOG_BASE + 4)
#define DPDE_LOG_SERIALIZE_TYPE_NAME(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPDE_LOG_SERIALIZE_TYPE_NAME_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to serialize GUID key 
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_SERIALIZE_GUID_EC                           (DPDE_LOG_BASE + 5)
#define DPDE_LOG_SERIALIZE_GUID(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPDE_LOG_SERIALIZE_GUID_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to serialize Default Unicast Locator parameter
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_SERIALIZE_DEFAULT_UNICAST_EC                (DPDE_LOG_BASE + 6)
#define DPDE_LOG_SERIALIZE_DEFAULT_UNICAST(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPDE_LOG_SERIALIZE_DEFAULT_UNICAST_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to serialize Protocol Version parameter
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_SERIALIZE_PROTOCOL_VERSION_EC               (DPDE_LOG_BASE + 7)
#define DPDE_LOG_SERIALIZE_PROTOCOL_VERSION(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPDE_LOG_SERIALIZE_PROTOCOL_VERSION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to deserialize Protocol Version parameter
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_DESERIALIZE_PROTOCOL_VERSION_EC             (DPDE_LOG_BASE + 8)
#define DPDE_LOG_DESERIALIZE_PROTOCOL_VERSION(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPDE_LOG_DESERIALIZE_PROTOCOL_VERSION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to deserialize Vendor ID parameter
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_DESERIALIZE_VENDOR_ID_EC                    (DPDE_LOG_BASE + 9)
#define DPDE_LOG_DESERIALIZE_VENDOR_ID(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPDE_LOG_DESERIALIZE_VENDOR_ID_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to serialize Vendor ID parameter
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_SERIALIZE_VENDOR_ID_EC                     (DPDE_LOG_BASE + 10)
#define DPDE_LOG_SERIALIZE_VENDOR_ID(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPDE_LOG_SERIALIZE_VENDOR_ID_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to serialize Product Version parameter
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_SERIALIZE_PRODUCT_VERSION_EC               (DPDE_LOG_BASE + 11)
#define DPDE_LOG_SERIALIZE_PRODUCT_VERSION(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPDE_LOG_SERIALIZE_PRODUCT_VERSION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to serialize Lease Duration parameter
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_SERIALIZE_LEASE_DURATION_EC                (DPDE_LOG_BASE + 12)
#define DPDE_LOG_SERIALIZE_LEASE_DURATION(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPDE_LOG_SERIALIZE_LEASE_DURATION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create Publisher for built-in endpoint DataWriters
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_CREATE_PUBLISHER_EC                        (DPDE_LOG_BASE + 13)
#define DPDE_LOG_CREATE_PUBLISHER(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPDE_LOG_CREATE_PUBLISHER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create Subscriber for built-in endpoint DataReaders
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_CREATE_SUBSCRIBER_EC                       (DPDE_LOG_BASE + 14)
#define DPDE_LOG_CREATE_SUBSCRIBER(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPDE_LOG_CREATE_SUBSCRIBER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create built-in endpoint for participant discovery after 
 * creating local user DomainParticipant 
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_CREATE_PARTICIPANT_DISCOVERY_EC            (DPDE_LOG_BASE + 15)
#define DPDE_LOG_CREATE_PARTICIPANT_DISCOVERY(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPDE_LOG_CREATE_PARTICIPANT_DISCOVERY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create built-in publication endpoint after creating local 
 * user DomainParticipant 
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_CREATE_PUBLICATION_DISCOVERY_EC            (DPDE_LOG_BASE + 16)
#define DPDE_LOG_CREATE_PUBLICATION_DISCOVERY(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPDE_LOG_CREATE_PUBLICATION_DISCOVERY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create built-in subscription endpoint after creating local 
 * DomainParticipant 
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_CREATE_SUBSCRIPTION_DISCOVERY_EC           (DPDE_LOG_BASE + 17)
#define DPDE_LOG_CREATE_SUBSCRIPTION_DISCOVERY(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPDE_LOG_CREATE_SUBSCRIPTION_DISCOVERY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to delete subscription built-in topic
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_DELETE_SUBSCRIPTION_TOPIC_EC               (DPDE_LOG_BASE + 18)
#define DPDE_LOG_DELETE_SUBSCRIPTION_TOPIC(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPDE_LOG_DELETE_SUBSCRIPTION_TOPIC_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to delete publication built-in topic
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_DELETE_PUBLICATION_TOPIC_EC                (DPDE_LOG_BASE + 19)
#define DPDE_LOG_DELETE_PUBLICATION_TOPIC(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPDE_LOG_DELETE_PUBLICATION_TOPIC_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to delete participant built-in topic
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_DELETE_PARTICIPANT_TOPIC_EC                (DPDE_LOG_BASE + 20)
#define DPDE_LOG_DELETE_PARTICIPANT_TOPIC(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPDE_LOG_DELETE_PARTICIPANT_TOPIC_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to dispose built-in participant when 
 * deleting DomainParticipant 
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_DISPOSE_PARTICIPANT_EC                     (DPDE_LOG_BASE + 21)
#define DPDE_LOG_DISPOSE_PARTICIPANT(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPDE_LOG_DISPOSE_PARTICIPANT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Out of memory to allocate discovery plugin
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_ALLOCATE_DPDE_EC                           (DPDE_LOG_BASE + 22)
#define DPDE_LOG_ALLOCATE_DPDE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPDE_LOG_ALLOCATE_DPDE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to serialize Builtin Endpoint Mask parameter
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_SERIALIZE_BUILTIN_ENDPOINTS_EC             (DPDE_LOG_BASE + 23)
#define DPDE_LOG_SERIALIZE_BUILTIN_ENDPOINTS(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPDE_LOG_SERIALIZE_BUILTIN_ENDPOINTS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to deserialize Builtin Endpoint Mask parameter
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_DESERIALIZE_BUILTIN_ENDPOINTS_EC           (DPDE_LOG_BASE + 24)
#define DPDE_LOG_DESERIALIZE_BUILTIN_ENDPOINTS(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPDE_LOG_DESERIALIZE_BUILTIN_ENDPOINTS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM )

/*e
 * \brief Deserialized a parameter of unknown ID
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_DESERIALIZE_UNKNOWN_PID_EC                 (DPDE_LOG_BASE + 25)
#define DPDE_LOG_DESERIALIZE_UNKNOWN_PID(level_,pid_,length_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DPDE_LOG_DESERIALIZE_UNKNOWN_PID_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"pid",(pid_),"length",(length_))

/*e
 * \brief Failed to write a dynamic participant discovery message
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_ANNOUNCEMENT_EC                            (DPDE_LOG_BASE + 26)
#define DPDE_LOG_ANNOUNCEMENT(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPDE_LOG_ANNOUNCEMENT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to schedule an event to assert the next participant 
 * discovery announcement 
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_UPDATE_PARTICIPANT_ASSERT_PERIOD_EC        (DPDE_LOG_BASE + 27)
#define DPDE_LOG_UPDATE_PARTICIPANT_ASSERT_PERIOD(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPDE_LOG_UPDATE_PARTICIPANT_ASSERT_PERIOD_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to advance the sequence number of a participant discovery 
 * announcement 
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_ADVANCE_SN_EC                              (DPDE_LOG_BASE + 28)
#define DPDE_LOG_ADVANCE_SN(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPDE_LOG_ADVANCE_SN_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to write initial participant discovery announcement 
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_ANNOUNCE_WRITE_EC                          (DPDE_LOG_BASE + 29)
#define DPDE_LOG_ANNOUNCE_WRITE(level_,ddsrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPDE_LOG_ANNOUNCE_WRITE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"ddsrc",(ddsrc_))

/*e
 * \brief Failed to schedule event for asserting participant discovery 
 * announcements
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_SCHEDULE_FAST_ASSERTION_EC                 (DPDE_LOG_BASE + 30)
#define DPDE_LOG_SCHEDULE_FAST_ASSERTION(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPDE_LOG_SCHEDULE_FAST_ASSERTION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to register a built-in type for discovery
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_REGISTER_TYPE_EC                           (DPDE_LOG_BASE + 32)
#define DPDE_LOG_REGISTER_TYPE(level_,type_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DPDE_LOG_REGISTER_TYPE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"type",(type_))

/*e
 * \brief Failed to create a topic for discovery
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_CREATE_TOPIC_EC                            (DPDE_LOG_BASE + 33)
#define DPDE_LOG_CREATE_TOPIC(level_,topic_,type_) \
OSAPI_LOG_ENTRY_ADD_2STRING((level_),DPDE_LOG_CREATE_TOPIC_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"topic",(topic_),"type",(type_))

/*e
 * \brief Failed to create a built-in DataWriter for discovery
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_CREATE_WRITER_EC                           (DPDE_LOG_BASE + 34)
#define DPDE_LOG_CREATE_WRITER(level_,topic_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DPDE_LOG_CREATE_WRITER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"topic",(topic_))

/*e
 * \brief Failed to create a built-in DataReader for discovery
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_CREATE_READER_EC                           (DPDE_LOG_BASE + 35)
#define DPDE_LOG_CREATE_READER(level_,topic_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DPDE_LOG_CREATE_READER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"topic",(topic_))

/*e
 * \brief Failed to assert and complete discovery of a remote participant
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_ASSERT_REMOTE_PARTICIPANT_EC               (DPDE_LOG_BASE + 36)
#define DPDE_LOG_ASSERT_REMOTE_PARTICIPANT(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPDE_LOG_ASSERT_REMOTE_PARTICIPANT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to enable and complete discovery of a remote participant
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_ENABLE_REMOTE_PARTICIPANT_EC               (DPDE_LOG_BASE + 37)
#define DPDE_LOG_ENABLE_REMOTE_PARTICIPANT(level_,ddsrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPDE_LOG_ENABLE_REMOTE_PARTICIPANT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"ddsrc",(ddsrc_))

/*e
 * \brief Failed to refresh liveliness for a discovered remote participant
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_REFRESH_REMOTE_PARTICIPANT_EC              (DPDE_LOG_BASE + 38)
#define DPDE_LOG_REFRESH_REMOTE_PARTICIPANT(level_,ddsrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPDE_LOG_REFRESH_REMOTE_PARTICIPANT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"ddsrc",(ddsrc_))

/*e
 * \brief Failed to take a sample from a participant discovery DataReader  
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_TAKE_PARTICIPANT_SAMPLE_EC                 (DPDE_LOG_BASE + 39)
#define DPDE_LOG_TAKE_PARTICIPANT_SAMPLE(level_,ddsrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPDE_LOG_TAKE_PARTICIPANT_SAMPLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"ddsrc",(ddsrc_))

/*e
 * \brief Received a participant discovery announcement with invalid state
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_INVALID_PARTICIPANT_SAMPLE_EC              (DPDE_LOG_BASE + 40)
#define DPDE_LOG_INVALID_PARTICIPANT_SAMPLE(level_,val_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPDE_LOG_INVALID_PARTICIPANT_SAMPLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"state",(val_))

/*e
 * \brief Failed to return a loan on a participant discovery sample
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_RETURN_PARTICIPANT_SAMPLE_EC               (DPDE_LOG_BASE + 41)
#define DPDE_LOG_RETURN_PARTICIPANT_SAMPLE(level_,ddsrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPDE_LOG_RETURN_PARTICIPANT_SAMPLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"ddsrc",(ddsrc_))

/*e
 * \brief Failed to dispose an instance for a publication  
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_DISPOSE_PUBLICATION_EC                     (DPDE_LOG_BASE + 42)
#define DPDE_LOG_DISPOSE_PUBLICATION(level_,ddsrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPDE_LOG_DISPOSE_PUBLICATION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"ddsrc",(ddsrc_))

/*e
 * \brief Failed to dispose an instance of a subscription
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_DISPOSE_SUBSCRIPTION_EC                    (DPDE_LOG_BASE + 43)
#define DPDE_LOG_DISPOSE_SUBSCRIPTION(level_,ddsrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPDE_LOG_DISPOSE_SUBSCRIPTION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"ddsrc",(ddsrc_))

/*e
 * \brief Failed to assert and complete discovery of a remote publication
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_ASSERT_REMOTE_PUBLICATION_EC               (DPDE_LOG_BASE + 44)
#define DPDE_LOG_ASSERT_REMOTE_PUBLICATION(level_,ddsrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPDE_LOG_ASSERT_REMOTE_PUBLICATION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"ddsrc",(ddsrc_))

/*e
 * \brief Failed to assert and complete discovery of a remote subscription
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_ASSERT_REMOTE_SUBSCRIPTION_EC              (DPDE_LOG_BASE + 45)
#define DPDE_LOG_ASSERT_REMOTE_SUBSCRIPTION(level_,ddsrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPDE_LOG_ASSERT_REMOTE_SUBSCRIPTION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"ddsrc",(ddsrc_))

/*e
 * \brief Failed to take a sample from a publication discovery DataReader
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_TAKE_PUBLICATION_SAMPLE_EC                 (DPDE_LOG_BASE + 46)
#define DPDE_LOG_TAKE_PUBLICATION_SAMPLE(level_,ddsrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPDE_LOG_TAKE_PUBLICATION_SAMPLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"ddsrc",(ddsrc_))

/*e
 * \brief Failed to dispose or unregister a remote publication instance
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_REMOVE_REMOTE_PUBLICATION_EC               (DPDE_LOG_BASE + 47)
#define DPDE_LOG_REMOVE_REMOTE_PUBLICATION(level_,ddsrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPDE_LOG_REMOVE_REMOTE_PUBLICATION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"ddsrc",(ddsrc_))

/*e
 * \brief Received a publication discovery announcement with invalid state
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_INVALID_PUBLICATION_SAMPLE_EC              (DPDE_LOG_BASE + 48)
#define DPDE_LOG_INVALID_PUBLICATION_SAMPLE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPDE_LOG_INVALID_PUBLICATION_SAMPLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to return a loan on a publication discovery sample
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_RETURN_PUBLICATION_SAMPLE_EC               (DPDE_LOG_BASE + 49)
#define DPDE_LOG_RETURN_PUBLICATION_SAMPLE(level_,ddsrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPDE_LOG_RETURN_PUBLICATION_SAMPLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"ddsrc",(ddsrc_))

/*e
 * \brief Failed to take a sample from a subscription discovery DataReader
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_TAKE_SUBSCRIPTION_SAMPLE_EC                (DPDE_LOG_BASE + 51)
#define DPDE_LOG_TAKE_SUBSCRIPTION_SAMPLE(level_,ddsrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPDE_LOG_TAKE_SUBSCRIPTION_SAMPLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"ddsrc",(ddsrc_))

/*e
 * \brief Received a subscription discovery sample with invalid state
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_INVALID_SUBSCRIPTION_SAMPLE_EC             (DPDE_LOG_BASE + 52)
#define DPDE_LOG_INVALID_SUBSCRIPTION_SAMPLE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPDE_LOG_INVALID_SUBSCRIPTION_SAMPLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to return a loan on a subscription discovery sample
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_RETURN_SUBSCRIPTION_SAMPLE_EC              (DPDE_LOG_BASE + 53)
#define DPDE_LOG_RETURN_SUBSCRIPTION_SAMPLE(level_,ddsrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPDE_LOG_RETURN_SUBSCRIPTION_SAMPLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"ddsrc",(ddsrc_))

/*e
 * \brief Failed to dispose or unregister a remote subscription instance
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_REMOVE_REMOTE_SUBSCRIPTION_EC              (DPDE_LOG_BASE + 54)
#define DPDE_LOG_REMOVE_REMOTE_SUBSCRIPTION(level_,ddsrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPDE_LOG_REMOVE_REMOTE_SUBSCRIPTION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"ddsrc",(ddsrc_))

/*e
 * \brief A locator sequence is full, the remaining locators of the specified
 *        kind are dropped
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_LOCATORS_FULL_EC                          (DPDE_LOG_BASE + 55)
#define DPDE_LOG_LOCATORS_FULL(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPDE_LOG_LOCATORS_FULL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"kind",(kind_))

/*e
 * \brief Failed to deserialize a locator of the specified kind
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_DESERIALIZE_LOCATOR_EC                     (DPDE_LOG_BASE + 56)
#define DPDE_LOG_DESERIALIZE_LOCATOR(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPDE_LOG_DESERIALIZE_LOCATOR_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"kind",(kind_))

/*e
 * \brief The locator kind is not supported by any transport registered with the
 *        domain participant and is dropped
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_UNSUPPORTED_LOCATOR_EC                     (DPDE_LOG_BASE + 57)
#define DPDE_LOG_UNSUPPORTED_LOCATOR(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPDE_LOG_UNSUPPORTED_LOCATOR_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"kind",(kind_))

/*e
 * \brief Failed to add the specified entity as a peer
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_ADD_PEER_EC                                (DPDE_LOG_BASE + 58)
#define DPDE_LOG_ADD_PEER(level_,entity_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPDE_LOG_ADD_PEER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"entity_",(entity_))


/*e
 * \brief Failed to add the specified entity as a peer
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_REMOVE_PEER_EC                             (DPDE_LOG_BASE + 59)
#define DPDE_LOG_REMOVE_PEER(level_,entity_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DPDE_LOG_REMOVE_PEER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"entity_",(entity_))


/*e
 * \brief Failed to serialize Presentation parameter
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_SERIALIZE_PRESENTATION_EC                  (DPDE_LOG_BASE + 60)
#define DPDE_LOG_SERIALIZE_PRESENTATION(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPDE_LOG_SERIALIZE_PRESENTATION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief A participant with incompatible checksum was discovered and ignored
 * \ingroup DPDELogCodesClass
 */
#define DPDE_LOG_CHECKSUM_INCOMPATIBLE_PARTICIPANT_IGNORED_EC (DPDE_LOG_BASE + 61)
#define DPDE_LOG_CHECKSUM_INCOMPATIBLE_PARTICIPANT_IGNORED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DPDE_LOG_CHECKSUM_INCOMPATIBLE_PARTICIPANT_IGNORED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

#endif

