/*
 * FILE: vxHeap.c - VxWorks heap functionality
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
 * 13apr2015,eh MICRO-228/PR#1407 Return NULL for 0 size allocation
 * 25feb2015,eh MICRO-356/PR#1496 Add OSAPI_Heap_allocate function for Cert
 * 09mar2012,tk Created
 *
 */
/*ce
 * \file
 * \brief VxWorks implementation of OSAPI heap routines
 */
#include "osapi/osapi_config.h"


#if OSAPI_INCLUDE_VXWORKS

#include <stdlib.h>
#include <vxWorks.h>
#include <memLib.h>
#include "osapi/osapi_heap.h"
#include "osapi/osapi_string.h"
#include "osapi/osapi_log.h"
#endif

/*** SOURCE_BEGIN ***/

#if OSAPI_INCLUDE_VXWORKS
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

    *buffer = NULL;

#ifndef RTI_CERT
    if (OSAPI_Heap_gv_disabled_alloc)
    {
        return;
    }
#endif /* !RTI_CERT */

    /* This is added a safety since a buffer of size 0 is legal and a NULL
     * value is not guaranteed
     */
    if (size == 0)
    {
        return;
    }

    if (alignment == OSAPI_ALIGNMENT_DEFAULT)
    {
        alignment = sizeof(void*);
    }

    /* From the documentation of memalign: This routine allocates a buffer of
     * size <size> from the system memory partition.  Additionally, it insures
     * that the allocated buffer begins on a memory address evenly divisible by
     * the specified alignment parameter.  The alignment parameter must be a
     * power of 2 THIS DOCUMENTATION IS MISLEADING: the address where the buffer
     * starts is NOT an even multiple; but just a multiple...so the call below is
     * NOT too conservative. */
    *buffer = memalign(alignment, size);
    if (*buffer == NULL)
    {
        return;
    }

    OSAPI_Memory_zero(*buffer, size);

#ifndef RTI_CERT
    OSAPI_gv_AllocatedByteCount += size;
#endif /* !RTI_CERT */
}

#endif /* OSAPI_PLATFORM_VXWORKS */
