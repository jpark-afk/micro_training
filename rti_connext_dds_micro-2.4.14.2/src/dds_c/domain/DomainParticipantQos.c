/*
 * FILE: DomainParticipantQos.c - DomainParticipantQos implementation
 *
 * (c) Copyright, Real-Time Innovations, 2008-2015. 
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
 *   - DDS_DomainParticipantQos_initialize
 *   - DDS_DomainParticipantQos_copy
 *   - DDS_DomainParticipantQos_finalize (not relevant for Cert)
 * 13apr2021,tk MICRO-3028/PR.28960
 * - Added robustness check to DDS_DomainParticipantQos_is_equal
 * 06apr2021,tk MICRO-2937/PR.28896
 *   - Check that the transport, user_traffic and disocvery enabled_transports
 *     Qos policy does not contain NULL elements.
 * 24feb2021,tk MICRO-2913/PR#28851
 *   - Added always enable preconditions for DDS_DomainParticipantQos_copy
 *     and DDS_DomainParticipantQos_initialize.
 * 20feb2015,eh  MICRO-813/PR#9172 Fix Lint warnings
 * 20sep2014,as Remove initialize operation for QosPolicy types (already initialized by Qos
 *              type's initialize operation) 
 * 19may2014,tk MICRO-792: Update DomainParticipantQoS support functions
 * 15may2014,as MICRO-317 (Verocel PR#1443) Replaced String_cmp with String_ncmp
 * 28apr2014,as MICRO-371 (Verocel PR#1527) Removed use of OSAPI_Memory_compare
 *              in DDS_DomainParticipantQos_is_equal and other is_equal
 *              operations, in favor of direct comparison of qos structure's
 *              members where possible. Refactored is_equal() to leverage
 *              immutable_is_equal().
 * 19jul2013,as  Added support for C++
 * 27jun2012,tk  Major update
 * 30apr2008,tk  Written
 */
/*ce
 * \file
 * \brief  DomainParticipantQos implementation
 *
 * \details
 * This file implements functions to manage the life-cycle of the participant
 * QoS policy.
 */
/*ci \addtogroup DDSDomainModule
 * @{
 */
#ifndef dds_c_infrastructure_h
#include "dds_c/dds_c_infrastructure.h"
#endif
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_domain_h
#include "dds_c/dds_c_domain.h"
#endif

#include "QosPolicy.h"
#include "DomainQosPolicy.h"
#include "DomainParticipantQos.h"

/*** SOURCE_BEGIN ***/

const struct DDS_DomainParticipantQos DDS_PARTICIPANT_QOS_DEFAULT =
                                        DDS_DomainParticipantQos_INITIALIZER;

