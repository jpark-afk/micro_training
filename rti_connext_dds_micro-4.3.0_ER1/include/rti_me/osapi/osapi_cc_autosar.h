/*
 * FILE: osapi_cc_autosar.h - Specific compiler related definitions
 *
 * (c) Copyright, Real-Time Innovations, 2019-2026
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
 * 22jul2021,fmt MICRO-3046/PR.29135
 * - Removed tests for #ifdef uint64 and #ifdef sint64
 * - Removed incorrect type definition typedef signed long long int RTI_UINT64
 * 27aug2019,fmt  Refactored from osapi_cc_stdc.h
 */
/*ce \file
 *   \brief OS API Configuration
 */
#ifndef osapi_cc_autosar_h
#define osapi_cc_autosar_h

#include <Platform_Types.h>
#include <limits.h>

/*i \brief NULL pointer definition for AutoSAR */
#ifndef NULL
#define NULL ((void*)0)
#endif

/* In case gcc extensions are disabled */
#ifndef RTIME_FUNCTION_NAME
#ifdef __func__
#define RTIME_FUNCTION_NAME __func__
#else
#define RTIME_FUNCTION_NAME "undefined"
#endif
#endif

/*i
 */
typedef sint8 RTI_INT8;

/*i
 */
typedef uint8 RTI_UINT8;

/*i
 */
typedef sint16 RTI_INT16;

/*i
 */
typedef uint16 RTI_UINT16;

/*i
 */
typedef sint32 RTI_INT32;

/*i
 */
#ifndef RTI_UINT32_DEFINED
#define RTI_UINT32_DEFINED
typedef uint32 RTI_UINT32;
#endif

#if defined(RTIME_AUTOSAR_WINCORE)
/* 
 * WinCore 4.0.3 targets i86 systems and does not define a 64bit integer
 * type as the PlatformTypes support for that was added in 4.1.2. This 
 * definition leverages the fact that gcc is used to compile WinCore
 */
/*i
 */
typedef long long int RTI_INT64;

/*i
 */
typedef unsigned long long int RTI_UINT64;
#else
/*i
 */
typedef sint64 RTI_INT64;

/*i
 */
typedef uint64 RTI_UINT64;
#endif

/*******************************************************************************
 *                  Define compiler floating point mappings
 ******************************************************************************/
/*i
 */
typedef float32 RTI_FLOAT32;

#if defined(RTIME_AUTOSAR_MICROSAR)
#define RTI_HAVE_FLOAT
#endif

/*i
 */
typedef float64 RTI_DOUBLE64;

#if defined(RTIME_AUTOSAR_MICROSAR)
#define RTI_HAVE_DOUBLE
#endif

/*i
 */
typedef struct RTI_DOUBLE128
{
    char bytes[16];
} RTI_DOUBLE128;

/*******************************************************************************
 *                          Define CPU endian
 ******************************************************************************/

#if !defined(RTI_ENDIAN_LITTLE) && !defined(RTI_ENDIAN_BIG)
#if __CORE_TC16X__ || __CORE_TC162__
/* Infineon tricore is little endian */
#define RTI_ENDIAN_LITTLE 1
#elif defined(VVIRTUALTARGET) || defined(_X86_)
/* x86 is little endian */
#define RTI_ENDIAN_LITTLE 1
#else
#error "Unable to determine endianess."
#endif
#endif /* !defined(RTI_ENDIAN_LITTLE) && !defined(RTI_ENDIAN_BIG) */

/*******************************************************************************
 *                   Define various compiler compile options
 ******************************************************************************/

/* Function attributes */
#if defined(__GNUC__) && (__GNUC__ >= 4)
#define MUST_CHECK_RETURN __attribute__((warn_unused_result))
#define SHOULD_CHECK_RETURN
#define FUNCTION_SHOULD_TYPEDEF(ftdef_) \
typedef SHOULD_CHECK_RETURN ftdef_;
#define FUNCTION_MUST_TYPEDEF(ftdef_) \
typedef MUST_CHECK_RETURN ftdef_;
#else
#define SHOULD_CHECK_RETURN
#define FUNCTION_SHOULD_TYPEDEF(ftdef_) \
typedef ftdef_;
#define FUNCTION_MUST_TYPEDEF(ftdef_) \
typedef ftdef_;
#endif

#endif /* osapi_cc_autosar_h */

