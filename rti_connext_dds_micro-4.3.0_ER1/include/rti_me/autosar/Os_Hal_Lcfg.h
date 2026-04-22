/*
 * FILE: Os_Hal_Lcfg.h - AutoSAR 4.2.2 OS User configuration
 *
 * (c) Copyright, Real-Time Innovations, 2020-2026
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

/* This file is required as it is included by AutoSAR headers when
 * compiling Micro. However, none of the contents are required by Micro.
 */
 
#ifndef OS_HAL_LCFG_H
#define OS_HAL_LCFG_H

#if defined(RTIME_AUTOSAR_MICROSAR)
#if defined(__CPU_TC39XB__)
typedef uint32 Os_Hal_AddressType;
#endif
#else
typedef uint32 Os_Hal_AddressType;
#endif

#endif
