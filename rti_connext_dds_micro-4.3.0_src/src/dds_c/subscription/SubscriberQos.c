/*
 * FILE: SubscriberQos.c - Subscriber Qos implementation
 *
 * (c) Copyright 2008-2024 Real-Time Innovations,
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
 * 16sep2014,tk MICRO-897/PR#10783 Fixed return value in precondition test
 * 28apr2014,as MICRO-371 (Verocel PR#1527) Removed use of OSAPI_Memory_compare
 *              in DDS_SubscriberQos_is_equal in favor of direct comparison of
 *              qos structure's members.
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief Subscriber Qos implementation
 *
 * \details
 * This file implements functions to manage the SubscriberQos structure
 */
/*ci \addtogroup DDSSubscriptionModule
 * @{
 */
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_domain_h
#include "dds_c/dds_c_domain.h"
#endif

#include "SubscriberImpl.h"
#include "SubscriberQos.h"
#include "QosPolicy.h"
#include "UserDataQosPolicy.h"
#include "PartitionQosPolicy.h"

const struct DDS_SubscriberQos DDS_SUBSCRIBER_QOS_DEFAULT =
                                                DDS_SubscriberQos_INITIALIZER;

/*** SOURCE_BEGIN ***/

DDS_ReturnCode_t
DDS_SubscriberQos_set_from(
        struct DDS_SubscriberQos *out,
        const struct DDS_SubscriberQos *in,
        DDS_Boolean shallow_copy,
        DDS_DomainParticipant *participant)
{
    OSAPI_PRECONDITION((out == NULL) || (in == NULL) ||
                            (shallow_copy && (participant == NULL)),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("out",out,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("in",in,RTI_FALSE);
                        OSAPI_Log_entry_add_uint("shallow_copy",shallow_copy,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("participant",participant,RTI_TRUE);)

    if (shallow_copy)
    {
        if (!DDS_StringSeq_set_maximum(&out->partition.name,
            DDS_StringSeq_get_length(&in->partition.name)))
        {
            return DDS_RETCODE_ERROR;
        }

        if (!DDS_PartitionQosPolicy_set_from(&out->partition,&in->partition,
               DDS_DomainParticipant_get_partition_string_manager(participant)))
        {
            return DDS_RETCODE_ERROR;
        }
    }
    else
    {
        if (!DDS_PartitionQosPolicy_copy(&out->partition,&in->partition))
        {
            return DDS_RETCODE_ERROR;
        }
    }

    out->entity_factory = in->entity_factory;
    out->management = in->management;

    if (shallow_copy)
    {
        if (!DDS_UserDataManager_assert_user_data(
                DDS_DomainParticipant_get_user_data_manager(participant),
                DDS_USER_DATA_SUBSCRIBER_TYPE,
                &in->group_data.value,
                &out->group_data.value))
        {
            return DDS_RETCODE_ERROR;
        }
    }
    else
    {
        if (DDS_GroupDataQosPolicy_copy(&out->group_data,&in->group_data) != DDS_RETCODE_OK)
        {
            return DDS_RETCODE_ERROR;
        }
    }

#if DDS_ENABLE_APPGEN
    out->subscriber_name = in->subscriber_name;
#endif /* DDS_ENABLE_APPGEN */

    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t
DDS_SubscriberQos_copy(struct DDS_SubscriberQos *out,
                       const struct DDS_SubscriberQos *in)
{
    OSAPI_PRECONDITION_ALWAYS(out == NULL || in == NULL,
                            return DDS_RETCODE_BAD_PARAMETER,
                            OSAPI_Log_entry_add_pointer("out",out,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("in",in,RTI_TRUE);)

    return DDS_SubscriberQos_set_from(out, in, DDS_BOOLEAN_FALSE, NULL);
}

DDS_ReturnCode_t
DDS_SubscriberQos_initialize(struct DDS_SubscriberQos *self)
{

    struct DDS_SubscriberQos initVal = DDS_SubscriberQos_INITIALIZER;

    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                            return DDS_RETCODE_BAD_PARAMETER,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    *self = initVal;

    if (DDS_PartitionQosPolicy_initialize(&self->partition) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_GroupDataQosPolicy_initialize(&self->group_data) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}

#ifndef RTI_CERT
DDS_ReturnCode_t
DDS_SubscriberQos_finalize(struct DDS_SubscriberQos *self)
{
    PRECOND_ARG(self)

    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (DDS_PartitionQosPolicy_finalize(&self->partition) == DDS_RETCODE_ERROR)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_GroupDataQosPolicy_finalize(&self->group_data) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t
DDS_SubscriberQos_finalize_managed(
        struct DDS_SubscriberQos *self,
        DDS_DomainParticipant *participant)
{
    PRECOND_ARG(participant)

    OSAPI_PRECONDITION((self == NULL) || (participant == NULL),
                           return DDS_RETCODE_PRECONDITION_NOT_MET,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("participant",participant,RTI_TRUE);)

    if (DDS_PartitionQosPolicy_finalize_no_dealloc(&self->partition,
         DDS_DomainParticipant_get_partition_string_manager(participant))
         != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_GroupDataQosPolicy_finalize_no_dealloc(
            &self->group_data,
            DDS_DomainParticipant_get_user_data_manager(participant),
            DDS_USER_DATA_SUBSCRIBER_TYPE) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_SubscriberQos_finalize(self) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}

#endif /*RTI_CERT*/

DDS_Boolean
DDS_SubscriberQos_is_equal(const struct DDS_SubscriberQos *left,
                           const struct DDS_SubscriberQos *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    return ((left->entity_factory.autoenable_created_entities ==
             right->entity_factory.autoenable_created_entities) &&
            (left->management.is_anonymous == right->management.is_anonymous) &&
            (left->management.is_hidden == right->management.is_hidden)
            && (DDS_PartitionQosPolicy_is_equal(&left->partition,
                                                &right->partition))
            && DDS_GroupDataQosPolicy_is_equal(&left->group_data,
                                                &right->group_data)
#if DDS_ENABLE_APPGEN
            && DDS_EntityNameQosPolicy_is_equal(&left->subscriber_name,
                                                &right->subscriber_name)
#endif /* DDS_ENABLE_APPGEN */

            ) ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/*ci @} */

