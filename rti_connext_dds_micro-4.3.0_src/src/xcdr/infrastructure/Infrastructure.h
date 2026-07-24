/*
(c) Copyright, Real-Time Innovations, 2014-2025.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
============================================================================= */

#ifndef Infrastructure_h
#define Infrastructure_h

#include "xcdr/xcdr_infrastructure.h"

/* ------------------------------------------------------------------------- */
/* ---- Logging ------------------------------------------------------------ */
/* ------------------------------------------------------------------------- */

typedef int RTIXCdrLogSeverity;

typedef enum RTIXCdrLogParamKind {
    RTI_XCDR_LOG_STR_PARAM,
    RTI_XCDR_LOG_LONG_PARAM,
    RTI_XCDR_LOG_ULONG_PARAM,
    RTI_XCDR_LOG_LONGLONG_PARAM,
    RTI_XCDR_LOG_ULONGLONG_PARAM,
    RTI_XCDR_LOG_DOUBLE_PARAM
} RTIXCdrLogParamKind;

typedef struct RTIXCdrLogParam {
    RTIXCdrLogParamKind kind;
    struct RTIXCdrLogParamValue
    {
        const char *strVal;
        RTIXCdrLong lVal;
        RTIXCdrUnsignedLong ulVal;
        RTIXCdrLongLong llVal;
        RTIXCdrUnsignedLongLong ullVal;
        RTIXCdrDouble dVal;
    } value;
} RTIXCdrLogParam;

/* Important: When adding a new log message, do not forget to update the
 * function RTIXCdrLog_logWithParams to map this new msg into a product
 * specific message (Micro or Pro)
 */
