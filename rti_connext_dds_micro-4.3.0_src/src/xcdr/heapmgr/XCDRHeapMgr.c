/*
 * FILE: XCDRHeapMgr.c
 *
 * Copyright (c) 2018-2025 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#include "osapi/osapi_log.h"
#include "osapi/osapi_string.h"
#include "XCDRHeapMgr.h"

RTI_PRIVATE RTI_INT32
XCDR_HeapMgr_indexer_compare(
        const void * const record,
        RTI_BOOL key_is_record,
        const void * const key)
{
    /* We index based on the address of the sample
     */
    UNUSED_ARG(key_is_record);

    if (record == key)
    {
        return 0;
    }
    else if (record > key)
    {
        return 1;
    }
    else
    {
        return -1;
    }
}

RTI_PRIVATE RTI_BOOL
XCDR_HeapMgr_buffer_initialize(void *initialize_param, void *buffer)
{
    struct XCDR_HeapMgr *self = (struct XCDR_HeapMgr*)initialize_param;
    struct XCDR_HeapMgrBufferHeader *header = (struct XCDR_HeapMgrBufferHeader*)buffer;

    if (self->initialize_func == NULL)
    {
        return RTI_TRUE;
    }

    /*
     * Because the buffer pool buffer that is being initialized also includes
     * the header (i.e)
     * +-----+-------------------------------+
     * | HDR | Sample presented to the user  |
     * +-----+-------------------------------+
     *
     * We only want to call the initialize on the memory that will ultimately be
     * presented to the user
     */

    return self->initialize_func(&header[1]);
}

RTI_PRIVATE RTI_BOOL
XCDR_HeapMgr_buffer_finalize(void *finalize_param, void *buffer)
{
    struct XCDR_HeapMgr *self = (struct XCDR_HeapMgr*)finalize_param;
    struct XCDR_HeapMgrBufferHeader *header = (struct XCDR_HeapMgrBufferHeader*)buffer;

    if (self->finalize_func == NULL)
    {
        return RTI_TRUE;
    }

    return self->finalize_func(&header[1]);
}

