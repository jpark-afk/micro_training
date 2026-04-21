/*
 * FILE: NETIOBindResolver.c - NETIO Bind implementation
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
 * 17sep2021,tk MICRO-3276/PR.29664
 * - If a locator cannot be resolved or reserved by any transport in
 *   NETIO_BindResolver_reserve_addresses, release addresses already
 *   reserved (within the call), reset the input unicast
 *   and multicast sequence to the original length and return failure.
 * 29jun2015,tk MICRO-1358/PR#15173 Added/Updated comments
 * 16mar2015,tk MICRO-1127/PR#14270 Added comments
 * 10dec2014,tk MICRO-978/PR#12914  Consistently return records on failure
 * 31jul2014,tk MICRO-241/PR#1413   Removed superfluous DB fields
 * 31jul2014,tk MICRO-842/PR#9683   Removed superfluous paramaters in DB
 *                                  compare function
 * 05may2014,tk MICRO-72            Updated based on CR-232
 * 16sep2012,tk Written
 */
/*ci
 * \file
 * \brief NETIO Bind implementation
 *
 * \details
 * This file implements the bind resolver. A bind resolver takes a address
 * to listen to and finds an interface capable of listen to the specified
 * address. It also supports reserving and releasing address on a NETIO
 * interface.
 *
 * \addtogroup NETIO_BindClass
 * @{
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
#ifndef netio_address_h
#include "netio/netio_address.h"
#endif
#ifndef netio_route_h
#include "netio/netio_route.h"
#endif

/*ci
 * \brief The BindResolver class
 */
struct NETIO_BindResolver
{
    /*ci
     * \brief The properties the resolver was created with
     */
    struct NETIO_BindResolverProperty property;

    /*ci
     * \brief Table to keep track of which interface can listen to what
     */
    DB_Table_T route_table;

    /*ci
     * \brief The database to create tables in
     */
    DB_Database_T db;

    /*ci
     * \brief The address resolver to use
     */
    NETIO_AddressResolver_T *nar;
};

/*ci
 * \brief The key for an entry in the bind table
 */
struct NETIO_RouteEntryKey
{
    /*ci
     * \brief The type of route
     */
    NETIO_RouteKind_T kind;

    /*ci
     * \brief The component factory id which can handle the route
     */
    RT_ComponentFactoryId_T id;

    /*ci
     * \brief The address which can be bound/listened to
     */
    struct NETIO_Address address;
};

/*ci
 * \brief Entry in the bind table
 */
struct NETIO_RouteEntry
{
    /*ci
     * \brief The key for the entry as defined in \ref NETIO_RouteEntryKey
     */
    struct NETIO_RouteEntryKey key;

    /*ci
     * \brief The interface which to bind to to listen to the address specified
     *        in key
     */
    NETIO_Interface_T *intf;

#ifndef RTI_CERT
    /*ci
     * \brief The number of binds to this route.
     */
    RTI_UINT32 ref_count;
#endif
};

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Compare entries in the table of bind entries. The function is
 *        compatible with \ref DB_IndexCompare_T
 *
 * \param[in] flags Used to distinguish between op2 being a key or record
 * \param[in] op1   A NETIO_RouteEntry already in the database
 * \param[in] op2   Either a NETIO_RouteEntry being added or a key
 *                  being searched for
 *
 * \return positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2
 *         zero if op1 is equal to op2
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_INT32
NETIO_BindResolver_compare_entry(RTI_INT32 flags,
                                 const DB_Record_T op1, void *op2)
{
    struct NETIO_RouteEntry *record_left = (struct NETIO_RouteEntry*)op1;
    const struct NETIO_RouteEntryKey *id_right;
    RTI_INT32 i;

    if (DB_SELECT_OP2_IS_KEY(flags))
    {
        id_right = (const struct NETIO_RouteEntryKey *)op2;
    }
    else
    {
        id_right = &((struct NETIO_RouteEntry*)op2)->key;
    }

    if (record_left->key.kind > id_right->kind)
    {
        return 1;
    }

    if (record_left->key.kind < id_right->kind)
    {
        return -1;
    }

    if (record_left->key.id._value._high > id_right->id._value._high)
    {
        return 1;
    }

    if (record_left->key.id._value._high < id_right->id._value._high)
    {
        return -1;
    }

    if (record_left->key.id._value._low > id_right->id._value._low)
    {
        return 1;
    }

    if (record_left->key.id._value._low < id_right->id._value._low)
    {
        return -1;
    }

    if (record_left->key.address.port > id_right->address.port)
    {
        return 1;
    }

    if (record_left->key.address.port < id_right->address.port)
    {
        return -1;
    }

    for (i = 0; i < NETIO_ADDRESS_MAX_32BIT; ++i)
    {
        if (record_left->key.address.value.as_uint32.value[i] >
            id_right->address.value.as_uint32.value[i])
        {
            return 1;
        }

        if (record_left->key.address.value.as_uint32.value[i] <
            id_right->address.value.as_uint32.value[i])
        {
            return -1;
        }
    }

    return 0;
}

