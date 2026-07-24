/*
 * FILE: XCDRHeapMgr.h
 *
 * Copyright (c) 2018-2025 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef XCDRHeapMgr_h
#define XCDRHeapMgr_h

#ifndef osapi_config_h
#include "osapi/osapi_config.h"
#endif
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif
#ifndef reda_log_h
#include "reda/reda_log.h"
#endif
#ifndef xcdr_heap_mgr_h
#include "xcdr/xcdr_heapmgr.h"
#endif
#ifndef reda_buffer_pool_h
#include "reda/reda_bufferpool.h"
#endif
#ifndef osapi_mutex_h
#include "osapi/osapi_mutex.h"
#endif

typedef enum 
{
    XCDR_SAMPLE_HEADER_STATE_FREE,
    XCDR_SAMPLE_HEADER_STATE_LOANED,
    XCDR_SAMPLE_HEADER_STATE_SERIALIZED,
    XCDR_SAMPLE_HEADER_STATE_REMOVED
} XCDR_HeapMgrBufferState; 

struct XCDR_HeapMgrBufferListNode
{
    struct REDA_CircularListNode _node;
    void *address;
};

struct XCDR_HeapMgrBufferHeader
{
#ifdef RTI_64BIT
    void *owner_private_addr;                                  /* 8 bytes (8)  */
#else /* 32-bit */
    void *owner_private_addr;                                  /* 4 bytes (4)  */
    RTI_INT32 padding_1;                                       /* 8 bytes (8)  */
#endif
    RTI_INT32 state;                                           /* 8 bytes (16) */
    RTI_INT32 old_state;                                       /* 8 bytes (16) */
};

struct XCDR_HeapMgr
{
    struct REDA_Indexer *heap_sample_indexer;
    struct REDA_BufferPool *heap_sample_pool;
    struct REDA_BufferPool *heap_buf_list_node_pool;
    REDA_CircularList_T free_list;
    OSAPI_Mutex_T *free_list_lock;
    XCDR_HeapMgr_gen_init initialize_func;
    XCDR_HeapMgr_gen_init finalize_func;
};

#endif /* REDAHeapMgr_h */
