/*
 * FILE: DDS_ipcLiveliness.c - IPC Liveliness channel
 *
 * Copyright (c) 2017-2025 Real-Time Innovations,Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 */
/*ce
 */
#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#include "DDS_IpcLiveliness.h"

/*** SOURCE_BEGIN ***/
DDS_Boolean
DDS_IpcLiveliness_assert_routes(
    struct DDS_IpcLiveliness *ipc,
    struct DDS_RemoteParticipantImpl *remote_dp)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    struct DDS_DataReaderQos *dr_qos = NULL;
    struct DDS_DataWriterQos *dw_qos = NULL;
    DDS_ReliabilityQosPolicyKind orig_kind;

    dr_qos = &ipc->dr_qos;
    dw_qos = &ipc->dw_qos;

    /* Restore the original kind */
    orig_kind = dr_qos->reliability.kind;

    /* if received a valid builtin endpoint qos mask use it */
    if (DDS_BUILTIN_ENDPOINT_QOS_IS_VALID(
                                remote_dp->data.builtin_endpoint_qos_mask))
    {
        if (remote_dp->data.builtin_endpoint_qos_mask &
            DDS_BUILTIN_ENDPOINT_QOS_BIT_BEST_EFFORT_PARTICIPANT_MESSAGE_DATA_READER)
        {
            dr_qos->reliability.kind = DDS_BEST_EFFORT_RELIABILITY_QOS;
        }
        else
        {
            dr_qos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
        }
    }
    else
    {
        struct DDS_VendorId vendor_id = {{RTI_CONNEXT_PRO_VENDOR_ID_MAJOR,
                                          RTI_CONNEXT_PRO_VENDOR_ID_MINOR}};

        /* Remote participant is not announcing a builtinEndpointQosMask.
         *
         * RTI Connext DDS Pro 5.2 and above: we assume remote participant
         * uses the same configuration we use.
         */
        if (DDS_VendorId_is_equal(&remote_dp->data.rtps_vendor_id, &vendor_id) &&
            ((remote_dp->data.product_version.major > (DDS_Char)0x05) ||
             ((remote_dp->data.product_version.major == (DDS_Char)0x05) &&
              (remote_dp->data.product_version.minor > (DDS_Char)0x01))))
        {
            dr_qos->reliability.kind =
                ipc->participant->builtin_data.participant_message_reader_reliability_kind;
        }
        else
        {
            dr_qos->reliability.kind = DDS_BEST_EFFORT_RELIABILITY_QOS;
        }
    }

    if (DDS_DataReader_add_peer(
                (DDS_DataReader*)ipc->reader,
                &remote_dp->orig_key,
                dw_qos,
                dw_qos->protocol.rtps_object_id) != DDS_RETCODE_OK)
    {
       DDSC_LOG_IPC_CHANNEL_ADD_PEER(OSAPI_LOGKIND_ERROR,
                                      NDDS_BUILTIN_IPC_CHANNEL_MESSAGE_DATA,
                                      DDSC_LOG_DATAREADER_ENTITY);
        goto done;
    }

    if (DDS_DataWriter_add_peer(
                (DDS_DataWriter*)ipc->writer,
                &remote_dp->orig_key,
                dr_qos,
                dr_qos->protocol.rtps_object_id) != DDS_RETCODE_OK)
    {
         DDSC_LOG_IPC_CHANNEL_ADD_PEER(OSAPI_LOGKIND_ERROR,
                                       NDDS_BUILTIN_IPC_CHANNEL_MESSAGE_DATA,
                                       DDSC_LOG_DATAWRITER_ENTITY)
         if (DDS_DataReader_remove_peer(
                (DDS_DataReader*)ipc->reader,
                    &remote_dp->orig_key,
                    dw_qos->protocol.rtps_object_id) != DDS_RETCODE_OK)
        {
            DDSC_LOG_IPC_CHANNEL_REMOVE_PEER(OSAPI_LOGKIND_ERROR,
                                NDDS_BUILTIN_IPC_CHANNEL_MESSAGE_DATA,
                                DDSC_LOG_DATAREADER_ENTITY)
        }
        goto done;
    }

    retval = DDS_BOOLEAN_TRUE;

done:

    dr_qos->reliability.kind = orig_kind;

    return retval;
}

DDS_Boolean
DDS_IpcLiveliness_remove_routes(
        struct DDS_IpcLiveliness *ipc,
        struct DDS_RemoteParticipantImpl *remote_dp)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    struct DDS_DataReaderQos *dr_qos = NULL;
    struct DDS_DataWriterQos *dw_qos = NULL;

    dr_qos = &ipc->dr_qos;
    dw_qos = &ipc->dw_qos;

    if (DDS_DataReader_remove_peer(
                (DDS_DataReader*)ipc->reader,
                &remote_dp->orig_key,
                dw_qos->protocol.rtps_object_id) != DDS_RETCODE_OK)
    {
         DDSC_LOG_IPC_CHANNEL_REMOVE_PEER(OSAPI_LOGKIND_ERROR,
                                  NDDS_BUILTIN_IPC_CHANNEL_MESSAGE_DATA,
                                  DDSC_LOG_DATAREADER_ENTITY)
        goto done;
    }

    if (DDS_DataWriter_remove_peer(
                (DDS_DataWriter*)ipc->writer,
                &remote_dp->orig_key,
                dr_qos->protocol.rtps_object_id) != DDS_RETCODE_OK)
    {
         DDSC_LOG_IPC_CHANNEL_REMOVE_PEER(OSAPI_LOGKIND_ERROR,
                                  NDDS_BUILTIN_IPC_CHANNEL_MESSAGE_DATA,
                                  DDSC_LOG_DATAWRITER_ENTITY)
        goto done;
    }

    retval = DDS_BOOLEAN_TRUE;

done:

    return retval;
}

RTI_PRIVATE DDS_Boolean
DDS_IpcLiveliness_refresh_liveliness(
    struct DDS_IpcLiveliness *ipc,
    struct DDS_Liveliness_ParticipantMessageData *sample)
{
    DDS_Boolean ret_value = DDS_BOOLEAN_FALSE;
    struct DDS_DomainParticipantImpl *participant =
                (struct DDS_DomainParticipantImpl *)ipc->participant;
    DB_Cursor_T cursor = NULL;
    DB_ReturnCode_T dbrc = DB_RETCODE_ERROR;
    DDS_InstanceHandle_t remote_handle = DDS_HANDLE_NIL;
    struct DDS_BuiltinTopicKey_t remote_key = DDS_BUILTINTOPICKEY_UNKNOWN;
    RemoteWriterMatchEntry_t *record = NULL;
    struct RTPS_Guid remote_guid = RTPS_GUID_UNKNOWN;
    struct DDS_DataReaderImpl *local_reader = NULL;
    NETIO_Packet_T packet = NETIO_Packet_INITIALIZER;
    RTI_BOOL recv_res = RTI_FALSE;
    CDR_Octet msg_kind = 0;
    RTI_UINT32 liveliness_kind = 0;

