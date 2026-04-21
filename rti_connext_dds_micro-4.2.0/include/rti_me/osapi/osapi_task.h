/*
 * FILE: osapi_task.h - Definition of Task API
 *
 * Copyright 2018-2024 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 */
/*e \file
  * \brief Task interface definition
  */
#ifndef osapi_task_h
#define osapi_task_h

#include "osapi/osapi_config.h"
#ifndef osapi_dll_h
#include "osapi/osapi_dll.h"
#endif

#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#include "osapi/osapi_system.h"
#include "osapi/osapi_task.h"
#include "osapi/osapi_time.h"
#include "osapi/osapi_thread.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    OSAPI_TASK_STATE_INVALID,

    OSAPI_TASK_STATE_INITIALIZED,

    OSAPI_TASK_STATE_COMPLETED,

    OSAPI_TASK_STATE_READY = 5,

    OSAPI_TASK_STATE_RUNNING

} OSAPI_TaskState_T;

struct OSAPI_Task;

typedef OSAPI_TaskState_T
(*OSAPI_Task_run_T)(struct OSAPI_Task *task, RTI_BOOL period_expired);

struct OSAPI_Task
{
    struct OSAPI_Task *_next;

    struct OSAPI_Task *_prev;

    OSAPI_Task_run_T run;

    OSAPI_TaskState_T state;

    RTI_INT32 lapsed_sec;

    RTI_UINT32 lapsed_nanosec;

    RTI_INT32 period_sec;

    RTI_UINT32 period_nanosec;

    RTI_BOOL wake_up;
};

#define OSAPI_Task_INITIALIZER \
{\
    NULL,\
    NULL,\
    NULL,\
    OSAPI_TASK_STATE_INVALID,\
    0,0,\
    0,0,\
    RTI_FALSE\
}

typedef struct OSAPI_Task OSAPI_Task_T;
typedef  struct OSAPI_Task* OSAPI_Task;

typedef struct OSAPI_TaskScheduler
{
    struct OSAPI_Task empty_task;

    OSAPI_Semaphore_T *task_sem;

    OSAPI_Mutex_T *task_mutex;

    struct OSAPI_Thread *task_thread;

    RTI_UINT32 clock_rate;

} OSAPI_TaskScheduler_T;

#define OSAPI_TaskScheduler_INITIALIZER \
{\
    OSAPI_Task_INITIALIZER,\
    NULL,\
    NULL,\
    NULL,\
    0\
}

OSAPIDllExport RTI_BOOL
OSAPI_Task_initialize(struct OSAPI_Task *task,OSAPI_Task_run_T run);

OSAPIDllExport RTI_BOOL
OSAPI_Task_finalize(struct OSAPI_Task *task);

OSAPIDllExport void
OSAPI_Task_deschedule(OSAPI_TaskScheduler_T *sched,
                      struct OSAPI_Task *task);

OSAPIDllExport RTI_BOOL
OSAPI_Task_schedule_periodic(OSAPI_TaskScheduler_T *sched,
                             struct OSAPI_Task *task,
                             RTI_INT32 sec,
                             RTI_UINT32 nanosec,
                             RTI_UINT32 priority,
                             RTI_BOOL wake_up);
OSAPIDllExport RTI_BOOL
OSAPI_TaskScheduler_initialize(OSAPI_TaskScheduler_T *sched,
                               OSAPI_Mutex_T *mutex);

OSAPIDllExport RTI_BOOL
OSAPI_TaskScheduler_finalize(OSAPI_TaskScheduler_T *sched);

#ifdef __cplusplus
}                               /* extern "C" */
#endif


#endif /* osapi_task_h */
