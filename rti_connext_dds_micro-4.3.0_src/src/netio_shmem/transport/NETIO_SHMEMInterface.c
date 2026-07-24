/*
 * FILE: NETIO_SHMEMInterface.c
 *
 * (c) Copyright 2018-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#include "netio/netio_log.h"
#include "osapi/osapi_process.h"
#include "netio_shmem/netio_shmem.h"
#include "cdr/cdr_md5.h"

#include "NETIO_SHMEMInterface.h"

#define SHMEM_WAKEUP_RECEIVE_THREAD_PING_COUNT 100

RTI_PRIVATE struct NETIO_InterfaceI NETIO_SHMEMInterface_fv_Intf;

struct NETIO_SHMEMInterfaceFactoryProperty
        NETIO_SHMEMINTERFACE_FACTORY_PROPERTY_DEFAULT =
        NETIO_SHMEMInterfaceFactoryProperty_INITIALIZER;

/*ci
 * \brief SHMEM wakeup a receive thread
 *
 * \details
 *
 * SHMEM receive threads calls blocking read calls. To unblock a receive
 * thread when a SHMEM receive thread is deleted this wakeup function is called
 * which sends signal to the signaling semaphore to unblock the thread.
 *
 * \param[in] thread_info Thread specific data passed from OSAPI
 *
 * \return RTI_FALSE on failure, RTI_TRUE on success
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_SHMEMInterface_wakeup_receive_thread(
        struct OSAPI_ThreadInfo *thread_info)
{
    struct SHMEMPortEntry *port_entry = (struct
            SHMEMPortEntry*)thread_info->user_data;
    RTI_INT32 i = 0;
    RTI_INT32 status_out;

    OSAPI_TRACE_NET("wake up receive thread:", RTI_FALSE)
    OSAPI_TRACE_INT32("port", (RTI_INT32)port_entry->source.port, RTI_FALSE)
    OSAPI_TRACE_INT32(
            "address",
            (RTI_INT32)port_entry->source.value.ipv4.address,
            RTI_TRUE)

    port_entry->_recv_resource->_is_force_unblocked = RTI_TRUE;

    for (i = 0; i < SHMEM_WAKEUP_RECEIVE_THREAD_PING_COUNT; ++i)
    {
        if (!NETIO_SharedMemorySignalingSemaphore_signal(
                &port_entry->_recv_resource->_shm_semaphore,
                &status_out))
        {
            return RTI_FALSE;
        }
    }
    return RTI_TRUE;
}

RTI_PRIVATE RTI_INT32
NETIO_SHMEMInterface_compare_route_entry(
        RTI_INT32 flags,
        const DB_Record_T op1,
        void *op2)
{
    struct SHMEMRouteEntry *left_type = (struct SHMEMRouteEntry*)op1;
    struct SHMEMRouteEntry *right_type = (struct SHMEMRouteEntry*)op2;
    UNUSED_ARG(flags);

    if (left_type->send_resource.port != right_type->send_resource.port)
    {
        return (RTI_INT32)(left_type->send_resource.port - right_type->send_resource.port);
    }

    return OSAPI_Memory_compare(
            &right_type->send_resource.address,
            &left_type->send_resource.address,
            16);
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_INT32
NETIO_SHMEMInterface_compare_port(
        RTI_INT32 flags,
        const DB_Record_T op1,
        void *op2)
{
    struct SHMEMPortEntry *record_left = (struct SHMEMPortEntry*)op1;
    const struct NETIO_Address *key;

    if (DB_SELECT_OP2_IS_KEY(flags))
    {
        key = (const struct NETIO_Address*)op2;
    }
    else
    {
        key = &((struct SHMEMPortEntry*)op2)->source;
    }

    if (record_left->source.port > key->port)
    {
        return 1;
    }

    if (record_left->source.port < key->port)
    {
        return -1;
    }
    return 0;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize a logger interface instance
 *
 * \param[in] netio_intf Interface to finalize
 *
 * \sa \ref NETIO_SHMEMInterface_initialize
 */
RTI_PRIVATE void
NETIO_SHMEMInterface_finalize(
        struct NETIO_SHMEMInterface *netio_intf)
{
    struct SHMEMRouteEntry *r_entry = NULL;
    DB_Cursor_T cursor = NULL;
    DB_ReturnCode_T dbrc;

    /* Only the bind table needs to be removed and deleted */
    NETIO_Interface_finalize(&netio_intf->_parent);

    /*
     * At this point, any remaining route entries in the route table should
     * have their reference count at 1. This means that a single call
     * to delete_route will all of their resources. This will leave the
     * route_table empty. If this is not the case, then we will fail
     * to delete the table.
     */
    cursor = NULL;
    dbrc = DB_Table_select_all_default(netio_intf->_parent._rtable, &cursor);
    if (dbrc == DB_RETCODE_OK)
    {
        dbrc = DB_Cursor_get_next(cursor, (DB_Record_T*)&r_entry);
        while (dbrc == DB_RETCODE_OK)
        {
            struct NETIO_Address dst_addr = NETIO_Address_INITIALIZER;
            RTI_BOOL existed = RTI_TRUE;
            dst_addr.port = r_entry->send_resource.port;
            dst_addr.value.guid = r_entry->send_resource.address;

            if (!NETIO_Interface_delete_route(
                    (NETIO_Interface_T* )netio_intf,
                    &dst_addr,
                    NULL,
                    NULL,
                    &existed))
            {
                NETIO_SHMEM_LOG_ROUTE_DELETE_FAILED(OSAPI_LOGKIND_ERROR);
            }
            dbrc = DB_Cursor_get_next(cursor, (DB_Record_T*)&r_entry);
        }
    }

    DB_Cursor_finish(netio_intf->_parent._rtable, cursor);

    dbrc = DB_Database_delete_table(
            netio_intf->property._parent.db,
            netio_intf->_parent._rtable);
    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_SHMEM_LOG_DELETE_TABLE(OSAPI_LOGKIND_ERROR, dbrc);
    }
}
#endif

#ifndef RTI_CERT
/*ci
 * \brief Delete a logger interface instance
 *
 * \param[in] netio_intf Interface to delete
 *
 * \sa \ref NETIO_SHMEMInterface_create
 */
RTI_PRIVATE void
NETIO_SHMEMInterface_delete(struct NETIO_SHMEMInterface *self)
{
    NETIO_SHMEMInterface_finalize(self);
    OSAPI_Heap_free_struct(self->shm_transport);
    OSAPI_Heap_free_struct(self);
}
#endif

/*ci
 * \brief Initialize a logger interface instance
 *
 * \param[in] test_intf Interface to initialize
 * \param[in] factory   Factory that is creating the instance
 * \param[in] property  The property of the new logger interface
 * \param[in] listener  The listener for the new logger interface
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref NETIO_SHMEMInterface_finalize
 *
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_SHMEMInterface_initialize(
        struct NETIO_SHMEMInterface *shmem_intf,
        struct NETIO_SHMEMInterfaceFactory *factory,
        const struct NETIO_InterfaceProperty* const property,
        const struct NETIO_InterfaceListener* const listener)
{
    struct DB_TableProperty tbl_property = DB_TableProperty_INITIALIZER;
    struct NETIO_SHMEMTransport *shared_mem_object;
    char tbl_name[NETIO_TABLE_NAME_SIZE];
    union RT_ComponentFactoryId id;
    DB_ReturnCode_T dbrc;

    if (property == NULL)
    {
        NETIO_SHMEM_LOG_INVALID_PROPERTY(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    shmem_intf->property = *property;

    if (!NETIO_Interface_initialize(
            &shmem_intf->_parent,
            &NETIO_SHMEMInterface_fv_Intf,
            (const struct NETIO_InterfaceProperty* const )&property->_parent,
            listener))
    {
        NETIO_SHMEM_LOG_INITIALIZE_FAILED(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    id._value = factory->_parent._id._value;

    /* Bind Table */
    tbl_property.max_records = (RTI_SIZE_T)property->max_binds;
    NETIO_Interface_Table_name_from_id(
            tbl_name,
            &id,
            'b',
            factory->instance_counter);

    dbrc = DB_Database_create_table(
            &shmem_intf->_parent._btable,
            property->_parent.db,
            &tbl_name[0],
            sizeof(struct SHMEMBindEntry),
            NETIO_Interface_compare_bind,
            &tbl_property);

    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_SHMEM_LOG_CREATE_TABLE(OSAPI_LOGKIND_ERROR, dbrc)
        return RTI_FALSE;
    }

    /* Port Entry Table */
    shmem_intf->shmem_receive_thread_table = NULL;

    NETIO_Interface_Table_name_from_id(
            tbl_name,
            &id,
            't',
            factory->instance_counter);

    dbrc = DB_Database_create_table(
            &shmem_intf->shmem_receive_thread_table,
            property->_parent.db,
            &tbl_name[0],
            sizeof(struct SHMEMPortEntry),
            NETIO_SHMEMInterface_compare_port,
            &tbl_property);

    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_SHMEM_LOG_CREATE_TABLE(OSAPI_LOGKIND_ERROR, dbrc)
        return RTI_FALSE;
    }

    /* Route Table */
    tbl_property.max_records = (RTI_SIZE_T)property->max_routes;
    NETIO_Interface_Table_name_from_id(
            tbl_name,
            &id,
            'r',
            factory->instance_counter);

    dbrc = DB_Database_create_table(
            &shmem_intf->_parent._rtable,
            property->_parent.db,
            &tbl_name[0],
            sizeof(struct SHMEMPortEntry),
            NETIO_SHMEMInterface_compare_route_entry,
            &tbl_property);

    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_SHMEM_LOG_CREATE_TABLE(OSAPI_LOGKIND_ERROR, dbrc)
        return RTI_FALSE;
    }

    shmem_intf->factory = factory;
    ++factory->instance_counter;

    OSAPI_Heap_allocate_struct(&shared_mem_object, struct NETIO_SHMEMTransport);
    OSAPI_Memory_zero(shared_mem_object, sizeof(struct NETIO_SHMEMTransport));

    shared_mem_object->_property.received_message_count_max =
            shmem_intf->factory->property.received_message_count_max;
    shared_mem_object->_property.message_size_max =
            shmem_intf->factory->property.message_size_max;
    shared_mem_object->_property.receive_buffer_size =
            shmem_intf->factory->property.receive_buffer_size;

    shared_mem_object->_property.segment_key_offset = 0x400000;
    shared_mem_object->_property.segment_key_factor = 1;
    shared_mem_object->_property.semaphore_key_offset = 0x800000;
    shared_mem_object->_property.semaphore_key_factor = 1;
    shared_mem_object->_property.mutex_key_offset = 0xB00000;
    shared_mem_object->_property.mutex_key_factor = 1;
    shared_mem_object->_major_version = factory->major_version;
    shared_mem_object->_property.max_allowed_writer_send_duration.sec = 1;
    shared_mem_object->_property.max_allowed_writer_send_duration.nanosec = 0;

    shmem_intf->shm_transport = shared_mem_object;

    return RTI_TRUE;
}