    msg_kind = sample->key.kind[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID];

    switch (msg_kind)
    {
        case PARTICIPANT_MESSAGE_DATA_KIND_AUTOMATIC_LIVELINESS_UPDATE:
            liveliness_kind = NETIO_RTPS_FLAGS_AUTO_LIVELINESS;
        break;

    case PARTICIPANT_MESSAGE_DATA_KIND_MANUAL_LIVELINESS_UPDATE:
            liveliness_kind = NETIO_RTPS_FLAGS_MANUAL_LIVELINESS;
        break;

    default:
        /* Should never get here */
        DDSC_LOG_MESSAGE_DATA_UNKNOWN_LIVELINESS_KIND(
                OSAPI_LOGKIND_WARNING, msg_kind);
        goto done;
    }

    OSAPI_Memory_copy(remote_handle.octet,
                      sample->key.participant_guid_prefix,
                      sizeof(sample->key.participant_guid_prefix));
    DDS_InstanceHandle_set_suffix(&remote_handle, 0);
    DDS_InstanceHandle_to_rtps(&remote_guid, &remote_handle);

    remote_key.value[0] = remote_guid.prefix.host_id;
    remote_key.value[1] = remote_guid.prefix.app_id ;
    remote_key.value[2] = remote_guid.prefix.instance_id;
    remote_key.value[3] = remote_guid.object_id;

    dbrc = DB_Table_select_all(ipc->liveliness_match_table,
                               DB_TABLE_DEFAULT_INDEX,&cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,
                              ipc->liveliness_match_table,
                              dbrc)
        goto done;
    }

    packet.info.rtps_flags = liveliness_kind;
    packet.source.kind = NETIO_ADDRESS_KIND_INTRA;
    packet.source.port = 0;

    dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&record);
    while (dbrc == DB_RETCODE_OK)
    {
        if (!DDS_BuiltinTopicKey_prefix_equals(&record->remote_writer_key,
                                              &remote_key))
        {
            goto next;
        }
        /* if the writer belongs to the participant which sent the data(m)
         * get the reader is send it a liveliness message */
        dbrc = DB_Table_select_match(participant->local_reader_table,
                                     DB_TABLE_DEFAULT_INDEX,
                                     (DB_Record_T*)&local_reader,
                                     (DB_Key_T)&record->local_reader_oid);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_RECORD_LOOKUP(OSAPI_LOGKIND_ERROR,
                                   DDSC_LOG_DATAREADER_RECORD)
            goto next;
        }

        NETIO_Address_set_guid_from_key(
            &packet.source,0,
            (struct NETIO_AddressInt32*)&record->remote_writer_key);
        recv_res = NETIO_Interface_receive(
                        local_reader->dr_intf,NULL,
                        NULL,&packet);
        if (!recv_res)
        {
            DDSC_LOG_REFRESH_LIVELINESS_FAILED(OSAPI_LOGKIND_ERROR)
        }
next:
        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&record);
    }

    DB_Cursor_finish(ipc->liveliness_match_table,cursor);

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    ret_value = DDS_BOOLEAN_TRUE;

done:

    return ret_value;
}

RTI_PRIVATE void
DDS_IpcLiveliness_on_data_available(
        void *listener_data,
        DDS_DataReader *reader)
{
    struct DDS_IpcLiveliness *ipc = (struct DDS_IpcLiveliness*)listener_data;
    struct DDS_Liveliness_ParticipantMessageData *sample = NULL;
    struct DDS_SampleInfo *info;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    int i = 0;
    RTI_INT32 seq_len = 0;

    UNUSED_ARG(reader);

    do
    {
        /* WARNING: Sample memory is loaned from the DataReader.
         * Both sequence "rx_sample_seq" and "rx_info_seq" should not be modified
         * using any of the Sequence API operations (such as set_length or
         * set_maximum). */
        retcode = DDS_Liveliness_ParticipantMessageDataDataReader_take(
                        ipc->reader,
                        &ipc->data,
                        &ipc->info,
                        DDS_LENGTH_UNLIMITED,
                        DDS_ANY_SAMPLE_STATE,
                        DDS_ANY_VIEW_STATE,
                        DDS_ANY_INSTANCE_STATE);

        if (retcode != DDS_RETCODE_OK)
        {
            break;
        }

        seq_len = DDS_SampleInfoSeq_get_length(&ipc->info);
        for (i = 0; i < seq_len; ++i)
        {
            info = DDS_SampleInfoSeq_get_reference(&ipc->info,i);
            sample = DDS_Liveliness_ParticipantMessageDataSeq_get_reference(&ipc->data, i);
            if (!info->valid_data)
            {
                continue;
            }

            if (!DDS_IpcLiveliness_refresh_liveliness(ipc,sample))
            {
                DDSC_LOG_IPC_CHANNEL_PROCESS_SAMPLE(OSAPI_LOGKIND_ERROR,
                                        NDDS_BUILTIN_IPC_CHANNEL_MESSAGE_DATA);
            }
        }

        retcode = DDS_Liveliness_ParticipantMessageDataDataReader_return_loan(
                            ipc->reader,&ipc->data,&ipc->info);
    } while (retcode == DDS_RETCODE_OK);


    if (retcode != DDS_RETCODE_OK && retcode != DDS_RETCODE_NO_DATA)
    {
        DDSC_LOG_IPC_CHANNEL_RETURN_LOAN(OSAPI_LOGKIND_ERROR,
                                         NDDS_BUILTIN_IPC_CHANNEL_MESSAGE_DATA)
    }
}

