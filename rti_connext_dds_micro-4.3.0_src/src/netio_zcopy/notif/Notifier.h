/*
 * FILE: Notifier.h
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
#ifndef Notifier_h
#define Notifier_h

#include "netio_zcopy/netio_zcopy_guid.h"
#include "NotificationObject.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*i
 * \defgroup ZCOPY_NotifierClass ZCOPY_Notifier
 * \brief Notifier is a class which enables the notification of a Notifiee.
 */
struct ZCOPY_Notifier
{
    /*i Notification object handle */
    struct ZCOPY_NotificationObjectHandle *handle;

    /*i Guid identifying this notifier */
    struct ZCOPY_Guid id;

    /*i Flag to track whether opening the NotificationObject has succeeded yet */
    RTI_BOOL is_connected;
};

/*i
 * \ingroup ZCOPY_NotifierClass
 *
 * \brief Get the number of bytes required to store a notifier
 *
 * \return Number of bytes required to store a notifier
 */
extern RTI_SIZE_T
ZCOPY_Notifier_get_size(void);

/*i
 * \ingroup ZCOPY_NotifierClass
 *
 * \brief Initialize a notifier
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 *
 * The notifier is initialize in a disconnected state and cannot notify until it
 * is connected to a notifier with \ref ZCOPY_Notifier_open
 */
extern RTI_BOOL
ZCOPY_Notifier_initialize(struct ZCOPY_Notifier *self);

#ifndef RTI_CERT
/*i
 * \ingroup ZCOPY_NotifierClass
 *
 * \brief Finalize a notifier
 *
 * \param[in] self The notifier
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 */
extern RTI_BOOL
ZCOPY_Notifier_finalize(struct ZCOPY_Notifier *self);
#endif /* !RTI_CERT */

/*i
 * \ingroup ZCOPY_NotifierClass
 *
 * \brief Open a connection from a notifier to a notifiee
 *
 * \param[in] self The notifier
 * \param[in] notifier_id Guid identifying the notifier
 * \param[in] notifiee_id Guid identifying the notifiee
 * \param[in] port Port number
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 */
extern RTI_BOOL
ZCOPY_Notifier_open(
        struct ZCOPY_Notifier *self,
        const struct ZCOPY_Guid *notifier_id,
        const struct ZCOPY_Guid *notifiee_id,
        RTI_UINT32 port);

/*i
 * \ingroup ZCOPY_NotifierClass
 *
 * \brief Close a connection from a notifier to a notifiee
 *
 * \param[in] self The notifier
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 */
extern RTI_BOOL
ZCOPY_Notifier_close(struct ZCOPY_Notifier *self);

/*i
 * \ingroup ZCOPY_NotifierClass
 *
 * \brief Notify the notifiee connected to a notifier
 *
 * \param[in] self The notifier
 *
 * \return RTI_TRUE on success. RTI_FALSE if an error occurred.
 */
extern RTI_BOOL
ZCOPY_Notifier_notify(struct ZCOPY_Notifier *self);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* Notifier_h */