/*ci
 * \brief Create a new logger interface instance
 *
 * \param[in] factory   Factory creating the new instance
 * \param[in] property  The property of the new logger interface
 * \param[in] listener  The listener for the new logger interface
 *
 * \return Pointer to new logger interface instance on success, NULL
 *         on failure
 *
 * \sa \ref NETIO_SHMEMInterface_delete
 */
MUST_CHECK_RETURN RTI_PRIVATE struct NETIO_SHMEMInterface*
NETIO_SHMEMInterface_create(
        struct NETIO_SHMEMInterfaceFactory *factory,
        const struct NETIO_InterfaceProperty* const property,
        const struct NETIO_InterfaceListener* const listener)
{
    struct NETIO_SHMEMInterface *netio_intf = NULL;

    OSAPI_PRECONDITION(
            (factory == NULL) || (property == NULL),
            return NULL,
            OSAPI_Log_entry_add_pointer("factory",factory,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("property",property,RTI_TRUE); )

    OSAPI_Heap_allocate_struct(&netio_intf, struct NETIO_SHMEMInterface);
    if (netio_intf == NULL)
    {
        NETIO_SHMEM_LOG_FAILED_TO_ALLOCATE_STRUCT(OSAPI_LOGKIND_ERROR)
        return NULL;
    }

    if (!NETIO_SHMEMInterface_initialize(
            netio_intf,
            factory,
            property,
            listener))
    {
#ifndef RTI_CERT
        OSAPI_Heap_free_struct(netio_intf);
#endif
        NETIO_SHMEM_LOG_FAILED_TO_INITIALIZE(OSAPI_LOGKIND_ERROR)
        return NULL;
    }

    return netio_intf;
}

/*ci
 * \brief Implementation of the NETIO_Interface_xmit_remove function
 *
 * \details
 *
 * Although the logger interface cannot cancel transmission of packet,
 * an upstream interface does not necessarily keep track of the capabilities
 * of the downstream interface and will call the xmite_remove function on the
 * downstream interface.
 *
 * \param[in] intf        NETIO interface to cancel a transmit on
 * \param[in] destination The destination address of the packet
 * \param[in] packet_id   The packet_id/SN of the NETIO_Packet to cancel
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_SHMEMInterface_xmit_remove(
        NETIO_Interface_T *netio_intf,
        struct NETIO_Address *destination,
        NETIO_PacketId_T *packet_id)
{
    UNUSED_ARG(netio_intf);
    UNUSED_ARG(destination);
    UNUSED_ARG(packet_id);
    return RTI_TRUE;
}

/*ci
 * \brief Implementation of the NETIO_Interface_send function
 *
 * \param[in] netio_intf NETIO interface to send from
 * \param[in] source     The source of the packet
 * \param[in] address    The destination address
 * \param[in] packet     The packet to send
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_SHMEMInterface_send(
        NETIO_Interface_T *self,
        struct NETIO_Interface *source,
        struct NETIO_Address *address,
        NETIO_Packet_T *packet)
{
    struct NETIO_SHMEMInterface *intf = (struct NETIO_SHMEMInterface*)self;
    struct NETIO_Address *dest_addr = NULL;
    struct SHMEM_SendResource *cached_send_resource = NULL;
    struct SHMEMRouteEntry route_key;
    struct SHMEMRouteEntry *new_route_entry = NULL;

    RTI_BOOL rval = RTI_TRUE;
    RTI_INT32 i = 0;

    DB_ReturnCode_T dbrc;

    UNUSED_ARG(address);
    UNUSED_ARG(source);

    OSAPI_PRECONDITION(
            (self == NULL) || (packet == NULL),
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("netio_intf",self,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("packet",packet,RTI_TRUE); )

    for (i = 0; i < NETIO_AddressSeq_get_length(packet->dests); ++i)
    {
        dest_addr = NETIO_AddressSeq_get_reference(packet->dests, i);

        /* Find the send resource in the route table
         */
        route_key.send_resource.port = dest_addr->port;
        route_key.send_resource.address = dest_addr->value.guid;

        dbrc = DB_Table_select_match(
                self->_rtable,
                DB_TABLE_DEFAULT_INDEX,
                (DB_Record_T*)&new_route_entry,
                (DB_Key_T) & route_key);

        if (dbrc == DB_RETCODE_OK)
        {
            cached_send_resource = &new_route_entry->send_resource;
        }
        else
        {
            NETIO_SHMEM_LOG_ROUTE_LOOKUP_FAILED(OSAPI_LOGKIND_ERROR);
            continue;
        }

        /* Send may fail, for instance, when the receiver shared segment
         * does not exist. However, this is not necessarily an error
         */
        rval = CoreShmTransport_send(
                intf->shm_transport,
                &cached_send_resource,
                (RTI_INT32)dest_addr->port,
                packet);

        IGNORE_RETVAL(rval);
    }

    return RTI_TRUE;
}