DDS_Boolean
DDS_IpcLiveliness_delete(struct DDS_IpcLiveliness *ipc)
{
    DDS_DomainParticipant *participant = ipc->participant;
    DB_ReturnCode_T dbrc;
    DDS_Subscriber *builtin_sub = NULL;
    DDS_Publisher *builtin_pub = NULL;

    if (DDS_DataReaderQos_finalize(&ipc->dr_qos) != DDS_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (DDS_DataWriterQos_finalize(&ipc->dw_qos) != DDS_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (ipc->min_lease_idx != NULL)
    {
        dbrc = DB_Table_delete_index(
                            participant->local_writer_table,
                            ipc->min_lease_idx);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_DELETE_INDEX(OSAPI_LOGKIND_ERROR,
                    participant->local_writer_table,dbrc)
            return DDS_BOOLEAN_FALSE;
        }
        ipc->min_lease_idx = NULL;
    }

    if (ipc->automatic_idx != NULL)
    {
        dbrc = DB_Table_delete_index(participant->local_writer_table,
                                     ipc->automatic_idx);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_DELETE_INDEX(OSAPI_LOGKIND_ERROR,
                    participant->local_writer_table,dbrc)
            return DDS_BOOLEAN_FALSE;
        }
        ipc->automatic_idx = NULL;
    }

    if (ipc->liveliness_match_table != NULL)
    {
        dbrc = DB_Database_delete_table(participant->database,
                                        ipc->liveliness_match_table);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_TABLE_DELETE(OSAPI_LOGKIND_ERROR,
                                  ipc->liveliness_match_table,
                                  dbrc)
            return DDS_BOOLEAN_FALSE;
        }
        ipc->liveliness_match_table = NULL;
    }

    if (!CDR_OctetSeq_finalize(&ipc->liveliness_sample.data))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_Liveliness_ParticipantMessageDataKey_finalize(
                                &ipc->liveliness_sample.key))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_SampleInfoSeq_finalize(&ipc->info))
    {
        return DDS_BOOLEAN_FALSE;
    }

    builtin_pub = DDS_DomainParticipant_get_builtin_publisher(participant);
    if (builtin_pub == NULL)
    {
        return DDS_BOOLEAN_FALSE;
    }

    builtin_sub = DDS_DomainParticipant_get_builtin_subscriber(participant);
    if (builtin_sub == NULL)
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (DDS_RETCODE_OK != DDS_Publisher_delete_datawriter(builtin_pub,
            DDS_Liveliness_ParticipantMessageDataDataWriter_as_datawriter(
                    ipc->writer)))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (DDS_RETCODE_OK != DDS_Subscriber_delete_datareader(builtin_sub,
        DDS_Liveliness_ParticipantMessageDataDataReader_as_datareader(
                ipc->reader)))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (DDS_RETCODE_OK != DDS_DomainParticipant_delete_topic(participant,
                                                             ipc->topic))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (DDS_DomainParticipant_unregister_type(participant,
                DDS_PARTICIPANT_MESSAGE_DATA_TYPE_NAME) !=
                DDS_Liveliness_ParticipantMessageDataTypePlugin_get())
    {
        return DDS_BOOLEAN_FALSE;
    }

    OSAPI_Heap_free_string(ipc);

    return DDS_BOOLEAN_TRUE;
}

