/*
(c) Copyright, Real-Time Innovations, 2018-2024.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
============================================================================= */

#ifndef xcdr_infrastructure_impl_h
#define xcdr_infrastructure_impl_h


#ifdef __cplusplus
    extern "C" {
#endif


/* ------------------------------------------------------------------------- */
/* ---- Macro implementations ---------------------------------------------- */
/* ------------------------------------------------------------------------- */
#define RTIXCdrAlignment_alignSizeUp(size, alignment) \
    (((size) + (((RTIXCdrUnsignedLong)(alignment)) - 1)) & \
        ~(((RTIXCdrUnsignedLong)(alignment)) - 1))

#define RTIXCdrUtility_pointerToULongLong(pointer__) \
    (RTI_UINT64)((char*)(pointer__) - (const char*)OSAPI_CC_NullPtr)

/* Initialize */

#define RTIXCdrType_init16Byte(value__)                                   \
    (((value__) == NULL)                                                  \
             ? RTI_XCDR_FALSE                                             \
             : (RTIXCdrMemory_zero((value__), RTI_XCDR_LONG_DOUBLE_SIZE), \
                RTI_XCDR_TRUE))

/* Copy */

#define RTIXCdrType_copy8Byte(out__, in__)                        \
    (((out__) == NULL || (in__) == NULL)                          \
             ? RTI_XCDR_FALSE                                     \
             : (RTIXCdrMemory_copy((out__), (in__), RTI_XCDR_LONG_LONG_SIZE), \
                RTI_XCDR_TRUE))

#define RTIXCdrType_copy16Byte(out__, in__)                         \
    (((out__) == NULL || (in__) == NULL)                            \
             ? RTI_XCDR_FALSE                                       \
             : (RTIXCdrMemory_copy((out__), (in__), RTI_XCDR_LONG_DOUBLE_SIZE), \
                RTI_XCDR_TRUE))

#define RTIXCdrType_copyChar(out__, in__)                  \
    (((void *) (out__) == NULL || (void *) (in__) == NULL) \
             ? RTI_XCDR_FALSE                              \
             : (*(out__) = *(in__), RTI_XCDR_TRUE))

#define RTIXCdrType_copyWchar(out__, in__)                 \
    (((void *) (out__) == NULL || (void *) (in__) == NULL) \
             ? RTI_XCDR_FALSE                              \
             : (*(out__) = *(in__), RTI_XCDR_TRUE))

#define RTIXCdrType_copyOctet(out__, in__)                 \
    (((void *) (out__) == NULL || (void *) (in__) == NULL) \
             ? RTI_XCDR_FALSE                              \
             : (*(out__) = *(in__), RTI_XCDR_TRUE))

#define RTIXCdrType_copyShort(out__, in__)                 \
    (((void *) (out__) == NULL || (void *) (in__) == NULL) \
             ? RTI_XCDR_FALSE                              \
             : (*(out__) = *(in__), RTI_XCDR_TRUE))

#define RTIXCdrType_copyUnsignedShort(out__, in__)         \
    (((void *) (out__) == NULL || (void *) (in__) == NULL) \
             ? RTI_XCDR_FALSE                              \
             : (*(out__) = *(in__), RTI_XCDR_TRUE))

#define RTIXCdrType_copyLong(out__, in__)                  \
    (((void *) (out__) == NULL || (void *) (in__) == NULL) \
             ? RTI_XCDR_FALSE                              \
             : (*(out__) = *(in__), RTI_XCDR_TRUE))

#define RTIXCdrType_copyUnsignedLong(out__, in__)          \
    (((void *) (out__) == NULL || (void *) (in__) == NULL) \
             ? RTI_XCDR_FALSE                              \
             : (*(out__) = *(in__), RTI_XCDR_TRUE))

#define RTIXCdrType_copyLongLong(out__, in__) \
    RTIXCdrType_copy8Byte((RTIXCdr8Byte *) (out__), (RTIXCdr8Byte *) (in__))

#define RTIXCdrType_copyUnsignedLongLong(out__, in__) \
    RTIXCdrType_copy8Byte((RTIXCdr8Byte *) (out__), (RTIXCdr8Byte *) (in__))

#define RTIXCdrType_copyFloat(out__, in__)                 \
    (((void *) (out__) == NULL || (void *) (in__) == NULL) \
             ? RTI_XCDR_FALSE                              \
             : (*(out__) = *(in__), RTI_XCDR_TRUE))

#define RTIXCdrType_copyDouble(out__, in__)                \
    (((void *) (out__) == NULL || (void *) (in__) == NULL) \
             ? RTI_XCDR_FALSE                              \
             : (*(out__) = *(in__), RTI_XCDR_TRUE))

#define RTIXCdrType_copyLongDouble(out__, in__) \
    RTIXCdrType_copy16Byte((RTIXCdr16Byte *) (out__), (RTIXCdr16Byte *) (in__))

#define RTIXCdrType_copyBoolean(out__, in__)               \
    (((void *) (out__) == NULL || (void *) (in__) == NULL) \
             ? RTI_XCDR_FALSE                              \
             : (*(out__) = *(in__), RTI_XCDR_TRUE))

#define RTIXCdrType_copyArray(out__, in__, length__, elementSize__)      \
    (((void *) (out__) == NULL || (void *) (in__) == NULL)               \
             ? RTI_XCDR_FALSE                                            \
             : (RTIXCdrMemory_copy(                                                  \
                        (out__),                                         \
                        (in__),                                          \
                        ((elementSize__) * (RTI_SIZE_T) (length__))),        \
                RTI_XCDR_TRUE))

#define RTIXCdrType_copyString(out__, in__, maximumLength__)               \
    (RTIXCdrBoolean) (((void *) (in__) == NULL) ? RTI_XCDR_FALSE           \
                               : ((RTIXCdrString_len((in__)) + 1 > (maximumLength__)) \
                                          ? RTI_XCDR_FALSE                 \
                                          : RTIXCdrType_copyArray(         \
                                                  (out__),                 \
                                                  (in__),                  \
                                                  RTIXCdrString_len((in__)) + 1u,      \
                                                  (RTIXCdrUnsignedLong)sizeof(RTIXCdrChar))))

#define RTIXCdrType_copyWstring(out__, in__, maximumLength__)                  \
    (RTIXCdrBoolean) (((void *) (in__) == NULL)                                                 \
             ? RTI_XCDR_FALSE                                                  \
             : ((RTIXCdrType_getWstringLength((in__)) + 1 > (maximumLength__)) \
                        ? RTI_XCDR_FALSE                                       \
                        : RTIXCdrType_copyArray(                               \
                                (out__),                                       \
                                (in__),                                        \
                                RTIXCdrType_getWstringLength((in__)) + 1u,      \
                                (RTIXCdrUnsignedLong)sizeof(RTIXCdrWchar))))

#define RTIXCdrType_initArray(value__, length__, elementSize__)       \
    (((void *) (value__) == NULL)                                     \
             ? RTI_XCDR_FALSE                                         \
             : (RTIXCdrMemory_zero((value__), ((elementSize__) * (length__))), \
                RTI_XCDR_TRUE))

#define RTIXCdrType_initArrayUnsafe(value__, length__, elementSize__)       \
    RTIXCdrMemory_zero((value__), ((elementSize__) * (length__)))

#define RTIXCdrType_initString(value__, maximumLength__) \
    RTIXCdrType_initArray((value__), (maximumLength__), sizeof(RTIXCdrChar))

#define RTIXCdrType_initWstring(value__, maximumLength__) \
    RTIXCdrType_initArray((value__), (maximumLength__), sizeof(RTIXCdrWchar))

#ifdef __cplusplus
    }   /* extern "C" */
#endif

/******************************************************************************
 * Including infrastructure_psm_impl must be the last thing we do in this file.
 ******************************************************************************/
#include "xcdr/xcdr_infrastructure_psm_impl.h"

#endif /* xcdr_infrastructure_impl_h */
