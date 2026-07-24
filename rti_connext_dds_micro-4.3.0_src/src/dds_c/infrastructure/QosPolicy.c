/*
 * FILE: QosPolicy.c - QoSPolicy implementation
 *
 * Copyright (c) 2008-2025 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 28jul2015,tk MICRO-1471/PR#15633 Added robustness check that deadline and
 *                                  liveliness durations are normalized
 * 20jul2015,tk MICRO-1452/PR#15565 Check deadline.period for INFINITE, not the
 *                                  sample_freq period
 * 20jul2015,tk MICRO-1445/PR#15492 Allow either matching_reader_writer_pair_allocation
 *                                  or matching_writer_reader_pair_allocation to be
 *                                  0. but not both
 * 20jul2015,tk MICRO-1426/PR#15358 Added Deadline_get_sample_freq()
 * 13jul2015,tk MICRO-1421/PR#15309 Added max_components to
 *                              SystemResourceLimitsQosPolicy_is_consistent
 *                              and
 *                              SystemResourceLimitsQosPolicy_immutable_is_equal
 * 20feb2015,eh MICRO-813/PR#9172 Fix Lint warnings
 * 20may2014,tk MICRO-792: Qos consistency checks
 * 15may2014,as MICRO-317 (Verocel PR#1443) Replaced String_cmp with String_ncmp
 * 10mar2014,eh MICRO-726: AUTOMATIC or MANUAL_BY_PARTICIPANT liveliness allowed
 *              with infinite duration
 * 19jul2013,as Added support for C++ (added functions to each QosPolicy type)
 * 30apr2008,tk Created
 */
/*ci
 * \file
 * \brief QoSPolicy implementation
 *
 * \details
 * This file implements functions related to individual Qos policies, such
 * as comparisons.
 */
/*ci \addtogroup DDSInfrastructureModule
 * @{
 */
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif

#include "QosPolicy.h"
#include "RtpsWellKnownPorts.h"

const struct DDS_PresentationQosPolicy DDS_PRESENTATION_QOS_PUBLICATION_DEFAULT =
                        DDS_PRESENTATION_QOS_POLICY_PUBLICATION_DEFAULT;

const struct DDS_GroupDataQosPolicy DDS_GROUP_DATA_QOS_DEFAULT =
                                DDS_GROUP_DATA_QOS_POLICY_DEFAULT;

const struct DDS_PartitionQosPolicy DDS_PARTITION_QOS_DEFAULT =
                                DDS_PARTITION_QOS_POLICY_DEFAULT;

const struct DDS_DataRepresentationQosPolicy DDS_DATAREPRESENTATION_DEFAULT =
                                    DDS_DATA_REPRESENTATION_QOS_POLICY_DEFAULT;

const struct DDS_TransportEncapsulationQosPolicy DDS_TRANSPORT_ENCAPSULATION_DEFAULT =
                                DDS_TRANSPORT_ENCAPSULATION_QOS_POLICY_DEFAULT;

const struct DDS_PublishModeQosPolicy DDS_PUBLISHMODE_DEFAULT =
                                DDS_PUBLISH_MODE_QOS_POLICY_DEFAULT;

const struct DDS_UserDataQosPolicy  DDS_USER_DATA_DEFAULT =
                                DDS_USER_DATA_QOS_POLICY_DEFAULT;

const struct DDS_LatencyBudgetQosPolicy DDS_LATENCY_BUDGET_DEFAULT =
                                 DDS_LATENCY_BUDGET_QOS_POLICY_DEFAULT;

const struct DDS_TopicDataQosPolicy DDS_TOPIC_DATA_QOS_DEFAULT =
                                 DDS_TOPIC_DATA_QOS_POLICY_DEFAULT;

const struct DDS_EntityFactoryQosPolicy DDS_ENTITY_FACTORY_QOS_DEFAULT =
                                DDS_ENTITY_FACTORY_QOS_POLICY_DEFAULT;

const struct DDS_DiscoveryQosPolicy DDS_DISCOVERY_QOS_DEFAULT =
                                DDS_DISCOVERY_QOS_POLICY_DEFAULT;

const struct DDS_DomainParticipantResourceLimitsQosPolicy
        DDS_DOMAIN_PARTICIPANT_RESOURCE_LIMITS_QOS_DEFAULT =
                    DDS_DOMAIN_PARTICIPANT_RESOURCE_LIMITS_QOS_POLICY_DEFAULT;

const struct DDS_WireProtocolQosPolicy DDS_WIRE_PROTOCOL_QOS_DEFAULT =
                    DDS_WIRE_PROTOCOL_QOS_POLICY_DEFAULT;

const struct DDS_TransportQosPolicy DDS_TRANSPORT_QOS_DEFAULT =
                    DDS_TRANSPORT_QOS_POLICY_DEFAULT;

const struct DDS_UserTrafficQosPolicy DDS_USERTRAFFIC_QOS_DEFAULT =
                    DDS_USERTRAFFIC_QOS_POLICY_DEFAULT;

const struct DDS_PropertyQosPolicy DDS_PROPERTY_QOS_DEFAULT =
                    DDS_PROPERTY_QOS_POLICY_DEFAULT;

const struct DDS_UserDataQosPolicy DDS_USER_DATA_QOS_DEFAULT =
                    DDS_USER_DATA_QOS_POLICY_DEFAULT;

const struct DDS_TransportPriorityQosPolicy DDS_TRANSPORT_PRIORITY_DEFAULT =
                    DDS_TRANSPORT_PRIORITY_QOS_POLICY_DEFAULT;

const char DDS_ENTITY_NAME_DEFAULT[1] = "";

#if DDS_FILTERING_ENABLED
const struct DDS_ContentFilterQosPolicy DDS_CONTENT_FILTER_DEFAULT =
                                DDS_CONTENT_FILTER_QOS_POLICY_DEFAULT;
#endif /* DDS_FILTERING_ENABLED */

/*** SOURCE_BEGIN ***/

int
DDS_SequenceNumber_compare(const struct DDS_SequenceNumber_t *sn1,
                           const struct DDS_SequenceNumber_t *sn2)
{
    return ((((sn1)->high) > ((sn2)->high)) ? 1 :
            ((((sn1)->high) < ((sn2)->high)) ? -1 :
                    ((((sn1)->low) > ((sn2)->low)) ? 1 :
                            ((((sn1)->low) < ((sn2)->low)) ? -1 : 0))));
}

/*************************
    DDS_DurabilityQosPolicy
*************************/

/*ci
 * \brief Test if two DDS_DurabilityQosPolicy policies are equal
 *
 * \param[in] left side of comparison
 * \param[in] right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_DurabilityQosPolicy_is_equal(const struct DDS_DurabilityQosPolicy *left,
                                 const struct DDS_DurabilityQosPolicy *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    return (left->kind == right->kind) ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}


/*ci
 * \brief Check that a DDS_DurabilityQosPolicy policy has legal values
 *
 * \param[in] self Durability structure to be validated
 *
 * \return DDS_BOOLEAN_TRUE if the content is valid, DDS_BOOLEAN_FALSE if not
 */
DDS_Boolean
DDS_DurabilityQosPolicy_is_consistent(const struct DDS_DurabilityQosPolicy *self)
{
    if ((self->kind == DDS_VOLATILE_DURABILITY_QOS) ||
        (self->kind == DDS_TRANSIENT_LOCAL_DURABILITY_QOS))
    {
        return DDS_BOOLEAN_TRUE;
    }

    return DDS_BOOLEAN_FALSE;
}

/*ci
 * \brief Check if a requested DDS_DurabilityQosPolicy is compatible with the
 *        offered DDS_DurabilityQosPolicy
 *
 * \details
 *
 * The compatibility check is according to the DDS specification
 *
 * \param[in] request The requested durability
 * \param[in] offered The offered durability
 *
 * \return DDS_BOOLEAN_TRUE if the requested policy is compatible with the
 *         offered DDS_DurabilityQosPolicy, DDS_BOOLEAN_FALSE if not
 */
