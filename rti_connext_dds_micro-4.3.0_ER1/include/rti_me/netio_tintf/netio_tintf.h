/*
 * FILE: netio_tintf.h - NETIO test interface
 *
 * Copyright (c) 2015-2024 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 25apr2012,tk  Written
 */
/*ci
 * \file
 * \defgroup NETIO_TestInterfaceClass NETIO_TestInterface Interface
 * \ingroup NETIOModule
 * \brief NETIO Logger Interface
 *
 * \details
 *
 * The test interface is implemented as a NETIO interface and NETIO interface
 * factory.
 */
/*ci \addtogroup NETIO_TestInterfaceClass
 * @{
 */
#ifndef netio_tintf_h
#define netio_tintf_h

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
#ifndef netio_tintf_dll_h
#include "netio_tintf/netio_tintf_dll.h"
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

#ifdef __cplusplus
extern "C"
{
#endif

#define NETIO_ADDRESS_KIND_LOGGER         10
#define NETIO_ADDRESS_KIND_DUMP           11

#ifdef __cplusplus
extern "C"
{
#endif

struct NETIO_TestInterfaceFactoryProperty;
NETIO_TINTFDllExport RTI_BOOL
NETIO_TestInterfaceFactoryProperty_initialize(
            struct NETIO_TestInterfaceFactoryProperty* self);

NETIO_TINTFDllExport RTI_BOOL
NETIO_TestInterfaceFactoryProperty_finalize(
        struct NETIO_TestInterfaceFactoryProperty *p);

#ifdef __cplusplus
}
#endif

struct NETIO_TestInterfaceFactoryProperty
{
    struct RT_ComponentFactoryProperty _parent;
#ifdef RTI_CPP
     public:
        NETIO_TestInterfaceFactoryProperty()
        {
            NETIO_TestInterfaceFactoryProperty_initialize(this);
        }
        ~NETIO_TestInterfaceFactoryProperty() { }
    private:\
        NETIO_TestInterfaceFactoryProperty(
                const struct NETIO_TestInterfaceFactoryProperty& from )
        {
            UNUSED_ARG(from);
        }
        struct NETIO_TestInterfaceFactoryProperty& operator=(
                const struct NETIO_TestInterfaceFactoryProperty& from )
        {
            UNUSED_ARG(from);
            return *this;
        }
        bool operator==(
                const struct NETIO_TestInterfaceFactoryProperty& other)
        {
            UNUSED_ARG(other);
            return false;
        }
        bool operator!=(
                const struct NETIO_TestInterfaceFactoryProperty& other)
        {
            UNUSED_ARG(other);
            return false;
        }
#endif
        struct NETIO_InterfaceI netio_interface;
};

#ifdef __cplusplus
extern "C"
{
#endif

#define NETIO_TESTINTERFACE_INTERFACE_ID RT_MKINTERFACEID(\
            RT_COMPONENT_CLASS_NETIO,RT_COMPONENT_INSTANCE_UDP)

NETIO_TINTFDllVariable extern struct NETIO_TestInterfaceFactoryProperty
                                NETIO_TESTINTERFACE_FACTORY_PROPERTY_DEFAULT;

/*ci
 * \brief Function to retrieve the concrete implementation of the
 *        NETIO UDP interface.
 *
 * \return Pointer to UDP interface implementation
 */
MUST_CHECK_RETURN NETIO_TINTFDllExport struct RT_ComponentFactoryI*
NETIO_TestInterfaceFactory_get_interface(void);

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif /* netio_udp_h */

/*ci @} */

