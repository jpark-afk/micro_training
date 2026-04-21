/*
 * FILE: posixRobustShmMutex.h - Shared memory mutex using Robust POSIX Mutex
 * 
 * Copyright (c) 2024=2024,  Real-Time Innovations, Inc.
 * 
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef posixRobustShmMutex_h
#define posixRobustShmMutex_h

#include "rti_me_psl.h"

#include "osapi/osapi_log.h"
#include "osapi/osapi_types.h"
#include "netio_shmem/netio_shmem.h"
#include "osapi/osapi_process.h"
#include "osapi/osapi_string.h"

extern RTI_BOOL
NETIO_SharedMemoryPthreadMutex_create_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 *status_out,
        RTI_INT32 key);


extern RTI_BOOL
NETIO_SharedMemoryPthreadMutex_attach_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 *status_out,
        RTI_INT32 key);


extern RTI_BOOL
NETIO_SharedMemoryPthreadMutex_give_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 *status_out);

extern RTI_BOOL
NETIO_SharedMemoryPthreadMutex_take_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 *status_out);

extern RTI_BOOL
NETIO_SharedMemoryPthreadMutex_detach_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl);

extern RTI_BOOL
NETIO_SharedMemoryPthreadMutex_delete_os(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl);

#endif
