/*
 * FILE: Timer.c - Implementation of Timer API
 *
 * Copyright 2008-2021 Real-Time Innovations, Inc.
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
 * 07jul2021,tk MICRO-2845/PR.28682
 * - Correctly check the return value from OSAPI_Mutex_give() in
 *   OSAPI_Timer_create_timeout()
 * 15apr2021,tk MICRO-2979/PR.27549
 * - Removed unused functions from CERT:
 *   OSAPI_TimeoutHandle_get_user_data
 * 04apr2021,tk MICRO-2845/PR.28682
 *  - Removed empty blocks in OSAPI_Timer_create_timeout()
 *  - Removed empty blocks in OSAPI_Timer_update_timeout()
 * 23feb2021,tk MICRO-2903/PR#28731
 *     - Use OSAPI_TIMER_NANOSEC_PER_MS instead of OSAPI_TIMER_USEC_PER_SEC in
 *       OSAPI_Timer_new() for clarity. No functional change.
 * 07jan2021,tk
 *   - MICRO-2794/PR#28559
 *     - Rewrote OSAPI_Timer_calculate_ticks() using 64-bit math to avoid
 *       rounding errors.
 * 20oct2020,tk  MICRO-2623/PR#28237 Replaced tabs with spaces
 * 28may2015,tk - MICRO-1272/PR#14880 Always make timeouts less than 1 tick off
 * 28may2015,tk - MICRO-1266/PR#14867 Initialize wheel_count to 0 in
 *                                    schedule_entry
 * 15may2015,tk - MICRO-1176/PR#14677 Changed create_timeout and update_timeout
 *                                    to take timeout in sec,nsec
 * 31jul2014,tk - MICRO-831/PR#9612 Make new() 100% reversible with delete()
 * 29jul2014,tk - MICRO-839/PR#9645 Fixed timeouts < timer resolution
 * 08mar2013,tk - MICRO-223/PR#1403
 *              - MICRO-221/PR#1401
 *              - MICRO-217/PR#1099
 *              - Updated logging
 * 22mar2012,tk Written
 */

/*ce
 * \file
 * \brief Platform independent timer functionality
 * \ingroup OSAPIModule
 *
 *  \details
 */
#include "osapi/osapi_config.h"
#include "osapi/osapi_heap.h"
#include "osapi/osapi_mutex.h"
#include "osapi/osapi_system.h"
#include "osapi/osapi_timer.h"
#include "osapi/osapi_log.h"

#include "System.h"

/*ci \brief The default number of max timer entries
 */
RTI_PRIVATE const RTI_INT32 OSAPI_TIMER_DEFAULT_MAX_ENTRIES = 32;

/*ci \brief Constant for invalid epoch
 */
RTI_PRIVATE const RTI_INT32 OSAPI_TIMER_EPOCH_INVALID = -1;

/*ci \brief Forward declaration of the timer tick function
 */
RTI_PRIVATE void OSAPI_Timer_tick(OSAPI_Timer_T timer);

#if RTIME_USE_32BIT_TIMER_MATH
/*ci \brief microseconds per seconds
 */
RTI_PRIVATE const RTI_INT32 OSAPI_TIMER_USEC_PER_SEC = 1000000;
#endif

/*ci \brief 1 millisecond in nanoseconds
 */
RTI_PRIVATE const RTI_INT32 OSAPI_TIMER_NANOSEC_PER_MS = 1000000;

/*ci \brief nanoseconds per seconds
 */
RTI_PRIVATE const RTI_INT32 OSAPI_TIMER_NSEC_PER_SEC = 1000000000;

/* NOTE: Don't use the REDA data-types as REDA is in a module not available
 * to OSAPI. The functionality is simple and not much code is duplicated.
 */
struct OSAPI_TimerCircularListNode
{
    struct OSAPI_TimerCircularListNode *_prev;
    struct OSAPI_TimerCircularListNode *_next;
};

typedef struct OSAPI_TimerCircularListNode OSAPI_TimerCircularList_T;

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Initialize a list
 *
 * \param[in] c_list List to initialize
 */
RTI_PRIVATE void
OSAPI_TimerCircularList_init(OSAPI_TimerCircularList_T *c_list)
{
    c_list->_next = c_list;
    c_list->_prev = c_list;
}

/*ci
 * \brief Initialize a circular list node
 *
 * \param[in] node List node to initialize
 */
RTI_PRIVATE void
OSAPI_TimerCircularListNode_init(struct OSAPI_TimerCircularListNode *node)
{
  node->_next = NULL;
  node->_prev = NULL;
}

/*ci
 * \brief Link a list node after another node
 *
 * \param[in] prev_node List node to link after
 * \param[in] node List node to link in
 */
RTI_PRIVATE void
OSAPI_TimerCircularList_link_node_after(
                                struct OSAPI_TimerCircularListNode *prev_node,
                                struct OSAPI_TimerCircularListNode *node)
{
  node->_next = prev_node->_next;
  prev_node->_next = node;
  node->_next->_prev = node;
  node->_prev = prev_node;
}

/*ci
 * \brief Unlink a list node
 *
 * \param[in] node List node to unlink
 */
