/*
 * FILE: REDABufferPool.c - BufferPool implementation
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
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 * 02feb2015,tk MICRO-1050/PR#13578 Added extra robustness check for double free
 * 01dec2014,tk MICRO-948/PR#12307  Removed redundant if test for CERT
 * 28jul2014,tk MICRO-835/PR#9619   Added REDA_Log_bufferpool_out_of_resources
 * 28jul2014,tk MICRO-836/PR#9620   Call finalize_func for Cert
 * 11mar2013,tk MICRO-171/PR#1051
 * 25may2012,tk Written
 */
/*ce
 * \file
 * \brief Implementation of REDA BufferPool API
 */
#ifndef osapi_config_h
#include "osapi/osapi_config.h"
#endif
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif
#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef reda_log_h
#include "reda/reda_log.h"
#endif
#ifndef reda_bufferpool_h
#include "reda/reda_bufferpool.h"
#endif

#include "REDABufferPool.h"

/* Convenience macro to ensure than a buffer is aligned to a specific size
 */
#define REDA_BUFFERPOOL_HEADER_SIZE (sizeof(struct REDA_BufferPoolEntry))
#define REDA_BUFFERPOOL_ALIGNUP(size_) \
(RTI_SIZE_T)(((((size_) + REDA_BUFFERPOOL_HEADER_SIZE) & (sizeof(void*)-1)) ? \
 (((size_) + REDA_BUFFERPOOL_HEADER_SIZE + sizeof(void*)) & ~(sizeof(void*)-1))\
  : (size_) + REDA_BUFFERPOOL_HEADER_SIZE))

/*** SOURCE_BEGIN ***/

