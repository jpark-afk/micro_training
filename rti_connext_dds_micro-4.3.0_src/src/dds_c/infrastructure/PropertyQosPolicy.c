/*
 * FILE: PropertyQosPolicy.c - PropertyQosPolicy Helper Functions
 *
 * (c) Copyright, Real-Time Innovations, 2017-2026
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
 * 04Oct2017,hc Created.
 */

/*ci
 * \brief PropertyQosPolicy.c
 *
 * \details
 * This file implements helper functions for DDS PropertyQosPolicy
 */
#ifndef dds_c_common_impl
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_sequence_h
#include "dds_c/dds_c_sequence.h"
#endif
#include "dds_c/dds_c_trust.h"

const char *DDS_XTYPES_COMPLIANCE_MASK_PROPERTY = "dds.xtypes.compliance_mask";
const char *DDS_DATAWRITER_PROTOCOL_ZCV2_VERSION_PROPERTY = "dds.datawriter.protocol.zcv2.version";
const DDS_UnsignedShort DDS_DATAWRITER_PROTOCOL_ZCV2_VERSION_DEFAULT = ((DDS_UnsignedShort)-1);

#define DDS_PROPERTY_QOS_SUPPORTED_MAX_LENGTH 8


DDS_ReturnCode_t
DDS_PropertyQosPolicy_initialize(struct DDS_PropertyQosPolicy *policy)
{
    struct DDS_PropertyQosPolicy d = DDS_PROPERTY_QOS_POLICY_DEFAULT;

    *policy = d;

    return DDS_RETCODE_OK;
}

#ifndef RTI_CERT
/* ci
 *
 * \brief
 * Finalize the DDS_PropertyQosPolicy
 *
 * \details
 *  This function helps to free up the resources
 *  allocated for the DDS_PropertyQosPolicy instance.
 *
 * \param[in]  policy DDS_PropertyQosPolicy which needs
 *             to be finalized.
 *
 * \return RTI_TRUE if DDS_PropertyQosPolicy instance is
 *         successfully finalized, RTI_FALSE otherwise.
 */

DDS_ReturnCode_t
DDS_PropertyQosPolicy_finalize(struct DDS_PropertyQosPolicy *policy)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION((policy == NULL), return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("policy", policy, RTI_FALSE);)

    if (!CDR_PropertySeq_finalize(&policy->value))
    {
        goto done;
    }
    retval = DDS_RETCODE_OK;

done:
    return retval;
}
#endif /* !RTI_CERT */

/* ci
 *
 * \brief
 * Make a copy of the DDS_PropertyQosPolicy instance.
 *
 * \details
 *  This function helps to make a copy of a DDS_PropertyQosPolicy instance.
 *
 * \param[in]   from_policy Pointer to DDS_PropertyQosPolicy instance whose copy
 *                          needs to be made
 * \param[out]  to_policy Pointer to DDS_PropertyQosPolicy which will be a copy
 *                        of the from_policy.
 *
 * \return RTI_TRUE if the DDS_PropertyQosPolicy instance is copied successfully,
 *         RTI_FALSE otherwise
 */
DDS_ReturnCode_t
DDS_PropertyQosPolicy_copy(struct DDS_PropertyQosPolicy *to_policy,
                           const struct DDS_PropertyQosPolicy *from_policy)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION(to_policy == NULL || from_policy == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("to_policy",to_policy,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("from_policy",from_policy,RTI_FALSE);)

    if (!CDR_PropertySeq_copy(&to_policy->value, &from_policy->value))
    {
        goto done;
    }

    retval = DDS_RETCODE_OK;

done:
    return retval;
}

