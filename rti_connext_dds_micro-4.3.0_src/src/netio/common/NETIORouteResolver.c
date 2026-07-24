/*
 * FILE: NETIORouteResolver.c - NETIO Route implementation
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
 * 15jul2015,tk MICRO-1433/PR#15418 Added robustness checks for netmask bits
 * 05may2015,tk MICRO-1174/PR#14672 Fixed netio mask table and bitmasks
 *              MICRO-1173/PR#14670
 * 23feb2015,eh MICRO-1081: fix function/var names to conform to coding std
 * 10dec2014,tk MICRO-978/PR#12914 Consistently return records on failure
 * 31jul2014,tk MICRO-241/PR#1413 - Removed superfluous DB fields
 * 31jul2014,tk MICRO-842/PR#9683 - Removed superfluous paramaters in DB
 *                                  compare function
 * 05may2014,tk MICRO-72 - Updated based on CR-232
 * 16sep2012,tk Written
 */
/*ci
 * \file
 * \brief NETIO Route implementation
 *
 * \details
 * This file implements functions to determine routes to a peer address.
 */

/*ci \addtogroup NETIO_RouteClass
 *  @{
 */
#include "osapi/osapi_config.h"

#ifndef osapi_string_h
#include "osapi/osapi_string.h"
#endif
#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef reda_string_h
#include "reda/reda_string.h"
#endif
#ifndef db_api_h
#include "db/db_api.h"
#endif
#ifndef rt_rt_h
#include "rt/rt_rt.h"
#endif
#ifndef netio_log_h
#include "netio/netio_log.h"
#endif
#ifndef netio_common_h
#include "netio/netio_common.h"
#endif
#ifndef netio_address_h
#include "netio/netio_address.h"
#endif
#ifndef netio_route_h
#include "netio/netio_route.h"
#endif
#ifndef netio_common_h
#include "netio/netio_common.h"
#endif

#include "NETIORouteResolver.h"

/* NOTE: 33 entries, 0-32 bits! */
RTI_PRIVATE const RTI_UINT32 NETIO_masks[]=
{
        0x00000000,0x80000000,0xc0000000,0xe0000000,
        0xf0000000,0xf8000000,0xfc000000,0xfe000000,
        0xff000000,0xff800000,0xffc00000,0xffe00000,
        0xfff00000,0xfff80000,0xfffc0000,0xfffe0000,
        0xffff0000,0xffff8000,0xffffc000,0xffffe000,
        0xfffff000,0xfffff800,0xfffffc00,0xfffffe00,
        0xffffff00,0xffffff80,0xffffffc0,0xffffffe0,
        0xfffffff0,0xfffffff8,0xfffffffc,0xfffffffe,
        0xffffffff
};

/*ci
 * \brief Operations to apply to a route table
 */
typedef enum
{
    /*ci
     * \brief Add a route to the route table
     */
    NETIO_ROUTE_OPERATION_ADD,

    /*ci
     * \brief Add a route to a peer to the route table. This is used with
              the initial peers and add_peer APIs.
     */
    NETIO_ROUTE_OPERATION_ADD_PEER,

    /*ci
     * \brief Delete a route from the route table
     */
    NETIO_ROUTE_OPERATION_DELETE,

    /*ci
     * \brief Lookup a route in the route table
     */
    NETIO_ROUTE_OPERATION_LOOKUP
} NETIO_RouteOperation_T;

/*** SOURCE_BEGIN ***/

RTI_BOOL
NETIO_RouteResolver_fill_netmask(struct NETIO_Netmask *netmask)
{
    RTI_UINT32 i,j;

    if (netmask->bits > 128U)
    {
        return RTI_FALSE;
    }

    /* Don't fill in an already filled in mask */
    if ((netmask->mask[0] != 0U) ||
        (netmask->mask[1] != 0U) ||
        (netmask->mask[2] != 0U) ||
        (netmask->mask[3] != 0U))
    {
        return RTI_FALSE;
    }

    i = netmask->bits / NETIO_NETMASK_MASK_BITS_PER_UNIT;
    for (j = 0; j < i; j++)
    {
        /* all bits are set */
        netmask->mask[j] = NETIO_masks[32];
    }

    i = netmask->bits % NETIO_NETMASK_MASK_BITS_PER_UNIT;
    if (i)
    {
        /* less than 32 bits to fill in. Note that the index is the number
         * of bits (index 0 is 0 bits, index 1 is 1 bit etc).
         */
        netmask->mask[j] = NETIO_masks[i];
    }

    return RTI_TRUE;
}

