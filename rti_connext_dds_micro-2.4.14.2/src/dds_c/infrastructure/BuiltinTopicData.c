/*
 * FILE: BuiltinTopicData.c - BuiltinTopicData implementation
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
 * 06sep2022,tk MICRO-4151/PR.30884
 * - DDS_SubscriptionBuiltinTopicData_copy
 *   - Removed extraneous space before in pointer argument
 * 23feb2022,am MICRO-3442/PR.30141
 * - Added precondition check to
 *   DDS_PublicationBuiltinTopicData_initialize
 *   DDS_SubscriptionBuiltinTopicData_initialize
 *   DDS_SubscriptionBuiltinTopicData_initialize
 * 13apr2021,tk MICRO-3028/PR.28960
 * - Added robustness check to DDS_ParticipantBuiltinTopicData_is_equal,
 *   DDS_ParticipantBuiltinTopicData_copy
 *   DDS_ParticipantBuiltinTopicData_initialize,
 *   DDS_PublicationBuiltinTopicData_is_equal,
 *   DDS_PublicationBuiltinTopicData_initialize,
 *   DDS_PublicationBuiltinTopicData_copy,
 *   DDS_SubscriptionBuiltinTopicData_is_equal,
 *   DDS_SubscriptionBuiltinTopicData_initialize,
 *   DDS_SubscriptionBuiltinTopicData_copy
 * - Made documentation for Public APIs external.
 * 21feb2021,tk MICRO-2897/PR#28838
 *   - Added PresentationQosPolicy to DDS_SubscriptionBuiltinTopicData_copy()
 *     and DDS_SubscriptionBuiltinTopicData_is_equal().
 * 20oct2020,tk MICRO-2623/PR#28237 Replaced tabs with spaces
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

#ifndef RTI_CERT
#ifndef dds_c_string_manager_h
#include "dds_c/dds_c_string_manager.h"
#endif
#endif

#include "QosPolicy.h"
#include "DomainParticipant.h"

/*** SOURCE_BEGIN ***/

#ifdef T
#undef T
#endif

/*ci
 * \brief Initialize a ParticipantBuiltinTopicData sequence element
 *
 * \param[in] self ParticipantBuiltinTopicData structure to initialize
 *
 * \return This function always returns RTI_TRUE
 */
RTI_PRIVATE RTI_BOOL
DDS_ParticipantBuiltinTopicData_initialize_el(
                                struct DDS_ParticipantBuiltinTopicData *self)
{
    struct DDS_ParticipantBuiltinTopicData init_val =
                                   DDS_ParticipantBuiltinTopicData_INITIALIZER;

    *self = init_val;

    return RTI_TRUE;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize a ParticipantBuiltinTopicData sequence element
 *
 * \param[in] self ParticipantBuiltinTopicData structure to finalize
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
DDS_ParticipantBuiltinTopicData_finalize_el(
                                struct DDS_ParticipantBuiltinTopicData *self)
{
    if (self == NULL)
    {
        return(RTI_FALSE);
    }

    if (!DDS_LocatorSeq_finalize(&self->default_unicast_locators))
    {
        return(RTI_FALSE);
    }

    if (!DDS_LocatorSeq_finalize(&self->metatraffic_unicast_locators))
    {
        return(RTI_FALSE);
    }

    if (!DDS_LocatorSeq_finalize(&self->metatraffic_multicast_locators))
    {
        return(RTI_FALSE);
    }

    if (!DDS_LocatorSeq_finalize(&self->default_multicast_locators))
    {
        return(RTI_FALSE);
    }

