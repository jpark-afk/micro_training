/*
 * FILE: DataWriterInterface.c - DataWriterInterface implementation
 *
 * (c) Copyright 2012-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 22feb2017,tk MICRO-1581 Don't count acking_readers readers twice when state
 *                         is inactive _and_ the participant liveliness is also
 *                         lost.
 * 06jun2016,tk MICRO-1543 Reference count shared resources for matched entities (needed
 *                         after changes to related to MICRO-1505
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 * 16mar2015,tk MICRO-1125/PR#14262 Do not allocate redundant index for btable
 * 16mar2015,tk MICRO-1124/PR#14261 Removed redundant NULL check
 * 10dec2014,tk MICRO-978/PR#12914 Consistently return records on failure
 * 16oct2014,tk MICRO-866: Iterate over the complete intf index
 * 14aug2014,eh MICRO-885: acknack() return
 * 04aug2014,eh MICRO-866: fix redundant send
 * 31jul2014,tk MICRO-241/PR#1413 - Removed superfluous DB fields
 * 31jul2014,tk MICRO-842/PR#9683 - Removed superfluous paramaters in DB
 *                                  compare function
 * 03feb2014,eh MICRO-714: support discard and acknack
 * 27jan2014,eh MICRO-714: add post_event, for reliable reader activity change
 * 01aug2013,tk MICRO-677: on_liveliness_callback on remote deletion
 * 17may2013,eh Fixed MICRO-385: remove unused fns/fields of packet
 * 23mar2013,tk Major update
 * 14may2012,tk Written
 */
/*ce
 * \file
 * \brief NETIO interface for DDS DataWriter
 *
 * \details
 * The datawriter sends/receives data by implementing a NETIO interface and
 * implementing the required methods. The datawriter interface is the highest
 * layer in the NETIO stack for the DDS datawriter and thus does not implement
 * any methods which may be called from an upstream interface. The datawriter
 * interface is tightly coupled to the datawriter discovery functionality and
 * thus only implements the required methods.
 */
/*ci \addtogroup DDSPublicationModule
 * @{
 */
#ifndef dds_c_config_h
#include "dds_c/dds_c_config.h"
#endif
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef reda_bufferpool_h
#include "reda/reda_bufferpool.h"
#endif
#ifndef reda_indexer_h
#include "reda/reda_indexer.h"
#endif
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif
#ifndef netio_address_h
#include "netio/netio_address.h"
#endif
#ifndef netio_route_h
#include "netio/netio_route.h"
#endif
#ifndef netio_interface_h
#include "netio/netio_interface.h"
#endif
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_domain_h
#include "dds_c/dds_c_domain.h"
#endif
#include "Entity.h"
#include "DataWriterImpl.h"
#include "DataWriterQos.h"
#include "DataWriterEvent.h"
#include "DataWriterInterface.h"

#if DDS_FILTERING_ENABLED
#include "DataWriterFilter.h"
#endif /* DDS_FILTERING_ENABLED */


/*ci
 * \brief Datawriter NETIO interface factory implementation
 */
struct DDS_DataWriterInterfaceFactory
{
    /*ci
     * \brief base-class
     */
    struct RT_ComponentFactory _parent;
};

/*ci
 * \brief The Datawriter NETIO interface factory exists as a singleton
 */
LINK_SECTION_BSS_SDRAM
RTI_PRIVATE struct NETIO_InterfaceI DDS_DataWriterInterface_fv_Intf;

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DataWriterInterface_return_loan(NETIO_Interface_T *self,
                                    struct NETIO_Address *source,
                                    NETIO_Packet_T *packet,
                                    NETIO_PacketId_T *packet_id);

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Compare entries in the table of route entries. The function is
 *        compatible with \ref DB_IndexCompare_T
 *
 * \param[in] flags Used to distinguish between op2 being a key or record
 * \param[in] op1   A DDS_DataWriterRouteEntry already in the database
 * \param[in] op2   Either a DDS_DataWriterRouteEntry being added or a key
 *                  being searched for
 *
 * \return positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2,
 *         zero if op1 is equal to op2
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_INT32
DDSDataWriterInterface_compare_route(RTI_INT32 flags,
                                     const DB_Record_T op1, void *op2)
{
    struct DDS_DataWriterRouteEntry *left_record = (struct DDS_DataWriterRouteEntry *)op1;
    struct DDS_DataWriterRouteEntryKey *right_record;
    RTI_INT32 diff;
    UNUSED_ARG(flags);

    right_record = ((struct DDS_DataWriterRouteEntryKey*)op2);

    diff = OSAPI_Memory_compare(&left_record->reader,
                                &right_record->reader,
                                sizeof(struct NETIO_Guid));
    if (diff != 0)
    {
        return diff;
    }

    if (left_record->intf > right_record->intf)
    {
        return 1;
    }

    if (left_record->intf < right_record->intf)
    {
        return -1;
    }

    if (left_record->encapsulation > right_record->encapsulation)
    {
        return 1;
    }

    if (left_record->encapsulation < right_record->encapsulation)
    {
        return -1;
    }

    return 0;
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_INT32
DDSDataWriterInterface_compare_route_intf(RTI_INT32 flags,
                                          const DB_Record_T op1,
                                          void *op2)
{
    struct DDS_DataWriterRouteEntry *left_record = (struct DDS_DataWriterRouteEntry *)op1;
    struct DDS_DataWriterRouteEntryKey *right_record;
    UNUSED_ARG(flags);

    right_record = ((struct DDS_DataWriterRouteEntryKey*)op2);

    if (left_record->intf > right_record->intf)
    {
        return 1;
    }

    if (left_record->intf < right_record->intf)
    {
        return -1;
    }

    if (left_record->encapsulation > right_record->encapsulation)
    {
        return 1;
    }

    if (left_record->encapsulation < right_record->encapsulation)
    {
        return -1;
    }

    return 0;
}

/*ci
 * \brief Compare entries in the table of bind entries. The function is
 *        compatible with \ref DB_IndexCompare_T
 *
 * \param[in] flags Used to distinguish between op2 being a key or record
 * \param[in] op1   A DDS_DataWriterBindEntry already in the database
 * \param[in] op2   Either a DDS_DataWriterBindEntryKey being added or a
 *                  NETIO_Guid key being searched for.
 *
 * \return positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2,
 *         zero if op1 is equal to op2
 */
RTI_PRIVATE RTI_INT32
DDS_DataWriterInterface_compare_bind(RTI_INT32 flags,
                                     const DB_Record_T op1, void *op2)
{
    struct NETIO_Guid *lkey = &((struct DDS_DataWriterBindEntry*)op1)->source;
    struct NETIO_Guid *rkey;

    if (DB_SELECT_OP2_IS_KEY(flags))
    {
        rkey = &((struct DDS_DataWriterBindEntryKey*)op2)->source;
    }
    else
    {
        rkey = &((struct DDS_DataWriterBindEntry*)op2)->source;
    }

    return DDS_BuiltinTopicKey_compare(
            OSAPI_Compiler_reinterpret_cast(DDS_BuiltinTopicKey_t*,lkey),
            OSAPI_Compiler_reinterpret_cast(DDS_BuiltinTopicKey_t*,rkey));
}

/*ci
 * \brief Initialize a Datawriter NETIO interface instance
 *
 * \param[in] self     Interface to delete
 * \param[in] factory  Datawriter NETIO interface factory that is creating
 *                     the instance
 * \param[in] property The property of the new Datawriter NETIO interface
 *                     interface. Cannot be NULL.
 * \param[in] listener The listener for the new Datawriter NETIO interface
 *                     interface.
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref DDS_DataWriterInterface_finalize
 *
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DataWriterInterface_initialize(struct DDS_DataWriterInterface *self,
                struct DDS_DataWriterInterfaceFactory *factory,
                const struct DDS_DataWriterInterfaceProperty *const property,
                const struct NETIO_InterfaceListener *const listener)
{
    struct DB_TableProperty tbl_property = DB_TableProperty_INITIALIZER;
    char tbl_name[NETIO_TABLE_NAME_SIZE];
    union RT_ComponentFactoryId id;
    DB_ReturnCode_T dbrc;
    struct DB_IndexProperty idxp = DB_IndexProperty_INITIALIZER;

    if (!NETIO_Interface_initialize(&self->_parent,
                           &DDS_DataWriterInterface_fv_Intf,
                           &property->_parent,listener))
    {
        return RTI_FALSE;
    }

    /* Use the entity ID as the unique ID in table names. The entity ID is
     * unique for each DataWriter within a DomainParticipant and each
     * DomainParticipant has its own database.
     */
    id._value = factory->_parent._id._value;
    NETIO_Interface_Table_name_from_id(tbl_name,&id,'r',
        (RTI_INT32)property->intf_address.value.guid_prefix.entity);
    tbl_property.max_records = (RTI_SIZE_T)property->_parent.max_routes;
    tbl_property.max_indices = 2;
    tbl_property.max_cursors = 2;
    self->enc_index = NULL;

    dbrc = DB_Database_create_table(&self->_parent._rtable,
                            property->_parent._parent.db,
                            &tbl_name[0],
                            sizeof(struct DDS_DataWriterRouteEntry),
                            DDSDataWriterInterface_compare_route,
                            &tbl_property);

    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_CREATE(OSAPI_LOGKIND_ERROR,tbl_name,dbrc)
        return RTI_FALSE;
    }

    tbl_property.max_indices = 2;
    idxp.max_entries = (RTI_SIZE_T)property->_parent.max_routes;
    dbrc = DB_Table_create_index(self->_parent._rtable,
                                 &self->enc_index,
                                 DDSDataWriterInterface_compare_route_intf,
                                 &idxp);

    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_CREATE(OSAPI_LOGKIND_ERROR,tbl_name,dbrc)
        return RTI_FALSE;
    }

    tbl_property.max_records = (RTI_SIZE_T)property->_parent.max_binds;
    tbl_property.max_indices = 0;
    NETIO_Interface_Table_name_from_id(tbl_name,&id,'b',
        (RTI_INT32)property->intf_address.value.guid_prefix.entity);
    dbrc = DB_Database_create_table(&self->_parent._btable,
                                property->_parent._parent.db,&tbl_name[0],
                                sizeof(struct DDS_DataWriterBindEntry),
                                DDS_DataWriterInterface_compare_bind,
                                &tbl_property);

    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_CREATE(OSAPI_LOGKIND_ERROR,tbl_name,dbrc)
        return RTI_FALSE;
    }

    self->factory = factory;
    self->datawriter = property->datawriter;
    self->type_plugin = property->type_plugin;

    /* Always use the local_address, not the property */
    self->_parent.local_address = property->intf_address;
#ifdef RTI_ENDIAN_LITTLE
    CDR_Stream_byteswap_set(&self->stream, RTI_TRUE);
#else
    CDR_Stream_byteswap_set(&self->stream, RTI_FALSE);