/* ci
 *
 * \brief
 * Check if the PropertyQosPolicy has been populated consistently.
 *
 * \details
 *  This function checks that a DDS_PropertyQosPolicy has been consistently
 *  populated by a user. A DDS_Property contained in the policy is considered
 *  consistent if both its name and value are set to non-NULL values, while a
 *  DDS_BinaryProperty must have a non-NULL name in order to be consistent
 *
 *  PropertyQosPolicy is generic, hence the plugins must perform plugin specific
 *  consistency checks themselves.
 *
 * \param[in]  policy DDS_PropertyQosPolicy that has to be checked for
 *                    consistency
 *
 * \return RTI_TRUE if the policy is consistent, RTI_FALSE otherwise
 *
 */
RTI_BOOL
DDS_PropertyQosPolicy_is_consistent(const struct DDS_PropertyQosPolicy *policy)
{
    RTI_BOOL retval = RTI_FALSE;

    OSAPI_PRECONDITION(policy == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("policy",policy,RTI_FALSE);)

    if (!CDR_PropertySeq_is_consistent(&policy->value))
    {
        goto done;
    }

    retval = RTI_TRUE;

done:
    return retval;
}


DDS_Boolean
DDS_PropertyQosPolicy_immutable_is_equal(const struct DDS_PropertyQosPolicy *left,
                               const struct DDS_PropertyQosPolicy *right)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    RTI_INT32 left_length, right_length, i;
    struct DDS_Property_t *left_property = NULL;

    OSAPI_PRECONDITION(left == NULL || right == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    left_length = CDR_PropertySeq_get_length(&left->value);
    right_length = CDR_PropertySeq_get_length(&right->value);

    if (left_length != right_length)
    {
        goto done;
    }

    for (i = 0; i < left_length; i++)
    {
        left_property = CDR_PropertySeq_get_reference(&left->value, i);
        if (left_property == NULL)
        {
            goto done;
        }

        if (left_property->name == NULL)
        {
            goto done;
        }
        /* Passphrase is a mutable property*/
        if (!REDA_String_ncompare(left_property->name, DDS_TRUST_RTPS_PSK_PASSPHRASE_PROPERTY, CDR_PROPERTY_NAME_MAX_LEN))
        {
            continue;
        }

        if ((CDR_Property_compare(left_property,
            CDR_PropertySeq_get_reference(&right->value, i))) != 0)
        {
            goto done;
        }

    }
    retval = DDS_BOOLEAN_TRUE;

done:
    return retval;
}



RTI_BOOL
DDS_PropertyQosPolicy_is_equal(const struct DDS_PropertyQosPolicy *left,
                               const struct DDS_PropertyQosPolicy *right)
{
    RTI_BOOL retval = RTI_FALSE;

    if (!DDS_PropertySeq_is_equal(&left->value, &right->value))
    {
        goto done;
    }

    retval = RTI_TRUE;

done:
    return retval;
}

/*ci \brief assert a property without precondition checks
 *
 * \param[in] policy - The policy to assert the property to.
 * \param[in] name - The name of the policy to assert.
 * \param[in] value - The value of the policy to assert.
 * \param[in] propagate - Whether the property should be propagated or not.
 *
 * \return One of the Standard Return Codes, DDS_RETCODE_OUT_OF_RESOURCES
 */
