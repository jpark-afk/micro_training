/*
 * FILE: reda_bufferpool.c - BufferPool interface
 *
 * Copyright 2008-2021 Real-Time Innovations, Inc.
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
 * 11mar2013,tk MICRO-171/PR#1051
 * 25may2012,tk Written
 *
 */
/*ci
 * \file
 *
 * \brief The REDA BufferPool interface provides a platform independent API to
 *        manage a pool of fixed size elements.
 *
 * \defgroup REDA_BufferPoolClass REDA BufferPool
 * \ingroup REDAModule
 *
 * \details
 *
 * The REDA BufferPool interface defines an API manage a fixed number of
 * fixed sized elements. It is important to note that the number of elements
 * in the pool cannot grow beyond what was specified when the pool was created.
 * Callbacks can be installed to be notified when the buffer pool allocates
 * a new buffer or free a buffer.
 */

/*ci \addtogroup REDA_BufferPoolClass
 *   @{
 */
#ifndef reda_bufferpool_h
#define reda_bufferpool_h

#ifndef reda_dll_h
#include "reda/reda_dll.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*ci
 * \brief REDA_Buffer pool properties specified when a buffer pool is created
 */
struct REDA_BufferPoolProperty
{
    /*ci
     * \brief The size of an element in the buffer pool. Note that in case the
     * buffer contains pointers to other memory location, the size must be
     * the size of the top-level structure only.
     */
    RTI_SIZE_T buffer_size;

    /*ci
     * \brief The maximum number of buffers managed by the pool
     */
    RTI_SIZE_T max_buffers;

    /*ci
     * \brief Additional flags to specify the behavior of the pool
     */
    RTI_UINT32 flags;
};

/*ci
 * \def REDA_BUFFERPOOL_FLAG_ARRAY_ALLOC
 * \brief Create a single array of elements. This could improve memory
 *        utilization for smaller sizes
 */
#define REDA_BUFFERPOOL_FLAG_ARRAY_ALLOC (0x1U)

/*ci
 * \def REDA_BufferPoolProperty_INITIALIZER
 * \brief REDA_BufferPoolProperty initializer
 */
#define REDA_BufferPoolProperty_INITIALIZER \
{ \
    0,\
    0,\
    0\
}

/*ci
 * \def REDA_BUFFERPOOL_UNLIMITED
 * \brief Constant to indicate an UNLIMITED buffer-pool. Specifying this constant
 *        will result in a run-time error.
 */
#define REDA_BUFFERPOOL_UNLIMITED (0xffffffffU)

struct REDA_BufferPool;

/*ci
 * \brief Abstract REDA_BufferPool type
 */
typedef struct REDA_BufferPool *REDA_BufferPool_T;

/*ci
 *
 * \brief REDA_BufferPool initialization function type.
 *
 * \details
 *
 * For each buffer that is allocated from system memory
 * REDA buffer calls this function exactly once.
 *
 * \param[in] initialize_param Parameter passed to \ref REDA_BufferPool_new
 * \param[in] buffer           Buffer to initialize
 *
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*REDA_BufferPool_initializeFunc_T)(void *initialize_param, void *buffer)
)

/*ci
 *
 * \brief REDA_BufferPool finalization type
 *
 * \details
 * For each buffer that is returned to system memory REDA buffer calls
 * this function exactly once
 *
 * \param[in] finalize_param Parameter passed to \ref REDA_BufferPool_new
 * \param[in] buffer         Buffer to finalize
 */
FUNCTION_SHOULD_TYPEDEF(
RTI_BOOL
(*REDA_BufferPool_finalizeFunc_T)(void *finalize_param, void *buffer)
)

/*ci
 * \brief Create a new bufferpool
 *
 * \param[in] The name of the buffer-pool, used for debugging
 * \param[in] property BufferPool property. Refer to \ref
 *            REDA_BufferPoolProperty for details.
 * \param[in] initialize_func Buffer initialization function. This function
 *            is called exactly once for each buffer managed by the pool. NULL
 *            is legal, in which case a buffer is not guaranteed to have a
 *            known state.
 * \param[in] initialize_param Opaque parameter passed to the initialize_func
 * \param[in] finalize_func Buffer finalizing function. This function is called
 *            exactly once before a buffer before it is returned to system
 *            memory. NULL is legal, in which case the buffer is returned
 *            as is to the system memory.
 * \param[in] finalize_param Opaque parameter passed to the finalize_func
 *
 * \return Pointer to new buffer pool on success, NULL on failure
 *
 * \sa REDA_BufferPool_delete
 *
 */
MUST_CHECK_RETURN REDADllExport REDA_BufferPool_T
REDA_BufferPool_new(const char* name,
                    struct REDA_BufferPoolProperty *property,
                    REDA_BufferPool_initializeFunc_T initialize_func,
                    void *initialize_param,
                    REDA_BufferPool_finalizeFunc_T finalize_func,
                    void *finalize_param);

#ifndef RTI_CERT
/*ci
 *
 * \brief Delete a bufferpool
 *
 * \details
 *
 * Return all buffers managed by the buffer-pool to system memory. It is not
 * legal to delete a buffer-pool with outstanding buffers.
 *
 * \param[in] pool BufferPool to delete
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * \sa REDA_BufferPool_delete
 */
SHOULD_CHECK_RETURN REDADllExport RTI_BOOL
REDA_BufferPool_delete(REDA_BufferPool_T pool);
#endif /* !RTI_CERT */

/*ci
 * \brief Get a buffer from the buffer pool
 *
 * \details
 *
 * Take a buffer out of the free-pool and return it to the caller for use.
 * Note that the buffer returned from the pool has only been initialized with
 * the initialize_func passed to the REDA_BufferPool_new when the pool was
 * created.
 *
 * \param[in] pool BufferPool to get buffer from
 *
 * \return pointer to buffer on success, NULL on failure
 *
 * \sa REDA_BufferPool_return_buffer
 */
MUST_CHECK_RETURN REDADllExport void*
REDA_BufferPool_get_buffer(REDA_BufferPool_T pool);

/*ci
 * \brief Return a buffer to the buffer-pool
 *
 * \details
 *
 * Return a buffer to the buffer-pool and make it available for future
 * calls to \ref REDA_BufferPool_get_buffer. Note that the finalize_func
 * is <em> not </em> called when a buffer is returned to the pool.
 *
 * \param[in] pool   BufferPool to return buffer to
 * \param[in] buffer buffer to return to pool
 *
 * \return pointer to buffer on success, NULL on failure
 *
 * \sa REDA_BufferPool_return_buffer
 */
REDADllExport void
REDA_BufferPool_return_buffer(REDA_BufferPool_T pool,void *buffer);

#ifdef __cplusplus
}                               /* extern "C" */
#endif


#endif /* reda_bufferpool_h */

/*ci @} */