#endif

    /* destination sequence for send() */
    if (!NETIO_AddressSeq_initialize(&self->send_dests_seq))
    {
        DDSC_LOG_SEQ_INITIALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DESTINATION_SEQUENCE)
        return RTI_FALSE;
    }

    if (!NETIO_AddressSeq_set_maximum(&self->send_dests_seq,
                                      property->max_send_fanout))
    {
        DDSC_LOG_SEQ_SETMAX(OSAPI_LOGKIND_ERROR,DDSC_LOG_DESTINATION_SEQUENCE,
                            property->max_send_fanout)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize a Datawriter NETIO interface instance
 *
 * \param[in] netio_intf Interface to finalize
 *
 * \sa \ref DDS_DataWriterInterface_initialize
 */
SHOULD_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DataWriterInterface_finalize(struct DDS_DataWriterInterface *netio_intf)
{
    struct DDS_DataWriterRouteEntry *route = NULL;
    DB_Cursor_T cursor = NULL;
    DB_ReturnCode_T dbrc;
    struct DDS_DataWriterBindEntry *bentry;

    NETIO_Interface_finalize(&netio_intf->_parent);

    dbrc = DB_Table_select_all_default(netio_intf->_parent._rtable,&cursor);

    if (dbrc != DB_RETCODE_OK)
    {
        return RTI_FALSE;
    }

    dbrc = DB_Table_delete_index(netio_intf->_parent._rtable,
                                 netio_intf->enc_index);

    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_DELETE(OSAPI_LOGKIND_ERROR,
                              netio_intf->_parent._rtable,
                              dbrc);
        return RTI_FALSE;
    }

    do
    {
        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&route);
        if (dbrc == DB_RETCODE_OK)
        {
            dbrc = DB_Table_delete_record(netio_intf->_parent._rtable,
                                          (DB_Record_T)route);
        }
    } while (dbrc == DB_RETCODE_OK);
    DB_Cursor_finish(netio_intf->_parent._rtable,cursor);

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    dbrc = DB_Database_delete_table(netio_intf->datawriter->config->db,
                                    netio_intf->_parent._rtable);

    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_DELETE(OSAPI_LOGKIND_ERROR,
                              netio_intf->_parent._rtable,dbrc)
        return RTI_FALSE;
    }

    cursor = NULL;
    dbrc = DB_Table_select_all_default(netio_intf->_parent._btable,&cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        return RTI_FALSE;
    }

    do
    {
        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&bentry);
        if (dbrc == DB_RETCODE_OK)
        {
            dbrc = DB_Table_delete_record(
                              netio_intf->_parent._btable,(DB_Record_T)bentry);
        }
    } while (dbrc == DB_RETCODE_OK);
    DB_Cursor_finish(netio_intf->_parent._btable,cursor);

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    dbrc = DB_Database_delete_table(netio_intf->datawriter->config->db,
            netio_intf->_parent._btable);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_DELETE(OSAPI_LOGKIND_ERROR,
                              netio_intf->_parent._btable,dbrc)
        return RTI_FALSE;
    }

    if (!NETIO_AddressSeq_finalize(&netio_intf->send_dests_seq))
    {
        DDSC_LOG_SEQ_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DESTINATION_SEQUENCE)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}
#endif /* !RTI_CERT */


#ifndef RTI_CERT
/*ci
 * \brief Delete a Datawriter NETIO interface instance
 *
 * \param[in] netio_intf Datawriter NETIO Interface to delete
 *
 * \sa \ref DDS_DataWriterInterface_create
 */
RTI_PRIVATE void
DDS_DataWriterInterface_delete(struct DDS_DataWriterInterface *netio_intf)
{
    (void)DDS_DataWriterInterface_finalize(netio_intf);
    OSAPI_Heap_free_struct(netio_intf);
}
#endif /* !RTI_CERT */

/*ci
 * \brief Create a new Datawriter NETIO interface instance
 *
 * \param[in] factory   Factory creating the new instance
 * \param[in] property  The property of the new Datawriter NETIO interface
 * \param[in] listener  The listener for the new Datawriter NETIO interface
 *
 * \return Pointer to new Datawriter NETIO interface instance on success, NULL
 *         on failure
 *
 * \sa \ref DDS_DataWriterInterface_delete
 */
MUST_CHECK_RETURN RTI_PRIVATE struct DDS_DataWriterInterface*
DDS_DataWriterInterface_create(struct DDS_DataWriterInterfaceFactory *factory,
                  const struct DDS_DataWriterInterfaceProperty *const property,
                  const struct NETIO_InterfaceListener *const listener)
{
    struct DDS_DataWriterInterface *dw_intf = NULL;

    OSAPI_Heap_allocate_struct(&dw_intf, struct DDS_DataWriterInterface);

    if (dw_intf == NULL)
    {
        DDSC_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITERIO_OBJECT)
        return NULL;
    }

    if (!DDS_DataWriterInterface_initialize(dw_intf,factory,property,listener))
    {
        return NULL;
    }

    return dw_intf;
}

/*ci
 * \brief Create a new Datawriter NETIO interface instance
 *
 * \details
 * This function serializes a sample based on the encapsulation parameter.
 *
 * \param[in] self          The data writer interface
 * \param[in] packet        The packet to add serialized data to
 * \param[in] encapsulation The encapsulation to serialize to
 * \param[in] destination   The destination address for the serialized data
 *
 * \return DDS_BOOLEAN_TRUE when the sample was successfully serialized,
 *         DDS_BOOLEAN_FALSE on failure.
 */
