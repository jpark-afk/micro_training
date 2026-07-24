/*
 * FILE: osapi_cc_tasking.h - Specific compiler related definitions
 *
 * (c) Copyright, Real-Time Innovations, 2012-2026
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
 */
/*ce \file
 *   \brief OS API Configuration
 */
#ifndef osapi_cc_tasking_h
#define osapi_cc_tasking_h

#if !__TASKING__
#error "This is not a Tasking CCcompiler"
#endif


#if RTI_PIL
extern void
qsort(void *base, unsigned long nitems, unsigned long size, int (*compar)(const void *, const void*));
#endif

/* Typically for more standard PILs we dissallow the use of standard library
 * includes as they can include platform specific code. Since each Tasking
 * distrubution is for a specific CPU platform, we can include these headers
*/ 
#include <float.h> //21.1.13
#include <limits.h>


#ifndef NULL
#define NULL ((void*)0)
#endif

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
#error "Inconsistent defintion of SHAR_MAX"
#endif /* SCHAR_MAX */

#if SHRT_MAX == 32767
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

#if INT_MAX == 2147483647
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

#if LLONG_MAX == 9223372036854775807LL
/*i
 */
 typedef signed long long int RTI_INT64;

 /*i
  */
 typedef unsigned long long int RTI_UINT64;
#else /* no size defined */
#error "Could not determine RTI_INT64/RTI_UINT64 type"
#endif

/*******************************************************************************
 *                  Define compiler floating point mappings
 ******************************************************************************/
#if (FLT_MAX_EXP == +128)
typedef float RTI_FLOAT32;
#define RTI_HAVE_FLOAT
#else
#error "Inconsistent defintion of FLT_MAX_EXP, could not determine RTI_FLOAT32."
#endif

#if (DBL_MAX_EXP == +1024) && __DOUBLE_FP__
typedef double RTI_DOUBLE64;
#define RTI_HAVE_DOUBLE
#elif  (DBL_MAX_EXP == FLT_MAX_EXP)
/* If compiled with --fp-model=+float then doubles are floats */
#if LLONG_MAX == 9223372036854775807LL
typedef signed long long int RTI_DOUBLE64;
#else
typedef struct RTI_DOUBLE64
{
    char bytes[8];
} RTI_DOUBLE64;
#endif
#else
#error "Inconsistent defintion of DBL_MAX_EXP, could not determine RTI_DOUBLE64."
#endif

#if (LDBL_MAX_EXP == +16384)
typedef struct RTI_DOUBLE128
{
    char bytes[16];
} RTI_DOUBLE128;
#elif (LDBL_MAX_EXP == DBL_MAX_EXP) || (LDBL_MAX_EXP == FLT_MAX_EXP)
/* Not supported by the compiler */
typedef struct RTI_DOUBLE128
{
    char bytes[16];
} RTI_DOUBLE128;
#else
#error "Could not determine if long double is supported."
#endif

/*******************************************************************************
 *                          Define CPU endian
 ******************************************************************************/
#if !defined(RTI_ENDIAN_LITTLE) && !defined(RTI_ENDIAN_BIG)
#if __CORE_TC16X__ || __CORE_TC162__
/* Infineon tricore is little endian */
#define RTI_ENDIAN_LITTLE 1
#else
#error "Unable to determine endianess."
#endif
#endif

/*******************************************************************************
 *                   Define various compiler compile options
 ******************************************************************************/

/*e
 * \ingroup OSAPI_HeapClass
 * \brief Gets alignment of a type
 */
#define OSAPI_get_alignment_of(testType) \
({\
    struct S_ { char c; testType member; } ; \
    (int)((char*)&(((struct S_*)NULL)->member) - (char*)NULL);\
})

/* Function attributes */
#define MUST_CHECK_RETURN
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

#endif /* osapi_cc_tasking_h */
