/*
 * FILE: threadxSystem.c - ThreadX system functionality
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
 * 13dec2016,francisco  File created
 *
 */
/*ce
 * \file
 * \brief ThreadX implementation of OSAPI system routines
 */
#include "osapi/osapi_config.h"


#if OSAPI_INCLUDE_THREADX

#include "tx_api.h"

#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef osapi_mutex_h
#include "osapi/osapi_mutex.h"
#endif
#ifndef osapi_string_h
#include "osapi/osapi_string.h"
#endif
#ifndef osapi_thread_h
#include "osapi/osapi_thread.h"
#endif
#ifndef osapi_system_h
#include "osapi/osapi_system.h"
#endif
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif

#include "../common/System.h"
#include "../common/Thread.h"

RTI_PRIVATE RTI_BOOL
OSAPI_ThreadXSystem_initialize(void);

RTI_PRIVATE RTI_BOOL
OSAPI_ThreadXSystem_finalize(void);

RTI_PRIVATE RTI_BOOL
OSAPI_ThreadXSystem_generate_uuid(struct OSAPI_SystemUUID *uuid_out);

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_ThreadXSystem_get_ticktime(RTI_INT32 *sec,RTI_UINT32 *nanosec);

RTI_PRIVATE RTI_BOOL
OSAPI_ThreadXSystem_start_timer(OSAPI_Timer_T                  self,
                                OSAPI_TimerTickHandlerFunction tick_handler);

RTI_PRIVATE RTI_BOOL
OSAPI_ThreadXSystem_get_time(OSAPI_NtpTime *now);
RTI_PRIVATE RTI_INT32
OSAPI_ThreadXSystem_get_timer_resolution(void);
RTI_PRIVATE RTI_BOOL
OSAPI_ThreadXSystem_stop_timer(OSAPI_Timer_T self);

#define OSAPISYSTEM_MAX_TIMERS          8
#define OSAPISYSTEM_MS_TIMER_RESOLUTION 10
#define OSAPISYSTEM_TIMER_TICKS (OSAPISYSTEM_MS_TIMER_RESOLUTION / \
                                 OSAPI_MS_TIMER_TICK)

/* According to ThreadX documentation:
 *
 * "Application timers are executed from a hidden system thread. It is,
 * therefore, important not to select suspension on any ThreadX service
 * calls made from within the application timer's expiration function."
 *
 * But note that the micro timer callback needs to take a mutex. This is why
 * system is implemented using a thread instead of a timer.
 */
#define USE_TIMER 0

struct OSAPI_SystemTimerHandler
{
    OSAPI_TimerTickHandlerFunction  handler;
    void                           *param;
};

struct OSAPI_SystemThreadX
{
    struct OSAPI_System              _parent;
    struct OSAPI_SystemTimerHandler  timer_handler[OSAPISYSTEM_MAX_TIMERS];
    RTI_INT32                        timer_count;
    struct OSAPI_Mutex              *mutex;
    RTI_BOOL                         is_deleted;
    RTI_BOOL                         timer_is_busy;
    RTI_INT32                        tick_sec;
    RTI_UINT32                       tick_nanosec;
    struct OSAPI_Mutex              *tick_mutex;
#if USE_TIMER
    TX_TIMER                         system_timer;
#else
    struct OSAPI_Thread             *timer_thread;
#endif
};

LINK_SECTION_DATA_SDRAM
RTI_PRIVATE struct OSAPI_SystemThreadX OSAPI_System_g =
{
        ._parent = OSAPI_System_INITIALIZER
};

LINK_SECTION_DATA_SDRAM
RTI_PRIVATE
struct OSAPI_SystemThreadX *OSAPI_System_fv_SystemThreadX = &OSAPI_System_g;

LINK_SECTION_DATA_SDRAM
struct OSAPI_System *OSAPI_System_gv_System = &OSAPI_System_g._parent;

LINK_SECTION_DATA_SDRAM
RTI_UINT32 OSAPI_System_gv_Size = sizeof(struct OSAPI_SystemThreadX);