RTI_PRIVATE DDS_ReturnCode_t
DDS_DataWriterInterface_serialize_sample(struct DDS_DataWriterInterface *self,
                                         NETIO_Packet_T *packet,
                                         DDS_EncapsulationId_t encapsulation,
                                         struct NETIO_Address *destination)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    struct NETIO_PacketInfo *pkt_info;
    DDS_UnsignedShort pid_id;
    DDS_UnsignedShort pid_length;
    struct DDS_DataWriterSample *sample;
    struct DDS_TypePluginBuffer *payload;
    struct DDS_TypePluginBuffer *payload_prev;
    DDS_InstanceHandle_t dst_ih;

    pkt_info = NETIO_Packet_get_info(packet);
    sample = packet->ref;
    pkt_info->encapsulation = encapsulation;

    DDS_InstanceHandle_from_netio_address(&dst_ih,destination);

    /* General protocol data */
    if (self->datawriter->send_key_hash)
    {
        char *keyhash = (char*)NETIO_PacketBuffer_get_head(&sample->inline_pbuf);

        /* Skip the PID */
        keyhash += RTPS_SUBMESSAGE_HEADER_LENGTH;
        OSAPI_Memory_copy(&pkt_info->instance,keyhash,RTPS_KEY_HASH_MAX_LENGTH);
    }
    else
    {
        OSAPI_Memory_zero(&pkt_info->instance,RTPS_KEY_HASH_MAX_LENGTH);
    }

    if  ((encapsulation == DDS_ENCAPSULATION_ID_MEMORY) ||
         (encapsulation == DDS_ENCAPSULATION_ID_SHMEM_V2))
    {
        if (sample->sample_info.status_info & RTPS_DISPOSE_STATUS_INFO)
        {
            pkt_info->rtps_flags |= NETIO_RTPS_FLAGS_DISPOSE;
        }

        if (sample->sample_info.status_info & RTPS_UNREGISTER_STATUS_INFO)
        {
            pkt_info->rtps_flags |= NETIO_RTPS_FLAGS_UNREGISTER;
        }

        if ((sample->instance_data != NULL) &&
            (sample->sample_info.status_info == RTPS_NO_STATUS_INFO))
        {
            pkt_info->rtps_flags |= NETIO_RTPS_FLAGS_DATA;
            pkt_info->valid_data = 1;
        }

        return DDS_RETCODE_OK;
    }

    /* A sample must be able to handle different payload buffers, one per
     * encapsulation. Each data writer sample has a list of 0 or more
     * payload buffers.
     */
    payload = sample->payload;

    while (payload != NULL)
    {
        if (payload->encapsulation == encapsulation)
        {
            break;
        }
        else
        {
            payload = (struct DDS_TypePluginBuffer*)payload->_next;
        }
    }

    if (payload == NULL)
    {
        /* Did not find a sample already of the requested encapsulation,
         * allocate one the type-plugin.
         */
        if (!DDS_TypePlugin_set_encapsulation(self->type_plugin,encapsulation))
        {
            goto done;
        }

        /* if we are here and get a payload we need to encode
         * we will use our own buffer to serialize and then set
         * the payload as the "encoded buffer" for the encrypt
         * call
         */
        payload = DDS_TypePlugin_get_buffer(self->type_plugin);
        if (payload == NULL)
        {
            DDSC_LOG_TYPE_PLUGIN_GET_BUFFER(OSAPI_LOGKIND_ERROR);
            goto done;
        }

        payload->encapsulation = encapsulation;
        payload->_next = sample->payload;
        sample->payload = payload;

        /* First serialize the inline qos. The inline qos buffer is owned by
         * the datawriter and is part of the sample (not the user payload).
         */
#ifdef RTI_ENDIAN_LITTLE
        self->stream.endian = RTI_CDR_ENDIAN_LITTLE;
#else
        self->stream.endian = RTI_CDR_ENDIAN_BIG;
#endif
        self->stream.need_byte_swap = RTI_FALSE;

        if (self->type_plugin->big_endian)
        {
            CDR_Stream_set_endianess(&self->stream,RTI_TRUE);
        }
        else
        {
            CDR_Stream_set_endianess(&self->stream,RTI_FALSE);
        }

        /* Reset the inline pbuf. Note that this does _not_ clear the content
         * of the buffer, it only resets head and tail position. Thus, the
         * key, if present, is still in the buffer at the correct location.
         * A keyhash is _always_ sent for a keyed datawriter.
         */
        NETIO_PacketBuffer_reset(&sample->inline_pbuf);

        if (!CDR_Stream_set_buffer(&self->stream,
                                   sample->inline_pbuf.buffer,
                                   sample->inline_pbuf.max_length))
        {
            DDSC_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_MD5STREAM_OBJECT)
            goto done;
        }

        /* Always serialize the inline Qos in the following order:
         * KEYHASH_PID (4)
         * KEYHASH (key length)
         * STATUS_INFO_PID (4)
         * STATUS_INFO (4)
         * EPOCH_PID (4)
         * EPOCH (8)
         * PID_SENTINEL (4)
         *
         * The location of the KEYHASH must be in this exact location. This is
         * because the keyhash is copied directly to inline qos buffer.
         */
        if (self->datawriter->send_key_hash)
        {
            pid_id = RTPS_PID_KEY_HASH;

            /* The RTPS specification says the length is always 16.
             */
            pid_length = (DDS_UnsignedShort)RTPS_KEY_HASH_MAX_LENGTH;

            if (!CDR_Stream_serialize_unsigned_short(&self->stream, &pid_id))
            {
                DDSC_LOG_CDR_SERIALIZE_PID(OSAPI_LOGKIND_ERROR,
                        DDSC_LOG_DATAWRITER_CDR,(RTI_INT32)pid_id)
                    goto done;
            }

            if (!CDR_Stream_serialize_unsigned_short(&self->stream, &pid_length))
            {
                DDSC_LOG_CDR_SERIALIZE_PID_LENGTH(OSAPI_LOGKIND_ERROR,
                        DDSC_LOG_DATAWRITER_CDR,(RTI_INT32)pid_length)
                    goto done;
            }

            /* The keyhash is already in the buffer, jump over it  */
            if (!CDR_Stream_increment_current_position(&self->stream,
                                                   RTPS_KEY_HASH_MAX_LENGTH))
            {
                DDSC_LOG_CDR_SERIALIZE_KEYHASH(OSAPI_LOGKIND_ERROR,
                        DDSC_LOG_DATAWRITER_CDR,RTPS_KEY_HASH_MAX_LENGTH)
                    goto done;
            }
            pkt_info->rtps_flags |= NETIO_RTPS_FLAGS_INLINEQOS;
        }

        if (sample->sample_info.status_info &
                (RTPS_DISPOSE_STATUS_INFO | RTPS_UNREGISTER_STATUS_INFO))
        {
            pid_id = RTPS_PID_STATUS_INFO;
            pid_length = 4;

            if (!CDR_Stream_serialize_unsigned_short(&self->stream, &pid_id))
            {
                DDSC_LOG_CDR_SERIALIZE_PID(OSAPI_LOGKIND_ERROR,
                        DDSC_LOG_DATAWRITER_CDR,(RTI_INT32)pid_id)
                    goto done;
            }

            if (!CDR_Stream_serialize_unsigned_short(&self->stream, &pid_length))
            {
                DDSC_LOG_CDR_SERIALIZE_PID_LENGTH(OSAPI_LOGKIND_ERROR,
                        DDSC_LOG_DATAWRITER_CDR,(RTI_INT32)pid_length)
                    goto done;
            }

            if (!CDR_Stream_serialize_unsigned_long_to_big_endian(
                    &self->stream, &sample->sample_info.status_info))
            {
                DDSC_LOG_CDR_SERIALIZE_STATUS_INFO(OSAPI_LOGKIND_ERROR,
                        DDSC_LOG_DATAWRITER_CDR,sample->sample_info.status_info)
                    goto done;
            }

            if (sample->sample_info.status_info & RTPS_DISPOSE_STATUS_INFO)
            {
                pkt_info->rtps_flags |= NETIO_RTPS_FLAGS_DISPOSE;
            }

            if (sample->sample_info.status_info & RTPS_UNREGISTER_STATUS_INFO)
            {
                pkt_info->rtps_flags |= NETIO_RTPS_FLAGS_UNREGISTER;
            }

            pkt_info->rtps_flags |= NETIO_RTPS_FLAGS_INLINEQOS;
        }

        if (self->datawriter->transfer_mode.shmem_ref_settings.enable_data_consistency_check)
        {
            if (!DDS_TypePlugin_serialize_inline_qos(self->type_plugin,
                                                     &self->stream,
                                                     sample->instance_data,
                                                     &dst_ih))
            {
                goto done;
            }
            pkt_info->rtps_flags |= NETIO_RTPS_FLAGS_INLINEQOS;
        }

        if ((pkt_info->rtps_flags & NETIO_RTPS_FLAGS_INLINEQOS) &&
             CDR_Stream_get_current_position_offset(&self->stream) > 0)
        {
            pid_id = RTPS_PID_SENTINEL;
            pid_length = 0;

            if (!CDR_Stream_serialize_unsigned_short(&self->stream, &pid_id))
            {
                DDSC_LOG_CDR_SERIALIZE_PID(OSAPI_LOGKIND_ERROR,
                        DDSC_LOG_DATAWRITER_CDR,(RTI_INT32)pid_id)
                goto done;
            }

            if (!CDR_Stream_serialize_unsigned_short(&self->stream, &pid_length))
            {
                DDSC_LOG_CDR_SERIALIZE_PID_LENGTH(OSAPI_LOGKIND_ERROR,
                        DDSC_LOG_DATAWRITER_CDR,(RTI_INT32)pid_length)
                    goto done;
            }
        }
        else
        {
            pkt_info->rtps_flags &= ~NETIO_RTPS_FLAGS_INLINEQOS;
        }

        if (!NETIO_PacketBuffer_adjust_tail(&sample->inline_pbuf,
           (RTI_INT32)CDR_Stream_get_current_position_offset(&self->stream)))
        {
            goto done;
        }

        sample->rtps_flags |= pkt_info->rtps_flags;

        /* Serialize the data, if any */
        if ((sample->instance_data != NULL) &&
            (sample->sample_info.status_info == RTPS_NO_STATUS_INFO))
        {
#if 0
            /* Serialize the header unless disabled by the type plugin. Note
             * that in this case each encapsulation _must_ serialize the
             * header itself. This capability is useful when the data to be
             * sent is already in serialized form in memory. If the header is
             * serialized here it is assumed that the type-plugin buffer
             * has space.
             */
            if (DDS_TypePlugin_get_eh_location(self->type_plugin) == NDDS_TYPEPLUGIN_EH_LOCATION_INLINE)
            {
                /* Serialize the header on the inline Qos stream */
                if (!CDR_Stream_serialize_header(&self->stream,
                                                 encapsulation,
                                                 RTI_CDR_ENCAPSULATION_OPTIONS_NONE))
                {
                    DDSC_LOG_CDR_SERIALIZE_DATA(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_CDR)
                                goto done;
                }
            }
#endif

            /* Serialize the data. Note that the stream is provided as a
             * "service" to the type-plugin. The plugin is free to serialize
             * the data however it sees fit.
             */
            /*8--<-------------------------------------------------------------- */
            if (!DDS_TypePlugin_set_stream(self->type_plugin,&self->stream,payload))
            {
                goto done;
            }

            {
                RTI_UINT32 eh_offset = UINT_MAX;
                RTI_UINT32 data_start = 0;
                RTI_UINT32 data_end = 0;

                if (DDS_TypePlugin_get_eh_location(self->type_plugin)
                    == NDDS_TYPEPLUGIN_EH_LOCATION_PAYLOAD)
                {
                    if (self->datawriter->set_cdr_options_padding)
                    {
                        eh_offset = CDR_Stream_get_current_position_offset(&self->stream);
                    }

                    if (!CDR_Stream_serialize_header(
                                        &self->stream,
                                        encapsulation,
                                        RTI_CDR_ENCAPSULATION_OPTIONS_NONE))
                    {
                        DDSC_LOG_CDR_SERIALIZE_DATA(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_CDR)
                        goto done;
                    }
                }

                data_start = CDR_Stream_get_current_position_offset(&self->stream);
                if (!DDS_TypePlugin_serialize_sample(self->type_plugin,
                                                    &self->stream,
                                                    sample->instance_data,
                                                    &dst_ih))
                {
                    DDSC_LOG_CDR_SERIALIZE_DATA(OSAPI_LOGKIND_ERROR,
                                                DDSC_LOG_DATAWRITER_CDR)
                    goto done;
                }

                data_end = CDR_Stream_get_current_position_offset(&self->stream);

                if (eh_offset != UINT_MAX)
                {
                    RTI_UINT32 padding;

                    padding = (data_end - data_start) % RTI_CDR_ENCAPSULATION_HEADER_ALIGNMENT;
                    if (padding)
                    {
                        padding = RTI_CDR_ENCAPSULATION_HEADER_ALIGNMENT - padding;

                        if (!CDR_Stream_set_current_position_offset(&self->stream,
                                                                    eh_offset))
                        {
                            goto done;
                        }

                        /* Serialize the header on the inline Qos stream */
                        if (!CDR_Stream_serialize_header(&self->stream,
                                                         encapsulation,
                                                         (RTI_UINT16)padding))
                        {
                            DDSC_LOG_CDR_SERIALIZE_DATA(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_CDR)
                            goto done;
                        }
                        if (!CDR_Stream_set_current_position_offset(&self->stream,
                                                                    data_end + padding))
                        {
                            goto done;
                        }
                    }
                }
            }

            pkt_info->rtps_flags |= NETIO_RTPS_FLAGS_DATA;

            /* NOTE: DDS does not guarantee that the tail is on an even boundary.
             * Although RTPS requires this, it may not be required by other protocols.
             */
            payload->data_pbuf.head_pos = 0;
            payload->data_pbuf.tail_pos =
                    CDR_Stream_get_current_position_offset(&self->stream);

            /* NOTE: DDS does not guarantee that the tail is on an even boundary.
             * Although RTPS requires this, it may not be required by other protocols.
             */
            payload->data_pbuf.tail_pos =
                        OSAPI_Heap_align_size_up(payload->data_pbuf.tail_pos,4);

            sample->rtps_flags |= pkt_info->rtps_flags;
        }
    }

    /* General protocol data. If the sample was already serialized, set
     * flags directly.
     */
    pkt_info->rtps_flags = sample->rtps_flags;
    pkt_info->valid_data = pkt_info->rtps_flags & NETIO_RTPS_FLAGS_DATA ? 1 : 0;

    /* At most 2 pbufs are passed downstream:
     * - Inline Qos and header (if any) in the first buffer
     * - Payload data
     */
    if ((pkt_info->rtps_flags & NETIO_RTPS_FLAGS_DATA) &&
        NETIO_PacketBuffer_get_length(&sample->inline_pbuf) > 0)
    {
        sample->inline_pbuf._next = &payload->data_pbuf;
        if (!NETIO_Packet_set_pbuf(packet,&sample->inline_pbuf,&payload->data_pbuf))
        {
            goto done;
        }
    }
    else if ((pkt_info->rtps_flags & NETIO_RTPS_FLAGS_DATA) &&
             (NETIO_PacketBuffer_get_length(&sample->inline_pbuf) == 0))
    {
        sample->inline_pbuf._next = NULL;
        if (!NETIO_Packet_set_pbuf(packet,&payload->data_pbuf,&payload->data_pbuf))
        {
            goto done;
        }
    }
    else if (NETIO_PacketBuffer_get_length(&sample->inline_pbuf) > 0)
    {
        sample->inline_pbuf._next = NULL;
        if (!NETIO_Packet_set_pbuf(packet,&sample->inline_pbuf,&sample->inline_pbuf))
        {
            goto done;
        }
    }

    OSAPI_Trace_write("serialized packet: SN=%S head_pbuf=%p/%p/%^d/%^d,tail_buf=%p/%p/%^d/%^d",
                        &pkt_info->sn,
                        packet->head_pbuf,
                        packet->head_pbuf->buffer,
                        &packet->head_pbuf->head_pos,
                        &packet->head_pbuf->tail_pos,
                        packet->tail_pbuf,
                        packet->tail_pbuf->buffer,
                        &packet->tail_pbuf->head_pos,
                        &packet->tail_pbuf->tail_pos,
                        NULL);

    retcode = DDS_RETCODE_OK;

done:

    if ((retcode != DDS_RETCODE_OK) && (payload != NULL))
    {
        if (payload == sample->payload)
        {
            sample->payload = sample->payload->_next;
        }
        else
        {
            payload_prev = sample->payload;
            while (payload_prev != NULL)
            {
                if ((payload_prev->_next != NULL) &&
                    (payload_prev->_next == payload))
                {
                    payload_prev->_next = payload->_next;
                }
                else
                {
                    payload_prev = payload_prev->_next;
                }
            }
        }

        DDS_TypePlugin_return_buffer(self->type_plugin,payload);
    }

    return retcode;
}

