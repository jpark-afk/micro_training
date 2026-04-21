/*
 * FILE: DataReaderQos.c - DataReaderQos implementation
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
 * 18may2022,tk MICRO-3572/PR.30496
 * - Return DDS_RETCODE_BAD_PARAMETER instead of
 *   DDS_RETCODE_PRECONDITION_NOT_MET from
 *   - DDS_DataReaderQos_initialize
 *   - DDS_DataReaderQos_copy
 *   - DDS_DataReaderQos_finalize (not relevant for Cert)
 * 16apr2021,tk MICRO-2845/PR.28682
 *  - Removed empty statements (LOG statements ending in ;)
 * 13apr2021,tk MICRO-3028/PR.28960
 * - Added robustness check to DDS_DataReaderQos_is_equal
 * 06apr2021,tk MICRO-2937/PR.28896
 *   - Check that the transport Qos policy does not contain NULL elements.
 * 24feb2021,tk MICRO-2913/PR#28851
 *   - Added always enable preconditions for DDS_DataReaderQos_copy
 *     and DDS_DataReaderQos_initialize.
 * 14dec2020,tk
 *     - MICRO-2684/PR.28270 Return FALSE in is_consistent() if
 *       disable_unregister_dispose_for_unpublished_instance is TRUE
 * 10jul2015,tk MICRO-1407/PR#15287 Removed magic constant
 * 19feb2015,tk MICRO-1077/PR#14014 Removed redundant code in copy()
 *              MICRO-1078/PR#14017 Removed redundant test code in is_equal()
 * 20may2014,tk MICRO-792 Qos consistency checks
 * 19jul2013,as Fixed MICRO-670 (DDS_DataReaderQos_is_equal)
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief DataReaderQos implementation
 *
 * \details
 * This file implements functions to manage the life-cycle of the datareader
 * QoS policy as well as support functions.
 */
/*ci \addtogroup DDSSubscriptionModule
 * @{
 */
#ifndef netio_config_h
#include "netio/netio_config.h"
#endif
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_subscription_h
#include "dds_c/dds_c_subscription.h"
#endif

#include "QosPolicy.h"
#include "DataReaderQos.h"

const struct DDS_DataReaderQos DDS_DATAREADER_QOS_DEFAULT =
    DDS_DataReaderQos_INITIALIZER;

/*** SOURCE_BEGIN ***/

