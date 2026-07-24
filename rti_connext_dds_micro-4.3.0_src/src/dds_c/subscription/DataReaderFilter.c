/*
 * FILE: DataReaderFilter.c - DataReader filter related functions implementation
 *
 * Copyright (c) 2025-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*ci
 * \brief DataReader filter related functions implementation
 */

#include "DataReaderFilter.h"
#include "DataReaderImpl.h"
#include "Topic.h"

/*** SOURCE_BEGIN ***/

DDS_Boolean
DDS_DataReader_update_filter(
        DDS_DataReader *dr,
        const struct DDS_ContentFilterQosPolicy *new_qos_policy)
{
    DDS_ReturnCode_t retval;

    OSAPI_PRECONDITION((dr == NULL) || (new_qos_policy == NULL) || !DDS_DataReader_is_filtering_enabled(dr),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("dr", dr, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("new_qos_policy", new_qos_policy, RTI_TRUE);)

    if ((new_qos_policy->filter_name == NULL)
         || (DDS_String_length(new_qos_policy->filter_name) == 0))
    {
        /* The DataReader has not configured a content filter */
        if (dr->compiled_filter != NULL)
        {
            DDS_DataReader_finalize_filter(dr);
        }
    }
    else if (!DDS_FilterPlugin_reader_compile(dr->config->filter_plugin,
                                         dr,
                                         new_qos_policy,
                                         &dr->compiled_filter))
    {
        /* This is expected if the new filter is not valid */
        return DDS_BOOLEAN_FALSE;
    }

    /* Finalize our shallow copy of the old filter */
    DDS_FilterPlugin_filter_qos_finalize_shallow_copy(dr->config->filter_plugin, dr->content_filter_qos);

    /* Shallow copy the new filter qos into the DataReader. The function should
     * never fail if we were successfully able to compile the filter. If it did
     * fail, then the DataReader would be left in an inconsistent state, but
     * this should never happen.
     */
    retval = DDS_FilterPlugin_filter_qos_shallow_copy(dr->config->filter_plugin,
                                                      dr->content_filter_qos,
                                                      new_qos_policy);
    IGNORE_RETVAL(retval);

    return DDS_BOOLEAN_TRUE;
}

void
DDS_DataReader_finalize_filter(DDS_DataReader *dr)
{
    OSAPI_PRECONDITION((dr == NULL) || (dr->compiled_filter == NULL),
                        return,
                        OSAPI_Log_entry_add_pointer("dr", dr, RTI_TRUE);)

    DDS_FilterPlugin_reader_finalize(dr->config->filter_plugin, dr->compiled_filter);
    dr->compiled_filter = NULL;
}

DDS_Boolean
DDS_DataReader_get_filter_property(
        DDS_DataReader *dr,
        struct DDS_ContentFilterProperty *filter_property_out)
{
    OSAPI_PRECONDITION((dr == NULL) || (filter_property_out == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("dr", dr, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("filter_property_out", filter_property_out, RTI_TRUE);)

    if (!DDS_DataReader_is_filtering_enabled(dr))
    {
        struct DDS_ContentFilterProperty default_filter_property = DDS_CONTENT_FILTER_PROPERTY_DEFAULT;

        return DDS_ContentFilterProperty_copy(filter_property_out,
                                              &default_filter_property);
    }

    return DDS_FilterPlugin_filter_property_copy_from_filter(
                dr->config->filter_plugin, dr, dr->compiled_filter, filter_property_out);
}

DDS_Boolean
DDS_DataReader_serialize_filter_property(
        const DDS_DataReader *dr,
        struct CDR_Stream_t *stream)
{
    OSAPI_PRECONDITION((dr == NULL) || (stream == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("dr", dr, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("stream", stream, RTI_TRUE);)

    if (dr->compiled_filter == NULL)
    {
        return DDS_BOOLEAN_TRUE;
    }

    return DDS_FilterPlugin_filter_property_serialize(
                dr->config->filter_plugin, stream, dr, dr->compiled_filter);
}

DDS_Boolean
DDS_DataReader_process_filter_info(
        DDS_DataReader *dr,
        NETIO_Packet_T *packet,
        struct CDR_Stream_t *stream,
        DDS_UnsignedShort pid_length,
        DDS_Boolean *filtered_out,
        DDS_Boolean *result_out)
{
    OSAPI_PRECONDITION((dr == NULL) || (packet == NULL) || (stream == NULL) || (filtered_out == NULL) || (result_out == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("dr", dr, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("packet", packet, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("stream", stream, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("filtered_out", filtered_out, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("result_out", result_out, RTI_TRUE);)

    if ((dr->compiled_filter == NULL) || !packet->info.valid_data)
    {
        /* Skip over filter info */
        if (!CDR_Stream_increment_current_position(stream, pid_length))
        {
            return DDS_BOOLEAN_FALSE;
        }
        *filtered_out = DDS_BOOLEAN_FALSE;
        return DDS_BOOLEAN_TRUE;
    }

    return DDS_FilterPlugin_reader_process_filter_info(
                dr->config->filter_plugin,
                dr->compiled_filter,
                stream,
                filtered_out,
                result_out);
}

DDS_Boolean
DDS_DataReader_evaluate_filter(
        DDS_DataReader *dr,
        const void *sample,
        DDS_Boolean *sample_dropped_out)
{
    OSAPI_PRECONDITION((dr == NULL) || (sample == NULL) || (sample_dropped_out == NULL)
                            || (dr->compiled_filter == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("dr", dr, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("sample", sample, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("sample_dropped_out", sample_dropped_out, RTI_TRUE);)

    return DDS_FilterPlugin_reader_evaluate(
                dr->config->filter_plugin,
                dr->compiled_filter,
                sample,
                sample_dropped_out);
}
