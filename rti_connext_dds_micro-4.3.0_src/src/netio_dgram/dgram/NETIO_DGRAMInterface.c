/*
 * FILE: NETIO_DGRAMInterface.c - NETIO_DGRAMInterface implementation
 *
 * Copyright (c) 2022-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 01sep2023,tk MICRO-5552/PR.32162
 * - NETIO_DGRAM_Interface_resolve_ipv4_address
 *   - Changed OSAPI_PRECONDITION_ALWAYS to OSAPI_PRECONDITION since
 *     this function is no longer an API.
 * - NETIO_DGRAM_InterfaceFactory_register
 *   - If user_intf.resolve_address is NULL then use
 *     NETIO_DGRAM_Interface_resolve_ipv4_address as the default address
 *     resolver.
 *   - Remove user_intf->resolve_address != NULL as a precondition
 * 15aug2023,ad MICRO-5486/PR.32114
 * - Updated NETIO_DGRAM_Interface_resolve_ipv4_address to return an error if
 *   the address string is a 3 integer value separated and ending with a '.'
 * 03aug2023,ad MICRO-5464/PR.31927
 * - Updated comments for NETIO_DGRAM_Interface_create_port_entry
 * - Removed incorrect comment in NETIO_DGRAM_Interface_reserve_address
 * 25jul2023,ad MICRO-5418/PR.32064
 * - Updated Interface_receive to also compare the locator kind in the bind_entry
 *   with the source locator kind in case the same port has been reserved
 *   with different locator kinds or an unknown locator kind is passed as source.
 * - Added kind to the comparison in Interface_compare_port to handle the
 *   case where different locator kinds can reserve the same port.
 * 17jul2023,tk MICRO-5364/PR.32033
 * - NETIO_DGRAM_Interface_reserve_address:
 *   - Check for the 0 address before checking for multicast to avoid a 0 address
 *     ever being treated as a multicast address.
 *   - Removed NETIO_ADDRESS_FLAG_MULTICAST assignment, already set by resolver.
 *   - Use NETIO_Address_is_multicast to check if an address is multicast. It
 *     is assumed the that address has been resolved first, before being
 *     reserved.
 * - NETIO_DGRAM_Interface_resolve_ipv4_address:
 *   - Updated to be compatible with
 *     NETIO_DGRAMUserInterface_resolve_adddressFunc type
 *   - Return error if the locator_kind is not NETIO_ADDRESS_KIND_UDPv4
 *   - Removed the test for the multicast group since this is now set by
 *     NETIO_DGRAM_Interface_resolve_address
 *   - Removed setting the NETIO_ADDRESS_KIND_UDPv4 kind since this is
 *     now set by NETIO_DGRAM_Interface_resolve_address
 *   - Initialize all address octets to zero
 * - NETIO_DGRAM_Interface_initialize_user:
 *   - Removed assignment of self->_user_netio_intf.resolve_address since
 *     NETIO_DGRAM_Interface_resolve_address does this.
 *   - Return an error if an interface address is within the interface's
 *     multicast group
 * - NETIO_DGRAM_Interface_resolve_address:
 *   - Query each interface entry to resolve the address. The first one to
 *     return RTI_TRUE is used and the locator kind and multicast flag is set.
 *     based on the interface entry that resolved the entry.
 *   - Zero <address_value> in case the resolve doesn't zero the address.
 * 17jul2023,tk MICRO-5349/PR.32040
 * - NETIO_DGRAM_Interface_reserve_address use NETIO_Address_compare
 *   to check for duplicate addresses in the input argument of reserved
 *   addresses.
 * 10jul2023,ad MICRO-5291/PR.32023
 * - Added NETIO_DGRAM_Interface_upstream_receive as the public API to
 *   forward data for processing data from the downstream interface to
 *   the upstream interface.
 * - Removed precondition check for NETIO_DGRAM_Interface_receive
 * 06Jul2023,ad MICRO-5204/PR.31988
 * - Removed TSeq_loan_contiguous from NETIO_DGRAM_InterfaceTableEntrySeq
 * 29jun2023,ad MICRO-5216/PR.31995
 * - Changed NETIO_ADDRESS_UDPv4_ANY_MULTICAST_INITIALIZER and
 *   NETIO_ADDRESS_UDPv4_ANY_MULTICAST_INITIALIZER to use NETIO_htonl to always
 *   convert the route to big-endian.
 * 25may2023,tk MICRO-4890/PR.31651 Address interface changes
 * - Updated NETIO_DGRAM_Interface_is_multicast_address to create a netmask
 *   and compare in host order to determine if an address is a multicast
 *   group.
 * - Fixed NETIO_DGRAM_Interface_resolve_ipv4_address to correctly convert
 *   the address in network order and use correct size for temp value.
 * 25may2023,ad MICRO-5018
 * - Removed UNUSED_ARG(self); from NETIO_DGRAM_Interface_unbind_external
 * - Removed UNUSED_ARG(self); from NETIO_DGRAM_Interface_create_port_entry
 * - Changed all uses of RTI_SIZEOF(struct NETIO_Guid) to
 *   RTI_SIZEOF(union NETIO_AddressValue)
 * - Added <exists> parameter to the comments for
 *   NETIO_DGRAM_Interface_create_port_entry
 * - Removed self->_parent._rtable = NULL; and self->rx_port_table = NULL;
 *   from NETIO_DGRAM_Interface_create
 * - change “too” to “to” in the comments for NETIO_DGRAM_Interface_add_route
 * - Corrected the description for <netio_intf> parameter in
 *   NETIO_DGRAM_Interface_delete_route
 * - Added <is_invalid> to NETIO_DGRAM_Interface_resolve_address precondition
 *   check
 * - Removed redundant “and” from NETIO_DGRAM_Interface_rollback_reservation
 * - Updated the description of the parameter <resvd_addr> from the comments in
 *   NETIO_DGRAM_Interface_reserve_address
 * - Updated a comment in NETIO_DGRAM_Interface_receive to remove a spelling
 *   error and to change NETIO_Guid to NETIO_AddressValue
 * - Fixed various spelling mistakes
 * 22may2023,tk MICRO-5012/PR.31818
 * - Changed the behavior of NETIO_DGRAM_Interface_reserve_address to return
 *   an error if a requested address is not valid for any of the interfaces.
 *   This would mean that the transport was able to resolve an address, but
 *   that the reseloved address cannot be reserved. This change is in tandem
 *   with MICRO-5013/PR.31819.
 * 22may2023,tk MICRO-4824/PR.31538
 * - Filter out MSB in send call so user will only receive a locator_kind
 *   they specified.
 * - Updated NETIO_DGRAM_Interface_initialize_user to check that registered
 *   locator kinds are in the range [1,0x01ffffff] or
 *   [0x03000000,0x7fffffff] to allow custom transports that implement a
 *   standard locator.
 * - use is_req_addr_shared instead of bit in locator kind to avoid
 *   locator conflicts.
 * 20may2023,tk MICRO-4825/PR.31547
 * - Renamed NETIO_DGRAM_Interface_is_address_multicast to
 *   NETIO_DGRAM_Interface_is_multicast_address
 * - Fixed NETIO_DGRAM_Interface_is_address_multicast to correctly
 *   check that low <= high, address >= low, and address <= high
 *   for the full range of netmasks.
 * 18may2023,ad MICRO-5011/PR.31817
 * - Added a check for the return value of NETIO_Address_set_multicast.
 * 08may2023,tk MICRO-4873/PR.31594
 * - Added check that packet != NULL and that dst_addr == NULL in
 *   NETIO_DGRAM_Interface_receive.
 * 24apr2023,ad MICRO-4815/PR.31519
 * - Removed UDP constants as they are no longer needed.
 * - Changed UDP constants to have uniform naming convention.
 * 19may2023,ad MICRO-5018
 * - Fixed file name in comment header to correctly match file name.
 * 27apr2023,ad MICRO-4842/PR.31552
 * - For consistency, NETIO_DGRAM_Interface_create_port_entry now initializes
 *   <exists> to FALSE.
 * - Renamed NETIO_DGRAM_Interface_create_bind_entry to
 *   NETIO_DGRAM_Interface_create_port_entry
 * 14May2023,ad MICRO-4897/PR.31745
 * - In is_address_multicast, removed redundant kind check.
 * 24apr2023,ad MICRO-4840/PR.31540
 * - NETIO_DGRAM_Interface_resolve_ipv4_address no longer dereferences
 *   is_invalid before validating that the pointer is not NULL.
 * 18apr2023,tk MICRO-4871/PR.31590
 * - Fixed unreserve_address and unbind_external to restore
 *   reference counters in case of failure.
 * 26oct2022,tk Written
 */
#include "NETIO_DGRAMInterface.h"

/*ci \brief Forward declaration or NETIO_DGRAM interface
 */
RTI_PRIVATE struct NETIO_InterfaceI NETIO_DGRAM_Interface_fv_Intf;

/*ci
 * \brief Internal structure used to pass properties between the NETIO_DGRAM
 * interfactory and instance.
 */
struct NETIO_DGRAM_InterfaceFactoryProperty
{
    /*ci
     * \brief baseclass for factory properties
     */
    struct RT_ComponentFactoryProperty _parent;

    /*ci
     * \brief The downstream transport interface structure. Keep a copy
     */
    NETIO_DGRAM_InterfaceI user_intf;

    /*ci
     * \brief Pass through pointer to the downstream transport. Must
     *        be valid for the life-cycle of the transport.
     */
    void *user_property;
};

/*i \brief Forward declaration of NETIO_DGRAM struct
 */
struct NETIO_DGRAM_InterfaceFactory;

/*i \brief Forward declaration of NETIO_DGRAM_Interface_is_multicast_address
 */
RTI_PRIVATE RTI_BOOL
NETIO_DGRAM_Interface_is_multicast_address(
                struct NETIO_DGRAM_InterfaceTableEntry *if_entry,
                struct NETIO_Address *address);

/*i \brief UDPv4 unicast address initializer
 */
#define NETIO_ADDRESS_UDPv4_ANY_UNICAST_INITIALIZER \
{\
    NETIO_ADDRESS_KIND_UDPv4,\
    0,\
    {{0,0,0,0}}\
}

/*i \brief UDPv4 unicast mask initializer
 */
#define NETIO_NETMASK_UDPv4_ANY_UNICAST_MASK_INITIALIZER \
{\
    0,\
    {0,0,0,0}\
}

/*i \brief UDPv4 multicast initializer
 */
#define NETIO_ADDRESS_UDPv4_ANY_MULTICAST_INITIALIZER \
{\
    NETIO_ADDRESS_KIND_UDPv4,\
    0,\
    {{NETIO_htonl(0xe0000000),0,0,0}}\
}

/*i \brief UDPv4 multicast mask initializer
 */
#define NETIO_NETMASK_UDPv4_ANY_MULTICAST_MASK_INITIALIZER \
{\
    4,\
    {0,0,0,0}\
}