    return RTI_TRUE;
}
#endif /* !RTI_CERT*/

#define T struct DDS_ParticipantBuiltinTopicData
#define TSeq DDS_ParticipantBuiltinTopicDataSeq
#define T_initialize DDS_ParticipantBuiltinTopicData_initialize_el
#define T_finalize DDS_ParticipantBuiltinTopicData_finalize_el
#define T_copy DDS_ParticipantBuiltinTopicData_copy
#define REDA_SEQUENCE_USER_API
#include "reda/reda_sequence_defn.h"

DDS_Boolean
DDS_ParticipantBuiltinTopicData_is_equal(
                        const struct DDS_ParticipantBuiltinTopicData *left,
                        const struct DDS_ParticipantBuiltinTopicData *right)
{
    OSAPI_PRECONDITION_ALWAYS((left == NULL) || (right == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    if (!DDS_BuiltinTopicKey_equals(&left->key, &right->key))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_EntityNameQosPolicy_is_equal(&left->participant_name,
                                          &right->participant_name))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (left->dds_builtin_endpoints != right->dds_builtin_endpoints)
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_ProtocolVersion_is_equal(&left->rtps_protocol_version,
                                      &right->rtps_protocol_version))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_VendorId_is_equal(&left->rtps_vendor_id, &right->rtps_vendor_id))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_LocatorSeq_is_equal(&left->default_unicast_locators,
                                 &right->default_unicast_locators))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_LocatorSeq_is_equal(&left->default_multicast_locators,
                                 &right->default_multicast_locators))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_LocatorSeq_is_equal(&left->metatraffic_unicast_locators,
                                 &right->metatraffic_unicast_locators))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_LocatorSeq_is_equal(&left->metatraffic_multicast_locators,
                                 &right->metatraffic_multicast_locators))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_Duration_equal(&left->liveliness_lease_duration,
                           &right->liveliness_lease_duration))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_ProductVersion_is_equal(&left->product_version,
                                     &right->product_version))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_ChecksumProperty_is_equal(&left->checksum,&right->checksum))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

DDS_Boolean
DDS_ParticipantBuiltinTopicData_initialize(
                                struct DDS_ParticipantBuiltinTopicData *self)
{
    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)
    
    return DDS_ParticipantBuiltinTopicData_initialize_el(self) ?
                                DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize a ParticipantBuiltinTopicData structure without any deallocations
 *
 * \param[in] self ParticipantBuiltinTopicData structure to finalize
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DDS_ParticipantBuiltinTopicData_finalize_no_dealloc(
                                struct DDS_ParticipantBuiltinTopicData *self)
{
    DDS_Boolean result = DDS_BOOLEAN_FALSE;

    if (self == NULL)
    {
        goto done;
    }

    if (!DDS_LocatorSeq_set_length(&self->default_unicast_locators,0))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_set_length(&self->metatraffic_unicast_locators,0))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_set_length(&self->metatraffic_multicast_locators,0))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_set_length(&self->default_multicast_locators,0))
    {
        goto done;
    }

    result = DDS_BOOLEAN_TRUE;

done:
    return result;
}
#endif /* !RTI_CERT */

#ifndef RTI_CERT
DDS_Boolean
DDS_ParticipantBuiltinTopicData_finalize(
                                struct DDS_ParticipantBuiltinTopicData *self)
{
    return DDS_ParticipantBuiltinTopicData_finalize_el(self) ?
                                        DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}
#endif /* !RTI_CERT*/

