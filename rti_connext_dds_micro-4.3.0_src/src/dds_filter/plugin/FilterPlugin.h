/*
 * FILE: FilterPlugin.h - Filter plugin definitions
 *
 * Copyright (c) 2025-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#ifndef FilterPlugin_h
#define FilterPlugin_h

#include "dds_c/dds_c_filter_plugin.h"
#include "dds_filter/dds_filter_content_filter.h"

#include "FilterPluginFactory.h"
#include "../filter/FilterSignature.h"
#include "../filter/ContentFilterInfo.h"

#ifdef __cplusplus
extern "C"
{
#endif

struct DDS_FilterPlugin
{
    struct RT_Component _parent;

    struct DDS_FilterPluginFactory *factory;

    struct DDS_FilterPluginProperty property;

    DDS_StringManager_T *filter_class_string_manager;

    DDS_StringManager_T *filter_expression_string_manager;

    DDS_StringManager_T *filter_parameter_string_manager;

    DB_Table_T filter_class_table;

    DB_Table_T compiled_filter_table;

#ifndef RTI_CERT
    /*ci \brief The number of writers using the filter plugin
     *
     *   \details This count is only necessary for preventing the filter plugin
     *            from being finalized while in use by a writer and is therefore
     *            unnecessary for CERT.
     */
    RTI_INT32 writer_count;
#endif /* !RTI_CERT */

    RTI_UINT32 compiled_filter_count;
};

/*ci
 * \brief Content filter class entry on a filter plugin
 */
struct DDS_ContentFilterClass
{
    /*ci
     * \brief The name of the filter class
     */
    DDS_String name;

    /*ci
     * \brief The content filter interface for this filter class
     */
    const struct DDS_ContentFilterI *intf;

    /*ci
     * \brief The instance of the filter class
     */
    void *instance;

    /*ci
     * \brief Reference count for the filter class. The filter class can only be
     *        unregistered when the reference count is 0.
     */
    RTI_UINT32 ref_count;
};

/*ci
 * \brief Instance of a compiled content filter stored in the filter plugin
 */
struct DDS_ContentFilterCompiledFilter
{
    /*ci
     * \brief The filter signature for this compiled filter and the key which
     *        uniquely identifies the compiled filter in the filter plugin.
     */
    struct DDS_FilterSignature filter_signature;

    /*ci
     * \brief The name of the content filtered topic the filter is compiled for.
     *        This name is used only for interoperability with RTPS standard.
     */
    DDS_String cft_name;

    /*ci
     * \brief The filter class for this compiled filter
     */
    struct DDS_ContentFilterClass *filter_class;

    /*ci
     * \brief The instance of the compiled filter
     */
    void *compiled_filter_instance;

    /*ci
     * \brief Reference count for the compiled filter.
     */
    RTI_UINT32 ref_count;
};

/*ci
 * \brief Initialize a new instance of the Filter plugin
 *
 * \param[in] self      The new Filter plugin instance to initialize
 * \param[in] factory   Factory initialing the new Filter plugin
 * \param[in] property  The property to use to initialize
 * \param[in] listener  The plugin listener
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
extern MUST_CHECK_RETURN RTI_BOOL
DDS_FilterPluginImpl_initialize(
        struct DDS_FilterPlugin *self,
        struct DDS_FilterPluginFactory *factory,
        const struct DDS_FilterPluginProperty *const property);

#ifndef RTI_CERT
/*ci
 * \brief Finalize a Filter plugin instance.
 *
 * \param[in] self The Filter plugin instance to finalize
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
extern RTI_BOOL
DDS_FilterPluginImpl_finalize(struct DDS_FilterPlugin *self);
#endif /* !RTI_CERT */

/*ci
 * \brief Register a new filter class with the filter plugin
 *
 * \param[in] self The filter plugin to register the filter class with
 * \param[in] name The name of the filter class to register
 * \param[in] filter_intf The content filter interface for the filter class
 * \param[in] filter_property The properties for the filter class
 *
 * \return DDS_RETCODE_OK on success, an error code on failure
 */
extern DDS_ReturnCode_t
DDS_FilterPluginImpl_register_filter_class(
        struct DDS_FilterPlugin *self,
        const char *name,
        const struct DDS_ContentFilterI *filter_intf,
        const void *filter_property);

/*ci
 * \brief Compile a filter using the content filter class
 *
 * \param[in] self The filter plugin to compile the filter for
 * \param[in] signature The filter signature to use as a key for the compiled filter
 * \param[in] cft_name The content filtered topic name for the compiled filter
 * \param[in] filter_class_name The name of the filter class to use for the compiled filter
 * \param[in] filter_expression The filter expression to compile
 * \param[in] expression_parameters The parameters for the filter expression
 * \param[in] type_code The type code of the data the filter is compiled for
 * \param[in] type_class_name The registered type name of the data type
 * \param[inout] filter_inout A pointer to a pointer to a previously compiled filter
 *               that should be replaced or NULL. If the filter is successfully
 *               compiled, then any previous filter will be finalized and this
 *               pointer will be set to the newly compiled filter.
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure.
 */
extern DDS_Boolean
DDS_FilterPluginImpl_compile_filter(
        struct DDS_FilterPlugin *self,
        struct DDS_FilterSignature *signature,
        DDS_String cft_name,
        DDS_String filter_class_name,
        DDS_String filter_expression,
        const struct DDS_StringSeq *expression_parameters,
        struct DDS_TypeCode *type_code,
        const char *type_class_name,
        struct DDS_ContentFilterCompiledFilter **filter_inout);

/*ci
 * \brief Release the resources allocated for a previously compiled filter
 *
 * \param[in] plugin The filter plugin to finalize the filter for
 * \param[in] compiled_filter The compiled filter to finalize
 */
extern void
DDS_FilterPluginImpl_finalize_filter(
        struct DDS_FilterPlugin *plugin,
        struct DDS_ContentFilterCompiledFilter *compiled_filter);

#ifdef __cplusplus
}
#endif

#endif /* FilterPlugin_h */
