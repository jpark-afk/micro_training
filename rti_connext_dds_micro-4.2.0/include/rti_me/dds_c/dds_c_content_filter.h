/*
 * FILE: dds_c_content_filter.h - DDS Content Filter definitions
 *
 * Copyright (c) 2025-2025 Real-Time Innovations,Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*ce
 * \file
 * \brief DDS Content Filter definitions
 */

#ifndef dds_c_content_filter_h
#define dds_c_content_filter_h

#ifndef dds_c_infrastructure_h
#include "dds_c/dds_c_infrastructure.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*e \dref_CONTENT_FILTER_NAME_MAX_LENGTH
 */
#define DDS_CONTENT_FILTER_NAME_MAX_LENGTH 15

/*e \dref_CONTENT_FILTER_CLASS_NAME_MAX_LENGTH
 */
#define DDS_CONTENT_FILTER_CLASS_NAME_MAX_LENGTH 15

/*e \dref_CONTENT_FILTER_EXPRESSION_MAX_LENGTH
 */
#define DDS_CONTENT_FILTER_EXPRESSION_MAX_LENGTH 255

/*e \dref_CONTENT_FILTER_MAX_PARAMETERS
 */
#define DDS_CONTENT_FILTER_MAX_PARAMETERS 16

/* ================================================================= */
/*                        DDS_Filter                                 */
/* ================================================================= */

/*e \dref_FilterQosGroupDocs
 */

/*e \dref_FilterResourceLimits
 */
struct DDSCPPDllExport DDS_FilterResourceLimits
{
    /*e \dref_FilterResourceLimits_filter_class_max_length
     */
    DDS_Long filter_class_max_length;

    /*e \dref_FilterResourceLimits_filter_class_max_count
     */
    DDS_Long filter_class_max_count;

    /*e \dref_FilterResourceLimits_filter_expression_max_length
     */
    DDS_Long filter_expression_max_length;

    /*e \dref_FilterResourceLimits_filter_expression_max_count
     */
    DDS_Long filter_expression_max_count;

    /*e \dref_FilterResourceLimits_filter_parameter_max_count_per_expression
     */
    DDS_Long filter_parameter_max_count_per_expression;

    /*e \dref_FilterResourceLimits_filter_parameter_max_length
     */
    DDS_Long filter_parameter_max_length;

    /*e \dref_FilterResourceLimits_sql_predicate_max_count_per_expression
     */
    DDS_Long sql_predicate_max_count_per_expression;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_FilterResourceLimits)
};

#define DDS_FilterResourceLimits_INITIALIZER \
{\
    0, /* filter_class_max_length */      \
    0, /* filter_class_max_count */       \
    0, /* filter_expression_max_length */ \
    0, /* filter_expression_max_count */  \
    0, /* filter_parameter_max_count_per_expression */   \
    0, /* filter_parameter_max_length */  \
    0  /* sql_predicate_max_count_per_expression */     \
}

/*e \dref_FILTER_RESOURCE_LIMITS_DEFAULT
 */
#define DDS_FILTER_RESOURCE_LIMITS_DEFAULT \
{\
    7,               /* filter_class_max_length */      \
    1,               /* filter_class_max_count */       \
    63,              /* filter_expression_max_length */ \
    DDS_LENGTH_AUTO, /* filter_expression_max_count */  \
    4,               /* filter_parameter_max_count_per_expression */   \
    15,              /* filter_parameter_max_length */ \
    4                /* sql_predicate_max_count_per_expression */     \
}

/*e \dref_FilterQosPolicy
 */
struct DDSCPPDllExport DDS_FilterQosPolicy
{
    /*e \dref_FilterQosPolicy_name
     */
    RT_ComponentFactoryId_T name;

    /*e \dref_FilterQosPolicy_resource_limits
     */
    struct DDS_FilterResourceLimits resource_limits;

    /*e \dref_FilterQosPolicy_disable_writer_filtering
     */
    DDS_Boolean disable_writer_filtering;

    /*e \dref_FilterQosPolicy_disable_builtin_sql_filter
     */
    DDS_Boolean disable_builtin_sql_filter;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_FilterQosPolicy)
};

/*e \dref_FILTER_PLUGIN_FACTORY_DEFAULT_NAME
 */
#define DDS_FILTER_PLUGIN_FACTORY_DEFAULT_NAME "filter"

#define DDS_FILTER_PLUGIN_FACTORY_DEFAULT_ID \
{ \
    {{ DDS_FILTER_PLUGIN_FACTORY_DEFAULT_NAME }} \
}

