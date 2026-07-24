/*
 * FILE: DataWriterImpl.c - DataWriter implementation
 *
 * Copyright (c) 2012-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 21sep2016,tk  MICRO-1546 Removed special handling of best-effort/no deadline
 *                          treat it as any other Qos
 * 04nov2015,tk  MICRO-1505 Reduce memory footprint
 * 17jul2015,tk  MICRO-1442/PR#15468 Pass plugin_data to get_key_kind and
 *                                   get_serialized_key_max_size
 * 09jul2015,tk  MICRO-1369          Properly handle auto-registration of
 *                                   dispose and unregister
 * 30jun2015,tk  MICRO-1376/PR#15191 Fixed comment for is_consistent()
 * 29mar2015,tk  MICRO-1117/PR#14253 Removed invalid comment
 * 11jun2015,tk  MICRO-1119/PR#14255 Improved and simplified key-handling code
 * 16mar2015,tk  MICRO-1117/PR#14253 Removed magic constants and commented on
 *                                   how the maximum sample buffer is created
 * 16mar2015,tk  MICRO-1118/PR#14254 Removed redundant code
 * 16mar2015,tk  MICRO-1120/PR#14256 Use NETIO_RTPS_FLAGS_UNREGISTER instead
 *                                   of RTPS_UNREGISTER_STATUS_INFO (same value)
 * 16mar2015,tk  MICRO-1121/PR#14257 Return resources in all cases of failure
 * 19sep2014,tk  MICRO-875 Added checks for writer enabled where applicable
 * 16sep2014,as  MICRO-903/PR#11236 - Incorrect handling of status events and listeners
 * 31jul2014,tk  MICRO-842/PR#9683 - Removed superfluous parameters in DB
 *                                  compare function
 * 19mar2014,tk  MICRO-74: Support endpoint specific transport
 * 11nov2013,as  MICRO-718 Add get_X_status() API to DataReader and DataWriter
 * 08nov2013,as  MICRO-681 Complete implementation of WaitSets
 *               and support for StatusConditions
 * 06jun2013,kaj MICRO-183: CDR stream alignment reset moved to (de)ser_header
 * 23mar2013,tk  Updated logging, robustness checks
 * 06feb2013,eh  Refix MICRO-180: md5Stream only for USER_KEY
 * 06feb2013,eh  Fix sending keyHash for GUID_KEY
 * 06feb2013,eh  MICRO-262: assign RTPS resource limits
 * 14dec2012,kaj MICRO-180: DDS_DataWriterImpl_initialize was not
 *                         allocating md5Stream with serialized_key_max_size
 * 06jun2012,tk Written
 */
/*ce
 * \file
 * \brief DataWriter implementation
 *
 * \details
 * This file implements internal functions needed to support the public
 * DDS datawriter API, mainly related to the life-cycle of a DDS datawriter
 * object.
 */
/*ci \addtogroup DDSPublicationModule
 * @{
 */
#ifndef dds_c_config_h
#include "dds_c/dds_c_config.h"
#endif
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif
#ifndef reda_bufferpool_h
#include "reda/reda_bufferpool.h"
#endif
#ifndef netio_address_h
#include "netio/netio_address.h"
#endif
#ifndef netio_route_h
#include "netio/netio_route.h"
#endif
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_discovery_plugin_h
#include "dds_c/dds_c_discovery_plugin.h"

#endif

#include "InstanceHandle.h"
#include "QosPolicy.h"
#include "Transport.h"
#include "Entity.h"
#include "TopicDescription.h"
#include "Topic.h"
#include "Type.h"
#include "DataWriterQos.h"
#include "DataWriterEvent.h"
#include "DataWriterInterface.h"
#include "DataWriterImpl.h"
#include "PublisherImpl.h"
#include "DataWriterProperty.h"
#include "UserDataQosPolicy.h"
#include "BuiltinCdr.h"

#if DDS_FLOW_CONTROLLER_ENABLED
#include "FlowControl.h"
#endif

#if DDS_FILTERING_ENABLED
#include "DataWriterFilter.h"
#endif

const char* const DDSHST_WRITER_DEFAULT_HISTORY_NAME = "wh";
const char* const DDS_DEFAULT_DATAWRITER_NETIO_NAME = "wi";

/*** SOURCE_BEGIN ***/

DDS_ReturnCode_t
DDS_DataWriter_get_qos_from(DDS_DataWriter *self,
                               struct DDS_DataWriterQos *out)
{
    OSAPI_PRECONDITION((self == NULL) || (out == NULL),
                        return DDS_RETCODE_BAD_PARAMETER,
                        OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("out",out,RTI_FALSE);)

    out->deadline = self->deadline;
    out->liveliness = self->liveliness;
    out->history = self->history;
    out->resource_limits = self->resource_limits;
    out->ownership = self->ownership;
    out->ownership_strength = self->ownership_strength;
    out->reliability = self->reliability;
    out->protocol = self->protocol;
    out->type_support = self->type_support;
    out->management = self->management;
    out->durability = self->durability;
    out->writer_resource_limits = self->writer_resource_limits;
    out->destination_order = self->destination_order;
    out->transfer_mode = self->transfer_mode;
    out->latency_budget = self->latency_budget;

    if (!REDA_String_copy(out->publication_name.name,DDS_ENTITYNAME_QOS_NAME_MAX,
                     self->publication_name))
    {
        return DDS_RETCODE_ERROR;
    }

    /* NOTE: data is an an internal variable, don't copy */

