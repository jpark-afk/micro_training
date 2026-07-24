/*
 * FILE: NETIO_SDMMemPool.c - Memory pool manager
 *
 * (c) Copyright 2018-2024 Real-Time Innovations,Inc.
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

#include "reda/reda_circularlist.h"
#include "reda/reda_bufferpool.h"
#include "xcdr/xcdr_flat_data.h"

#include "NETIO_SDMDataWriterMemMgr.h"
#include "NETIO_SDMMemPool.h"

/* This function returns the state of a local address, called during
 * serialization.
 */
RTI_BOOL
SDM_MemPool_get_buffer_header_from_sample(
        struct SDM_SampleHeader *state,
        const void *sample)
{
    RTI_BOOL retval = RTI_TRUE;
    struct SDM_SampleHeader *buffer_state = NULL;

    buffer_state = OSAPI_Compiler_reinterpret_cast(
                        struct SDM_SampleHeader*,
                        ((char*)sample - sizeof(struct SDM_SampleHeader)));

    if (buffer_state->state != SDM_MEMBUFFERSTATE_SERIALIZED)
    {
        SDM_LOG_ILLEGAL_BUFFER_STATE(OSAPI_LOGKIND_ERROR);
        return RTI_FALSE;
    }

    state->related_epoch = buffer_state->related_epoch;
    state->key = buffer_state->key;
    state->shm_segment_epoch = buffer_state->shm_segment_epoch;

    /* index = which data-buffer it is in the logical array in each
     * segment
     */
    state->index = buffer_state->index;

    retval = RTI_TRUE;

    return retval;
}

MUST_CHECK_RETURN NETIODllExport struct SDM_MemPool*
SDM_MemPool_new(
        struct DataWriterShmMgr *admin,
        struct NETIO_Guid *owner,
        struct SDM_MemPoolProperty *property)
{
    struct SDM_MemPool *mem_pool = NULL;
    OSAPI_PRECONDITION(
            admin == NULL || property == NULL,
            return NULL,
            OSAPI_Log_entry_add_pointer("admin",admin,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("prop",property,RTI_TRUE); )

    OSAPI_Heap_allocate_struct(&mem_pool, struct SDM_MemPool);

    if (mem_pool == NULL)
    {
        SDM_LOG_MEMPOOL_ALLOC_FAILED(OSAPI_LOGKIND_ERROR);
        goto done;
    }

    if (!SDM_MemPool_initialize(admin,mem_pool,owner,property))
    {
        SDM_LOG_MEMPOOL_INIT_FAILED(OSAPI_LOGKIND_ERROR);
        SDM_MemPool_delete(mem_pool);
        mem_pool = NULL;
        goto done;
    }

done:

    return mem_pool;
}

RTI_BOOL
SDM_MemPool_delete(struct SDM_MemPool *self)
{
    OSAPI_PRECONDITION(
            self == NULL,
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE); )

    if (!SDM_MemPool_finalize(self))
    {
        return RTI_FALSE;
    }

    OSAPI_Heap_free_struct(self);
    return RTI_TRUE;
}

RTI_PRIVATE RTI_UINT32
SDM_MemPool_extend_pool(
        struct SDM_MemPool *self,
        RTI_UINT32 extend_count)
{
    struct SDM_MemSegment *segment;
    struct SDM_MemSegmentProperty seg_property =
            SDM_MEMSEGMENT_DEFAULT_PROPERTY;
    struct SDM_MemBufferFreeNode *free_buffer;
    struct SDM_SampleHeader *buffer_state;
    SDM_DWShmMgr_Retcode_T segment_retcode = 0;
    RTI_UINT32 alloc_count = 0;
    RTI_UINT32 allocated_count = 0;
    RTI_UINT32 i;

    seg_property.buffer_alignment = self->property.buffer_alignment;
    seg_property.buffer_size = self->property.buffer_size;
    alloc_count = extend_count;

    while (alloc_count > 0)
    {
        seg_property.buffer_count = alloc_count;
        segment = DataWriterShmMgr_new_segment(
                self->dw_shm_mgr,
                &segment_retcode,
                &seg_property);

        if (segment == NULL)
        {
            if (segment_retcode == SDM_DWSHMMGR_RETCODE_USER_TYPE_TOO_BIG)
            {
                goto done;
            }

            if (alloc_count == 1)
            {
                SDM_FAILED_ALLOCATE_SEGMENT_SINGLE_SAMPLE(OSAPI_LOGKIND_ERROR);
                goto done;
            }
            alloc_count /= 2;
            continue;

        }

        REDA_CircularList_append(
                &self->segment_list,
                (struct REDA_CircularListNode* )segment);

        for (i = 0; i < alloc_count; ++i)
        {
            free_buffer = OSAPI_Compiler_reinterpret_cast(
                                struct SDM_MemBufferFreeNode*,
                                REDA_BufferPool_get_buffer(
                                        self->free_buffer_pool));
            if (free_buffer == NULL)
            {
                allocated_count += i;
                goto done;
            }

            free_buffer->address =
                    (char*)SDM_MemSegment_get_buffer_start_address(segment)
                    + i * SDM_MemSegment_get_buffer_distance(segment);

            buffer_state = OSAPI_Compiler_reinterpret_cast(
                                struct SDM_SampleHeader*,
                                    ((char*)free_buffer->address -
                                      sizeof(struct SDM_SampleHeader)));

            buffer_state->index = i;
            buffer_state->key = SDM_MemSegment_get_key(segment);
            REDA_SequenceNumber_set_zero(&buffer_state->related_epoch);
            buffer_state->state = SDM_MEMBUFFERSTATE_FREE;
            buffer_state->kind = SDM_MEMBUFFERKIND_SHMEM;
            buffer_state->owner_private_addr = NULL;
            buffer_state->shm_segment_epoch =
                    segment->shared_hdr->shm_segment_epoch;
            buffer_state->size_of_buffer_state = sizeof(struct
                    SDM_SampleHeader);

            REDA_CircularList_append(
                    &self->free_buffer_list,
                    (struct REDA_CircularListNode* )free_buffer);
        }

        allocated_count += alloc_count;
        if (alloc_count > (extend_count - allocated_count))
        {
            alloc_count = extend_count - allocated_count;
        }
    }

done:
    return allocated_count;
}

