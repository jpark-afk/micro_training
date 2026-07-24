/*
 * FILE: netio_shmem_log.h
 *
 * Copyright (c) 2018-2025 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*e
 * \file
 * \brief NETIO Zero Copy Shared Data module log codes
 */
#ifndef netio_shmem_log_h
#define netio_shmem_log_h

#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif

/*e
 * \brief Failed to delete a database table for a NETIO shared memory interface
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_DELETE_TABLE_EC                     (SHMEM_LOG_BASE + 1)
#define NETIO_SHMEM_LOG_DELETE_TABLE(level_,dbrc_) \
    OSAPI_LOG_ENTRY_ADD_1INT(                      \
        (level_),                                  \
        NETIO_SHMEM_LOG_DELETE_TABLE_EC,           \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,             \
        "dbrc",                                    \
        (dbrc_))

/*e
 * \brief Failed to create a database table for a NETIO shared memory interface
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_CREATE_TABLE_EC                    (SHMEM_LOG_BASE +  2)
#define NETIO_SHMEM_LOG_CREATE_TABLE(level_,dbrc_) \
    OSAPI_LOG_ENTRY_ADD_1INT(                      \
        (level_),                                  \
        NETIO_SHMEM_LOG_CREATE_TABLE_EC,           \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,             \
        "dbrc",                                    \
        (dbrc_))

/*e
 * \brief Failed to select a valid record when sending with the NETIO
 *        shared memory
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_SELECT_TABLE_EC                     (SHMEM_LOG_BASE + 3)
#define NETIO_SHMEM_LOG_SELECT_TABLE(level_,dbrc_) \
    OSAPI_LOG_ENTRY_ADD_1INT(                      \
        (level_),                                  \
        NETIO_SHMEM_LOG_SELECT_TABLE_EC,           \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,             \
        "dbrc",                                    \
        (dbrc_))

/*e
 * \brief Failed to send forward with the NETIO shared memory interface
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_FWD_EC                              (SHMEM_LOG_BASE + 4)
#define NETIO_SHMEM_LOG_FWD(level_) \
    OSAPI_LOG_ENTRY_ADD(            \
        (level_),                   \
        NETIO_SHMEM_LOG_FWD_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to bind an external interface with the NETIO
 *        shared memory interface
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_BINDX_DB_EC                         (SHMEM_LOG_BASE + 5)
#define NETIO_SHMEM_LOG_BINDX_DB(level_,dbrc_) \
    OSAPI_LOG_ENTRY_ADD_1INT(                  \
        (level_),                              \
        NETIO_SHMEM_LOG_BINDX_DB_EC,           \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,         \
        "dbrc",                                \
        (dbrc_))

/*e
 * \brief Failed to unbind an external interface with the NETIO shared memory
 *        interface
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_UNBINDX_DB_EC                       (SHMEM_LOG_BASE + 6)
#define NETIO_SHMEM_LOG_UNBINDX_DB(level_,dbrc_) \
    OSAPI_LOG_ENTRY_ADD_1INT(                    \
        (level_),                                \
        NETIO_SHMEM_LOG_UNBINDX_DB_EC,           \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,           \
        "dbrc",                                  \
        (dbrc_))

/*e
 * \brief Failed to set the length of an address sequence
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_SET_LENGTH_EC                       (SHMEM_LOG_BASE + 7)
#define NETIO_SHMEM_LOG_SET_LENGTH(level_,len_) \
    OSAPI_LOG_ENTRY_ADD_1INT(                   \
        (level_),                               \
        NETIO_SHMEM_LOG_SET_LENGTH_EC,          \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,          \
        "len",                                  \
        (len_))

/*e
 * \brief Invalid property passed in when creating a shared memory interface
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_INVALID_PROPERTY_EC                 (SHMEM_LOG_BASE + 8)
#define NETIO_SHMEM_LOG_INVALID_PROPERTY(level_) \
    OSAPI_LOG_ENTRY_ADD(                         \
        (level_),                                \
        NETIO_SHMEM_LOG_INVALID_PROPERTY_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to initialize the shared memory interface
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_INITIALIZE_FAILED_EC                (SHMEM_LOG_BASE + 9)
#define NETIO_SHMEM_LOG_INITIALIZE_FAILED(level_) \
    OSAPI_LOG_ENTRY_ADD(                          \
        (level_),                                 \
        NETIO_SHMEM_LOG_INITIALIZE_FAILED_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief An error occurred with a cursor while iterating over a table
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_CURSOR_ERROR_EC                    (SHMEM_LOG_BASE + 10)
#define NETIO_SHMEM_LOG_CURSOR_ERROR(level_) \
    OSAPI_LOG_ENTRY_ADD(                     \
        (level_),                            \
        NETIO_SHMEM_LOG_CURSOR_ERROR_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief An invalid factory was passed in to create a shared memory interface
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_INVALID_FACTORY_EC                 (SHMEM_LOG_BASE + 11)
#define NETIO_SHMEM_LOG_INVALID_FACTORY(level_) \
    OSAPI_LOG_ENTRY_ADD(                        \
        (level_),                               \
        NETIO_SHMEM_LOG_INVALID_FACTORY_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief An invalid component was passed in to delete a shared memory interface
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_INVALID_COMPONENT_EC               (SHMEM_LOG_BASE + 12)
#define NETIO_SHMEM_LOG_INVALID_COMPONENT(level_) \
    OSAPI_LOG_ENTRY_ADD(                          \
        (level_),                                 \
        NETIO_SHMEM_LOG_INVALID_COMPONENT_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Found incompatible cookie in shared memory header
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_INCOMPATIBLE_COOKIE_EC             (SHMEM_LOG_BASE + 13)
#define NETIO_SHMEM_LOG_INCOMPATIBLE_COOKIE(level_,value_) \
    OSAPI_LOG_ENTRY_ADD_1INT(                              \
        (level_),                                          \
        NETIO_SHMEM_LOG_INCOMPATIBLE_COOKIE_EC,            \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,                     \
        "value",                                           \
        (value_))

/*e
 * \brief Found incompatible major version in shared memory header
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_INCOMPATIBLE_VERSION_EC            (SHMEM_LOG_BASE + 14)
#define NETIO_SHMEM_LOG_INCOMPATIBLE_VERSION(level_,value_) \
    OSAPI_LOG_ENTRY_ADD_1INT(                               \
        (level_),                                           \
        NETIO_SHMEM_LOG_INCOMPATIBLE_VERSION_EC,            \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,                      \
        "value",                                            \
        (value_))

/*e
 * \brief Found incompatible header size in shared memory header
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_INCOMPATIBLE_HEADER_SIZE_EC        (SHMEM_LOG_BASE + 15)
#define NETIO_SHMEM_LOG_INCOMPATIBLE_HEADER_SIZE(level_,value_) \
    OSAPI_LOG_ENTRY_ADD_1INT(                                   \
        (level_),                                               \
        NETIO_SHMEM_LOG_INCOMPATIBLE_COOKIE_EC,                 \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,                          \
        "value",                                                \
        (value_))

/*e
 * \brief Attempted to attach to shared memory header transport concurrent
 *        queue that is incompatible
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_INCOMPATIBLE_HEADER_EC             (SHMEM_LOG_BASE + 16)
#define NETIO_SHMEM_LOG_INCOMPATIBLE_HEADER(level_) \
    OSAPI_LOG_ENTRY_ADD(                            \
        (level_),                                   \
        NETIO_SHMEM_LOG_INCOMPATIBLE_HEADER_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create or attach to a shared memory segment
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_ADDRESS_IN_USE_EC                  (SHMEM_LOG_BASE + 17)
#define NETIO_SHMEM_LOG_ADDRESS_IN_USE(level_, port_) \
    OSAPI_LOG_ENTRY_ADD_1INT(                         \
        (level_),                                     \
        NETIO_SHMEM_LOG_ADDRESS_IN_USE_EC,            \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,                \
        "port",                                       \
        (port_))

/*e
 * \brief Possible failure due to misconfigured system settings on Darwin
 *        operating systems
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_CHECK_SYSTEM_SETTINGS_EC           (SHMEM_LOG_BASE + 18)
#define NETIO_SHMEM_LOG_CHECK_SYSTEM_SETTINGS(level_) \
    OSAPI_LOG_ENTRY_ADD(                              \
        (level_),                                     \
        NETIO_SHMEM_LOG_CHECK_SYSTEM_SETTINGS_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 *
 * \brief Failed to lock shared memory mutex protecting the shared memory
 *        segment
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_SHMEM_MUTEX_LOCK_FAILED_EC         (SHMEM_LOG_BASE + 19)
#define NETIO_SHMEM_LOG_SHMEM_MUTEX_LOCK_FAILED(level_, ec_) \
    OSAPI_LOG_ENTRY_ADD_1INT(                                \
        (level_),                                            \
        NETIO_SHMEM_LOG_SHMEM_MUTEX_LOCK_FAILED_EC,          \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,                       \
        "ec",                                                \
        (ec_))

/*e
 * \brief  Failed to unlock shared memory mutex protecting the shared memory
 *         segment
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_SHMEM_MUTEX_UNLOCK_FAILED_EC       (SHMEM_LOG_BASE + 20)
#define NETIO_SHMEM_LOG_SHMEM_MUTEX_UNLOCK_FAILED(level_, ec_) \
    OSAPI_LOG_ENTRY_ADD_1INT(                                  \
        (level_),                                              \
        NETIO_SHMEM_LOG_SHMEM_MUTEX_UNLOCK_FAILED_EC,          \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,                         \
        "ec",                                                  \
        (ec_))

/*e
 * \brief Failed to allocate memory needed by shared memory transport
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_FAILED_TO_ALLOCATE_STRUCT_EC       (SHMEM_LOG_BASE + 21)
#define NETIO_SHMEM_LOG_FAILED_TO_ALLOCATE_STRUCT(level_) \
    OSAPI_LOG_ENTRY_ADD(                                  \
        (level_),                                         \
        NETIO_SHMEM_LOG_FAILED_TO_ALLOCATE_STRUCT_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to initialize shared memory transport
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_FAILED_TO_INITIALIZE_EC            (SHMEM_LOG_BASE + 22)
#define NETIO_SHMEM_LOG_FAILED_TO_INITIALIZE(level_) \
    OSAPI_LOG_ENTRY_ADD(                             \
        (level_),                                    \
        NETIO_SHMEM_LOG_FAILED_TO_INITIALIZE_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create or attach. Possible shared memory key conflict
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_CREATE_ATTACH_INFINITE_EC          (SHMEM_LOG_BASE + 23)
#define NETIO_SHMEM_LOG_CREATE_ATTACH_INFINITE(level_) \
    OSAPI_LOG_ENTRY_ADD(                               \
        (level_),                                      \
        NETIO_SHMEM_LOG_CREATE_ATTACH_INFINITE_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 *
 * \brief Failed to create a new shared memory mutex or failed to attach
 *        to an existing one
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_FAILED_TO_INIT_MUTEX_EC            (SHMEM_LOG_BASE + 24)
#define NETIO_SHMEM_LOG_FAILED_TO_INIT_MUTEX(level_,key_) \
    OSAPI_LOG_ENTRY_ADD_1INT(                             \
        (level_),                                         \
        NETIO_SHMEM_LOG_FAILED_TO_INIT_MUTEX_EC,          \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,                    \
        "key",                                            \
        (key_))

/*e
 *
 * \brief Failed to create signaling semaphore. A Domain Participant's shared
 *        memory transport blocks on a signaling semaphore when waiting for data
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_FAILED_TO_INIT_SIG_SEM_EC          (SHMEM_LOG_BASE + 25)
#define NETIO_SHMEM_LOG_FAILED_TO_INIT_SIG_SEM(level_,key_) \
    OSAPI_LOG_ENTRY_ADD_1INT(                               \
        (level_),                                           \
        NETIO_SHMEM_LOG_FAILED_TO_INIT_SIG_SEM_EC,          \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,                      \
        "key",                                              \
        (key_))

#define NETIO_SHMEM_LOG_FAILED_TO_ATTACH_SIG_SEM_EC        (SHMEM_LOG_BASE + 26)
#define NETIO_SHMEM_LOG_FAILED_TO_ATTACH_SIG_SEM(level_,key_) \
    OSAPI_LOG_ENTRY_ADD_1INT(                                 \
        (level_),                                             \
        NETIO_SHMEM_LOG_FAILED_TO_ATTACH_SIG_SEM_EC,          \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,                        \
        "key",                                                \
        (key_))

/*e
 *
 * \brief Failed to take a shared memory signaling semaphore
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_FAILED_TAKE_SIGN_SEM_EC            (SHMEM_LOG_BASE + 27)
#define NETIO_SHMEM_LOG_FAILED_TAKE_SIGN_SEM(level_) \
    OSAPI_LOG_ENTRY_ADD(                             \
        (level_),                                    \
        NETIO_SHMEM_LOG_FAILED_TAKE_SIGN_SEM_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 *
 * \brief Failed to give a shared memory signaling semaphore
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_SIG_SEM_SIGNAL_FAILED_EC           (SHMEM_LOG_BASE + 28)
#define NETIO_SHMEM_LOG_SIG_SEM_SIGNAL_FAILED(level_) \
    OSAPI_LOG_ENTRY_ADD(                              \
        (level_),                                     \
        NETIO_SHMEM_LOG_SIG_SEM_SIGNAL_FAILED_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 *
 * \brief Failed to attach to a shared memory segment because it doesn't exist
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_FAILED_TO_INIT_SEGMENT_ADDR_EC     (SHMEM_LOG_BASE + 29)
#define NETIO_SHMEM_LOG_FAILED_TO_INIT_SEGMENT_ADDR(level_) \
    OSAPI_LOG_ENTRY_ADD(                                    \
        (level_),                                           \
        NETIO_SHMEM_LOG_FAILED_TO_INIT_SEGMENT_ADDR_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to initialize a receive packet
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_PACKET_INIT_EC                     (SHMEM_LOG_BASE + 30)
#define NETIO_SHMEM_LOG_PACKET_INIT(level_,packet_,buf_,len_) \
    OSAPI_LOG_ENTRY_CREATE(                                   \
        (level_),                                             \
        NETIO_SHMEM_LOG_PACKET_INIT_EC,                       \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,                        \
        RTI_FALSE)                                            \
    OSAPI_LOG_ENTRY_ADD_POINTER("pkt",(packet_),RTI_FALSE)    \
    OSAPI_LOG_ENTRY_ADD_POINTER("buf",(buf_),RTI_FALSE)       \
    OSAPI_LOG_ENTRY_ADD_INT("len",(len_),RTI_TRUE)

/*e
 * \brief Failed to create a concurrent queue
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_FAILED_TO_INIT_CONCURRENT_Q_EC     (SHMEM_LOG_BASE + 31)
#define NETIO_SHMEM_LOG_FAILED_TO_INIT_CONCURRENT_Q(level_, port_) \
    OSAPI_LOG_ENTRY_ADD_1INT(                                      \
        (level_),                                                  \
        NETIO_SHMEM_LOG_FAILED_TO_INIT_CONCURRENT_Q_EC,            \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,                             \
        "port",                                                    \
        (port_))

/*e
 * \brief Failed to attach to a concurrent queue
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_FAILED_TO_ATTACH_CONCURRENT_Q_EC   (SHMEM_LOG_BASE + 32)
#define NETIO_SHMEM_LOG_FAILED_TO_ATTACH_CONCURRENT_Q(level_) \
    OSAPI_LOG_ENTRY_ADD(                                      \
        (level_),                                             \
        NETIO_SHMEM_LOG_FAILED_TO_ATTACH_CONCURRENT_Q_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Attempting to attach to an incompatible concurrent queue with
 *        incompatible message_size_max
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_INCOMPATIBLE_CONCURRENT_Q_EC       (SHMEM_LOG_BASE + 33)
#define NETIO_SHMEM_LOG_INCOMPATIBLE_CONCURRENT_Q(level_, found_, needed_) \
    OSAPI_LOG_ENTRY_ADD_2INT(                                              \
        (level_),                                                          \
        NETIO_SHMEM_LOG_INCOMPATIBLE_CONCURRENT_Q_EC,                      \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,                                     \
        "found",                                                           \
        (found_),                                                          \
        "needed",                                                          \
        (needed_))

/*e
 * \brief Failed to get timestamp
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_FAILED_TO_GET_TIMESTAMP_EC         (SHMEM_LOG_BASE + 33)
#define NETIO_SHMEM_LOG_FAILED_TO_GET_TIMESTAMP(level_) \
    OSAPI_LOG_ENTRY_ADD(                                \
        (level_),                                       \
        NETIO_SHMEM_LOG_FAILED_TO_GET_TIMESTAMP_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to unlock shared memory mutex
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_NONFATAL_SIG_SEM_RC_EC             (SHMEM_LOG_BASE + 34)
#define NETIO_SHMEM_LOG_NONFATAL_SIG_SEM_RC(level_) \
    OSAPI_LOG_ENTRY_ADD(                            \
        (level_),                                   \
        NETIO_SHMEM_LOG_NONFATAL_SIG_SEM_RC_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to unlock shared memory mutex
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_INCONSISTENT_MESSAGE_SIZE_EC       (SHMEM_LOG_BASE + 35)
#define NETIO_SHMEM_LOG_INCONSISTENT_MESSAGE_SIZE(level_) \
    OSAPI_LOG_ENTRY_ADD(                                  \
        (level_),                                         \
        NETIO_SHMEM_LOG_INCONSISTENT_MESSAGE_SIZE_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Database operation on record failed
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_RECORD_EC                          (SHMEM_LOG_BASE + 36)
#define NETIO_SHMEM_LOG_RECORD(level_,dbrc_) \
    OSAPI_LOG_ENTRY_ADD_1INT(                \
        (level_),                            \
        NETIO_SHMEM_LOG_RECORD_EC,           \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,       \
        "dbrc",                              \
        (dbrc_))
/*e
 * \brief Failed to set packet head
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_PACKET_HEAD_EC                     (SHMEM_LOG_BASE + 37)
#define NETIO_SHMEM_LOG_PACKET_HEAD(level_,packet_,adjust_)   \
    OSAPI_LOG_ENTRY_CREATE(                                   \
        (level_),                                             \
        NETIO_SHMEM_LOG_PACKET_HEAD_EC,                       \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,                        \
        RTI_FALSE)                                            \
    OSAPI_LOG_ENTRY_ADD_POINTER("packet",(packet_),RTI_FALSE) \
    OSAPI_LOG_ENTRY_ADD_INT("adjust",(adjust_),RTI_TRUE)

/*e
 * \brief Failed reception upstream for a received packet
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_PACKET_FWD_EC                      (SHMEM_LOG_BASE + 38)
#define NETIO_SHMEM_LOG_PACKET_FWD(level_) \
    OSAPI_LOG_ENTRY_ADD(                   \
        (level_),                          \
        NETIO_SHMEM_LOG_PACKET_FWD_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed reception upstream for a received packet
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_QUEUE_FULL_EC                      (SHMEM_LOG_BASE + 39)
#define NETIO_SHMEM_LOG_QUEUE_FULL(level_, port_, msg_count_max_, size_) \
    OSAPI_LOG_ENTRY_ADD_3INT(                                            \
        (level_),                                                        \
        NETIO_SHMEM_LOG_QUEUE_FULL_EC,                                   \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,                                   \
        "port",                                                          \
        (port_),                                                         \
        "msg_count_max",                                                 \
        (msg_count_max_),                                                \
        "buf_size",                                                      \
        (size_))

/*e
 * \brief Failed to evict find an entry in the route table
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_ROUTE_LOOKUP_FAILED_EC             (SHMEM_LOG_BASE + 40)
#define NETIO_SHMEM_LOG_ROUTE_LOOKUP_FAILED(level_) \
    OSAPI_LOG_ENTRY_ADD(                            \
        (level_),                                   \
        NETIO_SHMEM_LOG_ROUTE_LOOKUP_FAILED_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to delete an entry from the route table
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_ROUTE_DELETE_FAILED_EC             (SHMEM_LOG_BASE + 41)
#define NETIO_SHMEM_LOG_ROUTE_DELETE_FAILED(level_) \
    OSAPI_LOG_ENTRY_ADD(                            \
        (level_),                                   \
        NETIO_SHMEM_LOG_ROUTE_DELETE_FAILED_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief The shared memory transport is ignoring the requested
 *        transport minimum compatibility version because it cannot
 *        be backward compatible
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_IGNORE_TRANSPORT_COMPATIBILITY_EC  (SHMEM_LOG_BASE + 42)
#define NETIO_SHMEM_LOG_IGNORE_TRANSPORT_COMPATIBILITY(level_) \
    OSAPI_LOG_ENTRY_ADD(                                       \
        (level_),                                              \
        NETIO_SHMEM_LOG_IGNORE_TRANSPORT_COMPATIBILITY_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Unsupported minimum compatibility version because it requires
 *        a version of the shared memory transport which is not supported.
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_SHMEM_LOG_UNSUPPORTED_TRANSPORT_COMPATIBILITY_EC  (SHMEM_LOG_BASE + 43)
#define NETIO_SHMEM_LOG_UNSUPPORTED_TRANSPORT_COMPATIBILITY(level_) \
    OSAPI_LOG_ENTRY_ADD(                                           \
        (level_),                                                  \
        NETIO_SHMEM_LOG_UNSUPPORTED_TRANSPORT_COMPATIBILITY_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*******************************************************************************
 *                    Concurrent Queue Log messages  
 ******************************************************************************/