    if (DDS_TransportQosPolicy_copy(&out->transport,&self->transport) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_UserDataQosPolicy_copy(&out->user_data,self->user_data) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_PropertyQosPolicy_copy(&out->property,&self->property) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_TransportEncapsulationQosPolicy_copy(&out->encapsulation,
                                         self->encapsulation) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_DataRepresentationQosPolicy_copy(&out->representation,
                                     self->representation) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_PublishModeQosPolicy_copy(&out->publish_mode,
                                      self->publish_mode) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t
DDS_DataWriter_set_qos_from(DDS_DataWriter *self,
                            const struct DDS_DataWriterQos *in,
                            DDS_DomainParticipant *participant)
{
    OSAPI_PRECONDITION((self == NULL) || (in == NULL) || (participant == NULL),
                        return DDS_RETCODE_BAD_PARAMETER,
                        OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("in",in,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("participant",participant,RTI_TRUE);)

    self->deadline = in->deadline;
    self->liveliness = in->liveliness;
    self->history = in->history;
    self->resource_limits = in->resource_limits;
    self->ownership = in->ownership;
    self->ownership_strength = in->ownership_strength;
    self->reliability = in->reliability;
    self->protocol = in->protocol;
    self->type_support = in->type_support;
    self->management = in->management;
    self->durability = in->durability;
    self->writer_resource_limits = in->writer_resource_limits;
    self->destination_order = in->destination_order;
    self->transfer_mode = in->transfer_mode;
    self->latency_budget = in->latency_budget;

    if (DDS_PropertyQosPolicy_copy(&self->property,&in->property) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_TransportQosPolicy_copy(&self->transport,&in->transport) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    /* Optional Qos policies may require memory allocation
     * user_data (ptr)
     * encapsulation (ptr)
     * representation
     * publish_mode
     * publication_name
     */
    if (!DDS_UserDataQosPolicy_is_equal(self->user_data,&in->user_data))
    {
        if (self->user_data == &DDS_USER_DATA_DEFAULT)
        {
            OSAPI_Heap_allocate_struct(&self->user_data,
                                       struct DDS_UserDataQosPolicy);
            if (self->user_data == NULL)
            {
                return DDS_RETCODE_OUT_OF_RESOURCES;
            }
            DDS_UserDataQosPolicy_initialize(self->user_data);
        }

        if (!DDS_UserDataManager_assert_user_data(
                DDS_DomainParticipant_get_user_data_manager(participant),
                DDS_USER_DATA_DATAWRITER_TYPE,
                &in->user_data.value,
                &self->user_data->value))
        {
            return DDS_RETCODE_ERROR;
        }
    }

    if (REDA_String_compare(self->publication_name,in->publication_name.name))
    {
#ifndef RTI_CERT
        if (self->publication_name != DDS_ENTITY_NAME_DEFAULT)
        {
            REDA_String_free(self->publication_name);
        }
#endif
        self->publication_name = REDA_String_dup(in->publication_name.name);
    }

    /* NOTE: data is an an internal variable, don't copy */

    if (!DDS_TransportEncapsulationQosPolicy_is_equal(self->encapsulation,
                                                      &in->encapsulation))
    {
        if (self->encapsulation == &DDS_TRANSPORT_ENCAPSULATION_DEFAULT)
        {
            OSAPI_Heap_allocate_struct(&self->encapsulation,
                                   struct DDS_TransportEncapsulationQosPolicy);
            if (self->encapsulation == NULL)
            {
                return DDS_RETCODE_OUT_OF_RESOURCES;
            }
            *self->encapsulation = DDS_TRANSPORT_ENCAPSULATION_DEFAULT;
        }
        if (DDS_TransportEncapsulationQosPolicy_copy(self->encapsulation,
                                                     &in->encapsulation) != DDS_RETCODE_OK)
        {
            return DDS_RETCODE_ERROR;
        }
    }

    if (!DDS_DataRepresentationQosPolicy_is_equal(self->representation,
                                                  &in->representation))
    {
        if (self->representation == &DDS_DATAREPRESENTATION_DEFAULT)
        {
            OSAPI_Heap_allocate_struct(&self->representation,
                                   struct DDS_DataRepresentationQosPolicy);
            if (self->representation == NULL)
            {
                return DDS_RETCODE_OUT_OF_RESOURCES;
            }
            *self->representation = DDS_DATAREPRESENTATION_DEFAULT;
        }
        if (DDS_DataRepresentationQosPolicy_copy(self->representation,
                                                 &in->representation) != DDS_RETCODE_OK)
        {
            return DDS_RETCODE_ERROR;
        }
    }

    if (!DDS_PublishModeQosPolicy_is_equal(self->publish_mode,&in->publish_mode))
    {
        if (self->publish_mode == &DDS_PUBLISHMODE_DEFAULT)
        {
            OSAPI_Heap_allocate_struct(&self->publish_mode,
                                       struct DDS_PublishModeQosPolicy);
            if (self->publish_mode == NULL)
            {
                return DDS_RETCODE_OUT_OF_RESOURCES;
            }
            *self->publish_mode = DDS_PUBLISHMODE_DEFAULT;
        }
        if (DDS_PublishModeQosPolicy_copy(self->publish_mode,
                                          &in->publish_mode) != DDS_RETCODE_OK)
        {
            return DDS_RETCODE_ERROR;
        }
    }

    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t
DDS_DataWriter_finalize_managed(DDS_DataWriter *self,
                                DDS_DomainParticipant *participant)
{
    OSAPI_PRECONDITION((self == NULL) || (participant == NULL),
                       return DDS_RETCODE_BAD_PARAMETER,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);
                       OSAPI_Log_entry_add_pointer("participant",participant,RTI_FALSE);)

    if (self->user_data != &DDS_USER_DATA_DEFAULT)
    {
        if (DDS_UserDataQosPolicy_finalize_no_dealloc(
                self->user_data,
                DDS_DomainParticipant_get_user_data_manager(participant),
                DDS_USER_DATA_DATAWRITER_TYPE) != DDS_RETCODE_OK)
        {
            return DDS_RETCODE_ERROR;
        }
#ifndef RTI_CERT
        OSAPI_Heap_free_struct(self->user_data);
#endif
        /* the cast is ok, we never write to this variable unless allocated */
        self->user_data = (struct DDS_UserDataQosPolicy*)&DDS_USER_DATA_DEFAULT;
    }

#ifndef RTI_CERT
    if (DDS_TransportQosPolicy_finalize(&self->transport) != DDS_RETCODE_OK)
    {
       return DDS_RETCODE_ERROR;
    }
#endif

    if (self->encapsulation != &DDS_TRANSPORT_ENCAPSULATION_DEFAULT)
    {
#ifndef RTI_CERT
        if (DDS_TransportEncapsulationQosPolicy_finalize(self->encapsulation) != DDS_RETCODE_OK)
        {
            return DDS_RETCODE_ERROR;
        }
        OSAPI_Heap_free_struct(self->encapsulation);
#endif
        /* the cast is ok, we never write to this variable unless allocated */
        self->encapsulation = (struct DDS_TransportEncapsulationQosPolicy*)&DDS_TRANSPORT_ENCAPSULATION_DEFAULT;
    }

    if (self->representation != &DDS_DATAREPRESENTATION_DEFAULT)
    {
#ifndef RTI_CERT
        if (DDS_DataRepresentationQosPolicy_finalize(self->representation) != DDS_RETCODE_OK)
        {
            return DDS_RETCODE_ERROR;
        }
        OSAPI_Heap_free_struct(self->representation);
#endif
        /* the cast is ok, we never write to this variable unless allocated */
        self->representation = (struct DDS_DataRepresentationQosPolicy*)&DDS_DATAREPRESENTATION_DEFAULT;
    }

    if (self->publish_mode != &DDS_PUBLISHMODE_DEFAULT)
    {
#ifndef RTI_CERT
        if (DDS_PublishModeQosPolicy_finalize(self->publish_mode) != DDS_RETCODE_OK)
        {
            return DDS_RETCODE_ERROR;
        }
        OSAPI_Heap_free_struct(self->publish_mode);
#endif
    }

    if (self->publication_name != DDS_ENTITY_NAME_DEFAULT)
    {
#ifndef RTI_CERT
        OSAPI_Heap_free_string(self->publication_name);
#endif
        /* the cast is ok, we never write to this variable unless allocated */
        self->publication_name = (char*)DDS_ENTITY_NAME_DEFAULT;
    }

#ifndef RTI_CERT
    if (DDS_PropertyQosPolicy_finalize(&self->property) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }
#endif

    return DDS_RETCODE_OK;
}

DDS_Boolean
DDS_DataWriter_serialize(const DDS_DataWriter *self,struct CDR_Stream_t *stream)
{
    const struct DDS_LatencyBudgetQosPolicy default_latency_budget =
        DDS_LATENCY_BUDGET_QOS_POLICY_DEFAULT;
    const struct DDS_DeadlineQosPolicy default_deadline =
        DDS_DEADLINE_QOS_POLICY_DEFAULT;
    const struct DDS_OwnershipQosPolicy default_ownership =
        DDS_OWNERSHIP_QOS_POLICY_DEFAULT;
    const struct DDS_OwnershipStrengthQosPolicy default_ownership_strength =
        DDS_OWNERSHIP_STRENGTH_QOS_POLICY_DEFAULT;
    const struct DDS_ReliabilityQosPolicy default_reliability =
        DDS_DATAWRITER_RELIABILITY_QOS_POLICY_DEFAULT;
    const struct DDS_LivelinessQosPolicy default_liveliness =
        DDS_LIVELINESS_QOS_POLICY_DEFAULT;
    const struct DDS_DurabilityQosPolicy default_durability =
        DDS_DURABILITY_QOS_POLICY_DEFAULT;
    const struct DDS_DestinationOrderQosPolicy default_destination_order =
        DDS_DESTINATION_ORDER_QOS_POLICY_DEFAULT;

    DDS_Boolean has_non_default_latency_budget =
        !DDS_Duration_equal(&self->latency_budget.duration,
                            &default_latency_budget.duration);
    DDS_Boolean has_non_default_deadline =
        !DDS_Duration_equal(&self->deadline.period, &default_deadline.period);
    DDS_Boolean has_non_default_ownership =
        (self->ownership.kind != default_ownership.kind);
    DDS_Boolean has_non_default_ownership_strength =
        (self->ownership_strength.value != default_ownership_strength.value);
    DDS_Boolean has_non_default_reliability =
        (self->reliability.kind != default_reliability.kind) ||
        !DDS_Duration_equal(&self->reliability.max_blocking_time,
                            &default_reliability.max_blocking_time);
    DDS_Boolean has_non_default_liveliness =
        !DDS_LivelinessQosPolicy_is_equal(&self->liveliness, &default_liveliness);
    DDS_Boolean has_non_default_durability =
        !DDS_DurabilityQosPolicy_is_equal(&self->durability, &default_durability);
    DDS_Boolean has_non_default_destination_order =
        (self->destination_order.kind != default_destination_order.kind);

    if (has_non_default_latency_budget &&
        !DDS_CdrQosPolicy_serialize_latency_budget(stream,
                                                   &self->latency_budget, NULL))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (has_non_default_deadline &&
        !DDS_CdrQosPolicy_serialize_deadline(stream,
                                             &self->deadline, NULL))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (has_non_default_ownership &&
        !DDS_CdrQosPolicy_serialize_ownership(stream,&self->ownership, NULL))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (has_non_default_ownership_strength &&
        !DDS_CdrQosPolicy_serialize_ownership_strength(stream,
                                            &self->ownership_strength,NULL))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (has_non_default_reliability &&
        !DDS_CdrQosPolicy_serialize_reliability(stream,&self->reliability,NULL))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (has_non_default_liveliness &&
        !DDS_CdrQosPolicy_serialize_liveliness(stream,
                                               &self->liveliness, NULL))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (has_non_default_durability &&
        !DDS_CdrQosPolicy_serialize_durability(stream,
                                                &self->durability, NULL))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (has_non_default_destination_order &&
        !DDS_CdrQosPolicy_serialize_destination_order(stream,
                                                &self->destination_order, NULL))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (self->writer_data.unicast_locator != NULL)
    {
        if (!DDS_CdrQosPolicy_serialize_locator_sequence(
                stream,
                self->writer_data.unicast_locator,
                RTPS_PID_UNICAST_LOCATOR6,
                RTPS_PID_UNICAST_LOCATOR6_EX,
                NULL,
                NULL))
        {
            return DDS_BOOLEAN_FALSE;
        }
    }

    if (!DDS_CdrQosPolicy_serialize_user_data(stream,self->user_data))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

DDS_Boolean
DDS_DataWriter_is_immutable_qos_equal(const DDS_DataWriter *left,
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
        !DDS_TransportEncapsulationQosPolicy_is_equal(left->encapsulation,
                                                      &right->encapsulation) ||
        !DDS_DataRepresentationQosPolicy_is_equal(left->representation,
                                                  &right->representation) ||
        !RTI_ManagementQosPolicy_is_equal(&left->management,&right->management) ||
        !DDS_DataWriterResourceLimitsQosPolicy_is_equal(&left->writer_resource_limits,
                                                        &right->writer_resource_limits) ||
        !DDS_UserDataQosPolicy_is_equal(left->user_data,&right->user_data) ||
        !DDS_PublishModeQosPolicy_is_equal(left->publish_mode,&right->publish_mode) ||
        !DDS_PropertyQosPolicy_is_equal(&left->property,&right->property) ||
        REDA_String_compare(left->publication_name,
                            right->publication_name.name))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Compare entries in the table of datawriter entries. The function is
 *        compatible with \ref DB_IndexCompare_T
 *
 * \param[in] flags Used to distinguish between op2 being a key or record
 * \param[in] op1   A DDS_DataWriterImpl already in the database
 * \param[in] op2   Either a DDS_DataWriterImpl being added or a key
 *                  being searched for
 *
 * \return positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2,
 *         zero if op1 is equal to op2
 */
RTI_INT32
DDS_DataWriterImpl_compare(RTI_INT32 flags,const DB_Record_T op1, void *op2)
{
    struct DDS_DataWriterImpl *record_left = (struct DDS_DataWriterImpl*)op1;
    const DDS_UnsignedLong *id_right;

    if (DB_SELECT_OP2_IS_KEY(flags))
    {
        id_right = (const DDS_UnsignedLong*)op2;
    }
    else
    {
        id_right = &((struct DDS_DataWriterImpl*)op2)->as_entity.entity_id;
    }

    return ((record_left->as_entity.entity_id == *id_right) ? 0 :
                 (record_left->as_entity.entity_id > *id_right ? 1 : -1));
}

#if DDS_LIVELINESS_CHANNEL_ENABLED

RTI_PRIVATE RTI_INT32
DDS_DataWriterImpl_compare_automatic_liveliness_kind_record(
        struct DDS_DataWriterImpl *record_left,
        struct DDS_DataWriterImpl *record_right)
{
    RTI_INT32 retval = 0;

    /* we are not interested in manual_by_topic writers or
     * infinite lease duration
     */
    if ((record_right->liveliness.kind != DDS_AUTOMATIC_LIVELINESS_QOS) ||
        (DDS_Duration_is_infinite(&record_right->liveliness.lease_duration)))
    {
        goto done;
    }

    retval = DDS_Duration_compare(&record_left->liveliness.lease_duration,
                                  &record_right->liveliness.lease_duration);

    if (retval != 0)
    {
        goto done;
    }

    /* keep the new record in the index even if the lease duration is the same.
     * in case the already existing record is deleted we will need to new one
     */
    retval = ((record_left->as_entity.entity_id ==
               record_right->as_entity.entity_id) ? 0 :
                   (record_left->as_entity.entity_id >
                       record_right->as_entity.entity_id ? 1 : -1));

done:
    return retval;
}

RTI_PRIVATE RTI_INT32
DDS_DataWriterImpl_compare_lease_duration_record(
        struct DDS_DataWriterImpl *record_left,
        struct DDS_DataWriterImpl *record_right)
{
    RTI_INT32 retval = 0;

    /* we are not interested in manual_by_topic writers or
     * infinite lease duration
     */
    if ((record_right->liveliness.kind ==DDS_MANUAL_BY_TOPIC_LIVELINESS_QOS) ||
        (DDS_Duration_is_infinite(&record_right->liveliness.lease_duration)))
    {
        goto done;
    }

    retval = DDS_Duration_compare(&record_left->liveliness.lease_duration,
                                  &record_right->liveliness.lease_duration);

    if (retval != 0)
    {
        goto done;
    }

    /* keep the new record in the index even if the lease duration is the same.
     * in case the already existing record is deleted we will need to new one
     */
    retval = ((record_left->as_entity.entity_id ==
               record_right->as_entity.entity_id) ? 0 :
                   (record_left->as_entity.entity_id >
                       record_right->as_entity.entity_id ? 1 : -1));

done:
    return retval;
}

/*ci
 * \brief Compare entries in the table of datawriter entries using the DW
 * lease duration. If the op2 record has infinite lease duration or liveliness
 * kind is MANUAL_BY_TOPIC 0 is returned as if record already exits.
 * The function is compatible with \ref DB_IndexCompare_T
 *
 * \param[in] flags Used to distinguish between op2 being a key or record
 * \param[in] op1   A DDS_DataWriterImpl already in the database
 * \param[in] op2   A DDS_DataWriterImpl being added
 *
 * \return positive integer if op1 lease duration is greater than op2 lease
 *         duration,
 *         negative integer if op1 lease duration is less than op2 lease
 *         duration,
 *         zero if op1 lease duration is equal to op2 lease duration or if
 *         op2 lease duration is infinite or liveliness kind is manual_by_topic
 */
RTI_INT32
DDS_DataWriterImpl_compare_lease_duration(RTI_INT32 flags,
                                          const DB_Record_T op1,
                                          void *op2)
{
    struct DDS_DataWriterImpl *record_left = (struct DDS_DataWriterImpl*)op1;
    struct DDS_DataWriterImpl *record_right = NULL;
    UNUSED_ARG(flags);

    record_right = (struct DDS_DataWriterImpl*)op2;

    return DDS_DataWriterImpl_compare_lease_duration_record(
                        record_left, record_right);
}

/*ci
 * \brief Compare entries in the table of datawriter entries using the DW
 * liveliness kind. If the op2 record has NOT automatic liveliness or its
 * lease duration is infinite 0 is returned as if record already exits.
 * The function is compatible with \ref DB_IndexCompare_T
 *
 * If trust extensions are included, then datawriters with protected
 * liveliness are excluded from the index.
 *
 * \param[in] flags Used to distinguish between op2 being a key or record
 * \param[in] op1   A DDS_DataWriterImpl already in the database
 * \param[in] op2   A DDS_DataWriterImpl being added
 *
 * \return positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2,
 *         zero if op1 is equal to op2
 */
RTI_INT32
DDS_DataWriterImpl_compare_automatic_liveliness_kind(RTI_INT32 flags,
                                                     const DB_Record_T op1,
                                                     void *op2)
{
    struct DDS_DataWriterImpl *record_left = (struct DDS_DataWriterImpl*)op1;
    struct DDS_DataWriterImpl *record_right = NULL;
    UNUSED_ARG(flags);

    record_right = (struct DDS_DataWriterImpl*)op2;

    return DDS_DataWriterImpl_compare_automatic_liveliness_kind_record(
                record_left,record_right);
}
#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */

/*ci
 * \brief Return the object-id for the datawriter
 *
 * \param[in] self Datawriter to return object-id for
 *
 * \return The datawriter object-id
 */
DDS_UnsignedLong
DDS_DataWriter_get_objectid(DDS_DataWriter *self)
{
    return self->as_entity.entity_id;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize a datawriter
 *
 * \details
 * Free up all resources used by a datawriter. Note that this function does
 * not free the memory used to hold the datawriter itself, this memory is
 * freed by the publisher because the publisher is the factory.
 *
 * \param[in] self Datawriter to finalize
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DDS_DataWriterImpl_finalize(DDS_DataWriter *self)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    struct RT_ComponentFactory *factory;
    struct DDS_DataWriterImpl *datawriter = (struct DDS_DataWriterImpl *)self;
    RTI_BOOL netiorc;

    if (DDS_Entity_is_enabled(DDS_DataWriter_as_entity(self)) &&
        !DDS_Duration_is_infinite(&datawriter->deadline.period))
    {
        if (!OSAPI_Timer_delete_timeout(datawriter->config->timer,
                                        &datawriter->deadline_event))
        {
            DDSC_LOG_OBJECT_DELETE(OSAPI_LOGKIND_ERROR,DDSC_LOG_TIMEROUT_OBJECT)
            goto done;
        }
    }

    if (DDS_Entity_is_enabled(DDS_DataWriter_as_entity(self)) &&
        (datawriter->liveliness.kind != DDS_AUTOMATIC_LIVELINESS_QOS) &&
        !DDS_Duration_is_infinite(&datawriter->liveliness.lease_duration) &&
        (datawriter->writer_state == WRITERSTATE_ALIVE))
    {
        if (!OSAPI_Timer_delete_timeout(datawriter->config->timer,
                                        &datawriter->liveliness_event))
        {
            DDSC_LOG_OBJECT_DELETE(OSAPI_LOGKIND_ERROR,DDSC_LOG_TIMEROUT_OBJECT)
            goto done;
        }
    }

    if (DDS_LocatorSeq_get_length(&datawriter->uc_locator_seq) > 0)
    {
        if (DB_Database_unlock(datawriter->config->db) != DB_RETCODE_OK)
        {
            return DDS_BOOLEAN_FALSE;
        }

        netiorc = NETIO_BindResolver_release_addresses(
                        datawriter->config->bind_resolver,
                        datawriter->config->enabled_transports,
                        NETIO_ROUTEKIND_USER,
                        (struct NETIO_AddressSeq*)&datawriter->uc_locator_seq);

        if (DB_Database_lock(datawriter->config->db) != DB_RETCODE_OK)
        {
            return DDS_BOOLEAN_FALSE;
        }

        if (!netiorc)
        {
            goto done;
        }

        if (!DDS_LocatorSeq_finalize(&datawriter->uc_locator_seq))
        {
            goto done;
        }
    }

    if ((datawriter->flow_controller != NULL) && (datawriter->config->release_flowcontroller != NULL))
    {
        if (!datawriter->config->release_flowcontroller(
                    DDS_Publisher_get_participant(self->publisher),
                    datawriter->flow_controller))
        {
            goto done;
        }
        datawriter->flow_controller = NULL;
    }

    if (datawriter->rtps_intf != NULL)
    {
        factory = RT_Registry_lookup(datawriter->config->registry,
                                     NETIO_DEFAULT_RTPS_NAME);
        if (factory == NULL)
        {
            DDSC_LOG_COMPONENT_LOOKUP(OSAPI_LOGKIND_ERROR,
                                     NETIO_DEFAULT_RTPS_NAME)
            goto done;
        }

        NETIO_InterfaceFactory_delete_component(factory,datawriter->rtps_intf);
        datawriter->rtps_intf = NULL;
    }

#if DDS_FILTERING_ENABLED
    if (datawriter->writer_filter != NULL)
    {
        DDS_DataWriter_delete_filter(datawriter, datawriter->writer_filter);
        datawriter->writer_filter = NULL;
    }
#endif

    if (datawriter->wh != NULL)
    {
        factory = RT_Registry_lookup(datawriter->config->registry,
                                     DDSHST_WRITER_DEFAULT_HISTORY_NAME);
        if (factory == NULL)
        {
            DDSC_LOG_COMPONENT_LOOKUP(OSAPI_LOGKIND_ERROR,
                                      DDSHST_WRITER_DEFAULT_HISTORY_NAME)
            goto done;
        }
        DDSHST_WriterFactory_delete_component(factory, datawriter->wh);
        datawriter->wh = NULL;
    }

    if (datawriter->dw_intf != NULL)
    {
        factory = RT_Registry_lookup(datawriter->config->registry,
                                     DDS_DEFAULT_DATAWRITER_NETIO_NAME);
        if (factory == NULL)
        {
            DDSC_LOG_COMPONENT_LOOKUP(OSAPI_LOGKIND_ERROR,
                                      DDS_DEFAULT_DATAWRITER_NETIO_NAME)
            goto done;
        }
        NETIO_InterfaceFactory_delete_component(factory,datawriter->dw_intf);
        datawriter->dw_intf = NULL;
    }

    if ((datawriter->sample_pool != NULL) &&
        !REDA_BufferPool_delete(datawriter->sample_pool))
    {
        DDSC_LOG_CDR_POOL_DELETE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_CDR)
        goto done;
    }

    if (self->md5_stream != NULL)
    {
        CDR_Stream_free(self->md5_stream);
    }

    if (datawriter->type_plugin != NULL)
    {
        DDS_TypePlugin_delete(datawriter->type_plugin);
    }

    if (DDS_DataWriter_finalize_managed(self,
            DDS_Publisher_get_participant(self->publisher)) != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_QOS)
        goto done;
    }

    if (!DDS_EntityImpl_finalize(&self->as_entity))
    {
        DDSC_LOG_ENTITY_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_ENTITY)
        goto done;
    }

    if (datawriter->write_lock != NULL)
    {
        OSAPI_Mutex_delete(datawriter->write_lock);
    }

    retval = DDS_BOOLEAN_TRUE;

done:

    return retval;
}
#endif /* !RTI_CERT */

/*ci
 * \brief Return the instance handle of a datawriter
 *
 * \details
 * This function overrides the DDS entity get_instance_handle function and
 * is called via DDS_Entity_get_instance_handle.
 *
 * \param[in] entity The base-class for the datawriter
 *
 * \return The instance handle
 */
MUST_CHECK_RETURN RTI_PRIVATE DDS_InstanceHandle_t
DDS_DataWriterImpl_get_instance_handle(DDS_Entity *entity)
{
    struct DDS_DataWriterImpl *dw = (struct DDS_DataWriterImpl*)entity;
    DDS_InstanceHandle_t retval;

    retval = dw->config->get_parent_handle((DDS_Entity*)dw->publisher);

    DDS_InstanceHandle_set_suffix(&retval,dw->as_entity.entity_id);

    return retval;
}

RTI_PRIVATE RTI_BOOL
DDS_DataWriterImpl_initialize_sample_buffer(void *param, void *buffer)
{
    struct DDS_DataWriterSample *buf = (struct DDS_DataWriterSample*)buffer;
    char *inline_qos = NULL;
    RTI_SIZE_T *inline_qos_max_size = (RTI_SIZE_T*)param;

    if (*inline_qos_max_size > 0)
    {
        /* Only allocate if needed */
        OSAPI_Heap_allocate_buffer(&inline_qos,
                                   *inline_qos_max_size,
                                   OSAPI_ALIGNMENT_DEFAULT);

        if (inline_qos == NULL)
        {
            return RTI_FALSE;
        }

        if (!NETIO_PacketBuffer_set(&buf->inline_pbuf,inline_qos,
                                    *inline_qos_max_size,0,0))
        {
            return RTI_FALSE;
        }
    }
    else
    {
        if (!NETIO_PacketBuffer_set(&buf->inline_pbuf,NULL,0,0,0))
        {
            return RTI_FALSE;
        }
    }

    return RTI_TRUE;
}

RTI_PRIVATE RTI_BOOL
DDS_DataWriterImpl_finalize_sample_buffer(void *param, void *buffer)
{
    struct DDS_DataWriterSample *buf = (struct DDS_DataWriterSample*)buffer;
    UNUSED_ARG(param);

    if (buf->inline_pbuf.buffer != NULL)
    {
#ifndef RTI_CERT
        OSAPI_Heap_free_buffer(buf->inline_pbuf.buffer);
#endif
        buf->inline_pbuf.buffer = NULL;
    }

    return RTI_TRUE;
}


/*ci
 * \brief Initialize a datawriter
 *
 * \details
 *
 * The publisher allocates memory to store the datawriter data and passes
 * it to the datawriter for initialization. The datawriter allocates all its
 * internal resources. The datawriter is passed shared resources in the
 * config structure, such as database, timers resolvers etc. These resources
 * are typically managed by the domain participant.
 *
 * \param[in] datawriter A datawriter structure to initialize
 * \param[in] publisher  The publisher creating the datawriter
 * \param[in] topic      The topic the datawriter is publishing
 * \param[in] qos        The datawriter qos policy
 * \param[in] listener   The datawriter listener
 * \param[in] mask       Mask with enabled statuses on the datawriter
 * \param[in] object_id  The datawriter object id generated by the factory
 * \param[in] config     General datawriter configuration
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DDS_DataWriterImpl_initialize(
        struct DDS_DataWriterImpl *datawriter,
        DDS_Publisher *publisher,
        DDS_Topic *topic,
        const struct DDS_DataWriterQos *qos,
        const struct DDS_DataWriterListener *listener,
        DDS_StatusMask mask,
        DDS_UnsignedLong object_id,
        struct NDDS_DataWriterConfig *config)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    struct REDA_BufferPoolProperty cdr_property =
                                            REDA_BufferPoolProperty_INITIALIZER;
    struct DDSHST_WriterProperty wh_property =
                                DDSHST_WriterProperty_INITIALIZER;
    struct DDSHST_WriterListener wh_listener =
                                DDSHST_WriterListener_INITIALIZE;
    struct RT_ComponentFactory *factory;
    struct DDS_DataWriterInterfaceProperty dwintf_property =
                                DDS_DataWriterInterfaceProperty_INITIALIZER;
    DDS_InstanceHandle_t instance_handle;
    struct RTPS_InterfaceProperty rtps_property =
                                            RTPS_InterfaceProperty_INITIALIZER;
    DDS_UnsignedLong serialized_key_size;
    const struct DDS_DataWriterListener nil_listener =
                                            DDS_DataWriterListener_INITIALIZER;
    NDDS_TypePluginKeyKind key_kind;
    struct DDS_TypePluginProperty type_property = DDS_TypePluginProperty_INITIALIZER;
    RTI_INT32 min_mtu;
    struct DDS_DomainParticipantQos *dp_qos = NULL;
#if INCLUDE_API_QOS
    DDS_ReturnCode_t ddsrc;
#endif

    OSAPI_Memory_zero(datawriter,sizeof(struct DDS_DataWriterImpl));

    datawriter->topic = topic;
    datawriter->config = config;
    datawriter->publisher = publisher;

    /* Initialize optional members, all others are
     * set in DDS_DataWriter_set_qos_from.
     * the cast is ok, we never write to these variables unless allocated
     */
    datawriter->user_data = (struct DDS_UserDataQosPolicy*)&DDS_USER_DATA_DEFAULT;
    datawriter->encapsulation = (struct DDS_TransportEncapsulationQosPolicy*)&DDS_TRANSPORT_ENCAPSULATION_DEFAULT;
    datawriter->representation = (struct DDS_DataRepresentationQosPolicy*)&DDS_DATAREPRESENTATION_DEFAULT;
    datawriter->publish_mode = (struct DDS_PublishModeQosPolicy*)&DDS_PUBLISHMODE_DEFAULT;
    datawriter->publication_name = (char*)DDS_ENTITY_NAME_DEFAULT;

    datawriter->write_lock = OSAPI_Mutex_new();

    if (datawriter->write_lock == NULL)
    {
        goto done;
    }

    if ((listener != NULL) &&
        !DDS_DataWriterListener_is_consistent(listener, mask))
    {
        DDSC_LOG_LISTENER_INCONSISTENT(OSAPI_LOGKIND_ERROR,
                                       DDSC_LOG_DATAWRITER_LISTENER,mask)
        goto done;
    }

    if (!DDS_PropertyQosPolicy_is_valid(&qos->property,
                                        DDS_DATAWRITER_ENTITY_KIND))
    {
        DDSC_LOG_ENTITY_INVALID_PROPERTY(OSAPI_LOGKIND_ERROR,
                                          DDSC_LOG_DATAWRITER_ENTITY)
        goto done;
    }

    dp_qos = DDS_DomainParticipant_get_qos_ref(publisher->participant);
    if (!DDS_UserDataQosPolicy_is_consistent(&qos->user_data,
                        dp_qos->resource_limits.writer_user_data_max_length))
    {
        DDSC_LOG_QOS_INCONSISTENT(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_QOS)
        goto done;
    }

    if (!DDS_DataWriterQos_is_consistent(qos))
    {
        DDSC_LOG_QOS_INCONSISTENT(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_QOS)
        goto done;
    }

#if INCLUDE_API_QOS
    if (qos == &DDS_DATAWRITER_QOS_DEFAULT)
    {
            ddsrc = DDS_DataWriter_set_qos_from(datawriter,
                                                    publisher->default_qos,
                                                    publisher->participant);
        if (ddsrc != DDS_RETCODE_OK)
        {
            DDSC_LOG_QOS_GET(OSAPI_LOGKIND_ERROR,DDSC_LOG_DEFAULTDATAWRITER_QOS)
            goto done;
        }
    }
    else
#endif
    {
        if (DDS_DataWriter_set_qos_from(datawriter, qos,
                                     publisher->participant) != DDS_RETCODE_OK)
        {
            DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_QOS)
            goto done;
        }
    }

#if DDS_XTYPES_IS_ENABLED
    datawriter->xtypes_compliance_mask =
                    NDDS_CONFIG_XTYPES_ENCAPSULATION_OPTIONS_WITH_PADDING_BIT;
#endif
    datawriter->zcv2_protocol_version = DDS_DATAWRITER_PROTOCOL_ZCV2_VERSION_DEFAULT;

