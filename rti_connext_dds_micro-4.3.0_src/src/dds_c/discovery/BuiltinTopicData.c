/*
 * FILE: BuiltinTopicData.c - BuiltinTopicData implementation
 *
 * Copyright (c) 2008-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 29may2015,as MICRO-1331 Correct function header comments in _initialize()
 * 19may2015,as MICRO-1193 Refactoring of Sequence API levels
 * 18may2015,tk MICRO-1223/PR#14769 Don’t reallocate strings copy for Cert
 * 18may2015,tk MICRO-1222/PR#14768 Added check for NULL in initialize
 * 20sep2014,as Removed use of deprecated header dds_c_tpolicy_gen.h
 * 15may2014,as MICRO-317 (Verocel PR#1443) Replaced String_cmp with String_ncmp
 * 19mar2014,tk MICRO-74: Support endpoint specific transport
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief BuiltinTopicData implementation
 *
 * \details
 * This file implements the API to manage the DDS built-in topic types for
 * Participants, Publications, and Subscriptions.
 *
 * @ingroup DDSInfrastructureModule
 */
/*ci \addtogroup DDSInfrastructureModule
 * @{
 */
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif

#include "QosPolicy.h"
#include "PartitionQosPolicy.h"
#include "BuiltinTopicData.h"
#include "UserDataQosPolicy.h"

#if DDS_FILTERING_ENABLED
#include "DomainParticipantFilter.h"
#include "DataReaderFilter.h"
#endif

/*** SOURCE_BEGIN ***/

#ifdef T
#undef T
#endif

RTI_PRIVATE RTI_BOOL
DDS_ParticipantBuiltinTopicData_initialize_el(
                                struct DDS_ParticipantBuiltinTopicData *self)
{
    return DDS_ParticipantBuiltinTopicData_initialize(self)
            ? RTI_TRUE : RTI_FALSE;
}

#ifndef RTI_CERT
RTI_PRIVATE RTI_BOOL
DDS_ParticipantBuiltinTopicData_finalize_el(
                                struct DDS_ParticipantBuiltinTopicData *self)
{
    return DDS_ParticipantBuiltinTopicData_finalize(self)
            ? RTI_TRUE : RTI_FALSE;
}
#endif

RTI_PRIVATE RTI_BOOL
DDS_ParticipantBuiltinTopicData_copy_el(
                            struct DDS_ParticipantBuiltinTopicData *out,
                            const struct DDS_ParticipantBuiltinTopicData *in)
{
    return DDS_ParticipantBuiltinTopicData_copy(out,in)
            ? RTI_TRUE : RTI_FALSE;
}

#define T struct DDS_ParticipantBuiltinTopicData
#define TSeq DDS_ParticipantBuiltinTopicDataSeq
#define T_initialize DDS_ParticipantBuiltinTopicData_initialize_el
#ifndef RTI_CERT
#define T_finalize DDS_ParticipantBuiltinTopicData_finalize_el
#endif
#define T_copy DDS_ParticipantBuiltinTopicData_copy_el
#define REDA_SEQUENCE_USER_API
#include "reda/reda_sequence_defn.h"
#include "DomainParticipant.h"


DDS_Boolean
DDS_ParticipantBuiltinTopicData_is_equal(
                        const struct DDS_ParticipantBuiltinTopicData *left,
                        const struct DDS_ParticipantBuiltinTopicData *right)
{
    DDS_Boolean result = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION_ALWAYS((left == NULL) || (right == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    if (!DDS_BuiltinTopicKey_equals(&left->key, &right->key))
    {
        goto done;
    }

    if (!DDS_EntityNameQosPolicy_is_equal(&left->participant_name,
                                          &right->participant_name))
    {
        goto done;
    }

    if (left->dds_builtin_endpoints != right->dds_builtin_endpoints)
    {
        goto done;
    }

    if (!DDS_ProtocolVersion_is_equal(&left->rtps_protocol_version,
                                      &right->rtps_protocol_version))
    {
        goto done;
    }

    if (!DDS_VendorId_is_equal(&left->rtps_vendor_id, &right->rtps_vendor_id))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_is_equal(&left->default_unicast_locators,
                                 &right->default_unicast_locators))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_is_equal(&left->default_multicast_locators,
                                 &right->default_multicast_locators))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_is_equal(&left->metatraffic_unicast_locators,
                                 &right->metatraffic_unicast_locators))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_is_equal(&left->metatraffic_multicast_locators,
                                 &right->metatraffic_multicast_locators))
    {
        goto done;
    }

    if (!DDS_Duration_equal(&left->liveliness_lease_duration,
                            &right->liveliness_lease_duration))
    {
        goto done;
    }

    if (DDS_ProductVersion_compare(&left->product_version,
                                   &right->product_version) != 0)
    {
        goto done;
    }

    if (!DDS_ChecksumProperty_is_equal(&left->checksum,&right->checksum))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_PropertySeq_is_equal(&left->property.value,
                                  &right->property.value))
    {
        goto done;
    }

    if (!DDS_UserDataQosPolicy_is_equal(&left->user_data, &right->user_data))
    {
        goto done;
    }

#if DDS_LIVELINESS_CHANNEL_ENABLED
    if (left->participant_message_reader_reliability_kind !=
        right->participant_message_reader_reliability_kind)
    {
        goto done;
    }

    if (left->builtin_endpoint_qos_mask != right->builtin_endpoint_qos_mask)
    {
        goto done;
    }
#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */

    result = DDS_BOOLEAN_TRUE;

done:
    return result;
}


/*ci
 * \brief Initialize a ParticipantBuiltinTopicData structure
 *
 * \param[in] self ParticipantBuiltinTopicData structure to initialize
 *
 * \return This function always returns DDS_BOOLEAN_TRUE
 */
DDS_Boolean
DDS_ParticipantBuiltinTopicData_initialize(
                                struct DDS_ParticipantBuiltinTopicData *self)
{
    DDS_Boolean result = DDS_BOOLEAN_FALSE;
    struct DDS_ParticipantBuiltinTopicData init_val =
                        DDS_ParticipantBuiltinTopicData_INITIALIZER;

    *self = init_val;

    if (!DDS_LocatorSeq_initialize(&self->default_unicast_locators))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_initialize(&self->default_multicast_locators))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_initialize(&self->metatraffic_unicast_locators))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_initialize(&self->metatraffic_multicast_locators))
    {
        goto done;
    }

    if (!DDS_PropertySeq_initialize(&self->property.value))
    {
        goto done;
    }

    if (DDS_UserDataQosPolicy_initialize(&self->user_data) != DDS_RETCODE_OK)
    {
        goto done;
    }

    result = DDS_BOOLEAN_TRUE;

done:
    return result;
}

DDS_Boolean
DDS_ParticipantBuiltinTopicData_initialize_shallow(
                                struct DDS_ParticipantBuiltinTopicData *self,
                                const struct DDS_DomainParticipantQos *dp_qos)
{
    DDS_Boolean result = DDS_BOOLEAN_FALSE;

    PRECOND_ARG(dp_qos)

    OSAPI_PRECONDITION((self == NULL) || (dp_qos == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);
                        OSAPI_Log_entry_add_pointer("dp_qos",dp_qos,RTI_TRUE);)

    if (!DDS_ParticipantBuiltinTopicData_initialize(self))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_set_maximum(
               &self->default_unicast_locators,
               RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_set_length(&self->default_unicast_locators, 0))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_set_maximum(
               &self->default_multicast_locators,
               RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_set_length(&self->default_multicast_locators, 0))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_set_maximum(
                &self->metatraffic_unicast_locators,
                RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX))
    {
        DDSC_LOG_SEQ_SETMAX(OSAPI_LOGKIND_ERROR,
                            DDSC_LOG_METAUNICAST_SEQUENCE,
                            RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX)
        goto done;
    }

    if (!DDS_LocatorSeq_set_length(&self->metatraffic_unicast_locators, 0))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_set_maximum(
               &self->metatraffic_multicast_locators,
               RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX))
    {
        DDSC_LOG_SEQ_SETMAX(OSAPI_LOGKIND_ERROR,
                            DDSC_LOG_METAMULTICAST_SEQUENCE,
                            RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX)
        goto done;
    }

    if (!DDS_LocatorSeq_set_length(&self->metatraffic_multicast_locators, 0))
    {
        goto done;
    }

    result = DDS_BOOLEAN_TRUE;

