/*
 * FILE: netio_zcopy_log.h - ZeroCopy Log definitions
 *
 * (c) Copyright 2022-2025 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*e
 * \file 
 * \brief ZeroCopy module log codes 
 *
 * \details
 * Log codes of the ZeroCopy module
 */
#ifndef netio_zcopy_log_h
#define netio_zcopy_log_h

#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif

/*e
 * \defgroup ZeroCopyLogCodesClass ZeroCopy
 * \brief Zero Copy V2. ModuleID = 14
 * \ingroup LoggingModule
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*e
 * \brief Failed to convert GUID to string representation
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_GUID_BUFFER_TOO_SMALL_EC      (NETIO_ZCOPY_LOG_BASE + 1)
#define ZCOPY_LOG_NOTIF_GUID_BUFFER_TOO_SMALL(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_GUID_BUFFER_TOO_SMALL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to connect because already connected
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_ALREADY_CONNECTED_EC          (NETIO_ZCOPY_LOG_BASE + 2)
#define ZCOPY_LOG_NOTIF_ALREADY_CONNECTED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_ALREADY_CONNECTED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to perform operation because notif is not connected
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_NOT_CONNECTED_EC              (NETIO_ZCOPY_LOG_BASE + 3)
#define ZCOPY_LOG_NOTIF_NOT_CONNECTED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_NOT_CONNECTED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to connect because of incompatible ZCV2 version
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_INCOMPATIBLE_VERSION_EC       (NETIO_ZCOPY_LOG_BASE + 4)
#define ZCOPY_LOG_NOTIF_INCOMPATIBLE_VERSION(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_INCOMPATIBLE_VERSION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to assign a slot because it is already assigned
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_SLOT_ALREADY_ASSIGNED_EC      (NETIO_ZCOPY_LOG_BASE + 5)
#define ZCOPY_LOG_NOTIF_SLOT_ALREADY_ASSIGNED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_SLOT_ALREADY_ASSIGNED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to assign a slot because there is no free slot
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_NO_FREE_SLOT_EC               (NETIO_ZCOPY_LOG_BASE + 6)
#define ZCOPY_LOG_NOTIF_NO_FREE_SLOT(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_NO_FREE_SLOT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to connect because of invalid number of slots
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_INVALID_NUM_SLOTS_EC          (NETIO_ZCOPY_LOG_BASE + 7)
#define ZCOPY_LOG_NOTIF_INVALID_NUM_SLOTS(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_INVALID_NUM_SLOTS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to convert GUID to string representation because of an invalid port number
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_INVALID_PORT_NUMBER_EC        (NETIO_ZCOPY_LOG_BASE + 8)
#define ZCOPY_LOG_NOTIF_INVALID_PORT_NUMBER(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_INVALID_PORT_NUMBER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to open notification interface
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_OPEN_FAILED_EC                (NETIO_ZCOPY_LOG_BASE + 9)
#define ZCOPY_LOG_NOTIF_OPEN_FAILED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_OPEN_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)
/*e
 * \brief Notification interface failed to select a database record
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_DB_SELECT_RECORD_EC          (NETIO_ZCOPY_LOG_BASE + 10)
#define ZCOPY_LOG_NOTIF_DB_SELECT_RECORD(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),ZCOPY_LOG_NOTIF_DB_SELECT_RECORD_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Notification interface failed to create a database record
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_DB_CREATE_RECORD_EC          (NETIO_ZCOPY_LOG_BASE + 11)
#define ZCOPY_LOG_NOTIF_DB_CREATE_RECORD(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),ZCOPY_LOG_NOTIF_DB_CREATE_RECORD_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Notification interface failed to insert a database record
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_DB_INSERT_RECORD_EC          (NETIO_ZCOPY_LOG_BASE + 12)
#define ZCOPY_LOG_NOTIF_DB_INSERT_RECORD(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),ZCOPY_LOG_NOTIF_DB_INSERT_RECORD_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Notification interface failed to remove a database record
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_DB_REMOVE_RECORD_EC          (NETIO_ZCOPY_LOG_BASE + 13)
#define ZCOPY_LOG_NOTIF_DB_REMOVE_RECORD(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),ZCOPY_LOG_NOTIF_DB_REMOVE_RECORD_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Notification interface failed to delete a database record
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_DB_DELETE_RECORD_EC          (NETIO_ZCOPY_LOG_BASE + 14)
#define ZCOPY_LOG_NOTIF_DB_DELETE_RECORD(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),ZCOPY_LOG_NOTIF_DB_DELETE_RECORD_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Notification interface failed to get the next database record from a cursor
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_DB_CURSOR_NEXT_EC            (NETIO_ZCOPY_LOG_BASE + 15)
#define ZCOPY_LOG_NOTIF_DB_CURSOR_NEXT(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),ZCOPY_LOG_NOTIF_DB_CURSOR_NEXT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Notification interface failed to lock or unlock its database
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_DB_LOCK_EC                   (NETIO_ZCOPY_LOG_BASE + 16)
#define ZCOPY_LOG_NOTIF_DB_LOCK(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),ZCOPY_LOG_NOTIF_DB_LOCK_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Notification interface failed to create a table
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_DB_TABLE_CREATE_EC           (NETIO_ZCOPY_LOG_BASE + 17)
#define ZCOPY_LOG_NOTIF_DB_TABLE_CREATE(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),ZCOPY_LOG_NOTIF_DB_TABLE_CREATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Notification interface failed to delete a table
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_DB_TABLE_DELETE_EC           (NETIO_ZCOPY_LOG_BASE + 18)
#define ZCOPY_LOG_NOTIF_DB_TABLE_DELETE(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),ZCOPY_LOG_NOTIF_DB_TABLE_DELETE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Notification interface initialized with an invalid property
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_INVALID_PROPERTY_EC          (NETIO_ZCOPY_LOG_BASE + 19)
#define ZCOPY_LOG_NOTIF_INVALID_PROPERTY(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_INVALID_PROPERTY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Notification interface failed to open a shared queue reader
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_SQ_OPEN_EC                   (NETIO_ZCOPY_LOG_BASE + 20)
#define ZCOPY_LOG_NOTIF_SQ_OPEN(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_SQ_OPEN_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Notification interface failed to close a shared queue reader
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_SQ_CLOSE_EC                  (NETIO_ZCOPY_LOG_BASE + 21)
#define ZCOPY_LOG_NOTIF_SQ_CLOSE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_SQ_CLOSE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Notification interface ran out of notifiers in its pool of notifiers
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_NO_MORE_NOTIFIERS_EC         (NETIO_ZCOPY_LOG_BASE + 22)
#define ZCOPY_LOG_NOTIF_NO_MORE_NOTIFIERS(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_NO_MORE_NOTIFIERS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Notification interface failed to connect a notifier to a notifiee
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_NOTIFIER_CONNECT_EC          (NETIO_ZCOPY_LOG_BASE + 23)
#define ZCOPY_LOG_NOTIF_NOTIFIER_CONNECT(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_NOTIFIER_CONNECT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Notification interface failed to disconnect a notifier from a notifiee
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_NOTIFIER_DISCONNECT_EC       (NETIO_ZCOPY_LOG_BASE + 24)
#define ZCOPY_LOG_NOTIF_NOTIFIER_DISCONNECT(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_NOTIFIER_DISCONNECT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Notification interface failed to notify with a notifier
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_NOTIFIER_NOTIFY_EC           (NETIO_ZCOPY_LOG_BASE + 25)
#define ZCOPY_LOG_NOTIF_NOTIFIER_NOTIFY(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_NOTIFIER_NOTIFY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Notification interface failed to allow a notifier to notify
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_NOTIFIER_ALLOW_EC            (NETIO_ZCOPY_LOG_BASE + 26)
#define ZCOPY_LOG_NOTIF_NOTIFIER_ALLOW(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_NOTIFIER_ALLOW_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Notification interface failed to destroy a notifiee
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_NOTIFIEE_DESTROY_EC          (NETIO_ZCOPY_LOG_BASE + 27)
#define ZCOPY_LOG_NOTIF_NOTIFIEE_DESTROY(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_NOTIFIEE_DESTROY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Notification interface failed to raise a flag
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_NOTIFIEE_RAISE_FLAG_EC       (NETIO_ZCOPY_LOG_BASE + 28)
#define ZCOPY_LOG_NOTIF_NOTIFIEE_RAISE_FLAG(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_NOTIFIEE_RAISE_FLAG_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Notification interface failed to get next notification
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_NOTIFIEE_NEXT_EC             (NETIO_ZCOPY_LOG_BASE + 29)
#define ZCOPY_LOG_NOTIF_NOTIFIEE_NEXT(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_NOTIFIEE_NEXT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Notification interface failed to create a receive thread
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_THREAD_CREATE_EC             (NETIO_ZCOPY_LOG_BASE + 30)
#define ZCOPY_LOG_NOTIF_THREAD_CREATE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_THREAD_CREATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Notification interface failed to destroy a receive thread
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_THREAD_DESTROY_EC            (NETIO_ZCOPY_LOG_BASE + 31)
#define ZCOPY_LOG_NOTIF_THREAD_DESTROY(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_THREAD_DESTROY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Notification interface failed to forward a packet upstream to a DRI
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_INTF_RECEIVE_EC              (NETIO_ZCOPY_LOG_BASE + 32)
#define ZCOPY_LOG_NOTIF_INTF_RECEIVE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_INTF_RECEIVE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Notification interface tried to release a port that was still in use
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_PORT_IN_USE_EC               (NETIO_ZCOPY_LOG_BASE + 33)
#define ZCOPY_LOG_NOTIF_PORT_IN_USE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_PORT_IN_USE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Notification interface factory failed to allocate memory
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_ALLOC_EC                     (NETIO_ZCOPY_LOG_BASE + 34)
#define ZCOPY_LOG_NOTIF_ALLOC(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_ALLOC_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Notification interface factory failed to initialize a notification interface
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_INTF_INIT_EC                 (NETIO_ZCOPY_LOG_BASE + 35)
#define ZCOPY_LOG_NOTIF_INTF_INIT(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_INTF_INIT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Notification interface factory failed to finalize a notification interface
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_INTF_FINALIZE_EC             (NETIO_ZCOPY_LOG_BASE + 36)
#define ZCOPY_LOG_NOTIF_INTF_FINALIZE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_INTF_FINALIZE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Tried to finalize a notification interface factory which was still in use
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_FACTORY_IN_USE_EC            (NETIO_ZCOPY_LOG_BASE + 37)
#define ZCOPY_LOG_NOTIF_FACTORY_IN_USE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_FACTORY_IN_USE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Notification mechanism failed to create instance
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_MECHANISM_CREATE_INST_EC     (NETIO_ZCOPY_LOG_BASE + 38)
#define ZCOPY_LOG_NOTIF_MECHANISM_CREATE_INST(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_MECHANISM_CREATE_INST_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Notification mechanism failed to reserve an address
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_MECHANISM_RESERVE_ADDR_EC    (NETIO_ZCOPY_LOG_BASE + 39)
#define ZCOPY_LOG_NOTIF_MECHANISM_RESERVE_ADDR(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_MECHANISM_RESERVE_ADDR_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Notification mechanism failed to release an address
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_MECHANISM_RELEASE_ADDR_EC    (NETIO_ZCOPY_LOG_BASE + 40)
#define ZCOPY_LOG_NOTIF_MECHANISM_RELEASE_ADDR(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_MECHANISM_RELEASE_ADDR_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Notification mechanism failed to add route
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_MECHANISM_ADD_ROUTE_EC       (NETIO_ZCOPY_LOG_BASE + 41)
#define ZCOPY_LOG_NOTIF_MECHANISM_ADD_ROUTE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_MECHANISM_ADD_ROUTE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Notification mechanism failed to delete route
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_MECHANISM_DELETE_ROUTE_EC    (NETIO_ZCOPY_LOG_BASE + 42)
#define ZCOPY_LOG_NOTIF_MECHANISM_DELETE_ROUTE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_MECHANISM_DELETE_ROUTE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Notification mechanism failed to bind
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_MECHANISM_BIND_EC            (NETIO_ZCOPY_LOG_BASE + 43)
#define ZCOPY_LOG_NOTIF_MECHANISM_BIND(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_MECHANISM_BIND_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Notification mechanism failed to unbind
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_MECHANISM_UNBIND_EC          (NETIO_ZCOPY_LOG_BASE + 44)
#define ZCOPY_LOG_NOTIF_MECHANISM_UNBIND(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_MECHANISM_UNBIND_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Notification mechanism failed to send
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_MECHANISM_SEND_EC            (NETIO_ZCOPY_LOG_BASE + 45)
#define ZCOPY_LOG_NOTIF_MECHANISM_SEND(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_MECHANISM_SEND_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Writer history wrap failed. 
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_WH_REGISTER_FAILED_EC            (NETIO_ZCOPY_LOG_BASE + 46)
#define ZCOPY_LOG_WH_REGISTER_FAILED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_WH_REGISTER_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Timer creation failed
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_TIMER_CREATE_EC            (NETIO_ZCOPY_LOG_BASE + 47)
#define ZCOPY_LOG_NOTIF_TIMER_CREATE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_TIMER_CREATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Receive on Writer match event failed
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_RECEIVE_ON_EVENT_EC            (NETIO_ZCOPY_LOG_BASE + 48)
#define ZCOPY_LOG_NOTIF_RECEIVE_ON_EVENT(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_RECEIVE_ON_EVENT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to delete timeout
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_TIMEOUT_DELETE_EC            (NETIO_ZCOPY_LOG_BASE + 49)
#define ZCOPY_LOG_NOTIF_TIMEOUT_DELETE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_TIMEOUT_DELETE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Timeout creation failed
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_NOTIF_TIMEOUT_CREATE_EC            (NETIO_ZCOPY_LOG_BASE + 50)
#define ZCOPY_LOG_NOTIF_TIMEOUT_CREATE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_NOTIF_TIMEOUT_CREATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief failed to add an indexer entry
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_ADD_ENTRY_EC            (NETIO_ZCOPY_LOG_BASE + 51)
#define ZCOPY_LOG_ADD_ENTRY(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_ADD_ENTRY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

 /*e
 * \brief failed to remove an indexer entry
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_REMOVE_ENTRY_EC            (NETIO_ZCOPY_LOG_BASE + 52)
#define ZCOPY_LOG_REMOVE_ENTRY(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_REMOVE_ENTRY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief The memory manager was not able to delete a resource.
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_DELETE_POOL_EC            (NETIO_ZCOPY_LOG_BASE + 53)
#define ZCOPY_LOG_DELETE_POOL(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_DELETE_POOL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief A shared memory pool is no longer usable because a
 *        process died while holding the pool's lock.
 *
 * \ingroup ZeroCopyLogCodesClass
 */
#define ZCOPY_LOG_SHARED_MEM_POOL_OWNER_DEAD_EC      (NETIO_ZCOPY_LOG_BASE + 54)
#define ZCOPY_LOG_SHARED_MEM_POOL_OWNER_DEAD(level_) \
OSAPI_LOG_ENTRY_ADD((level_),ZCOPY_LOG_SHARED_MEM_POOL_OWNER_DEAD_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* netio_zcopy_log_h */
