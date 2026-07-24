/*
 * FILE: netio_route.h - NETIO Route API
 *
 * Copyright (c) 2012-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 29jun2015,tk MICRO-1358/PR#15173 Added/Updated comments
 * 29jun2015,tk MICRO-1332/PR#15078 Added/updated comments
 * 31jul2014,tk MICRO-842/PR#9683   Removed superfluous paramaters in DB
 *                                  compare function
 * 05may2014,tk MICRO-72            Updated based on CR-232
 * 25apr2012,tk Written
 */
/*ci
 * \file
 * \defgroup NETIO_ResolverClass NETIO Resolver API
 * \ingroup NETIOCommon
 * \brief NETIO Resolver API
 */

#ifndef netio_route_h
#define netio_route_h

#ifndef netio_dll_h
#include "netio/netio_dll.h"
#endif

#ifndef osapi_config_h
#include "osapi/osapi_config.h"
#endif

#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif

#ifndef reda_string_h
#include "reda/reda_string.h"
#endif

#ifndef netio_address_h
#include "netio/netio_address.h"
#endif

#ifndef netio_interface_h
#include "netio/netio_interface.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*ci
 * \defgroup NETIO_AddressResolver NETIO Address Resolver API
 * \ingroup NETIO_ResolverClass
 * \addtogroup NETIO_AddressResolver
 * @{
 */
/*ci
 *\brief Properties for an address resolver
 */
struct NETIO_AddressResolverProperty
{
    /*ci
     * \brief The maximum number of interfaces which can translate addresses
     */
    RTI_UINT32 max_interfaces;

    /*ci
     * \brief The default low index if none is specified in the address string
     */
    RTI_INT32 default_low_index;

    /*ci
     * \brief The default high index if none is specified in the address string
     */
    RTI_INT32 default_high_index;

    /*ci
     * \brief The default port number
     */
    RTI_UINT32 default_base_port;
};

/*ci
 * \brief Constant to initialize NETIO_AddressResolverProperty
 */
#define NETIO_AddressResolverProperty_INITIALIZER \
{\
  1,\
  0,\
  4,\
  7400\
}

/*ci
 * \brief Implementation of the NETIO_AddressResolver
 */
typedef struct NETIO_AddressResolver
{
    /*ci
     * \brief Properties the address resolver was created with
     */
    struct NETIO_AddressResolverProperty property;

    /*ci
     * \brief Table with registered address resolvers
     */
    DB_Table_T resolver_table;

    /*ci
     * \brief Database where the address resolvers are stored
     */
    DB_Database_T db;
} NETIO_AddressResolver_T;

struct NETIO_AddressResolveEntry;

/*ci
 * \brief State information when resolving addresses. It should not be accessed
 *        directly
 *
 * \details
 *
 * When an address string is resolved, a resolved context is created. The
 * resolved context can be iterated over in case an address string resolved
 * to more than one entry, for example [1,10]\@mammoth. The \ref
 * NETIO_AddressResolver_get_next function is used to iterate over a context.
 */
typedef struct NETIO_AddressResolveContext
{
    /*ci
     * \brief Whether the context has been initialized or not
     */
    RTI_BOOL is_initialized;

    /*ci
     * \brief The lowest index based on the address string and the default
     *         low index.
     */
    RTI_INT32 low_index;

    /*ci
     * \brief The highest index based on the address string and the default
     *         low index. The range from low_index is always in increments of 1
     */
    RTI_INT32 high_index;

    /*ci
     * \brief The base-port for port calculations
     */
    RTI_UINT32 base_port;

    /*ci
     * \brief The index which is currently iterated over.
     */
    RTI_INT32 current_index;

    /*ci
     * \brief The resolver which contributed the address resolution
     */
    struct NETIO_AddressResolveEntry *r_entry;

    /*ci
     * \brief The cursor into the address resolver table.
     */
    DB_Cursor_T cursor;

    /*ci
     * \brief The address part of a NETIO_Address string format. NETIO
     *        does not interpret this part, it is up to a specific layer
     *        to do so.
     */
    char out_address_string[NETIO_ADDRESS_TOKEN_MAX_SIZE];

    /*ci
     * \brief The current interface Id
     */
    RT_ComponentFactoryId_T intfid;

    /*ci
     * \brief The current kind of address being resolved
     */
    NETIO_RouteKind_T kind;
} NETIO_AddressResolveContext_T;

/*ci
 * \brief Constant to initialize NETIO_AddressResolveContext
 */
#define NETIO_AddressResolveContext_INITIALIZER \
{ \
   RTI_FALSE\
}

/*ci
 * \brief Create a new address resolver
 *
 * \param[in] name     The name of the address resolver, for information only
 * \param[in] db       The database to create the resolver table in
 * \param[in] property The resolver properties to use, cannot be NULL
 *
 * \return Pointer to created address resolver on success, NULL on failure
 *
 * \sa \ref NETIO_AddressResolver_delete
 */
MUST_CHECK_RETURN NETIODllExport NETIO_AddressResolver_T*
NETIO_AddressResolver_new(const char *name,
                          DB_Database_T db,
                          struct NETIO_AddressResolverProperty *property);

#ifndef RTI_CERT
/*ci
 * \brief Delete an address resolver
 *
 * \param[in] nar Address resolver to delete
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref NETIO_AddressResolver_new
 */
MUST_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_AddressResolver_delete(NETIO_AddressResolver_T *nar);
#endif /* !RTI_CERT */

/*ci
 * \brief Add an address resolver interface to the address resolver
 *
 * \param[in] nar          Address resolver to add interface to
 * \param[in] name         The address prefix name the transport understands
 * \param[in] port_resolve The port resolver function. Can be NULL
 * \param[in] port_resolve_param Parameter to add to port resolver function
 * \param[in] intf         The interface to call to resolve an address
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref NETIO_AddressResolver_delete_interface
 */
MUST_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_AddressResolver_add_interface(NETIO_AddressResolver_T *nar,
                                    const char *name,
                                    NETIO_PortCalculateFunc_T port_resolve,
                                    void *port_resolve_param,
                                    NETIO_Interface_T *intf);

#ifndef RTI_CERT
/*ci
 * \brief Delete an address resolver interface from the address resolver
 *
 * \param[in]  nar      Address resolver to delete interface from
 * \param[in]  name     The address prefix name the transport understands
 * \param[out] existed  RTI_TRUE is the interface existed, otherwise RTI_FALSE
 * \param[out] intf     If the interface existed it is returned in this
 *                      parameter
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref NETIO_AddressResolver_add_interface
 */
MUST_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_AddressResolver_delete_interface(NETIO_AddressResolver_T *nar,
                                       const char *name,
                                       RTI_BOOL *existed,
                                       NETIO_Interface_T **intf);
#endif /* !RTI_CERT */

/*ci
 * \brief Find a resolver interface based on the address prefix
 *
 * \param[in]  nar   Address resolver to search in
 * \param[in]  name  The address prefix name the transport understands
 * \param[out] intf  If the interface existed it is returned in this
 *                   parameter
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref NETIO_AddressResolver_add_interface
 */
MUST_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_AddressResolver_lookup_interface(NETIO_AddressResolver_T *nar,
                                    const char *name,
                                    NETIO_Interface_T **intf);

MUST_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_AddressResolver_lookup_name(NETIO_AddressResolver_T *nar,
                                  NETIO_Interface_T *intf,
                                  RT_ComponentFactoryId_T *id);

/*ci
 * \brief Resolve a NETIO_Address address string
 *
 * \details
 *
 * Resolve an address string to one or more NETIO_Address structures.
 * This function parses the address, but does return the NETIO_Address.
 * Instead use NETIO_AddressResolver_get_next to retrieve the addresses.
 *
 * \param[in]  nar       Address resolver to search in
 * \param[in]  try_index If try_index is >= 0, only try the specified index
 *                       otherwise use the index specified in the address string
 * \param[in]  kind      The type of address to convert
 * \param[in]  address_string The address string to resolve
 * \param[inout] context On success the resolver context is returned. The
 *                       context must have been initialized with before
 *                       calling this function.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref NETIO_AddressResolver_get_next
 */
MUST_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_AddressResolver_resolve(NETIO_AddressResolver_T *nar,
                              RTI_INT32 try_index,
                              NETIO_RouteKind_T kind,
                              const char *address_string,
                              NETIO_AddressResolveContext_T *context);
/*ci
 * \brief Iterate over a resolver context
 *
 * \details
 * After an address resolver context has been created with
 * \ref NETIO_AddressResolver_resolve, the result can be iterated over with
 * this function. Each call to this function return a single address
 *
 * \param[in]    nar        Address resolver the context was created from
 * \param[out]   netio_intf The NETIO interface which resolved the context
 * \param[inout] id         The id of the NETIO interface factory that resolved
 *                          the address
 * \param[inout] address    A NETIO_Address structure with a valid address
 * \param[inout] context    The context to iterate over
 * \param[inout] error      An error occurred
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref NETIO_AddressResolver_resolve
 */
MUST_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_AddressResolver_get_next(NETIO_AddressResolver_T *nar,
                               NETIO_Interface_T **netio_intf,
                               RT_ComponentFactoryId_T *id,
                               struct NETIO_Address *address,
                               NETIO_AddressResolveContext_T *context,
                               RTI_BOOL *error);

/*ci
 * \brief Initialize a \ref NETIO_AddressResolveContext_T
 *
 * \param[in] c Context to initialize
 *
 * \sa \ref NETIO_AddressResolver_resolve \ref NETIO_AddressResolver_get_next
 */
NETIODllExport void
NETIO_AddressResolveContext_init(NETIO_AddressResolveContext_T *c);

/*ci @} */

/*ci
 * \defgroup NETIO_RouteClass NETIO Route Resolver API
 * \ingroup NETIO_ResolverClass
 * \addtogroup NETIO_RouteClass
 * @{
 */

/*ci
 * \brief Properties for the route resolver
 */
struct NETIO_RouteResolverProperty
{
    /*ci
     * \brief The maximum number of destination which can be resolved
     */
    RTI_SIZE_T max_routes;
};

/*ci
 * \def NETIO_RouteResolverProperty_INITIALIZER
 * \brief Iniitalizer constant for NETIO_RouteResolverProperty
 */
#define NETIO_RouteResolverProperty_INITIALIZER \
{\
    16\
}

struct NETIO_RouteResolver;

/*ci
 * \brief Abstract NETIO_RouteResolver type
 */
typedef struct NETIO_RouteResolver NETIO_RouteResolver_T;

/*ci
 * \brief Fill a netmask from left to right in host order with 1s
 *
 * \param[in,out] netmask The netmask to fill.
*
 * \return RTI_TRUE on success, RTI_FALSE if netmask.bits > 128 or the
 *         input netmask has a non-zero mask.
 */
MUST_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_RouteResolver_fill_netmask(struct NETIO_Netmask *netmask);

/*ci
 * \brief Create a new route resolver
 *
 * \details
 *
 * Create a new route resolver
 *
 * \param[in] db       The database to use when creating tables
 * \param[in] nar      The address resolver to use
 * \param[in] name     The name of the route resolver, typically used when
 *                     creating database tables
 * \param[in] property The properties for the route resolver, cannot be NULL
 *
 * \return A new route resolver on success, NULL on failure
 *
 * \sa \ref NETIO_RouteResolver_delete
 */
MUST_CHECK_RETURN NETIODllExport NETIO_RouteResolver_T*
NETIO_RouteResolver_new(DB_Database_T db,NETIO_AddressResolver_T *nar,
                        const char* const name,
                        struct NETIO_RouteResolverProperty *const property);

#ifndef RTI_CERT
/*ci
 * \brief Delete a route resolver
 *
 * \details
 *
 * Delete a route resolver
 *
 * \param[in] r_table
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref NETIO_RouteResolver_new
 */
SHOULD_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_RouteResolver_delete(NETIO_RouteResolver_T *r_table);
#endif /* !RTI_CERT */

/*ci
 * \brief Add an interface to the route resolver
 *
 * \details
 *
 * When a route is resolved (finding an outgoing interface), the route
 * resolver searches among the interfaces that has been added with this
 * call.
 *
 * \param[in]  r_table Route resolver to add the interface to
 * \param[in]  intf    The interface routing to the source
 * \param[in]  address The address which the interface can route
 * \param[in]  netmask The netmask applied before testing for a match
 * \param[out] exists  Whether an interface existed or not
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref NETIO_RouteResolver_delete_interface
 */
MUST_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_RouteResolver_add_interface(NETIO_RouteResolver_T *r_table,
                                  const char *name,
                                  NETIO_Interface_T *intf,
                                  struct NETIO_Address *address,
                                  struct NETIO_Netmask *netmask,
                                  RTI_BOOL *exists);

/*ci
 * \brief Determine if the specified address is supported by a registered
 *        interface
 *
 * \param[in] r_table Route resolver to search for the interface in
 * \param[in] address The address an interface needs to support
 *
 * \return RTI_TRUE on if a capable interface is found, otherwise RTI_FALSE
 */
SHOULD_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_RouteResolver_lookup_interface(NETIO_RouteResolver_T *const r_table,
                                     const struct NETIO_Address *const address);

/*ci
 * \brief Determine if the specified address is reachable by the transport
 *
 * \param[in] r_table Route resolver to search for the interface in
 * \param[in] address The address to check for reachability
 *
 * \return RTI_TRUE on if a capable interface is found, otherwise RTI_FALSE
 */
SHOULD_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_RouteResolver_is_address_reachable(NETIO_RouteResolver_T *const r_table,
                                     const struct NETIO_AddressEx *const address,
                                         RTI_BOOL *is_reachable);

/*ci
 * \brief Determine the minimum maximum send size across all transports
 *
 * \param[in] r_table Route resolver to search for the interface in
 *
 * \return minimum maximum send size
 *         returns 0 is mtu cannot be determined or -1 in case of failure.
 */
NETIODllExport RTI_INT32
NETIO_RouteResolver_get_minimum_mtu(NETIO_RouteResolver_T *const r_table);


/*ci
\brief Determine the maximum maximum send size across all transports
 *
 * \param[in] r_table Route resolver to search for the interface in
 *
 * \return maximum maximum send size
 *         returns 0 is mtu cannot be determined or -1 in case of failure.
 */

RTI_INT32
NETIO_RouteResolver_get_maximum_mtu(NETIO_RouteResolver_T *const r_table);

#ifndef RTI_CERT
/*ci
 * \brief Delete an interface from the route resolver
 *
 * \details
 *
 *  Delete an interface from the route resolver
 *
 * \param[in]  r_table Route resolver to add the interface to
 * \param[in]  intf    The interface routing to the source
 * \param[in]  address The address which the interface can route
 * \param[in]  netmask The netmask to apply before testing for a match
 * \param[out] exists  Whether an interface existed or not
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref NETIO_RouteResolver_add_interface
 */
SHOULD_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_RouteResolver_delete_interface(NETIO_RouteResolver_T *r_table,
                                  NETIO_Interface_T *intf,
                                  struct NETIO_Address *address,
                                  struct NETIO_Netmask *netmask,
                                  RTI_BOOL *exists);
#endif /* !RTI_CERT */

/*ci
 * \brief Add a new route to a destination
 *
 * \details
 *
 * Add a new route to a destination if one does not exist. This function
 * also takes the \a via_address parameter which indicates which address
 * to pass to a downstream interface to reach the destination. For example,
 * to reach an RTPS endpoint over UDP the \a via_address contains the UDP
 * address to pass to the downstream UDP NETIO interface.
 *
 * \param[in]  r_table       Route resolver
 * \param[in]  src_intf      The source address of the route
 * \param[in]  dst_reader    The destination address to send to
 * \param[in]  via_address   The downstream address to use
 * \param[in]  property      The properties for this route
 * \param[out] route_existed Whether the route existed or not
 * \param[out] found_route   Whether an interface capable of routing to the
 *                           destination was found or not
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref NETIO_RouteResolver_delete_route
 */
MUST_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_RouteResolver_add_route(NETIO_RouteResolver_T *r_table,
                           NETIO_Interface_T *src_intf,
                           struct NETIO_Address *dst_reader,
                           struct NETIO_Address *via_address,
                           struct NETIORouteProperty *property,
                           RTI_BOOL *route_existed,
                           RTI_BOOL *found_route);

/*ci
 * \brief Delete a route from a route resolver.
 *
 * \param[in]  r_table       Route resolver
 * \param[in]  src_intf      The source address of the route
 * \param[in]  dst_reader    The destination address to send to
 * \param[in]  via_address   The downstream address to use
 * \param[out] route_existed Whether the route existed or not
 * \param[out] found_route   Whether an interface capable of routing to the
 *                           destination was found or not
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref NETIO_RouteResolver_add_route
 */
SHOULD_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_RouteResolver_delete_route(NETIO_RouteResolver_T *r_table,
                              NETIO_Interface_T *src_intf,
                              struct NETIO_Address *dst_reader,
                              struct NETIO_Address *via_address,
                              RTI_BOOL *route_existed,
                              RTI_BOOL *found_route);

/*ci
 * \brief Add peers to the route resolver
 *
 * \details
 *
 * The function is similar to \ref NETIO_RouteResolver_add_route, but serves
 * a slightly different purpose. Instead of adding a route as an address
 * specified in the NETIO_Address format, this function takes the address in
 * the NETIO Address string format. The route resolver parses the address
 * string and adds the resulting NETIO_Address (there may be more than one
 * depending on the index, for example 2@... will yield 3 address). There are
 * two kinds of peers (meta and user). In all other respects this function
 * behaves as \ref NETIO_RouteResolver_add_route
 *
 * \param[in]  r_table       Route resolver
 * \param[in]  src_intf      The source address of the route
 * \param[in]  dst_reader    The destination address to send too
 * \param[in]  peer_kind     The type of address
 * \param[in]  peer_address  Peer address string
 * \param[in]  property      The properties for this route
 * \param[out] route_existed Whether the route existed or not
 * \param[out] found_route   Whether an interface capable of routing to the
 *                           destination was found or not. If the function
 *                           returns FALSE and found_route != NULL, found_route
 *                           is set to TRUE if at least one route was added.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * \sa \ref NETIO_RouteResolver_add_route
 */
SHOULD_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_RouteResolver_add_peer(NETIO_RouteResolver_T *r_table,
                           NETIO_Interface_T *src_intf,
                           struct NETIO_Address *dst_reader,
                           NETIO_RouteKind_T peer_kind,
                           const char *peer_address,
                           struct NETIORouteProperty *property,
                           RTI_BOOL *route_existed,
                           RTI_BOOL *found_route);

#ifndef RTI_CERT
/*ci
 * \brief Delete peers from the route resolver
 *
 * \details
 *
 * The function is similar to \ref NETIO_RouteResolver_delete_route, but serves
 * a slightly different purpose. Instead of adding a route an address specified
 * in NETIO_Address format, this function takes the address in the NETIO
 * Address string format. The route resolver parses the address string and
 * removes the resulting NETIO_Address (there may be more than one depending
 * on the index, for example 2@... will yield 3 address). There are two kinds
 * of peers (meta and user). In all other respects this function behaves as
 * \ref NETIO_RouteResolver_add_route
 *
 * \param[in]  r_table       Route resolver
 * \param[in]  src_intf      The source address of the route
 * \param[in]  dst_reader    The destination address to send too
 * \param[in]  peer_kind     The type of address
 * \param[in]  peer_address  Peer address string
 * \param[out] route_existed Whether the route existed or not
 * \param[out] found_route   Whether an interface capable of routing to the
 *                           destination was found or not
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref NETIO_RouteResolver_add_route
 */
SHOULD_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_RouteResolver_delete_peer(NETIO_RouteResolver_T *r_table,
                           NETIO_Interface_T *src_intf,
                           struct NETIO_Address *dst_reader,
                           NETIO_RouteKind_T peer_kind,
                           const char *peer_address,
                           RTI_BOOL *route_existed,
                           RTI_BOOL *found_route);
#endif /* !RTI_CERT */


/*ci
 * \brief Lookup a route in the route table
 *
 * \param[in]  r_table       Route resolver
 * \param[in]  src_intf      The source address of the route
 * \param[in]  dst_reader    The destination address to send to
 * \param[in]  via_address   The downstream address to use
 * \param[out] route_exists  RTI_TRUE if a route exists, RTI_FALSE otherwise
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_RouteResolver_lookup_route(NETIO_RouteResolver_T *r_table,
                                 NETIO_Interface_T *src_intf,
                                 struct NETIO_Address *dst_reader,
                                 struct NETIO_Address *via_address,
                                 RTI_BOOL *route_exists);

MUST_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_RouteResolver_find_interface(NETIO_RouteResolver_T *r_table,
                                   NETIO_Interface_T **src_intf,
                                   const struct NETIO_Address *const via_addr);

MUST_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_RouteResolver_interface_supports_kind(NETIO_RouteResolver_T *r_table,
                                            NETIO_Interface_T *intf,
                                            RTI_INT32 kind);

/*ci @} */

/*ci
 * \defgroup NETIO_BindClass NETIO Bind Resolver API
 * \ingroup NETIO_ResolverClass
 * \addtogroup NETIO_BindClass
 * @{
 */
struct NETIO_BindResolver;

/*ci
 * \brief Abstract NETIO_BindResolver type
 */
typedef struct NETIO_BindResolver NETIO_BindResolver_T;

/*ci
 * \brief Properties for the bind resolver
 */
struct NETIO_BindResolverProperty
{
    /*ci
     * \brief The maximum number of sources which can listened to
     */
    RTI_SIZE_T max_routes;
};

/*ci
 * \def NETIO_BindResolverProperty_INITIALIZER
 * \brief Initializer constant for NETIO_BindResolverProperty
 */
#define NETIO_BindResolverProperty_INITIALIZER \
{\
    1\
}

/*ci
 * \brief Create a new bind resolver
 *
 * \details
 *
 * Create a new bind resolver
 *
 * \param[in] db       The database to use for tables
 * \param[in] nar      The address resolver to use to resolve addresses
 *                     specified as strings
 * \param[in] name     The name of the bind resolver, typically used
 *                     when creating database tables
 * \param[in] property The bind resolver properties
 *
 * \return Pointer to the newly created bind resolver on exit, NULL on failure
 *
 * \sa \ref NETIO_BindResolver_delete
 */
MUST_CHECK_RETURN NETIODllExport NETIO_BindResolver_T*
NETIO_BindResolver_new(DB_Database_T db,NETIO_AddressResolver_T *nar,
                       const char* const name,
                       struct NETIO_BindResolverProperty *const property);

#ifndef RTI_CERT
/*ci
 * \brief Delete a bind resolver
 *
 * \details
 *
 * Create a new bind resolver
 *
 * \param[in] r_table Delete a bind resolver
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref NETIO_BindResolver_new
 */
SHOULD_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_BindResolver_delete(NETIO_BindResolver_T* r_table);
#endif

/*ci
 * \brief Add a new route to the bind resolver
 *
 * \details
 *
 * Add a new interface that may be listened to.
 *
 * \param[in]  r_table  The bind table
 * \param[in]  kind     The type of address
 * \param[in]  id       The interface factory
 * \param[in]  address  The address which can be listened to
 * \param[in]  intf     The interface to listen on for the address
 * \param[out] exists   Whether the route already existed or not
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref NETIO_BindResolver_delete_route
 */
SHOULD_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_BindResolver_add_route(NETIO_BindResolver_T *r_table,
                           NETIO_RouteKind_T kind,
                           RT_ComponentFactoryId_T *id,
                           struct NETIO_Address *address,
                           NETIO_Interface_T *intf,
                           RTI_BOOL *exists);

/*ci
 * \brief Delete a route from the bind resolver
 *
 * \details
 *
 * Delete a route from the bind resolver, it can no longer be listened to.
 *
 * \param[in]  r_table  The bind table
 * \param[in]  kind     The type of address
 * \param[in]  id       The interface factory
 * \param[in]  address  The address to no longer listen on
 * \param[out] exists   Whether the route already existed or not
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref NETIO_BindResolver_add_route
 */
SHOULD_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_BindResolver_delete_route(NETIO_BindResolver_T *r_table,
                              NETIO_RouteKind_T kind,
                              RT_ComponentFactoryId_T *id,
                              struct NETIO_Address *address,
                              RTI_BOOL *existed);

/*ci
 * \brief Lookup an interface by name
 *
 * \details
 *
 * Given a kind, index, an address in string format the bind resolver will
 * search among its routes to find an interface capable of receiving the
 * address.
 *
 * \param[in]  r_table    The bind table
 * \param[in]  kind       The type of address
 * \param[in]  try_index  The index to use,
 * \param[in]  name       The address string to listen to
 * \param[out] id         The factory which successfully resolved the address
 * \param[out] intf       The interface to receive on
 * \param[out] resolved   Sequence of addresses resolved from the address string
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref NETIO_BindResolver_lookup_by_address,
 *     \ref NETIO_BindResolver_lookup_by_kind
 */
MUST_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_BindResolver_lookup_by_name(NETIO_BindResolver_T *r_table,
                                NETIO_RouteKind_T kind,
                                RTI_INT32 try_index,
                                const char *const name,
                                RT_ComponentFactoryId_T *id,
                                NETIO_Interface_T **intf,
                                struct NETIO_AddressSeq *resolved);

/*ci
 * \brief Lookup an interface by address
 *
 * \details
 *
 * Given a kind, an address and a factory, find the interface which can
 * listen on the address.
 *
 * \param[in]  r_table  The bind table
 * \param[in]  kind     The type of address
 * \param[out] id       The factory which successfully resolved the address
 * \param[out] address  The address
 * \param[out] intf     The interface to receive on
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref NETIO_BindResolver_lookup_by_name,
 *     \ref NETIO_BindResolver_lookup_by_kind
 */
MUST_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_BindResolver_lookup_by_address(NETIO_BindResolver_T *r_table,
                                      NETIO_RouteKind_T kind,
                                      RT_ComponentFactoryId_T *id,
                                      struct NETIO_Address *address,
                                      NETIO_Interface_T **intf);

/*ci
 * \brief Release addresses previously reserved with
 *        NETIO_BindResolver_reserve_addresses
 *
 * \details
 *
 * \param[in]  r_table     The bind table
 * \param[in]  transport   A sequence of registered NETIO interfaces
 * \param[out] kind        The kind of address to reserve
 * \param[out] locator_seq On success a sequence of addresses which have been
 *                         reserved
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref NETIO_BindResolver_reserve_addresses
 */
SHOULD_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_BindResolver_release_addresses(NETIO_BindResolver_T* r_table,
                                     struct REDA_StringSeq *transport,
                                     NETIO_RouteKind_T kind,
                                     struct NETIO_AddressSeq *locator_seq);

/*ci
 * \brief Reserve addresses from one or more NETIO interfaces
 *
 * \details
 *
 *  This function reserves addresses. What it means to reserve an address is
 *  defined by the NETIO layer. For example, in the case of UDP it means to
 *  reserve ports. While NETIO itself does not make a distinction between
 *  unicast and multicast, this function will sort addresses into two output
 *  sequences, one for multicast and one for unicast.
 *
 * \param[in]    r_table          The bind table
 * \param[in]    try_index        The index to use
 * \param[in]    transports       A sequence of registered NETIO interfaces
 * \param[in]    locators         A sequence of locators to reserve
 * \param[in]    kind             The kind of address to reserve
 * \param[inout] mc_locator_seq   The reserved multicast addresses
 * \param[inout] uc_locator_seq   The reserved unicast addresses

 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref NETIO_BindResolver_release_addresses
 */
MUST_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_BindResolver_reserve_addresses(NETIO_BindResolver_T* r_table,
                                     RTI_INT32 try_index,
                                     struct REDA_StringSeq *transports,
                                     struct REDA_StringSeq *locators,
                                     NETIO_RouteKind_T kind,
                                     struct NETIO_AddressSeq *mc_locator_seq,
                                     struct NETIO_AddressSeq *uc_locator_seq);

/*ci
 * \brief Unbind from a NETIO address
 *
 * \details
 *
 * Remove an entry listening to a specific address.
 *
 * \param[in]  r_table       The bind table
 * \param[in]  transport     A sequence of addresses to remove
 * \param[in]  kind          The kind of address remove binding for
 * \param[in]  from_address  The source address being listened too
 * \param[in]  to_intf       The destination interface being forward to
 * \param[in]  to_address    The destination address to forward to
 * \param[out] route_existed Whether a bind entry existed or not
 * \param[out] found_route   Whether a bind entry was found or not

 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref NETIO_BindResolver_bind
 */
MUST_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_BindResolver_unbind(NETIO_BindResolver_T* r_table,
                         struct REDA_StringSeq *transport,
                         NETIO_RouteKind_T kind,
                         struct NETIO_Address *from_address,
                         NETIO_Interface_T *to_intf,
                         struct NETIO_Address *to_address,
                         RTI_BOOL *route_existed,
                         RTI_BOOL *found_route);

/*ci
 * \brief Bind to an NETIO address
 *
 * \details
 * This function is used to find an interface that can listen to the specified
 * address.
 *
 * \param[in]  r_table      The bind table
 * \param[in]  transport    A sequence of addresses to listen to
 * \param[in]  kind         The kind of address to bind to
 * \param[in]  from_address The source address to listened too
 * \param[in]  to_intf      The destination interface to forward to
 * \param[in]  to_address   The destination address to forward to
 * \param[in]  route_existed Whether a bind entry already existed or not
 * \param[in]  found_route   Whether a bind entry was found or not

 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref NETIO_BindResolver_unbind
 */
MUST_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_BindResolver_bind(NETIO_BindResolver_T* r_table,
                        struct REDA_StringSeq *transport,
                        NETIO_RouteKind_T kind,
                        struct NETIO_Address *from_address,
                        NETIO_Interface_T *to_intf,
                        struct NETIO_Address *to_address,
                        RTI_BOOL *route_existed,
                        RTI_BOOL *found_route);

/*ci
 * \brief Get Minimum mtu for the transports reserved for a given kind.
 *
 * \param[in]  r_table      The bind resolver
 * \param[in]  kind         The kind of address to check MTU for

 * \return min mtu for the given route kind, -1 on failure
 */
NETIODllExport RTI_INT32
NETIO_BindResolver_get_min_mtu_by_kind(NETIO_BindResolver_T *r_table,
                                        NETIO_RouteKind_T kind);


#ifdef __cplusplus
}
#endif

#endif /* netio_route_h */

/*ci @} */