/*ci
 * \brief Calculate the index into \ref NETIO_masks
 *        given the netmask \a bits and the current
 *        index \a i
 *
 * \param[in] bits The number of bits in the netmask
 * \param[in] i    The current index in a netmask array
 *
 * \return The index in the NETIO_masks table to use when applying the
 *         mask to a bitmap element in the bitmap array
 */
RTI_PRIVATE RTI_UINT32
NETIO_calculate_mask_index(RTI_UINT32 bits,RTI_UINT32 i)
{
    return (bits >= ((i+1)*NETIO_NETMASK_MASK_BITS_PER_UNIT) ?
        NETIO_NETMASK_MASK_BITS_PER_UNIT :
        (bits % NETIO_NETMASK_MASK_BITS_PER_UNIT));
}

/*ci
 * \brief Compare entries in the table of route entries. The function is
 *        compatible with \ref DB_IndexCompare_T
 *
 * \param[in] flags Used to distinguish between op2 being a key or record
 * \param[in] op1   A NETIO_RouteResolverRecord already in the database
 * \param[in] op2   Either a NETIO_RouteResolverRecord being added or a key
 *                  being searched for
 *
 * \return positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2
 *         zero if op1 is equal to op2
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_INT32
NETIO_RouteResolver_compare_entry(RTI_INT32 flags,
                                  const DB_Record_T op1, void *op2)
{
    struct NETIO_RouteResolverRecord *left = (struct NETIO_RouteResolverRecord*)op1;
    struct NETIO_RouteResolverRecord *right = (struct NETIO_RouteResolverRecord*)op2;
    RTI_INT32 diff;
    RTI_UINT32 left_mask_index;
    RTI_UINT32 left_mask_value;
    RTI_UINT32 i,j;
    UNUSED_ARG(flags);

    diff = NETIO_Address_get_kind(&left->address) -
           NETIO_Address_get_kind(&right->address);
    if (diff)
    {
        return diff;
    }

    if (left->netmask.bits > right->netmask.bits)
    {
        return -1;
    }

    if (left->netmask.bits < right->netmask.bits)
    {
        return 1;
    }

    /* Netmask is of same size, use address to sort */
    j = (left->netmask.bits / NETIO_NETMASK_MASK_BITS_PER_UNIT) +
               (left->netmask.bits % NETIO_NETMASK_MASK_BITS_PER_UNIT ? 1 : 0);
    for (i = 0; i < j; ++i)
    {
        RTI_UINT32 left_value;
        RTI_UINT32 right_value;

        left_mask_index = NETIO_calculate_mask_index(left->netmask.bits,i);
        left_mask_value = left->netmask.mask[i] & NETIO_masks[left_mask_index];

        /* Do not calculate diff with a subtraction. The address are sorted
         * in descending order of highest address, biggest mask and down
         * The address is treated as an unsigned.
         */
        left_value = NETIO_ntohl(left->address.value.as_uint32.value[i])
                            & left_mask_value;
        right_value = NETIO_ntohl(right->address.value.as_uint32.value[i])
                            & left_mask_value;

        if (left_value > right_value)
        {
            diff = -1;
            break;
        }
        else if (left_value < right_value)
        {
            diff = 1;
            break;
        }
        else if (left->netmask.bits <= (32 * (i + 1)))
        {
            /* Here, all the netmask bits have been compared, must be equal */
            diff = 0;
            break;
        }
    }

    return diff;
}

