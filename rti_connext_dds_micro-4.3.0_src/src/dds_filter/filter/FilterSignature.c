/*
 * FILE: FilterSignature.c - Filter signature implementation
 *
 * Copyright (c) 2025-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#include "FilterSignature.h"

/*** SOURCE_BEGIN ***/

#define T    struct DDS_FilterSignature
#define TSeq DDS_FilterSignatureSeq
#define TSeq_loan_contiguous
#define REDA_SEQUENCE_USER_API
#include <reda/reda_sequence_defn.h>

RTI_INT32
DDS_FilterSignature_compare(const struct DDS_FilterSignature *left,
                            const struct DDS_FilterSignature *right)
{
    return OSAPI_Memory_compare(left, right, sizeof(struct DDS_FilterSignature));
}


RTI_BOOL
DDS_FilterSignature_calculate_from_property(
        struct DDS_FilterSignature *signature,
        struct DDS_ContentFilterProperty *property)
{
    return DDS_FilterSignature_calculate(
            signature,
            property->content_filtered_topic_name,
            property->related_topic_name,
            property->filter_class_name,
            property->filter_expression,
            &property->expression_parameters);
}

RTI_BOOL
DDS_FilterSignature_calculate(struct DDS_FilterSignature *signature,
                              DDS_String content_filtered_topic_name,
                              DDS_String related_topic_name,
                              DDS_String filter_class_name,
                              DDS_String filter_expression,
                              const struct DDS_StringSeq *expression_parameters)
{
    struct OSAPI_Buffer bufs[4 + DDS_CONTENT_FILTER_MAX_PARAMETERS];
    RTI_UINT32 n_bufs = 4;
    RTI_INT32 i, parameter_count;

    OSAPI_PRECONDITION(
            (signature == NULL) || (content_filtered_topic_name == NULL) ||
            (related_topic_name == NULL) || (filter_class_name == NULL) ||
            (filter_expression == NULL) || (expression_parameters == NULL),
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("signature", signature, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("content_filtered_topic_name", content_filtered_topic_name, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("related_topic_name", related_topic_name, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("filter_class_name", filter_class_name, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("filter_expression", filter_expression, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("expression_parameters", expression_parameters, RTI_TRUE);)

    parameter_count = DDS_StringSeq_get_length(expression_parameters);
    if (parameter_count > DDS_CONTENT_FILTER_MAX_PARAMETERS)
    {
        return RTI_FALSE;
    }

    bufs[0].pointer = content_filtered_topic_name;
    bufs[0].length = OSAPI_String_length(content_filtered_topic_name) + 1;

    bufs[1].pointer = related_topic_name;
    bufs[1].length = OSAPI_String_length(related_topic_name) + 1;

    bufs[2].pointer = filter_class_name;
    bufs[2].length = OSAPI_String_length(filter_class_name) + 1;

    bufs[3].pointer = filter_expression;
    bufs[3].length = OSAPI_String_length(filter_expression) + 1;

    for (i = 0; i < parameter_count; i++)
    {
        DDS_String *parameter = DDS_StringSeq_get_reference(expression_parameters, i);
        if ((parameter == NULL) || (*parameter == NULL))
        {
            return RTI_FALSE;
        }
        bufs[n_bufs].pointer = *parameter;
        bufs[n_bufs].length = OSAPI_String_length(*parameter) + 1;
        n_bufs++;
    }

    OSAPI_Hash_compute_buffer_scatter_md5(bufs, n_bufs, (RTI_UINT8 *)&signature->value);

    return RTI_TRUE;
}
