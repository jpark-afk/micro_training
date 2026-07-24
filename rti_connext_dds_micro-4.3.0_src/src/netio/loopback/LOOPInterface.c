/*
 * FILE: LOOPInterface.c - LOOPBack interface implementation
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
 * 31jul2015,tk MICRO-1482/PR#_____ Reject all but the "" empty address
 * 09jul2015,tk MICRO-1341/PR#15121 Corrected comments
 * 29jun2015,tk MICRO-1341/PR#15121 Corrected function comment for set_state()
 * 12mar2015,tk MICRO-1105/PR#14180 Check for NULL return value
 * 25feb2015,tk MICRO-1087/PR#14073 Correctly return RTI_TRUE in set_state
 * 10dec2014,tk MICRO-978/PR#12914 Consistently return records on failure
 * 31jul2014,tk MICRO-241/PR#1413 - Removed superfluous DB fields
 * 05may2014,tk MICRO-72 - Updated based on CR-232
 * 20mar2013,tk Updated
 * 07jul2012,tk Updated
 * 07jul2012,tk Updated
 * 27apr2012,tk Written
 */
/*ci
 * \file
 * \brief LOOPBack interface implementation
 *
 * \details
 * This file implements a loopback interface. All data sent on this interface
 * is automatically looped back to all listening interfaces. Data sent on this
 * interface is never passed downstream. It is important to understand that this
 * interface is synchronous. All data sent on this interface is looped back
 * to all bound interfaces in the same calling context.
 */
/*ci \addtogroup NETIO_LoopbackInterfaceClass
 * @{
 */
#include "osapi/osapi_config.h"

#ifndef osapi_string_h
#include "osapi/osapi_string.h"
#endif
#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
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
#ifndef netio_loopback_h
#include "netio/netio_loopback.h"
#endif

#include "LOOPInterface.h"

LINK_SECTION_BSS_SDRAM
RTI_PRIVATE struct NETIO_InterfaceI LOOP_Interface_fv_Intf;

/*** SOURCE_BEGIN ***/

#ifndef RTI_CERT
/*ci
 * \brief Finalize a loopback interface instance
 *
 * \param[in] netio_intf Interface to finalize
 *
 * \sa \ref LOOP_Interface_initialize
 */
RTI_PRIVATE void
LOOP_Interface_finalize(struct LOOP_Interface *netio_intf)
{
    struct NETIORouteEntry *route = NULL;
    struct NETIOBindEntry *bind = NULL;
    DB_Cursor_T cursor = NULL;
    DB_ReturnCode_T dbrc;

    NETIO_Interface_finalize(&netio_intf->_parent);

    dbrc = DB_Table_select_all_default(netio_intf->_parent._rtable,&cursor);
    if (dbrc == DB_RETCODE_OK)
    {
        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&route);
        while (dbrc == DB_RETCODE_OK)
        {
            dbrc = DB_Table_delete_record(
                    netio_intf->_parent._rtable,(DB_Record_T)route);
#if OSAPI_ENABLE_LOG
            if (dbrc != DB_RETCODE_OK)
            {
                NETIO_LOG_LOOP_DELETE_TABLE(OSAPI_LOGKIND_ERROR,dbrc)
            }
#endif
            dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&route);
        }
        DB_Cursor_finish(netio_intf->_parent._rtable,cursor);
    }

    cursor = NULL;
    dbrc = DB_Table_select_all_default(netio_intf->_parent._btable,&cursor);
    if (dbrc == DB_RETCODE_OK)
    {
        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&bind);
        while (dbrc == DB_RETCODE_OK)
        {
            dbrc = DB_Table_delete_record(netio_intf->_parent._btable,
                                          (DB_Record_T)bind);
#if OSAPI_ENABLE_LOG
            if (dbrc != DB_RETCODE_OK)
            {
                NETIO_LOG_LOOP_DELETE_TABLE(OSAPI_LOGKIND_ERROR,dbrc)
            }
#endif
            dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&bind);
        }
        DB_Cursor_finish(netio_intf->_parent._btable,cursor);
    }

    dbrc = DB_Database_delete_table(netio_intf->property._parent._parent.db,
                                    netio_intf->_parent._rtable);

#if OSAPI_ENABLE_LOG
    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_LOG_LOOP_DELETE_TABLE(OSAPI_LOGKIND_ERROR,dbrc)
    }
#else
    IGNORE_RETVAL(dbrc);
#endif

    dbrc = DB_Database_delete_table(netio_intf->property._parent._parent.db,
                                    netio_intf->_parent._btable);
#if OSAPI_ENABLE_LOG
    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_LOG_LOOP_DELETE_TABLE(OSAPI_LOGKIND_ERROR,dbrc)
    }
#else
    IGNORE_RETVAL(dbrc);
#endif
}
#endif

#ifndef RTI_CERT
/*ci
 * \brief Delete a loopback interface instance
 *
 * \param[in] netio_intf Interface to delete
 *
 * \sa \ref LOOP_Interface_create
 */
RTI_PRIVATE void
LOOP_Interface_delete(struct LOOP_Interface *netio_intf)
{
    LOOP_Interface_finalize(netio_intf);
    OSAPI_Heap_free_struct(netio_intf);
}
#endif /* RTI_CERT */

