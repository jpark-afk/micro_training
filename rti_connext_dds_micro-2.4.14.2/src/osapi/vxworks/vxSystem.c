/*
 * FILE: vxSystem.c - VxWorks system functionality
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
 * 18sep2015,tk MICRO-1501/PR#16627 Fixed deadlock in get_ticktime by using a
 *                                  tick mutex
 * 15jul2015,tk MICRO-1426/PR#15358 Added support for OSAPI_System_get_ticktime
 * 15jun2015,tk MICRO-1299/PR#14981 Changed generate_uuid() to return FALSE on
 *                                  on failure
 * 20feb2015,eh MICRO-813/PR#9172 Fix Lint warnings
 * 17feb2015,tk MICRO-161/PR#13665 Removed redundant check in start_timer()
 * 08may2014,eh Revert MICRO-692: remove clock_settime()
 * 30oct2013,tk VxWorks 6 ..: Use CLOCK_MONOTONIC if possible, otherwise use
 *                            CLOCK_REALTIME
 *              Other ......: Fallback timer is thread + sleep loop
 *
 * 18oct2013,tk Implemented new system interface
 * 12jun2013,tk MICRO-240/PR#1412
 * 20aug2012,tk CR-75
 * 04mar2012,tk Written
 */
/*ce
 * \file
 * \brief VxWorks implementation of OSAPI system routines
 */
#include "osapi/osapi_config.h"

#if OSAPI_INCLUDE_VXWORKS

#if defined(RTI_RTP)
  #ifdef _POSIX_C_SOURCE
  #undef _POSIX_C_SOURCE
  #endif
  #define _POSIX_C_SOURCE 199309L
  #include <unistd.h>
  #include <rtpLibCommon.h>
  #include <errno.h>
  #include <rtpLib.h>
#else
  #include <taskLib.h>
#endif

#include <stdlib.h>
#include <limits.h>
#include <sysLib.h>
#include <tickLib.h>
#include <objLib.h>
#include <signal.h>
#ifndef RTI_CERT
#include <hostLib.h>
#endif
#include "osapi/osapi_heap.h"
#include "osapi/osapi_mutex.h"
#include "osapi/osapi_semaphore.h"
#include "osapi/osapi_system.h"
#include "osapi/osapi_thread.h"
#include "osapi/osapi_string.h"
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif

#include "../common/System.h"

#ifndef OSAPISYSTEM_MAX_TIMERS
#define OSAPISYSTEM_MAX_TIMERS          8
#endif

/*ci
 *  Timer callback information
 */
struct OSAPI_SystemTimerHandler
{
    OSAPI_TimerTickHandlerFunction handler;
    void *param;
};

/*ci
 * System variables
 */
struct OSAPI_SystemVxWorks
{
    struct OSAPI_System _parent;
    struct OSAPI_Mutex *mutex;
    struct OSAPI_Mutex *tick_mutex;
    struct OSAPI_SystemTimerHandler timer_handler[OSAPISYSTEM_MAX_TIMERS];
#ifdef USE_TIMER_THREAD_SEMAPHORE
    OSAPI_Semaphore_T *timer_sem;
#endif
    struct OSAPI_Thread *timer_thread;
    RTI_INT32 timer_count;
    struct timespec timer_resolution;
    struct itimerspec tick_resolution;
    clockid_t rt_clock;
    struct sigaction old_sa;
    timer_t timer;
#ifdef USE_WD_TIMER
    WDOG_ID wd_timer;
#endif
    RTI_INT32 tick_sec;
    RTI_UINT32 tick_nanosec;
};

RTI_PRIVATE struct OSAPI_SystemVxWorks OSAPI_System_g =
{
        ._parent = OSAPI_System_INITIALIZER
};

RTI_PRIVATE struct OSAPI_SystemVxWorks *OSAPI_System_fv_SystemVxWorks = &OSAPI_System_g;
#ifndef RTI_CERT
RTI_UINT32 OSAPI_System_gv_Size = sizeof(struct OSAPI_SystemVxWorks);
#endif /* !RTI_CERT */
struct OSAPI_System *OSAPI_System_gv_System = &OSAPI_System_g._parent;

#endif

/*** SOURCE_BEGIN ***/