/*ci
 * \brief Compare entries in the table of route entries based only on the
 *        locator kind. The function is compatible with \ref DB_IndexCompare_T
 *
 * \param[in] flags Used to distinguish between op2 being a key or record
 * \param[in] op1   A NETIO_RouteResolverRecord already in the database
 * \param[in] op2   Either a NETIO_Address being added or a key
 *                  being searched for
 *
 * \return positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2
 *         zero if op1 is equal to op2
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_INT32
NETIO_RouteResolver_compare_kind(RTI_INT32 flags,
                                 const DB_Record_T op1, void *op2)
{
    struct NETIO_RouteResolverRecord *left = (struct NETIO_RouteResolverRecord*)op1;
    struct NETIO_Address *right;

    if (DB_SELECT_OP2_IS_KEY(flags))
    {
        right = (struct NETIO_Address*)op2;
    }
    else
    {
        right = &((struct NETIO_RouteResolverRecord*)op2)->address;
    }

    return (NETIO_Address_get_kind(&left->address) -
            NETIO_Address_get_kind(right));
}

NETIO_RouteResolver_T*
NETIO_RouteResolver_new(DB_Database_T db,NETIO_AddressResolver_T *nar,
                     const char* const name,
                     struct NETIO_RouteResolverProperty *const property)
{
    NETIO_RouteResolver_T *rtable = NULL;
    NETIO_RouteResolver_T *rval = NULL;
    struct DB_TableProperty tbl_property = DB_TableProperty_INITIALIZER;
    DB_ReturnCode_T dbrc;
    struct DB_IndexProperty idx_property = DB_IndexProperty_INITIALIZER;

    OSAPI_PRECONDITION((db == NULL) || (name == NULL) ||
                           (property == NULL) || (nar == NULL),
                    return NULL,
                    OSAPI_Log_entry_add_pointer("db",db,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("name",name,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("property",property,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("nar",nar,RTI_TRUE);)

    OSAPI_Heap_allocate_struct(&rtable,struct NETIO_RouteResolver);
    if (rtable == NULL)
    {
        NETIO_LOG_ROUTE_RTABLE_ALLOC(OSAPI_LOGKIND_ERROR)
        return NULL;
    }

    rtable->property = *property;
    rtable->db = db;
    rtable->nar = nar;

    tbl_property.max_cursors = 1;
    tbl_property.max_indices = 1;
    tbl_property.max_records = rtable->property.max_routes;

    dbrc = DB_Database_create_table(&rtable->route_table,
            db,name,sizeof(struct NETIO_RouteResolverRecord),
            NETIO_RouteResolver_compare_entry,&tbl_property);

    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_LOG_ROUTE_RTABLE_CREATE(OSAPI_LOGKIND_ERROR,dbrc)
#ifndef RTI_CERT
        OSAPI_Heap_free_struct(rtable);
#endif
        goto done;
    }

    idx_property.max_entries = rtable->property.max_routes;

    dbrc = DB_Table_create_index(rtable->route_table,
                                 &rtable->kind_index,
                                 NETIO_RouteResolver_compare_kind,&idx_property);
    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_LOG_ROUTE_RTABLE_CREATE(OSAPI_LOGKIND_ERROR,dbrc)
#ifndef RTI_CERT
        OSAPI_Heap_free_struct(rtable);
#endif
        goto done;
    }

    rval = rtable;

done:
    return rval;
}

#ifndef RTI_CERT
RTI_BOOL
NETIO_RouteResolver_delete(NETIO_RouteResolver_T *r_table)
{
    DB_ReturnCode_T dbrc;

    OSAPI_PRECONDITION(r_table == NULL,return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("r_table",r_table,RTI_TRUE);)

    dbrc = DB_Table_delete_index(r_table->route_table,
                                 r_table->kind_index);
    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_LOG_ROUTE_RTABLE_DELETE(OSAPI_LOGKIND_ERROR,dbrc)
        return RTI_FALSE;
    }

    dbrc = DB_Database_delete_table(r_table->db,r_table->route_table);
    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_LOG_ROUTE_RTABLE_DELETE(OSAPI_LOGKIND_ERROR,dbrc)
        return RTI_FALSE;
    }

    OSAPI_Heap_free_struct(r_table);

    return RTI_TRUE;
}
#endif /* !RTI_CERT */

