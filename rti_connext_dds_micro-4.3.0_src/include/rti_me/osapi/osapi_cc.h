/*
 * FILE: osapi_cc.h - Common compiler related definitions
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
 * 28jul2016,tk  Refactored from osapi_config.h
 */
/*ce \file
 *   \brief OS API Configuration
 */
#ifndef osapi_cc_h
#define osapi_cc_h

#ifndef OSAPI_CC_DEF_H
#if defined(__autosar__)
#define OSAPI_CC_DEF_H osapi_cc_autosar.h
#elif defined(__STDC__) || defined(_MSC_VER) || defined(RTI_WIN32)
#define OSAPI_CC_DEF_H osapi_cc_stdc.h
#elif defined(__TASKING__)
#define OSAPI_CC_DEF_H osapi_cc_tasking.h
#else
#error "Unable to determine compiler, please define OSAPI_CC_DEF_H"
#endif
#endif /* OSAPI_CC_DEF_H */

#include OSAPI_CC_STRINGIFY_DEFINE(OSAPI_CC_DEF_H)

/*ce
 * \brief If the compiler does not support static override it here.
 */
#ifndef OSAPI_CC_STATIC
#define RTI_PRIVATE static
#endif

/* This is used to prevent the compiler from issuing warnings for variables
 * not used on purpose (for example interface variables).
 */
#if OSAPI_ENABLE_PRECONDITION
#define PRECOND_ARG(x_)
#else
#define PRECOND_ARG(x_) (void)(x_);
#endif
#define UNUSED_ARG(x) (void)(x)
#define IGNORE_RETVAL(x) (void)(x)

#ifndef MUST_CHECK_RETURN
#define MUST_CHECK_RETURN
#endif

#ifndef SHOULD_CHECK_RETURN
#define SHOULD_CHECK_RETURN
#endif

#ifndef FUNCTION_SHOULD_TYPEDEF
#define FUNCTION_SHOULD_TYPEDEF(ftdef_) \
typedef SHOULD_CHECK_RETURN ftdef_;
#endif

#ifndef FUNCTION_MUST_TYPEDEF
#define FUNCTION_MUST_TYPEDEF(ftdef_) \
typedef MUST_CHECK_RETURN ftdef_;
#endif

#ifndef RTIME_FUNCTION_NAME
#define RTIME_FUNCTION_NAME "undefined"
#endif

#ifndef LINK_SECTION_DATA_SDRAM
#define LINK_SECTION_DATA_SDRAM
#endif
#ifndef LINK_SECTION_BSS_SDRAM
#define LINK_SECTION_BSS_SDRAM
#endif

/*ci \brief The boolean type (true/false)
 */
typedef RTI_INT32 RTI_BOOL;

/* Include this for convenient compatibility with RTI Connext Core
 */
typedef RTI_BOOL RTIBool;

typedef RTI_INT64 RTI_PTRDIFF_T;

/*ci \brief Boolean FALSE
 */
#define RTI_FALSE   ((RTI_BOOL) 0)

/*ci \brief Boolean TRUE
 */
#define RTI_TRUE    ((RTI_BOOL) 1)

/*ci \brief Consistent 32-bit unsigned integer for all platforms
 */
typedef RTI_UINT32 RTI_SIZE_T;

#define RTI_SIZE_MAX        4294967295U

#define RTI_SIZEOF(size_) ((RTI_SIZE_T)(sizeof(size_)))

#define RTI_SIZE_INVALID 0xffffffff

#if !defined(RTI_ENDIAN_LITTLE) && !defined(RTI_ENDIAN_BIG)
#error "RTI_ENDIAN_LITTLE or RTI_ENDIAN_BIG not defined"
#endif


#define OSAPI_MEMBER_OFFSET(type_,member_) \
    ((char*)(&((type_*)NULL)->member_) - (char*)OSAPI_CC_NullPtr)

/*
 */
#define OSAPI_BASE_FROM_MEMBER(ptr_,type_,member_) \
    OSAPI_Compiler_reinterpret_cast(type_*,((char*)(ptr_) - \
                                          OSAPI_MEMBER_OFFSET(type_,member_)))

/*ci \brief Macro to type-cast between different pointer types.
 */
#define OSAPI_Compiler_reinterpret_cast(type_,in_ptr_) \
    (type_)((void*)(in_ptr_))

/*ci \brief Macro to type-cast between different const pointer types. The type_
 *          argument is informational only to clarify the intent
 */
#define OSAPI_Compiler_reinterpret_constcast(type_,in_ptr_) \
    (const type_)((const void*)(in_ptr_))

/*ci \brief Macro that returns an aligned unsigned value
 */
#define OSAPI_Compiler_align_unsigned(in_,align_) \
        (((in_) + ((align_) - 1U)) & ~((align_)-1U))

#endif /* osapi_cc_h */
