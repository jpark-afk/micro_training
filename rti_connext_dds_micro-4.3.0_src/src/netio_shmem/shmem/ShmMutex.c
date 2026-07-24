/*
 * FILE: ShmMutex.c
 *
 * Copyright 2018-2024 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#include "osapi/osapi_config.h"

#include "osapi/osapi_log.h"
#include "osapi/osapi_string.h"
#include "ShmMutex.h"

/*** SOURCE_BEGIN ***/

RTIBool
NETIO_SharedMemoryMutex_create(
        struct NETIO_SharedMemoryMutexHandle *handle,
        RTI_INT32 *status_out,
        RTI_INT32 key)
{
    return NETIO_SharedMemorySemMutex_create(
            (struct NETIO_SharedMemorySemMutexHandle*)handle,
            status_out,
            key,
            SEMMUTEX_TYPE_MUTEX);
}

RTIBool
NETIO_SharedMemoryMutex_attach(
        struct NETIO_SharedMemoryMutexHandle *handle,
        RTI_INT32 *status_out,
        RTI_INT32 key)
{
    return NETIO_SharedMemorySemMutex_attach(
            (struct NETIO_SharedMemorySemMutexHandle*)handle,
            status_out,
            key,
            SEMMUTEX_TYPE_MUTEX);
}

RTIBool
NETIO_SharedMemoryMutex_create_or_attach(
        struct NETIO_SharedMemoryMutexHandle *handle,
        RTI_INT32 *status_out,
        RTI_INT32 key)
{
    return NETIO_SharedMemorySemMutex_create_or_attach(
            (struct NETIO_SharedMemorySemMutexHandle*)handle,
            status_out,
            key,
            SEMMUTEX_TYPE_MUTEX);
}

RTIBool
NETIO_SharedMemoryMutex_unlock(
        struct NETIO_SharedMemoryMutexHandle *handle,
        RTI_INT32 *status_out)
{
    return NETIO_SharedMemorySemMutex_give(
            (struct NETIO_SharedMemorySemMutexHandle*)handle,
            status_out,
            SEMMUTEX_TYPE_MUTEX);
}

RTIBool
NETIO_SharedMemoryMutex_lock(
        struct NETIO_SharedMemoryMutexHandle *handle,
        RTI_INT32 *status_out)
{
    return NETIO_SharedMemorySemMutex_take(
            (struct NETIO_SharedMemorySemMutexHandle*)handle,
            status_out,
            SEMMUTEX_TYPE_MUTEX);
}

RTIBool
NETIO_SharedMemoryMutex_detach(struct NETIO_SharedMemoryMutexHandle *handle)
{
    return NETIO_SharedMemorySemMutex_detach(
            (struct NETIO_SharedMemorySemMutexHandle*)handle,
            SEMMUTEX_TYPE_MUTEX);
}

RTIBool
NETIO_SharedMemoryMutex_delete(struct NETIO_SharedMemoryMutexHandle *handle)
{
    return NETIO_SharedMemorySemMutex_delete(
            (struct NETIO_SharedMemorySemMutexHandle*)handle,
            SEMMUTEX_TYPE_MUTEX);
}
