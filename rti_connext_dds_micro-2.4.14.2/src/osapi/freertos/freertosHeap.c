/*
 * FILE: freertosHeap.c - Heap functionality
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
 */
/*ce
 * \file
 * \brief FreeRTOS implementation of OSAPI heap routines
 */

#include "osapi/osapi_config.h"


#if OSAPI_INCLUDE_FREERTOS
#include "osapi/osapi_log.h"
#include "osapi/osapi_heap.h"
#include "osapi/osapi_string.h"
#endif /* OSAPI_INCLUDE_FREERTOS */

/*** SOURCE_BEGIN ***/

#if OSAPI_INCLUDE_FREERTOS

/* allocate the specified size of memory and align it as well */
void
OSAPI_Heap_allocate_buffer(char **buffer,
                           RTI_SIZE_T size,
                           OSAPI_Alignment_T alignment)
{
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

    *buffer = (char *)pvPortMalloc(size);

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


/* ----------------------------------------------------------------- */
/* free the previously allocated memory */
#ifndef RTI_CERT
void
OSAPI_Heap_free_buffer(void *buffer)
{
    OSAPI_PRECONDITION(buffer == NULL,return,
                        OSAPI_Log_entry_add_pointer("buffer",buffer,RTI_TRUE);)

    vPortFree(buffer);
}
#endif

#endif /* OSAPI_INCLUDE_FREERTOS */