typedef enum RTIXCdrLogMessageId {
    RTI_XCDR_LOG_PRECONDITION_FAILURE_MSG_ID_s = 0,
    RTI_XCDR_LOG_ALLOCATE_BUFFER_FAILURE_MSG_ID_d,
    RTI_XCDR_LOG_ALLOCATE_STRUCTURE_FAILURE_MSG_ID_d,
    RTI_XCDR_LOG_ALLOCATE_STRUCTURE_FAILURE_MSG_ID_su,
    RTI_XCDR_LOG_ALLOCATE_ARRAY_FAILURE_MSG_ID_dd,
    RTI_XCDR_LOG_CDR_SERIALIZE_FAILURE_ID_s,
    RTI_XCDR_LOG_CDR_SERIALIZE_FAILURE_ID_ss,
    RTI_XCDR_LOG_CDR_SERIALIZE_OUT_OF_BOUNDS_STRING_FAILURE_ID_ssuu,
    RTI_XCDR_LOG_CDR_SERIALIZE_OUT_OF_BOUNDS_SEQUENCE_FAILURE_ID_ssuu,
    RTI_XCDR_LOG_CDR_SERIALIZE_INVALID_ENUMERATOR_ID_ssd,
    RTI_XCDR_LOG_SERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssddd,
    RTI_XCDR_LOG_SERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssuuu,
    RTI_XCDR_LOG_SERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssfff,
    RTI_XCDR_LOG_CDR_DESERIALIZE_FAILURE_ID_s,
    RTI_XCDR_LOG_CDR_DESERIALIZE_FAILURE_ID_ss,
    RTI_XCDR_LOG_CDR_DESERIALIZE_OUT_OF_BOUNDS_STRING_FAILURE_ID_ssuu,
    RTI_XCDR_LOG_CDR_DESERIALIZE_OUT_OF_BOUNDS_STRING_STREAM_FAILURE_ID_ssuu,
    RTI_XCDR_LOG_CDR_DESERIALIZE_OUT_OF_BOUNDS_SEQUENCE_FAILURE_ID_ssuu,
    RTI_XCDR_LOG_CDR_DESERIALIZE_OUT_OF_BOUNDS_SEQUENCE_STREAM_FAILURE_ID_ssuu,
    RTI_XCDR_LOG_CDR_DESERIALIZE_INVALID_ENUMERATOR_ID_ssd,
    RTI_XCDR_LOG_CDR_DESERIALIZE_INVALID_UNION_DISC_ssd,
    RTI_XCDR_LOG_CDR_DESERIALIZE_UNKNOWN_PARAMETER_ID_su,
    RTI_XCDR_LOG_CDR_DESERIALIZE_NOT_NULL_TERMINATED_STRING_FAILURE_ID_ss,
    RTI_XCDR_LOG_DESERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssddd,
    RTI_XCDR_LOG_DESERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssuuu,
    RTI_XCDR_LOG_DESERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssfff,
    RTI_XCDR_LOG_CDR_SKIP_FAILURE_ID_s,
    RTI_XCDR_LOG_CDR_SKIP_FAILURE_ID_ss,
    RTI_XCDR_LOG_CDR_SKIP_OUT_OF_BOUNDS_STRING_FAILURE_ID_ssuu,
    RTI_XCDR_LOG_CDR_SKIP_OUT_OF_BOUNDS_SEQUENCE_FAILURE_ID_ssuu,
    RTI_XCDR_LOG_GENERATE_PROGRAM_FAILURE_ID_s,
    RTI_XCDR_LOG_GENERATE_PROGRAM_FAILURE_ID_ss,
    RTI_XCDR_LOG_GENERATE_PROGRAM_OUT_OF_BOUNDS_STRING_FAILURE_ID_ssus,
    RTI_XCDR_LOG_GENERATE_PROGRAM_OUT_OF_BOUNDS_PRIMITIVE_FAILURE_ID_ss,
    RTI_XCDR_LOG_GENERATE_PROGRAM_OUT_OF_BOUNDS_FAILURE_ID_ss,
    RTI_XCDR_LOG_UNSUPPORTED_FAILURE_ID_s,
    RTI_XCDR_LOG_UNSUPPORTED_FAILURE_ID_su,
    RTI_XCDR_LOG_CREATE_FAILURE_ID_ss,
    RTI_XCDR_LOG_GET_FAILURE_ID_ss,
    RTI_XCDR_LOG_GET_FAILURE_ID_s,
    RTI_XCDR_LOG_SET_FAILURE_ID_s,
    RTI_XCDR_LOG_INITIALIZE_FAILURE_ID_s,
    RTI_XCDR_LOG_INITIALIZE_FAILURE_ID_ss,
    RTI_XCDR_LOG_FINALIZE_FAILURE_ID_ss,
    RTI_XCDR_LOG_COPY_FAILURE_ID_ss,
    RTI_XCDR_LOG_CREATE_FAILURE_ID_s,
    RTI_XCDR_LOG_BUILDER_OUT_OF_RESOURCES_FAILURE_ID,
    RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss,
    RTI_XCDR_LOG_UNKNOWN_FAILURE_ID,
    RTI_XCDR_LOG_EXCEED_SEQ_MAX_LENGTH_ID_dd,
    RTI_XCDR_LOG_ASSERT_FAILURE_ID_s,
    RTI_XCDR_LOG_ADD_FAILURE_ID_s,
    RTI_XCDR_LOG_BAD_PARAM_FAILURE_ID_s,
} RTIXCdrLogMessageId;

extern void RTIXCdrLog_logWithParams(
        const char *fileName,
        const char *functionName,
        RTIXCdrUnsignedLong lineNumber,
        RTIXCdrLogSeverity severity,
        RTIXCdrLogMessageId messageId,
        RTIXCdrUnsignedLong paramCount,
        const RTIXCdrLogParam *params);

extern void RTIXCdrLog_log(
        RTIXCdrLogSeverity severity,
        RTIXCdrLogMessageId messageId);

extern void RTIXCdrLog_logLong(
        RTIXCdrLogSeverity severity,
        RTIXCdrLogMessageId messageId,
        RTIXCdrLong val);