DDS_Boolean
DDS_IpcLiveliness_enable(struct DDS_IpcLiveliness *ipc)
{
    if (DDS_Entity_enable(
            DDS_Topic_as_entity(ipc->topic)) != DDS_RETCODE_OK)
    {
        DDSC_LOG_ENTITY_ENABLE(OSAPI_LOGKIND_ERROR,DDSC_LOG_TOPIC_ENTITY)
        return DDS_BOOLEAN_FALSE;
    }

    if (DDS_Entity_enable(
            DDS_DataWriter_as_entity(ipc->writer)) != DDS_RETCODE_OK)
    {
        DDSC_LOG_ENTITY_ENABLE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAWRITER_ENTITY)
        return DDS_BOOLEAN_FALSE;
    }

    if (DDS_Entity_enable(
            DDS_DataReader_as_entity(ipc->reader)) != DDS_RETCODE_OK)
    {
        DDSC_LOG_ENTITY_ENABLE(OSAPI_LOGKIND_ERROR,DDSC_LOG_DATAREADER_ENTITY)
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

RTI_PRIVATE struct DDS_IpcLiveliness*
DDS_IpcLiveliness_create(DDS_DomainParticipant *participant,
                         struct DDS_DomainParticipantQos *dp_qos,
                         struct DDS_ParticipantBuiltinTopicData *builtin_data)
{
    struct DDS_IpcLiveliness *ipc = NULL;
    struct DDS_IpcLiveliness *retval = NULL;
    DDS_Subscriber *builtin_subscriber = NULL;
    DDS_Publisher *builtin_publisher = NULL;
    struct DDS_TopicQos topic_qos = DDS_TOPIC_QOS_DEFAULT;
    struct DDS_DataReaderListener dr_listener = DDS_DataReaderListener_INITIALIZER;
    struct DB_TableProperty tbl_prop = DB_TableProperty_INITIALIZER;
    DB_ReturnCode_T dbrc;
    struct DB_IndexProperty idx_prop = DB_IndexProperty_INITIALIZER;
    DDS_InstanceHandle_t instance_handle;

    OSAPI_Heap_allocate_struct(&ipc,struct DDS_IpcLiveliness);
    if (ipc == NULL)
    {
        return NULL;
    }
    OSAPI_Memory_zero(ipc,sizeof(struct DDS_IpcLiveliness));

    if (DDS_DataReaderQos_initialize(&ipc->dr_qos) != DDS_RETCODE_OK)
    {
        OSAPI_Heap_free_struct(ipc);
        return NULL;
    }

    if (DDS_DataWriterQos_initialize(&ipc->dw_qos) != DDS_RETCODE_OK)
    {
        OSAPI_Heap_free_struct(ipc);
        return NULL;
    }

    ipc->participant = participant;
    ipc->liveliness_timer_duration = DDS_DURATION_INFINITE;

    tbl_prop.max_records =
        (RTI_SIZE_T)(dp_qos->resource_limits.remote_writer_allocation *
                     dp_qos->resource_limits.local_reader_allocation);

    ipc->liveliness_match_table = NULL;
    dbrc = DB_Database_create_table(&ipc->liveliness_match_table,
                                    participant->database,
                                    DDS_REMOTE_MATCH_DW_TABLE_NAME,
                                    sizeof(struct RemoteWriterMatchEntry_t),
                                    DDS_DomainParticipant_matched_dw_compare,
                                    &tbl_prop);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_CREATE(OSAPI_LOGKIND_ERROR,
                              DDS_REMOTE_MATCH_DW_TABLE_NAME,
                              dbrc);
        goto done;
    }

    dbrc = DB_Table_create_index(participant->local_writer_table,
                                 &ipc->min_lease_idx,
                                 DDS_DataWriterImpl_compare_lease_duration,
                                 &idx_prop);

    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_CREATE_INDEX(OSAPI_LOGKIND_ERROR,participant->local_writer_table,dbrc)
        goto done;
    }

    dbrc = DB_Table_create_index(participant->local_writer_table,
                                 &ipc->automatic_idx,
                                 DDS_DataWriterImpl_compare_automatic_liveliness_kind,
                                 &idx_prop);

    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_CREATE_INDEX(OSAPI_LOGKIND_ERROR,participant->local_writer_table,dbrc)
        goto done;
    }

    /* Initialize liveliness sample.
     * Reason for not calling DDS_Liveliness_ParticipantMessageData_initialize():
     * this function would allocate the sequence field 'data' in the
     * sample. We do not need that, as this is sent always with length 0.
     * This is why instead only the sequence is initialized with
     * length = maximum = 0
     */
    if (!CDR_OctetSeq_initialize(&ipc->liveliness_sample.data))
    {
        goto done;
    }
    if (!DDS_Liveliness_ParticipantMessageDataKey_initialize(
                                 &ipc->liveliness_sample.key))
    {
        goto done;
    }

    instance_handle = DDS_Entity_get_instance_handle(
                          DDS_DomainParticipant_as_entity(participant));
    OSAPI_Memory_copy(ipc->liveliness_sample.key.participant_guid_prefix,
                      instance_handle.octet,
                      sizeof(DDS_GuidPrefix_t));

    if (!DDS_Liveliness_ParticipantMessageDataSeq_initialize(&ipc->data))
    {
        return NULL;
    }

    if (!DDS_SampleInfoSeq_initialize(&ipc->info))
    {
        return NULL;
    }

    builtin_publisher = DDS_DomainParticipant_get_builtin_publisher(
                                participant);
    if (builtin_publisher == NULL)
    {
        DDSC_LOG_IPC_INIT_FAILED(OSAPI_LOGKIND_ERROR);
        goto done;
    }

    builtin_subscriber = DDS_DomainParticipant_get_builtin_subscriber(
                                participant);
    if (builtin_subscriber == NULL)
    {
        DDSC_LOG_IPC_INIT_FAILED(OSAPI_LOGKIND_ERROR);
        goto done;
    }

    if (DDS_DomainParticipant_register_type(participant,
                DDS_PARTICIPANT_MESSAGE_DATA_TYPE_NAME,
                DDS_Liveliness_ParticipantMessageDataTypePlugin_get())
                != DDS_RETCODE_OK)
    {
        DDSC_LOG_IPC_CHANNEL_TYPE_REGISTER(OSAPI_LOGKIND_ERROR,0)
        goto done;
    }

    topic_qos.management.is_hidden = DDS_BOOLEAN_TRUE;
    topic_qos.management.is_announced = DDS_BOOLEAN_FALSE;

    ipc->topic = DDS_DomainParticipant_create_topic(participant,
        NDDS_BUILTIN_IPC_TOPIC_NAME_MESSAGE_DATA,
        DDS_PARTICIPANT_MESSAGE_DATA_TYPE_NAME,
        &topic_qos,
        NULL,
        DDS_STATUS_MASK_NONE);

    if (ipc->topic == NULL)
    {
        DDSC_LOG_IPC_CHANNEL_TOPIC_CREATE(OSAPI_LOGKIND_ERROR,
                                    NDDS_BUILTIN_IPC_CHANNEL_MESSAGE_DATA);
        goto done;
    }

    ipc->dr_qos.management.is_hidden = DDS_BOOLEAN_TRUE;
    ipc->dr_qos.management.is_announced = DDS_BOOLEAN_FALSE;
    ipc->dr_qos.protocol.rtps_object_id = RTPS_OBJECT_ID_READER_IPC_MESSAGE_DATA;
    ipc->dr_qos.reliability.kind = builtin_data->participant_message_reader_reliability_kind;
    ipc->dr_qos.durability.kind = DDS_TRANSIENT_LOCAL_DURABILITY_QOS;
    ipc->dr_qos.history.kind = DDS_KEEP_LAST_HISTORY_QOS;
    ipc->dr_qos.history.depth = 1;
    ipc->dr_qos.resource_limits.max_instances =
                    2 * dp_qos->resource_limits.remote_participant_allocation;
    ipc->dr_qos.resource_limits.max_samples_per_instance = 1;
    ipc->dr_qos.resource_limits.max_samples =
    ipc->dr_qos.resource_limits.max_instances *
    ipc->dr_qos.resource_limits.max_samples_per_instance;
    ipc->dr_qos.reader_resource_limits.max_remote_writers =
                    dp_qos->resource_limits.remote_participant_allocation;
    ipc->dr_qos.reader_resource_limits.max_routes_per_writer =
                        DDS_DEFAULT_MAX_LOCATORS_PER_DISCOVERED_PARTICIPANT;

    dr_listener.as_listener.listener_data = ipc;
    dr_listener.on_data_available = DDS_IpcLiveliness_on_data_available;
    ipc->reader = DDS_Liveliness_ParticipantMessageDataDataReader_narrow(
                    DDS_Subscriber_create_datareader(
                                builtin_subscriber,
                                DDS_Topic_as_topicdescription(ipc->topic),
                                &ipc->dr_qos,
                                &dr_listener,
                                DDS_DATA_AVAILABLE_STATUS));

    if (ipc->reader == NULL)
    {
        DDSC_LOG_IPC_CHANNEL_READER_CREATE(OSAPI_LOGKIND_ERROR,
                                        NDDS_BUILTIN_IPC_CHANNEL_MESSAGE_DATA);
        goto done;
    }

    ipc->dw_qos.management.is_hidden = DDS_BOOLEAN_TRUE;
    ipc->dw_qos.management.is_announced = DDS_BOOLEAN_FALSE;
    ipc->dw_qos.protocol.rtps_object_id = RTPS_OBJECT_ID_WRITER_IPC_MESSAGE_DATA;
    ipc->dw_qos.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    ipc->dw_qos.durability.kind = DDS_TRANSIENT_LOCAL_DURABILITY_QOS;
    ipc->dw_qos.history.kind = DDS_KEEP_LAST_HISTORY_QOS;
    ipc->dw_qos.history.depth = 1;
    ipc->dw_qos.resource_limits.max_instances = 2;
    ipc->dw_qos.resource_limits.max_samples_per_instance = 1;
    ipc->dw_qos.resource_limits.max_samples =
                                ipc->dw_qos.resource_limits.max_instances *
                                ipc->dw_qos.resource_limits.max_samples_per_instance;
    ipc->dw_qos.writer_resource_limits.max_remote_readers =
                        dp_qos->resource_limits.remote_participant_allocation;
    ipc->dw_qos.writer_resource_limits.max_routes_per_reader =
                            DDS_DEFAULT_MAX_LOCATORS_PER_DISCOVERED_PARTICIPANT;

    ipc->writer = DDS_Liveliness_ParticipantMessageDataDataWriter_narrow(
                        DDS_Publisher_create_datawriter(
                                builtin_publisher,
                                ipc->topic,
                                &ipc->dw_qos,
                                NULL,
                                DDS_STATUS_MASK_NONE));

    if (ipc->writer == NULL)
    {
        DDSC_LOG_IPC_CHANNEL_WRITER_CREATE(OSAPI_LOGKIND_ERROR,
                                        NDDS_BUILTIN_IPC_CHANNEL_MESSAGE_DATA);
        goto done;
    }

    retval = ipc;

