/*
 * FILE: DataWriterQos.c - DataWriter QoS implementation
 *
 * (c) Copyright 2008-2024 Real-Time Innovations
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
 * 10jul2015,tk MICRO-1407/PR#15287 Removed magic constant
 * 30jun2015,tk MICRO-1378/PR#15203 Updated comments
 * 25feb2015,tk MICRO-1083/PR#14046 Removed redundant check
 * 20may2014,tk MICRO-792 Qos consistency checks
 * 23mar23012tk Updated logging
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \ingroup DDSDomainModule
 * \brief DataWriter QoS implementation
 *
 * \details
 * This file implements functions to manage the life-cycle of the datawriter
 * QoS policy as well as support functions.
 */
/*ci \addtogroup DDSPublicationModule
 * @{
 */
#ifndef netio_config_h
#include "netio/netio_config.h"
#endif
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif

#include "QosPolicy.h"
#include "DataWriterImpl.h"
#include "DataWriterQos.h"
#include "UserDataQosPolicy.h"

const struct DDS_DataWriterQos DDS_DATAWRITER_QOS_DEFAULT =
                                                DDS_DataWriterQos_INITIALIZER;

/*** SOURCE_BEGIN ***/

DDS_ReturnCode_t
DDS_DataWriterQos_set_from(
        struct DDS_DataWriterQos *out,
        const struct DDS_DataWriterQos *in,
        DDS_Boolean shallow_copy,
        DDS_DomainParticipant *participant)
{
    OSAPI_PRECONDITION((out == NULL) || (in == NULL) ||
                            (shallow_copy && (participant == NULL)),
                        return DDS_RETCODE_BAD_PARAMETER,
                        OSAPI_Log_entry_add_pointer("out",out,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("in",in,RTI_FALSE);
                        OSAPI_Log_entry_add_uint("shallow_copy",shallow_copy,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("participant",participant,RTI_TRUE);)

    out->deadline = in->deadline;
    out->liveliness = in->liveliness;
    out->history = in->history;
    out->resource_limits = in->resource_limits;
    out->ownership = in->ownership;
    out->ownership_strength = in->ownership_strength;
    out->reliability = in->reliability;
    out->protocol = in->protocol;
    out->type_support = in->type_support;
    out->management = in->management;
    out->durability = in->durability;
    out->writer_resource_limits = in->writer_resource_limits;
    out->destination_order = in->destination_order;
    out->transfer_mode = in->transfer_mode;
    out->latency_budget = in->latency_budget;
    out->transport_priority = in->transport_priority;

#if DDS_ENABLE_APPGEN
    out->publication_name = in->publication_name;
#endif /* DDS_ENABLE_APPGEN */

    /* NOTE: data is an an internal variable, don't copy */

    if (DDS_TransportQosPolicy_copy(&out->transport,&in->transport) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (shallow_copy)
    {
        if (!DDS_UserDataManager_assert_user_data(
                DDS_DomainParticipant_get_user_data_manager(participant),
                DDS_USER_DATA_DATAWRITER_TYPE,
                &in->user_data.value,
                &out->user_data.value))
        {
            return DDS_RETCODE_ERROR;
        }
    }
    else
    {
        if (DDS_UserDataQosPolicy_copy(&out->user_data,&in->user_data) != DDS_RETCODE_OK)
        {
            return DDS_RETCODE_ERROR;
        }
    }

    if (DDS_PropertyQosPolicy_copy(&out->property,&in->property) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_TransportEncapsulationQosPolicy_copy(&out->encapsulation,
                                                 &in->encapsulation) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_DataRepresentationQosPolicy_copy(&out->representation,
                                             &in->representation) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_PublishModeQosPolicy_copy(&out->publish_mode,
                                      &in->publish_mode) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t
DDS_DataWriterQos_copy(struct DDS_DataWriterQos *out,
                       const struct DDS_DataWriterQos *in)
{
    OSAPI_PRECONDITION_ALWAYS((out == NULL) || (in == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("out",out,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("in",in,RTI_TRUE);)

    return DDS_DataWriterQos_set_from(out, in, DDS_BOOLEAN_FALSE, NULL);
}

DDS_ReturnCode_t
DDS_DataWriterQos_initialize(struct DDS_DataWriterQos * self)
{
    struct DDS_DataWriterQos initVal = DDS_DATAWRITER_QOS_DEFAULT;

    OSAPI_PRECONDITION_ALWAYS((self == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    *self = initVal;

    if (DDS_UserDataQosPolicy_initialize(&self->user_data) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}

#ifndef RTI_CERT
DDS_ReturnCode_t
DDS_DataWriterQos_finalize(struct DDS_DataWriterQos *self)
{
    OSAPI_PRECONDITION_ALWAYS((self == NULL),
                       return DDS_RETCODE_BAD_PARAMETER,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (DDS_TransportQosPolicy_finalize(&self->transport) != DDS_RETCODE_OK)
    {
       return DDS_RETCODE_ERROR;
    }

    if (DDS_TransportEncapsulationQosPolicy_finalize(&self->encapsulation) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_DataRepresentationQosPolicy_finalize(&self->representation) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_PublishModeQosPolicy_finalize(&self->publish_mode) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_UserDataQosPolicy_finalize(&self->user_data) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_PropertyQosPolicy_finalize(&self->property) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t
DDS_DataWriterQos_finalize_managed(
    struct DDS_DataWriterQos *self,
    DDS_DomainParticipant *participant)
{
    PRECOND_ARG(participant)

    OSAPI_PRECONDITION((self == NULL) || (participant == NULL),
                       return DDS_RETCODE_BAD_PARAMETER,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);
                       OSAPI_Log_entry_add_pointer("participant",participant,RTI_FALSE);)

    if (DDS_UserDataQosPolicy_finalize_no_dealloc(
            &self->user_data,
            DDS_DomainParticipant_get_user_data_manager(participant),
            DDS_USER_DATA_DATAWRITER_TYPE) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_DataWriterQos_finalize(self) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}
#endif /* !RTI_CERT */

/*ci
 * \brief Check that the immutable part of a datawriter qos policy has changed
 *
 * \param[in] left  Left side of comparison
 * \param[in] right Right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_DataWriterQos_immutable_is_equal(const struct DDS_DataWriterQos *left,
                                     const struct DDS_DataWriterQos *right)
{
    if (!DDS_DeadlineQosPolicy_is_equal(&left->deadline,
                                        &right->deadline) ||
        !DDS_LivelinessQosPolicy_is_equal(&left->liveliness,
                                          &right->liveliness) ||
        !DDS_HistoryQosPolicy_is_equal(&left->history,
                                       &right->history) ||
        !DDS_ResourceLimitsQosPolicy_is_equal(&left->resource_limits,
                                              &right->resource_limits) ||
        !DDS_OwnershipQosPolicy_is_equal(&left->ownership,
                                         &right->ownership) ||
        !DDS_OwnershipStrengthQosPolicy_is_equal(&left->ownership_strength,
                                                 &right->ownership_strength) ||
        !DDS_TypeSupportQosPolicy_is_equal(&left->type_support,
                                           &right->type_support) ||
        !DDS_DataWriterProtocolQosPolicy_is_equal(&left->protocol,
                                                  &right->protocol) ||
        !DDS_LatencyBudgetQosPolicy_is_equal(&left->latency_budget,
                                           &right->latency_budget) ||
        !DDS_ReliabilityQosPolicy_is_equal(&left->reliability,
                                           &right->reliability) ||
        !DDS_DurabilityQosPolicy_is_equal(&left->durability,
                                          &right->durability) ||
        !DDS_DestinationOrderQosPolicy_is_equal(&left->destination_order,
                                                &right->destination_order) ||
        !DDS_TransportQosPolicy_is_equal(&left->transport, &right->transport) ||
        !DDS_TransportEncapsulationQosPolicy_is_equal(&left->encapsulation,
                                                      &right->encapsulation) ||
        !DDS_DataRepresentationQosPolicy_is_equal(&left->representation,
                                                  &right->representation) ||
        !RTI_ManagementQosPolicy_is_equal(&left->management,&right->management) ||
        !DDS_DataWriterResourceLimitsQosPolicy_is_equal(&left->writer_resource_limits,
                                                        &right->writer_resource_limits) ||
        !DDS_UserDataQosPolicy_is_equal(&left->user_data,&right->user_data) ||
#if DDS_ENABLE_APPGEN
        !DDS_EntityNameQosPolicy_is_equal(&left->publication_name,
                                          &right->publication_name) ||
#endif /* DDS_ENABLE_APPGEN */
        !DDS_PublishModeQosPolicy_is_equal(&left->publish_mode,&right->publish_mode) ||
        !DDS_PropertyQosPolicy_is_equal(&left->property,&right->property) ||
        !DDS_TransportPriorityQosPolicy_is_equal(&left->transport_priority,
                                                    &right->transport_priority))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

DDS_Boolean
DDS_DataWriterQos_is_equal(const struct DDS_DataWriterQos *left,
                        const struct DDS_DataWriterQos *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                           return RTI_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    return DDS_DataWriterQos_immutable_is_equal(left,right);
}

/*ci
 * \brief Check that a DataWriter qos is consistent
 *
 * \param[in] self Datawriter qos to check for consistency
 *
 * \return DDS_BOOLEAN_TRUE if consistent, DDS_BOOLEAN_FALSE if not
 */
DDS_Boolean
DDS_DataWriterQos_is_consistent(const struct DDS_DataWriterQos *self)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION((self == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

#if !RTPS_RELIABILITY
    if (self->reliability.kind == DDS_RELIABLE_RELIABILITY_QOS)
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAWRITERQOS_OBJECT,DDSC_LOG_RELIABILITY_QOS_POLICY)
        goto done;
    }
#endif

    if (!DDS_DeadlineQosPolicy_is_consistent(&self->deadline))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAWRITERQOS_OBJECT,DDSC_LOG_DEADLINE_QOS_POLICY)
        goto done;
    }

    if (!DDS_LivelinessQosPolicy_is_consistent(&self->liveliness))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAWRITERQOS_OBJECT,DDSC_LOG_LIVELINESS_QOS_POLICY)
        goto done;
    }

    if (!DDS_HistoryQosPolicy_is_consistent(&self->history))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAWRITERQOS_OBJECT,DDSC_LOG_HISTORY_QOS_POLICY)
        goto done;
    }

    if (!DDS_ResourceLimitsQosPolicy_is_consistent(&self->resource_limits))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAWRITERQOS_OBJECT,DDSC_LOG_RESOURCE_LIMIT_QOS_POLICY)
        goto done;
    }

    if (!DDS_DataWriterProtocolQosPolicy_is_consistent(&self->protocol))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAWRITERQOS_OBJECT,DDSC_LOG_PROTOCOL_QOS_POLICY)
        goto done;
    }

    if (!DDS_ReliabilityQosPolicy_is_consistent(&self->reliability))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAWRITERQOS_OBJECT,DDSC_LOG_RELIABILITY_QOS_POLICY)
        goto done;
    }

    if (!DDS_DurabilityQosPolicy_is_consistent(&self->durability))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAWRITERQOS_OBJECT,DDSC_LOG_DURABILITY_QOS_POLICY)
        goto done;
    }

    if (!DDS_OwnershipQosPolicy_is_consistent(&self->ownership))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAWRITERQOS_OBJECT,DDSC_LOG_OWNERSHIP_QOS_POLICY)
        goto done;
    }

    if (!DDS_OwnershipStrengthQosPolicy_is_consistent
        (&self->ownership_strength))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAWRITERQOS_OBJECT,DDSC_LOG_OWNERSHIP_STRENGTH_QOS_POLICY)

        goto done;
    }

    if (!DDS_DataWriterResourceLimitsQosPolicy_is_consistent(
                                                &self->writer_resource_limits))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAWRITERQOS_OBJECT,DDSC_LOG_DATAWRITER_RESOURCE_QOS_POLICY)

        goto done;
    }

    if (!DDS_DestinationOrderQosPolicy_is_consistent(&self->destination_order))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAWRITERQOS_OBJECT,DDSC_LOG_DESTINATION_ORDER_POLICY);

        goto done;
    }

    if (!DDS_PropertyQosPolicy_is_consistent(&self->property))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAWRITERQOS_OBJECT,DDSC_LOG_PROPERTY_QOS_POLICY);

        goto done;
    }

    if (!DDS_TypeSupportQosPolicy_is_consistent(&self->type_support))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAWRITERQOS_OBJECT,DDSC_LOG_TYPE_SUPPORT_QOS_POLICY)

        goto done;
    }

    if (DDS_DataRepresentationIdSeq_get_length(&self->representation.value) > 1)
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAWRITERQOS_OBJECT,DDSC_LOG_DATA_REPRESENTATION_QOS_POLICY);
        goto done;
    }

    if (!DDS_DataRepresentationQosPolicy_is_consistent(&self->representation))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAWRITERQOS_OBJECT,DDSC_LOG_DATA_REPRESENTATION_QOS_POLICY);
        goto done;
    }

    if (!DDS_PublishModeQosPolicy_is_consistent(&self->publish_mode))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAWRITERQOS_OBJECT,DDSC_LOG_PUBLISH_MODE_QOS_POLICY);
        goto done;
    }

    if (!DDS_DataWriterTransferModeQosPolicy_is_consistent(&self->transfer_mode))
    {
         DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                 DDSC_LOG_DATAWRITERQOS_OBJECT,DDSC_LOG_DATA_REPRESENTATION_QOS_POLICY);
         goto done;
    }

    if (!DDS_LatencyBudgetQosPolicy_is_consistent(&self->latency_budget))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
             DDSC_LOG_DATAWRITERQOS_OBJECT,DDSC_LOG_LATENCY_BUDGET_QOS_POLICY);
        goto done;
    }

    /*---------- Additional inter-policy consistency checking ----------*/

    if ((self->resource_limits.max_samples != DDS_LENGTH_UNLIMITED) &&
        (self->reliability.kind == DDS_RELIABLE_RELIABILITY_QOS &&
         self->protocol.rtps_reliable_writer.heartbeats_per_max_samples >
         self->resource_limits.max_samples))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                                         DDSC_LOG_DATAWRITERQOS_OBJECT,
                                         DDSC_LOG_HEARTBEATS_QOS_POLICY)
        goto done;
    }

    if ((self->history.kind == DDS_KEEP_LAST_HISTORY_QOS) &&
        (self->history.depth > self->resource_limits.max_samples_per_instance))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICIES(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAWRITERQOS_OBJECT,DDSC_LOG_HISTORY_QOS_POLICY,
                DDSC_LOG_RESOURCE_LIMIT_QOS_POLICY)

        goto done;
    }

    /* A user can specify objectids < OSAPI_SYSTEM_OBJECTID_START.
     * Objectid >= OSAPI_SYSTEM_OBJECTID_START are generated by the
     * OSAPI_System_next_object_id() function.
     */
    if (self->protocol.rtps_object_id >= OSAPI_SYSTEM_OBJECTID_START)
    {
        DDSC_LOG_ILLEGAL_OBJECTID(OSAPI_LOGKIND_ERROR,
                                  self->protocol.rtps_object_id)
        goto done;
    }

    retval = DDS_BOOLEAN_TRUE;

done:

    return retval;
}

/*ci @} */