    /* Set from participant first, then override */
    if (!DDS_DataWriter_set_from_property(
                    datawriter,
                    config->participant_property_qos_policy))
    {
        DDSC_LOG_ENTITY_INVALID_PROPERTY(OSAPI_LOGKIND_ERROR,
                                          DDSC_LOG_DATAWRITER_ENTITY)
        goto done;
    }

    if (!DDS_DataWriter_set_from_property(datawriter,&datawriter->property))
    {
        DDSC_LOG_ENTITY_INVALID_PROPERTY(OSAPI_LOGKIND_ERROR,
                                          DDSC_LOG_DATAWRITER_ENTITY)
        goto done;
    }

#if 0
    /* This has been disabled to support misconfiguration by the application.
     */
    if ((DDS_DataRepresentationIdSeq_get_length(
                            &datawriter->qos.representation.value) > 0) &&
        !DDS_Type_valid_representations(
                DDS_Topic_get_type(datawriter->topic)->plugin,
                &datawriter->qos.representation.value))
    {
        goto done;
    }
#endif

    /* Add space for the encapsulation header which is not part of calculation
     * of the maximum serialized data.
     */
    type_property.plugin_param = datawriter->type_support.plugin_data;

    /* This may duplicate space for the encapsulation added by type-plugin.
     * However, it guarantees space for the encapsulation header.
     */
    type_property.head_padding = 0;
    type_property.tail_padding = 0;
    type_property.max_buffers = datawriter->resource_limits.max_samples;
    type_property.plugin_param = datawriter->type_support.plugin_data;