done:

    if (retval == NULL)
    {
        DDS_IpcLiveliness_delete(ipc);
    }

    return retval;
}

void
DDS_IpcLiveliness_unreserve(DDS_DomainParticipant *participant)
{
    DB_ReturnCode_T dbrc;

    participant->ipc_liveliness = NULL;
    if (participant->ipc_livelines_type != NULL)
    {
        dbrc = DB_Table_delete_record(participant->type_table,
                               participant->ipc_livelines_type);
        IGNORE_RETVAL(dbrc);
        participant->ipc_livelines_type = NULL;
    }
    if (participant->ipc_livelines_topic != NULL)
    {
        dbrc = DB_Table_delete_record(participant->topic_table,
                               participant->ipc_livelines_topic);
        IGNORE_RETVAL(dbrc);
        participant->ipc_livelines_topic = NULL;
    }
    if (participant->ipc_livelines_reader != NULL)
    {
        dbrc = DB_Table_delete_record(participant->local_reader_table,
                               participant->ipc_livelines_reader);
        IGNORE_RETVAL(dbrc);
        participant->ipc_livelines_reader = NULL;

    }
    if (participant->ipc_livelines_writer != NULL)
    {
        dbrc = DB_Table_delete_record(participant->local_writer_table,
                               participant->ipc_livelines_writer);
        IGNORE_RETVAL(dbrc);
        participant->ipc_livelines_writer = NULL;
    }
}

DDS_Boolean
DDS_IpcLiveliness_reserve(DDS_DomainParticipant *participant)
{
    DB_ReturnCode_T dbrc;
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;

    participant->ipc_liveliness = NULL;
    participant->ipc_livelines_type = NULL;
    participant->ipc_livelines_topic = NULL;
    participant->ipc_livelines_reader = NULL;
    participant->ipc_livelines_writer = NULL;

    /* This is necessary to prevent an application from consuming the required
     * IPC resoruces before IPC liveliness is possibly started.
     */
    dbrc = DB_Table_create_record(participant->type_table,
                          (DB_Record_T*)&participant->ipc_livelines_type);
    if (dbrc != DB_RETCODE_OK)
    {
        goto done;
    }

    dbrc = DB_Table_create_record(participant->topic_table,
                          (DB_Record_T*)&participant->ipc_livelines_topic);
    if (dbrc != DB_RETCODE_OK)
    {
        goto done;
    }

    dbrc = DB_Table_create_record(participant->local_reader_table,
                         (DB_Record_T*)&participant->ipc_livelines_reader);
    if (dbrc != DB_RETCODE_OK)
    {
        goto done;
    }

    dbrc = DB_Table_create_record(participant->local_writer_table,
                          (DB_Record_T*)&participant->ipc_livelines_writer);
    if (dbrc != DB_RETCODE_OK)
    {
        goto done;
    }

    retval = DDS_BOOLEAN_TRUE;

done:

    if (!retval)
    {
        DDS_IpcLiveliness_unreserve(participant);
    }

    return retval;
}

DDS_Boolean
DDS_IpcLiveliness_assert(DDS_DomainParticipant *participant)
{
    DB_Cursor_T cursor = NULL;
    DB_ReturnCode_T dbrc = DB_RETCODE_ERROR;
    struct DDS_RemoteParticipantImpl *rem_participant;

    if (DDS_DomainParticipant_is_ipc_liveliness_enabled(participant))
    {
        return DDS_BOOLEAN_TRUE;
    }

    /* Unreserve the resources since this call will reserve them */
    DDS_IpcLiveliness_unreserve(participant);

    participant->ipc_liveliness = DDS_IpcLiveliness_create(
                                    participant,
                                    &participant->qos,
                                    &participant->builtin_data);

    if (participant->ipc_liveliness == NULL)
    {
        DDSC_LOG_IPC_INIT_FAILED(OSAPI_LOGKIND_ERROR);
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_Entity_is_enabled(DDS_DomainParticipant_as_entity(participant)))
    {
        return DDS_BOOLEAN_TRUE;
    }

    if (!DDS_IpcLiveliness_enable(participant->ipc_liveliness))
    {
        return DDS_BOOLEAN_FALSE;
    }

    /* If the IPC channel is created AND the participant is already enabled
     * assert liveliness routes for already discovered participants.
     */
    dbrc = DB_Table_select_all_default(participant->remote_participant_table,
                                       &cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,participant->remote_participant_table,dbrc)
        return DDS_BOOLEAN_FALSE;
    }

    do
    {
        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&rem_participant);
        if ((dbrc == DB_RETCODE_OK) &&
            (rem_participant->as_entity.state == RTIDDS_ENTITY_STATE_ENABLED))
        {
            DDS_Boolean brc;

            /* DDS_IpcLiveliness_assert_routes will report errors, but it is
             * not sufficient to fail.
             */
            brc = DDS_IpcLiveliness_assert_routes(participant->ipc_liveliness,
                                                 rem_participant);
            IGNORE_RETVAL(brc);
        }
    } while (dbrc == DB_RETCODE_OK);

    DB_Cursor_finish(participant->remote_participant_table,cursor);

   if (dbrc != DB_RETCODE_NO_DATA)
   {
       DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
   }

   return DDS_BOOLEAN_TRUE;
}