/*i \brief UDPv6 unicast address initializer
 */
#define NETIO_ADDRESS_UDPv6_ANY_UNICAST_INITIALIZER \
{\
    NETIO_ADDRESS_KIND_UDPv6,\
    0,\
    {{0,0,0,0}},\
}

/*i \brief UDPv6 unicast mask initializer
 */
#define NETIO_NETMASK_UDPv6_ANY_UNICAST_MASK_INITIALIZER \
{\
    0,\
    {0,0,0,0}\
}

/*i \brief UDPv6 multicast initializer
 */
#define NETIO_ADDRESS_UDPv6_ANY_MULTICAST_INITIALIZER \
{\
    NETIO_ADDRESS_KIND_UDPv6,\
    0,\
    {{NETIO_htonl(0xff000000),0,0,0}},\
}

/*i \brief UDPv6 multicast mask initializer
 */
#define NETIO_NETMASK_UDPv6_ANY_MULTICAST_MASK_INITIALIZER \
{\
    8,\
    {0,0,0,0}\
}

const struct NETIO_DGRAM_InterfaceRouteEntry
NETIO_ADDRESS_UDPv4_ANY_UNICAST_ROUTE =
{
    NETIO_ADDRESS_UDPv4_ANY_UNICAST_INITIALIZER,
    NETIO_NETMASK_UDPv4_ANY_UNICAST_MASK_INITIALIZER
};

const struct NETIO_DGRAM_InterfaceRouteEntry
NETIO_ADDRESS_UDPv4_ANY_MULTICAST_ROUTE =
{
    NETIO_ADDRESS_UDPv4_ANY_MULTICAST_INITIALIZER,
    NETIO_NETMASK_UDPv4_ANY_MULTICAST_MASK_INITIALIZER
};

const struct NETIO_DGRAM_InterfaceRouteEntry
NETIO_ADDRESS_UDPv6_ANY_UNICAST_ROUTE =
{
    NETIO_ADDRESS_UDPv6_ANY_UNICAST_INITIALIZER,
    NETIO_NETMASK_UDPv6_ANY_UNICAST_MASK_INITIALIZER
};

const struct NETIO_DGRAM_InterfaceRouteEntry
NETIO_ADDRESS_UDPv6_ANY_MULTICAST_ROUTE =
{
    NETIO_ADDRESS_UDPv6_ANY_MULTICAST_INITIALIZER,
    NETIO_NETMASK_UDPv6_ANY_MULTICAST_MASK_INITIALIZER
};

/*** SOURCE_BEGIN ***/

/*i \brief Implementation of NETIO_DGRAM_InterfaceTableEntrySeq
 */
#define T struct NETIO_DGRAM_InterfaceTableEntry
#define TSeq NETIO_DGRAM_InterfaceTableEntrySeq
#include "reda/reda_sequence_defn.h"

MUST_CHECK_RETURN RTI_BOOL
NETIO_DGRAM_Interface_resolve_ipv4_address(
                NETIO_Interface_T *netio_intf,
                const struct NETIO_DGRAM_InterfaceTableEntry *if_entry,
                const char *address_string,
                struct NETIO_Address *address_value,
                RTI_BOOL *is_invalid)
{
#define INADDRSZ         4
    RTI_UINT8 tmp[INADDRSZ]={0,0,0,0};
    const RTI_UINT8 *cptr = (RTI_UINT8*)address_string;
    RTI_INT32 digit,index,last_index;
    RTI_BOOL retval = RTI_TRUE;
    UNUSED_ARG(netio_intf);

    OSAPI_PRECONDITION((netio_intf == NULL)
                                || (address_value == NULL)
                                || (if_entry == NULL)
                                || (is_invalid == NULL),
                                return RTI_FALSE,
              OSAPI_Log_entry_add_pointer("netio_intf",
                                          netio_intf,RTI_FALSE);
              OSAPI_Log_entry_add_pointer("if_entry",
                                          if_entry,RTI_FALSE);
              OSAPI_Log_entry_add_pointer("address_value",
                                          address_value,RTI_FALSE);
              OSAPI_Log_entry_add_pointer("is_invalid",
                                          is_invalid,RTI_TRUE);)

    *is_invalid = RTI_FALSE;
    OSAPI_Memory_zero(address_value->value.address.octet,
                      RTI_SIZEOF(address_value->value.address.octet));
    if (if_entry->locator_kind != NETIO_ADDRESS_KIND_UDPv4)
    {
        return RTI_FALSE;
    }

    if (address_string == NULL)
    {
        *is_invalid = RTI_TRUE;
        return RTI_FALSE;
    }

    /* We cannot be 100% sure if the address is valid, assume it is. */
    if (address_string[0] == 0)
    {
        return RTI_TRUE;
    }

    for (index = 0,digit = 0; (digit < INADDRSZ); digit++,index++)
    {
        last_index = index;
        for (; address_string[index] != 0; index++)
        {
            if ((cptr[index] >= '0') && (cptr[index] <= '9'))
            {
                RTI_UINT32 value = tmp[digit];

                /* cannot exceed unsigned 32-bit */
                value = (value * 10U) + (RTI_UINT32)(cptr[index] - '0');
                if (value > 255U)
                {
                    *is_invalid = RTI_TRUE;
                    return RTI_FALSE;
                }
                /* range checked */
                tmp[digit] = (RTI_UINT8)value;
            }
            else if ((cptr[index] == '.') &&
                    (index > last_index)  &&
                    (address_string[index+1] != 0) )
            {
                break;
            }
            else
            {
                return RTI_FALSE;
            }
        }

        if (address_string[index] == 0)
        {
            break;
        }
    }

    if (digit != (INADDRSZ - 1))
    {
        return RTI_FALSE;
    }

    OSAPI_Memory_copy(&address_value->value.ipv4.address,&tmp,INADDRSZ);

    return retval;
}

/*ci
 * \brief Compare entries in the table of NETIO_DGRAM_PortEntry port entries.
 *        The function is compatible with \ref DB_IndexCompare_T
 *
 * \param[in] flags Used to distinguish between op2 being a key or record
 * \param[in] op1   A NETIO_DGRAM_PortEntry already in the database
 * \param[in] op2   Either a NETIO_DGRAM_PortEntry being added or a key
 *                  being searched for
 *
 * \return positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2,
 *         zero if op1 is equal to op2
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_INT32
NETIO_DGRAM_Interface_compare_port(RTI_INT32 flags,
                                  const DB_Record_T op1,
                                  void *op2)
{
    struct NETIO_DGRAM_PortEntry *record_left =
                                            (struct NETIO_DGRAM_PortEntry*)op1;
    const struct NETIO_Address *id_right;
    RTI_INT32 diff_kind;

    if (DB_SELECT_OP2_IS_KEY(flags))
    {
        id_right = (const struct NETIO_Address *)op2;
    }
    else
    {
        id_right = &((struct NETIO_DGRAM_PortEntry*)op2)->source;
    }

    diff_kind = NETIO_Address_get_kind(&record_left->source) -
                                            NETIO_Address_get_kind(id_right);
    if (diff_kind != 0)
    {
         return diff_kind;
    }

    if (record_left->source.port > id_right->port)
    {
        return 1;
    }

    if (record_left->source.port < id_right->port)
    {
        return -1;
    }

    /* A shared address means that only the port has to be unique. Data
     * received on any address on the specific port is treated the same.
     *
     * If an address is not shared, then the port and the address is considered
     * unique. Data received on two different addresses, but the same port are
     * considered different.
     */
    if (record_left->is_shared)
    {
        return 0;
    }

    /* Always compare the full address.
     */
    return OSAPI_Memory_compare(&record_left->source.value,
                                &id_right->value,
                                RTI_SIZEOF(union NETIO_AddressValue));
}

/*ci
 * \brief Create a listener for a NETIO_DGRAM_PortEntry port
 *
 * \details
 *
 * Each NETIO_DGRAM_PortEntry port being listened on has its own receive buffer,
 * A NETIO_DGRAM_PortEntry port-entry can be shared between
 * multiple NETIO interfaces upstream, for example multiple RTPS interfaces
 * can listen to the same DGRAM port.
 *
 * \param[in]  self       NETIO_DGRAM_Interface interface to create entry on
 * \param[in]  src_addr   The address to listen on
 * \param[in]  property   The properties of the address to listen to
 * \param[out] exists     If not NULL, TRUE on output if the address was
 *                        already being listened to
 * \param[in]  is_shared  TRUE if the port entry is shared
 *
 * \return Pointer to new port entry on success, NULL on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE struct NETIO_DGRAM_PortEntry*
NETIO_DGRAM_Interface_create_port_entry(struct NETIO_DGRAM_Interface *self,
                                       struct NETIO_Address *src_addr,
                                       struct NETIOBindProperty *property,
                                       RTI_BOOL *exists,
                                       RTI_BOOL is_shared)
{
    struct NETIO_DGRAM_PortEntry *port_entry = NULL;
    DB_ReturnCode_T dbrc;
    struct NETIO_DGRAM_PortEntry match_entry;

    UNUSED_ARG(property);

    *exists = RTI_FALSE;
    match_entry.source = *src_addr;

    dbrc = DB_Table_select_match(self->rx_port_table,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&port_entry,
                                 (DB_Key_T)&match_entry);

    if (dbrc == DB_RETCODE_OK)
    {
        *exists = RTI_TRUE;
        port_entry->_ref_count++;
        goto done;
    }

    /* Create bind entry */
    dbrc = DB_Table_create_record(self->rx_port_table,
                                  (DB_Record_T *)&port_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        return NULL;
    }

    OSAPI_Memory_zero(port_entry,RTI_SIZEOF(struct NETIO_DGRAM_PortEntry));
    port_entry->source = *src_addr;
    port_entry->_dgram_intf = self;
    port_entry->_ref_count = 1;
    port_entry->is_shared = is_shared;

    dbrc = DB_Table_insert_record(self->rx_port_table,
                                 (DB_Record_T)port_entry);

    if (dbrc != DB_RETCODE_OK)
    {
        dbrc = DB_Table_delete_record(self->rx_port_table,port_entry);

        /* Ignore since failure is returned */
        IGNORE_RETVAL(dbrc);
        return NULL;
    }

done:

    return port_entry;
}

/*ci
 * \brief Compare entries in the table of bind entries. The function is
 *        compatible with \ref DB_IndexCompare_T
 *
 * \param[in] flags Used to distinguish between op2 being a key or record
 * \param[in] op1   A NETIOBindEntry already in the database
 * \param[in] op2   Either a NETIOBindEntry being added or a key
 *                  being searched for
 *
 * \return positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2,
 *         zero if op1 is equal to op2
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_INT32
NETIO_DGRAM_Interface_compare_bind(RTI_INT32 flags,
                                  const DB_Record_T op1,
                                  void *op2)
{
    struct NETIOBindEntry *left_record = (struct NETIOBindEntry*)op1;
    struct NETIO_Address src;
    UNUSED_ARG(flags);

    src = ((struct NETIOBindEntry *)op2)->source;

    /* OSAPI_Memory_compare is used because struct NETIO_Address is 24 bytes
     */
    return OSAPI_Memory_compare(&left_record->source, &src,
                                RTI_SIZEOF(struct NETIO_Address));
}

