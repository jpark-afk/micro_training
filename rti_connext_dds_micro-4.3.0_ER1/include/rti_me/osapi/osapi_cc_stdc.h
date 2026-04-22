/*
 * FILE: osapi_cc_stdc.h - Specific compiler related definitions
 *
 * Copyright (c) 2012-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 28jul2016,tk  Refactored from osapi_config.h
 */
/*ce \file
 *   \brief OS API Configuration
 */
#ifndef osapi_cc_stdc_h
#define osapi_cc_stdc_h

/* NOTE: We (RTI) build for Windows using a cygwin cpp.exe to generate
 * the dependencies. In addition, MSC only defines __STDC__ when using the /Za
 * option. Thus, check for more than just __STDC__ as we test MSC
 */
#if !defined(__STDC__) && !defined(_MSC_VER) && !defined(RTI_WIN32)
#error "osapi_cc_stdc.h used used with a non-standard C compiler"
#endif

#include "rti_me_psl/ospsl/ospsl_os.h"

/*******************************************************************************
 * Define types for for MS VS. The supported types can be found here:
 *
 * https://msdn.microsoft.com/en-us/library/s3f49ktz.aspx
 ******************************************************************************/
#if defined(_MSC_VER) || defined(RTI_WIN32)

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

#else /* checks when __STDC__ is defined */

/*******************************************************************************
 *                   Define/Check STDC compiler integer mappings
 ******************************************************************************/
#if SCHAR_MAX == 127
/*i
 */
typedef signed char RTI_INT8;

/*i
 */
typedef unsigned char RTI_UINT8;
#else
#error "SCHAR_MAX is unknown, could not determine RTI_INT8/UINT8."
#endif /* SCHAR_MAX */

#if SHRT_MAX == 32767
/*i
 */
typedef signed short RTI_INT16;

/*i
 */
typedef unsigned short RTI_UINT16;
#else
#error "SHRT_MAX is unknown, could not determine RTI_INT16/UINT16."
#endif /* SHRT_MAX */

#if INT_MAX == 2147483647
/*i
 */
typedef signed int RTI_INT32;

/*i
 */
typedef unsigned int RTI_UINT32;
#else
#error "INT_MAX is unknown, could not determine  RTI_INT32/UINT32."
#endif /* INT_MAX */

#ifdef LLONG_MAX
#if LLONG_MAX == 9223372036854775807LL
/*i
 */
 typedef signed long long int RTI_INT64;

 /*i
  */
 typedef unsigned long long int RTI_UINT64;
#else
 #error "LLONG_MAX is unknown, could not determine  RTI_INT64/UINT64."
#endif /* LLONG_MAX has unknown size */
#elif defined(__LONG_LONG_MAX__)
#if __LONG_LONG_MAX__ == 9223372036854775807LL
/*i
 */
 typedef signed long long int RTI_INT64;

 /*i
  */
 typedef unsigned long long int RTI_UINT64;
#else /* __LONG_LONG_MAX__ has unknown size */
#error "__LONG_LONG_MAX__ is unknown, could not determine RTI_INT64/UINT64."
#endif
#else /* no size defined */
#error "Could not determine RTI_INT64/RTI_UINT64 type"
#endif

/*******************************************************************************
 *                  Define compiler floating point mappings
 ******************************************************************************/
#if (__SIZEOF_FLOAT__ == 4) || (__FLT_MAX_EXP__ == 128) || (FLT_MAX_EXP == 128) || defined(__VOS__)
typedef float RTI_FLOAT32;
#define RTI_HAVE_FLOAT
#else
#error "__SIZEOF_FLOAT__ is unknown (GCC), could not determine RTI_FLOAT32."
#endif

#if (__SIZEOF_DOUBLE__ == 8) || (__DBL_MAX_EXP__ == 1024) || (DBL_MAX_EXP == 1024) || defined(__VOS__)
typedef double RTI_DOUBLE64;
#define RTI_HAVE_DOUBLE
#elif (__SIZEOF_LONG_LONG__ == 8)
/* Some compilers define double as 4 bytes and long long as 8 bytes */
typedef long long RTI_DOUBLE64;
#else
#error "__SIZEOF_DOUBLE__ is unknown (GCC), could not determine RTI_DOUBLE64."
#endif

#if OSAPI_ENABLE_LONG_DOUBLE
#if __GNUC__ > 4 || (__GNUC__ == 4 && (__GNUC_MINOR__ > 2))
typedef __float128 RTI_DOUBLE128;
#define RTI_HAVE_LONG_DOUBLE
#else
#error "Could not determine if long double is supported."
#endif
#else
typedef struct RTI_DOUBLE128
{
    char bytes[16];
} RTI_DOUBLE128;
#endif

#endif /* __STDC__ */

/*******************************************************************************
 *                          Define CPU endian
 ******************************************************************************/
#if !defined(RTI_ENDIAN_LITTLE) && !defined(RTI_ENDIAN_BIG) && defined(__VOS__)
#define RTI_ENDIAN_BIG
#endif

