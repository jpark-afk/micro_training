/*
 * FILE: NETIOAddress.c - NETIO Address implementation
 *
 * Copyright 2012-2021 Real-Time Innovations, Inc.
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
 * 09aug2022,tk MICRO-4083/PR.30125
 * - Removed redundant test on (prefix_slash == 2) in NETIO_Address_get_token
     for NETIO_ADDRESSTOKEN_PREFIX
 * - Removed redundant test on && prefix_slash in NETIO_Address_get_token
     for NETIO_ADDRESSTOKEN_PREFIX
 * - Removed redundant test on && range_hyphen in NETIO_Address_parse
     for NETIO_ADDRESSSTATE_HIGH_RANGE
 * 27may2022,am MICRO-3537/PR.30409
 * - Removed declaration of NETIO_Address_is_udpv4_multicast as it is 
 *   no longer used. 
 * 11mar2021, am MICRO-3515
 * - Remove NETIO_Address_set_ipv4 when builtin udp is excluded. 
 * 16apr2021,tk MICRO-2845/PR.28682
 *  - Removed empty blocks in NETIO_AddressResolver_resolve_address
 * 15apr2021,tk MICRO-2979/PR.27549
 * - Removed unused functions from CERT:
 *   NETIO_Address_set_guid_from_int32
 *   NETIO_Address_set_multicast
 *   NETIO_Address_set_internal
 *   NETIO_Address_kind
 * 31jul2015,tk MICRO-1478/PR#15694 Allow " " as the first characater
 * 31jul2015,tk MICRO-1479/PR#15696 Check for overflow in current_index. Added
 *                                  robustness check on default_low_index and
 *                                  default_high_index.
 * 15jul2015,tk MICRO-1472/PR#15669 RT_MAX_FACTORY_NAME is the maximum prefix
 *                                  not NETIO_PREFIX_TOKEN_MAX_SIZE. Return
 *                                  error on valid prefix format but invalid
 *                                  prefix name.
 * 15jul2015,tk MICRO-1435/PR#15383 Added robustness checks for buffer
 *                                  overflow
 * 15jul2015,tk MICRO-1424/PR#15348 Only allow positive integers (check for
 *                                  overflow on "long" integers)
 * 14jul2015,tk MICRO-1427/PR#15373 Only ignore whitespace characters
 *                                  when no valid token is found.
 * 13jul2015,tk MICRO-1419/PR#15323 Fixed parsing of [x y] as invalid
 * 08jun2015,tk MICRO-1115/PR#14222 Updated to be consistent with HLR
 * 19may2015,as MICRO-1193          Refactoring of Sequence API levels
 * 12mar2015,tk MICRO-1116/PR#14232 Relax rules for valid address
 * 02feb2015,tk MICRO-1045/PR#13568 Return cursor in case it is invalidated
 * 10dec2014,tk MICRO-978/PR#12914  Consistently return records on failure
 * 16sep2014,tk MICRO-913/PR#11266  Added constraint low_index <= high_index
 * 15sep2014,tk MICRO-910/PR#11259  Updated comments for NETIO_Address_parse
 * 31jul2014,tk MICRO-241/PR#1413   Removed superfluous DB fields
 * 31jul2014,tk MICRO-842/PR#9683   Removed superfluous parameters in DB
 *                                  compare function
 * 05may2014,tk MICRO-72            Updated based on CR-232
 * 13feb2014,tk MICRO-719           Support full peer syntax in address parser
 * 04jun2012,tk Written
 */
/*ci
 * \file
 * \brief NETIO Address implementation
 *
 * \details
 * This function implements functions to manage, parse and resolve NETIO
 * addresses. All NETIO interfaces uses NETIO addresses to communicate and
 * a NETIO address is a generic structure that can be interpreted in many
 * different ways. The layout of a NETIO address is compatible with a DDS
 * locator by design. This is to simplify conversion between the two, a
 * simple cast is sufficient. It is important to maintain this type
 * equivalence. Also note that the NETIO module is generic and sits
 * below DDS and cannot make any references to DDS types.
 *
 * \addtogroup NETIOAddressClass
 * @{
 */
#include "osapi/osapi_config.h"

#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#include "osapi/osapi_string.h"
#ifndef db_api_h
#include "db/db_api.h"
#endif
#ifndef rt_rt_h
#include "rt/rt_rt.h"
#endif
#include "netio/netio_config.h"
#include "netio/netio_common.h"
#include "reda/reda_sequence.h"
#ifndef netio_log_h
#include "netio/netio_log.h"
#endif
#ifndef netio_address_h
#include "netio/netio_address.h"
#endif
#ifndef netio_route_h
#include "netio/netio_route.h"
#endif

/*ci
 * \brief Parsing states when breaking down in address in individual parts
 */
typedef enum
{
    /*ci
     * \brief Initialization
     */
    NETIO_ADDRESSSTATE_INIT,

    /*ci
     * \brief Parse the prefix
     */
    NETIO_ADDRESSSTATE_PREFIX,

    /*ci
     * \brief Parse the low-range of an index
     */
    NETIO_ADDRESSSTATE_LOW_RANGE,

    /*ci
     * \brief Parse the high-range of an index
     */
    NETIO_ADDRESSSTATE_HIGH_RANGE,

    /*ci
     * \brief Parse the end an index range
     */
    NETIO_ADDRESSSTATE_END_RANGE,

    /*ci
     * \brief Parse the address part
     */
    NETIO_ADDRESSSTATE_ADDRESS,

    /*ci
     * \brief Parse the @ sign
     */
    NETIO_ADDRESSSTATE_AT
} NETIO_AddressState_T;

/*ci
 * \brief The minimum number of tokens needed to parse an address
 */
typedef enum
{
    /*ci
     * \brief No token
     */
    NETIO_ADDRESSTOKEN_NONE,

    /*ci
     * \brief The end-of-string token
     */
    NETIO_ADDRESSTOKEN_EOS,

    /*ci
     * \brief The @ sign
     */
    NETIO_ADDRESSTOKEN_AT,

    /*ci
     * \brief A range is defined as [\\d+[-\\d+]]
     */
    NETIO_ADDRESSTOKEN_RANGE,

    /*ci
     * \brief An integer
     */
    NETIO_ADDRESSTOKEN_INTEGER,

    /*ci
     * \brief The address part, typically the remainder of the string after
     *        all other tokens have been parsed
     */
    NETIO_ADDRESSTOKEN_ADDRESS,

    /*ci
     * \brief The prefix token is defined as [a-zA-Z_0-9]://
     */
    NETIO_ADDRESSTOKEN_PREFIX,

    /*ci
     * \brief [
     */
    NETIO_ADDRESSTOKEN_LEFT_BRACKET,

    /*ci
     * \brief ]
     */
    NETIO_ADDRESSTOKEN_RIGHT_BRACKET,

    /*ci
     * \brief -
     */
    NETIO_ADDRESSTOKEN_HYPHEN
} NETIO_AddressTokenKind_T;

