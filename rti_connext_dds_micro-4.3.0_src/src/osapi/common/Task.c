/*
 * FILE: Task.c - Task support
 *
 * Copyright 2018-2024 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#include "osapi/osapi_log.h"
#include "osapi/osapi_config.h"
#include "osapi/osapi_task.h"
#include "osapi/osapi_thread.h"

#define OSAPI_NANOSEC_IN_SEC (1000000000U)

RTI_BOOL
OSAPI_Task_initialize(struct OSAPI_Task *task,OSAPI_Task_run_T run)
{
    struct OSAPI_Task i_task = OSAPI_Task_INITIALIZER;

    *task = i_task;
    task->run = run;
    task->state = OSAPI_TASK_STATE_INITIALIZED;

    return RTI_TRUE;
}

void
OSAPI_Task_deschedule(struct OSAPI_TaskScheduler *sched,
                      struct OSAPI_Task *task)
{
    if (!OSAPI_Mutex_take(sched->task_mutex))
    {
        return;
    }

    if (task->state == OSAPI_TASK_STATE_RUNNING)
    {
        task->state = OSAPI_TASK_STATE_COMPLETED;
    }
    else if ((task->_next != NULL) && (task->_prev != NULL))
    {
        task->_prev->_next = task->_next;
        task->_next->_prev = task->_prev;
        task->_next = NULL;
        task->_prev = NULL;
        task->state = OSAPI_TASK_STATE_INITIALIZED;
        task->lapsed_sec = 0;
        task->lapsed_nanosec = 0;
    }

    if (!OSAPI_Mutex_give(sched->task_mutex))
    {

    }
}

RTI_PRIVATE void
OSAPI_Task_add_time(struct OSAPI_Task *task,
                    RTI_INT32 sec,
                    RTI_UINT32 nanosec)
{
    task->lapsed_sec += sec;
    task->lapsed_nanosec += nanosec;
    if (task->lapsed_nanosec > OSAPI_NANOSEC_IN_SEC)
    {
        ++task->lapsed_sec;
        task->lapsed_nanosec -= OSAPI_NANOSEC_IN_SEC;
    }
}

RTI_PRIVATE RTI_BOOL
OSAPI_Task_is_runnable(struct OSAPI_Task *task)
{
    if ((task->lapsed_sec > task->period_sec) ||
        ((task->lapsed_sec == task->period_sec) &&
         (task->lapsed_nanosec >= task->period_nanosec)))
    {
        task->lapsed_sec -= task->period_sec;
        if (task->lapsed_nanosec >= task->period_nanosec)
        {
            task->lapsed_nanosec -= task->period_nanosec;
        }
        else
        {
            --task->lapsed_sec;
            task->lapsed_nanosec += (OSAPI_NANOSEC_IN_SEC - task->period_nanosec);
        }

        return RTI_TRUE;
    }

    return RTI_FALSE;
}

RTI_BOOL
OSAPI_Task_schedule_periodic(struct OSAPI_TaskScheduler *sched,
                             struct OSAPI_Task *task,
                             RTI_INT32 sec,
                             RTI_UINT32 nanosec,
                             RTI_UINT32 priority,
                             RTI_BOOL wake_up)
{
    struct OSAPI_Task *last_task;

    UNUSED_ARG(priority);

    if (!OSAPI_Mutex_take(sched->task_mutex))
    {
        return RTI_FALSE;
    }

    if (task->state == OSAPI_TASK_STATE_INVALID)
    {
        if (!OSAPI_Mutex_give(sched->task_mutex))
        {
        }
        return RTI_FALSE;
    }

    /* already ready or running */
    if (task->state > OSAPI_TASK_STATE_COMPLETED)
    {
        if (wake_up)
        {
            task->wake_up = RTI_TRUE;
            if (!OSAPI_Semaphore_give(sched->task_sem))
            {
            }
        }
        if (!OSAPI_Mutex_give(sched->task_mutex))
        {
        }
        return RTI_TRUE;
    }

    if ((task->_next != NULL) && (task->_prev != NULL))
    {
        task->_prev->_next = task->_next;
        task->_next->_prev = task->_prev;
        task->_next = NULL;
        task->_prev = NULL;
    }

    task->state = OSAPI_TASK_STATE_READY;
    task->period_sec = sec;
    task->period_nanosec = nanosec;
    task->lapsed_sec = sec;
    task->lapsed_nanosec = nanosec;

    /* append task */
    last_task = sched->empty_task._prev;
    task->_next = last_task->_next;
    last_task->_next = task;
    task->_next->_prev = task;
    task->_prev = last_task;

    if (!OSAPI_Semaphore_give(sched->task_sem))
    {

    }

    if (!OSAPI_Mutex_give(sched->task_mutex))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_PRIVATE void
