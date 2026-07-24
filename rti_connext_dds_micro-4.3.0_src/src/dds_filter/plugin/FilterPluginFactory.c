/*
 * FILE: FilterPluginFactory.c - Filter plugin factory implementation
 *
 * Copyright (c) 2025-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#include "FilterPluginFactory.h"
#include "FilterPlugin.h"
#if DDS_ENABLE_SQL_FILTER
#include "../sql/SqlContentFilter.h"
#endif

#include "dds_c/dds_c_domain.h"
#include "rt/rt_rt.h"

/*** SOURCE_BEGIN ***/

RTI_PRIVATE struct RT_ComponentFactoryI DDS_FilterPluginFactory_fv_Intf;

DDS_ReturnCode_t
DDS_FilterPluginFactory_register(void)
{
    if (!RT_Registry_register(
            DDS_DomainParticipantFactory_get_registry(
                    DDS_DomainParticipantFactory_get_instance()),
            DDS_FILTER_PLUGIN_FACTORY_DEFAULT_NAME,
            &DDS_FilterPluginFactory_fv_Intf,
            NULL,
            NULL))
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}

#ifndef RTI_CERT
DDS_ReturnCode_t
DDS_FilterPluginFactory_unregister(void)
{
    if (!RT_Registry_unregister(
            DDS_DomainParticipantFactory_get_registry(
                    DDS_DomainParticipantFactory_get_instance()),
            DDS_FILTER_PLUGIN_FACTORY_DEFAULT_NAME,
            NULL,
            NULL))
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}
#endif

/*ci
 * \brief Create a new instance of the Filter plugin using the factory
 *
 * \details
 * Implementation of the RT ComponentFactory create component method. This
 * method creates a new instance of the Filter plugin. It is never
 * called directly, only via the component factory plugin.
 *
 * \param[in] factory   The factory creating the component
 * \param[in] property  The component property
 * \param[in] listener  The component listener
 *
 * \return A new instance of the Filter plugin on success, NULL on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RT_Component_T *
DDS_FilterPluginFactory_create_component_impl(
        struct RT_ComponentFactory *factory,
        struct RT_ComponentProperty *property,
        struct RT_ComponentListener *listener)
{
    struct DDS_FilterPluginFactory *self = (struct DDS_FilterPluginFactory *)factory;
    struct DDS_FilterPlugin *filter_plugin = NULL;
    struct DDS_FilterPluginProperty *filter_prop = (struct DDS_FilterPluginProperty *)property;

    RT_Component_T *result = NULL;
#ifndef RTI_CERT
    RTI_BOOL initialized = RTI_FALSE;
#endif /* !RTI_CERT */

    UNUSED_ARG(listener);

    OSAPI_PRECONDITION_ALWAYS((factory == NULL) || (property == NULL),
                       return NULL,
                       OSAPI_Log_entry_add_pointer("factory", factory, RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("property", property, RTI_TRUE);)

    OSAPI_Heap_allocate_struct(&filter_plugin, struct DDS_FilterPlugin);
    if (filter_plugin == NULL)
    {
        DDS_FILTER_LOG_HEAP_ALLOC(OSAPI_LOGKIND_ERROR, DDS_FILTER_LOG_FILTER_PLUGIN_BUFFER)
        goto done;
    }

    if (!DDS_FilterPluginImpl_initialize(filter_plugin, self, filter_prop))
    {
        DDS_FILTER_LOG_FILTER_PLUGIN_INIT(OSAPI_LOGKIND_ERROR)
        goto done;
    }
#ifndef RTI_CERT
    initialized = RTI_TRUE;
#endif /* !RTI_CERT */

#if DDS_ENABLE_SQL_FILTER
    /* Automatically enable the builtin SQL filter for the participant
     * unless the user explicitly disabled it in the property.
     */
    if (!filter_prop->disable_builtin_sql_filter)
    {
        if (DDS_FilterPluginImpl_register_filter_class(
                        filter_plugin,
                        DDS_SQL_CONTENT_FILTER_CLASS,
                        DDS_SqlContentFilter_get_interface(),
                        NULL) != DDS_RETCODE_OK)
        {
            /* Error will be logged by register_filter_class */
            goto done;
        }
    }
#endif

    ++self->instance_counter;

    result = &filter_plugin->_parent;

done:
#ifndef RTI_CERT
    if ((result == NULL) && (filter_plugin != NULL))
    {
        if (initialized)
        {
            DDS_FilterPluginImpl_finalize(filter_plugin);
        }
        OSAPI_Heap_free_struct(filter_plugin);
    }
#endif /* !RTI_CERT */
    return result;
}

#ifndef RTI_CERT
/*ci
 * \brief
 *
 * \param[in] factory   The factory that created the component
 * \param[in] component The component to be deleted
 *
 * \sa \ref DDS_FilterPluginFactory_create_component
 */