DDS_ReturnCode_t
DDS_DomainParticipantQos_copy(struct DDS_DomainParticipantQos *out,
                              const struct DDS_DomainParticipantQos *in)
{
    OSAPI_PRECONDITION_ALWAYS(out == NULL || in == NULL,
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("out",out,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("in",in,RTI_TRUE);)

    out->entity_factory = in->entity_factory;

    if (DDS_DiscoveryQosPolicy_copy(&out->discovery,
                                    &in->discovery) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    out->resource_limits = in->resource_limits;
    out->participant_name = in->participant_name;
    out->protocol = in->protocol;

    if (DDS_TransportQosPolicy_copy(&out->transports,
                                     &in->transports) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_UserTrafficQosPolicy_copy(&out->user_traffic,
                                       &in->user_traffic) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t
DDS_DomainParticipantQos_initialize(struct DDS_DomainParticipantQos *self)
{
    struct DDS_DomainParticipantQos val = DDS_DomainParticipantQos_INITIALIZER;

    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    *self = val;

    return DDS_RETCODE_OK;
}

#ifndef RTI_CERT
DDS_ReturnCode_t
DDS_DomainParticipantQos_finalize(struct DDS_DomainParticipantQos *self)
{
    OSAPI_PRECONDITION(self == NULL,
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (DDS_TransportQosPolicy_finalize(&self->transports) != DDS_RETCODE_OK
        || DDS_DiscoveryQosPolicy_finalize(&self->discovery) != DDS_RETCODE_OK
        || DDS_UserTrafficQosPolicy_finalize(&self->user_traffic) != DDS_RETCODE_OK )
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}
#endif /* !RTI_CERT */

/*ci
 * \brief Check that the immutable part a participant qos has changed
 *
 * \param[in] left  Left side of comparison
 * \param[in] right Right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_DomainParticipantQos_immutable_is_equal(
                                const struct DDS_DomainParticipantQos *left,
                                const struct DDS_DomainParticipantQos *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    if (!DDS_DiscoveryQosPolicy_is_equal(&left->discovery,&right->discovery) ||
        !DDS_DomainParticipantResourceLimitsQosPolicy_is_equal(
                                                    &left->resource_limits,
                                                    &right->resource_limits) ||
        REDA_String_ncompare(left->participant_name.name,
                             right->participant_name.name,
                             DDS_ENTITYNAME_QOS_NAME_MAX) ||
        !DDS_WireProtocolQosPolicy_is_equal(&left->protocol,&right->protocol) ||
        !DDS_TransportQosPolicy_is_equal(&left->transports,&right->transports) ||
        !DDS_UserTrafficQosPolicy_is_equal(&left->user_traffic,&right->user_traffic))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

DDS_Boolean
DDS_DomainParticipantQos_is_equal(const struct DDS_DomainParticipantQos *left,
                                  const struct DDS_DomainParticipantQos *right)
{
    OSAPI_PRECONDITION_ALWAYS(left == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    if (left->entity_factory.autoenable_created_entities !=
        right->entity_factory.autoenable_created_entities)
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_DomainParticipantQos_immutable_is_equal(left,right);
}


/*ci
 * \brief Check that a DDS_DomainParticipantQos is consistent
 *
 * \param[in] self DDS_DomainParticipantQos to check
 *
 * \return DDS_BOOLEAN_TRUE if valid, DDS_BOOLEAN_FALSE if not
 */
DDS_Boolean
DDS_DomainParticipantQos_is_consistent(
                                const struct DDS_DomainParticipantQos *self)
{
    OSAPI_PRECONDITION(self == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (!DDS_DomainParticipantResourceLimitsQosPolicy_is_consistent(
                                                       &self->resource_limits))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_WireProtocolQosPolicy_is_consistent(&self->protocol))
    {
        return DDS_BOOLEAN_FALSE;
    }

    {
        RTI_INT32 i;

        for (i = 0; i < DDS_StringSeq_get_length(&self->transports.enabled_transports); ++i)
        {
            if (*DDS_StringSeq_get_reference(&self->transports.enabled_transports,i) == NULL)
            {
                DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                            DDSC_LOG_PARTICIPANTQOS_OBJECT,
                            DDSC_LOG_TRANSPORT_QOS_POLICY)

                return DDS_BOOLEAN_FALSE;
            }
        }

        for (i = 0; i < DDS_StringSeq_get_length(&self->user_traffic.enabled_transports); ++i)
        {
            if (*DDS_StringSeq_get_reference(&self->user_traffic.enabled_transports,i) == NULL)
            {
                DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                            DDSC_LOG_PARTICIPANTQOS_OBJECT,
                            DDSC_LOG_USER_TRAFFIC_QOS_POLICY)

                return DDS_BOOLEAN_FALSE;
            }
        }

        for (i = 0; i < DDS_StringSeq_get_length(&self->discovery.enabled_transports); ++i)
        {
            if (*DDS_StringSeq_get_reference(&self->discovery.enabled_transports,i) == NULL)
            {
                DDSC_LOG_QOS_INCONSISTENT_POLICY(OSAPI_LOGKIND_ERROR,
                            DDSC_LOG_PARTICIPANTQOS_OBJECT,
                            DDSC_LOG_DISCOVERY_TRAFFIC_QOS_POLICY)

                return DDS_BOOLEAN_FALSE;
            }
        }
    }

    return DDS_BOOLEAN_TRUE;
}

/*ci @} */
