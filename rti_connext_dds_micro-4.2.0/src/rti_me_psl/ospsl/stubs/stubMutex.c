/*
 * FILE: stubMutex.c - Stub mutex functionality
 *
 * Copyright 2024-2024 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 */
/*ce
 * \file
 * \brief Win implementation of OSAPI mutex routines
 */
#include "rti_me_psl.h"

#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef osapi_mutex_h
#include "osapi/osapi_mutex.h"
#endif
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif

struct OSAPI_Mutex
{
    RTI_UINT32 depth;
};

/*** SOURCE_BEGIN ***/

#ifndef RTI_CERT
RTI_BOOL
OSAPI_Mutex_delete(struct OSAPI_Mutex *mutex)
{
    OSAPI_PRECONDITION(mutex == NULL,return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("mutex",mutex,RTI_TRUE);)
    UNUSED_ARG(mutex);

    return RTI_FALSE;
}
#endif

struct OSAPI_Mutex *
OSAPI_Mutex_new(void)
{
    return NULL;
}

RTI_BOOL
OSAPI_Mutex_take(struct OSAPI_Mutex *mutex)
{
    OSAPI_PRECONDITION(mutex == NULL,return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("mutex",mutex,RTI_TRUE);)

    UNUSED_ARG(mutex);

    return RTI_FALSE;
}

RTI_BOOL
OSAPI_Mutex_give(struct OSAPI_Mutex *mutex)
{

    OSAPI_PRECONDITION(mutex == NULL,return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("mutex",mutex,RTI_TRUE);)

    UNUSED_ARG(mutex);

    return RTI_FALSE;
}

RTI_UINT32
OSAPI_Mutex_get_depth(OSAPI_Mutex_T *mutex)
{
    OSAPI_PRECONDITION(mutex == NULL,return RTI_FALSE,
                    OSAPI_Log_entry_add_pointer("mutex",mutex,RTI_TRUE);)
    UNUSED_ARG(mutex);

    return 0;
}

