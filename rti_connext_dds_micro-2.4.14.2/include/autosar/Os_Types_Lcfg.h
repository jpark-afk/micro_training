/*
 * FILE: Os_Types_Lcfg.h - AutoSAR 4.0.3 OS User configuration
 *
 * (c) Copyright, Real-Time Innovations, 2022-2022
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

#ifndef OS_TYPES_LCFG_H
#define OS_TYPES_LCFG_H

# include "Std_Types.h"

#define join(a, b) a ## b
#define GLOBENUM(x) \
typedef enum        \
{                   \
  join(dummy, x)    \
} x;

GLOBENUM(TaskType)
GLOBENUM(ISRType)
GLOBENUM(Os_NonTrustedFunctionIndexType)
GLOBENUM(SpinlockIdType)
GLOBENUM(Os_BarrierIdType)
GLOBENUM(CounterType)
GLOBENUM(AlarmType)
GLOBENUM(ResourceType)
GLOBENUM(ScheduleTableType)
GLOBENUM(ApplicationType)
GLOBENUM(Os_PeripheralIdType)
GLOBENUM(TrustedFunctionIndexType)
GLOBENUM(Os_FastTrustedFunctionIndexType)

#undef GLOBENUM

#endif
