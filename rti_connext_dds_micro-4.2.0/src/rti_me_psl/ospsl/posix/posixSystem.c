/*
 * FILE: posixSystem.c - POSIX system functionality
 *
 * Copyright (c) 2012-2024 Real-Time Innovations, Inc.
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
 * 13dec2021,tk MICRO-3226/PR.29479
 * - Removed static variable uuid_counter.
 * 09nov2021,tk MICRO-3330/PR.29900
 * - Take semaphore with timeout value OSAPI_SEMAPHORE_TIMEOUT_EXTERNAL in
 *   OSAPI_SystemPosix_timer_thread
 * - Corrected timer deletion in OSAPI_SystemPosix_stop_tick (not relevant for
 *   Cert)
 * - Prevent sem->ticks value from being less than 0 in
 *   OSAPI_SystemPosix_update_semaphores.
 * 19jul2021,tk MICRO-3110/PR.29337
 * - Simplified OSAPI_SystemPosix_add_timed_sem() to return if a semaphore
 *   is already on the list.
 * 15apr2021,tk MICRO-2979/PR.27549
 * - Removed unused functions from CERT:
 *   Use OSAPI_NtpTime_from_nanosec instead of OSAPI_NtpTime_from_microsec
 * 13apr2021,tk MICRO-3028/PR.28960
 * - Added robustness check for OSAPI_System_get_native_interface
 * 04apr2021,tk MICRO-2845/PR.28682
 *  - Removed empty blocks in OSAPI_PosixSystem_start_timer()
 *  - Removed empty blocks in OSAPI_SystemPosix_apple_timer_intr()
 * 23feb2021,tk MICRO-2770/PR#28495
 *   - Truncate the default RCC host name in case it is defined too long.
 * 23feb2021,tk MICRO-2904/PR#28728
 *   - Check return value from sigemptyset.
 *   - Removed timer_is_busy when USE_TIMER_SIGNAL and RTI_CERT is TRUE.
 *   - Reorderd #ifdef for CLOCK_MONOTONIC for correct fallback to
 *     CLOCK_REALTIME.
 * 13jan2021,tk MICRO-2807/PR#27549 Removed unused functions for CERT.
 * 10jan2021,tk MICRO-2807/PR#27549  Removed unused functions for CERT
 * 05jan2020,tk
 *   MICRO-2792/PR#28533
 *     - Use self instead of OSAPI_SystemPosix_fv_System in
 *       OSAPI_SystemPosix_run_timers().
 *     - Replaced the runtime test for the resolution with a preprocessor test
 *       in OSAPI_SystemPosix_run_timers().
 *   MICRO-2773/PR#28500
 *     - Updated OSAPI_SystemPosix_update_semaphores() to explicitly ignore
 *       return values that are not used.
 *   MICRO-2772/PR#28499
 *     - Ignore unused return value in OSAPI_SystemPosix_remove_timed_sem()
 *   MICRO-2770/PR#28495
 *     - Fixed OSAPI_SystemPosix_get_hostname() to use gethostname() when
 *       OSAPI_POSIX_GETHOSTNAME is 1 and not retrieve the hostname when
 *       RTI_CERT is defined.
 *   MICRO-2769/PR#28490
 *     - Use self_pid instead of self_int in OSAPI_SystemPosix_generate_uuid()
 *     - Removed self_int from union in OSAPI_SystemPosix_generate_uuid()
 *   MICRO-2763/PR#28429
 *     - Return 0 instead of RTI_FALSE in OSAPI_PosixSystem_get_timer_resolution()
 *   MICRO-2766/PR#28487
 *     - Set intf->finalize to NULL for RTI_CERT in
 *       OSAPI_System_get_native_interface
 * 20oct2020,tk MICRO-2623/PR#28237 Replaced tabs with spaces
 * 02jul2015,tk MICRO-1152 Use _unknown as the hostname if uname is unsupported
 * 15jul2015,tk MICRO-1426/PR#15358 Added support for OSAPI_System_get_ticktime
 * 30oct2013,tk OS X ...: Use GCD for timer sources on 10.6 and higher
 *              Linux ..: Switched back to POSIX timers using CLOCK_MONOTONIC
 *              Other ..: Fallback timer is thread + sleep loop
 * 18oct2013,tk Implemented system interface
 * 20aug2012,tk CR-75 cleanup
 * 04mar2012,tk Rewritten
 *
 */
/*ce
 * \file
 * \brief POSIX implementation of OSAPI system routines
 */
#include "rti_me_psl.h"

#include "posixSystem.h"
#include "posixThread.h"

/*ci \brief Maximum number of timer that can be created. Documented as a
 *   hard resource-limit.
 */
#define OSAPISYSTEM_MAX_TIMERS  OSAPI_SystemPosix_fv_System->_parent.property.max_timers

/*ci \brief The timer resolution in ms
 */
#define OSAPISYSTEM_TIMER_RESOLUTION    10

/*ci \brief The timer resolution in nanoseconds
 */
#define OSAPISYSTEM_TIMER_RESOLUTION_NS (OSAPISYSTEM_TIMER_RESOLUTION * 1000000)

/*ci \brief The signal number to use for the POSIX.4 real-time timer.
 */
#define OSAPISYSTEM_POSIX4_TIMER_SIGNAL (SIGRTMIN)

/*ci
 *  Timer callback information
 */
struct OSAPI_SystemTimerHandler
{
    /*ci \brief The callback
     */
    OSAPI_TimerTickHandlerFunction handler;

    /*ci \brief User defined parameter passed to the callback
     */
    void *param;

    /*ci \brief Flag indicating this timer is enabled or not
     */
    volatile RTI_BOOL enabled;

#ifndef RTI_CERT
    /*ci \brief Flag indicating if this timer is being deleted
     */
    volatile RTI_BOOL deleted;
#endif
};

/*ci
 * POSIX System variables
 */
struct OSAPI_SystemPosix
{
    /*ci \brief base-class
     */
    struct OSAPI_System _parent;

    /*ci \brief Array of timer objects
     */
    struct OSAPI_SystemTimerHandler *timer_handler;

    /*ci \brief List of timed semaphores
     */
    OSAPI_Semaphore_T timed_sem_head;

#ifdef USE_TIMER_THREAD_SEMAPHORE
    /*ci \brief Semaphore to wakeup thread running timers
     */
    OSAPI_Semaphore_T *timer_sem;

