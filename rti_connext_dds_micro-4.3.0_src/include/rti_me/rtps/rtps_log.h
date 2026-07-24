/*
 * FILE: rtps_log.h - RTPS Log definitions
 *
 * Copyright (c) 2013-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*e
 * \file
 * \brief RTPS module log codes
 *
 * \details
 * Log codes of the RTPS module
 */
#ifndef rtps_log_h
#define rtps_log_h


#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif

/*e
 * \defgroup RTPSLogCodesClass RTPS
 * \brief Real-Time Publish-Subscribe. ModuleID = 6
 * \ingroup LoggingModule
 */

/*e
 * \brief Failed to initialize an RTPS interface
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_INITIALIZE_INTERFACE_EC                     (RTPS_LOG_BASE + 1)
#define RTPS_LOG_INITIALIZE_INTERFACE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_INITIALIZE_INTERFACE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to allocate heap memory for internal resources
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_ALLOCATE_EC                                 (RTPS_LOG_BASE + 2)
#define RTPS_LOG_ALLOCATE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_ALLOCATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create a database table for storing RTPS route info
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_CREATE_ROUTE_TABLE_EC                       (RTPS_LOG_BASE + 3)
#define RTPS_LOG_CREATE_ROUTE_TABLE(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),RTPS_LOG_CREATE_ROUTE_TABLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to create database index for route-peer info
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_DB_CREATE_ROUTE_PEER_INDEX_EC               (RTPS_LOG_BASE + 4)
#define RTPS_LOG_DB_CREATE_ROUTE_PEER_INDEX(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),RTPS_LOG_DB_CREATE_ROUTE_PEER_INDEX_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to create database index for route-transport info
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_DB_CREATE_ROUTE_XPORT_INDEX_EC              (RTPS_LOG_BASE + 5)
#define RTPS_LOG_DB_CREATE_ROUTE_XPORT_INDEX(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_DB_CREATE_ROUTE_XPORT_INDEX_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create database index for bind-peer info
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_DB_CREATE_BIND_PEER_INDEX_EC                (RTPS_LOG_BASE + 6)
#define RTPS_LOG_DB_CREATE_BIND_PEER_INDEX(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_DB_CREATE_BIND_PEER_INDEX_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to add an entry to a database index
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_INDEX_ADD_ENTRY_EC                          (RTPS_LOG_BASE + 8)
#define RTPS_LOG_INDEX_ADD_ENTRY(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_INDEX_ADD_ENTRY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to select all entries of a database table
* \details Failure reason given by database return code (dbrc)
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_DB_SELECT_ALL_EC                            (RTPS_LOG_BASE + 9)
#define RTPS_LOG_DB_SELECT_ALL(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),RTPS_LOG_DB_SELECT_ALL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Matching entry not found in a database table.
 * \details Failure reason given by database return code (dbrc)
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_DB_SELECT_MATCH_EC                         (RTPS_LOG_BASE + 10)
#define RTPS_LOG_DB_SELECT_MATCH(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),RTPS_LOG_DB_SELECT_MATCH_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief No matching entries found in a database table.
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_DB_SELECT_RANGE_EC                         (RTPS_LOG_BASE + 11)
#define RTPS_LOG_DB_SELECT_RANGE(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),RTPS_LOG_DB_SELECT_RANGE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to create database entry for route.
 *
 * \details May have exceeded
 *        DomainParticipantQos.resource_limits.matching_writer_reader_pair_allocation,
 *        DataWriterQos.writer_resource_limits.max_remote_readers, or
 *        DataReaderQos.reader_resource_limits.max_remote_writers
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_DB_CREATE_ROUTE_ENTRY_EC                   (RTPS_LOG_BASE + 12)
#define RTPS_LOG_DB_CREATE_ROUTE_ENTRY(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),RTPS_LOG_DB_CREATE_ROUTE_ENTRY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to insert entry into route table
 * \details Failure reason given by database return code (dbrc)
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_DB_INSERT_ROUTE_ENTRY_EC                   (RTPS_LOG_BASE + 13)
#define RTPS_LOG_DB_INSERT_ROUTE_ENTRY(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),RTPS_LOG_DB_INSERT_ROUTE_ENTRY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to remove entry from route table
 * \details Failure reason given by database return code (dbrc)
 */
