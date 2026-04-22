/*
 * FILE: dds_c_filter_plugin.h - DDS Filter Plugin definitions
 *
 * Copyright (c) 2025-2025 Real-Time Innovations,Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#ifndef dds_c_filter_plugin_h
#define dds_c_filter_plugin_h

#include "rtps/rtps_filter_plugin.h"
#include "dds_c/dds_c_content_filter.h"
#include "dds_c/dds_c_string_manager.h"

#ifdef __cplusplus
extern "C"
{
#endif

struct DDS_FilterPluginProperty
{
    struct RT_ComponentProperty _parent;

    DDS_StringManager_T *topic_string_manager;

    struct DDS_FilterResourceLimits resource_limits;

    DDS_Boolean disable_builtin_sql_filter;
};

#define DDS_FilterPluginProperty_INITIALIZER \
{ \
    RT_ComponentProperty_INITIALIZER, \
    NULL, \
    DDS_FilterResourceLimits_INITIALIZER, \
    DDS_BOOLEAN_FALSE \
}

/* Forward-declaration of an opaque filter plugin */
struct DDS_FilterPlugin;

/* Forward-declaration of an opaque ContentFilterI */
struct DDS_ContentFilterI;

/* Forward-declaration of an opaque ContentFilterCompiledFilter */
struct DDS_ContentFilterCompiledFilter;

/* ========================================================================== */
/*                          Memory management functions                       */
/* ========================================================================== */

FUNCTION_MUST_TYPEDEF(
DDS_ReturnCode_t
(*DDS_FilterPlugin_filter_qos_shallow_copyFunc)(
        struct DDS_FilterPlugin *plugin,
        struct DDS_ContentFilterQosPolicy *out,
        const struct DDS_ContentFilterQosPolicy *in))

typedef void
(*DDS_FilterPlugin_filter_qos_finalize_shallow_copyFunc)(
        struct DDS_FilterPlugin *plugin,
        struct DDS_ContentFilterQosPolicy *policy);

FUNCTION_MUST_TYPEDEF(
DDS_Boolean
(*DDS_FilterPlugin_filter_property_shallow_copyFunc)(
        struct DDS_FilterPlugin *plugin,
        struct DDS_ContentFilterProperty *out,
        const struct DDS_ContentFilterProperty *in))

typedef void
(*DDS_FilterPlugin_filter_property_finalize_shallow_copyFunc)(
        struct DDS_FilterPlugin *plugin,
        struct DDS_ContentFilterProperty *prop);

FUNCTION_MUST_TYPEDEF(
DDS_Boolean
(*DDS_FilterPlugin_filter_property_copy_from_filterFunc)(
        struct DDS_FilterPlugin *plugin,
        DDS_DataReader *reader,
        struct DDS_ContentFilterCompiledFilter *compiled_filter,
        struct DDS_ContentFilterProperty *out))

FUNCTION_MUST_TYPEDEF(
RTI_UINT32
(*DDS_FilterPlugin_filter_property_get_max_serialized_sizeFunc)(
        struct DDS_FilterPlugin *plugin,
        struct DDS_DomainParticipantQos *dp_qos,
        RTI_UINT32 size))

FUNCTION_MUST_TYPEDEF(
DDS_Boolean
(*DDS_FilterPlugin_filter_property_serializeFunc)(
        const struct DDS_FilterPlugin *self,
        struct CDR_Stream_t *stream,
        const DDS_DataReader *reader,
        const struct DDS_ContentFilterCompiledFilter *compiled_filter))

FUNCTION_MUST_TYPEDEF(
DDS_Boolean
(*DDS_FilterPlugin_filter_property_deserializeFunc)(
        struct DDS_FilterPlugin *plugin,
        struct CDR_Stream_t *stream,
        struct DDS_ContentFilterProperty *out))

/* ========================================================================== */
/*                       Participant filtering functions                      */
/* ========================================================================== */

FUNCTION_MUST_TYPEDEF(
DDS_ReturnCode_t
(*DDS_FilterPlugin_register_filter_classFunc)(
        struct DDS_FilterPlugin *plugin,
        const char *filter_class,
        const struct DDS_ContentFilterI *filter_intf,
        const void *filter_property))

FUNCTION_MUST_TYPEDEF(
DDS_ReturnCode_t
(*DDS_FilterPlugin_unregister_filter_classFunc)(
        struct DDS_FilterPlugin *plugin,
        const char *filter_class))

/* ========================================================================== */
/*                          Reader filtering functions                        */
/* ========================================================================== */