DDS_Boolean
DDS_ParticipantBuiltinTopicData_copy(
                            struct DDS_ParticipantBuiltinTopicData *out,
                            const struct DDS_ParticipantBuiltinTopicData *in)
{
    OSAPI_PRECONDITION_ALWAYS((out == NULL) || (in == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("out",out,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("in",in,RTI_TRUE);)

    out->key = in->key;
    out->participant_name = in->participant_name;
    out->dds_builtin_endpoints = in->dds_builtin_endpoints;
    out->rtps_protocol_version = in->rtps_protocol_version;
    out->rtps_vendor_id = in->rtps_vendor_id;

    if (!DDS_LocatorSeq_copy(&out->default_unicast_locators,
                             &in->default_unicast_locators))
    {
        return(DDS_BOOLEAN_FALSE);
    }

    if (!DDS_LocatorSeq_copy(&out->default_multicast_locators,
                             &in->default_multicast_locators))
    {
        return(DDS_BOOLEAN_FALSE);
    }

    if (!DDS_LocatorSeq_copy(&out->metatraffic_unicast_locators,
                             &in->metatraffic_unicast_locators))
    {
        return(DDS_BOOLEAN_FALSE);
    }

    if (!DDS_LocatorSeq_copy(&out->metatraffic_multicast_locators,
                             &in->metatraffic_multicast_locators))
    {
        return(DDS_BOOLEAN_FALSE);
    }

    out->liveliness_lease_duration = in->liveliness_lease_duration;
    out->product_version = in->product_version;
    out->checksum = in->checksum;

    return DDS_BOOLEAN_TRUE;
}

/******************************************************************************
 *                     DDS_PublicationBuiltinTopicData                        *
 ******************************************************************************/

/*ci
 * \brief Initialize a PublicationBuiltinTopicData sequence element
 *
 * \param[in] self PublicationBuiltinTopicData structure to initialize
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
DDS_PublicationBuiltinTopicData_initialize_el(
                                struct DDS_PublicationBuiltinTopicData *self)
{
    struct DDS_PublicationBuiltinTopicData init_val =
                        DDS_PublicationBuiltinTopicData_INITIALIZER;

    *self = init_val;

    self->topic_name = DDS_String_alloc(RTPS_PATHNAME_LEN_MAX + 1);
    if (self->topic_name == NULL)
    {
        return RTI_FALSE;
    }

    self->type_name = DDS_String_alloc(RTPS_PATHNAME_LEN_MAX + 1);
    if (self->type_name == NULL)
    {
        return RTI_FALSE;
    }

    if (!DDS_LocatorSeq_set_maximum(&self->unicast_locator,
                                    RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX))
    {
        return RTI_FALSE;
    }

    if (!DDS_LocatorSeq_set_length(&self->unicast_locator, 0))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize a PublicationBuiltinTopicData sequence element
 *
 * \param[in] self PublicationBuiltinTopicData structure to finalize
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
DDS_PublicationBuiltinTopicData_finalize_el(
                                struct DDS_PublicationBuiltinTopicData *self)
{
    if (self == NULL)
    {
        return (RTI_FALSE);
    }

    if (self->topic_name)
    {
        DDS_String_free(self->topic_name);
        self->topic_name = NULL;
    }

    if (self->type_name)
    {
        DDS_String_free(self->type_name);
        self->type_name = NULL;
    }

    if (!DDS_LocatorSeq_finalize(&self->unicast_locator))
    {
        return(RTI_FALSE);
    }

    return RTI_TRUE;
}
#endif /* !RTI_CERT */

#define T struct DDS_PublicationBuiltinTopicData
#define TSeq DDS_PublicationBuiltinTopicDataSeq
#define T_initialize DDS_PublicationBuiltinTopicData_initialize_el
#define T_finalize DDS_PublicationBuiltinTopicData_finalize_el
#define T_copy DDS_PublicationBuiltinTopicData_copy
#define REDA_SEQUENCE_USER_API
#include "reda/reda_sequence_defn.h"

DDS_Boolean
DDS_PublicationBuiltinTopicData_is_equal(
                        const struct DDS_PublicationBuiltinTopicData *left,
                        const struct DDS_PublicationBuiltinTopicData *right)
{
    OSAPI_PRECONDITION_ALWAYS((left == NULL) || (right == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)


    if (!DDS_BuiltinTopicKey_equals(&left->key, &right->key))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_BuiltinTopicKey_equals(&left->participant_key,
                                    &right->participant_key))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (DDS_String_ncmp(left->topic_name, right->topic_name,
                        RTPS_PATHNAME_LEN_MAX))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (DDS_String_ncmp(left->type_name, right->type_name,
            RTPS_PATHNAME_LEN_MAX))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_DeadlineQosPolicy_is_equal(&left->deadline, &right->deadline))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_ReliabilityQosPolicy_is_equal(&left->reliability,
                                           &right->reliability))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_OwnershipQosPolicy_is_equal(&left->ownership, &right->ownership))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_DurabilityQosPolicy_is_equal(&left->durability, &right->durability))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_OwnershipStrengthQosPolicy_is_equal(&left->ownership_strength,
                                                 &right->ownership_strength))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_LivelinessQosPolicy_is_equal(&left->liveliness,
                                          &right->liveliness))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_LocatorSeq_is_equal(&left->unicast_locator,
                                 &right->unicast_locator))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_DestinationOrderQosPolicy_is_equal(&left->destination_order,
                                                &right->destination_order))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