RTI_BOOL
SDM_MemPool_initialize(
        struct DataWriterShmMgr *admin,
        struct SDM_MemPool *self,
        struct NETIO_Guid *owner,
        struct SDM_MemPoolProperty *property)
{
    RTIBool retval = RTI_FALSE;
    struct REDA_BufferPoolProperty fbprop = REDA_BufferPoolProperty_INITIALIZER;
    RTI_UINT32 extended_count = 0;

    self->free_buffer_list_lock = NULL;
    self->free_buffer_pool = NULL;
    self->free_buffer_list_lock = OSAPI_Mutex_new();
    self->property = *property;
    self->dw_shm_mgr = admin;
    self->owner = *owner;

    REDA_SequenceNumber_set_zero(&self->related_epoch_counter);
    REDA_CircularList_init(&self->free_buffer_list);
    REDA_CircularList_init(&self->in_use_list);
    REDA_CircularList_init(&self->segment_list);

    if (self->free_buffer_list_lock == NULL)
    {
        SDM_LOG_MEMPOOL_LOCK_CREATION_FAILED(OSAPI_LOGKIND_ERROR);
        goto done;
    }

    fbprop.buffer_size = sizeof(struct SDM_MemBufferFreeNode);
    fbprop.max_buffers = self->property.max_buffer_count;
    fbprop.flags = REDA_BUFFERPOOL_FLAG_ARRAY_ALLOC;

    self->free_buffer_pool = REDA_BufferPool_new(
            "free_buffer_pool",
            &fbprop,
            NULL,
            NULL,
            NULL,
            NULL);
    if (self->free_buffer_pool == NULL)
    {
        SDM_LOG_BUF_POOL_CREATION_FAILED(OSAPI_LOGKIND_ERROR);
        goto done;
    }

    extended_count = SDM_MemPool_extend_pool(
            self,
            self->property.max_buffer_count);

    if (extended_count != self->property.max_buffer_count)
    {
        SDM_LOG_MEMPOOL_FAILED_TO_EXTEND(OSAPI_LOGKIND_ERROR);
        goto done;
    }

    retval = RTI_TRUE;

done:

    return retval;
}

