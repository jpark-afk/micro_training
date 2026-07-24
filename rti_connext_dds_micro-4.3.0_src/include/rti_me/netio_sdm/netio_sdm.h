/*
 * FILE: netio_sdm.h
 *
 * (c) Copyright 2017-2024 Real-Time Innovations,Inc.
 *
 * All rights reserved.
 *
 * Copyright 2018-2018 Real-Time Innovations, Inc.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef netio_sdm_h
#define netio_sdm_h

#include "netio_sdm_dll.h"
#include "netio_shmem/netio_shmem.h"
#include "netio/netio_address.h"
#include "reda/reda_sequenceNumber.h"
#include "osapi/osapi_time.h"

#define SDM_SEGMENT_COOKIE 0xCDD5DD5C
#define SDM_SEGMENT_MAJOR_VERSION 1
#define SDM_SEGMENT_MINOR_VERSION 0

/*
 * SH - Sample Header
 * UD - User Data
 * +----------+-----------+------+------+------+------+
 * | OSAPIHDR | SAMPLEHDR | SH 1 | UD 1 | SH 2 | UD 2 |
 * +----------+-----------+------+------+------+------+
 */

 /* Pro uses the following struct for their shm_segment_epoch.
  * For both Pro and Micro, the epoch is used as a way to identify
  * segments as unique. It's not used for time or timing. As a result,
  * it is okay that sec is an int32 even though it cannot hold our
  * max internal system time of UINT_MAX.
  */
struct SDM_MemSegmentSharedSegmentEpoch
{
    RTI_INT32 sec;
    RTI_UINT32 frac;
};

struct SDM_MemSegmentShared
{
    RTI_UINT32 cookie;
    RTI_UINT16 major_version;
    RTI_UINT16 minor_version;
    struct SDM_MemSegmentSharedSegmentEpoch shm_segment_epoch;

    /* Offset to the first user data */
    RTI_INT32 offset_to_1st_user_sample;
    RTI_UINT32 buffer_distance;
    RTI_UINT32 buffer_count;
    struct NETIO_Guid owner_guid;
};

typedef enum
{
    SDM_MEMBUFFERSTATE_FREE,
    SDM_MEMBUFFERSTATE_ALLOCATED,
    SDM_MEMBUFFERSTATE_REMOVED,
    SDM_MEMBUFFERSTATE_SERIALIZED
} SDM_MemBufferState_T;

typedef enum
{
    SDM_MEMBUFFERKIND_SHMEM   = 0x1
} SDM_MemBufferKind_T;

#define SIZE_OF_SHM_REFERENCE 16U

struct SDM_SampleHeader
{

#ifdef RTI_64BIT
    void *owner_private_addr;                                  /* 8 bytes (8) */
#else /* 32-bit */
    void *owner_private_addr;                                  /* 4 bytes (4) */
    RTI_INT32 padding_1;                                       /* 8 bytes (8) */
#endif
    /* ----------------------------------------------- 8 bytes until here (8) */
    SDM_MemBufferKind_T kind;                                 /* 4 bytes (12) */
    SDM_MemBufferState_T state;                               /* 4 bytes (16) */
    SDM_MemBufferState_T old_state;                           /* 4 bytes (20) */
    struct REDA_SequenceNumber related_epoch;             /* 2 x 4 bytes (28) */

    /* SHMEM_REF starts here */
    RTI_INT32 key;                                              /* 4 bytes (32) */
    RTI_UINT32 index;                                           /* 4 bytes (36) */
    struct SDM_MemSegmentSharedSegmentEpoch shm_segment_epoch;  /* 2 x 4 bytes (44) */
    /* SHMEM_REF ends here */

    /* Add each new field right above size_of_buffer_state
     * to maintain backward compatibility with older versions
     * IMPORTANT: Must add in 8 bytes at a time to make sure structure
     * is always aligned to 8 bytes
     */
    RTI_INT32 size_of_buffer_state;                           /* 4 bytes (48) */
    /* -------------------------------------------------  48 bytes until here */
};

NETIO_SDMDllExport const char*
NETIO_SDM_get_version(void);

#endif /* NETIO_NAME_h */

