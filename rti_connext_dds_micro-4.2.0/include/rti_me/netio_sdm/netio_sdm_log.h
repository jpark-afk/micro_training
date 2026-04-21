/*
 * FILE: netio_sdm_log.h
 *
 * Copyright 2018-2018 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*e
 * \file
 * \brief NETIO Zero Copy Shared Data module log codes
 */
#ifndef netio_sdm_log_h
#define netio_sdm_log_h

#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif

/*e
 * \brief Failed to lock MemPool
 * \ingroup NETIOLogCodesClass
 */
#define SDM_LOG_MEMPOOL_LOCK_FAILED_EC                        (SDM_LOG_BASE + 1)
#define SDM_LOG_MEMPOOL_LOCK_FAILED(level_) \
    OSAPI_LOG_ENTRY_ADD(                    \
        (level_),                           \
        SDM_LOG_MEMPOOL_LOCK_FAILED_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to unlock MemPool
 * \ingroup NETIOLogCodesClass
 */
#define SDM_LOG_MEMPOOL_UNLOCK_FAILED_EC                      (SDM_LOG_BASE + 2)
#define SDM_LOG_MEMPOOL_UNLOCK_FAILED(level_) \
    OSAPI_LOG_ENTRY_ADD(                      \
        (level_),                             \
        SDM_LOG_MEMPOOL_UNLOCK_FAILED_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to allocate MemPool
 * \ingroup NETIOLogCodesClass
 */
#define SDM_LOG_MEMPOOL_ALLOC_FAILED_EC                       (SDM_LOG_BASE + 3)
#define SDM_LOG_MEMPOOL_ALLOC_FAILED(level_) \
    OSAPI_LOG_ENTRY_ADD(                     \
        (level_),                            \
        SDM_LOG_MEMPOOL_ALLOC_FAILED_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to allocate resources for Shared Data Manager
 * \ingroup NETIOLogCodesClass
 */
#define SDM_LOG_MEMMGR_ALLOC_FAILED_EC                        (SDM_LOG_BASE + 4)
#define SDM_LOG_MEMMGR_ALLOC_FAILED(level_) \
    OSAPI_LOG_ENTRY_ADD(                    \
        (level_),                           \
        SDM_LOG_MEMMGR_ALLOC_FAILED_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to init DataWriterShmManager
 * \ingroup NETIOLogCodesClass
 */
#define SDM_LOG_DW_MGR_INIT_FAILED_EC                         (SDM_LOG_BASE + 5)
#define SDM_LOG_DW_MGR_INIT_FAILED(level_) \
    OSAPI_LOG_ENTRY_ADD(                   \
        (level_),                          \
        SDM_LOG_DW_MGR_INIT_FAILED_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to init MemPool
 * \ingroup NETIOLogCodesClass
 */
#define SDM_LOG_MEMPOOL_INIT_FAILED_EC                        (SDM_LOG_BASE + 6)
#define SDM_LOG_MEMPOOL_INIT_FAILED(level_) \
    OSAPI_LOG_ENTRY_ADD(                    \
        (level_),                           \
        SDM_LOG_MEMPOOL_INIT_FAILED_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to get buffer from memory pool when finding a segment
 * \ingroup NETIOLogCodesClass
 */
#define SDM_LOG_FAILED_TO_GET_BUFFER_EC                       (SDM_LOG_BASE + 7)
#define SDM_LOG_FAILED_TO_GET_BUFFER(level_) \
    OSAPI_LOG_ENTRY_ADD(                     \
        (level_),                            \
        SDM_LOG_FAILED_TO_GET_BUFFER_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failure to map remote segment into memory manager
 * \ingroup NETIOLogCodesClass
 */
#define MEMMGR_LOG_MEMADMIN_FAILED_TO_MAP_SEGMENT_EC          (SDM_LOG_BASE + 8)
#define MEMMGR_LOG_MEMADMIN_FAILED_TO_MAP_SEGMENT(level_) \
    OSAPI_LOG_ENTRY_ADD(                                  \
        (level_),                                         \
        MEMMGR_LOG_MEMADMIN_FAILED_TO_MAP_SEGMENT_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failure to delete segment
 * \ingroup NETIOLogCodesClass
 */
#define SDM_LOG_FAILED_TO_DEL_SEGMENT_EC                      (SDM_LOG_BASE + 9)
#define SDM_LOG_FAILED_TO_DEL_SEGMENT(level_) \
    OSAPI_LOG_ENTRY_ADD(                      \
        (level_),                             \
        SDM_LOG_FAILED_TO_DEL_SEGMENT_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failure to add segment to segment list when finding segment
 * \ingroup NETIOLogCodesClass
 */
#define SDM_LOG_ADD_REMOTE_SEG_LIST_EC                       (SDM_LOG_BASE + 10)
#define SDM_LOG_ADD_REMOTE_SEG_LIST(level_) \
    OSAPI_LOG_ENTRY_ADD(                    \
        (level_),                           \
        SDM_LOG_ADD_REMOTE_SEG_LIST_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failure to remove segment to remote segment list when finding segment
 * \ingroup NETIOLogCodesClass
 */
#define SDM_LOG_REMOVE_FROM_REMOTE_SEG_LIST_EC               (SDM_LOG_BASE + 11)
#define SDM_LOG_REMOVE_FROM_REMOTE_SEG_LIST(level_) \
    OSAPI_LOG_ENTRY_ADD(                            \
        (level_),                                   \
        SDM_LOG_REMOVE_FROM_REMOTE_SEG_LIST_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failure to attach to segment which the middleware is trying to
 *        map in
 * \ingroup NETIOLogCodesClass
 */
#define SDM_LOG_MEMADMIN_MAP_SEGMENT_ATTACH_FAILED_EC        (SDM_LOG_BASE + 12)
#define SDM_LOG_MAP_SEGMENT_ATTACH_FAILED(level_)      \
    OSAPI_LOG_ENTRY_ADD(                               \
        (level_),                                      \
        SDM_LOG_MEMADMIN_MAP_SEGMENT_ATTACH_FAILED_EC, \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief DataReader Shared Memory Manager cannot map in any additional
 * remote segments
 * \ingroup NETIOLogCodesClass
 */
#define SDM_LOG_MEMADMIN_NO_SPACE_REMOTE_SEGMENT_EC          (SDM_LOG_BASE + 13)
#define SDM_LOG_MEMADMIN_NO_SPACE_REMOTE_SEGMENT(level_) \
    OSAPI_LOG_ENTRY_ADD(                                 \
        (level_),                                        \
        SDM_LOG_MEMADMIN_NO_SPACE_REMOTE_SEGMENT_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Memory Admin failed to create segment
 * \ingroup NETIOLogCodesClass
 */
#define SDM_LOG_FAILED_TO_CREATE_SEGMENT_EC                  (SDM_LOG_BASE + 14)
#define SDM_LOG_FAILED_TO_CREATE_SEGMENT(level_) \
    OSAPI_LOG_ENTRY_ADD(                         \
        (level_),                                \
        SDM_LOG_FAILED_TO_CREATE_SEGMENT_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create a shared memory segment for a single buffer
 * \ingroup NETIOLogCodesClass
 */
#define SDM_LOG_MEMPOOL_SEGMENT_CREATION_FAILED_EC           (SDM_LOG_BASE + 15)
#define SDM_LOG_MEMPOOL_SEGMENT_CREATION_FAILED(level_) \
    OSAPI_LOG_ENTRY_ADD(                                \
        (level_),                                       \
        SDM_LOG_MEMPOOL_SEGMENT_CREATION_FAILED_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to extend memory pool
 * \ingroup NETIOLogCodesClass
 */
#define SDM_LOG_MEMPOOL_FAILED_TO_EXTEND_EC                  (SDM_LOG_BASE + 16)
#define SDM_LOG_MEMPOOL_FAILED_TO_EXTEND(level_) \
    OSAPI_LOG_ENTRY_ADD(                         \
        (level_),                                \
        SDM_LOG_MEMPOOL_FAILED_TO_EXTEND_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Memory Admin is cannot map in any additional remote segments.
 * \ingroup NETIOLogCodesClass
 */
#define SDM_LOG_FAILED_TO_DELETE_SEGMENT_EC                  (SDM_LOG_BASE + 17)
#define SDM_LOG_FAILED_TO_DELETE_SEGMENT(level_) \
    OSAPI_LOG_ENTRY_ADD(                         \
        (level_),                                \
        SDM_LOG_FAILED_TO_DELETE_SEGMENT_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to detach segment from DataReader Shared Memory Manager
 * \ingroup NETIOLogCodesClass
 */
#define SDM_LOG_FAILED_TO_DETACH_SEGMENT_EC                  (SDM_LOG_BASE + 18)
#define SDM_LOG_FAILED_TO_DETACH_SEGMENT(level_) \
    OSAPI_LOG_ENTRY_ADD(                         \
        (level_),                                \
        SDM_LOG_FAILED_TO_DETACH_SEGMENT_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)


/*e
 * \brief MemAdmin is cannot map in any additional remote segments.
 * \ingroup NETIOLogCodesClass
 */
#define MEMMGR_LOG_MEMADMIN_INVALID_PROPERTY_EC              (SDM_LOG_BASE + 19)
#define MEMMGR_LOG_MEMADMIN_INVALID_PROPERTY(level_) \
    OSAPI_LOG_ENTRY_ADD(                             \
        (level_),                                    \
        MEMMGR_LOG_MEMADMIN_INVALID_PROPERTY_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create segment buffer pool
 * \ingroup NETIOLogCodesClass
 */
#define SDM_LOG_BUF_POOL_CREATION_FAILED_EC                  (SDM_LOG_BASE + 20)
#define SDM_LOG_BUF_POOL_CREATION_FAILED(level_) \
    OSAPI_LOG_ENTRY_ADD(                         \
        (level_),                                \
        SDM_LOG_BUF_POOL_CREATION_FAILED_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create indexer for indexing segments
 * \ingroup NETIOLogCodesClass
 */
#define SDM_MEMMGR_LOG_IDX_CREATION_FAILED_EC                (SDM_LOG_BASE + 21)
#define SDM_MEMMGR_LOG_IDX_CREATION_FAILED(level_) \
    OSAPI_LOG_ENTRY_ADD(                           \
        (level_),                                  \
        SDM_MEMMGR_LOG_IDX_CREATION_FAILED_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create lock used to protect the MemAdmin
 * \ingroup NETIOLogCodesClass
 */
#define SDM_LOG_ILLEGAL_BUFFER_STATE_EC                      (SDM_LOG_BASE + 22)
#define SDM_LOG_ILLEGAL_BUFFER_STATE(level_) \
    OSAPI_LOG_ENTRY_ADD(                     \
        (level_),                            \
        SDM_LOG_ILLEGAL_BUFFER_STATE_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create lock used to protect the MemPool
 * \ingroup NETIOLogCodesClass
 */
#define SDM_LOG_MEMPOOL_LOCK_CREATION_FAILED_EC              (SDM_LOG_BASE + 23)
#define SDM_LOG_MEMPOOL_LOCK_CREATION_FAILED(level_) \
    OSAPI_LOG_ENTRY_ADD(                             \
        (level_),                                    \
        SDM_LOG_MEMPOOL_LOCK_CREATION_FAILED_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief DataReader attempted to attach to a segment after receiving
 * a shared memory reference however, it was unable to evict any existing
 * segment out of the current cache
 */
#define SDM_LOG_MEMMGR_OUT_OF_SEGMENTS_EC                    (SDM_LOG_BASE + 24)
#define SDM_LOG_MEMMGR_OUT_OF_SEGMENTS(level_) \
    OSAPI_LOG_ENTRY_ADD(                       \
        (level_),                              \
        SDM_LOG_MEMMGR_OUT_OF_SEGMENTS_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief DataReader attempted to attach to a segment after receiving
 * a shared memory reference however, it was unable to evict any existing
 * segment out of the current cache
 */
#define SDM_LOG_DW_MEMMGR_SEGMENT_ALLOC_FAILED_EC            (SDM_LOG_BASE + 25)
#define SDM_LOG_DW_MEMMGR_SEGMENT_ALLOC_FAILED(level_) \
    OSAPI_LOG_ENTRY_ADD(                               \
        (level_),                                      \
        SDM_LOG_DW_MEMMGR_SEGMENT_ALLOC_FAILED_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failure when trying to get time for shared memory segment epoch
 * \ingroup NETIOLogCodesClass
 */
#define SDM_LOG_FAILED_TO_GET_TIME_EC                        (SDM_LOG_BASE + 26)
#define SDM_LOG_FAILED_TO_GET_TIME(level_) \
    OSAPI_LOG_ENTRY_ADD(                   \
        (level_),                          \
        SDM_LOG_FAILED_TO_GET_TIME_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Address residing in shared memory segment not
 * \ingroup NETIOLogCodesClass
 */
#define SDM_LOG_SEGMENT_NOT_ALIGNED_EC                       (SDM_LOG_BASE + 27)
#define SDM_LOG_SEGMENT_NOT_ALIGNED(level_) \
    OSAPI_LOG_ENTRY_ADD(                    \
        (level_),                           \
        SDM_LOG_SEGMENT_NOT_ALIGNED_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to delete mempool
 * \ingroup NETIOLogCodesClass
 */
#define SDM_LOG_DW_MEMMGR_MEMPOOL_DELETION_FAILED_EC         (SDM_LOG_BASE + 28)
#define SDM_LOG_DW_MEMMGR_MEMPOOL_DELETION_FAILED(level_) \
    OSAPI_LOG_ENTRY_ADD(                                  \
        (level_),                                         \
        SDM_LOG_DW_MEMMGR_MEMPOOL_DELETION_FAILED_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create mempool
 * \ingroup NETIOLogCodesClass
 */
#define SDM_LOG_DW_MEMMGR_MEMPOOL_CREATION_FAILED_EC         (SDM_LOG_BASE + 29)
#define SDM_LOG_LW_MEMMGR_MEMPOOL_CREATION_FAILED(level_) \
    OSAPI_LOG_ENTRY_ADD(                                  \
        (level_),                                         \
        SDM_LOG_DW_MEMMGR_MEMPOOL_CREATION_FAILED_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Datawriter Shared Data manager out of available segments
 * \ingroup NETIOLogCodesClass
 */
#define SDM_LOG_DW_OUT_OF_AVAILABLE_SEGMENTS_EC              (SDM_LOG_BASE + 30)
#define SDM_LOG_DW_OUT_OF_AVAILABLE_SEGMENTS(level_) \
    OSAPI_LOG_ENTRY_ADD(                             \
        (level_),                                    \
        SDM_LOG_DW_OUT_OF_AVAILABLE_SEGMENTS_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to evict a mapped segment out of the DataReader Shared Data
 * \ingroup NETIOLogCodesClass
 */
#define SDM_LOG_EVICTION_FAILED_EC                           (SDM_LOG_BASE + 31)
#define SDM_LOG_EVICTION_FAILED(level_) \
    OSAPI_LOG_ENTRY_ADD(                \
        (level_),                       \
        SDM_LOG_EVICTION_FAILED_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief A segment is being evicted from the DataReader shared data manager.
 * This means that we reached the maximum allowable segments that a DataReader
 * can attach to. This is similar to "thrashing" and will cause degraded
 * performance
 *
 * \ingroup NETIOLogCodesClass
 */
#define SDM_LOG_SEGMENT_EVICTED_EC                           (SDM_LOG_BASE + 32)
#define SDM_LOG_SEGMENT_EVICTED(level_, segment_max_cnt_) \
    OSAPI_LOG_ENTRY_ADD_1INT(                             \
        (level_),                                         \
        SDM_LOG_SEGMENT_EVICTED_EC,                       \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,                    \
        "dbrc",                                           \
        (segment_max_cnt_))

/*e
 * \brief Incompatible segment cookie
 * \ingroup NETIOLogCodesClass
 */
#define SDM_LOG_SEGMENT_COOKIE_EC                            (SDM_LOG_BASE + 33)
#define SDM_LOG_SEGMENT_COOKIE(level_) \
    OSAPI_LOG_ENTRY_ADD(               \
        (level_),                      \
        SDM_LOG_SEGMENT_COOKIE_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Incompatible segment version
 * \ingroup NETIOLogCodesClass
 */
#define SDM_LOG_SEGMENT_VERSION_EC                           (SDM_LOG_BASE + 34)
#define SDM_LOG_SEGMENT_VERSION(level_) \
    OSAPI_LOG_ENTRY_ADD(                \
        (level_),                       \
        SDM_LOG_SEGMENT_VERSION_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to allocate a segment for a single sample
 * \ingroup NETIOLogCodesClass
 */
#define SDM_FAILED_ALLOCATE_SEGMENT_SINGLE_SAMPLE_EC         (SDM_LOG_BASE + 35)
#define SDM_FAILED_ALLOCATE_SEGMENT_SINGLE_SAMPLE(level_) \
    OSAPI_LOG_ENTRY_ADD(                                  \
        (level_),                                         \
        SDM_FAILED_ALLOCATE_SEGMENT_SINGLE_SAMPLE_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to unmap segment from DataReader shared memory manager
 * \ingroup NETIOLogCodesClass
 */
#define SDM_FAILED_TO_UNMAP_SEGMENT_EC                       (SDM_LOG_BASE + 36)
#define SDM_FAILED_TO_UNMAP_SEGMENT(level_) \
    OSAPI_LOG_ENTRY_ADD(                    \
        (level_),                           \
        SDM_FAILED_TO_UNMAP_SEGMENT_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Unexpected situation when decrementing a ref_count a reference
 * in use by a DataReader
 * \ingroup NETIOLogCodesClass
 */
#define SDM_DECREMENT_REFERENCE_EC                           (SDM_LOG_BASE + 37)
#define SDM_DECREMENT_REFERENCE(level_) \
    OSAPI_LOG_ENTRY_ADD(                \
        (level_),                       \
        SDM_DECREMENT_REFERENCE_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Size of user sample is exceeding the 2 GB limit
 * in use by a DataReader
 * \ingroup NETIOLogCodesClass
 */
#define SDM_USER_TYPE_TOO_BIG_EC                             (SDM_LOG_BASE + 38)
#define SDM_USER_TYPE_TOO_BIG(level_, requested_, max_) \
    OSAPI_LOG_ENTRY_ADD_2INT(                           \
        (level_),                                       \
        SDM_USER_TYPE_TOO_BIG_EC,                       \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,                  \
        "req",                                          \
        (requested_),                                   \
        "max",                                          \
        (max_))

/*e
 * \brief Error occurred during serialization
 * \ingroup NETIOLogCodesClass
 */
#define SDM_LOG_SERIALIZE_EC                               (SDM_LOG_BASE + 39)
#define SDM_LOG_SERIALIZE(level_) \
    OSAPI_LOG_ENTRY_ADD(            \
        (level_),                   \
        SDM_LOG_SERIALIZE_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \ingroup NETIOLogCodesClass
 */
#define SDM_LOG_DESERIALIZE_EC                               (SDM_LOG_BASE + 40)
#define SDM_LOG_DESERIALIZE(level_) \
    OSAPI_LOG_ENTRY_ADD(            \
        (level_),                   \
        SDM_LOG_DESERIALIZE_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief  Something went wrong when using sample state. This should never go wrong
 * \ingroup NETIOLogCodesClass
 */
#define SDM_LOG_SAMPLE_STATE_EC                              (SDM_LOG_BASE + 41)
#define SDM_LOG_SAMPLE_STATE(level_) \
    OSAPI_LOG_ENTRY_ADD(            \
        (level_),                   \
        SDM_LOG_SAMPLE_STATE_EC,     \
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

#endif /* netio_log_h */
