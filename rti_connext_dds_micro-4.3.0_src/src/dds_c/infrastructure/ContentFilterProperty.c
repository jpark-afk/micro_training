/*
 * FILE: ContentFilterProperty.c - Content Filter Property API implementation
 *
 * (c) Copyright, Real-Time Innovations, 2025-2025.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*ci
 * \brief Content Filter Property API implementation
 */

#include "dds_c/dds_c_content_filter.h"
#include "dds_c/dds_c_domain.h"
#include "dds_c/dds_c_topic.h"
#include "osapi/osapi_log.h"

/*** SOURCE_BEGIN ***/

#ifndef RTI_CERT
DDS_Boolean
DDS_ContentFilterProperty_finalize(struct DDS_ContentFilterProperty *policy)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION((policy == NULL), goto done,
            OSAPI_Log_entry_add_pointer("policy", policy, RTI_TRUE);)

    DDS_String_free(policy->content_filtered_topic_name);
    policy->content_filtered_topic_name = NULL;

    DDS_String_free(policy->related_topic_name);
    policy->related_topic_name = NULL;

    DDS_String_free(policy->filter_class_name);
    policy->filter_class_name = NULL;

    DDS_String_free(policy->filter_expression);
    policy->filter_expression = NULL;

    if (!DDS_StringSeq_finalize(&policy->expression_parameters))
    {
        goto done;
    }

    retval = DDS_BOOLEAN_TRUE;
done:
    return retval;
}
#endif /* !RTI_CERT */

DDS_Boolean
DDS_ContentFilterProperty_initialize(struct DDS_ContentFilterProperty *policy)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION((policy == NULL), goto done,
            OSAPI_Log_entry_add_pointer("policy", policy, RTI_TRUE);)

    *policy = (struct DDS_ContentFilterProperty)DDS_CONTENT_FILTER_PROPERTY_DEFAULT;

    if (!DDS_StringSeq_initialize(&policy->expression_parameters))
    {
        goto done;
    }

    retval = DDS_BOOLEAN_TRUE;
done:
    return retval;
}

DDS_Boolean
DDS_ContentFilterProperty_copy(struct DDS_ContentFilterProperty *out,
                         const struct DDS_ContentFilterProperty *in)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION((out == NULL) || (in == NULL),
                    return DDS_BOOLEAN_FALSE,
                    OSAPI_Log_entry_add_pointer("out",out,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("in",in,RTI_TRUE);)

   if (!REDA_String_copy_w_max(
            &out->content_filtered_topic_name,
            in->content_filtered_topic_name,
            RTPS_PATHNAME_LEN_MAX))
    {
        goto done;
    }

    if (!REDA_String_copy_w_max(
            &out->related_topic_name,
            in->related_topic_name,
            RTPS_PATHNAME_LEN_MAX))
    {
        goto done;
    }

    if (!REDA_String_copy_w_max(
            &out->filter_class_name,
            in->filter_class_name,
            DDS_CONTENT_FILTER_CLASS_NAME_MAX_LENGTH))
    {
        goto done;
    }

    if (!REDA_String_copy_w_max(
            &out->filter_expression,
            in->filter_expression,
            DDS_CONTENT_FILTER_EXPRESSION_MAX_LENGTH))
    {
        goto done;
    }

    /* DDS_StringSeq_copy() will have different behavior depending on how
     * the sequence was initialized. If it was initialized with a maximum
     * string length, then the strings will be copied into the existing
     * buffers. If it was initialized without a maximum string length,
     * then the strings will be freed and new buffers will be allocated.
     */
    if (!DDS_StringSeq_copy(&out->expression_parameters,
                            &in->expression_parameters))
    {
        goto done;
    }

    retval = DDS_BOOLEAN_TRUE;
done:
    return retval;
}

/* Treat NULL and "" as equal: deserialize may yield NULL, user assignment
 * may yield "". Both represent the absence of a filter field.
 */
RTI_PRIVATE DDS_Boolean
DDS_ContentFilterProperty_field_string_is_equal(const char *left,
                                                const char *right)
{
    DDS_Boolean left_empty = (DDS_Boolean)((left == NULL) || (left[0] == '\0'));
    DDS_Boolean right_empty = (DDS_Boolean)((right == NULL) || (right[0] == '\0'));

    if (left_empty && right_empty)
    {
        return DDS_BOOLEAN_TRUE;
    }
    if (left_empty != right_empty)
    {
        return DDS_BOOLEAN_FALSE;
    }
    return DDS_String_cmp(left, right) == 0;
}

RTI_BOOL
DDS_ContentFilterProperty_is_equal(const struct DDS_ContentFilterProperty *left,
                                    const struct DDS_ContentFilterProperty *right)
{
    RTI_BOOL retval = RTI_FALSE;

    OSAPI_PRECONDITION((left == NULL) || (right == NULL),
                    goto done,
                    OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    if (!DDS_ContentFilterProperty_field_string_is_equal(
                left->content_filtered_topic_name,
                right->content_filtered_topic_name))
    {
        goto done;
    }
    if (!DDS_ContentFilterProperty_field_string_is_equal(
                left->related_topic_name, right->related_topic_name))
    {
        goto done;
    }
    if (!DDS_ContentFilterProperty_field_string_is_equal(
                left->filter_class_name, right->filter_class_name))
    {
        goto done;
    }
    if (!DDS_ContentFilterProperty_field_string_is_equal(
                left->filter_expression, right->filter_expression))
    {
        goto done;
    }
    if (!DDS_StringSeq_is_equal(&left->expression_parameters,
                                &right->expression_parameters))
    {
        goto done;
    }

    retval = RTI_TRUE;
done:
    return retval;
}
