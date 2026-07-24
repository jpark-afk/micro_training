/*
 * FILE: NETIO_SDMDataWriterMemMgr.c - DataWriter memory manager
 *
 * (c) Copyright 2018-2024 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#include "netio_shmem/netio_shmem.h"
#include "osapi/osapi_heap.h"
#include "osapi/osapi_string.h"
#include "osapi/osapi_process.h"
#include "osapi/osapi_thread.h"
#include "osapi/osapi_time.h"

#include "reda/reda_circularlist.h"
#include "reda/reda_bufferpool.h"
#include "reda/reda_sequenceNumber.h"
#include "reda/reda_indexer.h"

#include "netio/netio_address.h"
#include "netio_sdm/netio_sdm_log.h"

#include "osapi/osapi_log.h"
#include "osapi/osapi_log_impl.h"

#include "NETIO_SDMDataWriterMemMgr.h"
#include "NETIO_SDMMemPool.h"

RTI_PRIVATE void
DataWriterShmMgr_calculate_base_key(
        struct DataWriterShmMgr *self,
        struct DataWriterShmMgrProperty *property)
{
    self->base_shm_key = property->base_key
            + (property->domain_id * property->domain_gain)
            + (property->participant_index * (property->participant_gain));
}

struct SDM_MemSegment*
DataWriterShmMgr_new_segment(
        struct DataWriterShmMgr *self,
        SDM_DWShmMgr_Retcode_T *status_out,
        struct SDM_MemSegmentProperty *property)
{
    struct SDM_MemSegment *new_segment = NULL;
    RTI_INT32 key_index = 0;
    RTI_BOOL have_new_segment = RTI_FALSE;

    RTI_UINT32 buffer_distance = 0;
    RTI_INT32 shared_data_offset_aligned = 0;
    RTI_INT32 key_to_try = 0;
#define shared_hdr_addr_scalar(a) ((char*)a - (char*)OSAPI_CC_NullPtr)

    RTI_UINT64 requested_segment_size_64 = 0;
    RTI_UINT64 buffer_distance_64 = 0;

    RTI_UINT32 shared_hdr_size;
    RTI_UINT32 sample_hdr_size;
    RTI_UINT32 max_buffer_distance = 0;
    struct OSAPI_SystemTime systemTime = OSAPI_TIME_ZERO;
    RTI_UINT32 uint_sec = 0;
    RTI_UINT32 uint_frac = 0;

    OSAPI_PRECONDITION(
            (self == NULL || property == NULL || status_out == NULL),
            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE),
            return NULL);

    new_segment = OSAPI_Heap_allocate(1, sizeof(struct SDM_MemSegment));
    if (new_segment == NULL)
    {
        SDM_LOG_DW_MEMMGR_SEGMENT_ALLOC_FAILED(OSAPI_LOGKIND_ERROR);
        goto failure;
    }

    OSAPI_Memory_zero(
            &new_segment->shm_handle,
            sizeof(struct NETIO_SharedMemorySegmentHandle));

    /* the sample header size is guaranteed to be aligned to 8 bytes. We
     * test for this
     */
    sample_hdr_size = sizeof(struct SDM_SampleHeader);

    /* The user data needs to be aligned at the buffer_alignment value. Since
     * user data is prepended with buffer state, the shared header size is
     * computed by adding the size of buffer state during alignup and then the
     * size of buffer state is subtracted.
     */
    shared_hdr_size = (RTI_UINT32)OSAPI_Heap_align_size_up(
            sizeof(struct SDM_MemSegmentShared) + sample_hdr_size,
            property->buffer_alignment) - sample_hdr_size;

    new_segment->buffer_state_size = shared_hdr_size;

    /* The maximum buffer distance is the maximum size of a _SINGLE_
     * sample header + sample. Anything greater than that, we need to
     */
    max_buffer_distance = NETIO_SharedMemorySegment_get_max_size() - shared_hdr_size;
    buffer_distance_64 = OSAPI_Heap_align_size_up(
            sizeof(struct SDM_SampleHeader) + (RTI_UINT64)property->buffer_size,
            property->buffer_alignment);

    /* The sample itself is too big. A single sample (with a header) cannot
     * be allocated in a single shared memory segment
     */
    if (buffer_distance_64 > max_buffer_distance)
    {
        *status_out = SDM_DWSHMMGR_RETCODE_USER_TYPE_TOO_BIG;
        SDM_USER_TYPE_TOO_BIG(OSAPI_LOGKIND_ERROR, (RTI_INT32)property->buffer_size, (RTI_INT32)max_buffer_distance);
        goto failure;
    }

    buffer_distance = OSAPI_Heap_align_size_up(
            (RTI_UINT32)sizeof(struct SDM_SampleHeader) + property->buffer_size,
            property->buffer_alignment);

    requested_segment_size_64 = (RTI_UINT64) buffer_distance
            * (RTI_UINT64) property->buffer_count + shared_hdr_size;

    if (requested_segment_size_64 > NETIO_SharedMemorySegment_get_max_size())
    {
        /* Cannot fit requested size into the 2 GB limit. Therefore, we fail
         * and then the calling function will try again with a smaller buffer
         * count
         */
        goto failure;
    }

    new_segment->requested_size = buffer_distance * property->buffer_count + shared_hdr_size;

    SDM_Bitmap_set_search_range(
            self->key_bitmap,
            0,
            self->key_bitmap->num_bits - 1,
            RTI_TRUE);

    while (SDM_Bitmap_get_prev_index(self->key_bitmap, &key_index))
    {
        RTI_BOOL created_or_attached = RTI_FALSE;
        RTI_BOOL shm_operation_out;
        key_to_try = self->base_shm_key + key_index;

        created_or_attached = NETIO_SharedMemorySegment_create_or_attach(
                &new_segment->shm_handle,
                &shm_operation_out,
                key_to_try,
                new_segment->requested_size,
                OSAPI_Process_getpid());

        if (created_or_attached)
        {
            if (shm_operation_out == OSAPI_SHARED_MEMORY_ATTACHED)
            {
                /* If we have successfully attached, it means the owner process
                 * is dead and we have successfully took ownership
                 * If we attached, we try to delete it and recreate it.
                 * We want to guarantee that we are creating a fresh segment
                 */
                if (NETIO_SharedMemorySegment_delete(&new_segment->shm_handle))
                {
                    if (NETIO_SharedMemorySegment_create(
                            &new_segment->shm_handle,
                            &shm_operation_out,
                            key_to_try,
                            new_segment->requested_size,
                            OSAPI_Process_getpid()))
                    {
                        have_new_segment = RTI_TRUE;
                        break;
                    }
                }
            }
            else if (shm_operation_out == OSAPI_SHARED_MEMORY_CREATED)
            {
                /* We were able to create a segment on the first try.
                 * We are done;
                 */
                have_new_segment = RTI_TRUE;
                break;
            }
        }

        /*
         * Because get_prev_index actually "allocates it"
         */
        SDM_Bitmap_free_index(self->key_bitmap, key_index);
    }

    if (!have_new_segment)
    {
        SDM_LOG_FAILED_TO_CREATE_SEGMENT(OSAPI_LOGKIND_ERROR);
        goto failure;
    }

    SDM_Bitmap_allocate_index(self->key_bitmap, key_index);

    /* Information stored in the shared memory segment header. This is needed
     * by type-plugins which attach so samples can be found inside the
     * shared memory region
     */
    new_segment->shm_key = key_to_try;

    new_segment->shared_hdr =
            OSAPI_Compiler_reinterpret_cast(
                        struct SDM_MemSegmentShared*,
                        NETIO_SharedMemorySegment_get_address(
                                                &new_segment->shm_handle));

    new_segment->shared_hdr->buffer_distance = buffer_distance;
    new_segment->shared_hdr->buffer_count = property->buffer_count;
    new_segment->shared_hdr->owner_guid = self->datawriter_guid;
    new_segment->shared_hdr->cookie = SDM_SEGMENT_COOKIE;
    new_segment->shared_hdr->major_version = SDM_SEGMENT_MAJOR_VERSION;
    new_segment->shared_hdr->minor_version = SDM_SEGMENT_MINOR_VERSION;

    if (!OSAPI_System_get_time(&systemTime))
    {
        SDM_LOG_FAILED_TO_GET_TIME(OSAPI_LOGKIND_ERROR);
        goto failure;
    }

    /* Seconds past UINT_MAX isn't supported. The cast is fine. */
    OSAPI_SystemTime_to_ntp(
            &uint_sec,
            &uint_frac,
            &systemTime);
            
    /* We only use the epoch for the indexer key. It's okay if these 
     * values end up as negative in this cast
     */
    new_segment->shared_hdr->shm_segment_epoch.sec = (RTI_INT32)uint_sec;
    new_segment->shared_hdr->shm_segment_epoch.frac = (RTI_UINT32)uint_frac;

    /* Sanity check */
    if (shared_hdr_addr_scalar(new_segment->shared_hdr) % 8 != 0)
    {
        SDM_LOG_SEGMENT_NOT_ALIGNED(OSAPI_LOGKIND_ERROR);
        goto failure;
    }

    /* Calculate the offset so that the address of the first sample
     * is aligned. The offset cannot exceed a signed 32 bit value. The
     * calculation is done avoid the use of ptrdiff_t.
     */
    shared_data_offset_aligned = (RTI_INT32)(OSAPI_Heap_align_size_up(
            shared_hdr_addr_scalar(new_segment->shared_hdr) +
            new_segment->buffer_state_size +
            (RTI_UINT32)sizeof(struct SDM_MemSegmentShared),
            property->buffer_alignment) -
            shared_hdr_addr_scalar(new_segment->shared_hdr));

    new_segment->shared_hdr->offset_to_1st_user_sample =
                                                    shared_data_offset_aligned;

    /* Information stored here is only required by the owner of the buffer and
     * is this stored in local memory
     */
    new_segment->allocated_size =(RTI_UINT32)NETIO_SharedMemorySegment_get_size(
            &new_segment->shm_handle);
    new_segment->buffer_start_address = (char*)new_segment->shared_hdr +
            new_segment->shared_hdr->offset_to_1st_user_sample;
    SDM_MemSegment_set_buffer_end_address(new_segment);
    new_segment->ref_count = 0;

    *status_out = SDM_DWSHMMGR_RETCODE_SUCCESS;

    return new_segment;