NETIO_BindResolver_T*
NETIO_BindResolver_new(DB_Database_T db,
                       NETIO_AddressResolver_T *nar,
                       const char* const name,
                       struct NETIO_BindResolverProperty *const property)
{
    NETIO_BindResolver_T *rtable = NULL;
    NETIO_BindResolver_T *rval = NULL;
    struct DB_TableProperty tbl_property = DB_TableProperty_INITIALIZER;
    DB_ReturnCode_T dbrc;

    OSAPI_PRECONDITION((db == NULL) || (name == NULL) ||
                           (property == NULL) || (nar == NULL),
                           return NULL,
                           OSAPI_Log_entry_add_pointer("db",db,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("name",name,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("property",property,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("nar",nar,RTI_TRUE);)

    OSAPI_Heap_allocate_struct(&rtable,struct NETIO_BindResolver);
    if (rtable == NULL)
    {
        NETIO_LOG_BIND_NEW(OSAPI_LOGKIND_ERROR)
        return NULL;
    }

    rtable->property = *property;
    rtable->db = db;
    rtable->nar = nar;
    tbl_property.max_cursors = 1;
    tbl_property.max_indices = 1;
    tbl_property.max_records = rtable->property.max_routes;

    dbrc = DB_Database_create_table(&rtable->route_table,
            db,name,sizeof(struct NETIO_RouteEntry),
            NETIO_BindResolver_compare_entry,&tbl_property);

    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_LOG_BIND_NEW_RTABLE(OSAPI_LOGKIND_ERROR,dbrc)
        goto done;
    }

    rval = rtable;

done:
    return rval;
}

#ifndef RTI_CERT
RTI_BOOL
NETIO_BindResolver_delete(NETIO_BindResolver_T* r_table)
{
    DB_ReturnCode_T dbrc;

    OSAPI_PRECONDITION((r_table == NULL),return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("r_table",r_table,RTI_TRUE);)

    dbrc = DB_Database_delete_table(r_table->db,r_table->route_table);
    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_LOG_BIND_DELETE_RTABLE(OSAPI_LOGKIND_ERROR,dbrc)
        return RTI_FALSE;
    }

    OSAPI_Heap_free_struct(r_table);

    return RTI_TRUE;
}
#endif /* !RTI_CERT */

RTI_BOOL
NETIO_BindResolver_add_route(NETIO_BindResolver_T *r_table,
                           NETIO_RouteKind_T kind,
                           RT_ComponentFactoryId_T *id,
                           struct NETIO_Address *address,
                           NETIO_Interface_T *intf,
                           RTI_BOOL *exists)
{
    struct NETIO_RouteEntry *r_entry = NULL;
    DB_ReturnCode_T dbrc;
    RTI_BOOL rval = RTI_FALSE;
    struct NETIO_RouteEntryKey key;

    OSAPI_PRECONDITION((r_table == NULL) || (id == NULL) ||
                           (address == NULL) || (intf == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("r_table",r_table,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("id",id,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("address",address,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("intf",intf,RTI_TRUE);)

    if (exists)
    {
        *exists = RTI_FALSE;
    }

    RT_ComponentFactoryId_clear(&key.id);
    key.id = *id;
    key.address = *address;
    key.kind = kind;

    dbrc = DB_Table_select_match(r_table->route_table,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*) &r_entry,
                                 (DB_Key_T) &key);

    if (dbrc == DB_RETCODE_OK)
    {
        if (exists)
        {
            *exists = RTI_TRUE;
        }
#ifndef RTI_CERT
        /* If the route already exists increase reference count */
        ++r_entry->ref_count;
#endif

        rval = RTI_TRUE;
        goto done;
    }

    dbrc = DB_Table_create_record(r_table->route_table,(DB_Record_T*)&r_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_LOG_BIND_CREATE_RTABLE(OSAPI_LOGKIND_ERROR,dbrc)
        goto done;
    }

    r_entry->key = key;
    r_entry->intf = intf;
#ifndef RTI_CERT
    r_entry->ref_count = 1;
#endif

    dbrc = DB_Table_insert_record(r_table->route_table,(DB_Record_T)r_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_LOG_BIND_CREATE_RTABLE(OSAPI_LOGKIND_ERROR,dbrc)
        (void)DB_Table_delete_record(r_table->route_table,(DB_Record_T)r_entry);
        goto done;
    }

    OSAPI_TRACE_NET("added bind route failed to add anon route:",RTI_FALSE)
    OSAPI_TRACE_INT32("kind",kind,RTI_FALSE)
    OSAPI_TRACE_STRING("id",key.id._name._name,RTI_FALSE)
    OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(&key.address),RTI_FALSE)
    OSAPI_TRACE_INT32("port",key.address.port,RTI_FALSE)
    OSAPI_TRACE_INT32("address",key.address.value.ipv4.address,RTI_TRUE)

    rval = RTI_TRUE;

done:
    return rval;
}


RTI_BOOL
NETIO_BindResolver_delete_route(NETIO_BindResolver_T *r_table,
                              NETIO_RouteKind_T kind,
                              RT_ComponentFactoryId_T *id,
                              struct NETIO_Address *address,
                              RTI_BOOL *existed)
{
    struct NETIO_RouteEntry *r_entry = NULL;
    DB_ReturnCode_T dbrc;
    RTI_BOOL rval = RTI_FALSE;
    struct NETIO_RouteEntryKey key;

    OSAPI_PRECONDITION((r_table == NULL) || (id == NULL) || (address == NULL),
                            return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("r_table",r_table,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("id",id,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("address",address,RTI_TRUE);)

    if (existed)
    {
        *existed = RTI_FALSE;
    }

    RT_ComponentFactoryId_clear(&key.id);
    key.id = *id;
    key.address = *address;
    key.kind = kind;

    OSAPI_TRACE_NET("delete bind route:",RTI_FALSE)
    OSAPI_TRACE_INT32("kind",kind,RTI_FALSE)
    OSAPI_TRACE_STRING("id",key.id._name._name,RTI_FALSE)
    OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(&key.address),RTI_FALSE)
    OSAPI_TRACE_INT32("port",key.address.port,RTI_FALSE)
    OSAPI_TRACE_INT32("address",key.address.value.ipv4.address,RTI_TRUE)

#ifdef RTI_CERT
    dbrc = DB_Table_remove_record(r_table->route_table,
                                 (DB_Record_T*)&r_entry,(DB_Key_T)&key);
#else
    dbrc = DB_Table_select_match(r_table->route_table,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*) &r_entry,
                                 (DB_Key_T) &key);
#endif
    if (dbrc == DB_RETCODE_NO_DATA)
    {
        if (existed)
        {
            *existed = RTI_FALSE;
        }
        rval = RTI_TRUE;
        goto done;
    }

    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_LOG_BIND_DELETE_RTABLE(OSAPI_LOGKIND_ERROR,dbrc)
        goto done;
    }

    if (existed)
    {
        *existed = RTI_TRUE;
    }

#ifndef RTI_CERT
    --r_entry->ref_count;

    /* Do not delete the route if there are outstanding references */
    if (r_entry->ref_count > 0)
    {
        rval = RTI_TRUE;
        goto done;
    }

    r_entry = NULL;
    dbrc = DB_Table_remove_record(r_table->route_table,
                                 (DB_Record_T*) &r_entry,
                                 (DB_Key_T) &key);
    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_LOG_BIND_DELETE_RTABLE(OSAPI_LOGKIND_ERROR,dbrc)
        goto done;
    }
#endif

    dbrc = DB_Table_delete_record(r_table->route_table,(DB_Record_T)r_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_LOG_BIND_DELETE_RTABLE(OSAPI_LOGKIND_ERROR,dbrc)
        goto done;
    }

    OSAPI_TRACE_NET("deleted bind route:",RTI_FALSE)
    OSAPI_TRACE_INT32("kind",kind,RTI_FALSE)
    OSAPI_TRACE_STRING("id",key.id._name._name,RTI_FALSE)
    OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(&key.address),RTI_FALSE)
    OSAPI_TRACE_INT32("port",key.address.port,RTI_FALSE)
    OSAPI_TRACE_INT32("address",key.address.value.ipv4.address,RTI_TRUE)

