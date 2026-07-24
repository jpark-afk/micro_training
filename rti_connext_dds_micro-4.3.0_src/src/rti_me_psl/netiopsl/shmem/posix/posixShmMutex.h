/*
 * FILE: posixShmMutex.h = Shared Memory using POSIX APIs
 * 
 * Copyright (c) 2018 - 2024  Real-Time Innovations, Inc.
 * 
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef posixShmMutex_h
#define posixShmMutex_h

#include "rti_me_psl.h"

#include "osapi/osapi_log.h"
#include "osapi/osapi_types.h"
#include "netio_shmem/netio_shmem.h"
#include "osapi/osapi_process.h"
#include "osapi/osapi_string.h"

extern RTIBool
NETIO_SharedMemoryPosixSemMutex_create_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 *status_out,
        RTI_INT32 key,
        RTI_INT32 sem_type);

extern RTIBool
NETIO_SharedMemoryPosixSemMutex_attach_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 *status_out,
        RTI_INT32 key,
        RTI_INT32 sem_type);

extern RTIBool
NETIO_SharedMemoryPosixSemMutex_give_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 *status_out,
        RTI_INT32 sem_type);

RTIBool
NETIO_SharedMemoryPosixSemMutex_take_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 *status_out,
        RTI_INT32 sem_type);

extern RTIBool
NETIO_SharedMemoryPosixSemMutex_detach_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 sem_type);


extern RTIBool
NETIO_SharedMemoryPosixSemMutex_delete_os(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 sem_type);

#endif