#ifndef RTI_CERT
/*ci
 * \brief Delete a NETIO_DGRAM interface instance
 *
 * \param[in] netio_intf Interface to delete
 *
 * \sa \ref NETIO_DGRAM_Interface_create
 */
RTI_PRIVATE void
NETIO_DGRAM_Interface_delete(struct NETIO_DGRAM_Interface *netio_intf)
{
    struct NETIO_DGRAM_Interface *self =
                            (struct NETIO_DGRAM_Interface*)netio_intf;

    if (self == NULL)
    {
        return;
    }

    if (!NETIO_Interface_finalize(&self->_parent))
    {
        return;
    }

    if (!NETIO_Interface_finalize(self->user_netio))
    {
        return;
    }

    NETIO_DGRAM_InterfaceTableEntrySeq_finalize(&self->if_table);

    NETIO_AddressSeq_finalize(&self->req_addr);

    if (self->_parent._btable != NULL)
    {
        (void)DB_Database_delete_table(self->property._parent.db,
                                       self->_parent._btable);
    }

    if (self->rx_port_table != NULL)
    {
        (void)DB_Database_delete_table(self->property._parent.db,
                                       self->rx_port_table);
    }

    if (self->user_netio != NULL)
    {
        NETIO_DGRAM_Interface_delete_instance(self,self->user_netio);
    }

    OSAPI_Heap_free_struct(self);
}
#endif

/*ci
 * \brief Initialize the downstream interface
 *
 * \details
 * This function sets up the downstream interface for an instance of the
 * NETIO_DGRAM transport.
 *
 * \param[in] self       NETIO_DGRAM Interface
 * \param[in] dgram_intf Downstream user interface
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_DGRAM_Interface_initialize_user(
                        struct NETIO_DGRAM_Interface *self,
                        struct NETIO_DGRAM_InterfaceI *dgram_intf)
{
    RTI_INT32 seq_index,seq_len;
    struct NETIO_DGRAM_InterfaceTableEntry *seq_entry;

    self->_user_netio_intf.release_address = dgram_intf->release_address;
    self->_user_netio_intf.send = dgram_intf->send;
    self->_user_netio_intf.get_route_table = dgram_intf->get_route_table;

    if (!NETIO_Interface_initialize(self->user_netio,
                                    &self->_user_netio_intf,
                                    NULL,NULL))
    {
        return RTI_FALSE;
    }

    if (!NETIO_DGRAM_Interface_get_interface_list(self,&self->if_table))
    {
#ifndef RTI_CERT
        RTI_BOOL brc;

        brc = NETIO_Interface_finalize(self->user_netio);
        IGNORE_RETVAL(brc);
#endif
        return RTI_FALSE;
    }

    /* Check that that no locators returned have a locator kind <= 0 */
    seq_len = NETIO_DGRAM_InterfaceTableEntrySeq_get_length(&self->if_table);
    for (seq_index = 0; seq_index < seq_len; seq_index++)
    {
        seq_entry = NETIO_DGRAM_InterfaceTableEntrySeq_get_reference(
                                                &self->if_table,seq_index);
        if (seq_entry == NULL)
        {
            break;
        }

        /* RTPS defines the following ranges:
         *
         * 0x00000003 - 0x01ffffff (inclusive) are reserved for vendor-specific
         * Locator_t kinds and will not be used by any future versions of the
         * RTPS protocol.
         *
         * 0x02000000 - 0x02ffffff (inclusive) are reserved for future use by
         * the RTPS specification
         *
         * 0x03000000 and greater are reserved for Locator_t kinds that
         * identify a transport developed by a third-party (i.e., are neither
         * vendor nor protocol-specific) and will not be used by any future
         * versions of the RTPS protocol.
         *
         * Micro reserves:
         * [NETIO_ADDRESS_RESERVED_LOW, NETIO_ADDRESS_RESERVED_HIGH]
         */
        if ((seq_entry->locator_kind <= 0) ||
             ((seq_entry->locator_kind >= NETIO_ADDRESS_RTPS_RESERVED_LOW) &&
              (seq_entry->locator_kind <= NETIO_ADDRESS_RTPS_RESERVED_HIGH)))
        {
            /* break, locator not in the range:
             * - <0,NETIO_ADDRESS_RTPS_RESERVED_LOW>
             * - <NETIO_ADDRESS_RTPS_RESERVED_HIGH,NETIO_ADDRESS_RESERVED_LOW>
             */
            break;
        }

        /* It is only allowed to set number of bits in the netmask, not the
         * mask
         */
        if ((seq_entry->multicast_group.netmask.bits > 128) ||
             (seq_entry->multicast_group.netmask.mask[0] != 0) ||
             (seq_entry->multicast_group.netmask.mask[1] != 0) ||
             (seq_entry->multicast_group.netmask.mask[2] != 0) ||
             (seq_entry->multicast_group.netmask.mask[3] != 0))
        {
            break;
        }

        /* The ports are calculated before an address is reserved,
         * but in the case of the 0 address it is not possible to
         * know if the address is multicast or unicast until after the ports
         * have been calculated. This may cause the wrong ports to
         * be used. Thus, the interface address must be unicast.
         */
        if (NETIO_DGRAM_Interface_is_multicast_address(seq_entry,
                                                        &seq_entry->address))
        {
            break;
        }
    }

    if (seq_index < seq_len)
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
* \brief Return the NETIO properties the NETIO_DGRAM was created with.
*
* \details
* These properties are needed so the existing UDP transport can be a downstream
* interface for the NETIO_DGRAM transport
*
* \param[in]  self     NETIO_DGRAM Interface
* \param[out] property The NETIO properties the transport was created with
*/
void
NETIO_DGRAM_Interface_get_netio_property(
                struct NETIO_Interface *netio_intf,
                struct NETIO_InterfaceProperty *property)
{
    struct NETIO_DGRAM_Interface *self =
                                    (struct NETIO_DGRAM_Interface*)netio_intf;

    *property = self->property;
}
/*ci
 * \brief Create a new NETIO_DGRAM_Interface instance
 *
 * \param[in] factory   Factory creating the new instance
 * \param[in] property  The property of the new NETIO_DGRAM interface
 * \param[in] listener  The listener for the new NETIO_DGRAM interface
 *
 * \return Pointer to new NETIO_DGRAM interface instance on success, NULL
 *         on failure
 *
 * \sa \ref NETIO_DGRAM_Interface_delete
 */
MUST_CHECK_RETURN RTI_PRIVATE struct NETIO_DGRAM_Interface*
NETIO_DGRAM_Interface_create(struct NETIO_DGRAM_InterfaceFactory *factory,
                        const struct NETIO_InterfaceProperty *const property,
                        const struct NETIO_InterfaceListener *const listener)
{
    struct NETIO_DGRAM_Interface *self = NULL;
    struct DB_TableProperty tbl_prop = DB_TableProperty_INITIALIZER;
    char tbl_name[NETIO_TABLE_NAME_SIZE];
    union RT_ComponentFactoryId id;
    DB_ReturnCode_T dbrc;
    struct NETIO_DGRAM_Interface *retval = NULL;

    if (property == NULL)
    {
        return NULL;
    }

    OSAPI_Heap_allocate_struct(&self,struct NETIO_DGRAM_Interface);
    if (self == NULL)
    {
        return NULL;
    }

    OSAPI_Memory_zero(self,RTI_SIZEOF(struct NETIO_DGRAM_Interface));

    self->factory = factory;
    self->property = *property;

    if (!NETIO_Interface_initialize(&self->_parent,
                                    &NETIO_DGRAM_Interface_fv_Intf,
                                    (struct NETIO_InterfaceProperty*)property,
                                    listener))
    {
#ifndef RTI_CERT
        OSAPI_Heap_free_struct(self);
#endif
        return NULL;
    }

    self->user_netio = NETIO_DGRAM_Interface_create_instance(self);
    if (self->user_netio == NULL)
    {
#ifndef RTI_CERT
        OSAPI_Heap_free_struct(self);
#endif
        return NULL;
    }


    if (!NETIO_DGRAM_InterfaceTableEntrySeq_initialize(&self->if_table))
    {
        goto done;
    }

    if (!NETIO_DGRAM_InterfaceTableEntrySeq_set_maximum(&self->if_table,
                                    NETIO_DGRAM_INTERFACE_MAX_INTERFACES))
    {
        goto done;
    }

    if (!NETIO_AddressSeq_initialize(&self->req_addr))
    {
        goto done;
    }

    if (!NETIO_AddressSeq_set_maximum(&self->req_addr,
                                      NETIO_DGRAM_INTERFACE_MAX_ADDRESSES))
    {
        goto done;
    }


    if (!NETIO_DGRAM_Interface_initialize_user(self,&factory->user_intf))
    {
        goto done;
    }

    id._value = factory->_parent._id._value;
    NETIO_Interface_Table_name_from_id(tbl_name,&id,'b',
                                       factory->instance_counter);

    tbl_prop.max_records = property->max_binds;
    dbrc = DB_Database_create_table(&self->_parent._btable,
                                property->_parent.db,&tbl_name[0],
                                RTI_SIZEOF(struct NETIO_DGRAM_BindEntry),
                                NETIO_DGRAM_Interface_compare_bind,&tbl_prop);

    if (dbrc != DB_RETCODE_OK)
    {
        UDP_LOG_UDP_CREATE_TABLE(OSAPI_LOGKIND_ERROR,tbl_name,dbrc)
        goto done;
    }

    tbl_prop.max_records = property->max_binds;
    NETIO_Interface_Table_name_from_id(tbl_name,&id,'p',
                                       factory->instance_counter);

    dbrc = DB_Database_create_table(&self->rx_port_table,
                            property->_parent.db,&tbl_name[0],
                            RTI_SIZEOF(struct NETIO_DGRAM_PortEntry),
                            NETIO_DGRAM_Interface_compare_port,&tbl_prop);

    if (dbrc != DB_RETCODE_OK)
    {
        UDP_LOG_UDP_CREATE_TABLE(OSAPI_LOGKIND_ERROR,tbl_name,dbrc)
        goto done;
    }

    /* Create an interface address. Use the name notation as for the table
     * name, but use - as table kind.
     */
    NETIO_Address_init(&self->_parent.local_address,
                       NETIO_ADDRESS_KIND_INTRA);

    NETIO_Interface_Table_name_from_id(
                    (char*)&self->_parent.local_address.value.guid,&id,'-',
                    factory->instance_counter);

    ++factory->instance_counter;


    retval = self;

done:

#ifndef RTI_CERT
    if (retval == NULL)
    {
        NETIO_DGRAM_Interface_delete(self);
    }
#endif /* !RTI_CERT */

    return retval;
}

