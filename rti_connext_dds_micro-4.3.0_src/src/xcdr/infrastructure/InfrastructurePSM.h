/*
 * FILE: InfrastructurePSM.h XTypes PSM file
 *
 * Copyright (c) 2018-2026 Real-Time Innovations,Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef InfrastructurePSM_h
#define InfrastructurePSM_h

#include "xcdr/xcdr_infrastructure.h"
#include "osapi/osapi_string.h"
#include "osapi/osapi_heap.h"

/* -------------------------------------------------------------------------- */
/* ----- Other -------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

#define RTI_XCDR_FUNCTION_NAME RTI_FUNCTION_NAME

/* -------------------------------------------------------------------------- */
/* ----- Heap Implementation ------------------------------------------------ */
/* -------------------------------------------------------------------------- */
#define RTIXCdrHeap_allocateArray(arrayStoragePointer, elementCount, elementType) \
        OSAPI_Heap_allocate_array(arrayStoragePointer,\
                                     (RTI_UINT32)elementCount, elementType)

extern void*
RTIXCdrHeap_reallocateArrayImpl(void **arrayStoragePointer,
                                RTIXCdrUnsignedLong elementCount,
                                RTI_UINT32 elementSize);

#ifndef RTI_CERT
#define RTIXCdrHeap_reallocateArray(arrayStoragePointer_,\
                                    elementCount_,\
                                    elementType_) \
(elementType_*)RTIXCdrHeap_reallocateArrayImpl((void**)arrayStoragePointer_,\
                                              elementCount_,\
                                              (RTI_UINT32)sizeof(elementType_))
#else
#define RTIXCdrHeap_reallocateArray(arrayStoragePointer_,\
            elementCount_,\
            elementType_) \
    OSAPI_Heap_allocate_array(arrayStoragePointer_,\
                               (RTI_UINT32)elementCount_,\
                               elementType_)
#endif

#ifndef RTI_CERT
#define RTIXCdrHeap_freeStructure(s_) \
    OSAPI_Heap_free_struct(s_)
#else
#define RTIXCdrHeap_freeStructure(s_) \
    UNUSED_ARG(s_)
#endif

#ifndef RTI_CERT
#define RTIXCdrHeap_freeArray(arrayStorage) \
    OSAPI_Heap_free_array(arrayStorage)
#else
#define RTIXCdrHeap_freeArray(arrayStorage) \
    UNUSED_ARG(arrayStorage)
#endif

#define RTIXCdrHeap_allocateString(stringStoragePointer, length) \
    OSAPI_Heap_allocate_string(stringStoragePointer, (RTI_SIZE_T)length)

#ifndef RTI_CERT
#define RTIXCdrHeap_freeString(string) \
    OSAPI_Heap_free_string(string)
#else
#define RTIXCdrHeap_freeString(string) \
    UNUSED_ARG(string)
#endif

/* -------------------------------------------------------------------------- */
/* ----- Logging/Debuggability ---------------------------------------------- */
/* -------------------------------------------------------------------------- */

/*Do nothing for this for now*/
#define RTIXCdrLog_dumpBacktrace(logLevel)

#define RTIXCdrLog_addFunctionToDebugInfo(functionPointer)

#define RTIXCdrLog_logDebugInfo(logLevel)

#define RTI_XCDR_LOG_EXCEPTION   (0x00000001)
#define RTI_XCDR_LOG_WARNING     (0x00000002)

/* -------------------------------------------------------------------------- */
/* ----- Utility Implementation --------------------------------------------- */
/* -------------------------------------------------------------------------- */

#define RTIXCdrUtility_isnanf(x__) RTI_XCDR_FALSE

#define RTIXCdrUtility_isnan(x__) RTI_XCDR_FALSE

#define RTIXCdrUtility_isinff(x__) RTI_XCDR_FALSE

#define RTIXCdrUtility_isinf(x__)  RTI_XCDR_FALSE

#define RTIXCdrUtility_floatNearlyEqual(first__, second__) RTI_XCDR_TRUE

#define RTIXCdrUtility_doubleNearlyEqual(first__, second__) RTI_XCDR_TRUE

#define RTIXCdrUtility_intToPointer(i_) ((void*)(i_))

/* -------------------------------------------------------------------------- */
/* ----- Checks                 --------------------------------------------- */
/* -------------------------------------------------------------------------- */

#define RTIXCDR_UTILITY_CHECK_AND_GOTO_DONE_IF_NULL(ptr,context) \
    do {                                \
        if ((ptr) == NULL) {            \
        context->expectedSpaceError = RTI_XCDR_FALSE; \
            GotoDoneWithLine();         \
        }                               \
    } while (0)

#define RTIXCDR_UTILITY_CHECK_POSITION_FOR_NULL(ptr, context) \
    do {                                                       \
        if (((ptr) == NULL) || ((ptr)->_currentPosition == NULL)) { \
            (context)->expectedSpaceError = RTI_XCDR_FALSE;       \
            GotoDoneWithLine();                                \
        }                                                      \
    } while (0)

#endif /* InfrastructurePSM_h */