DDS_Boolean
DDS_IpcLiveliness_add_remote_writer(struct DDS_IpcLiveliness *ipc,
                                    DDS_UnsignedLong reader_oid,
                                    const DDS_BuiltinTopicKey_t *key,
                                    const struct DDS_LivelinessQosPolicy *liveliness)
{
    DB_ReturnCode_T dbrc;
    RemoteWriterMatchEntry_t *record = NULL;

    if (((liveliness->kind == DDS_AUTOMATIC_LIVELINESS_QOS) ||
         (liveliness->kind == DDS_MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)) &&
          !DDS_Duration_is_infinite(&liveliness->lease_duration))
    {
        dbrc = DB_Table_create_record(ipc->liveliness_match_table,
                                      (DB_Record_T*)&record);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_RECORD_CREATE(OSAPI_LOGKIND_ERROR,
                                   DDSC_LOG_MATCHED_DW_RECORD,dbrc)
            return DDS_BOOLEAN_FALSE;
        }

        record->local_reader_oid = reader_oid;
        record->remote_writer_key = *key;

        dbrc = DB_Table_insert_record(ipc->liveliness_match_table,
                                      (DB_Record_T)record);

        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_RECORD_INSERT(OSAPI_LOGKIND_ERROR,
                         DDSC_LOG_MATCHED_DW_RECORD,dbrc)
            return DDS_BOOLEAN_FALSE;
        }
    }

    return DDS_BOOLEAN_TRUE;
}

DDS_Boolean
DDS_IpcLiveliness_remove_remote_writer(struct DDS_IpcLiveliness *ipc,
                                       DDS_UnsignedLong reader_oid,
                                       const DDS_BuiltinTopicKey_t *key)
{
    DB_ReturnCode_T dbrc;
    RemoteWriterMatchEntry_t *record = NULL;
    RemoteWriterMatchEntry_t record_key;

    record_key.local_reader_oid = reader_oid;
    record_key.remote_writer_key = *key;

    dbrc = DB_Table_remove_record(ipc->liveliness_match_table,
                                 (DB_Record_T*)&record,
                                 (DB_Key_T)&record_key);
    /* note that only remote writers with finite lease duration
    * and liveliness kind automatic or manual_by_participant
    * are added to this table, so it is ok if the record is not found
    */
    if (dbrc == DB_RETCODE_OK)
    {
        dbrc = DB_Table_delete_record(ipc->liveliness_match_table,
                                      (DB_Record_T)record);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_RECORD_DELETE(OSAPI_LOGKIND_ERROR,
                                   DDSC_LOG_MATCHED_DW_RECORD,
                                   dbrc)
            return DDS_BOOLEAN_FALSE;
        }
    }

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Send participant message samples.
 *
 * \details
 *
 * Automatic liveliness sample is always sent. Manual_by_participant is only
 * sent if any of the DW asserted the participant.
 *
 * \param[in]   participant   Domain participant
 *
 * \return DDS_RETCODE_OK success, one of the standard return codes on failure
 */
RTI_PRIVATE DDS_ReturnCode_t
DDS_IpcLiveliness_send_liveliness(struct DDS_IpcLiveliness *ipc)
{
    DDS_ReturnCode_t ret_code = DDS_RETCODE_ERROR;
    DB_Cursor_T dw_cursor = NULL;
    DB_ReturnCode_T dbrc = DB_RETCODE_ERROR;
    struct DDS_DataWriterImpl *local_writer = NULL;
    DDS_ReturnCode_t ret_send_auto = RTI_FALSE;
    DDS_ReturnCode_t ret_send_manual = RTI_FALSE;

    /* no need to send automatic liveliness if we do not have any
     * automatic dw with finite lease duration
     */
    if (ipc->automatic_finite_dw)
    {
        ipc->liveliness_sample.key.kind[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] =
            PARTICIPANT_MESSAGE_DATA_KIND_AUTOMATIC_LIVELINESS_UPDATE;

        ret_send_auto = DDS_Liveliness_ParticipantMessageDataDataWriter_write(
                        ipc->writer,&ipc->liveliness_sample,&DDS_HANDLE_NIL);

        if (ret_send_auto != DDS_RETCODE_OK)
        {
            DDSC_LOG_PARTMESSAGE_WRITE_SAMPLE_FAILED(OSAPI_LOGKIND_ERROR)
            /* do not finish. even if there was a failure try to send
             * the other sample */
        }
    }
    else
    {
        ret_send_auto = DDS_RETCODE_OK;
    }

    /* send manual liveliness only if it was asserted */
    if (ipc->manual_by_participant_asserted)
    {
        ipc->liveliness_sample.key.kind[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] =
            PARTICIPANT_MESSAGE_DATA_KIND_MANUAL_LIVELINESS_UPDATE;

        ret_send_manual =
                DDS_Liveliness_ParticipantMessageDataDataWriter_write(
                        ipc->writer, &ipc->liveliness_sample,&DDS_HANDLE_NIL);
        if (ret_send_manual != DDS_RETCODE_OK)
        {
            DDSC_LOG_PARTMESSAGE_WRITE_SAMPLE_FAILED(OSAPI_LOGKIND_ERROR)
            goto done;
        }

        ipc->manual_by_participant_asserted = RTI_FALSE;

        /* refresh liveliness for all manual_by_participant dw */
        dbrc = DB_Table_select_all(ipc->participant->local_writer_table,
                                   DB_TABLE_DEFAULT_INDEX,&dw_cursor);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,
                                  ipc->participant->local_writer_table,dbrc)
            goto done;
        }

        do
        {
            dbrc = DB_Cursor_get_next(dw_cursor,(DB_Record_T*)&local_writer);
            if (dbrc == DB_RETCODE_OK)
            {
                if ((local_writer->liveliness.kind ==
                                DDS_MANUAL_BY_PARTICIPANT_LIVELINESS_QOS) &&
                    !DDS_Duration_is_infinite(
                                &local_writer->liveliness.lease_duration))
                {
                    /* not much that can be done in case of failure. continue
                     * with the rest of DW but mark send_manual as failed
                     */
                    if (!DDS_DataWriter_update_liveliness(local_writer))
                    {
                        DDSC_LOG_UPDATE_LIVELINESS_FAILED(OSAPI_LOGKIND_ERROR)
                        ret_send_manual = DDS_RETCODE_ERROR;
                    }
                }
            }
        } while (dbrc == DB_RETCODE_OK);

        DB_Cursor_finish(ipc->participant->local_writer_table,dw_cursor);

#if OSAPI_ENABLE_LOG
        if (dbrc != DB_RETCODE_NO_DATA)
        {
            DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
        }
#endif
    }
    else
    {
        ret_send_manual = DDS_RETCODE_OK;
    }

    if ((ret_send_auto == DDS_RETCODE_OK) && (ret_send_manual == DDS_RETCODE_OK))
    {
        ret_code = DDS_RETCODE_OK;
    }

done:

    return ret_code;
}

/*ci
 * \brief Timeout handler for asserting datawriter liveliness
 *
 * \details
 *
 * The participant uses a timer to send periodic liveliness packets.
 *
 * \param[in] storage Timeout data passed in when the timeout was created
 *
 * \return  Always OSAPI_TIMEOUT_OP_AUTOMATIC
 */
