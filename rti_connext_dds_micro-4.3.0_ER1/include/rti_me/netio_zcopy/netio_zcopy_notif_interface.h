/*
 * FILE: netio_zcopy_notif_interface.h - Notification Interface Definitions
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
/*e
 * \file
 * \defgroup ZCOPY_NotifInterfaceClass Notification Interface
 * \ingroup ZCV2Module
 * \brief ZCOPY Notification Interface
 *
 * \details
 *
 * The Notification Interface is implemented as a NETIO interface and NETIO
 * interface factory.
 */
/*ce
 * \addtogroup ZCOPY_NotifInterfaceClass
 * @{
 */
#ifndef netio_zcopy_notif_interface_h
#define netio_zcopy_notif_interface_h

#ifndef rt_rt_h
#include "rt/rt_rt.h"
#endif

#ifndef netio_zcopy_notif_mechanism_intf_h
#include "netio_zcopy/netio_zcopy_notif_mechanism_intf.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*e \dref_ZCOPY_NotifInterfaceFactoryProperty
 */
struct ZCOPY_NotifInterfaceFactoryProperty
{
    /*i \dref_ZCOPY_NotifInterfaceFactoryProperty_parent
     */
    struct NETIO_InterfaceFactoryProperty _parent;

    /*e \dref_ZCOPY_NotifInterfaceFactoryProperty_max_samples_per_notif
     */
    RTI_UINT32 max_samples_per_notif;

    /*e \dref_ZCOPY_NotifInterfaceFactoryProperty_user_intf
     */
    struct ZCOPY_NotifUserInterfaceI *user_intf;

    /*e \dref_ZCOPY_NotifInterfaceFactoryProperty_user_property
     */
    void *user_property;
};

/*e \dref_ZCOPY_NotifInterfaceFactoryProperty_INITIALIZER
 */
#define ZCOPY_NotifInterfaceFactoryProperty_INITIALIZER {                   \
    NETIO_InterfaceFactoryProperty_INITIALIZER, /* _parent */               \
    1,                                          /* max_samples_per_notif */ \
    NULL,                                       /* user_intf */             \
    NULL,                                       /* user_property */         \
}

/*i \dref_ZCOPY_NotifInterface_receive
 */
extern RTI_BOOL
ZCOPY_NotifInterface_receive(NETIO_Interface_T *netio_intf,
                              const struct NETIO_Address *port,
                              RTI_BOOL *has_more_data_out);

/*e \dref_ZCOPY_NotifInterfaceFactory_register
 */
extern MUST_CHECK_RETURN RTI_BOOL
ZCOPY_NotifInterfaceFactory_register(
        RT_Registry_T *registry,
        const char *name,
        struct ZCOPY_NotifInterfaceFactoryProperty *property);

/*e \dref_ZCOPY_NotifInterfaceFactory_unregister
 */
extern MUST_CHECK_RETURN RTI_BOOL
ZCOPY_NotifInterfaceFactory_unregister(RT_Registry_T *registry, const char *name);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* netio_zcopy_notif_interface_h */

/*ce @} */
