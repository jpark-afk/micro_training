/*
 * FILE: REDABufferPool.h - BufferPool interface
 *
 * Copyright 2012-2021 Real-Time Innovations, Inc.
 * 
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 * 25may2012,tk Written
 *
 */
#ifndef REDABufferPool_h
#define REDABufferPool_h

#define REDA_BUFFERPOOL_FLAGS_FULL 0x00010000

/*ci \addtogroup REDA_BufferPoolClass
 * @{
 */
struct REDA_BufferPool;

/*ci
 * \brief Each buffer-pool entry has a header with a pointer to the
 *        next entry in the pool
 */
struct REDA_BufferPoolEntry
{
    /*ci
     * \brief Pointer to next available buffer
     */
    struct REDA_BufferPoolEntry *_next;
};

/*ci
 * \brief Concrete implementation of the buffer-pool
 */
struct REDA_BufferPool
{
    RTI_UINT32 flags;

#if OSAPI_ENABLE_PRECONDITION
    /*ci
     * \brief The properties the buffer-pool was created with
     */
    struct REDA_BufferPoolProperty property;
#endif
    /*ci
     * \brief Link to the first available entry in the buffer-pool
     */
    struct REDA_BufferPoolEntry buffer_pool;

#if OSAPI_ENABLE_PRECONDITION
    /*ci
     * \brief Current number of allocated buffers in the pool
     */
    RTI_SIZE_T allocated_count;
#endif

    /*ci
     * \brief Pointer to the allocated memory for the pool
     */
    void *block_alloc;

    /*ci
     * \brief Function to call on each buffer in the pool before the
     * buffer pool is deleted
     */
    REDA_BufferPool_finalizeFunc_T finalize_func;

    /*ci
     * \brief Parameter to pass to finalize_func
     */
    void *finalize_param;
};

struct REDA_BufferPoolBasic
{
    RTI_UINT32 flags;
#if OSAPI_ENABLE_PRECONDITION
    /*ci
     * \brief The properties the buffer-pool was created with
     */
    struct REDA_BufferPoolProperty property;
#endif
    /*ci
     * \brief Link to the first available entry in the buffer-pool
     */
    struct REDA_BufferPoolEntry buffer_pool;

#if OSAPI_ENABLE_PRECONDITION
    /*ci
     * \brief Current number of allocated buffers in the pool
     */
    RTI_SIZE_T allocated_count;
#endif

    /*ci
     * \brief Pointer to the allocated memory for the pool
     */
    void *block_alloc;
};

/*ci @} */

#endif