/*ci
 * \brief Structure to hold range information
 */
typedef struct
{
    /*ci
     * \brief the low range index
     */
    RTI_INT32 low;

    /*ci
     * \brief the high range index. If high is not specified, high = low
     */
    RTI_INT32 high;
} NETO_RangeToken_T;

/*ci
 * \brief All possible tokens
 */
typedef union
{
    /*ci
     * \brief The token as an address
     */
    char address_token[NETIO_ADDRESS_TOKEN_MAX_SIZE];

    /*ci
     * \brief The token as a prefix
     */
    char prefix_token[NETIO_PREFIX_TOKEN_MAX_SIZE];

    /*ci
     * \brief The token as an integer
     */
    RTI_INT32 integer_token;

    /*ci
     * \brief The token as a range
     */
    NETO_RangeToken_T range_token;
} NETIO_AddressTokenValue_T;

/*
 * \brief A token
 */
typedef struct
{
    /*ci
     * \brief The token discriminator
     */
    NETIO_AddressTokenKind_T kind;

    /*ci
     * \brief The token value
     */
    NETIO_AddressTokenValue_T value;
} NETO_AddressToken_T;

/*ci
 * \brief Structure holding information about an address resolver for port
 *        calculations
 */
struct NETIO_AddressResolveEntry
{
    /*ci
     * \brief The id of the component factory doing the port calculation
     */
    RT_ComponentFactoryId_T id;

    /*ci
     * \brief The interface instance doing the port calculation
     */
    NETIO_Interface_T *intf;

    /*ci
     * \brief An optional function to call to resolve ports, NULL if not present
     */
    NETIO_PortCalculateFunc_T port_resolve;

    /*ci
     * \brief Optional parameter to pass to the port_resolve function
     */
    void *port_resolve_param;
};

/*** SOURCE_BEGIN ***/

#define T struct NETIO_Address
#define TSeq NETIO_AddressSeq
#include "reda/reda_sequence_defn.h"

#define T struct NETIO_Netmask
#define TSeq NETIO_NetmaskSeq
#include "reda/reda_sequence_defn.h"

void
NETIO_Address_init(struct NETIO_Address *addr,RTI_INT32 kind)
{
    addr->kind = kind; addr->port = 0;
    addr->value.init.val0 = addr->value.init.val1 = 0;
    addr->value.init.val2 = addr->value.init.val3 = 0;
}

#if !UDP_EXCLUDE_BUILTIN  
void
NETIO_Address_set_ipv4(struct NETIO_Address *addr,RTI_UINT32 port,RTI_UINT32 address)
{
    addr->kind = NETIO_ADDRESS_KIND_UDPv4;
    addr->port = port;
    addr->value.ipv4.address = address;
}
#endif
void
NETIO_Address_set_guid(struct NETIO_Address *addr,RTI_UINT32 port,struct NETIO_Guid *guid)
{
    addr->kind = NETIO_ADDRESS_KIND_INTRA;
    addr->port = port;
    OSAPI_Memory_copy(&addr->value.guid,guid,16);
}

void
NETIO_Address_set_guid_from_key(struct NETIO_Address *addr,RTI_UINT32 port,
                                struct NETIO_AddressInt32 *key)
{
#if RTI_ENDIAN_BIG
    addr->kind = NETIO_ADDRESS_KIND_INTRA;
    addr->port = port;
    OSAPI_Memory_copy(&addr->value.guid,key,16);
#else
    addr->kind = NETIO_ADDRESS_KIND_INTRA;
    addr->port = port;
    addr->value.rtps_guid.host_id = NETIO_htonl(key->value[0]);
    addr->value.rtps_guid.app_id = NETIO_htonl(key->value[1]);
    addr->value.rtps_guid.instance_id = NETIO_htonl(key->value[2]);
    addr->value.rtps_guid.object_id = NETIO_htonl(key->value[3]);
#endif
}

#ifndef RTI_CERT
void
NETIO_Address_set_guid_from_int32(struct NETIO_Address *addr,RTI_UINT32 port,
                                  RTI_UINT32 int0,RTI_UINT32 int1,
                                  RTI_UINT32 int2,RTI_UINT32 int3)
{

#if RTI_ENDIAN_BIG
    addr->kind = NETIO_ADDRESS_KIND_INTRA;
    addr->port = port;
    addr->value.rtps_guid.host_id = int0;
    addr->value.rtps_guid.app_id = int1;
    addr->value.rtps_guid.instance_id = int2;
    addr->value.rtps_guid.object_id = int3;
#else
    addr->kind = NETIO_ADDRESS_KIND_INTRA;
    addr->port = port;
    addr->value.rtps_guid.host_id = NETIO_htonl(int0);
    addr->value.rtps_guid.app_id = NETIO_htonl(int1);
    addr->value.rtps_guid.instance_id = NETIO_htonl(int2);
    addr->value.rtps_guid.object_id = NETIO_htonl(int3);
#endif
}
#endif

void
NETIO_Address_set_guid_from_array(struct NETIO_Address *addr,
                                  RTI_UINT32 port,
                                  RTI_UINT8 *octet_array)
{
    addr->kind = NETIO_ADDRESS_KIND_INTRA;
    addr->port = port;
    OSAPI_Memory_copy(&addr->value.guid,octet_array,16);
}


RTI_BOOL
NETIO_Address_is_multicast(const struct NETIO_Address *const addr)
{
    return (addr->kind & (RTI_INT32)NETIO_ADDRESS_FLAG_MULTICAST ? RTI_TRUE : RTI_FALSE);
}

RTI_BOOL
NETIO_Address_is_internal(const struct NETIO_Address *const addr)
{
    return (addr->kind & (RTI_INT32)NETIO_ADDRESS_FLAG_INTERNAL ? RTI_TRUE : RTI_FALSE);
}