#if OSAPI_INCLUDE_VXWORKS

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
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_System_lock(void)
{
    return OSAPI_Mutex_take(OSAPI_System_fv_SystemVxWorks->mutex);
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
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_System_unlock(void)
{
    return OSAPI_Mutex_give(OSAPI_System_fv_SystemVxWorks->mutex);
}

/*ci
 * \brief Run all the system timers
 *
 * \details
 *
 * Update periodic system timers created with \ref OSAPI_System_start_timer.
 *
 * \param[in] self Pointer to VxWorks system instance
 */
RTI_PRIVATE void
OSAPI_SystemVxWorks_run_timers(struct OSAPI_SystemVxWorks *self)
{
    RTI_INT32 i, j;
    RTI_BOOL bretval;
    RTI_UINT32 last_tick_ns;

    if (!OSAPI_System_lock())
    {
        return;
    }

    for (i = 0, j = self->timer_count; (i < OSAPISYSTEM_MAX_TIMERS) && j; ++i)
    {
        if (self->timer_handler[i].handler)
        {
            j--;
            self->timer_handler[i].handler(self->timer_handler[i].param);
        }
    }

    /* The results are stored in temporary variables to make sure that
     * the assignments are atomic for each value.
     * OSAPI_SystemVxWorks_get_ticktime depends on this
     */
    if (!OSAPI_Mutex_take(self->tick_mutex))
    {
        /* Ignore the return value as we must return from this function
         * regardless of the outcome.
         */
        bretval = OSAPI_System_unlock();
        IGNORE_RETVAL(bretval);
        return;
    }

    last_tick_ns = self->tick_nanosec;
    self->tick_nanosec += (RTI_UINT32)self->tick_resolution.it_interval.tv_nsec;
    self->tick_nanosec %= 1000000000;
    
    if (self->tick_nanosec < last_tick_ns)
    {
        ++self->tick_sec;
    }

    /* Ignore the return code as we have to proceed. Otherwise the syslock
     * will be taken and that will halt the system.
     */
    bretval = OSAPI_Mutex_give(self->tick_mutex);
    IGNORE_RETVAL(bretval);

    /* The API requires us to check the return value, but since this
     * function does not return anything, ignore the return value
     */
    bretval = OSAPI_System_unlock();
    IGNORE_RETVAL(bretval);
}

/*ci
 * \brief OS specifc timer interrupt handler
 *
 * \details
 *
 * This function is called periodically by the OS to update the system timer.
 *
 * \param[in] sig The signal that caused the interrupt to be called
 *
 * \param[in] code OS specific information
 *
 * \param[in] siginfo_t Pointer to OS specific structure with signal information
 *
 * \param[in] param Optional user parameter passed in then the timer interrupt
 *                  was installed
 *
 */
#if defined(USE_TIMER_SIGNAL)
RTI_PRIVATE void
#ifdef _WRS_KERNEL
OSAPI_SystemVxWorks_timer_intr(int sig,
                                int code,void *param)
#else
OSAPI_SystemVxWorks_timer_intr(int sig,
                                siginfo_t *extra,void *param)
#endif
{
#ifdef USE_TIMER_THREAD_SEMAPHORE
    RTI_BOOL bretval;
#ifdef _WRS_KERNEL
    UNUSED_ARG(sig);
    UNUSED_ARG(code);
    UNUSED_ARG(param);
#else
    UNUSED_ARG(sig);
    UNUSED_ARG(extra);
    UNUSED_ARG(param);
#endif

    /* Ignore the return value because this function is called by a thread
     * interrupt and does not return any failure.
     */
    bretval = OSAPI_Semaphore_give(OSAPI_System_fv_SystemVxWorks->timer_sem);
    IGNORE_RETVAL(bretval);
#else
    UNUSED_ARG(sig);
    UNUSED_ARG(param);
    struct OSAPI_SystemVxWorks *self =
                        (struct OSAPI_SystemVxWorks*)extra->si_value.sival_ptr;

    OSAPI_SystemVxWorks_run_timers(self);
#endif
}
#elif defined(USE_WD_TIMER)

/*ci
 * \brief OS specifc timer interrupt handler
 *
 * \details
 *
 * This function is called periodically by the OS watch-dog timer to udpate
 * the system timer.
 *
 * \param[in] param - VxWorks system structure
 *
 */
LOCAL void
OSAPI_SystemVxWorks_timer_intr(char *param)
{
    struct OSAPI_SystemVxWorks *self = (struct OSAPI_SystemVxWorks *)param;
#ifdef USE_TIMER_THREAD_SEMAPHORE
    RTI_BOOL bretval;

    /* Ignore the return value because this function is called by a thread or
     * interrupt and does not return any failure.
     */
    bretval = OSAPI_Semaphore_give(self->timer_sem);
    IGNORE_RETVAL(bretval);
#else
    OSAPI_SystemVxWorks_run_timers(self);
#endif
}
#elif defined(USE_TIMER_THREAD_RT)
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
OSAPI_SystemVxWorks_timer_thread(struct OSAPI_ThreadInfo* thread_param)
{
    struct OSAPI_SystemVxWorks *self =
                        (struct OSAPI_SystemVxWorks *)thread_param->user_data;
    int rval;

#if VXWORKS_VERSION_6_3_OR_BETTER
    struct timespec this_iter_start_time;
    struct timespec next_iter_start_time;
#endif

    while (!thread_param->stop_thread)
    {

#if VXWORKS_VERSION_6_3_OR_BETTER
        /* determine time of next iteration */
        if (clock_gettime(self->rt_clock, &this_iter_start_time) < 0)
        {
#if OSAPI_ENABLE_LOG
            if (OSAPI_Log_is_initialized())
            {
                OSAPI_LOG_SYSTEM_GET_TIME(OSAPI_LOGKIND_ERROR,errno)
            }
#endif
            continue;
        }

        next_iter_start_time = this_iter_start_time;
        next_iter_start_time.tv_nsec += self->tick_resolution.it_interval.tv_nsec;

        /* tv_nsec is 0 - 999999999 */
        next_iter_start_time.tv_nsec %= 1000000000;

        /* If nsec wrapped around, increment sec */
        if (next_iter_start_time.tv_nsec < this_iter_start_time.tv_nsec)
        {
            ++next_iter_start_time.tv_sec;
        }
#endif

        OSAPI_SystemVxWorks_run_timers(self);

#if VXWORKS_VERSION_6_3_OR_BETTER
        do
        {
            rval = clock_nanosleep(self->rt_clock, TIMER_ABSTIME,
                                   &next_iter_start_time, NULL);
        } while ((rval == EINTR) && !thread_param->stop_thread);
#else
        taskDelay(1);
#endif
    }

    return RTI_TRUE;
}
#endif

#if defined(USE_TIMER_THREAD_SEMAPHORE)
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
OSAPI_SystemVxWorks_timer_thread_wakeup(struct OSAPI_ThreadInfo* thread_param)
{
    struct OSAPI_SystemVxWorks *self =
                        (struct OSAPI_SystemVxWorks *)thread_param->user_data;

    return OSAPI_Semaphore_give(self->timer_sem);
}

/*ci
 * \brief Wakeup periodic timer thread
 *
 * \details
 *
 * The periodic timer thread wakes up based in a semaphore being
 * giving in a timer-interrupt. This function waits for the semaphore being
 * given and runs the system timers when woken up.
 *
 * \param[in] thread_param - VxWorks system thread
 *
 * return RTI_TRUE on successful completion, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_SystemVxWorks_timer_thread(struct OSAPI_ThreadInfo* thread_param)
{
    struct OSAPI_SystemVxWorks *self =
                        (struct OSAPI_SystemVxWorks *)thread_param->user_data;
    RTI_INT32 ec;

    while (!thread_param->stop_thread)
    {
        if (!OSAPI_Semaphore_take(self->timer_sem,
                                  OSAPI_SEMAPHORE_TIMEOUT_INFINITE,&ec))
        {
            continue;
        }
        if (thread_param->stop_thread)
        {
            break;
        }
        OSAPI_SystemVxWorks_run_timers(self);
#ifdef USE_WD_TIMER
        wdStart(self->wd_timer,1,
                (FUNCPTR)OSAPI_SystemVxWorks_timer_intr,
                (OSAPI_UserArg)self);
#endif
    }

    return RTI_TRUE;
}
#endif

/*ci
 *
 * \brief Internal VX system initialization routine
 *
 * \details
 *
 * This function initializes the OSAPI VxWorks system abstraction and
 * sets up the timer handling.
 *
 * \param[in]    self - Vx system structure

 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_SystemVxWorks_initialize_impl(struct OSAPI_SystemVxWorks *self)
{
    RTI_INT32 i;
    RTI_BOOL retval = RTI_FALSE;
#if defined(USE_TIMER_SIGNAL)
    struct sigaction sa;
    struct sigevent timer_event;
#endif

    /* NOTE: VxWorks 6.8+ SMP is not locked, not yet multi task safe */
#if !defined(RTI_RTP) && !(VXWORKS_VERSION_6_8_OR_BETTER)
    taskLock();
#endif

    for (i = 0; i < OSAPISYSTEM_MAX_TIMERS; ++i)
    {
        self->timer_handler[i].handler = NULL;
        self->timer_handler[i].param = NULL;
    }

    self->timer_count = 0;
#if defined(CLOCK_MONOTONIC)
    self->rt_clock = CLOCK_MONOTONIC;
#else
    self->rt_clock = CLOCK_REALTIME;
#endif
    if (clock_getres(self->rt_clock, &self->timer_resolution) < 0)
    {
        if (errno == EINVAL)
        {
#if defined(CLOCK_MONOTONIC)
            self->rt_clock = CLOCK_REALTIME;
            if (clock_getres(self->rt_clock,&self->timer_resolution) < 0)
            {
                goto done;
            }
#else
            goto done;
#endif
        }
        else
        {
            goto done;
        }
    }

    /* Require resolution > 1 Hz */
    if (self->timer_resolution.tv_sec > 0)
    {
        goto done;
    }

    self->tick_resolution.it_interval.tv_sec = 0;

    self->tick_resolution.it_interval.tv_nsec = 1000000000 / sysClkRateGet();

    self->tick_resolution.it_value = self->tick_resolution.it_interval;

    self->mutex = OSAPI_Mutex_new();
    if (self->mutex == NULL)
    {
        goto done;
    }

    self->tick_mutex = OSAPI_Mutex_new();
    if (self->tick_mutex == NULL)
    {
        goto done;
    }

#ifdef USE_TIMER_THREAD_SEMAPHORE
    self->timer_sem = OSAPI_Semaphore_new();
    if (self->timer_sem == NULL)
    {
        goto done;
    }

    self->timer_thread = NULL;
    self->timer_thread = OSAPI_Thread_create("osapi_timer",
                                &self->_parent.property.timer_property.thread,
                                OSAPI_SystemVxWorks_timer_thread,(void*)self,
                                OSAPI_SystemVxWorks_timer_thread_wakeup);

    if (self->timer_thread == NULL)
    {
        goto done;
    }

    if (!OSAPI_Thread_start(self->timer_thread))
    {
        goto done;
    }
#endif

#if defined(USE_TIMER_SIGNAL)

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = OSAPI_SystemVxWorks_timer_intr;

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

#elif defined(USE_TIMER_THREAD_RT)
    self->timer_thread = OSAPI_Thread_create("osapi_timer",
            &self->_parent.property.timer_property.thread,
            OSAPI_SystemVxWorks_timer_thread,(void *)self,NULL);

    if (self->timer_thread == NULL)
    {
        goto done;
    }

    if (!OSAPI_Thread_start(self->timer_thread))
    {
        goto done;
    }
#elif defined(USE_WD_TIMER)

    self->wd_timer = wdCreate();
    if (self->wd_timer == NULL)
    {
        goto done;
    }

    if (wdStart(self->wd_timer,1,
                (FUNCPTR)OSAPI_SystemVxWorks_timer_intr,
                (OSAPI_UserArg)self) != OK)
    {
        goto done;
    }

#endif

    self->tick_sec = 0;
    self->tick_nanosec = 0;

    retval = RTI_TRUE;

done:

#if !defined(RTI_RTP) && !(VXWORKS_VERSION_6_8_OR_BETTER)
    taskUnlock();
#endif

    return retval;
}

