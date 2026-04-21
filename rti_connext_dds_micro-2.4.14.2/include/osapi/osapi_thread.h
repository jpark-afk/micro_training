/*
 * FILE: osapi_thread.h - Definition of System API
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
 * 11mar2022, am MICRO-3515
 * - Removed unused functions for DEOS when compiling for RTI_CERT because UDP is excluded.
 *   OSAPI_Thread_wakeup
 *   OSAPI_Thread_destroy 
 * 20sep2021,tk MICRO-3152/PR.29564
 * - Only include OSAPI_THREAD_SUSPEND_ENABLE when OSAPI_THREAD_SEMAPHORE_ENABLED
 *   is TRUE.
 * 15apr2021,tk MICRO-2979/PR.27549
 * - Removed unused functions from CERT for OS with no threads:
 *   OSAPI_Thread_wakeup
 *   OSAPI_Thread_create
 * 13apr2021,tk MICRO-3028/PR.28960
 * - Made all public documentation external
 * 23feb2021,tk MICRO-2914/PR#28853
 *    - Define OSAPI_THREAD_PRIORITY_INHERIT
 *    - Define OSAPI_THREAD_PRIORITY_DEFAULT
 * 10jan2021,tk MICRO-2807 / PR.27549  Removed unused functions for CERT
 * 13dec2020,tk MICRO-2740/PR.28040
 *              - Renamed is_premptive to is_preemptive
 * 08sep2020,tk  MICRO-2506/PR#28039 Corrected internal/external doxygen tags
 * 12mar2012,tk Written
 */
/*i \file
  * \brief Thread interface definition
  */
#ifndef osapi_thread_h
#define osapi_thread_h

#include "osapi/osapi_config.h"
#ifndef osapi_dll_h
#include "osapi/osapi_dll.h"
#endif
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*e \dref_OSAPIThreadGroupDocs
 */

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

#if OSAPI_HAVE_PORT_THREAD_PROPERTY
    /*e
     *  \brief System specific properties
     */
    struct OSAPI_Port_ThreadProperty port_property;

#endif    
};

#if OSAPI_HAVE_PORT_THREAD_PROPERTY
#ifndef OSAPI_Port_ThreadProperty_INITIALIZER
#error "OSAPI_HAVE_PORT_THREAD_PROPERTY is TRUE, but OSAPI_Port_ThreadProperty_INITIALIZER is undefined"
#endif

/*\ci Initializer for Port properties when present
 */
#define OSAPI_PORT_THREAD_PROPERTY_INITIALIZER ,OSAPI_Port_ThreadProperty_INITIALIZER

#else
#define OSAPI_PORT_THREAD_PROPERTY_INITIALIZER 
#endif /* OSAPI_HAVE_PORT_THREAD_PROPERTY */

/*ci \brief Default priority for POSIX is to inherit from parent
 */
#if defined(OSAPI_INCLUDE_POSIX)
#define OSAPI_THREAD_PRIORITY_DEFAULT OSAPI_THREAD_PRIORITY_INHERIT
#else
#define OSAPI_THREAD_PRIORITY_DEFAULT OSAPI_THREAD_PRIORITY_NORMAL
#endif

/*e \dref_OSAPI_THREAD_PROPERTY_DEFAULT
 *
 */
#define OSAPI_THREAD_PROPERTY_DEFAULT \
{ \
    OSAPI_THREAD_USE_OSDEFAULT_STACKSIZE, \
    OSAPI_THREAD_PRIORITY_DEFAULT, \
    OSAPI_THREAD_DEFAULT_OPTIONS \
    OSAPI_PORT_THREAD_PROPERTY_INITIALIZER \
}

/*e \dref_OSAPI_ThreadProperty_INITIALIZER
 */
#define OSAPI_ThreadProperty_INITIALIZER OSAPI_THREAD_PROPERTY_DEFAULT

/*i
 * \ingroup OSAPI_ThreadClass
 * \brief Thread info
 *
 */
struct OSAPI_ThreadInfo
{
    /*e Stop executing thread */
    RTI_BOOL stop_thread;

    /*e Whether or not the created thread is preemptive */
    RTI_BOOL is_preemptive;

    /*e Parameter passed by thread creator. Passed to thread. */
    void *user_data;
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

/*i \ingroup OSAPI_ThreadClass
 *
 * \brief Abstract thread class.
 */
struct OSAPI_Thread;

#if !OSAPI_NO_THREADS
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
#endif 

#ifndef RTI_CERT
/*i \ingroup OSAPI_ThreadClass
 *  \brief Suspend a thread for a specified amount of time.
 *
 *  \param [in] ms Sleep time.
 *
 */
OSAPIDllExport void
OSAPI_Thread_sleep(RTI_UINT32 ms);
#endif

#if !OSAPI_NO_THREADS
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
                   const struct OSAPI_ThreadProperty *properties,
                   OSAPI_ThreadRoutine user_routine,
                   void *user_data,
                   OSAPI_ThreadRoutine wakeup_routine);
#endif



/*i \ingroup OSAPI_ThreadClass
 *
 *  \brief Return thread ID.
 *
 *  \return thread ID of the calling thread.
 *
 *  \sa \ref OSAPI_Thread_create
 */
OSAPIDllExport OSAPI_ThreadId
OSAPI_Thread_self(void);

#ifdef __cplusplus
}                               /* extern "C" */
#endif


#endif /* osapi_thread_h */
