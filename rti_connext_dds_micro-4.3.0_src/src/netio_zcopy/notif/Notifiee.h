/*
 * FILE: Notifiee.h
 *
 * Copyright 2022-2023 Real-Time Innovations, Inc.
 * All rights reserved. 
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef Notifiee_h
#define Notifiee_h

#include "netio_zcopy/netio_zcopy_guid.h"
#include "NotificationObject.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*i
 * \defgroup ZCOPY_NotifieeClass ZCOPY_Notifiee
 * \brief Notifiee is a class which provides a mechanism
 *        for polling if a notification has been received
 *        and getting the source of the notification.
 */

struct ZCOPY_Notifiee
{
    /*i Notification object handle */
    struct ZCOPY_NotificationObjectHandle *handle;
};

/*i
 * \ingroup ZCOPY_NotifieeClass
 *
 * \brief Check if a notifiee identified by a given guid already exists
 *
 * \param[in] guid Guid used to identify the notifiee
 * \param[in] port Port number
 *
 * \return RTI_TRUE if the notifiee exists. Otherwise, RTI_FALSE.
 */
extern RTI_BOOL
ZCOPY_Notifiee_exists(const struct ZCOPY_Guid *guid, RTI_UINT32 port);

/*i
 * \ingroup ZCOPY_NotifieeClass
 *
 * \brief Creates a notifiee
 *
 * \param[in] id Guid used to identify the notifiee
 * \param[in] port Port number
 * \param[in] slots Maximum number of notifiers which can notify this notifiee
 *
 * \return Pointer to Notifiee if successful. NULL on failure.
 */
extern struct ZCOPY_Notifiee *
ZCOPY_Notifiee_create(const struct ZCOPY_Guid *guid, RTI_UINT32 port, RTI_UINT32 slots);

/*i
 * \ingroup ZCOPY_NotifieeClass
 *
 * \brief Destroy a notifiee
 *
 * \param[in] self The notifiee
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 *
 * Releases all the resources used by the notifiee and allows another notifiee
 * to use this notifiee's id. After this function is called, this notifiee,
 * can no longer be used.
 */
extern RTI_BOOL
ZCOPY_Notifiee_destroy(struct ZCOPY_Notifiee *self);

/*i
 * \ingroup ZCOPY_NotifieeClass
 *
 * \brief Take the next notification from the queue of pending notifications
 *
 * \param[in] self The notifiee
 * \param[out] source_out Guid of the notifier which notified
 * \param[out] notified_out Whether or not there was a pending notification
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 */
extern RTI_BOOL
ZCOPY_Notifiee_next(
        struct ZCOPY_Notifiee *self,
        struct ZCOPY_Guid *source_out,
        RTI_BOOL *notified_out);

/*i
 * \ingroup ZCOPY_NotifieeClass
 *
 * \brief Allow a notifier to notify a notifiee
 *
 * \param[in] self The notifiee
 * \param[in] id Guid identifying the notifier to allow
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 */
extern RTI_BOOL
ZCOPY_Notifiee_allow_notifier(struct ZCOPY_Notifiee *self, const struct ZCOPY_Guid *guid);

/*i
 * \ingroup ZCOPY_NotifieeClass
 *
 * \brief Revoke a notifier's ability to notify a notifiee
 *
 * \param[in] self The notifiee
 * \param[in] id Guid identifying the notifier to remove
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 */
extern RTI_BOOL
ZCOPY_Notifiee_remove_notifier(
        struct ZCOPY_Notifiee *self,
        const struct ZCOPY_Guid *guid);

/*i
 * \ingroup ZCOPY_NotifieeClass
 *
 * \brief Indicate that a notifier's notification requires more processing
 *        and defer this processing until later. Does not signal the notifiee
 *        because this function should be called from the same thread that is
 *        waiting on the notifiee.
 *
 * \param[in] self The notifiee
 * \param[in] id Guid identifying the notifier to
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 */
extern RTI_BOOL
ZCOPY_Notifiee_raise_flag(struct ZCOPY_Notifiee *self, const struct ZCOPY_Guid *guid);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* Notifiee_h */