/*ci
 * \brief Initialize a loopback interface instance
 *
 * \param[in] test_intf Interface to initialize
 * \param[in] factory   Factory that is creating the instance
 * \param[in] property  The property of the new loopback interface
 * \param[in] listener  The listener for the new loopback interface
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref LOOP_Interface_finalize
 *
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
LOOP_Interface_initialize(struct LOOP_Interface *test_intf,
                          struct LOOP_InterfaceFactory *factory,
                          const struct LOOP_InterfaceProperty *const property,
                          const struct NETIO_InterfaceListener *const listener)
{
    struct DB_TableProperty tbl_property = DB_TableProperty_INITIALIZER;
    char tbl_name[NETIO_TABLE_NAME_SIZE];
    union RT_ComponentFactoryId id;
    DB_ReturnCode_T dbrc;

    if (property == NULL)
    {
        NETIO_LOG_LOOP_INVALID_PROPERTY(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    test_intf->property = *property;

    if (!NETIO_Interface_initialize(&test_intf->_parent,
                                    &LOOP_Interface_fv_Intf,
                                    &property->_parent,listener))
    {
        NETIO_LOG_LOOP_INITIALIZE_FAILED(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    id._value = factory->_parent._id._value;
    NETIO_Interface_Table_name_from_id(tbl_name,&id,'r',
                                       factory->instance_counter);

    tbl_property.max_records = (RTI_SIZE_T)property->_parent.max_routes;
    dbrc = DB_Database_create_table(&test_intf->_parent._rtable,
                                    property->_parent._parent.db,
                                    &tbl_name[0],
                                    sizeof(struct NETIORouteEntry),
                                    NETIO_Interface_compare_route,
                                    &tbl_property);
    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_LOG_LOOP_CREATE_TABLE(OSAPI_LOGKIND_ERROR,dbrc)
        return RTI_FALSE;
    }

    tbl_property.max_records = (RTI_SIZE_T)property->_parent.max_binds;
    NETIO_Interface_Table_name_from_id(tbl_name,&id,'b',
                                       factory->instance_counter);
    if (DB_Database_create_table(&test_intf->_parent._btable,
                                 property->_parent._parent.db,
                                 &tbl_name[0],
                                 sizeof(struct NETIOBindEntry),
                                 NETIO_Interface_compare_bind,
                                 &tbl_property) != DB_RETCODE_OK)
    {
        NETIO_LOG_LOOP_CREATE_TABLE(OSAPI_LOGKIND_ERROR,dbrc)
        return RTI_FALSE;
    }

    test_intf->factory = factory;
    ++factory->instance_counter;

    return RTI_TRUE;
}

/*ci
 * \brief Create a new loopback interface instance
 *
 * \param[in] factory   Factory creating the new instance
 * \param[in] property  The property of the new loopback interface
 * \param[in] listener  The listener for the new loopback interface
 *
 * \return Pointer to new loopback interface instance on success, NULL
 *         on failure
 *
 * \sa \ref LOOP_Interface_delete
 */
MUST_CHECK_RETURN RTI_PRIVATE struct LOOP_Interface*
LOOP_Interface_create(struct LOOP_InterfaceFactory *factory,
                      const struct LOOP_InterfaceProperty *const property,
                      const struct NETIO_InterfaceListener *const listener)
{
    struct LOOP_Interface *lo_intf;

    OSAPI_PRECONDITION((factory == NULL) ||
                           (property == NULL),
                   return NULL,
                   OSAPI_Log_entry_add_pointer("factory",factory,RTI_FALSE);
                   OSAPI_Log_entry_add_pointer("property",property,RTI_TRUE);)

    OSAPI_Heap_allocate_struct(&lo_intf,struct LOOP_Interface);

    if (lo_intf == NULL)
    {
        return NULL;
    }

    if (!LOOP_Interface_initialize(lo_intf,factory,property,listener))
    {
#ifndef RTI_CERT
        OSAPI_Heap_free_struct(lo_intf);
#endif
        return NULL;
    }

    return lo_intf;
}

/*ci
 * \brief Cancel transmission of a packet on the loopback interface
 *
 * \details
 * Implementation of the NETIO_Interface_xmit_remove function.
 * Although the loopback interface cannot cancel transmission of a packet,
 * an upstream interface does not necessarily keep track of the capabilities
 * of the downstream interface and will call the xmite_remove function on the
 * downstream interface.
 *
 * \param[in] intf        NETIO interface to cancel a transmit on
 * \param[in] destination The destination address of the packet
 * \param[in] packet_id   The packet_id/SN of the NETIO_Packet to cancel
 *
 * \return This function always returns RTI_TRUE
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
LOOP_Interface_xmit_remove(NETIO_Interface_T *intf,
                           struct NETIO_Address *destination,
                           NETIO_PacketId_T *packet_id)
{
    /* xmit_remove is NOOP for loopback intf */
    UNUSED_ARG(intf);
    UNUSED_ARG(destination);
    UNUSED_ARG(packet_id);

    return RTI_TRUE;
}


