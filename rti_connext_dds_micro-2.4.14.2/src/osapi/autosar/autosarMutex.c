/*
 * FILE: autosarMutex.c - AutoSAR mutex functionality
 *
 * Copyright 2019-2021 Real-Time Innovations, Inc.
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
 * 6oct2020,fmt MICRO-2585/PR.28155
 *   - Move consistency check of properties from
 *     OSAPI_AutosarMutex_is_initialized() to
 *     OSAPI_SystemAutosar_initialize()
 * 21may2020,fmt MICRO-2409/PR.27650 Exclude from cert build finalize() functions
 * 21may2020,fmt MICRO-2405/PR.27629 Simplify Autosar initialize() functions
 * 13mar2019,fmt Written
 *
 */
/*ce
 * \file
 * \brief AutoSAR implementation of OSAPI mutex routines
 */
#include "osapi/osapi_config.h"

#if OSAPI_INCLUDE_AUTOSAR

#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef osapi_mutex_h
#include "osapi/osapi_mutex.h"
#endif
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif

#include "../common/Mutex.h"
#include "autosarMutex.h"

#if OSAPI_HAS_ATOMICS
#include "Atomics.h"
#endif /* OSAPI_HAS_ATOMICS */


/*ci \brief Next Autosar Resource ID used in a mutex object.
 */
RTI_PRIVATE ResourceType OSAPI_Mutex_fv_CurrentResourceId = 0;

/*ci \brief When the synchronization method is OSAPI_AUTOSAR_SYNCKIND_SPINLOCK a
 * singleton mutex is used for all mutexes created.
 */
RTI_PRIVATE struct OSAPI_Mutex OSAPI_Mutex_fv_Mutex = {{0, RTI_FALSE, 0}, 0};

/*ci \brief Whether the Autosar mutexes have been initialized or not
 */
RTI_PRIVATE RTI_BOOL OSAPI_AutosarMutex_fv_IsInitialized = RTI_FALSE;

/*** SOURCE_BEGIN ***/

/* OSAPI autosar module private functions */

/*ci \brief Checks if the mutex is owned by the current thread.
 *
 *  \param[in] mutex Mutex to check for ownership.
 *
 *  \return TRUE if the mutex is owned by the current thread. Otherwise FALSE.
 */
RTI_PRIVATE FUNC(boolean, SOAD_CODE)
OSAPI_Mutex_is_owned(struct OSAPI_Mutex *mutex)
{
    OSAPI_ThreadId thread_self = OSAPI_Thread_self();

    return (mutex->_base.owned &&
            (mutex->_base.owner == thread_self)) ? TRUE : FALSE;
}

/*ci \brief Ensures that Autosar mutex module is initialized.
 *
 *  \return TRUE if module is initialized with no error or already initialized.
 *          Otherwise FALSE.
 */
RTI_PRIVATE FUNC(boolean, SOAD_CODE)
OSAPI_AutosarMutex_is_initialized(void)
{
    if (OSAPI_AutosarMutex_fv_IsInitialized)
    {
        return TRUE;
    }

    OSAPI_Mutex_fv_CurrentResourceId =
        OSAPI_System_gv_PortProperty->first_resource_id + 1;
    OSAPI_Mutex_initialize(&OSAPI_Mutex_fv_Mutex);
    OSAPI_AutosarMutex_fv_IsInitialized = RTI_TRUE;

    return TRUE;
}

/* OSAPI autosar module public functions */

#ifndef RTI_CERT
FUNC(void, SOAD_CODE)
OSAPI_AutosarMutex_finalize(void)
{
    /* set global variables to their default values */

    OSAPI_Mutex_fv_CurrentResourceId = 0;
    OSAPI_Mutex_finalize(&OSAPI_Mutex_fv_Mutex);
    OSAPI_AutosarMutex_fv_IsInitialized = RTI_FALSE;
}
#endif

