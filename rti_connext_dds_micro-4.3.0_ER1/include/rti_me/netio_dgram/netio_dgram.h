/*
 * FILE: netio_dgram.h - NETIO DGRAM API
 *
 * Copyright (c) 2022-2024 Real-Time Innovations, Inc. 
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
 * 05sep2023,tk MICRO-5552/PR.32162
 * - Fixed documentation reference for resolve_address to include
 *   information about default behavior when resolve_address is NULL.
 * - Fixed documentation reference for send,release_address, and
 *   get_route_table to refer to documentation for NETIO_DGRAM, not NETIO,
 *   for consistency with resolve_address.
 * 31aug2023,tk MICRO-5487 Reduce memory usage
 * - Reduced NETIO_DGRAM_INTERFACE_MAX_INTERFACES to 8
 * - Reduced NETIO_DGRAM_INTERFACE_MAX_ADDRESSES to 8
 * 03aug2023,ad MICRO-5464/PR.31927
 * - Added a missing "s" to "#ifdef __cplusplus"
 * 26jul2023,ad MICRO-5173/PR.31918
 * - Changed "netio_dgram/netio_common.h" to "netio/netio_common.h"
 * 17jul2023,tk MICRO-5364/PR.32033 Updates to multicast group and address
 * resolution
 * - Added type NETIO_DGRAMUserInterface_resolve_addressFunc
 * - Updated NETIO_DGRAM_Interface_resolve_ipv4_address to be compatible with
 *   NETIO_DGRAMUserInterface_resolve_addressFunc
 * - Changed type of NETIO_DGRAM_InterfaceI.resolve_address to
 *   NETIO_DGRAMUserInterface_resolve_addressFunc
 * 10jul2023,ad MICRO-5291/PR.32023
 * - Added NETIO_DGRAM_Interface_upstream_receive
 * 06Jul2023,ad MICRO-5204/PR.31988
 * - Removed TSeq_loan_contiguous from NETIO_DGRAM_InterfaceTableEntrySeq
 * 25may2023,tk MICRO-4890/PR.31651 address interface changes
 * - Set all netmask mask array elements to 0
 * - Set the number of netmask bits in:
 *   - NETIO_DGRAM_InterfaceMultiCastGroup_UDPv4 (4)
 *   - NETIO_DGRAM_InterfaceMultiCastGroup_UDPv6 (8)
 * - Set multicast addresses in network order
 * - Changed NETIO_DGRAMINTERFACETABLEENTRY_UDPV4 to set address in network
 *   order in the first 4 bytes
 * 25may2023,ad MICRO-5018
 * - Changed NETIO_DGRAM_INTERFACE_SHARED_PORT_FLAG reference comment to
 *   dref_NETIO_DGRAM_INTERFACE_SHARED_PORT_FLAG
 * - Changed NETIO_Interface_resolve_adddressFunc to
 *    NETIO_Interface_resolve_addressFunc
 * 15may2023,ad MICRO-4590/PR.31770
 * - Removed a redundant comma in the InterfaceMultiCastGroup_UDPv6 definition
 * 26oct2022,tk Written
 */
 /*ci
  * \file
  * \defgroup NETIO_DGRAMInterfaceClass NETIO DGRAM Interface
  * \ingroup NETIOModule
  * \brief NETIO DGRAM Interface
  *
  */
#ifndef netio_dgram_h
#define netio_dgram_h

#ifndef netio_dgram_dll_h
#include "netio_dgram/netio_dgram_dll.h"
#endif
#ifndef netio_dgram_config_h
#include "netio_dgram/netio_dgram_config.h"
#endif
#ifndef netio_common_h
#include "netio/netio_common.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*e \dref_NETIO_DGRAM_INTERFACE_SHARED_PORT_FLAG
 */
#define NETIO_DGRAM_INTERFACE_SHARED_PORT_FLAG (0x1U)

/*e \dref_NETIO_DGRAM_INTERFACE_MAX_INTERFACES
 */
#define NETIO_DGRAM_INTERFACE_MAX_INTERFACES (8)

/*e \dref_NETIO_DGRAM_INTERFACE_MAX_ADDRESSES
 */
