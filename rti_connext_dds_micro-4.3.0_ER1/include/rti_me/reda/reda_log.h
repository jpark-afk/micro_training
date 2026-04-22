/*
 * FILE: reda_log.h - Log codes
 *
 * Copyright 2012-2025 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 05sep2012,tk Written
 */
/*e
 * \file 
 * \brief REDA module log codes
 */
#ifndef reda_log_h
#define reda_log_h

#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif

/*e
 * \defgroup REDALogCodesClass REDA
 * \brief Real-time Efficient Data Structures and Algorithms. ModuleID = 1
 * \ingroup LoggingModule
 */

/*e
 * \brief Sample buffer pool object
 * \ingroup REDALogCodesClass
 */
#define REDA_LOG_HEAP_MGR_SAMPLE_BUFFERPOOL              1

/*e
 * \brief Listnode buffer pool object
 * \ingroup REDALogCodesClass
 */
#define REDA_LOG_HEAP_MGR_LISTNODE_BUFFERPOOL            2

/*e
 * \brief Heap manager object
 * \ingroup REDALogCodesClass
 */
#define REDA_LOG_HEAP_MGR_OBJECT_MEMORY                  3

/*e
 * \brief REDA indexer object
 * \ingroup REDALogCodesClass
 */
#define REDA_LOG_HEAP_MGR_INDEXER                        4

/*e
 * \brief REDA heap manager object
 * \ingroup REDALogCodesClass
 */
#define REDA_LOG_HEAP_MGR_MUTEX                          5

/*e
 * \brief Not sufficient memory to allocate buffer-pool
 * \ingroup REDALogCodesClass
 */