RTI_BOOL
NETIO_RouteResolver_add_interface(NETIO_RouteResolver_T *r_table,
                                  const char *name,
                                  NETIO_Interface_T *intf,
                                  struct NETIO_Address *address,
                                  struct NETIO_Netmask *netmask,
                                  RTI_BOOL *exists)
{
    struct NETIO_RouteResolverRecord *r_entry = NULL;
    DB_ReturnCode_T dbrc;
    RTI_BOOL rval = RTI_FALSE;
    struct NETIO_RouteResolverRecord key = NETIO_RouteResolverRecord_INITIALIZER;

    OSAPI_PRECONDITION((r_table == NULL) || (address == NULL) ||
                           (intf == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("r_table",r_table,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("address",address,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("intf",intf,RTI_TRUE);)

    if ((netmask->bits > NETIO_NETMASK_MASK_MAX_BITS))
    {
        return RTI_FALSE;
    }

    if (exists)
    {
        *exists = RTI_FALSE;
    }

    key.address = *address;
    key.netmask = *netmask;

    /* If the netmask is not specified (assumed to be in in host order),
     * create one based on the number of bits in the netmask.
     */
    if ((key.netmask.bits > 0)
         && (key.netmask.mask[0] == 0U)
         && (key.netmask.mask[1] == 0U)
         && (key.netmask.mask[2] == 0U)
         && (key.netmask.mask[3] == 0U)
         && !NETIO_RouteResolver_fill_netmask(&key.netmask))
    {
        return RTI_FALSE;
    }

    key.intf = intf;

    dbrc = DB_Table_select_match(r_table->route_table,
            DB_TABLE_DEFAULT_INDEX,(DB_Record_T*)&r_entry,(DB_Key_T)&key);
    if (dbrc == DB_RETCODE_OK)
    {
        if (exists)
        {
            *exists = RTI_TRUE;
        }
        rval = RTI_TRUE;
        goto done;
    }

    dbrc = DB_Table_create_record(r_table->route_table,(DB_Record_T*)&r_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_LOG_ROUTE_RTABLE_ADD(OSAPI_LOGKIND_ERROR,dbrc)
        goto done;
    }

    *r_entry = key;

    if (!RT_ComponentFactoryId_set_name(&r_entry->id,name))
    {
        goto done;
    }

    dbrc = DB_Table_insert_record(r_table->route_table,(DB_Record_T)r_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_LOG_ROUTE_RTABLE_ADD(OSAPI_LOGKIND_ERROR,dbrc)
        (void)DB_Table_delete_record(r_table->route_table,(DB_Record_T)r_entry);
        goto done;
    }

    OSAPI_TRACE_NET("added route interface:",RTI_FALSE)
    OSAPI_TRACE_INT32("kind",address->kind,RTI_FALSE)
    OSAPI_TRACE_INT32("port",address->port,RTI_FALSE)
    OSAPI_TRACE_GUID("address",&address->value.as_int32,RTI_FALSE)
    OSAPI_TRACE_GUID("netmask",&netmask->mask,RTI_FALSE)
    OSAPI_TRACE_INT32("netmask.bits",netmask->bits,RTI_TRUE)

    rval = RTI_TRUE;

done:
    return rval;
}

RTI_BOOL
NETIO_RouteResolver_lookup_interface(NETIO_RouteResolver_T *const r_table,
                                     const struct NETIO_Address *const address)
{
    struct NETIO_RouteResolverRecord *r_entry = NULL;
    DB_ReturnCode_T dbrc;

    dbrc = DB_Table_select_match(r_table->route_table,
                                 r_table->kind_index,
                                 (DB_Record_T*)&r_entry,
                                 (DB_Key_T)address);

#if OSAPI_ENABLE_LOG
    if ((dbrc != DB_RETCODE_OK) && (dbrc != DB_RETCODE_NO_DATA))
    {
        NETIO_LOG_ROUTE_RTABLE_UPDATE(OSAPI_LOGKIND_ERROR,dbrc)
    }
#endif

    return (dbrc == DB_RETCODE_OK ? RTI_TRUE : RTI_FALSE);
}

RTI_BOOL
NETIO_RouteResolver_is_address_reachable(NETIO_RouteResolver_T *const r_table,
                                         const struct NETIO_AddressEx *const address,
                                         RTI_BOOL *is_reachable)

{
    NETIO_Interface_T *src_intf;
    struct NETIO_Address basic_addr = NETIO_Address_INITIALIZER;

    *is_reachable = RTI_FALSE;

    basic_addr.kind = address->kind;
    basic_addr.port = address->port;
    OSAPI_Memory_copy(&basic_addr.value,
                      &address->value,
                      sizeof(basic_addr.value));

    if (!NETIO_RouteResolver_find_interface(r_table,&src_intf,&basic_addr))
    {
        return RTI_FALSE;
    }

    if (!NETIO_Interface_has_is_address_reachable(src_intf))
    {
        return RTI_TRUE;
    }

    return NETIO_Interface_is_address_reachable(src_intf,address,is_reachable);
}

RTI_INT32
NETIO_RouteResolver_get_minimum_mtu(NETIO_RouteResolver_T *const r_table)
{
    DB_Cursor_T cursor = NULL;
    DB_ReturnCode_T dbrc;
    struct NETIO_RouteResolverRecord *r_entry;
    RTI_UINT32 min_mtu = INT_MAX;
    struct NETIO_TransportProperty p = NETIO_TransportProperty_INITIALIZER;

    dbrc = DB_Table_select_all(r_table->route_table,
                               DB_TABLE_DEFAULT_INDEX,&cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_LOG_ROUTE_RTABLE_UPDATE(OSAPI_LOGKIND_ERROR,dbrc)
        return -1;
    }

    dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&r_entry);
    if (dbrc == DB_RETCODE_NO_DATA)
    {
        DB_Cursor_finish(r_table->route_table,cursor);
        return 0;
    }

    while (dbrc == DB_RETCODE_OK)
    {
        NETIO_Interface_get_transport_properties(r_entry->intf,&p);
        if (p.send_size_max < min_mtu)
        {
            min_mtu = p.send_size_max;
        }
        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&r_entry);
    }


    DB_Cursor_finish(r_table->route_table,cursor);

    /* Work on 2GB maximum MTU since DDS tends to work on signed ints */
    return (RTI_INT32)min_mtu;
}

RTI_INT32
NETIO_RouteResolver_get_maximum_mtu(NETIO_RouteResolver_T *const r_table)
{
    DB_Cursor_T cursor = NULL;
    DB_ReturnCode_T dbrc;
    struct NETIO_RouteResolverRecord *r_entry;
    RTI_UINT32 max_mtu = 0;
    struct NETIO_TransportProperty p = NETIO_TransportProperty_INITIALIZER;
    dbrc = DB_Table_select_all(r_table->route_table,
                               DB_TABLE_DEFAULT_INDEX,&cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_LOG_ROUTE_RTABLE_UPDATE(OSAPI_LOGKIND_ERROR,dbrc)
        return -1;
    }

    dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&r_entry);
    if (dbrc == DB_RETCODE_NO_DATA)
    {
        DB_Cursor_finish(r_table->route_table,cursor);
        return 0;
    }
    else
    {
        while (dbrc == DB_RETCODE_OK)
        {
            NETIO_Interface_get_transport_properties(r_entry->intf,&p);
            if (p.send_size_max > max_mtu)
            {
                max_mtu = p.send_size_max;
            }
            dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&r_entry);
        }
    }

    DB_Cursor_finish(r_table->route_table,cursor);

    /* Work on 2GB maximum MTU since DDS tends to work on signed ints */
    if (max_mtu > INT_MAX)
    {
        max_mtu = INT_MAX;
    }
    return (RTI_INT32)max_mtu;
}