extern void RTIXCdrLog_logTwoLong(
        RTIXCdrLogSeverity severity,
        RTIXCdrLogMessageId messageId,
        RTIXCdrLong val1,
        RTIXCdrLong val2);

extern void RTIXCdrLog_logStr(
        RTIXCdrLogSeverity severity,
        RTIXCdrLogMessageId messageId,
        const char *val);

extern void RTIXCdrLog_logTwoStr(
        RTIXCdrLogSeverity severity,
        RTIXCdrLogMessageId messageId,
        const char *val1,
        const char *val2);

extern void RTIXCdrLog_logTwoStrThreeLongLong(
        RTIXCdrLogSeverity severity,
        RTIXCdrLogMessageId messageId,
        const char *val1,
        const char *val2,
        RTIXCdrLongLong val3,
        RTIXCdrLongLong val4,
        RTIXCdrLongLong val5);

extern void RTIXCdrLog_logTwoStrThreeULongLong(
        RTIXCdrLogSeverity severity,
        RTIXCdrLogMessageId messageId,
        const char *val1,
        const char *val2,
        RTIXCdrUnsignedLongLong val3,
        RTIXCdrUnsignedLongLong val4,
        RTIXCdrUnsignedLongLong val5);

extern void RTIXCdrLog_logTwoStrThreeDouble(
        RTIXCdrLogSeverity severity,
        RTIXCdrLogMessageId messageId,
        const char *val1,
        const char *val2,
        RTIXCdrDouble val3,
        RTIXCdrDouble val4,
        RTIXCdrDouble val5);

extern void RTIXCdrLog_addFunctionToDebugInfo(void *pointerFuncName);