RTI_BOOL
SDM_MemPool_finalize(struct SDM_MemPool *self)
{
    RTI_BOOL retval = RTI_FALSE;
    struct SDM_MemSegment *segment;
    struct SDM_MemBufferFreeNode *free_buffer;

    OSAPI_PRECONDITION(
            (self == NULL),
            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE),
            return RTI_FALSE);

    while (!REDA_CircularList_is_empty(&self->segment_list))
    {
        segment = (struct SDM_MemSegment*)
                REDA_CircularList_get_first(&self->segment_list);
        REDA_CircularList_unlink_node((struct REDA_CircularListNode*)segment);
        DataWriterShmMgr_delete_segment(self->dw_shm_mgr,segment);
        segment = NULL;
    }

    if (self->free_buffer_list_lock != NULL)
    {
        OSAPI_Mutex_delete(self->free_buffer_list_lock);
        self->free_buffer_list_lock = NULL;
    }

    /* Return all buffers to the free_buffer_pool otherwise it
     * cannot be deleted
     */
    while (!REDA_CircularList_is_empty(&self->free_buffer_list))
    {
        free_buffer = (struct
                SDM_MemBufferFreeNode*)REDA_CircularList_get_first(
                &self->free_buffer_list);
        REDA_BufferPool_return_buffer(self->free_buffer_pool, free_buffer);
        REDA_CircularList_unlink_node(
                (struct
                REDA_CircularListNode*)free_buffer);
    }

    /* Return all buffers to the free_buffer_pool otherwise it
     * cannot be deleted
     */
    while (!REDA_CircularList_is_empty(&self->in_use_list))
    {
        free_buffer = (struct SDM_MemBufferFreeNode*) REDA_CircularList_get_first(
                &self->in_use_list);

        REDA_BufferPool_return_buffer(self->free_buffer_pool, free_buffer);
        REDA_CircularList_unlink_node((struct REDA_CircularListNode*) free_buffer);
    }


    if (self->free_buffer_pool != NULL)
    {
        if (!REDA_BufferPool_delete(self->free_buffer_pool))
        {
            goto done;
        }
        self->free_buffer_pool = NULL;
    }

    retval = RTI_TRUE;
done:

    return retval;
}

void*
SDM_MemPool_allocate_buffer(struct SDM_MemPool *self)
{
    RTI_BOOL sem_status;
    struct SDM_MemBufferFreeNode *node;
    struct SDM_SampleHeader *buffer_state;
    void *return_sample = NULL;

    OSAPI_PRECONDITION(
            (self == NULL),
            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE),
            return NULL);

    sem_status = OSAPI_Mutex_take(self->free_buffer_list_lock);
    if (sem_status != RTI_TRUE)
    {
        SDM_LOG_MEMPOOL_LOCK_FAILED(OSAPI_LOGKIND_ERROR);
        return NULL;
    }

    if (!REDA_CircularList_is_empty(&self->free_buffer_list))
    {
        /* Still space in free buffer list */
        node = (struct SDM_MemBufferFreeNode*)REDA_CircularList_get_first(
                &self->free_buffer_list);
    }
    else
    {
        SDM_LOG_DW_OUT_OF_AVAILABLE_SEGMENTS(OSAPI_LOGKIND_ERROR);
        goto done;
    }

    REDA_CircularList_unlink_node((struct REDA_CircularListNode*)node);

    return_sample = node->address;

    buffer_state = OSAPI_Compiler_reinterpret_cast(
                    struct SDM_SampleHeader*,
                    ((char*)return_sample - sizeof(struct SDM_SampleHeader)));

    buffer_state->owner_private_addr = node;
    buffer_state->state = SDM_MEMBUFFERSTATE_ALLOCATED;

    REDA_SequenceNumber_plusplus(&self->related_epoch_counter);
    buffer_state->related_epoch = self->related_epoch_counter;

    REDA_CircularList_append(
            &self->in_use_list,
            (struct REDA_CircularListNode* )node);

done:
    sem_status = OSAPI_Mutex_give(self->free_buffer_list_lock);
    if (sem_status != RTI_TRUE)
    {
        SDM_LOG_MEMPOOL_UNLOCK_FAILED(OSAPI_LOGKIND_ERROR);
        return NULL;
    }

    return return_sample;
}