FUNCTION_MUST_TYPEDEF(
DDS_Boolean
(*DDS_FilterPlugin_reader_compileFunc)(
        struct DDS_FilterPlugin *plugin,
        DDS_DataReader *reader,
        const struct DDS_ContentFilterQosPolicy *filter_qos,
        struct DDS_ContentFilterCompiledFilter **compiled_filter_inout))

typedef void
(*DDS_FilterPlugin_reader_finalizeFunc)(
        struct DDS_FilterPlugin *plugin,
        struct DDS_ContentFilterCompiledFilter *compiled_filter);

FUNCTION_MUST_TYPEDEF(
DDS_Boolean
(*DDS_FilterPlugin_reader_process_filter_infoFunc)(
        struct DDS_FilterPlugin *plugin,
        struct DDS_ContentFilterCompiledFilter *compiled_filter,
        struct CDR_Stream_t *stream,
        DDS_Boolean *filtered_out,
        DDS_Boolean *result_out))

FUNCTION_MUST_TYPEDEF(
DDS_Boolean
(*DDS_FilterPlugin_reader_evaluateFunc)(
        struct DDS_FilterPlugin *plugin,
        struct DDS_ContentFilterCompiledFilter *compiled_filter,
        const void *sample,
        DDS_Boolean *sample_dropped_out))

/* ========================================================================== */
/*                         Writer filtering functions                         */
/* ========================================================================== */

FUNCTION_MUST_TYPEDEF(
DDS_Boolean
(*DDS_FilterPlugin_writer_attachFunc)(
        struct DDS_FilterPlugin *plugin,
        DDS_DataWriter *writer,
        DDS_UnsignedLong max_remote_reader_filters,
        DDS_UnsignedLong max_remote_readers,
        DDS_UnsignedLong max_routes_per_reader,
        DDS_UnsignedLong max_samples,
        struct RTPS_FilterPluginWriterFilter **writer_filter_out))

typedef void
(*DDS_FilterPlugin_writer_detachFunc)(
        struct DDS_FilterPlugin *plugin,
        struct RTPS_FilterPluginWriterFilter *writer_filter);

FUNCTION_MUST_TYPEDEF(
DDS_Boolean
(*DDS_FilterPlugin_writer_add_remote_readerFunc)(
        struct DDS_FilterPlugin *plugin,
        struct RTPS_FilterPluginWriterFilter *writer_filter,
        const DDS_BuiltinTopicKey_t *reader_key,
        const struct DDS_ContentFilterProperty *filter_property,
        DDS_Boolean reliable))

FUNCTION_MUST_TYPEDEF(
DDS_Boolean
(*DDS_FilterPlugin_writer_add_local_readerFunc)(
        struct DDS_FilterPlugin *plugin,
        struct RTPS_FilterPluginWriterFilter *writer_filter,
        const DDS_BuiltinTopicKey_t *reader_key,
        struct DDS_ContentFilterCompiledFilter *reader_filter,
        DDS_Boolean reliable))

typedef void
(*DDS_FilterPlugin_writer_remove_readerFunc)(
        struct DDS_FilterPlugin *plugin,
        struct RTPS_FilterPluginWriterFilter *writer_filter,
        const DDS_BuiltinTopicKey_t *reader_key);

FUNCTION_MUST_TYPEDEF(
DDS_Boolean
(*DDS_FilterPlugin_writer_evaluateFunc)(
        struct DDS_FilterPlugin *plugin,
        struct RTPS_FilterPluginWriterFilter *writer_filter,
        const struct REDA_SequenceNumber *sn,
        DDS_Boolean unregister_or_dispose,
        const void *sample))

FUNCTION_MUST_TYPEDEF(
DDS_Boolean
(*DDS_FilterPlugin_writer_apply_reader_filterFunc)(
        struct DDS_FilterPlugin *plugin,
        struct RTPS_FilterPluginWriterFilter *writer_filter,
        const struct NETIO_Guid *reader_guid,
        const struct REDA_SequenceNumber *sn,
        DDS_Boolean *sample_dropped_out))

struct DDS_FilterPluginI
{
    struct RTPS_FilterPluginI _parent;

    /* Memory management functions */
    DDS_FilterPlugin_filter_qos_shallow_copyFunc filter_qos_policy_shallow_copy;
    DDS_FilterPlugin_filter_qos_finalize_shallow_copyFunc filter_qos_policy_finalize_shallow_copy;
    DDS_FilterPlugin_filter_property_shallow_copyFunc filter_property_shallow_copy;
    DDS_FilterPlugin_filter_property_finalize_shallow_copyFunc filter_property_finalize_shallow_copy;
    DDS_FilterPlugin_filter_property_copy_from_filterFunc filter_property_copy_from_filter;
    DDS_FilterPlugin_filter_property_get_max_serialized_sizeFunc filter_property_get_max_serialized_size;
    DDS_FilterPlugin_filter_property_serializeFunc filter_property_serialize;
    DDS_FilterPlugin_filter_property_deserializeFunc filter_property_deserialize;

