/*
 * FILE: Compiler_Cfg.h - Compiler configuration file
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

#ifndef COMPILER_CFG_H
#define COMPILER_CFG_H

#define OS_CODE
#define OS_APPL_DATA
#define OS_APPL_VAR
#define OS_VAR_NOINIT_FAST
#define OS_VAR_NOINIT
#define TCPIP_CODE
#define TCPIP_APPL_VAR
#define TCPIP_APPL_DATA
#define TCPIP_VAR_ZERO_INIT
#define TCPIP_VAR_CLEARED
#define TCPIP_APPL_CONST
#define AUTOSAR_COMSTACKDATA
#define SOAD_CODE
#define SOAD_APPL_DATA
#define DET_CODE

#endif
