/*
 * FILE: autosarHeap.h - Autosar heap module public functions.
 *
 * Copyright 2012-2026 Real-Time Innovations, Inc.
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

#ifndef autosarHeap_h
#define autosarHeap_h

#include "rti_me_psl.h"

#ifndef RTI_CERT
/*\ci \brief Finalizes autosar heap. This function needs to be called in case
 * OSAPI system is finalized.
 * This module self initializes when the fist semaphore is created.
 */
extern FUNC(void, SOAD_CODE)
OSAPI_AutosarHeap_finalize(void);
#endif

#endif  /* autosarMutex_h */