#define NETIO_DGRAM_INTERFACE_MAX_ADDRESSES (8)

/*e \dref_NETIO_DGRAM_InterfaceMultiCastGroup
 */
struct DDSCPPDllExport NETIO_DGRAM_InterfaceMultiCastGroup
{
    /*e \dref_NETIO_DGRAM_InterfaceMultiCastGroup_address_low
     */
    struct NETIO_Address address_low;

    /*e \dref_NETIO_DGRAM_InterfaceMultiCastGroup_address_high
     */
    struct NETIO_Address address_high;

    /*e \dref_NETIO_DGRAM_InterfaceMultiCastGroup_netmask
     */
    struct NETIO_Netmask netmask;
};

/*e \dref_NETIO_DGRAM_InterfaceMultiCastGroup_INITIALIZER
 */
#define NETIO_DGRAM_InterfaceMultiCastGroup_INITIALIZER \
{\
    NETIO_Address_INITIALIZER,\
    NETIO_Address_INITIALIZER,\
    NETIO_Netmask_INITIALIZER\
}

/*e \dref_NETIO_DGRAM_InterfaceMultiCastGroup_Invalid
 */
#define NETIO_DGRAM_InterfaceMultiCastGroup_Invalid \
{\
    {-1,0,{{0,0,0,0}},{0}},\
    {-1,0,{{0,0,0,0}},{0}},\
    {0,{0,0,0,0}} \
}

/*e \dref_NETIO_DGRAM_InterfaceMultiCastGroup_UDPv4
 */
#define NETIO_DGRAM_InterfaceMultiCastGroup_UDPv4 \
{\
    {NETIO_ADDRESS_KIND_UDPv4,0,{{NETIO_htonl(0xe0000000),0,0,0}},{0}},\
    {NETIO_ADDRESS_KIND_UDPv4,0,{{NETIO_htonl(0xefffffff),0,0,0}},{0}},\
    {4,{0,0,0,0}} \
}

/*e \dref_NETIO_DGRAM_InterfaceMultiCastGroup_UDPv6
 */
#define NETIO_DGRAM_InterfaceMultiCastGroup_UDPv6 \
{\
    {NETIO_ADDRESS_KIND_UDPv6,0,{{NETIO_htonl(0xff000000),0,0,0}},{0}},\
    {NETIO_ADDRESS_KIND_UDPv6,0,{{0xffffffff,0xffffffff,0xffffffff,0xffffffff}},{0}},\
    {8,{0,0,0,0}} \
}

/*e \dref_NETIO_DGRAM_InterfaceTableEntry
 */
struct DDSCPPDllExport NETIO_DGRAM_InterfaceTableEntry
{
    /*e \dref_NETIO_DGRAM_InterfaceTableEntry_locator_kind
     */
    RTI_INT32 locator_kind;

    /*e \dref_NETIO_DGRAM_InterfaceTableEntry_flags
     */
    RTI_UINT32 flags;

    /*e \dref_NETIO_DGRAM_InterfaceTableEntry_mtu
     */
    RTI_UINT32 mtu;

    /*e \dref_NETIO_DGRAM_InterfaceTableEntry_address
     */
    struct NETIO_Address address;

    /*e \dref_NETIO_DGRAM_InterfaceTableEntry_multicast_group
     */
    struct NETIO_DGRAM_InterfaceMultiCastGroup multicast_group;

    /*e \dref_NETIO_DGRAM_InterfaceTableEntry_ifname
     */
    const char *ifname;
};

/*e \dref_NETIO_DGRAM_InterfaceTableEntry_INITIALIZER
 */
#define NETIO_DGRAM_InterfaceTableEntry_INITIALIZER \
{\
    0U,\
    0U,\
    0U,\
    NETIO_Address_INITIALIZER,\
    NETIO_DGRAM_InterfaceMultiCastGroup_INITIALIZER,\
    NULL\
}

/*e \dref_NETIO_DGRAMINTERFACETABLEENTRY_UDPV4
 */