/*ci
 * \brief Implementation of the NETIO_Interface_send function
 *
 * \details
 *
 * This function delegates sending the packet to the downstream
 * interface. One simplification is that the downstream interface
 * is called once per address in packet's destination sequence since
 * the packet's destination sequence is internal.
 *
 * \param[in] netio_intf NETIO interface to send from
 * \param[in] source     The source of the packet
 * \param[in] address    The destination address
 * \param[in] packet     The packet to send
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_DGRAM_Interface_send(NETIO_Interface_T *netio_intf,
                    struct NETIO_Interface *source,
                    struct NETIO_Address *address,
                    NETIO_Packet_T *packet)
{
    struct NETIO_DGRAM_Interface *self =
                            (struct NETIO_DGRAM_Interface*)netio_intf;
    RTI_INT32 length,index;
    RTI_BOOL retval;

    UNUSED_ARG(address);

    OSAPI_PRECONDITION((netio_intf == NULL) ||
                       (source == NULL) || (packet == NULL),
                       return RTI_FALSE,
               OSAPI_Log_entry_add_pointer("netio_intf",netio_intf,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("source",source,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("packet",packet,RTI_TRUE);)

    /* no destinations to which to send */
    if (packet->dests == NULL)
    {
        return RTI_FALSE;
    }

    length = NETIO_AddressSeq_get_length(packet->dests);
    for (index = 0; index < length; ++index)
    {
        struct NETIO_Address *an_address;
        RTI_INT32 orig_kind;

        an_address = NETIO_AddressSeq_get_reference(packet->dests, index);
        if (an_address == NULL)
        {
            /* This is unexpected, but continue */
            continue;
        }
        /* filter out MSB, if set */
        orig_kind = an_address->kind;
        an_address->kind = NETIO_Address_kind(orig_kind);

        /* We own the packet and the packet->dests. We do not need to
         * sanitize the data.
         */
        /* coverity[tainted_data] */
        retval = NETIO_Interface_send(self->user_netio,
                                      source,an_address,packet);
         an_address->kind = orig_kind;
        /* Ignore the return code because there is no way of knowing what failed
         */
        IGNORE_RETVAL(retval);
    }

    return RTI_TRUE;
}

/*ci
 * \brief Implementation of the NETIO add_route function
 *
 * \details
 *
 * The NETIO_DGRAM interface does not keep track of any route since it is
 * stateless. An implementation is included to comply with the NETIO design.
 *
 * \param[in] netio_intf NETIO interface to add the route to
 * \param[in] dst_addr   The destination address for the route
 * \param[in] via_intf   The downstream interface
 * \param[in] via_addr   The address to pass to the downstream interface
 * \param[in] property   The route property
 * \param[in] existed    Whether the route already existed
 *
 * \return Always returns RTI_TRUE.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_DGRAM_Interface_add_route(NETIO_Interface_T *netio_intf,
                                struct NETIO_Address *dst_addr,
                                NETIO_Interface_T *via_intf,
                                struct NETIO_Address *via_addr,
                                struct NETIORouteProperty *property,
                                RTI_BOOL *existed)
{
    UNUSED_ARG(netio_intf);
    UNUSED_ARG(dst_addr);
    UNUSED_ARG(via_intf);
    UNUSED_ARG(via_addr);
    UNUSED_ARG(property);
    UNUSED_ARG(existed);

    return RTI_TRUE;
}

/*ci
 * \brief Implementation of the NETIO delete_route function
 *
 * \details
 *
 * The NETIO_DGRAM interface does not keep track of any routes, but implements
 * this function to be compliant with the NETIO interface and minimize the
 * burden on the controller to keep track of which interface maintains
 * state and not.
 *
 * \param[in]  netio_intf NETIO interface to delete the route from.
 * \param[in]  dst_addr   The destination address for the route
 * \param[in]  via_intf   The downstream interface
 * \param[in]  via_addr   The address to pass to the downstream interface
 * \param[out] existed    Whether the route existed
 *
 * \return Always returns RTI_TRUE.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_DGRAM_Interface_delete_route(NETIO_Interface_T *netio_intf,
                            struct NETIO_Address *dst_addr,
                            NETIO_Interface_T *via_intf,
                            struct NETIO_Address *via_addr,
                            RTI_BOOL *existed)
{
    UNUSED_ARG(netio_intf);
    UNUSED_ARG(dst_addr);
    UNUSED_ARG(via_intf);
    UNUSED_ARG(via_addr);
    UNUSED_ARG(existed);

    return RTI_TRUE;
}

/*ci
 * \brief Implementation of the NETIO bind function
 *
 * \details
 *
 * NETIO_DGRAM does not maintain any state information about its peer, but this
 * function is implemented to comply with the NETIO interface and not burden
 * a controller with knowing which interface maintains state or not.
 *
 * \param[in]  netio_intf NETIO interface to bind
 * \param[in]  src_addr   The address to bind to
 * \param[in]  property   The property to use for the bind
 * \param[out] existed    Whether a previous bind existed or not
 *
 * \return Always returns RTI_TRUE.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_DGRAM_Interface_bind(NETIO_Interface_T *netio_intf,
                                 struct NETIO_Address *src_addr,
                                 struct NETIOBindProperty *property,
                                 RTI_BOOL *existed)
{
    UNUSED_ARG(netio_intf);
    UNUSED_ARG(src_addr);
    UNUSED_ARG(property);
    UNUSED_ARG(existed);

    return RTI_TRUE;
}

/*ci
 * \brief Implementation of the NETIO unbind function
 *
 * \details
 *
 * NETIO_DGRAM does not maintain any state information about its peer, but this
 * function is implemented to comply with the NETIO interface and not burden
 * a controller with knowing which interface maintains state or not.
 *
 * \param[in]  netio_intf NETIO interface to bind
 * \param[in]  src_addr   The address to bind to
 * \param[in]  dst_intf   Interface
 * \param[out] existed    Whether a bind existed or not
 *
 * \return Always returns RTI_TRUE.
 *
 * \sa \ref NETIO_DGRAM_Interface_bind
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_DGRAM_Interface_unbind(NETIO_Interface_T *netio_intf,
                      struct NETIO_Address *src_addr,
                      NETIO_Interface_T *dst_intf,
                      RTI_BOOL *existed)
{
    UNUSED_ARG(netio_intf);
    UNUSED_ARG(src_addr);
    UNUSED_ARG(dst_intf);
    UNUSED_ARG(existed);

    return RTI_TRUE;
}

/*ci
 * \brief Implementation of the NETIO bind_external function
 *
 * \details
 *
 * When an upstream interface want to listen to a NETIO_DGRAM interface it
 * binds to the downstream interface using the external bind function.
 * For DGRAM this means adding an interface to a bind table.
 *
 * \param[in]  netio_intf NETIO interface to bind to upstream interface
 * \param[in]  src_addr   The address to bind to
 * \param[in]  dst_intf   The upstream interface
 * \param[in]  dst_addr   The address to pass to the upstream interface
 * \param[in]  property   The properties for the bind
 * \param[out] existed     Whether a previous bind already existed for
 *                         this entry or not.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * \sa \ref NETIO_DGRAM_Interface_unbind_external
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_DGRAM_Interface_bind_external(NETIO_Interface_T *netio_intf,
                             struct NETIO_Address *src_addr,
                             NETIO_Interface_T *dst_intf,
                             struct NETIO_Address *dst_addr,
                             struct NETIOBindProperty *property,
                             RTI_BOOL *existed)
{
    struct NETIO_DGRAM_Interface *self =
                                    (struct NETIO_DGRAM_Interface*)netio_intf;
    struct NETIO_DGRAM_BindEntry *bind_entry = NULL;
    struct NETIO_DGRAM_PortEntry *port_entry = NULL;
    DB_ReturnCode_T dbrc;
    struct NETIOBindEntryKey bind_key;
    struct NETIO_Address src_address;
    UNUSED_ARG(property);

    OSAPI_PRECONDITION((self == NULL) ||
                        (src_addr == NULL) ||
                        (dst_intf == NULL) ||
                        (dst_addr == NULL),
                        return RTI_FALSE,
                OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                OSAPI_Log_entry_add_pointer("src_addr",src_addr,RTI_FALSE);
                OSAPI_Log_entry_add_pointer("dst_intf",dst_intf,RTI_FALSE);
                OSAPI_Log_entry_add_pointer("dst_addr",dst_addr,RTI_TRUE);)

    if (existed)
    {
        *existed = RTI_FALSE;
    }

    src_address = *src_addr;

    /* Find the receive entry */
    dbrc = DB_Table_select_match(self->rx_port_table,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&port_entry,
                                 (DB_Key_T)&src_address);
    if (dbrc != DB_RETCODE_OK)
    {
        return RTI_FALSE;
    }

    bind_key.source = src_address;
    bind_key.destination = *dst_addr;

    dbrc = DB_Table_select_match(self->_parent._btable,
                                DB_TABLE_DEFAULT_INDEX,
                                (DB_Record_T*)&bind_entry,
                                (DB_Key_T)&bind_key);
    if (dbrc == DB_RETCODE_OK)
    {
        if (existed)
        {
            *existed = RTI_TRUE;
        }
        ++bind_entry->ref_count;

        return RTI_TRUE;
    }

    dbrc = DB_Table_create_record(self->_parent._btable,
                                  (DB_Record_T*)&bind_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        UDP_LOG_RECORD(OSAPI_LOGKIND_ERROR,dbrc)
        return RTI_FALSE;
    }

    bind_entry->_parent.source = src_address;
    bind_entry->_parent.destination = *dst_addr;
    bind_entry->_parent.intf = dst_intf;
    bind_entry->ref_count = 1;
    bind_entry->is_shared = port_entry->is_shared;

    dbrc = DB_Table_insert_record(self->_parent._btable,
                                  (DB_Record_T)bind_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        UDP_LOG_RECORD(OSAPI_LOGKIND_ERROR,dbrc)
        (void)DB_Table_delete_record(self->_parent._btable,
                                     (DB_Record_T)bind_entry);
        return RTI_FALSE;
    }

    ++port_entry->_ref_count;

    return RTI_TRUE;
}

