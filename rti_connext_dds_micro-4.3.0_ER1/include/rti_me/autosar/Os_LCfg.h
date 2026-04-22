/*
 * FILE: OS_Lcfg.h - AutoSAR 4.2.2 OS User configuration
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
 
#ifndef OS_LCFG_H
#define OS_LCFG_H

#include "Os_Types.h"

#ifdef RTIME_AUTOSAR_MICROSAR
# include "Os_Hal_Lcfg.h"
#endif
typedef enum
{
    DUMMY
} CoreIdType;

#endif