    /*ci \brief The thread running timers
     */
    struct OSAPI_PosixNativeThread *timer_thread;
#endif
#if defined(USE_TIMER_SIGNAL) || defined(USE_TIMER_THREAD_RT) || \
    defined(USE_TIMER_THREAD_SLEEP)
    /*ci \brief The timer resolution reported by the system
     */
    struct timespec timer_resolution;
#endif
#if defined(USE_TIMER_SIGNAL) || defined(USE_TIMER_THREAD_RT)
    /*ci \brief The actual time resolution
     */
    struct itimerspec tick_resolution;

    /*ci \brief the OS timer
     */
    timer_t timer;
#ifdef USE_TIMER_SIGNAL
    /*ci \bief Signal mask when using signals for timing
     */
    struct sigaction old_sa;
#endif

    /*ci \brief The id of the real-time clock used
     */
    clockid_t rt_clock;

#ifndef RTI_CERT
    /*ci \brief Flag to indicated a timer has been deleted
     */
    RTI_BOOL timer_deleted;

    /*ci \brief Flag to indicate a timer is busy, used for busy waiting
     */
    RTI_BOOL timer_is_busy;
#endif

#elif defined(USE_TIMER_GCD)

    /*ci \brief Apples GCD timer
     */
    dispatch_source_t timer;

    /*ci \brief Timer queue
     */
    dispatch_queue_t queue;

    /*ci \brief Flag to indicated a timer has been deleted
     */
    RTI_BOOL timer_deleted;

    /*ci \brief Flag to indicate a timer is busy, used for busy waiting
     */
    RTI_BOOL timer_is_busy;
#endif
#if defined(USE_TIMER_THREAD_SLEEP) || defined(USE_TIMER_THREAD_RT)
    /*ci \brief The thread running timers
     */
    struct OSAPI_PosixNativeThread *timer_thread;
#endif
    /*ci
     * \brief Current tick in seconds
     */
    RTI_INT32 tick_sec;

    /*ci
     * \brief Current tick in nanosecs
     */
    RTI_INT32 tick_nanosec;

    /*ci
     * \brief Tick resolution in nanoseconds
     */
    RTI_INT32 tick_resolution_ns;

    /*ci \brief Lock to ensure atomic updates of ticks
     */
    OSAPI_Mutex_T *tick_mutex;
};

/*ci \brief The POSIX System state
 */
RTI_PRIVATE struct OSAPI_SystemPosix OSAPI_System_g =
{
        ._parent = OSAPI_System_INITIALIZER
};

/*ci \brief Pointer to POSIX system state
 */
RTI_PRIVATE struct OSAPI_SystemPosix *OSAPI_SystemPosix_fv_System = &OSAPI_System_g;

/*ci \brief Global lock, initialized to not taken
 */
pthread_mutex_t OSAPI_SystemPosix_gv_InitLock = PTHREAD_MUTEX_INITIALIZER;

#ifndef RTI_CERT
/*ci \brief Variable with size of the system structure, used for memory usage
 */
RTI_UINT32 OSAPI_System_gv_Size = sizeof(struct OSAPI_SystemPosix);
#endif

/*ci \brief Pointer to the system base-class, used by the common API
 */
struct OSAPI_System *OSAPI_System_gv_System = &OSAPI_System_g._parent;
struct OSAPI_LogEntryI *OSAPI_Log_gv_LogIntf = NULL;

#if !(defined(__APPLE__) && defined(__MACH__) && !defined(RTI_DARWIN7))
/*ci \brief Global counter to create UUID, incremented in each call to generate
 *   a UUID.
 */
RTI_PRIVATE RTI_UINT32 OSAPI_System_fv_UuidCounter = 0xdeadc0de;
#endif

/*** SOURCE_BEGIN ***/

RTI_PRIVATE MUST_CHECK_RETURN RTI_BOOL
OSAPI_SystemPosix_initialize_impl(struct OSAPI_SystemPosix *self);

/*** SOURCE_BEGIN ***/

/******************************************************************************
 *  Internal API
 ******************************************************************************/
/*ci
 * \brief Take the system lock
 *
 * \details
 *
 * Function to protect system resources
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa
 */
MUST_CHECK_RETURN RTI_BOOL
OSAPI_SystemPosix_lock(void)
{
    /* Justification: This is a mutex acquisition wrapper function.
     * The corresponding pthread_mutex_lock is intentionally handled
     * by another function in the call chain and not within this module.
     */
    /* coverity[misra_c_2012_rule_22_16_violation] */
    return (pthread_mutex_lock(&OSAPI_SystemPosix_gv_InitLock) ?
            RTI_FALSE : RTI_TRUE);
}

/*ci
 * \brief Release the system lock
 *
 * \details
 *
 * Release lock taken with \ref OSAPI_System_lock
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa
 */
MUST_CHECK_RETURN RTI_BOOL
OSAPI_SystemPosix_unlock(void)
{
    /* Justification: This is a mutex acquisition wrapper function.
     * The corresponding pthread_mutex_unlock is intentionally handled
     * by another function in the call chain and not within this module.
     */
    /* coverity[misra_c_2012_rule_22_16_violation] */
    return (pthread_mutex_unlock(&OSAPI_SystemPosix_gv_InitLock) ?
            RTI_FALSE : RTI_TRUE);
}