#ifndef RTI_CERT
void
NETIO_Address_set_multicast(struct NETIO_Address *const addr)
{
    addr->kind |=  (RTI_INT32)NETIO_ADDRESS_FLAG_MULTICAST;
}
#endif

#ifndef RTI_CERT
void
NETIO_Address_set_internal(struct NETIO_Address *const addr)
{
    addr->kind |=  (RTI_INT32)NETIO_ADDRESS_FLAG_INTERNAL;
}
#endif

void
NETIO_Address_set_kind(struct NETIO_Address *const addr,RTI_UINT32 kind,RTI_UINT32 flags)
{
    addr->kind = (RTI_INT32)(kind | flags);
}

RTI_INT32
NETIO_Address_get_kind(const struct NETIO_Address *const addr)
{
    return addr->kind & 0x01ffffff;
}

#ifndef RTI_CERT
RTI_INT32
NETIO_Address_kind(RTI_INT32 kind)
{
    return kind & 0x01ffffff;
}
#endif

RTI_INT32
NETIO_Address_compare(const struct NETIO_Address *laddr,
                      const struct NETIO_Address *raddr)
{
    RTI_INT32 diff;
    RTI_INT32 i;

    diff = NETIO_Address_get_kind(laddr) - NETIO_Address_get_kind(raddr);
    if (diff != 0)
    {
        return diff;
    }

    /* NOTE: The remaining comparisons are for unsigned values, hence
     * subtractions are not used.
     */
    if (laddr->port > raddr->port)
    {
        return 1;
    }

    if (laddr->port < raddr->port)
    {
        return -1;
    }

    for (i = 0; i < NETIO_ADDRESS_MAX_32BIT; ++i)
    {
        if (laddr->value.as_uint32.value[i] > raddr->value.as_uint32.value[i])
        {
            return 1;
        }

        if (laddr->value.as_uint32.value[i] < raddr->value.as_uint32.value[i])
        {
            return -1;
        }
    }

    return 0;
}

/*ci
 * \brief Function to compare address resolvers, used to add address
 *        resolvers to a table. The arguments are as defined for
 *        \ref DB_IndexCompare_T and the component id is used as the key
 *
 * \param[in] flags Used to distinguish between op2 being a key or record
 * \param[in] op1   A NETIO_AddressResolveEntry already in the database
 * \param[in] op2   Either a NETIO_AddressResolveEntry being added or a key
 *                  being searched for
 *
 * \return positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2
 *         zero if op1 is equal to op2
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_INT32
NETIO_AddressResolveEntry_compare(RTI_INT32 flags,const DB_Record_T op1, void *op2)
{
    struct NETIO_AddressResolveEntry *record_left =
                                    (struct NETIO_AddressResolveEntry*)op1;
    const RT_ComponentFactoryId_T *id_right;

    if (DB_SELECT_OP2_IS_KEY(flags))
    {
        id_right = (const RT_ComponentFactoryId_T *)op2;
    }
    else
    {
        id_right = &((struct NETIO_AddressResolveEntry*)op2)->id;
    }

    return RT_ComponentFactoryId_compare(&record_left->id,id_right);
}

NETIO_AddressResolver_T*
NETIO_AddressResolver_new(const char *name,
                          DB_Database_T db,
                          struct NETIO_AddressResolverProperty *property)
{
    struct DB_TableProperty tbl_property = DB_TableProperty_INITIALIZER;
    struct NETIO_AddressResolver *nar = NULL;
    struct NETIO_AddressResolver *retval = NULL;

    DB_ReturnCode_T dbrc;

    OSAPI_Heap_allocate_struct(&nar,struct NETIO_AddressResolver);
    if (nar == NULL)
    {
        NETIO_LOG_AR_ALLOC_FAILED(OSAPI_LOGKIND_ERROR)
        return NULL;
    }

    nar->property = *property;

    if ((property->default_low_index < 0) || (property->default_high_index < 0)
        || (property->default_low_index > property->default_high_index))
    {
        return NULL;
    }

    nar->db = db;
    tbl_property.max_cursors = 1;
    tbl_property.max_indices = 1;
    tbl_property.max_records = property->max_interfaces;

    dbrc = DB_Database_create_table(&nar->resolver_table,
            db,name,sizeof(struct NETIO_AddressResolveEntry),
            NETIO_AddressResolveEntry_compare,&tbl_property);

    if (dbrc != DB_RETCODE_OK)
    {
        goto done;
    }

    retval = nar;

done:
    return retval;
}

#ifndef RTI_CERT
RTI_BOOL
NETIO_AddressResolver_delete(NETIO_AddressResolver_T *nar)
{
    DB_ReturnCode_T dbrc;

    dbrc = DB_Database_delete_table(nar->db,nar->resolver_table);
    if (dbrc != DB_RETCODE_OK)
    {
        return RTI_FALSE;
    }

    OSAPI_Heap_free_struct(nar);
    return RTI_TRUE;
}
#endif /* !RTI_CERT */

RTI_BOOL
NETIO_AddressResolver_add_interface(NETIO_AddressResolver_T *nar,
                                    const char *name,
                                    NETIO_PortCalculateFunc_T port_resolve,
                                    void *port_resolve_param,
                                    NETIO_Interface_T *intf)
{
    DB_ReturnCode_T dbrc;
    RT_ComponentFactoryId_T id;
    struct NETIO_AddressResolveEntry *r_entry = NULL;
    RTI_BOOL rval = RTI_FALSE;

    if (!RT_ComponentFactoryId_set_name(&id,name))
    {
        return RTI_FALSE;
    }

    dbrc = DB_Table_select_match(nar->resolver_table,
            DB_TABLE_DEFAULT_INDEX,(DB_Record_T*)&r_entry,(DB_Key_T)&id);
    if (dbrc == DB_RETCODE_OK)
    {
        if ((r_entry->intf == intf) &&
            (r_entry->port_resolve == port_resolve) &&
            (r_entry->port_resolve_param == port_resolve_param))
        {
            rval = RTI_TRUE;
        }
#if OSAPI_ENABLE_LOG
        else
        {
            NETIO_LOG_AR_ADD_INVALID_INTERFACE(OSAPI_LOGKIND_ERROR)
        }
#endif
        goto done;
    }

    dbrc = DB_Table_create_record(nar->resolver_table,(DB_Record_T*)&r_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        goto done;
    }

    r_entry->id = id;
    r_entry->intf = intf;
    r_entry->port_resolve = port_resolve;
    r_entry->port_resolve_param = port_resolve_param;

    dbrc = DB_Table_insert_record(nar->resolver_table,(DB_Record_T)r_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        (void)DB_Table_delete_record(nar->resolver_table,(DB_Record_T)r_entry);
        goto done;
    }

    rval = RTI_TRUE;

done:
    return rval;
}

