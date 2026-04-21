/*
 * FILE: threadxHeap.c - ThreadX heap functionality
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
 * 13dec2016,francisco  File created
 *
 */
/*ce
 * \file
 * \brief ThreadX implementation of OSAPI heap routines
 */
#include "osapi/osapi_config.h"


#if OSAPI_INCLUDE_THREADX

#include "tx_api.h"

#include "osapi/osapi_heap.h"
#include "osapi/osapi_string.h"
#include "osapi/osapi_log.h"

/* pool for heap is created in system init */
struct OSAPIThreadXHeap
{
    RTI_BOOL     is_initialized;
    TX_BYTE_POOL pool;
};

LINK_SECTION_BSS_SDRAM
RTI_PRIVATE struct OSAPIThreadXHeap OSAPIThreadX_fv_heap = { RTI_FALSE };

LINK_SECTION_BSS_SDRAM
RTI_PRIVATE
RTI_UINT32 heap_fv_memory[OSAPI_PLATFORM_THREADX_HEAP_SIZE/(sizeof(RTI_UINT32))];

#endif

/*** SOURCE_BEGIN ***/

#if OSAPI_INCLUDE_THREADX

/* ----------------------------------------------------------------- */
/* allocate the specified size of memory and align it as well */
void
OSAPI_Heap_allocate_buffer(char **buffer,
                           RTI_SIZE_T size,
                           OSAPI_Alignment_T alignment)
{
    UINT rc;

    OSAPI_PRECONDITION((buffer == NULL),
                       return,
                       OSAPI_Log_entry_add_pointer("buffer",buffer,RTI_TRUE);)

    UNUSED_ARG(alignment);

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

    if (OSAPIThreadX_fv_heap.is_initialized == RTI_FALSE)
    {
        rc = tx_byte_pool_create(&OSAPIThreadX_fv_heap.pool,
                                 "micro-heap-pool",
                                  heap_fv_memory,
                                  sizeof(heap_fv_memory));

        if (rc != TX_SUCCESS)
        {
            OSAPI_LOG_HEAP_INTERNAL_ALLOCATE(OSAPI_LOGKIND_ERROR, rc)
            *buffer = NULL;
            return;
        }

        OSAPIThreadX_fv_heap.is_initialized = RTI_TRUE;
    }

    rc = tx_byte_allocate(&OSAPIThreadX_fv_heap.pool, (void**)buffer, 
                          size, TX_NO_WAIT);
    if ((*buffer == NULL) || (rc != TX_SUCCESS))
    {
        *buffer = NULL;
        OSAPI_LOG_HEAP_INTERNAL_ALLOCATE(OSAPI_LOGKIND_ERROR,size)
        return;
    }

    OSAPI_Memory_zero(*buffer, size);

#ifndef RTI_CERT
    OSAPI_gv_AllocatedByteCount += size;
#endif /* !RTI_CERT */
}

/* ----------------------------------------------------------------- */
/* free the previously allocated memory */
#ifndef RTI_CERT
void
OSAPI_Heap_free_buffer(void *buffer)
{
    UINT rc;

    OSAPI_PRECONDITION(buffer == NULL, return,
                       OSAPI_Log_entry_add_pointer("buffer",buffer,RTI_TRUE);)

    rc = tx_byte_release(buffer);
    IGNORE_RETVAL(rc);
}
#endif

#endif