/*ci
 * \brief Start a timer
 *
 * \details
 *
 * Implementation of OSAPI_System_start_timer.
 *
 * @param [in]  self         Timer object.
 * @param [in]  tick_handler Timer handle.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
OSAPI_PosixSystem_start_timer(OSAPI_Timer_T self,
                        OSAPI_TimerTickHandlerFunction tick_handler)
{
    RTI_INT32 i;
    RTI_BOOL brc;

    OSAPI_PRECONDITION((self == NULL) || (tick_handler == NULL),
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("tick_handler",tick_handler == NULL ? NULL : (void*)0x1,
                                        RTI_TRUE);)

    if (!OSAPI_SystemPosix_fv_System->_parent.is_initialized)
    {
        OSAPI_LOG_SYSTEM_TIMER_START(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (!OSAPI_SystemPosix_lock())
    {
        OSAPI_LOG_SYSTEM_TIMER_START(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    for (i = 0; i < OSAPISYSTEM_MAX_TIMERS; ++i)
    {
        if (!OSAPI_SystemPosix_fv_System->timer_handler[i].enabled)
        {
            break;
        }
    }

    if (i == OSAPISYSTEM_MAX_TIMERS)
    {
        OSAPI_LOG_SYSTEM_TIMER_START(OSAPI_LOGKIND_ERROR)
        brc = OSAPI_SystemPosix_unlock();
        /* ignore, already returning error */
        IGNORE_RETVAL(brc);
        return RTI_FALSE;
    }

    OSAPI_SystemPosix_fv_System->timer_handler[i].handler = tick_handler;
    OSAPI_SystemPosix_fv_System->timer_handler[i].param = self;

    /* At this point the timer is enabled and may fire before this function
     * returns.
     */
    OSAPI_SystemPosix_fv_System->timer_handler[i].enabled = RTI_TRUE;

    return OSAPI_SystemPosix_unlock();
}

#ifndef RTI_CERT
/*ci
 * \brief Stop the timer.
 *
 * \details
 * Implementation of OSAPI_System_stop_timer
 *
 * @param [in] self Timer
 *
 * @return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * @pre Initialized system.
 */