/*ci
 * \brief Send a packet to all interfaces bound to the loopback interface
 *
 * \details
 * Implementation of the NETIO_Interface_send function. All messages sent
 * on this interface is immediately sent to all listeners on this interface.
 * Thus, the receivers are receiving the data in the context of the sender.
 * Care must be taken to not block the sender.
 *
 * \param[in] netio_intf NETIO interface to send from
 * \param[in] source     The source of the packet
 * \param[in] address    The destination address
 * \param[in] packet     The packet to send
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
LOOP_Interface_send(NETIO_Interface_T *netio_intf,
                    struct NETIO_Interface *source,
                    struct NETIO_Address *address,
                    NETIO_Packet_T *packet)
{
    struct LOOP_Interface *self = (struct LOOP_Interface *)netio_intf;
    DB_Cursor_T cursor;
    DB_ReturnCode_T dbrc;
    struct NETIOBindEntry *bind_entry;
    struct NETIOBindEntryKey bind_key;
    RTI_INT32 i;
    struct NETIO_Address ADDR_UNKNOWN = NETIO_Address_INITIALIZER;
    struct NETIO_Address *cmp_address;
    RTI_BOOL bretval;
    UNUSED_ARG(address);
    UNUSED_ARG(source);

    OSAPI_PRECONDITION((netio_intf == NULL) || (source == NULL) ||
                           (packet == NULL),
                           return RTI_FALSE,
               OSAPI_Log_entry_add_pointer("netio_intf",netio_intf,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("source",source,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("packet",packet,RTI_TRUE);)

    if (packet->dests == NULL)
    {
        return RTI_FALSE;
    }

    for (i = 0; i < NETIO_AddressSeq_get_length(packet->dests); ++i)
    {
        bind_key.source = packet->source;
        bind_key.destination = *NETIO_AddressSeq_get_reference(packet->dests, i);

        dbrc = DB_Table_select_all_default(self->_parent._btable,&cursor);
        if (dbrc != DB_RETCODE_OK)
        {
            NETIO_LOG_LOOP_SELECT_TABLE(OSAPI_LOGKIND_ERROR,dbrc)
            return RTI_FALSE;
        }

        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&bind_entry);
        while (dbrc == DB_RETCODE_OK)
        {
            if (!NETIO_Address_compare(&bind_key.destination, &ADDR_UNKNOWN))
            {
                cmp_address = &bind_entry->source;
            }
            else
            {
                cmp_address = &bind_entry->destination;
            }

            packet->local_source = self->_parent.local_address;

            if (!NETIO_Address_compare(&packet->source,cmp_address))
            {
                /* The packet already has the source of the writer interface */
                bretval = NETIO_Interface_receive(bind_entry->intf,
                                                &self->_parent.local_address,
                                                &bind_entry->destination,
                                                packet);
#if OSAPI_ENABLE_LOG
                if (!bretval)
                {
                    NETIO_LOG_LOOP_FWD(OSAPI_LOGKIND_WARNING)
                }
#else
                IGNORE_RETVAL(bretval);
#endif
            }
            dbrc = DB_Cursor_get_next(cursor,(DB_Record_T)&bind_entry);
        }

#if OSAPI_ENABLE_LOG
        if (dbrc != DB_RETCODE_NO_DATA)
        {
            NETIO_LOG_LOOP_CURSOR_ERROR(OSAPI_LOGKIND_ERROR)
        }
#else
        IGNORE_RETVAL(dbrc);
#endif

        DB_Cursor_finish(self->_parent._btable,cursor);
    }

    return RTI_TRUE;
}