#ifndef RTI_CERT
RTI_BOOL
NETIO_RouteResolver_delete_interface(NETIO_RouteResolver_T *r_table,
                                  NETIO_Interface_T *intf,
                                  struct NETIO_Address *address,
                                  struct NETIO_Netmask *netmask,
                                  RTI_BOOL *exists)
{
    struct NETIO_RouteResolverRecord key;
    struct NETIO_RouteResolverRecord *r_entry = NULL;
    DB_ReturnCode_T dbrc;

    key.address = *address;
    key.netmask = *netmask;
    key.intf = intf;

    if ((netmask->bits > NETIO_NETMASK_MASK_MAX_BITS))
    {
        return RTI_FALSE;
    }

    if (exists)
    {
        *exists = RTI_FALSE;
    }

    dbrc = DB_Table_remove_record(r_table->route_table,
                    (DB_Record_T*)&r_entry,(DB_Key_T)&key);
    if (dbrc != DB_RETCODE_OK)
    {
        if (dbrc != DB_RETCODE_NO_DATA)
        {
            NETIO_LOG_ROUTE_RTABLE_DELETE(OSAPI_LOGKIND_ERROR,dbrc)
            return RTI_FALSE;
        }
        else
        {
            return RTI_TRUE;
        }
    }

    if (exists)
    {
        *exists = RTI_TRUE;
    }

    dbrc = DB_Table_delete_record(r_table->route_table,(DB_Record_T)r_entry);

#if OSAPI_ENABLE_LOG
    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_LOG_ROUTE_RTABLE_DELETE(OSAPI_LOGKIND_ERROR,dbrc)
    }
#endif

    OSAPI_TRACE_NET("deleted route interface:",RTI_FALSE)
    OSAPI_TRACE_INT32("kind",address->kind,RTI_FALSE)
    OSAPI_TRACE_INT32("port",address->port,RTI_FALSE)
    OSAPI_TRACE_GUID("address",&address->value.as_int32,RTI_FALSE)
    OSAPI_TRACE_GUID("netmask",&netmask->mask,RTI_FALSE)
    OSAPI_TRACE_INT32("netmask.bits",netmask->bits,RTI_TRUE)

    return (dbrc == DB_RETCODE_OK ? RTI_TRUE : RTI_FALSE);
}
#endif /* !RTI_CERT */

