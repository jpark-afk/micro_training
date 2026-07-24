/*
 * FILE: NETIO_SDMDataReaderMemMgr.c - DataReader memory manager
 *
 * (c) Copyright 2017-2024 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#include "osapi/osapi_types.h"
#include "netio_shmem/netio_shmem.h"
#include "osapi/osapi_string.h"
#include "reda/reda_circularlist.h"
#include "reda/reda_bufferpool.h"
#include "reda/reda_sequenceNumber.h"
#include "reda/reda_indexer.h"
#include "netio/netio_address.h"
#include "netio/netio_log.h"
#include "osapi/osapi_log_impl.h"
#include "NETIO_SDMDataReaderMemMgr.h"
#include "NETIO_SDMDataWriterMemMgr.h"
#include "netio/netio_common.h"
#include "netio_sdm/netio_sdm_log.h"

RTI_PRIVATE RTI_INT32
DataReaderShmMgr_Epoch_compare(
                        const struct SDM_MemSegmentSharedSegmentEpoch *time1,
                        const struct SDM_MemSegmentSharedSegmentEpoch *time2)
{
    return ((((time1)->sec) > ((time2)->sec)) ? 1 :
        ((((time1)->sec) < ((time2)->sec)) ? -1 :
        ((((time1)->frac) > ((time2)->frac)) ? 1 :
        ((((time1)->frac) < ((time2)->frac)) ? -1 : 0))));
}

RTI_BOOL
DataReaderShmMgr_decrement_reference(
        struct DataReaderShmMgr *self,
        void *buffer)
{
    struct SDM_MemSegment *segment = NULL;
    REDA_IndexIterator_T *it;
    RTI_BOOL retval = RTI_FALSE;

    it = REDA_Indexer_iterator_begin(self->remote_segment_list);
    segment = (struct SDM_MemSegment*)REDA_Indexer_iterator_next(it);

    while (segment != NULL)
    {
        if ((buffer >= segment->buffer_start_address) &&
                (buffer <= segment->buffer_end_address))
        {
            retval =  RTI_TRUE;
            break;
        }

        segment = (struct SDM_MemSegment*)REDA_Indexer_iterator_next(it);
    }

    if (retval)
    {
        segment->ref_count--;
    }
    else
    {
        goto done;
    }

    if (segment->ref_count < 0)
    {
        SDM_DECREMENT_REFERENCE(OSAPI_LOGKIND_ERROR);
        goto done;
    }

    if ((segment->ref_count == 0) && (segment->is_pending_detachment))
    {
        if (!DataReaderShmMgr_unmap_segment(self, segment))
        {
            SDM_FAILED_TO_UNMAP_SEGMENT(OSAPI_LOGKIND_ERROR);
        }
    }

done:
    return retval;
}

/* See unmap_all_segments_with_publication_id */
RTI_PRIVATE void
NETIO_DDS_GUID_from_rtps(struct NETIO_Guid *self,
                         const struct NETIO_Guid *other)
{
#if RTI_ENDIAN_BIG
    OSAPI_Memory_copy(self, other, RTI_SIZEOF(struct NETIO_Guid));
#else
    struct NETIO_AddressUInt32 rtps_tmp;
    struct NETIO_AddressUInt32 o = { {0, 0, 0, 0} };

    OSAPI_Memory_copy(&o,other,RTI_SIZEOF(struct NETIO_Guid));

    rtps_tmp.value[0] = NETIO_htonl(o.value[0]);
    rtps_tmp.value[1] = NETIO_htonl(o.value[1]);
    rtps_tmp.value[2] = NETIO_htonl(o.value[2]);
    rtps_tmp.value[3] = NETIO_htonl(o.value[3]);

    OSAPI_Memory_copy(self, &rtps_tmp, RTI_SIZEOF(struct NETIO_Guid));
#endif
}

RTI_BOOL
DataReaderShmMgr_unmap_all_segments_with_publication_id(
        struct DataReaderShmMgr *self,
        struct NETIO_Guid *publication_handle_to_umap)
{
    struct SDM_MemSegment *segment = NULL;
    REDA_IndexIterator_T *iterator = NULL;