    /* Participant filtering functions */
    DDS_FilterPlugin_register_filter_classFunc register_filter_class;
    DDS_FilterPlugin_unregister_filter_classFunc unregister_filter_class;

    /* Reader filtering functions */
    DDS_FilterPlugin_reader_compileFunc reader_compile;
    DDS_FilterPlugin_reader_finalizeFunc reader_finalize;
    DDS_FilterPlugin_reader_process_filter_infoFunc reader_process_filter_info;
    DDS_FilterPlugin_reader_evaluateFunc reader_evaluate;

    /* Writer filtering functions */
    DDS_FilterPlugin_writer_attachFunc writer_attach;
    DDS_FilterPlugin_writer_detachFunc writer_detach;
    DDS_FilterPlugin_writer_add_remote_readerFunc writer_add_remote_reader;
    DDS_FilterPlugin_writer_add_local_readerFunc writer_add_local_reader;
    DDS_FilterPlugin_writer_remove_readerFunc writer_remove_reader;
    DDS_FilterPlugin_writer_evaluateFunc writer_evaluate;
    DDS_FilterPlugin_writer_apply_reader_filterFunc writer_apply_reader_filter;
};

#define DDS_FilterPlugin_filter_qos_shallow_copy(plugin_, out_, in_) \
    ((struct DDS_FilterPluginI *)(((struct RT_Component *)(plugin_))->_intf))-> \
        filter_qos_policy_shallow_copy((plugin_), (out_), (in_))

#define DDS_FilterPlugin_filter_qos_finalize_shallow_copy(plugin_, policy_) \
    ((struct DDS_FilterPluginI *)(((struct RT_Component *)(plugin_))->_intf))-> \
        filter_qos_policy_finalize_shallow_copy((plugin_), (policy_))

#define DDS_FilterPlugin_filter_property_shallow_copy(plugin_, out_, in_) \
    ((struct DDS_FilterPluginI *)(((struct RT_Component *)(plugin_))->_intf))-> \
        filter_property_shallow_copy((plugin_), (out_), (in_))

#define DDS_FilterPlugin_filter_property_finalize_shallow_copy(plugin_, prop_) \
    ((struct DDS_FilterPluginI *)(((struct RT_Component *)(plugin_))->_intf))-> \
        filter_property_finalize_shallow_copy((plugin_), (prop_))

#define DDS_FilterPlugin_filter_property_copy_from_filter(plugin_, reader_, compiled_filter_, out_) \
    ((struct DDS_FilterPluginI *)(((struct RT_Component *)(plugin_))->_intf))-> \
        filter_property_copy_from_filter((plugin_), (reader_), (compiled_filter_), (out_))

#define DDS_FilterPlugin_filter_property_get_max_serialized_size(plugin_, dp_qos_, size_) \
    ((struct DDS_FilterPluginI *)(((struct RT_Component *)(plugin_))->_intf))-> \
        filter_property_get_max_serialized_size((plugin_), (dp_qos_), (size_))

#define DDS_FilterPlugin_filter_property_serialize(plugin_, stream_, reader_, compiled_filter_) \
    ((struct DDS_FilterPluginI *)(((struct RT_Component *)(plugin_))->_intf))-> \
        filter_property_serialize((plugin_), (stream_), (reader_), (compiled_filter_))

#define DDS_FilterPlugin_filter_property_deserialize(plugin_, stream_, out_) \
    ((struct DDS_FilterPluginI *)(((struct RT_Component *)(plugin_))->_intf))-> \
        filter_property_deserialize((plugin_), (stream_), (out_))

#define DDS_FilterPlugin_register_filter_class(plugin_, filter_class_, filter_intf_, filter_property_) \
    ((struct DDS_FilterPluginI *)(((struct RT_Component *)(plugin_))->_intf))-> \
        register_filter_class((plugin_), (filter_class_), (filter_intf_), (filter_property_))

#define DDS_FilterPlugin_unregister_filter_class(plugin_, filter_class_) \
    ((struct DDS_FilterPluginI *)(((struct RT_Component *)(plugin_))->_intf))-> \
        unregister_filter_class((plugin_), (filter_class_))