RTI_PRIVATE void
OSAPI_TimerCircularList_unlink_node(struct OSAPI_TimerCircularListNode *node)
{
    if ((node->_next != NULL) && (node->_prev != NULL))
    {
        node->_prev->_next = node->_next;
        node->_next->_prev = node->_prev;
        node->_next = NULL;
        node->_prev = NULL;
    }
}

/*ci
 * \brief Check if a list is empty
 *
 * \param[in] c_list List to check
 *
 * \return RTI_TRUE if list is empty, RTI_FALSE otherwise
 */
RTI_PRIVATE RTI_BOOL
OSAPI_TimerCircularList_is_empty(OSAPI_TimerCircularList_T * c_list)
{
    return (c_list->_next == c_list ? RTI_TRUE : RTI_FALSE);
}

/*ci
 * \brief Convenience macro to return the first node in a list
 */
#define OSAPI_TimerCircularList_get_first(c_list_) ((c_list_)->_next)

/*ci
 * \brief Convenience macro to return the next node
 */
#define OSAPI_TimerCircularListNode_get_next(c_node_) ((c_node_)->_next)

struct OSAPI_TimerEntry
{
    struct OSAPI_TimerCircularListNode _node;

    /*ci \brief The wheel count for restarting a timer
     */
    RTI_INT32 start_wheel_count;

    /*ci \brief The tick count for  restarting a timer
     */
    RTI_INT32 start_ticks;

    /* times aound the wheel */
    RTI_INT32 wheel_count;

    /* User-data stored with the time-out */
    struct OSAPI_TimeoutUserData user_data;

    /* In case an antry is re-used */
    RTI_INT32 epoch;

    /* user callback */
    OSAPI_TimeoutFunction_T timeout;

    RTI_INT32 flags;
};

/*ci \brief State information for one timer
 */
struct OSAPI_Timer
{
    /*ci \brief List of lists
     */
    OSAPI_TimerCircularList_T *wheel;

    /*ci \brief List of free timer entries
     */
    OSAPI_TimerCircularList_T free_head;

    /*ci \brief Copy of potentially modified properties
     */
    struct OSAPI_TimerProperty property;

    /* \brief Current position in wheel
     */
    RTI_INT32 current_slot;

    /* \brief Current epoch for handles
     */
    RTI_INT32 epoch;

    /* \brief Mutex to protect calls to timeouts
     */
    struct OSAPI_Mutex *mutex;

    /* \brief The resolution used internally
     */
    RTI_INT32 resolution;

    /* \brief Whether a mutex needs to be deleted or not
     */
    RTI_BOOL mutex_is_shared;

#if RTIME_USE_32BIT_TIMER_MATH
    /* \brief floor(OSAPI_TIMER_USEC_PER_SEC / timer->resolution)
     */
    RTI_INT32 ticks_per_sec_floor;

    /* \brief Integral number of rounds per sec
     */
    RTI_INT32 rounds_per_sec_floor;

    /* \brief Fractional ticks per wheel in one sec
     */
    RTI_INT32 frac_rounds_per_sec;
#endif

    /* \brief Actual # of slots used
     */
    RTI_INT32 max_slots;
};

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Check if a list is empty
 *
 * \param[in] c_list List to check
 *
 * \return RTI_TRUE if list is empty, RTI_FALSE otherwise
 */
RTI_BOOL
OSAPI_Timer_handle_is_valid(OSAPI_TimeoutHandle_T *h)
{
    return ((h->epoch != OSAPI_TIMER_EPOCH_INVALID) && (h->_entry != NULL) &&
            (h->_entry->epoch != OSAPI_TIMER_EPOCH_INVALID) && (h->_entry->epoch
                    == h->epoch));
}