#ifdef RTI_PRECONDITION_TEST
    #define RTIXCdrLog_logPreconditionFailedWExpr(preconditionExpression) \
        RTIXCdrLog_logStr( \
                RTI_XCDR_LOG_FATAL, \
                RTI_XCDR_LOG_PRECONDITION_FAILURE_MSG_ID_s, \
                #preconditionExpression)

    #ifdef RTILog_checkPrecondition /* If using PRO */
        #define RTIXCdrLog_checkPrecondition(preconditionExpression, failAction) \
            RTILog_checkPrecondition( \
                    RTIXCdr, \
                    preconditionExpression, \
                    failAction)

    #else /* If using MICRO */
        #define RTIXCdrLog_checkPrecondition(preconditionExpression, failAction) \
            if (preconditionExpression) { \
                RTIXCdrLog_logPreconditionFailedWExpr(preconditionExpression); \
                failAction; \
            }

    #endif /* RTILog_checkPrecondition */
#else
    #define RTIXCdrLog_checkPrecondition(preconditionExpression, failAction)
#endif /* RTI_PRECONDITION_TEST */

#define RTIXCdrLog_testPrecondition(preconditionExpression, failAction) \
    /*                                                                         \
     * RTIXCdrLog_addFunctionToDebugInfo will only add                         \
     * the debug info if RTI_FUNCTION_HISTORY_SUPPORTED is defined             \
     */                                                                        \
    RTIXCdrLog_addFunctionToDebugInfo((void *) RTI_FUNCTION_NAME); \
    RTIXCdrLog_checkPrecondition(preconditionExpression, failAction)

/* This macro should be called at the begining of every function. */
#define RTIXCdrLog_startFunction() \
    /*                                                                         \
     * RTIXCdrLog_addFunctionToDebugInfo will be executed                  \
     * if RTI_FUNCTION_HISTORY_SUPPORTED is defined                            \
     */                                                                        \
    RTIXCdrLog_addFunctionToDebugInfo((void *) RTI_FUNCTION_NAME);

/* This macro should be called at the end of every function. */
#define RTIXCdrLog_Log_endFunction()

#ifdef RTI_PRECONDITION_TEST
#define RTIXCdrLog_preconditionOnly(declaration) declaration
#else /* nothing */
#define RTIXCdrLog_preconditionOnly(declaration)
#endif /* RTI_PRECONDITION_TEST */

/* ------------------------------------------------------------------------- */
/* ---- Alignment ---------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

#define RTI_XCDR_INVALID_ALIGNMENT -1

#define RTI_XCDR_ONE_BYTE_ALIGNMENT 1
#define RTI_XCDR_TWO_BYTE_ALIGNMENT 2
#define RTI_XCDR_FOUR_BYTE_ALIGNMENT 4
#define RTI_XCDR_V1_EIGHT_BYTE_ALIGNMENT 8
#define RTI_XCDR_V1_SIXTEEN_BYTE_ALIGNMENT 8
#define RTI_XCDR_V2_EIGHT_BYTE_ALIGNMENT 4
#define RTI_XCDR_V2_SIXTEEN_BYTE_ALIGNMENT 4

#define RTI_XCDR_LEGACY_WCHAR_ALIGNMENT RTI_XCDR_FOUR_BYTE_ALIGNMENT
#define RTI_XCDR_WCHAR_ALIGNMENT RTI_XCDR_TWO_BYTE_ALIGNMENT

extern RTIXCdrBoolean RTIXCdrAlignment_isValid(RTIXCdrAlignment val);

extern void *RTIXCdrAlignment_alignAddressUp(
        void *location,
        RTIXCdrAlignment alignment);

/* ------------------------------------------------------------------------- */
/* ---- Heap --------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

typedef enum RTIXCdrHeaAllocatorKind {
    RTI_XCDR_STRUCT_ALLOC       = 0x4E444441,
    RTI_XCDR_STRING_ALLOC       = 0x4E444442,
    RTI_XCDR_ARRAY_ALLOC        = 0x4E444443,
    RTI_XCDR_BUFFER_ALLOC       = 0x4E444444,
    RTI_XCDR_BUFFER_ALIGN_ALLOC = 0x4E444445,
    RTI_XCDR_MALLOC_ALLOC       = 0x4E444446,
    RTI_XCDR_FREED_BLOCK        = 0x7b9b9b9b
} RTIXCdrHeapAllocatorKind;

#define TYPE RTIXCdrLong

extern void RTIXCdrHeap_allocateArray(
        TYPE **arrayStoragePointer,
        RTIXCdrUnsignedLong elementCount,
        TYPE);

extern void RTIXCdrHeap_reallocateArray(
        TYPE **arrayStoragePointer,
        RTIXCdrUnsignedLong elementCount,
        TYPE);

extern void RTIXCdrHeap_freeArray(TYPE *arrayStorage);

extern void * RTIXCdrHeap_allocateWithAllocKind(
        RTIXCdrUnsignedLong size,
        RTIXCdrHeapAllocatorKind allocKind);

extern void RTIXCdrHeap_allocateBuffer(
        void **buffer,
        RTIXCdrUnsignedLong size,
        RTIXCdrLong alignment);

extern void RTIXCdrHeap_freeBuffer(void *buffer);

/* ------------------------------------------------------------------------- */
/* ---- String Utilities --------------------------------------------------- */
/* ------------------------------------------------------------------------- */

extern
void RTIXCdrHeap_allocateString(char **stringStoragePointer,
        RTIXCdrUnsignedLong length);

extern RTIXCdrBoolean RTIXCdrHeap_reallocateString(
        char **stringStoragePointer,
        RTIXCdrUnsignedLong length);

extern
void RTIXCdrHeap_freeString(char *stringStorage);

/**
 * @brief Duplicates a string.
 *
 * @param[in] str. String to duplicate.
 *
 * @return Pointer to the duplicated string.
 */
extern char *RTIXCdrString_dup(const char *str);

/**
 * @brief Copies a string to a preallocated memory.
 * @param[out] str1. Pointer to the preallocated memory.
 * @param[in] str2. String to copy.
 *
 * @return The pointer to the preallocated memory with the copied string.
 */
extern char *RTIXCdrString_copy(char *str1, const char *str2);

/**
 * @brief Compares two strings.
 * @param[in] str1. First string to compare.
 * @param[in] str2. Second string to compare.
 *
 * @return Returns 0 if the strings are equal, a negative value if str1
 *      is less than str2, or a positive value if str1 is greater than str2.
 */
extern RTIXCdrLong RTIXCdrString_cmp(const char *str1, const char *str2);

/**
 * @brief Compares two wide strings.
 * @param[in] str1. First wide string to compare.
 * @param[in] str2. Second wide string to compare.
 *
 * @return Returns 0 if the strings are equal, a negative value if str1
 *      is less than str2, or a positive value if str1 is greater than str2.
 */
extern RTIXCdrLong RTIXCdrWString_cmp(
        const RTIXCdrWchar *str1,
        const RTIXCdrWchar *str2);

extern
RTIXCdrWchar* RTIXCdr_allocateWString(RTIXCdrUnsignedLong length);

extern
void RTIXCdrHeap_freeWString(const RTIXCdrWchar* string);

extern
RTIXCdrBoolean RTIXCdrWString_copy(RTIXCdrWchar *dst, const RTIXCdrWchar *src);

extern
RTIXCdrWchar* RTIXCdrWString_dup(const RTIXCdrWchar *string);


#define RTIXCdrHeap_freeWString(string) RTIXCdrHeap_freeArray(string)

/* ------------------------------------------------------------------------- */
/* ---- Utilities ---------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

#define RTIXCdrUtility_unusedParameter(parameter__) (void) (parameter__)

#define RTIXCdrUtility_unusedReturnValue(function, type) \
{                                                        \
    type unusedReturnValue;                              \
    unusedReturnValue = function;                        \
    (void)unusedReturnValue;                             \
} (void)0

#define RTIXCdrUtility_assignDereferenceFromValue(__dstPtr, __src, __srcType) \
{ \
    __srcType *__dstPtrTyped = (__srcType *) (__dstPtr); \
    *__dstPtrTyped = (__src); \
}

#define RTIXCdrUtility_compareNumericValue(left, right) \
    ( ((left) > (right)) \
            ? 1 \
            : ( ((left) < (right)) ? (-1) : 0))

#define RTIXCdrUtility_comparePointer(left, right) \
        RTIXCdrUtility_compareNumericValue( \
                ((void *) (left)), \
                ((void *) (right)))

/* To remove
 * ISO C forbids assignment between function pointer and 'void *'
 * warning
 */
#define RTIXCdrUtility_compareFunctionPointer(left, right) \
        RTIXCdrUtility_compareNumericValue( \
                (*(void**)&(left)), \
                (*(void**)&(right)))

extern
RTIXCdrBoolean RTIXCdrUtility_isnanf(float x);

extern
RTIXCdrBoolean RTIXCdrUtility_isnan(double x);

extern
RTIXCdrBoolean RTIXCdrUtility_isinff(float x);

extern
RTIXCdrBoolean RTIXCdrUtility_isinf(double x);

/**
 * @brief Compares equality of two float number.
 *
 * Note: This is a macro only function.
 *
 * When comparing values of float type for equality or non-equality, the results 
 * may vary depending on the processor being used and compiler settings.
 *
 * It is recommended to use this function instead of the operators '==' and 
 *'!=' to compare equality of float numbers.
 *  
 * @param[in] first. First float number.
 * @param[in] second. Second float number.
 *
 * @return RTI_XCDR_TRUE if the two numbers are nearly equal.
 */
extern 
RTIXCdrBoolean RTIXCdrUtility_floatNearlyEqual(
        RTIXCdrFloat first,
        RTIXCdrFloat second);

/**
 * @brief Compares equality of two double number.
 *
 * Note: This is a macro only function.
 *
 * When comparing values of double type for equality or non-equality, the results 
 * may vary depending on the processor being used and compiler settings.
 *
 * It is recommended to use this function instead of the operators '==' and 
 *'!=' to compare equality of double numbers.
 *  
 * @param[in] first. First double number.
 * @param[in] second. Second double number.
 *
 * @return RTI_XCDR_TRUE if the two numbers are nearly equal.
 */
extern 
RTIXCdrBoolean RTIXCdrUtility_doubleNearlyEqual(
        RTIXCdrDouble first,
        RTIXCdrDouble second);

/**
 * @brief Gets the value of an environment variable.
 *
 * @param[in] envVariableName. Name of the environment variable.
 * @param[out] buffer. Buffer to store the value of the environment variable.
 * @param[in] bufferSize. Size of the buffer.
 *
 * @return The value of the environment variable. If the environment variable
 * does not exist, the function returns NULL.
 */
extern char *RTIXCdrUtility_getEnvironmentVariable(
        const char *envVariableName,
        char *buffer,
        RTIXCdrUnsignedLong bufferSize);

/**
 * @brief Converts a C-string to an unsigned integer.
 *
 * @param[in] str. C-string to convert.
 * @param[out] value. Unsigned integer value.
 *
 * @return RTI_XCDR_TRUE if the conversion was successful.
 */
extern
RTIXCdrBoolean RTIXCdrUtility_strtoul(
        const char * str,
        RTIXCdrUnsignedLong *value);

  /**
   * @brief Performs a type-safe and portable static cast.
   *
   * This macro casts a pointer of one type to another. The intermediate
   * cast to `void *` is a standard C idiom to bypass strict aliasing
   * warnings and ensure portability for intentional type-punning.
   *
   * Example:
   * char *strSample = 0;
   * (char **)charPtr2;
   * warning: cast from 'char *' to 'char **' increases required alignment
   * from 1 to 8 [-Wcast-align]
   *
   * It is still the caller's responsibility to ensure that the memory
   * block pointed to by `in_ptr__` is actually large enough and correctly
   * aligned to be used as a `type__`.
   *
   * @param[in] type__. The target type to cast to.
   * @param[in] in_ptr__. The input pointer to cast.
   *
   * @return The converted pointer of type `type__`.
   */
  #define RTIXCdrUtility_staticCast(type__, in_ptr__) \
        ((type__) ((void *) (in_ptr__)))

/* ------------------------------------------------------------------------- */
/* ---- Logging Implementation --------------------------------------------- */
/* ------------------------------------------------------------------------- */

#define RTIXCdrLog_log(severity, messageId) \
    RTIXCdrLog_logWithParams( \
        __FILE__, \
        RTI_XCDR_FUNCTION_NAME, \
        __LINE__, \
        (severity), \
        (messageId), \
        0, \
        NULL)

#define RTIXCdrLog_logLong(severity, messageId, val) \
    { \
        RTIXCdrLogParam __param = { 0 }; \
        __param.kind = RTI_XCDR_LOG_LONG_PARAM; \
        __param.value.lVal = (val); \
        \
        RTIXCdrLog_logWithParams( \
            __FILE__, \
            RTI_XCDR_FUNCTION_NAME, \
            __LINE__, \
            (severity), \
            (messageId), \
            1, \
            &__param); \
    }

#define RTIXCdrLog_logTwoLong(severity, messageId, val1, val2) \
    { \
        RTIXCdrLogParam __param[2] = { 0 }; \
        __param[0].kind = RTI_XCDR_LOG_LONG_PARAM; \
        __param[0].value.lVal = (val1); \
        __param[1].kind = RTI_XCDR_LOG_LONG_PARAM; \
        __param[1].value.lVal = (val2); \
        \
        RTIXCdrLog_logWithParams( \
            __FILE__, \
            RTI_XCDR_FUNCTION_NAME, \
            __LINE__, \
            (severity), \
            (messageId), \
            2, \
            __param); \
    }

#define RTIXCdrLog_logStr(severity, messageId, val) \
    { \
        RTIXCdrLogParam __param = { 0 }; \
        __param.kind = RTI_XCDR_LOG_STR_PARAM; \
        __param.value.strVal = (val); \
        \
        RTIXCdrLog_logWithParams( \
            __FILE__, \
            RTI_XCDR_FUNCTION_NAME, \
            __LINE__, \
            (severity), \
            (messageId), \
            1, \
            &__param); \
    }

#define RTIXCdrLog_logTwoStr(severity, messageId, val1, val2) \
    { \
        RTIXCdrLogParam __param[2] = { 0 }; \
        __param[0].kind = RTI_XCDR_LOG_STR_PARAM; \
        __param[0].value.strVal = (val1); \
        __param[1].kind = RTI_XCDR_LOG_STR_PARAM; \
        __param[1].value.strVal = (val2); \
        \
        RTIXCdrLog_logWithParams( \
            __FILE__, \
            RTI_XCDR_FUNCTION_NAME, \
            __LINE__, \
            (severity), \
            (messageId), \
            2, \
            __param); \
    }

#define RTIXCdrLog_logStrULong(severity, messageId, val1, val2) \
{ \
    RTIXCdrLogParam __param[2] = { 0 }; \
    __param[0].kind = RTI_XCDR_LOG_STR_PARAM; \
    __param[0].value.strVal = (val1); \
    __param[1].kind = RTI_XCDR_LOG_ULONG_PARAM; \
    __param[1].value.ulVal = (val2); \
    \
    RTIXCdrLog_logWithParams( \
        __FILE__, \
        RTI_XCDR_FUNCTION_NAME, \
        __LINE__, \
        (severity), \
        (messageId), \
        2, \
        __param); \
}

#define RTIXCdrLog_logTwoStrThreeLongLong( \
        severity, \
        messageId, \
        val1, \
        val2, \
        val3, \
        val4, \
        val5) \
{ \
    RTIXCdrLogParam __param[5] = { 0 }; \
    __param[0].kind = RTI_XCDR_LOG_STR_PARAM; \
    __param[0].value.strVal = (val1); \
    __param[1].kind = RTI_XCDR_LOG_STR_PARAM; \
    __param[1].value.strVal = (val2); \
    __param[2].kind = RTI_XCDR_LOG_LONGLONG_PARAM; \
    __param[2].value.llVal = (RTIXCdrLongLong)(val3); \
    __param[3].kind = RTI_XCDR_LOG_LONGLONG_PARAM; \
    __param[3].value.llVal = (RTIXCdrLongLong)(val4); \
    __param[4].kind = RTI_XCDR_LOG_LONGLONG_PARAM; \
    __param[4].value.llVal = (RTIXCdrLongLong)(val5); \
    \
    RTIXCdrLog_logWithParams( \
            __FILE__, \
            RTI_XCDR_FUNCTION_NAME, \
            __LINE__, \
            (severity), \
            (messageId), \
            5, \
            __param); \
}