    rval = RTI_TRUE;

done:
    return rval;
}


RTI_BOOL
NETIO_BindResolver_lookup_by_address(NETIO_BindResolver_T* r_table,
                                      NETIO_RouteKind_T kind,
                                      RT_ComponentFactoryId_T *id,
                                      struct NETIO_Address *address,
                                      NETIO_Interface_T **intf)
{
    struct NETIO_RouteEntry *r_entry = NULL;
    DB_ReturnCode_T dbrc;
    RTI_BOOL rval = RTI_FALSE;
    struct NETIO_RouteEntryKey key;

    key.id = *id;
    key.address = *address;
    key.kind = kind;

    OSAPI_TRACE_NET("lookup bind route :",RTI_FALSE)
    OSAPI_TRACE_INT32("kind",kind,RTI_FALSE)
    OSAPI_TRACE_STRING("id",key.id._name._name,RTI_FALSE)
    OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(&key.address),RTI_FALSE)
    OSAPI_TRACE_INT32("port",key.address.port,RTI_FALSE)
    OSAPI_TRACE_INT32("address",key.address.value.ipv4.address,RTI_TRUE)

    dbrc = DB_Table_select_match(r_table->route_table,
            DB_TABLE_DEFAULT_INDEX,(DB_Record_T*)&r_entry,(DB_Key_T)&key);

    if (dbrc != DB_RETCODE_OK)
    {
        OSAPI_TRACE_NET("lookup bind failed :",RTI_FALSE)
        OSAPI_TRACE_INT32("kind",kind,RTI_FALSE)
        OSAPI_TRACE_STRING("id",key.id._name._name,RTI_FALSE)
        OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(&key.address),RTI_FALSE)
        OSAPI_TRACE_INT32("port",key.address.port,RTI_FALSE)
        OSAPI_TRACE_INT32("address",key.address.value.ipv4.address,RTI_TRUE)
        rval = RTI_FALSE;
        goto done;
    }

    *intf = r_entry->intf;
    rval = RTI_TRUE;

