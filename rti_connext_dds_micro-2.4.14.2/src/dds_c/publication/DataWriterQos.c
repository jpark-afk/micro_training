/*
 * FILE: DataWriterQos.c - DataWriter QoS implementation
 *
 * (c) Copyright 2008-2015 Real-Time Innovations,
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
 * 20sep2021,tk MICRO-3152/PR.29564
 * - Removed redundant check for max_blocking_time when
 *   DDS_BLOCKING_READER_ENABLED is FALSE.
 * 19jul2021,tk MICRO-3117/PR.28868
 * - Return DDS_BOOLEAN_FALSE instead of RTI_FALSE in DDS_DataWriterQos_is_equal
 * 16apr2021,tk MICRO-2845/PR.28682
 *  - Removed empty statements (LOG statements ending in ;)
 * 13apr2021,tk MICRO-3028/PR.28960
 * - Added robustness check to DDS_DataWriterQos_is_equal
 * 06apr2021,tk MICRO-2937/PR.28896
 *   - Check that the transport Qos policy does not contain NULL elements.
 * 24feb2021,tk MICRO-2913/PR#28851
 *   - Added always enable preconditions for DDS_DataWriterQos_copy
 *     and DDS_DataWriterQos_initialize.
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
#include "DataWriterQos.h"

const struct DDS_DataWriterQos DDS_DATAWRITER_QOS_DEFAULT =
                                                DDS_DataWriterQos_INITIALIZER;

/*** SOURCE_BEGIN ***/