/*ci
 * \brief Implementation of the NETIO_Interface_send function
 *
 * \details
 * This function is called to send data to all the matched peers of the
 * datawriter. This function sends one sample to each unique locator and does
 * not filter per remote match.
 *
 * \param[in] netio     NETIO interface to send from
 * \param[in] src_intf  The source interface of the packet
 * \param[in] address   The destination address
 * \param[in] packet    The packet to send
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DataWriterInterface_send(NETIO_Interface_T *netio,
                             NETIO_Interface_T *src_intf,
                             struct NETIO_Address *address,
                             NETIO_Packet_T *packet)
{
    struct DDS_DataWriterInterface *dwio =
                                       (struct DDS_DataWriterInterface *)netio;
    struct DDS_DataWriterImpl *dw = (struct DDS_DataWriterImpl *)dwio->datawriter;
    RTI_BOOL retval = RTI_TRUE;
    struct NETIO_Address ADDR_UNKNOWN = NETIO_Address_INITIALIZER;
    struct DDS_DataWriterRouteEntry *route_entry = NULL;
    DB_ReturnCode_T dbrc;
    DB_Cursor_T cursor = NULL;
    struct NETIO_PacketInfo *pkt_info;
    RTI_BOOL is_sent = RTI_FALSE;
    DDS_EncapsulationId_t current_eid = DDS_ENCAPSULATION_ID_INVALID;

    UNUSED_ARG(src_intf);
    UNUSED_ARG(address);

    OSAPI_TRACE_ONLY_VARIABLE(dw);

    /* Proceed with regular simple send rules */
    packet->source = dwio->_parent.local_address;
    packet->dests = &dwio->send_dests_seq;
    pkt_info = NETIO_Packet_get_info(packet);

    OSAPI_TRACE_PRINTF1("write topic =%s\n",
                        DDS_TopicDescription_get_name(
                            DDS_Topic_as_topicdescription(dw->topic)));

    OSAPI_TRACE_NET("datawriter publishing sample for topic:",RTI_FALSE)
    OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                            DDS_Topic_as_topicdescription(dw->topic)),RTI_TRUE)

    /* This implementation assumes the writer does not perform any filtering
     * on a per-remote-reader basis.  Thus, the destination address of every
     * DWI send() is unknown.
     *
     * Also, the implementation assumes the downstream (RTPS) send() will
     * interpret an unknown destination address as a broadcast of this packet
     * to all of its matched peers.  Thus, only a single send to a single RTPS
     * intf is necessary.
     */
    if (!NETIO_AddressSeq_set_length(packet->dests, 1))
    {
        DDSC_LOG_SEQ_SETLENGTH(OSAPI_LOGKIND_ERROR,DDSC_LOG_DESTINATION_SEQUENCE,1)
        return RTI_FALSE;
    }

    *NETIO_AddressSeq_get_reference(packet->dests, 0) = ADDR_UNKNOWN;

    cursor = NULL;
    dbrc = DB_Table_select_all(dwio->_parent._rtable,dwio->enc_index,&cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_NETIO_FORWARD_TOPIC(OSAPI_LOGKIND_ERROR,
                DDS_TopicDescription_get_name(
                        DDS_Topic_as_topicdescription(dw->topic)))
        return RTI_FALSE;
    }

    OSAPI_TRACE_NET("datawriter forwarding topic downstream:",RTI_FALSE)
    OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                        DDS_Topic_as_topicdescription(dw->topic)),RTI_TRUE)

    if (!DDS_TypePlugin_is_managed_samples(dw->type_plugin) &&
        (pkt_info->rtps_flags != NETIO_RTPS_FLAGS_LIVELINESS))
    {
        /* If the sample is unmanaged then the sample must be serialized
         * regardless of whether there are matched endpoints or not.
         * Only one encapsulation is supported on the writer for inband
         * unmanaged which is the current one.
         */
        if (DDS_DataWriterInterface_serialize_sample(dwio,packet,
                             DDS_TypePlugin_get_encapsulation(dw->type_plugin),
                             &ADDR_UNKNOWN) != DDS_RETCODE_OK)
        {
            retval = RTI_FALSE;
            goto done;
        }
    }

    dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&route_entry);
    while (dbrc == DB_RETCODE_OK)
    {
        packet->info.protocol_id = NETIO_PROTOCOL_INTRA;

        if ((pkt_info->rtps_flags != NETIO_RTPS_FLAGS_LIVELINESS) &&
            (current_eid != route_entry->encapsulation))
        {
            OSAPI_TRACE_PRINTF1("Switch to encapsulation %^d\n",
                                &route_entry->encapsulation)

            OSAPI_Trace_write("Switch to encapsulation %^d from %^d\n",
                              &route_entry->encapsulation,
                              &current_eid,
                              NULL,NULL,
                              NULL,NULL,NULL,NULL,NULL,NULL);

            current_eid = route_entry->encapsulation;

            /* Note that in the case the sample is unmanaged and has
             * already been serialized then this function just returns
             * the serialized sample again. In the case of managed samples
             * and multiple encapsulations this function is needed.
             */
            if (DDS_DataWriterInterface_serialize_sample(dwio,packet,
                                         route_entry->encapsulation,
                                         &ADDR_UNKNOWN) != DDS_RETCODE_OK)
            {
                retval = RTI_FALSE;
                goto done;
            }
        }

        OSAPI_TRACE_PRINTF2("Send payload to %G using encapsulation %^d\n",
                            &route_entry->reader,
                            &current_eid)

        /* DDS_ENfull_route_entry->reader; */

        *NETIO_AddressSeq_get_reference(packet->dests,0) = ADDR_UNKNOWN;
        pkt_info->encapsulation = current_eid;

        OSAPI_Trace_write("Send payload to %G using encapsulation %^d (entry uses %^d)\n",
                          &ADDR_UNKNOWN,
                          &current_eid,&route_entry->encapsulation,NULL,NULL,
                          NULL,NULL,NULL,NULL,NULL);

        is_sent = RTI_TRUE;

        if (!NETIO_Interface_send(route_entry->intf, netio,NULL,packet))
        {
            DDSC_LOG_NETIO_FORWARD_TOPIC(OSAPI_LOGKIND_WARNING,
                                         DDS_TopicDescription_get_name(
                                                 DDS_Topic_as_topicdescription(dw->topic)))
                                            retval = RTI_FALSE;
        }

        OSAPI_Trace_write("sent packet: SN=%S head_pbuf=%p/%p/%^d/%^d,tail_buf=%p/%p/%^d/%^d",
                        &pkt_info->sn,
                        packet->head_pbuf,packet->head_pbuf->buffer,&packet->head_pbuf->head_pos,&packet->head_pbuf->tail_pos,
                        packet->tail_pbuf,packet->tail_pbuf->buffer,&packet->tail_pbuf->head_pos,&packet->tail_pbuf->tail_pos,
                        NULL);

        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&route_entry);
    }

done:

    DB_Cursor_finish(dwio->_parent._rtable, cursor);

    if (!is_sent && !DDS_DataWriterInterface_return_loan(
                                    netio,NULL,packet,&packet->info.sn))
    {

    }

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    return retval;
}

