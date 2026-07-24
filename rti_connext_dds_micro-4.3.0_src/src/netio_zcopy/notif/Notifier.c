/*
 * FILE: Notifier.c - Notifier implementation
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
#include "Notifier.h"

#include "netio_zcopy/netio_zcopy_log.h"

/*** SOURCE_BEGIN ***/

RTI_SIZE_T
ZCOPY_Notifier_get_size(void)
{
    return sizeof(struct ZCOPY_Notifier);
}

RTI_BOOL
ZCOPY_Notifier_initialize(struct ZCOPY_Notifier *self)
{
    RTI_BOOL ok = RTI_FALSE;
    struct ZCOPY_NotificationObjectHandle *handle = NULL;

    OSAPI_PRECONDITION(self == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("self", self, RTI_TRUE);)

    handle = ZCOPY_NotificationObjectHandle_new();
    if (handle == NULL)
    {
        goto done;
    }

    self->handle = handle;
    self->id = (struct ZCOPY_Guid)ZCOPY_NOTIF_GUID_INVALID;
    self->is_connected = RTI_FALSE;

    ok = RTI_TRUE;

done:
    return ok;
}

#ifndef RTI_CERT
RTI_BOOL
ZCOPY_Notifier_finalize(struct ZCOPY_Notifier *self)
{
    OSAPI_PRECONDITION(self == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("self", self, RTI_TRUE);)

    if (self->handle != NULL)
    {
        ZCOPY_NotificationObjectHandle_delete(self->handle);
        self->handle = NULL;
    }

    return RTI_TRUE;
}
#endif /* !RTI_CERT */

RTI_BOOL
ZCOPY_Notifier_open(
        struct ZCOPY_Notifier *self,
        const struct ZCOPY_Guid *notifier_id,
        const struct ZCOPY_Guid *notifiee_id,
        RTI_UINT32 port)
{
    RTI_BOOL is_connected = RTI_FALSE;

    OSAPI_PRECONDITION((self == NULL) || (notifier_id == NULL) || (notifiee_id == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("notifier_id", notifier_id, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("notifiee_id", notifiee_id, RTI_TRUE);)

    if (!ZCOPY_NotificationObject_open(self->handle, notifiee_id, port, &is_connected))
    {
        ZCOPY_LOG_NOTIF_OPEN_FAILED(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    self->id = *notifier_id;
    self->is_connected = is_connected;

    return RTI_TRUE;
}

RTI_BOOL
ZCOPY_Notifier_close(struct ZCOPY_Notifier *self)
{
    OSAPI_PRECONDITION(self == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("self", self, RTI_TRUE);)

    return ZCOPY_NotificationObject_close(self->handle);
}

RTI_BOOL
ZCOPY_Notifier_notify(struct ZCOPY_Notifier *self)
{
    RTI_BOOL ok = RTI_FALSE;
    RTI_BOOL object_locked = RTI_FALSE;

    OSAPI_PRECONDITION(self == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("self", self, RTI_TRUE);)

    /* Make sure that we are connected, and retry if not */
    if (!self->is_connected)
    {
        /* Note that is_connected only gets updated in case of success */
        if (!ZCOPY_NotificationObject_open_retry(self->handle, &self->is_connected))
        {
            goto done;
        }
    }

    /* It may still be possible that we are not connected, so double check */
    if (self->is_connected)
    {
        if (!ZCOPY_NotificationObject_lock(self->handle))
        {
            goto done;
        }
        object_locked = RTI_TRUE;

        if (!ZCOPY_NotificationObject_raise_flag(self->handle, &self->id))
        {
            goto done;
        }
    }

    /* Note that not being connected is not considered an error, even though
     *   the Notifier was not actually able to do the notifying.
     *   The expectation is that connection will succeed later, when the
     *   system is fully initialized. */
    ok = RTI_TRUE;
done:
    if (object_locked &&
        !ZCOPY_NotificationObject_unlock(self->handle))
    {
        ok = RTI_FALSE;
    }

    return ok;
}
