/*
 * FILE: NotificationInterface.h - Notification Interface private definitions
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
 * \brief Notification Interface private definitions
 *
 * \addtogroup ZCOPY_NotifInterfaceClass
 * @{
 */
#ifndef NotificationInterface_h
#define NotificationInterface_h

#include "netio_zcopy/netio_zcopy_notif_interface.h"
#include "Notifier.h"
#include "reda/reda_bufferpool.h"
#include "netio_zcopy/netio_zcopy_sharedq.h"
#include "netio/netio_notif.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*ci \brief Notification interface class
 */
struct ZCOPY_NotifInterface
{
    /*ci \brief Parent class */
    struct NETIO_Interface _parent;

    /*ci \brief The properties that the notification interface was created with */
    struct ZCOPY_NotifInterfaceProperty property;

    /*ci \brief The factory that created this interface */
    struct ZCOPY_NotifInterfaceFactory *factory;

    /*ci \brief Instance of user notification mechanism */
    struct NETIO_Interface *user_netio;

    /*ci \brief Interface for calling NETIO_InterfaceI functions on user_netio */
    struct NETIO_InterfaceI _user_netio_intf;

    /*ci \brief Pool of shared queue readers used for binds on this interface */
    REDA_BufferPool_T sq_reader_pool;

    /*ci \brief Pool of notifiers used for routes on this interface */
    REDA_BufferPool_T notifier_pool;

    /*ci \brief Table with notif port entries, once for each unique port listened to */
    DB_Table_T port_table;

    /*ci \brief indexer for unbound shared queue readers */
    struct REDA_Indexer *unbound_index;
    
    /*ci \brief  A table to track reliable readers used by all routes. */
    DB_Table_T reliable_table;
};

/*ci \brief Notification interface route entry
 *   \details A route is the send path from a local DataWriter to a remote DataReader.
 */
struct ZCOPY_NotifInterfaceRouteEntry
{
    /*ci \brief Parent class */
    struct NETIORouteEntry _parent;

    /*ci \brief Reference counter to keep track of reuse of this route */
    RTI_UINT32 _ref_count;
    
    /*ci \brief  Count of the number of reliable readers for this route. */
    RTI_UINT32 _reliable_count;

    /*ci \brief Notifier object for this destination/route */
    struct ZCOPY_Notifier *_notifier;

    /*ci \brief User notification mechanism route entry */
    void *_user_entry;
};

/*ci \brief A key used for tracking reliable entries for notif routes
 */
struct ZCOPY_NotifInterfaceRouteKeyEntry
{
   /*ci \brief */
   struct NETIO_Guid key;

   /*ci \brief */
   struct ZCOPY_NotifInterfaceRouteEntry *route_ptr;
   
};

#define ZCOPY_NOTIFINTERFACE_ROUTE_KEY_ENTRY_INITIALIZER \
{ \
    NETIO_GUID_INITIALIZER, /* key */ \
    NULL /* route_ptr */ \
}

/*ci \brief Notification interface bind entry key
 */
struct ZCOPY_NotifInterfaceBindEntryKey
{
    /*ci \brief The source address to listen to */
    struct NETIO_Address source;

    /*ci \brief The destination address listening to the source */
    NETIO_Interface_T* interface;
};

/*ci \brief A sq_reader entry.
 */
struct ZCOPY_NotifInterfaceSqRecordEntry
{
   /*ci \brief Shared queue reader for this bind */
   NETIO_ZCOPY_SharedQReader *sq_reader;

   /*ci \brief Number of referenced samples used by the reader */
   RTI_INT64 ref_count;
};

/*ci \brief Bind entry
 *   \details A bind is the receive path from a remote DataWriter to a local DataReader.
 */
struct ZCOPY_NotifInterfaceBindEntry
{
    /*ci \brief Key field */
    struct ZCOPY_NotifInterfaceBindEntryKey key;

    /*ci \brief Shared queue reader and its sample ref count */
    struct ZCOPY_NotifInterfaceSqRecordEntry *_sq_reader_entry;

    /*ci \brief User notification mechanism bind entry */
    void *_user_entry;
};

/*ci \brief Notification interface port entry
 *   \details A port is an address on which local DataReaders can receive
 *            notifications. A port can be used by multiple DataReaders. A
 *            port corresponds to a single notifiee.
 */
struct ZCOPY_NotifInterfacePortEntry
{
    /*ci \brief The address to bind to and receive data from */
    struct NETIO_Address address;

    /*ci \brief The number of listeners to this port */
    RTI_UINT32 _ref_count;