/*ci
 * \brief Compare a \ref NETIO_Address to an entry in the route table
 *        top determine if the route can send to the address
 *
 * \param[in] address  The address to find a matching route entry for
 * \param[in] r_entry  An entry in the route table
 *
 * \return RTI_TRUE is the entry is a match, RTI_FALSE otherwise
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_address_match(const struct NETIO_Address *const address,
                    struct NETIO_RouteResolverRecord *r_entry)
{
    RTI_UINT32 diff=0;
    RTI_UINT32 left_mask_index;
    RTI_UINT32 left_mask_value;
    RTI_UINT32 i,j;

    if (NETIO_Address_get_kind(&r_entry->address) !=
        NETIO_Address_get_kind(address))
    {
        return RTI_FALSE;
    }

    j = (r_entry->netmask.bits / NETIO_NETMASK_MASK_BITS_PER_UNIT) +
            (r_entry->netmask.bits % NETIO_NETMASK_MASK_BITS_PER_UNIT ? 1 : 0);
    for (i = 0; i < j; ++i)
    {
        RTI_UINT32 left_value;
        RTI_UINT32 right_value;

        left_mask_index = NETIO_calculate_mask_index(r_entry->netmask.bits,i);
        left_mask_value = r_entry->netmask.mask[i] & NETIO_masks[left_mask_index];

        left_value = NETIO_ntohl(r_entry->address.value.as_uint32.value[i]);
        right_value = NETIO_ntohl(address->value.as_uint32.value[i]);

        diff = ((left_value & left_mask_value)
                - (right_value & left_mask_value));

        if (diff || (r_entry->netmask.bits <= (32*(i+1))))
        {
            break;
        }
    }

    return diff ? RTI_FALSE : RTI_TRUE;
}

/*ci
 * \brief Update a route table entry
 *
 * \details
 * This function is implements functionality to access a route table and
 * lookup, add or delete routes.
 *
 * \param[in]  r_table       The route table to update
 * \param[in]  src_intf      The source of the route
 * \param[in]  dst_reader    The destination address
 * \param[in]  via_address   The address to pass downstream
 * \param[in]  property      The properties of the route
 * \param[out] route_existed Whether the route already existed or not, not
 *                           considered an error
 * \param[in]  operation     Operation to apply to route table
 * \param[out] found_route   Whether a route was found or not
 * \param[out] route_exists  RTI_TRUE if the route exists, RTI_FALSE if not
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref NETIO_RouteResolver_add_route,
 *     \ref NETIO_RouteResolver_delete_route
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_RouteResolver_route_update(NETIO_RouteResolver_T *r_table,
                              NETIO_Interface_T *src_intf,
                              struct NETIO_Address *dst_reader,
                              struct NETIO_Address *via_address,
                              struct NETIORouteProperty *property,
                              RTI_BOOL *route_existed,
                              NETIO_RouteOperation_T operation,
                              RTI_BOOL *found_route,
                              RTI_BOOL *route_exists)
{
    DB_Cursor_T cursor = NULL;
    DB_ReturnCode_T dbrc;
    struct NETIO_RouteResolverRecord *r_entry;
    RTI_BOOL brc = RTI_TRUE;

    if (found_route)
    {
        *found_route = RTI_FALSE;
    }

    dbrc = DB_Table_select_all(r_table->route_table,
                               DB_TABLE_DEFAULT_INDEX,&cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_LOG_ROUTE_RTABLE_UPDATE(OSAPI_LOGKIND_ERROR,dbrc)
        return RTI_FALSE;
    }

    dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&r_entry);
    while (dbrc == DB_RETCODE_OK)
    {
        if (NETIO_address_match(via_address,r_entry))
        {
            if (found_route)
            {
                *found_route = RTI_TRUE;
            }

            if ((operation == NETIO_ROUTE_OPERATION_ADD) ||
                (operation == NETIO_ROUTE_OPERATION_ADD_PEER))
            {
                /* Also call the lower level interface, but only with the
                 * destination address. All other parameters are NULL since
                 * this interface is only called indirectly. Call the lower
                 * level first in case the upper-level needs to use the
                 * interface.
                 */
                brc = NETIO_Interface_add_route(
                        (NETIO_Interface_T*)r_entry->intf,via_address,
                        NULL,NULL,NULL,NULL);

                /* If the lower level interface cannot add the route, then
                 * do not add it to the upper level as the destination cannot
                 * be reached.
                 */
                if (brc)
                {
                    brc = NETIO_Interface_add_route(
                            (NETIO_Interface_T*)src_intf,dst_reader,
                            r_entry->intf,via_address,property,route_existed);
                }
                else if (operation == NETIO_ROUTE_OPERATION_ADD)
                {
                    /* It is alright for adding a route to fail because this
                     * interface may have received an unreachable locator during
                     * discovery or may be unable to reach its own locators.
                     */
                    brc = RTI_TRUE;
                }
            }
            else if (operation == NETIO_ROUTE_OPERATION_DELETE)
            {
                brc = NETIO_Interface_delete_route(
                        (NETIO_Interface_T*)src_intf,dst_reader,
                        r_entry->intf,via_address,route_existed);
                if (brc)
                {
                    brc = NETIO_Interface_delete_route(
                            (NETIO_Interface_T*)r_entry->intf,via_address,
                            NULL,NULL,NULL);
                }
            }
            else if (operation == NETIO_ROUTE_OPERATION_LOOKUP)
            {
                brc = NETIO_Interface_lookup_route(src_intf,
                        dst_reader,r_entry->intf,via_address,route_exists);
            }
            else
            {
                NETIO_LOG_ILLEGAL_ROUTE_OPERATION(OSAPI_LOGKIND_ERROR,operation)
                brc = RTI_FALSE;
            }
            break;
        }
        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&r_entry);
    }

    DB_Cursor_finish(r_table->route_table,cursor);

    return brc;
}

