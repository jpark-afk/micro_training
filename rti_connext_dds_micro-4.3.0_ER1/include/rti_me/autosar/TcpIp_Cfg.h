/*
 * FILE: TcpIp_Cfg.h - AutoSAR 4.0.3 TCP/IP configuration
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

#if !defined(TCPIP_CFG_H)
#define TCPIP_CFG_H

#include "TcpIp_CfgTypes.h"

#ifdef RTIME_AUTOSAR_MICROSAR
#if defined(__CPU_TC39XB__)
typedef uint8  TcpIp_SocketIdType;
#endif
#endif

#endif
