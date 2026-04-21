/*
 * FILE: winHeap.c - Win heap functionality
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
 * 09mar2012,tk Written
 *
 */
/*ce
 * \file
 * \brief Win implementation of OSAPI heap routines
 */
#include "osapi/osapi_config.h"


#if OSAPI_INCLUDE_WINDOWS
#include <stdlib.h>
#include <malloc.h>
#include "osapi/osapi_heap.h"
#include "osapi/osapi_string.h"
#include "osapi/osapi_log.h"

#endif

/*** SOURCE_BEGIN ***/

#if OSAPI_INCLUDE_WINDOWS
/* ----------------------------------------------------------------- */
/* allocate the specified size of memory and align it as well */
void
OSAPI_Heap_allocate_buffer(char **buffer,
                           RTI_UINT32 size,
                           OSAPI_Alignment_T alignment)
{
    OSAPI_PRECONDITION((buffer == NULL),
                    return,
                    OSAPI_Log_entry_add_pointer("buffer",buffer,RTI_TRUE);)

#ifndef RTI_CERT
    if (OSAPI_Heap_gv_disabled_alloc)
    {
        *buffer = NULL;
        return;
    }
#endif /* !RTI_CERT */

    if (alignment == OSAPI_ALIGNMENT_DEFAULT)
    {
        alignment = sizeof(void*);
    }

    *buffer = _aligned_malloc(size, alignment);
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

#if !OSAPI_ENABLE_STDC_REALLOC
void*
OSAPI_Heap_realloc(void *ptr,RTI_SIZE_T size)
{
    OSAPI_PRECONDITION(size <= 0,
                       return NULL,
                       OSAPI_Log_entry_add_int("size",size,RTI_TRUE);)

#ifndef RTI_CERT
    if (OSAPI_Heap_gv_disabled_alloc)
    {
        return NULL;
    }
#endif /* !RTI_CERT */

    return _aligned_realloc(ptr,size,sizeof(void*));
}
#endif

/* ----------------------------------------------------------------- */
/* free the previously allocated memory */
#ifndef RTI_CERT
void
OSAPI_Heap_free_buffer(void *buffer)
{
    OSAPI_PRECONDITION(buffer == NULL,return,
                        OSAPI_Log_entry_add_pointer("buffer",buffer,RTI_TRUE);)

    _aligned_free(buffer);
}
#endif

#endif


