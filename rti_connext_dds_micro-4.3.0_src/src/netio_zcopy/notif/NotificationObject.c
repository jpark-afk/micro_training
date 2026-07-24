/*
 * FILE: NotificationObject.c - NotificationObject implementation
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

#include "NotificationObject.h"
#include "osapi/osapi_log.h"
#include "osapi/osapi_string.h"
#include "netio_zcopy/netio_zcopy_log.h"

#define ZCOPY_NOTIF_BASE_NAME "NO"

#define ZCOPY_NotificationObject_list_end(_handle) (_handle)->notif_pi->num_slots

#define ZCOPY_NotificationObject_is_connected(handle_) ((handle_)->notif_pi != NULL)


/*** SOURCE_BEGIN ***/

const struct ZCOPY_NotificationObjectVersion ZCOPY_NotificationObject_gv_Version =
{
    {'N', 'O'},
    {
        1,
        0,
    },
};

RTI_PRIVATE void
ZCOPY_NotificationObject_list_append(
        struct ZCOPY_NotificationObjectHandle *handle,
        RTI_UINT32 slot_index)
{
    struct ZCOPY_NotificationSlot *slot;

    OSAPI_PRECONDITION(
            (handle == NULL) || (handle->notif_pi == NULL) ||
                    (slot_index >= handle->notif_pi->num_slots),
            return,
            OSAPI_Log_entry_add_pointer("handle", handle, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("notif_pi", handle->notif_pi, RTI_FALSE);
            OSAPI_Log_entry_add_uint("slot_index", slot_index, RTI_TRUE);)

    slot = &handle->notif_pi->slots[slot_index];

    /* If slot is in use and not currently in list, append it */
    if ((slot->in_use) && (!slot->flag))
    {
        /* If list is empty */
        if (handle->notif_pi->head == ZCOPY_NotificationObject_list_end(handle))
        {
            handle->notif_pi->head = slot_index;
            slot->prev = ZCOPY_NotificationObject_list_end(handle);
        }
        else
        {
            handle->notif_pi->slots[handle->notif_pi->tail].next = slot_index;
            slot->prev = handle->notif_pi->tail;
        }
        slot->next = ZCOPY_NotificationObject_list_end(handle);
        handle->notif_pi->tail = slot_index;
        slot->flag = RTI_TRUE;
    }
}

RTI_PRIVATE void
ZCOPY_NotificationObject_list_remove(
        struct ZCOPY_NotificationObjectHandle *handle,
        RTI_UINT32 slot_index)
{
    struct ZCOPY_NotificationSlot *slot;

    OSAPI_PRECONDITION(
            (handle == NULL) || (handle->notif_pi == NULL) ||
                    (slot_index >= handle->notif_pi->num_slots),
            return,
            OSAPI_Log_entry_add_pointer("handle", handle, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("notif_pi", handle->notif_pi, RTI_FALSE);
            OSAPI_Log_entry_add_uint("slot_index", slot_index, RTI_TRUE);)

    slot = &handle->notif_pi->slots[slot_index];

    /* If slot is currently in list, remove it */
    if (slot->flag)
    {
        /* If slot is session_tail */
        if (handle->notif_pi->session_tail == slot_index)
        {
            handle->notif_pi->session_tail = slot->prev;
        }
        /* If slot is head */
        if (handle->notif_pi->head == slot_index)
        {
            handle->notif_pi->head = slot->next;
        }
        else
        {
            handle->notif_pi->slots[slot->prev].next = slot->next;
        }
        /* If slot is tail */
        if (slot->next == ZCOPY_NotificationObject_list_end(handle))
        {
            handle->notif_pi->tail = slot->prev;
        }
        else
        {
            handle->notif_pi->slots[slot->next].prev = slot->prev;
        }
        slot->prev = ZCOPY_NotificationObject_list_end(handle);
        slot->next = ZCOPY_NotificationObject_list_end(handle);
        slot->flag = RTI_FALSE;
    }
}

RTI_BOOL
ZCOPY_NotificationObject_exists(const struct ZCOPY_Guid *guid, RTI_UINT32 port)
{
    char name[OSAPI_SHMEM_MAX_NAME_LENGTH + 1];

    if (!ZCOPY_Guid_to_string(
                guid,
                port,
                ZCOPY_NOTIF_BASE_NAME,
                name,
                (OSAPI_SHMEM_MAX_NAME_LENGTH + 1)))
    {
        return RTI_FALSE;
    }

    return OSAPI_SharedMemorySegment_exists(name);
}

struct ZCOPY_NotificationObjectHandle *
ZCOPY_NotificationObjectHandle_new(void)
{
    struct ZCOPY_NotificationObjectHandle *handle = NULL;

    OSAPI_Heap_allocate_struct(&handle, struct ZCOPY_NotificationObjectHandle);
    if (handle == NULL)
    {
        return NULL;
    }

    if (OSAPI_SharedMemory_is_robust_mutex_supported())
    {
        handle->shm_handle = OSAPI_SharedMemorySegmentHandle_new(OSAPI_SHMEM_MODE_ROBUST);
    }
    else
    {
        handle->shm_handle = OSAPI_SharedMemorySegmentHandle_new(OSAPI_SHMEM_MODE_LOCKABLE);
    }
    if (handle->shm_handle == NULL)
    {
#ifndef RTI_CERT
        OSAPI_Heap_free(handle);
#endif
        /* MICRO-5223: The memory for handle is leaked here only when compiling
         * with RTI_CERT because memory is intentionally never freed for cert.
         */
        /* coverity[leaked_storage] */
        return NULL;
    }

    handle->notif_pi = NULL;

    return handle;
}

#ifndef RTI_CERT
RTI_BOOL
ZCOPY_NotificationObjectHandle_delete(struct ZCOPY_NotificationObjectHandle *handle)
{
    OSAPI_PRECONDITION(handle == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("handle", handle, RTI_TRUE);)

    if (ZCOPY_NotificationObject_is_connected(handle))
    {
        if (!ZCOPY_NotificationObject_close(handle))
        {
            return RTI_FALSE;
        }
    }

    if (handle->shm_handle != NULL)
    {
        if (!OSAPI_SharedMemorySegmentHandle_delete(handle->shm_handle))
        {
            return RTI_FALSE;
        }
        handle->shm_handle = NULL;
    }

    OSAPI_Heap_free(handle);

    return RTI_TRUE;
}
#endif

RTI_BOOL
ZCOPY_NotificationObject_create(
        struct ZCOPY_NotificationObjectHandle *handle,
        const struct ZCOPY_Guid *guid,
        RTI_UINT32 port,
        RTI_UINT32 slots)
{
    char name[OSAPI_SHMEM_MAX_NAME_LENGTH + 1];
    RTI_BOOL result = RTI_FALSE;
    RTI_UINT32 i;
    RTI_SIZE_T segment_size;
    RTI_SIZE_T ZCOPY_NotificationObject_PI_size = sizeof(struct ZCOPY_NotificationObject_PI);
    RTI_SIZE_T ZCOPY_NotificationSlot_size = sizeof(struct ZCOPY_NotificationSlot);

    OSAPI_PRECONDITION((handle == NULL) || (guid == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("handle", handle, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("guid", guid, RTI_TRUE);)

    if (ZCOPY_NotificationObject_is_connected(handle))
    {
        ZCOPY_LOG_NOTIF_ALREADY_CONNECTED(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    segment_size = ZCOPY_NotificationObject_PI_size +
                   (slots * ZCOPY_NotificationSlot_size);
    if (!ZCOPY_Guid_to_string(guid, port, ZCOPY_NOTIF_BASE_NAME, name, sizeof(name)))
    {
        goto done;
    }
    if (!OSAPI_SharedMemorySegment_create(handle->shm_handle, name, segment_size))
    {
        goto done;
    }

    handle->notif_pi = OSAPI_SharedMemorySegment_get_address(handle->shm_handle);
    OSAPI_Memory_copy(
            &handle->notif_pi->version,
            &ZCOPY_NotificationObject_gv_Version,
            sizeof(struct ZCOPY_NotificationObjectVersion));
    handle->notif_pi->num_slots = slots;
    handle->notif_pi->head = slots;
    handle->notif_pi->tail = slots;
    handle->notif_pi->active_session = RTI_FALSE;
    handle->notif_pi->session_tail = slots;
    handle->slot_hint = NULL;
    handle->notif_pi->generation = 0;

    for (i = 0; i < handle->notif_pi->num_slots; ++i)
    {
        handle->notif_pi->slots[i].in_use = RTI_FALSE;
        handle->notif_pi->slots[i].prev = ZCOPY_NotificationObject_list_end(handle);
        handle->notif_pi->slots[i].next = ZCOPY_NotificationObject_list_end(handle);
    }

    /* This handle was created as lockable, which means we have been holding the
     *   lock since the segment was created. Need to unlock it now. */
    if (!OSAPI_SharedMemorySegment_unlock(handle->shm_handle))
    {
        goto done;
    }

    result = RTI_TRUE;
done:
    return result;
}

RTI_BOOL
ZCOPY_NotificationObject_destroy(struct ZCOPY_NotificationObjectHandle *handle)
{
    OSAPI_PRECONDITION(handle == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("handle", handle, RTI_TRUE);)

    if (!ZCOPY_NotificationObject_is_connected(handle))
    {
        ZCOPY_LOG_NOTIF_NOT_CONNECTED(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    handle->notif_pi = NULL;
    handle->slot_hint = NULL;

    if (!OSAPI_SharedMemorySegment_delete(handle->shm_handle))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
ZCOPY_NotificationObject_assign_slot(
        struct ZCOPY_NotificationObjectHandle *handle,
        const struct ZCOPY_Guid *guid)
{
    struct ZCOPY_NotificationSlot *slot = NULL;
    RTI_UINT32 i;

    OSAPI_PRECONDITION((handle == NULL) || (guid == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("handle", handle, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("guid", guid, RTI_TRUE);)

    if (!ZCOPY_NotificationObject_is_connected(handle))
    {
        ZCOPY_LOG_NOTIF_NOT_CONNECTED(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    /* Check if id is already assigned a slot */
    for (i = 0; i < handle->notif_pi->num_slots; ++i)
    {
        slot = &handle->notif_pi->slots[i];
        if (slot->in_use && ZCOPY_Guid_equals(guid, &slot->id))
        {
            ++slot->ref_count;
            return RTI_TRUE;
        }
    }

    /* Find the first free slot */
    for (i = 0; i < handle->notif_pi->num_slots; ++i)
    {
        slot = &handle->notif_pi->slots[i];
        if (!slot->in_use)
        {
            slot->id = *guid;
            slot->in_use = RTI_TRUE;
            slot->flag = RTI_FALSE;
            slot->next = ZCOPY_NotificationObject_list_end(handle);
            slot->prev = ZCOPY_NotificationObject_list_end(handle);
            slot->ref_count = 1;
            ++handle->notif_pi->generation;
            return RTI_TRUE;
        }
    }

    /* No slot found */
    ZCOPY_LOG_NOTIF_NO_FREE_SLOT(OSAPI_LOGKIND_ERROR)
    return RTI_FALSE;
}

extern RTI_BOOL
ZCOPY_NotificationObject_unassign_slot(
        struct ZCOPY_NotificationObjectHandle *handle,
        const struct ZCOPY_Guid *guid)
{
    struct ZCOPY_NotificationSlot *slot = NULL;
    RTI_UINT32 i;

    OSAPI_PRECONDITION((handle == NULL) || (guid == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("handle", handle, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("guid", guid, RTI_TRUE);)

    if (!ZCOPY_NotificationObject_is_connected(handle))
    {
        ZCOPY_LOG_NOTIF_NOT_CONNECTED(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    for (i = 0; i < handle->notif_pi->num_slots; ++i)
    {
        if ((handle->notif_pi->slots[i].in_use) &&
            ZCOPY_Guid_equals(&handle->notif_pi->slots[i].id, guid))
        {
            slot = &handle->notif_pi->slots[i];
            break;
        }
    }

    if (slot == NULL)
    {
        /* No slot with matching id found */
        return RTI_TRUE;
    }

    --slot->ref_count;
    if (slot->ref_count > 0)
    {
        return RTI_TRUE;
    }

    /* If slot is currently in the list, remove it */
    ZCOPY_NotificationObject_list_remove(handle, i);

    slot->in_use = RTI_FALSE;
    slot->ref_count = 0;
    ++handle->notif_pi->generation;

    return RTI_TRUE;
}

RTI_BOOL
ZCOPY_NotificationObject_next(
        struct ZCOPY_NotificationObjectHandle *handle,
        struct ZCOPY_Guid **guid_out)
{
    struct ZCOPY_NotificationSlot *slot = NULL;
    RTI_UINT32 slot_index;

    OSAPI_PRECONDITION((handle == NULL) || (guid_out == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("handle", handle, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("guid_out", guid_out, RTI_TRUE);)

    if (!ZCOPY_NotificationObject_is_connected(handle))
    {
        ZCOPY_LOG_NOTIF_NOT_CONNECTED(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    /* If list is empty, do nothing */
    if (handle->notif_pi->head == ZCOPY_NotificationObject_list_end(handle))
    {
        handle->notif_pi->active_session = RTI_FALSE;
        handle->notif_pi->session_tail = ZCOPY_NotificationObject_list_end(handle);
        *guid_out = NULL;
        handle->slot_hint = NULL;
        return RTI_TRUE;
    }

    if (handle->notif_pi->active_session)
    {
        if (handle->notif_pi->session_tail == ZCOPY_NotificationObject_list_end(handle))
        {
            /* There are no more notifications in this session */
            handle->notif_pi->active_session = RTI_FALSE;
            *guid_out = NULL;
            handle->slot_hint = NULL;
            return RTI_TRUE;
        }
        else if (handle->notif_pi->session_tail == handle->notif_pi->head)
        {
            /* Head is the last notification in this session */
            handle->notif_pi->session_tail = ZCOPY_NotificationObject_list_end(handle);
        }
    }
    else
    {
        /* Start a new session */
        handle->notif_pi->active_session = RTI_TRUE;
        if (handle->notif_pi->head != handle->notif_pi->tail)
        {
            /* Last notification in session will be the current tail of list */
            handle->notif_pi->session_tail = handle->notif_pi->tail;
        }
        else
        {
            /* The head will be the only notification in the session */
            handle->notif_pi->session_tail = ZCOPY_NotificationObject_list_end(handle);
        }
    }

    /* Pop first item from list */
    slot_index = handle->notif_pi->head;
    slot = &handle->notif_pi->slots[slot_index];
    ZCOPY_NotificationObject_list_remove(handle, slot_index);

    /* Return notification source */
    *guid_out = &slot->id;

    /* Save slot as hint */
    handle->slot_hint = slot;

    return RTI_TRUE;
}

RTI_PRIVATE RTI_BOOL
ZCOPY_NotificationObject_open_by_name(
    struct ZCOPY_NotificationObjectHandle *handle,
    const char *name,
    RTI_BOOL *is_connected_out)
{
    OSAPI_SHMEM_ATTACH_STATUS status;
    RTI_BOOL result = RTI_FALSE;
    RTI_BOOL is_connected;

    is_connected = ZCOPY_NotificationObject_is_connected(handle);

    if (!is_connected)
    {
        if (!OSAPI_SharedMemorySegment_attach(handle->shm_handle, name, &status))
        {
            goto done;
        }

        if (status == OSAPI_SHMEM_ATTACH_STATUS_OK)
        {
            struct ZCOPY_NotificationObject_PI *notif_pi =
                    OSAPI_SharedMemorySegment_get_address(handle->shm_handle);
            if (OSAPI_Memory_compare(
                        &notif_pi->version,
                        &ZCOPY_NotificationObject_gv_Version,
                        sizeof(struct ZCOPY_NotificationObjectVersion)) != 0)
            {
                RTI_BOOL retval;
                ZCOPY_LOG_NOTIF_INCOMPATIBLE_VERSION(OSAPI_LOGKIND_ERROR)
                /* The return value below is ignored as the operation has already failed
                * at this point, and we are attempting to clean up before exiting
                */
                retval = OSAPI_SharedMemorySegment_detach(handle->shm_handle);
                IGNORE_RETVAL(retval);
                goto done;
            }

            handle->notif_pi = notif_pi;
            handle->slot_hint = NULL;
            handle->generation = 0;
            is_connected = RTI_TRUE;
        }
        /* The only other known and allowed status is SEGMENT_NOT_FOUND,
         *   which is not considered an error because we can not know for
         *   sure whether owner of the NotificationObject has already
         *   completed its creation. Opening it can/will be re-tried later */
        else if (status != OSAPI_SHMEM_ATTACH_STATUS_SEGMENT_NOT_FOUND)
        {
            /* Any other status is considered an error though */
            goto done;
        }
    }

    if (is_connected_out != NULL)
    {
        *is_connected_out = is_connected;
    }
    result = RTI_TRUE;
done:
    return result;
}

RTI_BOOL
ZCOPY_NotificationObject_open(
        struct ZCOPY_NotificationObjectHandle *handle,
        const struct ZCOPY_Guid *guid,
        RTI_UINT32 port,
        RTI_BOOL *is_connected_out)
{
    RTI_BOOL result = RTI_FALSE;

    OSAPI_PRECONDITION((handle == NULL) || (guid == NULL),
                       goto done,
                       OSAPI_Log_entry_add_pointer("handle", handle, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("guid", guid, RTI_TRUE);)

    if (!ZCOPY_Guid_to_string(
                guid,
                port,
                ZCOPY_NOTIF_BASE_NAME,
                handle->name,
                sizeof(handle->name)))
    {
        goto done;
    }

    result = ZCOPY_NotificationObject_open_by_name(
        handle, handle->name, is_connected_out);
done:
    return result;
}

RTI_BOOL
ZCOPY_NotificationObject_open_retry(
        struct ZCOPY_NotificationObjectHandle *handle,
        RTI_BOOL *is_connected_out)
{
    OSAPI_PRECONDITION((handle == NULL) || (handle->name[0] == '\0'),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("handle", handle, RTI_FALSE);
                       OSAPI_Log_entry_add_string("handle->name", handle->name, RTI_TRUE);)

    return ZCOPY_NotificationObject_open_by_name(
        handle, handle->name, is_connected_out);
}

RTI_BOOL
ZCOPY_NotificationObject_close(struct ZCOPY_NotificationObjectHandle *handle)
{
    RTI_BOOL ok = RTI_FALSE;

    OSAPI_PRECONDITION(handle == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("handle", handle, RTI_TRUE);)

    if (!ZCOPY_NotificationObject_is_connected(handle))
    {
        ZCOPY_LOG_NOTIF_NOT_CONNECTED(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    ok = OSAPI_SharedMemorySegment_detach(handle->shm_handle);
    handle->notif_pi = NULL;
    handle->slot_hint = NULL;
    handle->generation = 0;

done:
    return ok;
}

RTI_BOOL
ZCOPY_NotificationObject_raise_flag(
        struct ZCOPY_NotificationObjectHandle *handle,
        const struct ZCOPY_Guid *guid)
{
    RTI_UINT32 i;
    const struct ZCOPY_NotificationSlot *slot = NULL;
    RTI_UINT32 slot_index;
    RTI_BOOL found = RTI_FALSE;

    OSAPI_PRECONDITION((handle == NULL) || (guid == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("handle", handle, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("guid", guid, RTI_TRUE);)

    if (!ZCOPY_NotificationObject_is_connected(handle))
    {
        ZCOPY_LOG_NOTIF_NOT_CONNECTED(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    /* Check if slot hint is valid */
    if (handle->slot_hint != NULL)
    {
        slot = handle->slot_hint;
        if ((slot->in_use) && (ZCOPY_Guid_equals(&slot->id, guid)))
        {
            found = RTI_TRUE;
            slot_index = (RTI_UINT32)(slot - handle->notif_pi->slots);
        }
    }

    /* Skip checking all slots if it was already checked during this generation */
    if ((!found) && (handle->generation < handle->notif_pi->generation))
    {
        for (i = 0; i < handle->notif_pi->num_slots; ++i)
        {
            slot = &handle->notif_pi->slots[i];
            if ((slot->in_use) && (ZCOPY_Guid_equals(&slot->id, guid)))
            {
                found = RTI_TRUE;
                handle->slot_hint = slot;
                slot_index = i;
                break;
            }
        }
        handle->generation = handle->notif_pi->generation;
    }

    if (!found)
    {
        /* Silently ignore request to raise flag for guid which is not assigned a slot */
        handle->slot_hint = NULL;
        return RTI_TRUE;
    }

    /* Add slot to notification list, if not in list */
    ZCOPY_NotificationObject_list_append(handle, slot_index);

    return RTI_TRUE;
}

RTI_PRIVATE void
ZCOPY_NotificationObject_rebuild_list(struct ZCOPY_NotificationObjectHandle *handle)
{
    RTI_UINT32 i;
    struct ZCOPY_NotificationSlot *slot = NULL;
    struct ZCOPY_NotificationSlot *prev_node = NULL;
    RTI_UINT32 prev_node_id;

    handle->notif_pi->head = ZCOPY_NotificationObject_list_end(handle);
    handle->notif_pi->tail = ZCOPY_NotificationObject_list_end(handle);

    for (i = 0; i < handle->notif_pi->num_slots; ++i)
    {
        slot = &handle->notif_pi->slots[i];
        slot->prev = ZCOPY_NotificationObject_list_end(handle);
        slot->next = ZCOPY_NotificationObject_list_end(handle);
        if (slot->in_use && slot->flag)
        {
            if (prev_node == NULL)
            {
                handle->notif_pi->head = i;
                handle->notif_pi->tail = i;
            }
            else
            {
                prev_node->next = i;
                slot->prev = prev_node_id;
                handle->notif_pi->tail = i;
            }

            prev_node = slot;
            prev_node_id = i;
        }
    }
}

RTI_BOOL
ZCOPY_NotificationObject_lock(struct ZCOPY_NotificationObjectHandle *handle)
{
    OSAPI_SHMEM_STATUS status;
    RTI_BOOL ok = RTI_FALSE;
    RTI_BOOL result;

    OSAPI_PRECONDITION(handle == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("handle", handle, RTI_TRUE);)

    if (!ZCOPY_NotificationObject_is_connected(handle))
    {
        ZCOPY_LOG_NOTIF_NOT_CONNECTED(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (!OSAPI_SharedMemorySegment_lock(handle->shm_handle, &status))
    {
        return RTI_FALSE;
    }
    if (status == OSAPI_SHMEM_STATUS_OK)
    {
        ok = RTI_TRUE;
        goto done;
    }
    else if (status != OSAPI_SHMEM_STATUS_OWNER_DEAD)
    {
        /* Unknown status returned
         * This check is purely defensive programming. OSAPI_SHMEM_STATUS is an
         * enum and should not contain an unknown value. However if it did,
         * acquiring the lock still succeeded we just do not understand the
         * status. Therefore, we release the lock and return RTI_FALSE.
         */
        goto done;
    }

    /* Previous owner died while holding lock, rebuild list */
    ZCOPY_NotificationObject_rebuild_list(handle);
    if (!OSAPI_SharedMemorySegment_mark_consistent(handle->shm_handle))
    {
        goto done;
    }
    ok = RTI_TRUE;

done:
    if (!ok)
    {
        result = OSAPI_SharedMemorySegment_unlock(handle->shm_handle);
        IGNORE_RETVAL(result);
    }
    return ok;
}

RTI_BOOL
ZCOPY_NotificationObject_unlock(struct ZCOPY_NotificationObjectHandle *handle)
{
    OSAPI_PRECONDITION(handle == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("handle", handle, RTI_TRUE);)

    if (!ZCOPY_NotificationObject_is_connected(handle))
    {
        ZCOPY_LOG_NOTIF_NOT_CONNECTED(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    return OSAPI_SharedMemorySegment_unlock(handle->shm_handle);
}
