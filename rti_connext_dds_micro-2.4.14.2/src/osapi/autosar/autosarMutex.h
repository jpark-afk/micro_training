/*
 * FILE: autosarMutex.h - Autosar mutex functionality
 *
 * Copyright 2012-2021 Real-Time Innovations, Inc.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission. Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 21may2020,fmt MICRO-2409/PR.27650 Exclude from cert build finalize() functions
 * 21may2020,fmt MICRO-2405/PR.27629 Simplify Autosar initialize() functions
 * 17Jun2019,fmt File created
 *
 */
/*ce
 * \file
 */

#ifndef autosarMutex_h
#define autosarMutex_h

#if OSAPI_INCLUDE_AUTOSAR

#include "osapi/osapi_os_autosar.h"

/*ci \brief Mutex implementation for Autosar
 */
struct OSAPI_Mutex
{
    struct OSAPI_MutexBase _base;
    ResourceType hmutex;
};

#ifndef RTI_CERT
/*\ci \brief Finalizes autosar mutex. This function needs to be called in case
 * OSAPI system is finalizes.
 * This module self initializes when the fist semaphore is created.
 */
extern FUNC(void, SOAD_CODE)
OSAPI_AutosarMutex_finalize(void);
#endif

/*  \brief Enters a critical section using the method configured in port properties
 *
 *  \param resource Resource id used to enter the critical section
 *
 *  \return E_OK if no error, error code in case of error
 */
extern FUNC(StatusType, SOAD_CODE)
OSAPI_AutosarMutex_enter_critical_section(ResourceType resource);

/*  \brief Leaves a critical section using the method configured in port properties
 *
 *  \param resource Resource id used to leave the critical section
 *
 *  \return E_OK if no error, error code in case of error
 */
extern FUNC(StatusType, SOAD_CODE)
OSAPI_AutosarMutex_leave_critical_section(ResourceType resource);

/*  \brief Initialize a preallocated memory structure OR return internal mutex
 *
 *  \details
 *
 *  This function initializes a mutex structure and returns an initialized
 *  mutex on success and NULL_PTR on failure. However, the returned mutex may
 *  not be the same mutex as passed in since, depending on the configuration,
 *  an internal mutex structure may be used instead. The memory returned should
 *  not be freed unless the caller has allocated the memory passed to this
 *  function and it needs to be freed _and_ checks that the return value
 *  is the same as the parameter passed in. However, in the autosar port
 *  this is not the case, the memory passed is is never freed.
 *
 *  \param mutex Mutex memory structure to initialize
 *
 *  \return A valid mutex on success, NULL_PTR on failure.
 */
extern FUNC(P2VAR(struct OSAPI_Mutex, AUTOMATIC, SOAD_APPL_DATA), SOAD_CODE)
OSAPI_AutosarMutex_initialize_mutex(P2VAR(struct OSAPI_Mutex, AUTOMATIC, SOAD_APPL_DATA) mutex);

#endif /* OSAPI_INCLUDE_AUTOSAR */

#endif  /* autosarMutex_h */