FUNC(StatusType, SOAD_CODE)
OSAPI_AutosarMutex_enter_critical_section(ResourceType resource)
{
    StatusType stat;

    switch (OSAPI_System_gv_PortProperty->sync_type)
    {
        case OSAPI_AUTOSAR_SYNCKIND_RESOURCES:
            stat = GetResource(resource);
            break;
        
#if RTIME_AUTOSAR_SPINLOCK_ENABLED
        case OSAPI_AUTOSAR_SYNCKIND_SPINLOCK:
            stat = GetSpinlock(OSAPI_System_gv_PortProperty->spinlock_id);
            break;
#endif /* RTIME_AUTOSAR_SPINLOCK_ENABLED */

        default:
            stat = E_NOT_OK;
            break;
    }
    
    return stat;
}

FUNC(StatusType, SOAD_CODE)
OSAPI_AutosarMutex_leave_critical_section(ResourceType resource)
{
    StatusType stat;

    switch (OSAPI_System_gv_PortProperty->sync_type)
    {
        case OSAPI_AUTOSAR_SYNCKIND_RESOURCES:
            stat = ReleaseResource(resource);
            break;
        
#if RTIME_AUTOSAR_SPINLOCK_ENABLED
        case OSAPI_AUTOSAR_SYNCKIND_SPINLOCK:
            stat = ReleaseSpinlock(OSAPI_System_gv_PortProperty->spinlock_id);
            break;
#endif /* RTIME_AUTOSAR_SPINLOCK_ENABLED */
            
        default:
            stat = E_NOT_OK;
            break;
    }
    
    return stat;
}

/* OSAPI public functions */

#ifndef RTI_CERT
FUNC(RTI_BOOL, SOAD_CODE)
OSAPI_Mutex_delete(P2VAR(struct OSAPI_Mutex, AUTOMATIC, SOAD_APPL_DATA) mutex)
{
    OSAPI_PRECONDITION(mutex == NULL_PTR,return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("mutex",mutex,RTI_TRUE);)

    OSAPI_Mutex_finalize(mutex);

    if (OSAPI_System_gv_PortProperty->sync_type == OSAPI_AUTOSAR_SYNCKIND_RESOURCES)
    {
        OSAPI_Heap_free_struct(mutex);
    }

    return RTI_TRUE;
}
#endif

FUNC(P2VAR(struct OSAPI_Mutex, AUTOMATIC, SOAD_APPL_DATA), SOAD_CODE)
OSAPI_AutosarMutex_initialize_mutex(P2VAR(struct OSAPI_Mutex, AUTOMATIC, SOAD_APPL_DATA) mutex)
{
    StatusType ret_value;

    if (!OSAPI_AutosarMutex_is_initialized())
    {
        OSAPI_LOG_MUTEX_NEW(OSAPI_LOGKIND_ERROR)
        return NULL_PTR;
    }

    if (OSAPI_System_gv_PortProperty->sync_type == OSAPI_AUTOSAR_SYNCKIND_RESOURCES)
    {
        OSAPI_Mutex_initialize(mutex);

        ret_value = OSAPI_AutosarMutex_enter_critical_section(
                        OSAPI_System_gv_PortProperty->first_resource_id);
        if (ret_value != E_OK)
        {
            OSAPI_LOG_MUTEX_NEW(OSAPI_LOGKIND_ERROR)
            goto failure;
        }

        if (OSAPI_Mutex_fv_CurrentResourceId > OSAPI_System_gv_PortProperty->last_resource_id)
        {
            ret_value = OSAPI_AutosarMutex_leave_critical_section(
                            OSAPI_System_gv_PortProperty->first_resource_id);
            IGNORE_RETVAL(ret_value);

            OSAPI_LOG_MUTEX_NEW(OSAPI_LOGKIND_ERROR)
            goto failure;
        }

        mutex->hmutex = OSAPI_Mutex_fv_CurrentResourceId;
        OSAPI_Mutex_fv_CurrentResourceId++;

        ret_value = OSAPI_AutosarMutex_leave_critical_section(
                        OSAPI_System_gv_PortProperty->first_resource_id);
        if (ret_value != E_OK)
        {
            OSAPI_LOG_MUTEX_NEW(OSAPI_LOGKIND_ERROR)
            goto failure;
        }
    }
    else
    {
        mutex = &OSAPI_Mutex_fv_Mutex;
    }

    return mutex;

failure:

    return NULL_PTR;
}

