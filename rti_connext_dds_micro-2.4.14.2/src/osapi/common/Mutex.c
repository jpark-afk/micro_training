/*
 * FILE: Mutex.c - Generic Mutex functionality
 *
 * Copyright 2018-2021 Real-Time Innovations, Inc.
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
 * 20oct2020,tk  MICRO-2623/PR#28237 Replaced tabs with spaces
 * 21may2020,fmt MICRO-2409/PR.27650 Exclude from cert build finalize() functions
 *
 */

/*ce
 * \file
 * \brief Generic implementation of OSAPI mutex routines
 */
#include "osapi/osapi_config.h"


#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef osapi_mutex_h
#include "osapi/osapi_mutex.h"
#endif
#ifndef osapi_string_h
#include "osapi/osapi_string.h"
#endif
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif

#include "Mutex.h"

/*** SOURCE_BEGIN ***/

void
OSAPI_Mutex_initialize(struct OSAPI_Mutex *self)
{
    struct OSAPI_MutexBase *base = (struct OSAPI_MutexBase*)self;

    OSAPI_Memory_zero(base,sizeof(*base));
}

#ifndef RTI_CERT
void
OSAPI_Mutex_finalize(struct OSAPI_Mutex *base)
{
    UNUSED_ARG(base);
}
#endif /* !RTI_CERT */


#if OSAPI_ENABLE_MUTEX_TRACE
RTI_BOOL
OSAPI_Mutex_take_(OSAPI_Mutex_T *self,char *file,RTI_INT32 lineno)
#else
RTI_BOOL
OSAPI_Mutex_take(OSAPI_Mutex_T *self)
#endif
{
    struct OSAPI_MutexBase *base = (struct OSAPI_MutexBase*)self;

    if (!OSAPI_Mutex_take_os(self))
    {
        return RTI_FALSE;
    }

    /* This is only used for potential debugging. */
    base->owner = OSAPI_Thread_self();

#if OSAPI_MUTEX_TRACE_ENABLED
    base->stack_depth = 0;
    base->stack[base->stack_depth].file = file;
    base->stack[base->stack_depth].lineno = lineno;
    ++base->stack_depth;
#endif

    return RTI_TRUE;
}
#if OSAPI_ENABLE_MUTEX_TRACE
RTI_BOOL
OSAPI_Mutex_give_(OSAPI_Mutex_T *self,char *file,RTI_INT32 lineno)
#else
RTI_BOOL
OSAPI_Mutex_give(OSAPI_Mutex_T *self)
#endif
{
    OSAPI_PRECONDITION(self == NULL,return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return OSAPI_Mutex_give_os(self);
}
