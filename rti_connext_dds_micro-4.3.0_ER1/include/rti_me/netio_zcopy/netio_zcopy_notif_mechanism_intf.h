/*
 * FILE: netio_zcopy_notif_mechanism_intf.h - Notification mechanism definitions
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
 * \defgroup ZCOPY_NotifUserInterfaceClass User notification mechanism interface
 * \brief Notification mechanism definitions
 */

/*ci
 * \addtogroup ZCOPY_NotifUserInterfaceClass
 * @{
 */
#ifndef netio_zcopy_notif_mechanism_intf_h
#define netio_zcopy_notif_mechanism_intf_h

#ifndef netio_interface_h
#include "netio/netio_interface.h"
#endif

#ifndef netio_zcopy_dll_h
#include "netio_zcopy/netio_zcopy_dll.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*e \dref_ZCOPY_NotifUserInterface_createFunc
 */
FUNCTION_MUST_TYPEDEF(
NETIO_Interface_T *
(*ZCOPY_NotifUserInterface_createFunc)(
                NETIO_Interface_T *upstream,
                void *user_property)
)

#ifndef RTI_CERT
/*e \dref_ZCOPY_NotifUserInterface_deleteFunc
 */
typedef void
(*ZCOPY_NotifUserInterface_deleteFunc)(
                NETIO_Interface_T *user_intf);
#endif /* !RTI_CERT */

/*e \dref_ZCOPY_NotifUserInterface_reserve_addressFunc
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*ZCOPY_NotifUserInterface_reserve_addressFunc)(
                NETIO_Interface_T *user_intf,
                struct NETIO_Address *src_addr,
                void** port_entry_out)
)

/*e \dref_ZCOPY_NotifUserInterface_release_addressFunc
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*ZCOPY_NotifUserInterface_release_addressFunc)(
                NETIO_Interface_T *user_intf,
                struct NETIO_Address *src_addr,
                void* port_entry)
)

/*e \dref_ZCOPY_NotifUserInterface_add_routeFunc
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*ZCOPY_NotifUserInterface_add_routeFunc)(
                NETIO_Interface_T *user_intf,
                struct NETIO_Address *source,
                struct NETIO_Address *destination,
                void** route_entry_out)
)

/*e \dref_ZCOPY_NotifUserInterface_delete_routeFunc
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*ZCOPY_NotifUserInterface_delete_routeFunc)(
                NETIO_Interface_T *user_intf,
                struct NETIO_Address *source,
                struct NETIO_Address *destination,
                void* route_entry)
)

/*e \dref_ZCOPY_NotifUserInterface_sendFunc
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*ZCOPY_NotifUserInterface_sendFunc)(
                NETIO_Interface_T *self,
                struct NETIO_Address *source,
                struct NETIO_Address *destination,
                void* route_entry)
)

/*e \dref_ZCOPY_NotifUserInterface_bindFunc
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*ZCOPY_NotifUserInterface_bindFunc)(
        NETIO_Interface_T *user_intf,
        struct NETIO_Address *src_addr, /* Remote NOTIF address */
        struct NETIO_Address *dst_addr, /* Local NOTIF address */
        void *port_entry,
        void **bind_entry_out)
)

/*e \dref_ZCOPY_NotifUserInterface_unbindFunc
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*ZCOPY_NotifUserInterface_unbindFunc)(
        NETIO_Interface_T *user_intf,
        struct NETIO_Address *src_addr, /* Remote NOTIF address */
        struct NETIO_Address *dst_addr, /* Local NOTIF address */
        void *port_entry,
        void *bind_entry)
)

/*e \dref_ZCOPY_NotifUserInterface_notify_portFunc
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*ZCOPY_NotifUserInterface_notify_portFunc)(
    NETIO_Interface_T *user_intf,
    void* port_entry)
)


/*e \dref_ZCOPY_NotifUserInterface_get_interface
 */
struct ZCOPY_NotifUserInterfaceI*
ZCOPY_NotifUserInterface_get_interface(void);


/*e \dref_ZCOPY_NotifUserInterface_get_property
 */
void*
ZCOPY_NotifUserInterface_get_property(void);


/*e \dref_ZCOPY_NotifUserInterfaceI
 */
typedef NETIO_ZCOPYDllExport struct ZCOPY_NotifUserInterfaceI
{
    /*e \dref_ZCOPY_NotifUserInterfaceI_create_instance
     */
    ZCOPY_NotifUserInterface_createFunc create_instance;

#ifndef RTI_CERT
    /*e \dref_ZCOPY_NotifUserInterfaceI_delete_instance
     */
    ZCOPY_NotifUserInterface_deleteFunc delete_instance;
#endif

    /*e \dref_ZCOPY_NotifUserInterfaceI_resolve_address
     */
    NETIO_Interface_resolve_addressFunc resolve_address;

    /*e \dref_ZCOPY_NotifUserInterfaceI_get_route_table;
     */
    NETIO_Interface_get_route_tableFunc get_route_table;

    /*e \dref_ZCOPY_NotifUserInterfaceI_reserve_address
     */
    ZCOPY_NotifUserInterface_reserve_addressFunc reserve_address;

    /*e \dref_ZCOPY_NotifUserInterfaceI_release_address
     */
    ZCOPY_NotifUserInterface_release_addressFunc release_address;

    /*e \dref_ZCOPY_NotifUserInterfaceI_add_route
     */
    ZCOPY_NotifUserInterface_add_routeFunc add_route;

    /*e \dref_ZCOPY_NotifUserInterfaceI_delete_route
     */
    ZCOPY_NotifUserInterface_delete_routeFunc delete_route;

    /*e \dref_ZCOPY_NotifUserInterfaceI_bind
     */
    ZCOPY_NotifUserInterface_bindFunc bind;

    /*e \dref_ZCOPY_NotifUserInterfaceI_unbind
     */
    ZCOPY_NotifUserInterface_unbindFunc unbind;

    /*e \dref_ZCOPY_NotifUserInterfaceI_send
     */
    ZCOPY_NotifUserInterface_sendFunc send;

    /*e \dref_ZCOPY_NotifUserInterfaceI_notify_recv_port
     */
    ZCOPY_NotifUserInterface_notify_portFunc notify_recv_port;
} ZCOPY_NotifUserInterfaceI;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* netio_zcopy_notif_mechanism_intf_h */

/*ci @} */
