/*
 * FILE: Heap.c - Heap functionality
 *
 * (c) Copyright, Real-Time Innovations, 2008-2020
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
 * 21may2020,fmt MICRO-2409/PR.27650 Exclude from cert build finalize() functions
 * 25aug2020,fmt MICRO-2443/PR.27634 - Remove OSAPI_Heap_free_buffer() from
 *                                     autosarHeap.c for CERT builds
 * 26jul2013,tk MICRO-373/PR#1535 - OSAPI_Heap_get_allocated_byte_count
 *                                  documentation updated
 * 22sep2008,tk Created
 *
 */

/*ci
 * \file
 * \brief Global Heap functions
 * \defgroup OSAPIHeapClass OSAPI Heap functions
 * \ingroup OSAPIModule
 *
 *  \details
 *  This provides platform independent heap functions.
 */
#include "osapi/osapi_config.h"
#include "osapi/osapi_dll.h"
#include "osapi/osapi_heap.h"
#include "osapi/osapi_types.h"
#include "osapi/osapi_log.h"

const char *OSAPI_ADDRESS_ZERO = 0;

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

/*** SOURCE_BEGIN ***/

void*
OSAPI_Heap_allocate(RTI_SIZE_T a,RTI_SIZE_T b)
{
    char *ptr;
    RTI_SIZE_T size = 0;

    OSAPI_PRECONDITION((a == 0) ||  (b == 0),
             return NULL,
             OSAPI_Log_entry_add_uint("a*b",a*b,RTI_TRUE);)

#ifndef RTI_CERT
    if (OSAPI_Heap_gv_disabled_alloc)
    {
        return NULL;
    }
#endif /* !RTI_CERT */

    size = a*b;

    OSAPI_Heap_allocate_buffer(&ptr,size,OSAPI_ALIGNMENT_DEFAULT);

    return ptr;
}

#if OSAPI_ENABLE_STDC_REALLOC
void*
OSAPI_Heap_realloc(void *ptr,RTI_SIZE_T size)
{
    OSAPI_PRECONDITION(size == 0,
                       return NULL,
                       OSAPI_Log_entry_add_uint("size",size,RTI_TRUE);)

#ifndef RTI_CERT
    if (OSAPI_Heap_gv_disabled_alloc)
    {
        return NULL;
    }
#endif /* !RTI_CERT */

    return realloc(ptr,size);
}
#endif

#ifndef RTI_CERT
void
OSAPI_Heap_free(void *buffer)
{
    OSAPI_PRECONDITION((buffer == NULL),
                       return,
                       OSAPI_Log_entry_add_pointer("buffer",buffer,RTI_TRUE);)

    OSAPI_Heap_free_buffer(buffer);
}
#endif

#ifndef RTI_CERT
void
OSAPI_Heap_disable_alloc(void)
{
    OSAPI_Heap_gv_disabled_alloc = RTI_TRUE;
}
#endif /* !RTI_CERT */

#if OSAPI_ENABLE_STDC_ALLOC
void
OSAPI_Heap_allocate_buffer(char **buffer,
                           RTI_SIZE_T size,
                           OSAPI_Alignment_T alignment)
{
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

    if (alignment != OSAPI_ALIGNMENT_DEFAULT)
    {
        OSAPI_LOG_HEAP_INTERNAL_ALLOCATE(OSAPI_LOGKIND_ERROR,size)
        return;
    }

    *buffer = malloc(size);

    if (*buffer == NULL)
    {
        OSAPI_LOG_HEAP_INTERNAL_ALLOCATE(OSAPI_LOGKIND_ERROR,size)
        return;
    }

    OSAPI_Memory_zero(*buffer, size);
}
#endif

#if OSAPI_ENABLE_STDC_FREE
#ifndef RTI_CERT
void
OSAPI_Heap_free_buffer(void *buffer)
{
    OSAPI_PRECONDITION(buffer == NULL,return,
                           OSAPI_Log_entry_add_pointer("buffer",buffer,RTI_TRUE);)

    free(buffer);
}
#endif
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