/*ci
 * \brief Implementation of the NETIO add_route function
 *
 * \details
 *
 * The logger interface does not keep track of any route since it is stateless.
 *
 * \param[in] netio_intf NETIO interface to add the route too
 * \param[in] dst_addr   The destination address for the route
 * \param[in] via_intf   The downstream interface
 * \param[in] via_addr   The address to pass to the downstream interface
 * \param[in] property   The route property
 * \param[in] existed    Whether the route already existed
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_SHMEMInterface_add_route(
        NETIO_Interface_T *netio_intf,
        struct NETIO_Address *dst_addr,
        NETIO_Interface_T *via_intf,
        struct NETIO_Address *via_addr,
        struct NETIORouteProperty *property,
        RTI_BOOL *existed)
{

    RTI_BOOL rval = RTI_TRUE;
    DB_ReturnCode_T dbrc;
    RTI_UINT32 destination_port = dst_addr->port;
    struct SHMEMRouteEntry route_key;
    struct SHMEMRouteEntry *new_route_entry = NULL;
    struct NETIO_SHMEMInterface *self =
            (struct NETIO_SHMEMInterface*)netio_intf;

    UNUSED_ARG(netio_intf);
    UNUSED_ARG(via_intf);
    UNUSED_ARG(via_addr);
    UNUSED_ARG(property);


    route_key.send_resource.port = destination_port;
    route_key.send_resource.address = dst_addr->value.guid;

    dbrc = DB_Table_select_match(
            self->_parent._rtable,
            DB_TABLE_DEFAULT_INDEX,
            (DB_Record_T*)&new_route_entry,
            (DB_Key_T) & route_key);

    if (existed != NULL)
    {
        *existed = (dbrc == DB_RETCODE_OK ? RTI_TRUE : RTI_FALSE);
    }

    if (dbrc == DB_RETCODE_NO_DATA)
    {
        dbrc = DB_Table_create_record(
                self->_parent._rtable,
                (DB_Record_T*)&new_route_entry);

        if (dbrc != DB_RETCODE_OK)
        {
            NETIO_SHMEM_LOG_RECORD(OSAPI_LOGKIND_ERROR, dbrc);
            return RTI_FALSE;
        }

        if (!CoreShmTransport_create_send_resource(
                self->shm_transport,
                &new_route_entry->send_resource,
                (RTI_INT32)destination_port))
        {
            OSAPI_TRACE_NET("failed to create send_resource", RTI_FALSE)
            OSAPI_TRACE_INT32("port", (RTI_INT32)dst_addr->port, RTI_FALSE)
            OSAPI_TRACE_GUID("address", &dst_addr->value.rtps_guid, RTI_TRUE)
            (void) DB_Table_delete_record(
                    self->_parent._rtable,
                    (DB_Record_T) new_route_entry);
            return RTI_FALSE;
        }

        new_route_entry->send_resource.port = destination_port;
        new_route_entry->send_resource.address = dst_addr->value.guid;
        new_route_entry->ref_count = 1;

        dbrc = DB_Table_insert_record(
                self->_parent._rtable,
                (DB_Record_T)new_route_entry);
        if (dbrc != DB_RETCODE_OK)
        {
            NETIO_SHMEM_LOG_RECORD(OSAPI_LOGKIND_ERROR, dbrc);
            (void)DB_Table_delete_record(
                    self->_parent._rtable,
                    (DB_Record_T)new_route_entry);
            return RTI_FALSE;
        }

    }
    else if (dbrc == DB_RETCODE_OK)
    {
        ++new_route_entry->ref_count;
    }
    else
    {
        NETIO_SHMEM_LOG_RECORD(OSAPI_LOGKIND_ERROR, dbrc)
        return RTI_FALSE;
    }

    return rval;
}

/*ci
 * \brief Implementation of the NETIO delete_route function
 *
 * \details
 *
 * The SHMEM interface does not keep track of any routes, but implements
 * this function to be compliant with the NETIO interface and minimize the
 * burden on the controller to keep track of which interface maintains
 * state and not.
 *
 * \param[in]  intf       NETIO interface to add the route too
 * \param[in]  dst_addr   The destination address for the route
 * \param[in]  via_intf   The downstream interface
 * \param[in]  via_addr   The address to pass to the downstream interface
 * \param[out] existed    Whether the route existed
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure. Note that is it not
 *         considered a failure if the interface didn't exist.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_SHMEMInterface_delete_route(
        NETIO_Interface_T *intf,
        struct NETIO_Address *dst_addr,
        NETIO_Interface_T *via_intf,
        struct NETIO_Address *via_addr,
        RTI_BOOL *existed)
{
    DB_ReturnCode_T dbrc;
    struct SHMEMRouteEntry route_key;
    struct SHMEMRouteEntry *route_entry = NULL;
    struct NETIO_SHMEMInterface *self = (struct NETIO_SHMEMInterface*)intf;

    UNUSED_ARG(via_intf);
    UNUSED_ARG(via_addr);

    OSAPI_PRECONDITION(
            (intf == NULL) || (dst_addr == NULL),
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("netio_intf",intf,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("dst_addr",dst_addr,RTI_TRUE));

    route_key.send_resource.port = dst_addr->port;
    route_key.send_resource.address = dst_addr->value.guid;

    dbrc = DB_Table_select_match(
            self->_parent._rtable,
            DB_TABLE_DEFAULT_INDEX,
            (DB_Record_T*)&route_entry,
            (DB_Key_T) & route_key);

    if (existed != NULL)
    {
        *existed = (dbrc == DB_RETCODE_OK ? RTI_TRUE : RTI_FALSE);
    }

    if (dbrc == DB_RETCODE_NO_DATA)
    {
        return RTI_TRUE;
    }

    --route_entry->ref_count;
    if (route_entry->ref_count > 0)
    {
        return RTI_TRUE;
    }

    CoreShmTransport_destroy_send_resource(&route_entry->send_resource);

    route_entry = NULL;
    dbrc = DB_Table_remove_record(
            self->_parent._rtable,
            (DB_Record_T*)&route_entry,
            &route_key);
    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_SHMEM_LOG_RECORD(OSAPI_LOGKIND_ERROR, dbrc);
        return RTI_FALSE;
    }

    dbrc = DB_Table_delete_record(self->_parent._rtable, route_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_SHMEM_LOG_RECORD(OSAPI_LOGKIND_ERROR, dbrc);
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief Implementation of the NETIO bind function
 *
 * \details
 *
 * SHMEMInterface does not maintain any state information about its peer,
 * but this function is implemented to comply with the NETIO interface
 * and not burden a controller with knowing which interface maintains state
 * or not.
 *
 * \param[in]  netio_intf NETIO interface to bind
 * \param[in]  src_addr   The address to bind to
 * \param[in]  property   The property to use for the bind
 * \param[out] existed    Whether a previous bind existed or not
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_SHMEMInterface_bind(
        NETIO_Interface_T *netio_intf,
        struct NETIO_Address *src_addr,
        struct NETIOBindProperty *property,
        RTI_BOOL *existed)
{
    UNUSED_ARG(netio_intf);
    UNUSED_ARG(src_addr);
    UNUSED_ARG(property);

    if (existed != NULL)
    {
        *existed = RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief Implementation of the NETIO unbind function
 *
 * \details
 *
 * SHMEM does not maintain any state information about its peer, but this
 * function is implemented to comply with the NETIO interface and not burden
 * a controller with knowing which interface maintains state or not.
 *
 * \param[in]  netio_intf NETIO interface to bind
 * \param[in]  src_addr   The address to bind to
 * \param[in]  dst_intf   Interface
 * \param[out] existed    Whether a bind existed or not
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * \sa \ref NETIO_SHMEMInterface_bind
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_SHMEMInterface_unbind(
        NETIO_Interface_T *netio_intf,
        struct NETIO_Address *src_addr,
        NETIO_Interface_T *dst_intf,
        RTI_BOOL *existed)
{

    PRECOND_ARG(netio_intf)
    PRECOND_ARG(src_addr)
    PRECOND_ARG(dst_intf)

    OSAPI_PRECONDITION(
            (netio_intf == NULL) || (src_addr == NULL) || (dst_intf == NULL),
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("netio_intf",netio_intf,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("src_addr",src_addr,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("dst_intf",dst_intf,RTI_TRUE); )

    if (existed != NULL)
    {
        *existed = RTI_FALSE;
    }
    return RTI_TRUE;
}

/*ci
 * \brief Implementation of the NETIO bind_external function
 *
 * \details
 *
 * When an upstream interface want to listen to a logger interface it
 * binds to the downstream interface using the external bind function.
 * For logger this means adding an interface to a bind table so
 * when send is called all the bound interfaces are called one by one.
 * Note that the logger interface is a synchronous interface.
 *
 * \param[in]  self       NETIO interface to bind to upstream interface
 * \param[in]  src_addr   The address to bind to
 * \param[in]  dst_intf   The upstream interface
 * \param[in]  dst_addr   The address to pass to the upstream interface
 * \param[in]  property   The properties for the bind
 * \param[out] existed    Whether a previous bind already existed for this entry
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * \sa \ref NETIO_SHMEMInterface_unbind_external
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_SHMEMInterface_bind_external(
        NETIO_Interface_T *self,
        struct NETIO_Address *src_addr,
        NETIO_Interface_T *dst_intf,
        struct NETIO_Address *dst_addr,
        struct NETIOBindProperty *property,
        RTI_BOOL *existed)
{
    struct NETIO_SHMEMInterface *src_intf = (struct
            NETIO_SHMEMInterface*)self;
    struct SHMEMBindEntry *bind_entry = NULL;
    struct SHMEMPortEntry *port_entry = NULL;
    DB_ReturnCode_T dbrc;
    struct NETIOBindEntryKey bind_key;
    struct NETIO_Address src_address;

    UNUSED_ARG(property);

    OSAPI_PRECONDITION(
            (src_intf == NULL) || (src_addr == NULL) || (dst_intf == NULL) ||
            (dst_addr == NULL),
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("src_intf",src_intf,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("src_addr",src_addr,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("dst_intf",dst_intf,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("dst_addr",dst_addr,RTI_TRUE); )

    if (existed)
    {
        *existed = RTI_FALSE;
    }

    src_address = *src_addr;

    /* Find the receive entry. It should already have been reserved during
     * reserve_address(). If it is not, then we cannot bind to it
     */
    dbrc = DB_Table_select_match(
            src_intf->shmem_receive_thread_table,
            DB_TABLE_DEFAULT_INDEX,
            (DB_Record_T*)&port_entry,
            (DB_Key_T) & src_address);
    if (dbrc != DB_RETCODE_OK)
    {
        OSAPI_TRACE_NET("address not found:", RTI_FALSE)
        OSAPI_TRACE_INT32("port", (RTI_INT32)src_address.port, RTI_FALSE)
        OSAPI_TRACE_GUID("address", &src_address.value.rtps_guid, RTI_TRUE)
        return RTI_FALSE;
    }

    bind_key.source = src_address;
    bind_key.destination = *dst_addr;

    /* If a bind entry exists in the bind table, increment its reference count.
     * Otherwise, create it and insert it
     */
    dbrc = DB_Table_select_match(
            src_intf->_parent._btable,
            DB_TABLE_DEFAULT_INDEX,
            (DB_Record_T*)&bind_entry,
            (DB_Key_T) & bind_key);
    if (dbrc == DB_RETCODE_OK)
    {
        if (existed)
        {
            *existed = RTI_TRUE;
        }
        ++bind_entry->ref_count;
        return RTI_TRUE;
    }

    dbrc = DB_Table_create_record(
            src_intf->_parent._btable,
            (DB_Record_T*)&bind_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_SHMEM_LOG_RECORD(OSAPI_LOGKIND_ERROR, dbrc);
        return RTI_FALSE;
    }

    bind_entry->_parent.source = src_address;
    bind_entry->_parent.destination = *dst_addr;
    bind_entry->_parent.intf = dst_intf;
    bind_entry->ref_count = 1;

    dbrc = DB_Table_insert_record(
            src_intf->_parent._btable,
            (DB_Record_T)bind_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_SHMEM_LOG_RECORD(OSAPI_LOGKIND_ERROR, dbrc);
        (void)DB_Table_delete_record(
                src_intf->_parent._btable,
                (DB_Record_T)bind_entry);
        return RTI_FALSE;
    }

    port_entry->_ref_count++;

    if (port_entry->_ref_count > 1)
    {
        return RTI_TRUE;
    }

    return OSAPI_Thread_start(port_entry->_rx_thread);
}