failure:

    if (new_segment != NULL)
    {
        OSAPI_Heap_free(new_segment);
    }
    new_segment = NULL;
    return new_segment;
}

RTIBool
DataWriterShmMgr_delete_segment(
        struct DataWriterShmMgr *self,
        struct SDM_MemSegment *segment)
{
    RTIBool retval = RTI_FALSE;

    if (!NETIO_SharedMemorySegment_delete(&segment->shm_handle))
    {
        SDM_LOG_FAILED_TO_DELETE_SEGMENT(OSAPI_LOGKIND_ERROR);
        goto done;
    }

    SDM_Bitmap_free_index(
            self->key_bitmap,
            segment->shm_key - self->base_shm_key);
    OSAPI_Heap_free(segment);
    segment = NULL;

    retval = RTI_TRUE;
done:
    return retval;
}

struct DataWriterShmMgr*
DataWriterShmMgr_new(struct DataWriterShmMgrProperty *property)
{
    struct DataWriterShmMgr *mem_admin = NULL;

    OSAPI_PRECONDITION(
            (property == NULL),
            OSAPI_Log_entry_add_pointer("property",property,RTI_TRUE),
            return NULL);

    mem_admin = OSAPI_Heap_allocate(1, sizeof(struct DataWriterShmMgr));
    if (mem_admin == NULL)
    {
        SDM_LOG_MEMMGR_ALLOC_FAILED(OSAPI_LOGKIND_ERROR);
        goto done;
    }

    if (!DataWriterShmMgr_initialize(mem_admin, property))
    {
        SDM_LOG_DW_MGR_INIT_FAILED(OSAPI_LOGKIND_ERROR);
        DataWriterShmMgr_delete(mem_admin);
        mem_admin = NULL;
        goto done;
    }

done:
    return mem_admin;
}

