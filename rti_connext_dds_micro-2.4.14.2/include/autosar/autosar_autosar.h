/*
 * FILE: autosar_autosar.h - AutoSAR API definitions
 *
 * (c) Copyright, Real-Time Innovations, 2020-2020
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 */
/*ci
 * \file
 * \defgroup AutoSARModule DB
 *
 * \brief Public DB API
 *
 * \details
 * This file defines the API for AutoSAR.
 */

/*ci \addtogroup AutoSARModule
 *  @{
 */
#ifndef autosar_autosar_h
#define autosar_autosar_h

#ifndef autosar_dll_h
#include "autosar/autosar_dll.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

AutoSARDllExport void
AutoSAR_init(void);


#ifdef __cplusplus
}
#endif


#endif