#define RTPS_LOG_DB_REMOVE_ROUTE_ENTRY_EC                   (RTPS_LOG_BASE + 14)
#define RTPS_LOG_DB_REMOVE_ROUTE_ENTRY(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),RTPS_LOG_DB_REMOVE_ROUTE_ENTRY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to delete entry from route table
 * \details Failure reason given by database return code (dbrc)
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_DB_DELETE_ROUTE_ENTRY_EC                   (RTPS_LOG_BASE + 15)
#define RTPS_LOG_DB_DELETE_ROUTE_ENTRY(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),RTPS_LOG_DB_DELETE_ROUTE_ENTRY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to create database entry for bind.
 *
 * \details If DomainParticipant, may have exceeded
 *        DomainParticipantQos.resource_limits.matching_writer_reader_pair_allocation.
 *        Failure reason given by database return code (dbrc).
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_DB_CREATE_BIND_ENTRY_EC                    (RTPS_LOG_BASE + 16)
#define RTPS_LOG_DB_CREATE_BIND_ENTRY(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),RTPS_LOG_DB_CREATE_BIND_ENTRY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to insert an entry into the bind table.
 * \details Failure reason given by database return code (dbrc)
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_DB_INSERT_BIND_ENTRY_EC                    (RTPS_LOG_BASE + 17)
#define RTPS_LOG_DB_INSERT_BIND_ENTRY(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),RTPS_LOG_DB_INSERT_BIND_ENTRY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to remove an entry from the bind table.
 * \details Failure reason given by database return code (dbrc)
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_DB_REMOVE_BIND_ENTRY_EC                    (RTPS_LOG_BASE + 18)
#define RTPS_LOG_DB_REMOVE_BIND_ENTRY(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),RTPS_LOG_DB_REMOVE_BIND_ENTRY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to remove an entry from the external bind table.
 * \details Failure reason given by database return code (dbrc)
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_DB_REMOVE_EXTERNAL_BIND_ENTRY_EC           (RTPS_LOG_BASE + 19)
#define RTPS_LOG_DB_REMOVE_EXTERNAL_BIND_ENTRY(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),RTPS_LOG_DB_REMOVE_EXTERNAL_BIND_ENTRY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to delete entry from external bind table
 * \details Failure reason given by database return code (dbrc)
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_DB_DELETE_EXTERNAL_BIND_ENTRY_EC           (RTPS_LOG_BASE + 20)
#define RTPS_LOG_DB_DELETE_EXTERNAL_BIND_ENTRY(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),RTPS_LOG_DB_DELETE_EXTERNAL_BIND_ENTRY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to send a packet due to a lower module failing to send the
 * packet.
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_SEND_EC                                    (RTPS_LOG_BASE + 21)
#define RTPS_LOG_SEND(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_SEND_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to receive a packet due to a higher module failing to receive
 * the packet.
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_RECEIVE_EC                                 (RTPS_LOG_BASE + 22)
#define RTPS_LOG_RECEIVE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_RECEIVE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to send a packet, when routing to a lower module or peer
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_ROUTE_PACKET_EC                            (RTPS_LOG_BASE + 23)
#define RTPS_LOG_ROUTE_PACKET(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_ROUTE_PACKET_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to receive a packet, when forwarding to a higher upstream module
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_FORWARD_UPSTREAM_EC                        (RTPS_LOG_BASE + 24)
#define RTPS_LOG_FORWARD_UPSTREAM(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_FORWARD_UPSTREAM_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Bad parameter to an RTPS interface function
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_BAD_PARAMETER_EC                           (RTPS_LOG_BASE + 25)
#define RTPS_LOG_BAD_PARAMETER(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_BAD_PARAMETER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed because interface is not enabled
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_NOT_ENABLED_EC                             (RTPS_LOG_BASE + 26)
#define RTPS_LOG_NOT_ENABLED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_NOT_ENABLED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief RTPS reader does not support send
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_READER_UNSUPPORTED_EC                      (RTPS_LOG_BASE + 27)
#define RTPS_LOG_READER_UNSUPPORTED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_READER_UNSUPPORTED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Upstream interface does not match source or destination of packet
 * being sent or received
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_INTERFACE_MISMATCH_EC                      (RTPS_LOG_BASE + 28)
#define RTPS_LOG_INTERFACE_MISMATCH(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_INTERFACE_MISMATCH_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to send due to reaching maximum send window size
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_FULL_SEND_WINDOW_EC                        (RTPS_LOG_BASE + 29)
#define RTPS_LOG_FULL_SEND_WINDOW(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_FULL_SEND_WINDOW_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to set tail of packet to send
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_NETIO_PACKET_SET_TAIL_EC                   (RTPS_LOG_BASE + 31)
#define RTPS_LOG_NETIO_PACKET_SET_TAIL(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_NETIO_PACKET_SET_TAIL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief RTPS interface does not support this function
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_FUNC_UNSUPPORTED_EC                        (RTPS_LOG_BASE + 32)
#define RTPS_LOG_FUNC_UNSUPPORTED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_FUNC_UNSUPPORTED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Out of transport entries.
 * \details Exceeded either
 *        DataWriterQos.writer_resource_limits.max_remote_readers or
 *        DataReaderQos.reader_resource_limits.max_remote_writers
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_EXCEEDED_LIMIT_TRANSPORTS_EC               (RTPS_LOG_BASE + 33)
#define RTPS_LOG_EXCEEDED_LIMIT_TRANSPORTS(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_EXCEEDED_LIMIT_TRANSPORTS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Out of peer entries.
 *
 * \details Exceeded either
 *        DataWriterQos.writer_resource_limits.max_remote_readers or
 *        DataReaderQos.reader_resource_limits.max_remote_writers
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_EXCEEDED_LIMIT_PEERS_EC                    (RTPS_LOG_BASE + 34)
#define RTPS_LOG_EXCEEDED_LIMIT_PEERS(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_EXCEEDED_LIMIT_PEERS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to assert a remote writer or reader
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_ASSERT_PEER_EC                             (RTPS_LOG_BASE + 35)
#define RTPS_LOG_ASSERT_PEER(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_ASSERT_PEER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to assert a downstream transport
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_ASSERT_TRANSPORT_EC                        (RTPS_LOG_BASE + 36)
#define RTPS_LOG_ASSERT_TRANSPORT(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_ASSERT_TRANSPORT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to lookup an external interface
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_FIND_EXTERNAL_INTERFACE_EC                 (RTPS_LOG_BASE + 37)
#define RTPS_LOG_FIND_EXTERNAL_INTERFACE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_FIND_EXTERNAL_INTERFACE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to delete a nonexistent route
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_NONEXISTENT_ROUTE_EC                       (RTPS_LOG_BASE + 38)
#define RTPS_LOG_NONEXISTENT_ROUTE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_NONEXISTENT_ROUTE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to remove a nonexistent bind entry
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_NONEXISTENT_BIND_EC                        (RTPS_LOG_BASE + 39)
#define RTPS_LOG_NONEXISTENT_BIND(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_NONEXISTENT_BIND_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to remove a nonexistent external bind entry
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_NONEXISTENT_EXTERNAL_BIND_EC               (RTPS_LOG_BASE + 40)
#define RTPS_LOG_NONEXISTENT_EXTERNAL_BIND(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_NONEXISTENT_EXTERNAL_BIND_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Dropped an ACKNACK submessage with an old epoch
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_STALE_ACK_EPOCH__EC                        (RTPS_LOG_BASE + 41)
#define RTPS_LOG_STALE_ACK_EPOCH(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_STALE_ACK_EPOCH__EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Dropped a HEARTBEAT submessage with an old epoch
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_STALE_HB_EPOCH_EC                          (RTPS_LOG_BASE + 42)
#define RTPS_LOG_STALE_HB_EPOCH(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_STALE_HB_EPOCH_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed an acknack() upstream
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_ACK_EC                                     (RTPS_LOG_BASE + 43)
#define RTPS_LOG_ACK(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_ACK_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed a request() upstream
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_REQUEST_EC                                 (RTPS_LOG_BASE + 45)
#define RTPS_LOG_REQUEST(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_REQUEST_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed a return_loan() upstream
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_RETURN_LOAN_EC                             (RTPS_LOG_BASE + 46)
#define RTPS_LOG_RETURN_LOAN(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_RETURN_LOAN_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to set the head of a packet
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_NETIO_PACKET_SET_HEAD_EC                   (RTPS_LOG_BASE + 47)
#define RTPS_LOG_NETIO_PACKET_SET_HEAD(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_NETIO_PACKET_SET_HEAD_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to send a HEARTBEAT submessage
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_SEND_HEARTBEAT_EC                          (RTPS_LOG_BASE + 48)
#define RTPS_LOG_SEND_HEARTBEAT(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_SEND_HEARTBEAT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to shift a bitmap
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_SHIFT_BITMAP_EC                            (RTPS_LOG_BASE + 49)
#define RTPS_LOG_SHIFT_BITMAP(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_SHIFT_BITMAP_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief A Reader is fully acknowledged and does not need to respond to a
 * final HEARTBEAT
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_FULLY_ACKED_READER_EC                      (RTPS_LOG_BASE + 50)
#define RTPS_LOG_FULLY_ACKED_READER(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_FULLY_ACKED_READER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Dropping a DATA submessage whose sequence number is outside of
 * a Reader's receive window
 *
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_DATA_OUT_OF_RANGE_EC                       (RTPS_LOG_BASE + 51)
#define RTPS_LOG_DATA_OUT_OF_RANGE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_DATA_OUT_OF_RANGE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Dropping a DATA submessage whose sequence number was previously
 * received
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_DATA_ALREADY_RECEIVED_EC                   (RTPS_LOG_BASE + 52)
#define RTPS_LOG_DATA_ALREADY_RECEIVED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_DATA_ALREADY_RECEIVED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to receive a message because interface is not enabled
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_INTERFACE_NOT_ENABLED_EC                   (RTPS_LOG_BASE + 53)
#define RTPS_LOG_INTERFACE_NOT_ENABLED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_INTERFACE_NOT_ENABLED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Dropped a message with an invalid packet header
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_INVALID_PACKET_EC                          (RTPS_LOG_BASE + 54)
#define RTPS_LOG_INVALID_PACKET(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_INVALID_PACKET_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Received a message on an unsupported interface
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_UNSUPPORTED_INTERFACE_EC                   (RTPS_LOG_BASE + 55)
#define RTPS_LOG_UNSUPPORTED_INTERFACE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_UNSUPPORTED_INTERFACE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Received a submessage with an unknown ID
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_UNKNOWN_SUBMESSAGE_EC                      (RTPS_LOG_BASE + 56)
#define RTPS_LOG_UNKNOWN_SUBMESSAGE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_UNKNOWN_SUBMESSAGE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to process received ACKNACK submessage
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_PROCESS_ACKNACK_EC                         (RTPS_LOG_BASE + 57)
#define RTPS_LOG_PROCESS_ACKNACK(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_PROCESS_ACKNACK_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to process received DATA submessage
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_PROCESS_DATA_EC                            (RTPS_LOG_BASE + 58)
#define RTPS_LOG_PROCESS_DATA(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_PROCESS_DATA_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to process received GAP submessage
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_PROCESS_GAP_EC                             (RTPS_LOG_BASE + 59)
#define RTPS_LOG_PROCESS_GAP(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_PROCESS_GAP_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to process received HEARTBEAT submessage
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_PROCESS_HEARTBEAT_EC                       (RTPS_LOG_BASE + 60)
#define RTPS_LOG_PROCESS_HEARTBEAT(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_PROCESS_HEARTBEAT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create periodic HEARTBEAT event
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_CREATE_HB_EVENT_EC                         (RTPS_LOG_BASE + 61)
#define RTPS_LOG_CREATE_HB_EVENT(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_CREATE_HB_EVENT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create periodic HEARTBEAT event timer
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_CREATE_EVENT_TIMER_EC                      (RTPS_LOG_BASE + 62)
#define RTPS_LOG_CREATE_EVENT_TIMER(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_CREATE_EVENT_TIMER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to set bitmap bit due to out-of-range sequence number
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_SEQ_NUM_OUT_OF_RANGE_EC                    (RTPS_LOG_BASE + 63)
#define RTPS_LOG_SEQ_NUM_OUT_OF_RANGE(level_,dist_,bc_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),RTPS_LOG_SEQ_NUM_OUT_OF_RANGE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "distance",(dist_),"bitcount",(bc_))

