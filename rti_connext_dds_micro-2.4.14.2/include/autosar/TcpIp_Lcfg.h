/*
 * FILE: TcpIp_LCfg.h - Minimum needed AutoSAR 4.2.2 TCP/IP configuration
 *
 * (c) Copyright, Real-Time Innovations, 2020-2022
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

#if !defined(TCPIP_LCFG_H)
#define TCPIP_LCFG_H

#include "Std_Types.h"
#include "Compiler.h"

#ifdef RTIME_AUTOSAR_MICROSAR
typedef uint8_least TcpIp_SocketOwnerConfigIterType;
typedef uint8   TcpIp_OsApplicationType;
#endif

#endif
