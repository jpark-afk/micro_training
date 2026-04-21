/*
 * FILE: commonShmSegment.c
 *
 * Copyright 2025-2025 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#include "rti_me_psl.h"
#include "netio_shmem/netio_shmem.h"

#ifndef netio_shm_segment_h
#include "rti_me_psl/netio/netio_shmem_segment.h"
#endif

RTI_INT32
NETIO_SharedMemorySegment_get_size(
        struct NETIO_SharedMemorySegmentHandle *handle)
{
    return ((struct
           NETIO_SharedMemorySegmentHandleImpl*)handle)->ptr_header->size;
}

RTI_UINT32
NETIO_SharedMemorySegment_get_max_size(void)
{
    return ((1U << 31) - 1U - sizeof(struct NETIO_SharedMemorySegmentHeader));
}