done:
    return rval;
}

RTI_BOOL
NETIO_BindResolver_reserve_addresses(NETIO_BindResolver_T* r_table,
                                     RTI_INT32 try_index,
                                     struct REDA_StringSeq *transports,
                                     struct REDA_StringSeq *locators,
                                     NETIO_RouteKind_T kind,
                                     struct NETIO_AddressSeq *mc_locator_seq,
                                     struct NETIO_AddressSeq *uc_locator_seq)
{
    struct NETIO_AddressSeq candidate_address = NETIO_AddressSeq_INITIALIZER;
    NETIO_Interface_T *netio_intf;
    RTI_INT32 i,k,l;
    RTI_BOOL retval = RTI_FALSE;
    RTI_INT32 mc_len;
    RTI_INT32 uc_len;
    RTI_INT32 orig_mc_len;
    RTI_INT32 orig_uc_len;
    RTI_INT32 max_len;
    RT_ComponentFactoryId_T id;
    NETIO_AddressResolveContext_T context;
    RTI_BOOL parse_error;
    struct NETIO_Address address;
    RTI_INT32 c_index;
    struct NETIO_AddressSeq reserved_address = NETIO_AddressSeq_INITIALIZER;
    RTI_BOOL bretval = RTI_FALSE;

    /* Save original length in case there is a failure and the sequences
     * are reverted their original state.
     */
    orig_mc_len = NETIO_AddressSeq_get_length(mc_locator_seq);
    orig_uc_len = NETIO_AddressSeq_get_length(uc_locator_seq);

    mc_len = NETIO_AddressSeq_get_maximum(mc_locator_seq) - NETIO_AddressSeq_get_length(mc_locator_seq);
    uc_len = NETIO_AddressSeq_get_maximum(uc_locator_seq) - NETIO_AddressSeq_get_length(uc_locator_seq);
    max_len = mc_len + uc_len;

    if (max_len == 0)
    {
        return RTI_TRUE;
    }

    if (!NETIO_AddressSeq_set_maximum(&reserved_address,max_len))
    {
        NETIO_LOG_BIND_SET_MAX(OSAPI_LOGKIND_ERROR,max_len)
        return RTI_FALSE;
    }

