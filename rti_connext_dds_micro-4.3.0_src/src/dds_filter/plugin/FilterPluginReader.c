/*
 * FILE: FilterPluginReader.c - Filter plugin reader implementation
 *
 * Copyright (c) 2025-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#include "dds_c/dds_c_infrastructure.h"
#include "dds_c/dds_c_subscription.h"

#include "FilterPluginReader.h"
#include "FilterPlugin.h"

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Get the content filtered topic name for a DataReader
 *
 * \details The content filtered topic name is used only for interoperability
 *          with the RTPS specification. Following how pro constructs the
 *          name when using XML app creation, the name is created by
 *          combining the related topic name and the filter name.
 *
 * \param[in]  plugin The reader's filter plugin
 * \param[in]  related_topic_name The name of the related topic
 * \param[in]  filter_name The name of the filter
 * \param[out] cft_name_out The output buffer for the content filter topic name
 *
 * \return A unique name for the reader's content filter topic or NULL if
 *         the name could not be created.
 */
RTI_PRIVATE DDS_Boolean
DDS_FilterPluginImpl_get_cft_name(
        struct DDS_FilterPlugin *plugin,
        DDS_String related_topic_name,
        DDS_String filter_name,
        DDS_String cft_name_out)
{
    char *buf = cft_name_out;
    RTI_SIZE_T related_topic_name_length = OSAPI_String_length(related_topic_name);
    RTI_SIZE_T filter_name_length = OSAPI_String_length(filter_name);

    UNUSED_ARG(plugin);

    if (related_topic_name_length + filter_name_length + 2 > RTPS_PATHNAME_LEN_MAX)
    {
        /* The name is too long */
        return DDS_BOOLEAN_FALSE;
    }

    /* Create the name "<related_topic_name>::<filter_name>" */
    OSAPI_Memory_copy(buf, related_topic_name, related_topic_name_length);
    buf += related_topic_name_length;

    *buf++ = ':';
    *buf++ = ':';

    OSAPI_Memory_copy(buf, filter_name, filter_name_length);
    buf += filter_name_length;

    *buf = '\0';

    return DDS_BOOLEAN_TRUE;
}

