/*
 * FILE: posixShmSegment.h - POSIX shared memory segment definitions
 *
 * Copyright 2022-2022 Real-Time Innovations, Inc.
 * All rights reserved.
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef posixShmSegment_h
#define posixShmSegment_h

#include "rti_me_psl.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "netio_zcopy/netio_zcopy_shm_segment.h"

#include "osapi/osapi_heap.h"

#ifndef osapi_atomic_h
#include "osapi/osapi_atomic.h"
#endif

#define OSAPI_SHMEM_NATIVE_HANDLE_INVALID_SEGMENT (-1)
#define OSAPI_SHMEM_SEGMENT_INITIALIZED_MAGIC_COOKIE (0xDECADE)
#define OSAPI_SHMEM_WAIT_FOR_INIT_MAX_TRIES 20
#define OSAPI_SHMEM_WAIT_FOR_INIT_SLEEP_US 50000

#define MMAP_PROT_ARG_RW (PROT_READ | PROT_WRITE)
#define MMAP_PROT_ARG_RO PROT_READ

/*i POSIX specific extension of \ref OSAPI_SharedMemorySegmentHeader. */
struct OSAPI_SharedMemorySegmentHeaderImpl
{
    /*i \brief Base class */
    struct OSAPI_SharedMemorySegmentHeader parent_;

    /*i \brief PID of the process that created the segment */
    OSAPI_ProcessId owner_pid;

    /*i \brief Name of the segment */
    char name[OSAPI_SHMEM_MAX_NAME_LENGTH + 1];

    /*i \brief Flag to indicate if this segment has been initialized
     * \details This member is marked as atomic because it's used to check if
     * the segment has been initialized without being protected by a critical
     * section.
     */
    RTI_ATOMIC(RTI_UINT32) initialized;

    /*i \brief Mutex to protect the segment */
    pthread_mutex_t lock;
};

/*i Extension of \ref OSAPI_SharedMemorySegmentHeaderImpl capable of being locked. */
struct OSAPI_SharedMemorySegmentHeaderImplLockable
{
    /*i \brief Base class */
    struct OSAPI_SharedMemorySegmentHeaderImpl parent_;

    /*i \brief Mutex to protect user data in the segment */
    pthread_mutex_t user_lock;
};

/*i POSIX specific extension of \ref OSAPI_SharedMemorySegmentHandle. */
struct OSAPI_SharedMemorySegmentHandleImpl
{
    /*i \brief Base class */
    struct OSAPI_SharedMemorySegmentHandle parent_;

    /*i \brief Native file descriptor to the segment */
    int native_handle;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* posixShmSegment_h */
