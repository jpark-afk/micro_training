/*
 * FILE: Mutex.h - Generic declarations of generic OSAPI mutex routines
 *
 * Copyright 2018-2021 Real-Time Innovations, Inc.
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
 * 06sep2022,tk MICRO-4151/PR.30884
 * - Corrected misspelling of specific and initialize
 * 21may2020,fmt MICRO-2409/PR.27650 Exclude from cert build finalize() functions
 */

/*ce
 * \file
 * \brief Generic declarations of generic OSAPI mutex routines
 */
#ifndef MUTEX_H
#define MUTEX_H

/*i
 * \brief Function to initialize Mutex base class
 *
 * \param[in]  base Base class to initialize
 */
extern void
OSAPI_Mutex_initialize(struct OSAPI_Mutex *base);

#ifndef RTI_CERT
/*i
 * \brief Function to finalize the mutex base class
 *
 * \param[in] base Base class to finalize
 */
extern void
OSAPI_Mutex_finalize(struct OSAPI_Mutex *base);
#endif /* !RTI_CERT */

/*ci
 * \brief OS specific version of OSAPI_Mutex_take
 */
extern RTI_BOOL
OSAPI_Mutex_take_os(OSAPI_Mutex_T *self);

/*ci
 * \brief OS specific version of OSAPI_Mutex_give
 */
extern RTI_BOOL
OSAPI_Mutex_give_os(OSAPI_Mutex_T *self);

#endif