#define RTIXCdrLog_logTwoStrThreeULongLong( \
        severity, \
        messageId, \
        val1, \
        val2, \
        val3, \
        val4, \
        val5) \
{ \
    RTIXCdrLogParam __param[5] = { 0 }; \
    __param[0].kind = RTI_XCDR_LOG_STR_PARAM; \
    __param[0].value.strVal = (val1); \
    __param[1].kind = RTI_XCDR_LOG_STR_PARAM; \
    __param[1].value.strVal = (val2); \
    __param[2].kind = RTI_XCDR_LOG_ULONGLONG_PARAM; \
    __param[2].value.ullVal = (RTIXCdrUnsignedLongLong)(val3); \
    __param[3].kind = RTI_XCDR_LOG_ULONGLONG_PARAM; \
    __param[3].value.ullVal = (RTIXCdrUnsignedLongLong)(val4); \
    __param[4].kind = RTI_XCDR_LOG_ULONGLONG_PARAM; \
    __param[4].value.ullVal = (RTIXCdrUnsignedLongLong)(val5); \
    \
    RTIXCdrLog_logWithParams( \
            __FILE__, \
            RTI_XCDR_FUNCTION_NAME, \
            __LINE__, \
            (severity), \
            (messageId), \
            5, \
            __param); \
}

