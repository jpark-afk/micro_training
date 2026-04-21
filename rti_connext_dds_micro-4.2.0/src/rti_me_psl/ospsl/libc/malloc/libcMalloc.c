/*
 * FILE: posixHeap.c - POSIX heap functionality
 *
 * Copyright (c) 2012-2024 Real-Time Innovations, Inc.
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
 * 04may2023,ad MICRO-4813/PR.3150
 * - Removed Heap_allocate, Heap_free, and get_allocated_byte_count
 * - Exclude realloc when building cert
 * - Always build allocate_buffer
 * - OSAPI_ENABLE_STDC_FREE no longer excludes free_buffer from builds.
 * 14sep2021,tk MICRO-3199/PR.29543
 * - Ensure a 0-sized allocation returns NULL in OSAPI_Heap_allocate_buffer
 * 20oct2020,tk MICRO-2623/PR#28237 Replaced tabs with spaces
 * 28jun2016,tk MICRO-1548 Support OSAPI_STRICT_POSIX
 * 09mar2012,tk Written
 */
/*ce
 * \file
 * \brief POSIX implementation of OSAPI heap routines
 */
#include "rti_me_psl.h"
#include "rti_me_psl/osapi/osapi_heap_test.h"

RTI_PRIVATE RTI_SIZE_T OSAPI_fv_AllocatedByteCount = 0;
RTI_PRIVATE RTI_BOOL OSAPI_fv_AllocatedByteCountDisabled = RTI_FALSE;
RTI_PRIVATE RTI_BOOL OSAPI_Heap_gv_disabled_alloc = RTI_FALSE;

/*** SOURCE_BEGIN ***/

void
OSAPI_Heap_add_allocated_byte_count(RTI_SIZE_T count)
{
    if (!OSAPI_fv_AllocatedByteCountDisabled)
    {
        OSAPI_fv_AllocatedByteCount += count;
    }
}

void
OSAPI_Heap_enable_allocated_byte_count(void)
{
    OSAPI_fv_AllocatedByteCountDisabled = RTI_FALSE;
}

void
OSAPI_Heap_disable_allocated_byte_count(void)
{
    OSAPI_fv_AllocatedByteCountDisabled = RTI_TRUE;
}

RTI_SIZE_T
OSAPI_Heap_get_allocated_byte_count_v2(void)
{
     return OSAPI_fv_AllocatedByteCount;
}

RTI_SIZE_T
OSAPI_Heap_get_allocated_byte_count(void)
{
     return OSAPI_fv_AllocatedByteCount;
}

void
OSAPI_Heap_disable_alloc(void)
{
    OSAPI_Heap_gv_disabled_alloc = RTI_TRUE;
}

void
OSAPI_Heap_enable_alloc(void)
{
    OSAPI_Heap_gv_disabled_alloc = RTI_FALSE;
}

#ifndef RTI_CERT
void*
OSAPI_Heap_realloc(void *ptr,RTI_SIZE_T size)
{
    OSAPI_PRECONDITION((size == 0),
                       return NULL,
                       OSAPI_Log_entry_add_uint("size",size,RTI_TRUE);)
   
#ifndef RTI_CERT
    if (OSAPI_Heap_gv_disabled_alloc)
    {
        return NULL;
    }
#endif
    return realloc(ptr,size);
}
#endif /* !RTI_CERT */

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

    OSAPI_Heap_add_allocated_byte_count(size);
}

#ifndef RTI_CERT
void
OSAPI_Heap_free_buffer(void *buffer)
{
    OSAPI_PRECONDITION(buffer == NULL,return,
                       OSAPI_Log_entry_add_pointer("buffer",buffer,RTI_TRUE);)
 
    free(buffer);
}
#endif