REDA_BufferPool_T
REDA_BufferPool_new(const char *name,struct REDA_BufferPoolProperty *property,
                   REDA_BufferPool_initializeFunc_T initialize_func,
                   void *initialize_param,
                   REDA_BufferPool_finalizeFunc_T finalize_func,
                   void *finalize_param)
{
    struct REDA_BufferPool *pool = NULL;
    RTI_SIZE_T real_size,alloc_size;
    struct REDA_BufferPoolEntry *pool_entry;
    RTI_SIZE_T i;
    UNUSED_ARG(name);

    OSAPI_PRECONDITION((property == NULL) || (property->buffer_size == 0) ||
                     (property->max_buffers == 0) ||
                     (property->buffer_size == REDA_BUFFERPOOL_UNLIMITED) ||
                     (property->max_buffers == REDA_BUFFERPOOL_UNLIMITED),
                     goto done,
                     OSAPI_Log_entry_add_pointer("property",property,RTI_FALSE);
                     OSAPI_Log_entry_add_uint("property->buffer_size",
                       property != NULL ? property->buffer_size : 0,RTI_FALSE);
                     OSAPI_Log_entry_add_uint("property->max_buffers",
                       property != NULL ? property->max_buffers : 0,RTI_TRUE);)

    if (finalize_func == NULL)
    {
        pool = (struct REDA_BufferPool*)OSAPI_Heap_allocate(1,sizeof(struct REDA_BufferPoolBasic));
    }
    else
    {
        pool = (struct REDA_BufferPool*)OSAPI_Heap_allocate(1,sizeof(struct REDA_BufferPool));
    }

    if (pool == NULL)
    {
        REDA_LOG_BUFFERPOOL_OUT_OF_RESOURCES(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    /* In an empty list the head points to itself */
    pool->buffer_pool._next = &pool->buffer_pool;

    pool->flags = property->flags;

    if (finalize_func != NULL)
    {
        pool->flags |= REDA_BUFFERPOOL_FLAGS_FULL;
        pool->finalize_func = finalize_func;
        pool->finalize_param = finalize_param;
    }

#if OSAPI_ENABLE_PRECONDITION
    pool->allocated_count = 0;
#endif

    pool->block_alloc = NULL;

    /* Align the real-size up to be able to hold the largest type (void*) */
    real_size = REDA_BUFFERPOOL_ALIGNUP(property->buffer_size);
    alloc_size = real_size;

    if (property->flags & REDA_BUFFERPOOL_FLAG_ARRAY_ALLOC)
    {
        alloc_size *= property->max_buffers;
    }

    OSAPI_Heap_allocate_buffer((char**)&pool->buffer_pool._next,
                               alloc_size,OSAPI_ALIGNMENT_DEFAULT);
    if (pool->buffer_pool._next == NULL)
    {
        REDA_LOG_BUFFERPOOL_OUT_OF_RESOURCES(OSAPI_LOGKIND_ERROR)
        goto failure;
    }
    pool->block_alloc = pool->buffer_pool._next;
    if (initialize_func &&
        !initialize_func(initialize_param,&pool->buffer_pool._next[1]))
    {
        REDA_LOG_BUFFERPOOL_BUFFER_INITIALIZATION_FAILED(
                                            OSAPI_LOGKIND_ERROR,
                                            initialize_func != NULL ? (void*)0x1 : NULL,
                                            initialize_param != NULL ? (void*)0x1 : NULL)
        goto failure;
    }

    pool_entry = pool->buffer_pool._next;
    for (i = 1; i < property->max_buffers; ++i)
    {
        if (property->flags & REDA_BUFFERPOOL_FLAG_ARRAY_ALLOC)
        {
	        char *next_pool_entry = (char*)pool_entry + real_size;
	        pool_entry->_next = OSAPI_Compiler_reinterpret_cast(
                                            struct REDA_BufferPoolEntry*,
                                            next_pool_entry);
        }
        else
        {
            OSAPI_Heap_allocate_buffer((char**)&pool_entry->_next,
                                     real_size,
                                     OSAPI_ALIGNMENT_DEFAULT);
            if (pool_entry->_next == NULL)
            {
                REDA_LOG_BUFFERPOOL_OUT_OF_RESOURCES(OSAPI_LOGKIND_ERROR)
                goto failure;
            }
        }
        if (initialize_func &&
            !initialize_func(initialize_param,&pool_entry->_next[1]))
        {
            REDA_LOG_BUFFERPOOL_BUFFER_INITIALIZATION_FAILED(
                                                OSAPI_LOGKIND_ERROR,
                                                initialize_func != NULL ? (void*)0x1 : NULL,
                                                initialize_param != NULL ? (void*)0x1 : NULL)
            goto failure;
        }
        pool_entry = pool_entry->_next;
    }

    /* Ensure that the last element points back to the head */
    pool_entry->_next = &pool->buffer_pool;

done:

    return pool;

failure:
#ifndef RTI_CERT
    (void)REDA_BufferPool_delete(pool);
#endif

    return NULL;
}

#ifndef RTI_CERT
RTI_BOOL
REDA_BufferPool_delete(REDA_BufferPool_T pool)
{
    RTI_BOOL retval = RTI_FALSE;
    struct REDA_BufferPoolEntry *pool_entry,*pool_entry_next;

    OSAPI_PRECONDITION(pool == NULL,return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("pool",pool,RTI_TRUE);)

#if OSAPI_ENABLE_PRECONDITION
    if (pool->allocated_count > 0)
    {
        REDA_LOG_BUFFERPOOL_NOT_EMPTY(OSAPI_LOGKIND_ERROR,pool->allocated_count)
        return RTI_FALSE;
    }
#endif

    pool_entry = pool->buffer_pool._next;
    while (pool_entry != &pool->buffer_pool)
    {
        if (pool_entry == NULL)
        {
            REDA_LOG_BUFFERPOOL_NULL_POINTER(OSAPI_LOGKIND_ERROR)
            goto done;
        }
        pool_entry_next = pool_entry->_next;
        if ((pool->flags & REDA_BUFFERPOOL_FLAGS_FULL) &&
             (pool->finalize_func != NULL))
        {
            pool->finalize_func(pool->finalize_param,(void*)&pool_entry[1]);
        }

        if (!(pool->flags & REDA_BUFFERPOOL_FLAG_ARRAY_ALLOC))
        {
            OSAPI_Heap_free_buffer(pool_entry);
        }

        pool_entry = pool_entry_next;
    }
    retval = RTI_TRUE;

done:
    if (pool->flags & REDA_BUFFERPOOL_FLAG_ARRAY_ALLOC)
    {
        OSAPI_Heap_free_buffer(pool->block_alloc);
    }

    OSAPI_Heap_free_struct(pool);

    return retval;
}
#endif /* !RTI_CERT */

void*
REDA_BufferPool_get_buffer(REDA_BufferPool_T pool)
{
    struct REDA_BufferPoolEntry *retval = NULL;

    OSAPI_PRECONDITION(pool == NULL,return NULL,
                            OSAPI_Log_entry_add_pointer("pool",pool,RTI_TRUE);)

    retval = (void*)pool->buffer_pool._next;
    if (retval == &pool->buffer_pool)
    {
        return NULL;
    }

    pool->buffer_pool._next = retval->_next;
    retval->_next = NULL;
#if OSAPI_ENABLE_PRECONDITION
    ++pool->allocated_count;
#endif

    return &retval[1];
}

#if REDA_BUFFERPOOL_TRACE_ENABLED
void
REDA_BufferPool_return_buffer_(REDA_BufferPool_T pool,void *buffer,
                              char *file,RTI_INT32 lineno)
#else
void
REDA_BufferPool_return_buffer(REDA_BufferPool_T pool,void *buffer)
#endif
{
    struct REDA_BufferPoolEntry *retval = (struct REDA_BufferPoolEntry *)buffer;

    OSAPI_PRECONDITION(((pool == NULL) || (buffer == NULL)),
                            return,
                            OSAPI_Log_entry_add_pointer("pool",pool,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("buffer",buffer,RTI_TRUE);)

    --retval;

    /* The next pointer is only set to NULL when it is removed from the list and
     * non-NULL when added back to the list. Thus, if it is non-NULL here
     * it must be a double free.
     */
    if (retval->_next != NULL)
    {
        REDA_LOG_BUFFERPOOL_DOUBLE_FREE(OSAPI_LOGKIND_ERROR)
        return;
    }

#if REDA_BUFFERPOOL_TRACE_ENABLED
    retval->last_file = file;
    retval->last_line_no = lineno;
#endif

    retval->_next = pool->buffer_pool._next;
    pool->buffer_pool._next = retval;
#if OSAPI_ENABLE_PRECONDITION
    --pool->allocated_count;
#endif
}