#define REDA_LOG_BUFFERPOOL_OUT_OF_RESOURCES_EC              (REDA_LOG_BASE + 1)
#define REDA_LOG_BUFFERPOOL_OUT_OF_RESOURCES(level_) \
OSAPI_LOG_ENTRY_ADD((level_),REDA_LOG_BUFFERPOOL_OUT_OF_RESOURCES_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief The buffer initialization method failed
 * \ingroup REDALogCodesClass
 */
#define REDA_LOG_BUFFERPOOL_BUFFER_INITIALIZATION_FAILED_EC  (REDA_LOG_BASE + 2)
#define REDA_LOG_BUFFERPOOL_BUFFER_INITIALIZATION_FAILED(level_,if_,ip_) \
OSAPI_LOG_ENTRY_CREATE((level_),REDA_LOG_BUFFERPOOL_BUFFER_INITIALIZATION_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_POINTER("if",(if_),RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_POINTER("ip",(ip_),RTI_TRUE)

/*e
 * \brief Cannot delete buffer-pool due to buffer(s) not returned to pool
 * \ingroup REDALogCodesClass
 */
#define REDA_LOG_BUFFERPOOL_NOT_EMPTY_EC                     (REDA_LOG_BASE + 3)
#define REDA_LOG_BUFFERPOOL_NOT_EMPTY(level_,count_) \
OSAPI_LOG_ENTRY_ADD_1UINT((level_),REDA_LOG_BUFFERPOOL_NOT_EMPTY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"count",(count_))

/*e
 * \brief A buffer was freed more than once
 * \ingroup REDALogCodesClass
 */
#define REDA_LOG_BUFFERPOOL_DOUBLE_FREE_EC                   (REDA_LOG_BASE + 4)
#define REDA_LOG_BUFFERPOOL_DOUBLE_FREE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),REDA_LOG_BUFFERPOOL_DOUBLE_FREE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Inconsistent properties specified for a REDA_MemPool
 * \ingroup REDALogCodesClass
 */
#define REDA_LOG_MEMPOOL_INCONSISTENT_PROPERTIES_EC          (REDA_LOG_BASE + 5)
#define REDA_LOG_MEMPOOL_INCONSISTENT_PROPERTIES(level_) \
OSAPI_LOG_ENTRY_ADD((level_),REDA_LOG_MEMPOOL_INCONSISTENT_PROPERTIES_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Insufficient memory to allocate a new REDA_MemPool
 * \ingroup REDALogCodesClass
 */
#define REDA_LOG_MEMPOOL_OUT_OF_RESOURCES_EC                 (REDA_LOG_BASE + 6)
#define REDA_LOG_MEMPOOL_OUT_OF_RESOURCES(level_) \
OSAPI_LOG_ENTRY_ADD((level_),REDA_LOG_MEMPOOL_OUT_OF_RESOURCES_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Error during allocation of the memory pool used by a REDA_MemPool
 * \ingroup REDALogCodesClass
 */
#define REDA_LOG_MEMPOOL_POOL_ALLOCATION_EC                  (REDA_LOG_BASE + 7)
#define REDA_LOG_MEMPOOL_POOL_ALLOCATION(level_) \
OSAPI_LOG_ENTRY_ADD((level_),REDA_LOG_MEMPOOL_POOL_ALLOCATION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief A pointer in a pool's linked list was NULL
 * \ingroup REDALogCodesClass
 */
#define REDA_LOG_BUFFERPOOL_NULL_POINTER_EC                  (REDA_LOG_BASE + 8)
#define REDA_LOG_BUFFERPOOL_NULL_POINTER(level_) \
OSAPI_LOG_ENTRY_ADD((level_),REDA_LOG_BUFFERPOOL_NULL_POINTER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief An error occurred while copying a sequence
 * \ingroup REDALogCodesClass
 */
#define REDA_LOG_SEQUENCE_COPY_FAILED_EC                   (REDA_LOG_BASE + 100)
#define REDA_LOG_SEQUENCE_COPY_FAILED(level_,dstbuf_,srcbuf_,len_,esize_) \
OSAPI_LOG_ENTRY_CREATE((level_),REDA_LOG_SEQUENCE_COPY_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_POINTER("dstbuf",(dstbuf_),RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_POINTER("srcbuf",(srcbuf_),RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_INT("len",(len_),RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_INT("esize",(esize_),RTI_TRUE)

/*e
 * \brief An error occurred while allocating space for a sequence
 * \ingroup REDALogCodesClass
 */
#define REDA_LOG_SEQUENCE_ALLOC_FAILED_EC                  (REDA_LOG_BASE + 101)
#define REDA_LOG_SEQUENCE_ALLOC_FAILED(level_,size_,align_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),REDA_LOG_SEQUENCE_ALLOC_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "size",(size_),"align",(align_))

/*e
 * \brief An error occurred while setting the maximum sequence size
 * \ingroup REDALogCodesClass
 */
#define REDA_LOG_SEQUENCE_SET_MAX_FAILED_EC                (REDA_LOG_BASE + 102)
#define REDA_LOG_SEQUENCE_SET_MAX_FAILED(level_,self_,max_) \
OSAPI_LOG_ENTRY_CREATE((level_),REDA_LOG_SEQUENCE_SET_MAX_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_POINTER("self",(self_),RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_INT("max",(max_),RTI_TRUE)

/*e
 * \brief An attempt was made to resize an already allocated sequence
 * \ingroup REDALogCodesClass
 */
#define REDA_LOG_SEQUENCE_REALLOCATION_EC                  (REDA_LOG_BASE + 103)
#define REDA_LOG_SEQUENCE_REALLOCATION(level_) \
OSAPI_LOG_ENTRY_ADD((level_),REDA_LOG_SEQUENCE_REALLOCATION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM )

/*e
 * \brief An error occurred while operating on two sequences (copy/compare)
 * \ingroup REDALogCodesClass
 */
#define REDA_LOG_SEQUENCE_INVALID_OPERATION_EC             (REDA_LOG_BASE + 104)
#define REDA_LOG_SEQUENCE_INVALID_OPERATION(level_,llen_,rlen_,lesz_,resz_) \
OSAPI_LOG_ENTRY_CREATE((level_),REDA_LOG_SEQUENCE_INVALID_OPERATION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_INT("llen",(llen_),RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_INT("rlen",(rlen_),RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_INT("lesz",(lesz_),RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_INT("resz",(resz_),RTI_TRUE)

/*e
 * \brief An attempt was made to set an illegal length (length < 0
 *        or length > max_length)
 * \ingroup REDALogCodesClass
 */
#define REDA_LOG_SEQUENCE_INVALID_LENGTH_EC                (REDA_LOG_BASE + 105)
#define REDA_LOG_SEQUENCE_INVALID_LENGTH(level_) \
OSAPI_LOG_ENTRY_ADD((level_),REDA_LOG_SEQUENCE_INVALID_LENGTH_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief An illegal sequence index was specified (index < 0 or index > length)
 * \ingroup REDALogCodesClass
 */
#define REDA_LOG_SEQUENCE_INDEX_OUT_OF_BOUNDS_EC           (REDA_LOG_BASE + 106)
#define REDA_LOG_SEQUENCE_INDEX_OUT_OF_BOUNDS(level_) \
OSAPI_LOG_ENTRY_ADD((level_),REDA_LOG_SEQUENCE_INDEX_OUT_OF_BOUNDS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief An error occurred while allocating memory for a string
 * \ingroup REDALogCodesClass
 */
#define REDA_LOG_STRING_ALLOC_FAILED_EC                    (REDA_LOG_BASE + 200)
#define REDA_LOG_STRING_ALLOC_FAILED(level_,size_) \
OSAPI_LOG_ENTRY_ADD_1UINT((level_),REDA_LOG_STRING_ALLOC_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"size",(size_))

/*e
 * \brief An attempt was made to add an element to a full index
 * \ingroup REDALogCodesClass
 */
#define REDA_LOG_INDEX_FULL_EC                             (REDA_LOG_BASE + 300)
#define REDA_LOG_INDEX_FULL(level_) \
OSAPI_LOG_ENTRY_ADD((level_),REDA_LOG_INDEX_FULL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief An attempt was made to add an existing element to an index
 * \ingroup REDALogCodesClass
 */
#define REDA_LOG_INDEX_ENTRY_EXISTS_EC                     (REDA_LOG_BASE + 301)
#define REDA_LOG_INDEX_ENTRY_EXISTS(level_) \
OSAPI_LOG_ENTRY_ADD((level_),REDA_LOG_INDEX_ENTRY_EXISTS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief An invalid list node was encountered
 * \ingroup REDALogCodesClass
 */
#define REDA_LOG_INVALID_LIST_NODE_EC                      (REDA_LOG_BASE + 302)
#define REDA_LOG_INVALID_LIST_NODE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),REDA_LOG_INVALID_LIST_NODE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to allocate buffer of a specified kind
 * \ingroup REDALogCodesClass
 */
#define REDA_LOG_BUFFER_OUT_OF_MEMORY_EC                   (REDA_LOG_BASE + 307)
#define REDA_LOG_BUFFER_OUT_OF_MEMORY(level_) \
OSAPI_LOG_ENTRY_ADD((level_),REDA_LOG_BUFFER_OUT_OF_MEMORY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Allocation error in REDA Heap Manager
 * \ingroup REDALogCodesClass
 */
#define REDA_LOG_HEAP_MGR_ALLOC_EC                         (REDA_LOG_BASE + 308)
#define REDA_LOG_HEAP_MGR_ALLOC(level_, kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),REDA_LOG_HEAP_MGR_ALLOC_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_))

/*e
 * \brief Error when there are no more available loanable buffers
 * \ingroup REDALogCodesClass
 */
#define REDA_LOG_HEAP_MGR_GET_BUFFER_EC                    (REDA_LOG_BASE + 309)
#define REDA_LOG_HEAP_MGR_GET_BUFFER(level_) \
OSAPI_LOG_ENTRY_ADD((level_),REDA_LOG_HEAP_MGR_GET_BUFFER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)
/*e
 * \brief Error during locking or unlocking of a mutex
 * \ingroup REDALogCodesClass
 */
#define REDA_LOG_HEAP_MGR_MUTEX_FAILURE_EC                 (REDA_LOG_BASE + 310)
#define REDA_LOG_HEAP_MGR_MUTEX_FAILURE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),REDA_LOG_HEAP_MGR_MUTEX_FAILURE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)


#endif