/*ci
 * \brief Implementation of the NETIO_Interface_acknack function
 *
 * \details
 *
 * An acknack is processed for a peer. If all peers who should receive this
 * sample has acked it, then it can be ack'ed in the queue and is up for removal.
 * A peer (source parameter) must acknack a sample at most once.
 *
 * \param[in] netio     The NETIO interface receiving the acknack
 * \param[in] source    The peer source address of the acknack
 * \param[in] packet_id The packet_id/SN of the NETIO_Packet being acked/nacked
 * \param[in] nack      RTI_TRUE if this is a negative acknowledgment
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DataWriterInterface_acknack(NETIO_Interface_T *netio,
                                struct NETIO_Address *source,
                                NETIO_PacketId_T *packet_id,
                                RTI_BOOL nack)
{
    RTI_BOOL retval = RTI_FALSE;
    struct REDA_SequenceNumber sn;
    struct DDS_DataWriterInterface *dwio = (struct DDS_DataWriterInterface *)netio;
    DDS_Boolean wh_nack;
    struct DDS_DataWriterBindEntryKey bind_key;
    struct DDS_DataWriterBindEntry *bind_entry = NULL;
    DB_ReturnCode_T dbrc;
    DDSHST_ReturnCode_T hst_rc;

    if (!OSAPI_Mutex_take(dwio->datawriter->write_lock))
    {
        return RTI_FALSE;
    }

    if (source != NULL)
    {
        /* If source is NULL, it has been removed as a peer.
         * If the remote reader was unmatched then the writer may no longer
         * have any knowledge about it. However, it is still necessary
         * to update the acknack count in the queue so that the writer
         * can reclaim resources.
         */

        bind_key.source = source->value.guid;
        dbrc = DB_Table_select_match(dwio->_parent._btable,DB_TABLE_DEFAULT_INDEX,
                                     (DB_Record_T *)&bind_entry,&bind_key);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_RECORD_ERROR(OSAPI_LOGKIND_ERROR,DDSC_LOG_BIND_RECORD,dbrc);
            goto done;
        }
    }

    /* We know that the current peer has acknowledged the SN */
    sn = *(struct REDA_SequenceNumber*)packet_id;

    wh_nack = (nack ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE);

    /* Only a specific SN is acknack'ed.
     * NOTE: The writer does not know _which_ reader has acknowledged
     * a sample. Thus, the contract downstream is that this function is
     * called at most once for each acked/nacked sample per remote reader.
     */

    hst_rc = DDSHST_Writer_acknack_sample(dwio->datawriter->wh,&sn, wh_nack);
    if ((hst_rc != DDSHST_RETCODE_SUCCESS) &&
        (hst_rc != DDSHST_RETCODE_NOT_EXISTS))
    {
        DDSC_LOG_DW_ACKNACK_FAILED(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    retval = RTI_TRUE;

done:

    if (!OSAPI_Mutex_give(dwio->datawriter->write_lock))
    {
        return RTI_FALSE;
    }

    return retval;
}

#if DDS_FILTERING_ENABLED
/*ci
 * \brief Advance a SN to the next available SN in the history which is not
 *        filtered for a given reader.
 *
 * \details
 * When a sample is request()'ed from the history it is automatically marked as
 * not ACKed to ensure that it is delivered when using KEEP_ALL. We don't want
 * to change the state of any samples which were filtered, so a new event was
 * added to the history to allow us to request the next SN without modifying
 * its state.
 *
 * \param[in,out] dwio The datawriter interface
 * \param[in,out] sn_inout The sequence number to advance
 * \param[in]     reader_guid The GUID of the reader to check for filtering
 * \param[in]     historical_only Whether to look for only historical samples
 */
RTI_PRIVATE void
DDS_DataWriterInterface_advance_to_next_unfiltered_sn(struct DDS_DataWriterInterface *dwio,
                                                      struct REDA_SequenceNumber *sn_inout,
                                                      const struct NETIO_Guid *reader_guid,
                                                      DDS_Boolean historical_only)
{
    struct DDSHST_WriterEvent wh_event;
    struct OSAPI_SystemTime now = OSAPI_TIME_ZERO;
    DDS_Boolean sample_dropped = DDS_BOOLEAN_FALSE;

    if (!OSAPI_System_get_time(&now))
    {
        /* not important */
    }

    do
    {
        wh_event.kind = DDSHST_WRITEREVENT_KIND_PEEK_NEXT_SN;
        wh_event.data.peek_next.sn = sn_inout;
        wh_event.data.peek_next.historical_only = historical_only;
        DDSHST_Writer_post_event(dwio->datawriter->wh,&wh_event,&now);

        if (!DDS_DataWriter_apply_reader_filter(dwio->datawriter,
                                                reader_guid,
                                                sn_inout,
                                                &sample_dropped))
        {
            /* Failed to check if the reader filtered this sample, so we will
             * send it anyway and let the reader filter it out if needed.
             */
            DDSC_LOG_APPLY_READER_FILTER(OSAPI_LOGKIND_WARNING)
            break;
        }

        if (sample_dropped)
        {
            /* The next available sample was filtered out, so advance to the
             * next one and check again.
             */
            REDA_SequenceNumber_plusplus(sn_inout);
        }
    } while (sample_dropped);
}
#endif /* DDS_FILTERING_ENABLED */

/*ci
 * \brief Implementation of the NETIO_Interface_request function
 *
 * \details
 * A downstream interface can request a sample from the DDS datawriter
 * by calling this method. It is not guaranteed that the requested
 * sample exists, in which case the first available sample is returned.
 * The caller must determine if this is useful or not. The returned sample
 * is valid until return_loan is called. It is important to call return_loan
 * when the sample is no longer needed, otherwise the sample may not be
 * reclaimed.
 *
 * \param[in] netio            The NETIO interface receiving the request
 * \param[in] source           The peer source address of the request
 * \param[in] dest             The peer destination address of the request
 * \param[in] packet           The requested packet if it existed, NULL
 *                             otherwise
 * \param[in] packet_id        The packet_id/SN of the NETIO_Packet being
 *                             requested
 * \param[in] actual_packet_id The if packet_id is not available, the next
 *                             available SN/packet_id
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DataWriterInterface_request(NETIO_Interface_T *netio,
                                struct NETIO_Address *source,
                                struct NETIO_Address *dest,
                                NETIO_Packet_T **packet,
                                NETIO_PacketId_T *packet_id,
                                NETIO_PacketId_T *actual_packet_id)
{
    struct DDS_DataWriterInterface *dwio =
                                       (struct DDS_DataWriterInterface *)netio;
    DDSHST_WriterSample_T *a_sample = NULL;
    struct REDA_SequenceNumber sn;
    struct DDS_DataWriterSample *cdr_sample = NULL;
    struct DDS_DataWriterBindEntryKey bind_key;
    struct DDS_DataWriterBindEntry *bind_entry = NULL;
    DB_ReturnCode_T dbrc;
    DDS_ReturnCode_t ddsrc;
    RTI_BOOL retval = RTI_FALSE;
    DDSHST_ReturnCode_T whrc;

    UNUSED_ARG(dest);

    if (!OSAPI_Mutex_take(dwio->datawriter->write_lock))
    {
        return RTI_FALSE;
    }

    if (dwio->datawriter->packet != NULL)
    {
        DDSC_LOG_NETIO_PACKETPOOL_GET(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    dwio->datawriter->packet = (struct NETIO_Packet*)REDA_BufferPool_get_buffer(
                                        dwio->datawriter->config->packet_pool);
    if (dwio->datawriter->packet == NULL)
    {
        DDSC_LOG_NETIO_PACKETPOOL_GET(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    sn = *(struct REDA_SequenceNumber*)packet_id;

    /* The History queue does not keep track of peer state. However,
     * it can serve requests from either historical data only, or from
     * all samples. If the request is for a sample which falls within the
     * historical window and it does not exists, get the next available
     * from the current view.
     *
     * IMPORTANT:
     * If a historical sample is being sent, then pushed out of the historical
     * view and then is requested again, it is considered not relevant anymore.
     */

    /* Find the peer record */
    bind_key.source = source->value.guid;
    dbrc = DB_Table_select_match(dwio->_parent._btable,DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T *)&bind_entry,&bind_key);
    if (dbrc != DB_RETCODE_OK)
    {
        /* Unknown peer record, must be an error */
        goto done;
    }

    if (REDA_SequenceNumber_compare(&sn,&bind_entry->high_history_sn) <= 0)
    {
#if DDS_FILTERING_ENABLED
        if (DDS_DataWriter_is_filtering_enabled(dwio->datawriter))
        {
            DDS_DataWriterInterface_advance_to_next_unfiltered_sn(
                    dwio, &sn, &source->value.guid, DDS_BOOLEAN_TRUE);
        }
#endif /* DDS_FILTERING_ENABLED*/

        /* The requested SN is within the history window */
        if (DDSHST_Writer_request_sample(dwio->datawriter->wh,
                                 &a_sample, &sn, actual_packet_id,
                                 DDS_BOOLEAN_TRUE) == DDSHST_RETCODE_SUCCESS)
        {
            cdr_sample = (struct DDS_DataWriterSample*)a_sample;
            ddsrc = DDS_DataWriter_prepare_sample(dwio->datawriter,cdr_sample,
                                                    dwio->datawriter->packet,
                                                    actual_packet_id);
            if (ddsrc != DDS_RETCODE_OK)
            {
                goto done;
            }

            OSAPI_Trace_write("Request sample %S on %s with encapsulation %^d",
                              actual_packet_id,
                              DDS_TopicDescription_get_name(
                                   DDS_Topic_as_topicdescription(
                                           dwio->datawriter->topic)),
                              &bind_entry->encapsulation_id,
                              NULL,NULL,
                              NULL,NULL,
                              NULL,NULL,NULL);

            /* request for sample are only done from RTPS today. Hence we can
             * always calls serialize
             */
            if (DDS_DataWriterInterface_serialize_sample(dwio,
                                                 dwio->datawriter->packet,
                                                 bind_entry->encapsulation_id,
                                                 dest) != DDS_RETCODE_OK)
            {
                goto done;
            }

            *packet = dwio->datawriter->packet;
            retval = RTI_TRUE;

            goto done;
        }
    }

    /* At this point the sample was not found in the historical window,
     * look for the next available in the current window.
     */
    if (REDA_SequenceNumber_compare(&sn,&bind_entry->next_new_sn) < 0)
    {
        /* Make sure the requested SN does match an existing, non-historical
         * sample. Return a sample with at least the current SN at match time.
         */
        sn = bind_entry->next_new_sn;
    }

#if DDS_FILTERING_ENABLED
    if (DDS_DataWriter_is_filtering_enabled(dwio->datawriter))
    {
        DDS_DataWriterInterface_advance_to_next_unfiltered_sn(
                dwio, &sn, &source->value.guid, DDS_BOOLEAN_FALSE);
    }
#endif /* DDS_FILTERING_ENABLED*/

    whrc = DDSHST_Writer_request_sample(dwio->datawriter->wh,&a_sample,&sn,
                                        actual_packet_id,DDS_BOOLEAN_FALSE);

    if (whrc != DDSHST_RETCODE_SUCCESS)
    {
        REDA_BufferPool_return_buffer(dwio->datawriter->config->packet_pool,
                                      dwio->datawriter->packet);
        dwio->datawriter->packet = NULL;
        *packet = NULL;
    }
    else
    {
        cdr_sample = (struct DDS_DataWriterSample *)a_sample;
        ddsrc = DDS_DataWriter_prepare_sample(dwio->datawriter,cdr_sample,
                                              dwio->datawriter->packet,
                                              actual_packet_id);
        if (ddsrc != DDS_RETCODE_OK)
        {
            goto done;
        }

        OSAPI_Trace_write("Request sample %S on %s with encapsulation %^d",
                          actual_packet_id,
                          DDS_TopicDescription_get_name(
                               DDS_Topic_as_topicdescription(dwio->datawriter->topic)),
                          &bind_entry->encapsulation_id,
                          NULL,NULL,
                          NULL,NULL,
                          NULL,NULL,NULL);

        /* request for sample are only done from RTPS today. Hence we can always calls serialize */
        if (DDS_DataWriterInterface_serialize_sample(dwio,
                                                 dwio->datawriter->packet,
                                                 bind_entry->encapsulation_id,
                                                 dest) != DDS_RETCODE_OK)
        {
            goto done;
        }

        *packet = dwio->datawriter->packet;
    }

    if (cdr_sample != NULL)
    {
        cdr_sample->rtps_flags |= NETIO_RTPS_FLAGS_IN_PROGRESS;
    }

    retval = RTI_TRUE;

done:

    if (!retval)
    {
        REDA_BufferPool_return_buffer(dwio->datawriter->config->packet_pool,
                                      dwio->datawriter->packet);
        dwio->datawriter->packet = NULL;
    }

    if (!OSAPI_Mutex_give(dwio->datawriter->write_lock))
    {
        return RTI_FALSE;
    }

    return retval;
}