RTI_PRIVATE RTI_BOOL
OSAPI_PosixSystem_stop_timer(OSAPI_Timer_T self)
{
    RTI_INT32 i;

    OSAPI_PRECONDITION((self == NULL),
                            return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (!OSAPI_SystemPosix_fv_System->_parent.is_initialized)
    {
        OSAPI_LOG_SYSTEM_TIMER_STOP(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    for (i = 0; i < OSAPISYSTEM_MAX_TIMERS; ++i)
    {
        if (OSAPI_SystemPosix_fv_System->timer_handler[i].param == self)
        {
            OSAPI_SystemPosix_fv_System->timer_handler[i].deleted = RTI_TRUE;

            /* Wait for the tick routine to acknowledge the deletion */
            while (OSAPI_SystemPosix_fv_System->timer_handler[i].deleted);

            /* The timer routine has acknowledged the deletion, caller can
             * safely free resources on return. A context switch is safe.
             */
        }
    }

    return RTI_TRUE;
}
#endif

/*ci
 *  \brief Get the resolution of the clock driving the timer in nanoseconds
 *
 *  \details
 *  Implementation of OSAPI_System_get_timer_resolution
 *
 *  This function returns the frequency of the system timer used to implement
 *  OSAPI_SystemI::start_timer and OSAPI_SystemI::stop_timer API.
 *
 *  @return timer resolution in nanoseconds or 0 if the system has not been
 *          initialized.

 */
RTI_INT32
OSAPI_PosixSystem_get_timer_resolution(void)
{
    if (!OSAPI_SystemPosix_fv_System->_parent.is_initialized)
    {
        return 0;
    }

#if defined(__APPLE__)
    return OSAPISYSTEM_TIMER_RESOLUTION * 1000000;
#elif !defined(USE_TIMER_THREAD_SLEEP)
    return (RTI_INT32)OSAPI_SystemPosix_fv_System->tick_resolution.it_interval.tv_nsec;
#else
    return OSAPI_SystemPosix_fv_System->tick_resolution_ns;
#endif
}

/* called withint syslock() */
RTI_PRIVATE void
OSAPI_SystemPosix_update_semaphores(struct OSAPI_SystemPosix *self)
{
    OSAPI_Semaphore_T *a_sem = self->timed_sem_head.next;
    OSAPI_Semaphore_T *n_sem = NULL;
    RTI_INT32 status;

    while (a_sem != &self->timed_sem_head)
    {
        n_sem = a_sem->next;
        status = pthread_mutex_lock(&a_sem->sem_mutex);
        if (status == 0)
        {
            /* a_sem->ticks is at least 1 to be in the list
             * NOTE: Prevent a_sem->ticks from being less than 0.
             * Without this test a_sem->ticks may become -2, interfering
             * with the special value for a externally timed semaphore.
             */
            if (a_sem->ticks > 0)
            {
                --a_sem->ticks;
            }

            if ((a_sem->remove) || (a_sem->ticks == 0))
            {
                /* Coverity incorrectly flags a_sem as modifiable by multiple
                 * racing threads. However, only a single location calls
                 * OSAPI_SystemPosix_update_semaphores, and it's protected by
                 * the sys lock. Mark this warning as a false positive.
                 */
                /* coverity[thread1_overwrites_value_in_field : FALSE] */
                a_sem->prev->next = a_sem->next;
                /* coverity[thread1_overwrites_value_in_field : FALSE] */
                a_sem->next->prev = a_sem->prev;
                /* coverity[thread1_overwrites_value_in_field : FALSE] */
                a_sem->prev = NULL;
                /* coverity[thread1_overwrites_value_in_field : FALSE] */
                a_sem->next = NULL;
            }
            if ((a_sem->ticks == 0) && (!a_sem->remove))
            {
                RTI_BOOL retval;
                /* a_sem->remove could be set just before entering the critical
                 * section, don't signal twice. Ignore the return value since
                 * this function does not return an error and there could be
                 * more semaphores to update. Failure to update the semaphore
                 * would indicate a serious OS level error.
                 */
                retval = OSAPI_SystemPosix_semaphore_timeout(a_sem,RTI_TRUE);
                IGNORE_RETVAL(retval);
            }
            status = pthread_mutex_unlock(&a_sem->sem_mutex);
#if OSAPI_ENABLE_LOG
            if (status != 0)
            {
                OSAPI_LOG_SEMAPHORE_TAKE(OSAPI_LOGKIND_ERROR,status)
            }
#else
            IGNORE_RETVAL(status);
#endif
        }
        else
        {
#if OSAPI_ENABLE_LOG
            OSAPI_LOG_SEMAPHORE_TAKE(OSAPI_LOGKIND_ERROR,status)
#else
            /* Ignore the return value since there is nothing else to do,
             * move to the next semaphore.
             */
            IGNORE_RETVAL(status);
#endif
        }

        a_sem = n_sem;
    }
}

/*ci
 * \brief Run all the system timers
 *
 * \details
 *
 * Update periodic system timers created with \ref OSAPI_System_start_timer.
 *
 * \param[in] self Pointer to POSIX system instance
 */
RTI_PRIVATE void
OSAPI_SystemPosix_run_timers(struct OSAPI_SystemPosix *self)
{
    RTI_INT32 i;
    RTI_INT32 last_tick_ns;
    RTI_BOOL bretval;

    for (i = 0; (i < OSAPISYSTEM_MAX_TIMERS); ++i)
    {
        /* Any timer with enabled set to true must be valid, it takes
         * at most one tick to delete a timer.
         */
        if (self->timer_handler[i].enabled)
        {
            self->timer_handler[i].handler(self->timer_handler[i].param);
        }
#ifndef RTI_CERT
        if (self->timer_handler[i].deleted)
        {
            self->timer_handler[i].deleted = RTI_FALSE;

            self->timer_handler[i].enabled = RTI_FALSE;

            /* At this point the timer may get re-allocated and set the
             * enabled flag to true.
             */
        }
#endif
    }

    /* Must take a sys lock because of timed semaphores */
    if (!OSAPI_SystemPosix_lock())
    {
        return;
    }

    OSAPI_SystemPosix_update_semaphores(self);

    bretval = OSAPI_SystemPosix_unlock();
    IGNORE_RETVAL(bretval);

    if (!OSAPI_Mutex_take(self->tick_mutex))
    {
        return;
    }


    last_tick_ns = self->tick_nanosec;

#if (OSAPISYSTEM_TIMER_RESOLUTION_NS >= OSAPI_TIME_NSEC_PER_SEC)
        /* Add seconds in the resolution to make sure the remainder is less
         * than 1 sec
         */
        self->tick_sec = self->tick_sec +
               (OSAPISYSTEM_TIMER_RESOLUTION_NS / OSAPI_TIME_NSEC_PER_SEC);

        /* Add the remaining nanoseconds in the resolution to the
         * current nanosec counter, but do not go past 1 sec.
         */
        self->tick_nanosec = (self->tick_nanosec +
             (OSAPISYSTEM_TIMER_RESOLUTION_NS % OSAPI_TIME_NSEC_PER_SEC)) %
                         OSAPI_TIME_NSEC_PER_SEC;
#else
        /* Add the remaining nanoseconds in the resolution to the
         * current nanosec counter, but do not go past 1 sec.
         */
        self->tick_nanosec = (self->tick_nanosec +
                OSAPISYSTEM_TIMER_RESOLUTION_NS) % OSAPI_TIME_NSEC_PER_SEC;
#endif

    /* If self->tick_nanosec wrapped around, add 1 more second */
    if (self->tick_nanosec < last_tick_ns)
    {
         self->tick_sec++;
    }

    /* Justification: OSAPI_Mutex_give is a thin wrapper function around
     * pthread_mutex_lock. The rule's strict interpretation flags this
     * because the definition of OSAPI_Mutex_give resides in a different
     * compilation unit than this calling function. The mutex is
     * correctly acquired and released within the same function.
     */
    /* coverity[misra_c_2012_rule_22_16_violation] */
    bretval = OSAPI_Mutex_give(self->tick_mutex);
    IGNORE_RETVAL(bretval);
}

#if defined(USE_TIMER_GCD)
/*ci
 * \brief Apple timer interrupt
 *
 * \param[in] source The source of the timer interrupt, passed from OS
 *
 */
RTI_PRIVATE void
OSAPI_SystemPosix_apple_timer_intr(void *source)
{
    RTI_BOOL brc;

    UNUSED_ARG(source);

    if (OSAPI_SystemPosix_fv_System->timer_deleted)
    {
        OSAPI_SystemPosix_fv_System->timer_is_busy = RTI_FALSE;
        return;
    }

#ifdef USE_TIMER_THREAD_SEMAPHORE
    brc = OSAPI_Semaphore_give(OSAPI_SystemPosix_fv_System->timer_sem);
    /* nothing to do */
    IGNORE_RETVAL(brc);
#else
    OSAPI_SystemPosix_run_timers(OSAPI_SystemPosix_fv_System);
#endif
}
#elif defined(USE_TIMER_SIGNAL)
/*ci
 * \brief Unix signal handle when using signals for timers
 *
 * \param[in] sig The signal that was raised
 * \param[in] extra Pointer to additional system specific data about the signal
 * \param[in] param User defined parameter
 *
 */
RTI_PRIVATE void
OSAPI_SystemPosix_timer_intr(int sig,siginfo_t *extra,void *param)
{
    struct OSAPI_SystemPosix *self = OSAPI_SystemPosix_fv_System;
    UNUSED_ARG(sig);
    UNUSED_ARG(param);
    UNUSED_ARG(extra);

#ifndef RTI_CERT
    if (self->timer_deleted)
    {
        OSAPI_SystemPosix_fv_System->timer_is_busy = RTI_FALSE;
        return;
    }
#endif

#ifdef USE_TIMER_THREAD_SEMAPHORE
    {
        RTI_BOOL retval;

        /* This is a callback function from the OS with no return value. This
         * there is no failure to return.
         */
        retval = OSAPI_Semaphore_give(self->timer_sem);
        IGNORE_RETVAL(retval);
    }
#else
    OSAPI_SystemPosix_run_timers(self);
#endif
}
#elif defined(USE_TIMER_THREAD_RT)
/*ci
 * \brief Timer thread when using a real-time thread to run timers
 *
 * \param[in] thread_param Thread information
 *
 * \return TRUE on successful termination, FALSE on error
 *
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_SystemPosix_timer_thread(struct OSAPI_ThreadInfo* thread_param)
{
    struct OSAPI_SystemPosix *self =
                        (struct OSAPI_SystemPosix *)thread_param->user_data;
    int rval;
    struct timespec next_iter_start_time;
    struct timespec time_remain;

    while (!thread_param->stop_thread)
    {
        next_iter_start_time.tv_sec = 0;
        next_iter_start_time.tv_nsec = self->tick_resolution.it_interval.tv_nsec;

        do
        {
            rval = clock_nanosleep(self->rt_clock, 0,
                                   &next_iter_start_time, &time_remain);
            if (rval == EINTR)
            {
                next_iter_start_time = time_remain;
            }
        } while ((rval == EINTR) && (!thread_param->stop_thread));

        OSAPI_SystemPosix_run_timers(self);
    }

    return RTI_TRUE;
}
#elif defined(USE_TIMER_THREAD_SLEEP)
/*ci
 * \brief Timer thread when using a sleep loop
 *
 * \param[in] thread_param Thread information
 *
 * \return TRUE on successful termination, FALSE on error
 *
 */
RTI_PRIVATE RTI_BOOL
OSAPI_SystemPosix_timer_thread(struct OSAPI_ThreadInfo *thread_param)
{
    struct OSAPI_SystemPosix *self =
                            (struct OSAPI_SystemPosix*)thread_param->user_data;
    struct timespec now,next;
    int rval;

    while (!thread_param->stop_thread)
    {
        OSAPI_SystemPosix_run_timers(self);

        next.tv_sec = 0;
        next.tv_nsec = self->tick_resolution_ns;

        do
        {
            rval = nanosleep(&next,&now);
            if ((rval == -1) && (errno == EINTR))
            {
                next = now;
            }
        } while ((rval == -1) && (errno == EINTR));
    }

    return RTI_TRUE;
}
#else
#error "Undefined tick generation"
#endif

#if defined(USE_TIMER_THREAD_SEMAPHORE)

#ifndef RTI_CERT
/*ci
 * \brief Wakeup periodic timer thread
 *
 * \details
 *
 * The periodic timer thread may wake up based in a semaphore being
 * giving in a timer-interrupt. This function gives the semaphore forcefully
 * to make sure the thread is woken up when it is time to finalize the system.
 *
 * \param[in] thread_param - VxWorks system thread structure
 *
 * \return RTI_TRUE on successful wake-up call, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_SystemPosix_timer_thread_wakeup(struct OSAPI_ThreadInfo* thread_param)
{
    struct OSAPI_SystemPosix *self =
                        (struct OSAPI_SystemPosix *)thread_param->user_data;

    if (!OSAPI_Semaphore_give(self->timer_sem))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}
#endif

/*ci
 * \brief Thread which runs the OS system timers.
 *
 * \details
 *
 * This thread runs run until the system is finalized and updates system
 * timers. The period is determined by the system clock resolution.
 *
 * \param[in] thread_param - Timer thread parameters
 *
 * return RTI_TRUE on successful completion, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_SystemPosix_timer_thread(struct OSAPI_ThreadInfo* thread_param)
{
    struct OSAPI_SystemPosix *self =
                        (struct OSAPI_SystemPosix *)thread_param->user_data;
    RTI_INT32 ec;

    while (!thread_param->stop_thread)
    {
        if (!OSAPI_Semaphore_take(self->timer_sem,
                                  OSAPI_SEMAPHORE_TIMEOUT_EXTERNAL,&ec))
        {
            continue;
        }

#ifndef RTI_CERT
        if (thread_param->stop_thread)
        {
            break;
        }
#endif

        OSAPI_SystemPosix_run_timers(self);
    }
    return RTI_TRUE;
}
#endif

/*ci
 * \brief Start a timer
 *
 * \details
 *
 * Implementation of OSAPI_System_start_timer.
 *
 * @param [in]  self         Timer object.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
OSAPI_SystemPosix_start_tick(struct OSAPI_SystemPosix *self)
{
    RTI_BOOL retval = RTI_FALSE;

#if defined(USE_TIMER_SIGNAL)
    struct sigaction sa;
    struct sigevent timer_event;
#endif

#if defined(USE_TIMER_SIGNAL) || defined(USE_TIMER_THREAD_RT)
#ifdef CLOCK_MONOTONIC
    self->rt_clock = CLOCK_MONOTONIC;
    if (clock_getres(self->rt_clock,&self->timer_resolution) < 0)
    {
        if (errno == EINVAL)
        {
            /* if CLOCK_MONOTONIC is defined but invalid, fall back
             * to CLOCK_REALTIME.
             */
#endif
            self->rt_clock = CLOCK_REALTIME;
            if (clock_getres(self->rt_clock,&self->timer_resolution) < 0)
            {
                goto done;
            }
#ifdef CLOCK_MONOTONIC
        }
        else
        {
            goto done;
        }
    }
#endif

    if (self->timer_resolution.tv_sec > 0)
    {
        goto done;
    }

    self->tick_resolution.it_interval.tv_sec = 0;

    /* As close as possible to 10ms */
    if (self->timer_resolution.tv_nsec > (OSAPISYSTEM_TIMER_RESOLUTION * 1000000))
    {
        self->tick_resolution.it_interval.tv_nsec =
            self->timer_resolution.tv_nsec;
    }
    else
    {
        self->tick_resolution.it_interval.tv_nsec =
            ((OSAPISYSTEM_TIMER_RESOLUTION * 1000000 / self->timer_resolution.tv_nsec)
                * self->timer_resolution.tv_nsec);
    }

    self->tick_resolution.it_value = self->tick_resolution.it_interval;
    self->tick_resolution_ns = (RTI_INT32)self->tick_resolution.it_interval.tv_nsec;
#endif

#if defined(USE_TIMER_THREAD_SLEEP)
#ifdef RTI_VXWORKS
    if (clock_getres(CLOCK_REALTIME, &self->timer_resolution) < 0)
    {
        goto done;
    }
    self->tick_resolution_ns = self->timer_resolution.tv_nsec;
#else
    self->tick_resolution_ns = (RTI_INT32)OSAPISYSTEM_TIMER_RESOLUTION * 1000000;
#endif /* !RTI_VXWORKS */
#endif

    self->tick_mutex = OSAPI_Mutex_new();
    if (self->tick_mutex == NULL)
    {
        goto done;
    }

#if defined(USE_TIMER_THREAD_SLEEP) || defined(USE_TIMER_THREAD_RT) || \
    defined(USE_TIMER_THREAD_SEMAPHORE)
    self->timer_thread = NULL;
#ifndef USE_TIMER_THREAD_SEMAPHORE
    self->timer_thread = OSAPI_Thread_create_posix_native("osapi_timer",
                            &self->_parent.property.timer_property.thread,
                            OSAPI_SystemPosix_timer_thread,(void*)self,
                            NULL);
#else
#ifndef RTI_CERT
    self->timer_thread = OSAPI_Thread_create_posix_native("osapi_timer",
                            &self->_parent.property.timer_property.thread,
                            OSAPI_SystemPosix_timer_thread,(void*)self,
                            OSAPI_SystemPosix_timer_thread_wakeup);
#else
    self->timer_thread = OSAPI_Thread_create_posix_native("osapi_timer",
                            &self->_parent.property.timer_property.thread,
                            OSAPI_SystemPosix_timer_thread,(void*)self,
                            NULL);
#endif
#endif
    if (self->timer_thread == NULL)
    {
        goto done;
    }
#endif

#if defined(USE_TIMER_GCD)

    self->timer_deleted = RTI_FALSE;
    self->queue = dispatch_queue_create("com.rti.micro.timer", NULL);
    if (self->queue == NULL)
    {
        goto done;
    }

    self->timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER,
                                         0,0,self->queue);
    if (self->timer == NULL)
    {
        goto done;
    }

    dispatch_source_set_timer(self->timer,
                              dispatch_walltime(NULL, 0),
                              (OSAPISYSTEM_TIMER_RESOLUTION * 1000000ull),
                              1ull * NSEC_PER_SEC);

    dispatch_source_set_event_handler_f(self->timer,
                                        OSAPI_SystemPosix_apple_timer_intr);

    dispatch_set_context(self->timer,(void*)self);

    self->timer_is_busy = RTI_TRUE;
    self->tick_resolution_ns = OSAPISYSTEM_TIMER_RESOLUTION * 1000000ull;
    dispatch_resume(self->timer);