#define DDS_FilterPlugin_reader_compile(plugin_, reader_, filter_qos_, compiled_filter_out_) \
    ((struct DDS_FilterPluginI *)(((struct RT_Component *)(plugin_))->_intf))-> \
        reader_compile((plugin_), (reader_), (filter_qos_), (compiled_filter_out_))

#define DDS_FilterPlugin_reader_finalize(plugin_, compiled_filter_) \
    ((struct DDS_FilterPluginI *)(((struct RT_Component *)(plugin_))->_intf))-> \
        reader_finalize((plugin_), (compiled_filter_))

#define DDS_FilterPlugin_reader_process_filter_info(plugin_, compiled_filter_, stream_, filtered_out_, result_out_) \
    ((struct DDS_FilterPluginI *)(((struct RT_Component *)(plugin_))->_intf))-> \
        reader_process_filter_info((plugin_), (compiled_filter_), (stream_), (filtered_out_), (result_out_))

#define DDS_FilterPlugin_reader_evaluate(plugin_, compiled_filter_, sample_, sample_dropped_out_) \
    ((struct DDS_FilterPluginI *)(((struct RT_Component *)(plugin_))->_intf))-> \
        reader_evaluate((plugin_), (compiled_filter_), (sample_), (sample_dropped_out_))

#define DDS_FilterPlugin_writer_attach(plugin_, writer_, max_remote_reader_filters_, max_remote_readers_, max_routes_per_reader_, max_samples_, writer_filter_out_) \
    ((struct DDS_FilterPluginI *)(((struct RT_Component *)(plugin_))->_intf))-> \
        writer_attach((plugin_), (writer_), (max_remote_reader_filters_), (max_remote_readers_), (max_routes_per_reader_), (max_samples_), (writer_filter_out_))

#define DDS_FilterPlugin_writer_detach(plugin_, writer_filter_) \
    ((struct DDS_FilterPluginI *)(((struct RT_Component *)(plugin_))->_intf))-> \
        writer_detach((plugin_), (writer_filter_))

#define DDS_FilterPlugin_writer_add_remote_reader(plugin_, writer_filter_, reader_key_, filter_property_, reliable_) \
    ((struct DDS_FilterPluginI *)(((struct RT_Component *)(plugin_))->_intf))-> \
        writer_add_remote_reader((plugin_), (writer_filter_), (reader_key_), (filter_property_), (reliable_))

#define DDS_FilterPlugin_writer_add_local_reader(plugin_, writer_filter_, reader_key_, reader_filter_, reliable_) \
    ((struct DDS_FilterPluginI *)(((struct RT_Component *)(plugin_))->_intf))-> \
        writer_add_local_reader((plugin_), (writer_filter_), (reader_key_), (reader_filter_), (reliable_))

#define DDS_FilterPlugin_writer_remove_reader(plugin_, writer_filter_, reader_key_) \
    ((struct DDS_FilterPluginI *)(((struct RT_Component *)(plugin_))->_intf))-> \
        writer_remove_reader((plugin_), (writer_filter_), (reader_key_))

#define DDS_FilterPlugin_writer_evaluate(plugin_, writer_filter_, sn_, unregister_or_dispose_, sample_) \
    ((struct DDS_FilterPluginI *)(((struct RT_Component *)(plugin_))->_intf))-> \
        writer_evaluate((plugin_), (writer_filter_), (sn_), (unregister_or_dispose_), (sample_))

#define DDS_FilterPlugin_writer_apply_reader_filter(plugin_, writer_filter_, reader_guid_, sn_, sample_dropped_out_) \
    ((struct DDS_FilterPluginI *)(((struct RT_Component *)(plugin_))->_intf))-> \
        writer_apply_reader_filter((plugin_), (writer_filter_), (reader_guid_), (sn_), (sample_dropped_out_))

/*ci
 * \brief Wrapper to call of the DDS_FilterPluginFactory->create_component
 *
 * \param[in] f_  The filter plugin factory
 * \param[in] p_  The property
 * \param[in] l_  The listener
 *
 * \return A reference to new writer cache on success, NULL otherwise
 */
#define DDS_FilterPluginFactory_create_component(f_,p_,l_) \
    (struct DDS_FilterPlugin*)((f_)->intf)->create_component(f_,p_,l_)

#ifndef RTI_CERT
/*ci
 * \brief Wrapper to call of the DDS_FilterPluginFactory->delete_component
 *
 * \param[in] f_  The filter plugin factory
 * \param[in] c_  The plugin to delete
 */
#define DDS_FilterPluginFactory_delete_component(f_,c_) \
    ((f_)->intf)->delete_component(f_,(RT_Component_T*)(c_))
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* dds_c_filter_plugin_h */
