/*
 * FILE: osapi_cc_msvsc.h - MSVSC compiler related definitions
 *
 * Copyright 2024-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*ce \file
 *   \brief OS API Configuration
 */
#ifndef osapi_cc_msvsc_h
#define osapi_cc_msvsc_h

#if !defined(_MSC_VER)
#error "Not MSVSC"
#endif

#if RTI_PIL
extern void
qsort(void *base, unsigned long nitems, unsigned long size, int (*compar)(const void *, const void*));
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

typedef signed char   RTI_INT8;
typedef unsigned char RTI_UINT8;
typedef signed short  RTI_INT16;
typedef unsigned short RTI_UINT16;
typedef signed int RTI_INT32;
typedef unsigned int RTI_UINT32;
typedef signed long long  RTI_INT64;
typedef unsigned long long RTI_UINT64;

#define RTI_HAVE_FLOAT
typedef float RTI_FLOAT32;

#define RTI_HAVE_DOUBLE
typedef double RTI_DOUBLE64;

#if OSAPI_ENABLE_LONG_DOUBLE
#error "Native long double is not supported on Win32."
#else
typedef struct RTI_DOUBLE128
{
    char bytes[16];
} RTI_DOUBLE128;
#endif

/*******************************************************************************
 *                          Define CPU endian
 ******************************************************************************/
#if !defined(RTI_ENDIAN_LITTLE) && !defined(RTI_ENDIAN_BIG)
#if defined(_M_IX86) || defined(_M_X64) || defined(__ARM_ARCH)
#define RTI_ENDIAN_LITTLE 1
#undef  RTI_ENDIAN_BIG
#else /* _M_IX86 || _M_X64 */
#error "Unable to determine MSC byte-order"
#endif /* _M_IX86 || _M_X64 */
#endif /* !RTI_ENDIAN_LITTLE && !RTI_ENDIAN_BIG */

/* Try to determine the ptr size unless it has been set by this flag */
#ifndef RTI_64BIT
#if _WIN64
#define RTI_64BIT 1
#endif /* _WIN64 */
#endif /* RTI_64BIT */

/*******************************************************************************
 *                   Define various compiler compile options
 ******************************************************************************/

/* Function attributes */
#if (_MSC_VER >= 1900) && !RTI_PIL
#define MUST_CHECK_RETURN _Check_return_
#elif !RTI_PIL /* _MSC_VER >= 1900) || RTI_PIL */
#include <CodeAnalysis\SourceAnnotations.h>
#define MUST_CHECK_RETURN [returnvalue:SA_Post(MustCheck=SA_Yes)]
#else /* _MSC_VER >= 1900) || RTI_PIL */
#define MUST_CHECK_RETURN
#endif

#define SHOULD_CHECK_RETURN
#define FUNCTION_SHOULD_TYPEDEF(ftdef_) \
typedef ftdef_;
#define FUNCTION_MUST_TYPEDEF(ftdef_) \
typedef ftdef_;

/* In case it is defined elsewhere */
#ifndef RTIME_FUNCTION_NAME
/* MS VC/VC++ is not 100% C99 compliant, some versions support __func__ */
#if (_MSC_VER >= 1900)
#define  RTIME_FUNCTION_NAME __func__
#else /* _MSC_VER >= 1900 */
#define RTIME_FUNCTION_NAME __FUNCTION__
#endif /* _MSC_VER >= 1900 */
#endif /* RTIME_FUNCTION_NAME */

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

#ifndef INT_MIN
#define INT_MIN (-2147483647 -1)
#endif

#ifndef USHRT_MAX
#define USHRT_MAX 65535
#elif USHRT_MAX != 65535
#error "Inconsistent defintion of USHRT_MAX"
#endif

#ifndef SHRT_MAX
#define SHRT_MAX 32767
#elif SHRT_MAX != 32767
#error "Inconsistent defintion of SHRT_MAX"
#endif

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

#define OSAPI_CC_EXPORT_LIB_FUNCTION __declspec( dllexport )
#define OSAPI_CC_EXPORT_LIB_VARIABLE __declspec( dllexport )
#define OSAPI_CC_IMPORT_LIB_FUNCTION
#define OSAPI_CC_IMPORT_LIB_VARIABLE __declspec( dllimport )

#include "osapi_atomic.h"

#endif /* osapi_cc_msvsc_h */
