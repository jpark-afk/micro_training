/*
 * FILE: dds_c_property_qos.h - DDS Property interface
 *
 * (c) Copyright, Real-Time Innovations, 2012-2025.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 */
#ifndef dds_c_property_qos_h
#define dds_c_property_qos_h
/* This file should not be included directly. It is included by the
 * top-level dds_c_infrastructure.h file.
 */
/*ce
 * \file
 * \brief DDS Property interface
 */
/*e @addtogroup DDSInfrastructureModule Infrastructure Module

    @brief Defines the \dds infrastructure package
*/
/* ================================================================= */
/*                            DDS Properties                         */
/* ================================================================= */
#define DDS_PropertySeq                 CDR_PropertySeq
#define DDS_Property                    CDR_Property
#define DDS_Property_t                  DDS_Property
#define DDS_Property_INITIALIZER        CDR_Property_INITIALIZER
#define DDS_PropertySeq_initialize      CDR_PropertySeq_initialize
#define DDS_PropertySeq_is_equal        CDR_PropertySeq_is_equal
#define DDS_PropertySeq_set_length      CDR_PropertySeq_set_length
#define DDS_PropertySeq_get_length      CDR_PropertySeq_get_length
#define DDS_PropertySeq_get_reference   CDR_PropertySeq_get_reference
#define DDS_PropertySeq_ensure_length   CDR_PropertySeq_ensure_length
#define DDS_PropertySeq_set_maximum     CDR_PropertySeq_set_maximum
#define DDS_PropertySeq_get_maximum     CDR_PropertySeq_get_maximum
#define DDS_PropertySeq_assert_property CDR_PropertySeq_assert_property
#define DDS_PropertySeq_lookup_property CDR_PropertySeq_lookup_property
#define DDS_PropertySeq_lookup_property_w_prefix \
                                        CDR_PropertySeq_lookup_property_w_prefix
#define DDS_PropertySeq_copy            CDR_PropertySeq_copy
#define DDS_PropertySeq_has_ownership   CDR_PropertySeq_has_ownership
#define DDS_PropertySeq_loan_discontiguous \
                                        CDR_PropertySeq_loan_contiguous
#define DDS_PropertySeq_has_discontiguous_buffer \
                                        CDR_PropertySeq_has_discontiguous_buffer
#define DDS_PropertySeq_get_contiguous_buffer \
                                        CDR_PropertySeq_get_contiguous_buffer
#define DDS_PropertySeq_set_contiguous_buffer \
                                        CDR_PropertySeq_set_contiguous_buffer
#define DDS_PropertySeq_unloan          CDR_PropertySeq_unloan
#define DDS_PropertySeq_to_array        CDR_PropertySeq_to_array
#define DDS_PropertySeq_from_array      CDR_PropertySeq_from_array
#ifndef RTI_CERT
#define DDS_PropertySeq_finalize        CDR_PropertySeq_finalize
#endif

/* ================================================================= */
/*                       Property Qos Policy                         */
/* ================================================================= */

/*e \dref_PropertyQosGroupDocs
 */

#ifdef DOXYGEN_DOCUMENTATION_ONLY
/*e \dref_Property_t
 */
struct DDS_Property_t
{
   /*e \dref_Property_t_name
    */
    DDS_String name;

   /*e \dref_Property_t_value
    */
    DDS_String value;

   /*e \dref_Property_t_propagate
    */
    DDS_Boolean propagate;
};

/*e \dref_PropertySeq
 */
 struct DDS_PropertySeq {};
#endif

/*e \dref_PropertyQosPolicy
 */
struct DDSCPPDllExport DDS_PropertyQosPolicy
{
    /*e \dref_PropertyQosPolicy_value
     */
    struct DDS_PropertySeq value;
#ifdef RTI_CPP
public:
        DDS_PropertyQosPolicy();
        ~DDS_PropertyQosPolicy();
#endif
};

#define DDS_PROPERTY_QOS_POLICY_DEFAULT \
{\
    DDS_SEQUENCE_INITIALIZER \
}

DDSC_QOS_POLICY_METHODS_DECL(DDS_PropertyQosPolicy)

DDSCDllExport DDS_ReturnCode_t
DDS_PropertyQosPolicy_initialize(struct DDS_PropertyQosPolicy *policy);


#ifndef RTI_CERT
DDSCDllExport extern DDS_ReturnCode_t
DDS_PropertyQosPolicy_finalize(struct DDS_PropertyQosPolicy *policy);
#endif /* !RTI_CERT */

/*e \dref_PropertyQosPolicyHelper_assert_property
 */
DDSCDllExport DDS_ReturnCode_t
DDS_PropertyQosPolicyHelper_assert_property(
        struct DDS_PropertyQosPolicy *policy,
        const char *name,const char *value,
        DDS_Boolean propagate);

/*e \dref_PropertyQosPolicyHelper_add_property
 */
DDSCDllExport DDS_ReturnCode_t
DDS_PropertyQosPolicyHelper_add_property(
        struct DDS_PropertyQosPolicy *policy,
        const char *name, const char *value,
        DDS_Boolean propagate);

/*e \dref_PropertyQosPolicyHelper_lookup_property
 */
DDSCDllExport struct DDS_Property_t*
DDS_PropertyQosPolicyHelper_lookup_property(
        const struct DDS_PropertyQosPolicy *policy,
        const char *name);

/*e \dref_PropertyQosPolicyHelper_remove_property
 */
DDSCDllExport DDS_ReturnCode_t
DDS_PropertyQosPolicyHelper_remove_property (
        struct DDS_PropertyQosPolicy *policy,
        const char *name);

DDSCDllExport DDS_ReturnCode_t
DDS_PropertyQosPolicy_copy(struct DDS_PropertyQosPolicy *to_policy,
                            const struct DDS_PropertyQosPolicy *from_policy);

DDSCDllExport RTI_BOOL
DDS_PropertyQosPolicy_is_consistent(const struct DDS_PropertyQosPolicy *policy);

DDSCDllExport RTI_BOOL
DDS_PropertyQosPolicy_is_equal(const struct DDS_PropertyQosPolicy *left,
                                const struct DDS_PropertyQosPolicy *right);


DDSCDllExport DDS_Boolean
DDS_PropertyQosPolicy_is_valid(const struct DDS_PropertyQosPolicy *policy,
                               DDS_EntityKind_t entity_kind);

DDSCDllExport DDS_Boolean
DDS_PropertyQosPolicy_immutable_is_equal(const struct DDS_PropertyQosPolicy *left,
                                         const struct DDS_PropertyQosPolicy *right);
#endif