/*ci
 * \brief  Implementation of the NETIO unbind_external function
 *
 * \details
 *
 * When an upstream interface want to remove a listener to a NETIO_DGRAM
 * interface it unbinds to the downstream interface using the external
 * unbind function.
 *
 * \param[in]  netio_intf NETIO interface to unbind from upstream interface
 * \param[in]  src_addr   The address to unbind from
 * \param[in]  dst_intf   The upstream interface
 * \param[in]  dst_addr   The address to passed to the upstream interface
 * \param[out] existed     Whether a previous bind already existed for this
 *                         entry or not.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * \sa \ref NETIO_DGRAM_Interface_bind_external
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_DGRAM_Interface_unbind_external(NETIO_Interface_T *netio_intf,
                                     struct NETIO_Address *src_addr,
                                     NETIO_Interface_T *dst_intf,
                                     struct NETIO_Address *dst_addr,
                                     RTI_BOOL *existed)
{
    struct NETIO_DGRAM_Interface *self =
                            (struct NETIO_DGRAM_Interface*)netio_intf;
    struct NETIO_DGRAM_BindEntry *bind_entry = NULL;
    struct NETIO_DGRAM_BindEntry *rm_bind_entry = NULL;
    DB_ReturnCode_T dbrc;
    struct NETIOBindEntryKey bind_key;
    struct NETIO_DGRAM_PortEntry *port_entry = NULL;
    struct NETIO_Address src_address;
    PRECOND_ARG(dst_intf)

    OSAPI_PRECONDITION((self == NULL) ||
                       (src_addr == NULL) ||
                       (dst_intf == NULL) ||
                       (dst_addr == NULL),
                       return RTI_FALSE,
                OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                OSAPI_Log_entry_add_pointer("src_addr",src_addr,RTI_FALSE);
                OSAPI_Log_entry_add_pointer("dst_intf",dst_intf,RTI_FALSE);
                OSAPI_Log_entry_add_pointer("dst_addr",dst_addr,RTI_TRUE);)

    if (existed)
    {
        *existed = RTI_FALSE;
    }

    src_address = *src_addr;

    bind_key.source = src_address;
    bind_key.destination = *dst_addr;

    dbrc = DB_Table_select_match(self->_parent._btable,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&bind_entry,
                                 (DB_Key_T)&bind_key);
    if (dbrc != DB_RETCODE_OK)
    {
        return RTI_TRUE;
    }

    if (existed)
    {
        *existed = RTI_TRUE;
    }

    --bind_entry->ref_count;

    if (bind_entry->ref_count > 0)
    {
        return RTI_TRUE;
    }

    dbrc = DB_Table_remove_record(self->_parent._btable,
                                (DB_Record_T*)&rm_bind_entry,
                                (DB_Key_T)&bind_key);
    if (dbrc != DB_RETCODE_OK)
    {
        bind_entry->ref_count++;
        UDP_LOG_RECORD(OSAPI_LOGKIND_ERROR,dbrc)
        return RTI_FALSE;
    }

    /* The record had been removed from the database and
     * resources have been released.
     */
    dbrc = DB_Table_delete_record(self->_parent._btable,
                                 (DB_Record_T)rm_bind_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        UDP_LOG_RECORD(OSAPI_LOGKIND_WARNING,dbrc)
        return RTI_FALSE;
    }

    /* Find the receive entry */
    dbrc = DB_Table_select_match(self->rx_port_table,
                                DB_TABLE_DEFAULT_INDEX,
                                (DB_Record_T*)&port_entry,
                                (DB_Key_T)&src_address);
    if (dbrc != DB_RETCODE_OK)
    {
        UDP_LOG_RECORD(OSAPI_LOGKIND_ERROR,dbrc)
        return RTI_FALSE;
    }

    --port_entry->_ref_count;

    return RTI_TRUE;
}

/*ci
 * \brief Implementation of the NETIO set_state function
 *
 * \details
 * NETIO_DGRAM does not maintain any state information this
 * function is implemented to comply with the NETIO interface and not burden
 * a controller with knowing which interface maintains state or not.
 *
 * \param[in] netio_intf NETIO interface to set state on
 * \param[in] state      New state
 *
 * \return Always returns RTI_TRUE.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_DGRAM_Interface_set_state(NETIO_Interface_T *netio_intf,
                               NETIO_InterfaceState_T state)
{
    UNUSED_ARG(netio_intf);
    UNUSED_ARG(state);


    return RTI_TRUE;
}

/*ci
 * \brief Implementation of the NETIO resolve_address function
 *
 * \details
 *
 * Call the downstream interface to resolve an address
 *
 * \param[out] netio_intf     Interface
 * \param[in]  address_string Address to convert
 * \param[out] address_value  Converted address on success
 * \param[out] is_invalid     Whether the address is valid or not
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_DGRAM_Interface_resolve_address(NETIO_Interface_T *netio_intf,
                               const char *address_string,
                               struct NETIO_Address *address_value,
                               RTI_BOOL *is_invalid)
{
    struct NETIO_DGRAM_Interface *self =
                                 (struct NETIO_DGRAM_Interface*)netio_intf;
    RTI_INT32 if_len,if_index;
    struct NETIO_DGRAM_InterfaceTableEntry *if_entry;
    RTI_BOOL retval = RTI_FALSE;
    union NETIO_AddressValue zero_address = {{0,0,0,0}};

    OSAPI_PRECONDITION((netio_intf == NULL)
                              || (address_string == NULL)
                              || (address_value == NULL)
                              || (is_invalid == NULL),
                              return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("netio_intf",
                                        netio_intf,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("address_string",
                                        address_string,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("address_value",
                                        address_value,RTI_TRUE);
            OSAPI_Log_entry_add_pointer("is_invalid",
                                        is_invalid,RTI_TRUE);)

    *is_invalid = RTI_FALSE;
    OSAPI_Memory_zero(address_value->value.address.octet,
                                RTI_SIZEOF(address_value->value.address.octet));

    if_len = NETIO_DGRAM_InterfaceTableEntrySeq_get_length(&self->if_table);

    for (if_index = 0; (if_index < if_len); if_index++)
    {
        if_entry = NETIO_DGRAM_InterfaceTableEntrySeq_get_reference(
                                            &self->if_table,if_index);
        if (if_entry == NULL)
        {
            break;
        }

        retval = NETIO_DGRAM_InterfaceEntry_resolve_address(self,if_entry,
                                address_string,address_value,is_invalid);

        if (!retval && *is_invalid)
        {
            break;
        }

        if (retval)
        {
            address_value->kind = if_entry->locator_kind;

            /* The 0 address is always a unicast address */
            if ((OSAPI_Memory_compare(&address_value->value,&zero_address,
                                      RTI_SIZEOF(zero_address)) != 0) &&
                (NETIO_DGRAM_Interface_is_multicast_address(if_entry,
                                                            address_value)))
            {
                /* Do not call NETIO_Address_set_multicast, this cannot fail */
                address_value->kind |= (RTI_INT32)NETIO_ADDRESS_FLAG_MULTICAST;
            }
            break;
        }
    }

    return retval;
}

/*ci
 * \brief Check if an address is a multicast address
 *
 * \param[in]  if_entry  NETIO_DGRAM Interface the check the address with
 * \param[in]  address  Address to check
 *
 * \return RTI_TRUE if the address is a multicast address, RTI_FALSE if not.
 */
RTI_PRIVATE RTI_BOOL
NETIO_DGRAM_Interface_is_multicast_address(
                struct NETIO_DGRAM_InterfaceTableEntry *if_entry,
                struct NETIO_Address *address)
{
    RTI_UINT32 i;
    RTI_UINT32 mask_len;
    RTI_UINT32 left,right_low,right_high,mask;
    RTI_BOOL left_ge_low = RTI_FALSE;
    RTI_BOOL low_le_high = RTI_FALSE;
    RTI_BOOL left_le_high = RTI_FALSE;
    struct NETIO_Netmask tmp_mask;
    struct NETIO_DGRAM_InterfaceMultiCastGroup *mgrp;

    mgrp = &if_entry->multicast_group;

    if (mgrp->netmask.bits == 0)
    {
        return RTI_FALSE;
    }

    mask_len = (mgrp->netmask.bits / 32) + ((mgrp->netmask.bits % 32) ? 1 : 0);

    tmp_mask = mgrp->netmask;
    if (!NETIO_RouteResolver_fill_netmask(&tmp_mask))
    {
        return RTI_FALSE;
    }

    for (i = 0; i < mask_len; ++i)
    {
        mask = tmp_mask.mask[i];
        right_low = NETIO_ntohl(mgrp->address_low.value.as_uint32.value[i]) & mask;
        right_high = NETIO_ntohl(mgrp->address_high.value.as_uint32.value[i]) & mask;
        left = NETIO_ntohl(address->value.as_uint32.value[i]) & mask;

        if (!low_le_high && (right_low != right_high))
        {
            if (right_low > right_high)
            {
                return RTI_FALSE;
            }
            else
            {
                /* stop checking if low > high, cannot be the case */
                low_le_high = RTI_TRUE;
            }
        }

        if (!left_ge_low && (left != right_low))
        {
            if (left < right_low)
            {
                return RTI_FALSE;
            }
            else
            {
                /* stop checking if left < low, cannot be the case */
                left_ge_low = RTI_TRUE;
            }
        }

        if (!left_le_high && (left != right_high))
        {
            if (left > right_high)
            {
                return RTI_FALSE;
            }
            else
            {
                /* stop checking if left > high, cannot be the case */
                left_le_high = RTI_TRUE;
            }
        }

        if (low_le_high && left_ge_low && left_le_high)
        {
            /* low <= address <= high */
            break;
        }
    }

    /* low <= address <= high */

    return RTI_TRUE;
}

/*ci
* \brief Unreserve an address and optionally release downstream.
*
* \param[in] self     NETIO_DGRAM Interface
* \param[in] address  Address to unreserve
* \param[in] release  If TRUE, also call downstream to release address
*/
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_DGRAM_Interface_unreserve_address(struct NETIO_DGRAM_Interface *self,
                                        struct NETIO_Address *address,
                                        RTI_BOOL release)
{
    struct NETIO_DGRAM_PortEntry *port_entry = NULL;
    struct NETIO_DGRAM_PortEntry *rm_port_entry = NULL;

    DB_ReturnCode_T dbrc;
    RTI_BOOL retval = RTI_FALSE;
    struct NETIO_Address src_address;

    dbrc = DB_Table_select_match(self->rx_port_table,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&port_entry,
                                 (DB_Key_T)address);

    if (dbrc != DB_RETCODE_OK)
    {
        goto done;
    }

    port_entry->_ref_count--;
    if (port_entry->_ref_count > 0)
    {
        retval = RTI_TRUE;
        goto done;
    }

    dbrc = DB_Table_remove_record(self->rx_port_table,
                                  (DB_Record_T*)&rm_port_entry,
                                  (DB_Key_T)address);

    if (dbrc != DB_RETCODE_OK)
    {
        port_entry->_ref_count++;
        goto done;
    }

    if (release)
    {
        src_address = *address;
        src_address.kind = NETIO_Address_kind(src_address.kind);
        if (!NETIO_Interface_release_address(self->user_netio,&src_address))
        {
            port_entry->_ref_count++;
            dbrc = DB_Table_insert_record(self->rx_port_table,
                                          (DB_Record_T*)port_entry);

            /* Ignore the return value since the call has already failed
             * and there is nothing that can be done.
             */
            IGNORE_RETVAL(dbrc);
            goto done;
        }
    }

    dbrc = DB_Table_delete_record(self->rx_port_table,
                                  (DB_Record_T)rm_port_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        /* The record had been removed from the database and resources
         * have been released.
         */
        goto done;
    }

    retval = RTI_TRUE;

done:

    return retval;
}