DDS_Boolean
DDS_PublicationBuiltinTopicData_initialize(
                                struct DDS_PublicationBuiltinTopicData *self)
{
    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)
    
    return DDS_PublicationBuiltinTopicData_initialize_el(self) ?
                                DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

#ifndef RTI_CERT
/*ci
 * \brief Initialize a PublicationBuiltinTopicData structure without deallocation
 *
 * \param[in] self PublicationBuiltinTopicData structure to initialize
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DDS_PublicationBuiltinTopicData_finalize_no_dealloc(
                                DDS_DomainParticipant *const participant,
                                struct DDS_PublicationBuiltinTopicData *self)
{
    
    DDS_Boolean result = DDS_BOOLEAN_FALSE;

    if (self == NULL)
    {
        goto done;
    }

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
        if (!DDS_StringManager_delete_string(
                                participant->string_manager,self->type_name))
        {
            goto done;
        }
        self->type_name = NULL;
    }

   
    if (!DDS_LocatorSeq_set_length(&self->unicast_locator,0))
    {
        goto done;
    }

    result = DDS_BOOLEAN_TRUE;

done:

    return result;
    
}
#endif /* !RTI_CERT */

#ifndef RTI_CERT
/*ci
 * \brief Finalize a PublicationBuiltinTopicData structure
 *
 * \param[in] self PublicationBuiltinTopicData structure to finalize
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DDS_PublicationBuiltinTopicData_finalize(
                                struct DDS_PublicationBuiltinTopicData *self)
{
    return DDS_PublicationBuiltinTopicData_finalize_el(self) ?
                                    DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}
#endif /* !RTI_CERT */