RTI_BOOL
NETIO_RouteResolver_find_interface(NETIO_RouteResolver_T *r_table,
                                   NETIO_Interface_T **src_intf,
                                   const struct NETIO_Address *const via_addr)
{
    DB_Cursor_T cursor = NULL;
    DB_ReturnCode_T dbrc;
    struct NETIO_RouteResolverRecord *r_entry;
    RTI_BOOL brc = RTI_FALSE;

    dbrc = DB_Table_select_all(r_table->route_table,
                               DB_TABLE_DEFAULT_INDEX,&cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_LOG_ROUTE_RTABLE_UPDATE(OSAPI_LOGKIND_ERROR,dbrc)
        return RTI_FALSE;
    }

    dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&r_entry);
    while (dbrc == DB_RETCODE_OK)
    {
        if (NETIO_address_match(via_addr,r_entry))
        {
            *src_intf = r_entry->intf;
            brc = RTI_TRUE;
            break;
        }
        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&r_entry);
    }

    DB_Cursor_finish(r_table->route_table,cursor);

    return brc;
}

RTI_BOOL
NETIO_RouteResolver_interface_supports_kind(NETIO_RouteResolver_T *r_table,
                                            NETIO_Interface_T *intf,
                                            RTI_INT32 kind)
{
    DB_Cursor_T cursor = NULL;
    DB_ReturnCode_T dbrc;
    struct NETIO_RouteResolverRecord *r_entry;
    RTI_BOOL brc = RTI_FALSE;

    dbrc = DB_Table_select_all(r_table->route_table,
                               DB_TABLE_DEFAULT_INDEX,&cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_LOG_ROUTE_RTABLE_UPDATE(OSAPI_LOGKIND_ERROR,dbrc)
        return RTI_FALSE;
    }

    dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&r_entry);
    while (dbrc == DB_RETCODE_OK)
    {
        if ((r_entry->intf == intf) && r_entry->address.kind == kind)
        {
            brc = RTI_TRUE;
            break;
        }
        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&r_entry);
    }

    DB_Cursor_finish(r_table->route_table,cursor);

    return brc;
}

RTI_BOOL
NETIO_RouteResolver_lookup_route(NETIO_RouteResolver_T *r_table,
                                 NETIO_Interface_T *src_intf,
                                 struct NETIO_Address *dst_reader,
                                 struct NETIO_Address *via_address,
                                 RTI_BOOL *route_exists)
{
    return NETIO_RouteResolver_route_update(
                  r_table,src_intf,dst_reader,via_address,NULL,
                  NULL,NETIO_ROUTE_OPERATION_LOOKUP,NULL,route_exists);
}

