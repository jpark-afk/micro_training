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

#ifndef OS_CFG_H
#define OS_CFG_H

#include "Std_Types.h"
#include "Os_Types.h"

#ifdef RTIME_AUTOSAR_OSEKCORE
/* Default to EB type of uint32 */
#ifndef RTIME_AUTOSAR_EVENTMASK_TYPE
#define RTIME_AUTOSAR_EVENTMASK_TYPE uint32
#endif
typedef VAR(RTIME_AUTOSAR_EVENTMASK_TYPE, TYPEDEF) EventMaskType;
#endif

#endif /* OS_CFG_H */