RTI_BOOL
NETIO_AddressResolver_lookup_interface(NETIO_AddressResolver_T *nar,
                                    const char *name,
                                    NETIO_Interface_T **intf)
{
    DB_ReturnCode_T dbrc;
    RT_ComponentFactoryId_T id;
    struct NETIO_AddressResolveEntry *r_entry = NULL;
    RTI_BOOL rval = RTI_FALSE;

    if (!RT_ComponentFactoryId_set_name(&id,name))
    {
        return RTI_FALSE;
    }

    *intf = NULL;

    dbrc = DB_Table_select_match(nar->resolver_table,
            DB_TABLE_DEFAULT_INDEX,(DB_Record_T*)&r_entry,(DB_Key_T)&id);

    if (dbrc == DB_RETCODE_OK)
    {
        *intf = r_entry->intf;
    }
    else if (dbrc != DB_RETCODE_NO_DATA)
    {
        goto done;
    }

    rval = RTI_TRUE;

done:
    return rval;
}

#ifndef RTI_CERT
RTI_BOOL
NETIO_AddressResolver_delete_interface(NETIO_AddressResolver_T *nar,
                                       const char *name,
                                       RTI_BOOL *existed,
                                       NETIO_Interface_T **intf)
{
    DB_ReturnCode_T dbrc;
    RT_ComponentFactoryId_T id;
    struct NETIO_AddressResolveEntry *r_entry = NULL;
    RTI_BOOL rval = RTI_FALSE;

    if (existed)
    {
        *existed = RTI_FALSE;
    }

    if (!RT_ComponentFactoryId_set_name(&id,name))
    {
        return RTI_FALSE;
    }

    dbrc = DB_Table_remove_record(nar->resolver_table,
                                 (DB_Record_T*)&r_entry,(DB_Key_T)&id);
    if (dbrc == DB_RETCODE_NO_DATA)
    {
        rval = RTI_TRUE;
        goto done;
    }

    if (dbrc != DB_RETCODE_OK)
    {
        goto done;
    }

    if (existed)
    {
        *existed = RTI_TRUE;
    }

    if (intf)
    {
        *intf = r_entry->intf;
    }

    dbrc = DB_Table_delete_record(nar->resolver_table,(DB_Record_T)r_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        goto done;
    }

    rval = RTI_TRUE;

done:
    return rval;
}
#endif /* !RTI_CERT */

/*ci
 * \brief Function to resolve an address, both port and address section
 *
 * \details
 * This function calls a NETIO interface to resolve an address's port
 * and address using the interface' registered address resolver and
 * port resolver (optionally installed port calculation function).
 *
 * \param[in]  r_entry        The address resolver
 * \param[in]  base_port      The base-port as passed in by the caller
 * \param[in]  index          The index to use as part of the port resolution
 * \param[in]  kind           The type of address
 * \param[in]  address_string The address part of a NETIO_Address string
 * \param[out] address        The resolved NETIO_Address
 * \param[out] is_invalid     Whether this was a valid address or not
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_AddressResolver_resolve_address(struct NETIO_AddressResolveEntry *r_entry,
                                      RTI_UINT32 base_port,
                                      RTI_INT32 index,
                                      NETIO_RouteKind_T kind,
                                      const char *address_string,
                                      struct NETIO_Address *address,
                                      RTI_BOOL *is_invalid)
{
    /* If the interface does not understand the address part, don't try to
     * resolve the port
     */
    if (!NETIO_Interface_resolve_address(r_entry->intf,
                                         address_string,address,is_invalid))
    {
#if OSAPI_ENABLE_LOG
        if (*is_invalid)
        {
            NETIO_LOG_AR_ADDRESS_RESOLVE_FAILED(OSAPI_LOGKIND_WARNING)
        }
#endif
        return RTI_FALSE;
    }

    /* If the optional port resolve function has been installed call it,
     * otherwise use the default port base
     */
    if (r_entry->port_resolve)
    {
        if (!r_entry->port_resolve(r_entry->port_resolve_param,kind,
                                   base_port,index,address))
        {
            NETIO_LOG_AR_PORT_RESOLVE_FAILED(OSAPI_LOGKIND_ERROR)
            return RTI_FALSE;
        }
    }
    else
    {
        address->port = base_port;
    }

    return RTI_TRUE;
}

void
NETIO_AddressResolveContext_init(NETIO_AddressResolveContext_T *c)
{
    c->is_initialized = RTI_FALSE;
}

