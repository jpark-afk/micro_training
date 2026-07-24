/*
 * FILE: winShmSegment.c
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
#include "osapi/osapi_process.h"
#include "osapi/osapi_string.h"

/*** SOURCE_BEGIN ***/

RTIBool
NETIO_SharedMemorySegment_create_or_attach_impl(
        struct NETIO_SharedMemorySegmentHandleImpl *h_impl,
        RTI_INT32 *status_out,
        RTI_INT32 segment_key,
        RTI_INT32 size,
        OSAPI_ProcessId pid_in)
{
    UNUSED_ARG(h_impl);
    UNUSED_ARG(status_out);
    UNUSED_ARG(segment_key);
    UNUSED_ARG(size);
    UNUSED_ARG(pid_in);

    return RTI_FALSE;
}

RTIBool
NETIO_SharedMemorySegment_create_impl(
        struct NETIO_SharedMemorySegmentHandleImpl *himpl,
        RTI_INT32 *status_out,
        RTI_INT32 key,
        RTI_INT32 size,
        OSAPI_ProcessId pidIn)
{
    UNUSED_ARG(himpl);
    UNUSED_ARG(status_out);
    UNUSED_ARG(key);
    UNUSED_ARG(size);
    UNUSED_ARG(pidIn);

    return RTI_FALSE;
}

RTIBool
NETIO_SharedMemorySegment_attach_impl(
        struct NETIO_SharedMemorySegmentHandleImpl *himpl,
        RTI_INT32 *status_out,
        RTI_INT32 key)
{
    UNUSED_ARG(himpl);
    UNUSED_ARG(status_out);
    UNUSED_ARG(key);

    return RTI_FALSE;
}

/*
 * Just like the POSIX implementation we clear the ptr_header but not the
 * native handle. In this case we don't need to use the native_hndl to carry
 * the key.
 */
RTIBool
NETIO_SharedMemorySegment_detach_impl(
        struct NETIO_SharedMemorySegmentHandleImpl *himpl,
        RTIBool clearPid)
{
    UNUSED_ARG(himpl);
    UNUSED_ARG(clearPid);

    return RTI_FALSE;
}

/*
 * Returns RTI_FALSE if delete fails. Do not log an error in this case.
 * A failure in the detach is ignored.
 */
RTIBool
NETIO_SharedMemorySegment_delete_impl(
        struct NETIO_SharedMemorySegmentHandleImpl *himpl)
{
    UNUSED_ARG(himpl);
    
    return RTI_FALSE;
}