#ifndef RTI_CERT
/*ci
 * \brief Initialize DDS_PublicationBuiltinTopicData with data from an existing structure
 *
 * \param[in] participant The participant that owns the structure
 * \param[inout] out DDS_PublicationBuiltinTopicData structure to initialize
 * \param[in] in Input data
 * 
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DDS_PublicationBuiltinTopicData_set_from(
                                    DDS_DomainParticipant *const participant,
                                    struct DDS_PublicationBuiltinTopicData *out,
                                    const struct DDS_PublicationBuiltinTopicData *in)
{
    
    DDS_Boolean result = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION((participant == NULL || out == NULL || in == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("participant",participant,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("out",out,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("in",in,RTI_TRUE);)

    out->key = in->key;
    out->participant_key = in->participant_key;
    
    out->topic_name = DDS_StringManager_assert_string(participant->string_manager,
                                                      in->topic_name);
    if (out->topic_name == NULL)
    {
        DDSC_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR, DDSC_LOG_STRING_OBJECT)
        goto done;
    }
    
    out->type_name = DDS_StringManager_assert_string(participant->string_manager,
                                                     in->type_name);
    if (out->type_name == NULL)
    {
         DDSC_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR, DDSC_LOG_STRING_OBJECT)
        goto done;
    }
    
    out->deadline = in->deadline;
    out->reliability = in->reliability;
    out->liveliness = in->liveliness;
    out->ownership = in->ownership;
    out->ownership_strength = in->ownership_strength;
    out->durability = in->durability;
    out->destination_order = in->destination_order;
    
    if (!DDS_LocatorSeq_copy(&out->unicast_locator, &in->unicast_locator))
    {
        goto done;
    }

    result = DDS_BOOLEAN_TRUE;

done:
    return result;
    
    
}
#endif

DDS_Boolean
DDS_PublicationBuiltinTopicData_copy(
                              struct DDS_PublicationBuiltinTopicData *out,
                              const struct DDS_PublicationBuiltinTopicData *in)
{
    OSAPI_PRECONDITION_ALWAYS((out == NULL) || (in == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("out",out,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("in",in,RTI_TRUE);)

    out->key = in->key;
    out->participant_key = in->participant_key;

#ifndef RTI_CERT
    if (DDS_String_replace(&out->topic_name, in->topic_name) == NULL)
    {
        return (DDS_BOOLEAN_FALSE);
    }

    if (DDS_String_replace(&out->type_name, in->type_name) == NULL)
    {
        return (DDS_BOOLEAN_FALSE);
    }
#else
    if (!REDA_String_copy(out->topic_name,RTPS_PATHNAME_LEN_MAX,in->topic_name))
    {
        return(DDS_BOOLEAN_FALSE);
    }

    if (!REDA_String_copy(out->type_name,RTPS_PATHNAME_LEN_MAX,in->type_name))
    {
        return(DDS_BOOLEAN_FALSE);
    }
#endif

    out->deadline = in->deadline;
    out->reliability = in->reliability;
    out->liveliness = in->liveliness;
    out->ownership = in->ownership;
    out->ownership_strength = in->ownership_strength;
    out->durability = in->durability;
    out->destination_order = in->destination_order;

    if (!DDS_LocatorSeq_copy(&out->unicast_locator, &in->unicast_locator))
    {
        return(DDS_BOOLEAN_FALSE);
    }

    return DDS_BOOLEAN_TRUE;
}

/******************************************************************************
 *                     DDS_SubscriptionBuiltinTopicData                       *
 ******************************************************************************/

/*ci
 * \brief Initialize a SubscriptionBuiltinTopicData sequence element
 *
 * \param[in] self SubscriptionBuiltinTopicData structure to initialize
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
DDS_SubscriptionBuiltinTopicData_initialize_el(
                                struct DDS_SubscriptionBuiltinTopicData *self)
{
    struct DDS_SubscriptionBuiltinTopicData init_val =
            DDS_SubscriptionBuiltinTopicData_INITIALIZER;

    *self = init_val;

    self->topic_name = DDS_String_alloc(RTPS_PATHNAME_LEN_MAX + 1);
    if (self->topic_name == NULL)
    {
        return RTI_FALSE;
    }

    self->type_name = DDS_String_alloc(RTPS_PATHNAME_LEN_MAX + 1);
    if (self->type_name == NULL)
    {
        return RTI_FALSE;
    }

    if (!DDS_LocatorSeq_set_maximum(&self->unicast_locator,
                                    RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX))
    {
        return RTI_FALSE;
    }

    if (!DDS_LocatorSeq_set_length(&self->unicast_locator, 0))
    {
        return RTI_FALSE;
    }

    if (!DDS_LocatorSeq_set_maximum(&self->multicast_locator,
                                    RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX))
    {
        return RTI_FALSE;
    }

    if (!DDS_LocatorSeq_set_length(&self->multicast_locator, 0))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize a SubscriptionBuiltinTopicData sequence element
 *
 * \param[in] self SubscriptionBuiltinTopicData structure to finalize
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
DDS_SubscriptionBuiltinTopicData_finalize_el(
                                struct DDS_SubscriptionBuiltinTopicData *self)
{
    if (self == NULL)
    {
        return (RTI_FALSE);
    }

    if (self->topic_name)
    {
        DDS_String_free(self->topic_name);
        self->topic_name = NULL;
    }

    if (self->type_name)
    {
        DDS_String_free(self->type_name);
        self->type_name = NULL;
    }

    if (!DDS_LocatorSeq_finalize(&self->unicast_locator))
    {
        return (RTI_FALSE);
    }

    if (!DDS_LocatorSeq_finalize(&self->multicast_locator))
    {
        return (RTI_FALSE);
    }

    return RTI_TRUE;
}
#endif /* !RTI_CERT */

