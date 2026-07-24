/*
 * FILE: InfrastructurePSM.c XTypes PSM file
 *
 * (c) Copyright 2018-2024 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#include "xcdr/xcdr_infrastructure.h"
#include "xcdr/xcdr_log.h"
#include "Infrastructure.h"
#include "osapi/osapi_heap.h"
#include "reda/reda_string.h"
#include "osapi/osapi_log.h"

void*
RTIXCdrHeap_reallocateArrayImpl(
        void **arrayStoragePointer,
        RTIXCdrUnsignedLong elementCount,
        RTI_UINT32 elementSize)
{
    OSAPI_Heap_free_array(*arrayStoragePointer);
    *arrayStoragePointer = (void*)OSAPI_Heap_allocate(elementCount,elementSize);
    return *arrayStoragePointer;
}

int
RTIXCdrString_cmp(const char *s1,const char *s2)
{
    return REDA_String_compare(s1,s2);
}

char*
RTIXCdrString_copy(char *s1,const char *s2)
{

    if (REDA_String_copy(s1,REDA_String_length(s2),s2))
    {
        return s1;
    }

    return NULL;
}

RTIXCdrUnsignedLong
RTIXCdrString_getLength(const char *s1)
{
    return (RTIXCdrUnsignedLong)REDA_String_length(s1);
}

void
RTIXCdrLog_logWithParams(
        const char *fileName,
        const char *functionName,
        RTIXCdrUnsignedLong lineNumber,
        RTIXCdrLogSeverity severity,
        RTIXCdrLogMessageId messageId,
        RTIXCdrUnsignedLong paramCount,
        const RTIXCdrLogParam *params)
{
#if !OSAPI_ENABLE_LOG
    UNUSED_ARG(fileName);
    UNUSED_ARG(functionName);
    UNUSED_ARG(lineNumber);
    UNUSED_ARG(severity);
    UNUSED_ARG(messageId);
    UNUSED_ARG(paramCount);
    UNUSED_ARG(params);
#else
    OSAPI_LogKind_T m_log_level = OSAPI_LOGKIND_ERROR;
    RTI_UINT32 micro_ec = 0;

    switch (severity)
    {
        case RTI_XCDR_LOG_EXCEPTION:
            m_log_level = OSAPI_LOGKIND_ERROR;
            break;
        case RTI_XCDR_LOG_WARNING:
            m_log_level = OSAPI_LOGKIND_WARNING;
            break;
    }

    switch (messageId)
    {
        case RTI_XCDR_LOG_UNKNOWN_FAILURE_ID:
            micro_ec = XCDR_LOG_UNKNOWN_FAILURE_ID;
            OSAPI_Log_entry_create(m_log_level, micro_ec, RTI_MODULE_NAME,fileName,functionName,(RTI_INT32)lineNumber, RTI_TRUE);
            break;
        case RTI_XCDR_LOG_PRECONDITION_FAILURE_MSG_ID_s:
            if (paramCount > 0 && params != NULL)
            {
                if (params[0].kind == RTI_XCDR_LOG_STR_PARAM && params[0].value.strVal != NULL)
                {
                    micro_ec = XCDR_LOG_PRECONDITION_FAILURE_MSG_ID_EC;
                    OSAPI_Log_entry_create(m_log_level, micro_ec, RTI_MODULE_NAME,fileName,functionName,(RTI_INT32)lineNumber, RTI_FALSE);
                    OSAPI_Log_entry_add_string("val1:", params[0].value.strVal, RTI_TRUE);
                }
            }
            break;
        case RTI_XCDR_LOG_UNSUPPORTED_FAILURE_ID_s:
            if (paramCount > 0 && params != NULL)
            {
                if (params[0].kind == RTI_XCDR_LOG_STR_PARAM && params[0].value.strVal != NULL)
                {
                    micro_ec = XCDR_LOG_UNSUPPORTED_FAILURE_ID_EC;
                    OSAPI_Log_entry_create(m_log_level, micro_ec, RTI_MODULE_NAME,fileName,functionName,(RTI_INT32)lineNumber, RTI_FALSE);
                    OSAPI_Log_entry_add_string("val1:", params[0].value.strVal, RTI_TRUE);
                }
            }
            break;
        case RTI_XCDR_LOG_ALLOCATE_BUFFER_FAILURE_MSG_ID_d:
            if (paramCount > 0 && params != NULL)
            {
                if (params[0].kind == RTI_XCDR_LOG_LONG_PARAM)
                {
                    micro_ec = XCDR_LOG_ALLOCATE_BUFFER_FAILURE_MSG_ID_EC;
                    OSAPI_Log_entry_create(m_log_level, micro_ec, RTI_MODULE_NAME,fileName,functionName,(RTI_INT32)lineNumber, RTI_FALSE);
                    OSAPI_Log_entry_add_int("val1:", params[0].value.lVal, RTI_TRUE);
                }
            }
            break;
        case RTI_XCDR_LOG_ALLOCATE_STRUCTURE_FAILURE_MSG_ID_d:
            if (paramCount > 0 && params != NULL)
            {
                if (params[0].kind == RTI_XCDR_LOG_LONG_PARAM)
                {
                    micro_ec = XCDR_LOG_ALLOCATE_STRUCTURE_FAILURE_MSG_ID_EC;
                    OSAPI_Log_entry_create(m_log_level, micro_ec, RTI_MODULE_NAME,fileName,functionName,(RTI_INT32)lineNumber, RTI_FALSE);
                    OSAPI_Log_entry_add_int("val1:", params[0].value.lVal, RTI_TRUE);
                }
            }
            break;
        case RTI_XCDR_LOG_ALLOCATE_ARRAY_FAILURE_MSG_ID_dd:
        case RTI_XCDR_LOG_EXCEED_SEQ_MAX_LENGTH_ID_dd:
            if (paramCount == 2 && params != NULL)
            {
                if (params[0].kind == RTI_XCDR_LOG_LONG_PARAM && params[1].kind == RTI_XCDR_LOG_LONG_PARAM)
                {

                    if (messageId == RTI_XCDR_LOG_EXCEED_SEQ_MAX_LENGTH_ID_dd)
                    {
                        micro_ec = XCDR_LOG_EXCEED_SEQ_MAX_LENGTH_ID_EC;
                    }
                    else
                    {
                        micro_ec = XCDR_LOG_ALLOCATE_ARRAY_FAILURE_MSG_ID_EC;
                    }
                    OSAPI_Log_entry_create(m_log_level, micro_ec, RTI_MODULE_NAME,fileName,functionName,(RTI_INT32)lineNumber, RTI_FALSE);
                    OSAPI_Log_entry_add_int("val1:", params[0].value.lVal, RTI_FALSE);
                    OSAPI_Log_entry_add_int("val2:", params[1].value.lVal, RTI_TRUE);
                }
            }
            break;
        case RTI_XCDR_LOG_GENERATE_PROGRAM_FAILURE_ID_s:
        case RTI_XCDR_LOG_CDR_DESERIALIZE_FAILURE_ID_s:
        case RTI_XCDR_LOG_CDR_SKIP_FAILURE_ID_s:
        case RTI_XCDR_LOG_CDR_SERIALIZE_FAILURE_ID_s:
        case RTI_XCDR_LOG_INITIALIZE_FAILURE_ID_s:
        case RTI_XCDR_LOG_ASSERT_FAILURE_ID_s:
        case RTI_XCDR_LOG_ADD_FAILURE_ID_s:
        case RTI_XCDR_LOG_BAD_PARAM_FAILURE_ID_s:
        case RTI_XCDR_LOG_CREATE_FAILURE_ID_s:
            if (paramCount > 0 && params != NULL)
            {
                if (params[0].kind == RTI_XCDR_LOG_STR_PARAM && params[0].value.strVal != NULL)
                {
                    if (messageId == RTI_XCDR_LOG_GENERATE_PROGRAM_FAILURE_ID_s)
                    {
                        micro_ec = XCDR_LOG_GENERATE_PROGRAM_FAILURE_ID_EC;
                    }
                    else if (messageId == RTI_XCDR_LOG_CDR_DESERIALIZE_FAILURE_ID_s)
                    {
                        micro_ec = XCDR_LOG_CDR_DESERIALIZE_FAILURE_ID_EC;
                    }
                    else if (messageId == RTI_XCDR_LOG_CDR_SERIALIZE_FAILURE_ID_s)
                    {
                        micro_ec = XCDR_LOG_CDR_SERIALIZE_FAILURE_ID_EC;
                    }
                    else if (messageId == RTI_XCDR_LOG_INITIALIZE_FAILURE_ID_s)
                    {
                        micro_ec = XCDR_LOG_INITIALIZE_FAILURE_ID_EC;
                    }
                    else if (messageId == RTI_XCDR_LOG_CREATE_FAILURE_ID_s)
                    {
                        micro_ec = XCDR_LOG_CREATE_FAILURE_ID_EC;
                    }
                    else
                    {
                        micro_ec = XCDR_LOG_CDR_SKIP_FAILURE_ID_EC;
                    }

                    /*
                        case RTI_XCDR_LOG_ASSERT_FAILURE_ID_s:
                        case RTI_XCDR_LOG_ADD_FAILURE_ID_s:
                        case RTI_XCDR_LOG_BAD_PARAM_FAILURE_ID_s:
                    */
                    OSAPI_Log_entry_create(m_log_level, micro_ec, RTI_MODULE_NAME,fileName,functionName,(RTI_INT32)lineNumber, RTI_FALSE);
                    OSAPI_Log_entry_add_string("val1:", params[0].value.strVal, RTI_TRUE);
                }
            }
            break;
        case RTI_XCDR_LOG_GENERATE_PROGRAM_FAILURE_ID_ss:
        case RTI_XCDR_LOG_CDR_DESERIALIZE_FAILURE_ID_ss:
        case RTI_XCDR_LOG_CDR_SKIP_FAILURE_ID_ss:
        case RTI_XCDR_LOG_CDR_SERIALIZE_FAILURE_ID_ss:
        case RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss:
        case RTI_XCDR_LOG_GENERATE_PROGRAM_OUT_OF_BOUNDS_PRIMITIVE_FAILURE_ID_ss:
        case RTI_XCDR_LOG_GENERATE_PROGRAM_OUT_OF_BOUNDS_FAILURE_ID_ss:
        case RTI_XCDR_LOG_GET_FAILURE_ID_ss:
        case RTI_XCDR_LOG_CREATE_FAILURE_ID_ss:
        case RTI_XCDR_LOG_INITIALIZE_FAILURE_ID_ss:
        case RTI_XCDR_LOG_FINALIZE_FAILURE_ID_ss:
        case RTI_XCDR_LOG_COPY_FAILURE_ID_ss:
        case RTI_XCDR_LOG_CDR_DESERIALIZE_NOT_NULL_TERMINATED_STRING_FAILURE_ID_ss:
            if (paramCount == 2 && params != NULL)
            {
                if (params[0].kind == RTI_XCDR_LOG_STR_PARAM&& params[0].value.strVal != NULL
                && params[1].kind == RTI_XCDR_LOG_STR_PARAM
                && params[1].value.strVal != NULL)
                {
                    if (messageId == RTI_XCDR_LOG_CDR_DESERIALIZE_FAILURE_ID_ss)
                    {
                        micro_ec = XCDR_LOG_CDR_DESERIALIZE_FAILURE_ID_EC;
                    }
                    else if (messageId == RTI_XCDR_LOG_CDR_SERIALIZE_FAILURE_ID_ss)
                    {
                        micro_ec = XCDR_LOG_CDR_SERIALIZE_FAILURE_ID_EC;
                    }
                    else if (messageId == RTI_XCDR_LOG_CDR_SKIP_FAILURE_ID_ss)
                    {
                        micro_ec = XCDR_LOG_CDR_SKIP_FAILURE_ID_EC;
                    }
                    else if (messageId == RTI_XCDR_LOG_GENERATE_PROGRAM_OUT_OF_BOUNDS_PRIMITIVE_FAILURE_ID_ss)
                    {
                        micro_ec = XCDR_LOG_GENERATE_PROGRAM_OUT_OF_BOUNDS_PRIMITIVE_FAILURE_ID_EC;
                    }
                    else if (messageId == RTI_XCDR_LOG_GENERATE_PROGRAM_OUT_OF_BOUNDS_FAILURE_ID_ss)
                    {
                        micro_ec = XCDR_LOG_GENERATE_PROGRAM_OUT_OF_BOUNDS_FAILURE_ID_EC;
                    }
                    else if (messageId == RTI_XCDR_LOG_INITIALIZE_FAILURE_ID_ss)
                    {
                        micro_ec = XCDR_LOG_INITIALIZE_FAILURE_ID_EC;
                    }
                    else if (messageId == RTI_XCDR_LOG_GET_FAILURE_ID_ss)
                    {
                        micro_ec = XCDR_LOG_GET_FAILURE_ID_EC;
                    }
                    else if (messageId == RTI_XCDR_LOG_CREATE_FAILURE_ID_ss)
                    {
                        micro_ec = XCDR_LOG_CREATE_FAILURE_ID_EC;
                    }
                    else if (messageId == RTI_XCDR_LOG_FINALIZE_FAILURE_ID_ss)
                    {
                        micro_ec = XCDR_LOG_FINALIZE_FAILURE_ID_EC;
                    }
                    else if (messageId == RTI_XCDR_LOG_COPY_FAILURE_ID_ss)
                    {
                        micro_ec = XCDR_LOG_COPY_FAILURE_ID_EC;
                    }
                    else if (messageId == RTI_XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_ss)
                    {
                        micro_ec = XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_EC;
                    }
                    else
                    {
                        micro_ec = XCDR_LOG_GENERATE_PROGRAM_FAILURE_ID_EC;
                    }

                    OSAPI_Log_entry_create(m_log_level, micro_ec, RTI_MODULE_NAME,fileName,functionName,(RTI_INT32)lineNumber, RTI_FALSE);
                    OSAPI_Log_entry_add_string("val1:", params[0].value.strVal, RTI_FALSE);
                    OSAPI_Log_entry_add_string("val2:", params[1].value.strVal, RTI_TRUE);
                }
            }
            break;
        case RTI_XCDR_LOG_CDR_DESERIALIZE_OUT_OF_BOUNDS_STRING_FAILURE_ID_ssuu:
        case RTI_XCDR_LOG_CDR_DESERIALIZE_OUT_OF_BOUNDS_STRING_STREAM_FAILURE_ID_ssuu:
        case RTI_XCDR_LOG_CDR_DESERIALIZE_OUT_OF_BOUNDS_SEQUENCE_STREAM_FAILURE_ID_ssuu:
        case RTI_XCDR_LOG_CDR_DESERIALIZE_OUT_OF_BOUNDS_SEQUENCE_FAILURE_ID_ssuu:
        case RTI_XCDR_LOG_CDR_SKIP_OUT_OF_BOUNDS_STRING_FAILURE_ID_ssuu:
        case RTI_XCDR_LOG_CDR_SKIP_OUT_OF_BOUNDS_SEQUENCE_FAILURE_ID_ssuu:
        case RTI_XCDR_LOG_CDR_SERIALIZE_OUT_OF_BOUNDS_STRING_FAILURE_ID_ssuu:
        case RTI_XCDR_LOG_CDR_SERIALIZE_OUT_OF_BOUNDS_SEQUENCE_FAILURE_ID_ssuu:
            if (paramCount == 4 && params != NULL)
            {
                if (params[0].kind == RTI_XCDR_LOG_STR_PARAM && params[0].value.strVal != NULL
                        && params[1].kind == RTI_XCDR_LOG_STR_PARAM && params[1].value.strVal != NULL
                        && params[2].kind == RTI_XCDR_LOG_ULONG_PARAM && params[3].kind == RTI_XCDR_LOG_ULONG_PARAM)
                {
                    if (messageId == RTI_XCDR_LOG_CDR_DESERIALIZE_OUT_OF_BOUNDS_STRING_FAILURE_ID_ssuu)
                    {
                        micro_ec = XCDR_LOG_CDR_DESERIALIZE_OUT_OF_BOUNDS_STRING_FAILURE_ID_EC;
                    }
                    else if (messageId == RTI_XCDR_LOG_CDR_DESERIALIZE_OUT_OF_BOUNDS_SEQUENCE_FAILURE_ID_ssuu)
                    {
                        micro_ec = XCDR_LOG_CDR_DESERIALIZE_OUT_OF_BOUNDS_SEQUENCE_FAILURE_ID_EC;
                    }
                    else if (messageId == RTI_XCDR_LOG_CDR_SKIP_OUT_OF_BOUNDS_STRING_FAILURE_ID_ssuu)
                    {
                        micro_ec = XCDR_LOG_CDR_SKIP_OUT_OF_BOUNDS_STRING_FAILURE_ID_EC;
                    }
                    else if (messageId == RTI_XCDR_LOG_CDR_SKIP_OUT_OF_BOUNDS_SEQUENCE_FAILURE_ID_ssuu)
                    {
                        micro_ec = XCDR_LOG_CDR_SKIP_OUT_OF_BOUNDS_SEQUENCE_FAILURE_ID_EC;
                    }
                    else if (messageId == RTI_XCDR_LOG_CDR_SERIALIZE_OUT_OF_BOUNDS_STRING_FAILURE_ID_ssuu)
                    {
                        micro_ec = XCDR_LOG_CDR_SERIALIZE_OUT_OF_BOUNDS_STRING_FAILURE_ID_EC;
                    }
                    else
                    {
                        micro_ec = XCDR_LOG_CDR_SERIALIZE_OUT_OF_BOUNDS_SEQUENCE_FAILURE_ID_EC;
                    }

                    /*
                        case RTI_XCDR_LOG_CDR_DESERIALIZE_OUT_OF_BOUNDS_STRING_STREAM_FAILURE_ID_ssuu:
                        case RTI_XCDR_LOG_CDR_DESERIALIZE_OUT_OF_BOUNDS_SEQUENCE_STREAM_FAILURE_ID_ssuu:
                    */
                    OSAPI_Log_entry_create(m_log_level, micro_ec, RTI_MODULE_NAME,fileName,functionName,(RTI_INT32)lineNumber, RTI_FALSE);
                    OSAPI_Log_entry_add_string("val1:", params[0].value.strVal, RTI_FALSE);
                    OSAPI_Log_entry_add_string("val2:", params[1].value.strVal, RTI_FALSE);
                    OSAPI_Log_entry_add_uint("val3:", params[2].value.ulVal, RTI_FALSE);
                    OSAPI_Log_entry_add_uint("val4:", params[3].value.ulVal, RTI_TRUE);
                }
            }
            break;
        case RTI_XCDR_LOG_GENERATE_PROGRAM_OUT_OF_BOUNDS_STRING_FAILURE_ID_ssus:
            if (paramCount == 4 && params != NULL)
            {
                if (params[0].kind == RTI_XCDR_LOG_STR_PARAM&& params[0].value.strVal != NULL
                && params[1].kind == RTI_XCDR_LOG_STR_PARAM
                && params[1].value.strVal != NULL
                && params[2].kind == RTI_XCDR_LOG_ULONG_PARAM
                && params[3].kind == RTI_XCDR_LOG_STR_PARAM
                && params[3].value.strVal != NULL)
                {
                    micro_ec = XCDR_LOG_ALLOCATE_STRUCTURE_FAILURE_MSG_ID_EC;
                    OSAPI_Log_entry_create(m_log_level, micro_ec, RTI_MODULE_NAME,fileName,functionName,(RTI_INT32)lineNumber, RTI_FALSE);
                    OSAPI_Log_entry_add_string("val1:", params[0].value.strVal, RTI_FALSE);
                    OSAPI_Log_entry_add_string("val2:", params[1].value.strVal, RTI_FALSE);
                    OSAPI_Log_entry_add_uint("val3:", params[2].value.ulVal, RTI_FALSE);
                    OSAPI_Log_entry_add_string("val4:", params[3].value.strVal, RTI_TRUE);
                }
            }
            break;
        case RTI_XCDR_LOG_CDR_SERIALIZE_INVALID_ENUMERATOR_ID_ssd:
        case RTI_XCDR_LOG_CDR_DESERIALIZE_INVALID_ENUMERATOR_ID_ssd:
        case RTI_XCDR_LOG_CDR_DESERIALIZE_INVALID_UNION_DISC_ssd:
            if (paramCount == 3 && params != NULL)
            {
                if (params[0].kind == RTI_XCDR_LOG_STR_PARAM && params[0].value.strVal != NULL
                        && params[1].kind == RTI_XCDR_LOG_STR_PARAM && params[1].value.strVal != NULL
                        && params[2].kind == RTI_XCDR_LOG_LONG_PARAM)
                {
                    if (messageId == RTI_XCDR_LOG_CDR_SERIALIZE_INVALID_ENUMERATOR_ID_ssd)
                    {
                        micro_ec = XCDR_LOG_CDR_SERIALIZE_INVALID_ENUMERATOR_ID_EC;
                    }
                    else if (messageId == RTI_XCDR_LOG_CDR_DESERIALIZE_INVALID_ENUMERATOR_ID_ssd)
                    {
                        micro_ec = XCDR_LOG_CDR_DESERIALIZE_INVALID_ENUMERATOR_ID_EC;
                    }
                    else if (messageId == RTI_XCDR_LOG_CDR_DESERIALIZE_INVALID_UNION_DISC_ssd)
                    {
                        micro_ec = XCDR_LOG_CDR_DESERIALIZE_INVALID_UNION_DISC_EC;
                    }

                    OSAPI_Log_entry_create(m_log_level, micro_ec, RTI_MODULE_NAME,fileName,functionName,(RTI_INT32)lineNumber, RTI_FALSE);
                    OSAPI_Log_entry_add_string("val1:", params[0].value.strVal, RTI_FALSE);
                    OSAPI_Log_entry_add_string("val2:", params[1].value.strVal, RTI_FALSE);
                    OSAPI_Log_entry_add_int("val3:", params[2].value.lVal, RTI_TRUE);
                }
            }
            break;
        case RTI_XCDR_LOG_CDR_DESERIALIZE_UNKNOWN_PARAMETER_ID_su:
            if (paramCount == 2 && params != NULL)
            {
                if (params[0].kind == RTI_XCDR_LOG_STR_PARAM && params[0].value.strVal != NULL
                        && params[1].kind == RTI_XCDR_LOG_ULONG_PARAM)
                {
                    micro_ec = XCDR_LOG_CDR_DESERIALIZE_UNKNOWN_PARAMETER_ID_EC;
                    OSAPI_Log_entry_create(m_log_level, micro_ec, RTI_MODULE_NAME,fileName,functionName,(RTI_INT32)lineNumber, RTI_FALSE);
                    OSAPI_Log_entry_add_string("val1:", params[0].value.strVal, RTI_FALSE);
                    OSAPI_Log_entry_add_uint("val3:", params[1].value.ulVal, RTI_TRUE);
                }
            }
            break;
        case RTI_XCDR_LOG_BUILDER_OUT_OF_RESOURCES_FAILURE_ID:

            micro_ec = XCDR_LOG_BUILDER_OUT_OF_RESOURCES_FAILURE_ID_EC;
            OSAPI_Log_entry_create(m_log_level, micro_ec, RTI_MODULE_NAME,fileName,functionName,(RTI_INT32)lineNumber, RTI_FALSE);
            break;
        case RTI_XCDR_LOG_SERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssddd:
        case RTI_XCDR_LOG_DESERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssddd:
            if (paramCount == 5 && params != NULL)
            {
                if (params[0].kind == RTI_XCDR_LOG_STR_PARAM && params[0].value.strVal != NULL
                        && params[1].kind == RTI_XCDR_LOG_STR_PARAM && params[1].value.strVal != NULL
                        && params[2].kind == RTI_XCDR_LOG_LONGLONG_PARAM
                        && params[3].kind == RTI_XCDR_LOG_LONGLONG_PARAM
                        && params[4].kind == RTI_XCDR_LOG_LONGLONG_PARAM)
                {
                    if (messageId == RTI_XCDR_LOG_SERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssddd)
                    {
                        micro_ec = XCDR_LOG_SERIALIZE_FAILURE_OUT_OF_RANGE_ID_EC;
                    }
                    else if (messageId == RTI_XCDR_LOG_DESERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssddd)
                    {
                        micro_ec = XCDR_LOG_DESERIALIZE_FAILURE_OUT_OF_RANGE_ID_EC;
                    }

                    OSAPI_Log_entry_create(m_log_level, micro_ec, RTI_MODULE_NAME,fileName,functionName,(RTI_INT32)lineNumber, RTI_FALSE);
                    OSAPI_Log_entry_add_string("val1:", params[0].value.strVal, RTI_FALSE);
                    OSAPI_Log_entry_add_string("val2:", params[1].value.strVal, RTI_TRUE);

                    /*RTIXCdrLog_setLogLevelAndPrint(logBitmap)(
                     METHOD_NAME,
                     logMsg,
                     params[0].value.strVal,
                     params[1].value.strVal,
                     params[2].value.llVal,
                     params[3].value.llVal,
                     params[4].value.llVal);*/
                }
            }
            break;
        case RTI_XCDR_LOG_SERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssuuu:
        case RTI_XCDR_LOG_DESERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssuuu:
            if (paramCount == 5 && params != NULL)
            {
                if (params[0].kind == RTI_XCDR_LOG_STR_PARAM && params[0].value.strVal != NULL
                        && params[1].kind == RTI_XCDR_LOG_STR_PARAM && params[1].value.strVal != NULL
                        && params[2].kind == RTI_XCDR_LOG_ULONGLONG_PARAM
                        && params[3].kind == RTI_XCDR_LOG_ULONGLONG_PARAM
                        && params[4].kind == RTI_XCDR_LOG_ULONGLONG_PARAM)
                {
                    if (messageId == RTI_XCDR_LOG_SERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssuuu)
                    {
                        micro_ec = XCDR_LOG_SERIALIZE_FAILURE_OUT_OF_RANGE_ID_EC;
                    }
                    else if (messageId == RTI_XCDR_LOG_DESERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssuuu)
                    {
                        micro_ec = XCDR_LOG_DESERIALIZE_FAILURE_OUT_OF_RANGE_ID_EC;
                    }

                    OSAPI_Log_entry_create(m_log_level, micro_ec, RTI_MODULE_NAME,fileName,functionName,(RTI_INT32)lineNumber, RTI_FALSE);
                    OSAPI_Log_entry_add_string("val1:", params[0].value.strVal, RTI_FALSE);
                    OSAPI_Log_entry_add_string("val2:", params[1].value.strVal, RTI_TRUE);

                    /*RTIXCdrLog_setLogLevelAndPrint(logBitmap)(
                     METHOD_NAME,
                     logMsg,
                     params[0].value.strVal,
                     params[1].value.strVal,
                     params[2].value.ullVal,
                     params[3].value.ullVal,
                     params[4].value.ullVal);*/
                }
            }
            break;
        case RTI_XCDR_LOG_SERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssfff:
        case RTI_XCDR_LOG_DESERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssfff:
            if (paramCount == 5 && params != NULL)
            {
                if (params[0].kind == RTI_XCDR_LOG_STR_PARAM && params[0].value.strVal != NULL
                        && params[1].kind == RTI_XCDR_LOG_STR_PARAM && params[1].value.strVal != NULL
                        && params[2].kind == RTI_XCDR_LOG_DOUBLE_PARAM && params[3].kind == RTI_XCDR_LOG_DOUBLE_PARAM
                        && params[4].kind == RTI_XCDR_LOG_DOUBLE_PARAM)
                {
                    if (messageId == RTI_XCDR_LOG_SERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssfff)
                    {
                        micro_ec = XCDR_LOG_DESERIALIZE_FAILURE_OUT_OF_RANGE_ID_EC;
                    }
                    else if (messageId == RTI_XCDR_LOG_DESERIALIZE_FAILURE_OUT_OF_RANGE_ID_ssfff)
                    {
                        micro_ec = XCDR_LOG_SERIALIZE_FAILURE_OUT_OF_RANGE_ID_EC;
                    }

                    OSAPI_Log_entry_create(m_log_level, micro_ec, RTI_MODULE_NAME,fileName,functionName,(RTI_INT32)lineNumber, RTI_FALSE);
                    OSAPI_Log_entry_add_string("val1:", params[0].value.strVal, RTI_FALSE);
                    OSAPI_Log_entry_add_string("val2:", params[1].value.strVal, RTI_TRUE);

                    /*RTIXCdrLog_setLogLevelAndPrint(logBitmap)(
                     METHOD_NAME,
                     logMsg,
                     params[0].value.strVal,
                     params[1].value.strVal,
                     params[2].value.dVal,
                     params[3].value.dVal,
                     params[4].value.dVal);*/
                }
            }
            break;
        default:
            break;
    }