/*ci
 * \brief  Implementation of the NETIO unbind_external function
 *
 * \details
 *
 * When an upstream interface want to remove a listener to a logger interface
 * it unbinds to the downstream interface using the external unbind function.
 *
 * \param[in]  src_intf   NETIO interface to unbind from upstream interface
 * \param[in]  src_addr   The address to unbind from
 * \param[in]  dst_intf   The upstream interface
 * \param[in]  dst_addr   The address to passed to the upstream interface
 * \param[out] existed    Whether a previous bind already existed for this entry
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * \sa \ref NETIO_SHMEMInterface_bind_external
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_SHMEMInterface_unbind_external(
        NETIO_Interface_T *netio_intf,
        struct NETIO_Address *src_addr,
        NETIO_Interface_T *dst_intf,
        struct NETIO_Address *dst_addr,
        RTI_BOOL *existed)
{
    struct NETIO_SHMEMInterface *self =
            (struct NETIO_SHMEMInterface*)netio_intf;
    struct SHMEMBindEntry *bind_entry = NULL;
    struct NETIOBindEntryKey bind_key;
    struct SHMEMPortEntry *port_entry = NULL;
    struct NETIO_Address src_address;
    DB_ReturnCode_T dbrc;

    PRECOND_ARG(dst_intf)

    OSAPI_PRECONDITION(
            (netio_intf == NULL) || (src_addr == NULL) || (dst_intf == NULL) ||
            (dst_addr == NULL),
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("src_intf",netio_intf,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("src_addr",src_addr,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("dst_intf",dst_intf,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("dst_addr",dst_addr,RTI_TRUE); )

    OSAPI_TRACE_NET("remove forwarding of data:", RTI_FALSE)
    OSAPI_TRACE_INT32("src.port", (RTI_INT32)src_addr->port, RTI_FALSE)
    OSAPI_TRACE_GUID("src.address", &src_addr->value.rtps_guid, RTI_FALSE)
    OSAPI_TRACE_INT32("dst.port", (RTI_INT32)dst_addr->port, RTI_FALSE)
    OSAPI_TRACE_GUID("dst.address", &dst_addr->value.rtps_guid, RTI_TRUE)

    if (existed)
    {
        *existed = RTI_FALSE;
    }

    src_address = *src_addr;

    bind_key.source = src_address;
    bind_key.destination = *dst_addr;

    dbrc = DB_Table_select_match(
            self->_parent._btable,
            DB_TABLE_DEFAULT_INDEX,
            (DB_Record_T*)&bind_entry,
            (DB_Key_T) & bind_key);
    if (dbrc != DB_RETCODE_OK)
    {
        OSAPI_TRACE_NET("address does not exist", RTI_FALSE)
        OSAPI_TRACE_INT32("kind", NETIO_Address_get_kind(src_addr), RTI_FALSE)
        OSAPI_TRACE_INT32("port", (RTI_INT32)src_addr->port, RTI_FALSE)
        OSAPI_TRACE_INT32("address", (RTI_INT32)src_addr->value.ipv4.address, RTI_TRUE)

        return RTI_TRUE;
    }

    if (existed)
    {
        *existed = RTI_TRUE;
    }

    --bind_entry->ref_count;

    OSAPI_TRACE_NET("remove forwarding of data:", RTI_FALSE)
    OSAPI_TRACE_INT32("ref_count", bind_entry->ref_count, RTI_FALSE)
    OSAPI_TRACE_INT32("src.port", (RTI_INT32)src_addr->port, RTI_FALSE)
    OSAPI_TRACE_GUID("src.address", &src_addr->value.rtps_guid, RTI_FALSE)
    OSAPI_TRACE_INT32("dst.port", (RTI_INT32)dst_addr->port, RTI_FALSE)
    OSAPI_TRACE_GUID("dst.address", &dst_addr->value.rtps_guid, RTI_TRUE)

    if (bind_entry->ref_count > 0)
    {
        return RTI_TRUE;
    }

    bind_entry = NULL;
    dbrc = DB_Table_remove_record(
            self->_parent._btable,
            (DB_Record_T*)&bind_entry,
            (DB_Key_T) & bind_key);
    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_SHMEM_LOG_RECORD(OSAPI_LOGKIND_ERROR, dbrc)
        return RTI_FALSE;
    }

    dbrc = DB_Table_delete_record(
            self->_parent._btable,
            (DB_Record_T)bind_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_SHMEM_LOG_RECORD(OSAPI_LOGKIND_WARNING, dbrc)
        return RTI_FALSE;
    }

    OSAPI_TRACE_NET("deleted forwarding of data:", RTI_FALSE)
    OSAPI_TRACE_INT32("src.port", (RTI_INT32)src_addr->port, RTI_FALSE)
    OSAPI_TRACE_GUID("src.address", &src_addr->value.rtps_guid, RTI_FALSE)
    OSAPI_TRACE_INT32("dst.port", (RTI_INT32)dst_addr->port, RTI_FALSE)
    OSAPI_TRACE_GUID("dst.address", &dst_addr->value.rtps_guid, RTI_TRUE)

    /* Find the receive entry */
    dbrc = DB_Table_select_match(
            self->shmem_receive_thread_table,
            DB_TABLE_DEFAULT_INDEX,
            (DB_Record_T*)&port_entry,
            (DB_Key_T) & src_address);
    if (dbrc != DB_RETCODE_OK)
    {
        NETIO_SHMEM_LOG_RECORD(OSAPI_LOGKIND_ERROR, dbrc);
        return RTI_FALSE;
    }

    --port_entry->_ref_count;

    OSAPI_TRACE_NET("port unbound:", RTI_FALSE)
    OSAPI_TRACE_INT32("ref_count", (RTI_INT32)port_entry->_ref_count, RTI_FALSE)
    OSAPI_TRACE_INT32("kind", NETIO_Address_get_kind(src_addr), RTI_FALSE)
    OSAPI_TRACE_INT32("port", (RTI_INT32)src_addr->port, RTI_FALSE)
    OSAPI_TRACE_INT32("address", (RTI_INT32)src_addr->value.ipv4.address, RTI_TRUE)

    return RTI_TRUE;

}

/*ci
 * \brief Implementation of the NETIO bind function
 *
 * \details
 *
 * When a logger interface is bound to a downstream interface it is
 * requested to provide which interface and address the downstream interface
 * should use when forwarding a NETIO_Packet.
 *
 * \param[in]   netio_intf   NETIO interface to get the external interface for
 * \param[in]   src_addr     The address to send to
 * \param[out]  dst_intf     The interface to use
 * \param[out]  dst_addr     The destination address to use
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_SHMEMInterface_get_external_interface(
        NETIO_Interface_T *netio_intf,
        struct NETIO_Address *src_addr,
        NETIO_Interface_T **dst_intf,
        struct NETIO_Address *dst_addr)
{
    UNUSED_ARG(netio_intf);
    UNUSED_ARG(src_addr);
    UNUSED_ARG(dst_addr);
    UNUSED_ARG(dst_intf);
    return RTI_TRUE;
}

/*ci
 * \brief Implementation of the NETIO set_state function
 *
 * \details
 *
 * \param[in] src_intf NETIO interface to set state on
 * \param[in] state    New state
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_SHMEMInterface_set_state(
        NETIO_Interface_T *self,
        NETIO_InterfaceState_T state)
{
    struct NETIO_SHMEMInterface *src_intf = (struct
            NETIO_SHMEMInterface*)self;
    OSAPI_PRECONDITION(
            (self == NULL),
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("src_intf",self,RTI_TRUE); )

    src_intf->_parent.state = state;
    return RTI_TRUE;
}

/*ci
 * \brief Implementation of the NETIO resolve_address function
 *
 * \details
 *
 * The logger interface does currently not support address conversion
 * and will mark any address as invalid
 *
 * \param[out] self           Interface
 * \param[in]  address_string Address to convert
 * \param[out] address_value  Converted address on success
 * \param[out] is_invalid     Whether the address is valid or not
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_SHMEMInterface_resolve_address(
        NETIO_Interface_T *netio_intf,
        const char *address_string,
        struct NETIO_Address *address_value,
        RTI_BOOL *is_invalid)
{
    UNUSED_ARG(netio_intf);
    UNUSED_ARG(address_string);
    UNUSED_ARG(address_value);
    UNUSED_ARG(is_invalid);

    /* No address format is specified, thus no string is invalid */
    *is_invalid = RTI_FALSE;

    NETIO_Address_init(address_value, NETIO_ADDRESS_KIND_SHMEM);

    if (address_string[0] == 0)
    {
        return RTI_TRUE;
    }

    return RTI_FALSE;
}

