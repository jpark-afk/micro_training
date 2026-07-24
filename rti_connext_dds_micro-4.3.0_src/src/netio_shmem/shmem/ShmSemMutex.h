/*
 * FILE: ShmSemMutex.h
 *
 * Copyright 2018-2024 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef SharedMemorySemMutex_h
#define SharedMemorySemMutex_h

#include "osapi/osapi_types.h"
#include "netio_shmem/netio_shmem.h"

/*
 * The following functions serve as generic implementations of
 * that will be called by the specific instance of it i.e
 * NETIO_SharedMemoryMutex_create will call NETIO_SharedMemorySemMutex_create
 * which in turn will call the platform specific implementation
 */

RTIBool
NETIO_SharedMemorySemMutex_create(
        struct NETIO_SharedMemorySemMutexHandle *handle,
        RTI_INT32 *status_out,
        RTI_INT32 key,
        RTI_INT32 type);

RTIBool
NETIO_SharedMemorySemMutex_attach(
        struct NETIO_SharedMemorySemMutexHandle *handle,
        RTI_INT32 *status_out,
        RTI_INT32 key,
        RTI_INT32 type);

RTIBool
NETIO_SharedMemorySemMutex_create_or_attach(
        struct NETIO_SharedMemorySemMutexHandle *handle,
        RTI_INT32 *status_out,
        RTI_INT32 key,
        RTI_INT32 type);

RTIBool
NETIO_SharedMemorySemMutex_give(
        struct NETIO_SharedMemorySemMutexHandle *handle,
        RTI_INT32 *fail_reason_out,
        RTI_INT32 type);

RTIBool
NETIO_SharedMemorySemMutex_take(
        struct NETIO_SharedMemorySemMutexHandle *handle,
        RTI_INT32 *fail_reason_out,
        RTI_INT32 type);

RTIBool
NETIO_SharedMemorySemMutex_detach(
        struct NETIO_SharedMemorySemMutexHandle *handle,
        RTI_INT32 type);

RTIBool
NETIO_SharedMemorySemMutex_delete(
        struct NETIO_SharedMemorySemMutexHandle *handle,
        RTI_INT32 type);

#endif /* SharedMemorySemMutex_h */
