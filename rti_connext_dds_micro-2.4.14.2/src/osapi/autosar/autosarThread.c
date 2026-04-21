/*
 * FILE: autosarThread.c - AutoSAR thread functionality
 *
 * Copyright 2019-2021 Real-Time Innovations, Inc.
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
#include "osapi/osapi_config.h"

#if OSAPI_INCLUDE_AUTOSAR
#include "osapi/osapi_string.h"
#include "osapi/osapi_thread.h"
#include "osapi/osapi_types.h"
#include "osapi/osapi_heap.h"
#include "osapi/osapi_semaphore.h"
#include "osapi/osapi_log.h"

#include "../common/Thread.h"

#endif

/*** SOURCE_BEGIN ***/

#if OSAPI_INCLUDE_AUTOSAR

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

    ret_value = GetTaskID(&task_id);
    
    UNUSED_ARG(ret_value);
    
    return (OSAPI_ThreadId)task_id;
}

#endif
