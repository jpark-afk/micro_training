/*
 * FILE: osapi_thread.h - Definition of System API
 *
 * Copyright (c) 2012-2024 Real-Time Innovations, Inc.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*i \file
  * \brief Thread interface definition
  */
#ifndef osapi_thread_h
#define osapi_thread_h

#include "osapi/osapi_config.h"

#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef osapi_mutex_h
#include "osapi/osapi_mutex.h"
#endif
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif
#ifndef osapi_process_h
#include "osapi/osapi_process.h"
#endif
#ifndef osapi_semaphore_h
#include "osapi/osapi_semaphore.h"
#endif
#ifndef osapi_string_h
#include "osapi/osapi_string.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*e \dref_OSAPIThreadGroupDocs
 */

/*i \ingroup OSAPI_ThreadClass
 *
 * \brief Abstract thread class.
 */
struct OSAPI_Thread;

/* these are #defined so users can pass in any int they want for priority as well
 *  i.e., OS-native thread priorities
 */
/*e \dref_OSAPI_THREAD_PRIORITY_LOW
 */
#define OSAPI_THREAD_PRIORITY_LOW               -1

/*e \dref_OSAPI_THREAD_PRIORITY_BELOW_NORMAL
 */
#define OSAPI_THREAD_PRIORITY_BELOW_NORMAL      -2

/*e \dref_OSAPI_THREAD_PRIORITY_NORMAL
 */
#define OSAPI_THREAD_PRIORITY_NORMAL            -3

/*e \dref_OSAPI_THREAD_PRIORITY_ABOVE_NORMAL
 */
#define OSAPI_THREAD_PRIORITY_ABOVE_NORMAL      -4

/*e \dref_OSAPI_THREAD_PRIORITY_HIGH
 */
#define OSAPI_THREAD_PRIORITY_HIGH              -5

/*e \dref_OSAPI_THREAD_PRIORITY_INHERIT
 */
#define OSAPI_THREAD_PRIORITY_INHERIT           -6

/*e \dref_OSAPI_THREAD_PRIORITY_DEFAULT
 */
#define OSAPI_THREAD_PRIORITY_DEFAULT          OSAPI_THREAD_PRIORITY_NORMAL

/*e \dref_OSAPI_THREAD_USE_OSDEFAULT_STACKSIZE
 */
#define OSAPI_THREAD_USE_OSDEFAULT_STACKSIZE    0

/*e \dref_OSAPI_ThreadOptions
 */
typedef RTI_UINT32 OSAPI_ThreadOptions;

/*e \dref_OSAPI_THREAD_DEFAULT_OPTIONS
*/
#define OSAPI_THREAD_DEFAULT_OPTIONS     0x00

/*e \dref_OSAPI_THREAD_FLOATING_POINT
*/
#define OSAPI_THREAD_FLOATING_POINT      0x01

/*e \dref_OSAPI_THREAD_STDIO
*/
#define OSAPI_THREAD_STDIO               0x02

/*e \dref_OSAPI_THREAD_REALTIME_PRIORITY
*/
#define OSAPI_THREAD_REALTIME_PRIORITY   0x08

#if OSAPI_THREAD_SEMAPHORE_ENABLED
/*e \dref_OSAPI_THREAD_SUSPEND_ENABLE
*/
#define OSAPI_THREAD_SUSPEND_ENABLE      0x40
#endif


typedef void* OSAPI_ThreadHandle;
typedef struct OSAPI_ThreadId
{
    /*e \dref_handle
    */
    union
    {
        /*e \dref_handle64
        */
        RTI_UINT64 handle64;
        
        /*e \dref_handle32
        */
        RTI_UINT32 handle32;
        
        /*e \dref_data
        */
        void *data;
    } handle;
} OSAPI_ThreadId;

/*e \dref_OSAPI_ThreadProperty
 */
struct OSAPI_ThreadProperty
{
    /*e \dref_OSAPI_ThreadProperty_stack_size
     */
    RTI_UINT32 stack_size;

    /*e \dref_OSAPI_ThreadProperty_priority
     */
    RTI_INT32 priority;

    /*e \dref_OSAPI_ThreadProperty_options
     */
    OSAPI_ThreadOptions options;
};

/*e \dref_OSAPI_THREAD_PROPERTY_DEFAULT
 *
 */
#define OSAPI_THREAD_PROPERTY_DEFAULT \
{ \
    OSAPI_THREAD_USE_OSDEFAULT_STACKSIZE, \
    OSAPI_THREAD_PRIORITY_DEFAULT, \
    OSAPI_THREAD_DEFAULT_OPTIONS \
}

/*e \dref_OSAPI_ThreadProperty_INITIALIZER
 */
#define OSAPI_ThreadProperty_INITIALIZER OSAPI_THREAD_PROPERTY_DEFAULT

/*ci \brief Type for function being called before a thread is started
 *
 * \param[in] self Thread being started
 */
typedef void (*OSAPI_ThreadOnBeforeStartFunc)(void *native_data);

/*ci \brief Type for function being called before a thread is deleted
 *
 * \param[in] self Thread being deleted
 */
typedef void (*OSAPI_ThreadOnBeforeDeleteFunc)(void *native_data);

struct OSAPI_ThreadState
{
    /*e Whether or not the created thread is preemptive */
    RTI_BOOL is_preemptive;
    
    OSAPI_ThreadOnBeforeStartFunc on_before_start;