RTI_PRIVATE DDS_ReturnCode_t
DDS_PropertyQosPolicyHelper_assert_no_precond(
        struct DDS_PropertyQosPolicy *policy,
        const char *name,
        const char *value,
        DDS_Boolean propagate)
{
    if (!CDR_PropertySeq_assert_property(&policy->value,name,value,propagate))
    {
        return DDS_RETCODE_OUT_OF_RESOURCES;
    }

    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t
DDS_PropertyQosPolicyHelper_assert_property(
        struct DDS_PropertyQosPolicy *policy,
        const char *name,
        const char *value,
        DDS_Boolean propagate)
{
    OSAPI_PRECONDITION_ALWAYS(
            (policy == NULL) ||
            (name == NULL) ||
            (value == NULL) ||
            (propagate != DDS_BOOLEAN_FALSE),
            return DDS_RETCODE_BAD_PARAMETER,
            OSAPI_Log_entry_add_pointer("policy", policy, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("name",name, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("value", value, RTI_FALSE);
            OSAPI_Log_entry_add_int("propagate", propagate, RTI_TRUE);)

    return DDS_PropertyQosPolicyHelper_assert_no_precond(
                            policy,name,value,propagate);
}

DDS_ReturnCode_t
DDS_PropertyQosPolicyHelper_add_property(
        struct DDS_PropertyQosPolicy *policy,
        const char *name,
        const char *value,
        DDS_Boolean propagate)
{
    struct DDS_Property_t *search = NULL;

    OSAPI_PRECONDITION_ALWAYS(
            (policy == NULL) ||
            (name == NULL) ||
            (value == NULL) ||
            (propagate != DDS_BOOLEAN_FALSE),
            return DDS_RETCODE_BAD_PARAMETER,
            OSAPI_Log_entry_add_pointer("policy", policy, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("name",name, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("value", value, RTI_FALSE);
            OSAPI_Log_entry_add_int("propagate", propagate, RTI_TRUE);)

    search = DDS_PropertyQosPolicyHelper_lookup_property(policy,name);

    if (search != NULL)
    {
        return DDS_RETCODE_PRECONDITION_NOT_MET;
    }

    return DDS_PropertyQosPolicyHelper_assert_no_precond(
                                        policy,name,value,propagate);
}

struct DDS_Property_t*
DDS_PropertyQosPolicyHelper_lookup_property(
        const struct DDS_PropertyQosPolicy *policy,
        const char *name)
{
    OSAPI_PRECONDITION_ALWAYS(
            (policy == NULL) ||
            (name == NULL),
            return NULL,
            OSAPI_Log_entry_add_pointer("policy", policy, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("name",name, RTI_TRUE);)

    return CDR_PropertySeq_lookup_property(&policy->value,name);
}

DDS_ReturnCode_t
DDS_PropertyQosPolicyHelper_remove_property (
        struct DDS_PropertyQosPolicy *policy,
        const char *name)
{
    struct DDS_Property_t *search = NULL;
    RTI_INT32 i,length;
    DDS_ReturnCode_t retval = DDS_RETCODE_PRECONDITION_NOT_MET;

    OSAPI_PRECONDITION_ALWAYS(
            (policy == NULL) ||
            (name == NULL),
            return DDS_RETCODE_BAD_PARAMETER,
            OSAPI_Log_entry_add_pointer("policy", policy, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("name",name, RTI_TRUE);)

    length = CDR_PropertySeq_get_length(&policy->value);

    for (i = 0; i < length; i++)
    {
        search = CDR_PropertySeq_get_reference(&policy->value, i);
        if (search == NULL)
        {
            goto done;
        }

        if (search->name == NULL)
        {
            goto done;
        }

        if (!REDA_String_ncompare(search->name, name,CDR_PROPERTY_NAME_MAX_LEN))
        {
            break;
        }
        search = NULL;
    }

    if (search != NULL)
    {
        DDS_String_free(search->name);
        DDS_String_free(search->value);
        search->name = NULL;
        search->value = NULL;

        for (; i < length - 1; i++)
        {
            struct CDR_Property *policy_ref =
                            CDR_PropertySeq_get_reference(&policy->value, i);
            search = CDR_PropertySeq_get_reference(&policy->value, i+1);

            if ((policy_ref == NULL) || (search == NULL))
            {
                return DDS_RETCODE_ERROR;
            }

            *policy_ref = *search;
            search->name = NULL;
            search->value = NULL;
        }

        if (!CDR_PropertySeq_set_length(&policy->value,length-1))
        {
            return DDS_RETCODE_ERROR;
        }

        retval = DDS_RETCODE_OK;
    }

done:

    return retval;
}

RTI_PRIVATE DDS_Boolean
DDS_PropertyQosPolicy_is_supported(DDS_String name)
{
    RTI_INT32 i;
    RTI_INT32 count = 0;
    const char *DDS_PropertyQosSupported [] =
    {
        DDS_XTYPES_COMPLIANCE_MASK_PROPERTY,
        DDS_TRUST_RTPS_PSK_SYMMETRIC_CIPHER_PROPERTY,
        DDS_TRUST_RTPS_PSK_PASSPHRASE_PROPERTY,
        DDS_TRUST_RTPS_PSK_PROTECTION_KIND_PROPERTY,
        DDS_TRUST_MAX_BLOCKS_PER_SESSION_PROPERTY,
        DDS_TRUST_FILES_POLL_INTERVAL_PROPERTY,
        DDS_TRUST_RTPS_PSK_PASSTRACKER_ARG_PROPERTY,
        DDS_DATAWRITER_PROTOCOL_ZCV2_VERSION_PROPERTY,
    };

    count = sizeof(DDS_PropertyQosSupported) / sizeof(DDS_PropertyQosSupported[0]);

    for (i = 0; i < count; i++)
    {
        if (!REDA_String_ncompare(DDS_PropertyQosSupported[i], name, CDR_PROPERTY_NAME_MAX_LEN))
        {
            return DDS_BOOLEAN_TRUE;
        }
    }

    return DDS_BOOLEAN_FALSE;
}

DDS_Boolean
DDS_PropertyQosPolicy_is_valid(const struct DDS_PropertyQosPolicy *policy,
                               DDS_EntityKind_t entity_kind)
{
    struct DDS_Property *search = NULL;
#if DDS_XTYPES_IS_ENABLED
    DDS_UnsignedLong xtypes_compliance_mask = 0;
#endif
    RTI_INT32 i, length;

    if (!DDS_PropertyQosPolicy_is_consistent(policy))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (DDS_PropertySeq_get_length(&policy->value) == 0)
    {
        return DDS_BOOLEAN_TRUE;
    }

    if ((entity_kind == DDS_UNKNOWN_ENTITY_KIND) ||
        (entity_kind == DDS_PUBLISHER_ENTITY_KIND) ||
        (entity_kind == DDS_SUBSCRIBER_ENTITY_KIND) ||
        (entity_kind == DDS_TOPIC_ENTITY_KIND) ||
        (entity_kind == DDS_DATAREADER_ENTITY_KIND))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (DDS_PropertySeq_get_length(&policy->value) > DDS_PROPERTY_QOS_SUPPORTED_MAX_LENGTH)
    {
        return DDS_BOOLEAN_FALSE;
    }

    length = CDR_PropertySeq_get_length(&policy->value);

    for (i = 0; i < length; i++)
    {
        search = CDR_PropertySeq_get_reference(&policy->value, i);
        if (search == NULL)
        {
            return DDS_BOOLEAN_FALSE;
        }

        if (search->name == NULL)
        {
            return DDS_BOOLEAN_FALSE;
        }

        if (search->value == NULL)
        {
            return DDS_BOOLEAN_FALSE;
        }

        if (!DDS_PropertyQosPolicy_is_supported(search->name))
        {
            return DDS_BOOLEAN_FALSE;
        }
#if DDS_XTYPES_IS_ENABLED
        if (!REDA_String_ncompare(search->name, DDS_XTYPES_COMPLIANCE_MASK_PROPERTY,
                                   CDR_PROPERTY_NAME_MAX_LEN))
        {
            /* special handling of the xtypes property as its not validated as soon the Domain Participant is created. */
            if (!OSAPI_String_parse_unsigned_long(search->value,
                                                &xtypes_compliance_mask))
            {
                return DDS_BOOLEAN_FALSE;
            }

            if ((xtypes_compliance_mask
                & (DDS_UnsignedLong)~NDDS_CONFIG_XTYPES_ENCAPSULATION_OPTIONS_WITH_PADDING_BIT) != 0)
            {
                return DDS_BOOLEAN_FALSE;
            }
        }
#endif
    }
    return DDS_BOOLEAN_TRUE;
}
