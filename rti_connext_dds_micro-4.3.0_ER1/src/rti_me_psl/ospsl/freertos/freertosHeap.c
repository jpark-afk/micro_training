/*
 * FILE: freertosHeap.c - FreeRTOS heap functionality
 *
 * (c) Copyright, Real-Time Innovations 2024-2025
 *  
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

/*ce
 * \file
 * \brief FreeRTOS implementation of OSAPI heap routines
 */
#include "rti_me_psl.h"
#include "rti_me_psl/osapi/osapi_heap_test.h"

/*** SOURCE_BEGIN ***/

#ifndef RTI_CERT
/*ci
 * \brief
 * Global variable holding the number of bytes allocated.
 */
LINK_SECTION_BSS_SDRAM
RTI_SIZE_T OSAPI_gv_AllocatedByteCount = 0;

LINK_SECTION_BSS_SDRAM
RTI_BOOL OSAPI_Heap_gv_disabled_alloc = RTI_FALSE;
#endif /* !RTI_CERT */

#ifndef RTI_CERT
/*ci
 * \brief
 * Reallocate memory block
 *
 * \param[in] ptr Pointer to the memory block to be reallocated.
 * \param[in] size New size of the memory block in bytes.
 *
 * \return Pointer to the reallocated memory block on success, NULL on failure.
 */
void*
OSAPI_Heap_realloc(void *ptr, RTI_SIZE_T size)
{
#if !OSAPI_ENABLE_PVPORTREALLOC && !OSAPI_ENABLE_STDC_REALLOC
    char *buffer = NULL;
    uint8_t *block_data;
    BlockLink_t *block_data_p;
    RTI_SIZE_T copy_size = 0;
#endif

    if (OSAPI_Heap_gv_disabled_alloc)
    {
        return NULL;
    }

#if OSAPI_ENABLE_PVPORTREALLOC
    return pvPortRealloc(ptr, size);
#elif OSAPI_ENABLE_STDC_REALLOC
    return realloc(ptr, size);
#else

    /* If size is 0, act like free and return NULL */
    if (size == 0)
    {
        OSAPI_Heap_free_buffer(ptr);
        return NULL;
    }

    OSAPI_Heap_allocate_buffer(&buffer, size, 0);

    if (buffer == NULL)
    {
        OSAPI_LOG_HEAP_INTERNAL_ALLOCATE(OSAPI_LOGKIND_ERROR,size)
        return NULL;
    }

    /* Handle standard realloc behavior: if ptr is NULL, act like malloc */
    if (ptr == NULL)
    {
        return buffer;
    }

    block_data = (uint8_t *)ptr;
    block_data -= xHeapStructSize;
    block_data_p = (void *)block_data;

    /* Validate the block is allocated */
    if ((block_data_p->xBlockSize & xBlockAllocatedBit) != 0)
    {
        copy_size = (block_data_p->xBlockSize & ~xBlockAllocatedBit) - \
                   xHeapStructSize;
        /* Copy the minimum of old and new size */
        copy_size = (copy_size < size) ? copy_size : size;
        /* Copy data from old buffer to new buffer */
        OSAPI_Memory_copy(buffer, ptr, copy_size);
    }

    /* Free the old buffer */
    OSAPI_Heap_free_buffer(ptr);
    
    return buffer;

#endif /* OSAPI_ENABLE_PVPORTREALLOC */
}
#endif /* !RTI_CERT */


#ifndef RTI_CERT
/*ci
 * \brief
 * Disable memory allocation
 *
 * \details
 * This function disables memory allocation by setting a global flag.
 */
void
OSAPI_Heap_disable_alloc(void)
{
    OSAPI_Heap_gv_disabled_alloc = RTI_TRUE;
}
#endif /* !RTI_CERT */


#ifndef RTI_CERT
/*ci
 * \brief Free a previously allocated buffer
 *
 * \param[in] buffer Pointer to the buffer to be freed.
 *
 * \return None.
 */
void
OSAPI_Heap_free_buffer(void *buffer)
{
    OSAPI_PRECONDITION(buffer == NULL,return,
                       OSAPI_Log_entry_add_pointer("buffer",buffer,RTI_TRUE);)

    
#if OSAPI_ENABLE_STDC_FREE
    free(buffer);
#else
    vPortFree(buffer);
#endif
}
#endif

#ifndef RTI_CERT
/*ci
 * \brief Return the number of bytes allocated through OSAPI_Heap functions
 *
 * \details
 * This function returns the number of bytes which have been allocated with the
 * OSAPI_Heap functions. The number is guaranteed to be no less than the sum
 * of all the sizes that have been allocated, but is not guaranteed to take
 * into account the overhead of the memory allocation itself.
 *
 * \return Number of bytes allocated
 */
RTI_SIZE_T
OSAPI_Heap_get_allocated_byte_count(void)
{

    return OSAPI_gv_AllocatedByteCount;
}
#endif /* !RTI_CERT */

/*ci
 * \brief Allocate a buffer with specified size and alignment
 *
 * \param[out] buffer Pointer to the allocated buffer.
 * \param[in] size Size of the buffer to allocate.
 * \param[in] alignment Alignment requirement for the buffer.
 */
void
OSAPI_Heap_allocate_buffer(char **buffer,
                           RTI_UINT32 size,
                           OSAPI_Alignment_T alignment)
{
    UNUSED_ARG(alignment);
    
    OSAPI_PRECONDITION((buffer == NULL),
                       return,
                       OSAPI_Log_entry_add_pointer("buffer",buffer,RTI_TRUE);)
    *buffer = NULL;

#ifndef RTI_CERT
    if (OSAPI_Heap_gv_disabled_alloc)
    {
        return;
    }
#endif /* !RTI_CERT */

    if (size == 0)
    {
        return;
    }

#if OSAPI_ENABLE_STDC_MALLOC
    *buffer = (char *)malloc(size);
#else
    *buffer = (char *)pvPortMalloc(size);
#endif /* !OSAPI_ENABLE_STDC_MALLOC */
    
    if (*buffer == NULL)
    {
        OSAPI_LOG_HEAP_INTERNAL_ALLOCATE(OSAPI_LOGKIND_ERROR,size)
        return;
    }

    OSAPI_Memory_zero(*buffer, size);

#ifndef RTI_CERT
    OSAPI_gv_AllocatedByteCount += size;
#endif /* !RTI_CERT */
}