/*ci
 * \brief Implementation of the NETIO_Interface_return_loan function
 *
 * \details
 *
 * A downstream interface must call this function to signal it is done with
 * a sample previously retrieved with request.
 *
 * \param[in] intf      NETIO interface to return the packet to
 * \param[in] source    The source of the returned packet
 * \param[in] packet    The packet that is returned
 * \param[in] packet_id The packet_id/SN of the NETIO_Packet that is returned
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DataWriterInterface_return_loan(NETIO_Interface_T *self,
                                    struct NETIO_Address *source,
                                    NETIO_Packet_T *packet,
                                    NETIO_PacketId_T *packet_id)
{
    struct DDS_DataWriterInterface *dwio =
                                       (struct DDS_DataWriterInterface*)self;
    struct DDS_DataWriterImpl *datawriter = dwio->datawriter;
    struct DDS_DataWriterSample *cdr_sample = NULL;
    struct DDSHST_WriterEvent wh_event;
    struct OSAPI_SystemTime now = OSAPI_TIME_ZERO;

    UNUSED_ARG(source);
    UNUSED_ARG(packet_id);

    /* Liveliness packets do not have any data associated with it */
    if (packet->info.rtps_flags == NETIO_RTPS_FLAGS_LIVELINESS)
    {
        return RTI_TRUE;
    }

    if (!OSAPI_System_get_time(&now))
    {
        /* not important */
    }


    cdr_sample = (struct DDS_DataWriterSample *)packet->ref;

    OSAPI_Trace_write("returning loan on SN=%S topic=%s serialize_on_write=%d kind=%d flags=%^x",
                        packet_id,
                        DDS_TopicDescription_get_name(
                           DDS_Topic_as_topicdescription(datawriter->topic)),
                        OSAPI_TRACE_INT_AS_PTR(datawriter->protocol.serialize_on_write),
                        OSAPI_TRACE_INT_AS_PTR(datawriter->reliability.kind),
                        &cdr_sample->rtps_flags,
                        NULL,NULL,NULL,NULL,NULL);

    if (!OSAPI_Mutex_take(dwio->datawriter->write_lock))
    {
        return RTI_FALSE;
    }

    /* Return the payload only if available
     */
    if (!(packet->info.rtps_flags & NETIO_RTPS_FLAGS_RETURN_PACKET_ONLY))
    {
        wh_event.data.sn = *packet_id;
        wh_event.kind = DDSHST_WRITEREVENT_KIND_ENTRY_STATE_SEND_COMPLETED;
        DDSHST_Writer_post_event(dwio->datawriter->wh,&wh_event,&now);

        /* Return the payload buffer. This ensures that the content is
         * serialized again when needed.
         */
        if ((cdr_sample != NULL) && (cdr_sample->payload != NULL) &&
            (!datawriter->protocol.serialize_on_write))
        {
            OSAPI_Trace_write("return sample payload %S",
                              packet_id,NULL,NULL,NULL,NULL,
                              NULL,NULL,NULL,NULL,NULL);

            DDS_DataWriter_return_sample_payload(dwio->datawriter,cdr_sample);
            cdr_sample->payload = NULL;
        }
    }

    /* it is possible that a loan is returned on a local packet downstream,
     * do not free these packets.
     */
    if (packet == datawriter->packet)
    {
        REDA_BufferPool_return_buffer(datawriter->config->packet_pool,packet);
        datawriter->packet = NULL;
    }

    if (!OSAPI_Mutex_give(dwio->datawriter->write_lock))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief Implementation of the NETIO_Interface_xmit_remove function
 *
 * \details
 *
 * Inform all downstream interfaces that all attempts to deliver a packet
 * should be cancelled.
 *
 * \param[in] intf        NETIO interface to cancel a transmit on
 * \param[in] destination The destination address of the packet
 * \param[in] packet_id   The packet_id/SN of the NETIO_Packet to cancel
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DataWriterInterface_xmit_remove(NETIO_Interface_T *intf,
                                    struct NETIO_Address *destination,
                                    NETIO_PacketId_T *packet_id)
{
    struct DDS_DataWriterInterface *dwio =
                                (struct DDS_DataWriterInterface *)intf;
    RTI_BOOL retval = RTI_TRUE;
    struct DDS_DataWriterRouteEntry *route_entry = NULL;
    DB_ReturnCode_T dbrc;
    DB_Cursor_T cursor = NULL;
    UNUSED_ARG(destination);

    /* When no routes in table, reset cached downstream (RTPS) interface */
    cursor = NULL;
    dbrc = DB_Table_select_all_default(dwio->_parent._rtable,&cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_NETIO_FORWARD_TOPIC(OSAPI_LOGKIND_ERROR,
                        DDS_TopicDescription_get_name(
                                DDS_Topic_as_topicdescription(
                                                  dwio->datawriter->topic)))
        return RTI_FALSE;
    }

    dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&route_entry);
    while (dbrc == DB_RETCODE_OK)
    {
        if (!NETIO_Interface_xmit_remove(route_entry->intf,
                                         NULL, packet_id))
        {
            retval = RTI_FALSE;
        }
        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&route_entry);
    }

    DB_Cursor_finish(dwio->_parent._rtable, cursor);

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    return retval;
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DataWriterInterface_add_route(NETIO_Interface_T *intf,
                                  struct NETIO_Address *dst_addr,
                                  NETIO_Interface_T *via_intf,
                                  struct NETIO_Address *via_addr,
                                  struct NETIORouteProperty *property,
                                  RTI_BOOL *existed)
{
    struct DDS_DataWriterInterface *self = (struct DDS_DataWriterInterface *)intf;
    struct DDS_DataWriterRouteEntry *route_entry = NULL;
    struct DDS_DataWriterRouteEntryKey route_key;
    struct DDS_DataWriterRouteProperty *rp = (struct DDS_DataWriterRouteProperty*)property;
    DB_ReturnCode_T dbrc;
    UNUSED_ARG(property);
    UNUSED_ARG(dst_addr);
    UNUSED_ARG(via_addr);

    route_key.reader = dst_addr->value.guid;
    route_key.intf = via_intf;
    route_key.encapsulation = rp->encapsulation;

    dbrc = DB_Table_select_match(self->_parent._rtable,
                                 DB_TABLE_DEFAULT_INDEX,(DB_Record_T*)&route_entry,
                                 (DB_Key_T)&route_key);

    if (existed != NULL)
    {
        *existed = (dbrc == DB_RETCODE_OK ? RTI_TRUE : RTI_FALSE);
    }

    if (dbrc == DB_RETCODE_NO_DATA)
    {
        dbrc = DB_Table_create_record(self->_parent._rtable,
                                      (DB_Record_T *)&route_entry);

        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_RECORD_CREATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_ROUTE_RECORD,dbrc)
            return RTI_FALSE;
        }

        route_entry->reader = dst_addr->value.guid;
        route_entry->intf = via_intf;
        route_entry->encapsulation = rp->encapsulation;
        route_entry->ref_count = 1;

        dbrc = DB_Table_insert_record(self->_parent._rtable,
                                      (DB_Record_T)route_entry);

        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_RECORD_INSERT(OSAPI_LOGKIND_ERROR,DDSC_LOG_ROUTE_RECORD,dbrc);
            (void)DB_Table_delete_record(self->_parent._rtable,
                                         (DB_Record_T)route_entry);
            return RTI_FALSE;
        }
    }
    else if (dbrc == DB_RETCODE_OK)
    {
        ++route_entry->ref_count;
    }
    else
    {
        DDSC_LOG_RECORD_ERROR(OSAPI_LOGKIND_ERROR,DDSC_LOG_ROUTE_RECORD,dbrc);
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief Implementation of the NETIO delete_route function
 *
 * \details
 *
 * Remove a route to a peer, either because it no longer matches or that the
 * peer has been deleted.
 *
 * \param[in]  netio      NETIO interface to delete the route to
 * \param[in]  dst_addr   The destination address for the route
 * \param[in]  via_intf   The downstream interface
 * \param[in]  via_addr   The address to pass to the downstream interface
 * \param[out] existed    Whether the route existed
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure. Note that is it not
 *         considered a failure if the interface didn't exist.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DataWriterInterface_delete_route(NETIO_Interface_T *netio,
                                     struct NETIO_Address *dst_addr,
                                     NETIO_Interface_T *via_intf,
                                     struct NETIO_Address *via_addr,
                                     RTI_BOOL *existed)
{
    struct DDS_DataWriterInterface *self = (struct DDS_DataWriterInterface *)netio;
    struct DDS_DataWriterRouteEntry *route_entry = NULL;
    struct DDS_DataWriterRouteEntryKey key;
    DB_Cursor_T cursor = NULL;
    DB_ReturnCode_T dbrc;
    UNUSED_ARG(dst_addr);
    UNUSED_ARG(via_addr);

    key.reader = dst_addr->value.guid;
    key.intf = via_intf;
    key.encapsulation = 0;

    dbrc = DB_Table_select_from(self->_parent._rtable,
                                DB_TABLE_DEFAULT_INDEX,
                                &cursor,&key);

    if (dbrc != DB_RETCODE_OK)
    {
        return RTI_FALSE;
    }

    dbrc = DB_Cursor_get_next(cursor,(DB_Record_T)&route_entry);

    while (dbrc == DB_RETCODE_OK)
    {
        if (!OSAPI_Memory_compare(&route_entry->reader,&dst_addr->value.guid,
                                  sizeof(struct NETIO_Guid)))
        {
            if (existed != NULL)
            {
                *existed = RTI_TRUE;
            }

            --route_entry->ref_count;

            if (route_entry->ref_count == 0)
            {
                key.reader = route_entry->reader;
                key.intf = route_entry->intf;
                key.encapsulation = route_entry->encapsulation;

                route_entry = NULL;
                dbrc = DB_Table_remove_record(self->_parent._rtable,
                                              (DB_Record_T *)&route_entry,
                                              &key);
                if (dbrc != DB_RETCODE_OK)
                {
                    DDSC_LOG_RECORD_REMOVE(OSAPI_LOGKIND_ERROR,DDSC_LOG_ROUTE_RECORD,dbrc)
                    return RTI_FALSE;
                }

                dbrc = DB_Table_delete_record(self->_parent._rtable,route_entry);
                if (dbrc != DB_RETCODE_OK)
                {
                    DDSC_LOG_RECORD_DELETE(OSAPI_LOGKIND_ERROR,DDSC_LOG_ROUTE_RECORD,dbrc)
                    return RTI_FALSE;
                }
            }
        }
        else
        {
            break;
        }
        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T)&route_entry);
    }

    DB_Cursor_finish(self->_parent._rtable,cursor);

    return RTI_TRUE;
}

/*ci
 * \brief Implementation of the NETIO bind function
 *
 * \details
 *
 * When a datawriter is matched with a datareader it creates an entry in the
 * bind table to listen to the datareader. There is never more than one entry
 * in the bind table per datareader.
 *
 * \param[in]  netio    NETIO interface to bind
 * \param[in]  src_addr The address to bind to
 * \param[in]  property The property to use for the bind, cannot be NULL
 * \param[out] existed  Whether a previous bind existed or not
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DataWriterInterface_bind(NETIO_Interface_T *netio,
                             struct NETIO_Address *src_addr,
                             struct NETIOBindProperty *property,
                             RTI_BOOL *existed)
{
    struct DDS_DataWriterInterface *dwintf =
                                    (struct DDS_DataWriterInterface *)netio;
    struct DDS_DataWriterBindEntry *bind_entry = NULL;
    DB_ReturnCode_T dbrc;
    struct DDS_DataWriterBindEntryKey bind_key;
    struct DDS_DataWriterBindProperty *b_property =
                                (struct DDS_DataWriterBindProperty*)property;
    DDS_InstanceHandle_t ih_peer;

    bind_key.source = src_addr->value.guid;

    OSAPI_TRACE_DDS("datawriter binding to datareader:",RTI_FALSE)
    OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                    DDS_Topic_as_topicdescription(
                            dwintf->datawriter->topic)),RTI_FALSE)
    OSAPI_TRACE_GUID("datawriter",&dwintf->_parent.local_address.value.rtps_guid,RTI_FALSE)
    OSAPI_TRACE_GUID("datareader",&src_addr->value.rtps_guid,RTI_TRUE)

    if (existed)
    {
        *existed = RTI_FALSE;
    }

    dbrc = DB_Table_select_match(dwintf->_parent._btable,DB_TABLE_DEFAULT_INDEX,
                                (DB_Record_T *)&bind_entry,&bind_key);
    if (dbrc == DB_RETCODE_OK)
    {
        if (existed)
        {
            *existed = RTI_TRUE;
        }
        return RTI_TRUE;
    }

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_RECORD_ERROR(OSAPI_LOGKIND_ERROR,DDSC_LOG_BIND_RECORD,dbrc)
        return RTI_FALSE;
    }

    dbrc = DB_Table_create_record(dwintf->_parent._btable,
                                  (DB_Record_T *)&bind_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_CREATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_BIND_RECORD,dbrc)
        return RTI_FALSE;
    }

    DDS_InstanceHandle_from_netio_address(&ih_peer,src_addr);

    if (!DDS_TypePlugin_add_peer(dwintf->datawriter->type_plugin,&ih_peer))
    {
        return RTI_FALSE;
    }

    OSAPI_Memory_zero(bind_entry,sizeof(struct DDS_DataWriterBindEntry));
    bind_entry->source = src_addr->value.guid;
    bind_entry->intf = netio;

    /* Keep track of a range of numbers that are available to the
     * peer. These numbers are only set at match-time. Anything
     * written after high_history_sn is considered current data.
     */
    bind_entry->high_history_sn = b_property->high_history_sn;
    bind_entry->next_new_sn = b_property->next_new_sn;
    bind_entry->state = 0;
    bind_entry->state |= DDS_DATAWRITER_BINDENTRY_STATE_ACTIVE;

    bind_entry->encapsulation_id = b_property->encapsulation_id;

    if (b_property->is_reliable)
    {
        bind_entry->state |= DDS_DATAWRITER_BINDENTRY_STATE_RELIABLE;
        ++dwintf->active_acking_readers;
    }

    dbrc = DB_Table_insert_record(dwintf->_parent._btable,
                                  (DB_Record_T)bind_entry);

    if ((dbrc != DB_RETCODE_OK) && (dbrc != DB_RETCODE_EXISTS))
    {
        DDSC_LOG_RECORD_INSERT(OSAPI_LOGKIND_ERROR,DDSC_LOG_BIND_RECORD,dbrc)
        (void)DB_Table_delete_record(dwintf->_parent._btable,
                                     (DB_Record_T)bind_entry);
        return RTI_FALSE;
    }

    OSAPI_TRACE_DDS("datawriter bound to datareader:",RTI_FALSE)
    OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                DDS_Topic_as_topicdescription(
                        dwintf->datawriter->topic)),RTI_FALSE)
    OSAPI_TRACE_GUID("datawriter",&dwintf->_parent.local_address.value.rtps_guid,RTI_FALSE)
    OSAPI_TRACE_GUID("datareader",&src_addr->value.rtps_guid,RTI_TRUE)

    return RTI_TRUE;
}