    if (!NETIO_AddressSeq_set_maximum(&candidate_address,max_len))
    {
        NETIO_LOG_BIND_SET_MAX(OSAPI_LOGKIND_ERROR,max_len)
        return RTI_FALSE;
    }

    l = 0;
    for (i = 0; i < REDA_StringSeq_get_length(locators); ++i)
    {
        if (!NETIO_AddressSeq_set_length(&candidate_address,0))
        {
            NETIO_LOG_BIND_SET_LENGTH(OSAPI_LOGKIND_ERROR,0)
            goto done;
        }

        NETIO_AddressResolveContext_init(&context);

        if (!NETIO_AddressResolver_resolve(r_table->nar,try_index,kind,
                                    *REDA_StringSeq_get_reference(locators,i),
                                    &context))
        {
            OSAPI_TRACE_NET("could not resolve address, transport not registered",RTI_FALSE)
            OSAPI_TRACE_STRING("id",*REDA_StringSeq_get_reference(locators,i),RTI_TRUE)
            goto done;
        }

        c_index = 0;

        mc_len = NETIO_AddressSeq_get_maximum(mc_locator_seq);
        uc_len = NETIO_AddressSeq_get_maximum(uc_locator_seq);
        while (c_index < (max_len - l))
        {
            if (!NETIO_AddressResolver_get_next(r_table->nar,&netio_intf,
                                           &id,&address,&context,&parse_error))
            {
                if (parse_error)
                {
                    /* If there was a parse error release all previously
                     * reserved addresses, if any.
                     */
                    goto done;
                }
                break;
            }

            /* If there is no space for multicast (as determined by the caller)
             * ignore the address and do not add it as a candidate
             */
            if ((mc_len == 0) && NETIO_Address_is_multicast(&address))
            {
                OSAPI_TRACE_NET("ignoring multi-cast address: ",RTI_FALSE)
                OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(&address),RTI_FALSE)
                OSAPI_TRACE_INT32("port",address.port,RTI_FALSE)
                OSAPI_TRACE_INT32("address",address.value.ipv4.address,RTI_TRUE)
                continue;
            }

            /* If there is no space for unicast (as determined by the caller)
             * ignore the address and do not add it as a candidate
             */
            if ((uc_len == 0) && !NETIO_Address_is_multicast(&address))
            {
                OSAPI_TRACE_NET("ignoring uni-cast address: ",RTI_FALSE)
                OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(&address),RTI_FALSE)
                OSAPI_TRACE_INT32("port",address.port,RTI_FALSE)
                OSAPI_TRACE_INT32("address",address.value.ipv4.address,RTI_TRUE)
                continue;
            }

            c_index++;

            if (!NETIO_AddressSeq_set_length(&candidate_address,c_index))
            {
                NETIO_LOG_BIND_SET_LENGTH(OSAPI_LOGKIND_ERROR,0)
                /* This is not expected, release all previously reserved
                 * addresses.
                 */
                goto done;
            }

            *NETIO_AddressSeq_get_reference(&candidate_address,c_index-1) =
                                                                    address;
        }

        if (c_index == 0)
        {
            /* If the candidate list is empty it means that the address was not
             * resolved by any available locator. This is considered an
             * error. Release all previously reserved addresses.
             */
            goto done;
        }

        if (!NETIO_Interface_reserve_address(netio_intf,
                                &candidate_address,&reserved_address,NULL))
        {
            /* Failed to reserve addresses on the locator. Release all
             * previously reserved addresses, if any.
             */
            goto done;
        }

        /* Add the resolved entry to the resource table
         */
        mc_len = NETIO_AddressSeq_get_length(mc_locator_seq);
        uc_len = NETIO_AddressSeq_get_length(uc_locator_seq);
        for (k = l; k < NETIO_AddressSeq_get_length(&reserved_address); ++k,l++)
        {
            if (!NETIO_BindResolver_add_route(r_table,
                    kind,
                    &id,
                    NETIO_AddressSeq_get_reference(&reserved_address,k),
                    netio_intf,NULL))
            {
                /* A reserved address could not be added to be listened too.
                 * This is an error, release all previsouly reserved addresses.
                 */
                goto done;
            }

            if (NETIO_Address_is_internal(
                        NETIO_AddressSeq_get_reference(&reserved_address,k)))
            {
                continue;
            }

            if (NETIO_Address_is_multicast(
                        NETIO_AddressSeq_get_reference(&reserved_address,k)))
            {
                if (NETIO_AddressSeq_set_length(mc_locator_seq,mc_len+1))
                {
                    *NETIO_AddressSeq_get_reference(mc_locator_seq, mc_len) =
                        *NETIO_AddressSeq_get_reference(&reserved_address,k);
                    ++mc_len;
                }
            }
            else
            {
                if (NETIO_AddressSeq_set_length(uc_locator_seq,uc_len+1))
                {
                    *NETIO_AddressSeq_get_reference(uc_locator_seq, uc_len) =
                            *NETIO_AddressSeq_get_reference(&reserved_address,k);
                    ++uc_len;
                }
            }
        }
    }

    retval = RTI_TRUE;

