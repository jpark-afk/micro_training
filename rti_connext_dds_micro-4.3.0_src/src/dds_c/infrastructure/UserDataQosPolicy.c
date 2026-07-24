/*
 * FILE: UserDataQosPolicy.c - User Data QoS API implementation
 *
 * (c) Copyright, Real-Time Innovations, 2023-2024.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*ci
 * \brief User Data QoS API implementation
 */

#include "UserDataQosPolicy.h"
#include "osapi/osapi_log.h"

/*** SOURCE_BEGIN ***/

#ifndef RTI_CERT
DDS_ReturnCode_t
DDS_UserDataQosPolicy_finalize(struct DDS_UserDataQosPolicy *policy)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION((policy == NULL), return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("policy", policy, RTI_TRUE);)

    if (!DDS_OctetSeq_finalize(&policy->value))
    {
        goto done;
    }
    retval = DDS_RETCODE_OK;

done:
    return retval;
}
#endif /* !RTI_CERT */

DDS_ReturnCode_t
DDS_UserDataQosPolicy_finalize_no_dealloc(
        struct DDS_UserDataQosPolicy *policy,
        DDS_UserDataManager_T *user_data_manager,
        DDS_UserDataType user_data_type)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION((policy == NULL), return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("policy", policy, RTI_TRUE);)

    if (!REDA_Sequence_has_ownership((struct REDA_Sequence *)&policy->value))
    {
        if (!DDS_UserDataManager_delete_user_data(
                user_data_manager,
                user_data_type,
                &policy->value))
        {
            goto done;
        }

        if (!REDA_Sequence_unloan((struct REDA_Sequence *)&policy->value))
        {
            goto done;
        }
    }
    else
    {
        if(!DDS_OctetSeq_set_length(&policy->value, 0))
        {
            goto done;
        }
    }
    retval = DDS_RETCODE_OK;

done:
    return retval;
}

DDS_ReturnCode_t
DDS_UserDataQosPolicy_initialize(struct DDS_UserDataQosPolicy *policy)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION((policy == NULL), return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("policy", policy, RTI_TRUE);)

    if (!DDS_OctetSeq_initialize(&policy->value))
    {
        goto done;
    }

    retval = DDS_RETCODE_OK;

done:
    return retval;
}

DDS_ReturnCode_t
DDS_UserDataQosPolicy_copy(struct DDS_UserDataQosPolicy *to_policy,
                           const struct DDS_UserDataQosPolicy *from_policy)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION((to_policy == NULL) || (from_policy == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("to_policy",to_policy,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("from_policy",from_policy,RTI_TRUE);)

    if (!DDS_OctetSeq_copy(&to_policy->value, &from_policy->value))
    {
        goto done;
    }

    retval = DDS_RETCODE_OK;

done:
    return retval;
}

RTI_BOOL
DDS_UserDataQosPolicy_is_consistent(const struct DDS_UserDataQosPolicy *policy,
                                    DDS_Long max_length)
{
    RTI_BOOL retval = RTI_FALSE;

    OSAPI_PRECONDITION(policy == NULL,
                       goto done,
                       OSAPI_Log_entry_add_pointer("policy",policy,RTI_TRUE);)

    if (DDS_OctetSeq_get_length(&policy->value) > max_length)
    {
        goto done;
    }

    retval = RTI_TRUE;

done:
    return retval;
}


RTI_BOOL
DDS_UserDataQosPolicy_is_equal(const struct DDS_UserDataQosPolicy *left,
                                   const struct DDS_UserDataQosPolicy *right)
{
    RTI_BOOL retval = RTI_FALSE;

    OSAPI_PRECONDITION((left == NULL) || (right == NULL),
                       goto done,
                       OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    if (!DDS_OctetSeq_is_equal(&left->value, &right->value))
    {
        goto done;
    }

    retval = RTI_TRUE;

done:
    return retval;
}

#ifndef RTI_CERT
DDS_ReturnCode_t
DDS_GroupDataQosPolicy_finalize(struct DDS_GroupDataQosPolicy *policy)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION((policy == NULL), return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("policy", policy, RTI_TRUE);)

    if (!DDS_OctetSeq_finalize(&policy->value))
    {
        goto done;
    }
    retval = DDS_RETCODE_OK;

done:
    return retval;
}
#endif /* !RTI_CERT */

DDS_ReturnCode_t
DDS_GroupDataQosPolicy_finalize_no_dealloc(
        struct DDS_GroupDataQosPolicy *policy,
        DDS_UserDataManager_T *user_data_manager,
        DDS_UserDataType user_data_type)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION((policy == NULL), return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("policy", policy, RTI_TRUE);)

    if (!REDA_Sequence_has_ownership((struct REDA_Sequence *)&policy->value))
    {
        if (!DDS_UserDataManager_delete_user_data(
                user_data_manager,
                user_data_type,
                &policy->value))
        {
            goto done;
        }

        if (!REDA_Sequence_unloan((struct REDA_Sequence *)&policy->value))
        {
            goto done;
        }
    }
    else
    {
        if(!DDS_OctetSeq_set_length(&policy->value, 0))
        {
            goto done;
        }
    }
    retval = DDS_RETCODE_OK;

done:
    return retval;
}