/*ci
 * \brief Implementation of the NETIO unbind function
 *
 * \details
 *
 * When a datawriter no longer matches with a datareader the bind entry
 * for the datareader is removed. No further communication with the datareader
 * is possible.
 *
 * \param[in]  netio    NETIO interface to unbind
 * \param[in]  src_addr The address to unbind from
 * \param[in]  dst_intf Interface to unbind from
 * \param[out] existed  Whether a bind existed or not
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 *
 * \sa \ref DDS_DataWriterInterface_bind
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DataWriterInterface_unbind(NETIO_Interface_T *netio,
                        struct NETIO_Address *src_addr,
                        NETIO_Interface_T *dst_intf,
                        RTI_BOOL *existed)
{
    struct DDS_DataWriterInterface *dwintf =
                                    (struct DDS_DataWriterInterface *)netio;
    struct DDS_DataWriterBindEntry *bind_entry = NULL;
    DB_ReturnCode_T dbrc;
    struct DDS_DataWriterBindEntryKey bind_key;
    DDS_InstanceHandle_t ih_peer;

    UNUSED_ARG(dst_intf);

    bind_key.source = src_addr->value.guid;

    OSAPI_TRACE_DDS("datawriter unbinding from datareader:",RTI_FALSE)
    OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                DDS_Topic_as_topicdescription(
                        dwintf->datawriter->topic)),RTI_FALSE)
    OSAPI_TRACE_GUID("datawriter",&dwintf->_parent.local_address.value.rtps_guid,RTI_FALSE)
    OSAPI_TRACE_GUID("datareader",&src_addr->value.rtps_guid,RTI_TRUE)

    dbrc = DB_Table_select_match(dwintf->_parent._btable,
                                DB_TABLE_DEFAULT_INDEX,
                                (DB_Record_T *)&bind_entry,
                                &bind_key);

    if (dbrc == DB_RETCODE_NO_DATA)
    {
        OSAPI_TRACE_DDS("datawriter unbinding from datareader, but no entry exists:",RTI_FALSE)
        OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                    DDS_Topic_as_topicdescription(
                            dwintf->datawriter->topic)),RTI_FALSE)
        OSAPI_TRACE_GUID("datawriter",&dwintf->_parent.local_address.value.rtps_guid,RTI_FALSE)
        OSAPI_TRACE_GUID("datareader",&src_addr->value.rtps_guid,RTI_TRUE)

        if (existed)
        {
            *existed = RTI_FALSE;
        }

        return RTI_TRUE;
    }

    if (dbrc != DB_RETCODE_OK)
    {
        OSAPI_TRACE_DDS("datawriter unbinding from datareader failed to look up entry",RTI_FALSE)
        OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                    DDS_Topic_as_topicdescription(
                            dwintf->datawriter->topic)),RTI_FALSE)
        OSAPI_TRACE_GUID("datawriter",&dwintf->_parent.local_address.value.rtps_guid,RTI_FALSE)
        OSAPI_TRACE_GUID("datareader",&src_addr->value.rtps_guid,RTI_FALSE)
        OSAPI_TRACE_INT32("dbrc",dbrc,RTI_TRUE);

        DDSC_LOG_RECORD_SELECT(OSAPI_LOGKIND_ERROR,DDSC_LOG_BIND_RECORD,dbrc)
        return RTI_FALSE;
    }

    if (existed)
    {
        *existed = RTI_TRUE;
    }

    DDS_InstanceHandle_from_netio_address(&ih_peer,src_addr);

    if (!DDS_TypePlugin_remove_peer(dwintf->datawriter->type_plugin,&ih_peer))
    {
        return RTI_FALSE;
    }

    if ((bind_entry->state & DDS_DATAWRITER_BINDENTRY_STATE_RELIABLE) &&
        (bind_entry->state & DDS_DATAWRITER_BINDENTRY_STATE_ACTIVE))
    {
        --dwintf->active_acking_readers;
    }

    dbrc = DB_Table_delete_record(dwintf->_parent._btable,
                                 (DB_Record_T)bind_entry);
    if (dbrc != DB_RETCODE_OK)
    {
        OSAPI_TRACE_DDS("datawriter unbinding from datareader failed to delete record:",RTI_FALSE)
        OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                    DDS_Topic_as_topicdescription(
                            dwintf->datawriter->topic)),RTI_FALSE)
        OSAPI_TRACE_GUID("datawriter",&dwintf->_parent.local_address.value.rtps_guid,RTI_FALSE)
        OSAPI_TRACE_GUID("datareader",&src_addr->value.rtps_guid,RTI_FALSE)
        OSAPI_TRACE_INT32("dbrc",dbrc,RTI_TRUE);

        DDSC_LOG_RECORD_DELETE(OSAPI_LOGKIND_ERROR,DDSC_LOG_BIND_RECORD,dbrc)
        return RTI_FALSE;
    }

    OSAPI_TRACE_DDS("datawriter unbound from datareader:",RTI_FALSE)
    OSAPI_TRACE_STRING("topic",DDS_TopicDescription_get_name(
                DDS_Topic_as_topicdescription(
                        dwintf->datawriter->topic)),RTI_FALSE)
    OSAPI_TRACE_GUID("datawriter",&dwintf->_parent.local_address.value.rtps_guid,RTI_FALSE)
    OSAPI_TRACE_GUID("datareader",&src_addr->value.rtps_guid,RTI_FALSE)

    return RTI_TRUE;
}

/*ci
 * \brief Implementation of the NETIO_Interface_get_external_interface
 *
 * \details
 *
 * When a datawriter is bound to a downstream interface it is requested to
 * provide which interface and address the downstream interface should use
 * when forwarding a NETIO_Packet.
 *
 * \param[in]   netio_intf   NETIO interface to get the external interface for
 * \param[in]   src_addr     The address to send to
 * \param[out]  dst_intf     The interface to use
 * \param[out]  dst_addr     The destination address to use
 *
 * \return This function always returns RTI_TRUE
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DataWriterInterface_get_external_interface(NETIO_Interface_T *netio_intf,
                                               struct NETIO_Address *src_addr,
                                               NETIO_Interface_T **dst_intf,
                                               struct NETIO_Address *dst_addr)
{
    struct DDS_DataWriterInterface *dwintf =
                                (struct DDS_DataWriterInterface *)netio_intf;
    UNUSED_ARG(src_addr);

    *dst_intf = netio_intf;
    *dst_addr = dwintf->_parent.local_address;

    return RTI_TRUE;
}

/*ci
 * \brief Implementation of the NETIO set_state function
 *
 * \details
 * Set the state of the datawriter. The datawriter does not do anything
 * in this function, thus the state is only set.
 *
 * \param[in] netio NETIO interface to set state on
 * \param[in] state New state
 *
 * \return This function always returns RTI_TRUE
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DataWriterInterface_set_state(NETIO_Interface_T *netio,
                                  NETIO_InterfaceState_T state)
{
    struct DDS_DataWriterInterface *dwintf =
                                       (struct DDS_DataWriterInterface *)netio;

    dwintf->_parent.state = state;

    return RTI_TRUE;
}

/*ci
 * \brief Update the state of a peer interface (datareader)
 *
 * \param[in] dwintf     The datawriter interface
 * \param[in] event      The reason for the change in peer state
 * \param[in] peer_event Additional data for the event
 */
RTI_PRIVATE void
DDS_DataWriterInterface_update_peer(struct DDS_DataWriterInterface *dwintf,
                                    NETIO_EventKind_T event,
                                    struct NETIO_Event_PeerActivity *peer_event)
{
    struct DDS_DataWriterBindEntry *bind_entry = NULL;
    DB_ReturnCode_T dbrc;
    struct DDS_DataWriterBindEntryKey bind_key;

    bind_key.source = peer_event->peer_addr.value.guid;

    dbrc = DB_Table_select_match(dwintf->_parent._btable,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T *)&bind_entry,
                                 &bind_key);

    if (dbrc == DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_RECORD_LOOKUP(OSAPI_LOGKIND_WARNING,DDSC_LOG_BIND_RECORD)
        return;
    }

    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_LOOKUP(OSAPI_LOGKIND_ERROR,DDSC_LOG_BIND_RECORD)
        return;
    }

    if ((bind_entry->state & DDS_DATAWRITER_BINDENTRY_STATE_ACTIVE) &&
            (event == NETIO_EVENTKIND_INACTIVE_PEER))
    {
        bind_entry->state &= ~DDS_DATAWRITER_BINDENTRY_STATE_ACTIVE;
        if (bind_entry->state & DDS_DATAWRITER_BINDENTRY_STATE_RELIABLE)
        {
            --dwintf->active_acking_readers;
        }
    }
    else if (!(bind_entry->state & DDS_DATAWRITER_BINDENTRY_STATE_ACTIVE) &&
              (event == NETIO_EVENTKIND_ACTIVE_PEER))
    {
        bind_entry->state |= DDS_DATAWRITER_BINDENTRY_STATE_ACTIVE;
        if (bind_entry->state & DDS_DATAWRITER_BINDENTRY_STATE_RELIABLE)
        {
            ++dwintf->active_acking_readers;
        }
    }
}

/*ci
 * \brief Implementation of the NETIO_Interface_post_event function
 *
 * \param[in] netio_intf The datawriter interface the event occurred on
 * \param[in] src_intf   The source of the event
 * \param[in] evt        The NETIO event
 *
 * \return This function always returns RTI_TRUE
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DataWriterInterface_post_event(NETIO_Interface_T *netio_intf,
                                   NETIO_Interface_T *src_intf,
                                   struct NETIO_Event *evt)
{
    struct DDS_DataWriterInterface *dwintf =
        (struct DDS_DataWriterInterface *)netio_intf;
    struct DDS_DataWriterImpl *dw =
        (struct DDS_DataWriterImpl *)dwintf->datawriter;
    struct DDS_ReliableReaderActivityChangedStatus *status = NULL;
    struct OSAPI_SystemTime now = OSAPI_TIME_ZERO;
    struct DDSHST_WriterEvent wh_event;
    RTI_BOOL bretval;

    UNUSED_ARG(src_intf);

    bretval = OSAPI_System_get_time(&now);
#if OSAPI_ENABLE_LOG
    /* Not important, can continue */
    if (!bretval)
    {
        DDSC_LOG_SYS_GETTIME(OSAPI_LOGKIND_ERROR)
    }
#else
    IGNORE_RETVAL(bretval);