#if !defined(RTI_ENDIAN_LITTLE) && !defined(RTI_ENDIAN_BIG)
#ifdef __GNUC__
#if defined(__BYTE_ORDER__) && ((__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) || \
    defined(__LITTLE_ENDIAN__) || (__BYTE_ORDER == __LITTLE_ENDIAN))
#define RTI_ENDIAN_LITTLE 1
#undef  RTI_ENDIAN_BIG
#elif defined(__BYTE_ORDER__) && ((__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) || \
    defined(__BIG_ENDIAN__) || (__BYTE_ORDER == __BIG_ENDIAN))
#define RTI_ENDIAN_BIG    1
#undef  RTI_ENDIAN_LITTLE
#elif i386 || i686 || __i686 || __i686__ || __i386 || __i386__ || __x86_64 || \
     __x86_64__ || __LITTLE_ENDIAN__ || _LITTLE_ENDIAN || __MIPSEL
#define RTI_ENDIAN_LITTLE 1
#undef  RTI_ENDIAN_BIG
#elif __BIG_ENDIAN__ || _BIG_ENDIAN || __MIPSEB
#define RTI_ENDIAN_BIG    1
#undef  RTI_ENDIAN_LITTLE
#else
#error "Unable to determine GCC byte-order"
#endif /* __BYTE_ORDER__ */
#elif defined(_MSC_VER)
#if defined(_M_IX86) || defined(_M_X64) || defined(__ARM_ARCH)
#define RTI_ENDIAN_LITTLE 1
#undef  RTI_ENDIAN_BIG
#else
#error "Unable to determine MSC byte-order"
#endif
#else
#error "Unable to determine endianess."
#endif /* !_M_IX86 */
#endif

/* Try to determine the ptr size unless it has been set by this flag */
#ifndef RTI_64BIT

#if defined(_MSC_VER)
#if _WIN64
#define RTI_64BIT 1
#endif /* _MSC_VER */
#elif defined(__GNUC__)
#if defined(__SIZEOF_PTRDIFF_T__)
#if ( __SIZEOF_PTRDIFF_T__ == 8 )
#define RTI_64BIT 1
#elif ( __SIZEOF_PTRDIFF_T__ != 4 )
#error "Size of PTR DIFF not 4 or 8 "
#endif /* __SIZEOF_PTRDIFF_T__ */
#elif __x86_64__ || __x86_64 || __ARM64__
#define RTI_64BIT 1
#elif i386 || i686 || __i686 || __i686__ || __i386 || __i386__ || _ARCH_PPC || \
      __mips__ || mips || __arm__ || PPC
    /* 32 bits */
#else
#error "GCC - Unable to determine whether architecture is 32 or 64 bit"
#endif
#else
#error "Unable to determine 32 bit or 64 architecture"
#endif

#endif /* RTI_64BIT */

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
#define OSAPI_CC_EXPORT_LIB_FUNCTION
#define OSAPI_CC_EXPORT_LIB_VARIABLE
#define OSAPI_CC_IMPORT_LIB_FUNCTION
#define OSAPI_CC_IMPORT_LIB_VARIABLE
#elif defined(_MSC_VER) || defined(WIN32)
#define OSAPI_CC_EXPORT_LIB_FUNCTION __declspec( dllexport )
#define OSAPI_CC_EXPORT_LIB_VARIABLE __declspec( dllexport )
#define OSAPI_CC_IMPORT_LIB_FUNCTION
#define OSAPI_CC_IMPORT_LIB_VARIABLE __declspec( dllimport )
#if _MSC_VER >= 1900
#define MUST_CHECK_RETURN _Check_return_
#else
#include <CodeAnalysis\SourceAnnotations.h>
#define MUST_CHECK_RETURN [returnvalue:SA_Post(MustCheck=SA_Yes)]
#endif
#define SHOULD_CHECK_RETURN
#define FUNCTION_SHOULD_TYPEDEF(ftdef_) \
typedef ftdef_;
#define FUNCTION_MUST_TYPEDEF(ftdef_) \
typedef ftdef_;
#else
#define SHOULD_CHECK_RETURN
#define FUNCTION_SHOULD_TYPEDEF(ftdef_) \
typedef ftdef_;
#define FUNCTION_MUST_TYPEDEF(ftdef_) \
typedef ftdef_;
#endif

/* In case it is defined elsewhere */
#ifndef RTIME_FUNCTION_NAME
/* MS VC/VC++ is not 100% C99 compliant, some versions support __func__ */
#ifdef _MSC_VER
#if (_MSC_VER >= 1900)
#define  RTIME_FUNCTION_NAME __func__
#else
#define RTIME_FUNCTION_NAME __FUNCTION__
#endif /* _MSC_VER >= 1900 */
#elif defined (__STDC__)
/* func was introduced in C99 */
#if __STDC_VERSION__ >= 199901L
#define RTIME_FUNCTION_NAME __func__
#else
/* Fallback */
#define RTIME_FUNCTION_NAME __FUNCTION__
#endif /* __STDC_VERSION__ */
#endif /* __STDC__ */
#endif /* RTIME_FUNCTION_NAME */


#endif /* osapi_cc_stdc_h */