    /*ci \brief Back reference to the notification interface that created this port */
    struct ZCOPY_NotifInterface *_notif_intf;

    /*ci \brief The notifiee object associated with this port */
    struct ZCOPY_Notifiee *_notifiee;

    /*ci \brief User notification mechanism port entry */
    void *_user_entry;

    /*ci \brief Flag to indicate if the timeout has been scheduled or not. Only
     *          updated inside critical section.
     */
     RTI_BOOL _timeout_scheduled;

    /*ci \brief The writer match event timeout handler. The same timeout is used
     *          for all DataWriters to this port entry
     */
    OSAPI_TimeoutHandle_T _writer_match_timeout;
};

/*ci \brief Factory for creating notification interfaces.
 */
struct ZCOPY_NotifInterfaceFactory
{
    /*ci \brief Parent class*/
    struct RT_ComponentFactory _parent;

    /*ci \brief The properties that the notification factory was created with */
    struct ZCOPY_NotifInterfaceFactoryProperty property;

    /* \brief The number of components created by this factory */
    RTI_UINT32 instance_counter;
};

/*ci \brief Iterator user data for visiting unseen samples.
 */
struct ZCOPY_NotifInterfaceSampleVisit
{
    /*ci Whether a sample has already been visited. */
    RTI_BOOL is_dirty;

    /*ci Whether there are more samples to visit. */
    RTI_BOOL are_more_samples;

    /*ci The packet info of the visited sample. */
    struct NETIO_PacketInfo *info;
};

/*ci \brief Convenience macro to call downstream interface
 */
#define ZCOPY_NotifUserInterface_create_instance(self_) \
    (self_)->factory->property.user_intf                \
            ->create_instance(&(self_)->_parent,        \
                              (self_)->factory->property.user_property)

#ifndef RTI_CERT
/*ci \brief Convenience macro to call downstream interface
 */
#define ZCOPY_NotifUserInterface_delete_instance(self_, instance_) \
    (self_)->factory->property.user_intf->delete_instance(instance_)
#endif

/*ci \brief Convenience macro to call downstream interface
 */
#define ZCOPY_NotifUserInterface_reserve_address(self_, addr_, port_entry_) \
    (self_)->factory->property.user_intf->reserve_address((self_)->user_netio, addr_, port_entry_)

/*ci \brief Convenience macro to call downstream interface
 */
#define ZCOPY_NotifUserInterface_release_address(self_, addr_, port_entry_) \
    (self_)->factory->property.user_intf->release_address((self_)->user_netio, addr_, port_entry_)

/*ci \brief Convenience macro to call downstream interface
 */
#define ZCOPY_NotifUserInterface_add_route(self_, route_entry_) \
    (self_)->factory->property.user_intf->add_route(            \
            (self_)->user_netio,                                \
            &(route_entry_)->_parent.intf_address,              \
            &(route_entry_)->_parent.destination,               \
            &(route_entry_)->_user_entry)

/*ci \brief Convenience macro to call downstream interface
 */
#define ZCOPY_NotifUserInterface_delete_route(self_, route_entry_) \
    (self_)->factory->property.user_intf->delete_route(            \
            (self_)->user_netio,                                   \
            &(route_entry_)->_parent.intf_address,                 \
            &(route_entry_)->_parent.destination,                  \
            (route_entry_)->_user_entry)

/*ci \brief Convenience macro to call downstream interface
 */
#define ZCOPY_NotifUserInterface_send(self_, src_, route_entry_) \
    (self_)->factory->property.user_intf->send(                  \
            (self_)->user_netio,                                 \
            src_,                                                \
            &(route_entry_)->_parent.destination,                \
            (route_entry_)->_user_entry)

/*ci \brief Convenience macro to call downstream interface
 */
#define ZCOPY_NotifUserInterface_bind(self_, src_, dest_, port_entry_, bind_entry_) \
    (self_)->factory->property.user_intf                                            \
            ->bind((self_)->user_netio, src_, dest_, port_entry_, bind_entry_)

/*ci \brief Convenience macro to call downstream interface
 */
#define ZCOPY_NotifUserInterface_unbind(self_, src_, dest_, port_entry_, bind_entry_) \
    (self_)->factory->property.user_intf                                              \
            ->unbind((self_)->user_netio, src_, dest_, port_entry_, bind_entry_)

/*ci \brief Convenience macro to call downstream interface
 */
#define ZCOPY_NotifUserInterface_notify_receive_port(self_,port_entry_) \
    (self_)->factory->property.user_intf->notify_recv_port((self_)->user_netio, port_entry_)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NotificationInterface_h */

/*ci @} */
