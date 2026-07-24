/*
 * FILE: ContentFilterQosPolicy.c - Content Filter QoS API implementation
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
 * \brief Content Filter QoS API implementation
 */

#include "dds_c/dds_c_content_filter.h"
#include "dds_c/dds_c_domain.h"
#include "osapi/osapi_log.h"

/*** SOURCE_BEGIN ***/

#ifndef RTI_CERT
DDS_ReturnCode_t
DDS_ContentFilterQosPolicy_finalize(struct DDS_ContentFilterQosPolicy *policy)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION((policy == NULL), goto done,
            OSAPI_Log_entry_add_pointer("policy", policy, RTI_TRUE);)

    DDS_String_free(policy->filter_name);
    policy->filter_name = NULL;

    DDS_String_free(policy->filter_class_name);
    policy->filter_class_name = NULL;

    DDS_String_free(policy->filter_expression);
    policy->filter_expression = NULL;

    if (!DDS_StringSeq_finalize(&policy->expression_parameters))
    {
        goto done;
    }

    retval = DDS_RETCODE_OK;
done:
    return retval;
}
#endif /* !RTI_CERT */

DDS_ReturnCode_t
DDS_ContentFilterQosPolicy_initialize(struct DDS_ContentFilterQosPolicy *policy)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION((policy == NULL), goto done,
            OSAPI_Log_entry_add_pointer("policy", policy, RTI_TRUE);)

    *policy = (struct DDS_ContentFilterQosPolicy)DDS_CONTENT_FILTER_QOS_POLICY_DEFAULT;

    if (!DDS_StringSeq_initialize(&policy->expression_parameters))
    {
        goto done;
    }

    retval = DDS_RETCODE_OK;
done:
    return retval;
}

DDS_ReturnCode_t
DDS_ContentFilterQosPolicy_copy(struct DDS_ContentFilterQosPolicy *to_policy,
                                const struct DDS_ContentFilterQosPolicy *from_policy)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION((to_policy == NULL) || (from_policy == NULL),
                       return DDS_RETCODE_PRECONDITION_NOT_MET,
                       OSAPI_Log_entry_add_pointer("to_policy", to_policy, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("from_policy", from_policy, RTI_TRUE);)

    if (!REDA_String_copy_w_max(
            &to_policy->filter_name,
            from_policy->filter_name,
            DDS_CONTENT_FILTER_NAME_MAX_LENGTH))
    {
        goto done;
    }

    if (!REDA_String_copy_w_max(
            &to_policy->filter_class_name,
            from_policy->filter_class_name,
            DDS_CONTENT_FILTER_CLASS_NAME_MAX_LENGTH))
    {
        goto done;
    }

    if (!REDA_String_copy_w_max(
            &to_policy->filter_expression,
            from_policy->filter_expression,
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
    if (!DDS_StringSeq_copy(&to_policy->expression_parameters,
                            &from_policy->expression_parameters))
    {
        goto done;
    }

    retval = DDS_RETCODE_OK;

done:
    return retval;
}

RTI_BOOL
DDS_ContentFilterQosPolicy_is_consistent(
        const struct DDS_ContentFilterQosPolicy *policy,
        struct DDS_FilterResourceLimits *resource_limits)
{
    RTI_BOOL retval = RTI_FALSE;
    RTI_INT32 parameter_count;
    DDS_String *parameter;

    OSAPI_PRECONDITION(policy == NULL,
                    goto done,
                    OSAPI_Log_entry_add_pointer("policy",policy,RTI_TRUE);)

    parameter_count = DDS_StringSeq_get_length(&policy->expression_parameters);

    if ((policy->filter_name == NULL)
            || (DDS_String_length(policy->filter_name) == 0))
    {
        if ((policy->filter_class_name != NULL)
                && (DDS_String_length(policy->filter_class_name) != 0))
        {
            goto done;
        }
        if ((policy->filter_expression != NULL)
                && (DDS_String_length(policy->filter_expression) != 0))
        {
            goto done;
        }
        if (parameter_count != 0)
        {
            goto done;
        }
    }
    else
    {
        if ((policy->filter_class_name == NULL)
                || (DDS_String_length(policy->filter_class_name) == 0))
        {
            goto done;
        }
        if ((policy->filter_expression == NULL)
                || (DDS_String_length(policy->filter_expression) == 0))
        {
            goto done;
        }

        if ((DDS_String_length(policy->filter_class_name)
                        > (RTI_SIZE_T)resource_limits->filter_class_max_length)
                || (DDS_String_length(policy->filter_expression)
                        > (RTI_SIZE_T)resource_limits->filter_expression_max_length)
                || (parameter_count
                        > resource_limits->filter_parameter_max_count_per_expression))
        {
            goto done;
        }

        for (RTI_INT32 i = 0; i < parameter_count; ++i)
        {
            parameter = DDS_StringSeq_get_reference(&policy->expression_parameters, i);
            if ((parameter == NULL) || (*parameter == NULL))
            {
                goto done;
            }
            if (DDS_String_length(*parameter)
                    > (RTI_SIZE_T)resource_limits->filter_parameter_max_length)
            {
                goto done;
            }
        }
    }

    retval = RTI_TRUE;

done:
    return retval;
}


RTI_BOOL
DDS_ContentFilterQosPolicy_is_equal(const struct DDS_ContentFilterQosPolicy *left,
                                    const struct DDS_ContentFilterQosPolicy *right)
{
    RTI_BOOL retval = RTI_FALSE;

    OSAPI_PRECONDITION((left == NULL) || (right == NULL),
                    goto done,
                    OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    if (DDS_String_cmp(left->filter_name, right->filter_name) != 0)
    {
        goto done;
    }
    if (DDS_String_cmp(left->filter_class_name, right->filter_class_name) != 0)
    {
        goto done;
    }
    if (DDS_String_cmp(left->filter_expression, right->filter_expression) != 0)
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