    if (!DDS_EntityImpl_initialize(&datawriter->as_entity,
                                       DDS_DATAWRITER_ENTITY_KIND,
                                       object_id,DDS_DataWriter_enable,
                                       DDS_DataWriterImpl_get_instance_handle))
    {
        DDSC_LOG_ENTITY_INITIALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_ENTITY)
        goto done;
    }

    datawriter->writer_data.unicast_locator = &datawriter->uc_locator_seq;
    datawriter->writer_data.resolved_participant_locators = config->default_unicast;
    if (DDS_StringSeq_get_length(&datawriter->transport.enabled_transports) > 0)
    {
        /* NOTE: No support for multicast writer locators */
        if (!DDS_LocatorSeq_set_maximum(&datawriter->uc_locator_seq,
                                        RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX))
        {
            goto done;
        }

        if (!DDS_LocatorSeq_set_length(&datawriter->uc_locator_seq,0))
        {
            goto done;
        }

        if (!NETIO_BindResolver_reserve_addresses(
             config->bind_resolver,*config->participant_id,
             config->enabled_transports,
             (struct REDA_StringSeq*)&datawriter->transport.enabled_transports,
             NETIO_ROUTEKIND_USER,
             (struct NETIO_AddressSeq*)&datawriter->mc_locator_seq,
             (struct NETIO_AddressSeq*)&datawriter->uc_locator_seq))
        {
            DDSC_LOG_RESERVE_LOCATORS(OSAPI_LOGKIND_ERROR)
            goto done;
        }
    }

    datawriter->type_plugin = DDS_TypeImpl_create_plugin(
                                    DDS_Topic_get_type(datawriter->topic),
                                    DDS_Publisher_get_participant(publisher),
                                    DDS_DomainParticipant_get_qos_ref(DDS_Publisher_get_participant(publisher)),
                                    DDS_TYPEPLUGIN_MODE_WRITER,
                                    datawriter,NULL,
                                    &type_property);

    if (datawriter->type_plugin == NULL)
    {
        DDSC_LOG_LOOKUP_TYPE_PLUGIN(OSAPI_LOGKIND_ERROR,
                                    DDS_Topic_get_type(datawriter->topic))
        goto done;
    }

    /* initialized to 0 by default, but set it explicitly to make it
     * clear.
     */
    datawriter->set_cdr_options_padding = RTI_FALSE;

    datawriter->cdr_id = DDS_TypePlugin_get_cdr_encapsulation(datawriter->type_plugin);

    /* Built-in entities are always compatible.
     *
     * These entities include the built-in endpoints used for discovery,
     * the inter-participant endpoints, and the security counterpart
     * endpoints.
     *
     * If Micro is ever extended with additional built-in entities, they
     * must be able to parse the padding bits.
     */
#if DDS_XTYPES_IS_ENABLED
     if (!DDS_ObjectId_is_builtin(datawriter->as_entity.entity_id))
    {
        struct DDS_TypeCode *type_code = NULL;

        /* Only if the type-code is available and the padding option bit is set
         * may padding be sent. When code is generated with -interpreted 0
         * the type-code is not available and padding is never sent.
         */
        type_code = DDS_TypeImpl_get_typecode(DDS_Topic_get_type(datawriter->topic));
        if ((type_code != NULL)
            && (datawriter->xtypes_compliance_mask
                & NDDS_CONFIG_XTYPES_ENCAPSULATION_OPTIONS_WITH_PADDING_BIT)
            && (datawriter->type_plugin->_intf->type_factory != NULL))
        {
            datawriter->set_cdr_options_padding = DDS_TypeInterfaceI_set_padding_options(
                        datawriter->type_plugin->_intf->type_factory,
                        type_code,
                        datawriter->cdr_id);
        }
    }
#endif
    /* Check the TransportEncapsulationQos policy for consistency or set the
     * defaults. If the policy is empty all supported encapsulations are
     * created, otherwise only those listed in the Qos.
     */
    if ((DDS_TransportEncapsulationSettingsSeq_get_length(
                                &datawriter->encapsulation->value) != 0)
        && !DDS_Transport_is_encapsulation_policy_valid(
                                datawriter->encapsulation,
                                datawriter->type_plugin,
                                datawriter->config->addr_resolver,
                                datawriter->config->route_resolver))
    {
        goto done;
    }

    datawriter->active_encapsulation =
                DDS_TypePlugin_get_encapsulation(datawriter->type_plugin);



    key_kind = DDS_TypePlugin_get_key_kind(datawriter->type_plugin);

    datawriter->md5_stream = NULL;
    if (key_kind != NDDS_TYPEPLUGIN_NO_KEY)
    {
        datawriter->send_key_hash = DDS_BOOLEAN_TRUE;
        serialized_key_size =
                DDS_TypePlugin_get_serialized_key_size(
                                                  datawriter->type_plugin,0);

        datawriter->md5_stream = CDR_Stream_alloc(serialized_key_size);
        if (datawriter->md5_stream == NULL)
        {
            DDSC_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_MD5STREAM_OBJECT)
            goto done;
        }
#ifdef RTI_ENDIAN_LITTLE
        CDR_Stream_byteswap_set(datawriter->md5_stream, RTI_TRUE);
#else
        CDR_Stream_byteswap_set(datawriter->md5_stream, RTI_FALSE);
#endif
    }
    else
    {
        datawriter->send_key_hash = DDS_BOOLEAN_FALSE;
    }

    if (listener == NULL)
    {
        datawriter->listener = nil_listener;
    }
    else
    {
        datawriter->listener = *listener;
    }
    datawriter->mask = mask;

    /* REDA and DDS sequence numbers are identical in layout. Use copy
     * to avoid aliasing warning.
     */
    OSAPI_Memory_copy((void*)&datawriter->last_sn,
                      (void*)&datawriter->
                      protocol.rtps_reliable_writer.first_write_sequence_number,
                      sizeof(struct REDA_SequenceNumber));

    REDA_SequenceNumber_minusminus(&datawriter->last_sn);


    /* The data-path in Micro 2.4 and under is hard-coded to either:
     *
     * DDS<->RTPS<->UDP (UDP transport)
     * DDS<->DDS (intra transport)
     *
     * The intra transport does not require any additional header/trailer
     * in the packet since the packet is not serialized/deserialized into
     * the packet payload.
     *
     * For the UDP transport the maximum overhead on a serialized sample is
     * calculated as the space needed to send a serialized sample encapsulated
     * in RTPS and then encapsulated in UDP. The constants are defined in the
     * netio_common.h header-file until this information can be retrieved by
     * calling an interface method on each interface.
     */
    cdr_property.buffer_size = (RTI_SIZE_T)sizeof(struct DDS_DataWriterSample);
    cdr_property.max_buffers = (RTI_SIZE_T)datawriter->resource_limits.max_samples;
    cdr_property.flags = 0;

    {
        RTI_SIZE_T inline_qos_max_size = 0;

        if (datawriter->send_key_hash)
        {
            /* enough for KEYHASH (20) + STATUS (8) */
            inline_qos_max_size += 28;
        }

        if (datawriter->transfer_mode.shmem_ref_settings.enable_data_consistency_check)
        {
            /* enough for EPOCH (12) */
            inline_qos_max_size += 12;
        }

        if (inline_qos_max_size > 0)
        {
            /* SENTINEL */
            inline_qos_max_size += RTPS_SUBMESSAGE_HEADER_LENGTH;
        }

        datawriter->sample_pool = REDA_BufferPool_new("sample_pool",
                                              &cdr_property,
                                              DDS_DataWriterImpl_initialize_sample_buffer,
                                              &inline_qos_max_size,
                                              DDS_DataWriterImpl_finalize_sample_buffer,
                                              datawriter);

        if (datawriter->sample_pool == NULL)
        {
            DDSC_LOG_CDR_POOL_ALLOC(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_CDR,
                                cdr_property.buffer_size,cdr_property.max_buffers)
            goto done;
        }
    }

    wh_listener.on_sample_removed = DDS_DataWriterEvent_on_sample_removed;
