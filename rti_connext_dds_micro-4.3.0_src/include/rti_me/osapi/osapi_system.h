/*
 * FILE: osapi_system.h - Definition of System API
 *
 * Copyright (c) 2012-2025 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 15jul2015,tk MICRO-1426/PR#15358 Added support for OSAPI_System_get_ticktime
 * 22mar2014,tk Updated API and documentation
 * 01deb2014,tk MICRO-716: Abstract system API
 * 12mar2012,tk Written
 */
/*ce
 * \file
 * \brief System API definition
 */
#ifndef osapi_system_h
#define osapi_system_h

#ifndef osapi_dll_h
#include "osapi/osapi_dll.h"
#endif
#ifndef osapi_time_h
#include "osapi/osapi_time.h"
#endif
#ifndef osapi_timer_h
#include "osapi/osapi_timer.h"
#endif
#ifndef osapi_semaphore_h
#include "osapi/osapi_semaphore.h"
#endif
#ifndef osapi_thread_h
#include "osapi/osapi_thread.h"
#endif
#ifdef __cplusplus
extern "C"
{
#endif

/*e \dref_OSAPI_SYSTEM_MAX_HOSTNAME
 */
#define OSAPI_SYSTEM_MAX_HOSTNAME (64)

/*e \dref_OSAPISystemGroupDocs
 */
struct OSAPI_System;

/*ci
 * \brief The first valid object ID returned by functions generating object IDs.
 *
 * \details
 * DDS_DomainParticipant_get_next_objectid generates object IDs for DDS
 * entities.
 */
#define OSAPI_SYSTEM_OBJECTID_START 0x01000000U

/*ci
 * \brief The maximum returned valid object ID returned by functions
 *        generating object IDs is OSAPI_SYSTEM_OBJECTID_MAX - 1.
 *
 * \details
 * DDS_DomainParticipant_get_next_objectid generates object IDs for DDS
 * entities.
 */
#define OSAPI_SYSTEM_OBJECTID_MAX   0x7fffffffU

/*e \dref_OSAPI_System_start_timer_T
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*OSAPI_System_start_timer_T)(OSAPI_Timer_T self,
                              OSAPI_TimerTickHandlerFunction tick_handler)
)

#ifndef RTI_CERT
#define RTI_INCLUDE_SYSTEM_FINALIZER (1)
#else
#define RTI_INCLUDE_SYSTEM_FINALIZER (0)
#endif

#if RTI_INCLUDE_SYSTEM_FINALIZER
/*i \ingroup OSAPI_SystemClass
 *
 *  stop timer definition
 */
FUNCTION_SHOULD_TYPEDEF(
RTI_BOOL
(*OSAPI_System_stop_timer_T)(OSAPI_Timer_T self)
)
#endif

/*e \dref_OSAPI_System_get_timer_resolution_T
 */
FUNCTION_SHOULD_TYPEDEF(
RTI_INT32
(*OSAPI_System_get_timer_resolution_T)(void)
)

/*e \dref_OSAPI_System_get_time_T
 */
FUNCTION_SHOULD_TYPEDEF(
RTI_BOOL
(*OSAPI_System_get_time_T)(OSAPI_SystemTime *now)
)

/*e \dref_OSAPI_System_initialize_T
 */
FUNCTION_SHOULD_TYPEDEF(
RTI_BOOL
(*OSAPI_System_initialize_T)(void)
)

#if RTI_INCLUDE_SYSTEM_FINALIZER
/*i \ingroup OSAPI_SystemClass
 *
 * finalize
 */
FUNCTION_SHOULD_TYPEDEF(
RTI_BOOL
(*OSAPI_System_finalize_T)(void)
)
#endif

/*e \dref_OSAPI_System_get_hostname_T
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*OSAPI_System_get_hostname_T)(char *const hostname)
)

struct OSAPI_SystemUUID;

/*e \dref_OSAPI_System_generate_uuid_T
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*OSAPI_System_generate_uuid_T)(struct OSAPI_SystemUUID *uuid_out)
)

/*e \dref_OSAPI_System_get_ticktime_T
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*OSAPI_System_get_ticktime_T)(RTI_INT32 *sec,RTI_UINT32 *nanosec)
)

FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*OSAPI_System_task_scheduler_start_T)(void)
)

FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*OSAPI_System_task_scheduler_stop_T)(void)
)

#if RTI_INCLUDE_SYSTEM_FINALIZER
#define RTI_INCLUDE_SYSTEM_FINALIZER_INITIALIZER \
    NULL,\
    NULL,
#else
#define RTI_INCLUDE_SYSTEM_FINALIZER_INITIALIZER
#endif

/*e \dref_OSAPI_SystemI
 */
typedef struct OSAPI_SystemI
{
    /*e \dref_OSAPI_SystemI_start_timer
     */
    OSAPI_System_start_timer_T start_timer;

#if RTI_INCLUDE_SYSTEM_FINALIZER
    /*i \dref_OSAPI_SystemI_stop_timer
     */
    OSAPI_System_stop_timer_T stop_timer;
#endif

    /*e \dref_OSAPI_SystemI_get_timer_resolution
     */
    OSAPI_System_get_timer_resolution_T get_timer_resolution;

    /*e \dref_OSAPI_SystemI_get_time
     */
    OSAPI_System_get_time_T get_time;

    /*i \dref_OSAPI_SystemI_initialize
     */
    OSAPI_System_initialize_T initialize;

#if RTI_INCLUDE_SYSTEM_FINALIZER
    /*i \dref_OSAPI_SystemI_finalize
     */
    OSAPI_System_finalize_T finalize;
#endif

    /*e \dref_OSAPI_SystemI_generate_uuid
     */
    OSAPI_System_generate_uuid_T generate_uuid;

    /*e \dref_OSAPI_SystemI_get_hostname
     */
    OSAPI_System_get_hostname_T get_hostname;

    /*e \dref_OSAPI_SystemI_get_ticktime
     */
    OSAPI_System_get_ticktime_T get_ticktime;

    /*ci
     * \brief Start the task scheduler
     *
     */
    OSAPI_System_task_scheduler_start_T task_scheduler_start;

    /*ci
     * \brief Start the task scheduler
     *
     */
    OSAPI_System_task_scheduler_stop_T task_scheduler_stop;
} OSAPI_SystemI;

/*ce \dref_OSAPI_SystemI_INITIALIZER
 */
#define OSAPI_SystemI_INITIALIZER \
{\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    RTI_INCLUDE_SYSTEM_FINALIZER_INITIALIZER\
    NULL,\
    NULL\
}

extern OSPSLDllVariable struct OSAPI_System *OSAPI_System_gv_System;

/*ce \dref_OSAPI_System_set_interface
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_System_set_interface(struct OSAPI_SystemI *intf);

#ifndef RTI_CERT
/*i \ingroup OSAPI_SystemClass
 *  \brief Get the current system interface
 *
 *  \details
 *  This function returns the current system interface
 *
 *  \param [out] intf - The current system interface
 *
 *  \return RTI_TRUE on success, RTI_FALSE on failure.
 */
SHOULD_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_System_get_interface(struct OSAPI_SystemI *intf);
#endif

/*i \ingroup OSAPI_SystemClass
 *
 *  System listener
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*OSAPI_System_on_system_initialize_T)(void *listener_data,struct OSAPI_System *system)
)

typedef void
(*OSAPI_System_on_system_finalize_T)(void *listener_data,struct OSAPI_System *system);

/*i \ingroup OSAPI_SystemClass
 *
 *  System listener
 */
struct OSAPI_SystemListener
{
    void *listener_data;
    OSAPI_System_on_system_initialize_T on_system_initialize;
    OSAPI_System_on_system_finalize_T on_system_finalize;
};

#define OSAPI_SystemListener_INITIALIZER \
{\
    NULL,\
    NULL,\
    NULL,\
}

#ifndef RTI_CERT
/*i \ingroup OSAPI_SystemClass
 *  \brief Install a system listeners
 *
 *  \details
 *
 *  The system listeners are called during the life-time of the system.
 *  Only listeners which are non-NULL are called, thus it is ok to install
 *  a partially set listener structure. System listeners can be installed
 *  until the system has been initialized by calling OSAPI_System_initialize.
 *  This function is not considered thread-safe. The listener value should
 *  always be initialized with OSAPI_SystemListener_DEFAULT before use.
 *
 *  \param [in] listener System listeners to call
 *
 *  \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_System_set_listener(struct OSAPI_SystemListener *listener);

/*i \ingroup OSAPI_SystemClass
 *  \brief Get current system listeners
 *
 *  \details
 *  Return the current set of system listeners. This function is not thread-safe.
 *
 *  \param[in] listener System listener structure to fill in.
 *
 *  \return RTI_TRUE on success, RTI_FALSE on failure.
 */
SHOULD_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_System_get_listener(struct OSAPI_SystemListener *listener);
#endif

/*e \dref_OSAPI_TaskProperty
 */
struct OSAPI_TaskProperty
{
    /*e \dref_OSAPI_TaskProperty_thread
     *
     * \brief Task thread properties
     */
    struct OSAPI_ThreadProperty thread;

    /*e \dref_OSAPI_TaskProperty_rate
     *
     * \brief The rate at which periodic tasks are run in nanosec
     */
    RTI_UINT32 rate;
};

/*e \dref_OSAPI_TaskProperty_INITIALIZER
 */
#define OSAPI_TaskProperty_INITIALIZER \
{\
    {  /* OSAPI_ThreadProperty */ \
        OSAPI_THREAD_USE_OSDEFAULT_STACKSIZE,\
        OSAPI_THREAD_PRIORITY_DEFAULT,\
        OSAPI_THREAD_DEFAULT_OPTIONS\
    },\
    100\
}

/*e \dref_OSAPI_SystemProperty
 *  \ingroup OSAPI_SystemClass
 *
 *  System properties
 */
struct OSAPI_SystemProperty
{
    /*e \dref_OSAPI_SystemProperty_timer_property
     */
    struct OSAPI_TimerProperty timer_property;

    /*e \dref_OSAPI_SystemProperty_hostname
     */
    char hostname[OSAPI_SYSTEM_MAX_HOSTNAME];

    /*e \dref_OSAPI_SystemProperty_max_user_blocking_threads
     */
    RTI_INT32 max_user_blocking_threads;

    /*e \dref_OSAPI_SystemProperty_max_timers
     */
    RTI_INT32 max_timers;

    /*e Properties for the task scheduler
     */
    struct OSAPI_TaskProperty task_scheduler;
};

/*e \dref_OSAPI_SystemProperty_INITIALIZER
 */
#define OSAPI_SystemProperty_INITIALIZER \
{\
    OSAPI_TimerProperty_INITIALIZER,\
    {0},\
    32,\
    8,\
    OSAPI_TaskProperty_INITIALIZER \
}

/*i \ingroup OSAPI_SystemClass
 * \brief UUID definition
 *
 *  Abstract UUID object,a 128-bit value.
 */
struct OSAPI_SystemUUID
{
    RTI_UINT32 value[4];
};

/*i \ingroup OSAPI_SystemClass
 * \brief Generate a unique universal identifier (UUID)
 *
 * \param [out] uuid_out The generated UUID value
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_System_generate_uuid(struct OSAPI_SystemUUID *uuid_out);

/*i \ingroup OSAPI_SystemClass
 * \brief Get the current system time.
 *
 *  \details
 * In general, the system time is used by components to correlate both
 * internal and external events, such as data reception and ordering. Thus,
 * it is recommended that this function returns the real time. However, it is not
 * strictly required.
 *
 * Notes:
 * - It is assumed that the time returned by get_time is monotonically
 *   increasing. It is up to the implementation of this function to ensure
 *   this holds true.
 * - It is ok to return the same time as the last call.
 * - The clock used to report real-time can be different than the clock used to
 *   support start_timer and stop_timer.
 *
 * @param [out] now The current time.
 *
 * @return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * @pre Initialized system.
 *
 * @exception None.
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_System_get_time(OSAPI_SystemTime *now);

/*i \ingroup OSAPI_SystemClass
 *  \brief Get the resolution of the clock driving the timer in nano seconds.
 *
 *  \details
 *
 *  This function must return the frequency of the system timer used to implement
 *  OSAPI_SystemI::start_timer and OSAPI_SystemI::stop_timer API.
 *
 *  @return timer resolution in nanoseconds.
 */
OSAPIDllExport RTI_INT32
OSAPI_System_get_timer_resolution(void);

/*i \ingroup OSAPI_SystemClass
 *
 * \brief Start the timer.
 *
 * @param [in] self         Timer object.
 * @param [in] tick_handler Timer handle.
 *
 * @pre Initialized system.
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_System_start_timer(OSAPI_Timer_T self,
                        OSAPI_TimerTickHandlerFunction tick_handler);

#ifndef RTI_CERT
/*i \ingroup OSAPI_SystemClass
 * \brief Stop the timer.
 *
 * @param [in] self Timer
 *
 * @return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * @pre Initialized system.
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_System_stop_timer(OSAPI_Timer_T self);
#endif

/*i \ingroup OSAPI_SystemClass
 *  \brief Initialize the system.
 *
 *  \details
 *
 *  This function initializes the system and calls the port specific initialize
 *  method first. The port specific initialization method must return RTI_TRUE
 *  on success and RTI_FALSE on failure. A system can only be initialized once.
 *
 *  @return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_System_initialize(void);

#ifndef RTI_CERT
/*i \ingroup OSAPI_SystemClass
    \brief Finalize the system.

    \details

    This function finalizes the system and calls the port specific finalize
    method last. The port specific initialization method must return RTI_TRUE
    on success and RTI_FALSE on failure.

    @return RTI_TRUE on success, RTI_FALSE on failure.
*/
SHOULD_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_System_finalize(void);
#endif /* !RTI_CERT */

/*e \dref_OSAPI_System_get_property
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_System_get_property(struct OSAPI_SystemProperty *property);

/*e \dref_OSAPI_System_set_property
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_System_set_property(struct OSAPI_SystemProperty *property);

/*e \dref_OSAPI_System_get_native_interface
 */
OSPSLDllExport void
OSAPI_System_get_native_interface(struct OSAPI_SystemI *intf);

/*i \ingroup OSAPI_SystemClass
 * \brief Get the hostname
 *
 * \details
 * Get the hostname
 *
 * @param [out] hostname  The buffer to store the hostname. Must be at
 *                        least OSAPI_SYSTEM_MAX_HOSTNAME bytes. If the
 *                        actual hostname is longer than
 *                        OSAPI_SYSTEM_MAX_HOSTNAME bytes (including \0) the
 *                        hostname is truncated.
 *
 * @pre Initialized system.
 *
 * @exception None.
 *
 * @return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * @mtsafety SAFE
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_System_get_hostname(char *const hostname);


/*i \ingroup OSAPI_SystemClass
 *
 * \brief Get current tick time
 *
 * \details
 *
 * The ticktime is a time measurement used by Micro to determine how much
 * time has elapsed in a period. It does not have to be an absolute time,
 * but it _must_ be monotonically increasing. For this reason it is important
 * to choose the time source with care. For example, the system time may
 * mbe adjusted backward or forward and this could affect the measure time
 * lapse. The resolution of the tick time is expected to be no less than the
 * system timer, although it is not a requirement.
 *
 * @param[out] sec     The current ticktime in seconds
 *
 * @param[out] nanosec Additional nanoseconds in the current ticktime
 *
 * @pre Initialized system.
 *
 * @return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_System_get_ticktime(RTI_INT32 *sec,RTI_UINT32 *nanosec);

#if OSAPI_THREAD_SEMAPHORE_ENABLED
/*i \ingroup OSAPI_SystemClass
 *
 * \brief Get a semaphore to suspend a thread
 *
 * \return A semaphore on success, NULL on failure.
 */
MUST_CHECK_RETURN OSAPIDllExport OSAPI_Semaphore_T*
OSAPI_System_get_thread_semaphore(void);

/*e \ingroup OSAPI_SystemClass
 *
 * \brief Get a semaphore to suspend a user thread
 *
 * \return A semaphore on success, NULL on failure.
 */
MUST_CHECK_RETURN OSAPIDllExport OSAPI_Semaphore_T*
OSAPI_System_get_user_thread_semaphore(void);

/*e \ingroup OSAPI_SystemClass
 *
 * \brief Return a semaphore acquired with \ref OSAPI_System_get_thread_semaphore
 *
 * \param[in] sem Semaphore to return
 */
OSAPIDllExport void
OSAPI_System_return_thread_semaphore(OSAPI_Semaphore_T *sem);

/*e \ingroup OSAPI_SystemClass
 *
 * \brief Return a user semaphore acquired with \ref OSAPI_System_get_thread_semaphore
 */
OSAPIDllExport void
OSAPI_System_return_user_thread_semaphore(OSAPI_Semaphore_T *sem);

/*e \ingroup OSAPI_SystemClass
 *
 * \brief Add a semaphore to possible suspend a thread created with
 *        OSAPI_THREAD_SUSPEND_ENABLE
 *
 * \details
 * Threads that are created with the OSAPI_THREAD_SUSPEND_ENABLE option
 * may be suspended in the receive path. When a thread is created with this
 * option enabled a semaphore is automatically created and added to a pool
 * by calling this method.
 *
 * Code paths that need to suspend the thread should use \ref
 * OSAPI_System_get_thread_semaphore and \ref
 * OSAPI_System_return_thread_semaphore.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_System_add_thread_semaphore(void);

#ifndef RTI_CERT
/*e \ingroup OSAPI_SystemClass
 *
 * \brief Delete a semaphore previously added with \ref
 * OSAPI_System_add_thread_semaphore.
 *
 * \details
 * When a thread is deleted that was created with the
 * OSAPI_THREAD_SUSPEND_ENABLE this function is automatically to delete
 * a semaphore from the pool.
 */
OSAPIDllExport void
OSAPI_System_delete_thread_semaphore(void);
#endif

#endif


#include "osapi/osapi_types.h"
#include "osapi/osapi_log.h"
#include "osapi/osapi_string.h"
#include "osapi/osapi_semaphore.h"

/*ci \brief Generic description of a system
 */
struct OSAPI_System
{
    /*ci \brief Flag to indicate if the system has been initialized
     */
    RTI_BOOL is_initialized;

    /*ci \brief The properties the system was initialized with
     */
    struct OSAPI_SystemProperty property;

    /*ci \brief The user defined system interface
     */
    struct OSAPI_SystemI u_intf;

    /*ci \brief The native system interface
     */
    struct OSAPI_SystemI n_intf;

    /*ci \brief The system interface ins use, u_intf + n_intf
     */
    struct OSAPI_SystemI r_intf;

    /*ci \brief User installed system listener
     */
    struct OSAPI_SystemListener listener;

    /*ci \brief TRUE if start_timer, stop_timer and get_ticktime are internal
     */
    RTI_BOOL is_timer_owner;

    OSAPI_SystemTime current_time;

    RTI_INT32 user_blocked_threads;

#if OSAPI_THREAD_SEMAPHORE_ENABLED
    /*ci \brief Mutex to protect thread semaphores
     */
    OSAPI_Mutex_T *thread_mutex;

    /*ci \brief Array of thread semaphores to support blocking threads
     */
    OSAPI_Semaphore_T **thread_semaphore;

    /*ci \brief The number of thread semaphores in use
     */
    RTI_INT32 thread_sem_count;

    /*ci \brief The highest index for a thread semaphore in use
     */
    RTI_INT32 thread_sem_last;
#endif
};

#if OSAPI_THREAD_SEMAPHORE_ENABLED
#define OSAPI_THREAD_SEMAPHORE_INITIALIZER \
        ,NULL,\
        NULL,\
        0,\
        0
#else
#define OSAPI_THREAD_SEMAPHORE_INITIALIZER
#endif
/*ci \brief Initialize for the OSAPI_SystemProperty
 */


#define OSAPI_System_INITIALIZER \
{ \
    RTI_FALSE, \
    OSAPI_SystemProperty_INITIALIZER, \
    OSAPI_SystemI_INITIALIZER,\
    OSAPI_SystemI_INITIALIZER,\
    OSAPI_SystemI_INITIALIZER,\
    OSAPI_SystemListener_INITIALIZER,\
    RTI_FALSE,\
    OSAPI_TIME_MAX,\
    0 \
    OSAPI_THREAD_SEMAPHORE_INITIALIZER \
}

#ifndef RTI_CERT
extern RTI_UINT32 OSAPI_System_gv_Size;
#endif /* !RTI_CERT */

/*e \dref_OSAPI_System_clock_tick
 */
OSAPIDllExport RTI_BOOL
OSAPI_System_clock_tick(RTI_UINT32 ticks);

/*e \dref_OSAPI_System_clock_tick_from_time
 */
OSAPIDllExport RTI_BOOL
OSAPI_System_clock_tick_from_time(void);

/*ci \brief Return RTI_TRUE if the internal timer functions are used
 *
 * \return RTI_TRUE is the internal timers are used, RTI_FALSE if not
 */
MUST_CHECK_RETURN OSAPIDllExport RTI_BOOL
OSAPI_System_is_timer_owner(void);

/*ci \brief Return the native Generic system API
 *
 * \param[out] intf The native generic interface
 */
OSAPIDllExport void
OSAPI_SystemGeneric_get_native_interface(struct OSAPI_SystemI *intf);

#ifndef RTI_CERT
/*ci \brief Set whether to wait for a timer to acknowledge deletion or not.
 *
 * \details
 * When timers are stopped, but are updated from a different thread than the
 * thread stopping the timer, it may be necessary to ensure that the
 * timer can no longer be updated.
 *
 * Calling this function with RTI_TRUE causes the timer loop to wait until
 * the next timer tick before removing the stopped timer. This ensures that
 * any timer resources will not be accessed after OSAPI_System_stop_timer
 * returns.
 *
 * The default behavior is RTI_FALSE which means not to wait. This value
 * must only be set to RTI_TRUE if the timers are updated from a thread
 * separate from the calling thread, otherwise a call to
 * OSAPI_System_stop_timer will not return.
 *
 * \param[in] value RTI_TRUE means wait for timer to be deleted
 *                  RTI_FALSE means do not wait for timer to be deleted
 *
 */
OSAPIDllExport void
OSAPI_SystemGeneric_set_wait_timer_delete(RTI_BOOL value);
#endif

MUST_CHECK_RETURN RTI_BOOL
OSAPI_System_leak_native_SystemI(struct OSAPI_SystemI **native_SysI);

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif /* osapi_system_h */
