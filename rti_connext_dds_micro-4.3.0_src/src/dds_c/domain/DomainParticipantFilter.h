/*
 * FILE: DomainParticipantFilter.h - DomainParticipant filter related functions
 *
 * Copyright (c) 2025-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef DomainParticipantFilter_h
#define DomainParticipantFilter_h

#include "dds_c/dds_c_content_filter.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define DDS_DomainParticipant_is_filtering_enabled(dp_) \
            ((dp_)->filter_plugin != NULL)

/*ci
 * \brief Shallow copy a content filter qos policy using a Participant's filter plugin
 *
 * \param[in] dp The participant to use to copy the qos policy.
 * \param[out] out The qos policy to copy to.
 * \param[in] in The qos policy to copy from.
 *
 * \return DDS_RETCODE_OK if the copy was successful or if the participant does not
 *         have a filter plugin. Otherwise, an error code is returned.
 */
extern DDS_ReturnCode_t
DDS_DomainParticipant_content_filter_qos_shallow_copy(
        DDS_DomainParticipant *dp,
        struct DDS_ContentFilterQosPolicy *out,
        const struct DDS_ContentFilterQosPolicy *in);

/*ci
 * \brief Return the resources used by a previous shallow copy of a content filter qos policy.
 *
 * \param[in] dp The participant to use to finalize the qos policy.
 * \param[in] policy The qos policy to finalize.
 */
extern void
DDS_DomainParticipant_content_filter_qos_finalize_shallow_copy(
        DDS_DomainParticipant *dp,
        struct DDS_ContentFilterQosPolicy *policy);

/*ci
 * \brief Shallow copy a content filter property using a Participant's filter plugin
 *
 * \param[in] dp The participant to use to copy the property.
 * \param[out] out The property to copy to.
 * \param[in] in The property to copy from.
 *
 * \return DDS_BOOLEAN_TRUE if the copy was successful or if the participant does not
 *         have a filter plugin. Otherwise, DDS_BOOLEAN_FALSE.
 */
extern DDS_Boolean
DDS_DomainParticipant_filter_property_shallow_copy(
        DDS_DomainParticipant *dp,
        struct DDS_ContentFilterProperty *out,
        const struct DDS_ContentFilterProperty *in);

/*ci
 * \brief Return the resources used by a previous shallow copy of a content filter property
 *
 * \param[in] dp The participant to use to finalize the property.
 * \param[in] prop The property to finalize.
 */
extern void
DDS_DomainParticipant_filter_property_finalize_shallow_copy(
        DDS_DomainParticipant *dp,
        struct DDS_ContentFilterProperty *prop);

/*ci
 * \brief Get tha maximum serialized size of a content filter property using a
 *        Participant's filter plugin
 *
 * \param[in] dp The participant to use to get the size.
 * \param[in] size The size of the serialized sample before the filter property is added
 *
 * \return The maximum serialized size of the content filter property.
 *         If the participant does not have a filter plugin, 0 is returned.
 */
extern RTI_UINT32
DDS_DomainParticipant_filter_property_get_max_serialized_size(
        DDS_DomainParticipant *dp,
        RTI_UINT32 size);

/*ci
 * \brief Deserialize a content filter property using a Participant's filter plugin
 *
 * \details If the participant does not have a filter plugin, the function will not
 *          deserialize the property and will return DDS_BOOLEAN_TRUE. The caller is
 *          responsible for skipping over the property in the stream.
 *
 * \param[in] dp The participant to use to deserialize the property.
 * \param[in] stream The stream to deserialize from.
 * \param[out] prop The property to deserialize into.
 *
 * \return DDS_BOOLEAN_TRUE if the deserialization was successful or if the participant does not
 *         have a filter plugin. Otherwise, DDS_BOOLEAN_FALSE.
 */
extern DDS_Boolean
DDS_DomainParticipant_deserialize_content_filter_property(
        DDS_DomainParticipant *dp,
        struct CDR_Stream_t *stream,
        struct DDS_ContentFilterProperty *prop);

/*ci
 * \brief Create a filter plugin for a participant using its QoS
 *
 * \details If the participant is configured to use the default filter plugin,
 *          and the filter library has not be registered, the function will
 *          return DDS_BOOLEAN_TRUE and not create a filter plugin. If the
 *          user has configured the participant to use a custom filter plugin,
 *          then the function will return DDS_BOOLEAN_TRUE if the filter plugin
 *          only if the filter plugin was created successfully.
 *
 * \param[in] participant The participant to create the filter plugin for.
 *
 * \return DDS_BOOLEAN_TRUE if successful. Otherwise, DDS_BOOLEAN_FALSE.
 */
extern DDS_Boolean
DDS_DomainParticipant_create_filter_plugin(DDS_DomainParticipant *participant);

#ifndef RTI_CERT
/*ci
 * \brief Finalize a domain participant's filter plugin
 *
 * \param[in] participant The participant to finalize the filter plugin for.
 *
 * \return DDS_BOOLEAN_TRUE if successful or if the participant does not
 *         have a filter plugin. Otherwise, DDS_BOOLEAN_FALSE.
 */
extern DDS_Boolean
DDS_DomainParticipant_finalize_filter_plugin(DDS_DomainParticipant *participant);
#endif /* !RTI_CERT */

#ifdef __cplusplus
}
#endif

#endif /* DomainParticipantFilter_h */
