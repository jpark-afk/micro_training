/*
 * FILE: Notifiee.c - Notifiee implementation
 *
 * Copyright 2022-2024 Real-Time Innovations, Inc.
 * 
 * All rights reserved. 
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#include "Notifiee.h"

#include "netio_zcopy/netio_zcopy_log.h"
#include "osapi/osapi_heap.h"


/*** SOURCE_BEGIN ***/

RTI_BOOL
ZCOPY_Notifiee_exists(const struct ZCOPY_Guid *guid, RTI_UINT32 port)
{
    OSAPI_PRECONDITION(guid == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("guid", guid, RTI_TRUE);)

    return ZCOPY_NotificationObject_exists(guid, port);
}

struct ZCOPY_Notifiee *
ZCOPY_Notifiee_create(const struct ZCOPY_Guid *guid, RTI_UINT32 port, RTI_UINT32 slots)
{
    RTI_BOOL ok = RTI_FALSE;
    struct ZCOPY_Notifiee *self = NULL;

    OSAPI_PRECONDITION(guid == NULL,
                       return NULL,
                       OSAPI_Log_entry_add_pointer("guid", guid, RTI_TRUE);)

    if (slots == 0)
    {
        ZCOPY_LOG_NOTIF_INVALID_NUM_SLOTS(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    OSAPI_Heap_allocate_struct(&self, struct ZCOPY_Notifiee);
    if (self == NULL)
    {
        goto done;
    }

    self->handle = ZCOPY_NotificationObjectHandle_new();
    if (self->handle == NULL)
    {
        goto done;
    }

    if (!ZCOPY_NotificationObject_create(self->handle, guid, port, slots))
    {
        /* This is an expected error if a port has already been bound */
        goto done;
    }

    ok = RTI_TRUE;

done:
    if ((!ok) && (self != NULL))
    {
#ifndef RTI_CERT
        OSAPI_Heap_free(self);
#endif
        /* MICRO-5132: The memory for self is leaked here only when compiling
         * with RTI_CERT because memory is intentionally never freed for cert.
         */
        /* coverity[overwrite_var] */
        self = NULL;
    }
    return self;
}

RTI_BOOL
ZCOPY_Notifiee_destroy(struct ZCOPY_Notifiee *self)
{
    OSAPI_PRECONDITION(self == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("self", self, RTI_TRUE);)

    if (self->handle != NULL)
    {
        if (!ZCOPY_NotificationObject_destroy(self->handle))
        {
            return RTI_FALSE;
        }
#ifndef RTI_CERT
        if (!ZCOPY_NotificationObjectHandle_delete(self->handle))
        {
            return RTI_FALSE;
        }
#endif
        self->handle = NULL;
    }

#ifndef RTI_CERT
    OSAPI_Heap_free(self);
#endif

    return RTI_TRUE;
}

RTI_BOOL
ZCOPY_Notifiee_next(
        struct ZCOPY_Notifiee *self,
        struct ZCOPY_Guid *source_out,
        RTI_BOOL *notified_out)
{
    RTI_BOOL ok = RTI_FALSE;
    struct ZCOPY_Guid *next_guid = NULL;

    OSAPI_PRECONDITION(
            (self == NULL) || (source_out == NULL) || (notified_out == NULL),
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("source_out", source_out, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("notified_out", notified_out, RTI_TRUE);)

    if (!ZCOPY_NotificationObject_lock(self->handle))
    {
        return RTI_FALSE;
    }

    if (!ZCOPY_NotificationObject_next(self->handle, &next_guid))
    {
        goto done;
    }

    if (next_guid != NULL)
    {
        *notified_out = RTI_TRUE;
        *source_out = *next_guid;
    }
    else
    {
        *notified_out = RTI_FALSE;
    }

    ok = RTI_TRUE;

done:
    if (!ZCOPY_NotificationObject_unlock(self->handle))
    {
        return RTI_FALSE;
    }
    return ok;
}

RTI_BOOL
ZCOPY_Notifiee_allow_notifier(struct ZCOPY_Notifiee *self, const struct ZCOPY_Guid *guid)
{
    RTI_BOOL ok = RTI_FALSE;

    OSAPI_PRECONDITION((self == NULL) || (guid == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("guid", guid, RTI_TRUE);)

    if (!ZCOPY_NotificationObject_lock(self->handle))
    {
        return RTI_FALSE;
    }

    ok = ZCOPY_NotificationObject_assign_slot(self->handle, guid);

    if (!ZCOPY_NotificationObject_unlock(self->handle))
    {
        return RTI_FALSE;
    }
    return ok;
}

RTI_BOOL
ZCOPY_Notifiee_remove_notifier(struct ZCOPY_Notifiee *self, const struct ZCOPY_Guid *guid)
{
    RTI_BOOL ok = RTI_FALSE;

    OSAPI_PRECONDITION((self == NULL) || (guid == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("guid", guid, RTI_TRUE);)

    if (!ZCOPY_NotificationObject_lock(self->handle))
    {
        return RTI_FALSE;
    }

    ok = ZCOPY_NotificationObject_unassign_slot(self->handle, guid);

    if (!ZCOPY_NotificationObject_unlock(self->handle))
    {
        return RTI_FALSE;
    }
    return ok;
}

RTI_BOOL
ZCOPY_Notifiee_raise_flag(struct ZCOPY_Notifiee *self, const struct ZCOPY_Guid *guid)
{
    RTI_BOOL ok = RTI_FALSE;

    OSAPI_PRECONDITION((self == NULL) || (guid == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("guid", guid, RTI_TRUE);)

    if (!ZCOPY_NotificationObject_lock(self->handle))
    {
        return RTI_FALSE;
    }

    ok = ZCOPY_NotificationObject_raise_flag(self->handle, guid);

    if (!ZCOPY_NotificationObject_unlock(self->handle))
    {
        return RTI_FALSE;
    }
    return ok;
}
