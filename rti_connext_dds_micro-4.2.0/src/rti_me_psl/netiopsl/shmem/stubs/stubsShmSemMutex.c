/*
 * FILE: winShmSemMutex.c
 *
 * Copyright 2018-2024 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#include "rti_me_psl.h"

#include "osapi/osapi_log.h"
#include "osapi/osapi_types.h"
#include "netio_shmem/netio_shmem.h"

/*** SOURCE_BEGIN ***/

RTIBool
NETIO_SharedMemoryMutex_is_robust(void)
{
    return RTI_FALSE;
}

RTIBool
NETIO_SharedMemorySemMutex_create_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 *status_out,
        RTI_INT32 key,
        RTI_INT32 sem_type)
{
    UNUSED_ARG(h_impl);
    UNUSED_ARG(status_out);
    UNUSED_ARG(key);
    UNUSED_ARG(sem_type);

    return RTI_FALSE;
}

RTIBool
NETIO_SharedMemorySemMutex_attach_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 *status_out,
        RTI_INT32 key,
        RTI_INT32 sem_type)
{
    UNUSED_ARG(h_impl);
    UNUSED_ARG(status_out);
    UNUSED_ARG(key);
    UNUSED_ARG(sem_type);

    return RTI_FALSE;
}

RTIBool
NETIO_SharedMemorySemMutex_give_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 *status_out,
        RTI_INT32 sem_type)
{
    UNUSED_ARG(h_impl);
    UNUSED_ARG(status_out);
    UNUSED_ARG(sem_type);

    return RTI_FALSE;
}

RTIBool
NETIO_SharedMemorySemMutex_take_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 *status_out,
        RTI_INT32 sem_type)
{
    UNUSED_ARG(h_impl);
    UNUSED_ARG(status_out);
    UNUSED_ARG(sem_type);

    return RTI_FALSE;
}

RTIBool
NETIO_SharedMemorySemMutex_detach_impl(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 sem_type)
{
    UNUSED_ARG(h_impl);
    UNUSED_ARG(sem_type);

    return RTI_FALSE;
}

RTIBool
NETIO_SharedMemorySemMutex_delete_os(
        struct NETIO_SharedMemorySemMutexHandleImpl *h_impl,
        RTI_INT32 sem_type)
{
    UNUSED_ARG(h_impl);
    UNUSED_ARG(sem_type);

    return RTI_FALSE;
}