/*ci
 * \brief Implementation of the NETIO release_address function
 *
 * \param[in] self     NETIO interface to release addresses on
 * \param[in] address  Address to release
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 *
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_SHMEMInterface_release_address(
        NETIO_Interface_T *self,
        struct NETIO_Address *address)
{
    struct NETIO_SHMEMInterface *src_intf = (struct
            NETIO_SHMEMInterface*)self;
    struct NETIO_Address release_addr;
    struct SHMEMPortEntry *port_entry = NULL;
    DB_ReturnCode_T dbrc;
    RTI_BOOL retval = RTI_FALSE;

    release_addr = *address;

    OSAPI_TRACE_NET("release address:", RTI_FALSE)
    OSAPI_TRACE_INT32("kind", NETIO_Address_get_kind(&release_addr), RTI_FALSE)
    OSAPI_TRACE_INT32("port", (RTI_INT32)release_addr.port, RTI_FALSE)
    OSAPI_TRACE_INT32("address", (RTI_INT32)release_addr.value.ipv4.address, RTI_TRUE)

    /* Lock */
    if (DB_Database_lock(src_intf->property._parent.db) != DB_RETCODE_OK)
    {
        return RTI_FALSE;
    }

    dbrc = DB_Table_select_match(
            src_intf->shmem_receive_thread_table,
            DB_TABLE_DEFAULT_INDEX,
            (DB_Record_T*)&port_entry,
            (DB_Key_T) & release_addr);

    if (dbrc == DB_RETCODE_NO_DATA)
    {
        OSAPI_TRACE_NET("release address not found:", RTI_FALSE)
        OSAPI_TRACE_INT32(
                "kind",
                NETIO_Address_get_kind(&release_addr),
                RTI_FALSE)
        OSAPI_TRACE_INT32("port", (RTI_INT32)release_addr.port, RTI_FALSE)
        OSAPI_TRACE_INT32("address", (RTI_INT32)release_addr.value.ipv4.address, RTI_TRUE)
        goto done;
    }

    if (dbrc != DB_RETCODE_OK)
    {
        OSAPI_TRACE_NET("release error:", RTI_FALSE)
        OSAPI_TRACE_INT32("dbrc", dbrc, RTI_FALSE)
        OSAPI_TRACE_INT32(
                "kind",
                NETIO_Address_get_kind(&release_addr),
                RTI_FALSE)
        OSAPI_TRACE_INT32("port", (RTI_INT32)release_addr.port, RTI_FALSE)
        OSAPI_TRACE_INT32("address", (RTI_INT32)release_addr.value.ipv4.address, RTI_TRUE)
        goto done;
    }

    if (port_entry->_ref_count != 0)
    {
        OSAPI_TRACE_NET("trying to release address in use:", RTI_FALSE)
        OSAPI_TRACE_INT32("ref_count", (RTI_INT32)port_entry->_ref_count, RTI_FALSE)
        OSAPI_TRACE_INT32(
                "kind",
                NETIO_Address_get_kind(&release_addr),
                RTI_FALSE)
        OSAPI_TRACE_INT32("port", (RTI_INT32)release_addr.port, RTI_FALSE)
        OSAPI_TRACE_INT32("address", (RTI_INT32)release_addr.value.ipv4.address, RTI_TRUE)
        goto done;
    }

    OSAPI_TRACE_NET("delete entry:", RTI_FALSE)
    OSAPI_TRACE_INT32("kind", NETIO_Address_get_kind(&release_addr), RTI_FALSE)
    OSAPI_TRACE_INT32("port", (RTI_INT32)release_addr.port, RTI_FALSE)
    OSAPI_TRACE_INT32("address", (RTI_INT32)release_addr.value.ipv4.address, RTI_TRUE)

    port_entry = NULL;
    dbrc = DB_Table_remove_record(
            src_intf->shmem_receive_thread_table,
            (DB_Record_T*)&port_entry,
            (DB_Key_T) & release_addr);

    if (dbrc != DB_RETCODE_OK)
    {
        OSAPI_TRACE_NET("release address not found:", RTI_FALSE)
        OSAPI_TRACE_INT32(
                "kind",
                NETIO_Address_get_kind(&release_addr),
                RTI_FALSE)
        OSAPI_TRACE_INT32("port", (RTI_INT32)release_addr.port, RTI_FALSE)
        OSAPI_TRACE_INT32("address", (RTI_INT32)release_addr.value.ipv4.address, RTI_TRUE)
        goto done;
    }

    /* Unlock */
    if (DB_Database_unlock(src_intf->property._parent.db) != DB_RETCODE_OK)
    {
        return RTI_FALSE;
    }

    if (port_entry->_rx_thread != NULL)
    {
        if (!OSAPI_Thread_destroy(port_entry->_rx_thread))
        {
            return RTI_FALSE;
        }
        port_entry->_rx_thread = NULL;
    }

    CoreShmTransport_destroy_recv_resource(
            port_entry->_recv_resource);

    if (DB_Database_lock(src_intf->property._parent.db) != DB_RETCODE_OK)
    {
        return RTI_FALSE;
    }

    dbrc = DB_Table_delete_record(
            src_intf->shmem_receive_thread_table,
            (DB_Record_T)port_entry);

    retval = (dbrc == DB_RETCODE_OK ? RTI_TRUE : RTI_FALSE);

done:

    if (DB_Database_unlock(src_intf->property._parent.db) != DB_RETCODE_OK)
    {
        return RTI_FALSE;
    }

    return retval;
}

/*ci
 * \brief Implementation of the NETIO get_route_table function
 *
 * \param[in]    netio_intf The NETIO interface
 * \param[inout] address    Sequence of NETIO addresses this interface understands
 * \param[inout] netmask    Sequence of the corresponding netmasks
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_SHMEMInterface_add_route_entry(
        struct NETIO_AddressSeq *address_seq,
        struct NETIO_NetmaskSeq *netmask_seq,
        RTI_INT32 address,
        RTI_INT32 netmask,
        RTI_INT32 netmask_bits)
{
    struct NETIO_Address src_address = NETIO_Address_INITIALIZER;
    struct NETIO_Address *address_ref = NULL;
    struct NETIO_Netmask src_netmask = NETIO_Netmask_INITIALIZER;
    struct NETIO_Netmask *netmask_ref = NULL;
    RTI_INT32 len;
    UNUSED_ARG(address);
    UNUSED_ARG(netmask);
    UNUSED_ARG(netmask_bits);

    len = NETIO_AddressSeq_get_length(address_seq);
    if (len >= NETIO_AddressSeq_get_maximum(address_seq))
    {
        return RTI_FALSE;
    }

    if (!NETIO_AddressSeq_set_length(address_seq, len + 1))
    {
        return RTI_FALSE;
    }

    if (!NETIO_NetmaskSeq_set_length(netmask_seq, len + 1))
    {
        (void)NETIO_AddressSeq_set_length(address_seq, len);
        return RTI_FALSE;
    }

    NETIO_Address_init(&src_address, NETIO_ADDRESS_KIND_SHMEM);

    address_ref = NETIO_AddressSeq_get_reference(address_seq, len);
    if (address_ref == NULL)
    {
        return RTI_FALSE;
    }

    *address_ref = src_address;

    src_netmask.bits = 0;
    src_netmask.mask[0] = 0;
    src_netmask.mask[1] = 0;
    src_netmask.mask[2] = 0;
    src_netmask.mask[3] = 0;

    netmask_ref = NETIO_NetmaskSeq_get_reference(netmask_seq, len);
    if (netmask_ref == NULL)
    {
        return RTI_FALSE;
    }

    *netmask_ref = src_netmask;

    return RTI_TRUE;
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_SHMEMInterface_get_route_table(
        NETIO_Interface_T *self,
        struct NETIO_AddressSeq *address,
        struct NETIO_NetmaskSeq *netmask)
{
    RTI_BOOL rval = RTI_FALSE;
    RTI_INT32 max_size;
    RTI_INT32 cur_addr_len;
    UNUSED_ARG(self);

    cur_addr_len = NETIO_AddressSeq_get_length(address);
    max_size = NETIO_AddressSeq_get_maximum(address) - cur_addr_len;

    if (max_size == 0)
    {
        return RTI_TRUE;
    }

    if (!NETIO_SHMEMInterface_add_route_entry(address, netmask, 0, 0, 0))
    {
        /* Route all NETIO_SHMEM addresses */
        return RTI_FALSE;
    }

    rval = RTI_TRUE;
    return rval;
}

/*ci
 * \brief The NETIO logger interface implementation
 *
 * \details
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_SHMEMInterface_acknack(
        NETIO_Interface_T *netio_intf,
        struct NETIO_Address *source,
        NETIO_PacketId_T *packet_id,
        RTI_BOOL nack)
{
    UNUSED_ARG(netio_intf);
    UNUSED_ARG(source);
    UNUSED_ARG(packet_id);
    UNUSED_ARG(nack);
    return RTI_TRUE;
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_SHMEMInterface_request(
        NETIO_Interface_T *netio_intf,
        struct NETIO_Address *source,
        struct NETIO_Address *dest,
        NETIO_Packet_T **packet,
        NETIO_PacketId_T *packet_id,
        NETIO_PacketId_T *actual_packet_id)
{

    UNUSED_ARG(netio_intf);
    UNUSED_ARG(source);
    UNUSED_ARG(dest);
    UNUSED_ARG(packet);
    UNUSED_ARG(packet_id);
    UNUSED_ARG(actual_packet_id);
    return RTI_TRUE;
}

/*ci
 * \brief SHMEM receive function
 *
 * \details
 *
 * This shared memory interface does not have a downstream interface.
 * Thus, this function is not called from another NETIO interface but from the
 * internal shared memory receive thread. Whenever the
 * shared memory receive thread receives a packet it calls this function to pass
 * the packet to upstream NETIO interfaces bound to this interface.
 *
 * \param[in] netio_intf NETIO interface to receive on
 * \param[in] source     The source NETIO address of the packet
 * \param[in] packet     The packet to process
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_SHMEMInterface_receive(
        NETIO_Interface_T *self,
        struct NETIO_Address *source,
        NETIO_Packet_T *packet)
{
    struct NETIO_SHMEMInterface *shmem_intf =
            (struct NETIO_SHMEMInterface*)self;
    DB_Cursor_T cursor = NULL;
    struct SHMEMBindEntry *bind_entry;
    DB_ReturnCode_T dbrc;
    RTI_SIZE_T pkt_head, pkt_tail;
    RTI_BOOL retval = RTI_FALSE;
    RTI_BOOL bretval;
    UNUSED_ARG(source);

    if (DB_Database_lock(shmem_intf->property._parent.db) != DB_RETCODE_OK)
    {
        return RTI_FALSE;
    }

    if (!OSAPI_Mutex_take(shmem_intf->property.network_lock))
    {
        return RTI_FALSE;
    }

    NETIO_Packet_save_positions_to(packet, &pkt_head, &pkt_tail);

    /* This logic was modeled after UDP interface. In the case of Shared Memory,
     * however, there is only expected to be on receive address.
     */
    dbrc = DB_Table_select_all(
            shmem_intf->_parent._btable,
            DB_TABLE_DEFAULT_INDEX,
            &cursor);
    if (dbrc == DB_RETCODE_OK)
    {
        dbrc = DB_Cursor_get_next(cursor, (DB_Record_T*)&bind_entry);

        while (dbrc == DB_RETCODE_OK)
        {
            bretval = NETIO_Interface_receive(
                    bind_entry->_parent.intf,
                    &shmem_intf->_parent.local_address,
                    &bind_entry->_parent.destination,
                    packet);

            UNUSED_ARG(bretval);

            NETIO_Packet_restore_positions_from(packet, pkt_head, pkt_tail);

            /* Force the while loop to terminate cleanly. All received SHMEM
             * traffic is forwarded upstream to the same RTPS interface.
             * This is by design. future versions may forward the same
             * packet to multiple upstream interfaces.
             */
            dbrc = DB_RETCODE_NO_DATA;
        }
        DB_Cursor_finish(shmem_intf->_parent._btable, cursor);

        if (dbrc != DB_RETCODE_NO_DATA)
        {
            NETIO_SHMEM_LOG_CURSOR_ERROR (OSAPI_LOGKIND_ERROR)
            goto done;
        }

        retval = RTI_TRUE;
    }