OSAPI_Timer_T
OSAPI_Timer_new(struct OSAPI_TimerProperty *property, struct OSAPI_Mutex *mutex)
{
    struct OSAPI_Timer *timer = NULL;
    RTI_INT32 i, max;
    struct OSAPI_TimerEntry *entry;

    OSAPI_PRECONDITION((property == NULL) ||
                       (property->max_slots == 0) ||
                       (property->max_entries == 0),
                       return NULL,
               OSAPI_Log_entry_add_pointer("property",property,RTI_FALSE);
               OSAPI_Log_entry_add_int("max_slots",property != NULL ? property->max_slots : 0,RTI_FALSE);
               OSAPI_Log_entry_add_int("max_entries",property != NULL ? property->max_entries : 0,RTI_TRUE);)

    OSAPI_Heap_allocate_struct(&timer, struct OSAPI_Timer);

    if (timer == NULL)
    {
        OSAPI_LOG_TIMER_NEW(OSAPI_LOGKIND_ERROR)
        return NULL;
    }

    timer->wheel = NULL;
    timer->mutex = NULL;
    timer->mutex_is_shared = RTI_FALSE;
    OSAPI_TimerCircularList_init(&timer->free_head);
    timer->property = *property;

    timer->resolution = OSAPI_System_get_timer_resolution();
    if (timer->resolution <= 0)
    {
        OSAPI_LOG_INVALID_TIMER_RES(OSAPI_LOGKIND_ERROR,timer->resolution)
        goto failure;
    }

    /* Ensure # of slots is between [OSAPI_TIMER_MIN_SLOTS,OSAPI_TIMER_MAX_SLOTS]
     * or if the resolution is < 1ms, use OSAPI_TIMER_MAX_SLOTS. For
     * resolutions < 1ms there will be an overflow in the wheel count for
     * resolutions < 1ms for long timeouts with a small number of slots.
     * This logic ensure than timeouts up to 1 year with resolutions as low
     * as 100us can be used. 1 year is that the longest timeout supported by
     *  the DDS Qos policies.
     */
    if ((timer->resolution < OSAPI_TIMER_NANOSEC_PER_MS) ||
        (timer->property.max_slots > OSAPI_TIMER_MAX_SLOTS))
    {
        timer->max_slots = OSAPI_TIMER_MAX_SLOTS;
    }
    else if (timer->property.max_slots < OSAPI_TIMER_MIN_SLOTS)
    {
        timer->max_slots = OSAPI_TIMER_MIN_SLOTS;
    }
    else
    {
        timer->max_slots = timer->property.max_slots;
    }

    OSAPI_Heap_allocate_array(&timer->wheel,
                              (RTI_SIZE_T)timer->max_slots,
                              OSAPI_TimerCircularList_T);
    if (timer->wheel == NULL)
    {
        OSAPI_LOG_TIMER_NEW_WHEEL(OSAPI_LOGKIND_ERROR,timer->max_slots)
        goto failure;
    }

    timer->current_slot = 0;
    timer->epoch = 0;

    for (i = 0; i < timer->max_slots; ++i)
    {
        OSAPI_TimerCircularList_init(&timer->wheel[i]);
    }

    if (timer->property.max_entries > 0)
    {
        max = timer->property.max_entries;
    }
    else
    {
        max = OSAPI_TIMER_DEFAULT_MAX_ENTRIES;
    }

    for (i = 0; i < max; ++i)
    {
        OSAPI_Heap_allocate_struct(&entry, struct OSAPI_TimerEntry);
        if (entry == NULL)
        {
            OSAPI_LOG_TIMER_NEW_ENTRY(OSAPI_LOGKIND_ERROR)
            goto failure;
        }
        OSAPI_TimerCircularListNode_init(&entry->_node);
        entry->epoch = 0;
        entry->start_wheel_count = 0;
        entry->start_ticks = 0;
        entry->wheel_count = 0;
        OSAPI_TimerCircularList_link_node_after(&timer->free_head,
                                                &entry->_node);
    }

    if (mutex == NULL)
    {
        timer->mutex = OSAPI_Mutex_new();
        if (timer->mutex == NULL)
        {
            OSAPI_LOG_TIMER_NEW_MUTEX(OSAPI_LOGKIND_ERROR)
            goto failure;
        }
    }
    else
    {
        timer->mutex = mutex;
        timer->mutex_is_shared = RTI_TRUE;
    }

#if RTIME_USE_32BIT_TIMER_MATH
    timer->ticks_per_sec_floor = OSAPI_TIMER_USEC_PER_SEC / timer->resolution;
    timer->rounds_per_sec_floor = (timer->ticks_per_sec_floor / timer->max_slots);
    timer->frac_rounds_per_sec = timer->ticks_per_sec_floor % timer->max_slots;
#endif

    if (!OSAPI_System_start_timer(timer, OSAPI_Timer_tick))
    {
        OSAPI_LOG_TIMER_NEW_START_TIMER(OSAPI_LOGKIND_ERROR,timer,NULL)
        goto failure;
    }

    return timer;

failure:
#ifndef RTI_CERT
      if (!OSAPI_Timer_delete(timer))
      {
          OSAPI_LOG_TIMER_DELETE(OSAPI_LOGKIND_ERROR)
      }
#endif /* !RTI_CERT */

    return NULL;
}

#ifndef RTI_CERT
RTI_BOOL
OSAPI_Timer_delete(OSAPI_Timer_T timer)
{
    struct OSAPI_TimerEntry *entry, *entry_next;
    RTI_INT32 slot;

    OSAPI_PRECONDITION(timer == NULL,return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("timer",timer,RTI_TRUE);)

    /* OSAPI_System_stop_timer return true even if the timer was
     * not started, thus this is safe.
     */
    if (!OSAPI_System_stop_timer(timer))
    {
        OSAPI_LOG_TIMER_DELETE_STOP_TIMER(OSAPI_LOGKIND_ERROR,timer)
        return RTI_FALSE;
    }

    if (timer->wheel != NULL)
    {
        for (slot = 0; slot < timer->max_slots; ++slot)
        {
            entry = (struct OSAPI_TimerEntry *)
                             OSAPI_TimerCircularList_get_first(&timer->wheel[slot]);
            while (entry != (struct OSAPI_TimerEntry *)&timer->wheel[slot])
            {
                entry_next = (struct OSAPI_TimerEntry*)
                                    OSAPI_TimerCircularListNode_get_next(&entry->_node);
                OSAPI_Heap_free_struct(entry);
                entry = entry_next;
            }
        }
        OSAPI_Heap_free_array(timer->wheel);
    }

    entry = (struct OSAPI_TimerEntry *)
                    OSAPI_TimerCircularList_get_first(&timer->free_head);
    while (entry != (struct OSAPI_TimerEntry *)&timer->free_head)
    {
        entry_next = (struct OSAPI_TimerEntry *)
                            OSAPI_TimerCircularListNode_get_next(&entry->_node);
        OSAPI_Heap_free_struct(entry);
        entry = entry_next;
    }

    if ((timer->mutex != NULL) && !timer->mutex_is_shared)
    {
        if (!OSAPI_Mutex_delete(timer->mutex))
        {
            OSAPI_LOG_TIMER_DELETE_MUTEX(OSAPI_LOGKIND_ERROR,timer->mutex)
            return RTI_FALSE;
        }
    }

    OSAPI_Heap_free_struct(timer);

    return RTI_TRUE;
}
#endif /* RTI_CERT */