DDS_Boolean
DDS_FilterPluginImpl_reader_compile(
        struct DDS_FilterPlugin *plugin,
        DDS_DataReader *reader,
        const struct DDS_ContentFilterQosPolicy *filter_qos,
        struct DDS_ContentFilterCompiledFilter **compiled_filter_inout)
{
    char cft_name[RTPS_PATHNAME_LEN_MAX + 1];
    struct DDS_FilterSignature signature = DDS_FilterSignature_INITIALIZER;
    DDS_TopicDescription* topic_description;
    DDS_String related_topic_name;
    struct DDS_TypeCode *type_code;
    DDS_Boolean result = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION(
            (plugin == NULL) || (reader == NULL) || (filter_qos == NULL) || (compiled_filter_inout == NULL),
            return DDS_BOOLEAN_FALSE,
            OSAPI_Log_entry_add_pointer("plugin", plugin, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("reader", reader, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("filter_qos", filter_qos, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("compiled_filter_inout", compiled_filter_inout, RTI_TRUE);)

    /* Calculate filter signature */
    topic_description = DDS_DataReader_get_topicdescription(reader);
    related_topic_name = (DDS_String)DDS_TopicDescription_get_name(topic_description);
    if (!DDS_FilterPluginImpl_get_cft_name(plugin, related_topic_name, filter_qos->filter_name, cft_name))
    {
        DDS_FILTER_LOG_CONTENT_FILTER_NAME_TOO_LONG(OSAPI_LOGKIND_ERROR,
                                                    related_topic_name,
                                                    filter_qos->filter_name)
        goto done;
    }
    if (!DDS_FilterSignature_calculate(
            &signature,
            cft_name,
            related_topic_name,
            filter_qos->filter_class_name,
            filter_qos->filter_expression,
            &filter_qos->expression_parameters))
    {
        goto done;
    }

#if DDS_XTYPES_IS_ENABLED
    type_code = DDS_DataReader_get_typecode(reader);
    if (type_code == NULL)
    {
        DDS_FILTER_LOG_TYPE_CODE_NOT_FOUND(OSAPI_LOGKIND_ERROR,
                                           DDS_TopicDescription_get_type_name(topic_description))
        goto done;
    }
#else
    type_code = NULL;
#endif

    if (!DDS_FilterPluginImpl_compile_filter(plugin,
                                             &signature,
                                             cft_name,
                                             filter_qos->filter_class_name,
                                             filter_qos->filter_expression,
                                             &filter_qos->expression_parameters,
                                             type_code,
                                             DDS_TopicDescription_get_type_name(topic_description),
                                             compiled_filter_inout))
    {
        /* Failed to compile */
        goto done;
    }

    result = DDS_BOOLEAN_TRUE;
done:
    return result;
}

RTI_PRIVATE RTI_BOOL
DDS_FilterPluginImpl_deserialize_filter_signature(struct CDR_Stream_t *stream,
                                                void *sample,
                                                void *param)
{
    DDS_FilterSignature_t *filter_signature = (DDS_FilterSignature_t *)sample;

    UNUSED_ARG(param);

    if (!CDR_Stream_deserialize_primitive_array(stream,
                                                filter_signature->value,
                                                16, CDR_OCTET_TYPE))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

DDS_Boolean
DDS_FilterPluginImpl_reader_process_filter_info(
        struct DDS_FilterPlugin *plugin,
        struct DDS_ContentFilterCompiledFilter *compiled_filter,
        struct CDR_Stream_t *stream,
        DDS_Boolean *filtered_out,
        DDS_Boolean *result_out)
{
    struct DDS_ContentFilterInfo filter_info = DDS_ContentFilterInfo_INITIALIZER;
    DDS_UnsignedLong filter_result[RTPS_CONTENT_FILTER_MAX_BITMASK_COUNT];
    DDS_FilterSignature_t signatures[RTPS_CONTENT_FILTER_MAX_SIGNATURE_COUNT];
    RTI_INT32 sig_len;

    OSAPI_PRECONDITION(
            (plugin == NULL) || (compiled_filter == NULL) || (stream == NULL)
                    || (filtered_out == NULL) || (result_out == NULL),
            return DDS_BOOLEAN_FALSE,
            OSAPI_Log_entry_add_pointer("plugin", plugin, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("compiled_filter", compiled_filter, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("stream", stream, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("filtered_out", filtered_out, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("result_out", result_out, RTI_TRUE);)

    PRECOND_ARG(plugin)

    *filtered_out = DDS_BOOLEAN_FALSE;

    /* Setup memory for deserializing the sequences */
    if (!CDR_UnsignedLongSeq_loan_contiguous(
            &filter_info.filter_result,
            filter_result,
            0,
            RTPS_CONTENT_FILTER_MAX_BITMASK_COUNT))
    {
        return DDS_BOOLEAN_FALSE;
    }
    if (!DDS_FilterSignatureSeq_loan_contiguous(
                &filter_info.filter_signatures,
                signatures,
                0,
                RTPS_CONTENT_FILTER_MAX_SIGNATURE_COUNT))
    {
        return DDS_BOOLEAN_FALSE;
    }

    /* Deserialize the filter info */
    if (!CDR_Stream_deserialize_primitive_sequence(
        stream, (struct REDA_Sequence *)&filter_info.filter_result, CDR_UNSIGNED_LONG_TYPE))
    {
        return DDS_BOOLEAN_FALSE;
    }
    if (!CDR_Stream_deserialize_non_primitive_sequence(
            stream, (struct REDA_Sequence *)&filter_info.filter_signatures,
            DDS_FilterPluginImpl_deserialize_filter_signature))
    {
        return DDS_BOOLEAN_FALSE;
    }

    /* Check if our signature is in the filter info */
    sig_len = DDS_FilterSignatureSeq_get_length(&filter_info.filter_signatures);
    for (RTI_INT32 i = 0; i < sig_len; i++)
    {
        struct DDS_FilterSignature *sig = DDS_FilterSignatureSeq_get_reference(&filter_info.filter_signatures, i);

        if (DDS_FilterSignature_compare(sig, &compiled_filter->filter_signature) == 0)
        {
            /* Found a match */
            RTI_UINT8 bitmask_index = (RTI_UINT8)(i / 32);
            RTI_UINT8 bitmask_offset = (RTI_UINT8)(31 - (i % 32));
            RTI_UINT32 *bitmask;

            bitmask = DDS_UnsignedLongSeq_get_reference(&filter_info.filter_result, bitmask_index);
            if (bitmask == NULL)
            {
                break;
            }

            *filtered_out = DDS_BOOLEAN_TRUE;
            *result_out = (*bitmask & (((RTI_UINT32)1) << bitmask_offset)) != 0;

            break;
        }
    }

    return DDS_BOOLEAN_TRUE;
}

DDS_Boolean
DDS_FilterPluginImpl_reader_evaluate(
        struct DDS_FilterPlugin *plugin,
        struct DDS_ContentFilterCompiledFilter *compiled_filter,
        const void *sample,
        DDS_Boolean *sample_dropped_out)
{
    DDS_Boolean filter_result = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION(
            (plugin == NULL) || (compiled_filter == NULL)
                || (sample == NULL) || (sample_dropped_out == NULL),
            return DDS_BOOLEAN_FALSE,
            OSAPI_Log_entry_add_pointer("plugin", plugin, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("compiled_filter", compiled_filter, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("sample", sample, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("sample_dropped_out", sample_dropped_out, RTI_TRUE);)

    PRECOND_ARG(plugin)

    /* Evaluate the filter using the filter class */
    if (!compiled_filter->filter_class->intf->evaluate(
                compiled_filter->filter_class->instance,
                compiled_filter->compiled_filter_instance,
                sample,
                &filter_result))
    {
        DDS_FILTER_LOG_FILTER_EVALUATE(OSAPI_LOGKIND_ERROR, compiled_filter->filter_class->name)
        return DDS_BOOLEAN_FALSE;
    }

    *sample_dropped_out = !filter_result;

    return DDS_BOOLEAN_TRUE;
}
