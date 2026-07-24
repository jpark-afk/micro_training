/*
 * FILE: ShmSemaphore.c
 *
 * Copyright 2018-2024 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#include "netio_shmem/netio_shmem_semaphore.h"
#include "ShmSemMutex.h"

/*** SOURCE_BEGIN ***/

RTIBool
NETIO_SharedMemorySignalingSemaphore_create(
        struct NETIO_SharedMemorySignalingSemaphoreHandle *handle,
        RTI_INT32 *status_out,
        RTI_INT32 semaphore_key)
{
    return NETIO_SharedMemorySemMutex_create(
            (struct NETIO_SharedMemorySemMutexHandle*)handle,
            status_out,
            semaphore_key,
            SEMMUTEX_TYPE_BINARYSEM);
}

RTIBool
NETIO_SharedMemorySignalingSemaphore_attach(
        struct NETIO_SharedMemorySignalingSemaphoreHandle *handle,
        RTI_INT32 *status_out,
        RTI_INT32 semaphore_key)
{
    return NETIO_SharedMemorySemMutex_attach(
            (struct NETIO_SharedMemorySemMutexHandle*)handle,
            status_out,
            semaphore_key,
            SEMMUTEX_TYPE_BINARYSEM);
}

RTIBool
NETIO_SharedMemorySignalingSemaphore_create_or_attach(
        struct NETIO_SharedMemorySignalingSemaphoreHandle *handle,
        RTI_INT32 *status_out,
        RTI_INT32 semaphore_key)
{
    return NETIO_SharedMemorySemMutex_create_or_attach(
            (struct NETIO_SharedMemorySemMutexHandle*)handle,
            status_out,
            semaphore_key,
            SEMMUTEX_TYPE_BINARYSEM);
}

RTIBool
NETIO_SharedMemorySignalingSemaphore_wait(
        struct NETIO_SharedMemorySignalingSemaphoreHandle *handle,
        RTI_INT32 *status_out)
{
    return NETIO_SharedMemorySemMutex_take(
            (struct NETIO_SharedMemorySemMutexHandle*)handle,
            status_out,
            SEMMUTEX_TYPE_BINARYSEM);
}

RTIBool
NETIO_SharedMemorySignalingSemaphore_signal(
        struct NETIO_SharedMemorySignalingSemaphoreHandle *handle,
        RTI_INT32 *status_out)
{
    return NETIO_SharedMemorySemMutex_give(
            (struct NETIO_SharedMemorySemMutexHandle*)handle,
            status_out,
            SEMMUTEX_TYPE_BINARYSEM);
}

RTIBool
NETIO_SharedMemorySignalingSemaphore_detach(
        struct NETIO_SharedMemorySignalingSemaphoreHandle *handle)
{
    return NETIO_SharedMemorySemMutex_detach(
            (struct NETIO_SharedMemorySemMutexHandle*)handle,
            SEMMUTEX_TYPE_BINARYSEM);
}

RTIBool
NETIO_SharedMemorySignalingSemaphore_delete(
        struct NETIO_SharedMemorySignalingSemaphoreHandle *handle)
{
    return NETIO_SharedMemorySemMutex_delete(
            (struct NETIO_SharedMemorySemMutexHandle*)handle,
            SEMMUTEX_TYPE_BINARYSEM);
}