/*ci
 * \brief Rollback a failed reservation
 *
 * \param[in] self     NETIO_DGRAM Interface
 * \param[in] req_addr Sequence of reservations to rollback
 * \param[in] first    First index in sequence included in rollback
 * \param[in] last     Last index in sequence not included in the rollback
 *                     unless is_bind_failure is TRUE
 * \param[in] is_bind_failure  If TRUE this was a bind failure and
 *                             the last port reserved is the last index.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
RTI_PRIVATE RTI_BOOL
NETIO_DGRAM_Interface_rollback_reservation(struct NETIO_DGRAM_Interface *self,
                                          struct NETIO_AddressSeq *req_addr,
                                          RTI_INT32 first,
                                          RTI_INT32 last,
                                          RTI_BOOL is_bind_failure)
{
    RTI_INT32 index;
    struct NETIO_Address *an_address;


    for (index = first; index < last; index++)
    {
        an_address = NETIO_AddressSeq_get_reference(req_addr,index);
        if (!NETIO_DGRAM_Interface_unreserve_address(self,an_address,RTI_TRUE))
        {
            return RTI_FALSE;
        }
    }

    if (is_bind_failure)
    {
        /* If this is a bind failure, then include the next address since
         * a port was successfully created and should be unreserved, but
         * not released.
         */
        an_address = NETIO_AddressSeq_get_reference(req_addr,index);
        if (!NETIO_DGRAM_Interface_unreserve_address(self,an_address,RTI_FALSE))
        {
            return RTI_FALSE;
        }
    }

    return RTI_TRUE;
}

/*ci
 * \brief Implementation of the NETIO reserve_address function
 *
 * \param[in]    netio_intf NETIO interface to reserve addresses on
 * \param[in]    req_addr   List of requested addresses
 * \param[inout] resvd_addr The list of addresses that are reserved
 * \param[in]    property   Properties to use to listen on the
                            reserved addresses
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * \sa \ref NETIO_DGRAM_Interface_release_address
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_DGRAM_Interface_reserve_address(NETIO_Interface_T *netio_intf,
                                     struct NETIO_AddressSeq *req_addr,
                                     struct NETIO_AddressSeq *resvd_addr,
                                     struct NETIOBindProperty *property)
{
    struct NETIO_DGRAM_Interface *self =
                                    (struct NETIO_DGRAM_Interface*)netio_intf;
    RTI_BOOL retval = RTI_FALSE;
    RTI_INT32 max_size;
    RTI_INT32 cur_addr_len;
    RTI_INT32 i, length;
    struct NETIO_Address *an_address = NULL;
    struct NETIO_Address src_address = NETIO_Address_INITIALIZER;
    struct NETIO_DGRAM_PortEntry *port_entry;
    RTI_INT32 k;
    union NETIO_AddressValue zero_address = {{0,0,0,0}};
    RTI_INT32 if_len, if_index;
    struct NETIO_DGRAM_InterfaceTableEntry *if_entry;
    RTI_INT32 j;
    struct NETIO_Address *left_addr;
    RTI_BOOL brc;

    cur_addr_len = NETIO_AddressSeq_get_length(resvd_addr);
    max_size = NETIO_AddressSeq_get_maximum(resvd_addr) - cur_addr_len;

    if (max_size == 0)
    {
        return RTI_TRUE;
    }

    if (!NETIO_AddressSeq_set_length(&self->req_addr,0))
    {
        return RTI_FALSE;
    }

    self->is_req_addr_shared = 0;

    if_len = NETIO_DGRAM_InterfaceTableEntrySeq_get_length(&self->if_table);

    for (k = 0,i = 0; (k < NETIO_AddressSeq_get_length(req_addr)) &&
                      (i < max_size); ++k)
    {
        RTI_BOOL addr_found = RTI_FALSE;

        an_address = NETIO_AddressSeq_get_reference(req_addr,k);

        for (if_index = 0; (if_index < if_len); if_index++)
        {
            if_entry = NETIO_DGRAM_InterfaceTableEntrySeq_get_reference(
                                                &self->if_table,if_index);
            if (if_entry == NULL)
            {
                break;
            }

            if (NETIO_Address_kind(an_address->kind) !=
                NETIO_Address_kind(if_entry->locator_kind))
            {
                /* locator_kind not supported by this interface */
                continue;
            }

            src_address.kind = an_address->kind;
            src_address.port = an_address->port;

            /* The 0 address is */
            if (OSAPI_Memory_compare(&an_address->value,&zero_address,
                                  RTI_SIZEOF(union NETIO_AddressValue)) == 0)
            {
                /* Use the interface address. Note that for IPv4, the address
                 * is assumed to be in the first 4 bytes.
                 */
                OSAPI_Memory_copy(&src_address.value,&if_entry->address.value,
                                  RTI_SIZEOF(union NETIO_AddressValue));
            }
            else if (NETIO_Address_is_multicast(an_address))
            {
                src_address.value.guid = an_address->value.guid;
            }
            else if (OSAPI_Memory_compare(&an_address->value,
                                  &if_entry->address.value,
                                  RTI_SIZEOF(union NETIO_AddressValue)) == 0)
            {
                /* If the interface address is specified, it must be the
                 * same as the interface address or a multicast. This
                 * allows us to filter out addresses that are listed
                 * but does not exist.
                 */
                src_address.value.guid = an_address->value.guid;
             }
            else
            {
                /* Address not valid for interface */
                continue;
            }

            addr_found = RTI_TRUE;

            for (j = 0; j < NETIO_AddressSeq_get_length(resvd_addr); ++j)
            {
                left_addr = NETIO_AddressSeq_get_reference(resvd_addr,j);
                if (NETIO_Address_compare(left_addr,&src_address) == 0)
                {
                    break;
                }
            }

            /* duplicate address, do not add */
            if (j < NETIO_AddressSeq_get_length(resvd_addr))
            {
                continue;
            }

            if (!NETIO_AddressSeq_set_length(resvd_addr,cur_addr_len + i + 1))
            {
                goto done;
            }

            *NETIO_AddressSeq_get_reference(resvd_addr,cur_addr_len + i) =
                                            src_address;

            if (!NETIO_AddressSeq_set_length(&self->req_addr,i + 1))
            {
                goto done;
            }

            *NETIO_AddressSeq_get_reference(&self->req_addr,i) = src_address;

            /* Ports are never shared across multicast addresses */
            if (!NETIO_Address_is_multicast(&src_address)
                && (if_entry->flags & NETIO_DGRAM_INTERFACE_SHARED_PORT_FLAG))
            {
                self->is_req_addr_shared |= 1U << i;
            }

            ++i;
        }

        if (!addr_found)
        {
            /* The address was not valid for any interface, return error */
            goto done;
        }
    }

    length = NETIO_AddressSeq_get_length(&self->req_addr);
    for (i = 0; i < length; ++i)
    {
        RTI_BOOL exists = RTI_FALSE;
        RTI_BOOL is_shared;

        is_shared = ((1U << i) & self->is_req_addr_shared ? RTI_TRUE : RTI_FALSE);

        an_address = NETIO_AddressSeq_get_reference(&self->req_addr, i);
        port_entry = NETIO_DGRAM_Interface_create_port_entry(self,
                                                            an_address,
                                                            property,
                                                            &exists,
                                                            is_shared);
        if (port_entry == NULL)
        {
            brc = NETIO_DGRAM_Interface_rollback_reservation(self,&self->req_addr,
                                                            0,i,
                                                            RTI_FALSE);
            /* returning failure regardless */
            IGNORE_RETVAL(brc);
            goto done;
        }

        if (exists)
        {
            continue;
        }

        /* Remove all additional flags before asking the downstream
         * interface to bind to the user cannot make a mistake.
         */
        src_address = *an_address;
        src_address.kind = NETIO_Address_kind(src_address.kind);

        if (!NETIO_DGRAM_Interface_bind_address(self,&src_address))
        {
            brc = NETIO_DGRAM_Interface_rollback_reservation(self,&self->req_addr,
                                                            0,i,
                                                            RTI_TRUE);
            /* returning failure regardless */
            IGNORE_RETVAL(brc);
            goto done;
        }
    }

    retval = RTI_TRUE;

done:

    if (!retval)
    {
        brc = NETIO_AddressSeq_set_length(resvd_addr,cur_addr_len);
        /* called has failed anyway */
        IGNORE_RETVAL(brc);
    }

    return retval;
}

/*ci
* \brief Implementation of the NETIO release_address function
*
* \param[in] netio_intf NETIO interface to release addresses on
* \param[in] address    Address to release
*
* \return RTI_TRUE on success, RTI_FALSE on failure.
*
*/
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_DGRAM_Interface_release_address(NETIO_Interface_T *netio_intf,
                                     struct NETIO_Address *address)
{
    struct NETIO_DGRAM_Interface *self =
                                    (struct NETIO_DGRAM_Interface*)netio_intf;
    return NETIO_DGRAM_Interface_unreserve_address(self,address,RTI_TRUE);
}

/*ce \dref_NETIO_DGRAM_Interface_add_route
 */
