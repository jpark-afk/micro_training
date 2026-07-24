/*
 * FILE: netio_loopback.h - NETIO Loopback API
 *
 * Copyright (c) 2012-2024 Real-Time Innovations, Inc.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 17may2012,tk Written
 */
/*ci
 * \file
 * \brief NETIO Loopback API
 */
/*ci
 * \file
 * \defgroup NETIO_LoopbackInterfaceClass Loopback Interface
 * \ingroup NETIOModule
 * \brief NETIO Loopback Interface
 *
 * \details
 *
 * The loopback interface is an internal used for communication within the same
 * memory space. Note that the actual use is defined by outside of the
 * loopback interface itself.
 */
/*ci \addtogroup NETIO_LoopbackInterfaceClass
 * @{
 */
#ifndef netio_loopback_h
#define netio_loopback_h

#ifndef rt_rt_h
#include "rt/rt_rt.h"
#endif

#ifndef netio_dll_h
#include "netio/netio_dll.h"
#endif

#ifndef osapi_config_h
#include "osapi/osapi_config.h"
#endif

#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif

#ifndef netio_address_h
#include "netio/netio_address.h"
#endif

#ifndef netio_route_h
#include "netio/netio_route.h"
#endif

#ifndef netio_interface_h
#include "netio/netio_interface.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*ci
 *\brief The available loopback modes
 */
typedef enum
{
    /*ci
     * \brief Synchronous loop back mode
     */
    LOOP_INTERFACEMODE_LOOPBACK
} LOOP_InterfaceMode_T;

/*ci
 * \brief Loopback interface specific properties
 */
struct LOOP_InterfaceProperty
{
    /*ci
     * \brief base-class properties
     */
    struct NETIO_InterfaceProperty _parent;

    /*ci
     * \brief interface mode
     */
    LOOP_InterfaceMode_T mode;
};

/*ci
 * \brief Constant to initialize a \ref LOOP_InterfaceProperty
 */
#define LOOP_InterfaceProperty_INITIALIZER \
{\
    NETIO_InterfaceProperty_INITIALIZER,\
    LOOP_INTERFACEMODE_LOOPBACK \
}

/*ci
 * \brief Definition of loopback NETIO interface class id
 */
#define INTRA_INTERFACE_INTERFACE_ID RT_MKINTERFACEID(\
            RT_COMPONENT_CLASS_NETIO,RT_COMPONENT_INSTANCE_INTRA)

/*ci
 * \brief Function to retrieve the concrete implementation of the
 *        NETIO Loopback interface
 *
 * \return Pointer to Loopback factory implementation
 */
MUST_CHECK_RETURN NETIODllExport struct RT_ComponentFactoryI*
LOOP_InterfaceFactory_get_interface(void);

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif /* netio_loopback_h */

/*ci @} */