done:

    if (!OSAPI_Mutex_give(shmem_intf->property.network_lock))
    {
        return RTI_FALSE;
    }

    if (DB_Database_unlock(shmem_intf->property._parent.db)
            != DB_RETCODE_OK)
    {
        return RTI_FALSE;
    }

    return retval;

}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_SHMEMInterface_receive_thread(
        struct OSAPI_ThreadInfo *thread_info)
{
    struct SHMEMPortEntry *port_entry =
                            (struct SHMEMPortEntry*)thread_info->user_data;
    struct NETIO_Packet packet_to_send_upstream;
    char *read_buffer = NULL;
    RTI_INT32 concurrent_q_handle;
    RTI_INT32 read_buffer_length;

    while (!thread_info->stop_thread)
    {
        /*
         * Try to receive data from shared memory
         */
        if (!CoreShmTransport_recv_from(
                port_entry->_shm_transport,
                port_entry->_recv_resource,
                &read_buffer,
                &concurrent_q_handle,
                &read_buffer_length))
        {
            continue;
        }

        /* Check length to see if any data was actually received */
        if (read_buffer_length <= 0)
        {
            continue;
        }

        if (!NETIO_Packet_initialize(
                &packet_to_send_upstream,
                read_buffer,
                (RTI_SIZE_T)read_buffer_length,
                0,
                NULL))
        {
            NETIO_SHMEM_LOG_PACKET_INIT(
                    OSAPI_LOGKIND_ERROR,
                    &packet_to_send_upstream,
                    read_buffer,
                    read_buffer_length);
            continue;
        }

        if (!NETIO_Packet_set_head(
                &packet_to_send_upstream,
                0 -
                read_buffer_length))
        {
            NETIO_SHMEM_LOG_PACKET_HEAD(
                    OSAPI_LOGKIND_ERROR,
                    &packet_to_send_upstream,
                    0 - read_buffer_length);
            continue;
        }
        packet_to_send_upstream.info.originating_transport =
                NETIO_ADDRESS_KIND_SHMEM;

        if (!NETIO_SHMEMInterface_receive(
                (NETIO_Interface_T*)port_entry->_shm_intf,
                NULL,
                &packet_to_send_upstream))
        {
            NETIO_SHMEM_LOG_PACKET_FWD(OSAPI_LOGKIND_ERROR);
        }

        /* Packet is processed by upper layer. Allow its space to be
         * reused inside the shared memory segment
         */
        NETIO_SHMEM_ConcurrentQueue_finish_read_ea(
                &port_entry->_recv_resource->concurrent_q,
                concurrent_q_handle);
    }

    return RTI_TRUE;
}

/*ci
 * \brief Bind a sequence of NETIO addresses
 *
 * \details
 *
 * Before SHMEM ports can be listened on they must be bound to the port.
 * This function takes a sequence of addresses, and the index to start at,
 * and creates a new port entry for each NETIO_Address to listen to.
 *
 * \param[in]  src_intf    SHMEM interface to bind entries on
 * \param[in]  start_index The first index in the sequence to listen to
 * \param[in]  addresses   Sequence of addresses to bind to
 * \param[out] property    The properties of the bind
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa ref \UDP_Interface_create_bind_entry
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_SHMEMInterface_bind_addresses(
        struct NETIO_SHMEMInterface *shmem_interface,
        RTI_INT32 start_index,
        struct NETIO_AddressSeq *addresses,
        struct NETIOBindProperty *property)
{
    RTI_INT32 i = 0;
    RTI_INT32 length = 0;
    RTI_INT32 success = RTI_TRUE;
    /* RTI_INT32 return_status; */
    DB_ReturnCode_T dbrc;
    struct NETIO_Address *out_address = NULL;
    struct SHMEM_RecvResource *shmem_receive_resource = NULL;
    char *thread_name = "shmem_uc";
    struct SHMEMPortEntry *port_entry = NULL;

    UNUSED_ARG(property);

    length = NETIO_AddressSeq_get_length(addresses);
    for (i = start_index; i < length; ++i)
    {
        out_address = NETIO_AddressSeq_get_reference(addresses, i);

        if (!CoreShmTransport_create_recv_resource(
                shmem_interface->shm_transport,
                &shmem_receive_resource,
                (RTI_INT32*)&out_address->port))
        {
            success = RTI_FALSE;
            break;
        }

        /*
        if (!NETIO_SharedMemoryMutex_lock(
                &shmem_receive_resource->_shm_mutex,
                &return_status))
        {
            NETIO_SHMEM_LOG_SHMEM_MUTEX_LOCK_FAILED(OSAPI_LOGKIND_ERROR, return_status);
        }*/

        OSAPI_Memory_copy(
                &shmem_receive_resource->_shm_transport_hdr->domain_participant_guid,
                &shmem_interface->property.intf_addr.value,
                16);

        /* Set the last 4 bytes to 0 as that is what Pro expects */
        shmem_receive_resource->_shm_transport_hdr->domain_participant_guid[3] = 0;

        /*
        if (!NETIO_SharedMemoryMutex_unlock(
                &shmem_receive_resource->_shm_mutex,
                &return_status))
        {
            NETIO_SHMEM_LOG_SHMEM_MUTEX_UNLOCK_FAILED(OSAPI_LOGKIND_ERROR, return_status);
        }
        */

        /*
         * Create the port entry
         */
        port_entry = NULL;

        dbrc = DB_Table_create_record(
                shmem_interface->shmem_receive_thread_table,
                (DB_Record_T*)&port_entry);
        if (dbrc != DB_RETCODE_OK)
        {
            NETIO_SHMEM_LOG_RECORD(OSAPI_LOGKIND_ERROR, dbrc);
        }

        OSAPI_Memory_zero(port_entry, sizeof(struct SHMEMPortEntry));

        port_entry->_shm_transport = shmem_interface->shm_transport;
        port_entry->_recv_resource = shmem_receive_resource;
        port_entry->_shm_intf = shmem_interface;
        port_entry->source = *out_address;
        port_entry->_rx_buffer.max_length =
                (RTI_SIZE_T)port_entry->_shm_intf->factory->property.message_size_max;

        dbrc = DB_Table_insert_record(
                shmem_interface->shmem_receive_thread_table,
                (DB_Record_T)port_entry);

        if ((dbrc != DB_RETCODE_OK) && (dbrc != DB_RETCODE_EXISTS))
        {
            NETIO_SHMEM_LOG_RECORD(OSAPI_LOGKIND_ERROR, dbrc);
        }

        port_entry->_rx_thread = OSAPI_Thread_create(
                thread_name,
                &shmem_interface->factory->property.recv_thread_property,
                NETIO_SHMEMInterface_receive_thread,
                port_entry,
                NETIO_SHMEMInterface_wakeup_receive_thread);
    }

    return success;
}

