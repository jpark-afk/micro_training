/*
 * FILE: posixSystem.h - POSIX system functionality
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
 * 09nov2021,tk MICRO-3330/PR.29900
 * - Added constant OSAPI_SEMAPHORE_TIMEOUT_EXTERNAL -2
 * - Added documentation for ticks
 * 10jan2021,tk MICRO-2807 / PR.27549  Removed unused functions for CERT
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

#ifndef posixSystem_h
#define posixSystem_h

/*ci \brief Semaphore structure for POSIX
 */
struct OSAPI_Semaphore
{
    /*ci \brief Semaphores are linked, link to previous or NULL
     */
    struct OSAPI_Semaphore *prev;

    /*ci \brief Next semaphore with a timeout
     */
    struct OSAPI_Semaphore *next;

    /*ci \brief Control mutex for semaphore used with pthread_conde
     */
    pthread_mutex_t sem_mutex;

    /*ci \brief Signal to implement the semaphore
     */
    pthread_cond_t sem_given;

    /*ci \brief Only binary semaphores are supported
     */
    RTI_INT32 sem_count;

    /*ci \brief TRUE if this semaphore timed out when unblocked
     *   or deleted
     */
    RTI_BOOL timed_out;

    /*ci \brief The number of ticks before the semaphore times out
     *   \details
     *   There are two special values:
     *   -1 - Timeout is INFINITE (OSAPI_SEMAPHORE_TIMEOUT_INFINITE)
     *   -2 - Timeout is triggered by an external clock
     *        (OSAPI_SEMAPHORE_TIMEOUT_EXTERNAL)
     *
     *   Thus, ticks must never be decremented unless it is > 0.
     */
    RTI_INT32 ticks;

    /*ci \brief Whether the semaphore has been deleted or not
     */
    RTI_BOOL remove;
};

/*i \brief The semaphore is timed externally
 *
 * \details
 * The timeout is determined by logic outside of
 * the semaphore implementation. That is, the
 * semaphore is not to be updated as a regular
 * timed semaphore. This is used by the POSIX
 * implementation when a timed semaphore is
 * given from a signal to avoid a signal unsafe
 * mutex lock from being taken. -2 is chosen as
 * OSAPI_SEMAPHORE_TIMEOUT_INFINITE is -1.
 *
 */
#define OSAPI_SEMAPHORE_TIMEOUT_EXTERNAL -2

MUST_CHECK_RETURN RTI_BOOL
OSAPI_SystemPosix_lock(void);

MUST_CHECK_RETURN RTI_BOOL
OSAPI_SystemPosix_unlock(void);

#ifndef RTI_CERT
void
OSAPI_SystemPosix_remove_timed_sem(OSAPI_Semaphore_T *a_sem);
#endif

void
OSAPI_SystemPosix_add_timed_sem(OSAPI_Semaphore_T *a_sem);

RTI_BOOL
OSAPI_SystemPosix_semaphore_timeout(OSAPI_Semaphore_T *self,RTI_BOOL timed_out);

extern RTI_INT32
OSAPI_PosixSystem_get_timer_resolution(void);

#endif
