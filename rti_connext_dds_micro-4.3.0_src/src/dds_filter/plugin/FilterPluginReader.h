/*
 * FILE: FilterPluginReader.h - Filter plugin reader definitions
 *
 * Copyright (c) 2025-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#ifndef FilterPluginReader_h
#define FilterPluginReader_h

#include "dds_c/dds_c_filter_plugin.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern DDS_Boolean
DDS_FilterPluginImpl_reader_compile(
        struct DDS_FilterPlugin *plugin,
        DDS_DataReader *reader,
        const struct DDS_ContentFilterQosPolicy *filter_qos,
        struct DDS_ContentFilterCompiledFilter **compiled_filter_inout);

extern DDS_Boolean
DDS_FilterPluginImpl_reader_process_filter_info(
        struct DDS_FilterPlugin *plugin,
        struct DDS_ContentFilterCompiledFilter *compiled_filter,
        struct CDR_Stream_t *stream,
        DDS_Boolean *filtered_out,
        DDS_Boolean *result_out);

extern DDS_Boolean
DDS_FilterPluginImpl_reader_evaluate(
        struct DDS_FilterPlugin *plugin,
        struct DDS_ContentFilterCompiledFilter *compiled_filter,
        const void *sample,
        DDS_Boolean *sample_dropped_out);

#ifdef __cplusplus
}
#endif

#endif /* FilterPluginReader_h */