done:
    return result;
}

DDS_Boolean
DDS_ParticipantBuiltinTopicData_initialize_from_qos(
                                struct DDS_ParticipantBuiltinTopicData *self,
                                const struct DDS_DomainParticipantQos *dp_qos)
{
    DDS_Boolean result = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (dp_qos == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("dp_qos",dp_qos,RTI_TRUE);)

    if (!DDS_ParticipantBuiltinTopicData_initialize_shallow(self, dp_qos))
    {
        goto done;
    }

    if (!DDS_OctetSeq_set_maximum(&self->user_data.value,
            dp_qos->resource_limits.participant_user_data_max_length))
    {
        goto done;
    }

    if (!DDS_OctetSeq_set_length(&self->user_data.value, 0))
    {
        goto done;
    }

    result = DDS_BOOLEAN_TRUE;
done:
    return result;

}

DDS_Boolean
DDS_ParticipantBuiltinTopicData_finalize_no_dealloc(
                                struct DDS_ParticipantBuiltinTopicData *self,
                                DDS_DomainParticipant *participant)
{
    DDS_Boolean result = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION((self == NULL) || (participant == NULL),
                        goto done,
                        OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("participant",participant,RTI_TRUE);)

    if (DDS_UserDataQosPolicy_finalize_no_dealloc(
            &self->user_data,
            DDS_DomainParticipant_get_user_data_manager(participant),
            DDS_USER_DATA_PARTICIPANT_TYPE) != DDS_RETCODE_OK)
    {
        goto done;
    }

    result = DDS_BOOLEAN_TRUE;

done:
    return result;
}



#ifndef RTI_CERT
DDS_Boolean
DDS_ParticipantBuiltinTopicData_finalize(
                                struct DDS_ParticipantBuiltinTopicData *self)
{
    DDS_Boolean result = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (!DDS_LocatorSeq_finalize(&self->default_unicast_locators))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_finalize(&self->metatraffic_unicast_locators))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_finalize(&self->metatraffic_multicast_locators))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_finalize(&self->default_multicast_locators))
    {
        goto done;
    }

    if (DDS_UserDataQosPolicy_finalize(&self->user_data) != DDS_RETCODE_OK)
    {
        goto done;
    }

    if (!DDS_PropertySeq_finalize(&self->property.value))
    {
        goto done;
    }

    result = DDS_BOOLEAN_TRUE;

done:
    return result;
}
#endif /* !RTI_CERT*/

/*ci
 * \brief Set the contents of a ParticipantBuiltinTopicData structure
 *
 * \details
 * Copy the contents of the source structure to the destination structure. The
 * destination structure must be preallocated and initialized.
 * If shallow_copy is DDS_BOOLEAN_TRUE, then the managed fields are only
 * shallow copied using the participant, otherwise they are deep copied.
 *
 * \param[inout] out The destination ParticipantBuiltinTopicData structure
 * \param[in]    in  The source ParticipantBuiltinTopicData structure
 * \param[in]    shallow_copy Perform a shallow copy of the managed fields
 * \param[in]    participant Optionally, the participant used to shallow copy
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DDS_ParticipantBuiltinTopicData_set_from(
                            struct DDS_ParticipantBuiltinTopicData *out,
                            const struct DDS_ParticipantBuiltinTopicData *in,
                            DDS_Boolean shallow_copy,
                            DDS_DomainParticipant *participant)
{
    DDS_Boolean result = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION((out == NULL) || (in == NULL) ||
                            (shallow_copy && (participant == NULL)),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("out",out,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("in",in,RTI_FALSE);
                        OSAPI_Log_entry_add_uint("shallow_copy",shallow_copy,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("participant",participant,RTI_TRUE);)

    out->key = in->key;
    out->participant_name = in->participant_name;
    out->dds_builtin_endpoints = in->dds_builtin_endpoints;
    out->rtps_protocol_version = in->rtps_protocol_version;
    out->rtps_vendor_id = in->rtps_vendor_id;

    if (!DDS_LocatorSeq_copy(&out->default_unicast_locators,
                             &in->default_unicast_locators))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_copy(&out->default_multicast_locators,
                             &in->default_multicast_locators))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_copy(&out->metatraffic_unicast_locators,
                             &in->metatraffic_unicast_locators))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_copy(&out->metatraffic_multicast_locators,
                             &in->metatraffic_multicast_locators))
    {
        goto done;
    }

    out->liveliness_lease_duration = in->liveliness_lease_duration;
    out->product_version = in->product_version;
    out->checksum = in->checksum;

    if (shallow_copy)
    {
        if (!DDS_UserDataManager_assert_user_data(
                DDS_DomainParticipant_get_user_data_manager(participant),
                DDS_USER_DATA_PARTICIPANT_TYPE,
                &in->user_data.value,
                &out->user_data.value))
        {
            goto done;
        }
    }
    else
    {
        if (DDS_UserDataQosPolicy_copy(&out->user_data, &in->user_data) != DDS_RETCODE_OK)
        {
            goto done;
        }
    }

#if DDS_LIVELINESS_CHANNEL_ENABLED
    out->participant_message_reader_reliability_kind =
                        in->participant_message_reader_reliability_kind;

    out->builtin_endpoint_qos_mask = in->builtin_endpoint_qos_mask;
#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */

    result = DDS_BOOLEAN_TRUE;

done:
    return result;
}

