/*
 * FILE: autosarSemaphore.h - Autosar semaphore functionality
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
 * 02Sep2019,fmt File created
 *
 */
/*ce
 * \file
 */

#ifndef autosarSemaphore_h
#define autosarSemaphore_h

#if OSAPI_INCLUDE_AUTOSAR

#ifndef RTI_CERT
/*\ci \brief Finalizes autosar semaphore. This function needs to be called in case
 * OSAPI system is finalized.
 * This module self initializes when the fist semaphore is created.
 */
extern FUNC(void, SOAD_CODE)
OSAPI_AutosarSemaphore_finalize(void);
#endif

#endif /* OSAPI_INCLUDE_AUTOSAR */

#endif  /* autosarSemaphore_h */
