/*
 * FILE: Heap.c - Heap functionality
 *
 * Copyright (c) 2008-2024 Real-Time Innovations, Inc.
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

const void *OSAPI_CC_NullPtr = (const void *)(0);

/*** SOURCE_BEGIN ***/

void*
OSAPI_Heap_allocate(RTI_SIZE_T a,RTI_SIZE_T b)
{
    char *ptr = NULL;
    RTI_SIZE_T size = 0;

    OSAPI_PRECONDITION((a == 0) ||  (b == 0),
             return NULL,
             OSAPI_Log_entry_add_uint("a*b",a*b,RTI_TRUE);)

    size = a * b;

    /* Check for an overflow */
    if ( ((RTI_UINT64)a * b) > UINT_MAX )
    {
        return NULL;
    }

    OSAPI_Heap_allocate_buffer(&ptr,size,OSAPI_ALIGNMENT_DEFAULT);

    return ptr;
}

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
