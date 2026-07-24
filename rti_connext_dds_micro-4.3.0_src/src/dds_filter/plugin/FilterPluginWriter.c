/*
 * FILE: FilterPluginWriter.c - Filter plugin Writer implementation
 *
 * Copyright (c) 2025-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#include "dds_c/dds_c_infrastructure.h"
#include "dds_c/dds_c_publication.h"

#include "FilterPluginWriter.h"
#include "FilterPlugin.h"

/*** SOURCE_BEGIN ***/

RTI_PRIVATE RTI_INT32
DDS_RemoteReaderFilterEntry_compare(RTI_INT32 flags, const DB_Record_T op1, void *op2)
{
    const struct DDS_ReaderFilterEntry *record_left = (const struct DDS_ReaderFilterEntry *)op1;
    const struct NETIO_Guid *guid_right;

    if (DB_SELECT_OP2_IS_KEY(flags))
    {
        guid_right = (const struct NETIO_Guid *)op2;
    }
    else
    {
        guid_right = &((const struct DDS_ReaderFilterEntry *)op2)->reader_guid;
    }

    return OSAPI_Memory_compare(&record_left->reader_guid, guid_right, sizeof(struct NETIO_Guid));
}

RTI_PRIVATE RTI_INT32
DDS_RouteFilterEntry_compare(RTI_INT32 flags, const DB_Record_T op1, void *op2)
{
    const struct DDS_RouteFilterEntry *record_left = (const struct DDS_RouteFilterEntry *)op1;
    const struct DDS_RouteFilterEntry *record_right = (const struct DDS_RouteFilterEntry *)op2;
    RTI_INT32 result;

    /* DB_SELECT_OP2_IS_KEY is not checked because the
     * key is always a full DDS_RouteFilterEntry struct.
     */
    UNUSED_ARG(flags);

    result = NETIO_Address_compare(&record_left->dest_address, &record_right->dest_address);
    if (result != 0)
    {
        return result;
    }

    if (record_left->reader > record_right->reader)
    {
        return 1;
    }
    else if (record_left->reader < record_right->reader)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

DDS_Boolean
DDS_FilterPluginImpl_writer_attach(
        struct DDS_FilterPlugin *plugin,
        DDS_DataWriter *writer,
        DDS_UnsignedLong max_remote_reader_filters,
        DDS_UnsignedLong max_remote_readers,
        DDS_UnsignedLong max_routes_per_reader,
        DDS_UnsignedLong max_samples,
        struct RTPS_FilterPluginWriterFilter **writer_filter_out)
{
    struct RTPS_FilterPluginWriterFilter *writer_filter = NULL;
    struct DDS_TypeCode *type_code = NULL;
    DDS_Boolean result = DDS_BOOLEAN_FALSE;
    char tbl_name[NETIO_TABLE_NAME_SIZE];
    union RT_ComponentFactoryId id;
    struct DB_TableProperty tbl_prop = DB_TableProperty_INITIALIZER;
    DB_ReturnCode_T dbrc;
    DDS_UnsignedLong max_window_size;
    DDS_InstanceHandle_t writer_handle;
    struct RTPS_Guid writer_guid;

    OSAPI_PRECONDITION((plugin == NULL) || (writer == NULL) || (writer_filter_out == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("plugin", plugin, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("writer", writer, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("writer_filter_out", writer_filter_out, RTI_TRUE);)

    *writer_filter_out = NULL;

    if (max_remote_reader_filters == 0)
    {
        /* The writer has explicitly disabled filtering */
        return DDS_BOOLEAN_TRUE;
    }

#if DDS_XTYPES_IS_ENABLED
    type_code = DDS_DataWriter_get_typecode(writer);
    if (type_code == NULL)
    {
        /* Writer's type does not support filtering */
        return DDS_BOOLEAN_TRUE;
    }
#endif

    if (max_samples == 0)
    {
        return DDS_BOOLEAN_FALSE;
    }
    else if (max_samples < RTPS_BITMAP_SIZE_MAX)
    {
        max_window_size = max_samples;
    }
    else
    {
        max_window_size = RTPS_BITMAP_SIZE_MAX;
    }

    OSAPI_Heap_allocate_struct(&writer_filter, struct RTPS_FilterPluginWriterFilter);
    if (writer_filter == NULL)
    {
        DDS_FILTER_LOG_HEAP_ALLOC(OSAPI_LOGKIND_ERROR, DDS_FILTER_LOG_WRITER_FILTER_BUFFER)
        return DDS_BOOLEAN_FALSE;
    }

#ifndef RTI_CERT
    /* Go ahead and increment because on failure DDS_FilterPluginImpl_writer_detach
     * will always decrement it back down.
     */
    plugin->writer_count++;
#endif /* !RTI_CERT */

    writer_filter->inline_qos_buf = NULL;
    writer_filter->reader_table = NULL;
    writer_filter->route_table = NULL;
    writer_filter->type_name = DDS_TopicDescription_get_type_name(
            DDS_Topic_as_topicdescription(DDS_DataWriter_get_topic(writer)));
    writer_filter->type_code = type_code;
    writer_filter->max_remote_reader_filters = max_remote_reader_filters;
    writer_filter->cur_remote_reader_filters = 0;
    writer_filter->max_window_size = max_window_size;
    writer_filter->last_evaluated_sn = (struct REDA_SequenceNumber)REDA_SEQUENCE_NUMBER_ZERO;

    writer_filter->inline_qos_buf = OSAPI_Heap_allocate(1, RTPS_INLINE_QOS_MAX_SIZE);
    if (writer_filter->inline_qos_buf == NULL)
    {
        DDS_FILTER_LOG_HEAP_ALLOC(OSAPI_LOGKIND_ERROR, DDS_FILTER_LOG_INLINE_QOS_BUFFER)
        goto done;
    }

    writer_handle = DDS_Entity_get_instance_handle(DDS_DataWriter_as_entity(writer));
    DDS_InstanceHandle_to_rtps(&writer_guid, &writer_handle);

    /* reader_table */
    id._value = plugin->factory->_parent._id._value;
    NETIO_Interface_Table_name_from_id(tbl_name,&id,'f',(RTI_INT32)writer_guid.object_id);

    tbl_prop.max_records = (RTI_SIZE_T)max_remote_readers;
    dbrc = DB_Database_create_table(
                &writer_filter->reader_table,
                plugin->property._parent.db,
                tbl_name,
                sizeof(struct DDS_ReaderFilterEntry),
                DDS_RemoteReaderFilterEntry_compare,
                &tbl_prop);
    if (dbrc != DB_RETCODE_OK)
    {
        DDS_FILTER_LOG_TABLE_CREATE(OSAPI_LOGKIND_ERROR,
                                    DDS_FILTER_LOG_READER_RECORD,
                                    dbrc)
        goto done;
    }

    /* route_table */
    id._value = plugin->factory->_parent._id._value;
    NETIO_Interface_Table_name_from_id(tbl_name,&id,'r',(RTI_INT32)writer_guid.object_id);

    tbl_prop.max_records = (RTI_SIZE_T)max_routes_per_reader * max_remote_readers;
    dbrc = DB_Database_create_table(
                &writer_filter->route_table,
                plugin->property._parent.db,
                tbl_name,
                sizeof(struct DDS_RouteFilterEntry),
                DDS_RouteFilterEntry_compare,
                &tbl_prop);
    if (dbrc != DB_RETCODE_OK)
    {
        DDS_FILTER_LOG_TABLE_CREATE(OSAPI_LOGKIND_ERROR,
                                    DDS_FILTER_LOG_ROUTE_RECORD,
                                    dbrc)
        goto done;
    }

    *writer_filter_out = writer_filter;
    result = DDS_BOOLEAN_TRUE;
done:
#ifndef RTI_CERT
    if (!result)
    {
        DDS_FilterPluginImpl_writer_detach(plugin, writer_filter);
    }
#endif /* !RTI_CERT */
    return result;
}

#ifndef RTI_CERT
void
DDS_FilterPluginImpl_writer_detach(
        struct DDS_FilterPlugin *plugin,
        struct RTPS_FilterPluginWriterFilter *writer_filter)
{
    DB_ReturnCode_T dbrc;

    OSAPI_PRECONDITION((plugin == NULL) || (writer_filter == NULL),
                        return,
                        OSAPI_Log_entry_add_pointer("plugin", plugin, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("writer_filter", writer_filter, RTI_TRUE);)

    if (writer_filter->cur_remote_reader_filters != 0)
    {
        DDS_FILTER_LOG_WRITER_DETACH(OSAPI_LOGKIND_ERROR,
                                     writer_filter->cur_remote_reader_filters)
        return;
    }

    if (writer_filter->route_table != NULL)
    {
        dbrc = DB_Database_delete_table(
                    plugin->property._parent.db,
                    writer_filter->route_table);
#if OSAPI_ENABLE_LOG
        if (dbrc != DB_RETCODE_OK)
        {
            DDS_FILTER_LOG_TABLE_DELETE(OSAPI_LOGKIND_ERROR,
                                        DDS_FILTER_LOG_ROUTE_RECORD,
                                        dbrc)
        }
#else
        IGNORE_RETVAL(dbrc);
#endif
    }

    if (writer_filter->reader_table != NULL)
    {
        dbrc = DB_Database_delete_table(
                    plugin->property._parent.db,
                    writer_filter->reader_table);
#if OSAPI_ENABLE_LOG
        if (dbrc != DB_RETCODE_OK)
        {
            DDS_FILTER_LOG_TABLE_DELETE(OSAPI_LOGKIND_ERROR,
                                        DDS_FILTER_LOG_READER_RECORD,
                                        dbrc)
        }
#else
        IGNORE_RETVAL(dbrc);
#endif
    }

    if (writer_filter->inline_qos_buf != NULL)
    {
        OSAPI_Heap_free(writer_filter->inline_qos_buf);
        writer_filter->inline_qos_buf = NULL;
    }

    OSAPI_Heap_free_struct(writer_filter);

    plugin->writer_count--;
}
#endif /* !RTI_CERT */

RTI_PRIVATE void
DDS_FilterPluginImpl_writer_detach_compiled_filter(
        struct DDS_FilterPlugin *plugin,
        struct RTPS_FilterPluginWriterFilter *writer_filter,
        struct DDS_ReaderFilterEntry *entry)
{
    if (entry->compiled_filter != NULL)
    {
        struct REDA_SequenceNumber sn_zero = REDA_SEQUENCE_NUMBER_ZERO;

        DDS_FilterPluginImpl_finalize_filter(plugin, entry->compiled_filter);
        entry->compiled_filter = NULL;
        if (!entry->is_local)
        {
            writer_filter->cur_remote_reader_filters--;
        }
        RTPS_Bitmap_reset(&entry->filter_result, &sn_zero, 0);
    }
}

/* On success *prior_compiled_filter holds the entry's compiled_filter at
 * lookup time (NULL for a new entry) so the caller can detect a filter change.
 */
RTI_PRIVATE DDS_Boolean
DDS_FilterPluginImpl_writer_assert_reader_entry(
        struct RTPS_FilterPluginWriterFilter *writer_filter,
        const DDS_BuiltinTopicKey_t *reader_key,
        DDS_Boolean reliable,
        struct DDS_ReaderFilterEntry **entry,
        struct DDS_ContentFilterCompiledFilter **prior_compiled_filter)
{
    struct NETIO_Address key;
    struct REDA_SequenceNumber sn_zero = REDA_SEQUENCE_NUMBER_ZERO;
    DB_ReturnCode_T dbrc;

    *entry = NULL;
    *prior_compiled_filter = NULL;

    NETIO_Address_set_guid_from_key(&key, 0, (struct NETIO_AddressInt32*)reader_key);
    dbrc = DB_Table_select_match(writer_filter->reader_table,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T *)entry,
                                 (DB_Key_T)&key.value.guid);
    if (dbrc == DB_RETCODE_OK)
    {
        *prior_compiled_filter = (*entry)->compiled_filter;
        (*entry)->reliable = reliable;
        return DDS_BOOLEAN_TRUE;
    }
    else if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDS_FILTER_LOG_RECORD_SELECT(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_READER_RECORD,
                                     dbrc)
        return DDS_BOOLEAN_FALSE;
    }

    dbrc = DB_Table_create_record(writer_filter->reader_table,
                                  (DB_Record_T *)entry);
    if (dbrc != DB_RETCODE_OK)
    {
        DDS_FILTER_LOG_RECORD_CREATE(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_READER_RECORD,
                                     dbrc)
        return DDS_BOOLEAN_FALSE;
    }

    (*entry)->reader_guid = key.value.guid;
    (*entry)->route_count = 0;
    (*entry)->rtps_intf = NULL;
    (*entry)->rtps_peer_entry = NULL;
    (*entry)->last_sent_sn = (struct REDA_SequenceNumber)REDA_SEQUENCE_NUMBER_ZERO;
    (*entry)->reliable = reliable;
    (*entry)->is_local = DDS_BOOLEAN_FALSE;
    (*entry)->compiled_filter = NULL;
    RTPS_Bitmap_reset(&(*entry)->filter_result, &sn_zero, 0);

    dbrc = DB_Table_insert_record(writer_filter->reader_table,
                                  (DB_Record_T)*entry);
    if (dbrc != DB_RETCODE_OK)
    {
        DDS_FILTER_LOG_RECORD_INSERT(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_READER_RECORD,
                                     dbrc)

        dbrc = DB_Table_delete_record(writer_filter->reader_table,
                                      (DB_Record_T)*entry);
#if OSAPI_ENABLE_LOG
        if (dbrc != DB_RETCODE_OK)
        {
            DDS_FILTER_LOG_RECORD_DELETE(OSAPI_LOGKIND_ERROR,
                                         DDS_FILTER_LOG_READER_RECORD,
                                         dbrc)
        }
#else
        IGNORE_RETVAL(dbrc);
#endif
        *entry = NULL;
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

DDS_Boolean
DDS_FilterPluginImpl_writer_add_remote_reader(
        struct DDS_FilterPlugin *plugin,
        struct RTPS_FilterPluginWriterFilter *writer_filter,
        const DDS_BuiltinTopicKey_t *reader_key,
        const struct DDS_ContentFilterProperty *filter_property,
        DDS_Boolean reliable)
{
    struct DDS_ReaderFilterEntry *record = NULL;
    struct DDS_ContentFilterCompiledFilter *prior_compiled_filter = NULL;
    struct DDS_FilterSignature signature = DDS_FilterSignature_INITIALIZER;
    struct REDA_SequenceNumber sn_zero = REDA_SEQUENCE_NUMBER_ZERO;
    DDS_Boolean has_filter_class;
    DDS_Boolean install_filter;

    OSAPI_PRECONDITION((plugin == NULL) || (writer_filter == NULL) || (reader_key == NULL) || (filter_property == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("plugin", plugin, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("writer_filter", writer_filter, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("reader_key", reader_key, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("filter_property", filter_property, RTI_TRUE);)

    has_filter_class = filter_property->filter_class_name != NULL
                    && filter_property->filter_class_name[0] != '\0';

    if (!DDS_FilterPluginImpl_writer_assert_reader_entry(
            writer_filter,
            reader_key,
            reliable,
            &record,
            &prior_compiled_filter))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (has_filter_class
            && !DDS_FilterSignature_calculate_from_property(
                    &signature,
                    (struct DDS_ContentFilterProperty *)filter_property))
    {
        /* We should always fallback to not applying writer-side filtering if
         * we are unable to handle a reader's filter.
         */
        DDS_FilterPluginImpl_writer_detach_compiled_filter(plugin, writer_filter, record);
        return DDS_BOOLEAN_TRUE;
    }

    /* A reader with an existing filter already counts toward the writer's
     * limit, so re-asserting it must be allowed even at capacity. Only a
     * brand-new filter is gated by the cur < max check.
     */
    install_filter = has_filter_class
                    && ((writer_filter->cur_remote_reader_filters
                                < writer_filter->max_remote_reader_filters)
                        || (prior_compiled_filter != NULL));

    if (install_filter)
    {
        /* Skip the bitmap reset when the compiled filter pointer hasn't
         * changed so accumulated results survive a discovery refresh.
         */
        if (DDS_FilterPluginImpl_compile_filter(plugin,
                                                &signature,
                                                filter_property->content_filtered_topic_name,
                                                filter_property->filter_class_name,
                                                filter_property->filter_expression,
                                                &filter_property->expression_parameters,
                                                writer_filter->type_code,
                                                writer_filter->type_name,
                                                &record->compiled_filter))
        {
            if (record->compiled_filter != prior_compiled_filter)
            {
                /* Only a fresh install bumps the writer-level count;
                 * a swap leaves it stable.
                 */
                if (prior_compiled_filter == NULL)
                {
                    writer_filter->cur_remote_reader_filters++;
                }
                RTPS_Bitmap_reset(&record->filter_result, &sn_zero, 0);
            }
        }
        else
        {
            /* It is expected that we may discover filters which we do not
             * support, so this is not a critical error.
             */
            DDS_FILTER_LOG_WRITER_FILTER_COMPILE(OSAPI_LOGKIND_WARNING)
            DDS_FilterPluginImpl_writer_detach_compiled_filter(plugin, writer_filter, record);
        }
    }
    else
    {
        /* The reader has no filter, or the writer is at the filter limit */
        DDS_FilterPluginImpl_writer_detach_compiled_filter(plugin, writer_filter, record);
    }

    return DDS_BOOLEAN_TRUE;
}

DDS_Boolean
DDS_FilterPluginImpl_writer_add_local_reader(
        struct DDS_FilterPlugin *plugin,
        struct RTPS_FilterPluginWriterFilter *writer_filter,
        const DDS_BuiltinTopicKey_t *reader_key,
        struct DDS_ContentFilterCompiledFilter *reader_filter,
        DDS_Boolean reliable)
{
    struct DDS_ReaderFilterEntry *reader_entry = NULL;
    struct DDS_ContentFilterCompiledFilter *prior_compiled_filter = NULL;
    struct REDA_SequenceNumber sn_zero = REDA_SEQUENCE_NUMBER_ZERO;

    /* reader_filter will be NULL if the reader does not have a filter */

    OSAPI_PRECONDITION((plugin == NULL) || (writer_filter == NULL) || (reader_key == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("plugin", plugin, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("writer_filter", writer_filter, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("reader_key", reader_key, RTI_TRUE);)

    if (!DDS_FilterPluginImpl_writer_assert_reader_entry(
            writer_filter,
            reader_key,
            reliable,
            &reader_entry,
            &prior_compiled_filter))
    {
        return DDS_BOOLEAN_FALSE;
    }
    reader_entry->is_local = DDS_BOOLEAN_TRUE;

    /* Re-asserting the same filter pointer is a no-op so a discovery refresh
     * does not wipe accumulated bitmap results.
     */
    if (reader_filter != prior_compiled_filter)
    {
        DDS_FilterPluginImpl_writer_detach_compiled_filter(plugin, writer_filter, reader_entry);

        if (reader_filter != NULL)
        {
            reader_entry->compiled_filter = reader_filter;
            reader_filter->ref_count++;
            RTPS_Bitmap_reset(&reader_entry->filter_result, &sn_zero, 0);
        }
    }

    return DDS_BOOLEAN_TRUE;
}

void
DDS_FilterPluginImpl_writer_remove_reader(
        struct DDS_FilterPlugin *plugin,
        struct RTPS_FilterPluginWriterFilter *writer_filter,
        const DDS_BuiltinTopicKey_t *reader_key)
{
    struct DDS_ReaderFilterEntry *reader = NULL;
    struct NETIO_Address key;
    DB_ReturnCode_T dbrc;

    OSAPI_PRECONDITION((plugin == NULL) || (writer_filter == NULL) || (reader_key == NULL),
                        return,
                        OSAPI_Log_entry_add_pointer("plugin", plugin, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("writer_filter", writer_filter, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("reader_key", reader_key, RTI_TRUE);)

    NETIO_Address_set_guid_from_key(&key, 0, (struct NETIO_AddressInt32*)reader_key);
    dbrc = DB_Table_select_match(writer_filter->reader_table,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T *)&reader,
                                 (DB_Key_T)&key.value.guid);
    if (dbrc == DB_RETCODE_NO_DATA)
    {
        return;
    }
    else if (dbrc != DB_RETCODE_OK)
    {
        DDS_FILTER_LOG_RECORD_SELECT(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_READER_RECORD,
                                     dbrc)
        return;
    }

    if (reader->route_count > 0)
    {
        DDS_FILTER_LOG_WRITER_FILTER_REMOVE_READER(OSAPI_LOGKIND_ERROR, reader->route_count)
        return;
    }

    reader = NULL;
    dbrc = DB_Table_remove_record(writer_filter->reader_table,
                                  (DB_Record_T)&reader,
                                  (DB_Key_T)&key.value.guid);
    if (dbrc != DB_RETCODE_OK)
    {
        DDS_FILTER_LOG_RECORD_REMOVE(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_READER_RECORD,
                                     dbrc)
        return;
    }

    DDS_FilterPluginImpl_writer_detach_compiled_filter(plugin, writer_filter, reader);

    dbrc = DB_Table_delete_record(writer_filter->reader_table,
                                  (DB_Record_T)reader);
#if OSAPI_ENABLE_LOG
    if (dbrc != DB_RETCODE_OK)
    {
        DDS_FILTER_LOG_RECORD_DELETE(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_READER_RECORD,
                                     dbrc)
    }
#else
    IGNORE_RETVAL(dbrc);
#endif
}

DDS_Boolean
DDS_FilterPluginImpl_writer_evaluate(
        struct DDS_FilterPlugin *plugin,
        struct RTPS_FilterPluginWriterFilter *writer_filter,
        const struct REDA_SequenceNumber *sn,
        DDS_Boolean unregister_or_dispose,
        const void *sample)
{
    struct DDS_ReaderFilterEntry *reader_entry = NULL;
    struct DDS_ContentFilterCompiledFilter *compiled_filter = NULL;
    DB_Cursor_T cursor;
    DB_ReturnCode_T dbrc;
    struct REDA_SequenceNumber prev_sn;
    struct REDA_SequenceNumber sn_zero = REDA_SEQUENCE_NUMBER_ZERO;
    struct REDA_SequenceNumber min_lead = REDA_SEQUENCE_NUMBER_ZERO;
    struct REDA_SequenceNumber max_window_distance;
    DDS_Boolean invalidate_prev_results = DDS_BOOLEAN_FALSE;

    UNUSED_ARG(plugin);

    OSAPI_PRECONDITION((plugin == NULL) || (writer_filter == NULL) || (sn == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("plugin", plugin, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("writer_filter", writer_filter, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("sn", sn, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("sample", sample, RTI_TRUE);)

    if (!unregister_or_dispose && (sample == NULL))
    {
        /* Sample must be provided for non-dispose/unregister samples */
        return DDS_BOOLEAN_FALSE;
    }

    prev_sn = *sn;
    REDA_SequenceNumber_minusminus(&prev_sn);

    if (REDA_SequenceNumber_compare(&prev_sn, &writer_filter->last_evaluated_sn) != 0)
    {
        /* SNs must be evaluated in order so that all samples in the evaluated
         * range have a valid result. This condition is not ever expected, but
         * for robustness we will invalidate all previous results if it occurs
         * to ensure correctness.
         */
        DDS_FILTER_LOG_WRITER_FILTER_EVALUATE_OUT_OF_ORDER(OSAPI_LOGKIND_WARNING)
        invalidate_prev_results = DDS_BOOLEAN_TRUE;
    }

    max_window_distance = (struct REDA_SequenceNumber){0, writer_filter->max_window_size - 1};
    if (REDA_SequenceNumber_compare(sn, &max_window_distance) > 0)
    {
        min_lead = *sn;
        REDA_SequenceNumber_decrement(&min_lead, &max_window_distance);
    }

    dbrc = DB_Table_select_all(writer_filter->reader_table,
                               DB_TABLE_DEFAULT_INDEX,
                               &cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        DDS_FILTER_LOG_RECORD_SELECT(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_READER_RECORD,
                                     dbrc)
        return DDS_BOOLEAN_FALSE;
    }

    /* Evaluate the filter for each matched reader and store the result per
     * reader. In the future, this could be optimized to only evaluate
     * the unique filters and store the result per filter instead of per reader.
     */
    dbrc = DB_Cursor_get_next(cursor, (DB_Record_T *)&reader_entry);
    while (dbrc == DB_RETCODE_OK)
    {
        if (invalidate_prev_results)
        {
            RTPS_Bitmap_reset(&reader_entry->filter_result, &sn_zero, 0);
        }

        compiled_filter = reader_entry->compiled_filter;
        if (compiled_filter != NULL)
        {
            DDS_Boolean filter_result = DDS_BOOLEAN_FALSE;

            if (unregister_or_dispose)
            {
                /* Skip evaluating the filter for dispose and unregister samples.
                 * The result doesn't matter because filtering is not applied to
                 * these samples, and no content filter info will be included
                 * with them.
                 */
                filter_result = DDS_BOOLEAN_TRUE;
            }
            else if (!compiled_filter->filter_class->intf->evaluate(
                            compiled_filter->filter_class->instance,
                            compiled_filter->compiled_filter_instance,
                            sample,
                            &filter_result))
            {
                /* If the writer fails to evaluate a sample, we should still
                 * send the sample to the reader in case it can successfully
                 * evaluate it. However, this is not equivalent to passing the
                 * filter because we will not provide an indicate in the inline
                 * QoS of whether or not the sample passed the filter.
                 * We have to invalidate all results because there is not a valid
                 * result for this sample.
                 */
                DDS_FILTER_LOG_FILTER_EVALUATE(OSAPI_LOGKIND_WARNING, compiled_filter->filter_class->name)

                RTPS_Bitmap_reset(&reader_entry->filter_result, &sn_zero, 0);

                dbrc = DB_Cursor_get_next(cursor, (DB_Record_T *)&reader_entry);
                continue;
            }

            if (REDA_SequenceNumber_is_zero(&reader_entry->filter_result.lead))
            {
                /* This is the first time this filter is being evaluated */
                RTPS_Bitmap_reset(&reader_entry->filter_result, sn, (RTI_INT32)writer_filter->max_window_size);
            }

            /* If needed, shift the result bitmap so that this result is the last SN in the bitmap */
            if (REDA_SequenceNumber_compare(&min_lead, &reader_entry->filter_result.lead) > 0)
            {
                if (!RTPS_Bitmap_shift(&reader_entry->filter_result, &min_lead))
                {
                    DDS_FILTER_LOG_WRITER_FILTER_STORE_RESULT(OSAPI_LOGKIND_ERROR)
                    RTPS_Bitmap_reset(&reader_entry->filter_result, &sn_zero, 0);
                    dbrc = DB_Cursor_get_next(cursor, (DB_Record_T *)&reader_entry);
                    continue;
                }
            }

            /* Set the result for this SN */
            if (!RTPS_Bitmap_set_bit(&reader_entry->filter_result, NULL, sn, filter_result))
            {
                DDS_FILTER_LOG_WRITER_FILTER_STORE_RESULT(OSAPI_LOGKIND_ERROR)
                RTPS_Bitmap_reset(&reader_entry->filter_result, &sn_zero, 0);
                dbrc = DB_Cursor_get_next(cursor, (DB_Record_T *)&reader_entry);
                continue;
            }
        }
        dbrc = DB_Cursor_get_next(cursor, (DB_Record_T *)&reader_entry);
    }
    DB_Cursor_finish(writer_filter->reader_table, cursor);

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDS_FILTER_LOG_RECORD_SELECT(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_READER_RECORD,
                                     dbrc)
        return DDS_BOOLEAN_FALSE;
    }

    writer_filter->last_evaluated_sn = *sn;

    return DDS_BOOLEAN_TRUE;
}

DDS_Boolean
DDS_FilterPluginImpl_writer_apply_reader_filter(
        struct DDS_FilterPlugin *plugin,
        struct RTPS_FilterPluginWriterFilter *writer_filter,
        const struct NETIO_Guid *reader_guid,
        const struct REDA_SequenceNumber *sn,
        DDS_Boolean *sample_dropped_out)
{
    struct DDS_ReaderFilterEntry *reader_entry = NULL;
    RTI_BOOL sample_passed;
    DB_ReturnCode_T dbrc;

    OSAPI_PRECONDITION((plugin == NULL) || (writer_filter == NULL)
                            || (reader_guid == NULL) || (sn == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("plugin", plugin, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("writer_filter", writer_filter, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("reader_guid", reader_guid, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("sn", sn, RTI_TRUE);)

    PRECOND_ARG(plugin)

    /* Find reader entry */
    dbrc = DB_Table_select_match(writer_filter->reader_table,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T *)&reader_entry,
                                 (DB_Key_T)reader_guid);
    if (dbrc == DB_RETCODE_NO_DATA)
    {
        return DDS_BOOLEAN_TRUE;
    }
    else if (dbrc != DB_RETCODE_OK)
    {
        /* Error, there should be a record for every reader */
        DDS_FILTER_LOG_RECORD_SELECT(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_READER_RECORD,
                                     dbrc)
        return DDS_BOOLEAN_FALSE;
    }

    if (reader_entry->compiled_filter == NULL)
    {
        /* The reader does not have a filter */
        *sample_dropped_out = DDS_BOOLEAN_FALSE;
        return DDS_BOOLEAN_TRUE;
    }

    if ((REDA_SequenceNumber_compare(sn, &writer_filter->last_evaluated_sn) > 0)
            || !RTPS_Bitmap_get_bit(&reader_entry->filter_result, &sample_passed, sn))
    {
        /* This sn is outside of the evaluated range, so we cannot apply writer-side filtering */
        *sample_dropped_out = DDS_BOOLEAN_FALSE;
        return DDS_BOOLEAN_TRUE;
    }

    *sample_dropped_out = !sample_passed;

    return DDS_BOOLEAN_TRUE;
}

RTI_BOOL
DDS_FilterPluginImpl_writer_add_route(
        struct RTPS_FilterPlugin *plugin,
        struct RTPS_FilterPluginWriterFilter *writer_filter,
        struct NETIO_Guid *reader_guid,
        struct NETIO_Address *dest_address,
        struct RTPS_Interface *rtps_intf,
        struct RTPS_PeerEntry *rtps_peer_entry,
        const struct REDA_SequenceNumber *last_sent_sn)
{
    struct DDS_ReaderFilterEntry *reader_entry = NULL;
    struct DDS_RouteFilterEntry *route_record = NULL;
    struct DDS_RouteFilterEntry route_key;
    DB_ReturnCode_T dbrc;

    OSAPI_PRECONDITION((plugin == NULL) || (writer_filter == NULL) || (reader_guid == NULL)
                                || (dest_address == NULL) || (rtps_intf == NULL)
                                || (rtps_peer_entry == NULL) || (last_sent_sn == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("plugin", plugin, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("writer_filter", writer_filter, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("reader_guid", reader_guid, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("dest_address", dest_address, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("rtps_intf", rtps_intf, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("rtps_peer_entry", rtps_peer_entry, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("last_sent_sn", last_sent_sn, RTI_TRUE);)

    PRECOND_ARG(plugin)

    /* Find reader entry */
    dbrc = DB_Table_select_match(writer_filter->reader_table,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T *)&reader_entry,
                                 (DB_Key_T)reader_guid);
    if (dbrc == DB_RETCODE_NO_DATA)
    {
        return DDS_BOOLEAN_TRUE;
    }
    else if (dbrc != DB_RETCODE_OK)
    {
        DDS_FILTER_LOG_RECORD_SELECT(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_READER_RECORD,
                                     dbrc)
        return DDS_BOOLEAN_FALSE;
    }

    /* Check if the route already exists */
    route_key.dest_address = *dest_address;
    route_key.reader = reader_entry;
    dbrc = DB_Table_select_match(writer_filter->route_table,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T *)&route_record,
                                 (DB_Key_T)&route_key);
    if (dbrc == DB_RETCODE_OK)
    {
        /* The route already exists */
        return DDS_BOOLEAN_TRUE;
    }
    else if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDS_FILTER_LOG_RECORD_SELECT(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_ROUTE_RECORD,
                                     dbrc)
        return DDS_BOOLEAN_FALSE;
    }

    /* Create a new route entry */
    dbrc = DB_Table_create_record(writer_filter->route_table,
                                  (DB_Record_T *)&route_record);
    if (dbrc != DB_RETCODE_OK)
    {
        DDS_FILTER_LOG_RECORD_CREATE(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_ROUTE_RECORD,
                                     dbrc)
        return DDS_BOOLEAN_FALSE;
    }

    route_record->dest_address = *dest_address;
    route_record->reader = reader_entry;

    /* Insert the new route entry */
    dbrc = DB_Table_insert_record(writer_filter->route_table,
                                  (DB_Record_T)route_record);
    if (dbrc != DB_RETCODE_OK)
    {
        DDS_FILTER_LOG_RECORD_INSERT(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_ROUTE_RECORD,
                                     dbrc)

        dbrc = DB_Table_delete_record(writer_filter->route_table,
                                      (DB_Record_T)route_record);
#if OSAPI_ENABLE_LOG
        if (dbrc != DB_RETCODE_OK)
        {
            DDS_FILTER_LOG_RECORD_DELETE(OSAPI_LOGKIND_ERROR,
                                         DDS_FILTER_LOG_ROUTE_RECORD,
                                         dbrc)
        }
#else
        IGNORE_RETVAL(dbrc);
#endif
        return DDS_BOOLEAN_FALSE;
    }

    /* Initialize the RTPS interface and peer entry if they are not set */
    if (reader_entry->rtps_intf == NULL)
    {
        reader_entry->rtps_intf = rtps_intf;
    }
    if (reader_entry->rtps_peer_entry == NULL)
    {
        reader_entry->rtps_peer_entry = rtps_peer_entry;
    }

    /* For late-joiners we want to treat samples written before
     * the reader matched as already sent.
     */
    if (REDA_SequenceNumber_compare(last_sent_sn, &reader_entry->last_sent_sn) > 0)
    {
        reader_entry->last_sent_sn = *last_sent_sn;
    }
    /* Increment the route count for the reader */
    reader_entry->route_count++;

    return DDS_BOOLEAN_TRUE;
}

RTI_BOOL
DDS_FilterPluginImpl_writer_delete_route(
        struct RTPS_FilterPlugin *plugin,
        struct RTPS_FilterPluginWriterFilter *writer_filter,
        struct NETIO_Guid *reader_guid,
        struct NETIO_Address *dest_address)
{
    struct DDS_ReaderFilterEntry *reader_entry = NULL;
    struct DDS_RouteFilterEntry *route_record = NULL;
    struct DDS_RouteFilterEntry route_key;
    DB_ReturnCode_T dbrc;

    OSAPI_PRECONDITION((plugin == NULL) || (writer_filter == NULL)
                            || (reader_guid == NULL) || (dest_address == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("plugin", plugin, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("writer_filter", writer_filter, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("reader_guid", reader_guid, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("dest_address", dest_address, RTI_TRUE);)

    PRECOND_ARG(plugin)

    /* Find reader entry */
    dbrc = DB_Table_select_match(writer_filter->reader_table,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T *)&reader_entry,
                                 (DB_Key_T)reader_guid);
    if (dbrc == DB_RETCODE_NO_DATA)
    {
        return DDS_BOOLEAN_TRUE;
    }
    else if (dbrc != DB_RETCODE_OK)
    {
        DDS_FILTER_LOG_RECORD_SELECT(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_READER_RECORD,
                                     dbrc)
        return DDS_BOOLEAN_FALSE;
    }

    /* Remove the route from the table index */
    route_key.dest_address = *dest_address;
    route_key.reader = reader_entry;
    dbrc = DB_Table_remove_record(writer_filter->route_table,
                                  (DB_Record_T *)&route_record,
                                  (DB_Key_T)&route_key);
    if (dbrc == DB_RETCODE_NO_DATA)
    {
        /* The route does not exist */
        return DDS_BOOLEAN_TRUE;
    }
    else if (dbrc != DB_RETCODE_OK)
    {
        DDS_FILTER_LOG_RECORD_REMOVE(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_ROUTE_RECORD,
                                     dbrc)
        return DDS_BOOLEAN_FALSE;
    }

    /* Delete the route record */
    dbrc = DB_Table_delete_record(writer_filter->route_table,
                                  (DB_Record_T)route_record);
#if OSAPI_ENABLE_LOG
    if (dbrc != DB_RETCODE_OK)
    {
        DDS_FILTER_LOG_RECORD_DELETE(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_ROUTE_RECORD,
                                     dbrc)
    }
#else
    IGNORE_RETVAL(dbrc);
#endif

    /* Decrement the route count for the reader */
    reader_entry->route_count--;

    if (reader_entry->route_count == 0)
    {
        reader_entry->rtps_intf = NULL;
        reader_entry->rtps_peer_entry = NULL;
    }

    return DDS_BOOLEAN_TRUE;
}

RTI_PRIVATE RTI_BOOL
DDS_CdrStream_serialize_filter_signature(struct CDR_Stream_t *stream,
                                         const void *data,
                                         void *param)
{
    struct DDS_FilterSignature *filter_signature = (struct DDS_FilterSignature *)data;

    UNUSED_ARG(param);

    if (!CDR_Stream_serialize_primitive_array(stream,
                                              filter_signature->value,
                                              16, CDR_OCTET_TYPE))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_PRIVATE RTI_BOOL
DDS_FilterPluginImpl_serialize_content_filter_info(
        struct RTPS_FilterPluginWriterFilter *writer_filter,
        const struct DDS_ContentFilterInfo *filter_info,
        NETIO_Packet_T *packet)
{
    struct CDR_Stream_t stream;
    RTI_UINT16 pid_id;
    RTI_UINT16 pid_length;
    RTI_UINT32 pid_length_offset;
    RTI_UINT32 pid_start_offset;
    RTI_UINT32 inline_qos_length;

    CDR_Stream_initialize(&stream);
    if (!CDR_Stream_set_buffer(&stream,
                               writer_filter->inline_qos_buf,
                               RTPS_INLINE_QOS_MAX_SIZE))
    {
        return RTI_FALSE;
    }

    pid_id = RTPS_PID_CONTENT_FILTER_INFO;
    if (!CDR_Stream_serialize_unsigned_short(&stream, &pid_id))
    {
        return RTI_FALSE;
    }

    /* Save offset to pid length and serialize a temporary value which will be overwritten */
    pid_length_offset = CDR_Stream_get_current_position_offset(&stream);
    pid_length = 0;
    if (!CDR_Stream_serialize_unsigned_short(&stream, &pid_length))
    {
        return RTI_FALSE;
    }
    pid_start_offset = CDR_Stream_get_current_position_offset(&stream);

    /* Serialize filter result and signatures */
    if (!CDR_Stream_serialize_primitive_sequence(
                &stream,
                (struct REDA_Sequence *)&filter_info->filter_result,
                CDR_UNSIGNED_LONG_TYPE))
    {
        return RTI_FALSE;
    }
    if (!CDR_Stream_serialize_non_primitive_sequence(
                &stream,
                (struct REDA_Sequence *)&filter_info->filter_signatures,
                DDS_CdrStream_serialize_filter_signature))
    {
        return RTI_FALSE;
    }

    inline_qos_length = CDR_Stream_get_current_position_offset(&stream);

    /* Go back and serialize the correct pid length */
    pid_length = (RTI_UINT16)(CDR_Stream_get_current_position_offset(&stream) - pid_start_offset);
    if (!CDR_Stream_set_current_position_offset(&stream, pid_length_offset)
        || !CDR_Stream_serialize_unsigned_short(&stream, &pid_length))
    {
        return RTI_FALSE;
    }

    /* If the DDS layer included an inline QoS, then it already includes
     * the sentinel. Otherwise, we need to add it.
     */
    if (!(packet->info.rtps_flags & NETIO_RTPS_FLAGS_INLINEQOS))
    {
        packet->info.rtps_flags |= NETIO_RTPS_FLAGS_INLINEQOS;

        pid_id = RTPS_PID_SENTINEL;
        pid_length = 0;

        if (!CDR_Stream_set_current_position_offset(&stream, inline_qos_length)
            || !CDR_Stream_serialize_unsigned_short(&stream, &pid_id)
            || !CDR_Stream_serialize_unsigned_short(&stream, &pid_length))
        {
            return RTI_FALSE;
        }

        inline_qos_length = CDR_Stream_get_current_position_offset(&stream);
    }

    /* Create a packet buffer to hold the inline qos */
    if (!NETIO_PacketBuffer_set(&writer_filter->inline_qos_pbuf,
                                writer_filter->inline_qos_buf,
                                RTPS_INLINE_QOS_MAX_SIZE,
                                0,
                                inline_qos_length))
    {
        return RTI_FALSE;
    }

    /* Prepend the inline qos buffer to the packet */
    writer_filter->inline_qos_pbuf._next = packet->head_pbuf;
    packet->head_pbuf = &writer_filter->inline_qos_pbuf;

    return RTI_TRUE;
}

RTI_PRIVATE RTI_BOOL
DDS_FilterPluginImpl_writer_construct_filter_info(
        struct RTPS_FilterPluginWriterFilter *writer_filter,
        struct DDS_RouteFilterEntry *route_key_low,
        struct DDS_RouteFilterEntry *route_key_high,
        const struct REDA_SequenceNumber *sn,
        DDS_UnsignedLong *result_bitmask,
        struct DDS_FilterSignature *signatures,
        RTI_INT32 *sig_count_out,
        RTI_BOOL *send_data_out)
{
    struct DDS_RouteFilterEntry *route_entry = NULL;
    DB_Cursor_T cursor = NULL;
    DB_ReturnCode_T dbrc;
    RTI_INT32 sig_count = 0;
    RTI_BOOL send_data = RTI_FALSE;
    RTI_INT32 i;

    dbrc = DB_Table_select_range(writer_filter->route_table,
                                 DB_TABLE_DEFAULT_INDEX,
                                 &cursor,
                                 (DB_Key_T)route_key_low,
                                 (DB_Key_T)route_key_high);
    if (dbrc != DB_RETCODE_OK)
    {
        DDS_FILTER_LOG_RECORD_SELECT(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_ROUTE_RECORD,
                                     dbrc)
        return RTI_FALSE;
    }

    /* Get the content filter info for this route by finding all readers on this
     * route and checking their filter results. In the future this could be
     * optimized by keeping track of filters per route instead of readers per route.
     */
    dbrc = DB_Cursor_get_next(cursor, (DB_Record_T *)&route_entry);
    while (dbrc == DB_RETCODE_OK)
    {
        RTI_BOOL result;

        if ((REDA_SequenceNumber_compare(sn,
                                         &writer_filter->last_evaluated_sn) <= 0)
                && RTPS_Bitmap_get_bit(&route_entry->reader->filter_result,
                                       &result,
                                       sn))
        {
            /* The reader has a filter and the filter was evaluated for this
             * sample. Include the filter result in the content filter info
             * if there is space for it.
             */
            if (sig_count < RTPS_CONTENT_FILTER_MAX_SIGNATURE_COUNT)
            {
                /* Check if this signature has already been included */
                for (i = 0; i < sig_count; i++)
                {
                    if (DDS_FilterSignature_compare(
                            &route_entry->reader->compiled_filter->filter_signature,
                            &signatures[i]) == 0)
                    {
                        break;
                    }
                }

                /* Add new filter signatures and ignore already included signatures */
                if (i == sig_count)
                {
                    result_bitmask[sig_count / 32] |= ((DDS_UnsignedLong)result) << (31 - (sig_count % 32));
                    signatures[sig_count] = route_entry->reader->compiled_filter->filter_signature;

                    sig_count++;
                }
            }

            /* If at least one filter passes then, the sample must be sent on this route */
            if (result)
            {
                send_data = RTI_TRUE;
            }
        }
        else
        {
            /* There is a reader on this route which either does not have a filter
             * or its filter was not evaluated for this sample. Either way we must
             * send the sample to this route.
             */
            send_data = RTI_TRUE;
        }

        dbrc = DB_Cursor_get_next(cursor, (DB_Record_T *)&route_entry);
    }
    DB_Cursor_finish(writer_filter->route_table, cursor);

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDS_FILTER_LOG_RECORD_SELECT(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_ROUTE_RECORD,
                                     dbrc)
        return RTI_FALSE;
    }

    *sig_count_out = sig_count;
    *send_data_out = send_data;

    return RTI_TRUE;
}

RTI_PRIVATE RTI_BOOL
DDS_FilterPluginImpl_writer_send_gaps(
        struct RTPS_FilterPluginWriterFilter *writer_filter,
        struct DDS_RouteFilterEntry *route_key_low,
        struct DDS_RouteFilterEntry *route_key_high,
        const struct REDA_SequenceNumber *packet_sn,
        RTPS_FilterPlugin_for_each_peerFunc send_gap_func)
{
    struct DDS_RouteFilterEntry *route_entry = NULL;
    DB_Cursor_T cursor = NULL;
    DB_ReturnCode_T dbrc;
    struct REDA_SequenceNumber prev_sn;
    RTI_BOOL retval;

    prev_sn = *packet_sn;
    REDA_SequenceNumber_minusminus(&prev_sn);

    dbrc = DB_Table_select_range(writer_filter->route_table,
                                 DB_TABLE_DEFAULT_INDEX,
                                 &cursor,
                                 (DB_Key_T)route_key_low,
                                 (DB_Key_T)route_key_high);
    if (dbrc != DB_RETCODE_OK)
    {
        DDS_FILTER_LOG_RECORD_SELECT(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_ROUTE_RECORD,
                                     dbrc)
        return RTI_FALSE;
    }

    dbrc = DB_Cursor_get_next(cursor, (DB_Record_T *)&route_entry);
    while (dbrc == DB_RETCODE_OK)
    {
        struct DDS_ReaderFilterEntry *reader = route_entry->reader;

        /* If the previous sample was not filtered, then the last_sent_sn should
         * be equal to the previous sample's SN. If it is less than the previous
         * sample's SN, then we need to send a GAP to indicate that the reader
         * should not wait for the filtered samples.
         *
         * If we are resending a sample, then prev_sn will be less than the
         * last sent SN, and we should not send a GAP.
         */
        if (REDA_SequenceNumber_compare(&prev_sn, &reader->last_sent_sn) > 0)
        {
            /* Send a GAP */
            retval = send_gap_func(reader->rtps_intf,
                                   reader->rtps_peer_entry,
                                   &reader->last_sent_sn);
#if OSAPI_ENABLE_LOG
            if (!retval)
            {
                DDS_FILTER_LOG_WRITER_FILTER_SEND_GAP(OSAPI_LOGKIND_ERROR)
            }
#else
            IGNORE_RETVAL(retval);
#endif
        }

        /* Update the last sent SN for the reader if we are sending a new sample */
        if (REDA_SequenceNumber_compare(packet_sn, &reader->last_sent_sn) > 0)
        {
            /* Update the last sent SN for this reader */
            reader->last_sent_sn = *packet_sn;
        }

        dbrc = DB_Cursor_get_next(cursor, (DB_Record_T *)&route_entry);
    }
    DB_Cursor_finish(writer_filter->route_table, cursor);

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDS_FILTER_LOG_RECORD_SELECT(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_ROUTE_RECORD,
                                     dbrc)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_PRIVATE RTI_BOOL
DDS_FilterPluginImpl_writer_serialize_filter_info(
        struct RTPS_FilterPluginWriterFilter *writer_filter,
        NETIO_Packet_T *packet,
        DDS_UnsignedLong *result_bitmask,
        struct DDS_FilterSignature *signatures,
        RTI_INT32 sig_count)
{
    struct DDS_ContentFilterInfo filter_info = DDS_ContentFilterInfo_INITIALIZER;

    if (!CDR_UnsignedLongSeq_loan_contiguous(&filter_info.filter_result,
                                             result_bitmask,
                                             (sig_count + 31) / 32,
                                             RTPS_CONTENT_FILTER_MAX_BITMASK_COUNT))
    {
        return RTI_FALSE;
    }
    if (!DDS_FilterSignatureSeq_loan_contiguous(&filter_info.filter_signatures,
                                                signatures,
                                                sig_count,
                                                RTPS_CONTENT_FILTER_MAX_SIGNATURE_COUNT))
    {
        return RTI_FALSE;
    }
    if (!DDS_FilterPluginImpl_serialize_content_filter_info(
            writer_filter, &filter_info, packet))
    {
        DDS_FILTER_LOG_WRITER_SERIALIZE_INLINE_QOS(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
DDS_FilterPluginImpl_writer_apply_filter(
        struct RTPS_FilterPlugin *plugin,
        struct RTPS_FilterPluginWriterFilter *writer_filter,
        struct NETIO_Address *dest_address,
        NETIO_Packet_T *packet,
        RTPS_FilterPlugin_for_each_peerFunc send_gap_func,
        RTI_BOOL *drop_sample_out)
{
    struct DDS_RouteFilterEntry route_key_low;
    struct DDS_RouteFilterEntry route_key_high;

    DDS_UnsignedLong result_bitmask[RTPS_CONTENT_FILTER_MAX_BITMASK_COUNT] = {0};
    struct DDS_FilterSignature signatures[RTPS_CONTENT_FILTER_MAX_SIGNATURE_COUNT];
    RTI_INT32 sig_count = 0;

    OSAPI_PRECONDITION((plugin == NULL) || (writer_filter == NULL) ||
                       (dest_address == NULL) || (packet == NULL) ||
                       (send_gap_func == NULL) || (drop_sample_out == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("plugin", plugin, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("writer_filter", writer_filter, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("dest_address", dest_address, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("packet", packet, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("drop_sample_out", drop_sample_out, RTI_TRUE);)

    PRECOND_ARG(plugin)

    route_key_low.dest_address = *dest_address;
    route_key_low.reader = (void *)0;
    route_key_high.dest_address = *dest_address;
    route_key_high.reader = (void *)-1;

    /* Filter info should only be included with samples which contain valid data,
     * but we may still need to send GAPs for samples without valid data.
     */
    if (packet->info.valid_data)
    {
        RTI_BOOL send_data = RTI_FALSE;

        if (!DDS_FilterPluginImpl_writer_construct_filter_info(
                writer_filter,
                &route_key_low,
                &route_key_high,
                &packet->info.sn,
                result_bitmask,
                signatures,
                &sig_count,
                &send_data))
        {
            return RTI_FALSE;
        }

        /* Break if this sample can be dropped */
        if (!send_data)
        {
            *drop_sample_out = RTI_TRUE;
            return RTI_TRUE;
        }
    }

    /* Determine if any gaps need to be sent */
    if (!DDS_FilterPluginImpl_writer_send_gaps(
            writer_filter,
            &route_key_low,
            &route_key_high,
            &packet->info.sn,
            send_gap_func))
    {
        return RTI_FALSE;
    }

    /* If there are no signatures, then we do not need to serialize the content filter info */
    if (sig_count == 0)
    {
        *drop_sample_out = RTI_FALSE;
        return RTI_TRUE;
    }

    /* Serialize the content filter info */
    if (!DDS_FilterPluginImpl_writer_serialize_filter_info(
            writer_filter,
            packet,
            result_bitmask,
            signatures,
            sig_count))
    {
        return RTI_FALSE;
    }

    *drop_sample_out = RTI_FALSE;
    return RTI_TRUE;
}

RTI_BOOL
DDS_FilterPluginImpl_for_each_reliable_peer(
        struct RTPS_FilterPlugin *plugin,
        struct RTPS_FilterPluginWriterFilter *writer_filter,
        RTPS_FilterPlugin_for_each_peerFunc for_each_func)
{
    struct DDS_ReaderFilterEntry *reader_entry = NULL;
    DB_Cursor_T cursor = NULL;
    DB_ReturnCode_T dbrc;

    OSAPI_PRECONDITION((plugin == NULL) || (writer_filter == NULL) || (for_each_func == NULL),
                        return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("plugin", plugin, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("writer_filter", writer_filter, RTI_TRUE);)

    PRECOND_ARG(plugin)

    dbrc = DB_Table_select_all(writer_filter->reader_table,
                               DB_TABLE_DEFAULT_INDEX,
                               &cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        DDS_FILTER_LOG_RECORD_SELECT(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_READER_RECORD,
                                     dbrc)
        return RTI_FALSE;
    }

    dbrc = DB_Cursor_get_next(cursor, (DB_Record_T *)&reader_entry);
    while (dbrc == DB_RETCODE_OK)
    {
        if (reader_entry->reliable
                && (reader_entry->rtps_intf != NULL)
                && (reader_entry->rtps_peer_entry != NULL))
        {
            if (!for_each_func(reader_entry->rtps_intf,
                               reader_entry->rtps_peer_entry,
                               &reader_entry->last_sent_sn))
            {
                DB_Cursor_finish(writer_filter->reader_table, cursor);
                return RTI_FALSE;
            }
        }

        dbrc = DB_Cursor_get_next(cursor, (DB_Record_T *)&reader_entry);
    }
    DB_Cursor_finish(writer_filter->reader_table, cursor);

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDS_FILTER_LOG_RECORD_SELECT(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_READER_RECORD,
                                     dbrc)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}