#define T struct DDS_SubscriptionBuiltinTopicData
#define TSeq DDS_SubscriptionBuiltinTopicDataSeq
#define T_initialize DDS_SubscriptionBuiltinTopicData_initialize_el
#define T_finalize DDS_SubscriptionBuiltinTopicData_finalize_el
#define T_copy DDS_SubscriptionBuiltinTopicData_copy
#define REDA_SEQUENCE_USER_API
#include "reda/reda_sequence_defn.h"

DDS_Boolean
DDS_SubscriptionBuiltinTopicData_is_equal(
                        const struct DDS_SubscriptionBuiltinTopicData *left,
                        const struct DDS_SubscriptionBuiltinTopicData *right)
{
    OSAPI_PRECONDITION_ALWAYS((left == NULL) || (right == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    if (!DDS_BuiltinTopicKey_equals(&left->key, &right->key))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_BuiltinTopicKey_equals(&left->participant_key,
                                    &right->participant_key))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (DDS_String_ncmp(left->topic_name, right->topic_name,
            RTPS_PATHNAME_LEN_MAX))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (DDS_String_ncmp(left->type_name, right->type_name,
            RTPS_PATHNAME_LEN_MAX))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_DeadlineQosPolicy_is_equal(&left->deadline, &right->deadline))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_ReliabilityQosPolicy_is_equal(&left->reliability,
                                           &right->reliability))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_OwnershipQosPolicy_is_equal(&left->ownership, &right->ownership))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_LivelinessQosPolicy_is_equal(&left->liveliness,
                                          &right->liveliness))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_DurabilityQosPolicy_is_equal(&left->durability, &right->durability))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_DestinationOrderQosPolicy_is_equal(&left->destination_order,
                                                &right->destination_order))
    {
        return DDS_BOOLEAN_FALSE;
    }

    /* The DDS_PresentationQosPolicy is not fully supported and there is
     * no DDS_PresentationQosPolicy_is_equal() function. Thus, do a
     * direct comparison here.
     */
    if ((left->presentation.access_scope != right->presentation.access_scope)
        || (left->presentation.coherent_access != right->presentation.coherent_access)
        || (left->presentation.ordered_access != right->presentation.ordered_access))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_LocatorSeq_is_equal(&left->unicast_locator,
                                 &right->unicast_locator))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_LocatorSeq_is_equal(&left->multicast_locator,
                                 &right->multicast_locator))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;

}

DDS_Boolean
DDS_SubscriptionBuiltinTopicData_initialize(
                                struct DDS_SubscriptionBuiltinTopicData *self)
{
    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)
    
    return DDS_SubscriptionBuiltinTopicData_initialize_el(self) ?
                                        DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize a SubscriptionBuiltinTopicData structure without deallocation
 *
 * \param[in] self SubscriptionBuiltinTopicData structure to finalize
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DDS_SubscriptionBuiltinTopicData_finalize_no_dealloc(
                                DDS_DomainParticipant *const participant,
                                struct DDS_SubscriptionBuiltinTopicData *self)
{
    DDS_Boolean result = DDS_BOOLEAN_FALSE;

    if (self == NULL)
    {
        goto done;
    }

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
        if (!DDS_StringManager_delete_string(participant->string_manager,self->type_name))
        {
            goto done;
        }
        self->type_name = NULL;
    }

    if (!DDS_LocatorSeq_set_length(&self->unicast_locator,0))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_set_length(&self->multicast_locator,0))
    {
        goto done;
    }

    result = DDS_BOOLEAN_TRUE;

done:
    return result;
}
#endif

#ifndef RTI_CERT
/*ci
 * \brief Finalize a SubscriptionBuiltinTopicData structure
 *
 * \param[in] self SubscriptionBuiltinTopicData structure to finalize
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DDS_SubscriptionBuiltinTopicData_finalize(
                                struct DDS_SubscriptionBuiltinTopicData *self)
{
    return DDS_SubscriptionBuiltinTopicData_finalize_el(self) ?
                                    DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}
#endif /* !RTI_CERT */

