/*
 * FILE: autosarSempahore.c - AutoSAR semaphore functionality
 *
 * Copyright 2019-2026 Real-Time Innovations, Inc.
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
 * 20sep2021,tk MICRO-3152/PR.29564
 * - Only include include reserved semaphore->id_offset = 0 when
 *   OSAPI_THREAD_SUSPEND_ENABLE when OSAPI_THREAD_SEMAPHORE_ENABLED is TRUE.
 * 14sep2021,tk MICRO-3152/PR.29564
 * - Changed OSAPI_Semaphore_new to use the reserved, the first, semaphore,
 *   semaphore to create a if the owner is the UDP Task.
 * 14sep2021,tk MICRO-3266/PR.29650
 * - Moved canceling an alarm from give() to immediately after WaitEvent
 *   returns in take() to prevent a previous alarm from
 *   prematurely waking up a semaphore on a following take().
 * 15mar2021,fmt MICRO-2953/PR#28897 
 *    - Fix calculation of event masks of semaphores.
 * 12dec2020,fmt MICRO-2778/PR#28506
 *    - Correctly set fail_reason in OSAPI_Semaphore_take().
 * 26nov2020,fmt MICRO-2664/PR#28260
 *    - Check if input parameter 'failReason' is NULL pointer
 *    - Update to 'fail_reason' the name of parameter 'failReason' in function
 *      OSAPI_Semaphore_take()
 * 20oct2020,tk  MICRO-2623/PR#28237 Replaced tabs with spaces
 * 6oct2020,fmt MICRO-2585/PR.28155
 *    - Move consistency check of properties from
 *      OSAPI_AutosarSemaphore_is_initialized() to
 *      OSAPI_SystemAutosar_initialize()
 * 2oct2020,fmt MICRO-2581/PR.28133
 *    - Handle return values of CancelAlarm() correctly
 * 25aug2020,fmt MICRO-2500/PR.28028
 *    - The AUTOSAR port does not check the return value for AUTOSAR API calls
 * 21may2020,fmt MICRO-2409/PR.27650 Exclude from cert build finalize() functions
 * 21may2020,fmt MICRO-2405/PR.27629 Simplify Autosar initialize() functions
 * 13mar2019,fmt Written
 *
 */
/*ce
 * \file
 * \brief AutoSAR implementation of OSAPI semaphore routines
 */
#include "rti_me_psl.h"

#include "autosarMutex.h"
#include "autosarSemaphore.h"

/*ci \brief Calculates the event id based on the first event and the event
 * offset.
 */
#define AUTOSAR_SEMAPHORE_GET_EVENT_ID(event, offset) ((event) << (offset))

/*ci \brief Note that first resource id is used to sync access to this variable.
 * Mutex module uses the same id to sync access to internal variables.
 */
RTI_PRIVATE ResourceType OSAPI_Autosar_fv_CurrentResourceId = 0;

/*ci \brief Whether the Autosar semaphores have been initialized or not
 */
RTI_PRIVATE RTI_BOOL OSAPI_AutosarSemaphore_fv_IsInitialized = RTI_FALSE;


/*** SOURCE_BEGIN ***/

/* OSAPI autosar module private functions */

/*ci \brief Ensures that Autosar semaphore module is initialized.
 *
 *  \return TRUE if module is initialized with no error or already initialized.
 *          Otherwise FALSE.
 */
RTI_PRIVATE FUNC(boolean, SOAD_CODE)
OSAPI_AutosarSemaphore_is_initialized(void)
{
    if (OSAPI_AutosarSemaphore_fv_IsInitialized)
    {
        return TRUE;
    }

    OSAPI_Autosar_fv_CurrentResourceId = 0;

    OSAPI_AutosarSemaphore_fv_IsInitialized = RTI_TRUE;

    return TRUE;
}

/* OSAPI autosar module public functions */

#ifndef RTI_CERT
FUNC(void, SOAD_CODE)
OSAPI_AutosarSemaphore_finalize(void)
{
    OSAPI_Autosar_fv_CurrentResourceId = 0;
    OSAPI_AutosarSemaphore_fv_IsInitialized = RTI_FALSE;
}
#endif

/* OSAPI public functions */