OSAPI_TaskScheduler_run(OSAPI_TaskScheduler_T *sched,RTI_UINT32 lapsed_ns)
{
    struct OSAPI_Task *a_task = NULL;
    RTI_INT32 lapsed_sec;
    RTI_BOOL done = RTI_FALSE;
    RTI_BOOL wake_up = RTI_FALSE;

    lapsed_sec = (RTI_INT32)(lapsed_ns / OSAPI_NANOSEC_IN_SEC);
    lapsed_ns -= (RTI_UINT32)lapsed_sec * OSAPI_NANOSEC_IN_SEC;

    while (!done)
    {

        if (!OSAPI_Mutex_take(sched->task_mutex))
        {
            return;
        }

        if (a_task == NULL)
        {
            a_task = sched->empty_task._next;
        }
        else
        {
            a_task = a_task->_next;
        }

        if (a_task == &sched->empty_task)
        {
            if (!OSAPI_Mutex_give(sched->task_mutex))
            {
            }
            break;
        }

        a_task->state = OSAPI_TASK_STATE_RUNNING;
        wake_up = a_task->wake_up;
        a_task->wake_up = RTI_FALSE;

        if (!OSAPI_Mutex_give(sched->task_mutex))
        {
            return;
        }

        if ((a_task->period_sec == 0) && (a_task->period_nanosec == 0))
        {
            OSAPI_TaskState_T task_state;

            task_state = a_task->run(a_task, RTI_TRUE);
            IGNORE_RETVAL(task_state);

            OSAPI_Task_deschedule(sched,a_task);
            a_task = NULL;
        }
        else
        {
            RTI_BOOL is_runnable;
            OSAPI_Task_add_time(a_task,lapsed_sec,lapsed_ns);

            is_runnable = OSAPI_Task_is_runnable(a_task);
            while (is_runnable || wake_up)
            {
                OSAPI_TaskState_T task_state;

                task_state = a_task->run(a_task, is_runnable);

                if (!OSAPI_Mutex_take(sched->task_mutex))
                {
                    break;
                }

                /* Task may have been completed as part of running. Only
                 * deschedule if it is still running and has not been
                 * rescheduled (and has state of READY) or has been
                 * descheduled while running.
                 */
                if (((task_state == OSAPI_TASK_STATE_COMPLETED) &&
                     (a_task->state == OSAPI_TASK_STATE_RUNNING)) ||
                     (a_task->state == OSAPI_TASK_STATE_COMPLETED))
                {
                    a_task->state = OSAPI_TASK_STATE_COMPLETED;

                    /* Coverity incorrectly flags a_task as modifiable by multiple
                     * racing threads. However, only the task scheduler manages
                     * scheduled tasks and their state.
                     */

                    /* coverity[use : FALSE] */
                    OSAPI_Task_deschedule(sched,a_task);
                    a_task = NULL;
                    if (!OSAPI_Mutex_give(sched->task_mutex))
                    {
                    }
                    break;
                }

                if (!OSAPI_Mutex_give(sched->task_mutex))
                {
                    break;
                }
                wake_up = RTI_FALSE;
                is_runnable = OSAPI_Task_is_runnable(a_task);
            }
        }
    }
}

