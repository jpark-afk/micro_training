/*
 * FILE: Platform_Types.h - AutoSAR 4.0.3 Basic Platform types
 *
 * (c) Copyright, Real-Time Innovations, 2020-2026
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
 * 1oct2020,fmt MICRO-2586/PR.28158 Remove definitions which are not needed
 */

/* This file is required as it is included by AutoSAR headers when
 * compiling Micro. However, none of the contents are required by Micro.
 */
#ifndef PLATFORM_TYPES_H
#define PLATFORM_TYPES_H

/*ci \brief CPU with 8-bit registers
 */
#define CPU_TYPE_8  8U

/*ci \brief CPU with 16-bit registers
 */
#define CPU_TYPE_16 16U

/*ci \brief CPU with 32-bit registers
 */
#define CPU_TYPE_32 32U

/*ci \brief CPU with 64-bit registers
 */
#define CPU_TYPE_64 64U

/*ci \brief Default is CPU with 32-bit registers
 */
#ifndef CPU_TYPE
#define CPU_TYPE CPU_TYPE_32
#endif

/*ci \brief TRUE per the AutoSAR standard
 */
#define TRUE (0x1)

/*ci \brief FALSE per the AutoSAR standard
 */
#define FALSE (0x0)

/*ci \brief unsigned 8-bit type per AutoSAR standard
 */
typedef unsigned char uint8;

/*ci \brief boolean type per AutoSAR standard
 */
typedef uint8 boolean;

/*ci \brief unsigned 16-bit type per AutoSAR standard
 */
typedef unsigned short uint16;

/*ci \brief unsigned 32-bit type per AutoSAR standard
 */
typedef unsigned int uint32;

/*ci \brief unsigned 64-bit type per AutoSAR standard
 */
typedef unsigned long long uint64;

/*ci \brief signed 8-bit type type per AutoSAR standard
 */
typedef signed char sint8;

/*ci \brief signed 16-bit type per AutoSAR standard
 */
typedef signed short sint16;

/*ci \brief signed 32-bit type per AutoSAR standard
 */
typedef signed int sint32;

/*ci \brief signed 64-bit type per AutoSAR standard
 */
typedef signed long long sint64;

/*ci \brief 32-bit float type per AutoSAR standard
 */
typedef float float32;

/*ci \brief 64-bit double type per AutoSAR standard
 */
typedef double float64;

#endif
