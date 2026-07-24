/*
 * FILE: DataReaderFilter.h - DataReader filter related functions declarations
 *
 * Copyright (c) 2025-2025 Real-Time Innovations,Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*ci
 * \brief DataReader filter related functions declarations
 */

#ifndef DataReaderFilter_h
#define DataReaderFilter_h

#include "dds_c/dds_c_filter_plugin.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define DDS_DataReader_is_filtering_enabled(dr_) ((dr_)->config->filter_plugin != NULL)

/*ci
 * \brief Replace the content filter QoS policy of a DataReader with a new one.
 *
 * \param[in]  dr The DataReader to update the QoS policy for
 * \param[in]  new_qos_policy The new content filter QoS policy
 *
 * \return DDS_BOOLEAN_TRUE if the update was successful or DDS_BOOLEAN_FALSE otherwise.
 */
extern DDS_Boolean
DDS_DataReader_update_filter(
        DDS_DataReader *dr,
        const struct DDS_ContentFilterQosPolicy *new_qos_policy);

/*ci
 * \brief Finalize this DataReader's filter.
 *
 * \param[in]  dr The DataReader to finalize the filter for
 */
extern void
DDS_DataReader_finalize_filter(DDS_DataReader *dr);

/*ci
 * \brief Get the ContentFilterProperty of this DataReader
 *
 * \param[in]  dr The DataReader to get the filter property from
 * \param[out] filter_property_out The property to copy the filter property into
 *
 * \return DDS_BOOLEAN_TRUE if the filter property was successfully copied or
 *         DDS_BOOLEAN_FALSE otherwise.
 */
extern DDS_Boolean
DDS_DataReader_get_filter_property(
        DDS_DataReader *dr,
        struct DDS_ContentFilterProperty *filter_property_out);

/*ci
 * \brief Serialize the ContentFilterProperty of this DataReader into a stream
 *        using the reader's filter plugin. If the reader does not have a filter
 *        configured, this function will do nothing and return DDS_BOOLEAN_TRUE.
 *
 * * \param[in]  dr The DataReader to serialize the filter property from
 * * \param[out] stream The stream to serialize the filter property into
 *
 * * \return DDS_BOOLEAN_TRUE if the serialization was successful or if the DataReader
 *           does not have a filter configured. DDS_BOOLEAN_FALSE otherwise.
 */
extern DDS_Boolean
DDS_DataReader_serialize_filter_property(
        const DDS_DataReader *dr,
        struct CDR_Stream_t *stream);

/*ci
 * \brief Process the filter info in the in-line QoS of a sample from a stream
 *        using the DataReader's filter plugin to determine if the writer applied
 *        this DataReader's filter to the sample. If the DataReader does not have a
 *        filter configured, this function will instead skip over the filter info
 *        in the stream and set filtered_out to DDS_BOOLEAN_FALSE.
 *
 * \param[in]  dr The DataReader to process the filter info for
 * \param[in]  packet The packet containing the sample
 * \param[in]  stream The stream to process the filter info from
 * \param[in]  pid_length The length of the filter info in the stream
 * \param[out] filtered_out If the writer applied the DataReader's filter
 * \param[out] result_out The result of applying the filter to the sample
 *
 * \return DDS_BOOLEAN_TRUE if the processing was successful or if the DataReader
 *         does not have a filter configured. DDS_BOOLEAN_FALSE otherwise.
 */
extern DDS_Boolean
DDS_DataReader_process_filter_info(
        DDS_DataReader *dr,
        NETIO_Packet_T *packet,
        struct CDR_Stream_t *stream,
        DDS_UnsignedShort pid_length,
        DDS_Boolean *filtered_out,
        DDS_Boolean *result_out);

/*ci
 * \brief Evaluate a sample against the filter configured for this DataReader.
 *        If the DataReader does not have a filter configured, result_out will
 *        always be set to DDS_BOOLEAN_TRUE and the function will return
 *        DDS_BOOLEAN_TRUE.
 *
 * \param[in]  dr The DataReader
 * \param[in]  sample The sample to evaluate
 * \param[out] sample_dropped_out If the sample should be dropped because it
 *                                did not pass the filter.
 *
 * \return DDS_BOOLEAN_TRUE if the evaluation was successful or if the DataReader
 *         does not have a filter configured. DDS_BOOLEAN_FALSE otherwise.
 */
extern DDS_Boolean
DDS_DataReader_evaluate_filter(
        DDS_DataReader *dr,
        const void *sample,
        DDS_Boolean *sample_dropped_out);

#ifdef __cplusplus
}
#endif

#endif /* DataReaderFilter_h */