DDS_Boolean
DDS_ParticipantBuiltinTopicData_copy(
                            struct DDS_ParticipantBuiltinTopicData *out,
                            const struct DDS_ParticipantBuiltinTopicData *in)
{
    OSAPI_PRECONDITION_ALWAYS((out == NULL) || (in == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("out",out,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("in",in,RTI_TRUE);)

    return DDS_ParticipantBuiltinTopicData_set_from(out, in, DDS_BOOLEAN_FALSE, NULL);
}

/******************************************************************************
 *                     DDS_PublicationBuiltinTopicData                        *
 ******************************************************************************/

RTI_PRIVATE RTI_BOOL
DDS_PublicationBuiltinTopicData_initialize_el(
                                struct DDS_PublicationBuiltinTopicData *self)
{
    return DDS_PublicationBuiltinTopicData_initialize(self)
            ? RTI_TRUE : RTI_FALSE;
}

#ifndef RTI_CERT
RTI_PRIVATE RTI_BOOL
DDS_PublicationBuiltinTopicData_finalize_el(
                                struct DDS_PublicationBuiltinTopicData *self)
{
    return DDS_PublicationBuiltinTopicData_finalize(self)
            ? RTI_TRUE : RTI_FALSE;
}
#endif

RTI_PRIVATE RTI_BOOL
DDS_PublicationBuiltinTopicData_copy_el(
                            struct DDS_PublicationBuiltinTopicData *out,
                            const struct DDS_PublicationBuiltinTopicData *in)
{
    return DDS_PublicationBuiltinTopicData_copy(out,in)
            ? RTI_TRUE : RTI_FALSE;
}

#define T struct DDS_PublicationBuiltinTopicData
#define TSeq DDS_PublicationBuiltinTopicDataSeq
#define T_initialize DDS_PublicationBuiltinTopicData_initialize_el
#ifndef RTI_CERT
#define T_finalize DDS_PublicationBuiltinTopicData_finalize_el
#endif
#define T_copy DDS_PublicationBuiltinTopicData_copy_el
#define REDA_SEQUENCE_USER_API
#include "reda/reda_sequence_defn.h"

DDS_Boolean
DDS_PublicationBuiltinTopicData_is_equal(
                        const struct DDS_PublicationBuiltinTopicData *left,
                        const struct DDS_PublicationBuiltinTopicData *right)
{
    DDS_Boolean result = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION_ALWAYS((left == NULL) || (right == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    if (!DDS_BuiltinTopicKey_equals(&left->key, &right->key))
    {
        goto done;
    }

    if (!DDS_BuiltinTopicKey_equals(&left->participant_key,
                                    &right->participant_key))
    {
        goto done;
    }

    if (DDS_String_ncmp(left->topic_name, right->topic_name,
                        RTPS_PATHNAME_LEN_MAX))
    {
        goto done;
    }

    if (DDS_String_ncmp(left->type_name, right->type_name,
            RTPS_PATHNAME_LEN_MAX))
    {
        goto done;
    }

    if (!DDS_DeadlineQosPolicy_is_equal(&left->deadline, &right->deadline))
    {
        goto done;
    }

    if (!DDS_ReliabilityQosPolicy_is_equal(&left->reliability,
                                           &right->reliability))
    {
        goto done;
    }

    if (!DDS_OwnershipQosPolicy_is_equal(&left->ownership, &right->ownership))
    {
        goto done;
    }

    if (!DDS_DurabilityQosPolicy_is_equal(&left->durability, &right->durability))
    {
        goto done;
    }

    if (!DDS_OwnershipStrengthQosPolicy_is_equal(&left->ownership_strength,
                                                 &right->ownership_strength))
    {
        goto done;
    }

    if (!DDS_LivelinessQosPolicy_is_equal(&left->liveliness,
                                          &right->liveliness))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_is_equal(&left->unicast_locator,
                                 &right->unicast_locator))
    {
        goto done;
    }

    if (!DDS_DestinationOrderQosPolicy_is_equal(&left->destination_order,
                                                &right->destination_order))
    {
        goto done;
    }

    if (!DDS_LatencyBudgetQosPolicy_is_equal(&left->latency_budget,
                                             &right->latency_budget))
    {
        goto done;
    }

    if (!DDS_PartitionQosPolicy_is_equal(&left->partition, &right->partition))
    {
        goto done;
    }

    if (!DDS_UserDataQosPolicy_is_equal(&left->user_data, &right->user_data))
    {
        goto done;
    }

    if (!DDS_GroupDataQosPolicy_is_equal(&left->group_data, &right->group_data))
    {
        goto done;
    }

    if (!DDS_TopicDataQosPolicy_is_equal(&left->topic_data, &right->topic_data))
    {
        goto done;
    }

    result = DDS_BOOLEAN_TRUE;

done:
    return result;
}

/*ci
 * \brief Initialize a PublicationBuiltinTopicData structure
 *
 * \param[in] self PublicationBuiltinTopicData structure to initialize
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DDS_PublicationBuiltinTopicData_initialize(
                                struct DDS_PublicationBuiltinTopicData *self)
{
    DDS_Boolean result = DDS_BOOLEAN_FALSE;
    struct DDS_PublicationBuiltinTopicData init_val =
                        DDS_PublicationBuiltinTopicData_INITIALIZER;

    *self = init_val;

    if (!DDS_LocatorSeq_initialize(&self->unicast_locator))
    {
        goto done;
    }

    if (!DDS_DataRepresentationIdSeq_initialize(&self->representation.value))
    {
        goto done;
    }

    if (DDS_PartitionQosPolicy_initialize(&self->partition) != DDS_RETCODE_OK)
    {
        goto done;
    }

    if (DDS_UserDataQosPolicy_initialize(&self->user_data) != DDS_RETCODE_OK)
    {
        goto done;
    }
    if (DDS_GroupDataQosPolicy_initialize(&self->group_data) != DDS_RETCODE_OK)
    {
        goto done;
    }
    if (DDS_TopicDataQosPolicy_initialize(&self->topic_data) != DDS_RETCODE_OK)
    {
        goto done;
    }

    result = DDS_BOOLEAN_TRUE;

done:
    return result;
}

DDS_Boolean
DDS_PublicationBuiltinTopicData_initialize_shallow(
                                struct DDS_PublicationBuiltinTopicData *self,
                                const struct DDS_DomainParticipantQos *dp_qos)
{
    DDS_Boolean result = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION((self == NULL) || (dp_qos == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);
                        OSAPI_Log_entry_add_pointer("dp_qos",dp_qos,RTI_TRUE);)

    if (!DDS_PublicationBuiltinTopicData_initialize(self))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_set_maximum(&self->unicast_locator,
                                    RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_set_length(&self->unicast_locator, 0))
    {
        goto done;
    }

    if (!DDS_DataRepresentationIdSeq_set_maximum(&self->representation.value,
                                                    DDS_TYPE_ENCAPSULATION_MAX_IDS))
    {
        goto done;
    }

    if (!DDS_DataRepresentationIdSeq_set_length(&self->representation.value, 0))
    {
        goto done;
    }

    if (!DDS_StringSeq_set_maximum(&self->partition.name,
            dp_qos->resource_limits.max_partitions))
    {
        goto done;
    }

    if (!DDS_StringSeq_set_length(&self->partition.name,0))
    {
        goto done;
    }

    result = DDS_BOOLEAN_TRUE;

done:
    return result;
}

DDS_Boolean
DDS_PublicationBuiltinTopicData_initialize_from_qos(
                                struct DDS_PublicationBuiltinTopicData *self,
                                const struct DDS_DomainParticipantQos *dp_qos)
{
    DDS_Boolean result = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (dp_qos == NULL),
                                return DDS_BOOLEAN_FALSE,
                                OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                                OSAPI_Log_entry_add_pointer("dp_qos",dp_qos,RTI_TRUE);)

    if (!DDS_PublicationBuiltinTopicData_initialize(self))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_set_maximum(&self->unicast_locator,
                                    RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_set_length(&self->unicast_locator, 0))
    {
        goto done;
    }

    if (!DDS_DataRepresentationIdSeq_set_maximum(&self->representation.value,
                                                    DDS_TYPE_ENCAPSULATION_MAX_IDS))
    {
        goto done;
    }

    if (!DDS_DataRepresentationIdSeq_set_length(&self->representation.value, 0))
    {
        goto done;
    }

    if (!DDS_PartitionQosPolicy_set_maximum_w_max(&self->partition,dp_qos))
    {
        goto done;
    }

    if (!DDS_StringSeq_set_length(&self->partition.name,0))
    {
        goto done;
    }

    self->type_name = DDS_String_alloc(RTPS_PATHNAME_LEN_MAX);
    if (self->type_name == NULL)
    {
        goto done;
    }

    self->topic_name = DDS_String_alloc(RTPS_PATHNAME_LEN_MAX);
    if (self->topic_name == NULL)
    {
        goto done;
    }

    if (!DDS_OctetSeq_set_maximum(&self->user_data.value,
            dp_qos->resource_limits.writer_user_data_max_length))
    {
        goto done;
    }

    if (!DDS_OctetSeq_set_length(&self->user_data.value, 0))
    {
        goto done;
    }

    if (!DDS_OctetSeq_set_maximum(&self->group_data.value,
            dp_qos->resource_limits.publisher_group_data_max_length))
    {
        goto done;
    }

    if (!DDS_OctetSeq_set_length(&self->group_data.value, 0))
    {
        goto done;
    }

    if (!DDS_OctetSeq_set_maximum(&self->topic_data.value,
            dp_qos->resource_limits.topic_data_max_length))
    {
        goto done;
    }

    if (!DDS_OctetSeq_set_length(&self->topic_data.value, 0))
    {
        goto done;
    }

    result = DDS_BOOLEAN_TRUE;
done:

    return result;
}

DDS_Boolean
DDS_PublicationBuiltinTopicData_finalize_no_dealloc(
                                struct DDS_PublicationBuiltinTopicData *self,
                                DDS_DomainParticipant *participant)
{
    DDS_Boolean result = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION((self == NULL) || (participant == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("participant",participant,RTI_TRUE);)

    if (self->topic_name != NULL)
    {
        if (!DDS_StringManager_delete_string(
                                participant->string_manager,self->topic_name))
        {
            goto done;
        }
        self->topic_name = NULL;
    }

    if (self->type_name != NULL)
    {
        if (!DDS_StringManager_delete_string(participant->string_manager,
                                                            self->type_name))
        {
            goto done;
        }
        self->type_name = NULL;
    }

    if (DDS_PartitionQosPolicy_finalize_no_dealloc(&self->partition,
            DDS_DomainParticipant_get_partition_string_manager(participant))
            != DDS_RETCODE_OK)
    {
        goto done;
    }

    if (DDS_UserDataQosPolicy_finalize_no_dealloc(
            &self->user_data,
            DDS_DomainParticipant_get_user_data_manager(participant),
            DDS_USER_DATA_DATAWRITER_TYPE) != DDS_RETCODE_OK)
    {
        goto done;
    }

    if (DDS_GroupDataQosPolicy_finalize_no_dealloc(
            &self->group_data,
            DDS_DomainParticipant_get_user_data_manager(participant),
            DDS_USER_DATA_PUBLISHER_TYPE) != DDS_RETCODE_OK)
    {
        goto done;
    }

    if (DDS_TopicDataQosPolicy_finalize_no_dealloc(
            &self->topic_data,
            DDS_DomainParticipant_get_user_data_manager(participant),
            DDS_USER_DATA_TOPIC_TYPE) != DDS_RETCODE_OK)
    {
        goto done;
    }

    result = DDS_BOOLEAN_TRUE;

done:

    return result;
}

#ifndef RTI_CERT
DDS_Boolean
DDS_PublicationBuiltinTopicData_finalize(
                                struct DDS_PublicationBuiltinTopicData *self)
{
    DDS_Boolean result = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (self->topic_name != NULL)
    {
        DDS_String_free(self->topic_name);
        self->topic_name = NULL;
    }

    if (self->type_name != NULL)
    {
        DDS_String_free(self->type_name);
        self->type_name = NULL;
    }

    if (!DDS_LocatorSeq_finalize(&self->unicast_locator))
    {
        goto done;
    }

    if (DDS_DataRepresentationQosPolicy_finalize(&self->representation) !=
                                                                DDS_RETCODE_OK)
    {
        goto done;
    }

    if (DDS_PartitionQosPolicy_finalize(&self->partition))
    {
        goto done;
    }

    if (DDS_UserDataQosPolicy_finalize(&self->user_data) != DDS_RETCODE_OK)
    {
        goto done;
    }

    if (DDS_GroupDataQosPolicy_finalize(&self->group_data) != DDS_RETCODE_OK)
    {
        goto done;
    }

    if (DDS_TopicDataQosPolicy_finalize(&self->topic_data) != DDS_RETCODE_OK)
    {
        goto done;
    }

    result = DDS_BOOLEAN_TRUE;

done:

    return result;
}
#endif /* !RTI_CERT */

/*ci
 * \brief Set the contents of a PublicationBuiltinTopicData structure
 *
 * \details
 * Copy the contents of the source structure to the destination structure. The
 * destination structure must be preallocated and initialized.
 * If shallow_copy is DDS_BOOLEAN_TRUE, then the managed fields are only
 * shallow copied using the participant, otherwise they are deep copied.
 *
 * \param[inout] out The destination PublicationBuiltinTopicData structure
 * \param[in]    in  The source PublicationBuiltinTopicData structure
 * \param[in]    shallow_copy Perform a shallow copy of the managed fields
 * \param[in]    participant Optionally, the participant used to shallow copy
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DDS_PublicationBuiltinTopicData_set_from(
                                    struct DDS_PublicationBuiltinTopicData *out,
                                    const struct DDS_PublicationBuiltinTopicData *in,
                                    DDS_Boolean shallow_copy,
                                    DDS_DomainParticipant *participant)
{
    DDS_Boolean result = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION((out == NULL) || (in == NULL) ||
                (shallow_copy && (participant == NULL)) ||
                (in->type_name == NULL) ||
                (in->topic_name == NULL),
            return DDS_BOOLEAN_FALSE,
            OSAPI_Log_entry_add_pointer("out",out,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("in",in,RTI_FALSE);
            OSAPI_Log_entry_add_uint("shallow_copy",shallow_copy,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("participant",participant,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("type_name",(in != NULL ? in->type_name : NULL),RTI_FALSE);
            OSAPI_Log_entry_add_pointer("topic_name",(in != NULL ? in->topic_name : NULL),RTI_TRUE);)

    out->key = in->key;
    out->participant_key = in->participant_key;

    if (shallow_copy)
    {
        if (out->topic_name != NULL)
        {
            /* Prevent potential resource leak; this object should have
             * had finalize_no_dealloc called on it before it was reused.
             */
            goto done;
        }
        out->topic_name = DDS_StringManager_assert_string(
                            participant->string_manager,in->topic_name);
        if (out->topic_name == NULL)
        {
            goto done;
        }

        if (out->type_name != NULL)
        {
            goto done;
        }
        out->type_name = DDS_StringManager_assert_string(
                            participant->string_manager,in->type_name);
        if (out->type_name == NULL)
        {
            goto done;
        }

        if (!DDS_PartitionQosPolicy_set_from(&out->partition, &in->partition,
                DDS_DomainParticipant_get_partition_string_manager(participant)))
        {
            goto done;
        }

        if (!DDS_UserDataManager_assert_user_data(
                participant->user_data_manager,
                DDS_USER_DATA_DATAWRITER_TYPE,
                &in->user_data.value,
                &out->user_data.value))
        {
            goto done;
        }

        if (!DDS_UserDataManager_assert_user_data(
                participant->user_data_manager,
                DDS_USER_DATA_PUBLISHER_TYPE,
                &in->group_data.value,
                &out->group_data.value))
        {
            goto done;
        }

        if (!DDS_UserDataManager_assert_user_data(
                participant->user_data_manager,
                DDS_USER_DATA_TOPIC_TYPE,
                &in->topic_data.value,
                &out->topic_data.value))
        {
            goto done;
        }

    }
    else
    {
        if (out->topic_name == NULL)
        {
            out->topic_name = DDS_String_alloc(RTPS_PATHNAME_LEN_MAX);
            if (out->topic_name == NULL)
            {
                goto done;
            }
        }

        if (!DDS_String_copy(&out->topic_name,&in->topic_name,RTPS_PATHNAME_LEN_MAX))
        {
            goto done;
        }

        if (out->type_name == NULL)
        {
            out->type_name = DDS_String_alloc(RTPS_PATHNAME_LEN_MAX);
            if (out->type_name == NULL)
            {
                goto done;
            }
        }

        if (!DDS_String_copy(&out->type_name,&in->type_name,RTPS_PATHNAME_LEN_MAX))
        {
            goto done;
        }

        if (!DDS_PartitionQosPolicy_copy(&out->partition, &in->partition))
        {
            goto done;
        }

        if (DDS_UserDataQosPolicy_copy(&out->user_data, &in->user_data) != DDS_RETCODE_OK)
        {
            goto done;
        }

        if (DDS_GroupDataQosPolicy_copy(&out->group_data, &in->group_data) != DDS_RETCODE_OK)
        {
            goto done;
        }

        if (DDS_TopicDataQosPolicy_copy(&out->topic_data, &in->topic_data) != DDS_RETCODE_OK)
        {
            goto done;
        }
    }

    out->deadline = in->deadline;
    out->reliability = in->reliability;
    out->liveliness = in->liveliness;
    out->ownership = in->ownership;
    out->ownership_strength = in->ownership_strength;
    out->durability = in->durability;
    out->destination_order = in->destination_order;
    out->latency_budget = in->latency_budget;

    if (DDS_DataRepresentationQosPolicy_copy(&out->representation, &in->representation)
            != DDS_RETCODE_OK)
    {
        goto done;
    }

    if (!DDS_LocatorSeq_copy(&out->unicast_locator, &in->unicast_locator))
    {
        goto done;
    }

    result = DDS_BOOLEAN_TRUE;

done:
    return result;
}