/*ci
 * \brief Implementation of the NETIO reserve_address function
 *
 *
 * \param[in]    self       NETIO interface to reserve addresses on
 * \param[in]    req_addr   List of requested addresses
 * \param[inout] resvd_addr The downstream interface
 * \param[in]    property   Properties to use to listen on the reserved addresses
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * \sa \ref NETIO_SHMEMInterface_release_address
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_SHMEMInterface_reserve_address(
        NETIO_Interface_T *self,
        struct NETIO_AddressSeq *req_addrs,
        struct NETIO_AddressSeq *reserved_addrs,
        struct NETIOBindProperty *property)
{
    struct NETIO_SHMEMInterface *shmem_interface =
            (struct NETIO_SHMEMInterface*)self;
    struct NETIO_Address *req_addr = NULL;
    struct NETIO_Address *out_address = NULL;
    RTI_BOOL success = RTI_FALSE;
    RTI_INT32 k = 0;
    RTI_INT32 j = 0;
    RTI_INT32 cur_addr_len = NETIO_AddressSeq_get_length(reserved_addrs);
    RTI_INT32 start_index = cur_addr_len;
    RTI_INT32 max_size = NETIO_AddressSeq_get_maximum(reserved_addrs) - cur_addr_len;
    RTI_INT32 rsv_count = 0;

    while ((k < NETIO_AddressSeq_get_length(req_addrs)) && (rsv_count < max_size))
    {
        req_addr = NETIO_AddressSeq_get_reference(req_addrs, k);

        if (NETIO_Address_get_kind(req_addr) != NETIO_ADDRESS_KIND_SHMEM)
        {
            continue;
        }

        /* Don't duplicate addresses */
        for (j = 0; j < NETIO_AddressSeq_get_length(reserved_addrs); ++j)
        {
            /* Don't check the address as it is not used in reservations */
            if ((NETIO_AddressSeq_get_reference(reserved_addrs, j)->port == req_addr->port) &&
                (NETIO_AddressSeq_get_reference(reserved_addrs, j)->kind == req_addr->kind))
            {
                break;
            }
        }

        if (j < NETIO_AddressSeq_get_length(reserved_addrs))
        {
            OSAPI_TRACE_NET("ignoring duplicate address:", RTI_FALSE)
            OSAPI_TRACE_INT32("kind",NETIO_Address_get_kind(req_addr),RTI_FALSE)
            OSAPI_TRACE_INT32("port", (RTI_INT32)req_addr->port, RTI_FALSE)
            OSAPI_TRACE_INT32("address", (RTI_INT32)req_addr->value.ipv4.address, RTI_TRUE)
            k++;
            continue;
        }

        NETIO_AddressSeq_set_length(reserved_addrs, cur_addr_len + k + 1);
        out_address = NETIO_AddressSeq_get_reference(reserved_addrs,
                                                     start_index + k);

        out_address->kind = NETIO_ADDRESS_KIND_SHMEM;
        out_address->port = req_addr->port;

        OSAPI_Memory_copy(&out_address->value.rtps_guid,
                          &shmem_interface->property.intf_addr.value.rtps_guid,
                          16);

        /* Set the last 4 bytes to 0 as that is what Pro expects */
        out_address->value.rtps_guid.object_id = 0;

        rsv_count++;
        k++;
    }

    if (!NETIO_SHMEMInterface_bind_addresses(shmem_interface,
                                             start_index,
                                             reserved_addrs,
                                             property))
    {
        goto done;
    }

    success = RTI_TRUE;

done:

    return success;
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_SHMEMInterface_post_event(NETIO_Interface_T *netio_intf,
                                NETIO_Interface_T *src_intf,
                                struct NETIO_Event *evt)
{
    UNUSED_ARG(netio_intf);
    UNUSED_ARG(src_intf);
    UNUSED_ARG(evt);
    return RTI_TRUE;
}

/*ci
 * \brief Check if a shared memory transport locator is reachable
 *
 * \details
 * This function determines whether a shared memory transport locator
 * originates from the same node or not and is used to choose between
 * the shared memory transport and another for communication between
 * nodes on the same host.
 *
 * 1. The address->port is the key of the segment to attach to. If the
 *    attachment fails then this function shall return RTI_TRUE and
 *    set is_reachable to RTI_FALSE.
 *
 * 2. If the attachment succeeds, but the owner field does not match the
 *    locator address this function shall return RTI_TRUE and set
 *    is_reachable to RTI_FALSE.
 *
 * 3. If the attachment succeeds and the owner field matches the locator
 *    address this function shall return RTI_TRUE and set is_reachable
 *    to RTI_TRUE.
 *
 * 4. If for any reason this function is unable to perform a valid check it
 *    shall return RTI_FALSE. The caller shall ignore the value of
 *    is_reachable.
 *
 * \return RTI_TRUE if the function was successful. The result of the test
 *         can be found in the output variable is_reachable. If is_reachable
 *         is RTI_FALSE then the locator cannot be reached. If the function
 *         returns RTI_FALSE the value of is_reachable is undefined and cannot
 *         be trusted.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
NETIO_SHMEMInterface_is_address_reachable(struct NETIO_Interface *self,
                                          const struct NETIO_AddressEx *const address,
                                          RTI_BOOL *is_reachable)
{

    struct NETIO_SHMEMTransportHdrShared *shmem_header = NULL;
    struct NETIO_SHMEMInterface *shm_intf = (struct NETIO_SHMEMInterface*)self;
    struct NETIO_SharedMemorySegmentHandle segment_hndl;
    struct SHMEMRouteEntry locator_key;
    struct SHMEMRouteEntry *new_locator_entry = NULL;
    struct NETIO_SHMEMTransportProperty *property =
            &shm_intf->shm_transport->_property;
    DB_ReturnCode_T dbrc;
    RTI_INT32 attach_status = 0;
    RTI_BOOL function_success = RTI_TRUE;
    RTI_BOOL success;
    RTI_INT32 key;

    locator_key.send_resource.port = address->port;
    locator_key.send_resource.address = address->value.guid;
    dbrc = DB_Table_select_match(self->_rtable,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&new_locator_entry,
                                 (DB_Key_T) & locator_key);

    if (dbrc == DB_RETCODE_OK)
    {
        *is_reachable = RTI_TRUE;
        goto done;
    }

    /* It does not exist in our cached table. Try attaching */
    key = NETIO_SHMEMTransport_compute_segment_key(property,
                                                   (RTI_INT32)address->port);

    success = NETIO_SharedMemorySegment_attach(&segment_hndl,
                                               &attach_status,
                                               key);

    if (!success)
    {
        goto done;
    }

    if (attach_status != OSAPI_SHARED_MEMORY_ATTACHED)
    {
        goto done;
    }

    shmem_header = OSAPI_Compiler_reinterpret_cast(
                        struct NETIO_SHMEMTransportHdrShared*,
                        NETIO_SharedMemorySegment_get_address(&segment_hndl));

    if (OSAPI_Memory_compare(&shmem_header->domain_participant_guid,
                             &address->value.guid,
                             sizeof(address->value.guid)) != 0)
    {
        /* Failed to validate that the address in the received locator matches
         * the address in the shared memory segment with key=port that we
         * attached to
         */
        goto done;
    }

    *is_reachable = RTI_TRUE;

done:

    if (attach_status == OSAPI_SHARED_MEMORY_ATTACHED)
    {
        NETIO_SharedMemorySegment_detach(&segment_hndl);
    }

    return function_success;
}

RTI_PRIVATE void
NETIO_SHMEMInterface_get_transport_properties(struct NETIO_Interface *netio_intf,
                                              struct NETIO_TransportProperty *properties)
{
    struct NETIO_SHMEMInterface *shmem_intf =
                                      (struct NETIO_SHMEMInterface*)netio_intf;

    properties->recv_size_max = (RTI_UINT32)shmem_intf->factory->property.message_size_max;
    properties->send_size_max = (RTI_UINT32)shmem_intf->factory->property.message_size_max;
}

RTI_PRIVATE
struct NETIO_InterfaceI NETIO_SHMEMInterface_fv_Intf =
{
    RT_COMPONENTI_BASE,
    NETIO_SHMEMInterface_send,
    NETIO_SHMEMInterface_acknack,
    NETIO_SHMEMInterface_request,
    NULL,
    NETIO_SHMEMInterface_xmit_remove,
    NETIO_SHMEMInterface_add_route,
    NETIO_SHMEMInterface_delete_route,
    NETIO_SHMEMInterface_reserve_address,
    NETIO_SHMEMInterface_bind,
    NETIO_SHMEMInterface_unbind,
    NULL, /* NETIO_SHMEMInterface_receive */
    NETIO_SHMEMInterface_get_external_interface,
    NETIO_SHMEMInterface_bind_external,
    NETIO_SHMEMInterface_unbind_external,
    NETIO_SHMEMInterface_set_state,
    NETIO_SHMEMInterface_release_address,
    NETIO_SHMEMInterface_resolve_address,
    NETIO_SHMEMInterface_get_route_table,
    NETIO_SHMEMInterface_post_event,
    NULL,
    NETIO_SHMEMInterface_is_address_reachable,
    NETIO_SHMEMInterface_get_transport_properties
};

/******************************************************************************
 *                                                                            *
 *                    NETIO_SHMEMInterfaceFactoryProperty                     *
 *                                                                            *
 ******************************************************************************/

RTI_BOOL
NETIO_SHMEMInterfaceFactoryProperty_initialize(
        struct NETIO_SHMEMInterfaceFactoryProperty *self)
{
    struct NETIO_SHMEMInterfaceFactoryProperty dflt =
            NETIO_SHMEMInterfaceFactoryProperty_INITIALIZER;
    *self = dflt;
    return RTI_TRUE;
}

RTI_BOOL
NETIO_SHMEMInterfaceFactoryProperty_finalize(
        struct NETIO_SHMEMInterfaceFactoryProperty *p)
{
    UNUSED_ARG(p);
    return RTI_TRUE;
}

/******************************************************************************
 *                                                                            *
 *                       NETIO_SHMEMInterfaceFactory                        *
 *                                                                            *
 ******************************************************************************/

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
NETIO_SHMEMInterfaceFactory_create_component(
        struct RT_ComponentFactory *factory,
        struct RT_ComponentProperty *property,
        struct RT_ComponentListener *listener)
{
    struct NETIO_SHMEMInterface *retval = NULL;
    struct NETIO_SHMEMInterfaceFactory *shmem_factory =
            (struct NETIO_SHMEMInterfaceFactory*)factory;

    retval = NETIO_SHMEMInterface_create(
            (struct NETIO_SHMEMInterfaceFactory*)factory,
            (const struct NETIO_InterfaceProperty* const )property,
            (const struct NETIO_InterfaceListener* const )listener);

    if (retval == NULL)
    {
        return NULL;
    }

