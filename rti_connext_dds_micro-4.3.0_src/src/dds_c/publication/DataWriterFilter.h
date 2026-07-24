/*
 * FILE: DataWriterFilter.h - DataWriter filter related functions declarations
 *
 * Copyright (c) 2025-2025 Real-Time Innovations,Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*ci
 * \brief DataWriter filter related functions declarations
 */

#ifndef DataWriterFilter_h
#define DataWriterFilter_h

#include "dds_c/dds_c_filter_plugin.h"
#include "dds_c/dds_c_discovery.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define DDS_DataWriter_is_filtering_enabled(dw_) ((dw_)->writer_filter != NULL)

extern DDS_Boolean
DDS_DataWriter_create_filter(
        DDS_DataWriter *dw,
        struct RTPS_FilterPluginWriterFilter **writer_filter_out);

#ifndef RTI_CERT
extern void
DDS_DataWriter_delete_filter(
        DDS_DataWriter *dw,
        struct RTPS_FilterPluginWriterFilter *writer_filter);
#endif /* !RTI_CERT */

extern DDS_Boolean
DDS_DataWriter_add_remote_reader_filter(
        DDS_DataWriter *dw,
        const struct DDS_SubscriptionBuiltinTopicData *reader_data);

extern DDS_Boolean
DDS_DataWriter_add_local_reader_filter(
        DDS_DataWriter *dw,
        const DDS_BuiltinTopicKey_t *reader_key,
        DDS_Boolean reliable,
        struct DDS_ContentFilterCompiledFilter *reader_filter);

extern void
DDS_DataWriter_delete_reader_filter(
        DDS_DataWriter *dw,
        const DDS_BuiltinTopicKey_t *reader_key);

extern DDS_Boolean
DDS_DataWriter_evaluate_filter(
        DDS_DataWriter *dw,
        const struct REDA_SequenceNumber *sn,
        DDS_Boolean unregister_or_dispose,
        const void *sample);

extern DDS_Boolean
DDS_DataWriter_apply_reader_filter(
        DDS_DataWriter *dw,
        const struct NETIO_Guid *reader_guid,
        const struct REDA_SequenceNumber *sn,
        DDS_Boolean *sample_dropped_out);

#ifdef __cplusplus
}
#endif

#endif /* DataWriterFilter_h */