#define RTIXCdrLog_logTwoStrThreeDouble( \
        severity, \
        messageId, \
        val1, \
        val2, \
        val3, \
        val4, \
        val5) \
{ \
    RTIXCdrLogParam __param[5] = { 0 }; \
    __param[0].kind = RTI_XCDR_LOG_STR_PARAM; \
    __param[0].value.strVal = (val1); \
    __param[1].kind = RTI_XCDR_LOG_STR_PARAM; \
    __param[1].value.strVal = (val2); \
    __param[2].kind = RTI_XCDR_LOG_DOUBLE_PARAM; \
    __param[2].value.dVal = (double)(val3); \
    __param[3].kind = RTI_XCDR_LOG_DOUBLE_PARAM; \
    __param[3].value.dVal = (double)(val4); \
    __param[4].kind = RTI_XCDR_LOG_DOUBLE_PARAM; \
    __param[4].value.dVal = (double)(val5); \
    \
    RTIXCdrLog_logWithParams( \
            __FILE__, \
            RTI_XCDR_FUNCTION_NAME, \
            __LINE__, \
            (severity), \
            (messageId), \
            5, \
            __param); \
}

  #ifdef RTI_PRECONDITION_TEST
void RTIXCdrLog_preconditionBreakPoint(void);
  #endif