RTI_PRIVATE void
DDS_FilterPluginFactory_delete_component_impl(
        struct RT_ComponentFactory *factory,
        RT_Component_T *component)
{
    struct DDS_FilterPluginFactory *self = (struct DDS_FilterPluginFactory *)factory;
    struct DDS_FilterPlugin *filter_plugin = (struct DDS_FilterPlugin *)component;

    RTI_BOOL ok = RTI_FALSE;

    OSAPI_PRECONDITION_ALWAYS(
            (factory == NULL) || (component == NULL), return,
            OSAPI_Log_entry_add_pointer("factory", factory, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("component", component, RTI_TRUE);)

    ok = DDS_FilterPluginImpl_finalize(filter_plugin);
    if (!ok)
    {
        DDS_FILTER_LOG_FILTER_PLUGIN_FINALIZE(OSAPI_LOGKIND_ERROR)
        return;
    }

    OSAPI_Heap_free_struct(filter_plugin);

    --self->instance_counter;
}
#endif /* !RTI_CERT */

/*ci
 * \brief Initialize the Filter plugin factory
 *
 * \details
 * Filter specific implementation of the RT ComponentFactory initialize
 * method. This method is never called directly. It is called by the
 * RT when the Filter factory is registered.
 *
 * \param[in] property The property the factory was registered with
 * \param[in] listener The listener the factory was registered with
 *
 * \return A fully initialized factory on success, NULL on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE struct RT_ComponentFactory *
DDS_FilterPluginFactory_initialize(
        struct RT_ComponentFactoryProperty *property,
        struct RT_ComponentFactoryListener *listener)
{
    struct RT_ComponentFactory *result = NULL;
    struct DDS_FilterPluginFactory *factory = NULL;

    UNUSED_ARG(property);
    UNUSED_ARG(listener);

    OSAPI_Heap_allocate_struct(&factory, struct DDS_FilterPluginFactory);
    if (factory == NULL)
    {
        DDS_FILTER_LOG_HEAP_ALLOC(OSAPI_LOGKIND_ERROR,
                                  DDS_FILTER_LOG_FILTER_PLUGIN_FACTORY_BUFFER)
        goto done;
    }

    factory->_parent._factory = &factory->_parent;
    factory->_parent.intf = &DDS_FilterPluginFactory_fv_Intf;

    factory->instance_counter = 0;

    result = (struct RT_ComponentFactory *)factory;
done:
    return result;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize the Filter plugin factory
 *
 * Filter specific implementation of the RT ComponentFactory finalize
 * method. This method is never called directly. It is called by the
 * RT when the Filter factory is unregistered.
 *
 * \param[in]  factory  The factory to finalize
 * \param[out] property The property the factory was registered with
 * \param[out] listener The listener the factory was registered with
 *
 * \sa \ref DDS_FilterPluginFactory_initialize
 */
RTI_PRIVATE void
DDS_FilterPluginFactory_finalize(
        struct RT_ComponentFactory *factory,
        struct RT_ComponentFactoryProperty **property,
        struct RT_ComponentFactoryListener **listener)
{
    struct DDS_FilterPluginFactory *self =
            (struct DDS_FilterPluginFactory *)factory;

    UNUSED_ARG(property);
    UNUSED_ARG(listener);

    OSAPI_PRECONDITION_ALWAYS(
            (factory == NULL),
            return,
            OSAPI_Log_entry_add_pointer("factory", factory, RTI_TRUE);)

    /* Do some consistency checking */
    if (self->instance_counter != 0)
    {
        DDS_FILTER_LOG_FILTER_PLUGIN_FACTORY_IN_USE(OSAPI_LOGKIND_ERROR,
                                                    self->instance_counter)
    }
    else
    {
        OSAPI_Heap_free_struct(self);
    }
}
#endif /* !RTI_CERT */

/*ci
 * \brief Implementation of the RT ComponentFactoryI plugin
 */
RTI_PRIVATE struct RT_ComponentFactoryI DDS_FilterPluginFactory_fv_Intf = {
        .id = DDS_FILTER_PLUGIN_INTERFACE_ID,
        .initialize = DDS_FilterPluginFactory_initialize,
        .create_component = DDS_FilterPluginFactory_create_component_impl,
#ifndef RTI_CERT
        .finalize = DDS_FilterPluginFactory_finalize,
        .delete_component = DDS_FilterPluginFactory_delete_component_impl,
#else
        .finalize = NULL,
        .delete_component = NULL,
#endif
        /* The Filter plugin factory does not support the following operations */
        .get_if = NULL,
        .get_property = NULL,
};

struct RT_ComponentFactoryI *
DDS_FilterPluginFactory_get_interface(void)
{
    return &DDS_FilterPluginFactory_fv_Intf;
}