#define DDS_FilterQosPolicy_INITIALIZER \
{\
    RT_ComponentFactoryId_INITIALIZER, \
    DDS_FilterResourceLimits_INITIALIZER, \
    DDS_BOOLEAN_FALSE, \
    DDS_BOOLEAN_FALSE \
}

#define DDS_FILTER_QOS_POLICY_DEFAULT \
{\
    DDS_FILTER_PLUGIN_FACTORY_DEFAULT_ID, \
    DDS_FILTER_RESOURCE_LIMITS_DEFAULT,   \
    DDS_BOOLEAN_FALSE,                    \
    DDS_BOOLEAN_FALSE                     \
}

/*ci
 * \brief Compare two DDS_FilterQosPolicy structures for equality
 *
 * \param[in] self  The left side of the comparison
 * \param[in] from  The right side of the comparison
 *
 * \return DDS_BOOLEAN_TRUE if left is equal to right, otherwise DDS_BOOLEAN_FALSE
 */
DDSCDllExport DDS_Boolean
DDS_FilterQosPolicy_is_equal(const struct DDS_FilterQosPolicy* self,
                                      const struct DDS_FilterQosPolicy* from);

/*ci
 * \brief Check that a DDS_FilterQosPolicy policy has legal values
 *
 * \param[in] self Structure to be validated
 *
 * \return DDS_BOOLEAN_TRUE if the content is valid, DDS_BOOLEAN_FALSE if not
 */
DDSCDllExport DDS_Boolean
DDS_FilterQosPolicy_is_consistent(const struct DDS_FilterQosPolicy* self);

/* ================================================================= */
/*                    DDS_ContentFilterQosPolicy                     */
/* ================================================================= */

/*e \dref_ContentFilterQosGroupDocs
 */

/*e \dref_ContentFilterQosPolicy
 */
struct DDSCPPDllExport DDS_ContentFilterQosPolicy
{
    /*e \dref_ContentFilterQosPolicy_filter_name
     */
    DDS_String filter_name;

    /*e \dref_ContentFilterQosPolicy_filter_class_name
     */
    DDS_String filter_class_name;

    /*e \dref_ContentFilterQosPolicy_filter_expression
     */
    DDS_String filter_expression;

    /*e \dref_ContentFilterQosPolicy_expression_parameters
     */
    struct DDS_StringSeq expression_parameters;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_ContentFilterQosPolicy)
};

#define DDS_CONTENT_FILTER_QOS_POLICY_DEFAULT \
{\
    NULL,\
    NULL,\
    NULL,\
    REDA_StringSeq_INITIALIZER\
}

#ifndef RTI_CERT
/*ci
 *
 * \brief Finalize the DDS_ContentFilterQosPolicy
 *
 * \param[in]  policy DDS_ContentFilterQosPolicy to be finalized
 *
 * \return RTI_TRUE if DDS_ContentFilterQosPolicy instance is
 *         successfully finalized, RTI_FALSE otherwise.
 */
DDSCDllExport DDS_ReturnCode_t
DDS_ContentFilterQosPolicy_finalize(struct DDS_ContentFilterQosPolicy *policy);
#endif /* !RTI_CERT */

/*ci
 * \brief Initialize the DDS_ContentFilterQosPolicy
 *
 * \param[in]  policy DDS_ContentFilterQosPolicy to be initialized
 *
 * \return RTI_TRUE if the DDS_ContentFilterQosPolicy instance is initialized
 *         successfully, RTI_FALSE otherwise
 */
MUST_CHECK_RETURN DDSCDllExport DDS_ReturnCode_t
DDS_ContentFilterQosPolicy_initialize(struct DDS_ContentFilterQosPolicy *policy);

/*ci
 * \brief Make a copy of the DDS_ContentFilterQosPolicy instance.
 *
 * \param[in]   from_policy Pointer to DDS_ContentFilterQosPolicy instance to be copied
 * \param[out]  to_policy Pointer to DDS_ContentFilterQosPolicy which will be a copy
 *              of the from_policy.
 *
 * \return RTI_TRUE if the DDS_ContentFilterQosPolicy instance is copied successfully,
 *         RTI_FALSE otherwise
 */
MUST_CHECK_RETURN DDSCDllExport DDS_ReturnCode_t
DDS_ContentFilterQosPolicy_copy(struct DDS_ContentFilterQosPolicy *to_policy,
                          const struct DDS_ContentFilterQosPolicy *from_policy);