/* ------------------------------------------------------------------------- */
/* ---- Alignment Implementation ------------------------------------------- */
/* ------------------------------------------------------------------------- */

  /**
   * Micro does not support size_t because it's platform dependent.
   * We need to use RTIXCdrUnsignedLongLong for 64 bits platforms
   * and RTIXCdrUnsignedLong for 32 bits platforms.
   * If you try to use the 32 bits version on a 64 bits platform
   * the output of the macro will be truncated to 32 bits.
   */
  #ifdef RTI_64BIT
    #define RTIXCdrAlignment_alignAddressUp(location, alignment) \
          ((void *) ((RTIXCdrUtility_pointerToUnsignedLongLong(location) \
                      + ((RTIXCdrUnsignedLongLong) (alignment) -1U)) \
                     & ~((RTIXCdrUnsignedLongLong) (alignment) -1U)))
  #else
    #define RTIXCdrAlignment_alignAddressUp(location, alignment) \
          ((void *) ((RTIXCdrUtility_pointerToUnsignedLong(location) \
                      + ((RTIXCdrUnsignedLong) (alignment) -1U)) \
                     & ~((RTIXCdrUnsignedLong) (alignment) -1U)))
  #endif /* RTI_64BIT */

  #define RTIXCdrAlignment_isValid(val) \
        (((val) == RTI_XCDR_ONE_BYTE_ALIGNMENT) \
            || ((val) == RTI_XCDR_TWO_BYTE_ALIGNMENT) \
            || ((val) == RTI_XCDR_FOUR_BYTE_ALIGNMENT) \
            || ((val) == RTI_XCDR_V1_EIGHT_BYTE_ALIGNMENT) \
            || ((val) == RTI_XCDR_V2_EIGHT_BYTE_ALIGNMENT) \
            || ((val) == RTI_XCDR_V1_SIXTEEN_BYTE_ALIGNMENT) \
            || ((val) == RTI_XCDR_V2_SIXTEEN_BYTE_ALIGNMENT))

#endif

#include "InfrastructurePSM.h"