    ++shmem_factory->instance_counter;
    return &retval->_parent._parent;
}

/*ci
 * \brief Implementation of the RT ComponentFactory delete method
 *
 * \param[in] factory   The factory that created the component
 * \param[in] component The component to be deleted
 *
 * \sa \ref NETIO_SHMEMInterfaceFactory_create_component
 */
#ifndef RTI_CERT
RTI_PRIVATE void
NETIO_SHMEMInterfaceFactory_delete_component(
        struct RT_ComponentFactory *factory,
        RT_Component_T *cmpnt)
{
    struct NETIO_SHMEMInterface *self = (struct NETIO_SHMEMInterface*)cmpnt;
    UNUSED_ARG(factory);
    NETIO_SHMEMInterface_delete(self);
}
#endif

MUST_CHECK_RETURN RTI_PRIVATE struct RT_ComponentFactory*
NETIO_SHMEMInterfaceFactory_initialize(
        struct RT_ComponentFactoryProperty *property,
        struct RT_ComponentFactoryListener *listener);

RTI_PRIVATE void
NETIO_SHMEMInterfaceFactory_finalize(
        struct RT_ComponentFactory *factory,
        struct RT_ComponentFactoryProperty **property,
        struct RT_ComponentFactoryListener **listener);

/*ci
 * \brief Implementation of the RT ComponentFactoryI interface
 */
RTI_PRIVATE
struct RT_ComponentFactoryI NETIO_SHMEMInterfaceFactory_fv_Intf =
{
    0,
    NETIO_SHMEMInterfaceFactory_initialize,
    NETIO_SHMEMInterfaceFactory_finalize,
    NETIO_SHMEMInterfaceFactory_create_component,
#ifndef RTI_CERT
    NETIO_SHMEMInterfaceFactory_delete_component,
#else
    NULL,
#endif
    NULL,
    NULL
};

/*ci
 * \brief logger interface factory
 *
 * \details
 * The logger interface class is implemented as a singleton, there are
 * no shared resources between interfaces.
 */
RTI_PRIVATE
struct NETIO_SHMEMInterfaceFactory NETIO_SHMEMInterfaceFactory_fv_Factory =
{
    {
        &NETIO_SHMEMInterfaceFactory_fv_Intf,
        NULL,
        {
            {
                {0, 0}
            }
        }
    }, 0,
    NETIO_SHMEMInterfaceFactoryProperty_INITIALIZER,
    NETIO_SHMEM_VERSION_MAJOR_DEFAULT
};

/*ci
 * \brief Determine what major version to use based on the factory property
 *
 * \param[in] property The factory property
 *
 * \return The major version to use
 */
RTI_PRIVATE RTI_BOOL
NETIO_SHMEMInterfaceFactory_get_major_version(
        const struct NETIO_SHMEMInterfaceFactoryProperty *property,
        RTI_INT16 *major_version_out)
{
    const struct DDS_ProductVersion UNKNOWN_PRODUCT_VERSION = DDS_PRODUCTVERSION_UNKNOWN;
    const struct DDS_ProductVersion BUG_14240_FIXED_PRODUCT_VERSION = {4, 5, 54, 0};
    const struct DDS_ProductVersion WRITER_COOKIE_ADDED_PRODUCT_VERSION = { 6, 1, 0, 0};
    const struct DDS_ProductVersion ROBUST_MUTEX_ADDED_PRODUCT_VERSION = { 7, 2, 0, 0};

    struct DDS_ProductVersion minimum_compatibility_version =
            property->pro_minimum_compatibility_version;
    RTI_INT16 major_version = NETIO_SHMEM_VERSION_MAJOR_DEFAULT;
    RTI_BOOL result = RTI_FALSE;

    /* If using robust mutexes, the minimum compatibility version must be
     * set to at least the version where robust mutexes were added
     */
    if (NETIO_SharedMemoryMutex_is_robust())
    {
        if (DDS_ProductVersion_compare(
                &minimum_compatibility_version,
                &UNKNOWN_PRODUCT_VERSION) == 0)
        {
            minimum_compatibility_version = ROBUST_MUTEX_ADDED_PRODUCT_VERSION;
        }
        else if (DDS_ProductVersion_compare(
                    &minimum_compatibility_version,
                    &ROBUST_MUTEX_ADDED_PRODUCT_VERSION) < 0)
        {
            /* If the minimum compatibility version is set to a version before
             * robust mutexes were added, set it to the version where robust
             * mutexes were added and log a warning
             */
            NETIO_SHMEM_LOG_IGNORE_TRANSPORT_COMPATIBILITY(OSAPI_LOGKIND_WARNING)
            minimum_compatibility_version = ROBUST_MUTEX_ADDED_PRODUCT_VERSION;
        }
    }

    /* If the minimum compatibility version is set, determine the major
     * version based on the minimum compatibility version
     */
    if (DDS_ProductVersion_compare(
                &minimum_compatibility_version,
                &UNKNOWN_PRODUCT_VERSION) != 0)
    {
        if ((DDS_ProductVersion_compare(
                &minimum_compatibility_version,
                &ROBUST_MUTEX_ADDED_PRODUCT_VERSION) >= 0)
            && NETIO_SharedMemoryMutex_is_robust())
        {
            major_version = NETIO_SHMEM_MAJOR_AFTER_ROBUST_PTHREAD_MUTEX;
        }
        else if (DDS_ProductVersion_compare(
                &minimum_compatibility_version,
                &WRITER_COOKIE_ADDED_PRODUCT_VERSION) >= 0)
        {
            major_version = NETIO_SHMEM_MAJOR_AFTER_WRITER_COOKIE_SUPPORT;
        }
        else if (DDS_ProductVersion_compare(
                &minimum_compatibility_version,
                &BUG_14240_FIXED_PRODUCT_VERSION) >= 0)
        {
            major_version = NETIO_SHMEM_MAJOR_AFTER_BUG_14240_FIX;
        }
        else
        {
            /* This is a fatal error because the user has explicitly requested
             * a version that we cannot support. This includes v1 of the shared
             * memory transport.
             */
            NETIO_SHMEM_LOG_UNSUPPORTED_TRANSPORT_COMPATIBILITY(OSAPI_LOGKIND_ERROR)
            goto done;
        }
    }

    *major_version_out = major_version;
    result = RTI_TRUE;
done:
    return result;
}

/*ci
 * \brief logger specific implementation of the RT ComponentFactory initialize
 *        method
 *
 * \param[in] property The properties registered with the logger interface
 * \param[in] listener The listener registered with the logger interface
 *
 * \return A fully initialized factory on success, NULL on failure
 *
 * \sa \ref NETIO_SHMEMInterfaceFactory_finalize
 */
MUST_CHECK_RETURN RTI_PRIVATE struct RT_ComponentFactory*
NETIO_SHMEMInterfaceFactory_initialize(
        struct RT_ComponentFactoryProperty *property,
        struct RT_ComponentFactoryListener *listener)
{
    struct NETIO_SHMEMInterfaceFactory *factory;
    struct NETIO_SHMEMInterfaceFactoryProperty *f_prop =
            (struct NETIO_SHMEMInterfaceFactoryProperty*) property;
    RTI_INT16 major_version;

    UNUSED_ARG(listener);

     OSAPI_PRECONDITION_ALWAYS(
            (property == NULL),
            return NULL,
            OSAPI_Log_entry_add_pointer("property",property,RTI_TRUE);)

    if ((f_prop->message_size_max <= 0)
            || (f_prop->receive_buffer_size < f_prop->message_size_max))
    {
        NETIO_SHMEM_LOG_INCONSISTENT_MESSAGE_SIZE(OSAPI_LOGKIND_ERROR);
        return NULL;
    }

    if (!NETIO_SHMEMInterfaceFactory_get_major_version(f_prop, &major_version))
    {
        return NULL;
    }

    OSAPI_Heap_allocate_struct(&factory, struct NETIO_SHMEMInterfaceFactory);
    if (factory == NULL)
    {
        return NULL;
    }

    *factory = NETIO_SHMEMInterfaceFactory_fv_Factory;
    factory->_parent._factory = &factory->_parent;
    factory->property =
            *(struct NETIO_SHMEMInterfaceFactoryProperty*)property;
    factory->major_version = major_version;

    return &factory->_parent;
}

/*ci
 * \brief Implementation of the RT ComponentFactory finalize method
 *
 * \param[in]  factory  The factory to finalize
 * \param[out] property The property the factory was registered with
 * \param[out] listener The listener the factory was registered with
 *
 * \sa \ref NETIO_SHMEMInterfaceFactory_initialize
 */
RTI_PRIVATE void
NETIO_SHMEMInterfaceFactory_finalize(
        struct RT_ComponentFactory *factory,
        struct RT_ComponentFactoryProperty **property,
        struct RT_ComponentFactoryListener **listener)
{
    UNUSED_ARG(property);
    UNUSED_ARG(listener);
#ifndef RTI_CERT
    OSAPI_Heap_free_struct(factory);
#else
    UNUSED_ARG(factory);
#endif
}

MUST_CHECK_RETURN RTI_BOOL
NETIO_SHMEMInterfaceFactory_register(
        RT_Registry_T *registry,
        const char *name,
        struct RT_ComponentFactoryProperty *property,
        struct RT_ComponentFactoryListener *listener)
{
    return RT_Registry_register(
            registry,
            name,
            NETIO_SHMEMInterfaceFactory_get_interface(),
            property,
            listener);
}

struct RT_ComponentFactoryI*
NETIO_SHMEMInterfaceFactory_get_interface(void)
{
    return &NETIO_SHMEMInterfaceFactory_fv_Intf;
}