#if RTIME_USE_32BIT_TIMER_MATH

/*ci
 * \brief Check if a + b will overflow
 *
 * \param[in] a  a
 * \param[in] b  b
 *
 * \return RTI_TRUE if overflow occurs, RTI_FALSE if not
 */
RTI_PRIVATE RTI_BOOL
OSAPI_Timer_add_ovf(RTI_INT32 a,RTI_INT32 b)
{
    return (a > (INT_MAX - b));
}

/*ci
 * \brief Check if a * b will overflow
 *
 * \param[in] a  a
 * \param[in] b  b
 *
 * \return RTI_TRUE if overflow occurs, RTI_FALSE if not
 */

RTI_PRIVATE RTI_BOOL
OSAPI_Timer_mult_ovf(RTI_INT32 a,RTI_INT32 b)
{
    return (b > 0) && (a > (INT_MAX / b));
}

/*ci
 * \brief Calculate the number of ticks for a timer.
 *
 *
 * \param[in] timer        Timer object
 * \param[in] entry        The timer entry to schedule
 * \param[in] timeout_sec  The timeout in seconds
 * \param[in] timeout_nsec Additional timeout in timeout_nsec
 *
 * \return RTI_FALSE on failure, RTI_TRUE on success.
 */
RTI_PRIVATE RTI_BOOL
OSAPI_Timer_calculate_ticks(OSAPI_Timer_T timer,
                            struct OSAPI_TimerEntry *entry,
                            RTI_INT32 timeout_sec,
                            RTI_INT32 timeout_nsec)
{
    RTI_INT32 ticks = 0;
    RTI_INT32 rem_time = 0;
    RTI_INT32 r0 =0 ;
    RTI_INT32 timeout_sec_adj;
    RTI_INT32 timeout_nsec_adj;
    RTI_INT32 wheel_count;
    RTI_INT32 wheel_count_add;

    timeout_sec_adj = timeout_sec + timeout_nsec / OSAPI_TIMER_NSEC_PER_SEC;
    timeout_nsec_adj = timeout_nsec % OSAPI_TIMER_NSEC_PER_SEC;

    if (timer->resolution < OSAPI_TIMER_USEC_PER_SEC)
    {
        /* Perform the equivalent of
         *
         * ticks_per_sec_floor * timeout_sec_adj
         * -------------------------------------
         *        timer->max_slots
         *
         * (ticks_per_sec_floor / timer->max_slots) = I + R
         * where I denotes the integral and R the remainder.
         *
         * I0 = floor(ticks_per_sec_floor / timer->max_slots)
         * R0 = ticks_per_sec_floor % timer->max_slots
         *
         * timeout_sec_adj * (I + R)
         *
         * wheel_count = I * timeout_sec_adj;
         *
         */

        /* Check if result will overflow */
        if (OSAPI_Timer_mult_ovf(timer->rounds_per_sec_floor,timeout_sec_adj))
        {
            OSAPI_LOG_MATH_OFV(OSAPI_LOGKIND_ERROR,timer->rounds_per_sec_floor,timeout_sec_adj)
            return RTI_FALSE;
        }

        wheel_count = timer->rounds_per_sec_floor * timeout_sec_adj;

        /*
         * It may be that wheel_count == 0, for example many slots, low
         * resolution. This, timeout_sec_adj may still be large. Thus,
         * determine additional rounds in the wheel and the equivalent of
         *
         *   R0 * timeout_sec_adj
         * -------------------------
         * timer->max_slots
         *
         * I1 = floor(timeout_sec_adj / timer->max_slots)
         * R1 = (timeout_sec_adj % timer->max_slots)
         *
         * wheel_count += I1 * R0
         */
        rem_time = (timeout_sec_adj / timer->max_slots);

        /* Check if result will overflow */
        if (OSAPI_Timer_mult_ovf(rem_time,timer->frac_rounds_per_sec))
        {
            OSAPI_LOG_MATH_OFV(OSAPI_LOGKIND_ERROR,rem_time,timer->frac_rounds_per_sec);
            return RTI_FALSE;
        }

        wheel_count_add = rem_time * timer->frac_rounds_per_sec;

        if (OSAPI_Timer_add_ovf(wheel_count,wheel_count_add))
        {
            OSAPI_LOG_MATH_OFV(OSAPI_LOGKIND_ERROR,wheel_count,wheel_count_add)
            return RTI_FALSE;
        }

        wheel_count += wheel_count_add;

        /* Finally, the remaining time is the number of ticks that needs to be
         * added to the starting point in the wheel (may be adjusted based on
         * fractions.
         */
        rem_time  = (timeout_sec_adj %  timer->max_slots);

        /* Check if result will overflow */
        if (OSAPI_Timer_mult_ovf(rem_time,timer->frac_rounds_per_sec))
        {
            OSAPI_LOG_MATH_OFV(OSAPI_LOGKIND_ERROR,rem_time,timer->frac_rounds_per_sec)
            return RTI_FALSE;
        }

        ticks = rem_time * timer->frac_rounds_per_sec;

        /* Add the nanosec part (normalized < 1s above) */
        rem_time = (timeout_nsec_adj / 1000) + ((timeout_nsec_adj % 1000) ? 1 : 0);

        r0 = rem_time / timer->resolution;

        if (OSAPI_Timer_add_ovf(ticks,r0))
        {
            OSAPI_LOG_MATH_OFV(OSAPI_LOGKIND_ERROR,ticks,r0)
            return RTI_FALSE;
        }

        ticks += r0;
    }
    else
    {
        /* When the resolution is in the seconds, divide the seconds, the
         * remainder is calculated later.
         */
        ticks = timeout_sec_adj / timer->resolution;
        wheel_count = ticks / timer->max_slots;
        ticks = timeout_sec_adj % timer->resolution;

        /* Add the nanosec part (normalized < 1s above) */
        rem_time = (timeout_nsec_adj / 1000) + ((timeout_nsec_adj % 1000) ? 1 : 0);

        r0 = rem_time / timer->resolution;

        if (OSAPI_Timer_add_ovf(ticks ,r0))
        {
            OSAPI_LOG_MATH_OFV(OSAPI_LOGKIND_ERROR,ticks,r0)
            return RTI_FALSE;
        }

        ticks += r0;
    }

    {
        /* This code applies only to systems with a timer resolution that is
         * not an integral number of 1000000000 nanoseconds.
         */
        RTI_INT32 ticks_per_tick_loss_floor = 0;
        RTI_INT32 nanosec_loss = OSAPI_TIMER_NSEC_PER_SEC % OSAPI_System_get_timer_resolution();
        RTI_INT32 lost_ticks = 0;
        RTI_INT32 rem_ticks = 0;
        RTI_INT32 rem_ticks1 = 0;
        RTI_INT32 ticks_sec_tick_loss = 0;

        if (nanosec_loss > 0)
        {
            RTI_INT32 nanosec_loss_per_tick = nanosec_loss / timer->ticks_per_sec_floor;


            if (nanosec_loss_per_tick > 0)
            {
                ticks_per_tick_loss_floor =  timer->resolution * 1000 / (nanosec_loss / timer->ticks_per_sec_floor);
            }
            else
            {
                ticks_per_tick_loss_floor = timer->resolution * 1000;
            }

            ticks_sec_tick_loss = ticks_per_tick_loss_floor / timer->ticks_per_sec_floor;

            lost_ticks = timeout_sec_adj / ticks_sec_tick_loss;

            if (OSAPI_Timer_add_ovf(ticks,lost_ticks))
            {
                OSAPI_LOG_MATH_OFV(OSAPI_LOGKIND_ERROR,ticks,lost_ticks)
                return RTI_FALSE;
            }

            rem_ticks = ticks / ticks_per_tick_loss_floor;
            rem_ticks1 = ticks % ticks_per_tick_loss_floor;

            ticks += lost_ticks;

            if (OSAPI_Timer_add_ovf(ticks,rem_ticks))
            {
                OSAPI_LOG_MATH_OFV(OSAPI_LOGKIND_ERROR,ticks,rem_ticks)
                return RTI_FALSE;
            }

            ticks += rem_ticks;
        }

        if ((rem_ticks1 != 0) || (rem_time % timer->resolution))
        {
            if (OSAPI_Timer_add_ovf(ticks,1))
            {
                OSAPI_LOG_MATH_OFV(OSAPI_LOGKIND_ERROR,ticks,1)
                        return RTI_FALSE;
            }
            ++ticks;

            /* Add 1 more tick if the sum of the remainders round up to
             * two ticks
             */
            if (rem_ticks1 > (ticks_per_tick_loss_floor))
            {
                if (OSAPI_Timer_add_ovf(ticks,1))
                {
                    OSAPI_LOG_MATH_OFV(OSAPI_LOGKIND_ERROR,ticks,lost_ticks)
                    return RTI_FALSE;
                }
                ++ticks;
            }
        }
    }

    /* Round up 1 tick. */
    if ((timeout_sec == 0) && (timeout_nsec == 0))
    {
        if (ticks == INT_MAX)
        {
            OSAPI_LOG_MATH_OFV(OSAPI_LOGKIND_ERROR,ticks,1)
            return RTI_FALSE;
        }
        ++ticks;
    }

    /* Handle case where an incomplete round was of by less than additional
     * calculated ticks, thus adding more rounds.
     */
    if (ticks >= timer->max_slots)
    {
        rem_time = ticks / timer->max_slots;

        if (OSAPI_Timer_add_ovf(wheel_count ,rem_time))
        {
            OSAPI_LOG_MATH_OFV(OSAPI_LOGKIND_ERROR,wheel_count,rem_time)
            return RTI_FALSE;
        }

        wheel_count += rem_time;

        ticks = ticks % timer->max_slots;

        /* For the first timeout a tick is added, thus make sure it cannot
         * overflow.
         */
        if ((ticks == INT_MAX) && (wheel_count == INT_MAX))
        {
            OSAPI_LOG_MATH_OFV(OSAPI_LOGKIND_ERROR,ticks,wheel_count)
            return RTI_FALSE;
        }
    }

    entry->start_wheel_count = wheel_count;
    entry->start_ticks = ticks;

    return RTI_TRUE;
}