DDS_ReturnCode_t
DDS_DataWriterQos_copy(struct DDS_DataWriterQos *out,
                       const struct DDS_DataWriterQos *in)
{
    OSAPI_PRECONDITION_ALWAYS((out == NULL) || (in == NULL),
                              return DDS_RETCODE_BAD_PARAMETER,
                              OSAPI_Log_entry_add_pointer("out",out,RTI_FALSE);
                              OSAPI_Log_entry_add_pointer("in",in,RTI_TRUE);)

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

    /* NOTE: data is an an internal variable, don't copy */

    if (DDS_TransportQosPolicy_copy(&out->transport,&in->transport) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t
DDS_DataWriterQos_initialize(struct DDS_DataWriterQos *self)
{
    struct DDS_DataWriterQos initVal = DDS_DATAWRITER_QOS_DEFAULT;

    OSAPI_PRECONDITION_ALWAYS((self == NULL),
                              return DDS_RETCODE_BAD_PARAMETER,
                              OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    *self = initVal;

    return DDS_RETCODE_OK;
}

#ifndef RTI_CERT
DDS_ReturnCode_t
DDS_DataWriterQos_finalize(struct DDS_DataWriterQos * self)
{
    OSAPI_PRECONDITION((self == NULL),
                            return DDS_RETCODE_BAD_PARAMETER,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

   if (DDS_TransportQosPolicy_finalize(&self->transport) != DDS_RETCODE_OK)
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
        !DDS_ReliabilityQosPolicy_is_equal(&left->reliability,
                                           &right->reliability) ||
        !DDS_DurabilityQosPolicy_is_equal(&left->durability,
                                          &right->durability) ||
        !DDS_DestinationOrderQosPolicy_is_equal(&left->destination_order,
                                                &right->destination_order) ||
        !DDS_TransportQosPolicy_is_equal(&left->transport, &right->transport) ||
        !RTI_ManagementQosPolicy_is_equal(&left->management,&right->management) ||
        !DDS_DataWriterResourceLimitsQosPolicy_is_equal(&left->writer_resource_limits,
                                                        &right->writer_resource_limits))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}


DDS_Boolean
DDS_DataWriterQos_is_equal(const struct DDS_DataWriterQos *left,
                        const struct DDS_DataWriterQos *right)
{
    OSAPI_PRECONDITION_ALWAYS(left == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
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
    DDS_Boolean retval = DDS_BOOLEAN_TRUE;

    OSAPI_PRECONDITION((self == NULL),
                            return DDS_BOOLEAN_FALSE,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

#if !RTPS_RELIABILITY
    if (self->reliability.kind == DDS_RELIABLE_RELIABILITY_QOS)
    {
#error "Reliability not enabled.  Please specify RTPS_RELIABILITY"
    }
#endif

    if (!DDS_DeadlineQosPolicy_is_consistent(&self->deadline))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAWRITERQOS_OBJECT,DDSC_LOG_DEADLINE_QOS_POLICY)
        retval = DDS_BOOLEAN_FALSE;
    }

    if (!DDS_LivelinessQosPolicy_is_consistent(&self->liveliness))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAWRITERQOS_OBJECT,DDSC_LOG_LIVELINESS_QOS_POLICY)
        retval = DDS_BOOLEAN_FALSE;
    }

    if (!DDS_HistoryQosPolicy_is_consistent(&self->history))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAWRITERQOS_OBJECT,DDSC_LOG_HISTORY_QOS_POLICY)
        retval = DDS_BOOLEAN_FALSE;
    }

    if (!DDS_ResourceLimitsQosPolicy_is_consistent(&self->resource_limits))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAWRITERQOS_OBJECT,DDSC_LOG_RESOURCE_LIMIT_QOS_POLICY)
        retval = DDS_BOOLEAN_FALSE;
    }

    if (!DDS_DataWriterProtocolQosPolicy_is_consistent(&self->protocol))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAWRITERQOS_OBJECT,DDSC_LOG_PROTOCOL_QOS_POLICY)
        retval = DDS_BOOLEAN_FALSE;
    }

    if (!DDS_ReliabilityQosPolicy_is_consistent(&self->reliability))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAWRITERQOS_OBJECT,DDSC_LOG_RELIABILITY_QOS_POLICY)
        retval = DDS_BOOLEAN_FALSE;
    }

    if (!DDS_DurabilityQosPolicy_is_consistent(&self->durability))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAWRITERQOS_OBJECT,DDSC_LOG_DURABILITY_QOS_POLICY)
        retval = DDS_BOOLEAN_FALSE;
    }

    if (!DDS_OwnershipQosPolicy_is_consistent(&self->ownership))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAWRITERQOS_OBJECT,DDSC_LOG_OWNERSHIP_QOS_POLICY)
        retval = DDS_BOOLEAN_FALSE;
    }

    if (!DDS_OwnershipStrengthQosPolicy_is_consistent(
            &self->ownership_strength))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAWRITERQOS_OBJECT,DDSC_LOG_OWNERSHIP_STRENGTH_QOS_POLICY)
        retval = DDS_BOOLEAN_FALSE;
    }

    if (!DDS_DataWriterResourceLimitsQosPolicy_is_consistent(
                                                &self->writer_resource_limits))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAWRITERQOS_OBJECT,DDSC_LOG_DATAWRITER_RESOURCE_QOS_POLICY)
        retval = DDS_BOOLEAN_FALSE;
    }

    if (!DDS_DestinationOrderQosPolicy_is_consistent(&self->destination_order))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAWRITERQOS_OBJECT,DDSC_LOG_DESTINATION_ORDER_POLICY)

        retval = DDS_BOOLEAN_FALSE;
    }

    {
        RTI_INT32 i;

        for (i = 0; i < DDS_StringSeq_get_length(&self->transport.enabled_transports); ++i)
        {
            if (*DDS_StringSeq_get_reference(&self->transport.enabled_transports,i) == NULL)
            {
                DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                            DDSC_LOG_DATAWRITERQOS_OBJECT,
                            DDSC_LOG_TRANSPORT_QOS_POLICY)

                return DDS_BOOLEAN_FALSE;
            }
        }
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
        retval = DDS_BOOLEAN_FALSE;
    }

    if (self->history.depth > self->resource_limits.max_samples_per_instance)
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICIES(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAWRITERQOS_OBJECT,DDSC_LOG_HISTORY_QOS_POLICY,
                DDSC_LOG_RESOURCE_LIMIT_QOS_POLICY)
        retval = DDS_BOOLEAN_FALSE;
    }

#if DDS_BLOCKING_READER_ENABLED
    /* The datawriter only support a max_blocking_time of 0 */
    if (DDS_Duration_compare(&self->reliability.max_blocking_time,
                             &DDS_DURATION_ZERO) != 0)
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAWRITERQOS_OBJECT,DDSC_LOG_RELIABILITY_QOS_POLICY)
        retval = DDS_BOOLEAN_FALSE;
    }
#endif

    /* A user can specify objectids < OSAPI_SYSTEM_OBJECTID_START.
     * Objectid >= OSAPI_SYSTEM_OBJECTID_START are generated by the
     * OSAPI_System_next_object_id() function.
     */
    if (self->protocol.rtps_object_id >= OSAPI_SYSTEM_OBJECTID_START)
    {
        DDSC_LOG_ILLEGAL_OBJECTID(OSAPI_LOGKIND_ERROR,
                                  self->protocol.rtps_object_id)
        return DDS_BOOLEAN_FALSE;
    }

    return retval;
}

/*ci @} */