/*e
 * \brief Error when attaching to concurrent queue. Inconsistent representation
 *        of size of integers between concurrent queue and attaching
 *        application
 * \ingroup REDALogCodesClass
 */
#define NETIO_SHMEM_LOG_CQ_INT_REPRESENTATION_EC          (SHMEM_LOG_BASE + 200)
#define NETIO_SHMEM_LOG_CQ_INT_REPRESENTATION(level_, expected_, actual_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),NETIO_SHMEM_LOG_CQ_INT_REPRESENTATION_EC,\
 OSAPI_LOG_MSG_PN_X2_STD_PARAM,"q uses",(expected_),"att prg uses", (actual_))

/*e
 * \brief Error when attaching to concurrent queue. Inconsistent representation
 *        of size of integers between concurrent queue and attaching
 *        application
 * \ingroup REDALogCodesClass
 */
#define NETIO_SHMEM_LOG_CQ_UNLINKED_EC                    (SHMEM_LOG_BASE + 201)
#define NETIO_SHMEM_LOG_CQ_UNLINKED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),NETIO_SHMEM_LOG_CQ_UNLINKED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Error when attaching to concurrent queue. Inconsistent representation
 *        of size of integers between concurrent queue and attaching
 *        application
 * \ingroup REDALogCodesClass
 */