#elif defined(USE_TIMER_SIGNAL)

#ifndef RTI_CERT
    self->timer_deleted = RTI_FALSE;
#endif

    if (sigemptyset(&sa.sa_mask) < 0)
    {
        goto done;
    }

    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = OSAPI_SystemPosix_timer_intr;

    if (sigaction(SIGRTMIN,&sa,&self->old_sa) < 0)
    {
        goto done;
    }

    OSAPI_Memory_zero(&timer_event,sizeof(timer_event));

    timer_event.sigev_notify = SIGEV_SIGNAL;
    timer_event.sigev_signo = SIGRTMIN;
    timer_event.sigev_value.sival_ptr = self;

    if (timer_create(self->rt_clock,&timer_event,&self->timer) < 0)
    {
        goto done;
    }

    if (timer_settime(self->timer,0,&self->tick_resolution,NULL) < 0)
    {
        goto done;
    }
#ifndef RTI_CERT
    self->timer_is_busy = RTI_TRUE;
#endif
#endif

    retval = RTI_TRUE;

done:
    return retval;
}

#ifndef RTI_CERT
/*ci
 * \brief Stop the timer.
 *
 * \details
 * Implementation of OSAPI_System_stop_timer
 *
 * @param [in] self Timer
 *
 * @return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * @pre Initialized system.
 */
