/*
 * FILE: Thread.h - Platform independent Thread definitions
 *
 * Copyright (c) 2008-2024 Real-Time Innovations
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
 */
#ifndef Thread_h
#define Thread_h

#ifndef osapi_config_h
#include "osapi/osapi_config.h"
#endif 

#define OSAPI_THREAD_MAX_NAME   (16)

/*i \defgroup OSAPI_ThreadClass OSAPI Thread
 *  \ingroup OSAPIModule
 */
struct OSAPI_Thread
{
    /*ci
     * Platform specific thread handle
     */
    OSAPI_ThreadHandle thread_handle;

    /*ci
     * Semaphore to synchronize thread creation
     */
    struct OSAPI_Semaphore *create_sem;

    /*ci
     * Semaphore to synchronize thread start/stop
     */
    struct OSAPI_Semaphore *start_sem;

    /*ci
     * User specific thread-entry point
     */
    OSAPI_ThreadRoutine user_routine;

    /*ci
     * Routing to wakeup thread currently pending (user defined)
     */
    OSAPI_ThreadRoutine wakeup_routine;

    /*ci
     * Thread information
     */
    struct OSAPI_ThreadInfo thread_info;

    /*ci
     * Thread name
     */
    char name[OSAPI_THREAD_MAX_NAME];

    /*ci
     * Thread id
     */
    OSAPI_ThreadId tid;

    /*ci
     * The thread options the thread was created with
     */
    OSAPI_ThreadOptions thread_options;
};


#endif /* Thread_pkg_h */
