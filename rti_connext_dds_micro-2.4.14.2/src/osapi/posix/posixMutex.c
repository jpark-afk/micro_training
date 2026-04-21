/*
 * FILE: posixMutex.c - Mutex functionality
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
 * 20jul2021,tk MICRO-3127 Addressed MISRA-C 2021 and SEI CERT C warnings:
 * - Added suppression of cert_pos54_c_violation in OSAPI_Mutex_new
 * 05jan2021,tk
 *   MICRO-2777/PR#28505
 *     - According to the POSIX standard, pthread_mutex_lock and
 *       pthread_mutex_unlock shall not return EINTR. Because of this, the
 *       functions OSAPI_Mutex_give_os() and OSAPI_Mutex_take_os() was
 *       simplified and the while loop testing for EINTR was removed.
 *   MICRO-2776/PR#28504
 *     - Explicitly ignore the return code from pthread_mutexattr_destroy()
 *       for RTI_CERT in OSAPI_Mutex_new() since it is only used for logging
 *       purposes.
 * 09mar2012,tk Written
 */
/*ce
 * \file
 * \brief POSIX implementation of OSAPI mutex routines
 */
#include "osapi/osapi_config.h"

#if OSAPI_INCLUDE_POSIX

#define RTI_INVALID_TID (-1)

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

struct OSAPI_Mutex
{
    struct OSAPI_MutexBase _base;
    pthread_mutex_t *mutex;

#if ENABLED_FACE_COMPLIANCE_NON_GENERAL
    RTI_INT32 depth;
    RTI_BOOL owned;
    pthread_t owner;
#endif
};
#endif /* OSAPI_INCLUDE_POSIX */

/*** SOURCE_BEGIN ***/

#if OSAPI_INCLUDE_POSIX

#ifndef RTI_CERT
RTI_BOOL
OSAPI_Mutex_delete(OSAPI_Mutex_T *self)
{
    int rc;
    PRECOND_ARG(self)

    OSAPI_PRECONDITION(self == NULL,return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    OSAPI_Mutex_finalize(self);

    if (self->mutex)
    {
        rc = pthread_mutex_destroy(self->mutex);
        if (rc != 0)
        {
            OSAPI_LOG_MUTEX_DELETE(OSAPI_LOGKIND_ERROR,rc)
            return RTI_FALSE;
        }

        OSAPI_Heap_free_struct(self->mutex);
    }

    OSAPI_Heap_free_struct(self);

    return RTI_TRUE;
}
#endif

#if defined(RTI_LINUX)
#define PTHREAD_MUTEX_RECURSIVE PTHREAD_MUTEX_RECURSIVE_NP
#endif

OSAPI_Mutex_T*
OSAPI_Mutex_new(void)
{
    pthread_mutexattr_t attr;
    int rc;
    struct OSAPI_Mutex *mutex = NULL;

    OSAPI_Heap_allocate_struct(&mutex, struct OSAPI_Mutex);

    if (mutex == NULL)
    {
        OSAPI_LOG_MUTEX_NEW(OSAPI_LOGKIND_ERROR)
        return NULL;
    }

    OSAPI_Mutex_initialize(mutex);

    mutex->mutex = NULL;

#if ENABLED_FACE_COMPLIANCE_NON_GENERAL
    /* Synchronize write of self->owned */
    mutex->_base.owned = RTI_FALSE;
    OSAPI_Memory_write_barrier();

    /* Don't initialize owner, is of unknown type */
#endif

    if (!pthread_mutexattr_init(&attr) 
#if !ENABLED_FACE_COMPLIANCE_NON_GENERAL
        && !pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE)
#endif
        )
    {
        OSAPI_Heap_allocate_struct(&mutex->mutex, pthread_mutex_t);
    }

    if (mutex->mutex)
    {
        rc = pthread_mutex_init(mutex->mutex, &attr);
        if ( rc != 0)
        {
            OSAPI_LOG_MUTEX_INIT(OSAPI_LOGKIND_ERROR,rc)
#ifndef RTI_CERT
            (void)OSAPI_Mutex_delete(mutex);
#endif
            return NULL;
        }
    }
    else
    {
        OSAPI_LOG_MUTEX_INIT(OSAPI_LOGKIND_ERROR,errno)
        return NULL;
    }

    /* rc is only used for logging purposes
     */
    /* coverity[cert_pos54_c_violation] */
    rc = pthread_mutexattr_destroy(&attr);
#if OSAPI_ENABLE_LOG
    if (rc != 0)
    {
        OSAPI_LOG_MUTEX_INIT(OSAPI_LOGKIND_WARNING,rc)
    }
#else
    /* rc is not accessed, so it is safe to ignore the return value */
    IGNORE_RETVAL(rc);
#endif

    return (OSAPI_Mutex_T*)mutex;
}

RTI_BOOL
OSAPI_Mutex_take_os(OSAPI_Mutex_T *self)
{
    int rc = 0;

#if ENABLED_FACE_COMPLIANCE_NON_GENERAL
    pthread_t owner;

    /* Synchronize load of owner before owned, to ensure pthread equality check 
     * uses consistent pairing.
     */
    owner = self->_base.owner;
    OSAPI_Memory_read_barrier();

    if (self->_base.owned && pthread_equal(owner, pthread_self()))
    {
        ++self->_base.depth;
        return RTI_TRUE;
    }
#endif

    rc = pthread_mutex_lock(self->mutex);
    if (rc != 0)
    {
        OSAPI_LOG_MUTEX_TAKE(OSAPI_LOGKIND_ERROR,rc)
        return RTI_FALSE;
    }

#if ENABLED_FACE_COMPLIANCE_NON_GENERAL
    /* Set owner before owned flag to ensure any thread taking or giving 
     * this lock will see the correct ID for an owned lock.
     */
    self->_base.owner = pthread_self();
    OSAPI_Memory_write_barrier();
    self->_base.owned = RTI_TRUE;
    self->_base.depth = 1;
#endif

    return RTI_TRUE;
}

RTI_BOOL
OSAPI_Mutex_give_os(OSAPI_Mutex_T *self)
{
    int rc;

#if ENABLED_FACE_COMPLIANCE_NON_GENERAL
    pthread_t owner;
#endif    

    OSAPI_PRECONDITION(self == NULL,return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

#if ENABLED_FACE_COMPLIANCE_NON_GENERAL
    /* Synchronize load of owner before owned, to ensure pthread equality check 
     * uses consistent pairing.
     */
    owner = self->_base.owner;
    OSAPI_Memory_read_barrier();

    if (!self->_base.owned || !pthread_equal(owner, pthread_self()))
    {
        return RTI_FALSE;
    }

    --self->_base.depth;

    if (self->_base.depth > 0)
    {
        return RTI_TRUE;
    }

    self->_base.owned = RTI_FALSE;
#endif

    rc = pthread_mutex_unlock(self->mutex);
    if (rc != 0)
    {
        OSAPI_LOG_MUTEX_GIVE(OSAPI_LOGKIND_ERROR,rc)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}
#endif /* OSAPI_INCLUDE_POSIX */