    /* Detach from all segments we are attached to */
    if (self->remote_segment_list)
    {
        iterator = REDA_Indexer_iterator_begin(self->remote_segment_list);
        segment = (struct SDM_MemSegment*)REDA_Indexer_iterator_next(iterator);
        while (segment != NULL)
        {
            /* There is currently a bug where the GUID is network order
             * when it should be in host order.
             */

            /* Update to the above note: It is not apparent if the bug
             * is still present or not. The history of the bug is lost
             * and we decided to leave the workaround for now.
             */
            struct NETIO_Guid temporary_workaround_guid =
                                                    NETIO_ADDRESS_GUID_UNKNOWN;
            RTI_BOOL try_to_unmap;
            NETIO_DDS_GUID_from_rtps(
                    &temporary_workaround_guid,
                    publication_handle_to_umap);

            try_to_unmap = OSAPI_Memory_compare(
                    &temporary_workaround_guid,
                    &segment->shared_hdr->owner_guid,
                    sizeof(struct NETIO_Guid)) == 0;

            if (!try_to_unmap)
            {
                try_to_unmap = OSAPI_Memory_compare(
                        publication_handle_to_umap,
                        &segment->shared_hdr->owner_guid,
                        sizeof(struct NETIO_Guid)) == 0;
            }

            if (try_to_unmap)
            {
                if (!DataReaderShmMgr_unmap_segment(self, segment))
                {
                    SDM_LOG_FAILED_TO_DEL_SEGMENT(
                            OSAPI_LOGKIND_ERROR);
                    return RTI_FALSE;
                }
            }
            segment = (struct SDM_MemSegment*)REDA_Indexer_iterator_next(
                    iterator);
        }
    }

    return RTI_TRUE;
}

RTI_BOOL
DataReaderShmMgr_evict_any_segment(
        struct DataReaderShmMgr *self)
{
    struct SDM_MemSegment *segment = NULL;
    REDA_IndexIterator_T *iterator = NULL;
    RTI_INT32 segment_evicted = RTI_FALSE;

    if (self->remote_segment_list)
    {
        iterator = REDA_Indexer_iterator_begin(self->remote_segment_list);
        segment = (struct SDM_MemSegment*)REDA_Indexer_iterator_next(iterator);
        while (segment != NULL)
        {
            if (segment->ref_count == 0)
            {
                if (DataReaderShmMgr_unmap_segment(self, segment))
                {
                    segment_evicted = RTI_TRUE;
                    break;
                }
            }

            segment = (struct SDM_MemSegment*)REDA_Indexer_iterator_next(
                    iterator);
        }
    }

    return segment_evicted;
}

struct SDM_MemSegment*
DataReaderShmMgr_find_segment(
        struct DataReaderShmMgr *self,
        RTI_INT32 key,
        struct SDM_MemSegmentSharedSegmentEpoch shm_segment_epoch)
{
    struct SDM_MemSegment *segment = NULL;
    struct SDM_MemSegment search_segment;
    struct SDM_MemSegment *found_entry = NULL;

    search_segment.shm_key = key;
    search_segment.shm_segment_epoch = shm_segment_epoch;

    found_entry = REDA_Indexer_find_entry(
            self->remote_segment_list,
            &search_segment);

    if (found_entry != NULL)
    {
        segment = found_entry;
        segment->ref_count++;
        goto done;
    }

    /* Check if we need to evict a shared memory segment */
    if (self->mapped_in_segment_count == self->mapped_in_segment_count_max)
    {
        if (!DataReaderShmMgr_evict_any_segment(self))
        {
            SDM_LOG_EVICTION_FAILED(OSAPI_LOGKIND_ERROR);
            return NULL;
        }
        SDM_LOG_SEGMENT_EVICTED(
                OSAPI_LOGKIND_WARNING,
                self->mapped_in_segment_count_max);
    }

    segment = REDA_BufferPool_get_buffer(self->segment_pool);

    if (segment == NULL)
    {
        SDM_LOG_FAILED_TO_GET_BUFFER(OSAPI_LOGKIND_ERROR);
        goto done;
    }

    if (!DataReaderShmMgr_map_segment(self,key,segment))
    {
        MEMMGR_LOG_MEMADMIN_FAILED_TO_MAP_SEGMENT(OSAPI_LOGKIND_ERROR);
        REDA_BufferPool_return_buffer(self->segment_pool,segment);
        segment = NULL;
        goto done;
    }

    /* Add the segment to the list of remote segments. */
    if (!REDA_Indexer_add_entry(self->remote_segment_list, segment))
    {
        SDM_LOG_ADD_REMOTE_SEG_LIST(OSAPI_LOGKIND_ERROR);
        REDA_BufferPool_return_buffer(self->segment_pool,segment);
        segment = NULL;
        goto done;
    }

