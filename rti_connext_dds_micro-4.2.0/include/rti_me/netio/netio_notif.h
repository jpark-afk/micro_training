/*
 * FILE: netio_zcopy.h - Zcopy definitions required by Micro
 *
 * Copyright 2023 Real-Time Innovations, Inc.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#ifndef netio_notif_h
#define netio_notif_h

#ifndef rt_rt_h
#include "rt/rt_rt.h"
#endif

#ifndef netio_address_h
#include "netio/netio_interface.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define NETIO_ADDRESS_KIND_NOTIF    0x01001002
#define NETIO_PROTOCOL_NOTIF        (4)
#define RT_COMPONENT_INSTANCE_NOTIF (6)
#define RT_COMPONENT_INSTANCE_NOTIF_LOADER (7)

/*ci
 * \ingroup ZCOPY_NotifInterfaceClass
 *
 * \brief Notification interface specific properties
 */
struct ZCOPY_NotifInterfaceProperty
{
    /*ci \brief base-class properties */
    struct NETIO_InterfaceProperty _parent;

    /*ci \brief Domain this notification interface lives in */
    RTI_INT32 domain_id;

    /*ci \brief Number of remote writers
     * This is a signed integer for compatibility with the DataReader QoS.
     */
    RTI_INT32 num_remote_writers;

    /*ci \brief Number of ports which can be reserved */
    RTI_UINT32 max_receive_ports;

    /*ci \brief A lock provided by the participant to be used by the timers*/
    struct OSAPI_Mutex *lock;
};

/*ci
 * \ingroup ZCOPY_NotifInterfaceClass
 *
 * \brief Constant to initialize a \ref ZCOPY_NotifInterfaceProperty
 */
#define ZCOPY_NotifInterfaceProperty_INITIALIZER                            \
{                                                                           \
    NETIO_InterfaceProperty_INITIALIZER, /* _parent */                      \
    0,                                   /* domain_id */                    \
    0,                                   /* num_remote_writers */           \
    0,                                   /* max_receive_ports */            \
    NULL,                                /*lock*/                           \
}

/*ci
 * \ingroup ZCOPY_NotifInterfaceClass
 *
 * \brief Definition of Notification NETIO interface class id
 */
#define NOTIF_INTERFACE_INTERFACE_ID RT_MKINTERFACEID(\
            RT_COMPONENT_CLASS_NETIO, RT_COMPONENT_INSTANCE_NOTIF)

#define NOTIF_LOADER_INTERFACE_INTERFACE_ID RT_MKINTERFACEID(\
            RT_COMPONENT_CLASS_NETIO, RT_COMPONENT_INSTANCE_NOTIF_LOADER)
            

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