/*e
 * \brief Invalid range of sequence numbers to fill in bitmap
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_FIRST_SN_GREATER_LAST_SN_EC                (RTPS_LOG_BASE + 65)
#define RTPS_LOG_FIRST_SN_GREATER_THAN_LAST_SN(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_FIRST_SN_GREATER_LAST_SN_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Deserialized an invalid out-of-bounds bitcount for a bitmap
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_BITCOUNT_OUT_OF_BOUNDS_EC                  (RTPS_LOG_BASE + 66)
#define RTPS_LOG_BITCOUNT_OUT_OF_BOUNDS(level_,bc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),RTPS_LOG_SEQ_NUM_OUT_OF_RANGE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"bitcount",(bc_))

/*e
 * \brief Failed to shift bitmap due to invalid starting sequence number
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_SHIFT_SN_OUT_OF_RANGE_EC                   (RTPS_LOG_BASE + 67)
#define RTPS_LOG_SHIFT_SEQ_NUM_OUT_OF_RANGE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_SHIFT_SN_OUT_OF_RANGE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to serialize host ID of GUID
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_SERIALIZE_HOST_ID_EC                       (RTPS_LOG_BASE + 68)
#define RTPS_LOG_SERIALIZE_HOST_ID(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_SERIALIZE_HOST_ID_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to serialize app ID of GUID
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_SERIALIZE_APP_ID_EC                        (RTPS_LOG_BASE + 69)
#define RTPS_LOG_SERIALIZE_APP_ID(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_SERIALIZE_APP_ID_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to serialized instance ID of GUID
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_SERIALIZE_INSTANCE_ID_EC                   (RTPS_LOG_BASE + 70)
#define RTPS_LOG_SERIALIZE_INSTANCE_ID(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_SERIALIZE_INSTANCE_ID_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to serialize object ID of GUID
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_SERIALIZE_OBJECT_ID_EC                     (RTPS_LOG_BASE + 71)
#define RTPS_LOG_SERIALIZE_OBJECT_ID(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_SERIALIZE_OBJECT_ID_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to deserialize host ID of GUID
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_DESERIALIZE_HOST_ID_EC                     (RTPS_LOG_BASE + 72)
#define RTPS_LOG_DESERIALIZE_HOST_ID(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_DESERIALIZE_HOST_ID_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Faild to deserialize app ID of GUID
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_DESERIALIZE_APP_ID_EC                      (RTPS_LOG_BASE + 73)
#define RTPS_LOG_DESERIALIZE_APP_ID(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_DESERIALIZE_APP_ID_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to deserialize instance ID of GUID
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_DESERIALIZE_INSTANCE_ID_EC                 (RTPS_LOG_BASE + 74)
#define RTPS_LOG_DESERIALIZE_INSTANCE_ID(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_DESERIALIZE_INSTANCE_ID_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to deserialize object ID of GUID
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_DESERIALIZE_OBJECT_ID_EC                   (RTPS_LOG_BASE + 75)
#define RTPS_LOG_DESERIALIZE_OBJECT_ID(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_DESERIALIZE_OBJECT_ID_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create database entry for external bind table.
 *
 * \details If DomainParticipant, may have exceeded
 *        DomainParticipantQos.resource_limits.matching_writer_reader_pair_allocation.
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_DB_CREATE_EXT_BIND_ENTRY_EC                (RTPS_LOG_BASE + 76)
#define RTPS_LOG_DB_CREATE_EXT_BIND_ENTRY(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_DB_CREATE_EXT_BIND_ENTRY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to set bit in bitmap
 *
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_BITMAP_SET_BIT_EC                          (RTPS_LOG_BASE + 77)
#define RTPS_LOG_BITMAP_SET_BIT(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_BITMAP_SET_BIT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Received and dropped currently unsupported INFO_REPLY submessage
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_UNSUPPORTED_INFO_REPLY_EC                  (RTPS_LOG_BASE + 78)
#define RTPS_LOG_UNSUPPORTED_INFO_REPLY(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_UNSUPPORTED_INFO_REPLY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Received and dropped currently unsupported INFO_REPLY_IP4 submessage
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_UNSUPPORTED_INFO_REPLY_IP4_EC              (RTPS_LOG_BASE + 79)
#define RTPS_LOG_UNSUPPORTED_INFO_REPLY_IP4(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_UNSUPPORTED_INFO_REPLY_IP4_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Received and dropped currently unsupported INFO_SRC submessage
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_UNSUPPORTED_INFO_SRC_EC                    (RTPS_LOG_BASE + 80)
#define RTPS_LOG_UNSUPPORTED_INFO_SRC(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_UNSUPPORTED_INFO_SRC_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to delete index of a database table
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_DELETE_INDEX_EC                            (RTPS_LOG_BASE + 81)
#define RTPS_LOG_DELETE_INDEX(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),RTPS_LOG_DELETE_INDEX_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to delete a database table
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_DELETE_TABLE_EC                            (RTPS_LOG_BASE + 82)
#define RTPS_LOG_DELETE_TABLE(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),RTPS_LOG_DELETE_TABLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to take database lock
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_DB_LOCK_EC                                 (RTPS_LOG_BASE + 83)
#define RTPS_LOG_DB_LOCK(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_DB_LOCK_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to give database lock
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_DB_UNLOCK_EC                               (RTPS_LOG_BASE + 84)
#define RTPS_LOG_DB_UNLOCK(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_DB_UNLOCK_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create a database table for storing RTPS bind info
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_CREATE_BIND_TABLE_EC                       (RTPS_LOG_BASE + 85)
#define RTPS_LOG_CREATE_BIND_TABLE(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),RTPS_LOG_CREATE_BIND_TABLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to update status for reliable reader activity changed.
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_STATUS_CHANGE_EC                           (RTPS_LOG_BASE + 86)
#define RTPS_LOG_STATUS_CHANGE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_STATUS_CHANGE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM )

/*e
 * \brief Failed to update status for reliable reader activity changed.
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_SET_GAP_EC                                 (RTPS_LOG_BASE + 87)
#define RTPS_LOG_SET_GAP(level_,start_,end_,sffx_) \
OSAPI_LOG_ENTRY_CREATE((level_),RTPS_LOG_SET_GAP_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_INT("start",(start_),RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_INT("end",(end_),RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_INT("suffix",(sffx_),RTI_TRUE)

/*e
 * \brief Out of memory to allocate send window pool
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_WINDOW_POOL_ALLOC_EC                       (RTPS_LOG_BASE + 88)
#define RTPS_LOG_WINDOW_POOL_ALLOC(level_,size_,count_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),RTPS_LOG_WINDOW_POOL_ALLOC_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "size",(size_),"count",(count_))

/*e
 * \brief Failed to get buffer from send window pool
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_GET_WINDOW_BUFFER_EC                       (RTPS_LOG_BASE + 89)
#define RTPS_LOG_GET_WINDOW_BUFFER(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_GET_WINDOW_BUFFER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Cannot insert sample into full window
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_INSERT_WINDOW_FULL_EC                      (RTPS_LOG_BASE + 90)
#define RTPS_LOG_INSERT_WINDOW_FULL(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_INSERT_WINDOW_FULL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to insert sample into send window
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_WINDOW_INSERT_EC                           (RTPS_LOG_BASE + 91)
#define RTPS_LOG_WINDOW_INSERT(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_WINDOW_INSERT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to delete a timeout event
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_TIMER_DELETE_TIMEOUT_EC                    (RTPS_LOG_BASE + 92)
#define RTPS_LOG_TIMER_DELETE_TIMEOUT(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_TIMER_DELETE_TIMEOUT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to delete a buffer pool
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_BUFFERPOOL_DELETE_EC                       (RTPS_LOG_BASE + 93)
#define RTPS_LOG_BUFFERPOOL_DELETE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_BUFFERPOOL_DELETE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to send a message to a specific peer
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_DIRECT_SEND_EC                             (RTPS_LOG_BASE + 94)
#define RTPS_LOG_DIRECT_SEND(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_DIRECT_SEND_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief RTPS writer failed to initialize a packet
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_PACKET_INITIALIZE_EC                       (RTPS_LOG_BASE + 95)
#define RTPS_LOG_PACKET_INITIALIZE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_PACKET_INITIALIZE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief RTPS reliable writer failed to request and resend history
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_WRITER_REQUEST_SENT_HISTORY_EC             (RTPS_LOG_BASE + 96)
#define RTPS_LOG_WRITER_REQUEST_SEND_HISTORY(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_WRITER_REQUEST_SENT_HISTORY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to shift bitmap to new lead sequence number
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_BITMAP_SHIFT_EC                            (RTPS_LOG_BASE + 97)
#define RTPS_LOG_BITMAP_SHIFT(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_BITMAP_SHIFT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Send window of a remote reader failed to advance
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_WINDOW_ADVANCE_EC                          (RTPS_LOG_BASE + 98)
#define RTPS_LOG_WINDOW_ADVANCE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_WINDOW_ADVANCE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed when advancing window ahead of unacknowledged sequence number
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_WINDOW_ADVANCE_UNACKED_EC                  (RTPS_LOG_BASE + 99)
#define RTPS_LOG_WINDOW_ADVANCE_UNACKED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_WINDOW_ADVANCE_UNACKED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to find peer that should exist
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_LOOKUP_PEER_EC                            (RTPS_LOG_BASE + 100)
#define RTPS_LOG_LOOKUP_PEER(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_LOOKUP_PEER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief RTPS reader failed to send an ACKNACK message
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_READER_SEND_ACKNACK_EC                    (RTPS_LOG_BASE + 101)
#define RTPS_LOG_READER_SEND_ACKNACK(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_READER_SEND_ACKNACK_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to finalize an RTPS interface
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_FINALIZE_INTERFACE_EC                     (RTPS_LOG_BASE + 102)
#define RTPS_LOG_FINALIZE_INTERFACE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_FINALIZE_INTERFACE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to remove index entry for external interface upon deleting
 *        RTPS interface
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_INDEXER_REMOVE_ENTRY_EC                   (RTPS_LOG_BASE + 103)
#define RTPS_LOG_INDEXER_REMOVE_ENTRY(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_INDEXER_REMOVE_ENTRY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed when looking up external bind entry from database
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_LOOKUP_EXT_BIND_ENTRY_EC                  (RTPS_LOG_BASE + 104)
#define RTPS_LOG_LOOKUP_EXT_BIND_ENTRY(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),RTPS_LOG_LOOKUP_EXT_BIND_ENTRY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to initialize a local sequence
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_SEQ_INIT_EC                               (RTPS_LOG_BASE + 105)
#define RTPS_LOG_SEQ_INIT(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_SEQ_INIT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to set maximum of a local sequence
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_SEQ_MAX_EC                                (RTPS_LOG_BASE + 106)
#define RTPS_LOG_SEQ_MAX(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_SEQ_MAX_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to set length of a local sequence
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_SEQ_LEN_EC                                (RTPS_LOG_BASE + 107)
#define RTPS_LOG_SEQ_LEN(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_SEQ_LEN_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Initializing an RTPS interface with undefined mode
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_INTF_MODE_UNDEF_EC                        (RTPS_LOG_BASE + 108)
#define RTPS_LOG_INTF_MODE_UNDEF(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_INTF_MODE_UNDEF_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to delete buffer pool
 * \details May still have buffers in use, thus cannot delete pool.
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_DELETE_BUFFER_POOL_EC                     (RTPS_LOG_BASE + 109)
#define RTPS_LOG_DELETE_BUFFER_POOL(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_DELETE_BUFFER_POOL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create buffer pool
 * \details May have insufficient memory to allocate pool.
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_CREATE_BUFFER_POOL_EC                     (RTPS_LOG_BASE + 110)
#define RTPS_LOG_CREATE_BUFFER_POOL(level_,size_,count_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),RTPS_LOG_CREATE_BUFFER_POOL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "size",(RTI_INT32)(size_),"count",(RTI_INT32)(count_))

/*e
 * \brief Failed to create index of peers
 * \details System may have insufficient memory to allocate new index.
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_CREATE_PEERS_INDEX_EC                     (RTPS_LOG_BASE + 111)
#define RTPS_LOG_CREATE_PEERS_INDEX(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_CREATE_PEERS_INDEX_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)


/*e
 * \brief Failed to delete indexer
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_DELETE_INDEXER_EC                         (RTPS_LOG_BASE + 112)
#define RTPS_LOG_DELETE_INDEXER(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_DELETE_INDEXER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to get buffer from peer buffer pool
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_GET_PEER_BUFFER_EC                        (RTPS_LOG_BASE + 113)
#define RTPS_LOG_GET_PEER_BUFFER(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_GET_PEER_BUFFER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create database index for direct sends
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_DB_CREATE_DIRECT_INDEX_EC                 (RTPS_LOG_BASE + 114)
#define RTPS_LOG_DB_CREATE_DIRECT_INDEX(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),RTPS_LOG_DB_CREATE_DIRECT_INDEX_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to create database index for group sends
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_DB_CREATE_GROUP_INDEX_EC                  (RTPS_LOG_BASE + 115)
#define RTPS_LOG_DB_CREATE_GROUP_INDEX(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),RTPS_LOG_DB_CREATE_GROUP_INDEX_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Insufficient buffer in packet
 * \details Receive buffer does not have enough space for a valid RTPS message
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_NETIO_PACKET_INSUFFICIENT_BUFFER_EC       (RTPS_LOG_BASE + 116)
#define RTPS_LOG_NETIO_PACKET_INSUFFICIENT_BUFFER(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_NETIO_PACKET_INSUFFICIENT_BUFFER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to process received DATA BATCH submessage
 * \details This indicates an invalid DATA_BATCH submessage
 *
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_PROCESS_DATA_BATCH_EC                     (RTPS_LOG_BASE + 136)
#define RTPS_LOG_PROCESS_DATA_BATCH(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_PROCESS_DATA_BATCH_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to process received HEARTBEAT_BATCH submessage
 * \details This indicates an invalid HEARTBEAT_BATCH submessage
 *
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_PROCESS_HEARTBEAT_BATCH_EC                (RTPS_LOG_BASE + 137)
#define RTPS_LOG_PROCESS_HEARTBEAT_BATCH(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_PROCESS_HEARTBEAT_BATCH_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief The reservation count it zero
 * \details The reservation count should never be less than 0, this is an
 *          internal error.
 *
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_NEGATIVE_RESERVATION_COUNT_EC             (RTPS_LOG_BASE + 138)
#define RTPS_LOG_NEGATIVE_RESERVATION_COUNT(level_,count_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),RTPS_LOG_NEGATIVE_RESERVATION_COUNT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"count",count_)

/*e
 * \brief Failed to lookup a remote participant's interceptor handle
 * \details An interceptor handle must be always supplied for a remote
 *  participant, if any encoding and/or decoding are to be performed.
 *  This is an internal error.
 *
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_TRUST_REMOTE_PARTICIPANT_HANDLE_LOOKUP_FAILED_EC   (RTPS_LOG_BASE + 139)
#define RTPS_LOG_TRUST_REMOTE_PARTICIPANT_HANDLE_LOOKUP_FAILED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),\
        RTPS_LOG_TRUST_REMOTE_PARTICIPANT_HANDLE_LOOKUP_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create a record to store a remote participant's
 *  interceptor handle
 * \details Interceptor handles for remote participants are stored in an
 *  indexed tabled for quicker access. This is an internal error.
 *
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_TRUST_REMOTE_PARTICIPANT_HANDLE_CREATE_FAILED_EC   (RTPS_LOG_BASE + 140)
#define RTPS_LOG_TRUST_REMOTE_PARTICIPANT_HANDLE_CREATE_FAILED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),\
        RTPS_LOG_TRUST_REMOTE_PARTICIPANT_HANDLE_CREATE_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to insert a record in the table storing interceptor handles
 * of matched remote participants.
 *
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_TRUST_REMOTE_PARTICIPANT_HANDLE_INSERT_FAILED_EC   (RTPS_LOG_BASE + 141)
#define RTPS_LOG_TRUST_REMOTE_PARTICIPANT_HANDLE_INSERT_FAILED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),\
        RTPS_LOG_TRUST_REMOTE_PARTICIPANT_HANDLE_INSERT_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Interceptor handle list out of resources
 *
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_TRUST_INTERCEPTOR_HANDLE_LIST_FULL_EC              (RTPS_LOG_BASE + 142)
#define RTPS_LOG_TRUST_INTERCEPTOR_HANDLE_LIST_FULL(level_) \
OSAPI_LOG_ENTRY_ADD((level_),\
        RTPS_LOG_TRUST_INTERCEPTOR_HANDLE_LIST_FULL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)


/*e
 * \brief Inconsistent handles found for remote peer
 *
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_TRUST_INCONSISTENT_INTERCEPTOR_HANDLE_EC            (RTPS_LOG_BASE + 143)
#define RTPS_LOG_TRUST_INCONSISTENT_INTERCEPTOR_HANDLE(level_,type_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),\
        RTPS_LOG_TRUST_INCONSISTENT_INTERCEPTOR_HANDLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
        "type",(type_))

/*e
 * \brief Failed to bind resources for trust behavior
 *
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_TRUST_BIND_FAILED_EC                                (RTPS_LOG_BASE + 144)
#define RTPS_LOG_TRUST_BIND_FAILED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),\
        RTPS_LOG_TRUST_BIND_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to bind resources for trust behavior
 *
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_TRUST_SUBMSG_CONVERSION_FAILED_EC         (RTPS_LOG_BASE + 145)
#define RTPS_LOG_TRUST_SUBMSG_CONVERSION_FAILED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),\
        RTPS_LOG_TRUST_SUBMSG_CONVERSION_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)


/*e
 * \brief A malformed trust message was received
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_TRUST_INVALID_SUBMSG_EC                      (RTPS_LOG_BASE + 146)
#define RTPS_LOG_TRUST_INVALID_SUBMSG(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),RTPS_LOG_TRUST_INVALID_SUBMSG_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"kind",kind_)

/*e
 * \brief The RTPS reader was in an unexpected state
 *
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_TRUST_UNEXPECTED_READER_STATE_EC         (RTPS_LOG_BASE + 147)
#define RTPS_LOG_TRUST_UNEXPECTED_READER_STATE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_TRUST_UNEXPECTED_READER_STATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)


/*e
 * \brief Failed to create database index for the selected route group
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_DB_CREATE_SELECTED_GROUP_INDEX_EC         (RTPS_LOG_BASE + 148)
#define RTPS_LOG_DB_CREATE_SELECTED_GROUP_INDEX(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),RTPS_LOG_DB_CREATE_SELECTED_GROUP_INDEX_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))


/*e
 * \brief Failed to take a lock
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_LOCK_EC                                 (RTPS_LOG_BASE + 149)
#define RTPS_LOG_LOCK(level_, lock_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),RTPS_LOG_DB_LOCK_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM, "lock", lock_)

/*e
 * \brief Failed to give a lock
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_UNLOCK_EC                               (RTPS_LOG_BASE + 150)
#define RTPS_LOG_UNLOCK(level_, lock_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),RTPS_LOG_DB_UNLOCK_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM, "lock", lock_)


/*e
 * \brief Failed to schedule a packet
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_SCHEDULE_PACKET_EC                               (RTPS_LOG_BASE + 151)
#define RTPS_LOG_SCHEDULE_PACKET(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_SCHEDULE_PACKET_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to finalize a sequence
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_SEQ_FINALIZE_EC                           (RTPS_LOG_BASE + 152)
#define RTPS_LOG_SEQ_FINALIZE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_SEQ_FINALIZE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to transform rtps message
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_TRANSFORM_RTPS_EC                         (RTPS_LOG_BASE + 153)
#define RTPS_LOG_TRANSFORM_RTPS(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_TRANSFORM_RTPS_EC, \
    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to restore trust loan buffer
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_RESTORE_TRUST_LOAN_BUFFER_EC              (RTPS_LOG_BASE + 154)
#define RTPS_LOG_RESTORE_TRUST_LOAN_BUFFER(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_RESTORE_TRUST_LOAN_BUFFER_EC, \
    OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create reassembly table
 *
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_CREATE_REASSEMBLY_TABLE_EC                (RTPS_LOG_BASE + 155)
#define RTPS_LOG_CREATE_REASSEMBLY_TABLE(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),RTPS_LOG_CREATE_REASSEMBLY_TABLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"dbrc",(dbrc_))

/*******************************************************************************
 *                                 Checksum related errors
 ******************************************************************************/