    OSAPI_ThreadOnBeforeDeleteFunc on_before_delete;

    void *native_data;
};

/*i
 * \ingroup OSAPI_ThreadClass
 * \brief Thread info
 *
 */
struct OSAPI_ThreadInfo
{
    /*e Stop executing thread 
     */
    RTI_BOOL stop_thread;

    void *user_data;

    /*e Whether or not the created thread is preemptive */
    RTI_BOOL is_preemptive;
    
    struct OSAPI_ThreadState thread_state;
};

/*i \ingroup OSAPI_ThreadClass
 *
 * \brief Thread task signature.
 *
 * \param[in] thread_info Thread information structure
 *
 * \return RTI_TRUE on successful execution, RTI_FALSE on failure.
 */
FUNCTION_SHOULD_TYPEDEF(
RTI_BOOL
(*OSAPI_ThreadRoutine)(struct OSAPI_ThreadInfo *thread_info)
)

#if !(defined(RTI_CERT) && defined(RTI_DEOS) && UDP_EXCLUDE_BUILTIN)
/*i \ingroup OSAPI_ThreadClass
 *
 *  \brief Wakeup user-thread
 *
 *  \details
 *
 *  If a user-defined thread function is blocking, e.g. waiting for
 *  data, and the user wants to delete the thread, it is necessary to
 *  unblock the user-thread. The user must provide a function which can
 *  unblock a thread. This function calls the wake up function to wake up a
 *  blocked user thread.
 *
 *  \param [in] self OSAPI_Thread to wakeup
 *
 *  \return RTI_TRUE on success, RTI_FALSE on failure.
 *
 *  \sa \ref OSAPI_Thread_start
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_Thread_wakeup(struct OSAPI_Thread *self);
#endif /*!(defined(RTI_CERT) && defined(RTI_DEOS) && UDP_EXCLUDE_BUILTIN)*/

/*i \ingroup OSAPI_ThreadClass
 *
 *  \brief Start a specific thread.
 *
 *  \param [in] me Thread to wake up
 *
 *  \return RTI_TRUE on success, RTI_FALSE on failure.
 *
 *  \sa \ref OSAPI_Thread_create
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_Thread_start(struct OSAPI_Thread *me);

#if !(defined(RTI_CERT) && defined(RTI_DEOS) && UDP_EXCLUDE_BUILTIN)
/*i \ingroup OSAPI_ThreadClass
 *  \brief Destroy a specific thread.
 *
 *  \param [in] self Thread.
 *
 *  \return RTI_TRUE on success, RTI_FALSE on failure.
 *
 *  \sa \ref OSAPI_Thread_create
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_Thread_destroy(struct OSAPI_Thread *self);
#endif /*!(defined(RTI_CERT) && defined(RTI_DEOS) && UDP_EXCLUDE_BUILTIN)*/

/*i \ingroup OSAPI_ThreadClass
 *
 *  \brief Create a thread.
 *
 *  \param [in] name The name of the thread.
 *
 *  \param [in] properties Thread properties. These properties are hints.
 *
 *  \param [in] user_routine  Thread task. The thread task cannot assume that
 *                          it can block; thus it must be written such that
 *                          it can be called repeatedly.
 *
 *  \param [in] user_data Parameters passed to the thread task.
 *
 *  \param [in] wakeup_routine Routine to wake up a thread, called to delete
 *                            a thread.
 *
 *  \return Handle to stopped thread on success, NULL on failure.
 *
 *  \sa \ref OSAPI_Thread_destroy
 */
MUST_CHECK_RETURN OSAPIDllExport struct OSAPI_Thread*
OSAPI_Thread_create(const char *name,
                    const struct OSAPI_ThreadProperty *property,
                    OSAPI_ThreadRoutine thread_entry,
                    void *thread_data,
                    OSAPI_ThreadRoutine thread_wakeup);

MUST_CHECK_RETURN OSPSLDllExport OSAPI_ThreadHandle
OSAPI_Thread_create_native(const char *name,
                           const struct OSAPI_ThreadProperty *property,
                           OSAPI_ThreadRoutine thread_entry,
                           void *thread_data,
                           OSAPI_ThreadRoutine thread_wakeup);

MUST_CHECK_RETURN OSPSLDllExport RTI_BOOL
OSAPI_Thread_delete_native(OSAPI_ThreadHandle *handle);

/*i \ingroup OSAPI_ThreadClass
 *
 *  \brief Return thread ID.
 *
 *  \return thread ID of the calling thread.
 *
 *  \sa \ref OSAPI_Thread_create
 */
OSPSLDllExport OSAPI_ThreadId
OSAPI_Thread_self(void);

#ifndef RTI_CERT
OSAPIDllExport void
OSAPI_Thread_delete(struct OSAPI_Thread *me);
#endif /* !RTI_CERT */

#ifndef RTI_CERT
/*i \ingroup OSAPI_ThreadClass
 *  \brief Suspend a thread for a specified amount of time.
 *
 *  \param [in] ms Sleep time.
 *
 */
OSPSLDllExport void
OSAPI_Thread_sleep(RTI_UINT32 ms);
#endif

OSPSLDllExport void
OSAPI_Thread_nanosleep(RTI_UINT32 ns);

OSAPIDllExport RTI_BOOL
OSAPI_Thread_is_self(OSAPI_ThreadId *pid);

#ifdef __cplusplus
}                               /* extern "C" */
#endif


#endif /* osapi_thread_h */
