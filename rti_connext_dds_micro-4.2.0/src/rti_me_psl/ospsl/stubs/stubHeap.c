/*
 * FILE: stubHeap.c - stub heap functionality
 *
 * Copyright (c) 2024-2024 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 */
/*ce
 * \file
 * \brief Win implementation of OSAPI heap routines
 */
#include "rti_me_psl.h"

#include "osapi/osapi_heap.h"
#include "rti_me_psl/osapi/osapi_heap_test.h"
#include "osapi/osapi_string.h"
#include "osapi/osapi_log.h"

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

/* ----------------------------------------------------------------- */
/* allocate the specified size of memory and align it as well */
void
OSAPI_Heap_allocate_buffer(char **buffer,
                           RTI_SIZE_T size,
                           OSAPI_Alignment_T alignment)
{
    UNUSED_ARG(buffer);
    UNUSED_ARG(size);
    UNUSED_ARG(alignment);
}

#ifndef RTI_CERT
void*
OSAPI_Heap_realloc(void *ptr,RTI_SIZE_T size)
{    
    UNUSED_ARG(size);
    UNUSED_ARG(ptr);

    return NULL;
}
#endif

#ifndef RTI_CERT
/* ----------------------------------------------------------------- */
/* free the previously allocated memory */
void
OSAPI_Heap_free_buffer(void *buffer)
{
    UNUSED_ARG(buffer);
}
#endif

