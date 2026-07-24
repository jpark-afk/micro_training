/*
 * FILE: NETIO_ZeroCopyLoader.c - ZCOPY loader implementation.
 *
 * Copyright 2023-2024 Real-Time Innovations, Inc.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*ci
 * \file
 * \brief Zcopy Loader implementation
 *
 * \addtogroup ZCOPY_NotifInterfaceClass
 * @{
 */
#include "NotificationInterface.h"
#include "netio_zcopy/netio_zcopy_log.h"
#include "osapi/osapi_string.h"
#include "netio_zcopy/netio_zcopy_notif_mechanism_intf.h"
#include "netio_zcopy/netio_zcopy.h"

/*ci
 * \brief Factory for the Loader.
 */
struct ZCOPY_LoaderFactory
{
    struct RT_ComponentFactory _parent;

    struct ZCOPY_NotifLoaderFactoryProperty *f_property;
};

RTI_PRIVATE struct RT_ComponentFactoryI ZCOPY_NotifLoaderFactory_fv_Intf;

/*ci
 * \brief Initialize the Notification Loader Factory
 *
 * \details
 * This Factory wraps NDDS_Transport_ZeroCopy_initialize and registration of notification mechanism.
 * This plugin is only used with MAG and provides a single point of entry for the MAG plugin to register.
 *
 * \param[in] property The property the factory was registered with
 * \param[in] listener The listener the factory was registered with
 *
 * \return A fully initialized factory on success, NULL on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE struct RT_ComponentFactory *
ZCOPY_Loader_initialize(
        struct RT_ComponentFactoryProperty *property,
        struct RT_ComponentFactoryListener *listener)
{
    DDS_DomainParticipantFactory* domain_factory = NULL;
    struct ZCOPY_LoaderFactory *factory = NULL;
    struct ZCOPY_NotifLoaderFactoryProperty *f_property =
            (struct ZCOPY_NotifLoaderFactoryProperty *)property;
    RT_Registry_T *registry;
    struct RT_ComponentFactory *result  = NULL;
    UNUSED_ARG(listener);

    domain_factory = DDS_DomainParticipantFactory_get_instance();
    registry = DDS_DomainParticipantFactory_get_registry(domain_factory);

    if (f_property == NULL)
    {
        goto done;
    }

    /*sets up the shared q writer history*/
    if (!NDDS_Transport_ZeroCopy_initialize(registry, NULL, NULL))
    {
        goto done;
    }

    /* if a implementation of notification mechanism is provided in XML and sent via MAG,
     *  use it otherwise get one from the implementation itself.
     */
    if (f_property->_parent.user_intf == NULL)
    {
        f_property->_parent.user_intf = ZCOPY_NotifUserInterface_get_interface();
        if (f_property->_parent.user_intf == NULL)
        {
            goto done;
        }
    }

    if (f_property->_parent.user_property == NULL )
    {
        f_property->_parent.user_property = ZCOPY_NotifUserInterface_get_property();
    }

    if (!ZCOPY_NotifInterfaceFactory_register(
        registry, f_property->notif_transport_name, &f_property->_parent))
    {
        goto done;
    }

    OSAPI_Heap_allocate_struct(&factory, struct ZCOPY_LoaderFactory);
    if (factory == NULL)
    {
        ZCOPY_LOG_NOTIF_ALLOC(OSAPI_LOGKIND_ERROR)
        goto done;
    }
    factory->_parent._factory = &factory->_parent;
    factory->_parent.intf = &ZCOPY_NotifLoaderFactory_fv_Intf;
    factory->f_property = f_property;
    result =  &factory->_parent;

done:
    return result;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize the Zcopy Loader factory
 *
 * \param[in]  factory  The factory to finalize
 * \param[out] property The property the factory was registered with
 * \param[out] listener The listener the factory was registered with
 *
 * \sa \ref ZCOPY_Loader_initialize
 */
RTI_PRIVATE void
ZCOPY_Loader_finalize(
        struct RT_ComponentFactory *factory,
        struct RT_ComponentFactoryProperty **property,
        struct RT_ComponentFactoryListener **listener)
{
    struct ZCOPY_LoaderFactory *self =
            (struct ZCOPY_LoaderFactory *)factory;
    DDS_DomainParticipantFactory* domain_factory = NULL;
    RT_Registry_T *registry;

    UNUSED_ARG(listener);
    OSAPI_PRECONDITION_ALWAYS(
            (factory == NULL),
            return,
            OSAPI_Log_entry_add_pointer("factory", factory, RTI_TRUE);)

    domain_factory = DDS_DomainParticipantFactory_get_instance();
    registry = DDS_DomainParticipantFactory_get_registry(domain_factory);
    if (property != NULL)
    {
        *property = (struct RT_ComponentFactoryProperty *)self->f_property;
    }
    if (ZCOPY_NotifInterfaceFactory_unregister(registry,
                    self->f_property->notif_transport_name))
    {
        OSAPI_Heap_free_struct(self);
    }
}
#endif /* !RTI_CERT */

/*ci
 * \brief Implementation of the RT ComponentFactoryI interface
 */
RTI_PRIVATE struct RT_ComponentFactoryI ZCOPY_NotifLoaderFactory_fv_Intf = {
        .id = NOTIF_LOADER_INTERFACE_INTERFACE_ID,
        .initialize = ZCOPY_Loader_initialize,
        .create_component = NULL,
#ifndef RTI_CERT
        .finalize = ZCOPY_Loader_finalize,
#else
        .finalize = NULL,
#endif
        .delete_component = NULL,
        .get_if = NULL,
        .get_property = NULL,
};


MUST_CHECK_RETURN NETIO_ZCOPYDllExport struct RT_ComponentFactoryI*
ZCOPY_Loader_get_interface(void)
{
    return &ZCOPY_NotifLoaderFactory_fv_Intf;
}

/*ci @} */