DDS_ReturnCode_t
DDS_DataReaderQos_copy(struct DDS_DataReaderQos *out,
                       const struct DDS_DataReaderQos *in)
{
    OSAPI_PRECONDITION_ALWAYS(out == NULL || in == NULL,
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("out",out,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("in",in,RTI_TRUE);)

    out->deadline = in->deadline;
    out->history = in->history;
    out->liveliness = in->liveliness;
    out->ownership = in->ownership;
    out->protocol = in->protocol;
    out->reliability = in->reliability;
    out->resource_limits = in->resource_limits;
    out->type_support = in->type_support;
    out->reader_resource_limits = in->reader_resource_limits;
    out->management = in->management;
    out->durability = in->durability;
    out->destination_order = in->destination_order;

    /* NOTE: data is an an internal variable, don't copy */

    if (DDS_TransportQosPolicy_copy(&out->transport,&in->transport) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t
DDS_DataReaderQos_initialize(struct DDS_DataReaderQos * self)
{
    struct DDS_DataReaderQos initVal = DDS_DataReaderQos_INITIALIZER;

    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                            return DDS_RETCODE_BAD_PARAMETER,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    *self = initVal;

    return DDS_RETCODE_OK;
}

#ifndef RTI_CERT
DDS_ReturnCode_t
DDS_DataReaderQos_finalize(struct DDS_DataReaderQos * self)
{
    OSAPI_PRECONDITION(self == NULL,
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
 * \brief Check that the immutable part of a datareader qos policy has changed
 *
 * \param[in] left  Left side of comparison
 * \param[in] right Right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_DataReaderQos_immutable_is_equal(const struct DDS_DataReaderQos *left,
                                     const struct DDS_DataReaderQos *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                            return DDS_BOOLEAN_FALSE,
                            OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    /* NOTE: data is an an internal variable, don't compare */
    if (!DDS_DeadlineQosPolicy_is_equal(&left->deadline,
                                        &right->deadline) ||
        !DDS_LivelinessQosPolicy_is_equal(&left->liveliness,
                                          &right->liveliness) ||
        !DDS_HistoryQosPolicy_is_equal(&left->history,
                                       &right->history) ||
        !DDS_ResourceLimitsQosPolicy_is_equal(&left->resource_limits,
                                              &right->resource_limits) ||
        !DDS_DataReaderResourceLimitsQosPolicy_is_equal(
                                              &left->reader_resource_limits,
                                              &right->reader_resource_limits) ||
        !DDS_OwnershipQosPolicy_is_equal(&left->ownership,
                                         &right->ownership) ||
        !DDS_TypeSupportQosPolicy_is_equal(&left->type_support,
                                           &right->type_support) ||
        !DDS_DataReaderProtocolQosPolicy_is_equal(&left->protocol,
                                                  &right->protocol) ||
        !DDS_ReliabilityQosPolicy_is_equal(&left->reliability,
                                           &right->reliability) ||
        !DDS_DurabilityQosPolicy_is_equal(&left->durability,
                                          &right->durability) ||
        !DDS_DestinationOrderQosPolicy_is_equal(&left->destination_order,
                                                &right->destination_order) ||
        !DDS_TransportQosPolicy_is_equal(&left->transport,
                                         &right->transport) ||
        !RTI_ManagementQosPolicy_is_equal(&left->management,&right->management))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

DDS_Boolean
DDS_DataReaderQos_is_equal(const struct DDS_DataReaderQos *left,
                           const struct DDS_DataReaderQos *right)
{
    OSAPI_PRECONDITION_ALWAYS(left == NULL || right == NULL,
                   return DDS_BOOLEAN_FALSE,
                   OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                   OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    return DDS_DataReaderQos_immutable_is_equal(left, right);
}

/*ci
 * \brief Check that a DataReader qos is consistent
 *
 * \param[in] self DataReader qos to check for consistency
 *
 * \return DDS_BOOLEAN_TRUE if valid, DDS_BOOLEAN_FALSE if not
 */
DDS_Boolean
DDS_DataReaderQos_is_consistent(const struct DDS_DataReaderQos *self)
{
    OSAPI_PRECONDITION(self == NULL,
                            return DDS_RETCODE_PRECONDITION_NOT_MET,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

#if !RTPS_RELIABILITY
    if (self->reliability.kind == DDS_RELIABLE_RELIABILITY_QOS)
    {
        #error "Reliability not enabled.  Please specify RTPS_RELIABILITY" 
    }
#endif

    if (self->management.disable_unregister_dispose_for_unpublished_instance)
    {
        return DDS_BOOLEAN_FALSE;
    }

    /* NOTE: data is an internal variable, don't check */

    if (!DDS_DeadlineQosPolicy_is_consistent(&self->deadline))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAREADERQOS_OBJECT,DDSC_LOG_DEADLINE_QOS_POLICY)
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_LivelinessQosPolicy_is_consistent(&self->liveliness))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAREADERQOS_OBJECT,DDSC_LOG_LIVELINESS_QOS_POLICY)
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_HistoryQosPolicy_is_consistent(&self->history))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAREADERQOS_OBJECT,DDSC_LOG_HISTORY_QOS_POLICY)
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_ResourceLimitsQosPolicy_is_consistent(&self->resource_limits))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAREADERQOS_OBJECT,DDSC_LOG_RESOURCE_LIMIT_QOS_POLICY)
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_OwnershipQosPolicy_is_consistent(&self->ownership))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAREADERQOS_OBJECT,DDSC_LOG_OWNERSHIP_QOS_POLICY)
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_ReliabilityQosPolicy_is_consistent(&self->reliability))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAREADERQOS_OBJECT,DDSC_LOG_RELIABILITY_QOS_POLICY)
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_DurabilityQosPolicy_is_consistent(&self->durability))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_DATAREADERQOS_OBJECT,DDSC_LOG_DURABILITY_QOS_POLICY)
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_DataReaderResourceLimitsQosPolicy_is_consistent(&self->reader_resource_limits))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
        DDSC_LOG_DATAREADERQOS_OBJECT,DDSC_LOG_DATAREADER_RESOURCE_QOS_POLICY)
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_DestinationOrderQosPolicy_is_consistent(&self->destination_order))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
             DDSC_LOG_DATAREADERQOS_OBJECT,DDSC_LOG_DESTINATION_ORDER_POLICY)
        return DDS_BOOLEAN_FALSE;
    }


    if (!DDS_DataReaderProtocolQosPolicy_is_consistent(&self->protocol))
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
             DDSC_LOG_DATAREADERQOS_OBJECT,DDSC_LOG_PROTOCOL_QOS_POLICY)
        return DDS_BOOLEAN_FALSE;
    }

    if (self->history.depth > self->resource_limits.max_samples_per_instance)
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICIES(OSAPI_LOGKIND_ERROR,
                    DDSC_LOG_DATAREADERQOS_OBJECT,DDSC_LOG_HISTORY_QOS_POLICY,
                    DDSC_LOG_RESOURCE_LIMIT_QOS_POLICY)
        return DDS_BOOLEAN_FALSE;
    }

    if (self->resource_limits.max_samples <
            self->reader_resource_limits.max_samples_per_remote_writer)
    {
        DDSC_LOG_QOS_INCONSISTENT_POLICIES(OSAPI_LOGKIND_ERROR,
                    DDSC_LOG_DATAREADERQOS_OBJECT,
                    DDSC_LOG_DATAREADER_RESOURCE_QOS_POLICY,
                    DDSC_LOG_RESOURCE_LIMIT_QOS_POLICY)
        return DDS_BOOLEAN_FALSE;
    }

    {
        RTI_INT32 i;

        for (i = 0; i < DDS_StringSeq_get_length(&self->transport.enabled_transports); ++i)
        {
            if (*DDS_StringSeq_get_reference(&self->transport.enabled_transports,i) == NULL)
            {
                DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                            DDSC_LOG_DATAREADERQOS_OBJECT,
                            DDSC_LOG_TRANSPORT_QOS_POLICY)

                return DDS_BOOLEAN_FALSE;
            }
        }
    }

    return DDS_BOOLEAN_TRUE;
}

/*ci @} */
