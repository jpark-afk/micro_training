/*
 * FILE: TopicQos.c - DDS TopicQos implementation
 *
 * (c) Copyright 2008-2025 Real-Time Innovations,
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
 * 16sep2014,tk MICRO-872 Removed empty DDS_TopicQos_is_consistent
 * 20may2014,tk MICRO-792 Qos consistency checks
 * 06may2012,tk Major update
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \ingroup DDSDomainModule
 * \brief DDS TopicQos implementation
 *
 * \details
 * This file implements functions to manage the TopicQos policy
 */
/*ci \addtogroup DDSDomainModule
 * @{
 */
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_domain_h
#include "dds_c/dds_c_domain.h"
#endif
#include "osapi/osapi_heap.h"
#include "QosPolicy.h"
#include "Topic.h"
#include "TopicQos.h"
#include "UserDataQosPolicy.h"

const struct DDS_TopicQos DDS_TOPIC_QOS_DEFAULT = DDS_TopicQos_INITIALIZER;

/*** SOURCE_BEGIN ***/

DDS_ReturnCode_t
DDS_TopicQos_set_from(
        struct DDS_TopicQos *out,
        const struct DDS_TopicQos *in,
        DDS_Boolean shallow_copy,
        DDS_DomainParticipant *participant)
{
    PRECOND_ARG(shallow_copy)
    PRECOND_ARG(participant)

    OSAPI_PRECONDITION((out == NULL) || (in == NULL) ||
                            (shallow_copy && (participant == NULL)),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("out",out,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("in",in,RTI_FALSE);
                        OSAPI_Log_entry_add_uint("shallow_copy",shallow_copy,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("participant",participant,RTI_TRUE);)

    out->management = in->management;

    if (shallow_copy)
    {
        if (!DDS_UserDataManager_assert_user_data(
                DDS_DomainParticipant_get_user_data_manager(participant),
                DDS_USER_DATA_TOPIC_TYPE,
                &in->topic_data.value,
                &out->topic_data.value))
        {
            return DDS_RETCODE_ERROR;
        }
    }
    else
    {
        if (DDS_TopicDataQosPolicy_copy(&out->topic_data,&in->topic_data) != DDS_RETCODE_OK)
        {
            return DDS_RETCODE_ERROR;
        }
    }

    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t
DDS_TopicQos_copy(struct DDS_TopicQos *out, const struct DDS_TopicQos *in)
{
    OSAPI_PRECONDITION_ALWAYS((out == NULL) || (in == NULL),
                           return DDS_RETCODE_PRECONDITION_NOT_MET,
                           OSAPI_Log_entry_add_pointer("out",out,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("in",in,RTI_TRUE);)

    return DDS_TopicQos_set_from(out, in, DDS_BOOLEAN_FALSE, NULL);
}

DDS_ReturnCode_t
DDS_TopicQos_initialize(struct DDS_TopicQos *self)
{
    struct DDS_TopicQos initVal = DDS_TopicQos_INITIALIZER;

    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                           return DDS_RETCODE_PRECONDITION_NOT_MET,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    *self = initVal;

    if (DDS_TopicDataQosPolicy_initialize(&self->topic_data) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}

#ifndef RTI_CERT
DDS_ReturnCode_t
DDS_TopicQos_finalize(struct DDS_TopicQos * self)
{
    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                           return DDS_RETCODE_PRECONDITION_NOT_MET,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (DDS_TopicDataQosPolicy_finalize(&self->topic_data) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t
DDS_TopicQos_finalize_managed(
        struct DDS_TopicQos *self,
        DDS_DomainParticipant *participant)
{
    PRECOND_ARG(participant)

    OSAPI_PRECONDITION((self == NULL) || (participant == NULL),
                           return DDS_RETCODE_PRECONDITION_NOT_MET,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("participant",participant,RTI_TRUE);)

    if (DDS_TopicDataQosPolicy_finalize_no_dealloc(
            &self->topic_data,
            DDS_DomainParticipant_get_user_data_manager(participant),
            DDS_USER_DATA_TOPIC_TYPE) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_TopicQos_finalize(self) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}
#endif /* !RTI_CERT */

/*ci
 * \brief Check that the immutable part of two topic qos policies are valid
 *
 * \param[in] left  Left side of comparison
 * \param[in] right Right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_TopicQos_immutable_is_equal(const struct DDS_TopicQos *left,
                                const struct DDS_TopicQos *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    if (!RTI_ManagementQosPolicy_is_equal(&left->management,&right->management))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_TopicDataQosPolicy_is_equal(&left->topic_data,&right->topic_data))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

DDS_Boolean
DDS_TopicQos_is_equal(const struct DDS_TopicQos *left,
                      const struct DDS_TopicQos *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    return DDS_TopicQos_immutable_is_equal(left,right);
}

/*ci @} */