    segment->ref_count++;

done:

    return segment;
}

RTI_BOOL
DataReaderShmMgr_is_buffer_from_here(
        struct DataReaderShmMgr *self,
        const
        void *buffer)
{
    struct SDM_MemSegment *segment = NULL;
    REDA_IndexIterator_T *it;
    RTI_BOOL retval = RTI_FALSE;

    it = REDA_Indexer_iterator_begin(self->remote_segment_list);
    segment = (struct SDM_MemSegment*)REDA_Indexer_iterator_next(it);

    while (segment != NULL)
    {
        if ((buffer >= segment->buffer_start_address) &&
                (buffer <= segment->buffer_end_address))
        {
            retval =  RTI_TRUE;
            break;
        }

        segment = (struct SDM_MemSegment*)REDA_Indexer_iterator_next(it);
    }

    return retval;
}

RTIBool
DataReaderShmMgr_map_segment(
        struct DataReaderShmMgr *self,
        RTI_INT32 key,
        struct SDM_MemSegment *segment)
{
    RTIBool attached_ok = RTI_FALSE;
    int status_out;
    RTIBool retval = RTI_FALSE;

    /* First find space for the segment. This is an entry in the shared
     * memory region to store the key, _not_ the segment itself
     */
    if (self->mapped_in_segment_count == self->mapped_in_segment_count_max)
    {
        SDM_LOG_MEMADMIN_NO_SPACE_REMOTE_SEGMENT(OSAPI_LOGKIND_ERROR);
        goto done;
    }

    OSAPI_Memory_zero(
            &segment->shm_handle,
            sizeof(struct NETIO_SharedMemorySegmentHandle));

    attached_ok = NETIO_SharedMemorySegment_attach(
            &segment->shm_handle,
            &status_out,
            key);
    if (!attached_ok)
    {
        SDM_LOG_MAP_SEGMENT_ATTACH_FAILED(OSAPI_LOGKIND_ERROR);
        goto done;
    }

    segment->shared_hdr = OSAPI_Compiler_reinterpret_cast(
                                struct SDM_MemSegmentShared*,
                                NETIO_SharedMemorySegment_get_address(
                                        &segment->shm_handle));

    if (segment->shared_hdr->cookie != SDM_SEGMENT_COOKIE)
    {
        SDM_LOG_SEGMENT_COOKIE(OSAPI_LOGKIND_ERROR);
    }

    if (segment->shared_hdr->major_version != SDM_SEGMENT_MAJOR_VERSION)
    {
        SDM_LOG_SEGMENT_VERSION(OSAPI_LOGKIND_ERROR);
    }

    /* Only retrieve information that is relevant for using the segment
     */
    segment->allocated_size = (RTI_UINT32)NETIO_SharedMemorySegment_get_size(
            &segment->shm_handle);
    segment->buffer_start_address = (char*)segment->shared_hdr +
            segment->shared_hdr->offset_to_1st_user_sample;
    segment->shm_segment_epoch = segment->shared_hdr->shm_segment_epoch;

    SDM_MemSegment_set_buffer_end_address(segment);

    segment->shm_key = key;
    segment->ref_count = 0;

    self->mapped_in_segment_count++;
    retval = RTI_TRUE;

done:
    return retval;
}

RTIBool
DataReaderShmMgr_unmap_segment(
        struct DataReaderShmMgr *self,
        struct SDM_MemSegment *segment)
{
    RTIBool retval = RTI_FALSE;

    if (segment->ref_count > 0)
    {
        segment->is_pending_detachment = RTI_TRUE;
        retval = RTI_TRUE;
        goto done;
    }

    if (!NETIO_SharedMemorySegment_detach(&segment->shm_handle))
    {
        SDM_LOG_FAILED_TO_DETACH_SEGMENT(OSAPI_LOGKIND_ERROR);
        goto done;
    }

    self->mapped_in_segment_count--;

    if (REDA_Indexer_remove_entry(self->remote_segment_list, segment) == NULL)
    {
        SDM_LOG_REMOVE_FROM_REMOTE_SEG_LIST(OSAPI_LOGKIND_ERROR);
        goto done;
    }

    REDA_BufferPool_return_buffer(self->segment_pool, segment);
    retval = RTI_TRUE;
done:

    return retval;
}