RTI_PRIVATE OSAPI_TimeoutOp_t
DDS_IpcLiveliness_on_datawriter_liveliness_timeout(
            struct OSAPI_TimeoutUserData *storage)
{
    RTI_BOOL retval = RTI_FALSE;
    struct DDS_IpcLiveliness *ipc = (struct DDS_IpcLiveliness*)storage->field[0];

    if (DB_RETCODE_OK != DB_Database_lock(ipc->participant->database))
    {
        return OSAPI_TIMEOUT_OP_AUTOMATIC;
    }

    if (DDS_RETCODE_OK != DDS_IpcLiveliness_send_liveliness(ipc))
    {
        goto done;
    }

    retval = RTI_TRUE;

done:

    if (DB_RETCODE_OK != DB_Database_unlock(ipc->participant->database))
    {
        retval = RTI_FALSE;
    }


    if (!retval)
    {
        DDSC_LOG_SEND_LIVELINESS_FAILED(OSAPI_LOGKIND_ERROR)
    }

    return OSAPI_TIMEOUT_OP_AUTOMATIC;
}

/*ci
 * \brief Gets the minimum lease duration from all data writers with finite
 *        lease duration and liveliness kind automatic or manual_by_participant.
 *        The minimum lease duration is divided by 3 because 3 is the default
 *        value for assertions_per_lease_duration.
 *
 * \details
 *
 * If there are no writers with finite lease duration and kind automatic or
 * manual_by_participant returned lease duration is infinite.
 *
 * \param[in]   participant          Domain participant
 * \param[out]  min_lease_duration   Minimum lease duration
 */
RTI_PRIVATE RTI_BOOL
DDS_IpcLiveliness_get_min_timer_duration(
    struct DDS_IpcLiveliness *ipc,
    struct DDS_Duration_t *min_lease_duration)
{
    RTI_BOOL retval = RTI_FALSE;

    DB_Cursor_T dw_cursor = NULL;
    DB_ReturnCode_T dbrc = DB_RETCODE_ERROR;
    struct DDS_DataWriterImpl *local_writer = NULL;

    /* this index already has dw sorted by lease duration and dw with lease
     * duration infinite or liveliness kind manual_by_topic are not added.
     * but the first dw created is always added. this is why we need to
     * check for the right liveliness kind
     */
    dbrc = DB_Table_select_all(ipc->participant->local_writer_table,
                               ipc->min_lease_idx, &dw_cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,
                              ipc->participant->local_writer_table,dbrc)
        goto done;
    }

    /* if no matching DW with finite lease duration, infinite is returned. */
    *min_lease_duration = DDS_DURATION_INFINITE;

    dbrc = DB_Cursor_get_next(dw_cursor,(DB_Record_T*)&local_writer);
    while (DDS_Duration_is_infinite(min_lease_duration) &&
            dbrc == DB_RETCODE_OK)
    {
        /* do not consider:
            * - writers with liveliness kind diferent to automatic or
            *   manual_by_participant.
            * The index do not add these kind of dw but the first
            * dw created is always added, so we need to filter.
            */
        if ((local_writer->liveliness.kind ==
                                 DDS_AUTOMATIC_LIVELINESS_QOS ||
             local_writer->liveliness.kind ==
                                 DDS_MANUAL_BY_PARTICIPANT_LIVELINESS_QOS) &&
            !DDS_Duration_is_infinite(
                    &local_writer->liveliness.lease_duration))
        {
            *min_lease_duration =
                    local_writer->liveliness.lease_duration;
        }
        else
        {
            dbrc = DB_Cursor_get_next(dw_cursor,(DB_Record_T*)&local_writer);
        }
    }
    DB_Cursor_finish(ipc->participant->local_writer_table,dw_cursor);

    /* 3 is the default value for assertions_per_lease_duration. As
     * we do not have this QoS in micro we will use the default value.
     */
    DDS_Duration_div(min_lease_duration, 3);


    retval = RTI_TRUE;

done:

    return retval;
}

/*ci
 * \brief Returns RTI_TRUE only if there is a dw with finite lease duration
 * and liveliness kind DDS_AUTOMATIC_LIVELINESS_QOS.
 *
 * \param[in]   participant          Domain participant
 *
 * \return  RTI_TRUE only if there is a dw with finite lease duration
 * and livelines kind DDS_AUTOMATIC_LIVELINESS_QOS.
 */
RTI_PRIVATE RTI_BOOL
DDS_IpcLiveliness_has_automatic_finite_liveliness(
                                struct DDS_IpcLiveliness *ipc)
{
    RTI_BOOL retval = RTI_FALSE;
    DB_Cursor_T dw_cursor = NULL;
    DB_ReturnCode_T dbrc = DB_RETCODE_ERROR;
    struct DDS_DataWriterImpl *local_writer = NULL;

    if (ipc == NULL)
    {
        return RTI_FALSE;
    }

    dbrc = DB_Table_select_all(ipc->participant->local_writer_table,
                               ipc->automatic_idx,&dw_cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,
                              ipc->participant->local_writer_table,dbrc)
        goto done;
    }

    dbrc = DB_Cursor_get_next(dw_cursor,(DB_Record_T*)&local_writer);
    while (!retval && dbrc == DB_RETCODE_OK)
    {
        /* Ensure that the dw has automatic liveliness and finite lease duration.
         * This is needed because the first dw that is created is alwasy added
         * to the index
         */
        if (local_writer->liveliness.kind ==
                                DDS_AUTOMATIC_LIVELINESS_QOS &&
            !DDS_Duration_is_infinite(
                    &local_writer->liveliness.lease_duration))
        {
            retval = RTI_TRUE;
        }
        else
        {
            dbrc = DB_Cursor_get_next(dw_cursor,(DB_Record_T*)&local_writer);
        }
    }
    DB_Cursor_finish(ipc->participant->local_writer_table,dw_cursor);

done:

    return retval;
}

/*ci
 * \brief Refreshes the liveliness timeout period if the new minimum lease
 * duration is smaller than the current one.
 *
 * \details
 *
 * \param[in]   participant   Domain participant
 *
 * \return DDS_RETCODE_OK success, one of the standard return codes on failure
 */