RTI_BOOL
DataWriterShmMgr_delete(struct DataWriterShmMgr *self)
{
    OSAPI_PRECONDITION(
            (self == NULL),
            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE),
            return RTI_FALSE);

    if (!DataWriterShmMgr_finalize(self))
    {
        return RTI_FALSE;
    }

    OSAPI_Heap_free(self);

    return RTI_TRUE;

}

RTI_BOOL
DataWriterShmMgr_initialize(
        struct DataWriterShmMgr *self,
        struct DataWriterShmMgrProperty *mgr_prop)
{
    RTI_BOOL retval = RTI_FALSE;
    struct SDM_MemPoolProperty mem_prop = SDM_MEMPOOL_DEFAULT_PROPERTY;

    OSAPI_Memory_zero(&self->datawriter_guid, sizeof(struct NETIO_Guid));

    self->key_bitmap = mgr_prop->key_bitmap;
    self->datawriter_guid = mgr_prop->datawriter_owner_guid;

    if ((mgr_prop->participant_index + 1) *
            mgr_prop->participant_gain >=
            mgr_prop->domain_gain)
    {
        MEMMGR_LOG_MEMADMIN_INVALID_PROPERTY(OSAPI_LOGKIND_ERROR);
        return RTI_FALSE;
    }

    DataWriterShmMgr_calculate_base_key(self, mgr_prop);