RTI_BOOL
NETIO_RouteResolver_add_route(NETIO_RouteResolver_T *r_table,
                           NETIO_Interface_T *src_intf,
                           struct NETIO_Address *dst_reader,
                           struct NETIO_Address *via_address,
                           struct NETIORouteProperty *property,
                           RTI_BOOL *route_existed,
                           RTI_BOOL *found_route)
{
    return NETIO_RouteResolver_route_update(
                  r_table,src_intf,dst_reader,via_address,property,
                  route_existed, NETIO_ROUTE_OPERATION_ADD,found_route,NULL);
}

RTI_BOOL
NETIO_RouteResolver_add_peer(NETIO_RouteResolver_T *r_table,
                           NETIO_Interface_T *src_intf,
                           struct NETIO_Address *dst_reader,
                           NETIO_RouteKind_T peer_kind,
                           const char *peer_address,
                           struct NETIORouteProperty *property,
                           RTI_BOOL *route_existed,
                           RTI_BOOL *found_route)
{
    NETIO_AddressResolveContext_T context;
    RTI_BOOL parse_error;
    struct NETIO_Address address;
    NETIO_Interface_T *netio_intf;
    RT_ComponentFactoryId_T id;
    RTI_BOOL rval = RTI_TRUE;
    RTI_BOOL added_route = RTI_FALSE;
    RTI_BOOL local_found_route = RTI_FALSE;

    NETIO_AddressResolveContext_init(&context);

    if (!NETIO_AddressResolver_resolve(r_table->nar,-1,peer_kind,
                                       peer_address,&context))
    {
        return RTI_FALSE;
    }

    while (NETIO_AddressResolver_get_next(r_table->nar,&netio_intf,
                                         &id,&address,&context,&parse_error))
    {
        if (!NETIO_RouteResolver_route_update(r_table,src_intf,dst_reader,
                                              &address,property,route_existed,
                                              NETIO_ROUTE_OPERATION_ADD_PEER,
                                              &local_found_route, NULL))
        {
            /* return FALSE if at least one call to add_route failed */
            rval = RTI_FALSE;
        }
        else if (local_found_route)
        {
            /* How many is not important, just that at least one was added */
            added_route = RTI_TRUE;
        }
       /*
        else
        {
             Call succeeded, but local_found_route is FALSE. If all calls
             to add_route succeed, but local_found_route is never TRUE, then
             added_route remains false and rval is set to false later.
        }
        */
    }

    if (parse_error || !added_route)
    {
        rval = RTI_FALSE;
    }

    if (found_route != NULL)
    {
        /* if the function returns TRUE, then all routes must have been added,
         * if the function returns FALSE, at least one route was added.
         */
        *found_route = added_route;
    }

    return rval;
}

RTI_BOOL
NETIO_RouteResolver_delete_route(NETIO_RouteResolver_T *r_table,
                              NETIO_Interface_T *src_intf,
                              struct NETIO_Address *dst_reader,
                              struct NETIO_Address *via_address,
                              RTI_BOOL *route_existed,
                              RTI_BOOL *found_route)
{
    return NETIO_RouteResolver_route_update(
                  r_table,src_intf,dst_reader,via_address,NULL,
                  route_existed,NETIO_ROUTE_OPERATION_DELETE,found_route,NULL);
}

#ifndef RTI_CERT
RTI_BOOL
NETIO_RouteResolver_delete_peer(NETIO_RouteResolver_T *r_table,
                           NETIO_Interface_T *src_intf,
                           struct NETIO_Address *dst_reader,
                           NETIO_RouteKind_T peer_kind,
                           const char *peer_address,
                           RTI_BOOL *route_existed,
                           RTI_BOOL *found_route)
{
    NETIO_AddressResolveContext_T context;
    RTI_BOOL parse_error;
    struct NETIO_Address address;
    NETIO_Interface_T *netio_intf;
    RT_ComponentFactoryId_T id;
    RTI_BOOL rval = RTI_TRUE;

    NETIO_AddressResolveContext_init(&context);

    if (!NETIO_AddressResolver_resolve(r_table->nar,-1,peer_kind,
                                       peer_address,&context))
    {
        return RTI_FALSE;
    }

    while (NETIO_AddressResolver_get_next(r_table->nar,&netio_intf,
                                         &id,&address,&context,&parse_error))
    {
        if (!NETIO_RouteResolver_delete_route(
                      r_table,src_intf,dst_reader,&address,route_existed,
                      found_route))
        {
            rval = RTI_FALSE;
        }
    }

    if (parse_error)
    {
        return RTI_FALSE;
    }

    return rval;
}
#endif /* !RTI_CERT */

/*ci @} */