RTI_PRIVATE RTI_BOOL
OSAPI_SystemPosix_stop_tick(struct OSAPI_SystemPosix *self)
{
#if defined(USE_TIMER_GCD)
    self->timer_deleted = RTI_TRUE;

    while (self->timer_is_busy)
    {
        OSAPI_Thread_sleep(1);
    }

    dispatch_release(self->timer);
    dispatch_release(self->queue);

    self->timer = NULL;
#elif defined(USE_TIMER_SIGNAL)
    self->timer_deleted = RTI_TRUE;

    while (self->timer_is_busy)
    {
        OSAPI_Thread_sleep(1);
    }

#if (ENABLE_FACE_COMPLIANCE == FACE_COMPLIANCE_LEVEL_NONE) || \
    (ENABLE_FACE_COMPLIANCE > FACE_COMPLIANCE_LEVEL_SAFETY_BASE)
    if (timer_delete(self->timer) < 0)
    {
        return RTI_FALSE;
    }
#endif

    if (sigaction(SIGRTMIN,&self->old_sa,NULL) < 0)
    {
        return RTI_FALSE;
    }
#endif

#if defined(USE_TIMER_THREAD_RT) || defined(USE_TIMER_THREAD_SLEEP) || \
      defined(USE_TIMER_THREAD_SEMAPHORE)
    if (!OSAPI_Thread_destroy_posix_native(self->timer_thread))
    {
        return RTI_FALSE;
    }
    self->timer_thread = NULL;
#endif

    return RTI_TRUE;
}
#endif

/*ci
 *
 * \brief Internal POSIX system initialization routine
 *
 * \details
 *
 * This function initializes the OSAPI POSIX system abstraction and
 * sets up the timer handling.
 *
 * \param[in]    self - POSIX system structure

 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 */
RTI_PRIVATE MUST_CHECK_RETURN RTI_BOOL
OSAPI_SystemPosix_initialize_impl(struct OSAPI_SystemPosix *self)
{
    RTI_INT32 i;
    RTI_BOOL retval = RTI_FALSE;

    self->timer_handler = NULL;

    if (OSAPISYSTEM_MAX_TIMERS > 1)
    {
        OSAPI_Heap_allocate_buffer((char**)&self->timer_handler,
                  (RTI_SIZE_T)OSAPISYSTEM_MAX_TIMERS *
                  (RTI_SIZE_T)sizeof(struct OSAPI_SystemTimerHandler),
                  OSAPI_ALIGNMENT_DEFAULT);

        if (self->timer_handler == NULL)
        {
            return RTI_FALSE;
        }
    }
    else
    {
        return RTI_FALSE;
    }

    /* Let the user do their own initialization first */
    for (i = 0; i < OSAPISYSTEM_MAX_TIMERS; ++i)
    {
        self->timer_handler[i].handler = NULL;
        self->timer_handler[i].param = NULL;
        self->timer_handler[i].enabled = RTI_FALSE;
#ifndef RTI_CERT
        self->timer_handler[i].deleted = RTI_FALSE;
#endif
    }

    self->timed_sem_head.next = &self->timed_sem_head;
    self->timed_sem_head.prev = &self->timed_sem_head;

#ifdef USE_TIMER_THREAD_SEMAPHORE
    self->timer_sem = OSAPI_Semaphore_new();
    if (self->timer_sem == NULL)
    {
        goto done;
    }
#endif

    if (OSAPI_System_gv_System->r_intf.start_timer == OSAPI_PosixSystem_start_timer)
    {
        if (!OSAPI_SystemPosix_start_tick(self))
        {
            goto done;
        }
    }


    /* OSAPI_System_initialize() is by design not thread-safe */
    /* coverity[missing_lock] */
    self->tick_sec = 0;
    /* coverity[missing_lock] */
    self->tick_nanosec = 0;

    retval = RTI_TRUE;

done:

    if (!retval)
    {
#ifndef RTI_CERT
        OSAPI_Heap_free_buffer(self->timer_handler);
#endif
        self->timer_handler = NULL;
    }
    return retval;
}

