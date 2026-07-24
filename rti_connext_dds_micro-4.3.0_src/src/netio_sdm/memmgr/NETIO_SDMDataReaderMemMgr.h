/*
 * FILE: NETIO_SDMDataReaderMemMgr.h - DataReader memory manager
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
#ifndef NETIO_SDMDataReaderMemMgr_h
#define NETIO_SDMDataReaderMemMgr_h

#include "netio_sdm/netio_sdm_dll.h"
#include "netio_sdm/netio_sdm.h"

#include "netio_shmem/netio_shmem.h"
#include "netio/netio_address.h"
#include "reda/reda_sequenceNumber.h"
#include "osapi/osapi_time.h"

struct DataReaderShmMgr
{
    struct REDA_Indexer *remote_segment_list;
    struct REDA_BufferPool *segment_pool;
    RTI_INT32 mapped_in_segment_count;
    RTI_INT32 mapped_in_segment_count_max;
};

struct DataReaderShmMgrProperty
{
    RTI_INT32 max_remote_segments;
};

#define DataReaderShmMgrProperty_INITIALIZER \
    {                                        \
        1                                    \
    }


RTI_BOOL
DataReaderShmMgr_decrement_reference(
        struct DataReaderShmMgr *self,
        void *buffer);

RTI_BOOL
DataReaderShmMgr_unmap_all_segments_with_publication_id(
        struct DataReaderShmMgr *self,
        struct NETIO_Guid *publication_handle_to_umap);

struct SDM_MemSegment*
DataReaderShmMgr_find_segment(
        struct DataReaderShmMgr *self,
        RTI_INT32 key,
        struct SDM_MemSegmentSharedSegmentEpoch shm_segment_epoch);

RTI_BOOL
DataReaderShmMgr_evict_any_segment(
        struct DataReaderShmMgr *self);

RTI_BOOL
DataReaderShmMgr_is_buffer_from_here(
        struct DataReaderShmMgr *self,
        const void *buffer);

RTIBool
DataReaderShmMgr_map_segment(
        struct DataReaderShmMgr *self,
        RTI_INT32 key,
        struct SDM_MemSegment *segment);

RTIBool
DataReaderShmMgr_unmap_segment(
        struct DataReaderShmMgr *self,
        struct SDM_MemSegment *segment);

struct DataReaderShmMgr*
DataReaderShmMgr_new(struct DataReaderShmMgrProperty *property);

RTI_BOOL
DataReaderShmMgr_delete(struct DataReaderShmMgr *self);

RTI_BOOL
DataReaderShmMgr_initialize(
        struct DataReaderShmMgr *self,
        struct DataReaderShmMgrProperty *mem_admin_property);

RTI_BOOL
DataReaderShmMgr_finalize(struct DataReaderShmMgr *self);

#endif /* NETIO_SDMDataReaderMemMgr_h */