#define NETIO_SHMEM_LOG_CQ_INVALID_SIGNATURE_EC           (SHMEM_LOG_BASE + 202)
#define NETIO_SHMEM_LOG_CQ_INVALID_SIGNATURE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),NETIO_SHMEM_LOG_CQ_INVALID_SIGNATURE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Error when attaching to concurrent queue. Inconsistent representation
 *        of size of integers between concurrent queue and attaching
 *        application
 * \ingroup REDALogCodesClass
 */
#define NETIO_SHMEM_LOG_CQ_INCOMPATIBLE_VERSION_EC        (SHMEM_LOG_BASE + 203)
#define NETIO_SHMEM_LOG_CQ_INCOMPATIBLE_VERSION(level_, expected_, actual_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),NETIO_SHMEM_LOG_CQ_INCOMPATIBLE_VERSION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"expected",(expected_),"found", (actual_))

/*e
 * \brief Found an inconsistent concurrent queue state while performing a
 *        a read or write. The queue memory may have been corrupted or
 *        otherwise tampered with.
 * \ingroup REDALogCodesClass
 */
#define NETIO_SHMEM_LOG_CQ_INCONSISTENT_STATE_EC          (SHMEM_LOG_BASE + 204)
#define NETIO_SHMEM_LOG_CQ_INCONSISTENT_STATE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),NETIO_SHMEM_LOG_CQ_INCONSISTENT_STATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

#endif /* netio_log_h */