DDS_ReturnCode_t
DDS_GroupDataQosPolicy_initialize(struct DDS_GroupDataQosPolicy *policy)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION((policy == NULL), return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("policy", policy, RTI_TRUE);)

    if (!DDS_OctetSeq_initialize(&policy->value))
    {
        goto done;
    }
    retval = DDS_RETCODE_OK;

done:
    return retval;
}

DDS_ReturnCode_t
DDS_GroupDataQosPolicy_copy(struct DDS_GroupDataQosPolicy *to_policy,
                           const struct DDS_GroupDataQosPolicy *from_policy)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION((to_policy == NULL) || (from_policy == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("to_policy",to_policy,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("from_policy",from_policy,RTI_TRUE);)

    if (!DDS_OctetSeq_copy(&to_policy->value, &from_policy->value))
    {
        goto done;
    }

    retval = DDS_RETCODE_OK;

done:
    return retval;
}

RTI_BOOL
DDS_GroupDataQosPolicy_is_consistent(const struct DDS_GroupDataQosPolicy *policy,
                                    DDS_Long max_length)
{
    RTI_BOOL retval = RTI_FALSE;

    OSAPI_PRECONDITION(policy == NULL,
                       goto done,
                       OSAPI_Log_entry_add_pointer("policy",policy,RTI_TRUE);)

    if (DDS_OctetSeq_get_length(&policy->value) > max_length)
    {
        goto done;
    }

    retval = RTI_TRUE;

done:
    return retval;
}

RTI_BOOL
DDS_GroupDataQosPolicy_is_equal(const struct DDS_GroupDataQosPolicy *left,
                                   const struct DDS_GroupDataQosPolicy *right)
{
    RTI_BOOL retval = RTI_FALSE;

    OSAPI_PRECONDITION((left == NULL) || (right == NULL),
                       goto done,
                       OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    if (!DDS_OctetSeq_is_equal(&left->value, &right->value))
    {
        goto done;
    }

    retval = RTI_TRUE;

done:
    return retval;
}

#ifndef RTI_CERT
DDS_ReturnCode_t
DDS_TopicDataQosPolicy_finalize(struct DDS_TopicDataQosPolicy *policy)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION((policy == NULL), return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("policy", policy, RTI_TRUE);)

    if (!DDS_OctetSeq_finalize(&policy->value))
    {
        goto done;
    }
    retval = DDS_RETCODE_OK;

done:
    return retval;
}
#endif /* !RTI_CERT */

DDS_ReturnCode_t
DDS_TopicDataQosPolicy_finalize_no_dealloc(
        struct DDS_TopicDataQosPolicy *policy,
        DDS_UserDataManager_T *user_data_manager,
        DDS_UserDataType user_data_type)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION((policy == NULL), return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("policy", policy, RTI_TRUE);)

    if (!REDA_Sequence_has_ownership((struct REDA_Sequence *)&policy->value))
    {
        if (!DDS_UserDataManager_delete_user_data(
                user_data_manager,
                user_data_type,
                &policy->value))
        {
            goto done;
        }

        if (!REDA_Sequence_unloan((struct REDA_Sequence *)&policy->value))
        {
            goto done;
        }
    }
    else
    {
        if(!DDS_OctetSeq_set_length(&policy->value, 0))
        {
            goto done;
        }
    }
    retval = DDS_RETCODE_OK;

done:
    return retval;
}

DDS_ReturnCode_t
DDS_TopicDataQosPolicy_initialize(struct DDS_TopicDataQosPolicy *policy)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION((policy == NULL), return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("policy", policy, RTI_TRUE);)

    if (!DDS_OctetSeq_initialize(&policy->value))
    {
        goto done;
    }
    retval = DDS_RETCODE_OK;

done:
    return retval;
}

DDS_ReturnCode_t
DDS_TopicDataQosPolicy_copy(struct DDS_TopicDataQosPolicy *to_policy,
                           const struct DDS_TopicDataQosPolicy *from_policy)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION((to_policy == NULL) || (from_policy == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("to_policy",to_policy,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("from_policy",from_policy,RTI_TRUE);)

    if (!DDS_OctetSeq_copy(&to_policy->value, &from_policy->value))
    {
        goto done;
    }

    retval = DDS_RETCODE_OK;

done:
    return retval;
}

RTI_BOOL
DDS_TopicDataQosPolicy_is_consistent(const struct DDS_TopicDataQosPolicy *policy,
                                    DDS_Long max_length)
{
    RTI_BOOL retval = RTI_FALSE;

    OSAPI_PRECONDITION(policy == NULL,
                       goto done,
                       OSAPI_Log_entry_add_pointer("policy",policy,RTI_TRUE);)

    if (DDS_OctetSeq_get_length(&policy->value) > max_length)
    {
        goto done;
    }

    retval = RTI_TRUE;

done:
    return retval;
}

RTI_BOOL
DDS_TopicDataQosPolicy_is_equal(const struct DDS_TopicDataQosPolicy *left,
                                const struct DDS_TopicDataQosPolicy *right)
{
    RTI_BOOL retval = RTI_FALSE;

    OSAPI_PRECONDITION((left == NULL) || (right == NULL),
                       goto done,
                       OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    if (!DDS_OctetSeq_is_equal(&left->value, &right->value))
    {
        goto done;
    }

    retval = RTI_TRUE;

done:
    return retval;
}