/*ci
 * \brief Check if the DDS_ContentFilterQosPolicy has been populated consistently.
 *
 * \brief A DDS_ContentFilterQosPolicy is consistent if is either entirely
 *        populated or not populated at all. If it is populated, it must
 *        fit within the specified resource limits.
 *
 * \param[in]  policy DDS_ContentFilterQosPolicy that has to be checked for consistency
 * \param[in]  resource_limits Resource limits to be used for the consistency check
 *
 * \return RTI_TRUE if the policy is consistent, RTI_FALSE otherwise
 */
MUST_CHECK_RETURN DDSCDllExport RTI_BOOL
DDS_ContentFilterQosPolicy_is_consistent(
        const struct DDS_ContentFilterQosPolicy *policy,
        struct DDS_FilterResourceLimits *resource_limits);
/*ci
 * \brief Check if two DDS_ContentFilterQosPolicy instances are equal
 *
 * \param[in]  left  Left side of comparison
 * \param[in]  right Right side of comparison
 * \return RTI_TRUE if left = right, RTI_FALSE otherwise
 */
MUST_CHECK_RETURN DDSCDllExport RTI_BOOL
DDS_ContentFilterQosPolicy_is_equal(const struct DDS_ContentFilterQosPolicy *left,
                                    const struct DDS_ContentFilterQosPolicy *right);

/* ================================================================= */
/*                   DDS_ContentFilterProperty                       */
/* ================================================================= */

/*e \dref_ContentFilterProperty
 */
struct DDSCPPDllExport DDS_ContentFilterProperty
{
    /*e \dref_ContentFilterProperty_content_filtered_topic_name
     */
    DDS_String content_filtered_topic_name;

    /*e \dref_ContentFilterProperty_related_topic_name
     */
    DDS_String related_topic_name;

    /*e \dref_ContentFilterProperty_filter_class_name
     */
    DDS_String filter_class_name;

    /*e \dref_ContentFilterProperty_filter_expression
     */
    DDS_String filter_expression;

    /*e \dref_ContentFilterProperty_expression_parameters
     */
    struct DDS_StringSeq expression_parameters;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_ContentFilterProperty)
};

#define DDS_CONTENT_FILTER_PROPERTY_DEFAULT \
{\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    REDA_StringSeq_INITIALIZER\
}

#ifndef RTI_CERT
/*ci
 *
 * \brief Finalize the DDS_ContentFilterProperty
 *
 * \param[in]  policy DDS_ContentFilterProperty to be finalized
 *
 * \return RTI_TRUE if DDS_ContentFilterProperty instance is
 *         successfully finalized, RTI_FALSE otherwise.
 */
DDSCDllExport DDS_Boolean
DDS_ContentFilterProperty_finalize(struct DDS_ContentFilterProperty *policy);
#endif /* !RTI_CERT */

/*ci
 * \brief Initialize the DDS_ContentFilterProperty
 *
 * \param[in]  policy DDS_ContentFilterProperty to be initialized
 *
 * \return RTI_TRUE if the DDS_ContentFilterProperty instance is initialized
 *         successfully, RTI_FALSE otherwise
 */
MUST_CHECK_RETURN DDSCDllExport DDS_Boolean
DDS_ContentFilterProperty_initialize(struct DDS_ContentFilterProperty *policy);

/*ci
 * \brief Make a copy of the DDS_ContentFilterProperty instance.
 *
 * \param[in]   from_policy Pointer to DDS_ContentFilterProperty instance to be copied
 * \param[out]  to_policy Pointer to DDS_ContentFilterProperty which will be a copy
 *              of the from_policy.
 *
 * \return RTI_TRUE if the DDS_ContentFilterProperty instance is copied successfully,
 *         RTI_FALSE otherwise
 */
MUST_CHECK_RETURN DDSCDllExport DDS_Boolean
DDS_ContentFilterProperty_copy(struct DDS_ContentFilterProperty *out,
                            const struct DDS_ContentFilterProperty *in);

/*ci
 * \brief Check if two DDS_ContentFilterProperty instances are equal
 *
 * \param[in]  left  Left side of comparison
 * \param[in]  right Right side of comparison
 * \return RTI_TRUE if left = right, RTI_FALSE otherwise
 */
MUST_CHECK_RETURN DDSCDllExport RTI_BOOL
DDS_ContentFilterProperty_is_equal(const struct DDS_ContentFilterProperty *left,
                                    const struct DDS_ContentFilterProperty *right);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* dds_c_content_filter_h */