RTI_PRIVATE DDS_Boolean
DDS_IpcLiveliness_refresh_timeout(
    struct DDS_IpcLiveliness *ipc,
    struct DDS_Duration_t *new_lease_duration)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    struct OSAPI_TimeoutUserData storage = OSAPI_TimeoutUserData_INITIALIZER;

    /* it is needed to update the current timeout? */
    if (DDS_Duration_compare(&ipc->liveliness_timer_duration,
                             new_lease_duration) > 0)
    {
        storage.field[0] = (void*)ipc;

        /* call the timeout callback function before updating it.
         * this is needed to ensure that the send period is not too long
         */
        if (OSAPI_TIMEOUT_OP_AUTOMATIC !=
                DDS_IpcLiveliness_on_datawriter_liveliness_timeout(&storage))
        {
            DDSC_LOG_UPDATE_LEASE_DURATION_TIMER(OSAPI_LOGKIND_ERROR)
            goto done;
        }


        if (!OSAPI_Timer_update_timeout(
                     ipc->participant->timer,
                     &ipc->liveliness_event,
                     new_lease_duration->sec,
                     (RTI_INT32)new_lease_duration->nanosec))
        {
            DDSC_LOG_UPDATE_LEASE_DURATION_TIMER(OSAPI_LOGKIND_ERROR)
            goto done;
        }

        ipc->liveliness_timer_duration = *new_lease_duration;
        ipc->manual_by_participant_asserted = RTI_FALSE;
    }

    retval = DDS_BOOLEAN_TRUE;

done:

    return retval;
}

/*ci
 * \brief After a DW is enabled it is needed to check whether any of the dw
 *        created and enabled has finite lease duration and liveliness kind
 *        automatic or manual_by_participant. If case that is true, this
 *        function ensures that the timeout responsible for sending periodic
 *        participant messages is created and has the right period.
 *
 * \details
 *
 * \param[in]   participant   Domain participant
 *
 * \return DDS_RETCODE_OK success, one of the standard return codes on failure
 */
DDS_ReturnCode_t
DDS_IpcLiveliness_update_datawriter_liveliness_timeout(
                                        struct DDS_IpcLiveliness *ipc)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    struct DDS_Duration_t new_lease_duration = DDS_DURATION_INFINITE;
    struct OSAPI_TimeoutUserData storage = OSAPI_TimeoutUserData_INITIALIZER;

    if (ipc == NULL)
    {
        return DDS_RETCODE_OK;
    }

    /* any writer with automatic finite liveliness? */
    ipc->automatic_finite_dw =
        DDS_IpcLiveliness_has_automatic_finite_liveliness(ipc);

    /* get the new minimum lease duration */
    if (!DDS_IpcLiveliness_get_min_timer_duration(
                                        ipc,&new_lease_duration))
    {
        goto done;
    }

    /* If current min_lease_duration is infinite timeout is not
     * created at the moment.
     */
    if (DDS_Duration_is_infinite(&ipc->liveliness_timer_duration))
    {
        /* if new lease duration is finite we need to create the timeout
         */
        if (!DDS_Duration_is_infinite(&new_lease_duration))
        {

            storage.field[0] = (void*)ipc;

            if (!OSAPI_Timer_create_timeout(
                     ipc->participant->timer,
                     &ipc->liveliness_event,
                     new_lease_duration.sec,
                     (RTI_INT32)new_lease_duration.nanosec,
                     OSAPI_TIMER_PERIODIC,
                     DDS_IpcLiveliness_on_datawriter_liveliness_timeout,
                     &storage))
            {
                DDSC_LOG_TIMER_CREATE_TIMEOUT(OSAPI_LOGKIND_ERROR)
                goto done;
            }

            ipc->liveliness_timer_duration = new_lease_duration;
            ipc->manual_by_participant_asserted = RTI_FALSE;
        }
    }
    else
    {
        if (!DDS_IpcLiveliness_refresh_timeout(
                    ipc, &new_lease_duration))
        {
            goto done;
        }
    }

    retcode = DDS_RETCODE_OK;

done:
    return retcode;
}

#ifndef RTI_CERT
/*ci
 * \brief After a DW is deleted it is needed to check whether any of the dw
 *        created and enabled has finite lease duration and liveliness kind
 *        automatic or manual_by_participant. If case that is false, this
 *        function ensures that the timeout responsible for sending periodic
 *        participant messages is not running.
 *
 * \details
 *
 * \param[in]   participant   Domain participant
 *
 * \return DDS_RETCODE_OK success, one of the standard return codes on failure
 */
DDS_Boolean
DDS_IpcLiveliness_delete_liveliness_timeout(struct DDS_IpcLiveliness *ipc)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    struct DDS_Duration_t min_lease_duration = DDS_DURATION_INFINITE;

    /* any writer with automatic finite liveliness?
     */
    ipc->automatic_finite_dw =
            DDS_IpcLiveliness_has_automatic_finite_liveliness(
                ipc);

    /* should be the timeout be deleted? */
    if (!DDS_Duration_is_infinite(&ipc->liveliness_timer_duration))
    {
        /* get the new minimum lease duration */
        if (!DDS_IpcLiveliness_get_min_timer_duration(
            ipc, &min_lease_duration))
        {
            goto done;
        }

        /* delete the timeout if we do not need it anymore */
        if (DDS_Duration_is_infinite(&min_lease_duration))
        {
            ipc->liveliness_timer_duration = DDS_DURATION_INFINITE;
            ipc->manual_by_participant_asserted = RTI_FALSE;
            ipc->automatic_finite_dw = RTI_FALSE;

            if (!OSAPI_Timer_delete_timeout(
                ipc->participant->timer,
                &ipc->liveliness_event))
            {
                DDSC_LOG_OBJECT_DELETE(OSAPI_LOGKIND_ERROR,
                                       DDSC_LOG_TIMEROUT_OBJECT)
                goto done;
            }
        }
        else
        {
            return DDS_IpcLiveliness_refresh_timeout(
                ipc, &min_lease_duration);
        }
    }

    retval = RTI_TRUE;

done:
    return retval;
}
#endif /* RTI_CERT */

void
DDS_IpcLiveliness_on_after_datawriter_deleted(struct DDS_IpcLiveliness *ipc,
                                              DDS_DataWriter *const writer)
{
    struct DDS_DomainParticipantImpl *participant = NULL;

    participant = DDS_Publisher_get_participant(
                                        DDS_DataWriter_get_publisher(writer));

    if (DB_Database_lock(participant->database) != DB_RETCODE_OK)
    {
        return;
    }

    if (!DDS_IpcLiveliness_delete_liveliness_timeout(ipc))
    {
        goto done;
    }

done:
    if (DB_Database_unlock(participant->database) != DB_RETCODE_OK)
    {
        return;
    }
}

void
DDS_IpcLiveliness_assert_liveliness(struct DDS_IpcLiveliness *ipc)
{
    ipc->manual_by_participant_asserted = RTI_TRUE;
}