/*ci
 * \brief Add a route to the loopback interface
 *
 * \details
 * Implementation of the NETIO add_route function
 * The loopback interface does not keep track of any route since it is stateless.
 *
 * \param[in] netio_intf NETIO interface to add the route too
 * \param[in] dst_addr   The destination address for the route
 * \param[in] via_intf   The downstream interface
 * \param[in] via_addr   The address to pass to the downstream interface
 * \param[in] property   The route property
 * \param[in] existed    Whether the route already existed
 *
 * \return This function always returns RTI_TRUE
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
LOOP_Interface_add_route(NETIO_Interface_T *netio_intf,
                         struct NETIO_Address *dst_addr,
                         NETIO_Interface_T *via_intf,
                         struct NETIO_Address *via_addr,
                         struct NETIORouteProperty *property,
                         RTI_BOOL *existed)
{
    PRECOND_ARG(netio_intf)
    PRECOND_ARG(dst_addr)
    PRECOND_ARG(via_intf)
    PRECOND_ARG(via_addr)
    UNUSED_ARG(property);

    /* The arguments via_intf and via_addr are unused because
     * LOOP_Interface does not send packets downstream.
     */
    OSAPI_PRECONDITION((netio_intf == NULL) ||
                           (dst_addr == NULL),
                           return RTI_FALSE,
           OSAPI_Log_entry_add_pointer("netio_intf",netio_intf,RTI_FALSE);
           OSAPI_Log_entry_add_pointer("dst_addr",dst_addr,RTI_FALSE);
           OSAPI_Log_entry_add_pointer("via_intf",via_intf,RTI_FALSE);
           OSAPI_Log_entry_add_pointer("via_addr",via_addr,RTI_TRUE);)

    if (existed != NULL)
    {
        *existed = RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief Delete a route from the loopback interface
 *
 * \details
 * Implementation of the NETIO delete_route function.
 * The loopback interface does not keep track of any routes, but implements
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
 * \return This function always returns RTI_TRUE
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
LOOP_Interface_delete_route(NETIO_Interface_T *netio_intf,
                            struct NETIO_Address *dst_addr,
                            NETIO_Interface_T *via_intf,
                            struct NETIO_Address *via_addr,
                            RTI_BOOL *existed)
{
    PRECOND_ARG(netio_intf)
    PRECOND_ARG(dst_addr)
    PRECOND_ARG(via_intf)
    PRECOND_ARG(via_addr)

    /* The arguments via_intf and via_addr are unused because
     * LOOP_Interface does not send packets downstream.
     */
    OSAPI_PRECONDITION((netio_intf == NULL) ||
                           (dst_addr == NULL),
                           return RTI_FALSE,
               OSAPI_Log_entry_add_pointer("netio_intf",netio_intf,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("dst_addr",dst_addr,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("via_intf",via_intf,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("via_addr",via_addr,RTI_TRUE);)

    if (existed != NULL)
    {
        *existed = RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief Add a bind to the loopback interface
 *
 * \details
 * Implementation of the NETIO bind function.
 *
 * Loopback does not maintain any state information about its peers, but this
 * function is implemented to comply with the NETIO interface and not burden
 * a controller with knowing which interface maintains state or not.
 *
 * \param[in]  netio_intf NETIO interface to bind
 * \param[in]  src_addr   The address to bind to
 * \param[in]  property   The property to use for the bind
 * \param[out] existed    Whether a previous bind existed or not
 *
 * \return This function always returns RTI_TRUE
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
LOOP_Interface_bind(NETIO_Interface_T *netio_intf,
                    struct NETIO_Address *src_addr,
                    struct NETIOBindProperty *property,
                    RTI_BOOL *existed)
{
    PRECOND_ARG(netio_intf)
    PRECOND_ARG(src_addr)
    UNUSED_ARG(property);

    OSAPI_PRECONDITION((netio_intf == NULL) ||
                           (src_addr == NULL),
                           return RTI_FALSE,
               OSAPI_Log_entry_add_pointer("netio_intf",netio_intf,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("src_addr",src_addr,RTI_TRUE);)

    if (existed != NULL)
    {
        *existed = RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief Remove a peer listener on the loopback interface
 *
 * \details
 * Implementation of the NETIO unbind function.
 *
 * Loopback does not maintain any state information about its peer, but this
 * function is implemented to comply with the NETIO interface and not burden
 * a controller with knowing which interface maintains state or not.
 *
 * \param[in]  netio_intf NETIO interface to bind
 * \param[in]  src_addr   The address to bind to
 * \param[in]  dst_intf   Interface
 * \param[out] existed    Whether a bind existed or not
 *
 * \return This function always returns RTI_TRUE
 *
 * \sa \ref LOOP_Interface_bind
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
LOOP_Interface_unbind(NETIO_Interface_T *netio_intf,
                      struct NETIO_Address *src_addr,
                      NETIO_Interface_T *dst_intf,
                      RTI_BOOL *existed)
{
    PRECOND_ARG(netio_intf)
    PRECOND_ARG(src_addr)
    PRECOND_ARG(dst_intf)

    OSAPI_PRECONDITION((netio_intf == NULL) ||
                           (src_addr == NULL) || (dst_intf == NULL),
                   return RTI_FALSE,
                   OSAPI_Log_entry_add_pointer("netio_intf",netio_intf,RTI_FALSE);
                   OSAPI_Log_entry_add_pointer("src_addr",src_addr,RTI_FALSE);
                   OSAPI_Log_entry_add_pointer("dst_intf",dst_intf,RTI_TRUE);)

    if (existed != NULL)
    {
        *existed = RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief Bind the loopback interface to a upstream interface
 *
 * \details
 * Implementation of the NETIO bind_external function
 * When an upstream interface want to listen to a loopback interface it
 * binds to the downstream interface using the external bind function.
 * For loopback this means adding an interface to a bind table so
 * when send is called all the bound interfaces are called one by one.
 * Note that the loopback interface is a synchronous interface. Thus,
 * sending data also causes data to be received in the sending thread.
 *
 * \param[in]  src_intf    NETIO interface to bind to upstream interface
 * \param[in]  src_addr   The address to bind to
 * \param[in]  dst_intf   The upstream interface
 * \param[in]  dst_addr   The address to pass to the upstream interface
 * \param[in]  property   The properties for the bind
 * \param[out] existed    Whether a previous bind already existed for this entry
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * \sa \ref LOOP_Interface_unbind_external
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
LOOP_Interface_bind_external(NETIO_Interface_T *src_intf,
                             struct NETIO_Address *src_addr,
                             NETIO_Interface_T *dst_intf,
                             struct NETIO_Address *dst_addr,
                             struct NETIOBindProperty *property,
                             RTI_BOOL *existed)
{
    struct LOOP_Interface *self = (struct LOOP_Interface *)src_intf;
    struct NETIOBindEntry *bind_entry = NULL;
    DB_ReturnCode_T dbrc;
    struct NETIOBindEntryKey bind_key;
    UNUSED_ARG(property);

    OSAPI_PRECONDITION((src_intf == NULL) ||
                           (src_addr == NULL) ||
                           (dst_intf == NULL) ||
                           (dst_addr == NULL),
                           return RTI_FALSE,
               OSAPI_Log_entry_add_pointer("src_intf",src_intf,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("src_addr",src_addr,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("dst_intf",dst_intf,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("dst_addr",dst_addr,RTI_TRUE);)

    bind_key.source = *src_addr;
    bind_key.destination = *dst_addr;

    dbrc = DB_Table_select_match(self->_parent._btable,
                                DB_TABLE_DEFAULT_INDEX,
                                (DB_Record_T *)&bind_entry,
                                &bind_key);

    if (dbrc == DB_RETCODE_OK)
    {
        if (existed)
        {
            *existed = RTI_TRUE;
        }
        return RTI_TRUE;
    }

    if (existed)
    {
      *existed = RTI_FALSE;
    }

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        NETIO_LOG_LOOP_BINDX_DB(OSAPI_LOGKIND_ERROR,dbrc)
        return RTI_FALSE;
    }

    dbrc = DB_Table_create_record(self->_parent._btable,
                                  (DB_Record_T *)&bind_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_LOG_LOOP_BINDX_DB(OSAPI_LOGKIND_ERROR,dbrc)
        return RTI_FALSE;
    }

    OSAPI_Memory_zero(bind_entry,sizeof(struct NETIOBindEntry));
    bind_entry->source = *src_addr;
    bind_entry->destination = *dst_addr;
    bind_entry->intf = dst_intf;

    dbrc = DB_Table_insert_record(self->_parent._btable,
                                  (DB_Record_T)bind_entry);

    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_LOG_LOOP_BINDX_DB(OSAPI_LOGKIND_ERROR,dbrc)
        (void)DB_Table_delete_record(self->_parent._btable,
                                     (DB_Record_T)bind_entry);
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief  Remove an upstream interface as a listener to the loopback interface.
 *
 * \details
 * Implementation of the NETIO unbind_external function.
 *
 * When an upstream interface want to remove a listener to a loopback interface
 * it unbinds from the downstream interface using the external unbind function.
 *
 * \param[in]  src_intf   NETIO interface to unbind from upstream interface
 * \param[in]  src_addr   The address to unbind from
 * \param[in]  dst_intf   The upstream interface
 * \param[in]  dst_addr   The address to passed to the upstream interface
 * \param[out] existed    Whether a previous bind already existed for this entry
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * \sa \ref LOOP_Interface_bind_external
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
LOOP_Interface_unbind_external(NETIO_Interface_T *src_intf,
                               struct NETIO_Address *src_addr,
                               NETIO_Interface_T *dst_intf,
                               struct NETIO_Address *dst_addr,
                               RTI_BOOL *existed)
{
    struct LOOP_Interface *self = (struct LOOP_Interface *)src_intf;
    struct NETIOBindEntry *bind_entry = NULL;
    DB_ReturnCode_T dbrc;
    struct NETIOBindEntryKey bind_key;
    PRECOND_ARG(dst_intf)

    OSAPI_PRECONDITION((src_intf == NULL) ||
                           (src_addr == NULL) ||
                           (dst_intf == NULL) ||
                           (dst_addr == NULL),
                           return RTI_FALSE,
                   OSAPI_Log_entry_add_pointer("src_intf",src_intf,RTI_FALSE);
                   OSAPI_Log_entry_add_pointer("src_addr",src_addr,RTI_FALSE);
                   OSAPI_Log_entry_add_pointer("dst_intf",dst_intf,RTI_FALSE);
                   OSAPI_Log_entry_add_pointer("dst_addr",dst_addr,RTI_TRUE);)

    bind_key.source = *src_addr;
    bind_key.destination = *dst_addr;

    dbrc = DB_Table_select_match(self->_parent._btable,DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T *)&bind_entry,&bind_key);

    if (dbrc == DB_RETCODE_NO_DATA)
    {
        if (existed)
        {
            *existed = RTI_FALSE;
        }
        return RTI_TRUE;
    }

    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_LOG_LOOP_UNBINDX_DB(OSAPI_LOGKIND_ERROR,dbrc)
        return RTI_FALSE;
    }

    if (existed)
    {
        *existed = RTI_TRUE;
    }

    dbrc = DB_Table_delete_record(self->_parent._btable,
                                  (DB_Record_T)bind_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_LOG_LOOP_UNBINDX_DB(OSAPI_LOGKIND_ERROR,dbrc)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief Request which external interface to bind to when forwarding data
 *        upstream.
 *
 * \details
 *
 * Implementation of the NETIO get_external_interface function.
 * When a loopback interface is bound to a downstream interface it is
 * requested to provide which interface and address the downstream interface
 * should use when forwarding a NETIO_Packet.
 *
 * \param[in]   netio_intf   NETIO interface to get the external interface for
 * \param[in]   src_addr     The address to send to
 * \param[out]  dst_intf     The interface to use
 * \param[out]  dst_addr     The destination address to use
 *
 * \return This function always returns RTI_TRUE
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
LOOP_Interface_get_external_interface(NETIO_Interface_T *netio_intf,
                                      struct NETIO_Address *src_addr,
                                      NETIO_Interface_T **dst_intf,
                                      struct NETIO_Address *dst_addr)
{
    PRECOND_ARG(src_addr)
    PRECOND_ARG(dst_addr)

    OSAPI_PRECONDITION((netio_intf == NULL) ||
                           (src_addr == NULL) ||
                           (dst_intf == NULL) ||
                           (dst_addr == NULL),
                           return RTI_FALSE,
                   OSAPI_Log_entry_add_pointer("netio_intf",netio_intf,RTI_FALSE);
                   OSAPI_Log_entry_add_pointer("src_addr",src_addr,RTI_FALSE);
                   OSAPI_Log_entry_add_pointer("dst_intf",dst_intf,RTI_FALSE);
                   OSAPI_Log_entry_add_pointer("dst_addr",dst_addr,RTI_TRUE);)

    *dst_intf = netio_intf;

    return RTI_TRUE;
}

/*ci
 * \brief Set the state of the loopback interface
 *
 * \details
 * Implementation of the NETIO set_state function. The loopback interface is
 * always enabled. This function is only set to comply with the NETIO interface.
 *
 * \param[in] src_intf NETIO interface to set state on
 * \param[in] state    New state
 *
 * \return This function always returns RTI_TRUE
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
LOOP_Interface_set_state(NETIO_Interface_T *src_intf,
                        NETIO_InterfaceState_T state)
{
    struct LOOP_Interface *self = (struct LOOP_Interface *)src_intf;

    OSAPI_PRECONDITION((src_intf == NULL),
                           return RTI_FALSE,
                    OSAPI_Log_entry_add_pointer("src_intf",src_intf,RTI_TRUE);)

    self->_parent.state = state;

    return RTI_TRUE;
}

/*ci
 * \brief Resolve a loopback address
 *
 * \details
 * Implementation of the NETIO resolve_address function.
 *
 * The loopback interface does not support address conversion
 * and will mark all addresses as valid (because no format is defined),
 * but return RTI_FALSE unless the input is "" because it cannot convert
 * it.
 *
 * \param[out] self           Interface
 * \param[in]  address_string Address to convert
 * \param[out] address_value  Converted address on success
 * \param[out] is_invalid     Whether the address is valid or not
 *
 * \return This function return RTI_TRUE if the address string is empty,
 *         RTI_FALSE otherwise.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
LOOP_Interface_resolve_address(NETIO_Interface_T *self,
                               const char *address_string,
                               struct NETIO_Address *address_value,
                               RTI_BOOL *is_invalid)
{
    UNUSED_ARG(self);

    OSAPI_PRECONDITION((address_string == NULL) ||
                        (address_value == NULL) ||
                        (is_invalid == NULL),
                        return RTI_FALSE,
        OSAPI_Log_entry_add_pointer("address_string",address_string,RTI_FALSE);
        OSAPI_Log_entry_add_pointer("address_value",address_value,RTI_FALSE);
        OSAPI_Log_entry_add_pointer("is_invalid",is_invalid,RTI_TRUE);)

    /* No address format is specified, thus no string is invalid */
    *is_invalid = RTI_FALSE;

    NETIO_Address_init(address_value,0);

    NETIO_Address_set_kind(address_value,NETIO_ADDRESS_KIND_INTRA,0);

    if (address_string[0] == 0)
    {
        return RTI_TRUE;
    }

    return RTI_FALSE;
}

/*ci
 * \brief Reserve addresses on the loopback interface
 *
 * \details
 * Implementation of the NETIO reserve_address function.
 *
 * The loopback interface does not require any reservation of addresses
 * as these are never advertised.
 *
 * \param[in]    self       NETIO interface to reserve addresses on
 * \param[in]    req_addr   List of requested addresses
 * \param[inout] resvd_addr The downstream interface
 * \param[in]    property   Properties to use to listen on the reserved addresses
 *
 * \return This function always returns RTI_TRUE
 *
 * \sa \ref LOOP_Interface_release_address
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
LOOP_Interface_reserve_address(NETIO_Interface_T *self,
                              struct NETIO_AddressSeq *req_addr,
                              struct NETIO_AddressSeq *resvd_addr,
                              struct NETIOBindProperty *property)
{
    UNUSED_ARG(self);
    UNUSED_ARG(req_addr);
    UNUSED_ARG(resvd_addr);
    UNUSED_ARG(property);

    return RTI_TRUE;
}

/*ci
 * \brief Release a previously reserved address
 *
 * \details
 * Implementation of the NETIO release_address function. It is not necessary to
 * reserve any addresses from the loopback interface, thus this function only
 * returns TRUE.
 *
 * \param[in] self     NETIO interface to release addresses on
 * \param[in] address  Address to release
 *
 * \return This function always returns RTI_TRUE
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
LOOP_Interface_release_address(NETIO_Interface_T *self,
                               struct NETIO_Address *address)
{
    UNUSED_ARG(self);
    UNUSED_ARG(address);

    return RTI_TRUE;
}

/*ci
 * \brief Get the route table for the loopback interface
 *
 * \details
 * Implementation of the NETIO get_route_table function. Retrieve the
 * addresses this interface is capable of sending.
 *
 * \param[in]    netio_intf The NETIO interface
 * \param[inout] address    Sequence of NETIO addresses this interface understands
 * \param[inout] netmask    Sequence of the corresponding netmasks
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
LOOP_Interface_get_route_table(NETIO_Interface_T *netio_intf,
                             struct NETIO_AddressSeq *address,
                             struct NETIO_NetmaskSeq *netmask)
{
    RTI_INT32 max_size;
    RTI_INT32 cur_addr_len;
    struct NETIO_Address src_address = NETIO_Address_INITIALIZER;
    RTI_INT32 i;
    UNUSED_ARG(netio_intf);

    cur_addr_len = NETIO_AddressSeq_get_length(address);
    max_size = NETIO_AddressSeq_get_maximum(address) - cur_addr_len;

    if (max_size == 0)
    {
        return RTI_TRUE;
    }

    NETIO_Address_init(&src_address,NETIO_ADDRESS_KIND_INTRA);

    if (!NETIO_AddressSeq_set_length(address,cur_addr_len+1))
    {
        NETIO_LOG_LOOP_SET_LENGTH(OSAPI_LOGKIND_ERROR,cur_addr_len+1)
        return RTI_FALSE;
    }

    /* Since set_length succeeded for it is assumed that
     * get_reference returns a valid address.
     */

    /* coverity[dereference] */
    /* coverity[cert_exp34_c_violation] */
    *NETIO_AddressSeq_get_reference(address,cur_addr_len) = src_address;

    if (!NETIO_NetmaskSeq_set_length(netmask,cur_addr_len+1))
    {
        NETIO_LOG_LOOP_SET_LENGTH(OSAPI_LOGKIND_ERROR,cur_addr_len+1)
        return RTI_FALSE;
    }

    /* NOTE: The netmask is all 128 bits because it is not possible to match
     * from right to left, only left to right. If a match could be done
     * right to left only the least 32 bits would be needed since the GUID
     * prefix for intra readers and writers must be the same.
     *
     * Since set_length succeeded for it is assumed that
     * get_reference returns a valid address.
     */

     /* coverity[dereference] */
     /* coverity[cert_exp34_c_violation] */
    NETIO_NetmaskSeq_get_reference(netmask,cur_addr_len)->bits = 0;

    for (i = 0; i < 4; ++i)
    {
        /* Since set_length succeeded for it is assumed that
         * get_reference returns a valid address.
         */
        /* coverity[dereference] */
        /* coverity[cert_exp34_c_violation] */
        NETIO_NetmaskSeq_get_reference(netmask,cur_addr_len)->mask[i] =
                                        src_address.value.as_uint32.value[i];
    }

    return RTI_TRUE;
}

RTI_PRIVATE RTI_BOOL
LOOP_Interface_is_address_reachable(struct NETIO_Interface *netio_intf,
                                   const struct NETIO_AddressEx *const address,
                                   RTI_BOOL *is_reachable)
{
    UNUSED_ARG(netio_intf);
    UNUSED_ARG(address);

    /* Thisd function would not be called if it had not already been
     * able to route to this address. It is only implemented to simiplify the
     * caller logic.
     */
    *is_reachable = RTI_TRUE;

    return RTI_TRUE;
}

RTI_PRIVATE void
LOOP_Interface_get_transport_properties(struct NETIO_Interface *netio_intf,
                                       struct NETIO_TransportProperty *properties)
{
    UNUSED_ARG(netio_intf);

    properties->recv_size_max = INT_MAX;
    properties->send_size_max = INT_MAX;
}

/******************************************************************************
 *
 * LOOP Component Interface
 */
/*ci
 * \brief The NETIO Loopback interface implementation
 *
 * \details
 *
 * The loopback interface works directly between two interfaces and does not
 * support reliability. Thus, ack(), request() and return_loan() will never
 * be executed.
 */
LINK_SECTION_DATA_SDRAM
RTI_PRIVATE struct NETIO_InterfaceI LOOP_Interface_fv_Intf =
{
    RT_COMPONENTI_BASE,
    LOOP_Interface_send,
    NULL,                                 /* ack not needed          */
    NULL,                                 /* request not needed      */
    NULL,                                 /* return_loan not needed  */
    LOOP_Interface_xmit_remove,
    LOOP_Interface_add_route,
    LOOP_Interface_delete_route,
    LOOP_Interface_reserve_address,
    LOOP_Interface_bind,
    LOOP_Interface_unbind,
    NULL,                                 /* receive not needed      */
    LOOP_Interface_get_external_interface,
    LOOP_Interface_bind_external,
    LOOP_Interface_unbind_external,
    LOOP_Interface_set_state,
    LOOP_Interface_release_address,
    LOOP_Interface_resolve_address,
    LOOP_Interface_get_route_table,
    NULL, /* no events supported     */
    NULL,
    LOOP_Interface_is_address_reachable,
    LOOP_Interface_get_transport_properties
};

/* ------------------------------------------------------------------------ */
/*                   Plugin factory                                         */
/* ------------------------------------------------------------------------ */

/*ci
 * \brief Create an instance of the loopback interface.
 *
 * \details
 * Implementation of the RT ComponentFactory create component method. This
 * method creates a new instance of the loopback interface. It is never
 * called directly, only via the component factory interface.
 *
 * \param[in] factory   The factory creating the component
 * \param[in] property  The component property
 * \param[in] listener  The component listener
 *
 * \return A new instance of the loopback interface on success, NULL on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RT_Component_T*
LOOP_InterfaceFactory_create_component(struct RT_ComponentFactory *factory,
                                       struct RT_ComponentProperty *property,
                                       struct RT_ComponentListener *listener)
{
    struct LOOP_Interface *retval = NULL;

    retval = LOOP_Interface_create(
                        (struct LOOP_InterfaceFactory*)factory,
                        (const struct LOOP_InterfaceProperty *)property,
                        (const struct NETIO_InterfaceListener *)listener);

    if (retval == NULL)
    {
        NETIO_LOG_LOOP_INVALID_COMPONENT(OSAPI_LOGKIND_ERROR)
        return NULL;
    }

    return &retval->_parent._parent;
}

#ifndef RTI_CERT
/*ci
 * \brief Delete an instance of the loopback interface
 *
 * \details
 * Implementation of the RT ComponentFactory delete method. This
 * method deletes an instance of the loopback interface. It is never
 * called directly, only via the component factory interface.
 *
 * \param[in] factory   The factory that created the component
 * \param[in] component The component to be deleted
 *
 * \sa \ref LOOP_InterfaceFactory_create_component
 */
RTI_PRIVATE void
LOOP_InterfaceFactory_delete_component(struct RT_ComponentFactory *factory,
                                       RT_Component_T *component)
{
    struct LOOP_Interface *self = (struct LOOP_Interface *)component;

    if (factory == NULL)
    {
        NETIO_LOG_LOOP_INVALID_FACTORY(OSAPI_LOGKIND_ERROR)
        return;
    }

    LOOP_Interface_delete(self);
}
#endif /* !RTI_CERT */

MUST_CHECK_RETURN RTI_PRIVATE struct RT_ComponentFactory*
LOOP_InterfaceFactory_initialize(struct RT_ComponentFactoryProperty*property,
                                struct RT_ComponentFactoryListener *listener);

#ifndef RTI_CERT
RTI_PRIVATE void
LOOP_InterfaceFactory_finalize(struct RT_ComponentFactory *factory,
                              struct RT_ComponentFactoryProperty **property,
                              struct RT_ComponentFactoryListener **listener);
#endif /* !RTI_CERT */

/*ci
 * \brief Implementation of the RT ComponentFactoryI interface
 */
#ifndef RTI_CERT
LINK_SECTION_DATA_SDRAM
RTI_PRIVATE struct RT_ComponentFactoryI LOOP_InterfaceFactory_fv_Intf =
{
    INTRA_INTERFACE_INTERFACE_ID,
    LOOP_InterfaceFactory_initialize,
    LOOP_InterfaceFactory_finalize,
    LOOP_InterfaceFactory_create_component,
    LOOP_InterfaceFactory_delete_component,
    NULL,
    NULL
};
#else
LINK_SECTION_DATA_SDRAM
RTI_PRIVATE struct RT_ComponentFactoryI LOOP_InterfaceFactory_fv_Intf =
{
    INTRA_INTERFACE_INTERFACE_ID,
    LOOP_InterfaceFactory_initialize,
    NULL, /* LOOP_InterfaceFactory_finalize, */
    LOOP_InterfaceFactory_create_component,
    NULL, /* LOOP_InterfaceFactory_delete_component, */
    NULL,
    NULL
};
#endif /* !RTI_CERT */

/*ci
 * \brief Loopback interface factory
 *
 * \details
 * The loopback interface class is implemented as a singleton, there are
 * no shared resources between interfaces.
 */
LINK_SECTION_DATA_SDRAM
RTI_PRIVATE
struct LOOP_InterfaceFactory LOOP_InterfaceFactory_fv_Factory =
{
  {
     &LOOP_InterfaceFactory_fv_Intf,
     NULL,
     {{{0,0}}}
  },
  0
};

/*ci
 * \brief Initialize the loopback interface factory
 *
 * \details
 * Loopback specific implementation of the RT ComponentFactory initialize
 * method. This method is never called directly. It is called by the
 * RT when the loopback factory is registered.
 *
 * \param[in] property The properties registered with the loopback interface
 * \param[in] listener The listener registered with the loopback interface
 *
 * \return A fully initialized factory on success, NULL on failure
 *
 * \sa \ref LOOP_InterfaceFactory_finalize
 */
MUST_CHECK_RETURN RTI_PRIVATE struct RT_ComponentFactory*
LOOP_InterfaceFactory_initialize(struct RT_ComponentFactoryProperty *property,
                                 struct RT_ComponentFactoryListener *listener)
{
    struct LOOP_InterfaceFactory *factory = &LOOP_InterfaceFactory_fv_Factory;
    UNUSED_ARG(property);
    UNUSED_ARG(listener);

    LOOP_InterfaceFactory_fv_Factory._parent._factory = &factory->_parent;
    factory->instance_counter = 0;

    return &LOOP_InterfaceFactory_fv_Factory._parent;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize the loopback interface factory
 *
 * Loopback specific implementation of the RT ComponentFactory finalize
 * method. This method is never called directly. It is called by the
 * RT when the loopback factory is unregistered.
 *
 * \param[in]  factory  The factory to finalize
 * \param[out] property The property the factory was registered with
 * \param[out] listener The listener the factory was registered with
 *
 * \sa \ref LOOP_InterfaceFactory_initialize
 */
RTI_PRIVATE void
LOOP_InterfaceFactory_finalize(struct RT_ComponentFactory *factory,
                              struct RT_ComponentFactoryProperty **property,
                              struct RT_ComponentFactoryListener **listener)
{
    UNUSED_ARG(factory);
    UNUSED_ARG(property);
    UNUSED_ARG(listener);
}
#endif /* !RTI_CERT */

struct RT_ComponentFactoryI*
LOOP_InterfaceFactory_get_interface(void)
{
    return &LOOP_InterfaceFactory_fv_Intf;
}

/*ci @} */
