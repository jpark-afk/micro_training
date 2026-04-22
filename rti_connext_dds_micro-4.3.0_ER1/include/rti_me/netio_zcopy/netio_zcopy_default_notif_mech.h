/*
 * FILE: netio_zcopy_default_notif_mech.h  - Default implementation for notification mechanism
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
 * \defgroup ZCOPY_NotifMechanismClass User notification mechanism implementation
 * \brief POSIX notification mechanism definitions
 */


#ifndef netio_zcopy_default_notif_mech_h
#define netio_zcopy_default_notif_mech_h

#ifndef netio_zcopy_notif_mechanism_h
#include "netio_zcopy/netio_zcopy_notif_mechanism_intf.h"
#endif

#ifndef netio_zcopy_notif_interface_h
#include "netio_zcopy/netio_zcopy_notif_interface.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define ZCOPY_NOTIF_MECH_BASE_NAME "NM"

/*e \dref_ZCOPY_NotifMechanismProperty
 */
struct ZCOPY_NotifMechanismProperty
{
    /*e \dref_ZCOPY_NotifMechanismProperty_intf_addr
     */
    RTI_UINT32 intf_addr;

    /*e \dref_ZCOPY_NotifMechanismProperty_thread_prop
     */
    struct OSAPI_ThreadProperty thread_prop;

    /*e \dref_ZCOPY_NotifMechanismProperty_max_receive_ports
     */
    RTI_UINT32 max_receive_ports;

    /*e \dref_ZCOPY_NotifMechanismProperty_max_routes
     */
    RTI_UINT32 max_routes;
};

#define ZCOPY_NotifMechanismProperty_INITIALIZER              \
{                                                             \
    0,                                /* intf_addr */         \
    OSAPI_ThreadProperty_INITIALIZER, /* thread_prop */       \
    2,                                /* max_receive_ports */ \
    32,                               /* max_routes */        \
}

/*ci
 * \brief Set the notification interface address part of a \ref NETIO_Address
 *
 * \param[out] addr       Address to initialize
 * \param[in]  notif_addr Notification address in host order
 * \param[in]  port       Address port
 */
extern void
NETIO_Address_set_from_notif_address(
        struct NETIO_Address *addr,
        RTI_UINT32 notif_intf_addr,
        RTI_UINT32 port);

/*e \dref_ZCOPY_NotifMechanism_register
 */
extern RTI_BOOL
ZCOPY_NotifMechanism_register(
        RT_Registry_T *registry,
        const char *name,
        struct ZCOPY_NotifInterfaceFactoryProperty *property);

/*e \dref_ZCOPY_NotifMechanism_unregister
 */
extern RTI_BOOL
ZCOPY_NotifMechanism_unregister(
        RT_Registry_T *registry,
        const char *name);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* netio_zcopy_default_notif_mech_h */