#ifndef RTI_CERT
    wh_listener.on_key_removed = DDS_DataWriterEvent_on_key_removed;
#endif
    wh_listener.on_deadline_missed = DDS_DataWriterEvent_on_deadline_missed;
    wh_listener.listener_data = (void *)datawriter;

    factory = RT_Registry_lookup(datawriter->config->registry,
                                 DDSHST_WRITER_DEFAULT_HISTORY_NAME);
    if (factory == NULL)
    {
        DDSC_LOG_COMPONENT_LOOKUP(OSAPI_LOGKIND_ERROR,
                                  DDSHST_WRITER_DEFAULT_HISTORY_NAME)
        goto done;
    }

    wh_property._parent.db = datawriter->config->db;
    wh_property.datawriter = datawriter;
    wh_property.type_plugin = datawriter->type_plugin;
    wh_property.write_lock = datawriter->write_lock;
    wh_property.deadline = datawriter->deadline;
    wh_property.destination_order = datawriter->destination_order;
    wh_property.history = datawriter->history;
    wh_property.durability = datawriter->durability;
    wh_property.reliability = datawriter->reliability;
    wh_property.resource_limits = datawriter->resource_limits;

    datawriter->wh = DDSHST_WriterFactory_create_component(factory,
                                                       &wh_property._parent,
                                                       &wh_listener._parent);
    if (datawriter->wh == NULL)
    {
        DDSC_LOG_COMPONENT_CREATE(OSAPI_LOGKIND_ERROR,
                                  DDSC_LOG_WRITERHISTORY_COMPONENT,
                                  DDSHST_WRITER_DEFAULT_HISTORY_NAME)
        goto done;
    }

    /* DataWriter Interface */
    factory = RT_Registry_lookup(datawriter->config->registry,
                                 DDS_DEFAULT_DATAWRITER_NETIO_NAME);
    if (factory == NULL)
    {
        DDSC_LOG_COMPONENT_LOOKUP(OSAPI_LOGKIND_ERROR,
                                  DDS_DEFAULT_DATAWRITER_NETIO_NAME)
        goto done;
    }

    dwintf_property._parent._parent.db = datawriter->config->db;

    dwintf_property._parent.max_routes =
                    (RTI_SIZE_T)(DDS_DATAWRITER_MAX_DESTINATION_PER_READER *
                    datawriter->writer_resource_limits.max_remote_readers);

    dwintf_property.datawriter = datawriter;
    dwintf_property._parent.max_binds =
                    (RTI_SIZE_T)datawriter->writer_resource_limits.max_remote_readers;
    dwintf_property._parent.packet_pool = datawriter->config->packet_pool;
    dwintf_property.type_plugin = datawriter->type_plugin;

    instance_handle = DDS_DataWriterImpl_get_instance_handle(
                                                    (DDS_Entity*)datawriter);

    NETIO_Address_set_guid(&dwintf_property.intf_address,
                          0,(struct NETIO_Guid*)instance_handle.octet);

    datawriter->dw_intf = NETIO_InterfaceFactory_create_component(factory,
                            &dwintf_property._parent._parent,NULL);
    if (datawriter->dw_intf == NULL)
    {
        DDSC_LOG_COMPONENT_CREATE(OSAPI_LOGKIND_ERROR,
                                  DDSC_LOG_DATAWRITERIO_COMPONENT,
                                  DDS_DEFAULT_DATAWRITER_NETIO_NAME)
        goto done;
    }

    factory = RT_Registry_lookup(datawriter->config->registry,
                                 NETIO_DEFAULT_RTPS_NAME);
    if (factory == NULL)
    {
        DDSC_LOG_COMPONENT_LOOKUP(OSAPI_LOGKIND_ERROR,
                                  NETIO_DEFAULT_RTPS_NAME)
        goto done;
    }

    /* Domain ID must be positive */
    NETIO_Address_set_guid(&rtps_property.intf_address,
                           (RTI_UINT32)*datawriter->config->domain_id,
                           (struct NETIO_Guid*)instance_handle.octet);
    rtps_property._parent._parent.db = datawriter->config->db;
    rtps_property._parent.packet_pool = datawriter->config->packet_pool;

    rtps_property.anonymous = datawriter->management.is_anonymous;

    rtps_property._parent.max_routes =
            (RTI_SIZE_T)(datawriter->writer_resource_limits.max_routes_per_reader *
                         datawriter->writer_resource_limits.max_remote_readers);

    rtps_property.mode = RTPS_INTERFACEMODE_WRITER;
    rtps_property.reliable =
        (datawriter->reliability.kind == DDS_RELIABLE_RELIABILITY_QOS);
    rtps_property.ext_rtps_intf = datawriter->config->ext_rtps_intf;

    /* Default window size */
    if (DDS_RTPSRELIABLEWRITER_DEFAULT_SEND_WINDOW ==
        datawriter->protocol.rtps_reliable_writer.max_send_window)
    {
        if (datawriter->history.kind == DDS_KEEP_LAST_HISTORY_QOS)
        {
            rtps_property.max_window_size =
                (datawriter->history.depth *
                 datawriter->resource_limits.max_instances);
        }
        else
        {
            rtps_property.max_window_size = datawriter->resource_limits.max_samples;
        }
    }
    else
    {
        rtps_property.max_window_size =
            datawriter->protocol.rtps_reliable_writer.max_send_window;
    }

    if (rtps_property.max_window_size > RTPS_RECEIVE_WINDOW_MAX_SIZE)
    {
        rtps_property.max_window_size = RTPS_RECEIVE_WINDOW_MAX_SIZE;
    }

    rtps_property.max_hb_retries =
            datawriter->protocol.rtps_reliable_writer.max_heartbeat_retries;

    rtps_property.max_peer_count =
                    datawriter->writer_resource_limits.max_remote_readers;

    min_mtu = NETIO_RouteResolver_get_minimum_mtu(config->route_resolver);

    if (min_mtu < 0)
    {
        DDSC_LOG_GET_MTU(OSAPI_LOGKIND_ERROR)
        goto done;
    }
    if (min_mtu == 0)
    {
        DDSC_LOG_GET_MTU_NO_ROUTES(OSAPI_LOGKIND_ERROR)
        goto done;
    }
    if (min_mtu >= 65507)
    {
        rtps_property.fragment_size_bytes = 65507;
    }
    else
    {
        /* min_mtu must be less than unsigned 16bit */
        rtps_property.fragment_size_bytes = (RTI_UINT16)min_mtu;
    }

    if ((datawriter->publish_mode->kind != DDS_SYNCHRONOUS_PUBLISH_MODE_QOS) &&
        ((DDS_TypePlugin_get_serialized_sample_size_max(datawriter->type_plugin)
                > rtps_property.fragment_size_bytes) ||
        (datawriter->publish_mode->kind == DDS_ASYNCHRONOUS_PUBLISH_MODE_QOS))
       )
    {
        rtps_property.max_fragmented_samples =
                                datawriter->resource_limits.max_samples;
    }
    else
    {
        rtps_property.max_fragmented_samples = 0;
    }

    rtps_property.netio_fc = NULL;

#if DDS_FLOW_CONTROLLER_ENABLED

    if ((datawriter->publish_mode->kind == DDS_ASYNCHRONOUS_PUBLISH_MODE_QOS) ||
         ((datawriter->publish_mode->kind == DDS_AUTOMATIC_PUBLISH_MODE_QOS) &&
          rtps_property.max_fragmented_samples > 0))
    {
        DDS_FlowController* fc = NULL;

        if (config->acquire_flowcontroller != NULL)
        {
            if (datawriter->publish_mode->flow_controller_name == NULL)
            {
                fc = config->acquire_flowcontroller(
                                DDS_Publisher_get_participant(publisher),
                                DDS_DEFAULT_FLOW_CONTROLLER_NAME);
            }
            else
            {
                fc = config->acquire_flowcontroller(
                                DDS_Publisher_get_participant(publisher),
                                datawriter->publish_mode->flow_controller_name);
            }
        }

        if (fc == NULL)
        {
            DDSC_LOG_DW_UNKNOWN_FLOW_CONTROLLER(OSAPI_LOGKIND_ERROR)
            goto done;
        }

        if ((fc->property.token_bucket.bytes_per_token != DDS_LENGTH_UNLIMITED)
            && (fc->property.token_bucket.bytes_per_token < rtps_property.fragment_size_bytes))
        {
            /* Decrease fragment size such that each token allows sending
             * at least one fragment.
             */
            rtps_property.fragment_size_bytes =
                (RTI_UINT16)fc->property.token_bucket.bytes_per_token;
        }

        rtps_property.netio_fc = (NETIO_FlowController*)fc;
        datawriter->flow_controller = fc;
    }
#else
    if ((datawriter->publish_mode->kind == DDS_ASYNCHRONOUS_PUBLISH_MODE_QOS) ||
        ((datawriter->publish_mode->kind == DDS_AUTOMATIC_PUBLISH_MODE_QOS) &&
          rtps_property.max_fragmented_samples > 0))
    {
        DDSC_LOG_DW_FLOW_CONTROLLER_REQUIRED(OSAPI_LOGKIND_ERROR)
        goto done;
    }
#endif

    /* Note: for this "local" bind, max_binds could in theory just be set to the
     * max number of upstream interfaces.  However, in RTPS implementation,
     * the bind table is used to track src-peer mappings, and thus has a record
     * per src-peer pair and must be set to max peers (max_remote_readers).
     */
    rtps_property._parent.max_binds =
         (RTI_SIZE_T)datawriter->writer_resource_limits.max_remote_readers;

    rtps_property.hb_period.sec =
            datawriter->protocol.rtps_reliable_writer.heartbeat_period.sec;
    rtps_property.hb_period.nanosec =
            datawriter->protocol.rtps_reliable_writer.heartbeat_period.nanosec;

    if (datawriter->protocol.rtps_reliable_writer.heartbeats_per_max_samples == 0)
    {
        rtps_property.samples_per_hb = 0;
    }
    else
    {
        if (datawriter->resource_limits.max_samples == DDS_LENGTH_UNLIMITED)
        {
            rtps_property.samples_per_hb =
                    DDS_INFINITE_MAX_SAMPLES /
                datawriter->protocol.rtps_reliable_writer.
                heartbeats_per_max_samples;
        }
        else
        {
            rtps_property.samples_per_hb =
                datawriter->resource_limits.max_samples /
                datawriter->protocol.rtps_reliable_writer.
                heartbeats_per_max_samples;
        }
    }

    rtps_property._parent._parent.timer = datawriter->config->timer;

#if OSAPI_ENABLE_TRACE
    rtps_property.session_name =
            DDS_TopicDescription_get_name(
                    DDS_Topic_as_topicdescription(datawriter->topic));
