/*
 * FILE: ContentFilterProperty.c - Content Filter Property API implementation
 *
 * Copyright (c) 2025-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*ci
 * \brief Content Filter Property API implementation
 */

#include "rt/rt_rt.h"

#include "DomainParticipantFilter.h"
#include "DomainParticipant.h"

/*** SOURCE_BEGIN ***/

DDS_ReturnCode_t
DDS_DomainParticipant_content_filter_qos_shallow_copy(
        DDS_DomainParticipant *dp,
        struct DDS_ContentFilterQosPolicy *out,
        const struct DDS_ContentFilterQosPolicy *in)
{
    OSAPI_PRECONDITION((dp == NULL) || (out == NULL) || (in == NULL),
                       return DDS_RETCODE_BAD_PARAMETER,
                       OSAPI_Log_entry_add_pointer("dp", dp, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("out", out, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("in", in, RTI_TRUE);)

    if (!DDS_DomainParticipant_is_filtering_enabled(dp))
    {
        return DDS_RETCODE_OK;
    }

    return DDS_FilterPlugin_filter_qos_shallow_copy(dp->filter_plugin, out, in);
}

void
DDS_DomainParticipant_content_filter_qos_finalize_shallow_copy(
        DDS_DomainParticipant *dp,
        struct DDS_ContentFilterQosPolicy *policy)
{
    OSAPI_PRECONDITION((dp == NULL) || (policy == NULL),
                       return,
                       OSAPI_Log_entry_add_pointer("dp", dp, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("policy", policy, RTI_TRUE);)

    if (DDS_DomainParticipant_is_filtering_enabled(dp))
    {
        DDS_FilterPlugin_filter_qos_finalize_shallow_copy(dp->filter_plugin, policy);
    }
}

DDS_Boolean
DDS_DomainParticipant_filter_property_shallow_copy(
        DDS_DomainParticipant *dp,
        struct DDS_ContentFilterProperty *out,
        const struct DDS_ContentFilterProperty *in)
{
    OSAPI_PRECONDITION((dp == NULL) || (out == NULL) || (in == NULL),
                       return DDS_BOOLEAN_FALSE,
                       OSAPI_Log_entry_add_pointer("dp", dp, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("out", out, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("in", in, RTI_TRUE);)

    if (!DDS_DomainParticipant_is_filtering_enabled(dp))
    {
        return DDS_BOOLEAN_TRUE;
    }

    return DDS_FilterPlugin_filter_property_shallow_copy(dp->filter_plugin, out, in);
}

void
DDS_DomainParticipant_filter_property_finalize_shallow_copy(
        DDS_DomainParticipant *dp,
        struct DDS_ContentFilterProperty *prop)
{
    OSAPI_PRECONDITION((dp == NULL) || (prop == NULL),
                       return,
                       OSAPI_Log_entry_add_pointer("dp", dp, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("prop", prop, RTI_TRUE);)

    if (DDS_DomainParticipant_is_filtering_enabled(dp))
    {
        DDS_FilterPlugin_filter_property_finalize_shallow_copy(dp->filter_plugin, prop);
    }
}

RTI_UINT32
DDS_DomainParticipant_filter_property_get_max_serialized_size(
        DDS_DomainParticipant *dp,
        RTI_UINT32 size)
{
    OSAPI_PRECONDITION(dp == NULL,
                       return 0,
                       OSAPI_Log_entry_add_pointer("dp", dp, RTI_TRUE);)

    if (!DDS_DomainParticipant_is_filtering_enabled(dp))
    {
        return 0;
    }

    return DDS_FilterPlugin_filter_property_get_max_serialized_size(
                        dp->filter_plugin,
                        DDS_DomainParticipant_get_qos_ref(dp),
                        size);
}

DDS_Boolean
DDS_DomainParticipant_deserialize_content_filter_property(
        DDS_DomainParticipant *dp,
        struct CDR_Stream_t *stream,
        struct DDS_ContentFilterProperty *prop)
{
    DDS_Boolean result;

    OSAPI_PRECONDITION((dp == NULL) || (stream == NULL) || (prop == NULL),
                       return DDS_BOOLEAN_FALSE,
                       OSAPI_Log_entry_add_pointer("dp", dp, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("stream", stream, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("prop", prop, RTI_TRUE);)

    if (!DDS_DomainParticipant_is_filtering_enabled(dp))
    {
        return DDS_BOOLEAN_TRUE;
    }

    result = DDS_FilterPlugin_filter_property_deserialize(dp->filter_plugin, stream, prop);

    /* We never want to fail to deserialize a filter property because it is not
     * a critical error. We expect this to fail if the filter we receive
     * exceeds our resource limits. If it is not within our limits, we should
     * just ignore it and not apply writer filtering.
     */
#if OSAPI_ENABLE_LOG
    if (!result)
    {
        DDSC_LOG_DESERIALIZE_REMOTE_CONTENT_FILTER(OSAPI_LOGKIND_WARNING)
    }
#else
    UNUSED_ARG(result);
#endif

    return DDS_BOOLEAN_TRUE;
}

DDS_Boolean
DDS_DomainParticipant_create_filter_plugin(DDS_DomainParticipant *participant)
{
    RT_ComponentFactoryId_T default_factory_id = DDS_FILTER_PLUGIN_FACTORY_DEFAULT_ID;
    struct RT_ComponentFactory *factory = NULL;
    struct DDS_FilterPluginProperty property = DDS_FilterPluginProperty_INITIALIZER;
    DDS_Boolean result = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION((participant == NULL),
                        goto done,
                        OSAPI_Log_entry_add_pointer("participant", participant, RTI_TRUE);)

    participant->filter_plugin = NULL;

    if (RT_ComponentFactoryId_is_nil(&participant->qos.filter.name))
    {
        /* User explicitly disabled filtering on this participant */
        result = DDS_BOOLEAN_TRUE;
        goto done;
    }

    factory = RT_Registry_lookup(
                    DDS_DomainParticipantFactory_get_registry(
                            DDS_DomainParticipantFactory_get_instance()),
                    RT_ComponentFactoryId_get_name(&participant->qos.filter.name));
    if (factory == NULL)
    {
        if (RT_ComponentFactoryId_compare(&participant->qos.filter.name, &default_factory_id) == 0)
        {
            /* Failed to find the default factory, filtering must not be enabled */
            result = DDS_BOOLEAN_TRUE;
        }
#if OSAPI_ENABLE_LOG
        else
        {
            DDSC_LOG_FILTER_PLUGIN_FACTORY_NOT_FOUND(OSAPI_LOGKIND_ERROR)
        }
#endif /* OSAPI_ENABLE_LOG */
        goto done;
    }

    property._parent.db = participant->database;
    property.topic_string_manager = participant->string_manager;
    property.disable_builtin_sql_filter = participant->qos.filter.disable_builtin_sql_filter;

    property.resource_limits = participant->qos.filter.resource_limits;
    if (property.resource_limits.filter_expression_max_count == DDS_LENGTH_AUTO)
    {
        property.resource_limits.filter_expression_max_count = participant->qos.resource_limits.local_reader_allocation;

        if (!participant->qos.filter.disable_writer_filtering)
        {
            property.resource_limits.filter_expression_max_count +=  participant->qos.resource_limits.remote_reader_allocation;
        }
    }

    /* Create the filter interface */
    participant->filter_plugin = DDS_FilterPluginFactory_create_component(
                                        factory,
                                        (struct RT_ComponentProperty *)&property,
                                        NULL);
    if (participant->filter_plugin == NULL)
    {
        goto done;
    }

    result = DDS_BOOLEAN_TRUE;
done:
    return result;
}

#ifndef RTI_CERT
DDS_Boolean
DDS_DomainParticipant_finalize_filter_plugin(DDS_DomainParticipant *participant)
{
    struct RT_ComponentFactory *factory = NULL;
    DDS_Boolean result = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION((participant == NULL),
                        goto done,
                        OSAPI_Log_entry_add_pointer("participant", participant, RTI_TRUE);)

    PRECOND_ARG(participant)

    /* Nothing todo if the interface was never created */
    if (participant->filter_plugin == NULL)
    {
        result = DDS_BOOLEAN_TRUE;
        goto done;
    }

    factory = RT_Registry_lookup(
                    DDS_DomainParticipantFactory_get_registry(
                            DDS_DomainParticipantFactory_get_instance()),
                    RT_ComponentFactoryId_get_name(&participant->qos.filter.name));
    if (factory == NULL)
    {
        DDSC_LOG_FILTER_PLUGIN_FACTORY_NOT_FOUND(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    /* Finalize the filter interface */
    DDS_FilterPluginFactory_delete_component(factory, participant->filter_plugin);
    participant->filter_plugin = NULL;

    result = DDS_BOOLEAN_TRUE;
done:
    return result;
}
#endif /* !RTI_CERT */

DDS_ReturnCode_t
DDS_DomainParticipant_register_contentfilter(
        DDS_DomainParticipant *self,
        const char *filter_class_name,
        const struct DDS_ContentFilterI *filter_intf,
        const void *filter_property)
{
    DDS_ReturnCode_t retval;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (filter_class_name == NULL) || (filter_intf == NULL),
           return DDS_RETCODE_BAD_PARAMETER,
           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
           OSAPI_Log_entry_add_pointer("filter_class_name",filter_class_name,RTI_FALSE);
           OSAPI_Log_entry_add_pointer("filter_intf",filter_intf,RTI_TRUE);)

    if (self->filter_plugin == NULL)
    {
        return DDS_RETCODE_PRECONDITION_NOT_MET;
    }

    if (DB_Database_lock(self->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    retval = DDS_FilterPlugin_register_filter_class(self->filter_plugin,
                                                    filter_class_name,
                                                    filter_intf,
                                                    filter_property);

    if (DB_Database_unlock(self->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retval;
}

DDS_ReturnCode_t
DDS_DomainParticipant_unregister_contentfilter(
        DDS_DomainParticipant *self,
        const char *filter_class_name)
{
    DDS_ReturnCode_t retval;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (filter_class_name == NULL),
           return DDS_RETCODE_BAD_PARAMETER,
           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
           OSAPI_Log_entry_add_pointer("filter_class_name",filter_class_name,RTI_TRUE);)

    if (self->filter_plugin == NULL)
    {
        return DDS_RETCODE_PRECONDITION_NOT_MET;
    }

    if (DB_Database_lock(self->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    retval = DDS_FilterPlugin_unregister_filter_class(self->filter_plugin, filter_class_name);

    if (DB_Database_unlock(self->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retval;
}
