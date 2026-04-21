/*
 * FILE: posixHeap.c - POSIX heap functionality
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
#include "osapi/osapi_config.h"

#if OSAPI_INCLUDE_POSIX
#include "osapi/osapi_log.h"
#include "osapi/osapi_heap.h"
#include "osapi/osapi_string.h"
#endif

/*** SOURCE_BEGIN ***/

#if OSAPI_INCLUDE_POSIX

/* ----------------------------------------------------------------- */
/* allocate the specified size of memory and align it as well */
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

    OSAPI_Memory_zero(*buffer,size);

#ifndef RTI_CERT
    /* Note that this code is not thread-safe and thus may not be accurate or even correct */
#if defined(RTI_DARWIN) && !OSAPI_ENABLE_STRICT_POSIX
    OSAPI_gv_AllocatedByteCount += (RTI_SIZE_T)malloc_size(*buffer);
#elif defined(RTI_LINUX) && !defined(__ANDROID__) && (ENABLE_FACE_COMPLIANCE == FACE_COMPLIANCE_LEVEL_NONE)
    OSAPI_gv_AllocatedByteCount += (RTI_SIZE_T)malloc_usable_size(*buffer);
#else
    OSAPI_gv_AllocatedByteCount += (RTI_SIZE_T)size;
#endif
#endif /* !RTI_CERT */
}

#endif /* INCLUDE_POSIX */