done:

    if (!retval)
    {
        bretval = NETIO_BindResolver_release_addresses(r_table,
                                                       transports,
                                                       kind,
                                                       &reserved_address);
#if OSAPI_ENABLE_LOG
            /* It is not considered an error if the port cannot be
             * released since it is unknown why the reservation
             * failed in the first place.
             */
            if (!bretval)
            {
                OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_WARNING)
            }
#else
            IGNORE_RETVAL(bretval);
#endif

        if (!NETIO_AddressSeq_set_length(mc_locator_seq,orig_mc_len))
        {
            return RTI_FALSE;
        }

        if (!NETIO_AddressSeq_set_length(uc_locator_seq,orig_uc_len))
        {
            return RTI_FALSE;
        }
    }

#ifndef RTI_CERT
    if (!NETIO_AddressSeq_finalize(&candidate_address))
    {
        return RTI_FALSE;
    }

    if (!NETIO_AddressSeq_finalize(&reserved_address))
    {
        return RTI_FALSE;
    }
#endif /* !RTI_CERT */

    return retval;
}


RTI_BOOL
NETIO_BindResolver_release_addresses(NETIO_BindResolver_T* r_table,
                                      struct REDA_StringSeq *transport,
                                      NETIO_RouteKind_T kind,
                                      struct NETIO_AddressSeq *locator_seq)
{
    struct NETIO_Address netio_address;
    NETIO_Interface_T *netio_intf;
    RTI_INT32 i,j;
    RTI_BOOL retval = RTI_FALSE;
    RTI_INT32 len;
    RT_ComponentFactoryId_T id;

    len = NETIO_AddressSeq_get_length(locator_seq);
    for (i = 0; i < len; ++i)
    {
        netio_address = *NETIO_AddressSeq_get_reference(locator_seq,i);
        for (j = 0; j < REDA_StringSeq_get_length(transport); ++j)
        {
            if (!RT_ComponentFactoryId_set_name(&id,*REDA_StringSeq_get_reference(transport,j)))
            {
                NETIO_LOG_SET_NAME(OSAPI_LOGKIND_WARNING,
                                   *REDA_StringSeq_get_reference(transport,j))
                continue;
            }

            if (NETIO_BindResolver_lookup_by_address(r_table,kind,&id,&netio_address,&netio_intf))
            {
                retval = NETIO_Interface_release_address(netio_intf,&netio_address);
#if OSAPI_ENABLE_LOG
                if (!retval)
                {
                    OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_WARNING)
                }
#else
                IGNORE_RETVAL(retval);
#endif
                retval = NETIO_BindResolver_delete_route(r_table,kind,&id,&netio_address,NULL);
#if OSAPI_ENABLE_LOG
                if (!retval)
                {
                    OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_WARNING)
                }
#else
                IGNORE_RETVAL(retval);
#endif
            }
        }
    }

    retval = RTI_TRUE;

    return retval;
}