#ifndef RTI_CERT
/*ci
 * \brief Initialize DDS_SubscriptionBuiltinTopicData with data from an existing structure
 *
 * \param[in] participant The participant that owns the structure
 * \param[inout] out DDS_SubscriptionBuiltinTopicData structure to initialize
 * \param[in] in Input data
 * 
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DDS_SubscriptionBuiltinTopicData_set_from(
                            DDS_DomainParticipant *const participant,
                            struct DDS_SubscriptionBuiltinTopicData *out,
                            const struct DDS_SubscriptionBuiltinTopicData *in)
{
    DDS_Boolean result = DDS_BOOLEAN_FALSE;

   OSAPI_PRECONDITION((participant == NULL || out == NULL || in == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("participant",participant,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("out",out,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("in",in,RTI_TRUE);)

    out->key = in->key;
    out->participant_key = in->participant_key;

    out->topic_name = DDS_StringManager_assert_string(
                        participant->string_manager,in->topic_name);
    
    if (out->topic_name == NULL)
    {
        DDSC_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR, DDSC_LOG_STRING_OBJECT)   
        goto done;
    }
    
    out->type_name = DDS_StringManager_assert_string(
                        participant->string_manager,in->type_name);
    
    if (out->type_name == NULL)
    {
        DDSC_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR, DDSC_LOG_STRING_OBJECT)
        goto done;
    }


    out->deadline = in->deadline;
    out->reliability = in->reliability;
    out->liveliness = in->liveliness;
    out->ownership = in->ownership;
    out->durability = in->durability;
    out->destination_order = in->destination_order;
    out->presentation = in->presentation;

    if (!DDS_LocatorSeq_copy(&out->unicast_locator, &in->unicast_locator))
    {
        goto done;
    }

    if (!DDS_LocatorSeq_copy(&out->multicast_locator, &in->multicast_locator))
    {
        goto done;
    }

    result = DDS_BOOLEAN_TRUE;

done:
    return result;
}
#endif

DDS_Boolean
DDS_SubscriptionBuiltinTopicData_copy(
                            struct DDS_SubscriptionBuiltinTopicData *out,
                            const struct DDS_SubscriptionBuiltinTopicData *in)
{
    OSAPI_PRECONDITION_ALWAYS((in == NULL) || (out == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("in",in,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("out",out,RTI_TRUE);)

    out->key = in->key;
    out->participant_key = in->participant_key;

#ifndef RTI_CERT
    if (DDS_String_replace(&out->topic_name, in->topic_name) == NULL)
    {
        return(DDS_BOOLEAN_FALSE);
    }

    if (DDS_String_replace(&out->type_name, in->type_name) == NULL)
    {
        return(DDS_BOOLEAN_FALSE);
    }
#else
    if (!REDA_String_copy(out->topic_name,RTPS_PATHNAME_LEN_MAX,in->topic_name))
    {
        return(DDS_BOOLEAN_FALSE);
    }

    if (!REDA_String_copy(out->type_name,RTPS_PATHNAME_LEN_MAX,in->type_name))
    {
        return(DDS_BOOLEAN_FALSE);
    }
#endif
    out->deadline = in->deadline;
    out->reliability = in->reliability;
    out->liveliness = in->liveliness;
    out->ownership = in->ownership;
    out->durability = in->durability;
    out->destination_order = in->destination_order;
    out->presentation = in->presentation;

    if (!DDS_LocatorSeq_copy(&out->unicast_locator, &in->unicast_locator))
    {
        return(DDS_BOOLEAN_FALSE);
    }

    if (!DDS_LocatorSeq_copy(&out->multicast_locator, &in->multicast_locator))
    {
        return(DDS_BOOLEAN_FALSE);
    }

    return DDS_BOOLEAN_TRUE;
}

#undef fail_with

/*ci @} */