#define NETIO_DGRAMINTERFACETABLEENTRY_UDPV4(name_,addr_) \
{\
    1,\
    NETIO_DGRAM_INTERFACE_SHARED_PORT_FLAG,\
    65507U,\
    {1,0,{{NETIO_htonl(addr_),0,0,0}},{0}},\
    NETIO_DGRAM_InterfaceMultiCastGroup_UDPv4,\
    name_\
}

/*e \dref_NETIO_DGRAM_InterfaceRouteEntry
 */
struct DDSCPPDllExport NETIO_DGRAM_InterfaceRouteEntry
{
    /*e \dref_NETIO_DGRAM_InterfaceRouteEntry_address
     */
    struct NETIO_Address address;

    /*e \dref_NETIO_DGRAM_InterfaceRouteEntry_netmask
     */
    struct NETIO_Netmask netmask;
};

/*e \dref_NETIO_ADDRESS_UDPv4_ANY_UNICAST_ROUTE
 */
extern DDSCPPDllExport const struct
NETIO_DGRAM_InterfaceRouteEntry NETIO_ADDRESS_UDPv4_ANY_UNICAST_ROUTE;

/*e \dref_NETIO_ADDRESS_UDPv4_ANY_MULTICAST_ROUTE
 */
extern DDSCPPDllExport const struct
NETIO_DGRAM_InterfaceRouteEntry NETIO_ADDRESS_UDPv4_ANY_MULTICAST_ROUTE;

/*e \dref_NETIO_ADDRESS_UDPv6_ANY_UNICAST_ROUTE
 */
extern DDSCPPDllExport const struct
NETIO_DGRAM_InterfaceRouteEntry NETIO_ADDRESS_UDPv6_ANY_UNICAST_ROUTE;

/*e \dref_NETIO_ADDRESS_UDPv6_ANY_MULTICAST_ROUTE
 */
extern DDSCPPDllExport const struct
NETIO_DGRAM_InterfaceRouteEntry NETIO_ADDRESS_UDPv6_ANY_MULTICAST_ROUTE;

#define T struct NETIO_DGRAM_InterfaceTableEntry
#define TSeq NETIO_DGRAM_InterfaceTableEntrySeq
#include <reda/reda_sequence_decl.h>
#ifdef DOXYGEN_DOCUMENTATION_ONLY
/*e \dref_NETIO_DGRAM_InterfaceTableEntrySeq
 */
struct NETIO_DGRAM_InterfaceTableEntrySeq {};
#endif

/*e \dref_NETIO_DGRAM_InterfaceTableEntrySeq_INITIALIZER
 */
#define NETIO_DGRAM_InterfaceTableEntrySeq_INITIALIZER \
       REDA_DEFINE_SEQUENCE_INITIALIZER(struct NETIO_DGRAM_InterfaceTableEntry)

/*e \dref_NETIO_DGRAM_Interface_createFunc
 */
typedef NETIO_Interface_T*
(*NETIO_DGRAM_Interface_createFunc)(NETIO_Interface_T *upstream,void *property);

/*e \dref_NETIO_DGRAMUserInterface_get_interface_listFunc
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*NETIO_DGRAMUserInterface_get_interface_listFunc)(
                NETIO_Interface_T *user_intf,
                struct NETIO_DGRAM_InterfaceTableEntrySeq *if_table)
)

/*e \dref_NETIO_DGRAMUserInterface_resolve_addressFunc
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*NETIO_DGRAMUserInterface_resolve_addressFunc)(
        NETIO_Interface_T *netio_intf,
        const struct NETIO_DGRAM_InterfaceTableEntry *if_entry,
        const char *address_string,
        struct NETIO_Address *address_value,
        RTI_BOOL *is_invalid)
)

/*e \dref_NETIO_DGRAMUserInterface_bind_addressFunc
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*NETIO_DGRAMUserInterface_bind_addressFunc)(
                NETIO_Interface_T *user_intf,
                struct NETIO_Address *source)
)

#ifndef RTI_CERT
/*e \dref_NETIO_DGRAMUserInterface_deleteFunc
 */
typedef void
(*NETIO_DGRAMUserInterface_deleteFunc)(NETIO_Interface_T *user_intf);
#endif

/*ci
 * \brief Function to retrieve the properties use to create the DGRAM
 *   transport. Internal function so the existing UDPv4 transport can be
 *   reused.
 */