/* Heap stack memory 
 * This is enough to have OSAPI_PLATFORM_THREADX_MAX_THREADS
 * thread stacks of size OSAPI_PLATFORM_THREADX_STACK_SIZE_DEFAULT 
 * each. 
 * As explained in ThreadX documentation, each free memory block in the pool 
 * requires the equivalent of two C pointers of overhead. In addition, the pool 
 * is created with two blocks, a large free block and a small permanently 
 * allocated block at the end of the memory area. This allocated block is used 
 * to improve performance of the allocation algorithm.
 */
#define THREAD_STACK_POOL_OVERHEAD 10
RTI_PRIVATE RTI_UINT32
thread_stack_g_buffer[(OSAPI_PLATFORM_THREADX_MAX_THREADS *
                      OSAPI_PLATFORM_THREADX_STACK_SIZE_DEFAULT /
                      sizeof(RTI_UINT32)) + 
                      THREAD_STACK_POOL_OVERHEAD];

LINK_SECTION_BSS_SDRAM
TX_BYTE_POOL thread_pool;

#endif

/*** SOURCE_BEGIN ***/

#if OSAPI_INCLUDE_THREADX

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_System_lock(void)
{
    return OSAPI_Mutex_take(OSAPI_System_fv_SystemThreadX->mutex);
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_System_unlock(void)
{
    return OSAPI_Mutex_give(OSAPI_System_fv_SystemThreadX->mutex);
}


RTI_PRIVATE VOID
OSAPI_SystemThreadX_timer_callback_impl(struct OSAPI_SystemThreadX *self)
{
    RTI_INT32  i, j;
    RTI_UINT32 last_tick_ns;
    RTI_BOOL   bretval;

    if (self->is_deleted)
    {
        self->timer_is_busy = RTI_FALSE;
        return;
    }

    if (!OSAPI_System_lock())
    {
        return;
    }

    for (i = 0, j = self->timer_count; (i < OSAPISYSTEM_MAX_TIMERS) && j;
            ++i)
    {
        if (self->timer_handler[i].handler)
        {
            j--;
            self->timer_handler[i].handler(self->timer_handler[i].param);
        }
    }

    if (!OSAPI_Mutex_take(self->tick_mutex))
    {
        bretval = OSAPI_System_unlock();
        IGNORE_RETVAL(bretval);
        return;
    }

    last_tick_ns = self->tick_nanosec;
    self->tick_nanosec += OSAPISYSTEM_MS_TIMER_RESOLUTION * 1000000;
    self->tick_nanosec %= 1000000000;

    if (self->tick_nanosec < last_tick_ns)
    {
        ++self->tick_sec;
    }

    bretval = OSAPI_Mutex_give(self->tick_mutex);
    IGNORE_RETVAL(bretval);

    bretval = OSAPI_System_unlock();
    IGNORE_RETVAL(bretval);
}

#if USE_TIMER

RTI_PRIVATE VOID
OSAPI_SystemThreadX_timer_callback(ULONG param)
{
    struct OSAPI_SystemThreadX *self = (struct OSAPI_SystemThreadX *)param;

    OSAPI_SystemThreadX_timer_callback_impl(self);
}

#else

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_SystemThreadX_timer_thread(struct OSAPI_ThreadInfo *thread_param)
{
    struct OSAPI_SystemThreadX *self;
    UINT rc;

    self = (struct OSAPI_SystemThreadX *)thread_param->user_data;

    while (!thread_param->stop_thread)
    {
        OSAPI_SystemThreadX_timer_callback_impl(self);

        rc = tx_thread_sleep(OSAPISYSTEM_TIMER_TICKS);
        IGNORE_RETVAL(rc);
    }

    return RTI_TRUE;
}

#endif /* #if USE_TIMER */

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_SystemThreadX_initialize(struct OSAPI_SystemThreadX *self)
{
    RTI_INT32 i;
    UINT      rc;

    if (self->_parent.is_initialized)
    {
        return RTI_TRUE;
    }
    
    /* create pool for threads stack */
    rc = tx_byte_pool_create(&thread_pool,
                             "micro-thread-stack-pool", 
                             thread_stack_g_buffer,
                             sizeof(thread_stack_g_buffer));
 
    if (rc != TX_SUCCESS)
    {
        return RTI_FALSE;
    }

    for (i = 0; i < OSAPISYSTEM_MAX_TIMERS; ++i)
    {
        self->timer_handler[i].handler = NULL;
        self->timer_handler[i].param = NULL;
    }

    self->timer_count = 0;

    self->mutex = OSAPI_Mutex_new();
    if (self->mutex == NULL)
    {
        goto fail;
    }

    self->tick_mutex = OSAPI_Mutex_new();
    if (self->tick_mutex == NULL)
    {
        goto fail;
    }

    self->timer_is_busy = RTI_TRUE;
    self->is_deleted = RTI_FALSE;
    self->tick_sec = 0;
    self->tick_nanosec = 0;

#if USE_TIMER
    rc = tx_timer_create(&self->system_timer, "systimer",
                         OSAPI_SystemThreadX_timer_callback,
                         (ULONG) self,
                         OSAPISYSTEM_TIMER_TICKS,
                         OSAPISYSTEM_TIMER_TICKS,
                         TX_AUTO_ACTIVATE);
    if (rc != TX_SUCCESS)
#else
    self->timer_thread = OSAPI_Thread_create("osapi_timer",
                          &self->_parent.property.timer_property.thread,
                          OSAPI_SystemThreadX_timer_thread,(void *)self, 
                          NULL);
    if (self->timer_thread == NULL)
#endif
    {
        goto fail;
    }

#if !USE_TIMER
    if (!OSAPI_Thread_start(self->timer_thread))
    {
        goto fail;
    }
#endif

    self->_parent.is_initialized = RTI_TRUE;

    return RTI_TRUE;

fail:

#ifndef RTI_CERT
    if (self->timer_thread != NULL)
    {
        self->timer_thread = NULL;
        OSAPI_Thread_delete(self->timer_thread);
    }

    if (self->tick_mutex != NULL)
    {
        self->tick_mutex = NULL;
        rc = OSAPI_Mutex_delete(self->tick_mutex);
        IGNORE_RETVAL(rc);
    }

    if (self->mutex != NULL)
    {
        self->mutex = NULL;
        rc = OSAPI_Mutex_delete(self->mutex);
        IGNORE_RETVAL(rc);
    }
#endif

    rc = tx_byte_pool_delete(&thread_pool);
    IGNORE_RETVAL(rc);

    return RTI_FALSE;
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_SystemThreadX_finalize(struct OSAPI_SystemThreadX *self)
{
    RTI_INT32 i;
    RTI_BOOL  bretval;
    UINT      rc;

    if (!self->_parent.is_initialized)
    {
        return RTI_FALSE;
    }

    /* This only works if the system was successfully initialized */
    self->is_deleted = RTI_TRUE;
    while (self->timer_is_busy)
    {
        rc = tx_thread_sleep(OSAPISYSTEM_TIMER_TICKS);
        IGNORE_RETVAL(rc);
    }

    if (!OSAPI_System_lock())
    {
        return RTI_FALSE;
    }

#ifndef RTI_CERT
#if USE_TIMER
    if (tx_timer_delete(&self->system_timer) != TX_SUCCESS)
#else
    if (!OSAPI_Thread_destroy(self->timer_thread))
#endif
    {
        bretval = OSAPI_System_unlock();
        IGNORE_RETVAL(bretval);
        return RTI_FALSE;
    }
#endif

    for (i = 0; i < OSAPISYSTEM_MAX_TIMERS; ++i)
    {
        self->timer_handler[i].handler = NULL;
        self->timer_handler[i].param = NULL;
    }

    self->timer_count = 0;

#ifndef RTI_CERT
    if ((self->tick_mutex != NULL) && !OSAPI_Mutex_delete(self->tick_mutex))
    {
        return RTI_FALSE;
    }
#endif

    if (!OSAPI_System_unlock())
    {
        return RTI_FALSE;
    }

#ifndef RTI_CERT
    if ((self->mutex != NULL) && !OSAPI_Mutex_delete(self->mutex))
    {
        return RTI_FALSE;
    }
#endif

    if (tx_byte_pool_delete(&thread_pool) != TX_SUCCESS)
    {
        return RTI_FALSE;
    }

    self->_parent.is_initialized = RTI_FALSE;

    return RTI_TRUE;
}

/******************************************************************************
 *  Public API
 ******************************************************************************/
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_ThreadXSystem_get_hostname(char *const hostname)
{
    RTI_INT32 len;

    len = sizeof(OSAPI_PLATFORM_THREADX_HOSTNAME);
    if (len >= OSAPI_SYSTEM_MAX_HOSTNAME)
    {
        len = OSAPI_SYSTEM_MAX_HOSTNAME - 1;
    }
    OSAPI_Memory_copy(hostname,OSAPI_PLATFORM_THREADX_HOSTNAME, len);
    hostname[len] = 0;

    return RTI_TRUE;
}

RTI_PRIVATE RTI_BOOL
OSAPI_ThreadXSystem_get_time(OSAPI_NtpTime *now)
{
    ULONG time;

    OSAPI_PRECONDITION(now == NULL,return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("now",now,RTI_TRUE);)

    time = (RTI_UINT64)tx_time_get();

    time *= OSAPI_MS_TIMER_TICK;

    OSAPI_NtpTime_from_millisec(now, time / 1000, time % 1000);

    return RTI_TRUE;
}

RTI_PRIVATE RTI_BOOL
OSAPI_ThreadXSystem_start_timer(OSAPI_Timer_T self,
                                OSAPI_TimerTickHandlerFunction tick_handler)
{
    RTI_INT32 i;
    RTI_BOOL  bretval;

    OSAPI_PRECONDITION((self == NULL) || (tick_handler == NULL),
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("tick_handler",tick_handler,RTI_TRUE);)

    if (!OSAPI_System_fv_SystemThreadX->_parent.is_initialized)
    {
        if (!OSAPI_SystemThreadX_initialize(OSAPI_System_fv_SystemThreadX))
        {
            OSAPI_LOG_SYSTEM_TIMER_START(OSAPI_LOGKIND_ERROR)
            return RTI_FALSE;
        }
    }

    if (!OSAPI_System_lock())
    {
        OSAPI_LOG_SYSTEM_TIMER_START(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (OSAPI_System_fv_SystemThreadX->timer_count == OSAPISYSTEM_MAX_TIMERS)
    {
        OSAPI_LOG_SYSTEM_TIMER_START(OSAPI_LOGKIND_ERROR)
        bretval = OSAPI_System_unlock();
        IGNORE_RETVAL(bretval);
        return RTI_FALSE;
    }

    for (i = 0; i < OSAPISYSTEM_MAX_TIMERS; ++i)
    {
        if (OSAPI_System_fv_SystemThreadX->timer_handler[i].handler == NULL)
        {
            break;
        }
    }


    if (i == OSAPISYSTEM_MAX_TIMERS)
    {
        OSAPI_LOG_SYSTEM_TIMER_START(OSAPI_LOGKIND_ERROR)
        bretval = OSAPI_System_unlock();
        IGNORE_RETVAL(bretval);
        return RTI_FALSE;
    }

    OSAPI_System_fv_SystemThreadX->timer_handler[i].handler = tick_handler;
    OSAPI_System_fv_SystemThreadX->timer_handler[i].param = self;
    ++OSAPI_System_fv_SystemThreadX->timer_count;

    return OSAPI_System_unlock();
}

RTI_PRIVATE RTI_BOOL
OSAPI_ThreadXSystem_stop_timer(OSAPI_Timer_T self)
{
    RTI_INT32 i;

    OSAPI_PRECONDITION((self == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (!OSAPI_System_fv_SystemThreadX->_parent.is_initialized)
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
        if (OSAPI_System_fv_SystemThreadX->timer_handler[i].param == self)
        {
            OSAPI_System_fv_SystemThreadX->timer_handler[i].handler = NULL;
            OSAPI_System_fv_SystemThreadX->timer_handler[i].param = NULL;
            --OSAPI_System_fv_SystemThreadX->timer_count;
        }
    }

    if (!OSAPI_System_unlock())
    {
        OSAPI_LOG_SYSTEM_TIMER_STOP(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_PRIVATE RTI_INT32
OSAPI_ThreadXSystem_get_timer_resolution(void)
{
    if (!OSAPI_System_fv_SystemThreadX->_parent.is_initialized)
    {
        if (!OSAPI_SystemThreadX_initialize(OSAPI_System_fv_SystemThreadX))
        {
            OSAPI_LOG_SYSTEM_TIMER_START(OSAPI_LOGKIND_ERROR)
            return 0;
        }
    }

    return OSAPISYSTEM_MS_TIMER_RESOLUTION * 1000000;
}

RTI_PRIVATE RTI_BOOL
OSAPI_ThreadXSystem_initialize(void)
{
    return OSAPI_SystemThreadX_initialize(OSAPI_System_fv_SystemThreadX);
}

RTI_PRIVATE RTI_BOOL
OSAPI_ThreadXSystem_finalize(void)
{
    return OSAPI_SystemThreadX_finalize(OSAPI_System_fv_SystemThreadX);
}

RTI_PRIVATE RTI_BOOL
OSAPI_ThreadXSystem_generate_uuid(struct OSAPI_SystemUUID *uuid_out)
{
    OSAPI_NtpTime     now;
    ULONG ip_address;
    ULONG network_mask;

    LINK_SECTION_DATA_SDRAM
    static RTI_UINT32 uuid_counter = 0xdeadc0de;

    OSAPI_PRECONDITION((uuid_out == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("uuid_out",uuid_out,RTI_TRUE);)

    if (!OSAPI_System_get_time(&now))
    {
        return RTI_FALSE;
    }

#if RTI_ENDIAN_LITTLE
    uuid_out->value[0] = ((uuid_counter&0xff000000U)>>24) |
                         ((uuid_counter&0x00ff0000U)>>8)  |
                         ((uuid_counter&0x000000ffU)<<24) |
                         ((uuid_counter&0x0000ff00U)<<8);
    uuid_counter++;
#else
    uuid_out->value[0] = uuid_counter++;
#endif
    if (nx_ip_address_get(&bsp_ip_bus, &ip_address, &network_mask) == NX_SUCCESS)
    {
        uuid_out->value[1] = ip_address;
    }
    else
    {
        uuid_out->value[1] = (RTI_UINT32)OSAPI_Thread_self();
    }
    uuid_out->value[2] = (RTI_UINT32)now.frac;
    uuid_out->value[3] = (RTI_UINT32)now.sec;

    return RTI_TRUE;
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
OSAPI_ThreadXSystem_get_ticktime(RTI_INT32 *sec,RTI_UINT32 *nanosec)
{

    if (!OSAPI_Mutex_take(OSAPI_System_fv_SystemThreadX->tick_mutex))
    {
        return RTI_FALSE;
    }

    *sec = OSAPI_System_fv_SystemThreadX->tick_sec;
    *nanosec = OSAPI_System_fv_SystemThreadX->tick_nanosec;

    if (!OSAPI_Mutex_give(OSAPI_System_fv_SystemThreadX->tick_mutex))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

void
OSAPI_System_get_native_interface(struct OSAPI_SystemI *intf)
{
    intf->start_timer = OSAPI_ThreadXSystem_start_timer;
    intf->stop_timer = OSAPI_ThreadXSystem_stop_timer;
    intf->get_timer_resolution = OSAPI_ThreadXSystem_get_timer_resolution;
    intf->get_time = OSAPI_ThreadXSystem_get_time;
    intf->initialize = OSAPI_ThreadXSystem_initialize;
    intf->finalize = OSAPI_ThreadXSystem_finalize;
    intf->generate_uuid = OSAPI_ThreadXSystem_generate_uuid;
    intf->get_hostname = OSAPI_ThreadXSystem_get_hostname;
    intf->get_ticktime = OSAPI_ThreadXSystem_get_ticktime;
}
#endif
