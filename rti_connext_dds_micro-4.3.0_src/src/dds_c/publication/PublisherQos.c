/*
 * FILE: PublisherQos.c - Publisher Qos implementation
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
 * 28apr2014,as MICRO-371 (Verocel PR#1527) Removed use of OSAPI_Memory_compare
 *              in DDS_PublisherQos_is_equal in favor of direct comparison of
 *              qos structure's members.
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief Publisher Qos implementation
 *
 * \details
 * This file implements functions to manage the PublisherQos structure
 *
 * \ingroup DDSDomainModule
 */
/*ci \addtogroup DDSPublicationModule
 * @{
 */
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif

#ifndef dds_c_publication_h
  #include "dds_c/dds_c_publication.h"
#endif

const struct DDS_PublisherQos DDS_PUBLISHER_QOS_DEFAULT = DDS_PublisherQos_INITIALIZER;

#include "PublisherImpl.h"
#include "PublisherQos.h"
#include "QosPolicy.h"
#include "UserDataQosPolicy.h"
#include "PartitionQosPolicy.h"

/*** SOURCE_BEGIN ***/

DDS_ReturnCode_t
DDS_PublisherQos_set_from(
        struct DDS_PublisherQos *out,
        const struct DDS_PublisherQos *in,
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

        if (!DDS_UserDataManager_assert_user_data(
                DDS_DomainParticipant_get_user_data_manager(participant),
                DDS_USER_DATA_PUBLISHER_TYPE,
                &in->group_data.value,
                &out->group_data.value))
        {
            return DDS_RETCODE_ERROR;
        }
    }
    else
    {
        if (DDS_PartitionQosPolicy_copy(&out->partition,&in->partition) !=
            DDS_BOOLEAN_TRUE)
        {
            return DDS_RETCODE_ERROR;
        }

        if (DDS_GroupDataQosPolicy_copy(&out->group_data,&in->group_data) !=
            DDS_RETCODE_OK)
        {
            return DDS_RETCODE_ERROR;
        }
    }

    out->entity_factory = in->entity_factory;
    out->management = in->management;
    out->publisher_name = in->publisher_name;

    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t
DDS_PublisherQos_copy(struct DDS_PublisherQos *out,
                      const struct DDS_PublisherQos *in)
{
    OSAPI_PRECONDITION_ALWAYS((out == NULL) || (in == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("out",out,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("in",in,RTI_TRUE);)

    return DDS_PublisherQos_set_from(out, in, DDS_BOOLEAN_FALSE, NULL);
}

DDS_ReturnCode_t
DDS_PublisherQos_initialize(struct DDS_PublisherQos *self)
{
    struct DDS_PublisherQos initVal = DDS_PublisherQos_INITIALIZER;

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
DDS_PublisherQos_finalize(struct DDS_PublisherQos *self)
{
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
DDS_PublisherQos_finalize_managed(
        struct DDS_PublisherQos *self,
        DDS_DomainParticipant *participant)
{
    OSAPI_PRECONDITION((self == NULL) || (participant == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
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
            DDS_USER_DATA_PUBLISHER_TYPE) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_PublisherQos_finalize(self) != DDS_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}
#endif /*RTI_CERT*/


DDS_Boolean
DDS_PublisherQos_is_equal(const struct DDS_PublisherQos *left,
                          const struct DDS_PublisherQos *right)
{
    OSAPI_PRECONDITION((left == NULL) || (right == NULL),
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
            && DDS_EntityNameQosPolicy_is_equal(&left->publisher_name,
                                                &right->publisher_name))
            ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/*ci @} */