#ifndef RTI_CERT
/*ci
 *
 * \brief Internal POSIX system finalization routine
 *
 * \details
 *
 * This function finalizes the OSAPI POSIX system abstraction and
 * deletes up the timer handling.
 *
 * \param[in]    self - POSIX system structure

 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_SystemPosix_finalize_impl(struct OSAPI_SystemPosix *self)
{
    RTI_BOOL retval = RTI_FALSE;

    if (OSAPI_System_gv_System->r_intf.stop_timer == OSAPI_PosixSystem_stop_timer)
    {
        if (!OSAPI_SystemPosix_stop_tick(self))
        {
            goto done;
        }
    }

    if (self->timer_handler != NULL)
    {
#ifndef RTI_CERT
        OSAPI_Heap_free_buffer(self->timer_handler);
#endif
        self->timer_handler = NULL;
    }

#ifdef USE_TIMER_THREAD_SEMAPHORE
    if (self->timer_sem != NULL)
    {
#ifndef RTI_CERT
        if (!OSAPI_Semaphore_delete(self->timer_sem))
        {
            goto done;
        }
#endif
    }
#endif

#ifndef RTI_CERT
    if (self->tick_mutex != NULL)
    {
        if (!OSAPI_Mutex_delete(self->tick_mutex))
        {
            goto done;
        }
    }
#endif

    retval = RTI_TRUE;

done:

    return retval;
}
#endif

/*ci
 * \brief Get the current system time.
 *
 * \details
 * Implementation of OSAPI_System_get_time
 *
 * @param[out] now Time.
 *
 * @return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * @pre Initialized system.
 */
RTI_PRIVATE RTI_BOOL
OSAPI_SystemPosix_get_time(OSAPI_SystemTime *now)
{
#if ENABLE_FACE_COMPLIANCE > FACE_COMPLIANCE_LEVEL_NONE
    struct timespec current_time;
#else
    struct timeval current_time;
#endif

    OSAPI_PRECONDITION(now == NULL,return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("now",now,RTI_TRUE);)

#if (ENABLE_FACE_COMPLIANCE == FACE_COMPLIANCE_LEVEL_NONE)
    if (gettimeofday(&current_time, NULL) == -1)
    {
        return RTI_FALSE;
    }

    now->sec = (RTI_INT64)current_time.tv_sec;
    now->nanosec = (RTI_UINT32)current_time.tv_usec*1000;
#else
    if (clock_gettime(CLOCK_REALTIME,&current_time) < 0)
    {
        return RTI_FALSE;
    }

    now->sec = (RTI_INT64)current_time.tv_sec;
    now->nanosec = (RTI_UINT32)current_time.tv_nsec;
#endif

    return RTI_TRUE;
}

/*ci
 * \brief Initialize the system.
 *
 * \details
 * Implementation of OSAPI_System_initialize
 *
 * @return RTI_TRUE on success, RTI_FALSE on failure.
 */
RTI_PRIVATE RTI_BOOL
OSAPI_SystemPosix_initialize(void)
{
    return OSAPI_SystemPosix_initialize_impl(OSAPI_SystemPosix_fv_System);
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize the system.
 *
 * \details
 * Implementation of OSAPI_System_finalize
 *
 * @return RTI_TRUE on success, RTI_FALSE on failure.
 */
RTI_PRIVATE RTI_BOOL
OSAPI_SystemPosix_finalize(void)
{
    return OSAPI_SystemPosix_finalize_impl(OSAPI_SystemPosix_fv_System);
}
#endif

/*ci
 * \brief Generate a unique universal identifier (UUID)
 *
 * \details
 * Implementation of OSAPI_System_generate_uuid
 *
 * \param[inout] uuid_out The generated UUID
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
OSAPI_SystemPosix_generate_uuid(struct OSAPI_SystemUUID *uuid_out)
{
#if defined(__APPLE__) && defined(__MACH__) && !defined(RTI_DARWIN7)
    uuid_t uuid_number;
#else
    OSAPI_SystemTime now = OSAPI_TIME_ZERO;
    union
    {
        OSAPI_ProcessId self_id;
        RTI_UINT32 self_uint;
    } self;

#endif /* apple, mach, darwin */

    OSAPI_PRECONDITION((uuid_out == NULL),
                   return RTI_FALSE,
                   OSAPI_Log_entry_add_pointer("uuid_out",uuid_out,RTI_TRUE);)

#if defined(__APPLE__) && defined(__MACH__) && !defined(RTI_DARWIN7)
    uuid_generate(uuid_number);
    OSAPI_Memory_copy(uuid_out,uuid_number,12);
#else
    if (!OSAPI_System_get_time(&now))
    {
        return RTI_FALSE;
    }
#if RTI_ENDIAN_LITTLE
    self.self_uint = OSAPI_System_fv_UuidCounter++;
    uuid_out->value[0] = ((self.self_uint>>24)&0x000000ffU) |
                         ((self.self_uint>>8)&0x0000ff00U)  |
                         ((self.self_uint<<24)&0xff000000U) |
                         ((self.self_uint<<8)&0x00ff0000U);
#else
    uuid_out->value[0] = OSAPI_System_fv_UuidCounter++;
#endif
    self.self_id = OSAPI_Process_getpid();
    uuid_out->value[1] = self.self_uint;
    uuid_out->value[2] = (RTI_UINT32)now.nanosec;
    uuid_out->value[3] = (RTI_UINT32)now.sec;
#endif

    return RTI_TRUE;
}

#ifndef RTI_CERT
/*ci \brief Remove a timed semaphore from the list of timed semaphores
 *
 * \param[in] a_sem Semaphore to remove
 */