RTI_BOOL
SDM_MemPool_free_buffer(
        struct SDM_MemPool *self,
        const void *address,
        RTIBool removed)
{
    struct SDM_SampleHeader *buffer_state;

    OSAPI_PRECONDITION(
            (self == NULL || address == NULL),
            OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("address",address,RTI_TRUE),
            return RTI_FALSE);

    buffer_state = OSAPI_Compiler_reinterpret_cast(
                        struct SDM_SampleHeader*,
                        ((char*)address - sizeof(struct SDM_SampleHeader)));

    if (!OSAPI_Mutex_take(self->free_buffer_list_lock))
    {
        SDM_LOG_MEMPOOL_LOCK_FAILED(OSAPI_LOGKIND_ERROR);
        return RTI_FALSE;
    }

    if ((removed && (buffer_state->state != SDM_MEMBUFFERSTATE_SERIALIZED)) ||
            (!removed && ((buffer_state->state !=
            SDM_MEMBUFFERSTATE_ALLOCATED) &&
            (buffer_state->state != SDM_MEMBUFFERSTATE_REMOVED))))
    {
        SDM_LOG_ILLEGAL_BUFFER_STATE(OSAPI_LOGKIND_ERROR);
        goto done;
    }

    if (removed)
    {
        buffer_state->state = SDM_MEMBUFFERSTATE_REMOVED;
    }
    else
    {
        buffer_state->state = SDM_MEMBUFFERSTATE_FREE;
    }

    if (buffer_state->owner_private_addr)
    {
        /* At this point, the node reside in the in_use_list
         * Unlink it from the in_use_list and add it to the list
         * which maintains the list of loanable buffers
         */
        REDA_CircularList_unlink_node(
                (struct REDA_CircularListNode*) buffer_state->owner_private_addr);

        REDA_CircularList_append(
                &self->free_buffer_list,
                (struct REDA_CircularListNode* )
                buffer_state->owner_private_addr);
        buffer_state->owner_private_addr = NULL;
    }

done:
    if (!OSAPI_Mutex_give(self->free_buffer_list_lock))
    {
        SDM_LOG_MEMPOOL_UNLOCK_FAILED(OSAPI_LOGKIND_ERROR);
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
SDM_MemPool_move_from_free_list_to_in_use(struct SDM_MemPool *self, const void *buffer)
{
    struct SDM_SampleHeader *sample_hdr = (struct SDM_SampleHeader*)buffer;
    sample_hdr--;

    /* unlink it from whatever list it may be in and ensure it ends up
     * in the in_use_list
     */
    REDA_CircularList_unlink_node(
            (struct REDA_CircularListNode*) sample_hdr->owner_private_addr);

    REDA_CircularList_append(&self->in_use_list, (struct REDA_CircularListNode* )
        sample_hdr->owner_private_addr);
    return RTI_TRUE;
}

struct SDM_MemSegment*
SDM_MemPool_is_buffer_from_here(struct SDM_MemPool *self, const void *buffer)
{
    struct SDM_MemSegment *segment =
            (struct SDM_MemSegment*)REDA_CircularList_get_first(
            &self->segment_list);
    while (!REDA_CircularList_node_at_head(&self->segment_list, segment))
    {
        if (buffer >= segment->buffer_start_address &&
                buffer < segment->buffer_end_address)
        {
            return segment;
        }

        segment = (struct SDM_MemSegment*)REDA_CircularListNode_get_next(
                &segment->list_node);
    }

    return NULL;
}

void*
SDM_Segment_get_data_from_reference(
        struct SDM_MemSegment *segment,
        struct SDM_SampleHeader *in_state)
{
    void *sample  = NULL;

    OSAPI_PRECONDITION(
            (segment == NULL || in_state == NULL),
            OSAPI_Log_entry_add_pointer("segment",segment,RTI_TRUE);
            OSAPI_Log_entry_add_pointer("in_state",in_state,RTI_FALSE),
            return NULL);

    sample = (char*)segment->buffer_start_address
            + in_state->index * segment->shared_hdr->buffer_distance;

    return sample;
}

void
SDM_MemPool_set_sample_state(
        const void *sample,
        SDM_MemBufferState_T state)
{
    struct SDM_SampleHeader *buffer_state = NULL;

    buffer_state = OSAPI_Compiler_reinterpret_cast(
                        struct SDM_SampleHeader*,
                        ((char*)sample - sizeof(struct SDM_SampleHeader)));

    buffer_state->state = state;
}

struct REDA_SequenceNumber
SDM_MemPool_get_related_epoch(const void* const sample)
{
    struct REDA_SequenceNumber return_value;
    struct SDM_SampleHeader *buffer_state = NULL;

    /* To allow for backwards compatibility, we need to check what the size
     * of the sample header preceding sample is
     */
    RTI_INT32 sample_hdr_sz = *(((RTI_INT32*)sample) - 1);

    buffer_state = OSAPI_Compiler_reinterpret_cast(
                        struct SDM_SampleHeader*,
                        (((char*)sample) - sample_hdr_sz));

    return_value = buffer_state->related_epoch;
    return return_value;
}

void*
SDM_MemSegment_get_buffer_start_address(struct SDM_MemSegment *self)
{
    return self->buffer_start_address;
}

/* The buffer_start_address points to the beginning of user data. The
 * buffer state of the first sample lies before the buffer_start_address.
 * Hence the buffer_end_address is computed by subtracting the size of
 * buffer state from the product of buffer distance and buffer count. */
void
SDM_MemSegment_set_buffer_end_address(struct SDM_MemSegment *self)
{
    self->buffer_end_address = (char*)self->buffer_start_address +
            (self->shared_hdr->buffer_distance *
            self->shared_hdr->buffer_count) -
            sizeof(struct SDM_SampleHeader);
}

RTI_UINT32
SDM_MemSegment_get_buffer_distance(struct SDM_MemSegment *self)
{
    return self->shared_hdr->buffer_distance;
}

RTI_INT32
SDM_MemSegment_get_key(struct SDM_MemSegment *self)
{
    return self->shm_key;
}