#ifndef RTI_CERT
FUNC(RTI_BOOL, SOAD_CODE)
OSAPI_Semaphore_delete(P2VAR(OSAPI_Semaphore_T, AUTOMATIC, SOAD_APPL_DATA) me)
{
    StatusType ret_value;
    uint16 current_id_offset;

    OSAPI_PRECONDITION(me == NULL_PTR,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("me",me,RTI_TRUE);)

    current_id_offset = me->id_offset;
    OSAPI_Heap_free_struct(me);
    
    ret_value = OSAPI_AutosarMutex_enter_critical_section(
                    OSAPI_System_gv_PortProperty->mutex_resource_id);
    if (ret_value != E_OK)
    {
        OSAPI_LOG_SEMAPHORE_DELETE(OSAPI_LOGKIND_ERROR, ret_value)
        goto failure;
    }

    /* If the semaphore being deleted is the last one created we can safely
     * decrease the current offset.
     */
    if ((current_id_offset + 1) == OSAPI_Autosar_fv_CurrentResourceId)
    {
        OSAPI_Autosar_fv_CurrentResourceId--;
    }
    
    ret_value = OSAPI_AutosarMutex_leave_critical_section(
                    OSAPI_System_gv_PortProperty->mutex_resource_id);
    if (ret_value != E_OK)
    {
        OSAPI_LOG_SEMAPHORE_DELETE(OSAPI_LOGKIND_ERROR, ret_value)
        goto failure;
    }

    return RTI_TRUE;

failure:
    return RTI_FALSE;
}
#endif

FUNC(P2VAR(OSAPI_Semaphore_T, AUTOMATIC, SOAD_APPL_DATA), SOAD_CODE)
OSAPI_Semaphore_new(void)
{
    StatusType ret_value;
    P2VAR(struct OSAPI_Semaphore, AUTOMATIC, SOAD_APPL_DATA) semaphore = NULL_PTR;

    if (!OSAPI_AutosarSemaphore_is_initialized())
    {
        OSAPI_LOG_SEMAPHORE_NEW(OSAPI_LOGKIND_ERROR)
        return NULL_PTR;
    }

    OSAPI_Heap_allocate_struct(&semaphore, struct OSAPI_Semaphore);
    if (semaphore == NULL_PTR)
    {
        OSAPI_LOG_SEMAPHORE_NEW(OSAPI_LOGKIND_ERROR)
        return NULL_PTR;
    }

    /* Get the taskId owner of the semaphore */
    ret_value = GetTaskID(&semaphore->task_id);
    if (ret_value != E_OK)
    {
        OSAPI_LOG_SEMAPHORE_NEW(OSAPI_LOGKIND_ERROR)
        goto failure;
    }

    ret_value = OSAPI_AutosarMutex_enter_critical_section(
                    OSAPI_System_gv_PortProperty->mutex_resource_id);
    if (ret_value != E_OK)
    {
        OSAPI_LOG_SEMAPHORE_NEW(OSAPI_LOGKIND_ERROR)
        goto failure;
    }

    if (OSAPI_Autosar_fv_CurrentResourceId >=
        OSAPI_System_gv_PortProperty->semaphore_max_count)
    {
        ret_value = OSAPI_AutosarMutex_leave_critical_section(
                        OSAPI_System_gv_PortProperty->mutex_resource_id);
        IGNORE_RETVAL(ret_value);

        OSAPI_LOG_SEMAPHORE_NEW(OSAPI_LOGKIND_ERROR)
        goto failure;
    }

    semaphore->id_offset = (uint16)OSAPI_Autosar_fv_CurrentResourceId;
    OSAPI_Autosar_fv_CurrentResourceId++;

    ret_value = OSAPI_AutosarMutex_leave_critical_section(
                    OSAPI_System_gv_PortProperty->mutex_resource_id);
    if (ret_value != E_OK)
    {
        OSAPI_LOG_SEMAPHORE_NEW(OSAPI_LOGKIND_ERROR)
        goto failure;
    }
    
    ret_value = ClearEvent(
        (AUTOSAR_SEMAPHORE_GET_EVENT_ID(
            OSAPI_System_gv_PortProperty->first_give_event, semaphore->id_offset)) |
        (AUTOSAR_SEMAPHORE_GET_EVENT_ID(
            OSAPI_System_gv_PortProperty->first_timeout_event, semaphore->id_offset)));
    if (ret_value != E_OK)
    {
        OSAPI_LOG_SEMAPHORE_NEW(OSAPI_LOGKIND_ERROR)
        goto failure;
    }


    return semaphore;

failure:
    OSAPI_Heap_free_struct(semaphore);

    return NULL_PTR;
}