#endif

#if DDS_FILTERING_ENABLED
    /* Writer-side filtering is not supported with asynchronous publication */
    if (rtps_property.netio_fc == NULL)
    {
        if (!DDS_DataWriter_create_filter(datawriter, &datawriter->writer_filter))
        {
            DDSC_LOG_CREATE_WRITER_FILTER(OSAPI_LOGKIND_ERROR)
            goto done;
        }
    }
    rtps_property.filter_plugin = (struct RTPS_FilterPlugin *)datawriter->config->filter_plugin;
    rtps_property.writer_filter = datawriter->writer_filter;
#endif

    rtps_property.transport_priority = qos->transport_priority.value;
    datawriter->rtps_intf = NETIO_InterfaceFactory_create_component(factory,
                                    &rtps_property._parent._parent,NULL);
    if (datawriter->rtps_intf == NULL)
    {
        DDSC_LOG_COMPONENT_CREATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_RTPS_COMPONENT,
                                  NETIO_DEFAULT_RTPS_NAME)
        goto done;
    }

    datawriter->as_entity.state = RTIDDS_ENTITY_STATE_CREATED;

    retval = DDS_BOOLEAN_TRUE;

done:
#ifndef RTI_CERT
    if (!retval)
    {
        (void)DDS_DataWriterImpl_finalize(datawriter);
    }
#endif
    return retval;
}

/*ci
 * \brief Refresh the datawriter liveliness
 *
 * \details
 * If the datawriter offers finite liveliness a timer is started, and if
 * the timer expires the datawriter has failed to maintain its liveliness.
 * This function refreshes the liveliess timer and is called when the
 * datawriter does something considered to be an indication of being alive,
 * such as writing samples.
 *
 * \param[in] self Datawriter to update liveliness on
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
DDS_DataWriter_update_liveliness(DDS_DataWriter *self)
{
    struct DDS_DataWriterImpl *datawriter = (struct DDS_DataWriterImpl *)self;

    if (!DDS_Duration_is_infinite(&datawriter->liveliness.lease_duration))
    {
        switch (datawriter->liveliness.kind)
        {
        case DDS_MANUAL_BY_PARTICIPANT_LIVELINESS_QOS:
        case DDS_MANUAL_BY_TOPIC_LIVELINESS_QOS:
            if (datawriter->writer_state == WRITERSTATE_NOT_ALIVE)
            {
                struct OSAPI_TimeoutUserData storage =
                                            OSAPI_TimeoutUserData_INITIALIZER;

                storage.field[0] = (void *)datawriter;

                datawriter->writer_state = WRITERSTATE_ALIVE;

                /* 0< = nanosec <= 1s in ns (DDS spec) */
                return OSAPI_Timer_create_timeout(
                           datawriter->config->timer,
                           &datawriter->liveliness_event,
                           datawriter->liveliness.lease_duration.sec,
                           (RTI_INT32)datawriter->liveliness.lease_duration.nanosec,
                           OSAPI_TIMER_PERIODIC,
                           DDS_DataWriterEvent_on_liveliness,
                           &storage);
            }
            else
            {
                return OSAPI_Timer_update_timeout(self->config->timer,
                       &datawriter->liveliness_event,
                       datawriter->liveliness.lease_duration.sec,
                       (RTI_INT32)datawriter->liveliness.lease_duration.nanosec);
            }

        case DDS_AUTOMATIC_LIVELINESS_QOS:
            /* participant automatically maintain liveliness so DW has nothing
             * to do/check
             */
            return RTI_TRUE;

        default:
            return RTI_FALSE;
        }
    }

    return RTI_TRUE;
}

/*ci
 * \brief Serialize a sample
 *
 * \details
 * This function serializes a sample and readies it for transmission. The
 * function works in two ways. If the payload is NULL, it allocates the
 * payload and serializes the sample. If the payload is != NULL, it assumes
 * that the payload is already serialized and there is no further work to be
 * done.
 *
 * This function
 * \param[in] self           Datawriter that is writing the sample
 * \param[in] instance_data  Sample to write, NULL if no data
 * \param[in] handle         The instance handle for the sample, if any
 * \param[in] sample_info    Meta-data about the sample
 *
 * \return DDS_RETCODE_OK on success, one of the \ref DDS_ReturnCode_t on
 *         failure
 *
 * \sa \ref DDS_DataWriter_write, \ref DDS_DataWriter_write_w_timestamp,
 *     \ref DDS_DataWriter_unregister_instance,
 *     \ref DDS_DataWriter_unregister_instance_w_timestamp,
 *     \ref DDS_DataWriter_dispose, \ref DDS_DataWriter_dispose_w_timestamp
 */
DDS_ReturnCode_t
DDS_DataWriter_prepare_sample(struct DDS_DataWriterImpl *self,
                                struct DDS_DataWriterSample *sample,
                                NETIO_Packet_T *packet,
                                struct REDA_SequenceNumber *sn)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    struct NETIO_PacketInfo *pkt_info;
    DDS_InstanceHandle_t ih;
    UNUSED_ARG(self);

    if (!NETIO_Packet_initialize(packet,NULL,0,0,NULL))
    {
        DDSC_LOG_PACKET_INIT(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_NETIO_KIND);
        goto done;
    }

    pkt_info = NETIO_Packet_get_info(packet);

    /* General protocol data */
    packet->ref = sample;
    pkt_info->sn = *sn;
    pkt_info->timestamp = sample->sample_info.timestamp;
    ih = DDS_Entity_get_instance_handle(DDS_DataWriter_as_entity(self));
    NETIO_Address_set_guid_from_array(&packet->source,0,ih.octet);

    /* Add intra-transport information. Since Micro does not support multi-
     * encapsulation this is the only option. Since intra-transport is
     * a synchronous call the data pointer is valid on the receiving end
     */
    pkt_info->protocol_id = NETIO_PROTOCOL_INTRA;
    pkt_info->protocol_data.intra_info.user_data = (void*)sample->instance_data;

    retcode = DDS_RETCODE_OK;

done:

    return retcode;
}

/*ci
 * \brief Write an untyped sample
 *
 * \details
 * This is a generic, untyped function that can write a sample of any type,
 * with or without an instance-handle and with or with data. Additional
 * meta-data about the sample is passed in, such as a timestamp.
 *
 * This function
 * \param[in] self           Datawriter that is writing the sample
 * \param[in] instance_data  Sample to write, NULL if no data
 * \param[in] handle         The instance handle for the sample, if any
 * \param[in] sample_info    Meta-data about the sample
 *
 * \return DDS_RETCODE_OK on success, one of the \ref DDS_ReturnCode_t on
 *         failure
 *
 * \sa \ref DDS_DataWriter_write, \ref DDS_DataWriter_write_w_timestamp,
 *     \ref DDS_DataWriter_unregister_instance,
 *     \ref DDS_DataWriter_unregister_instance_w_timestamp,
 *     \ref DDS_DataWriter_dispose, \ref DDS_DataWriter_dispose_w_timestamp
 */
DDS_ReturnCode_t
DDS_DataWriter_write_untyped(DDS_DataWriter *self,
                             const void *instance_data,
                             const DDS_InstanceHandle_t *handle,
                             struct NDDS_DataWriterSampleInfo *sample_info)
{
    struct DDS_DataWriterImpl *datawriter = (struct DDS_DataWriterImpl *)self;
    DDS_ReturnCode_t retcode = DDS_RETCODE_PRECONDITION_NOT_MET;
    struct DDS_DataWriterSample *sample = NULL;
    DDSHST_WriterSampleEntryRef_T sample_entry = NULL;
    struct DDSHST_WriterState *wh_state;
    struct NETIO_Address destination = NETIO_Address_INITIALIZER;
    DDS_InstanceHandle_t actual_instance = DDS_HANDLE_NIL_NATIVE;
    struct NETIO_PacketInfo *pkt_info;
    NDDS_TypePluginKeyKind key_kind;
    DDSHST_WriterEntryKind_T entry_kind;
    RTI_BOOL assert_key = RTI_TRUE;
    DDS_InstanceHandle_t ih;
    struct DDSHST_InstanceState key_state;
    DDSHST_WriterErrorKind_T ec;
    RTI_UINT32 v2_sample_handle = 0;
#if DDS_FILTERING_ENABLED
    DDS_Boolean retval;
#endif
    DDS_KeyHash_t key_hash_buf;

 	OSAPI_TRACE_PRINTF1("write topic: %s",DDS_TopicDescription_get_name(
 	                    DDS_Topic_as_topicdescription(datawriter->topic)))


    if (!OSAPI_Mutex_take(datawriter->write_lock))
    {
        DDSC_LOG_LOCK(OSAPI_LOGKIND_ERROR,"write_lock")
        return DDS_RETCODE_ERROR;
    }

    datawriter->packet = NULL;

    key_kind = DDS_TypePlugin_get_key_kind(datawriter->type_plugin);

    if ((key_kind != NDDS_TYPEPLUGIN_NO_KEY)
         && DDS_InstanceHandle_is_nil(handle)
         && (instance_data == NULL))
    {
        retcode = DDS_RETCODE_BAD_PARAMETER;
        goto done;
    }

