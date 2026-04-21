/*
 * FILE: netio_shmem_segment.h
 *
 * Copyright 2025-2025 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef netio_shm_segment_h
#define netio_shm_segment_h

#ifndef netio_shmem_dll_h
#include "netio_shmem/netio_shmem_dll.h"
#endif

#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif

#ifndef osapi_process_h
#include "osapi/osapi_process.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*e
 * \ingroup NETIO_SharedMemorySegmentClass
 *
 * Opaque handle initialized by the attach methods and
 * used to interact with a shared memory segment.
 */
struct NETIO_SharedMemorySegmentHeader
{
    /*i Size requested by the caller of attach_or_create */
    RTI_INT32 size;

    /*i
     * PID of the process that created the segment.
     * It is set to the PID when the segment is created, and is set
     * to ZERO when the segment is destroyed. In this way it makes
     * safe the check of PID == process_itself in the createOrAttach.
     * On Integrity this is not used because Integrity doesn't
     * guarantee an unique ID for each process.
     *
     * For non-VxWorks 64-bit DKM(kernel) platforms we keep using a
     * 32-bit PID to not break backwards compatibility with previous RTI Connext
     * versions. For 64-bit VxWorks, we use 64-bit PID to support the new
     * LP64 VxWorks data-model (64-bit TASK_ID pointer for process IDs)
     */
#if defined(RTI_VXWORKS) && defined(RTI_64BIT) && !defined(RTI_RTP)
    RTI_UINT64 owner_pid;
#else
    RTI_UINT32 owner_pid;
#endif

    /*i Key that identifies this segment */
    RTI_INT32 key;

    /*i The total size of the allocated buffer */
    RTI_INT32 allocated_size;
};

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif /* netio_shm_segment_h */