struct XCDR_HeapMgr*
XCDR_HeapMgr_new(struct XCDR_HeapMgrProperty *mngr_prop)
{
    struct XCDR_HeapMgr *new_mngr = NULL;
    struct REDA_IndexerProperty indexer_prop;
    struct REDA_BufferPoolProperty bp_prop = REDA_BufferPoolProperty_INITIALIZER;
    struct XCDR_HeapMgrBufferListNode *list_node = NULL;
    struct REDA_CircularListNode *current = NULL;
    struct XCDR_HeapMgrBufferHeader *buffer_header = NULL;
    struct XCDR_HeapMgrBufferHeader *actual_sample = NULL;

    RTI_SIZE_T num_samples = mngr_prop->max_buffers;
    RTI_SIZE_T i = 0;
    RTI_BOOL creation_success = RTI_FALSE;

    OSAPI_Heap_allocate_struct(&new_mngr, struct XCDR_HeapMgr);
    OSAPI_Memory_zero(new_mngr, sizeof(struct XCDR_HeapMgr));
    if (new_mngr == NULL)
    {
        REDA_LOG_HEAP_MGR_ALLOC(OSAPI_LOGKIND_ERROR, REDA_LOG_HEAP_MGR_SAMPLE_BUFFERPOOL);
        goto done;
    }

    new_mngr->free_list_lock = OSAPI_Mutex_new();
    if (new_mngr->free_list_lock == NULL)
    {
        REDA_LOG_HEAP_MGR_ALLOC(OSAPI_LOGKIND_ERROR, REDA_LOG_HEAP_MGR_MUTEX);
        goto done;
    }

    /* RTI_INT32 is sufficient for number of buffers */
    indexer_prop.max_entries = (RTI_INT32)mngr_prop->max_buffers;
    new_mngr->heap_sample_indexer = REDA_Indexer_new(XCDR_HeapMgr_indexer_compare, &indexer_prop);
    if (new_mngr->heap_sample_indexer == NULL)
    {
        REDA_LOG_HEAP_MGR_ALLOC(OSAPI_LOGKIND_ERROR, REDA_LOG_HEAP_MGR_INDEXER);
        goto done;
    }

    /* Create pool of actual samples
     * This sample will not actually be used past this creation function.
     * We get all the buffers index them, and then eventually return them.
     */
    bp_prop.buffer_size = mngr_prop->buffer_size +
                          (RTI_SIZE_T)sizeof(struct XCDR_HeapMgrBufferHeader);

    bp_prop.max_buffers = mngr_prop->max_buffers;
    new_mngr->initialize_func = mngr_prop->initialize_func;
    new_mngr->finalize_func = mngr_prop->finalize_func;

    new_mngr->heap_sample_pool = REDA_BufferPool_new(
            "hsp",
            &bp_prop,
            XCDR_HeapMgr_buffer_initialize, /* Mem_pool initialization function */
            new_mngr,     /* Mem_pool initialization PARAM    */
            XCDR_HeapMgr_buffer_finalize,   /* Mem_pool finalization function   */
            new_mngr);      /* Mem_pool finalization PARAM      */
    if (new_mngr->heap_sample_pool == NULL)
    {
        REDA_LOG_HEAP_MGR_ALLOC(OSAPI_LOGKIND_ERROR, REDA_LOG_HEAP_MGR_SAMPLE_BUFFERPOOL);
        goto done;
    }

    /* Create pool of lists nodes that will be linked together */
    bp_prop.buffer_size = sizeof(struct XCDR_HeapMgrBufferListNode);
    bp_prop.max_buffers = mngr_prop->max_buffers;
    new_mngr->heap_buf_list_node_pool = REDA_BufferPool_new("hsp",
            &bp_prop, NULL, NULL, NULL, NULL);
    if (new_mngr->heap_buf_list_node_pool == NULL)
    {
        REDA_LOG_HEAP_MGR_ALLOC(OSAPI_LOGKIND_ERROR, REDA_LOG_HEAP_MGR_SAMPLE_BUFFERPOOL);
        goto done;
    }

    REDA_CircularList_init(&new_mngr->free_list);

    for (i = 0; i < num_samples; i++)
    {
        void *buffer = REDA_BufferPool_get_buffer(new_mngr->heap_sample_pool);

        list_node = (struct XCDR_HeapMgrBufferListNode*)REDA_BufferPool_get_buffer(
                new_mngr->heap_buf_list_node_pool);
        list_node->address = buffer;

        buffer_header = (struct XCDR_HeapMgrBufferHeader*)buffer;
        buffer_header->state = XCDR_SAMPLE_HEADER_STATE_FREE;
        buffer_header->owner_private_addr = list_node;

        actual_sample = &buffer_header[1];

        REDA_CircularList_append(
                &new_mngr->free_list,
                (struct REDA_CircularListNode* )list_node);

        /*
         * We index the actual buffer that will be returned to the user
         */
        if (!REDA_Indexer_add_entry(new_mngr->heap_sample_indexer, actual_sample))
        {
            /* This code path should theoretically never be exercised */
            REDA_LOG_HEAP_MGR_ALLOC(OSAPI_LOGKIND_ERROR, REDA_LOG_HEAP_MGR_INDEXER);
            goto done;
        }
    }

    /* Now return all the buffer back to the pool. We won't be using the buffer pools
     * until this class is being destroyed and deallocated.
     */
    current = REDA_CircularList_get_first(&new_mngr->free_list);
    while (!REDA_CircularList_node_at_head(&new_mngr->free_list, current))
    {
        REDA_BufferPool_return_buffer(
                new_mngr->heap_sample_pool,
                ((struct XCDR_HeapMgrBufferListNode*) current)->address);

        REDA_BufferPool_return_buffer(
                new_mngr->heap_buf_list_node_pool,
                (struct XCDR_HeapMgrBufferListNode*) current);

        current = (struct REDA_CircularListNode*) REDA_CircularListNode_get_next(
                (struct REDA_CircularListNode* )current);
    }

    creation_success = RTI_TRUE;

done:

    if (!creation_success)
    {
        if (new_mngr != NULL)
        {
            if (!XCDR_HeapMgr_delete(new_mngr))
            {
                return NULL;
            }
        }
        new_mngr = NULL;
    }

    return new_mngr;
}