#endif /*OSAPI_ENABLE_LOG*/
}


void*
RTIXCdrHeap_allocate(RTIXCdrUnsignedLong size)
{
    char *buffer = NULL;

    buffer = OSAPI_Heap_allocate(1, (RTI_SIZE_T)size);

    return buffer;
}

void
RTIXCdrHeap_free(void * ptr)
{
    if (ptr == NULL)
    {
        return;
    }

#ifndef RTI_CERT
    OSAPI_Heap_free(ptr);
#endif
}

void*
RTIXCdrHeap_allocateWithAllocKind(
        RTIXCdrUnsignedLong size,
        RTIXCdrHeapAllocatorKind allocKind)
{
    /* The allocKind is used to tag structures in memory which is used
     * by the memory allocator in Pro. This is not supported by
     * Micro and ignoring it has no functional impact.
     */
    UNUSED_ARG(allocKind);

    return RTIXCdrHeap_allocate(size);
}

RTIXCdrBoolean
RTIXCdrHeap_reallocateString(char **ptr,RTIXCdrUnsignedLong string_size)
{
    char *retval;

    OSAPI_Heap_allocate_string(&retval,(RTI_SIZE_T)string_size);
    if (retval == NULL)
    {
        return 0;
    }

    OSAPI_Memory_copy(retval,*ptr,OSAPI_String_length(*ptr) + 1);
    *ptr = retval;

#ifndef RTI_CERT
    OSAPI_Heap_free(*ptr);
#endif

    return 1;
}

void
RTIXCdrMemory_copy(void *dest, const void *src, RTIXCdrUnsignedLong size)
{
    OSAPI_Memory_copy(dest, src, (RTI_SIZE_T)size);
}

void
RTIXCdrMemory_zero(void *ptr, RTIXCdrUnsignedLong size)
{
    OSAPI_Memory_zero(ptr, (RTI_SIZE_T)size);
}