struct DataReaderShmMgr*
DataReaderShmMgr_new(struct DataReaderShmMgrProperty *property)
{
    struct DataReaderShmMgr *dr_mem_mgr = NULL;

    OSAPI_PRECONDITION(
            (property == NULL),
            OSAPI_Log_entry_add_pointer("property",property,RTI_TRUE),
            return NULL);

    dr_mem_mgr = OSAPI_Heap_allocate(1, sizeof(struct DataReaderShmMgr));
    if (dr_mem_mgr == NULL)
    {
        SDM_LOG_MEMMGR_ALLOC_FAILED(OSAPI_LOGKIND_ERROR);
        goto done;
    }

    if (!DataReaderShmMgr_initialize(dr_mem_mgr,property))
    {
        SDM_LOG_DW_MGR_INIT_FAILED(OSAPI_LOGKIND_ERROR);
        DataReaderShmMgr_delete(dr_mem_mgr);
        dr_mem_mgr = NULL;
        goto done;
    }

done:

    return dr_mem_mgr;
}

RTI_BOOL
DataReaderShmMgr_delete(struct DataReaderShmMgr *self)
{
    OSAPI_PRECONDITION(
            (self == NULL),
            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE),
            return RTI_FALSE);

    if (!DataReaderShmMgr_finalize(self))
    {
        return RTI_FALSE;
    }

    OSAPI_Heap_free(self);
    return RTI_TRUE;

}

RTI_PRIVATE RTI_INT32
DataReaderShmMgr_compareSegment(
        const void* const record,
        RTI_BOOL key_is_record,
        const void* const key)
{
    struct SDM_MemSegment *left_type = (struct SDM_MemSegment*)record;
    struct SDM_MemSegment *right_type = (struct SDM_MemSegment*)key;
    UNUSED_ARG(key_is_record);

    if (left_type->shm_key == right_type->shm_key)
    {
        return DataReaderShmMgr_Epoch_compare(
                    &left_type->shm_segment_epoch,
                    &right_type->shm_segment_epoch);
    }

    return left_type->shm_key - right_type->shm_key;
}

RTI_BOOL
DataReaderShmMgr_initialize(
        struct DataReaderShmMgr *self,
        struct DataReaderShmMgrProperty *dr_shm_mgr_property)
{
    RTI_BOOL init_success = RTI_FALSE;
    struct REDA_BufferPoolProperty buffer_pool_prop;
    struct REDA_IndexerProperty idx_prop = REDA_IndexerProperty_INITIALIZER;

    self->segment_pool = NULL;
    self->remote_segment_list = NULL;

    buffer_pool_prop.flags = REDA_BUFFERPOOL_FLAG_ARRAY_ALLOC;
    buffer_pool_prop.buffer_size = sizeof(struct SDM_MemSegment);
    buffer_pool_prop.max_buffers = (RTI_SIZE_T)dr_shm_mgr_property->max_remote_segments;

    self->segment_pool = REDA_BufferPool_new(
            "segpool",
            &buffer_pool_prop,
            NULL,
            NULL,
            NULL,
            NULL);
    if (self->segment_pool == NULL)
    {
        SDM_LOG_BUF_POOL_CREATION_FAILED(OSAPI_LOGKIND_ERROR);
        return RTI_FALSE;
    }

    /*
     * Create segment list
     */
    idx_prop.max_entries = dr_shm_mgr_property->max_remote_segments;
    self->remote_segment_list = REDA_Indexer_new(
            DataReaderShmMgr_compareSegment,
            &idx_prop);
    if (self->remote_segment_list == NULL)
    {
        SDM_MEMMGR_LOG_IDX_CREATION_FAILED(OSAPI_LOGKIND_ERROR);
        goto done;
    }

    self->mapped_in_segment_count = 0;
    self->mapped_in_segment_count_max =
            dr_shm_mgr_property->max_remote_segments;
    init_success = RTI_TRUE;
done:

    return init_success;
}

RTI_BOOL
DataReaderShmMgr_finalize(struct DataReaderShmMgr *self)
{
    RTI_BOOL retval = RTI_FALSE;
    if (!REDA_Indexer_delete(self->remote_segment_list))
    {
        goto done;
    }

    self->remote_segment_list = NULL;

    if (self->segment_pool != NULL)
    {
        if (!REDA_BufferPool_delete(self->segment_pool))
        {
            goto done;
        }
    }

    self->segment_pool = NULL;
    retval = RTI_TRUE;
done:

    return retval;
}
