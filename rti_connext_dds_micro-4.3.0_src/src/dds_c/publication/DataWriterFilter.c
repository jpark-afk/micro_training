/*
 * FILE: DataWriterFilter.c - DataWriter filter related functions implementation
 *
 * Copyright (c) 2025-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*ci
 * \brief DataWriter filter related functions implementation
 */

#include "DataWriterFilter.h"
#include "DataWriterImpl.h"
#include "Topic.h"

/*** SOURCE_BEGIN ***/

extern DDS_Boolean
DDS_DataWriter_create_filter(
        DDS_DataWriter *dw,
        struct RTPS_FilterPluginWriterFilter **writer_filter_out)
{
    DDS_UnsignedLong  max_remote_reader_filters;

    OSAPI_PRECONDITION((dw == NULL) || (writer_filter_out == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("dw", dw, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("writer_filter_out", writer_filter_out, RTI_TRUE);)

    *writer_filter_out = NULL;
    if ((dw->config->filter_plugin == NULL)
        || DDS_ObjectId_is_builtin(dw->as_entity.entity_id)
#if DDS_XTYPES_IS_ENABLED
        || (DDS_DataWriter_get_typecode(dw) == NULL)
#endif
        )
    {
        /* The DataWriter does not support filtering, so there is nothing to do */
        return DDS_BOOLEAN_TRUE;
    }

    if (!DDS_Duration_is_infinite(&dw->liveliness.lease_duration))
    {
        /* Writer-side filtering requires an infinite liveliness lease duration */
        return DDS_BOOLEAN_TRUE;
    }

    if (dw->writer_resource_limits.max_remote_reader_filters == 0)
    {
        /* Writer has explicitly disabled filtering */
        return DDS_BOOLEAN_TRUE;
    }
    else if (dw->writer_resource_limits.max_remote_reader_filters == DDS_LENGTH_UNLIMITED)
    {
        max_remote_reader_filters =
                (DDS_UnsignedLong)dw->writer_resource_limits.max_remote_readers;
    }
    else
    {
        max_remote_reader_filters =
                (DDS_UnsignedLong)dw->writer_resource_limits.max_remote_reader_filters;
    }

    return DDS_FilterPlugin_writer_attach(
                dw->config->filter_plugin,
                dw,
                max_remote_reader_filters,
                (DDS_UnsignedLong)dw->writer_resource_limits.max_remote_readers,
                (DDS_UnsignedLong)dw->writer_resource_limits.max_routes_per_reader,
                (DDS_UnsignedLong)dw->resource_limits.max_samples,
                writer_filter_out);
}

#ifndef RTI_CERT
void
DDS_DataWriter_delete_filter(
        DDS_DataWriter *dw,
        struct RTPS_FilterPluginWriterFilter *writer_filter)
{
    OSAPI_PRECONDITION((dw == NULL) || (writer_filter == NULL),
                        return,
                        OSAPI_Log_entry_add_pointer("dw", dw, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("writer_filter", writer_filter, RTI_TRUE);)

    DDS_FilterPlugin_writer_detach(dw->config->filter_plugin, writer_filter);
}
#endif /* !RTI_CERT */

DDS_Boolean
DDS_DataWriter_add_remote_reader_filter(
        DDS_DataWriter *dw,
        const struct DDS_SubscriptionBuiltinTopicData *reader_data)
{
    OSAPI_PRECONDITION((dw == NULL) || (reader_data == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("dw", dw, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("reader_data", reader_data, RTI_TRUE);)

    if (!DDS_DataWriter_is_filtering_enabled(dw))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_FilterPlugin_writer_add_remote_reader(
                dw->config->filter_plugin,
                dw->writer_filter,
                &reader_data->key,
                &reader_data->content_filter,
                reader_data->reliability.kind == DDS_RELIABLE_RELIABILITY_QOS);
}

DDS_Boolean
DDS_DataWriter_add_local_reader_filter(
        DDS_DataWriter *dw,
        const DDS_BuiltinTopicKey_t *reader_key,
        DDS_Boolean reliable,
        struct DDS_ContentFilterCompiledFilter *reader_filter)
{
    NETIO_Interface_T *intra_intf = NULL;

    /* reader_filter will be NULL if the reader does not have a filter */
    OSAPI_PRECONDITION((dw == NULL) || (reader_key == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("dw", dw, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("reader_key", reader_key, RTI_TRUE);)

    if (!DDS_DataWriter_is_filtering_enabled(dw))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!NETIO_AddressResolver_lookup_interface(dw->config->addr_resolver,
                                                NETIO_DEFAULT_INTRA_NAME,
                                                &intra_intf))
    {
        return DDS_BOOLEAN_FALSE;
    }
    if (intra_intf != NULL)
    {
        /* If the participant has the intra transport enabled, then this
         * datawriter does not need to filter for any local readers because
         * the intra transport does not support writer-side filtering.
         */
        return DDS_BOOLEAN_TRUE;
    }

    return DDS_FilterPlugin_writer_add_local_reader(
                dw->config->filter_plugin,
                dw->writer_filter,
                reader_key,
                reader_filter,
                reliable);
}

void
DDS_DataWriter_delete_reader_filter(
        DDS_DataWriter *dw,
        const DDS_BuiltinTopicKey_t *reader_key)
{
    OSAPI_PRECONDITION((dw == NULL) || (reader_key == NULL)
                            || !DDS_DataWriter_is_filtering_enabled(dw),
                        return,
                        OSAPI_Log_entry_add_pointer("dw", dw, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("reader_key", reader_key, RTI_TRUE);)

    DDS_FilterPlugin_writer_remove_reader(dw->config->filter_plugin,
                                          dw->writer_filter,
                                          reader_key);
}

DDS_Boolean
DDS_DataWriter_evaluate_filter(
        DDS_DataWriter *dw,
        const struct REDA_SequenceNumber *sn,
        DDS_Boolean unregister_or_dispose,
        const void *sample)
{
    OSAPI_PRECONDITION((dw == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("dw", dw, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("sample", sample, RTI_TRUE);)

    if (!DDS_DataWriter_is_filtering_enabled(dw))
    {
        /* No filter is configured */
        return DDS_BOOLEAN_TRUE;
    }

    return DDS_FilterPlugin_writer_evaluate(
                dw->config->filter_plugin,
                dw->writer_filter,
                sn,
                unregister_or_dispose,
                sample);
}

DDS_Boolean
DDS_DataWriter_apply_reader_filter(
        DDS_DataWriter *dw,
        const struct NETIO_Guid *reader_guid,
        const struct REDA_SequenceNumber *sn,
        DDS_Boolean *sample_dropped_out)
{
    OSAPI_PRECONDITION((dw == NULL) || (reader_guid == NULL) || (sample_dropped_out == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("dw", dw, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("reader_guid", reader_guid, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("sample_dropped_out", sample_dropped_out, RTI_TRUE);)

    if (!DDS_DataWriter_is_filtering_enabled(dw))
    {
        /* No filter is configured */
        return DDS_BOOLEAN_TRUE;
    }

    return DDS_FilterPlugin_writer_apply_reader_filter(
                dw->config->filter_plugin,
                dw->writer_filter,
                reader_guid,
                sn,
                sample_dropped_out);
}