NETIO_DGRAMDllExport void
NETIO_DGRAM_Interface_get_netio_property(
        struct NETIO_Interface *intf,
        struct NETIO_InterfaceProperty *property);

/*e \dref_NETIO_DGRAM_InterfaceI
 */
typedef struct DDSCPPDllExport NETIO_DGRAM_InterfaceI
{
    /*e \dref_NETIO_DGRAM_InterfaceI_create_instance
     */
     NETIO_DGRAM_Interface_createFunc create_instance;

#ifndef RTI_CERT
    /*e \dref_NETIO_DGRAM_InterfaceI_delete_instance
     */
     NETIO_DGRAMUserInterface_deleteFunc delete_instance;
#endif
    /*e \dref_NETIO_DGRAM_InterfaceI_get_interface_list
     */
    NETIO_DGRAMUserInterface_get_interface_listFunc get_interface_list;

    /*e \dref_NETIO_DGRAM_InterfaceI_release_address
     */
    NETIO_Interface_release_addressFunc release_address;

    /*e \dref_NETIO_DGRAM_InterfaceI_resolve_address
     */
    NETIO_DGRAMUserInterface_resolve_addressFunc resolve_address;

    /*e \dref_NETIO_DGRAM_InterfaceI_send
     */
    NETIO_Interface_sendFunc send;

   /*e \dref_NETIO_DGRAM_InterfaceI_get_route_table
    */
    NETIO_Interface_get_route_tableFunc get_route_table;

   /*e \dref_NETIO_DGRAM_InterfaceI_bind_address
    */
    NETIO_DGRAMUserInterface_bind_addressFunc bind_address;

} NETIO_DGRAM_InterfaceI;

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*e \dref_NETIO_DGRAM_InterfaceFactory_register
 * \ingroup NETIO_DGRAMInterface
 */
MUST_CHECK_RETURN NETIO_DGRAMDllExport RTI_BOOL
NETIO_DGRAM_InterfaceFactory_register(
              RT_Registry_T *registry,
              const char *name,
              NETIO_DGRAM_InterfaceI *user_intf,
              void *user_property);

/*e \dref_NETIO_DGRAM_InterfaceFactory_unregister
 */
MUST_CHECK_RETURN NETIO_DGRAMDllExport RTI_BOOL
NETIO_DGRAM_InterfaceFactory_unregister(
                RT_Registry_T *registry,
                const char *name);

/*e \dref_NETIO_DGRAM_Interface_upstream_receive
 */
MUST_CHECK_RETURN NETIO_DGRAMDllExport RTI_BOOL
NETIO_DGRAM_Interface_upstream_receive(NETIO_Interface_T *netio_intf,
                                       struct NETIO_Address *source,
                                       NETIO_Packet_T *packet);

/*e \dref_NETIO_DGRAM_Interface_add_route_to_seq
 */
MUST_CHECK_RETURN NETIO_DGRAMDllExport RTI_BOOL
NETIO_DGRAM_Interface_add_route_to_seq(struct NETIO_AddressSeq *address_seq,
                                       struct NETIO_NetmaskSeq *netmask_seq,
                                       const struct NETIO_Address *address,
                                       const struct NETIO_Netmask *netmask);

/*e \dref_NETIO_DGRAM_Interface_add_route_entry_to_seq
 */
MUST_CHECK_RETURN NETIO_DGRAMDllExport RTI_BOOL
NETIO_DGRAM_Interface_add_route_entry_to_seq(
                         struct NETIO_AddressSeq *address_seq,
                         struct NETIO_NetmaskSeq *netmask_seq,
                         const struct NETIO_DGRAM_InterfaceRouteEntry *entry);

/*i \dref_NETIO_DGRAM_Interface_resolve_ipv4_address
 */
MUST_CHECK_RETURN NETIO_DGRAMDllExport RTI_BOOL
NETIO_DGRAM_Interface_resolve_ipv4_address(NETIO_Interface_T *netio_intf,
                            const struct NETIO_DGRAM_InterfaceTableEntry *if_entry,
                            const char *address_string,
                            struct NETIO_Address *address_value,
                            RTI_BOOL *is_invalid);

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif
