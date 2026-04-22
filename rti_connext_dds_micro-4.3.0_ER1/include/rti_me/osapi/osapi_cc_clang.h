/*
 * FILE: osapi_cc_clang.h - Specific compiler related definitions
 *
 * Copyright 2012-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*ce \file
 *   \brief OS API Configuration
 */
#ifndef osapi_cc_clang_h
#define osapi_cc_clang_h

#if !__clang__
#error "This is not a clang compiler"
#endif

/* Determine the ptr size unless it has been set by this flag */
#ifndef RTI_64BIT
#if __x86_64 || __aarch64__ || __arm64__
#define RTI_64BIT 1
#else
#error Unknown clang architecture
#endif
#endif

#ifdef __cplusplus
#ifndef NULL
#define NULL (0)
#endif
#else
#ifndef NULL
#define NULL (void*)(0)
#endif
#endif

/*******************************************************************************
 *                   Define/Check STDC compiler integer mappings
 ******************************************************************************/
#if __INT8_MAX__ == 127
/*i
 */
typedef signed char RTI_INT8;

/*i
 */
typedef unsigned char RTI_UINT8;
#else
#error "SCHAR_MAX is unknown, could not determine RTI_INT8/UINT8."
#endif /* SCHAR_MAX */

#if __INT16_MAX__ == 32767
/*i
 */
typedef signed short RTI_INT16;

/*i
 */
typedef unsigned short RTI_UINT16;

#ifndef USHRT_MAX
#define USHRT_MAX 65535
#elif USHRT_MAX != 65535
#error "Inconsistent defintion of USHRT_MAX"
#endif

#ifndef SHRT_MAX
#define SHRT_MAX 32767
#elif SHRT_MAX != 32767
#error "Inconsistent defintion of SHRT_MAX"
#endif /* SHRT_MAX */

#else
#error "SHRT_MAX is unknown, could not determine RTI_INT16/UINT16."
#endif /* SHRT_MAX */

#if __INT32_MAX__ == 2147483647
/*i
 */
typedef signed int RTI_INT32;

/*i
 */
typedef unsigned int RTI_UINT32;

#ifndef UINT_MAX
#define UINT_MAX 4294967295U
#elif UINT_MAX != 4294967295U
#error "Inconsistent defintion of UINT_MAX"
#endif

#ifndef INT_MAX
#define INT_MAX 2147483647
#elif INT_MAX != 2147483647
#error "Inconsistent defintion of INT_MAX"
#endif

#else
#error "INT_MAX is unknown, could not determine  RTI_INT32/UINT32."
#endif /* INT_MAX */

#if __INT64_MAX__ == 9223372036854775807LL
/*i
 */
 typedef signed long long int RTI_INT64;

#ifndef LLONG_MAX
#define LLONG_MAX 9223372036854775807LL
#elif LLONG_MAX != 9223372036854775807LL
#error "Inconsistent defintion of LLONG_MAX"
#endif

#ifndef LLONG_MIN
#define LLONG_MIN (-LLONG_MAX - 1)
#elif LLONG_MIN != (-LLONG_MAX - 1)
#error "Inconsistent defintion of LLONG_MIN"
#endif

 /*i
  */
 typedef unsigned long long int RTI_UINT64;
#else /* no size defined */
#error "Could not determine RTI_INT64/RTI_UINT64 type"
#endif

/*******************************************************************************
 *                  Define compiler floating point mappings
 ******************************************************************************/
#if (__SIZEOF_FLOAT__ == 4)
typedef float RTI_FLOAT32;
#define RTI_HAVE_FLOAT
#else
#error "__SIZEOF_FLOAT__ is unknown (GCC), could not determine RTI_FLOAT32."
#endif

#if (__SIZEOF_DOUBLE__ == 8)
typedef double RTI_DOUBLE64;
#define RTI_HAVE_DOUBLE
#else
#error "__SIZEOF_DOUBLE__ is unknown (GCC), could not determine RTI_DOUBLE64."
#endif

#if __SIZEOF_LONG_DOUBLE__ == 16
#if 0
typedef long double RTI_DOUBLE128;
#define RTI_HAVE_LONG_DOUBLE
#else
typedef struct RTI_DOUBLE128
{
    char bytes[16];
} RTI_DOUBLE128;
#endif
#else
typedef struct RTI_DOUBLE128
{
    char bytes[16];
} RTI_DOUBLE128;
#endif

/*******************************************************************************
 *                          Define CPU endian
 ******************************************************************************/
#if !defined(__BYTE_ORDER__) && !defined(__ORDER_BIG_ENDIAN__) && \
    !defined(__ORDER_BIG_ENDIAN__)
#error "Unable to determine endianess."
#endif

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define RTI_ENDIAN_LITTLE 1
#undef  RTI_ENDIAN_BIG
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define RTI_ENDIAN_BIG 1
#undef  RTI_ENDIAN_LITTLE
#else
#error "Unable to determine endianess."
#endif

/*******************************************************************************
 *                   Define various compiler compile options
 ******************************************************************************/

/* Function attributes */
#define MUST_CHECK_RETURN __attribute__((warn_unused_result))
#define SHOULD_CHECK_RETURN
#define FUNCTION_SHOULD_TYPEDEF(ftdef_) \
typedef SHOULD_CHECK_RETURN ftdef_;
#define FUNCTION_MUST_TYPEDEF(ftdef_) \
typedef MUST_CHECK_RETURN ftdef_;

/* In case it is defined elsewhere */
#ifndef RTIME_FUNCTION_NAME
#if defined (__STDC__)
/* func was introduced in C99 */
#if __STDC_VERSION__ >= 199901L
#define RTIME_FUNCTION_NAME __func__
#else
/* Fallback */
#define RTIME_FUNCTION_NAME __FUNCTION__
#endif /* __STDC_VERSION__ */
#endif /* __STDC__ */
#endif /* RTIME_FUNCTION_NAME */

#define RTIME_PSL_API __attribute__((weak))

#ifndef INT_MIN
#define INT_MIN (-2147483647 -1)
#endif

#define OSAPI_CC_EXPORT_LIB_FUNCTION 
#define OSAPI_CC_EXPORT_LIB_VARIABLE 
#define OSAPI_CC_IMPORT_LIB_FUNCTION 
#define OSAPI_CC_IMPORT_LIB_VARIABLE 

#include "osapi_atomic.h"

#endif /* osapi_cc_clang_h */

