/*
 * FILE: DefaultNotifMechanism.c - Default Notification mechanism implementation
 *
 * Copyright 2023-2024 Real-Time Innovations, Inc.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*ci
 * \file
 * \brief POSIX Notification mechanism implementation
 *
 * \addtogroup ZCOPY_NotifMechanismClass
 * @{
 */
#include "DefaultNotifMechanism.h"

#include "netio_zcopy/netio_zcopy_log.h"
#include "osapi/osapi_string.h"

RTI_PRIVATE RTI_UINT32 ZCOPY_NotifMechanism_fv_instance_count = 0;

RTI_PRIVATE struct ZCOPY_NotifMechanismProperty
    ZCOPY_NotifMechanism_fv_default_property = ZCOPY_NotifMechanismProperty_INITIALIZER;

/*** SOURCE_BEGIN ***/

RTI_SIZE_T
ZCOPY_NotifMechanism_addr_uint_to_string(char *buffer, RTI_SIZE_T max_length, RTI_UINT32 u)
{
    /* Enough space for a 32 bit unsigned value without NULL terminator */
    char converted[10];
    char *bptr;
    RTI_UINT32 r;
    const char digit[] = "0123456789";
    char *result;
    RTI_SIZE_T rlen;

    bptr = converted;

    /* Convert number to string in reverse order */
    do
    {
        r = u % 10;
        u = u / 10;
        *bptr = digit[r];
        bptr++;
        *bptr = 0;
    } while (u != 0);

    rlen = (RTI_SIZE_T)(bptr - converted);
    if (rlen + 1 > max_length)
    {
        /* Converted value is too large for buffer */
        return max_length;
    }

    /* Copy converted string to buffer in reverse order */
    bptr = converted + rlen - 1;
    result = buffer;
    while (bptr >= converted)
    {
        *(result++) = *(bptr--);
    }

    *result = '\0';

    return rlen;
}

RTI_BOOL
ZCOPY_NotifMechanism_addr_to_string(
        RTI_UINT32 addr,
        RTI_UINT32 port,
        char *buf,
        RTI_UINT32 buf_len)
{
    RTI_SIZE_T prefix_len;
    RTI_SIZE_T remaining_len = buf_len;
    RTI_SIZE_T written;

    /* Copy prefix into buffer */
    prefix_len = OSAPI_String_length(ZCOPY_NOTIF_MECH_BASE_NAME);
    if ((prefix_len + 1) > buf_len)
    {
        return RTI_FALSE;
    }
    OSAPI_Memory_copy(buf, ZCOPY_NOTIF_MECH_BASE_NAME, prefix_len);
    remaining_len -= prefix_len;
    buf += prefix_len;

    /* Write . */
    if (remaining_len < 2)
    {
        return RTI_FALSE;
    }
    buf[0] = '.';
    remaining_len--;
    buf++;

    /* Write address */
    written = ZCOPY_NotifMechanism_addr_uint_to_string(buf, remaining_len, addr);
    if (written == remaining_len)
    {
        return RTI_FALSE;
    }
    remaining_len -= written;
    buf += written;

    /* Write _ */
    if (remaining_len < 2)
    {
        return RTI_FALSE;
    }
    buf[0] = '_';
    remaining_len--;
    buf++;

    /* Write port */
    written = ZCOPY_NotifMechanism_addr_uint_to_string(buf, remaining_len, port);
    if (written == remaining_len)
    {
        return RTI_FALSE;
    }
    buf += written;

    /* Null terminate */
    buf[0] = '\0';

    return RTI_TRUE;
}

void
NETIO_Address_set_from_notif_address(
        struct NETIO_Address *addr,
        RTI_UINT32 notif_intf_addr,
        RTI_UINT32 port)
{
    NETIO_Address_init(addr, NETIO_ADDRESS_KIND_NOTIF);
    addr->value.ipv4.address = NETIO_htonl(notif_intf_addr);
    addr->port = port;
}

RTI_PRIVATE RTI_UINT32
NETIO_Address_get_notif_address(const struct NETIO_Address *addr)
{
    return NETIO_ntohl(addr->value.ipv4.address);
}



MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
ZCOPY_NotifMechanism_wakeup_receive_thread(struct OSAPI_ThreadInfo *thread_info)
{
    RTI_BOOL ok = RTI_FALSE;
    ZCOPY_NotifMechanismPortEntry *port_entry =
            (ZCOPY_NotifMechanismPortEntry *)thread_info->user_data;
    OSAPI_SHMEM_STATUS status;

    OSAPI_PRECONDITION(thread_info == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("thread_info", thread_info, RTI_TRUE);)

    if (port_entry->handle.monitor == NULL)
    {
        return RTI_FALSE;
    }

    if (!OSAPI_SharedMemoryMonitor_acquire(port_entry->handle.monitor, &status))
    {
        return RTI_FALSE;
    }
    if (status == OSAPI_SHMEM_STATUS_OWNER_DEAD)
    {
        if (!OSAPI_SharedMemoryMonitor_mark_consistent(port_entry->handle.monitor))
        {
            goto done;
        }
    }
    else if (status != OSAPI_SHMEM_STATUS_OK)
    {
        /* Defensive programming. Cannot happen unless enum is an invalid value. */
        goto done;
    }
    /* Raise flag to wake-up receive thread on signal */
    port_entry->handle.notif_pi->notified = RTI_TRUE;
    
    if (!OSAPI_SharedMemoryMonitor_signal(port_entry->handle.monitor))
    {
        goto done;
    }

    ok = RTI_TRUE;

done:
    if (!OSAPI_SharedMemoryMonitor_release(port_entry->handle.monitor))
    {
        return RTI_FALSE;
    }

    return ok;
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
ZCOPY_NotifMechanism_receive_thread(struct OSAPI_ThreadInfo *thread_info)
{
    ZCOPY_NotifMechanismPortEntry *port_entry =
            (ZCOPY_NotifMechanismPortEntry *)thread_info->user_data;
    OSAPI_SHMEM_STATUS status;
    RTI_BOOL has_more_data;

    while (!thread_info->stop_thread)
    {
        if (!OSAPI_SharedMemoryMonitor_acquire(port_entry->handle.monitor, &status))
        {
            return RTI_FALSE;
        }

        if (status == OSAPI_SHMEM_STATUS_OWNER_DEAD)
        {
            /* Set notified flag and mark as consistent */
            port_entry->handle.notif_pi->notified = RTI_TRUE;
            if (!OSAPI_SharedMemoryMonitor_mark_consistent(port_entry->handle.monitor))
            {
                return RTI_FALSE;
            }
        }
        else if (status != OSAPI_SHMEM_STATUS_OK)
        {
            /* Defensive programming. Cannot happen unless enum is an invalid value. */
            return RTI_FALSE;
        }

        while (!port_entry->handle.notif_pi->notified)
        {
            if (!OSAPI_SharedMemoryMonitor_wait(port_entry->handle.monitor, &status))
            {
                OSAPI_SharedMemoryMonitor_release(port_entry->handle.monitor);
                return RTI_FALSE;
            }

            if (status == OSAPI_SHMEM_STATUS_OWNER_DEAD)
            {
                /* Set notified flag and mark as consistent */
                port_entry->handle.notif_pi->notified = RTI_TRUE;
                if (!OSAPI_SharedMemoryMonitor_mark_consistent(
                            port_entry->handle.monitor))
                {
                    OSAPI_SharedMemoryMonitor_release(port_entry->handle.monitor);
                    return RTI_FALSE;
                }
            }
            else if (status != OSAPI_SHMEM_STATUS_OK)
            {
                /* Defensive programming. Cannot happen unless enum is an invalid value. */
                OSAPI_SharedMemoryMonitor_release(port_entry->handle.monitor);
                return RTI_FALSE;
            }
        }

        /* We have woken up from a notification */
        port_entry->handle.notif_pi->notified = RTI_FALSE;
        if (!OSAPI_SharedMemoryMonitor_release(port_entry->handle.monitor))
        {
            return RTI_FALSE;
        }

        if (thread_info->stop_thread)
        {
            break;
        }

        has_more_data = RTI_FALSE;
        do
        {
            if (!ZCOPY_NotifInterface_receive(
                        port_entry->upstream,
                        &port_entry->port,
                        &has_more_data))
            {
                return RTI_FALSE;
            }
            if (has_more_data)
            {
                OSAPI_Thread_nanosleep(ZCOPY_RECEIVE_THREAD_RATE);
                
            }
        } while (has_more_data);
    }

    return RTI_TRUE;
}

RTI_PRIVATE RTI_BOOL
ZCOPY_NotifMechanismHandle_initialize(void *initialize_param, void *buffer)
{
    struct ZCOPY_NotifMechanismHandle *self = (struct ZCOPY_NotifMechanismHandle *)buffer;

    UNUSED_ARG(initialize_param);

    self->shm_handle = OSAPI_SharedMemorySegmentHandle_new(OSAPI_SHMEM_MODE_WRITE);
    if (self->shm_handle == NULL)
    {
        return RTI_FALSE;
    }

    self->notif_pi = NULL;
    self->monitor = NULL;

    return RTI_TRUE;
}

#ifndef RTI_CERT
RTI_PRIVATE RTI_BOOL
ZCOPY_NotifMechanismHandle_finalize(void *finalize_param, void *buffer)
{
    struct ZCOPY_NotifMechanismHandle *self = (struct ZCOPY_NotifMechanismHandle *)buffer;

    UNUSED_ARG(finalize_param);

    self->monitor = NULL;
    self->notif_pi = NULL;

    if (self->shm_handle != NULL)
    {
        if (!OSAPI_SharedMemorySegmentHandle_delete(self->shm_handle))
        {
            return RTI_FALSE;
        }
    }

    return RTI_TRUE;
}

RTI_PRIVATE void
ZCOPY_NotifMechanism_delete_instance(NETIO_Interface_T *user_intf)
{
    struct ZCOPY_NotifMechanism *self = (struct ZCOPY_NotifMechanism *)user_intf;

    OSAPI_PRECONDITION(user_intf == NULL,
                       return,
                       OSAPI_Log_entry_add_pointer("user_intf", user_intf, RTI_TRUE);)

    self->upstream = NULL;

    if (self->route_pool != NULL)
    {
        if (!REDA_BufferPool_delete(self->route_pool))
        {
            return;
        }
        self->route_pool = NULL;
    }

    if (self->port_pool != NULL)
    {
        if (!REDA_BufferPool_delete(self->port_pool))
        {
            return;
        }
        self->port_pool = NULL;
    }

    OSAPI_Heap_free_struct(self);

    --ZCOPY_NotifMechanism_fv_instance_count;
}
#endif

RTI_PRIVATE NETIO_Interface_T *
ZCOPY_NotifMechanism_create_instance(NETIO_Interface_T *upstream, void *user_property)
{
    RTI_BOOL ok = RTI_FALSE;
    struct ZCOPY_NotifMechanism *self = NULL;
    struct ZCOPY_NotifMechanismProperty *prop =
            (struct ZCOPY_NotifMechanismProperty *)user_property;
    struct REDA_BufferPoolProperty pool_property = REDA_BufferPoolProperty_INITIALIZER;

    OSAPI_PRECONDITION(
            (upstream == NULL) || (user_property == NULL),
            return NULL,
            OSAPI_Log_entry_add_pointer("upstream", upstream, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("user_property", user_property, RTI_TRUE);)

    OSAPI_Heap_allocate_struct(&self, struct ZCOPY_NotifMechanism);
    if (self == NULL)
    {
        return NULL;
    }

    self->upstream = upstream;
    self->property = *prop;

    /* Create pool of port entries */
    pool_property.buffer_size = sizeof(ZCOPY_NotifMechanismPortEntry);
    pool_property.max_buffers = prop->max_receive_ports;
    pool_property.flags = 0;
    self->port_pool = REDA_BufferPool_new(
            "ports",
            &pool_property,
            ZCOPY_NotifMechanismHandle_initialize,
            NULL,
#ifndef RTI_CERT
            ZCOPY_NotifMechanismHandle_finalize,
#else
            NULL,
#endif
            NULL);
    if (self->port_pool == NULL)
    {
        goto done;
    }

    /* Create pool of route entries */
    pool_property.buffer_size = sizeof(ZCOPY_NotifMechanismRouteEntry);
    pool_property.max_buffers = prop->max_routes;
    pool_property.flags = 0;
    self->route_pool = REDA_BufferPool_new(
            "routes",
            &pool_property,
            ZCOPY_NotifMechanismHandle_initialize,
            NULL,
#ifndef RTI_CERT
            ZCOPY_NotifMechanismHandle_finalize,
#else
            NULL,
#endif
            NULL);
    if (self->route_pool == NULL)
    {
        goto done;
    }

    ++ZCOPY_NotifMechanism_fv_instance_count;
    ok = RTI_TRUE;

done:
    if (!ok)
    {
#ifndef RTI_CERT
        ZCOPY_NotifMechanism_delete_instance(&self->_parent);
#endif
        /* The memory for self is leaked here only when compiling with RTI_CERT
         * because memory is intentionally never freed for cert.
         */
        /* coverity[leaked_storage] */
        return NULL;
    }

    return &self->_parent;
}

RTI_PRIVATE RTI_BOOL
ZCOPY_NotifMechanism_resolve_address(
        NETIO_Interface_T *netio_intf,
        const char *address_string,
        struct NETIO_Address *address_value,
        RTI_BOOL *is_invalid)
{
    struct ZCOPY_NotifMechanism *self = (struct ZCOPY_NotifMechanism *)netio_intf;
    RTI_BOOL ok = RTI_FALSE;

    OSAPI_PRECONDITION(
            (netio_intf == NULL) || (address_string == NULL) || (address_value == NULL) ||
                    (is_invalid == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("netio_intf", netio_intf, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("address_string", address_string, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("address_value", address_value, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("is_invalid", is_invalid, RTI_TRUE);)

    /* Assume that the address is always valid to allow other transports
     * to resolve this address if we do not understand it
     */
    *is_invalid = RTI_FALSE;

    /* Only accept an empty string */
    if (address_string[0] != 0)
    {
        goto done;
    }

    NETIO_Address_set_from_notif_address(address_value, self->property.intf_addr, 0);

    ok = RTI_TRUE;

done:
    return ok;
}

RTI_PRIVATE RTI_BOOL
ZCOPY_NotifMechanism_get_route_table(
        NETIO_Interface_T *netio_intf,
        struct NETIO_AddressSeq *address,
        struct NETIO_NetmaskSeq *netmask)
{
    RTI_BOOL ok = RTI_FALSE;
    RTI_INT32 cur_addr_len;
    struct NETIO_Address src_address = NETIO_Address_INITIALIZER;
    struct NETIO_Netmask *a_netmask;

    UNUSED_ARG(netio_intf);

    /* Try to append an element to address sequence */
    cur_addr_len = NETIO_AddressSeq_get_length(address);
    if (!NETIO_AddressSeq_set_length(address, cur_addr_len + 1))
    {
        /* This is okay because it just means that there is no more space */
        ok = RTI_TRUE;
        goto done;
    }

    NETIO_Address_init(&src_address, NETIO_ADDRESS_KIND_NOTIF);

    /* Since set_length succeeded for it is assumed that
     * get_reference returns a valid address.
     */
    /* coverity[dereference] */
    /* coverity[cert_exp34_c_violation] */
    *NETIO_AddressSeq_get_reference(address, cur_addr_len) = src_address;

    if (!NETIO_NetmaskSeq_set_length(netmask, cur_addr_len + 1))
    {
        /* The return value is not checked here because the function has already
         * failed, and we are attempting to clean up. There is nothing else to do
         * if set_length fails.
         */
        /* coverity[check_return] */
        NETIO_AddressSeq_set_length(address, cur_addr_len);
        goto done;
    }

    /* Since set_length succeeded for it is assumed that
     * get_reference returns a valid address.
     */
    a_netmask = NETIO_NetmaskSeq_get_reference(netmask, cur_addr_len);

    /* Any notification address is possible, hence 0 bits netmask.
     * Reachability of a specific address is tested later.
     */
    /* coverity[dereference] */
    /* coverity[cert_exp34_c_violation] */
    a_netmask->bits = 0;

    ok = RTI_TRUE;

done:
    return ok;
}

RTI_PRIVATE RTI_BOOL
ZCOPY_NotifMechanism_reserve_address(
        NETIO_Interface_T *user_intf,
        struct NETIO_Address *src_addr,
        void **port_entry_out)
{
    RTI_BOOL ok = RTI_FALSE;
    struct ZCOPY_NotifMechanism *self = (struct ZCOPY_NotifMechanism *)user_intf;
    ZCOPY_NotifMechanismPortEntry *port_entry = NULL;
    struct ZCOPY_NotifMechanismHandle *handle = NULL;
    RTI_BOOL segment_created = RTI_FALSE;
    RTI_BOOL rtn;
    char name[OSAPI_SHMEM_MAX_NAME_LENGTH + 1];
    RTI_UINT32 monitor_offset;
    RTI_UINT32 segment_size;
    const char *tname = "notif_rx_uc";
    OSAPI_SHMEM_STATUS status;

    if (NETIO_Address_get_notif_address(src_addr) != self->property.intf_addr)
    {
        goto done;
    }

    port_entry = REDA_BufferPool_get_buffer(self->port_pool);
    if (port_entry == NULL)
    {
        goto done;
    }
    handle = &port_entry->handle;

    port_entry->port = *src_addr;

    if (!ZCOPY_NotifMechanism_addr_to_string(
                self->property.intf_addr,
                src_addr->port,
                name,
                (OSAPI_SHMEM_MAX_NAME_LENGTH + 1)))
    {
        goto done;
    }

    monitor_offset = (RTI_SIZE_T)(OSAPI_SHARED_MEMORY_ALIGN(
            sizeof(struct ZCOPY_NotifMechanism_PI),
            OSAPI_SHARED_MEMORY_ALIGNMENT));
    segment_size = monitor_offset + OSAPI_SharedMemoryMonitor_get_size();
    if (!OSAPI_SharedMemorySegment_create(
                handle->shm_handle,
                name,
                segment_size))
    {
        goto done;
    }
    segment_created = RTI_TRUE;

    handle->notif_pi =
            (struct ZCOPY_NotifMechanism_PI *)OSAPI_SharedMemorySegment_get_address(
                    handle->shm_handle);
    handle->notif_pi->monitor_offset = monitor_offset;
    handle->monitor = (struct OSAPI_SharedMemoryMonitor
                               *)((char *)handle->notif_pi + monitor_offset);

    /* We'll lock before setting notified to prevent a missing_lock warning */
    if (!OSAPI_SharedMemoryMonitor_acquire(handle->monitor, &status))
    {
        return RTI_FALSE;
    }

    if (status != OSAPI_SHMEM_STATUS_OK)
    {
        return RTI_FALSE;
    }

    handle->notif_pi->notified = RTI_FALSE;

    if (!OSAPI_SharedMemoryMonitor_release(handle->monitor))
    {
        return RTI_FALSE;
    }

    if (!OSAPI_SharedMemoryMonitor_initialize(
                segment_size - monitor_offset,
                (char *)handle->notif_pi + monitor_offset,
                &handle->monitor))
    {
        goto done;
    }

    port_entry->rx_thread = OSAPI_Thread_create(
            tname,
            &self->property.thread_prop,
            ZCOPY_NotifMechanism_receive_thread,
            (void *)port_entry,
            ZCOPY_NotifMechanism_wakeup_receive_thread);
    if (port_entry->rx_thread == NULL)
    {
        goto done;
    }

    port_entry->upstream = self->upstream;
    port_entry->ref_count = 0;

    *port_entry_out = port_entry;
    ok = RTI_TRUE;

done:
    if ((!ok) && (port_entry != NULL))
    {
        if (segment_created)
        {
            rtn = OSAPI_SharedMemorySegment_delete(handle->shm_handle);
            /* call has failed anyway */
            UNUSED_ARG(rtn);
        }
        REDA_BufferPool_return_buffer(self->port_pool, port_entry);
    }
    return ok;
}

RTI_PRIVATE RTI_BOOL
ZCOPY_NotifMechanism_release_address(
        NETIO_Interface_T *user_intf,
        struct NETIO_Address *src_addr,
        void *port_entry_in)
{
    struct ZCOPY_NotifMechanism *self = (struct ZCOPY_NotifMechanism *)user_intf;
    ZCOPY_NotifMechanismPortEntry *port_entry =
            (ZCOPY_NotifMechanismPortEntry *)port_entry_in;

    UNUSED_ARG(user_intf);
    UNUSED_ARG(src_addr);

    port_entry->upstream = NULL;

    if (!OSAPI_Thread_destroy(port_entry->rx_thread))
    {
        return RTI_FALSE;
    }

    if (!OSAPI_SharedMemorySegment_delete(port_entry->handle.shm_handle))
    {
        return RTI_FALSE;
    }

    REDA_BufferPool_return_buffer(self->port_pool, port_entry);

    return RTI_TRUE;
}

RTI_PRIVATE RTI_BOOL
ZCOPY_NotifMechanism_add_route(
        NETIO_Interface_T *user_intf,
        struct NETIO_Address *source,
        struct NETIO_Address *destination,
        void **route_entry_out)
{
    RTI_BOOL ok = RTI_FALSE;
    struct ZCOPY_NotifMechanism *self = (struct ZCOPY_NotifMechanism *)user_intf;
    ZCOPY_NotifMechanismRouteEntry *route_entry = NULL;
    char name[OSAPI_SHMEM_MAX_NAME_LENGTH + 1];
    OSAPI_SHMEM_ATTACH_STATUS shmem_status;

    UNUSED_ARG(source);

    route_entry = REDA_BufferPool_get_buffer(self->route_pool);
    if (route_entry == NULL)
    {
        goto done;
    }

    if (!ZCOPY_NotifMechanism_addr_to_string(
                NETIO_Address_get_notif_address(destination),
                destination->port,
                name,
                (OSAPI_SHMEM_MAX_NAME_LENGTH + 1)))
    {
        goto done;
    }

    if (!OSAPI_SharedMemorySegment_attach(route_entry->shm_handle, name, &shmem_status) ||
        (shmem_status != OSAPI_SHMEM_ATTACH_STATUS_OK))
    {
        /* Failing to attach to the remote's shared memory segment is considered
         *   an error in all cases, because it is expected to be present and
         *   valid. */
        goto done;
    }

    route_entry->notif_pi =
            (struct ZCOPY_NotifMechanism_PI *)OSAPI_SharedMemorySegment_get_address(
                    route_entry->shm_handle);
    route_entry->monitor =
            (struct OSAPI_SharedMemoryMonitor
                     *)((char *)route_entry->notif_pi + route_entry->notif_pi->monitor_offset);

    *route_entry_out = route_entry;
    ok = RTI_TRUE;

done:
    if ((!ok) && (route_entry != NULL))
    {
        REDA_BufferPool_return_buffer(self->route_pool, route_entry);
    }
    return ok;
}

RTI_PRIVATE RTI_BOOL
ZCOPY_NotifMechanism_delete_route(
        NETIO_Interface_T *user_intf,
        struct NETIO_Address *source,
        struct NETIO_Address *destination,
        void *route_entry_in)
{
    RTI_BOOL ok = RTI_FALSE;
    struct ZCOPY_NotifMechanism *self = (struct ZCOPY_NotifMechanism *)user_intf;
    ZCOPY_NotifMechanismRouteEntry *route_entry =
            (ZCOPY_NotifMechanismRouteEntry *)route_entry_in;

    UNUSED_ARG(source);
    UNUSED_ARG(destination);

    if (!OSAPI_SharedMemorySegment_detach(route_entry->shm_handle))
    {
        goto done;
    }

    REDA_BufferPool_return_buffer(self->route_pool, route_entry);

    ok = RTI_TRUE;

done:
    return ok;
}

RTI_PRIVATE RTI_BOOL
ZCOPY_NotifMechanism_bind(
        NETIO_Interface_T *user_intf,
        struct NETIO_Address *src_addr, /* Remote NOTIF address */
        struct NETIO_Address *dst_addr, /* Local NOTIF address */
        void *port_entry_in,
        void **bind_entry_out)
{
    ZCOPY_NotifMechanismPortEntry *port_entry =
            (ZCOPY_NotifMechanismPortEntry *)port_entry_in;

    UNUSED_ARG(user_intf);
    UNUSED_ARG(src_addr);
    UNUSED_ARG(dst_addr);

    *bind_entry_out = NULL;

    if (port_entry->ref_count++ == 0)
    {
        /* Start receive thread on first bind to the port */
        if (!OSAPI_Thread_start(port_entry->rx_thread))
        {
            return RTI_FALSE;
        }
    }

    return RTI_TRUE;
}

RTI_PRIVATE RTI_BOOL
ZCOPY_NotifMechanism_unbind(
        NETIO_Interface_T *user_intf,
        struct NETIO_Address *src_addr, /* Remote NOTIF address */
        struct NETIO_Address *dst_addr, /* Local NOTIF address */
        void *port_entry_in,
        void *bind_entry_in)
{
    UNUSED_ARG(user_intf);
    UNUSED_ARG(src_addr);
    UNUSED_ARG(dst_addr);
    UNUSED_ARG(port_entry_in);
    UNUSED_ARG(bind_entry_in);

    return RTI_TRUE;
}

RTI_PRIVATE RTI_BOOL
ZCOPY_NotifMechanism_send(
        NETIO_Interface_T *user_intf,
        struct NETIO_Address *source,
        struct NETIO_Address *destination,
        void *route_entry_in)
{
    RTI_BOOL ok = RTI_FALSE;
    ZCOPY_NotifMechanismRouteEntry *route_entry =
            (ZCOPY_NotifMechanismRouteEntry *)route_entry_in;
    OSAPI_SHMEM_STATUS status;

    UNUSED_ARG(user_intf);
    UNUSED_ARG(source);
    UNUSED_ARG(destination);
  
    if (!OSAPI_SharedMemoryMonitor_acquire(route_entry->monitor, &status))
    {
        return RTI_FALSE;
    }
    if (status == OSAPI_SHMEM_STATUS_OWNER_DEAD)
    {
        if (!OSAPI_SharedMemoryMonitor_mark_consistent(route_entry->monitor))
        {
            goto done;
        }
    }
    else if (status != OSAPI_SHMEM_STATUS_OK)
    {
        /* Defensive programming. Cannot happen unless enum is an invalid value. */
        goto done;
    }

    route_entry->notif_pi->notified = RTI_TRUE;
    if (!OSAPI_SharedMemoryMonitor_signal(route_entry->monitor))
    {
        goto done;
    }

    ok = RTI_TRUE;

done:
    if (!OSAPI_SharedMemoryMonitor_release(route_entry->monitor))
    {
        return RTI_FALSE;
    }

    return ok;
}

RTI_PRIVATE RTI_BOOL
ZCOPY_NotifMechanism_wakeup(NETIO_Interface_T *user_intf, void *port_entry_in)
{
    ZCOPY_NotifMechanismPortEntry *port_entry =
            (ZCOPY_NotifMechanismPortEntry *)port_entry_in;
    UNUSED_ARG(user_intf);

    return OSAPI_Thread_wakeup(port_entry->rx_thread);
}

RTI_PRIVATE struct ZCOPY_NotifUserInterfaceI ZCOPY_NotifMechanism_fv_intf =
{
    ZCOPY_NotifMechanism_create_instance,
#ifndef RTI_CERT
    ZCOPY_NotifMechanism_delete_instance,
#endif
    ZCOPY_NotifMechanism_resolve_address,
    ZCOPY_NotifMechanism_get_route_table,
    ZCOPY_NotifMechanism_reserve_address,
    ZCOPY_NotifMechanism_release_address,
    ZCOPY_NotifMechanism_add_route,
    ZCOPY_NotifMechanism_delete_route,
    ZCOPY_NotifMechanism_bind,
    ZCOPY_NotifMechanism_unbind,
    ZCOPY_NotifMechanism_send,
    ZCOPY_NotifMechanism_wakeup,
};

RTI_BOOL
ZCOPY_NotifMechanism_register(
        RT_Registry_T *registry,
        const char *name,
        struct ZCOPY_NotifInterfaceFactoryProperty *property)
{
    property->user_intf = &ZCOPY_NotifMechanism_fv_intf;
    if (!ZCOPY_NotifInterfaceFactory_register(registry, name, property))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
ZCOPY_NotifMechanism_unregister(
        RT_Registry_T *registry,
        const char *name)
{
    if (!ZCOPY_NotifInterfaceFactory_unregister(registry, name))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

void*
ZCOPY_NotifUserInterface_get_property(void)
{
    return &ZCOPY_NotifMechanism_fv_default_property;
}

struct ZCOPY_NotifUserInterfaceI*
ZCOPY_NotifUserInterface_get_interface(void)
{
    return &ZCOPY_NotifMechanism_fv_intf;
}

/*ci @} */
