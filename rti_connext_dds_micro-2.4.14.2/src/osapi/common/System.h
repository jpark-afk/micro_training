/*
 * FILE: System.h - Log functionality
 *
 * (c) Copyright 2012-2015 Real-Time Innovations
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
 * - Only include thread semaphore functionality when
 *   when OSAPI_THREAD_SEMAPHORE_ENABLED is TRUE.
 * 14sep2021,tk MICRO-3152/PR.29564
 * - Removed OSAPI_System_add_thread_semaphore and
 *   OSAPI_System_delete_thread_semaphore declarations. These are now
 *   in osapi_system.h to allow other modules to add semaphores without
 *   creating threads.
 * 10jan2021,tk MICRO-2807 / PR.27549  Removed unused functions for CERT
 * 20oct2020,tk MICRO-2623/PR#28237 Replaced tabs with spaces
 * 20feb2014,eh MICRO-813/PR#9172 Fix Lint warnings
 * 20aug2012,tk Written
 */
/*ce
 * \file
 */
#ifndef System_h
#define System_h

#include "osapi/osapi_thread.h"
#include "osapi/osapi_types.h"
#include "osapi/osapi_system.h"
#include "osapi/osapi_log.h"
#include "osapi/osapi_string.h"
#include "osapi/osapi_semaphore.h"

/*ci \brief Generic description of a system
 */
struct OSAPI_System
{
    /*ci \brief Flag to indicate if the system has been initialized
     */
    RTI_BOOL is_initialized;
    
    /*ci \brief The properties the system was initialized with
     */
    struct OSAPI_SystemProperty property;
    
    /*ci \brief The user defined system interface
     */
    struct OSAPI_SystemI u_intf;
    
    /*ci \brief The native system interface
     */
    struct OSAPI_SystemI n_intf;
    
    /*ci \brief The system interface ins use, u_intf + n_intf
     */
    struct OSAPI_SystemI r_intf;
    
    /*ci \brief User installed system listener
     */
    struct OSAPI_SystemListener listener;

    /*ci \brief Current time
     */    
    OSAPI_NtpTime current_time;
    
#if OSAPI_THREAD_SEMAPHORE_ENABLED
    /*ci \brief Mutex to protect thread semaphores
     */
    OSAPI_Mutex_T *thread_mutex;
    
    /*ci \brief Array of thread semaphores to support blocking threads
     */
    OSAPI_Semaphore_T *thread_semaphore[OSAPI_SYSTEM_MAX_THREAD_SEMAPHORE];
    
    /*ci \brief The number of thread semaphores in use
     */
    RTI_INT32 thread_sem_count;
    
    /*ci \brief The highest index for a thread semaphore in use
     */
    RTI_INT32 thread_sem_last;
#endif
};

#if OSAPI_THREAD_SEMAPHORE_ENABLED
#define OSAPI_THREAD_SEMAPHORE_INITIALIZER \
        ,NULL,\
        {0},\
        0,\
        0
#else
#define OSAPI_THREAD_SEMAPHORE_INITIALIZER
#endif
/*ci \brief Initialize for the OSAPI_SystemProperty
 */
#define OSAPI_System_INITIALIZER \
{ \
    RTI_FALSE, \
    OSAPI_SystemProperty_INITIALIZER, \
    OSAPI_SystemI_INITIALIZER,\
    OSAPI_SystemI_INITIALIZER,\
    OSAPI_SystemI_INITIALIZER,\
    OSAPI_SystemListener_INITIALIZER,\
    OSAPI_NTP_TIME_MAX\
    OSAPI_THREAD_SEMAPHORE_INITIALIZER \
}

#ifndef RTI_CERT
extern RTI_UINT32 OSAPI_System_gv_Size;
#endif /* !RTI_CERT */

#endif /* System_h */