#else
/*ci
 * \brief Calculate the number of ticks for a timer.
 *
 *
 * \param[in] timer        Timer object
 * \param[in] entry        The timer entry to schedule
 * \param[in] timeout_sec  The timeout in seconds
 * \param[in] timeout_nsec Additional timeout in timeout_nsec
 *
 * \return RTI_FALSE on failure, RTI_TRUE on success.
 */
RTI_PRIVATE RTI_BOOL
OSAPI_Timer_calculate_ticks(OSAPI_Timer_T timer,
                            struct OSAPI_TimerEntry *entry,
                            RTI_INT32 timeout_sec,
                            RTI_INT32 timeout_nsec)
{
    RTI_UINT64 total_time;
    RTI_UINT64 total_ticks;
    RTI_UINT64 total_wheel_count;

    entry->start_wheel_count = 0;
    entry->start_ticks = 0;

    if ((timeout_sec == 0) && (timeout_nsec == 0))
    {
        return RTI_TRUE;
    }

    /* This cannot overflow since the inputs are two INT32.
     */
    total_time = ((RTI_UINT64)timeout_sec * (RTI_UINT64)OSAPI_TIMER_NSEC_PER_SEC)
                 + (RTI_UINT64)timeout_nsec;

    total_ticks = total_time / (RTI_UINT64)timer->resolution;
    if (total_time % (RTI_UINT64)timer->resolution)
    {
        ++total_ticks;
    }

    total_wheel_count = total_ticks / (RTI_UINT64)timer->max_slots;

    /* Check that the result can be stored in an INT32 (start_wheel_count)
     */
    if (total_wheel_count > INT_MAX)
    {
        return RTI_FALSE;
    }

    entry->start_wheel_count = (RTI_INT32)total_wheel_count;
    entry->start_ticks = (RTI_INT32)(total_ticks % (RTI_UINT64)timer->max_slots);

    if ((entry->start_ticks == (timer->max_slots - 1))
         && (entry->start_wheel_count == INT_MAX))
    {
        /* A new entry always adds 1 tick. Thus if adding one more tick
         * would increment the start_wheel_count and start_wheel_count is
         * INT_MAX, there would be an overflow.
         */
        OSAPI_LOG_MATH_OFV(OSAPI_LOGKIND_ERROR,
                           entry->start_ticks,entry->start_wheel_count)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}
#endif


/*ci
 * \brief Schedule a timeout in the timer wheel
 *
 * \param[in] timer        Timer object
 * \param[in] entry        The timer entry to schedule
 */
RTI_PRIVATE void
OSAPI_Timer_schedule_entry(OSAPI_Timer_T timer,struct OSAPI_TimerEntry *entry)
{
    OSAPI_TimerCircularList_T *slot_entry;
    RTI_INT32 slot_num;
    RTI_INT32 ticks;

    entry->wheel_count = entry->start_wheel_count;
    ticks = entry->start_ticks;

    if (entry->flags & OSAPI_TIMER_NEW_ENTRY)
    {
        /* Round up the first tick since it is unknown exactly where in a
         * clock period the entry is scheduled. This means that the first
         * tick can be up to 2 clock ticks late.
         */
        ++ticks;
        entry->flags &= ~OSAPI_TIMER_NEW_ENTRY;
        if (ticks == timer->max_slots)
        {
            entry->wheel_count++;
            ticks = 0;
        }
    }

    slot_num = (timer->current_slot + ticks) % timer->max_slots;

    /* If this is true then entry->wheel_count must >= 1 */
    if (slot_num == timer->current_slot)
    {
        --entry->wheel_count;
    }

    slot_entry = &timer->wheel[slot_num];

    OSAPI_TimerCircularList_link_node_after(slot_entry, &entry->_node);
}

/*ci
 * \brief Update all timer entries 1 tick forward
 *
 * \details
 * This function is assumed to be called on each clock tick with the
 * resolution returned by get_timer_resolution. For each tick the
 * wheel rotates 1 slot and updates all timer elements in that slot. If
 * an entry's timeout has expired the timeout handler is called. If the timer
 * entry is manually updated it is not rescheduled, otherwise it is rescheduled
 * and placed in a new slot.
 *
 * \param[in] timer The timer object passed to the OS in start_timer().
 */
RTI_PRIVATE void
OSAPI_Timer_tick(OSAPI_Timer_T timer)
{
    OSAPI_TimerCircularList_T *entry_list;
    struct OSAPI_TimerEntry *entry, *entry_next;
    OSAPI_TimeoutOp_t timeout_op;
    RTI_BOOL bretval;

    if (!OSAPI_Mutex_take(timer->mutex))
    {
        OSAPI_LOG_TIMER_TICK_MUTEX(OSAPI_LOGKIND_ERROR,timer->mutex,1)
        return;
    }

    timer->current_slot = (timer->current_slot + 1) % timer->max_slots;
    entry_list = &timer->wheel[timer->current_slot];

    entry = (struct OSAPI_TimerEntry*)
                                 OSAPI_TimerCircularList_get_first(entry_list);

    /* NOTE: The (entry != NULL) is redundant. It cannot be NULL since this is a
     * circular list and entry_next must always be != NULL since it is assigned 
     * before entry is unlinked and the head can never be unlinked. 
     * It was added to avoid a warning from the Xcode static analyzer.
     */
    while ((entry != NULL) && (entry != (struct OSAPI_TimerEntry *)entry_list))
    {
        entry_next = (struct OSAPI_TimerEntry*)
                            OSAPI_TimerCircularListNode_get_next(&entry->_node);

        if (entry->wheel_count == 0)
        {
            OSAPI_TimerCircularList_unlink_node(&entry->_node);
            timeout_op = entry->timeout(&entry->user_data);

            if (timeout_op == OSAPI_TIMEOUT_OP_MANUAL)
            {
                entry = entry_next;
                continue;
            }

            /* Only reschedule a valid, periodic timer */
            if ((entry->flags & OSAPI_TIMER_PERIODIC) && (entry->epoch > 0))
            {
                OSAPI_Timer_schedule_entry(timer, entry);
            }
        }
        else
        {
            --entry->wheel_count;
        }

        entry = entry_next;
    }

    bretval = OSAPI_Mutex_give(timer->mutex);

#if OSAPI_ENABLE_LOG
    if (!bretval)
    {
        OSAPI_LOG_TIMER_TICK_MUTEX(OSAPI_LOGKIND_ERROR,timer->mutex,0)
    }
#else
    IGNORE_RETVAL(bretval);
#endif
}

RTI_BOOL
OSAPI_Timer_create_timeout(OSAPI_Timer_T timer,
                           OSAPI_TimeoutHandle_T *out_handle,
                           RTI_INT32 timeout_sec,
                           RTI_INT32 timeout_nsec,
                           RTI_INT32 flags,
                           OSAPI_TimeoutFunction_T timeout_handler,
                           struct OSAPI_TimeoutUserData *user_data)
{
    struct OSAPI_TimerEntry *entry;
    RTI_BOOL brc;

    OSAPI_PRECONDITION((timer == NULL) || (out_handle == NULL) ||
                            (timeout_sec < 0) || (timeout_nsec < 0) ||
                            (timeout_nsec > 999999999) ||
                            (timeout_handler == NULL),
                           return RTI_FALSE,
           OSAPI_Log_entry_add_pointer("timer",timer,RTI_FALSE);
           OSAPI_Log_entry_add_pointer("out_handle",out_handle,RTI_FALSE);
           OSAPI_Log_entry_add_int("timeout_sec",timeout_sec,RTI_FALSE);
           OSAPI_Log_entry_add_int("timeout_nsec",timeout_nsec,RTI_FALSE);
           OSAPI_Log_entry_add_pointer("timeout_handler",NULL,RTI_TRUE);)

    if (!OSAPI_Mutex_take(timer->mutex))
    {
        OSAPI_LOG_TIMER_MUTEX(OSAPI_LOGKIND_ERROR,timer->mutex,1)
        return RTI_FALSE;
    }

    if (OSAPI_TimerCircularList_is_empty(&timer->free_head))
    {
        brc = OSAPI_Mutex_give(timer->mutex);
#if OSAPI_ENABLE_LOG
        if (!brc)
        {
            OSAPI_LOG_TIMER_MUTEX(OSAPI_LOGKIND_ERROR,timer->mutex,0)
        }
#else
        IGNORE_RETVAL(brc);
#endif
        return RTI_FALSE;
    }

    entry = (struct OSAPI_TimerEntry *)
                        OSAPI_TimerCircularList_get_first(&timer->free_head);

    OSAPI_TimerCircularList_unlink_node(&entry->_node);

    ++timer->epoch;
    entry->epoch = timer->epoch;
    if (user_data != NULL)
    {
        entry->user_data = *user_data;
    }
    entry->timeout = timeout_handler;
    entry->flags = flags;
    entry->flags |= OSAPI_TIMER_NEW_ENTRY;

    if (!OSAPI_Timer_calculate_ticks(timer,entry,timeout_sec,timeout_nsec))
    {
        OSAPI_TimerCircularList_link_node_after(timer->free_head._prev,
                                                &entry->_node);
        brc = OSAPI_Mutex_give(timer->mutex);
#if OSAPI_ENABLE_LOG
        if (!brc)
        {
            OSAPI_LOG_TIMER_MUTEX(OSAPI_LOGKIND_ERROR,timer->mutex,0)
        }
#else
        IGNORE_RETVAL(brc);
#endif
        return RTI_FALSE;
    }

    OSAPI_Timer_schedule_entry(timer, entry);

    out_handle->epoch = entry->epoch;
    out_handle->_entry = entry;

    brc = OSAPI_Mutex_give(timer->mutex);
    if (!brc)
    {
        OSAPI_LOG_TIMER_MUTEX(OSAPI_LOGKIND_ERROR,timer->mutex,0)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
OSAPI_Timer_update_timeout(OSAPI_Timer_T timer,
                           OSAPI_TimeoutHandle_T *handle,
                           RTI_INT32 timeout_sec,
                           RTI_INT32 timeout_nsec)

{
    RTI_BOOL retval = RTI_FALSE;

    OSAPI_PRECONDITION((timer == NULL) || (handle == NULL) ||
                       (timeout_sec < 0) || (timeout_nsec < 0) ||
                       (timeout_nsec > 999999999),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("timer",timer,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("handle",handle,RTI_FALSE);
                       OSAPI_Log_entry_add_int("timeout_sec",timeout_sec,RTI_FALSE);
                       OSAPI_Log_entry_add_int("timeout_nsec",timeout_nsec,RTI_TRUE);)

    if (!OSAPI_Mutex_take(timer->mutex))
    {
        OSAPI_LOG_TIMER_MUTEX(OSAPI_LOGKIND_ERROR,timer->mutex,1)
        return RTI_FALSE;
    }

    if (OSAPI_Timer_handle_is_valid(handle))
    {
        OSAPI_TimerCircularList_unlink_node(&handle->_entry->_node);

        handle->_entry->flags |= OSAPI_TIMER_NEW_ENTRY;

        if (!OSAPI_Timer_calculate_ticks(timer,handle->_entry,timeout_sec,timeout_nsec))
        {
            retval = OSAPI_Mutex_give(timer->mutex);
#if OSAPI_ENABLE_LOG
            if (!retval)
            {
                OSAPI_LOG_TIMER_MUTEX(OSAPI_LOGKIND_ERROR,timer->mutex,0)
            }
#else
            IGNORE_RETVAL(retval);
#endif
            return RTI_FALSE;
        }

        OSAPI_Timer_schedule_entry(timer,handle->_entry);

        retval = RTI_TRUE;
    }

    if (!OSAPI_Mutex_give(timer->mutex))
    {
        OSAPI_LOG_TIMER_MUTEX(OSAPI_LOGKIND_ERROR,timer->mutex,0)
        return RTI_FALSE;
    }

    return retval;
}

RTI_BOOL
OSAPI_Timer_delete_timeout(OSAPI_Timer_T timer, OSAPI_TimeoutHandle_T *handle)
{
    RTI_BOOL retval = RTI_FALSE;

    OSAPI_PRECONDITION((timer == NULL) || (handle == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("timer",timer,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("handle",handle,RTI_TRUE);)

    if (!OSAPI_Mutex_take(timer->mutex))
    {
        OSAPI_LOG_TIMER_MUTEX(OSAPI_LOGKIND_ERROR,timer->mutex,1)
        return RTI_FALSE;
    }

    if (OSAPI_Timer_handle_is_valid(handle))
    {
        OSAPI_TimerCircularList_unlink_node(&handle->_entry->_node);
        OSAPI_TimerCircularList_link_node_after(timer->free_head._prev,
                                               &handle->_entry->_node);
        handle->epoch = OSAPI_TIMER_EPOCH_INVALID;
        handle->_entry->epoch = OSAPI_TIMER_EPOCH_INVALID;

        retval = RTI_TRUE;
    }

    if (!OSAPI_Mutex_give(timer->mutex))
    {
        OSAPI_LOG_TIMER_MUTEX(OSAPI_LOGKIND_ERROR,timer->mutex,0)
        return RTI_FALSE;
    }

    return retval;
}

#ifndef RTI_CERT
RTI_BOOL
OSAPI_TimeoutHandle_get_user_data(struct OSAPI_TimeoutUserData *out,
                                  OSAPI_TimeoutHandle_T *handle)
{
    OSAPI_PRECONDITION((out == NULL) || (handle == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("out",out,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("handle",(void*)handle,RTI_TRUE);)

    if ((handle->_entry != NULL) && (handle->epoch == handle->_entry->epoch))
    {
        *out = handle->_entry->user_data;
        return RTI_TRUE;
    }

    OSAPI_LOG_TIMER_GET_USER_DATA_EPOCH(
                OSAPI_LOGKIND_ERROR,handle,handle->_entry,
                handle->epoch,
                (handle->_entry == NULL) ? 0 : handle->_entry->epoch)

    return RTI_FALSE;
}
#endif
