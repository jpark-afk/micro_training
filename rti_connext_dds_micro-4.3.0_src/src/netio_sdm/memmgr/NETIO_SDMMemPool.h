/*
 * FILE: NETIO_SDMMemPool.h - Memory pool manager
 *
 * (c) Copyright 2018-2018 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef netio_sdm_mempool_h
#define netio_sdm_mempool_h

#include "osapi/osapi_types.h"
#include "osapi/osapi_log.h"
#include "osapi/osapi_log_impl.h"
#include "osapi/osapi_mutex.h"
#include "netio/netio_address.h"
#include "netio_sdm/netio_sdm_log.h"

typedef RTIBool
(*SDM_MemPool_initializeSampleFunction)(void*, RTIBool);

struct SDM_MemPoolProperty
{
    RTI_UINT32 buffer_size;
    RTI_UINT32 max_buffer_count;
    RTI_UINT32 buffer_alignment;
    SDM_MemPool_initializeSampleFunction initialize_buffer;
};

#define SDM_MEMPOOL_DEFAULT_PROPERTY {                 \
        0,                           /* buffer_size */ \
        0,                      /* max_buffer_count */ \
        8,                      /* buffer_alignment */ \
        NULL             /* initial_buffer_function */ \
}

struct SDM_MemPool
{
    struct SDM_MemPoolProperty property;
    struct NETIO_Guid owner;
    struct DataWriterShmMgr *dw_shm_mgr;
    REDA_CircularList_T free_buffer_list;

    /* segment_list maintained by this SDM_MemPool, one SDM_MemPool per DW */
    REDA_CircularList_T segment_list;

    /* This list will contain all buffers that have been loaned to the user
     * If the user does not write a loaned buffer, we will use this list
     * to de-allocate it.
     */
    REDA_CircularList_T in_use_list;
    OSAPI_Mutex_T *free_buffer_list_lock;

    /* The SDM_MemBufferFreeNode is obtained from free_buffer_pool. It points to
     * samples held in the shared memory segment
     */
    struct REDA_BufferPool *free_buffer_pool;
    struct REDA_SequenceNumber related_epoch_counter;
};

NETIO_SDMDllExport struct SDM_MemSegment*
SDM_MemPool_is_buffer_from_here(struct SDM_MemPool *self, const void *buffer);

NETIO_SDMDllExport RTI_BOOL
SDM_MemPool_move_from_free_list_to_in_use(struct SDM_MemPool *self, const void *buffer);

NETIO_SDMDllExport void*
SDM_Segment_get_data_from_reference(
        struct SDM_MemSegment *segment,
        struct SDM_SampleHeader *in_state);

NETIO_SDMDllExport RTI_BOOL
SDM_MemPool_get_buffer_header_from_sample(
        struct SDM_SampleHeader *state,
        const void *sample);

MUST_CHECK_RETURN NETIO_SDMDllExport struct SDM_MemPool*
SDM_MemPool_new(
        struct DataWriterShmMgr *admin,
        struct NETIO_Guid *owner,
        struct SDM_MemPoolProperty *property);

NETIO_SDMDllExport RTI_BOOL
SDM_MemPool_delete(struct SDM_MemPool *self);

MUST_CHECK_RETURN NETIO_SDMDllExport void*
SDM_MemPool_allocate_buffer(struct SDM_MemPool *self);

MUST_CHECK_RETURN NETIO_SDMDllExport RTI_BOOL
SDM_MemPool_free_buffer(
        struct SDM_MemPool *self,
        const void *address,
        RTIBool removed);

NETIO_SDMDllExport void
SDM_MemPool_set_related_epoch(
        void *sample,
        struct REDA_SequenceNumber related_epoch);


NETIO_SDMDllExport struct REDA_SequenceNumber
SDM_MemPool_get_related_epoch(const void* const sample);

void
SDM_MemPool_set_sample_state(
        const void *sample,
        SDM_MemBufferState_T state);

RTI_BOOL
SDM_MemPool_initialize(
        struct DataWriterShmMgr *admin,
        struct SDM_MemPool *self,
        struct NETIO_Guid *owner,
        struct SDM_MemPoolProperty *property);

RTI_BOOL
SDM_MemPool_finalize(struct SDM_MemPool *self);

RTI_INT32
SDM_MemSegment_get_key(struct SDM_MemSegment *self);

void*
SDM_MemSegment_get_buffer_start_address(struct SDM_MemSegment *self);

void
SDM_MemSegment_set_buffer_end_address(struct SDM_MemSegment *self);

RTI_UINT32
SDM_MemSegment_get_buffer_distance(struct SDM_MemSegment *self);

#endif /* netio_sdm_mempool_h */