DDS_Boolean
DDS_DurabilityQosPolicy_is_compatible(const struct DDS_DurabilityQosPolicy *request,
                                      const struct DDS_DurabilityQosPolicy *offered)
{
    return (request->kind <= offered->kind) ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/*******************************
 * DDS_DestinationOrderQosPolicy
 *******************************/

/*ci
 * \brief Test if two DDS_DestinationOrderQosPolicy policies are equal
 *
 * \param[in] left side of comparison
 * \param[in] right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_DestinationOrderQosPolicy_is_equal(
                         const struct DDS_DestinationOrderQosPolicy *left,
                         const struct DDS_DestinationOrderQosPolicy *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    if (left->kind != right->kind)
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_Duration_equal(&left->source_timestamp_tolerance,
                              &right->source_timestamp_tolerance);
}

/*ci
 * \brief Check that a DDS_DestinationOrderQosPolicy policy has legal values
 *
 * \param[in] self DDS_DestinationOrderQosPolicy structure to be validated
 *
 * \return DDS_BOOLEAN_TRUE if the content is valid, DDS_BOOLEAN_FALSE if not
 */
DDS_Boolean
DDS_DestinationOrderQosPolicy_is_consistent(
                            const struct DDS_DestinationOrderQosPolicy *self)
{
    if ((self->kind != DDS_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS) &&
        (self->kind != DDS_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_Duration_is_normalized(&self->source_timestamp_tolerance))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_Duration_is_infinite(&self->source_timestamp_tolerance) &&
        ((DDS_Duration_compare(&self->source_timestamp_tolerance,&DDS_DURATION_YEAR) > 0) ||
          DDS_Duration_compare(&self->source_timestamp_tolerance,&DDS_DURATION_ZERO) <= 0))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Check if a requested DDS_DurabilityQosPolicy is compatible with the
 *        offered DDS_DurabilityQosPolicy
 *
 * \details
 *
 * The compatibility check is according to the DDS specification
 *
 * \param[in] request The requested durability
 * \param[in] offered The offered durability
 *
 * \return DDS_BOOLEAN_TRUE if the requested policy is compatible with the
 *         offered DDS_DurabilityQosPolicy, DDS_BOOLEAN_FALSE if not
 */
DDS_Boolean
DDS_DestinationOrderQosPolicy_is_compatible(
                            const struct DDS_DestinationOrderQosPolicy *request,
                            const struct DDS_DestinationOrderQosPolicy *offered)
{
    return (request->kind <= offered->kind) ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/*******************************
 * DDS_PresentationQosPolicy
 *******************************/
/*ci
 * \brief Check if a requested DDS_PresentationQosPolicy is compatible with the
 *        offered DDS_PresentationQosPolicy
 *
 * \details
 *
 * The compatibility check is according to the DDS specification
 *
 * \param[in] request The requested presentation
 * \param[in] offered The offered presentation
 *
 * \return DDS_BOOLEAN_TRUE if the requested policy is compatible with the
 *         offered DDS_PresentationQosPolicy, DDS_BOOLEAN_FALSE if not
 */
DDS_Boolean
DDS_PresentationQosPolicy_is_compatible(
                            const struct DDS_PresentationQosPolicy *request,
                            const struct DDS_PresentationQosPolicy *offered)
{
    if (((request->access_scope != DDS_HIGHEST_OFFERED_PRESENTATION_QOS) &&
         (offered->access_scope < request->access_scope)) ||
        ((offered->coherent_access == RTI_FALSE) &&
         (request->coherent_access == RTI_TRUE)) ||
        ((offered->ordered_access == RTI_FALSE) &&
         (request->ordered_access == RTI_TRUE)))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}


/*************************
    DDS_DeadlineQosPolicy
*************************/

/*ci
 * \brief Test if two DDS_DeadlineQosPolicy policies are equal
 *
 * \param[in] left side of comparison
 * \param[in] right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_DeadlineQosPolicy_is_equal(const struct DDS_DeadlineQosPolicy *left,
                               const struct DDS_DeadlineQosPolicy *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    return DDS_Duration_compare(&left->period, &right->period) == 0 ?
                                DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/*ci
 * \brief Check that a DDS_DeadlineQosPolicy policy has consistent, legal values
 *
 * \param[in] self DDS_DeadlineQosPolicy structure to be validated
 *
 * \return DDS_BOOLEAN_TRUE if the content is valid, DDS_BOOLEAN_FALSE if not
 */
DDS_Boolean
DDS_DeadlineQosPolicy_is_consistent(const struct DDS_DeadlineQosPolicy *self)
{
    OSAPI_PRECONDITION(self == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (!DDS_Duration_is_normalized(&self->period))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if ((DDS_Duration_compare(&self->period, &DDS_DURATION_NANOSEC) < 0
         || DDS_Duration_compare(&self->period, &DDS_DURATION_YEAR) > 0)
        && DDS_Duration_compare(&self->period, &DDS_DURATION_INFINITE) != 0)
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Check if a requested DDS_DeadlineQosPolicy is compatible with the
 *        offered DDS_DeadlineQosPolicy
 *
 * \details
 *
 * The compatibility check is according to the DDS specification
 *
 * \param[in] request The requested deadline
 * \param[in] offered The offered deadline
 *
 * \return DDS_BOOLEAN_TRUE if the requested policy is compatible with the
 *         offered DDS_DeadlineQosPolicy, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_DeadlineQosPolicy_is_compatible(const struct DDS_DeadlineQosPolicy *request,
                                    const struct DDS_DeadlineQosPolicy *offered)
{
    if (DDS_Duration_compare(&request->period,
                             &DDS_DURATION_INFINITE) &&
        (DDS_Duration_compare(&request->period, &offered->period) < 0))
    {
        return DDS_BOOLEAN_FALSE;
    }
    return DDS_BOOLEAN_TRUE;
}

/* documented in dds_c_infrastructure.h */
void
DDS_DeadlineQosPolicy_get_sample_freq(
                            const struct DDS_DeadlineQosPolicy *const deadline,
                            struct DDS_Duration_t *const sample_freq)
{
    /* The sampling rate is hard-coded to 8x the deadline, with a maximum
     * detection time of 10s.
     */
    const RTI_INT32 sample_rate = 8;
    RTI_INT32 rate = 1;

    if (DDS_Duration_is_infinite(&deadline->period))
    {
        *sample_freq = DDS_DURATION_INFINITE;
        return;
    }

    *sample_freq = deadline->period;

    for (rate = 1; rate < sample_rate; rate = rate << 1)
    {
        if (sample_freq->sec & 1)
        {
            /* This is safe. The largest value, assuming the deadline is
             * valid, is 999999999. Adding 1000000000 will still fit in an
             * unsigned RTI_UINT32.
             */
            sample_freq->nanosec += 1000000000;
        }

        sample_freq->sec >>= 1;
        sample_freq->nanosec >>= 1;
    }

    /* Make sure the sampling is at least every 10s if the calculation is
     * less than 10s and 1ns if the calculation is zero.
     */
    if ((sample_freq->sec > 10) ||
        ((sample_freq->sec == 10) && (sample_freq->nanosec > 0)))
    {
        sample_freq->sec = 10;
        sample_freq->nanosec = 0;
    }
    else if (DDS_Duration_is_zero(sample_freq))
    {
        sample_freq->nanosec = 1;
    }
}

/*************************
    DDS_HistoryQosPolicy
*************************/

/*ci
 * \brief Test if two DDS_HistoryQosPolicy policies are equal
 *
 * \param[in] left side of comparison
 * \param[in] right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_HistoryQosPolicy_is_equal(const struct DDS_HistoryQosPolicy *left,
                                 const struct DDS_HistoryQosPolicy *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    return ((left->depth == right->depth) && (left->kind == right->kind)) ?
                        DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/*ci
 * \brief Check that a DDS_HistoryQosPolicy policy has legal values
 *
 * \param[in] self DDS_HistoryQosPolicy structure to be validated
 *
 * \return DDS_BOOLEAN_TRUE if the content is valid, DDS_BOOLEAN_FALSE if not
 */
DDS_Boolean
DDS_HistoryQosPolicy_is_consistent(const struct DDS_HistoryQosPolicy *self)
{
    OSAPI_PRECONDITION(self == NULL,
                            return DDS_BOOLEAN_FALSE,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if ((self->kind != DDS_KEEP_LAST_HISTORY_QOS) &&
        (self->kind != DDS_KEEP_ALL_HISTORY_QOS))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if ((self->kind == DDS_KEEP_LAST_HISTORY_QOS) && (self->depth <= 0))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

/******************************************
    DDS_SystemResourceLimitsQosPolicy
******************************************/
/*ci
 * \brief Test if two DDS_SystemResourceLimitsQosPolicy policies are equal for
 *        the immutable part only
 *
 * \param[in] left side of comparison
 * \param[in] right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_SystemResourceLimitsQosPolicy_immutable_is_equal(
                    const struct DDS_SystemResourceLimitsQosPolicy *left,
                    const struct DDS_SystemResourceLimitsQosPolicy *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                            return DDS_BOOLEAN_FALSE,
                            OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    return ((left->max_participants == right->max_participants) &&
            (left->max_components == right->max_components)) ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/*ci
 * \brief Check that a DDS_SystemResourceLimitsQosPolicy policy has consistent,
 *        legal values
 *
 * \param[in] self DDS_SystemResourceLimitsQosPolicy structure to be validated
 *
 * \return DDS_BOOLEAN_TRUE if the content is valid, DDS_BOOLEAN_FALSE if not
 */
DDS_Boolean
DDS_SystemResourceLimitsQosPolicy_is_consistent(
                    const struct DDS_SystemResourceLimitsQosPolicy *self)
{
    OSAPI_PRECONDITION(self == NULL,
                            return DDS_BOOLEAN_FALSE,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return ((self->max_participants > 0) && (self->max_components > 0)) ?
                    DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}


/**************************************************
    DDS_ResourceLimitsQosPolicy
**************************************************/
/*ci
 * \brief Test if two DDS_ResourceLimitsQosPolicy policies are equal
 *
 * \param[in] left side of comparison
 * \param[in] right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_ResourceLimitsQosPolicy_is_equal(
                            const struct DDS_ResourceLimitsQosPolicy *left,
                            const struct DDS_ResourceLimitsQosPolicy *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    return ((left->max_instances == right->max_instances &&
            left->max_samples == right->max_samples &&
            left->max_samples_per_instance == right->max_samples_per_instance)) ?
            DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/*ci
 * \brief Check that a DDS_ResourceLimitsQosPolicy policy has legal values
 *
 * \param[in] self DDS_ResourceLimitsQosPolicy structure to be validated
 *
 * \return DDS_BOOLEAN_TRUE if the content is valid, DDS_BOOLEAN_FALSE if not
 */
DDS_Boolean
DDS_ResourceLimitsQosPolicy_is_consistent(
                                const struct DDS_ResourceLimitsQosPolicy *self)
{
    OSAPI_PRECONDITION(self == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (((self->max_samples < 1) || (self->max_samples > 100000000)) ||
        ((self->max_instances < 1) || (self->max_instances > 1000000)) ||
        ((self->max_samples_per_instance != DDS_LENGTH_UNLIMITED) &&
         ((self->max_samples_per_instance < 1) ||
          (self->max_samples_per_instance > 100000000) ||
          (self->max_samples < self->max_samples_per_instance))))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}


/**************************************************
    DDS_OwnershipQosPolicy
**************************************************/
/*ci
 * \brief Test if two DDS_OwnershipQosPolicy policies are equal
 *
 * \param[in] left side of comparison
 * \param[in] right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_OwnershipQosPolicy_is_equal(const struct DDS_OwnershipQosPolicy *left,
                                const struct DDS_OwnershipQosPolicy *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                            return DDS_BOOLEAN_FALSE,
                            OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    return (left->kind == right->kind) ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/*ci
 * \brief Check that a DDS_OwnershipQosPolicy policy has legal values
 *
 * \param[in] self DDS_OwnershipQosPolicy structure to be validated
 *
 * \return DDS_BOOLEAN_TRUE if the content is valid, DDS_BOOLEAN_FALSE if not
 */
DDS_Boolean
DDS_OwnershipQosPolicy_is_consistent(const struct DDS_OwnershipQosPolicy *self)
{
    OSAPI_PRECONDITION(self == NULL,
                            return DDS_BOOLEAN_FALSE,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return ((self->kind == DDS_SHARED_OWNERSHIP_QOS) ||
            (self->kind == DDS_EXCLUSIVE_OWNERSHIP_QOS)) ?
            DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/*ci
 * \brief Check if a requested DDS_OwnershipQosPolicy is compatible with the
 *        offered DDS_OwnershipQosPolicy
 *
 * \details
 *
 * The compatibility check is according to the DDS specification
 *
 * \param[in] request The requested ownership
 * \param[in] offered The offered ownership
 *
 * \return DDS_BOOLEAN_TRUE if the requested policy is compatible with the
 *         offered DDS_OwnershipQosPolicy
 */
DDS_Boolean
DDS_OwnershipQosPolicy_is_compatible(const struct DDS_OwnershipQosPolicy *request,
                                     const struct DDS_OwnershipQosPolicy *offered)
{
    return (request->kind == offered->kind) ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/**************************************************
    DDS_OwnershipStrengthQosPolicy
**************************************************/
/*ci
 * \brief Test if two DDS_OwnershipStrengthQosPolicy policies are equal
 *
 * \param[in] left side of comparison
 * \param[in] right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_OwnershipStrengthQosPolicy_is_equal(
        const struct DDS_OwnershipStrengthQosPolicy *left,
        const struct DDS_OwnershipStrengthQosPolicy *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                            return DDS_BOOLEAN_FALSE,
                            OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    return (left->value == right->value) ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/*ci
 * \brief Check that a DDS_OwnershipStrengthQosPolicy policy has consistent,
 *        legal values
 *
 * \param[in] self DDS_OwnershipStrengthQosPolicy structure to be validated
 *
 * \return DDS_BOOLEAN_TRUE if the content is valid, DDS_BOOLEAN_FALSE if not
 */
DDS_Boolean
DDS_OwnershipStrengthQosPolicy_is_consistent(
                            const struct DDS_OwnershipStrengthQosPolicy *self)
{
    OSAPI_PRECONDITION(self == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE););

    if ((self->value >= 0) && (self->value <= 1000000))
    {
        return DDS_BOOLEAN_TRUE;
    }

    return DDS_BOOLEAN_FALSE;
}

/**************************************************
    DDS_LivelinessQosPolicy
**************************************************/
/*ci
 * \brief Test if two DDS_LivelinessQosPolicy policies are equal
 *
 * \param[in] left side of comparison
 * \param[in] right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_LivelinessQosPolicy_is_equal(const struct DDS_LivelinessQosPolicy *left,
                                 const struct DDS_LivelinessQosPolicy *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    return ((DDS_Duration_compare(&left->lease_duration,
                                  &right->lease_duration) == 0) &&
            (left->kind == right->kind)) ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/*ci
 * \brief Check that a DDS_LivelinessQosPolicy policy has consistent,
 *        legal values
 *
 * \details
 * This function check for liveliness compatibility. Note that we
 * ignore the kind if the duration is infinite since it does not matter.
 *
 * \param[in] self DDS_LivelinessQosPolicy structure to be validated
 *
 * \return DDS_BOOLEAN_TRUE if the content is valid, DDS_BOOLEAN_FALSE if not
 */
DDS_Boolean
DDS_LivelinessQosPolicy_is_consistent(
                                    const struct DDS_LivelinessQosPolicy *self)
{
    OSAPI_PRECONDITION(self == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (!DDS_Duration_is_normalized(&self->lease_duration))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if ((self->kind != DDS_MANUAL_BY_TOPIC_LIVELINESS_QOS) &&
        (self->kind != DDS_AUTOMATIC_LIVELINESS_QOS) &&
        (self->kind != DDS_MANUAL_BY_PARTICIPANT_LIVELINESS_QOS))
    {
        return DDS_BOOLEAN_FALSE;
    }

    /* In case P2P Participant messages channel is enabled all
     * liveliness kinds support finite lease duration. If the
     * channel is not enabled only MANUAL_BY_TOPIC supports
     * finite lease duration
     */
#if !DDS_LIVELINESS_CHANNEL_ENABLED
    /* Allow AUTOMATIC_BY_PARTICIPANT or MANUAL_BY_PARTICIPANT only if
     * lease_duration is INFINITE
     */
    if ((self->kind != DDS_MANUAL_BY_TOPIC_LIVELINESS_QOS) &&
        DDS_Duration_compare(&self->lease_duration, &DDS_DURATION_INFINITE))
    {
        return DDS_BOOLEAN_FALSE;
    }
#endif /* !DDS_LIVELINESS_CHANNEL_ENABLED */

    if ((DDS_Duration_compare(&self->lease_duration, &DDS_DURATION_ZERO) < 0 ||
         DDS_Duration_compare(&self->lease_duration, &DDS_DURATION_YEAR) > 0) &&
         DDS_Duration_compare(&self->lease_duration, &DDS_DURATION_INFINITE))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

DDS_Boolean
DDS_LivelinessQosPolicy_is_ipc_liveliness_required(const struct DDS_LivelinessQosPolicy *const self)
{
    if (((self->kind == DDS_AUTOMATIC_LIVELINESS_QOS) ||
         (self->kind == DDS_MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)) &&
          !DDS_Duration_is_infinite(&self->lease_duration))
    {
        return DDS_BOOLEAN_TRUE;
    }
    return DDS_BOOLEAN_FALSE;
}

/*ci
 * \brief Check if a requested DDS_LivelinessQosPolicy is compatible with the
 *        offered DDS_LivelinessQosPolicy
 *
 * \details
 *
 * The compatibility check is according to the DDS specification. Note that
 * we ignore the kind if the duration is infinite since it does not matter.
 *
 * \param[in] request The requested liveliness
 * \param[in] offered The offered liveliness
 *
 * \return DDS_BOOLEAN_TRUE if the requested policy is compatible with the
 *         offered DDS_LivelinessQosPolicy, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_LivelinessQosPolicy_is_compatible(const struct DDS_LivelinessQosPolicy *request,
                                      const struct DDS_LivelinessQosPolicy *offered)
{
    return ((request->kind <= offered->kind) &&
            (DDS_Duration_compare(&request->lease_duration,
                                  &offered->lease_duration) >= 0)) ?
             DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/**************************************************
    DDS_ReliabilityQosPolicy
**************************************************/
/*ci
 * \brief Test if two DDS_ReliabilityQosPolicy policies are equal
 *
 * \param[in] left side of comparison
 * \param[in] right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_ReliabilityQosPolicy_is_equal(const struct DDS_ReliabilityQosPolicy *left,
                                  const struct DDS_ReliabilityQosPolicy *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    return ((left->kind == right->kind)
            && (DDS_Duration_compare(&left->max_blocking_time,
                                     &right->max_blocking_time) == 0)) ?
            DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/*ci
 * \brief Check that a DDS_ReliabilityQosPolicy policy has consistent,
 *        legal values
 *
 * \param[in] self DDS_ReliabilityQosPolicy structure to be validated
 *
 * \return DDS_BOOLEAN_TRUE if the content is valid, DDS_BOOLEAN_FALSE if not
 */
DDS_Boolean
DDS_ReliabilityQosPolicy_is_consistent(
                                   const struct DDS_ReliabilityQosPolicy *self)
{
    OSAPI_PRECONDITION(self == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if ((self->kind != DDS_RELIABLE_RELIABILITY_QOS) &&
        (self->kind != DDS_BEST_EFFORT_RELIABILITY_QOS))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_Duration_is_normalized(&self->max_blocking_time))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return ((DDS_Duration_compare(&self->max_blocking_time,
                                  &DDS_DURATION_ZERO) >= 0) &&
            (DDS_Duration_compare(&self->max_blocking_time,
                                  &DDS_DURATION_YEAR) <= 0)) ||
            DDS_Duration_is_infinite(&self->max_blocking_time);
}

/*ci
 * \brief Check if a requested DDS_ReliabilityQosPolicy is compatible with the
 *        offered DDS_ReliabilityQosPolicy
 *
 * \details
 *
 * The compatibility check is according to the DDS specification
 *
 * \param[in] reader The requesting reliability
 * \param[in] writer The writer offering reliability
 *
 * \return DDS_BOOLEAN_TRUE if the requested policy is compatible with the
 *         offered DDS_ReliabilityQosPolicy, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_ReliabilityQosPolicy_is_compatible(const struct DDS_ReliabilityQosPolicy *reader,
                                       const struct DDS_ReliabilityQosPolicy *writer)
{
    if ((reader->kind == DDS_RELIABLE_RELIABILITY_QOS) &&
        (writer->kind != DDS_RELIABLE_RELIABILITY_QOS))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

/**************************************************
    DDS_TypeSupportQosPolicy
**************************************************/
/*ci
 * \brief Test if two DDS_DataWriterTransferModeQosPolicy policies are equal
 *
 * \param[in] left side of comparison
 * \param[in] right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_DataWriterTransferModeQosPolicy_is_equal(
        const struct DDS_DataWriterTransferModeQosPolicy *left,
        const struct DDS_DataWriterTransferModeQosPolicy *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    if (left->shmem_ref_settings.enable_data_consistency_check != right->shmem_ref_settings.enable_data_consistency_check)
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Check that a DDS_DataWriterTransferModeQosPolicy policy has legal values
 *
 * \param[in] self DDS_DataWriterTransferModeQosPolicy structure to be validated
 *
 * \return DDS_BOOLEAN_TRUE if the content is valid, DDS_BOOLEAN_FALSE if not
 */
DDS_Boolean
DDS_DataWriterTransferModeQosPolicy_is_consistent(
                                    const struct DDS_DataWriterTransferModeQosPolicy *self)
{
    PRECOND_ARG(self)

    OSAPI_PRECONDITION(self == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (self->shmem_ref_settings.enable_data_consistency_check != DDS_BOOLEAN_TRUE
            && self->shmem_ref_settings.enable_data_consistency_check != DDS_BOOLEAN_FALSE)
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

/**************************************************
    DDS_TypeSupportQosPolicy
**************************************************/
/*ci
 * \brief Test if two DDS_TypeSupportQosPolicy policies are equal
 *
 * \param[in] left side of comparison
 * \param[in] right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_TypeSupportQosPolicy_is_equal(
        const struct DDS_TypeSupportQosPolicy *left,
        const struct DDS_TypeSupportQosPolicy *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    return (left->plugin_data == right->plugin_data) ?
                        DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/*ci
 * \brief Check that a DDS_TypeSupportQosPolicy policy has legal values
 *
 * \param[in] self DDS_TypeSupportQosPolicy structure to be validated
 *
 * \return DDS_BOOLEAN_TRUE if the content is valid, DDS_BOOLEAN_FALSE if not
 */
DDS_Boolean
DDS_TypeSupportQosPolicy_is_consistent(
                                    const struct DDS_TypeSupportQosPolicy *self)
{
    PRECOND_ARG(self)

    OSAPI_PRECONDITION(self == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return DDS_BOOLEAN_TRUE;
}

/**************************************************
    DDS_DataWriterProtocolQosPolicy
**************************************************/
/*ci
 * \brief Test if two DDS_DataWriterProtocolQosPolicy policies are equal
 *
 * \param[in] left side of comparison
 * \param[in] right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_DataWriterProtocolQosPolicy_is_equal(
        const struct DDS_DataWriterProtocolQosPolicy *left,
        const struct DDS_DataWriterProtocolQosPolicy *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    return (left->rtps_object_id == right->rtps_object_id)
            && DDS_RtpsReliableWriterProtocol_t_is_equal(
                    &left->rtps_reliable_writer,&right->rtps_reliable_writer) ?
            DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/*ci
 * \brief Check that a DDS_DataWriterProtocolQosPolicy policy has consistent,
 *        legal values
 *
 * \param[in] self DDS_DataWriterProtocolQosPolicy structure to be validated
 *
 * \return DDS_BOOLEAN_TRUE if the content is valid, DDS_BOOLEAN_FALSE if not
 */
DDS_Boolean
DDS_DataWriterProtocolQosPolicy_is_consistent(
                            const struct DDS_DataWriterProtocolQosPolicy *self)
{
    OSAPI_PRECONDITION(self == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    /* No restrictions on  rtps_object_id */
    return DDS_RtpsReliableWriterProtocol_is_consistent(
                                        &self->rtps_reliable_writer);
}

/**************************************************
    DDS_DataReaderResourceLimitsQosPolicy
**************************************************/
/*ci
 * \brief Test if two DDS_DataReaderResourceLimitsQosPolicy policies are equal
 *
 * \param[in] left side of comparison
 * \param[in] right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_DataReaderResourceLimitsQosPolicy_is_equal(
        const struct DDS_DataReaderResourceLimitsQosPolicy *left,
        const struct DDS_DataReaderResourceLimitsQosPolicy *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    return (left->max_outstanding_reads == right->max_outstanding_reads
            && left->max_remote_writers == right->max_remote_writers
            && left->max_remote_writers_per_instance == right->max_remote_writers_per_instance
            && left->max_samples_per_remote_writer == right->max_samples_per_remote_writer
            && left->instance_replacement == right->instance_replacement
            && left->max_fragmented_samples == right->max_fragmented_samples
            && left->max_fragmented_samples_per_remote_writer == right->max_fragmented_samples_per_remote_writer) ?
            DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/*ci
 * \brief Check that a DDS_DataReaderResourceLimitsQosPolicy policy has
 *        consistent, legal values
 *
 * \param[in] self DDS_DataReaderResourceLimitsQosPolicy structure to be validated
 *
 * \return DDS_BOOLEAN_TRUE if the content is valid, DDS_BOOLEAN_FALSE if not
 */
DDS_Boolean
DDS_DataReaderResourceLimitsQosPolicy_is_consistent(
                    const struct DDS_DataReaderResourceLimitsQosPolicy *self)
{
    OSAPI_PRECONDITION(self == NULL,
                            return DDS_BOOLEAN_FALSE,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if ((self->max_remote_writers < 1)
        || (self->max_remote_writers > 1000000)
        || (self->max_samples_per_remote_writer < 1)
        || (self->max_samples_per_remote_writer > 100000000)
        || ((self->max_fragmented_samples < 1) &&
            (self->max_fragmented_samples != DDS_MAX_AUTO))
        || (self->max_fragmented_samples > 1000000)
        || ((self->max_fragmented_samples_per_remote_writer < 1) &&
            (self->max_fragmented_samples_per_remote_writer != DDS_MAX_AUTO))
        || (self->max_fragmented_samples_per_remote_writer > 1000000)
        || (self->max_fragmented_samples_per_remote_writer > self->max_fragmented_samples)
        || (self->max_remote_writers_per_instance < 1)
        || (self->max_remote_writers < self->max_remote_writers_per_instance)
        || (self->max_outstanding_reads < 1)
        || ((self->instance_replacement != DDS_NO_INSTANCE_REPLACEMENT_QOS) &&
            (self->instance_replacement != DDS_REPLACE_OLDEST_INSTANCE_REPLACEMENT_QOS)))
    {

        return DDS_BOOLEAN_FALSE;

    }

    if ((self->shmem_ref_transfer_mode_attached_segment_allocation < 1) &&
        (self->shmem_ref_transfer_mode_attached_segment_allocation != DDS_LENGTH_AUTO) &&
        (self->shmem_ref_transfer_mode_attached_segment_allocation != DDS_LENGTH_UNLIMITED))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (self->shmem_ref_transfer_mode_attached_segment_allocation > 1000000)
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

/**************************************************
    DDS_TransportQosPolicy
**************************************************/
/*ci
 * \brief Copy a DDS_TransportQosPolicy
 *
 * \param[in] left  The destination DDS_TransportQosPolicy
 * \param[in] right The source DDS_TransportQosPolicy
 *
 * \return DDS_RETCODE_OK on success, DDS_RETCODE_ERROR otherwise
 */
DDS_ReturnCode_t
DDS_TransportQosPolicy_copy(struct DDS_TransportQosPolicy *left,
                            const struct DDS_TransportQosPolicy *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                            return DDS_RETCODE_BAD_PARAMETER,
                            OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    if (DDS_StringSeq_copy(
                &left->enabled_transports, &right->enabled_transports) == &left->enabled_transports)
    {
        return DDS_RETCODE_OK;
    }

    return DDS_RETCODE_ERROR;
}

/*ci
 * \brief Test if two DDS_TransportQosPolicy policies are equal
 *
 * \param[in] left side of comparison
 * \param[in] right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_TransportQosPolicy_is_equal(const struct DDS_TransportQosPolicy *left,
                                const struct DDS_TransportQosPolicy *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    return DDS_StringSeq_is_equal(&left->enabled_transports,
                                  &right->enabled_transports)
                        ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize a DDS_TransportQosPolicy
 *
 * \param[in] self  Finalize a DDS_TransportQosPolicy
 *
 * \return DDS_RETCODE_OK on success, DDS_RETCODE_ERROR on failure
 */
DDS_ReturnCode_t
DDS_TransportQosPolicy_finalize(struct DDS_TransportQosPolicy *self)
{
    OSAPI_PRECONDITION(self == NULL,
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (!DDS_StringSeq_finalize(&self->enabled_transports))
    {
        return DDS_RETCODE_ERROR;
    }
    return DDS_RETCODE_OK;
}
#endif /* !RTI_CERT */

struct DDS_TransportEncapsulationSettings_t*
DDS_TransportEncapsulationQosPolicy_find_transport_setting(
                            struct DDS_TransportEncapsulationQosPolicy *policy,
                            RT_ComponentFactoryId_T *intf_name)
{
    RTI_INT32 ts_length;
    RTI_INT32 ts_index;
    RTI_INT32 ss_length;
    RTI_INT32 ss_index;
    struct DDS_TransportEncapsulationSettings_t *ts;
    const char* ss_name;

    ts_length = DDS_TransportEncapsulationSettingsSeq_get_length(&policy->value);
    for (ts_index = 0; ts_index < ts_length; ts_index++)
    {
        ts = DDS_TransportEncapsulationSettingsSeq_get_reference(&policy->value,
                                                                 ts_index);
        if (ts == NULL)
        {
            return NULL;
        }

        ss_length = DDS_StringSeq_get_length(&ts->transports);
        for (ss_index = 0; ss_index < ss_length; ++ss_index)
        {
            ss_name = *DDS_StringSeq_get_reference(&ts->transports,ss_index);

            if (!REDA_String_compare(RT_ComponentFactoryId_get_name(intf_name),
                                     ss_name))
            {
                return ts;
            }
        }
    }

    return NULL;
}

/**************************************************
    DDS_RtpsReliableWriterProtocol_t
**************************************************/
/*ci
 * \brief Test if two DDS_RtpsReliableWriterProtocol_t policies are equal
 *
 * \param[in] left side of comparison
 * \param[in] right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_RtpsReliableWriterProtocol_t_is_equal(
        const struct DDS_RtpsReliableWriterProtocol_t* left,
        const struct DDS_RtpsReliableWriterProtocol_t* right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    return ((DDS_Duration_compare(&left->heartbeat_period,
                                  &right->heartbeat_period) == 0) &&
            (left->heartbeats_per_max_samples ==
                    right->heartbeats_per_max_samples) &&
            (left->max_send_window == right->max_send_window) &&
            (left->max_heartbeat_retries == right->max_heartbeat_retries) &&
            (DDS_SequenceNumber_compare(&left->first_write_sequence_number,
                                    &right->first_write_sequence_number) == 0)) ?
            DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/*ci
 * \brief Check that a DDS_RtpsReliableWriterProtocol_t policy has
 *        consistent, legal values
 *
 * \param[in] self DDS_RtpsReliableWriterProtocol_t structure to be validated
 *
 * \return DDS_BOOLEAN_TRUE if the content is valid, DDS_BOOLEAN_FALSE if not
 */
DDS_Boolean
DDS_RtpsReliableWriterProtocol_is_consistent(
                          const struct DDS_RtpsReliableWriterProtocol_t *self)
{
    OSAPI_PRECONDITION(self == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    /* heartbeat_period */
    if (DDS_Duration_compare(&self->heartbeat_period,
                             &DDS_DURATION_NANOSEC) < 0 ||
        DDS_Duration_compare(&self->heartbeat_period, &DDS_DURATION_YEAR) > 0)
    {
        return DDS_BOOLEAN_FALSE;
    }

    /* heartbeats_per_max_samples */
    if (self->heartbeats_per_max_samples < 0 ||
        self->heartbeats_per_max_samples > 100000000)
    {
        return DDS_BOOLEAN_FALSE;
    }

    if ((self->max_heartbeat_retries < 1 ||
         self->max_heartbeat_retries > 1000000) &&
         self->max_heartbeat_retries != DDS_LENGTH_UNLIMITED)
    {
        return DDS_BOOLEAN_FALSE;
    }

    if ((self->max_send_window < 1 ||
         self->max_send_window > 256) &&
        (self->max_send_window != DDS_LENGTH_UNLIMITED))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if ((self->first_write_sequence_number.high < 0) ||
        (self->first_write_sequence_number.low == 0))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

/**************************************************
    DDS_EntityNameQosPolicy
**************************************************/
/*ci
 * \brief Test if two DDS_EntityNameQosPolicy policies are equal
 *
 * \param[in] left side of comparison
 * \param[in] right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_EntityNameQosPolicy_is_equal(const struct DDS_EntityNameQosPolicy *left,
                                 const struct DDS_EntityNameQosPolicy *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    return DDS_String_ncmp(left->name, right->name,DDS_ENTITYNAME_QOS_NAME_MAX) ?
                    DDS_BOOLEAN_FALSE : DDS_BOOLEAN_TRUE;
}

/*ce \dref_EntityNameQosPolicy_set_name
 */
DDS_Boolean
DDS_EntityNameQosPolicy_set_name(struct DDS_EntityNameQosPolicy *const self,
                                 const char *const name)
{
    OSAPI_PRECONDITION((self == NULL) || (name == NULL) ||
                       (REDA_String_length(name) > DDS_ENTITYNAME_QOS_NAME_MAX),
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("name",name,RTI_TRUE););

    return REDA_String_copy(self->name,DDS_ENTITYNAME_QOS_NAME_MAX,name) ?
                                    DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/**************************************************
    DDS_RtpsReliableReaderProtocol_t
**************************************************/
/*ci
 * \brief Test if two DDS_RtpsReliableReaderProtocol_t structures are equal
 *
 * \param[in] left side of comparison
 * \param[in] right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_RtpsReliableReaderProtocol_is_equal(
        const struct DDS_RtpsReliableReaderProtocol_t* left,
        const struct DDS_RtpsReliableReaderProtocol_t* right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    return DDS_Duration_equal(&left->nack_period,&right->nack_period);
}

/*ci
 * \brief Check that a DDS_RtpsReliableReaderProtocol_t structure is
 *        consistent, legal values
 *
 * \param[in] self DDS_RtpsReliableReaderProtocol_t structure to be validated
 *
 * \return DDS_BOOLEAN_TRUE if the content is valid, DDS_BOOLEAN_FALSE if not
 */
DDS_Boolean
DDS_RtpsReliableReaderProtocol_is_consistent(
                          const struct DDS_RtpsReliableReaderProtocol_t *self)
{
    OSAPI_PRECONDITION(self == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    /* nack_period */
    if ((DDS_Duration_compare(&self->nack_period, &DDS_DURATION_NANOSEC) < 0) ||
        (DDS_Duration_compare(&self->nack_period, &DDS_DURATION_YEAR) > 0))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

/**************************************************
    DDS_DataReaderProtocolQosPolicy
**************************************************/

/*ci
 * \brief Test if two DDS_DataReaderProtocolQosPolicy policies are equal
 *
 * \param[in] left side of comparison
 * \param[in] right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_DataReaderProtocolQosPolicy_is_equal(
                        const struct DDS_DataReaderProtocolQosPolicy *left,
                        const struct DDS_DataReaderProtocolQosPolicy *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    if (left->rtps_object_id != right->rtps_object_id)
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (left->propagate_dispose_of_unregistered_instances
        != right->propagate_dispose_of_unregistered_instances)
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_RtpsReliableReaderProtocol_is_equal(
                        &left->rtps_reliable_reader,
                        &right->rtps_reliable_reader);
}

/*ci
 * \brief Test if two DDS_DataReaderProtocolQosPolicy policies are equal
 *
 * \param[in] left side of comparison
 * \param[in] right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_DataReaderProtocolQosPolicy_is_consistent(
                        const struct DDS_DataReaderProtocolQosPolicy *self)
{
    OSAPI_PRECONDITION(self == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    /* A user can specify objectids < OSAPI_SYSTEM_OBJECTID_START.
     * Objectid >= OSAPI_SYSTEM_OBJECTID_START are generated by the
     * OSAPI_System_next_object_id() function.
     */
    if (self->rtps_object_id >= OSAPI_SYSTEM_OBJECTID_START)
    {
        return DDS_BOOLEAN_FALSE;
    }

    if ((self->propagate_dispose_of_unregistered_instances != DDS_BOOLEAN_TRUE) &&
        (self->propagate_dispose_of_unregistered_instances != DDS_BOOLEAN_FALSE))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_RtpsReliableReaderProtocol_is_consistent(
                                            &self->rtps_reliable_reader);
}

/**************************************************
    DDS_WireProtocolQosPolicy
**************************************************/
/*ci
 * \brief Test if two DDS_WireProtocolQosPolicy policies are equal
 *
 * \param[in] left side of comparison
 * \param[in] right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right ,DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_WireProtocolQosPolicy_is_equal(const struct DDS_WireProtocolQosPolicy *left,
                                   const struct DDS_WireProtocolQosPolicy *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                            return DDS_BOOLEAN_FALSE,
                            OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)


    return (left->participant_id == right->participant_id
            && left->rtps_app_id == right->rtps_app_id
            && left->rtps_host_id == right->rtps_host_id
            && left->rtps_instance_id == right->rtps_instance_id
            && DDS_RtpsWellKnownPorts_is_equal(
                &left->rtps_well_known_ports, &right->rtps_well_known_ports)
            && (left->compute_crc == right->compute_crc)
            && (left->check_crc == right->check_crc)
            && (left->require_crc == right->require_crc)
            && (left->computed_crc_kind == right->computed_crc_kind)
            && (left->allowed_crc_mask == right->allowed_crc_mask))
                ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/*ci
 * \brief Check that a DDS_WireProtocolQosPolicy policy has consistent,
 *        legal values
 *
 * \param[in] self DDS_WireProtocolQosPolicy structure to be validated
 *
 * \return DDS_BOOLEAN_TRUE if the content is valid, DDS_BOOLEAN_FALSE if not
 */
DDS_Boolean
DDS_WireProtocolQosPolicy_is_consistent(const struct DDS_WireProtocolQosPolicy *self)
{
    DDS_Long count = 0;
    DDS_ChecksumKindMask_t mask;

    OSAPI_PRECONDITION(self == NULL,
                            return DDS_BOOLEAN_FALSE,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if ((self->participant_id < -1) ||
            ((self->rtps_host_id == DDS_RTPS_AUTO_ID) &&
             ((self->rtps_app_id != DDS_RTPS_AUTO_ID) ||
              (self->rtps_instance_id != DDS_RTPS_AUTO_ID))) ||

            ((self->rtps_app_id == DDS_RTPS_AUTO_ID) &&
             ((self->rtps_host_id != DDS_RTPS_AUTO_ID) ||
              (self->rtps_instance_id != DDS_RTPS_AUTO_ID))) ||

            ((self->rtps_instance_id == DDS_RTPS_AUTO_ID) &&
             ((self->rtps_app_id != DDS_RTPS_AUTO_ID) ||
              (self->rtps_host_id != DDS_RTPS_AUTO_ID))) ||

            !DDS_RtpsWellKnownPorts_is_consistent(&self->rtps_well_known_ports))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (((self->computed_crc_kind != DDS_CHECKSUM_AUTO) && (self->computed_crc_kind > DDS_CHECKSUM_MASK_MAX)) ||
        ((self->allowed_crc_mask != DDS_CHECKSUM_AUTO) && (self->allowed_crc_mask > DDS_CHECKSUM_MASK_MAX)))
    {
        /* Out of range values */
        return DDS_BOOLEAN_FALSE;
    }

    if ((self->compute_crc &&
        (self->computed_crc_kind == DDS_CHECKSUM_NONE)) ||
        ((self->check_crc || self->require_crc) &&
         (self->allowed_crc_mask == DDS_CHECKSUM_NONE)))
    {
        /* Checksum either sent, checked or required, but none specified for
         * sending or reception.
         */
        return DDS_BOOLEAN_FALSE;
    }

    if (self->computed_crc_kind != DDS_CHECKSUM_AUTO)
    {
        /* Check if the computed_crc_kind has more than 1 bit */
        mask = self->computed_crc_kind;
        while (mask)
        {
            if (mask & 1U)
            {
                ++count;
            }
            mask = mask >> 1U;
        }

        if (count > 1)
        {
            return DDS_BOOLEAN_FALSE;
        }
    }

    return DDS_BOOLEAN_TRUE;
}

/**************************************************
    DDS_ChecksumProperty
 **************************************************/

/*ci
 * \brief Test if two DDS_ChecksumProperty structures are equal
 *
 * \param[in] left side of comparison
 * \param[in] right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_ChecksumProperty_is_equal(const struct DDS_ChecksumProperty *left,
                              const struct DDS_ChecksumProperty *right)
{
    /* Do not use memcpy due to potential padding */
    return (left->computed_crc_kind == right->computed_crc_kind) &&
           (left->allowed_crc_mask == right->allowed_crc_mask) &&
           (left->require_crc == right->require_crc);
}

/**************************************************
    RTI_ManagementQosPolicy
**************************************************/
/*ci
 * \brief Test if two RTI_ManagementQosPolicy policies are equal
 *
 * \param[in] left side of comparison
 * \param[in] right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
RTI_ManagementQosPolicy_is_equal(const struct RTI_ManagementQosPolicy *left,
                                 const struct RTI_ManagementQosPolicy *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    if ((left->is_anonymous != right->is_anonymous) ||
        (left->is_hidden != right->is_hidden))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

/**************************************************
    DDS_DataWriterResourceLimitsQosPolicy
**************************************************/
/*ci
 * \brief Test if two DDS_DataWriterResourceLimitsQosPolicy policies are equal
 *
 * \param[in] left side of comparison
 * \param[in] right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_DataWriterResourceLimitsQosPolicy_is_equal(
        const struct DDS_DataWriterResourceLimitsQosPolicy *left,
        const struct DDS_DataWriterResourceLimitsQosPolicy *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

#if DDS_FILTERING_ENABLED
    if (left->max_remote_reader_filters != right->max_remote_reader_filters)
    {
        return DDS_BOOLEAN_FALSE;
    }
#endif

    return (left->max_remote_readers == right->max_remote_readers) ?
                    DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/*ci
 * \brief Check that a DDS_DataWriterResourceLimitsQosPolicy policy has
 *        consistent, legal values
 *
 * \param[in] self DDS_DataWriterResourceLimitsQosPolicy structure to be validated
 *
 * \return DDS_BOOLEAN_TRUE if the content is valid, DDS_BOOLEAN_FALSE if not
 */
DDS_Boolean
DDS_DataWriterResourceLimitsQosPolicy_is_consistent(
                    const struct DDS_DataWriterResourceLimitsQosPolicy *self)
{
    OSAPI_PRECONDITION(self == NULL,
                       return DDS_BOOLEAN_FALSE,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if ((self->max_remote_readers < 1) || (self->max_remote_readers > 100000000L))
    {

        return DDS_BOOLEAN_FALSE;
    }

    /* If value is < 1, then it must be DDS_LENGTH AUTO or DDS_LENGTH_UNLIMITED
     * 0 is not allowed either */
    if ((self->writer_loaned_sample_allocation < 1)  &&
        (self->writer_loaned_sample_allocation != DDS_SIZE_AUTO) &&
        (self->writer_loaned_sample_allocation != DDS_LENGTH_UNLIMITED))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (self->writer_loaned_sample_allocation > 1000000)
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (self->initialize_writer_loaned_sample != DDS_BOOLEAN_FALSE
            && self->initialize_writer_loaned_sample != DDS_BOOLEAN_TRUE)
    {
        return DDS_BOOLEAN_FALSE;
    }

#if DDS_FILTERING_ENABLED
    if ((self->max_remote_reader_filters != DDS_LENGTH_UNLIMITED) &&
        (self->max_remote_reader_filters < 0))
    {
        return DDS_BOOLEAN_FALSE;
    }
#endif

    return DDS_BOOLEAN_TRUE;
}

/**************************************************
    DDS_DomainParticipantResourceLimitsQosPolicy
**************************************************/
/*ci
 * \brief Test if two DDS_DomainParticipantResourceLimitsQosPolicy policies are equal
 *
 * \param[in] left side of comparison
 * \param[in] right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_DomainParticipantResourceLimitsQosPolicy_is_equal(
        const struct DDS_DomainParticipantResourceLimitsQosPolicy *left,
        const struct DDS_DomainParticipantResourceLimitsQosPolicy *right)
{
    if ((left->local_publisher_allocation !=
         right->local_publisher_allocation) ||
        (left->local_reader_allocation !=
         right->local_reader_allocation) ||
        (left->local_subscriber_allocation !=
         right->local_subscriber_allocation) ||
        (left->local_topic_allocation !=
         right->local_topic_allocation) ||
        (left->local_type_allocation !=
         right->local_type_allocation) ||
        (left->local_writer_allocation !=
         right->local_writer_allocation) ||
        (left->matching_reader_writer_pair_allocation !=
         right->matching_reader_writer_pair_allocation) ||
        (left->matching_writer_reader_pair_allocation !=
         right->matching_writer_reader_pair_allocation) ||
        (left->max_destination_ports !=
         right->max_destination_ports) ||
        (left->max_receive_ports !=
         right->max_receive_ports) ||
        (left->remote_participant_allocation !=
         right->remote_participant_allocation) ||
        (left->remote_reader_allocation !=
         right->remote_reader_allocation) ||
        (left->remote_writer_allocation !=
         right->remote_writer_allocation))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if ((left->participant_user_data_max_length !=
         right->participant_user_data_max_length) ||
        (left->topic_data_max_length !=
         right->topic_data_max_length) ||
        (left->publisher_group_data_max_length !=
         right->publisher_group_data_max_length) ||
        (left->subscriber_group_data_max_length !=
         right->subscriber_group_data_max_length) ||
        (left->writer_user_data_max_length !=
         right->writer_user_data_max_length) ||
        (left->reader_user_data_max_length !=
         right->reader_user_data_max_length))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if ((left->participant_property_string_max_length !=
         right->participant_property_string_max_length)
        || (left->writer_property_string_max_length !=
            right->writer_property_string_max_length)
        || (left->reader_property_string_max_length !=
            right->reader_property_string_max_length))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if ((left->participant_property_list_max_length !=
         right->participant_property_list_max_length)
        || (left->writer_property_list_max_length !=
            right->writer_property_list_max_length)
        || (left->reader_property_list_max_length !=
            right->reader_property_list_max_length))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Check that a DDS_DomainParticipantResourceLimitsQosPolicy policy has
 *        consistent, legal values
 *
 * \param[in] self DDS_DomainParticipantResourceLimitsQosPolicy structure to be validated
 *
 * \return DDS_BOOLEAN_TRUE if the content is valid, DDS_BOOLEAN_FALSE if not
 */
DDS_Boolean
DDS_DomainParticipantResourceLimitsQosPolicy_is_consistent(
        const struct DDS_DomainParticipantResourceLimitsQosPolicy *self)
{
    if ((self->local_reader_allocation < 1) ||
            (self->local_writer_allocation < 1) ||
            (self->local_subscriber_allocation < 1) ||
            (self->local_publisher_allocation < 1) ||
            (self->local_topic_allocation < 1) ||
            (self->local_type_allocation < 1) ||
            (self->matching_reader_writer_pair_allocation < 0) ||
            (self->matching_writer_reader_pair_allocation < 0) ||
            ((self->matching_reader_writer_pair_allocation  +
              self->matching_writer_reader_pair_allocation) < 1) ||
            (self->max_destination_ports < 1) ||
            (self->max_receive_ports < 1) ||
            (self->remote_participant_allocation < 1) ||
            (self->remote_reader_allocation < 1) ||
            (self->remote_writer_allocation < 1))
    {
        return DDS_BOOLEAN_FALSE;
    }
    if ((self->participant_user_data_max_length < 0) ||
            (self->topic_data_max_length < 0) ||
            (self->publisher_group_data_max_length < 0) ||
            (self->subscriber_group_data_max_length < 0) ||
            (self->writer_user_data_max_length < 0) ||
            (self->reader_user_data_max_length < 0))
    {
        return DDS_BOOLEAN_FALSE;
    }
    if (((self->participant_user_data_max_count <= 0)
                && (self->participant_user_data_max_count != DDS_SIZE_AUTO)) ||
            ((self->topic_data_max_count <= 0)
                && (self->topic_data_max_count != DDS_SIZE_AUTO)) ||
            ((self->publisher_group_data_max_count <= 0)
                && (self->publisher_group_data_max_count != DDS_SIZE_AUTO)) ||
            ((self->subscriber_group_data_max_count <= 0)
                && (self->subscriber_group_data_max_count != DDS_SIZE_AUTO)) ||
            ((self->writer_user_data_max_count <= 0)
                && (self->writer_user_data_max_count != DDS_SIZE_AUTO)) ||
            ((self->reader_user_data_max_count <= 0)
                && (self->reader_user_data_max_count != DDS_SIZE_AUTO)))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if ((self->max_partitions > DDS_PARTITIONQOSPOLICY_MAX_PARTITIONS)
            || (self->max_partitions < 0))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if ((self->max_partition_cumulative_characters >
                DDS_PARTITIONQOSPOLICY_MAX_PARTITION_CHARACTERS) ||
                (self->max_partition_cumulative_characters < 1))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if ((self->max_partition_string_size != DDS_LENGTH_UNLIMITED) &&
                ((self->max_partition_string_size < 1) ||
                (self->max_partition_string_size >
                self->max_partition_cumulative_characters)))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if ((self->max_partition_string_allocation < 1) &&
            (self->max_partition_string_allocation != DDS_LENGTH_UNLIMITED))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (self->shmem_ref_transfer_mode_max_segments == 0
            || self->shmem_ref_transfer_mode_max_segments > 100000)
    {
        /* 100,000 seems like a good enough limit */
        return DDS_BOOLEAN_FALSE;
    }

    /* Property Qos policy limits
     * Nothing is propagated, require these to be 0. These are also not
     * exposed to the user.
     */
    if ((self->participant_property_string_max_length != DDS_PROPERTY_MAX_STRING_LENGTH_DEFAULT)
        || (self->writer_property_string_max_length != DDS_PROPERTY_MAX_STRING_LENGTH_DEFAULT)
        || (self->reader_property_string_max_length != 0))
    {
        return DDS_BOOLEAN_FALSE;
    }

    /* Property Qos policy limits
     * Only allow 1 ParticipantProperty and 1 WriterProperty for
     * the xtypes compliance mask
     */
    if ((self->participant_property_list_max_length != 1)
        || (self->writer_property_list_max_length != 1)
        || (self->reader_property_list_max_length != 0))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

/**************************************************
    DDS_TransportEncapsulationQosPolicy
**************************************************/
/*ci
 * \brief Copy a DDS_TransportEncapsulationQosPolicy
 *
 * \param[in] left  The destination DDS_TransportEncapsulationQosPolicy
 * \param[in] right The source DDS_TransportEncapsulationQosPolicy
 *
 * \return DDS_RETCODE_OK if left = right, DDS_RETCODE_ERROR otherwise
 */
DDS_ReturnCode_t
DDS_TransportEncapsulationQosPolicy_copy(
                    struct DDS_TransportEncapsulationQosPolicy *left,
                    const struct DDS_TransportEncapsulationQosPolicy *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                            return DDS_RETCODE_BAD_PARAMETER,
                            OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    if (DDS_TransportEncapsulationSettingsSeq_copy(
                &left->value, &right->value) == &left->value)
    {
        return DDS_RETCODE_OK;
    }

    return DDS_RETCODE_ERROR;
}

/*ci
 * \brief Test if two DDS_TransportQosPolicy policies are equal
 *
 * \param[in] left side of comparison
 * \param[in] right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_TransportEncapsulationQosPolicy_is_equal(
                    const struct DDS_TransportEncapsulationQosPolicy *left,
                    const struct DDS_TransportEncapsulationQosPolicy *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    return DDS_TransportEncapsulationSettingsSeq_is_equal(
                                    &left->value,
                                    &right->value)
                        ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize a DDS_TransportQosPolicy
 *
 * \param[in] self  Finalize a DDS_TransportQosPolicy
 *
 * \return DDS_RETCODE_OK on success, DDS_RETCODE_ERROR on failure
 */
DDS_ReturnCode_t
DDS_TransportEncapsulationQosPolicy_finalize(
                            struct DDS_TransportEncapsulationQosPolicy *self)
{
    OSAPI_PRECONDITION(self == NULL,
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (!DDS_TransportEncapsulationSettingsSeq_finalize(&self->value))
    {
        return DDS_RETCODE_ERROR;
    }
    return DDS_RETCODE_OK;
}
#endif /* !RTI_CERT */


/**************************************************
    DDS_DataRepresentationQosPolicy
**************************************************/

RTI_PRIVATE const DDS_DataRepresentationId_t DDS_DataRepresentationId_gv_V2[]=
{
        DDS_XCDR2_DATA_REPRESENTATION
};

RTI_PRIVATE const DDS_DataRepresentationId_t DDS_DataRepresentationId_gv_V2_V1[]=
{
        DDS_XCDR2_DATA_REPRESENTATION,
        DDS_XCDR_DATA_REPRESENTATION
};

/*ci
 * \brief Structure to send support for the V2 data representation.
 */
const struct DDS_DataRepresentationQosPolicy DDS_DataRepresentationQosPolicy_gv_V2 =
{
    DDS_DataRepresentationIdSeq_INITIALIZER_W_LOAN(DDS_DataRepresentationId_gv_V2,1,1)
};

/*ci
 * \brief Structure to send support for the V2 and V1 data representation.
 */
const struct DDS_DataRepresentationQosPolicy DDS_DataRepresentationQosPolicy_gv_V2_V1 =
{
    DDS_DataRepresentationIdSeq_INITIALIZER_W_LOAN(DDS_DataRepresentationId_gv_V2_V1,2,2)
};

DDS_ReturnCode_t
DDS_DataRepresentationQosPolicy_copy(
                    struct DDS_DataRepresentationQosPolicy *left,
                    const struct DDS_DataRepresentationQosPolicy *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                            return DDS_RETCODE_BAD_PARAMETER,
                            OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    if (DDS_DataRepresentationIdSeq_copy(
                &left->value, &right->value) == &left->value)
    {
        return DDS_RETCODE_OK;
    }

    return DDS_RETCODE_ERROR;
}

/*ci
 * \brief Test if two DDS_TransportQosPolicy policies are equal
 *
 * \param[in] left side of comparison
 * \param[in] right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_DataRepresentationQosPolicy_is_equal(
                    const struct DDS_DataRepresentationQosPolicy *left,
                    const struct DDS_DataRepresentationQosPolicy *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    return DDS_DataRepresentationIdSeq_is_equal(
                                    &left->value,
                                    &right->value)
                        ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize a DDS_TransportQosPolicy
 *
 * \param[in] self  Finalize a DDS_TransportQosPolicy
 *
 * \return DDS_RETCODE_OK on success, DDS_RETCODE_ERROR on failure
 */
DDS_ReturnCode_t
DDS_DataRepresentationQosPolicy_finalize(
                            struct DDS_DataRepresentationQosPolicy *self)
{
    OSAPI_PRECONDITION(self == NULL,
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (!DDS_DataRepresentationIdSeq_finalize(&self->value))
    {
        return DDS_RETCODE_ERROR;
    }
    return DDS_RETCODE_OK;
}
#endif

DDS_Boolean
DDS_DataRepresentationQosPolicy_is_consistent(
        const struct DDS_DataRepresentationQosPolicy *self)
{
    RTI_INT32 seq_length;
    RTI_INT32 seq_index;
    DDS_DataRepresentationId_t id;

    seq_length = DDS_DataRepresentationIdSeq_get_length(&self->value);
    for (seq_index = 0; (seq_index < seq_length); ++seq_index)
    {
        id = *DDS_DataRepresentationIdSeq_get_reference(&self->value,
                                                        seq_index);
        if ((id != DDS_XCDR2_DATA_REPRESENTATION) &&
            (id != DDS_XCDR_DATA_REPRESENTATION) &&
            (id != DDS_AUTO_DATA_REPRESENTATION))
        {
            return DDS_BOOLEAN_FALSE;
        }
    }

    return DDS_BOOLEAN_TRUE;
}

DDS_Boolean
DDS_DataRepresentationQosPolicy_is_compatible(
                        const struct DDS_DataRepresentationQosPolicy *request,
                        const struct DDS_DataRepresentationQosPolicy *offered,
                        DDS_DataRepresentationId_t *id)
{
    RTI_INT32 offered_length;
    RTI_INT32 offered_index;
    RTI_INT32 request_length;
    RTI_INT32 request_index;
    DDS_Boolean found = DDS_BOOLEAN_FALSE;

    *id = 0;

    offered_length = DDS_DataRepresentationIdSeq_get_length(&offered->value);
    request_length = DDS_DataRepresentationIdSeq_get_length(&request->value);

    if ((offered_length == 0) && (request_length == 0))
    {
        *id = DDS_XCDR_DATA_REPRESENTATION;

        return DDS_BOOLEAN_TRUE;
    }

    if (offered_length == 0)
    {
        request_length = DDS_DataRepresentationIdSeq_get_length(&request->value);
        for (request_index = 0; request_index < request_length; ++request_index)
        {
            if (DDS_XCDR_DATA_REPRESENTATION ==
                *DDS_DataRepresentationIdSeq_get_reference(&request->value,
                                                           request_index))
            {
                /* Found a matching id */
                *id = DDS_XCDR_DATA_REPRESENTATION;
                return DDS_BOOLEAN_TRUE;
            }
        }

        return DDS_BOOLEAN_FALSE;
    }

    if (request_length == 0)
    {
        for (offered_index = 0; (offered_index < offered_length) && !found; ++offered_index)
        {
            if (*DDS_DataRepresentationIdSeq_get_reference(&offered->value,
                                                           offered_index) ==
                DDS_XCDR_DATA_REPRESENTATION)
            {
                /* Found a matching id */
                *id = DDS_XCDR_DATA_REPRESENTATION;

                return DDS_BOOLEAN_TRUE;
            }
        }

        return DDS_BOOLEAN_FALSE;
    }

    for (offered_index = 0; (offered_index < offered_length) && !found; ++offered_index)
    {
        request_length = DDS_DataRepresentationIdSeq_get_length(&request->value);
        for (request_index = 0; request_index < request_length; ++request_index)
        {
            if (*DDS_DataRepresentationIdSeq_get_reference(&offered->value,
                                                           offered_index) ==
                *DDS_DataRepresentationIdSeq_get_reference(&request->value,
                                                           request_index))
            {
                /* Found a matching id */
                *id = *DDS_DataRepresentationIdSeq_get_reference(&offered->value,
                                                                 offered_index);
                found = DDS_BOOLEAN_TRUE;
                break;
            }
        }
    }

    return found;
}
/* ================================================================= */
/*                       PublishMode Qos Policy                     */
/* ================================================================= */

DDS_Boolean
DDS_PublishModeQosPolicy_is_consistent(const struct DDS_PublishModeQosPolicy *const p)
{
#if DDS_FLOW_CONTROLLER_ENABLED
    if ((p->flow_controller_name == NULL) &&
        ((p->kind != DDS_SYNCHRONOUS_PUBLISH_MODE_QOS) &&
         (p->kind != DDS_AUTOMATIC_PUBLISH_MODE_QOS)))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if ((p->kind != DDS_SYNCHRONOUS_PUBLISH_MODE_QOS) &&
        (p->kind != DDS_ASYNCHRONOUS_PUBLISH_MODE_QOS) &&
        (p->kind != DDS_AUTOMATIC_PUBLISH_MODE_QOS))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if ((p->priority < 0) ||
        ((p->priority > 0) && (p->kind != DDS_ASYNCHRONOUS_PUBLISH_MODE_QOS )))
    {
        return DDS_BOOLEAN_FALSE;
    }
#else
    if ((p->flow_controller_name != NULL) ||
        ((p->kind != DDS_SYNCHRONOUS_PUBLISH_MODE_QOS) &&
         (p->kind != DDS_AUTOMATIC_PUBLISH_MODE_QOS)))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (p->priority != 0)
    {
        return DDS_BOOLEAN_FALSE;
    }
#endif

    return DDS_BOOLEAN_TRUE;
}

DDS_Boolean
DDS_PublishModeQosPolicy_is_equal(const struct DDS_PublishModeQosPolicy *const l,
                                  const struct DDS_PublishModeQosPolicy *const r)
{
    if ((l->flow_controller_name != NULL) &&
        (r->flow_controller_name != NULL))
    {
        if (OSAPI_String_cmp(l->flow_controller_name,r->flow_controller_name))
        {
            return DDS_BOOLEAN_FALSE;
        }
    }

    if (l->flow_controller_name != r->flow_controller_name)
    {
        return DDS_BOOLEAN_FALSE;
    }

    if ((l->kind != r->kind) ||
        (l->priority != r->priority))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

DDS_ReturnCode_t
DDS_PublishModeQosPolicy_copy(struct DDS_PublishModeQosPolicy *l,
                              const struct DDS_PublishModeQosPolicy *const r)
{
    l->kind = r->kind;
    l->priority = r->priority;

    if (l->flow_controller_name != NULL)
    {
        DDS_String_free((char*)l->flow_controller_name);
    }

    l->flow_controller_name = NULL;

    if (r->flow_controller_name != NULL)
    {
        l->flow_controller_name = DDS_String_dup(r->flow_controller_name);
    }

    return DDS_RETCODE_OK;
}

#ifndef RTI_CERT
DDS_ReturnCode_t
DDS_PublishModeQosPolicy_finalize(struct DDS_PublishModeQosPolicy *p)
{
#if DDS_FLOW_CONTROLLER_ENABLED
  if ((p->flow_controller_name != NULL) &&
      (p->flow_controller_name != DDS_DEFAULT_FLOW_CONTROLLER_NAME) &&
      (p->flow_controller_name != DDS_FIXED_RATE_FLOW_CONTROLLER_NAME) &&
      (p->flow_controller_name != DDS_ON_DEMAND_FLOW_CONTROLLER_NAME))
    {
        DDS_String_free((char *)p->flow_controller_name);
    }
    p->flow_controller_name = NULL;
#else
    UNUSED_ARG(p);
#endif

    return DDS_RETCODE_OK;
}
#endif

/*****************************
    DDS_LatencyBudgetQosPolicy
******************************/

/*ci
 * \brief Test if two DDS_LatencyBudgetQosPolicy policies are equal
 *
 * \param[in] left side of comparison
 * \param[in] right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_LatencyBudgetQosPolicy_is_equal(
                            const struct DDS_LatencyBudgetQosPolicy *left,
                            const struct DDS_LatencyBudgetQosPolicy *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    return DDS_Duration_compare(&left->duration, &right->duration) == 0 ?
                                DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/*ci
 * \brief Check that a DDS_LatencyBudgetQosPolicy policy has consistent, legal values
 *
 * \param[in] self DDS_LatencyBudgetQosPolicy structure to be validated
 *
 * \return DDS_BOOLEAN_TRUE if the content is valid, DDS_BOOLEAN_FALSE if not
 */
DDS_Boolean
DDS_LatencyBudgetQosPolicy_is_consistent(
                                const struct DDS_LatencyBudgetQosPolicy *self)
{
    OSAPI_PRECONDITION(self == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (!DDS_Duration_is_normalized(&self->duration))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if ((DDS_Duration_compare(&self->duration, &DDS_DURATION_ZERO) < 0
         || DDS_Duration_compare(&self->duration, &DDS_DURATION_YEAR) > 0)
        && DDS_Duration_compare(&self->duration, &DDS_DURATION_INFINITE) != 0)
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Check if a requested DDS_LatencyBudgetQosPolicy is compatible with the
 *        offered DDS_LatencyBudgetQosPolicy
 *
 * \details
 *
 * The compatibility check is according to the DDS specification
 *
 * \param[in] request The requested latency_budget
 * \param[in] offered The offered latency_budget
 *
 * \return DDS_BOOLEAN_TRUE if the requested policy is compatible with the
 *         offered DDS_LatencyBudgetQosPolicy, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_LatencyBudgetQosPolicy_is_compatible(
                            const struct DDS_LatencyBudgetQosPolicy *request,
                            const struct DDS_LatencyBudgetQosPolicy *offered)
{
    if (DDS_Duration_compare(&request->duration,
                             &DDS_DURATION_INFINITE) &&
        (DDS_Duration_compare(&request->duration, &offered->duration) < 0))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

#if DDS_FILTERING_ENABLED
/* ============================
 * DDS_FilterQosPolicy
 * ============================
 */

DDS_Boolean
DDS_FilterQosPolicy_is_equal(const struct DDS_FilterQosPolicy *left,
                                      const struct DDS_FilterQosPolicy *right)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION((left == NULL) || (right == NULL),
                    goto done,
                    OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    if (RT_ComponentFactoryId_compare(&left->name, &right->name) != 0)
    {
        goto done;
    }

    if ((left->resource_limits.filter_class_max_length != right->resource_limits.filter_class_max_length)
        || (left->resource_limits.filter_class_max_count != right->resource_limits.filter_class_max_count)
        || (left->resource_limits.filter_expression_max_length != right->resource_limits.filter_expression_max_length)
        || (left->resource_limits.filter_expression_max_count != right->resource_limits.filter_expression_max_count)
        || (left->resource_limits.filter_parameter_max_length != right->resource_limits.filter_parameter_max_length)
        || (left->resource_limits.filter_parameter_max_count_per_expression != right->resource_limits.filter_parameter_max_count_per_expression))
    {
        goto done;
    }

    retval = DDS_BOOLEAN_TRUE;
done:
    return retval;
}

DDS_Boolean
DDS_FilterQosPolicy_is_consistent(const struct DDS_FilterQosPolicy* self)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION(self == NULL,
                       return DDS_BOOLEAN_FALSE,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (RT_ComponentFactoryId_is_nil(&self->name))
    {
        /* If the name is nil, then the resource limits are not checked
         * to allow the use of the DDS_FilterQosPolicy_INITIALIZER by MAG
         */
        retval = DDS_BOOLEAN_TRUE;
        goto done;
    }

    if ((self->resource_limits.filter_class_max_length <= 0) ||
        (self->resource_limits.filter_class_max_length > DDS_CONTENT_FILTER_CLASS_NAME_MAX_LENGTH) ||
        (self->resource_limits.filter_class_max_count <= 0) ||
        (self->resource_limits.filter_expression_max_length <= 0) ||
        (self->resource_limits.filter_expression_max_length > DDS_CONTENT_FILTER_EXPRESSION_MAX_LENGTH) ||
        (self->resource_limits.filter_parameter_max_length < 0) ||
        (self->resource_limits.filter_parameter_max_count_per_expression < 0) ||
        (self->resource_limits.filter_parameter_max_count_per_expression > DDS_CONTENT_FILTER_MAX_PARAMETERS))
    {
        goto done;
    }

    if ((self->resource_limits.filter_expression_max_count <= 0) &&
        (self->resource_limits.filter_expression_max_count != DDS_SIZE_AUTO))
    {
        goto done;
    }

    if ((self->resource_limits.filter_parameter_max_count_per_expression != 0) &&
        (self->resource_limits.filter_parameter_max_length == 0))
    {
        goto done;
    }

    retval = DDS_BOOLEAN_TRUE;
done:
    return retval;
}
#endif /* DDS_FILTERING_ENABLED */


DDS_Boolean
DDS_TransportPriorityQosPolicy_is_equal(const struct DDS_TransportPriorityQosPolicy *left,
                                    const struct DDS_TransportPriorityQosPolicy *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    return (left->value == right->value) ?
                    DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}



/*ci @} */