DDS_Boolean
DDS_PublicationBuiltinTopicData_copy(
                              struct DDS_PublicationBuiltinTopicData *out,
                              const struct DDS_PublicationBuiltinTopicData *in)
{
    OSAPI_PRECONDITION_ALWAYS((out == NULL) || (in == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("out",out,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("in",in,RTI_TRUE);)

    return DDS_PublicationBuiltinTopicData_set_from(out, in, DDS_BOOLEAN_FALSE, NULL);
}

DDS_Boolean
DDS_PublicationBuiltinTopicData_copy_from_datawriter(
        struct DDS_PublicationBuiltinTopicData* data_out,
        DDS_DataWriter *datawriter)
{
    DDS_Publisher *pub = NULL;
    DDS_Topic *topic = NULL;
    DDS_TopicDescription *topic_desc = NULL;
    DDS_DomainParticipant *dp = NULL;
    DDS_InstanceHandle_t instance_handle = DDS_HANDLE_NIL;
    DDS_Boolean result = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION((data_out == NULL) || (datawriter == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("data_out",data_out,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("datawriter",datawriter,RTI_TRUE);)

    pub = DDS_DataWriter_get_publisher(datawriter);
    topic = DDS_DataWriter_get_topic(datawriter);
    topic_desc = DDS_Topic_as_topicdescription(topic);
    dp = DDS_Publisher_get_participant(pub);

    if ( (pub == NULL) || (topic == NULL) || (topic_desc == NULL) || (dp == NULL))
    {
        goto done;
    }

    /* key */
    instance_handle = DDS_Entity_get_instance_handle(DDS_DataWriter_as_entity(datawriter));
    DDS_BuiltinTopicKey_from_instance_handle(&data_out->key, &instance_handle);

    /* participant_key */
    instance_handle = DDS_Entity_get_instance_handle(DDS_DomainParticipant_as_entity(dp));
    DDS_BuiltinTopicKey_from_instance_handle(&data_out->participant_key, &instance_handle);

    /* topic_name and type_name */
    if (data_out->topic_name == NULL)
    {
        data_out->topic_name = DDS_String_alloc(RTPS_PATHNAME_LEN_MAX);
        if (data_out->topic_name == NULL)
        {
            goto done;
        }
    }

    if (!REDA_String_copy(data_out->topic_name, RTPS_PATHNAME_LEN_MAX,
                         DDS_TopicDescription_get_name(topic_desc)))
    {
        goto done;
    }

    if (data_out->type_name == NULL)
    {
        data_out->type_name = DDS_String_alloc(RTPS_PATHNAME_LEN_MAX);
        if (data_out->type_name == NULL)
        {
            goto done;
        }
    }

    if (!REDA_String_copy(data_out->type_name, RTPS_PATHNAME_LEN_MAX,
                         DDS_TopicDescription_get_type_name(topic_desc)))
    {
        goto done;
    }

    /* QoSes */
    data_out->deadline = datawriter->deadline;
    data_out->ownership = datawriter->ownership;
    data_out->ownership_strength = datawriter->ownership_strength;
    data_out->latency_budget = datawriter->latency_budget;
    data_out->reliability = datawriter->reliability;
    data_out->liveliness = datawriter->liveliness;
    data_out->durability = datawriter->durability;
    data_out->destination_order = datawriter->destination_order;

    /* Locators */
    if (!DDS_LocatorSeq_copy(&data_out->unicast_locator,
                             datawriter->writer_data.unicast_locator))
    {
        goto done;
    }

    /* representation */
    if (DDS_DataRepresentationQosPolicy_copy(&data_out->representation,
                                             datawriter->representation) != DDS_RETCODE_OK)
    {
        goto done;
    }

    /* User data */
    if (DDS_UserDataQosPolicy_copy(&data_out->user_data,
                                    datawriter->user_data) != DDS_RETCODE_OK)
    {
        goto done;
    }

    if (DDS_Publisher_copy_group_data(pub,&data_out->group_data) != DDS_RETCODE_OK)
    {
        goto done;
    }

    if (DDS_Topic_get_topic_data(topic,&data_out->topic_data) != DDS_RETCODE_OK)
    {
        goto done;
    }

    result = DDS_BOOLEAN_TRUE;

done:
    return result;
}

/******************************************************************************
 *                     DDS_SubscriptionBuiltinTopicData                       *
 ******************************************************************************/
RTI_PRIVATE RTI_BOOL
DDS_SubscriptionBuiltinTopicData_initialize_el(
                                struct DDS_SubscriptionBuiltinTopicData *self)
{
    return DDS_SubscriptionBuiltinTopicData_initialize(self)
            ? RTI_TRUE : RTI_FALSE;
}

#ifndef RTI_CERT
RTI_PRIVATE RTI_BOOL
DDS_SubscriptionBuiltinTopicData_finalize_el(
                                struct DDS_SubscriptionBuiltinTopicData *self)
{
    return DDS_SubscriptionBuiltinTopicData_finalize(self)
            ? RTI_TRUE : RTI_FALSE;
}
#endif

RTI_PRIVATE RTI_BOOL
DDS_SubscriptionBuiltinTopicData_copy_el(
                            struct DDS_SubscriptionBuiltinTopicData *out,
                            const struct DDS_SubscriptionBuiltinTopicData *in)
{
    return DDS_SubscriptionBuiltinTopicData_copy(out,in)
            ? RTI_TRUE : RTI_FALSE;
}

#define T struct DDS_SubscriptionBuiltinTopicData
#define TSeq DDS_SubscriptionBuiltinTopicDataSeq
#define T_initialize DDS_SubscriptionBuiltinTopicData_initialize_el
#ifndef RTI_CERT
#define T_finalize DDS_SubscriptionBuiltinTopicData_finalize_el
#endif
#define T_copy DDS_SubscriptionBuiltinTopicData_copy_el
#define REDA_SEQUENCE_USER_API
#include "reda/reda_sequence_defn.h"

DDS_Boolean
DDS_SubscriptionBuiltinTopicData_immutable_is_equal(
                        const struct DDS_SubscriptionBuiltinTopicData *left,
                        const struct DDS_SubscriptionBuiltinTopicData *right)
{
    DDS_Boolean result = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION((left == NULL) || (right == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    if (!DDS_BuiltinTopicKey_equals(&left->key, &right->key))
    {
        goto done;
    }

    if (!DDS_BuiltinTopicKey_equals(&left->participant_key,
                                    &right->participant_key))
    {
        goto done;
    }

    if (DDS_String_ncmp(left->topic_name, right->topic_name,
            RTPS_PATHNAME_LEN_MAX))
    {
        goto done;
    }

    if (DDS_String_ncmp(left->type_name, right->type_name,
            RTPS_PATHNAME_LEN_MAX))
    {
        goto done;
    }

    if (!DDS_DeadlineQosPolicy_is_equal(&left->deadline, &right->deadline))
    {
        goto done;
    }

    if (!DDS_ReliabilityQosPolicy_is_equal(&left->reliability,
                                           &right->reliability))
    {
        goto done;
    }

    if (!DDS_OwnershipQosPolicy_is_equal(&left->ownership, &right->ownership))
    {
        goto done;
    }

    if (!DDS_LivelinessQosPolicy_is_equal(&left->liveliness,
                                          &right->liveliness))
    {
        goto done;
    }

    if (!DDS_DurabilityQosPolicy_is_equal(&left->durability, &right->durability))
    {
        goto done;
    }

    if (!DDS_DestinationOrderQosPolicy_is_equal(&left->destination_order,
                                                &right->destination_order))
    {
        goto done;
    }

    if (!DDS_LocatorExSeq_is_equal(&left->unicast_locator,
                                       &right->unicast_locator))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_is_equal(&left->multicast_locator,
                                  &right->multicast_locator))
    {
        goto done;
    }

    if (!DDS_LatencyBudgetQosPolicy_is_equal(&left->latency_budget,
                                             &right->latency_budget))
    {
        goto done;
    }

    if (!DDS_PartitionQosPolicy_is_equal(&left->partition, &right->partition))
    {
        goto done;
    }

    if (!DDS_UserDataQosPolicy_is_equal(&left->user_data, &right->user_data))
    {
        goto done;
    }

    if (!DDS_GroupDataQosPolicy_is_equal(&left->group_data, &right->group_data))
    {
        goto done;
    }

    if (!DDS_TopicDataQosPolicy_is_equal(&left->topic_data, &right->topic_data))
    {
        goto done;
    }

    result = DDS_BOOLEAN_TRUE;

done:
    return result;
}

DDS_Boolean
DDS_SubscriptionBuiltinTopicData_is_equal(
                        const struct DDS_SubscriptionBuiltinTopicData *left,
                        const struct DDS_SubscriptionBuiltinTopicData *right)
{
    DDS_Boolean result = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION_ALWAYS((left == NULL) || (right == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    if (!DDS_SubscriptionBuiltinTopicData_immutable_is_equal(left, right))
    {
        goto done;
    }

#if DDS_FILTERING_ENABLED
    if (!DDS_ContentFilterProperty_is_equal(&left->content_filter, &right->content_filter))
    {
        goto done;
    }
#endif

    result = DDS_BOOLEAN_TRUE;

done:
    return result;
}

/*ci
 * \brief Initialize a SubscriptionBuiltinTopicData structure
 *
 * \param[in] self SubscriptionBuiltinTopicData structure to initialize
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DDS_SubscriptionBuiltinTopicData_initialize(
                                struct DDS_SubscriptionBuiltinTopicData *self)
{
    DDS_Boolean result = DDS_BOOLEAN_FALSE;
    struct DDS_SubscriptionBuiltinTopicData init_val =
            DDS_SubscriptionBuiltinTopicData_INITIALIZER;

    *self = init_val;

    if (!DDS_LocatorExSeq_initialize(&self->unicast_locator))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_initialize(&self->multicast_locator))
    {
        goto done;
    }

    if (!DDS_DataRepresentationIdSeq_initialize(&self->representation.value))
    {
        goto done;
    }

    if (DDS_PartitionQosPolicy_initialize(&self->partition) != DDS_RETCODE_OK)
    {
        goto done;
    }

    if (DDS_UserDataQosPolicy_initialize(&self->user_data) != DDS_RETCODE_OK)
    {
        goto done;
    }

    if (DDS_GroupDataQosPolicy_initialize(&self->group_data) != DDS_RETCODE_OK)
    {
        goto done;
    }

    if (DDS_TopicDataQosPolicy_initialize(&self->topic_data) != DDS_RETCODE_OK)
    {
        goto done;
    }

#if DDS_FILTERING_ENABLED
    if (!DDS_ContentFilterProperty_initialize(&self->content_filter))
    {
        goto done;
    }
#endif

    result = DDS_BOOLEAN_TRUE;

done:
    return result;
}

DDS_Boolean
DDS_SubscriptionBuiltinTopicData_initialize_shallow(
                                struct DDS_SubscriptionBuiltinTopicData *self,
                                DDS_DomainParticipant *participant)
{
    DDS_Boolean result = DDS_BOOLEAN_FALSE;
    struct DDS_DomainParticipantQos *dp_qos = NULL;

    OSAPI_PRECONDITION((self == NULL) || (participant == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("participant",participant,RTI_TRUE);)

    dp_qos = DDS_DomainParticipant_get_qos_ref(participant);

    if (!DDS_SubscriptionBuiltinTopicData_initialize(self))
    {
        goto done;
    }

    if (!DDS_LocatorExSeq_set_maximum(&self->unicast_locator,
                                    RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX))
    {
        goto done;
    }

    if (!DDS_LocatorExSeq_set_length(&self->unicast_locator, 0))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_set_maximum(&self->multicast_locator,
                                    RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_set_length(&self->multicast_locator, 0))
    {
        goto done;
    }

    if (!DDS_DataRepresentationIdSeq_set_maximum(&self->representation.value,
                                                    DDS_TYPE_ENCAPSULATION_MAX_IDS))
    {
        goto done;
    }

    if (!DDS_DataRepresentationIdSeq_set_length(&self->representation.value,
                                                    0))
    {
        goto done;
    }

    if (!DDS_StringSeq_set_maximum(&self->partition.name,
            dp_qos->resource_limits.max_partitions))
    {
        goto done;
    }

    if (!DDS_StringSeq_set_length(&self->partition.name,0))
    {
        goto done;
    }

#if DDS_FILTERING_ENABLED
    if (DDS_DomainParticipant_is_filtering_enabled(participant))
    {
        if (!DDS_StringSeq_set_maximum(&self->content_filter.expression_parameters,
                dp_qos->filter.resource_limits.filter_parameter_max_count_per_expression))
        {
            goto done;
        }
        if (!DDS_StringSeq_set_length(&self->content_filter.expression_parameters,0))
        {
            goto done;
        }
    }
#endif

    result = DDS_BOOLEAN_TRUE;

done:
    return result;
}

DDSCDllExport DDS_Boolean
DDS_SubscriptionBuiltinTopicData_initialize_from_qos(
                                struct DDS_SubscriptionBuiltinTopicData *self,
                                const struct DDS_DomainParticipantQos *dp_qos)
{
    DDS_Boolean result = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (dp_qos == NULL),
                                return DDS_BOOLEAN_FALSE,
                                OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                                OSAPI_Log_entry_add_pointer("dp_qos",dp_qos,RTI_TRUE);)

    if (!DDS_SubscriptionBuiltinTopicData_initialize(self))
    {
        goto done;
    }

    if (!DDS_LocatorExSeq_set_maximum(&self->unicast_locator,
                                    RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX))
    {
        goto done;
    }

    if (!DDS_LocatorExSeq_set_length(&self->unicast_locator, 0))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_set_maximum(&self->multicast_locator,
                                    RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_set_length(&self->multicast_locator, 0))
    {
        goto done;
    }

    if (!DDS_DataRepresentationIdSeq_set_maximum(&self->representation.value,
                                                    DDS_TYPE_ENCAPSULATION_MAX_IDS))
    {
        goto done;
    }

    if (!DDS_DataRepresentationIdSeq_set_length(&self->representation.value, 0))
    {
        goto done;
    }

    if (!DDS_PartitionQosPolicy_set_maximum_w_max(&self->partition,dp_qos))
    {
        goto done;
    }

    if (!DDS_StringSeq_set_length(&self->partition.name,0))
    {
        goto done;
    }

    self->type_name = DDS_String_alloc(RTPS_PATHNAME_LEN_MAX);
    if (self->type_name == NULL)
    {
        goto done;
    }

    self->topic_name = DDS_String_alloc(RTPS_PATHNAME_LEN_MAX);
    if (self->topic_name == NULL)
    {
        goto done;
    }

    if (!DDS_OctetSeq_set_maximum(&self->user_data.value,
            dp_qos->resource_limits.reader_user_data_max_length))
    {
        goto done;
    }

    if (!DDS_OctetSeq_set_length(&self->user_data.value, 0))
    {
        goto done;
    }

    if (!DDS_OctetSeq_set_maximum(&self->group_data.value,
            dp_qos->resource_limits.subscriber_group_data_max_length))
    {
        goto done;
    }

    if (!DDS_OctetSeq_set_length(&self->group_data.value, 0))
    {
        goto done;
    }

    if (!DDS_OctetSeq_set_maximum(&self->topic_data.value,
            dp_qos->resource_limits.topic_data_max_length))
    {
        goto done;
    }

    if (!DDS_OctetSeq_set_length(&self->topic_data.value, 0))
    {
        goto done;
    }

#if DDS_FILTERING_ENABLED
    if (!RT_ComponentFactoryId_is_nil(&dp_qos->filter.name))
    {
        self->content_filter.content_filtered_topic_name = DDS_String_alloc(RTPS_PATHNAME_LEN_MAX);
        if (self->content_filter.content_filtered_topic_name == NULL)
        {
            goto done;
        }

        self->content_filter.related_topic_name = DDS_String_alloc(RTPS_PATHNAME_LEN_MAX);
        if (self->content_filter.related_topic_name == NULL)
        {
            goto done;
        }

        self->content_filter.filter_class_name = DDS_String_alloc(RTPS_PATHNAME_LEN_MAX);
        if (self->content_filter.filter_class_name == NULL)
        {
            goto done;
        }

        self->content_filter.filter_expression = DDS_String_alloc(
                (RTI_SIZE_T)dp_qos->filter.resource_limits.filter_expression_max_length);
        if (self->content_filter.filter_expression == NULL)
        {
            goto done;
        }

        if (!DDS_StringSeq_set_maximum_w_max(
                &self->content_filter.expression_parameters,
                dp_qos->filter.resource_limits.filter_parameter_max_count_per_expression,
                (RTI_UINT32)dp_qos->filter.resource_limits.filter_parameter_max_length))
        {
            goto done;
        }
    }
#endif

    result = DDS_BOOLEAN_TRUE;
done:

    return result;
}

DDS_Boolean
DDS_SubscriptionBuiltinTopicData_finalize_no_dealloc(
                                struct DDS_SubscriptionBuiltinTopicData *self,
                                DDS_DomainParticipant *participant)
{
    DDS_Boolean result = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION((self == NULL) || (participant == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("participant",participant,RTI_TRUE);)

    if (self->topic_name != NULL)
    {
        if (!DDS_StringManager_delete_string(
                participant->string_manager,self->topic_name))
        {
            goto done;
        }
        self->topic_name = NULL;
    }

    if (self->type_name != NULL)
    {
        if (!DDS_StringManager_delete_string(participant->string_manager,
                                                            self->type_name))
        {
            goto done;
        }
        self->type_name = NULL;
    }

    if (DDS_PartitionQosPolicy_finalize_no_dealloc(&self->partition,
            DDS_DomainParticipant_get_partition_string_manager(participant))
            != DDS_RETCODE_OK)
    {
        goto done;
    }

    if (DDS_UserDataQosPolicy_finalize_no_dealloc(
            &self->user_data,
            DDS_DomainParticipant_get_user_data_manager(participant),
            DDS_USER_DATA_DATAREADER_TYPE) != DDS_RETCODE_OK)
    {
        goto done;
    }

    if (DDS_GroupDataQosPolicy_finalize_no_dealloc(
            &self->group_data,
            DDS_DomainParticipant_get_user_data_manager(participant),
            DDS_USER_DATA_SUBSCRIBER_TYPE) != DDS_RETCODE_OK)
    {
        goto done;
    }

    if (DDS_TopicDataQosPolicy_finalize_no_dealloc(
            &self->topic_data,
            DDS_DomainParticipant_get_user_data_manager(participant),
            DDS_USER_DATA_TOPIC_TYPE) != DDS_RETCODE_OK)
    {
        goto done;
    }

#if DDS_FILTERING_ENABLED
    if (DDS_DomainParticipant_is_filtering_enabled(participant))
    {
        DDS_DomainParticipant_filter_property_finalize_shallow_copy(participant,
                                                                    &self->content_filter);
    }
#endif

    result = DDS_BOOLEAN_TRUE;

done:
    return result;
}

#ifndef RTI_CERT
DDS_Boolean
DDS_SubscriptionBuiltinTopicData_finalize(
                                struct DDS_SubscriptionBuiltinTopicData *self)
{
    DDS_Boolean result = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (self->topic_name != NULL)
    {
        DDS_String_free(self->topic_name);
        self->topic_name = NULL;
    }

    if (self->type_name != NULL)
    {
        DDS_String_free(self->type_name);
        self->type_name = NULL;
    }

    if (!DDS_LocatorExSeq_finalize(&self->unicast_locator))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_finalize(&self->multicast_locator))
    {
        goto done;
    }

    if (DDS_DataRepresentationQosPolicy_finalize(&self->representation) !=
                                                                DDS_RETCODE_OK)
    {
        goto done;
    }

    if (DDS_PartitionQosPolicy_finalize(&self->partition) != DDS_RETCODE_OK)
    {
        goto done;
    }

    if (DDS_UserDataQosPolicy_finalize(&self->user_data) != DDS_RETCODE_OK)
    {
        goto done;
    }

    if (DDS_GroupDataQosPolicy_finalize(&self->group_data) != DDS_RETCODE_OK)
    {
        goto done;
    }

    if (DDS_TopicDataQosPolicy_finalize(&self->topic_data) != DDS_RETCODE_OK)
    {
        goto done;
    }

#if DDS_FILTERING_ENABLED
    if (!DDS_ContentFilterProperty_finalize(&self->content_filter))
    {
        goto done;
    }
#endif

    result = DDS_BOOLEAN_TRUE;

done:
    return result;
}
#endif /* !RTI_CERT */

/*ci
 * \brief Set the contents of a SubscriptionBuiltinTopicData structure
 *
 * \details
 * Copy the contents of the source structure to the destination structure. The
 * destination structure must be preallocated and initialized.
 * If shallow_copy is DDS_BOOLEAN_TRUE, then the managed fields are only
 * shallow copied using the participant, otherwise they are deep copied.
 *
 * \param[inout] out The destination SubscriptionBuiltinTopicData structure
 * \param[in]    in  The source SubscriptionBuiltinTopicData structure
 * \param[in]    shallow_copy Perform a shallow copy of the managed fields
 * \param[in]    participant Optionally, the participant used to shallow copy
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DDS_SubscriptionBuiltinTopicData_set_from(
                            struct DDS_SubscriptionBuiltinTopicData *out,
                            const struct DDS_SubscriptionBuiltinTopicData *in,
                            DDS_Boolean shallow_copy,
                            DDS_DomainParticipant *participant)
{
    DDS_Boolean result = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION((out == NULL) || (in == NULL) ||
                (shallow_copy && (participant == NULL)) ||
                (in->type_name == NULL) ||
                (in->topic_name == NULL),
            return DDS_BOOLEAN_FALSE,
            OSAPI_Log_entry_add_pointer("out",out,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("in",in,RTI_FALSE);
            OSAPI_Log_entry_add_uint("shallow_copy",shallow_copy,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("participant",participant,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("type_name",(in != NULL ? in->type_name : NULL),RTI_FALSE);
            OSAPI_Log_entry_add_pointer("topic_name",(in != NULL ? in->topic_name : NULL),RTI_TRUE);)

    out->key = in->key;
    out->participant_key = in->participant_key;

    if (shallow_copy)
    {
        if (out->topic_name != NULL)
        {
            /* Prevent potential resource leak; this object should have
             * had finalize_no_dealloc called on it before it was reused.
             */
            goto done;
        }
        out->topic_name = DDS_StringManager_assert_string(
                            participant->string_manager,in->topic_name);
        if (out->topic_name == NULL)
        {
            goto done;
        }

        if (out->type_name != NULL)
        {
            goto done;
        }

        out->type_name = DDS_StringManager_assert_string(
                            participant->string_manager,in->type_name);
        if (out->type_name == NULL)
        {
            goto done;
        }

        if (!DDS_PartitionQosPolicy_set_from(&out->partition, &in->partition,
               DDS_DomainParticipant_get_partition_string_manager(participant)))
        {
            goto done;
        }

        if (!DDS_UserDataManager_assert_user_data(
                participant->user_data_manager,
                DDS_USER_DATA_DATAREADER_TYPE,
                &in->user_data.value,
                &out->user_data.value))
        {
            goto done;
        }

        if (!DDS_UserDataManager_assert_user_data(
                participant->user_data_manager,
                DDS_USER_DATA_SUBSCRIBER_TYPE,
                &in->group_data.value,
                &out->group_data.value))
        {
            goto done;
        }

        if (!DDS_UserDataManager_assert_user_data(
                participant->user_data_manager,
                DDS_USER_DATA_TOPIC_TYPE,
                &in->topic_data.value,
                &out->topic_data.value))
        {
            goto done;
        }
    }
    else
    {
        if (out->topic_name == NULL)
        {
            out->topic_name = DDS_String_alloc(RTPS_PATHNAME_LEN_MAX);
            if (out->topic_name == NULL)
            {
                goto done;
            }
        }

        if (!DDS_String_copy(&out->topic_name,&in->topic_name,RTPS_PATHNAME_LEN_MAX))
        {
            goto done;
        }

        if (out->type_name == NULL)
        {
            out->type_name = DDS_String_alloc(RTPS_PATHNAME_LEN_MAX);
            if (out->type_name == NULL)
            {
                goto done;
            }
        }

        if (!DDS_String_copy(&out->type_name,&in->type_name,RTPS_PATHNAME_LEN_MAX))
        {
            goto done;
        }

        if (!DDS_PartitionQosPolicy_copy(&out->partition, &in->partition))
        {
            goto done;
        }

        if (DDS_UserDataQosPolicy_copy(&out->user_data, &in->user_data) != DDS_RETCODE_OK)
        {
            goto done;
        }

        if (DDS_GroupDataQosPolicy_copy(&out->group_data, &in->group_data) != DDS_RETCODE_OK)
        {
            goto done;
        }

        if (DDS_TopicDataQosPolicy_copy(&out->topic_data, &in->topic_data) != DDS_RETCODE_OK)
        {
            goto done;
        }
    }

    out->deadline = in->deadline;
    out->reliability = in->reliability;
    out->liveliness = in->liveliness;
    out->ownership = in->ownership;
    out->durability = in->durability;
    out->destination_order = in->destination_order;
    out->latency_budget = in->latency_budget;
    out->presentation = in->presentation;

    if (DDS_DataRepresentationQosPolicy_copy(&out->representation, &in->representation)
            != DDS_RETCODE_OK)
    {
        goto done;
    }

    if (!DDS_LocatorExSeq_copy(&out->unicast_locator, &in->unicast_locator))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_copy(&out->multicast_locator, &in->multicast_locator))
    {
        goto done;
    }

#if DDS_FILTERING_ENABLED
    if (shallow_copy && DDS_DomainParticipant_is_filtering_enabled(participant))
    {
        if (!DDS_DomainParticipant_filter_property_shallow_copy(
                participant, &out->content_filter, &in->content_filter))
        {
            goto done;
        }
    }
    else
    {
        if (!DDS_ContentFilterProperty_copy(&out->content_filter, &in->content_filter))
        {
            goto done;
        }
    }
#endif

    result = DDS_BOOLEAN_TRUE;

done:
    return result;
}

DDS_Boolean
DDS_SubscriptionBuiltinTopicData_copy(
                            struct DDS_SubscriptionBuiltinTopicData *out,
                            const struct DDS_SubscriptionBuiltinTopicData *in)
{
    OSAPI_PRECONDITION_ALWAYS((out == NULL) || (in == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("out",out,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("in",in,RTI_TRUE);)

    return DDS_SubscriptionBuiltinTopicData_set_from(out, in, DDS_BOOLEAN_FALSE, NULL);
}

DDS_Boolean
DDS_SubscriptionBuiltinTopicData_copy_from_datareader(
        struct DDS_SubscriptionBuiltinTopicData* data_out,
        DDS_DataReader *datareader)
{
    DDS_Subscriber *sub = NULL;
    DDS_Topic *topic = NULL;
    DDS_TopicDescription *topic_desc = NULL;
    DDS_DomainParticipant *dp = NULL;
    DDS_InstanceHandle_t instance_handle = DDS_HANDLE_NIL;
    DDS_Boolean result = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION((data_out == NULL) || (datareader == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("data_out",data_out,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("datareader",datareader,RTI_TRUE);)

    sub = DDS_DataReader_get_subscriber(datareader);
    topic = DDS_DataReader_get_topic(datareader);
    topic_desc = DDS_Topic_as_topicdescription(topic);
    dp = DDS_Subscriber_get_participant(sub);

    if ((sub == NULL) || (topic == NULL) || (topic_desc == NULL) || (dp == NULL))
    {
        goto done;
    }

    /* key */
    instance_handle = DDS_Entity_get_instance_handle(DDS_DataReader_as_entity(datareader));
    DDS_BuiltinTopicKey_from_instance_handle(&data_out->key, &instance_handle);

    /* participant_key */
    instance_handle = DDS_Entity_get_instance_handle(DDS_DomainParticipant_as_entity(dp));
    DDS_BuiltinTopicKey_from_instance_handle(&data_out->participant_key, &instance_handle);

    /* topic_name and type_name */
    if (data_out->topic_name == NULL)
    {
        data_out->topic_name = DDS_String_alloc(RTPS_PATHNAME_LEN_MAX);
        if (data_out->topic_name == NULL)
        {
            goto done;
        }
    }

    if (!REDA_String_copy(data_out->topic_name, RTPS_PATHNAME_LEN_MAX,
                         DDS_TopicDescription_get_name(topic_desc)))
    {
        goto done;
    }

    if (data_out->type_name == NULL)
    {
        data_out->type_name = DDS_String_alloc(RTPS_PATHNAME_LEN_MAX);
        if (data_out->type_name == NULL)
        {
            goto done;
        }
    }

    if (!REDA_String_copy(data_out->type_name, RTPS_PATHNAME_LEN_MAX,
                         DDS_TopicDescription_get_type_name(topic_desc)))
    {
        goto done;
    }

    /* QoSes */
    data_out->deadline = datareader->deadline;
    data_out->ownership = datareader->ownership;
    data_out->latency_budget = datareader->latency_budget;
    data_out->reliability = datareader->reliability;
    data_out->liveliness = datareader->liveliness;
    data_out->durability = datareader->durability;
    data_out->destination_order = datareader->destination_order;

    /* Locators */
    {
        const struct DDS_LocatorSeq *src_seq;

        src_seq = DDS_DataReader_get_unicast_locator_ref(datareader);
        if (!DDS_LocatorExSeq_copy_from(&data_out->unicast_locator,src_seq))
        {
            goto done;
        }

        src_seq = DDS_DataReader_get_multicast_locator_ref(datareader);
        if (!DDS_LocatorSeq_copy(&data_out->multicast_locator,src_seq))
        {
            goto done;
        }
    }

    /* presentation */
    data_out->presentation = DDS_PRESENTATION_QOS_PUBLICATION_DEFAULT;

    /* representation */
    if (DDS_DataRepresentationQosPolicy_copy(&data_out->representation,
                                 datareader->representation) != DDS_RETCODE_OK)
    {
        goto done;
    }

    /* User data */
    if (DDS_UserDataQosPolicy_copy(&data_out->user_data,
                                    datareader->user_data) != DDS_RETCODE_OK)
    {
        goto done;
    }

    if (DDS_Subscriber_copy_group_data(sub,&data_out->group_data) != DDS_RETCODE_OK)
    {
        goto done;
    }

    if (DDS_Topic_get_topic_data(topic,&data_out->topic_data) != DDS_RETCODE_OK)
    {
        goto done;
    }

#if DDS_FILTERING_ENABLED
    if (!DDS_DataReader_get_filter_property(datareader, &data_out->content_filter))
    {
        goto done;
    }
#endif

    result = DDS_BOOLEAN_TRUE;

done:
    return result;
}

/*ci @} */