void
OSAPI_SystemPosix_remove_timed_sem(OSAPI_Semaphore_T *a_sem)
{
    RTI_BOOL retval;

    if (!OSAPI_SystemPosix_lock())
    {
        return;
    }

    if ((a_sem->next != NULL) && (a_sem->prev != NULL))
    {
        a_sem->prev->next = a_sem->next;
        a_sem->next->prev = a_sem->prev;
    }

    /* Ignore the return value since this function does not return a result */
    retval = OSAPI_SystemPosix_unlock();
    IGNORE_RETVAL(retval);

    return;
}
#endif

/*ci \brief Add a timed semaphore to the list of timed semaphores
 *
 * \param[in] a_sem Semaphore to remove
 */
void
OSAPI_SystemPosix_add_timed_sem(OSAPI_Semaphore_T *a_sem)
{
    struct OSAPI_SystemPosix *self = OSAPI_SystemPosix_fv_System;

    if ((a_sem->prev != NULL) && (a_sem->next != NULL))
    {
        return;
    }

    a_sem->next = self->timed_sem_head.next;
    self->timed_sem_head.next = a_sem;
    a_sem->prev = &self->timed_sem_head;
    a_sem->next->prev = a_sem;
}

/*ci
 * \brief Get the hostname
 *
 * \details
 * Implementation of OSAPI_System_get_hostname
 *
 * \param[in] hostname Hostname to fill in
 *
 * @return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_SystemPosix_get_hostname(char *const hostname)
{
#ifdef RTI_CERT
    const char *cert_host_name = "rti_me_cert";
    RTI_SIZE_T len;

   /*
    * Although there is no way to change the cert_host_name used in
    * OSAPI_SystemPosix_get_hostname() for RCC, truncate the name if
    * the length exceeds OSAPI_SYSTEM_MAX_HOSTNAME in case it is changed
    * to something too long.
    */
    len = OSAPI_String_length(cert_host_name);
    if (len >= OSAPI_SYSTEM_MAX_HOSTNAME)
    {
        len = OSAPI_SYSTEM_MAX_HOSTNAME-1;
    }
    OSAPI_Memory_copy(hostname,cert_host_name,len);
    hostname[len] = 0;
#elif !OSAPI_POSIX_GETHOSTNAME
    struct utsname utsinfo;
    RTI_SIZE_T len;

    if (uname(&utsinfo) < 0)
    {
        return RTI_FALSE;
    }
    /* Truncate hostname if it is too long */
    len = OSAPI_String_length(utsinfo.nodename);
    if (len >= OSAPI_SYSTEM_MAX_HOSTNAME)
    {
        len = OSAPI_SYSTEM_MAX_HOSTNAME-1;
    }
    OSAPI_Memory_copy(hostname,utsinfo.nodename,len);
    hostname[len] = 0;
#else /* !OSAPI_POSIX_GETHOSTNAME */
#if defined(RTI_VXWORKS)
    if (gethostname(hostname, OSAPI_SYSTEM_MAX_HOSTNAME) != OK)
#else
    if (gethostname(hostname, OSAPI_SYSTEM_MAX_HOSTNAME) != 0)
#endif
    {
        return RTI_FALSE;
    }
#endif /* !RTI_CERT */

    return RTI_TRUE;
}

/*ci
 * \brief Get the get_ticktime
 *
 * \details
 * Implementation of OSAPI_System_get_ticktime interface
 *
 * \param[inout] sec     The current tick-time seconds part
 * \param[inout] nanosec The current tick-time nano-seconds part
 *
 * @return This function returns RTI_TRUE on success, RTI_FALSE
 *         on failure.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_PosixSystem_get_ticktime(RTI_INT32 *sec,RTI_UINT32 *nanosec)
{
    /* Justification: OSAPI_Mutex_take is a thin wrapper function around
     * pthread_mutex_lock. The rule's strict interpretation flags this
     * because the definition of OSAPI_Mutex_take resides in a different
     * compilation unit than this calling function. The mutex is
     * correctly acquired and released within the same function.
     */
    /* coverity[misra_c_2012_rule_22_16_violation] */
    if (!OSAPI_Mutex_take(OSAPI_SystemPosix_fv_System->tick_mutex))
    {
        return RTI_FALSE;
    }

    *sec = OSAPI_SystemPosix_fv_System->tick_sec;
    *nanosec = (RTI_UINT32)OSAPI_SystemPosix_fv_System->tick_nanosec;

    /* coverity[misra_c_2012_rule_22_16_violation] */
    if (!OSAPI_Mutex_give(OSAPI_SystemPosix_fv_System->tick_mutex))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/******************************************************************************
 *  Public API
 ******************************************************************************/
struct OSAPI_SystemI OSAPI_PosixSystem_gv_SysIntf =
{
    OSAPI_PosixSystem_start_timer,
#if RTI_INCLUDE_SYSTEM_FINALIZER
#ifndef RTI_CERT
    OSAPI_PosixSystem_stop_timer,
#else
    NULL,
#endif
#endif
    OSAPI_PosixSystem_get_timer_resolution,
    OSAPI_SystemPosix_get_time,
    OSAPI_SystemPosix_initialize,
#if RTI_INCLUDE_SYSTEM_FINALIZER
#ifndef RTI_CERT
    OSAPI_SystemPosix_finalize,
#else
    NULL,
#endif
#endif
    OSAPI_SystemPosix_generate_uuid,
    OSAPI_SystemPosix_get_hostname,
    OSAPI_PosixSystem_get_ticktime,
    NULL,
    NULL
};

struct OSAPI_SystemI *OSAPI_System_gv_SysIntf = &OSAPI_PosixSystem_gv_SysIntf;

void
OSAPI_System_get_native_interface(struct OSAPI_SystemI *intf)
{
    OSAPI_PRECONDITION_ALWAYS(intf == NULL,
                           return,
                           OSAPI_Log_entry_add_pointer("intf",intf,RTI_TRUE);)

    *intf = OSAPI_PosixSystem_gv_SysIntf;
}

MUST_CHECK_RETURN RTI_BOOL
OSAPI_System_leak_native_SystemI(struct OSAPI_SystemI **native_SysI)
{
    OSAPI_System_get_native_interface(*native_SysI);
    return RTI_TRUE;
}