FUNC(RTI_BOOL, SOAD_CODE)
OSAPI_Semaphore_take(P2VAR(OSAPI_Semaphore_T, AUTOMATIC, SOAD_APPL_DATA) me,
                     RTI_INT32 timeoutMs,
                     P2VAR(RTI_INT32, AUTOMATIC, SOAD_APPL_DATA) fail_reason)
{
    StatusType status;
    StatusType cancel_status;
    EventMaskType event;

    OSAPI_PRECONDITION(me == NULL_PTR,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("me",me,RTI_TRUE);)

    /* Note that multiple threads taking the same semaphore is not supported,
     * and Micro does not do this internally. Thus, upon entering this function,
     * no alarms can be active.
     */

    if (fail_reason != NULL)
    {
        /* By default set the fail_reason to error, and only
         * if result is ok or there is a timeout the fail_reason
         * value will be udpated
         */
        *fail_reason = OSAPI_SEMAPHORE_RESULT_ERROR;
    }

    /* If timeoutMs is 0 can't be used with call to SetRelAlarm(). 
     * In that case just return expired if the event is not set or ok 
     * if the event is already set.
     */
    if (timeoutMs != 0)
    {
        /* No need to set alarm if timeout time is infinite. In that case only
         * a call to OSAPI_Semaphore_give() will continue this task execution.
         */
        if (timeoutMs != OSAPI_SEMAPHORE_TIMEOUT_INFINITE)
        {
            /* As indicated in documentation user is responsible for configuring
             * the alarm with 1 tick per millisecond.
             */
            status = SetRelAlarm(
                         OSAPI_System_gv_PortProperty->first_alarm + me->id_offset,
                         timeoutMs, 0);
            if (status != E_OK)
            {
                OSAPI_LOG_SEMAPHORE_TAKE(OSAPI_LOGKIND_ERROR, status)
                return FALSE;
            }
        }
        status = WaitEvent(
            AUTOSAR_SEMAPHORE_GET_EVENT_ID(
                OSAPI_System_gv_PortProperty->first_give_event,
                me->id_offset) |
            AUTOSAR_SEMAPHORE_GET_EVENT_ID(
                OSAPI_System_gv_PortProperty->first_timeout_event,
                me->id_offset));

        /* Cancel the alarm, if the alarm was already signaled it is ok.
         */
        cancel_status = CancelAlarm(OSAPI_System_gv_PortProperty->first_alarm + me->id_offset);
        if ((cancel_status != E_OK) && (cancel_status != E_OS_NOFUNC))
        {
            OSAPI_LOG_SEMAPHORE_GIVE(OSAPI_LOGKIND_ERROR, cancel_status)
            return FALSE;
        }

        if (status != E_OK)
        {
            OSAPI_LOG_SEMAPHORE_TAKE(OSAPI_LOGKIND_ERROR, status)
            return FALSE;
        }
    }
        
    status = GetEvent(me->task_id, &event);
    if (status != E_OK)
    {
        OSAPI_LOG_SEMAPHORE_TAKE(OSAPI_LOGKIND_ERROR, status)
        return FALSE;
    }
    status = ClearEvent(
                 AUTOSAR_SEMAPHORE_GET_EVENT_ID(
                     OSAPI_System_gv_PortProperty->first_give_event,
                     me->id_offset) |
                 AUTOSAR_SEMAPHORE_GET_EVENT_ID(
                     OSAPI_System_gv_PortProperty->first_timeout_event,
                     me->id_offset));
    if (status != E_OK)
    {
        OSAPI_LOG_SEMAPHORE_TAKE(OSAPI_LOGKIND_ERROR, status)
        return FALSE;
    }

    if (event & AUTOSAR_SEMAPHORE_GET_EVENT_ID(
                    OSAPI_System_gv_PortProperty->first_give_event,
                    me->id_offset))
    {
        if (fail_reason != NULL)
        {
            *fail_reason = OSAPI_SEMAPHORE_RESULT_OK;
        }
    }
    else if ((event & AUTOSAR_SEMAPHORE_GET_EVENT_ID(
                          OSAPI_System_gv_PortProperty->first_timeout_event,
                          me->id_offset)) ||
             (timeoutMs == 0))
    {
        if (fail_reason != NULL)
        {
            *fail_reason = OSAPI_SEMAPHORE_RESULT_TIMEOUT;
        }
    }
    else
    {
        /* *fail_reason, if not NULL, is already set to
         * OSAPI_SEMAPHORE_RESULT_ERROR
         */
        OSAPI_LOG_SEMAPHORE_TAKE(OSAPI_LOGKIND_ERROR, status)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

FUNC(RTI_BOOL, SOAD_CODE)
OSAPI_Semaphore_give(P2VAR(OSAPI_Semaphore_T, AUTOMATIC, SOAD_APPL_DATA) me)
{
    StatusType status;

    OSAPI_PRECONDITION(me == NULL_PTR,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("me",me,RTI_TRUE);)

    status = SetEvent(me->task_id,
                      AUTOSAR_SEMAPHORE_GET_EVENT_ID(
                          OSAPI_System_gv_PortProperty->first_give_event,
                          me->id_offset));
    if (status != E_OK)
    {
        OSAPI_LOG_SEMAPHORE_GIVE(OSAPI_LOGKIND_ERROR, status)
        return FALSE;
    }

    return RTI_TRUE;
}