MUST_CHECK_RETURN RTI_PRIVATE  RTI_BOOL
OSAPI_TaskScheduler_wakeup_thread(struct OSAPI_ThreadInfo *thrd_info)
{
    struct OSAPI_TaskScheduler *sched =
                        (struct OSAPI_TaskScheduler *)thrd_info->user_data;

    if (!OSAPI_Semaphore_give(sched->task_sem))
    {
        return RTI_FALSE;
    }
    return RTI_TRUE;
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_TaskScheduler_thread(struct OSAPI_ThreadInfo *thrd_info)
{
    struct OSAPI_TaskScheduler *sched =
                        (struct OSAPI_TaskScheduler *)thrd_info->user_data;
    RTI_INT32 reason = 0;
    struct OSAPI_SystemTime start,stop,diff;
    RTI_UINT32 tick_periods = 0;
    RTI_UINT32 acc_ns = 0;
    RTI_BOOL empty;

    if (!OSAPI_System_get_time(&start))
    {
    }

    while (!thrd_info->stop_thread)
    {
        if (!OSAPI_Mutex_take(sched->task_mutex))
        {
            OSAPI_LOG_MUTEX_TAKE(OSAPI_LOGKIND_ERROR,RTI_TRUE)
            return RTI_FALSE;
        }

        empty = (sched->empty_task._prev == &sched->empty_task) &&
                (sched->empty_task._next == &sched->empty_task);

        if (!OSAPI_Mutex_give(sched->task_mutex))
        {
            OSAPI_LOG_MUTEX_GIVE(OSAPI_LOGKIND_ERROR,RTI_TRUE)
            return RTI_FALSE;
        }

        if (empty)
        {
            if (!OSAPI_Semaphore_take(sched->task_sem,
                                      OSAPI_SEMAPHORE_TIMEOUT_INFINITE,
                                      &reason))
            {
            }

            if (thrd_info->stop_thread)
            {
                break;
            }

            if (!OSAPI_System_get_time(&start))
            {
            }
        }
        else
        {
            OSAPI_Thread_nanosleep(sched->clock_rate);
        }

        if (!OSAPI_System_get_time(&stop))
        {
        }

        OSAPI_SystemTime_subtract(&diff,&stop,&start);
        
        acc_ns += diff.nanosec;

        tick_periods = acc_ns / sched->clock_rate;

        acc_ns -= (tick_periods * sched->clock_rate);

        OSAPI_TaskScheduler_run(sched,tick_periods * sched->clock_rate);

        start = stop;
    } /* while (!empty) */

    return RTI_TRUE;
}

RTI_BOOL
OSAPI_TaskScheduler_initialize(OSAPI_TaskScheduler_T *sched,
                               OSAPI_Mutex_T *mutex)
{
    struct OSAPI_SystemProperty sp = OSAPI_SystemProperty_INITIALIZER;

    OSAPI_PRECONDITION((sched == NULL) || (mutex == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("sched",sched,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("mutex",mutex,RTI_TRUE);)

    sched->empty_task._next = &sched->empty_task;
    sched->empty_task._prev = &sched->empty_task;
    sched->task_mutex = mutex;

    sched->task_sem = OSAPI_Semaphore_new();
    if (sched->task_sem == NULL)
    {
        return RTI_FALSE;
    }

    if (!OSAPI_System_get_property(&sp))
    {
        return RTI_FALSE;
    }

    sched->clock_rate = sp.task_scheduler.rate;
    sched->task_thread = OSAPI_Thread_create("osapi_task",
                                             &sp.task_scheduler.thread,
                                             OSAPI_TaskScheduler_thread,
                                             (void*)sched,
                                             OSAPI_TaskScheduler_wakeup_thread);

    if (sched->task_thread == NULL)
    {
#ifndef RTI_CERT
        if (!OSAPI_Semaphore_delete(sched->task_sem))
        {

        }
#endif
        return RTI_FALSE;
    }

    if (!OSAPI_Thread_start(sched->task_thread))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
OSAPI_TaskScheduler_finalize(OSAPI_TaskScheduler_T *sched)
{
    OSAPI_PRECONDITION(sched == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("sched",sched,RTI_TRUE);)

    if ((sched->task_thread != NULL) &&
        !OSAPI_Thread_destroy(sched->task_thread))
    {
        return RTI_FALSE;
    }

    if (sched->task_sem != NULL)
    {
#ifndef RTI_CERT
        if (!OSAPI_Semaphore_delete(sched->task_sem))
        {
            return RTI_FALSE;
        }
#endif
    }

    sched->empty_task._next = &sched->empty_task;
    sched->empty_task._prev = &sched->empty_task;

    return RTI_TRUE;
}
