/*
 * FILE: netio_proxy.h - netio_proxy API
 *
 * Copyright (c) 2017-2024 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef netio_proxy_h
#define netio_proxy_h

#ifndef netio_proxy_dll_h
#include "netio_proxy/netio_proxy_dll.h"
#endif

#ifndef osapi_config_h
#include "osapi/osapi_config.h"
#endif
#ifndef osapi_thread_h
#include "osapi/osapi_thread.h"
#endif
#ifndef rt_rt_h
#include "rt/rt_rt.h"
#endif
#ifndef reda_string_h
#include "reda/reda_string.h"
#endif
#ifndef reda_sequence_h
#include "reda/reda_sequence.h"
#endif
#ifndef netio_config_h
#include "netio/netio_config.h"
#endif
#ifndef netio_address_h
#include "netio/netio_address.h"
#endif
#ifndef netio_route_h
#include "netio/netio_route.h"
#endif

#ifndef netio_proxy_config_h
#include "netio_proxy/netio_proxy_config.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

struct NETIO_PROXYInterfaceFactoryProperty;

NETIO_PROXYDllExport RTI_BOOL
NETIO_PROXYInterfaceFactoryProperty_initialize(
            struct NETIO_PROXYInterfaceFactoryProperty* self);

NETIO_PROXYDllExport RTI_BOOL
NETIO_PROXYInterfaceFactoryProperty_finalize(
                            struct NETIO_PROXYInterfaceFactoryProperty *p);

#ifdef __cplusplus
}
#endif

struct NETIO_PROXYInterfaceFactoryProperty
{
    struct RT_ComponentFactoryProperty _parent;
#ifdef RTI_CPP
     public:
        NETIO_PROXYInterfaceFactoryProperty()
        {
            NETIO_PROXYInterfaceFactoryProperty_initialize(this);
        }
        ~NETIO_PROXYInterfaceFactoryProperty() { }
     private:
        NETIO_PROXYInterfaceFactoryProperty(
                const struct NETIO_PROXYInterfaceFactoryProperty& from )
        {
            UNUSED_ARG(from);
        }
        struct NETIO_PROXYInterfaceFactoryProperty& operator=(
                const struct NETIO_PROXYInterfaceFactoryProperty& from )
        {
            UNUSED_ARG(from);
            return *this;
        }
        bool operator==(
                const struct NETIO_PROXYInterfaceFactoryProperty& other)
        {
            UNUSED_ARG(other);
            return false;
        }
        bool operator!=(
                const struct NETIO_PROXYInterfaceFactoryProperty& other)
        {
            UNUSED_ARG(other);
            return false;
        }
#endif
#ifdef RTI_CPP
public:
#endif
        struct NETIO_InterfaceI netio_interface;

        struct RT_ComponentFactoryI *r_factory;

        struct RT_ComponentFactoryProperty *r_property;

        struct RT_ComponentFactoryListener *r_listener;
};

#ifdef __cplusplus
extern "C"
{
#endif

#define NETIO_PROXY_INTERFACE_ID RT_MKINTERFACEID(\
            RT_COMPONENT_CLASS_NETIO,RT_COMPONENT_INSTANCE_UDP)

extern NETIO_PROXYDllVariable struct NETIO_PROXYInterfaceFactoryProperty
                                NETIO_PROXYINTERFACE_FACTORY_PROPERTY_DEFAULT;

/*ci
 * \brief Function to retrieve the concrete implementation of the
 *        NETIO_PROXY interface.
 *
 * \return Pointer to NETIO_PROXY interface implementation
 */
MUST_CHECK_RETURN NETIO_PROXYDllExport struct RT_ComponentFactoryI*
NETIO_PROXYInterfaceFactory_get_interface(void);

NETIO_PROXYDllExport RTI_BOOL
NETIO_PROXYInterface_register(RT_Registry_T *registry,
                          const char *const name,
                          struct NETIO_PROXYInterfaceFactoryProperty *property,
                          struct RT_ComponentFactoryI *r_intf,
                          struct RT_ComponentFactoryProperty *r_property,
                          struct RT_ComponentFactoryListener *r_listener);

NETIO_PROXYDllExport struct NETIO_PROXYInterface*
NETIO_PROXYInterfaceFactory_get_proxy_instance(
                                         struct RT_ComponentFactory *factory,
                                         RTI_INT32 instance);

NETIO_PROXYDllExport void
NETIO_PROXYInterface_set_send(struct NETIO_PROXYInterface *proxy,
                              RTI_BOOL enable);

NETIO_PROXYDllExport void
NETIO_PROXYInterface_set_drop_rate(struct NETIO_PROXYInterface *proxy,
                                   RTI_INT32 drop_rate);

NETIO_PROXYDllExport void
NETIO_PROXYInterface_set_corruption(struct NETIO_PROXYInterface *proxy,
                                    RTI_BOOL enable);

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif /* NETIO_NAME_h */