    /* Now create the MemPool */
    mem_prop.buffer_alignment = 8;
    mem_prop.buffer_size = mgr_prop->user_sample_buffer_max_size;
    mem_prop.max_buffer_count = mgr_prop->max_user_sample_buffers;
    mem_prop.initialize_buffer = NULL;

    self->mem_pool = SDM_MemPool_new(
            self,
            &self->datawriter_guid,
            &mem_prop);
    if (self->mem_pool == NULL)
    {
        SDM_LOG_LW_MEMMGR_MEMPOOL_CREATION_FAILED(OSAPI_LOGKIND_ERROR);
        goto done;
    }
    retval = RTI_TRUE;

done:
    return retval;
}

RTI_BOOL
DataWriterShmMgr_finalize(struct DataWriterShmMgr *self)
{
    if (self->mem_pool != NULL)
    {
        if (!SDM_MemPool_delete(self->mem_pool))
        {
            SDM_LOG_DW_MEMMGR_MEMPOOL_DELETION_FAILED(OSAPI_LOGKIND_ERROR);
            return RTI_FALSE;
        }
    }

    return RTI_TRUE;
}

struct SDM_MemSegment*
DataWriterShmMgr_is_buffer_from_here(
        struct DataWriterShmMgr *self,
        const void *buffer)
{
    return SDM_MemPool_is_buffer_from_here(self->mem_pool, buffer);
}

void*
DataWriterShmMgr_allocate_buffer(struct DataWriterShmMgr *self)
{
    return SDM_MemPool_allocate_buffer(self->mem_pool);
}

RTI_BOOL
DataWriterShmMgr_free_buffer(
        struct DataWriterShmMgr *self,
        const void *address,
        RTIBool removed)
{
    return SDM_MemPool_free_buffer(self->mem_pool, address, removed);
}

RTI_BOOL
DataWriterShmMgr_move_buffer_from_free_list_to_in_use(
        struct DataWriterShmMgr *self,
        const void *address)
{
    return SDM_MemPool_move_from_free_list_to_in_use(self->mem_pool, address);
}

/******************************************************************************
 *                             NETIO_SDMUserData
 ******************************************************************************/

struct SDM_UserData*
SDM_UserData_new(RTI_UINT32 num_segment_keys)
{
    struct SDM_UserData *new_sdm_user_data = NULL;
    OSAPI_Heap_allocate_struct(&new_sdm_user_data, struct SDM_UserData);

    SDM_Bitmap_init(&new_sdm_user_data->key_bitmap, num_segment_keys);
    new_sdm_user_data->ref_count = 0;
    return new_sdm_user_data;
}

void
SDM_UserData_delete(struct SDM_UserData *user_data)
{
    SDM_Bitmap_finalize(&user_data->key_bitmap);
    OSAPI_Heap_free(user_data);
}

void
SDM_UserData_increment_ref_count(struct SDM_UserData *user_data)
{
    user_data->ref_count++;
}

void
SDM_UserData_decrement_ref_count(struct SDM_UserData *user_data)
{
    user_data->ref_count--;
}

struct SDM_Bitmap*
SDM_UserData_get_key_bitmap(struct SDM_UserData *user_data)
{
    return &user_data->key_bitmap;
}

RTI_INT32
SDM_UserData_get_ref_count(struct SDM_UserData *user_data)
{
    return user_data->ref_count;
}