#ifndef RTI_CERT
/*ci
 *
 * \brief Internal VX system finalization routine
 *
 * \details
 *
 * This function finalizes the OSAPI VxWorks system abstraction and
 * deletes up the timer handling.
 *
 * \param[in]    self - Vx system structure

 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_SystemVxWorks_finalize_impl(struct OSAPI_SystemVxWorks *self)
{
    RTI_INT32 i;

#if defined(USE_TIMER_SIGNAL)
    if (sigaction(SIGRTMIN,NULL,&self->old_sa) < 0)
    {
        return RTI_FALSE;
    }

    if (timer_delete(self->timer) < 0)
    {
        return RTI_FALSE;
    }
#elif defined(USE_TIMER_THREAD_RT)
    if (!OSAPI_Thread_destroy(self->timer_thread))
    {
        return RTI_FALSE;
    }
    self->timer_thread = NULL;
#elif defined(USE_WD_TIMER)
    wdDelete(self->wd_timer);
#endif


#ifdef USE_TIMER_THREAD_SEMAPHORE
    if (!OSAPI_Thread_destroy(self->timer_thread))
    {
        return RTI_FALSE;
    }
    self->timer_thread = NULL;

    if (self->timer_sem != NULL)
    {
        if (!OSAPI_Semaphore_delete(self->timer_sem))
        {
            return RTI_FALSE;
        }
    }
#endif

    for (i = 0; i < OSAPISYSTEM_MAX_TIMERS; ++i)
    {
        self->timer_handler[i].handler = NULL;
        self->timer_handler[i].param = NULL;
    }

    self->timer_count = 0;

    if (!OSAPI_Mutex_delete(self->mutex))
    {
        return RTI_FALSE;
    }

    if (!OSAPI_Mutex_delete(self->tick_mutex))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}
#endif /* !RTI_CERT */

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
OSAPI_SystemVxWorks_start_timer(OSAPI_Timer_T self,
                                OSAPI_TimerTickHandlerFunction tick_handler)
{
    RTI_INT32 i;
    RTI_BOOL retval;

    OSAPI_PRECONDITION((self == NULL) || (tick_handler == NULL),
                        return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("tick_h",tick_handler,RTI_TRUE);)

    if (!OSAPI_System_fv_SystemVxWorks->_parent.is_initialized)
    {
        OSAPI_LOG_SYSTEM_TIMER_START(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (!OSAPI_System_lock())
    {
        OSAPI_LOG_SYSTEM_TIMER_START(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    for (i = 0; i < OSAPISYSTEM_MAX_TIMERS; ++i)
    {
        if (OSAPI_System_fv_SystemVxWorks->timer_handler[i].handler == NULL)
        {
            break;
        }
    }

    if (i == OSAPISYSTEM_MAX_TIMERS)
    {
        OSAPI_LOG_SYSTEM_TIMER_START(OSAPI_LOGKIND_ERROR)
        retval = OSAPI_System_unlock();

        /* Since RTI_FALSE must be returned, ignore the retval from unlock() */
        IGNORE_RETVAL(retval);
        return RTI_FALSE;
    }

    OSAPI_System_fv_SystemVxWorks->timer_handler[i].handler = tick_handler;
    OSAPI_System_fv_SystemVxWorks->timer_handler[i].param = self;
    ++OSAPI_System_fv_SystemVxWorks->timer_count;

    return OSAPI_System_unlock();
}

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
OSAPI_SystemVxWorks_stop_timer(OSAPI_Timer_T self)
{
    RTI_INT32 i;

    OSAPI_PRECONDITION((self == NULL),
                        return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (!OSAPI_System_fv_SystemVxWorks->_parent.is_initialized)
    {
        OSAPI_LOG_SYSTEM_TIMER_STOP(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (!OSAPI_System_lock())
    {
        OSAPI_LOG_SYSTEM_TIMER_STOP(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    for (i = 0; i < OSAPISYSTEM_MAX_TIMERS; ++i)
    {
        if (OSAPI_System_fv_SystemVxWorks->timer_handler[i].param == self)
        {
            OSAPI_System_fv_SystemVxWorks->timer_handler[i].handler = NULL;
            OSAPI_System_fv_SystemVxWorks->timer_handler[i].param = NULL;
            --OSAPI_System_fv_SystemVxWorks->timer_count;
        }
    }

    if (!OSAPI_System_unlock())
    {
        OSAPI_LOG_SYSTEM_TIMER_STOP(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 *  \brief Get the resolution of the clock driving the timer in nanoseconds
 *
 *  \details
 *  Implementation of OSAPI_System_get_timer_resolution
 *
 *  This function returns the frequency of the system timer used to implement
 *  OSAPI_SystemI::start_timer and OSAPI_SystemI::stop_timer API.
 *
 *  @return timer resolution in nanoseconds.

 */
RTI_PRIVATE RTI_INT32
OSAPI_SystemVxWorks_get_timer_resolution(void)
{
    return (RTI_INT32)OSAPI_System_fv_SystemVxWorks->tick_resolution.it_interval.tv_nsec;
}

/*ci
 * \brief Get the current system time.
 *
 * \details
 * Implementation of OSAPI_System_get_time
 *
 * @param[out] now Time in NtpTime format.
 *
 * @return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * @pre Initialized system.
 */
RTI_PRIVATE  RTI_BOOL
OSAPI_SystemVxWorks_get_time(OSAPI_NtpTime *now)
{
    struct timespec clk_time;

    OSAPI_PRECONDITION(now == NULL,return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("now",now,RTI_TRUE);)

    if (clock_gettime(CLOCK_REALTIME, &clk_time) == -1)
    {
        return RTI_FALSE;
    }

    OSAPI_NtpTime_from_nanosec(now, (RTI_INT32)clk_time.tv_sec,
                                    (RTI_UINT32)clk_time.tv_nsec);

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
OSAPI_SystemVxWorks_initialize(void)
{
    return OSAPI_SystemVxWorks_initialize_impl(OSAPI_System_fv_SystemVxWorks);
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
OSAPI_SystemVxWorks_finalize(void)
{
    return OSAPI_SystemVxWorks_finalize_impl(OSAPI_System_fv_SystemVxWorks);
}
#endif /* !RTI_CERT */

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
OSAPI_SystemVxWorks_generate_uuid(struct OSAPI_SystemUUID *uuid_out)
{
    static RTI_UINT32 uuid_counter = 0xdeadc0de;
    OSAPI_NtpTime now;
    union
    {
        OSAPI_ThreadId th;
        RTI_UINT32 as_uint32;
    } th32;
    union
    {
        OSAPI_ThreadId th;
        struct
        {
            RTI_UINT32 high;
            RTI_UINT32 low;
        } as_uint64;
    } th64;

    OSAPI_PRECONDITION((uuid_out == NULL),
                        return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("uuid_out",uuid_out,RTI_TRUE);)

    if (!OSAPI_System_get_time(&now))
    {
        return RTI_FALSE;
    }

    if (sizeof(OSAPI_ThreadId) == sizeof(RTI_INT64))
    {
        th64.th = OSAPI_Thread_self();
        if (th64.as_uint64.low == 0)
        {
            uuid_out->value[0] = th64.as_uint64.high;
        }
        else
        {
            uuid_out->value[0] = th64.as_uint64.low;
        }
    }
    else
    {
        th32.th = OSAPI_Thread_self();
        uuid_out->value[0] = th32.as_uint32;
    }

    uuid_out->value[1] = uuid_counter++;
    uuid_out->value[2] = (RTI_UINT32)now.frac + 1;
    uuid_out->value[3] = (RTI_UINT32)now.sec + 1;

    return RTI_TRUE;
}

/*ci
 * \brief Get the hostname
 *
 * \details
 * Implementation of OSAPI_System_get_hostname
 *
 * @return RTI_TRUE on success, RTI_FALSE on failure.
 */
RTI_PRIVATE RTI_BOOL
OSAPI_SystemVxWorks_get_hostname(char *const hostname)
{
#ifndef RTI_CERT
    if (gethostname(hostname,OSAPI_SYSTEM_MAX_HOSTNAME) != OK)
    {
        return RTI_FALSE;
    }
#else
    const char* HOST_NAME = "unknown";
    OSAPI_Memory_copy(hostname, HOST_NAME,
                      OSAPI_String_length(HOST_NAME) + 1);
#endif
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
OSAPI_SystemVxWorks_get_ticktime(RTI_INT32 *sec,RTI_UINT32 *nanosec)
{
    if (!OSAPI_Mutex_take(OSAPI_System_fv_SystemVxWorks->tick_mutex))
    {
        return RTI_FALSE;
    }

    *sec = OSAPI_System_fv_SystemVxWorks->tick_sec;
    *nanosec = OSAPI_System_fv_SystemVxWorks->tick_nanosec;

    if (!OSAPI_Mutex_give(OSAPI_System_fv_SystemVxWorks->tick_mutex))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/******************************************************************************
 *  Public API
 ******************************************************************************/
void
OSAPI_System_get_native_interface(struct OSAPI_SystemI *intf)
{

    intf->start_timer = OSAPI_SystemVxWorks_start_timer;
    intf->stop_timer = OSAPI_SystemVxWorks_stop_timer;
    intf->get_timer_resolution = OSAPI_SystemVxWorks_get_timer_resolution;
    intf->get_time = OSAPI_SystemVxWorks_get_time;
    intf->initialize = OSAPI_SystemVxWorks_initialize;
    intf->generate_uuid = OSAPI_SystemVxWorks_generate_uuid;
    intf->get_hostname = OSAPI_SystemVxWorks_get_hostname;
    intf->get_ticktime = OSAPI_SystemVxWorks_get_ticktime;


#ifndef RTI_CERT
    intf->finalize = OSAPI_SystemVxWorks_finalize;
#else
    intf->finalize = NULL;
#endif /* !RTI_CERT */

}

#endif