RTI_BOOL
NETIO_DGRAM_Interface_add_route_to_seq(struct NETIO_AddressSeq *address_seq,
                                       struct NETIO_NetmaskSeq *netmask_seq,
                                       const struct NETIO_Address *address,
                                       const struct NETIO_Netmask *netmask)
{
    RTI_INT32 addr_len;
    RTI_INT32 mask_len;

    OSAPI_PRECONDITION_ALWAYS((address_seq == NULL) ||
                              (netmask_seq == NULL) ||
                              (address == NULL) ||
                              (netmask == NULL),
                              return RTI_FALSE,
          OSAPI_Log_entry_add_pointer("address_seq",address_seq,RTI_FALSE);
          OSAPI_Log_entry_add_pointer("netmask_seq",netmask_seq,RTI_FALSE);
          OSAPI_Log_entry_add_pointer("address",address_seq,RTI_FALSE);
          OSAPI_Log_entry_add_pointer("netmask",netmask_seq,RTI_TRUE);)

    addr_len = NETIO_AddressSeq_get_length(address_seq);
    if (addr_len >= NETIO_AddressSeq_get_maximum(address_seq))
    {
        UDP_LOG_GET_LENGTH(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    mask_len = NETIO_NetmaskSeq_get_length(netmask_seq);
    if (mask_len >= NETIO_NetmaskSeq_get_maximum(netmask_seq))
    {
        UDP_LOG_GET_LENGTH(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (addr_len != mask_len)
    {
        return RTI_FALSE;
    }

    if (!NETIO_AddressSeq_set_length(address_seq,addr_len+1))
    {
        UDP_LOG_SET_LENGTH(OSAPI_LOGKIND_ERROR,addr_len+1)
        return RTI_FALSE;
    }

    if (!NETIO_NetmaskSeq_set_length(netmask_seq,addr_len+1))
    {
        (void)NETIO_AddressSeq_set_length(address_seq,addr_len);
        UDP_LOG_SET_LENGTH(OSAPI_LOGKIND_ERROR,addr_len+1)
        return RTI_FALSE;
    }

    /* Since set_length succeeded for both sequences it is assumed that
     * get_reference returns a valid address.
     */
    /* coverity[dereference] */
    /* coverity[cert_exp34_c_violation] */
    *NETIO_AddressSeq_get_reference(address_seq,addr_len) = *address;

    /* coverity[dereference] */
    /* coverity[cert_exp34_c_violation] */
    *NETIO_NetmaskSeq_get_reference(netmask_seq,addr_len) = *netmask;

    return RTI_TRUE;
}

/*ce \dref_NETIO_DGRAM_Interface_add_route_entry
 */
RTI_BOOL
NETIO_DGRAM_Interface_add_route_entry_to_seq(
                        struct NETIO_AddressSeq *address_seq,
                        struct NETIO_NetmaskSeq *netmask_seq,
                        const struct NETIO_DGRAM_InterfaceRouteEntry *entry)
{
    OSAPI_PRECONDITION_ALWAYS((address_seq == NULL) ||
                              (netmask_seq == NULL) ||
                              (entry == NULL),
                              return RTI_FALSE,
          OSAPI_Log_entry_add_pointer("address_seq",address_seq,RTI_FALSE);
          OSAPI_Log_entry_add_pointer("netmask_seq",netmask_seq,RTI_FALSE);
          OSAPI_Log_entry_add_pointer("entry",entry,RTI_TRUE);)

    return NETIO_DGRAM_Interface_add_route_to_seq(address_seq,netmask_seq,
                                               &entry->address,&entry->netmask);
}

/*ci
 * \brief Implementation of the NETIO get_route_table function
 *
 * \details
 * Call the downstream interface to retrieve the addresses NETIO_DGRAM can
 * send to.
*
 * \param[in]    netio_intf The NETIO_DGRAM interface
 * \param[inout] address    Sequence of NETIO addresses this interface
 *                          understands
 * \param[inout] netmask    Sequence of the corresponding netmasks
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_DGRAM_Interface_get_route_table(NETIO_Interface_T *netio_intf,
                             struct NETIO_AddressSeq *address,
                             struct NETIO_NetmaskSeq *netmask)
{
    struct NETIO_DGRAM_Interface *self =
                                (struct NETIO_DGRAM_Interface*)netio_intf;

    return NETIO_Interface_get_route_table(self->user_netio,
                                           address,netmask);
}

/*ci
 * \brief Implementation of the NETIO receive function
 *
 * \details
 *
 *  A NETIO_DGRAM interface receives data from a downstream interface.
 *  The data can be in different protocol formats. Depending on the
 *  downstream protocol different actions may be taken, but semantically it
 *  does not matter where data is coming from. The receive interface is
 *  synchronous, _all_ processing takes place in the context of the downstream
 *  receive context, such as a thread.
 *
 * \param[in] netio_intf The interface receiving data
 * \param[in] src_addr   The source address of the data
 * \param[in] dst_addr   The destination address of the data
 * \param[in] packet     A NETIO_Packet with the payload
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE MUST_CHECK_RETURN RTI_BOOL
NETIO_DGRAM_Interface_receive(NETIO_Interface_T *netio_intf,
                             struct NETIO_Address *source,
                             struct NETIO_Address *dst_addr,
                             NETIO_Packet_T *packet)
{
    struct NETIO_DGRAM_Interface *self =
                                    (struct NETIO_DGRAM_Interface*)netio_intf;
    DB_Cursor_T cursor = NULL;
    struct NETIO_DGRAM_BindEntry *bind_entry;
    DB_ReturnCode_T dbrc;
    RTI_SIZE_T pkt_head, pkt_tail;
    RTI_BOOL retval = RTI_FALSE;
    RTI_BOOL bretval;
    RTI_INT32 rx_len;
    UNUSED_ARG(dst_addr);

    rx_len = (RTI_INT32)NETIO_Packet_get_payload_length(packet);

    OSAPI_TRACE_NET("wait for data:",RTI_FALSE)
    OSAPI_TRACE_INT32("port",source->port,RTI_FALSE)
    OSAPI_TRACE_INT32("address",source->value.ipv4.address,RTI_FALSE)
    OSAPI_TRACE_INT32("length",rx_len,RTI_TRUE)

    if (rx_len <= 0)
    {
#if OSAPI_ENABLE_LOG
        if (rx_len < 0)
        {
            UDP_LOG_RECV_ERROR(OSAPI_LOGKIND_ERROR,
                               OSAPI_Log_get_last_error_code())
        }
#endif
        return RTI_FALSE;
    }

    if (NETIO_Packet_is_ndds_ping(packet))
    {
        return RTI_TRUE;
    }

    if (DB_Database_lock(self->property._parent.db) != DB_RETCODE_OK)
    {
        return RTI_FALSE;
    }

    NETIO_Packet_save_positions_to(packet, &pkt_head, &pkt_tail);

    dbrc = DB_Table_select_all(self->_parent._btable,
                               DB_TABLE_DEFAULT_INDEX,&cursor);
    if (dbrc == DB_RETCODE_OK)
    {
        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&bind_entry);
        while (dbrc == DB_RETCODE_OK)
        {
            /* If the receive port is shared, do not compare the address.
             * NETIO_AddressValue is used, doesn't matter as all 16 bytes are
             * compared.
             */
            if ((NETIO_Address_get_kind(&bind_entry->_parent.source) ==
                NETIO_Address_get_kind(source)) &&
                (bind_entry->_parent.source.port == source->port) &&
                (bind_entry->is_shared ||
                OSAPI_Memory_compare(&bind_entry->_parent.source.value,
                                      &source->value,
                                      RTI_SIZEOF(union NETIO_AddressValue)) == 0))
            {
                OSAPI_TRACE_NET("received data:",RTI_FALSE)
                OSAPI_TRACE_INT32("src.port",
                    bind_entry->_parent.source.port,RTI_FALSE)
                OSAPI_TRACE_GUID("src.address",
                    &bind_entry->_parent.source.value.rtps_guid,RTI_FALSE)
                OSAPI_TRACE_INT32("dst.port",
                    bind_entry->_parent.destination.port,RTI_FALSE)
                OSAPI_TRACE_GUID("dst.address",
                    &bind_entry->_parent.destination.value.rtps_guid,RTI_TRUE)

                /* Assign the packet which address the packet was received on */
                packet->local_source = bind_entry->_parent.source;

                bretval = NETIO_Interface_receive(bind_entry->_parent.intf,
                                              &self->_parent.local_address,
                                              &bind_entry->_parent.destination,
                                              packet);
#if OSAPI_ENABLE_LOG
                if (!bretval)
                {
                    UDP_LOG_PACKET_FWD(OSAPI_LOGKIND_WARNING)
                }
#else
                IGNORE_RETVAL(bretval);
#endif

                NETIO_Packet_restore_positions_from(packet, pkt_head, pkt_tail);
                /* Force the while to terminate cleanly. All received DGRAM
                 * traffic is forwarded upstream to the same RTPS interface.
                 * This is by design, future versions may forward the same
                 * packet to multiple upstream interfaces.
                 */
                dbrc = DB_RETCODE_NO_DATA;
            }
            else
            {
                dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&bind_entry);
            }
        }
        DB_Cursor_finish(self->_parent._btable,cursor);

        if (dbrc != DB_RETCODE_NO_DATA)
        {
            UDP_LOG_CURSOR_ERROR(OSAPI_LOGKIND_ERROR)
            goto done;
        }

        retval = RTI_TRUE;
    }

done:

    if (DB_Database_unlock(self->property._parent.db) != DB_RETCODE_OK)
    {
        return RTI_FALSE;
    }

    return retval;
}

/*ci
 * \brief Implementation of the NETIO get_transport_properties. Return the
 *        minimum mtu across all available interfaces
 *
 * \param[in] netio_intf The interface
 * \param[out] properties The transport properties
 */
RTI_PRIVATE void
NETIO_DGRAM_Interface_get_transport_properties(
                                    struct NETIO_Interface *netio_intf,
                                    struct NETIO_TransportProperty *properties)
{
    struct NETIO_DGRAM_Interface *self =
                                    (struct NETIO_DGRAM_Interface*)netio_intf;
    RTI_INT32 if_len;
    RTI_INT32 if_index;
    RTI_UINT32 max_mtu = INT_MAX;
    struct NETIO_DGRAM_InterfaceTableEntry *if_entry;

    if_len = NETIO_DGRAM_InterfaceTableEntrySeq_get_length(&self->if_table);
    for (if_index = 0; if_index < if_len; if_index++)
    {
        if_entry = NETIO_DGRAM_InterfaceTableEntrySeq_get_reference(
                                                    &self->if_table,if_index);
        if (if_entry == NULL)
        {
            break;
        }

        if (if_entry->mtu < max_mtu)
        {
            max_mtu = if_entry->mtu;
        }
    }

    properties->recv_size_max = max_mtu;
    properties->send_size_max = max_mtu;
}

/*ci
 * \brief Implementation of the NETIO is_address_reachable. Return TRUE if
 *        any interface supports the address locator kind.
 *
 * \details
 * *
 * \param[in] netio_intf   The interface
 * \param[in] addr         The destination address to check
 * \param[in] is_reachable Set to TRUE on success if any interface supports the
 *                         locator, FALSE otherwise.
 *
 * \return RTI_TRUE if the call suceeded, RTI_FALSE if an error occured.
 */
RTI_PRIVATE RTI_BOOL
NETIO_DGRAM_Interface_is_address_reachable(
                        struct NETIO_Interface *netio_intf,
                        const struct NETIO_AddressEx *const addr,
                        RTI_BOOL *is_reachable)
{
    struct NETIO_DGRAM_Interface *self =
                                    (struct NETIO_DGRAM_Interface*)netio_intf;
    RTI_INT32 if_len;
    RTI_INT32 if_index;
    struct NETIO_DGRAM_InterfaceTableEntry *if_entry;

    if_len = NETIO_DGRAM_InterfaceTableEntrySeq_get_length(&self->if_table);
    for (if_index = 0; if_index < if_len; if_index++)
    {
        if_entry = NETIO_DGRAM_InterfaceTableEntrySeq_get_reference(
                                &self->if_table,if_index);
        if (if_entry == NULL)
        {
            return RTI_FALSE;
        }

        if (NETIO_Address_kind(if_entry->locator_kind)
            == NETIO_Address_kind((addr->kind)))
        {
            *is_reachable = RTI_TRUE;
            return RTI_TRUE;
        }
    }

    *is_reachable = RTI_FALSE;
    return RTI_TRUE;
}

