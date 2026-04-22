/*
 * FILE: autosarThread.c - AutoSAR thread functionality
 *
 * Copyright 2019-2026 Real-Time Innovations, Inc.
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
 * 13mar2019,fmt Written
 *
 */
/*ce
 * \file
 * \brief AUTOSAR implementation of OSAPI thread routines
 */
#include "rti_me_psl.h"

/*** SOURCE_BEGIN ***/

/* Notes:
 * 1) OSAPI_Thread_create() is not implemented in the Autosar port and there will
 * be a link error in case user tries to call this function. Instead user should
 * defined as many tasks as needed.
 * 2) OSAPI_Thread_sleep() is not implemented in the Autosar port and there will
 * be a link error in case user tries to call this function. Instead user should
 * use a periodic task, events, etc, to synchronize tasks.
 */

FUNC(OSAPI_ThreadId, SOAD_CODE)
OSAPI_Thread_self(void)
{
    TaskType task_id;
    StatusType ret_value;
    OSAPI_ThreadId result;

    ret_value = GetTaskID(&task_id);
    
    IGNORE_RETVAL(ret_value);

    result.handle.data = (void*)task_id;
    
    return result;
}

/* TODO: THIS SHOULD NOT BE SUPPORTED ON AUTOSAR PORT */
OSAPI_ThreadHandle
OSAPI_Thread_create_native(const char *name,
                           const struct OSAPI_ThreadProperty *property,
                           OSAPI_ThreadRoutine thread_entry,
                           void *thread_data,
                           OSAPI_ThreadRoutine thread_wakeup)
{
    static OSAPI_ThreadHandle native_thread;

    return &native_thread;
}

/* TODO: THIS SHOULD NOT BE SUPPORTED ON AUTOSAR PORT */
RTI_BOOL
OSAPI_Thread_delete_native(OSAPI_ThreadHandle *handle)
{
    UNUSED_ARG(handle);

    return RTI_TRUE;
}

/* TODO: THIS SHOULD NOT BE SUPPORTED ON AUTOSAR PORT */
void
OSAPI_Thread_nanosleep(RTI_UINT32 ns)
{
    UNUSED_ARG(ns);
    /* Not supported on Autosar port */
}
