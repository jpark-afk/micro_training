/*
 * FILE: NotificationObject.h
 *
 * Copyright 2022-2024 Real-Time Innovations, Inc.
 * 
 * All rights reserved. 
 * 
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef NotificationObject_h
#define NotificationObject_h

#include "netio_zcopy/netio_zcopy_guid.h"
#include "netio_zcopy/netio_zcopy_shm_segment.h"
#include "netio_zcopy/netio_zcopy_shm_monitor.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*i
 * \defgroup ZCOPY_NotificationObjectClass ZCOPY_NotificationObject
 *
 * \brief Allow a process to block until it receives a notification to wake-up
 *        from another process.
 */

/*i
 * \ingroup ZCOPY_NotificationObjectClass
 *
 * Generic version info struct for detecting compatiblity with a notification object
 */
struct ZCOPY_NotificationObjectVersion
{
    char id[2];
    struct
    {
        RTI_UINT8 high;
        RTI_UINT8 low;
    } version;
};

/*i
 * \ingroup ZCOPY_NotificationObjectClass
 *
 * Specific version info notification object's created by this version of code
 */
extern const struct ZCOPY_NotificationObjectVersion ZCOPY_NotificationObject_gv_Version;

/*i
 * \ingroup ZCOPY_NotificationObjectClass
 *
 * A slot in a notification object, which can be assigned to notifier to allow
 * it to notify. Each slot is a position independent linked list node. If the
 * flag is raised then, it is in the linked list of pending notifications.
 */
struct ZCOPY_NotificationSlot
{
    /*i The owner allowed to use this slot */
    struct ZCOPY_Guid id;

    /*i Previous node in linked list */
    RTI_UINT32 prev;

    /*i Next node in linked list */
    RTI_UINT32 next;

    /*i Whether or not the slot has been assigned */
    RTI_BOOL in_use;

    /* Whether or not the owner has signaled a pending notification */
    RTI_BOOL flag;

    /*i Reference count for the slot */
    RTI_UINT32 ref_count;
};

/*i
 * \ingroup ZCOPY_NotificationObjectClass
 *
 * A position independent notification object which can be placed in shared
 * memory. It contains a certain number of slots which can be assigned to
 * notifiers to allow them to notify. A slot can be inserted into the linked
 * list to indicate a pending notification.
 */
struct ZCOPY_NotificationObject_PI
{
    /*i Version used to check compatibility */
    struct ZCOPY_NotificationObjectVersion version;

    /*i Size of flexible array of slots */
    RTI_UINT32 num_slots;

    /*i Index in the slots of the head of the linked list */
    RTI_UINT32 head;

    /*i Index in the slots of the tail of the linked list */
    RTI_UINT32 tail;

    /*i Whether or not a session of emptying the queue is in progress */
    RTI_BOOL active_session;

    /*i Index in the slots of last notification to include in current session */
    RTI_UINT32 session_tail;

    /*i Generation of currently allowed notifiers */
    RTI_UINT64 generation;

    /*i Array of linked list nodes for notifiers */
    struct ZCOPY_NotificationSlot slots[/* num_slots */];
};

/*i
 * \ingroup ZCOPY_NotificationObjectClass
 *
 * Handle to be stored on stack or heap for storing pointers to a
 * position independent notification object in shared memory.
 */
struct ZCOPY_NotificationObjectHandle
{
    /*i Handle to shared memory storing a notification object */
    struct OSAPI_SharedMemorySegmentHandle *shm_handle;

    /*i Pointer to position independent notification object in shared memory */
    struct ZCOPY_NotificationObject_PI *notif_pi;

    /*i Hint to last accessed index to speed up search for an assigned slot */
    const struct ZCOPY_NotificationSlot *slot_hint;

    /*i Generation of slots when it was last checked for an assigned slot */
    RTI_UINT64 generation;

    /* Name used to open the NotificationObject */
    char name[OSAPI_SHMEM_MAX_NAME_LENGTH + 1];
};

/*i
 * \ingroup ZCOPY_NotificationObjectClass
 *
 * \brief Check if a notification object identified by a given guid already exists
 *
 * \param[in] id Guid used to identify the notifiee
 *
 * \return RTI_TRUE if the notifiee exists. Otherwise, RTI_FALSE.
 */
extern RTI_BOOL
ZCOPY_NotificationObject_exists(const struct ZCOPY_Guid *guid, RTI_UINT32 port);

/*i
 * \ingroup ZCOPY_NotificationObjectClass
 *
 * \brief Creates a handle to be used to create or open a notification object.
 *
 * \return Pointer to handle if successful. NULL on failure.
 */
extern struct ZCOPY_NotificationObjectHandle *
ZCOPY_NotificationObjectHandle_new(void);

#ifndef RTI_CERT
/*i
 * \ingroup ZCOPY_NotificationObjectClass
 *
 * \brief Delete a handle to a notification object.
 *
 * \param[in] self The handle to the notification object
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 *
 * Once this function is called (even if it returns RTI_FALSE) you can no longer
 * use this notification object handle.
 */
extern RTI_BOOL
ZCOPY_NotificationObjectHandle_delete(struct ZCOPY_NotificationObjectHandle *handle);
#endif

/*i
 * \ingroup ZCOPY_NotificationObjectClass
 *
 * \brief Lock a notification object to gain mutual exclusion.
 *
 * \param[in] self The handle to the notification object
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 */
extern RTI_BOOL
ZCOPY_NotificationObject_lock(struct ZCOPY_NotificationObjectHandle *handle);