FUNC(P2VAR(struct OSAPI_Mutex, AUTOMATIC, SOAD_APPL_DATA), SOAD_CODE)
OSAPI_Mutex_new(void)
{
    P2VAR(struct OSAPI_Mutex, AUTOMATIC, SOAD_APPL_DATA) mutex = NULL_PTR;

    if (!OSAPI_AutosarMutex_is_initialized())
    {
        OSAPI_LOG_MUTEX_NEW(OSAPI_LOGKIND_ERROR)
        return NULL_PTR;
    }

    if (OSAPI_System_gv_PortProperty->sync_type == OSAPI_AUTOSAR_SYNCKIND_RESOURCES)
    {
        OSAPI_Heap_allocate_struct(&mutex, struct OSAPI_Mutex);

        if (mutex == NULL_PTR)
        {
            OSAPI_LOG_MUTEX_NEW(OSAPI_LOGKIND_ERROR)
            return NULL_PTR;
        }

        mutex = OSAPI_AutosarMutex_initialize_mutex(mutex);
    
        if (mutex == NULL_PTR)
        {
            goto failure;
        }
    }
    else
    {
        mutex = &OSAPI_Mutex_fv_Mutex;
    }

    return mutex;

failure:
#ifndef RTI_CERT
    if (OSAPI_System_gv_PortProperty->sync_type == OSAPI_AUTOSAR_SYNCKIND_RESOURCES)
    {
        OSAPI_Heap_free_struct(mutex);
    }
#endif

    return NULL_PTR;
}

FUNC(RTI_BOOL, SOAD_CODE)
OSAPI_Mutex_take_os(P2VAR(struct OSAPI_Mutex, AUTOMATIC, SOAD_APPL_DATA) mutex)
{
    StatusType ret_value;

    OSAPI_PRECONDITION(mutex == NULL_PTR,return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("mutex",mutex,RTI_TRUE);)

    if (OSAPI_Mutex_is_owned(mutex))
    {
        ++mutex->_base.depth;
        RTI_OS_DSYNC();
        return RTI_TRUE;
    }

    ret_value = OSAPI_AutosarMutex_enter_critical_section(mutex->hmutex);
    if (ret_value == E_OK)
    {
        mutex->_base.owner = OSAPI_Thread_self();
        mutex->_base.owned = RTI_TRUE;
        mutex->_base.depth = 1;
        RTI_OS_DSYNC();

        return RTI_TRUE;
    }

    OSAPI_LOG_MUTEX_TAKE(OSAPI_LOGKIND_ERROR, ret_value)

    return RTI_FALSE;
}

FUNC(RTI_BOOL, SOAD_CODE)
OSAPI_Mutex_give_os(P2VAR(struct OSAPI_Mutex, AUTOMATIC, SOAD_APPL_DATA) mutex)
{
    StatusType ret_value;

    OSAPI_PRECONDITION(mutex == NULL_PTR,return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("mutex",mutex,RTI_TRUE);)

    if (!OSAPI_Mutex_is_owned(mutex))
    {
        return RTI_FALSE;
    }

    --mutex->_base.depth;

    if (mutex->_base.depth > 0)
    {
        RTI_OS_DSYNC();
        return RTI_TRUE;
    }

    mutex->_base.owned = RTI_FALSE;
    RTI_OS_DSYNC();

    ret_value = OSAPI_AutosarMutex_leave_critical_section(mutex->hmutex);
    if (ret_value == E_OK)
    {
        return RTI_TRUE;
    }

    OSAPI_LOG_MUTEX_GIVE(OSAPI_LOGKIND_ERROR, ret_value)

    return RTI_FALSE;
}

#endif /* OSAPI_INCLUDE_AUTOSAR */