RTI_BOOL
NETIO_BindResolver_bind(NETIO_BindResolver_T* r_table,
                        struct REDA_StringSeq *transport,
                        NETIO_RouteKind_T kind,
                        struct NETIO_Address *from_address,
                        NETIO_Interface_T *to_intf,
                        struct NETIO_Address *to_address,
                        RTI_BOOL *route_existed,
                        RTI_BOOL *found_route)
{
    NETIO_Interface_T *netio_intf;
    RTI_INT32 k;
    RT_ComponentFactoryId_T id;
    RTI_BOOL brc = RTI_FALSE;
    RTI_BOOL local_route_existed;

    if (found_route)
    {
        *found_route = RTI_FALSE;
    }

    for (k = 0; k < REDA_StringSeq_get_length(transport); ++k)
    {
        if (!RT_ComponentFactoryId_set_name(&id,*REDA_StringSeq_get_reference(transport,k)))
        {
            NETIO_LOG_SET_NAME(OSAPI_LOGKIND_WARNING,
                               *REDA_StringSeq_get_reference(transport,k))
            continue;
        }

        if (!NETIO_BindResolver_lookup_by_address(r_table,kind,&id,from_address,&netio_intf))
        {
            continue;
        }

        OSAPI_TRACE_NET("create route:",RTI_FALSE)
        OSAPI_TRACE_INT32("kind",kind,RTI_FALSE)
        OSAPI_TRACE_INT32("from.kind",from_address->kind,RTI_FALSE)
        OSAPI_TRACE_INT32("from.port",from_address->port,RTI_FALSE)
        OSAPI_TRACE_GUID("from.address",&from_address->value.rtps_guid,RTI_FALSE)
        OSAPI_TRACE_INT32("to.kind",to_address->kind,RTI_FALSE)
        OSAPI_TRACE_INT32("to.port",to_address->port,RTI_FALSE)
        OSAPI_TRACE_GUID("to.address",&to_address->value.rtps_guid,RTI_TRUE)

        if (!NETIO_Interface_bind_external(
                netio_intf,from_address,to_intf,to_address,
                NULL,&local_route_existed))
        {
            NETIO_LOG_BIND_EXTERNAL_FAILED(OSAPI_LOGKIND_ERROR)
            goto done;
        }

        if (found_route)
        {
            *found_route = RTI_TRUE;
        }

        if (route_existed)
        {
            *route_existed = local_route_existed;
        }
    }

    brc = RTI_TRUE;

done:
    return brc;
}

RTI_BOOL
NETIO_BindResolver_unbind(NETIO_BindResolver_T* r_table,
                          struct REDA_StringSeq *transport,
                          NETIO_RouteKind_T kind,
                          struct NETIO_Address *from_address,
                          NETIO_Interface_T *to_intf,
                          struct NETIO_Address *to_address,
                          RTI_BOOL *route_existed,
                          RTI_BOOL *found_route)
{
    NETIO_Interface_T *netio_intf;
    RTI_INT32 k;
    RT_ComponentFactoryId_T id;
    RTI_BOOL brc = RTI_FALSE;
    RTI_BOOL local_route;

    if (found_route)
    {
        *found_route = RTI_FALSE;
    }

    for (k = 0; k < REDA_StringSeq_get_length(transport); ++k)
    {
        if (!RT_ComponentFactoryId_set_name(&id,*REDA_StringSeq_get_reference(transport,k)))
        {
            NETIO_LOG_SET_NAME(OSAPI_LOGKIND_WARNING,
                               *REDA_StringSeq_get_reference(transport,k))
            continue;
        }

        if (!NETIO_BindResolver_lookup_by_address(r_table,kind,&id,from_address,&netio_intf))
        {
            continue;
        }

        OSAPI_TRACE_NET("delete route:",RTI_FALSE)
        OSAPI_TRACE_INT32("kind",kind,RTI_FALSE)
        OSAPI_TRACE_INT32("from.kind",from_address->kind,RTI_FALSE)
        OSAPI_TRACE_INT32("from.port",from_address->port,RTI_FALSE)
        OSAPI_TRACE_GUID("from.address",&from_address->value.rtps_guid,RTI_FALSE)
        OSAPI_TRACE_INT32("to.kind",to_address->kind,RTI_FALSE)
        OSAPI_TRACE_INT32("to.port",to_address->port,RTI_FALSE)
        OSAPI_TRACE_GUID("to.address",&to_address->value.rtps_guid,RTI_TRUE)

        if (!NETIO_Interface_unbind_external(
                netio_intf,from_address,to_intf,to_address,&local_route))
        {
            NETIO_LOG_UNBIND_EXTERNAL_FAILED(OSAPI_LOGKIND_ERROR)
            goto done;
        }

        if (found_route && local_route)
        {
            *found_route = RTI_TRUE;
        }

        if (route_existed)
        {
            *route_existed = local_route;
        }
    }

    brc = RTI_TRUE;

done:

    return brc;
}

/*ci @} */