/*i
 * \ingroup ZCOPY_NotificationObjectClass
 *
 * \brief Unlock a notification object to give up mutual exclusion.
 *
 * \param[in] self The handle to the notification object
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 *
 * The lock on the notification object must be held by the thread calling this
 * function.
 */
extern RTI_BOOL
ZCOPY_NotificationObject_unlock(struct ZCOPY_NotificationObjectHandle *handle);

/*i
 * \ingroup ZCOPY_NotificationObjectClass
 *
 * \brief Create a notification object identified by a given guid
 *
 * \param[in] self The handle to the notification object
 * \param[in] guid Guid to be used to identify the notification object
 * \param[in] port Port number
 * \param[in] slots Number of slots to be allocated for notifiers
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 */
extern RTI_BOOL
ZCOPY_NotificationObject_create(
        struct ZCOPY_NotificationObjectHandle *handle,
        const struct ZCOPY_Guid *guid,
        RTI_UINT32 port,
        RTI_UINT32 slots);

/*i
 * \ingroup ZCOPY_NotificationObjectClass
 *
 * \brief Destroy a notification object
 *
 * \param[in] self The handle to the notification object
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 *
 * Once this function is called, the notification object can no longer be
 * used, but the handle can be reused to create or attach to a new notification
 * object.
 */
extern RTI_BOOL
ZCOPY_NotificationObject_destroy(struct ZCOPY_NotificationObjectHandle *handle);

/*i
 * \ingroup ZCOPY_NotificationObjectClass
 *
 * \brief Assign a slot in a notification object to a notifier
 *
 * \param[in] self The handle to the notification object
 * \param[in] guid The guid of the notifier
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 *
 * The lock on the notification object must be held by the thread calling this
 * function.
 */
extern RTI_BOOL
ZCOPY_NotificationObject_assign_slot(
        struct ZCOPY_NotificationObjectHandle *handle,
        const struct ZCOPY_Guid *guid);

/*i
 * \ingroup ZCOPY_NotificationObjectClass
 *
 * \brief Reclaim a slot from a notifier to prevent it from notifying. If it
 *        has a pending notification, remove it from the linked list.
 *
 * \param[in] self The handle to the notification object
 * \param[in] guid The guid of the notifier
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 *
 * The lock on the notification object must be held by the thread calling this
 * function.
 */
extern RTI_BOOL
ZCOPY_NotificationObject_unassign_slot(
        struct ZCOPY_NotificationObjectHandle *handle,
        const struct ZCOPY_Guid *guid);

/*i
 * \ingroup ZCOPY_NotificationObjectClass
 *
 * \brief Pop the next pending notification from a notification object
 *
 * \param[in] self The handle to the notification object
 * \param[out] guid_out The guid of the source of the notification or NULL
 *                      if there are no pending notifications.
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 *
 * The lock on the notification object must be held by the thread calling this
 * function.
 */
extern RTI_BOOL
ZCOPY_NotificationObject_next(
        struct ZCOPY_NotificationObjectHandle *handle,
        struct ZCOPY_Guid **guid_out);

/*i
 * \ingroup ZCOPY_NotificationObjectClass
 *
 * \brief Open a connection to an existing notification object
 *
 * \param[in] self The handle to the notification object
 * \param[in] guid Guid used to identify the notification object. It must match
 *                 the guid previously used to call \ref ZCOPY_NotificationObject_create
 * \param[in] port Port number
 * \param[out] is_connected_out In case of success, an indication whether the
 *                              actual connection to the NotificationObject was
 *                              set up (TRUE) or not (FALSE).
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 *         Note that the notification object may not have actually attached to the
 *         associated shared memory object yet. Checking the is_connected_out
 *         parameter is necessary for that.
 */
extern RTI_BOOL
ZCOPY_NotificationObject_open(
        struct ZCOPY_NotificationObjectHandle *handle,
        const struct ZCOPY_Guid *guid,
        RTI_UINT32 port,
        RTI_BOOL *is_connected_out);

/*i
 * \ingroup ZCOPY_NotificationObjectClass
 *
 * \brief Re-try opening a connection to an existing notification object
 *
 * \param[in] self The handle to the notification object. The open() function
 *                 must have been called on it before.
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 *         Note that the notification object may not have actually attached to the
 *         associated shared memory object yet. Checking the is_connected_out
 *         parameter is necessary for that.
 *
 * This function is intended to ensure that the notification object is connected.
 * It is idempotent and can be invoked multiple times without any harm. Retrying
 * to open an already connected NotificationObject is a cheap action.
 */
extern RTI_BOOL
ZCOPY_NotificationObject_open_retry(
        struct ZCOPY_NotificationObjectHandle *handle,
        RTI_BOOL *is_connected_out);
/*i
 * \ingroup ZCOPY_NotificationObjectClass
 *
 * \brief Close a connection to a connected notification object
 *
 * \param[in] self The handle to the notification object
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 */
extern RTI_BOOL
ZCOPY_NotificationObject_close(struct ZCOPY_NotificationObjectHandle *handle);

/*i
 * \ingroup ZCOPY_NotificationObjectClass
 *
 * \brief Add a notifier to the list of pending notifications
 *
 * \param[in] self The handle to the notification object
 * \param[in] guid Guid of the notifier
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 *
 * The lock on the notification object must be held by the thread calling this
 * function.
 */
extern RTI_BOOL
ZCOPY_NotificationObject_raise_flag(
        struct ZCOPY_NotificationObjectHandle *handle,
        const struct ZCOPY_Guid *guid);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NotificationObject_h */