RTI_BOOL
NETIO_AddressResolver_resolve(NETIO_AddressResolver_T *nar,
                              RTI_INT32 try_index,
                              NETIO_RouteKind_T kind,
                              const char *address_string,
                              NETIO_AddressResolveContext_T *context)
{
    DB_ReturnCode_T dbrc;
    RTI_BOOL rval = RTI_FALSE;

    if (context->is_initialized)
    {
        NETIO_LOG_AR_CONTEXT_UNINITIALIZED(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (!NETIO_Address_parse(address_string,&context->base_port,
                             &context->low_index,&context->high_index,
                             &context->intfid,context->out_address_string,
                             NETIO_ADDRESS_TOKEN_MAX_SIZE))
    {
        return RTI_FALSE;
    }

    if (try_index == -1)
    {
        if (context->low_index == -1)
        {
            context->low_index = nar->property.default_low_index;
        }

        if (context->high_index == -1)
        {
            context->high_index = nar->property.default_high_index;
        }
    }
    else
    {
        context->low_index = try_index;
        context->high_index = try_index;
    }

    if (context->base_port == 0)
    {
        context->base_port =  nar->property.default_base_port;
    }

    context->cursor = NULL;

    /* If the parsing was successful, we need to turn it into an actual
     * NETIO_Address. If this prefix is empty we need to query all the
     * interfaces available.
     */
    if (RT_ComponentFactoryId_equals(&context->intfid,""))
    {
        /* Not prefix specified, iterate over all interfaces */
        dbrc = DB_Table_select_all(nar->resolver_table,
                                    DB_TABLE_DEFAULT_INDEX,
                                    &context->cursor);
        if (dbrc != DB_RETCODE_OK)
        {
            goto done;
        }
        dbrc = DB_Cursor_get_next(context->cursor,
                                  (DB_Record_T*)&context->r_entry);
        if (dbrc != DB_RETCODE_OK)
        {
            DB_Cursor_finish(nar->resolver_table,context->cursor);
            goto done;
        }
    }
    else
    {
        dbrc = DB_Table_select_match(nar->resolver_table,
                           DB_TABLE_DEFAULT_INDEX,
                           (DB_Record_T*)&context->r_entry,
                           (DB_Key_T)&context->intfid);

        if (dbrc != DB_RETCODE_OK)
        {
            goto done;
        }
    }

    context->is_initialized = RTI_TRUE;
    context->current_index = context->low_index;
    context->kind = kind;

    rval = RTI_TRUE;

done:

    return rval;
}

RTI_BOOL
NETIO_AddressResolver_get_next(NETIO_AddressResolver_T *nar,
                               NETIO_Interface_T **netio_intf,
                               RT_ComponentFactoryId_T *id,
                               struct NETIO_Address *address,
                               NETIO_AddressResolveContext_T *context,
                               RTI_BOOL *error)
{
    DB_ReturnCode_T dbrc;
    RTI_BOOL found_next = RTI_FALSE;
    RTI_BOOL is_invalid;

    *error = RTI_FALSE;

    if (!context->is_initialized)
    {
        NETIO_LOG_AR_CONTEXT_UNINITIALIZED(OSAPI_LOGKIND_ERROR)
        *error = RTI_TRUE;
        return RTI_FALSE;
    }

    while (!found_next)
    {
        /* At the end of the current expansion */

        if ((context->current_index > context->high_index) ||
            (context->current_index < 0))
        {
            /* Was this an address without prefix, go to the next one */
            if (context->cursor != NULL)
            {
                dbrc = DB_Cursor_get_next(context->cursor,
                                          (DB_Record_T*)&context->r_entry);
                if ((dbrc == DB_RETCODE_NO_DATA) ||
                    (dbrc == DB_RETCODE_INVALIDATED_CURSOR))
                {
                    /* This was the last prefix or an error occurred,
                     * expansion is complete
                     */
                    DB_Cursor_finish(nar->resolver_table,context->cursor);
                    if (dbrc == DB_RETCODE_INVALIDATED_CURSOR)
                    {
                        *error = RTI_TRUE;
                    }
                    return RTI_FALSE;
                }

                /* Start at the first index again */
                context->current_index = context->low_index;
            }
            else
            {
                /* Last index has been returned */
                return RTI_FALSE;
            }
        }

        /* Resolve the address:
         * NOTE: If this call fails it is not necessarily an error because
         * the interface may or may not actually understand the address, e.g.
         * if no prefix was given.
         */
        *netio_intf = context->r_entry->intf;
        *id = context->r_entry->id;
        if (!NETIO_AddressResolver_resolve_address(context->r_entry,
                               context->base_port,context->current_index,
                               context->kind,
                               context->out_address_string,address,&is_invalid))
        {
            /* If the address was not understood, then is_invalid is false */
            if (is_invalid)
            {
                *error = RTI_TRUE;
                if (context->cursor)
                {
                    DB_Cursor_finish(nar->resolver_table,context->cursor);
                }
                return RTI_FALSE;
            }
            /* The address was not understood, continue */
        }
        else
        {
            found_next = RTI_TRUE;
        }

        /* Setup the next index */
        ++context->current_index;
    }

    return RTI_TRUE;
}

/*ci
 * \brief Copy a string from an address string into a buffer.
 *
 * \details
 *
 * Copy the source string into the destination buffer. If the number of
 * characters to copy exceeds max_buffer, then the function returns RTI_FALSE,
 * otherwise RTI_TRUE is returned. If max_length is specified no adjustment
 * is made for the NUL character.
 *
 * NOTE: This function does not perform any checks on the input arguments
 *
 * \param[inout] dest       Destination buffer to copy to.
 * \param[in]    max_buffer The maximum number of characters that can be copied
 *                          into the destination buffer.
 * \param[in]    source     The beginning of the string to copy.
 * \param[inout] c_ptr      On success and if != NULL, this pointer is updated
 *                          to point to the NUL character if max_length = 0,
 *                          otherwise c_ptr points to the character after the
 *                          last copied.
 * \param[in]    max_length The maximum number of characters to copy. If 0,
 *                          the entire ASCIIZ string pointer to by source is
 *                          copied (if space).
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_Address_copy_string_token(char *const dest,
                                RTI_SIZE_T max_buffer,
                                const char *const source,
                                const char **c_ptr,
                                RTI_SIZE_T max_length)
{
    RTI_SIZE_T copy_length = max_length;

    if (copy_length == 0)
    {
        max_length = OSAPI_String_length(source);
        copy_length = max_length + 1;
    }

    if (copy_length > max_buffer)
    {
        NETIO_LOG_AR_ADDRESS_STRING_EXCEEDED(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    OSAPI_Memory_copy(dest,source,copy_length);

    if (c_ptr != NULL)
    {
        *c_ptr = source + max_length;
    }

    return RTI_TRUE;
}

/*ci
 * \brief Break a string into tokens as recognized by the the address parser
 *
 * \param[inout] c_ptr         The current position in the string, pointing
 *                             to the next character after current token on
 *                             output
 * \param[out]   token         Token value on output
 * \param[in]    allow_address Whether the address part of the address string
 *                             is expected. When parsing transports this
 *                             should be false
 * \param[in]    force_address Force the remaining characters to be interpreted
 *                             as an address. This is used when the parser
 *                             has determined that no other valid tokens can be
 *                             parsed.
 * \param[inout] p_ptr         Pointer to the beginning of the token in case the
 *                             parser must be reset. This is used when the
 *                             parser has determined that the current token
 *                             must be an address, and forces the token to
 *                             be parsed again.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_Address_get_token(const char **c_ptr,NETO_AddressToken_T *token,
                        RTI_BOOL allow_address,RTI_BOOL force_address,
                        const char **p_ptr)
{
    RTI_BOOL done = RTI_FALSE;
    RTI_BOOL rval = RTI_FALSE;
    const char *s_ptr = *c_ptr;
    RTI_INT32 digit_base = 10;
    RTI_SIZE_T s_ptr_length;
    RTI_BOOL prefix_colon = RTI_FALSE;
    RTI_BOOL prefix_complete = RTI_FALSE;
    RTI_INT32 prefix_slash = RTI_FALSE;

    token->kind = NETIO_ADDRESSTOKEN_NONE;
    *p_ptr = *c_ptr;

    if (force_address)
    {
        token->kind = NETIO_ADDRESSTOKEN_ADDRESS;
        rval = NETIO_Address_copy_string_token(
                token->value.address_token,NETIO_ADDRESS_TOKEN_MAX_SIZE,
                s_ptr,c_ptr,0);
    }
    else
    {
        while (**c_ptr && !done)
        {
            switch (token->kind)
            {
                case NETIO_ADDRESSTOKEN_NONE:
                    s_ptr = *c_ptr;
                    if ((**c_ptr >= '0') && (**c_ptr <= '9'))
                    {
                        token->kind = NETIO_ADDRESSTOKEN_INTEGER;
                        token->value.integer_token = **c_ptr - '0';
                        (*c_ptr)++;
                    }
                    else if (**c_ptr == '[')
                    {
                        token->kind = NETIO_ADDRESSTOKEN_LEFT_BRACKET;
                        done = RTI_TRUE;
                        rval = RTI_TRUE;
                        (*c_ptr)++;
                    }
                    else if (**c_ptr == ']')
                    {
                        token->kind = NETIO_ADDRESSTOKEN_RIGHT_BRACKET;
                        done = RTI_TRUE;
                        rval = RTI_TRUE;
                        (*c_ptr)++;
                    }
                    else if (**c_ptr == '-')
                    {
                        token->kind = NETIO_ADDRESSTOKEN_HYPHEN;
                        done = RTI_TRUE;
                        rval = RTI_TRUE;
                        (*c_ptr)++;
                    }
                    else if (**c_ptr == '@')
                    {
                        token->kind = NETIO_ADDRESSTOKEN_AT;
                        done = RTI_TRUE;
                        rval = RTI_TRUE;
                        (*c_ptr)++;
                    }
                    else if (((**c_ptr >= 'a') && (**c_ptr <= 'z')) ||
                             ((**c_ptr >= 'A') && (**c_ptr <= 'Z')) ||
                              (**c_ptr == '_'))
                    {
                        /* This could be either address or prefix, assume
                         * prefix
                         */
                        token->kind = NETIO_ADDRESSTOKEN_PREFIX;
                        prefix_colon = RTI_FALSE;
                        prefix_slash = 2;
                        prefix_complete = RTI_FALSE;
                        (*c_ptr)++;
                    }
                    else if (allow_address)
                    {
                        /* Take the rest of the string as an address */
                        token->kind = NETIO_ADDRESSTOKEN_ADDRESS;
                        rval = NETIO_Address_copy_string_token(
                                token->value.address_token,
                                NETIO_ADDRESS_TOKEN_MAX_SIZE,s_ptr,c_ptr,0);
                        done = RTI_TRUE;
                    }
                    else
                    {
                        rval = RTI_FALSE;
                        done = RTI_TRUE;
                        NETIO_LOG_AR_INVALID_TOKEN(OSAPI_LOGKIND_ERROR)
                    }
                    break;
                case NETIO_ADDRESSTOKEN_INTEGER:
                    if ((**c_ptr >= '0') && (**c_ptr <= '9'))
                    {
                        token->value.integer_token *= digit_base;
                        token->value.integer_token += **c_ptr - '0';
                        (*c_ptr)++;
                    }
                    else if (((**c_ptr == 'x') || (**c_ptr == 'X')) &&
                             (token->value.integer_token == 0) &&
                             (digit_base == 10))
                    {
                        /* Handle 0x, change base to hex */
                        digit_base = 16;
                        (*c_ptr)++;
                    }
                    else if ((digit_base == 16) &&
                             (((**c_ptr >= 'a') && (**c_ptr <= 'f')) ||
                              ((**c_ptr >= 'A') && (**c_ptr <= 'F'))))
                    {
                        /* Handle hex numbers */
                        token->value.integer_token *= digit_base;

                        /* NOTE: 'a' is > 'A', thus check if digit > 'a' */
                        if (**c_ptr >= 'a')
                        {
                            token->value.integer_token += (**c_ptr - 'a') + 10;
                        }
                        else
                        {
                            token->value.integer_token += (**c_ptr - 'A') + 10;
                        }

                        (*c_ptr)++;
                    }
                    else if (**c_ptr == '@')
                    {
                        /* We have something on the form integer@, return a
                         * range. c_ptr is advanced since we don't need the
                         * @ anymore.
                         */
                        token->value.range_token.high = token->value.integer_token;
                        token->value.range_token.low  = -1;
                        token->kind = NETIO_ADDRESSTOKEN_RANGE;
                        done = RTI_TRUE;
                        rval = RTI_TRUE;
                        (*c_ptr)++;
                    }
                    else if (allow_address)
                    {
                        /* We are parsing something that is not an integer.
                         * We know it cannot be a prefix because a prefix cannot
                         * start with an integer. Thus, we can only assume that
                         * this is an address and return the rest of the
                         * string as an address.
                         */
                        token->kind = NETIO_ADDRESSTOKEN_ADDRESS;
                        rval = NETIO_Address_copy_string_token(
                                      token->value.address_token,
                                      NETIO_ADDRESS_TOKEN_MAX_SIZE,
                                      s_ptr,c_ptr,0);
                        done = RTI_TRUE;
                    }
                    else
                    {
                        /* An integer, check that it is complete in case it is a
                         * hex integer.
                         */
                        done = RTI_TRUE;
                        if ((digit_base == 16) &&
                            ((*((*c_ptr)-1) == 'X') || (*((*c_ptr)-1) == 'x')))
                        {
                            NETIO_LOG_AR_INVALID_TOKEN(OSAPI_LOGKIND_ERROR)
                            rval = RTI_FALSE;
                        }
                        else
                        {
                            rval = RTI_TRUE;
                        }
                    }

                    /* Only 0 or positive values are allowed. Check if the
                     * value has overflowed  into a negative number only if
                     * the token is still considered an integer
                     */
                    if ((token->kind == NETIO_ADDRESSTOKEN_INTEGER) &&
                        (token->value.integer_token < 0))
                    {
                        done = RTI_TRUE;
                        rval = RTI_FALSE;
                    }
                    break;
                case NETIO_ADDRESSTOKEN_PREFIX:
                    if (!prefix_colon &&
                        (((**c_ptr >= 'a') && (**c_ptr <= 'z')) ||
                         ((**c_ptr >= 'A') && (**c_ptr <= 'Z')) ||
                         ((**c_ptr >= '0') && (**c_ptr <= '9')) ||
                         (**c_ptr == '_')))
                    {
                        (*c_ptr)++;
                    }
                    else if ((**c_ptr == ':') && !prefix_colon)
                    {
                        prefix_colon = RTI_TRUE;
                        (*c_ptr)++;
                    }
                    else if (prefix_colon && (**c_ptr == '/'))
                    {
                        (*c_ptr)++;
                        --prefix_slash;
                        if (!prefix_slash)
                        {
                            /* Copy everything except :// as the prefix
                             * NOTE: NETIO_PREFIX_TOKEN_MAX_SIZE includes
                             * the NULL termination, the maximum allowed
                             * prefix is RT_MAX_FACTORY_NAME characters
                             */
                            s_ptr_length = (RTI_SIZE_T)(*c_ptr - s_ptr);
                            rval = NETIO_Address_copy_string_token(
                                                token->value.prefix_token,
                                                RT_MAX_FACTORY_NAME,
                                                s_ptr,c_ptr,s_ptr_length-3);
                            /* NETIO_Address_copy_string_token only updated
                             * c_ptr up to but not including :
                             * need to add another 3 to move beyond ://
                             */
                            if (rval)
                            {
                                *c_ptr += 3;
                                /* prefix_token has a length  =
                                 * RT_MAX_FACTORY_NAME +1, thus this is always
                                 * safe.
                                 */
                                token->value.prefix_token[s_ptr_length - 3] = 0;
                            }
                            prefix_complete = RTI_TRUE;
                            done = RTI_TRUE;
                        }
                    }
                    else if (allow_address)
                    {
                        /* not prefix://, take the rest of the string as an
                         * address
                         */
                        token->kind = NETIO_ADDRESSTOKEN_ADDRESS;
                        rval = NETIO_Address_copy_string_token(
                                token->value.address_token,
                                NETIO_ADDRESS_TOKEN_MAX_SIZE,
                                s_ptr,c_ptr,0);
                        done = RTI_TRUE;
                    }
                    else
                    {
                        NETIO_LOG_AR_INVALID_TOKEN(OSAPI_LOGKIND_ERROR)
                        done = RTI_TRUE;
                        rval = RTI_FALSE;
                    }
                    break;
                default:
                    NETIO_LOG_AR_INVALID_TOKEN(OSAPI_LOGKIND_ERROR)
                    done = RTI_TRUE;
                    rval = RTI_FALSE;
                    break;
            }
        }
    }

    /* The empty string, the grammar must determine would to do */
    if ((token->kind == NETIO_ADDRESSTOKEN_NONE) && (**c_ptr == 0))
    {
        token->kind = NETIO_ADDRESSTOKEN_EOS;
    }

    /* Special case. If we only get an integer or a partial prefix
     * treat it as an address.
     */
    if (**c_ptr == 0)
    {
        if ((token->kind == NETIO_ADDRESSTOKEN_INTEGER) ||
            ((token->kind == NETIO_ADDRESSTOKEN_PREFIX) && !prefix_complete))
        {
            token->kind = NETIO_ADDRESSTOKEN_ADDRESS;
            rval = NETIO_Address_copy_string_token(token->value.address_token,
                                                   NETIO_ADDRESS_TOKEN_MAX_SIZE,
                                                   s_ptr,c_ptr,0);
        }
        else if (token->kind != NETIO_ADDRESSTOKEN_PREFIX)
        {
            /* The get_token() parses the prefix and sets rval if it is valid
             * or not. For all other tokens it is up to the grammar, the caller
             * of get_token(), to determine if the token is valid or not
             */
            rval = RTI_TRUE;
        }
    }

    return rval;
}