    if (!DDS_Entity_is_enabled(&self->as_entity))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_ENTITY)
        retcode = DDS_RETCODE_NOT_ENABLED;
        goto done;
    }

    if (DDS_TypePlugin_is_managed_samples(datawriter->type_plugin))
    {
        DDS_LoanedSampleState_T loaned_sample_state;

#if OSAPI_ENABLE_PRECONDITION
        /* Only check the owner of the sample in DEBUG mode to limit impact on
         * performance
         */
        if (!DDS_TypeMemoryPlugin_is_owner(datawriter->type_plugin->allocator_plugin,
                                           instance_data))
        {
            retcode = DDS_RETCODE_PRECONDITION_NOT_MET;
            goto done;
        }
#endif

        /* For memory manager v2 we need to add a reference from the pool to the history sample
         * It also makes sure we are using a loaned sample. The check above also provides the same
         * but it is disabled for Release mode.
         */
        if (DDS_TypePlugin_get_allocator_plugin_memory_kind(
                datawriter->type_plugin) == RTI_MEMORY_MANAGER_SHMEMV2)
        {
            if (!DDS_TypeMemoryPlugin_get_reference(
                datawriter->type_plugin->allocator_plugin, instance_data,
                &v2_sample_handle))
            {
                /* if the sample handle could not be retreived we return */
                retcode = DDS_RETCODE_PRECONDITION_NOT_MET;
                goto done;
            }
        }

        /* Check the sample state and return an error if the sample has been committed
         * to the DataWriter history.
         */
        if (!DDS_TypePlugin_get_sample_state(datawriter->type_plugin,
                                             instance_data,
                                             &loaned_sample_state))
        {
            retcode = DDS_RETCODE_ERROR;
            goto done;
        }

        if (loaned_sample_state == TYPEPLUGIN_SAMPLE_STATE_COMMITTED)
        {
            retcode = DDS_RETCODE_ERROR;
            goto done;
        }
    }

    if (!DDS_DataWriter_update_liveliness(self))
    {
        retcode = DDS_RETCODE_ERROR;
        goto done;
    }

    if ((key_kind == NDDS_TYPEPLUGIN_USER_KEY) ||
        (key_kind == NDDS_TYPEPLUGIN_GUID_KEY))
    {
        if (DDS_InstanceHandle_is_nil(handle))
        {
            key_hash_buf.length = RTPS_KEY_HASH_MAX_LENGTH;
            OSAPI_Memory_zero(key_hash_buf.value, key_hash_buf.length);
            OSAPI_Memory_zero(&actual_instance.octet, RTPS_KEY_HASH_MAX_LENGTH);
            if ((key_kind == NDDS_TYPEPLUGIN_USER_KEY) ||
                (self->as_entity.entity_id == RTPS_OBJECT_ID_WRITER_SDP_PARTICIPANT))
            {
                CDR_Stream_reset(self->md5_stream);
                if (!DDS_TypePlugin_instance_to_keyhash(datawriter->type_plugin,
                                                        self->md5_stream,
                                                        &key_hash_buf,
                                                        instance_data,
                                                        datawriter->cdr_id))
                {
                    DDSC_LOG_DW_KEYHASH_CREATE(OSAPI_LOGKIND_ERROR)
                    retcode = DDS_RETCODE_PRECONDITION_NOT_MET;
                    goto done;
                }
            }
            else
            {
                /* The only endpoints which use NDDS_TYPEPLUGIN_GUID_KEY are:
                 *   - WRITER_SDP_PUBLICATION
                 *   - WRITER_SDP_SUBSCRIPTION
                 *   - WRITER_SEDP_PUBLICATION
                 *   - WRITER_SEDP_SUBSCRIPTION
                 *   - WRITER_SEDP_PARTICIPANT
                 *
                 * Since WRITER_SEDP_PARTICIPANT is handled above, only the
                 * remaining ones must be handled here.
                 *   */
                if (self->as_entity.entity_id == RTPS_OBJECT_ID_WRITER_SDP_PUBLICATION)
                {
                    ih = DDS_Entity_get_instance_handle(DDS_DataReader_as_entity(instance_data));
                }
                else
                {
                    ih = DDS_Entity_get_instance_handle(DDS_DataWriter_as_entity(instance_data));
                }
                OSAPI_Memory_copy(key_hash_buf.value, ih.octet, RTPS_KEY_HASH_MAX_LENGTH);
            }
            OSAPI_Memory_copy(&actual_instance.octet, &key_hash_buf.value,
                              key_hash_buf.length);
        }
        else
        {
            OSAPI_Memory_copy(key_hash_buf.value, handle->octet, RTPS_KEY_HASH_MAX_LENGTH);

            /* The generated code gives the _max_ size which could be
             * larger than RTPS_KEY_HASH_MAX_LENGTH. In that case the length to
             * RTPS_KEY_HASH_MAX_LENGTH since that is the max key hash that
             * is sent. For keyhashes < RTPS_KEY_HASH_MAX_LENGTH the keyhash
             * is padded to 0s to always be of length RTPS_KEY_HASH_MAX_LENGTH;
             */
            key_hash_buf.length = RTPS_KEY_HASH_MAX_LENGTH;
            actual_instance = *handle;
        }
    }
    else if (key_kind != NDDS_TYPEPLUGIN_NO_KEY)
    {
        DDSC_LOG_DW_ILLEGAL_KEY_KIND(OSAPI_LOGKIND_ERROR,key_kind)
        retcode = DDS_RETCODE_PRECONDITION_NOT_MET;
        goto done;
    }

    if ((sample_info->status_info & (RTPS_DISPOSE_STATUS_INFO |
                                    RTPS_UNREGISTER_STATUS_INFO)) ==
           (RTPS_DISPOSE_STATUS_INFO | RTPS_UNREGISTER_STATUS_INFO))
    {
        entry_kind = DDSHST_WRITER_ENTRY_UNREGISTER_DISPOSE;
    }
    else if (sample_info->status_info & RTPS_UNREGISTER_STATUS_INFO)
    {
        entry_kind = DDSHST_WRITER_ENTRY_UNREGISTER;
    }
    else if (sample_info->status_info & RTPS_DISPOSE_STATUS_INFO)
    {
        entry_kind = DDSHST_WRITER_ENTRY_DISPOSE;
    }
    else
    {
        entry_kind = DDSHST_WRITER_ENTRY_NORMAL;
    }

    /* If we need to save the sample, add it to the history queue. It would be
     * nice to store the same in case of best-effort and send it once also
     * for late joiners.
     */
    actual_instance.is_valid = DDS_BOOLEAN_TRUE;

    /* Allocate a queue entry. There are three cases:
     * 1. write() - If the handle is not NIL, use it. If it is NIL a key
     *    is calculated and it is auto-registered (if there are resources)
     *
     * 2. dispose() - If the handle is supplied then only allow
     *    the operation to complete if the key already exists. If the handle
     *    is NIL then auto-registration is done and a key may be reserved even
     *    if it does not exist.
     *
     * 2. unregister() - If the handle is supplied then only allow
     *    the operation to complete if the key already exists. If the handle
     *    is NIL then auto-registration is done, but an entry cannot be added
     *    unless it already exists.
     *
     * The default behavior is to allow a key to be added even if it does not
     * exist. The exception is if a non nil-handle is used or if an instance
     * is unregistered.
     */
    if (!DDS_InstanceHandle_is_nil(handle) ||
        (sample_info->status_info == RTPS_UNREGISTER_STATUS_INFO))
    {
        assert_key = RTI_FALSE;
    }

    /* If an instance has been registered/asserted but has not been published,
     * then check if a dispose/unregister should still be sent. This feature
     * is useful when instances resources are preallocated, but not actually
     * used.
     */
    if ((sample_info->status_info & (RTPS_DISPOSE_STATUS_INFO |
                                     RTPS_UNREGISTER_STATUS_INFO)) &&
         self->management.disable_unregister_dispose_for_unpublished_instance)
    {
        if (DDS_InstanceHandle_is_nil(handle))
        {
            retcode = DDS_RETCODE_BAD_PARAMETER;
            goto done;
        }

        if (DDSHST_Writer_get_instance_state(self->wh,
                      &actual_instance,&key_state) != DDSHST_RETCODE_SUCCESS)
        {
            retcode = DDS_RETCODE_BAD_PARAMETER;
            goto done;
        }

        if (key_state.sample_count == 0)
        {
            /* Unregistering the key is a local operation and immediate */
            if (DDSHST_Writer_unregister_key(self->wh,&actual_instance) !=
                    DDSHST_RETCODE_SUCCESS)
            {
                retcode = DDS_RETCODE_BAD_PARAMETER;
                goto done;
            }
            retcode = DDS_RETCODE_OK;
            goto done;
        }

        /* Getting here means that at least one sample has been published
         * for the instance and unregister/dispose must proceed as normal
         * regardless of disable_unregister_dispose_for_unpublished_instance
         */
    }

    sample_entry = DDSHST_Writer_get_entry(self->wh, &actual_instance,
                                           entry_kind,assert_key,
                                           &sample_info->timestamp,
                                           &ec);

    if (sample_entry == NULL)
    {
        DDSC_LOG_RESOURCE_EXCEEDED(OSAPI_LOGKIND_ERROR,DDSC_LOG_SAMPLE_RESOURCES)

        if ((ec == DDSHST_WRITER_ERROR_OUT_OF_SAMPLE_RESOURCES) ||
            (ec == DDSHST_WRITER_ERROR_OUT_OF_INSTANCE_RESOURCES) ||
            (ec == DDSHST_WRITER_ERROR_BLOCKING))
        {
            retcode = DDS_RETCODE_OUT_OF_RESOURCES;
            goto done;
        }
        else if (ec == DDSHST_WRITER_ERROR_TIMEOUT)
        {
            retcode = DDS_RETCODE_TIMEOUT;
            goto done;
        }
        else if (!assert_key)
        {
            /* The key did not exist, return here */
            retcode = DDS_RETCODE_BAD_PARAMETER;
            goto done;
        }
        else
        {
            retcode = DDS_RETCODE_ERROR;
            goto done;
        }
    }

    sample = (struct DDS_DataWriterSample*)
                                  REDA_BufferPool_get_buffer(self->sample_pool);
    if (sample == NULL)
    {
        DDSC_LOG_RESOURCE_EXCEEDED(OSAPI_LOGKIND_ERROR,DDSC_LOG_SAMPLE_RESOURCES)
        DDSHST_Writer_return_entry(self->wh, sample_entry);
        retcode = DDS_RETCODE_OUT_OF_RESOURCES;
        goto done;
    }

    /* Anonymous writers may re-publish the same sample, e.g for discovery */
    if (!self->management.is_anonymous)
    {
        REDA_SequenceNumber_plusplus(&(self->last_sn));
    }

    /* Move the serialization of samples to the DataWriterInterface. The
     * reason for this is that it is the writer interface that has access
     * to the information on encapsulations per locator. To avoid either
     * duplication or sharing of data between the writer and the writer
     * interface the serialization is moved to the send loop. Another
     * advantage is that when the serialization is done it is sent to all
     * relevant locators. Thus, in theory it is possible to serialize a
     * new format while an existing one is being sent out.
     *
     * Thus, the writer does everything _but_ serialize the sample. What is
     * committed to the writer queue is an "empty sample".
     */
    sample->payload = NULL;
    sample->sample_info = *sample_info;
    sample->instance_data = instance_data;

    /* Reset the inline pbuf. Note that this does _not_ clear the content
     * of the buffer, it only resets head and tail position. Thus, the
     * key, if present, is still in the buffer at the correct location.
     * A keyhash is _always_ sent for a keyed datawriter.
     */
    NETIO_PacketBuffer_reset(&sample->inline_pbuf);

    if (datawriter->send_key_hash)
    {
        /* copy the keyhash into the inline Qos buffer after the submessage
         * header which added later.
         */
        char *inline_keyhash_offset =
                (char*)NETIO_PacketBuffer_get_head(&sample->inline_pbuf)
                + RTPS_SUBMESSAGE_HEADER_LENGTH;

        OSAPI_Memory_copy(inline_keyhash_offset,
                          &actual_instance.octet[0],
                          RTPS_KEY_HASH_MAX_LENGTH);
    }

    /* set the v2 sample handle retreived before. Other plugins
     * do not use this value and it is okay to be uninitialized.
     */
    sample->_sample.sample_index = v2_sample_handle;

    datawriter->packet = (struct NETIO_Packet*)REDA_BufferPool_get_buffer(
                                            datawriter->config->packet_pool);
    if (datawriter->packet == NULL)
    {
        DDSC_LOG_NETIO_PACKETPOOL_GET(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    retcode = DDS_DataWriter_prepare_sample(datawriter,
                                            sample,
                                            datawriter->packet,
                                            &self->last_sn);

    if (retcode != DDS_RETCODE_OK)
    {
        DDSHST_Writer_return_entry(self->wh, sample_entry);
        goto done;
    }

    /* Make sure an error is returned if any of the succeeding calls fails
     */
    retcode = DDS_RETCODE_ERROR;

    /* Need to set this flag before committing the sample because the
     * sample have to be committed to the writer cache to be able to send
     * it.
     */
    sample->rtps_flags = NETIO_RTPS_FLAGS_IN_PROGRESS;

    if (DDS_TypePlugin_is_managed_samples(datawriter->type_plugin))
    {
        if (!DDS_TypePlugin_set_sample_state(
                datawriter->type_plugin,
                instance_data,
                TYPEPLUGIN_SAMPLE_STATE_COMMITTED,
                DDS_BOOLEAN_FALSE))
        {
            DDSC_LOG_DW_COMMIT(OSAPI_LOGKIND_ERROR)
            goto done;
        }
    }

    if (DDSHST_Writer_commit_entry(self->wh,
            sample_entry, &sample->_sample,
            &self->last_sn,
            ((struct DDS_DataWriterInterface*)datawriter->dw_intf)->active_acking_readers) != DDSHST_RETCODE_SUCCESS)
    {
        DDSC_LOG_DW_COMMIT(OSAPI_LOGKIND_ERROR)

        /* Revert setting the sample state */
        if (!DDS_TypePlugin_set_sample_state(
                datawriter->type_plugin,
                instance_data,
                TYPEPLUGIN_SAMPLE_STATE_COMMITTED, /* This parameter isn't used when */
                DDS_BOOLEAN_TRUE))                 /* when this one is TRUE          */
        {
            DDSC_LOG_DW_COMMIT(OSAPI_LOGKIND_ERROR)
        }
        goto done;
    }

    wh_state = DDSHST_Writer_get_state(datawriter->wh);

    pkt_info = NETIO_Packet_get_info(datawriter->packet);
    pkt_info->committable_sn = wh_state->high_sn;
    pkt_info->first_available_sn = wh_state->low_sn;

    /* The protocol dictates that commitable is everything up to last SN + 1 */
    REDA_SequenceNumber_plusplus(&pkt_info->committable_sn);

#if DDS_FILTERING_ENABLED
    if (DDS_DataWriter_is_filtering_enabled(datawriter))
    {
        DDS_Boolean unregister_or_dispose = (sample->sample_info.status_info &
                                (RTPS_DISPOSE_STATUS_INFO | RTPS_UNREGISTER_STATUS_INFO)) != 0;

        /* Evaluate the filters of all matched readers against the sample
         * before it is serialized.
         */
        retval = DDS_DataWriter_evaluate_filter(datawriter,
                                                &self->last_sn,
                                                unregister_or_dispose,
                                                sample->instance_data);
#if OSAPI_ENABLE_LOG
        if (!retval)
        {
            DDSC_LOG_EVALUATE_WRITER_FILTER(OSAPI_LOGKIND_WARNING)
        }
#else
        IGNORE_RETVAL(retval);
#endif
    }
#endif /* DDS_FILTERING_ENABLED */

    /* Pass packet to the datawriter interface to send it to the correct
     * destinations.
     */

    /* Coverity incorrectly warns that NETIO_Interface_send goes to the
     * shmem interface. However, the call only goes to the datawriter interface.
     * Mark this warning as a false positive.
     */

    /* coverity[var_deref_model : FALSE] */
    /* coverity[cert_exp34_c_violation : FALSE] */
    if (!NETIO_Interface_send(datawriter->dw_intf,
                              datawriter->dw_intf,&destination,
                              datawriter->packet))
    {
        DDSC_LOG_NETIO_SEND_FAILED(OSAPI_LOGKIND_ERROR)
    }

    retcode = DDS_RETCODE_OK;

done:


    if (retcode != DDS_RETCODE_OK)
    {
        if (sample_entry != NULL)
        {
            DDSHST_Writer_return_entry(self->wh, sample_entry);
        }

        if (sample != NULL)
        {
            REDA_BufferPool_return_buffer(self->sample_pool,sample);
        }
    }

    if (datawriter->packet != NULL)
    {
         REDA_BufferPool_return_buffer(datawriter->config->packet_pool,
                                       datawriter->packet);
        datawriter->packet = NULL;
    }

    if (!OSAPI_Mutex_give(datawriter->write_lock))
    {
        DDSC_LOG_UNLOCK(OSAPI_LOGKIND_ERROR,"write_lock")
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}

/*ci
 * \brief Register a key in the datawriter
 *
 * \details
 * The key registration reserve an entry in the datawriter history-cache.
 * The function assumes that at least all the key fields are set to the
 * correct values, there is no attempt to determine whether a key field
 * is set or not.
 *
 * \param[in]  datawriter    Datawriter to register key in
 * \param[out] handle        Handle to the registered key
 * \param[in]  instance_data A sample with at least all the key fields set
 * \param[in]  timestamp     The timestamp when the key was registered,
 *                           not used
 * \return DDS_RETCODE_OK on success, one of the \ref DDS_ReturnCode_t
 *         on error
 */
DDS_ReturnCode_t
DDS_DataWriter_register_key(struct DDS_DataWriterImpl *datawriter,
                            DDS_InstanceHandle_t *handle,
                            const void *instance_data,
                            const struct OSAPI_SystemTime *timestamp)
{
    DDS_KeyHash_t keyHashBuffer = DDS_KEY_HASH_DEFAULT;
    DDSHST_ReturnCode_T whrc;

    if (DDS_TypePlugin_get_key_kind(datawriter->type_plugin) == NDDS_TYPEPLUGIN_GUID_KEY)
    {
        /* GUID keys */
        DDS_InstanceHandle_from_rtps(
           handle, (const struct RTPS_Guid *)instance_data);
    }
    else
    {
        /* user key */
        keyHashBuffer.length = RTPS_KEY_HASH_MAX_LENGTH;
        OSAPI_Memory_zero(keyHashBuffer.value, keyHashBuffer.length);

        if (!DDS_TypePlugin_instance_to_keyhash(datawriter->type_plugin,
                                                datawriter->md5_stream,
                                                &keyHashBuffer,
                                                instance_data,
                                                datawriter->cdr_id))
        {
            DDSC_LOG_DW_KEYHASH_CREATE(OSAPI_LOGKIND_ERROR)
            return DDS_RETCODE_PRECONDITION_NOT_MET;
        }
        OSAPI_Memory_copy(&handle->octet,
                          &keyHashBuffer.value, keyHashBuffer.length);
        handle->is_valid = DDS_BOOLEAN_TRUE;
    }

    whrc = DDSHST_Writer_register_key(datawriter->wh, handle,timestamp);
    if (whrc != DDSHST_RETCODE_SUCCESS)
    {
        DDSC_LOG_DW_HISTORY_REGISTER_KEY(OSAPI_LOGKIND_ERROR)
        return DDS_RETCODE_OUT_OF_RESOURCES;
    }

    return DDS_RETCODE_OK;
}

/*ci
 * \brief Update the expected number of acknowledgments for a sample
 *
 * \details
 * The writer keeps track of how many reliable readers it is matched with
 * to determine how many acknowledgments are expected for a given
 * SN. This is again used to determine which samples can be purged if
 * resources need to be reclaimed.
 *
 * \param[in] self The datawriter to update the expected acknowledgment count
 *                 for
 */
void
DDS_DataWriter_update_historical_ackcount(DDS_DataWriter *self)
{
    struct DDS_DataWriterImpl *dw = (struct DDS_DataWriterImpl *)self;
    struct OSAPI_SystemTime now = OSAPI_TIME_ZERO;
    struct DDSHST_WriterEvent event;
    RTI_BOOL bretval;

    event.kind = DDSHST_WRITEREVENT_KIND_HISTORICAL_DATA_REQUESTED;

    bretval = OSAPI_System_get_time(&now);

#if OSAPI_ENABLE_LOG
    /* Not important, can continue */
    if (!bretval)
    {
        DDSC_LOG_SYS_GETTIME(OSAPI_LOGKIND_ERROR)
    }
#else
    IGNORE_RETVAL(bretval);
#endif

    DDSHST_Writer_post_event(dw->wh,&event,&now);
}

/*ci
 * \brief Check if a DDS_DataWriterListener is consistent
 *
 * \param[in] l DDS_DataWriterListener to test for consistency
 * \param[in] m The current status mask to test
 *
 * \return DDS_BOOLEAN_TRUE if the listener is consistent, DDS_BOOLEAN_FALSE
 *         otherwise
 */
DDS_Boolean
DDS_DataWriterListener_is_consistent(const struct DDS_DataWriterListener *l,
                                     DDS_StatusMask m)
{
    return (((!((m) & DDS_OFFERED_DEADLINE_MISSED_STATUS)) ||
            (l->on_offered_deadline_missed != NULL)) &&
            ((!((m) & DDS_OFFERED_INCOMPATIBLE_QOS_STATUS)) ||
            (l->on_offered_incompatible_qos != NULL)) &&
            ((!((m) & DDS_LIVELINESS_LOST_STATUS)) ||
            (l->on_liveliness_lost != NULL)) &&
            ((!((m) & DDS_PUBLICATION_MATCHED_STATUS)) ||
            (l->on_publication_matched != NULL)) &&
            ((!((m) & DDS_RELIABLE_READER_ACTIVITY_CHANGED_STATUS)) ||
            (l->on_reliable_reader_activity_changed != NULL))) ?
            DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

void
DDS_DataWriter_return_sample_payload(DDS_DataWriter *self,
                                     struct DDS_DataWriterSample *sample)
{
    struct DDS_DataWriterImpl *datawriter = (struct DDS_DataWriterImpl*)self;
    struct DDS_TypePluginBuffer *payload;
    struct DDS_TypePluginBuffer *payload_next;

    payload = sample->payload;
    while (payload != NULL)
    {
        /* It is technically not needed to unlink the payload before returning
         * since this is all within critical section. However, it is defensive
         * if the type-plugin tried to access it.
         */
        payload_next = payload->_next;
        payload->_next = NULL;
        DDS_TypePlugin_return_buffer(datawriter->type_plugin,payload);
        payload = payload_next;
    }

    if (sample->payload != NULL)
    {
        sample->payload = NULL;
    }

    if ((sample->instance_data != NULL) &&
        (sample->rtps_flags & NETIO_RTPS_FLAGS_REMOVED) &&
        DDS_TypePlugin_is_managed_samples(self->type_plugin) &&
        (!DDS_TypePlugin_allocator_plugin_is_v2(self->type_plugin)))
    {
        /* The cast to void* is neccesary because of a collision of
         * APIs. The public API is a const to the data, but internally
         * we may have to free up resources with the sample if the
         * sample is owned by the middle-ware.
         */
        if (!DDS_TypePlugin_delete_sample(datawriter->type_plugin,
                                          (void*)sample->instance_data))
        {

        }
    }
}

DDS_Long
DDS_DataWriter_get_writer_loaned_sample_allocation(const DDS_DataWriter *self)
{
    return self->writer_resource_limits.writer_loaned_sample_allocation;
}

DDS_Boolean
DDS_DataWriter_get_initialize_writer_loaned_sample(const DDS_DataWriter *self)
{
    return self->writer_resource_limits.initialize_writer_loaned_sample;
}

DDS_Long
DDS_DataWriter_get_max_samples(const DDS_DataWriter *self)
{
    return self->resource_limits.max_samples;
}

DDS_Long
DDS_DataWriter_get_max_remote_readers(const DDS_DataWriter *self)
{
    return self->writer_resource_limits.max_remote_readers;
}

DDS_UnsignedShort
DDS_DataWriter_get_zcv2_protocol_version(const DDS_DataWriter *self)
{
    return self->zcv2_protocol_version;
}

DDS_Boolean
DDS_DataWriter_is_announced(const DDS_DataWriter *self)
{
    return self->management.is_announced;
}

DDS_Boolean
DDS_DataWriter_is_anonymous(const DDS_DataWriter *self)
{
    return self->management.is_anonymous;
}

const struct DDS_DataRepresentationQosPolicy*
DDS_DataWriter_get_data_representation_ref(const DDS_DataWriter *self)
{
    return self->representation;
}

const struct DDS_TransportEncapsulationQosPolicy*
DDS_DataWriter_get_encapsulation_ref(const DDS_DataWriter *self)
{
    return self->encapsulation;
}

const struct DDS_LocatorSeq *
DDS_DataWriter_get_unicast_locator_ref(const DDS_DataWriter *self)

{
    return self->writer_data.unicast_locator;
}

const struct DDS_LatencyBudgetQosPolicy*
DDS_DataWriter_get_latency_budget_ref(const DDS_DataWriter *self)
{
    return &self->latency_budget;
}

const struct DDS_LocatorSeq*
DDS_DataWriter_get_resolved_locator_ref(const DDS_DataWriter *self)

{
    return self->writer_data.resolved_participant_locators;
}

const struct DDS_DataWriterResourceLimitsQosPolicy*
DDS_DataWriter_get_writer_resource_limits_ref(const DDS_DataWriter *self)
{
    return &self->writer_resource_limits;
}

void
DDS_DataWriter_get_request_offered_qos(const DDS_DataWriter *self,
                                       struct DDS_DataWriterQos *qos)
{
    qos->deadline = self->deadline;
    qos->destination_order = self->destination_order;
    qos->durability = self->durability;
    qos->latency_budget = self->latency_budget;
    qos->liveliness = self->liveliness;
    qos->ownership = self->ownership;
    qos->reliability = self->reliability;
    qos->representation = *self->representation;
}

/*ci @} */