/*ci \brief The NETIO_InterfaceI definition for the NETIO_DGRAM interface
 */
RTI_PRIVATE struct NETIO_InterfaceI NETIO_DGRAM_Interface_fv_Intf =
{
    RT_COMPONENTI_BASE,
    NETIO_DGRAM_Interface_send,
    NULL,
    NULL,
    NULL,
    NULL,
    NETIO_DGRAM_Interface_add_route,
    NETIO_DGRAM_Interface_delete_route,
    NETIO_DGRAM_Interface_reserve_address,
    NETIO_DGRAM_Interface_bind,
    NETIO_DGRAM_Interface_unbind,
    NETIO_DGRAM_Interface_receive,
    NULL,
    NETIO_DGRAM_Interface_bind_external,
    NETIO_DGRAM_Interface_unbind_external,
    NETIO_DGRAM_Interface_set_state,
    NETIO_DGRAM_Interface_release_address,
    NETIO_DGRAM_Interface_resolve_address,
    NETIO_DGRAM_Interface_get_route_table,
    NULL,
    NULL,
    NETIO_DGRAM_Interface_is_address_reachable,
    NETIO_DGRAM_Interface_get_transport_properties
};

/* ------------------------------------------------------------------------ */
/*                   Plugin factory                                         */
/* ------------------------------------------------------------------------ */

/*ci
 * \brief Implementation of the RT ComponentFactory create component method
 *
 * \param[in] factory   The factory creating the component
 * \param[in] property  The component property
 * \param[in] listener  The component listener
 *
 * \return A new component on success, NULL on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RT_Component_T*
NETIO_DGRAM_InterfaceFactory_create_component(
                                       struct RT_ComponentFactory *factory,
                                       struct RT_ComponentProperty *property,
                                       struct RT_ComponentListener *listener)
{
    struct NETIO_DGRAM_Interface *retval = NULL;

    retval = NETIO_DGRAM_Interface_create(
                        (struct NETIO_DGRAM_InterfaceFactory*)factory,
                        (const struct NETIO_InterfaceProperty *const)property,
                        (const struct NETIO_InterfaceListener *const)listener);

    if (retval == NULL)
    {
        return NULL;
    }

    return &retval->_parent._parent;
}

/*ci
 * \brief Implementation of the RT ComponentFactory delete method
 *
 * \param[in] factory   The factory that created the component
 * \param[in] component The component to be deleted
 *
 * \sa \ref NETIO_DGRAM_InterfaceFactory_create_component
 */
#ifndef RTI_CERT
RTI_PRIVATE void
NETIO_DGRAM_InterfaceFactory_delete_component(
                                        struct RT_ComponentFactory *factory,
                                        RT_Component_T *component)
{
    struct NETIO_DGRAM_Interface *self =
                            (struct NETIO_DGRAM_Interface *)component;
    struct NETIO_DGRAM_InterfaceFactory *f =
                            (struct NETIO_DGRAM_InterfaceFactory*)factory;

    if (f == NULL)
    {
        return;
    }

    NETIO_DGRAM_Interface_delete(self);
}

RTI_PRIVATE void
NETIO_DGRAM_InterfaceFactory_finalize(struct RT_ComponentFactory *factory,
                              struct RT_ComponentFactoryProperty **property,
                              struct RT_ComponentFactoryListener **listener);

#endif

MUST_CHECK_RETURN RTI_PRIVATE struct RT_ComponentFactory*
NETIO_DGRAM_InterfaceFactory_initialize(
                                struct RT_ComponentFactoryProperty*property,
                                struct RT_ComponentFactoryListener *listener);

/*ci
 * \brief Implementation of the RT ComponentFactoryI interface
 */
RTI_PRIVATE struct RT_ComponentFactoryI NETIO_DGRAM_InterfaceFactory_fv_Intf =
{
    0,
    NETIO_DGRAM_InterfaceFactory_initialize,
#ifndef RTI_CERT
    NETIO_DGRAM_InterfaceFactory_finalize,
#else
    NULL,
#endif
    NETIO_DGRAM_InterfaceFactory_create_component,
#ifndef RTI_CERT
    NETIO_DGRAM_InterfaceFactory_delete_component,
#else
    NULL,
#endif
    NULL,
    NULL
};

/*ci
 * \brief Singleton for the NETIO_DGRAM_InterfaceFactory
 *
 * \details
 * The NETIO_DGRAM interface class is implemented as a singleton, there are
 * no shared resources between interfaces.
 */
RTI_PRIVATE const struct NETIO_DGRAM_InterfaceFactory
NETIO_DGRAM_InterfaceFactory_fv_Factory =
{
    {
       &NETIO_DGRAM_InterfaceFactory_fv_Intf,
       NULL,
       {{{0,0}}}
    },
    0,
    {0},
    NULL
};

/*ci
 * \brief NETIO_DGRAM specific implementation of the RT ComponentFactory initialize
 *        method
 *
 * \param[in] property The properties registered with the NETIO_DGRAM interface
 * \param[in] listener The listener registered with the NETIO_DGRAM interface
 *
 * \return A fully initialized factory on success, NULL on failure
 *
 * \sa \ref NETIO_DGRAM_InterfaceFactory_finalize
 */
MUST_CHECK_RETURN RTI_PRIVATE struct RT_ComponentFactory*
NETIO_DGRAM_InterfaceFactory_initialize(
                                  struct RT_ComponentFactoryProperty *property,
                                  struct RT_ComponentFactoryListener *listener)
{
    struct NETIO_DGRAM_InterfaceFactory *factory = NULL;
    struct NETIO_DGRAM_InterfaceFactoryProperty *fproperty =
                        (struct NETIO_DGRAM_InterfaceFactoryProperty*)property;
    UNUSED_ARG(listener);

    if (property == NULL)
    {
        return NULL;
    }

    OSAPI_Heap_allocate_struct(&factory,struct NETIO_DGRAM_InterfaceFactory);
    if (factory == NULL)
    {
        return NULL;
    }

    *factory = NETIO_DGRAM_InterfaceFactory_fv_Factory;
    factory->user_intf = fproperty->user_intf;
    factory->user_property = fproperty->user_property;

    return &factory->_parent;
}

#ifndef RTI_CERT
/*ci
 * \brief Implementation of the RT ComponentFactory finalize method
 *
 * \param[in]  factory  The factory to finalize
 * \param[out] property The property the factory was registered with
 * \param[out] listener The listener the factory was registered with
 *
 * \sa \ref NETIO_DGRAM_InterfaceFactory_initialize
 */
RTI_PRIVATE void
NETIO_DGRAM_InterfaceFactory_finalize(
                            struct RT_ComponentFactory *factory,
                            struct RT_ComponentFactoryProperty **property,
                            struct RT_ComponentFactoryListener **listener)
{
    struct NETIO_DGRAM_InterfaceFactory *f =
                            (struct NETIO_DGRAM_InterfaceFactory*)factory;
    UNUSED_ARG(property);
    UNUSED_ARG(listener);

    OSAPI_Heap_free_struct(f);
}
#endif

/*e
 * \dref_NETIO_DGRAM_InterfaceFactory_register
 */
RTI_BOOL
NETIO_DGRAM_InterfaceFactory_register(
                RT_Registry_T *registry,
                const char *name,
                NETIO_DGRAM_InterfaceI *user_intf,
                void *user_property)
{
    struct NETIO_DGRAM_InterfaceFactoryProperty property;
    UNUSED_ARG(registry);

    OSAPI_PRECONDITION_ALWAYS((registry == NULL) ||
                              (name == NULL) ||
                              (user_intf == NULL),
                              return RTI_FALSE,
          OSAPI_Log_entry_add_pointer("registry",registry,RTI_FALSE);
          OSAPI_Log_entry_add_pointer("name",name,RTI_FALSE);
          OSAPI_Log_entry_add_pointer("user_intf",user_intf,RTI_TRUE);)

    property._parent.dummy = NULL;
    property.user_intf = *user_intf;
    property.user_property = user_property;

    if (property.user_intf.resolve_address == NULL)
    {
        property.user_intf.resolve_address = NETIO_DGRAM_Interface_resolve_ipv4_address;
    }

    if ((user_intf->create_instance == NULL) ||
#ifndef RTI_CERT
        (user_intf->delete_instance == NULL)  ||
#endif
        (user_intf->get_interface_list == NULL) ||
        (user_intf->release_address == NULL)  ||
        (user_intf->send == NULL)  ||
        (user_intf->get_route_table == NULL)  ||
        (user_intf->bind_address == NULL))
    {
        return RTI_FALSE;
    }

    return RT_Registry_register(registry, name,
                                &NETIO_DGRAM_InterfaceFactory_fv_Intf,
                                (struct RT_ComponentFactoryProperty*)&property,
                                NULL);
}

/*e
* \dref_NETIO_DGRAM_InterfaceFactory_unregister
*/
RTI_BOOL
NETIO_DGRAM_InterfaceFactory_unregister(
                RT_Registry_T *registry,
                const char *name)
{
    RT_ComponentFactory_T *c_factory = NULL;

    OSAPI_PRECONDITION_ALWAYS((registry == NULL) ||
                              (name == NULL),
                              return RTI_FALSE,
          OSAPI_Log_entry_add_pointer("registry",registry,RTI_FALSE);
          OSAPI_Log_entry_add_pointer("name",name,RTI_TRUE);)

    c_factory = RT_Registry_lookup(registry, name);
    if (c_factory == NULL)
    {
        return RTI_FALSE;
    }

    if (c_factory->intf != &NETIO_DGRAM_InterfaceFactory_fv_Intf)
    {
        return RTI_FALSE;
    }

    return RT_Registry_unregister(registry, name,NULL,NULL);
}

/*e
* \dref_NETIO_DGRAM_Interface_upstream_receive
*/
RTI_BOOL
NETIO_DGRAM_Interface_upstream_receive(NETIO_Interface_T *netio_intf,
                                       struct NETIO_Address *source,
                                       NETIO_Packet_T *packet)
{
    OSAPI_PRECONDITION_ALWAYS((netio_intf == NULL) ||
                              (source == NULL) ||
                              (packet == NULL),
                              return RTI_FALSE,
        OSAPI_Log_entry_add_pointer("netio_intf",netio_intf,RTI_FALSE);
        OSAPI_Log_entry_add_pointer("source",source,RTI_FALSE);
        OSAPI_Log_entry_add_pointer("packet",packet,RTI_TRUE);)

    return NETIO_DGRAM_Interface_receive(netio_intf,source,NULL,packet);
}