RTI_BOOL
XCDR_HeapMgr_delete(struct XCDR_HeapMgr* self)
{
#ifndef RTI_CERT
    if (self->heap_sample_pool != NULL)
    {
        if (!REDA_BufferPool_delete(self->heap_sample_pool))
        {
            return RTI_FALSE;
        }
    }

    if (self->heap_buf_list_node_pool != NULL)
    {
        if (!REDA_BufferPool_delete(self->heap_buf_list_node_pool))
        {
            return RTI_FALSE;
        }
    }

    if (self->heap_sample_indexer != NULL)
    {
        if (!REDA_Indexer_delete(self->heap_sample_indexer))
        {
            return RTI_FALSE;
        }
    }


    if (self->free_list_lock != NULL)
    {
        if (!OSAPI_Mutex_delete(self->free_list_lock))
        {
            return RTI_FALSE;
        }
    }

    OSAPI_Heap_free(self);
#else
    UNUSED_ARG(self);
#endif

    return RTI_TRUE;
}

void*
XCDR_HeapMgr_allocate_buffer(struct XCDR_HeapMgr *mngr)
{
    struct XCDR_HeapMgrBufferListNode *node = NULL;
    struct XCDR_HeapMgrBufferHeader *sample = NULL;

    if (!OSAPI_Mutex_take(mngr->free_list_lock))
    {
        REDA_LOG_HEAP_MGR_MUTEX_FAILURE(OSAPI_LOGKIND_ERROR);
        goto done;
    }

    if (!REDA_CircularList_is_empty(&mngr->free_list))
    {
        /* Still space in free buffer list */
        node = (struct XCDR_HeapMgrBufferListNode*) REDA_CircularList_get_first(
                &mngr->free_list);
    }
    else
    {
        REDA_LOG_HEAP_MGR_GET_BUFFER(OSAPI_LOGKIND_ERROR);
        goto done;
    }

    REDA_CircularList_unlink_node((struct REDA_CircularListNode*) node);

    sample = (struct XCDR_HeapMgrBufferHeader*)node->address;
    sample->state = XCDR_SAMPLE_HEADER_STATE_LOANED;

    /*
     * Increment by the sample header to return the actual sample.
     */
    sample++;

done:

    if (!OSAPI_Mutex_give(mngr->free_list_lock))
    {
        REDA_LOG_HEAP_MGR_MUTEX_FAILURE(OSAPI_LOGKIND_ERROR);
        sample = NULL;
    }

    return sample;
}

RTI_BOOL
XCDR_HeapMgr_return_buffer(struct XCDR_HeapMgr *mngr, void *sample)
{
    /* Because we maintain a free_list of sample headers, we need to get access
     * to the sample headers from the actual samples
     */
    struct XCDR_HeapMgrBufferHeader *header = NULL;
    struct REDA_CircularListNode *list_node = NULL;

    if (!OSAPI_Mutex_take(mngr->free_list_lock))
    {
        REDA_LOG_HEAP_MGR_MUTEX_FAILURE(OSAPI_LOGKIND_ERROR);
        return RTI_FALSE;
    }

    header = ((struct XCDR_HeapMgrBufferHeader*)sample) - 1;
    header->state = XCDR_SAMPLE_HEADER_STATE_REMOVED;

    list_node = (struct REDA_CircularListNode*) header->owner_private_addr;

    REDA_CircularList_append(&mngr->free_list, list_node);

    if (!OSAPI_Mutex_give(mngr->free_list_lock))
    {
        REDA_LOG_HEAP_MGR_MUTEX_FAILURE(OSAPI_LOGKIND_ERROR);
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
XCDR_HeapMgr_is_owner(struct XCDR_HeapMgr *mngr, const void *sample)
{
    void *found_entry = NULL;
    RTI_BOOL success = RTI_FALSE;

    if (!OSAPI_Mutex_take(mngr->free_list_lock))
    {
        REDA_LOG_HEAP_MGR_MUTEX_FAILURE(OSAPI_LOGKIND_ERROR);
        goto done;
    }

    found_entry = REDA_Indexer_find_entry(mngr->heap_sample_indexer, sample);
    if (found_entry == NULL)
    {
        goto done;
    }

    success = RTI_TRUE;
done:

    if (!OSAPI_Mutex_give(mngr->free_list_lock))
    {
        REDA_LOG_HEAP_MGR_MUTEX_FAILURE(OSAPI_LOGKIND_ERROR);
        success = RTI_FALSE;
    }

    return success;
}

RTI_PRIVATE RTI_BOOL
XCDR_HeapMgr_set_buffer_state(
        struct XCDR_HeapMgr *mngr,
        const void* sample,
        XCDR_HeapMgrBufferState new_state)
{
    struct XCDR_HeapMgrBufferHeader *sample_hdr = NULL;
    RTI_BOOL success = RTI_FALSE;
    sample_hdr = (struct XCDR_HeapMgrBufferHeader *)sample;
    sample_hdr -= 1;

    OSAPI_PRECONDITION(
            ((sample_hdr->state != XCDR_SAMPLE_HEADER_STATE_LOANED) &&
             (sample_hdr->state != XCDR_SAMPLE_HEADER_STATE_SERIALIZED) &&
             (sample_hdr->state != XCDR_SAMPLE_HEADER_STATE_REMOVED)),
            return RTI_FALSE,
            OSAPI_Log_entry_add_int("state",sample_hdr->state,RTI_TRUE);)

    if (!OSAPI_Mutex_take(mngr->free_list_lock))
    {
        REDA_LOG_HEAP_MGR_MUTEX_FAILURE(OSAPI_LOGKIND_ERROR);
        goto done;
    }

    if (new_state == XCDR_SAMPLE_HEADER_STATE_REMOVED)
    {
        if ((sample_hdr->state != XCDR_SAMPLE_HEADER_STATE_LOANED) &&
            (sample_hdr->state != XCDR_SAMPLE_HEADER_STATE_SERIALIZED))
        {
            goto done;
        }
    }
    else if (new_state == XCDR_SAMPLE_HEADER_STATE_SERIALIZED)
    {
        if ((sample_hdr->state != XCDR_SAMPLE_HEADER_STATE_LOANED) &&
             sample_hdr->state != XCDR_SAMPLE_HEADER_STATE_REMOVED)
        {
            goto done;
        }
    }
    else
    {
        /* Currently this function is only expected to be called to change
         * the state to SERIALIZED (i.e committed) and to removed (when a
         * sample is removed from from DataWriter history.
         */
        goto done;
    }

    sample_hdr->old_state = sample_hdr->state;
    sample_hdr->state = (RTI_INT32)new_state;

    success = RTI_TRUE;
done:

    if (!OSAPI_Mutex_give(mngr->free_list_lock))
    {
        REDA_LOG_HEAP_MGR_MUTEX_FAILURE(OSAPI_LOGKIND_ERROR);
        success = RTI_FALSE;
    }

    return success;
}

RTI_BOOL
XCDR_HeapMgr_revert_sample_state(struct XCDR_HeapMgr *mngr, const void* sample)
{
    struct XCDR_HeapMgrBufferHeader *sample_hdr = NULL;
    sample_hdr = (struct XCDR_HeapMgrBufferHeader *) sample;
    sample_hdr -= 1;

    if (!OSAPI_Mutex_take(mngr->free_list_lock))
    {
        REDA_LOG_HEAP_MGR_MUTEX_FAILURE(OSAPI_LOGKIND_ERROR);
        goto done;
    }

    sample_hdr->state = sample_hdr->old_state;

done:

    if (!OSAPI_Mutex_give(mngr->free_list_lock))
    {
        REDA_LOG_HEAP_MGR_MUTEX_FAILURE(OSAPI_LOGKIND_ERROR);
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
XCDR_HeapMgr_set_buffer_in_use(struct XCDR_HeapMgr *mngr, const void* sample)
{
    struct XCDR_HeapMgrBufferHeader *sample_hdr = NULL;
    sample_hdr = (struct XCDR_HeapMgrBufferHeader *) sample;
    sample_hdr -= 1;

    /* In the Shared Memory manager used by Zero Copy, we need to maintain an
     * in_use_list because we need to handle the case when a user creates a
     * DataWriter, gets a loan, and deletes the DataWriter - we need to keep track of
     * which samples need to be returned to the REDABufferPool so that the REDABufferPool
     * can be successfully deleted without any outstanding buffers.
     *
     * In Heap Memory manager
     * used for FLAT_DATA, we don't need to maintain an in_use_list because we index
     * all samples from the buffer_pool of samples and return them back.
     *
     * Still, if the user cached a buffer from on_sample_removed, we just need
     * to remove it from the free_list because it was added there
     */
    if (!OSAPI_Mutex_take(mngr->free_list_lock))
    {
        REDA_LOG_HEAP_MGR_MUTEX_FAILURE(OSAPI_LOGKIND_ERROR);
        return RTI_FALSE;
    }

    if (sample_hdr->state == XCDR_SAMPLE_HEADER_STATE_REMOVED)
    {
        REDA_CircularList_unlink_node(
                (struct REDA_CircularListNode*) sample_hdr->owner_private_addr);
    }

    if (!OSAPI_Mutex_give(mngr->free_list_lock))
    {
        REDA_LOG_HEAP_MGR_MUTEX_FAILURE(OSAPI_LOGKIND_ERROR);
        return RTI_FALSE;
    }

    return XCDR_HeapMgr_set_buffer_state(
            mngr,
            sample,
            XCDR_SAMPLE_HEADER_STATE_SERIALIZED);
}

RTI_BOOL
XCDR_HeapMgr_set_buffer_not_in_use(struct XCDR_HeapMgr *mngr, const void* sample)
{
    return XCDR_HeapMgr_set_buffer_state(mngr, sample, XCDR_SAMPLE_HEADER_STATE_REMOVED);
}

RTI_BOOL
XCDR_HeapMgr_is_buffer_in_use(
        struct XCDR_HeapMgr *mngr,
        const void* sample,
        RTI_BOOL *in_use)
{
    struct XCDR_HeapMgrBufferHeader *sample_hdr = NULL;
    RTI_BOOL success = RTI_FALSE;
    sample_hdr = (struct XCDR_HeapMgrBufferHeader *)sample;
    sample_hdr -= 1;

    OSAPI_PRECONDITION(
            ((sample_hdr->state != XCDR_SAMPLE_HEADER_STATE_LOANED) &&
             (sample_hdr->state != XCDR_SAMPLE_HEADER_STATE_SERIALIZED) &&
             (sample_hdr->state != XCDR_SAMPLE_HEADER_STATE_REMOVED)) ||
            (in_use == NULL),
            return RTI_FALSE,
            OSAPI_Log_entry_add_int("state",sample_hdr->state,RTI_TRUE);)

    *in_use = RTI_FALSE;

    if (!OSAPI_Mutex_take(mngr->free_list_lock))
    {
        REDA_LOG_HEAP_MGR_MUTEX_FAILURE(OSAPI_LOGKIND_ERROR);
        goto done;
    }

    if (sample_hdr->state == XCDR_SAMPLE_HEADER_STATE_SERIALIZED)
    {
        *in_use = RTI_TRUE;
    }

    success = RTI_TRUE;

done:

    if (!OSAPI_Mutex_give(mngr->free_list_lock))
    {
        REDA_LOG_HEAP_MGR_MUTEX_FAILURE(OSAPI_LOGKIND_ERROR);
        success = RTI_FALSE;
    }

    return success;
}