/*e
 * \brief The property tries to override the BUILTIN128 checksum without setting
 *        allow_builtin_override to TRUE.
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_CHECKSUM_ILLEGAL_OVERRIDE_CHECKSUM128_EC  (RTPS_LOG_BASE + 200)
#define RTPS_LOG_CHECKSUM_ILLEGAL_OVERRIDE_CHECKSUM128(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_CHECKSUM_ILLEGAL_OVERRIDE_CHECKSUM128_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief The property tries to override the BUILTIN32 checksum without setting
 *        allow_builtin_override to TRUE.
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_CHECKSUM_ILLEGAL_OVERRIDE_CHECKSUM32_EC   (RTPS_LOG_BASE + 201)
#define RTPS_LOG_CHECKSUM_ILLEGAL_OVERRIDE_CHECKSUM32(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_CHECKSUM_ILLEGAL_OVERRIDE_CHECKSUM32_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief The property tries to override the BUILTIN64 checksum without setting
 *        allow_builtin_override to TRUE.
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_CHECKSUM_ILLEGAL_OVERRIDE_CHECKSUM64_EC   (RTPS_LOG_BASE + 202)
#define RTPS_LOG_CHECKSUM_ILLEGAL_OVERRIDE_CHECKSUM64(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_CHECKSUM_ILLEGAL_OVERRIDE_CHECKSUM64_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief The property override of the BUILTIN128 checksum is invalid. It is not legal
 *        to reuse any of the built-in functions.
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_CHECKSUM_INVALID_OVERRIDE_BUILTIN128_EC   (RTPS_LOG_BASE + 203)
#define RTPS_LOG_CHECKSUM_INVALID_OVERRIDE_BUILTIN128(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_CHECKSUM_INVALID_OVERRIDE_BUILTIN128_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief The property override of the BUILTIN32 checksum is invalid. It is not legal
 *        to reuse any of the built-in functions functions.
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_CHECKSUM_INVALID_OVERRIDE_BUILTIN32_EC    (RTPS_LOG_BASE + 204)
#define RTPS_LOG_CHECKSUM_INVALID_OVERRIDE_BUILTIN32(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_CHECKSUM_INVALID_OVERRIDE_BUILTIN32_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)
/*e
 * \brief The property override of the BUILTIN64 checksum is invalid. It is not legal
 *        to reuse any of the built-in functions functions.
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_CHECKSUM_INVALID_OVERRIDE_BUILTIN64_EC    (RTPS_LOG_BASE + 205)
#define RTPS_LOG_CHECKSUM_INVALID_OVERRIDE_BUILTIN64(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_CHECKSUM_INVALID_OVERRIDE_BUILTIN64_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief RTPS_Interface_calculate_checksum() failed to calculate the checksum
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_CHECKSUM_CHECKSUM_CALCULATION_FAILED_EC   (RTPS_LOG_BASE + 210)
#define RTPS_LOG_CHECKSUM_CHECKSUM_CALCULATION_FAILED(level_,cindex_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),RTPS_LOG_CHECKSUM_CHECKSUM_CALCULATION_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"cindex",(cindex_))

/*e
 * \brief The HeaderExtension contained a mandatory PID that was not understood
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_UNSUPPORTED_MANDTORY_HDR_EXT_PID_EC       (RTPS_LOG_BASE + 211)
#define RTPS_LOG_UNSUPPORTED_MANDTORY_HDR_EXT_PID(level_,pid_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),RTPS_LOG_UNSUPPORTED_MANDTORY_HDR_EXT_PID_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"pid_",(RTI_INT32)(pid_))

/*e
 * \brief An invalid HeaderExtension PID was detected
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_INVALID_HDR_EXT_PID_LIST_EC               (RTPS_LOG_BASE + 212)
#define RTPS_LOG_INVALID_HDR_EXT_PID_LIST(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_INVALID_HDR_EXT_PID_LIST_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to decode the HeaderExtension PID list
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_INVALID_HDR_EXT_PID_ERROR_EC              (RTPS_LOG_BASE + 213)
#define RTPS_LOG_INVALID_HDR_EXT_PID_ERROR(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_INVALID_HDR_EXT_PID_ERROR_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief The HeaderExtension length is isconsistent with the decoded
 *        encoded HeaderExtension length.
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_INCONSISTENT_HDR_EXT_LENGTH_ERROR_EC      (RTPS_LOG_BASE + 214)
#define RTPS_LOG_INCONSISTENT_HDR_EXT_LENGTH_ERROR(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_INCONSISTENT_HDR_EXT_LENGTH_ERROR_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief A RTPS message which was longer than the indicated message length
 *        in the RTPS header was received. This causes the rest of the RTPS
 *        message to be invalidated and the remaining submessages to be
 *        ignored.
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_MESSAGE_EXCEED_LENGTH_ERROR_EC            (RTPS_LOG_BASE + 215)
#define RTPS_LOG_MESSAGE_EXCEED_LENGTH_ERROR(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_MESSAGE_EXCEED_LENGTH_ERROR_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief The CRC32 submsg flags are invalid
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_CHECKSUM_INVALID_CRC32_FLAGS_EC           (RTPS_LOG_BASE + 216)
#define RTPS_LOG_CHECKSUM_INVALID_CRC32_FLAGS(level_,flags_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),RTPS_LOG_CHECKSUM_INVALID_CRC32_FLAGS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"flags",(RTI_INT32)(flags_))

/*e
 * \brief A header extension was received, but the checksum length is not present
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_CHECKSUM_MISSING_CHECKSUM_LENGTH_EC       (RTPS_LOG_BASE + 217)
#define RTPS_LOG_CHECKSUM_MISSING_CHECKSUM_LENGTH(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_CHECKSUM_MISSING_CHECKSUM_LENGTH_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Unsupported checksum bits received, message dropped
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_CHECKSUM_UNSUPPORTED_EC                   (RTPS_LOG_BASE + 218)
#define RTPS_LOG_CHECKSUM_UNSUPPORTED(level_,cbits_,allowed_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),RTPS_LOG_CHECKSUM_UNSUPPORTED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "cbits",(RTI_INT32)(cbits_),"allowed_",(RTI_INT32)(allowed_))

/*e
 * \brief Checksum length could not be deserialized
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_CHECKSUM_LENGTH_DESERIALIZE_ERROR_EC      (RTPS_LOG_BASE + 219)
#define RTPS_LOG_CHECKSUM_LENGTH_DESERIALIZE_ERROR(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_CHECKSUM_LENGTH_DESERIALIZE_ERROR_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief The payload length is less than the deserialized checksum length
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_CHECKSUM_INCONSISTENT_LENGTH_EC           (RTPS_LOG_BASE + 220)
#define RTPS_LOG_CHECKSUM_INCONSISTENT_LENGTH(level_,payload_,checksum_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),RTPS_LOG_CHECKSUM_INCONSISTENT_LENGTH_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "payload",(RTI_INT32)(payload_),"checksum",(RTI_INT32)(checksum_))

/*e
 * \brief Checksum length could not be deserialized
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_CHECKSUM_CHECKSUM_DESERIALIZE_ERROR_EC    (RTPS_LOG_BASE + 221)
#define RTPS_LOG_CHECKSUM_CHECKSUM_DESERIALIZE_ERROR(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_CHECKSUM_CHECKSUM_DESERIALIZE_ERROR_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief The checksum is invalid
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_CHECKSUM_CHECKSUM_ERROR_EC                (RTPS_LOG_BASE + 222)
#define RTPS_LOG_CHECKSUM_CHECKSUM_ERROR(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_CHECKSUM_CHECKSUM_ERROR_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Found too many scatter-gather buffers when calculating checksum
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_CHECKSUM_TOO_MANY_SG_BUFFERS_EC           (RTPS_LOG_BASE + 223)
#define RTPS_LOG_CHECKSUM_TOO_MANY_SG_BUFFERS(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_CHECKSUM_TOO_MANY_SG_BUFFERS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failure to send Liveliness packets.
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_SEND_LIVELINESS_EC           (RTPS_LOG_BASE + 224)
#define RTPS_LOG_SEND_LIVELINESS(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_SEND_LIVELINESS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failure to delete a record.
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_DELETE_RECORD_EC        (RTPS_LOG_BASE + 225)
#define RTPS_LOG_DELETE_RECORD(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_DELETE_RECORD_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)



/*e
 * \brief Failed to apply a content filter to a message
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_APPLY_CONTENT_FILTER_EC                   (RTPS_LOG_BASE + 300)
#define RTPS_LOG_APPLY_CONTENT_FILTER(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_APPLY_CONTENT_FILTER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to update the state of reliable peers while applying filtering
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_UPDATE_RELIABLE_PEERS_FILTER_EC           (RTPS_LOG_BASE + 301)
#define RTPS_LOG_UPDATE_RELIABLE_PEERS_FILTER(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_UPDATE_RELIABLE_PEERS_FILTER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to add a filtered route
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_ADD_FILTERED_ROUTE_EC                     (RTPS_LOG_BASE + 302)
#define RTPS_LOG_ADD_FILTERED_ROUTE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_ADD_FILTERED_ROUTE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to delete a filtered route
 * \ingroup RTPSLogCodesClass
 */
#define RTPS_LOG_DELETE_FILTERED_ROUTE_EC                  (RTPS_LOG_BASE + 303)
#define RTPS_LOG_DELETE_FILTERED_ROUTE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),RTPS_LOG_DELETE_FILTERED_ROUTE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

#endif /* rtps_log_h */
