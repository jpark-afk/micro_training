/*
 * FILE: Thread.h - Platform independent Thread definitions
 *
 * (c) Copyright 2008-2015 Real-Time Innovations
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
 * 15aug2022,ad MICRO-3813/PR.30698
 * - Exclude create_sem for cert deos
 * 15apr2021,tk MICRO-2979/PR.27549
 * - Removed unused functions from CERT:
 *   OSAPI_Thread_new
 *   OSAPI_Thread_exec
 * 20aug2008,tk Written
 */
/*ce
 * \file
 */
#ifndef Thread_h
#define Thread_h

#ifndef osapi_config_h
#include "osapi/osapi_config.h"
#endif 

/* Forward declaration for pointer use
 */
struct OSAPI_Thread;

/*ci \brief Type for function being called before a thread is started
 * 
 * \param[in] self Thread being started
 */
typedef void (*OSAPI_ThreadOnBeforeStartFunc)(struct OSAPI_Thread *self);

/*ci \brief Type for function being called before a thread is deleted
 * 
 * \param[in] self Thread being deleted
 */
typedef void (*OSAPI_ThreadOnBeforeDeleteFunc)(struct OSAPI_Thread *self);

/*ci \brief The maximum length of a thread name, including NUL
 */
#if ENABLE_ARINC_653
#define OSAPI_THREAD_MAX_NAME   MAX_NAME_LENGTH
#else
#define OSAPI_THREAD_MAX_NAME   (16)
#endif

/*i \defgroup OSAPI_ThreadClass OSAPI Thread
 *  \ingroup OSAPIModule
 */
struct OSAPI_Thread
{
    /*ci
     * Platform specific thread handle
     */
    OSAPI_ThreadHandle thread_handle;

#if !(defined(RTI_CERT) && defined(RTI_DEOS) && UDP_EXCLUDE_BUILTIN)
    /*ci
     * Semaphore to synchronize thread creation
     */
    struct OSAPI_Semaphore *create_sem;
#endif

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
     * If installed, call this function before starting the user-thread
     */
    OSAPI_ThreadOnBeforeStartFunc on_before_start;

    /*ci
     * If installed, call this function after stopping the user-thread,
     * but before it is deleted.
     */
    OSAPI_ThreadOnBeforeDeleteFunc on_before_delete;

    /*ci
     * The thread options the thread was created with
     */
    OSAPI_ThreadOptions thread_options;
};

#ifndef RTI_CERT
extern void
OSAPI_Thread_delete(struct OSAPI_Thread *me);
#endif /* !RTI_CERT */

#if !OSAPI_NO_THREADS
MUST_CHECK_RETURN extern struct OSAPI_Thread*
OSAPI_Thread_new(const struct OSAPI_ThreadProperty *properties);

MUST_CHECK_RETURN extern void*
OSAPI_Thread_exec(void *param);
#endif

#endif /* Thread_pkg_h */