RTI_BOOL
NETIO_Address_parse(const char *name,
                    RTI_UINT32 *base_port,
                    RTI_INT32 *low_index,
                    RTI_INT32 *high_index,
                    RT_ComponentFactoryId_T *out_id,
                    char *address_string,
                    RTI_SIZE_T address_max_string)
{
    RTI_BOOL rval = RTI_FALSE;
    const char *c_ptr;
    const char *p_ptr;
    NETIO_AddressState_T parse_state = NETIO_ADDRESSSTATE_INIT;
    NETO_AddressToken_T token;
    RTI_BOOL done = RTI_FALSE;
    RTI_BOOL range_hyphen = RTI_FALSE;
    RTI_BOOL allow_address;
    RTI_BOOL force_address;

    OSAPI_PRECONDITION((name == NULL) || (out_id == NULL) ||
                           (address_string == NULL) ||
                           (address_max_string < NETIO_ADDRESS_TOKEN_MAX_SIZE),
                           return RTI_FALSE,
                           OSAPI_Log_entry_add_pointer("name",name,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("out_id",out_id,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("address_string",address_string,RTI_FALSE);
                           OSAPI_Log_entry_add_uint("address_max_string",address_max_string,RTI_TRUE);)

    /* Clear all inputs. By definition a valid index must be >= 0, but the
     * parser does not know what a valid value is in the case where a default
     * non-zero index is used if none is specified. Thus, the output cannot
     * be set to 0 because the caller cannot distinguish between for example
     * address and 0@address. The caller can however check for -1 to determine
     * that no index was specified and then set the default value.
     */
    *low_index = -1;
    *high_index = -1;

    /* The parser does not support specifying a port number, thus it is always
     * set to 0. The caller should always have a well-defined base-port in case
     * one is not specified.
     */
    *base_port = 0;

    /* Clear the address output in case the string does not contain one */
    address_string[0] = 0;

    /* Clear the interface id in case the string does not contain a prefix */
    RT_ComponentFactoryId_clear(out_id);

    c_ptr = name;

    /* This flag is used to control whether the tokenizer should accept and
     * address or not.
     */
    allow_address = RTI_TRUE;
    force_address = RTI_FALSE;

    /* If the empty string is given, return as a valid address string
     * valid all default indices
     */
    if (*c_ptr == 0)
    {
        /* The empty string is OK. The output string has already been cleared */
        return RTI_TRUE;
    }

    while (NETIO_Address_get_token(&c_ptr,&token,allow_address,force_address,&p_ptr)
            && !done)
    {
        /* If no errors has occurred before this, then accept the string
         * and return the current token
         */
        if (token.kind == NETIO_ADDRESSTOKEN_EOS)
        {
            rval = RTI_TRUE;
            break;
        }
        switch (parse_state)
        {
        case NETIO_ADDRESSSTATE_INIT:
            switch (token.kind)
            {
            case NETIO_ADDRESSTOKEN_RANGE:
                *low_index = token.value.range_token.low;
                *high_index = token.value.range_token.high;
                parse_state = NETIO_ADDRESSSTATE_PREFIX;
                break;
            case NETIO_ADDRESSTOKEN_ADDRESS:
                done = RTI_TRUE;
                rval = NETIO_Address_copy_string_token(
                        address_string,
                        address_max_string,token.value.address_token,NULL,0);
                break;
            case NETIO_ADDRESSTOKEN_PREFIX:
                if (!RT_ComponentFactoryId_set_name(out_id,
                                                    token.value.prefix_token))
                {
                    done = RTI_TRUE;
                    rval = RTI_FALSE;
                }
                else
                {
                    parse_state = NETIO_ADDRESSSTATE_ADDRESS;
                    force_address = RTI_TRUE;
                }
                break;
            case NETIO_ADDRESSTOKEN_LEFT_BRACKET:
                parse_state = NETIO_ADDRESSSTATE_LOW_RANGE;
                allow_address = RTI_FALSE;
                range_hyphen = RTI_FALSE;
                break;
            case NETIO_ADDRESSTOKEN_AT:
                parse_state = NETIO_ADDRESSSTATE_PREFIX;
                break;
            default:
                /* The parser got something that cannot be a valid
                 * token. Re-parse the previous token again, this time at
                 * as an address.
                 */
                parse_state = NETIO_ADDRESSSTATE_ADDRESS;
                force_address = RTI_TRUE;
                c_ptr = p_ptr;
                break;
            }
            break;
        case NETIO_ADDRESSSTATE_PREFIX:
            if (token.kind == NETIO_ADDRESSTOKEN_PREFIX)
            {
                if (!RT_ComponentFactoryId_set_name(out_id,
                                                    token.value.prefix_token))
                {
                    done = RTI_TRUE;
                    rval = RTI_FALSE;
                }
            }
            else
            {
                /* The parser expected name:// but got something else.
                 * Assume it is part of the address by reading tokens again,
                 * but this time enforcing it as an address
                 */
                parse_state = NETIO_ADDRESSSTATE_ADDRESS;
                force_address = RTI_TRUE;
                c_ptr = p_ptr;
            }
            break;
        case NETIO_ADDRESSSTATE_LOW_RANGE:
            if (token.kind == NETIO_ADDRESSTOKEN_INTEGER)
            {
                /* [x */
                *low_index = token.value.integer_token;
                parse_state = NETIO_ADDRESSSTATE_HIGH_RANGE;

            }
            else if (token.kind == NETIO_ADDRESSTOKEN_HYPHEN)
            {
                /* [-
                 * Leave low_index at -1 so the caller knows that it has not
                 * been set.
                 */
                range_hyphen = RTI_TRUE;
                parse_state = NETIO_ADDRESSSTATE_HIGH_RANGE;
            }
            else
            {
                NETIO_LOG_AR_INVALID_TOKEN(OSAPI_LOGKIND_ERROR)
                done = RTI_TRUE;
                rval = RTI_FALSE;
            }
            break;
        case NETIO_ADDRESSSTATE_HIGH_RANGE:
            /* When this state is entered the parser is in one of two
             * states (x and y are integers):
             * [- (range_hyphen = TRUE)
             * [x (range_hyphen = FALSE)
             */
            if ((token.kind == NETIO_ADDRESSTOKEN_HYPHEN) && !range_hyphen)
            {
                /* [x-] */
                range_hyphen = RTI_TRUE;
                parse_state = NETIO_ADDRESSSTATE_HIGH_RANGE;
            }
            else if (token.kind == NETIO_ADDRESSTOKEN_INTEGER)
            {
                /* [x-y] or [-x]*/
                *high_index = token.value.integer_token;
                parse_state = NETIO_ADDRESSSTATE_END_RANGE;
            }
            else if ((token.kind == NETIO_ADDRESSTOKEN_RIGHT_BRACKET) && !range_hyphen)
            {
                /* [x] */
                *high_index = *low_index;
                parse_state = NETIO_ADDRESSSTATE_AT;
            }
            else
            {
                NETIO_LOG_AR_INVALID_TOKEN(OSAPI_LOGKIND_ERROR)
                done = RTI_TRUE;
                rval = RTI_FALSE;
            }
            break;
        case NETIO_ADDRESSSTATE_END_RANGE:
            if (token.kind != NETIO_ADDRESSTOKEN_RIGHT_BRACKET)
            {
                NETIO_LOG_AR_INVALID_TOKEN(OSAPI_LOGKIND_ERROR)
                done = RTI_TRUE;
                rval = RTI_FALSE;
            }
            else
            {
                parse_state = NETIO_ADDRESSSTATE_AT;
            }
            break;
        case NETIO_ADDRESSSTATE_AT:
            if (token.kind != NETIO_ADDRESSTOKEN_AT)
            {
                NETIO_LOG_AR_INVALID_TOKEN(OSAPI_LOGKIND_ERROR)
                done = RTI_TRUE;
                rval = RTI_FALSE;
            }
            else
            {
                parse_state = NETIO_ADDRESSSTATE_PREFIX;
                allow_address = RTI_TRUE;
            }
            break;
        case NETIO_ADDRESSSTATE_ADDRESS:
            done = RTI_TRUE;
            rval = NETIO_Address_copy_string_token(
                    address_string,
                    address_max_string,token.value.address_token,NULL,0);
            break;
        default:
            NETIO_LOG_AR_INVALID_TOKEN(OSAPI_LOGKIND_ERROR)
            done = RTI_TRUE;
            rval = RTI_FALSE;
            break;
        }
    }

    if (*low_index > *high_index)
    {
        NETIO_LOG_INVALID_ADDRESS_INDEX(OSAPI_LOGKIND_ERROR,
                                        *low_index,*high_index)
        rval = RTI_FALSE;
    }

    return rval;
}

/*ci @} */