#endif

    if (!OSAPI_Mutex_take(dwintf->datawriter->write_lock))
    {
        return RTI_FALSE;
    }

    bretval = RTI_TRUE;
    switch (evt->kind)
    {
        case NETIO_EVENTKIND_ACTIVE_PEER:
        case NETIO_EVENTKIND_INACTIVE_PEER:
            status = &dw->reliable_reader_activity_changed_status;

            status->active_count = evt->value.peer_activity.active_total;
            status->active_count_change = evt->value.peer_activity.active_change;
            status->inactive_count = evt->value.peer_activity.inactive_total;
            status->inactive_count_change = evt->value.peer_activity.inactive_change;

            DDS_InstanceHandle_from_netio_address(&status->last_instance_handle,
                                                  &evt->value.peer_activity.peer_addr);

            DDS_DataWriterEvent_on_reliable_reader_activity_changed(dw);
            DDS_DataWriterInterface_update_peer(dwintf,evt->kind,&evt->value.peer_activity);
            break;

        case NETIO_EVENTKIND_PACKET_QUEUED:
            wh_event.data.sn = evt->value.sn;
            wh_event.kind = DDSHST_WRITEREVENT_KIND_ENTRY_STATE_QUEUED;
            DDSHST_Writer_post_event(dw->wh,&wh_event,&now);
            break;

        case NETIO_EVENTKIND_PACKET_IN_PROGRESS:
            wh_event.data.sn = evt->value.sn;
            wh_event.kind = DDSHST_WRITEREVENT_KIND_ENTRY_STATE_IN_PROGRESS;
            DDSHST_Writer_post_event(dw->wh,&wh_event,&now);
            if (wh_event.event_ignored)
            {
                bretval = RTI_FALSE;
            }
            break;

        case NETIO_EVENTKIND_PACKET_SEND_COMPLETED:
            wh_event.data.sn = evt->value.sn;
            wh_event.kind = DDSHST_WRITEREVENT_KIND_ENTRY_STATE_SEND_COMPLETED;
            DDSHST_Writer_post_event(dw->wh,&wh_event,&now);
            break;

        default:
            break;
    }

    if (!OSAPI_Mutex_give(dwintf->datawriter->write_lock))
    {
        return RTI_FALSE;
    }

    return bretval;
}

/*ci
 * \brief Implementation of the NETIO_Interface_lookup_route
 *
 * \details
 *
 * Check if a NETIO interface has a route to a specific destination
 *
 * \param[in]  netio_intf   The source interface
 * \param[in]  dst_reader   The destination address
 * \param[in]  via_intf     The downstream interface
 * \param[in]  via_address  The address to pass to the downstream interface
 * \param[out] route_exists RTI_TRUE if the route existed, RTI_FALSE if not
 *
 *
 * \return RTI_TRUE if the event was handled successfully, RTI_FALSE if not
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_DataWriterInterface_lookup_route(struct NETIO_Interface *netio_intf,
                                     struct NETIO_Address *dst_reader,
                                     struct NETIO_Interface *via_intf,
                                     struct NETIO_Address *via_address,
                                     RTI_BOOL *route_exists)
{
    struct DDS_DataWriterInterface *dwintf =
                                (struct DDS_DataWriterInterface *)netio_intf;
    struct DDS_DataWriterBindEntryKey bind_key;
    struct DDS_DataWriterBindEntry *bind_entry = NULL;
    DB_ReturnCode_T dbrc;
    UNUSED_ARG(via_intf);
    UNUSED_ARG(via_address);

    bind_key.source = dst_reader->value.guid;

    *route_exists = RTI_FALSE;

    dbrc = DB_Table_select_match(dwintf->_parent._btable,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&bind_entry,
                                 (DB_Key_T)&bind_key);

    if ((dbrc != DB_RETCODE_NO_DATA) && (dbrc != DB_RETCODE_OK))
    {
        return RTI_FALSE;
    }

    if (dbrc == DB_RETCODE_OK)
    {
        *route_exists = RTI_TRUE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief The Datawriter NETIO interface implementation
 *
 * \details
 *
 * The Datawriter NETIO interface communicates either with RTPS or loopback.
 * It is always on top of the stack and does not receive any data from a
 * datareader.
 */
LINK_SECTION_DATA_SDRAM
RTI_PRIVATE struct NETIO_InterfaceI DDS_DataWriterInterface_fv_Intf =
{
    RT_COMPONENTI_BASE,
    DDS_DataWriterInterface_send,           /* send */
    DDS_DataWriterInterface_acknack,        /* ack */
    DDS_DataWriterInterface_request,        /* request */
    DDS_DataWriterInterface_return_loan,    /* return_loan*/
    DDS_DataWriterInterface_xmit_remove,    /* xmit_remove */
    DDS_DataWriterInterface_add_route,      /* add_route */
    DDS_DataWriterInterface_delete_route,   /* remove_peer */
    NULL,                                   /* reserve_public_address */
    DDS_DataWriterInterface_bind,           /* bind */
    DDS_DataWriterInterface_unbind,         /* unbind */
    NULL,                                   /* receive not needed */
    DDS_DataWriterInterface_get_external_interface, /* get_external interface */
    NULL,                                   /* external_bind */
    NULL,                                   /* external_unbind */
    DDS_DataWriterInterface_set_state,
    NULL,                                   /* release_address */
    NULL,                                   /* resolve_address */
    NULL,                                   /* get_route_table */
    DDS_DataWriterInterface_post_event,
    DDS_DataWriterInterface_lookup_route,
    NULL,
    NULL
};

/* ------------------------------------------------------------------------ */
/*                   Plugin factory                                         */
/* ------------------------------------------------------------------------ */
/*ci
 * \brief Creates a new datawriter interface.
 *
 * \details
 * Implementation of the RT ComponentFactory create_component method. This
 * method is never called directly, only via a factory interface type.
 *
 * \param[in] factory   The factory creating the component
 * \param[in] property  The component property
 * \param[in] listener  The component listener
 *
 * \return A new component on success, NULL on failure
 *
 * \sa \ref DDS_DataWriterInterfaceFactory_delete_component
 */
MUST_CHECK_RETURN RTI_PRIVATE RT_Component_T*
DDS_DataWriterInterfaceFactory_create_component(
                                        struct RT_ComponentFactory *factory,
                                        struct RT_ComponentProperty *property,
                                        struct RT_ComponentListener *listener)
{
    struct DDS_DataWriterInterface *retval = NULL;

    retval = DDS_DataWriterInterface_create(
            (struct DDS_DataWriterInterfaceFactory*)factory,
            (const struct DDS_DataWriterInterfaceProperty*)property,
            (const struct NETIO_InterfaceListener*)listener);

    if (retval != NULL)
    {
        return &retval->_parent._parent;
    }
    else
    {
        return NULL;
    }
}

#ifndef RTI_CERT
/*ci
 * \brief Delete a datawriter interface
 *
 * \details
 * Implementation of the RT ComponentFactory delete method. This method deletes
 * a datawriter interface. It is never called directly, only via a factory
 * interface type.
 *
 * \param[in] factory   The factory that created the component
 * \param[in] component The component to be deleted
 *
 * \sa \ref DDS_DataWriterInterfaceFactory_create_component
 */
RTI_PRIVATE void
DDS_DataWriterInterfaceFactory_delete_component(
                                        struct RT_ComponentFactory *factory,
                                        RT_Component_T *component)
{
    struct DDS_DataWriterInterface *self =
                                    (struct DDS_DataWriterInterface*)component;
    UNUSED_ARG(factory);

    DDS_DataWriterInterface_delete(self);
}
#endif /* !RTI_CERT */

MUST_CHECK_RETURN RTI_PRIVATE struct RT_ComponentFactory*
DDS_DataWriterInterfaceFactory_initialize(
        struct RT_ComponentFactoryProperty*property,
        struct RT_ComponentFactoryListener *listener);

#ifndef RTI_CERT
RTI_PRIVATE void
DDS_DataWriterInterfaceFactory_finalize(
        struct RT_ComponentFactory *factory,
        struct RT_ComponentFactoryProperty **property,
        struct RT_ComponentFactoryListener **listener);
#endif /* !RTI_CERT */

/*ci
 * \brief Implementation of the RT ComponentFactoryI interface
 */
#ifndef RTI_CERT
LINK_SECTION_DATA_SDRAM
RTI_PRIVATE struct RT_ComponentFactoryI DDS_DataWriterInterfaceFactory_fv_Intf =
{
    DDSWI_INTERFACE_INTERFACE_ID,
    DDS_DataWriterInterfaceFactory_initialize,
    DDS_DataWriterInterfaceFactory_finalize,
    DDS_DataWriterInterfaceFactory_create_component,
    DDS_DataWriterInterfaceFactory_delete_component,
    NULL,
    NULL
};
#else
LINK_SECTION_DATA_SDRAM
RTI_PRIVATE struct RT_ComponentFactoryI DDS_DataWriterInterfaceFactory_fv_Intf =
{
    DDSWI_INTERFACE_INTERFACE_ID,
    DDS_DataWriterInterfaceFactory_initialize,
    NULL, /* DDS_DataWriterInterfaceFactory_finalize, */
    DDS_DataWriterInterfaceFactory_create_component,
    NULL, /* DDS_DataWriterInterfaceFactory_delete_component, */
    NULL,
    NULL
};
#endif /* !RTI_CERT */

/*ci
 * \brief Datawriter NETIO interface factory
 *
 * \details
 * The Datawriter NETIO interface class is implemented as a singleton, there are
 * no shared resources between interfaces.
 */
LINK_SECTION_DATA_SDRAM
RTI_PRIVATE
struct DDS_DataWriterInterfaceFactory DDS_DataWriterInterfaceFactory_fv_Factory =
{
  {
     &DDS_DataWriterInterfaceFactory_fv_Intf,
     NULL,
     {{{0,0}}}
  }
};

/*ci
 * \brief Initialize the datawriter NETIO interface factory
 *
 * \details
 * The Datawriter NETIO specific implementation of the RT ComponentFactory
 * initialize method. This method is called by RT when the factory is
 * registered.
 *
 * \param[in] property The properties registered with the Datawriter NETIO
 *                     interface factory
 * \param[in] listener The listener registered with the Datareader NETIO
 *                     interface factory
 *
 * \return A fully initialized factory
 *
 * \sa \ref DDS_DataWriterInterfaceFactory_finalize
 */
MUST_CHECK_RETURN RTI_PRIVATE struct RT_ComponentFactory*
DDS_DataWriterInterfaceFactory_initialize(
        struct RT_ComponentFactoryProperty *property,
        struct RT_ComponentFactoryListener *listener)
{
    struct DDS_DataWriterInterfaceFactory *factory =
                                    &DDS_DataWriterInterfaceFactory_fv_Factory;
    UNUSED_ARG(property);
    UNUSED_ARG(listener);

    DDS_DataWriterInterfaceFactory_fv_Factory._parent._factory = &factory->_parent;

    return &DDS_DataWriterInterfaceFactory_fv_Factory._parent;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize the datawriter NETIO interface factory
 *
 * \details
 * The Datawriter NETIO specific implementation of the RT ComponentFactory
 * finalize method. This method is called by RT when the factory is
 * unregistered.
 *
 * \param[in]  c_factory  The factory to finalize
 * \param[out] property   The property the factory was registered with
 * \param[out] listener   The listener the factory was registered with
 *
 * \sa \ref DDS_DataWriterInterfaceFactory_initialize
 */
RTI_PRIVATE void
DDS_DataWriterInterfaceFactory_finalize(
        struct RT_ComponentFactory *c_factory,
        struct RT_ComponentFactoryProperty **property,
        struct RT_ComponentFactoryListener **listener)
{
    UNUSED_ARG(property);
    UNUSED_ARG(listener);
    UNUSED_ARG(c_factory);
}
#endif /* RTI_CERT */

struct RT_ComponentFactoryI*
DDS_DataWriterInterfaceFactory_get_interface(void)
{
    return &DDS_DataWriterInterfaceFactory_fv_Intf;
}

/*ci @} */
