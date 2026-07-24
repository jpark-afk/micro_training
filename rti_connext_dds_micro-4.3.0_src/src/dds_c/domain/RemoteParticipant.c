/*
 * FILE: RemoteParticipant.c - Remote participant implementation
 *
 * Copyright (c) 2008-2026 Real-Time Innovations,Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 * 05may2015,tk MICRO-1190/PR#14713 Properly unlock database on sequence error
 * 05may2015,tk MICRO-1189/PR#14712 Removed unused code for Cert
 * 12mar2015,tk MICRO-1108 Return BAD_PARAMETER instead of PRECONDITION_NOT_MET
 * 20feb2014,eh MICRO-1076 Preallocate remote participant data's sequences
 * 27jan2015,tk MICRO-1023/PR#13438 Set dbrc to DB_RETCODE_OK if participant
 *                                  if found by name
 * 27jan2015,tk MICRO-1014/PR#13352 Check if an unknown suffix (0) is received
 * 10dec2014,tk MICRO-978/PR#12914 Consistently return records on failure
 * 09dec2014,tk MICRO-986/PR#12972 Return error-code if DB unlock fails
 * 09dec2014,tk MICRO-989/PR#12978 Correctly return error for select errors
 * 03dec2014,tk MICRO-983/PR#12963 Check value returned by
 *                                 OSAPI_System_get_next_object_id
 * 03dec2014,tk MICRO-985/PR#12970 Initialize timeout event
 * 03dec2014,tk MICRO-987/PR#12973 Log key of remote participant, not name
 * 03dec2014,tk MICRO-988/PR#12974 Return error if timeout update fails
 * 02dec2014,tk MICRO-981/PR#12958 Made is_new optional in assert_remote_participant
 * 31jul2014,tk MICRO-842/PR#9683 - Removed superfluous parameters in DB
 *                                  compare function
 * 15may2014,as MICRO-317 (Verocel PR#1443) Replaced String_cmp with String_ncmp
 * 17may2012,tk Updated
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief Remote participant implementation
 *
 * \details
 * This file implements functions used to manage remote participants, such
 * as discovery and liveliness updates. The functions are typically called
 * by discovery plugins.
 */
/*ci
 * \addtogroup DDSDomainModule
 * @{
 */
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_discovery_plugin_h
#include "dds_c/dds_c_discovery_plugin.h"
#endif

#include "Entity.h"
#include "RemoteEntity.h"
#include "RemoteEndpoint.h"
#include "QosPolicy.h"
#include "BuiltinTopicKey.h"
#include "TopicDescription.h"
#include "TopicQos.h"
#include "Topic.h"
#include "Type.h"
#include "DomainParticipantQos.h"
#include "DomainParticipantEvent.h"
#include "DomainParticipant.h"
#include "RemoteParticipant.h"
#include "RemotePublication.h"
#include "RemoteSubscription.h"
#include "BuiltinTopicData.h"
#include "DomainParticipantTrust.h"

const char *const DDS_PARTICIPANT_BUILTIN_TOPIC_TYPE_NAME = "DDS_ParticipantBuiltinTopicData";
const char *const DDS_PARTICIPANT_BUILTIN_TOPIC_NAME = "DCPSParticipant";

/*******************************************************************************
 *
 *                                 Internal API
 *
 ******************************************************************************/
MUST_CHECK_RETURN RTI_PRIVATE DDS_ReturnCode_t
NDDS_RemoteParticipant_enable(DDS_DomainParticipant *const participant,
                              NDDS_RemoteEntity *entity,
                              const DDS_BuiltinTopicKey_t *new_key);

MUST_CHECK_RETURN RTI_PRIVATE DDS_ReturnCode_t
NDDS_RemoteParticipant_remove_internal(DDS_DomainParticipant *const self,
                                       const DDS_BuiltinTopicKey_t *const key,
                                       DDS_Boolean reset_entry);

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Compare entries in the table of remote participants. The function is
 *        compatible with \ref DB_IndexCompare_T
 *
 * \param[in] flags Used to distinguish between op2 being a key or record
 * \param[in] op1   A DDS_RemoteParticipantImpl already in the database
 * \param[in] op2   Either a DDS_RemoteParticipantImpl being added or a key
 *                  being searched for
 *
 * \return positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2,
 *         zero if op1 is equal to op2
 */
MUST_CHECK_RETURN RTI_INT32
DDS_RemoteParticipantImpl_compare(RTI_INT32 flags,const DB_Record_T op1, void *op2)
{
    struct DDS_RemoteParticipantImpl *record_left =
                                        (struct DDS_RemoteParticipantImpl*)op1;
    const DDS_BuiltinTopicKey_t *key_right;

    if (DB_SELECT_OP2_IS_KEY(flags))
    {
        key_right = (const DDS_BuiltinTopicKey_t*)op2;
    }
    else
    {
        key_right = &((struct DDS_RemoteParticipantImpl*)op2)->data.key;
    }

    return DDS_BuiltinTopicKey_compare(&record_left->data.key,key_right);
}

/*ci
 * \brief Lookup a remote participant by name
 *
 * \details
 *
 * Find a remote participant by name in the local participant database.
 *
 * \param[in] participant The local participant to search in
 * \param[in] name        The remote participant name to search in
 *
 * \return Pointer to remote participant if found, NULL otherwise
 */
struct DDS_RemoteParticipantImpl*
NDDS_DomainParticipant_lookup_name(DDS_DomainParticipant *const participant,
                                   const char *name)
{
    struct DDS_DomainParticipantImpl *self =
                        (struct DDS_DomainParticipantImpl *)participant;
    DB_ReturnCode_T dbrc;
    DB_Cursor_T cursor = NULL;
    struct DDS_RemoteParticipantImpl *rem_participant = NULL;

    dbrc = DB_Table_select_all(self->remote_participant_table,
                                    DB_TABLE_DEFAULT_INDEX,&cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        return NULL;
    }

    do
    {
        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&rem_participant);
        if (dbrc == DB_RETCODE_OK)
        {
            if (!DDS_String_ncmp(name,
                    rem_participant->data.participant_name.name,
                    DDS_ENTITYNAME_QOS_NAME_MAX - 1))
            {
                break;
            }
            rem_participant = NULL;
        }
    } while (dbrc == DB_RETCODE_OK);

    DB_Cursor_finish(self->remote_participant_table,cursor);

    /* The result is not affected by this error */
#if OSAPI_ENABLE_LOG
    if ((dbrc != DB_RETCODE_NO_DATA) && (dbrc != DB_RETCODE_OK))
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
    }
#endif

    return rem_participant;
}

/*ci
 * \brief Liveliness timer for a remote participant
 *
 * \details
 *
 * Each discovered remote participant has a lease-duration, possible different
 * for each discovered participant. A timer is started when a remote participant
 * is first discovered and when a participant liveliness is detected the timer
 * is restarted. Thus, if the timer expired the participant is assumed to be
 * gone and the discovery plugin, if any, is informed.
 *
 * \param[in] storage Timer storageto retrieve specific parameters from

 * \return Always OSAPI_TIMEOUT_OP_MANUAL
 */
RTI_PRIVATE OSAPI_TimeoutOp_t
NDDS_RemoteParticipant_on_liveliness(struct OSAPI_TimeoutUserData *storage)
{
    struct DDS_RemoteParticipantImpl *remote_participant;
    DDS_DomainParticipant *participant;
    DDS_Boolean reset_entry;
    DDS_ReturnCode_t ddsrc;

    remote_participant = (struct DDS_RemoteParticipantImpl *)storage->field[0];
    participant = (DDS_DomainParticipant *)storage->field[1];

    reset_entry = (remote_participant->status &
                   DDS_REMOTE_PARTICIPANT_STATUS_STATIC) ?
                  DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;

    ddsrc = NDDS_RemoteParticipant_remove_internal(participant,
                                               &remote_participant->data.key,
                                               reset_entry);
    if (ddsrc != DDS_RETCODE_OK)
    {
        DDSC_LOG_DISC_REMOTE_PARTICIPANT_REMOVE(OSAPI_LOGKIND_ERROR,ddsrc)
    }

    return OSAPI_TIMEOUT_OP_MANUAL;
}

/*******************************************************************************
 *
 *                                 Public API
 *
 ******************************************************************************/
RTI_BOOL
NDDS_DomainParticipant_delete_remote_participant_routes(
        struct DDS_DomainParticipantImpl *participant,
        struct DDS_RemoteParticipantImpl *record)
{
#if !DDS_LIVELINESS_CHANNEL_ENABLED
    UNUSED_ARG(participant);
    UNUSED_ARG(record);
    return RTI_TRUE;
#else
    RTI_BOOL result = RTI_FALSE;

#if DDS_LIVELINESS_CHANNEL_ENABLED
    if (record->ch_partmsgdata_asserted)
    {
        if (DDS_DomainParticipant_is_ipc_liveliness_enabled(participant) &&
            !DDS_IpcLiveliness_remove_routes(participant->ipc_liveliness,
                                             record))
        {
            goto done;
        }
    }

#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */

    result = RTI_TRUE;
done:
    return result;
#endif
}

DDS_ReturnCode_t
NDDS_DomainParticipant_create_remote_participant(
        DDS_DomainParticipant *const self,
        struct DDS_ParticipantBuiltinTopicData *const data,
        RTI_BOOL assign_guid,
        RTI_UINT32 assigned_oid,
        struct DDS_RemoteParticipantImpl **record_out)
{
    struct DDS_DomainParticipantImpl *participant =
            (struct DDS_DomainParticipantImpl*)self;
    struct DDS_RemoteParticipantImpl *record = NULL;
    DB_ReturnCode_T dbrc = DB_RETCODE_ERROR;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
#ifndef RTI_CERT
    RTI_BOOL record_data_initialized = RTI_FALSE;
#endif
    struct OSAPI_TimeoutHandle init_handle = OSAPI_TimeoutHandle_INITIALIZER;

    dbrc = DB_Table_create_record(participant->remote_participant_table,
                                     (DB_Record_T*)&record);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_CREATE(OSAPI_LOGKIND_ERROR,
                        DDSC_LOG_PARTICIPANT_RECORD,dbrc)
        retcode = DDS_RETCODE_OUT_OF_RESOURCES;
        goto done;
    }

    record->status = DDS_REMOTE_PARTICIPANT_STATUS_DEFAULT;

    /* flag NEW is reset at the end of this method */
    DDS_RemoteParticipant_enable_status(
                record, DDS_REMOTE_PARTICIPANT_STATUS_NEW);

#ifndef RTI_CERT
    record_data_initialized = RTI_TRUE;
#endif

    if (!DDS_ParticipantBuiltinTopicData_set_from(&record->data, data,
                                                    DDS_BOOLEAN_TRUE, participant))
    {
        DDSC_LOG_OBJECT_COPY(OSAPI_LOGKIND_ERROR,
                             DDSC_LOG_PARTICIPANTDATA_OBJECT)
        retcode = DDS_RETCODE_OUT_OF_RESOURCES;
        goto done;
    }

    if (assign_guid)
    {
        /* Assign an internal GUID so that remote endpoints can
         * be asserted without having to store the name of the parent
         * participant (that is, when the real GUID is known we can
         * look up the internal GUID instead of having to look up the name
         */
        record->data.key.value[DDS_BUILTIN_TOPIC_KEY_TYPE_NATIVE_LENGTH-1]
                                       = assigned_oid;
        DDS_RemoteParticipant_enable_status(
                record, DDS_REMOTE_PARTICIPANT_STATUS_STATIC);
    }
    record->as_entity.enable_func = NDDS_RemoteParticipant_enable;
    record->as_entity.kind = DDS_PARTICIPANT_ENTITY_KIND;
    record->as_entity.state = RTIDDS_ENTITY_STATE_CREATED;
    record->timer = DDS_DomainParticipant_get_timer(participant);
    record->orig_key = record->data.key;
    record->lease_duration_event = init_handle;

#if DDS_LIVELINESS_CHANNEL_ENABLED
    record->ch_partmsgdata_asserted = RTI_FALSE;
#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */

    /* Insert into the table */
    dbrc = DB_Table_insert_record(participant->remote_participant_table,
                                 (DB_Record_T)record);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_INSERT(OSAPI_LOGKIND_ERROR,
                        DDSC_LOG_PARTICIPANT_RECORD,dbrc)
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:

    if (retcode != DDS_RETCODE_OK && record != NULL)
    {
#ifndef RTI_CERT
        if (record_data_initialized)
        {
            DDS_ParticipantBuiltinTopicData_finalize_no_dealloc(&record->data, participant);
        }
#endif

        dbrc = DB_Table_delete_record(participant->remote_participant_table,
                                   (DB_Record_T)record);
#if OSAPI_ENABLE_LOG
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_RECORD_DELETE(OSAPI_LOGKIND_ERROR,
                                  DDSC_LOG_PARTICIPANT_RECORD,dbrc)
        }
#else
        IGNORE_RETVAL(dbrc);
#endif /* OSAPI_ENABLE_LOG */

        *record_out = NULL;
    }
    else if (retcode == DDS_RETCODE_OK)
    {
        *record_out = record;
    }

    return retcode;
}


DDS_ReturnCode_t
NDDS_DomainParticipant_assert_remote_participant(
                            DDS_DomainParticipant *const self,
                            struct DDS_ParticipantBuiltinTopicData *const data,
                            DDS_RemoteParticipantStatusMask *status)
{
    struct DDS_DomainParticipantImpl *participant =
                                    (struct DDS_DomainParticipantImpl *)self;
    struct DDS_RemoteParticipantImpl *record = NULL;
    struct DDS_RemoteParticipantImpl *record_test = NULL;
    DB_ReturnCode_T dbrc = DB_RETCODE_NO_DATA;
    DDS_Boolean assign_guid = DDS_BOOLEAN_FALSE;
    RTI_UINT32 assigned_oid = 0;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR,
                     fn_retcode = DDS_RETCODE_ERROR;
    DDS_RemoteParticipantStatusMask status_out =
            DDS_REMOTE_PARTICIPANT_STATUS_DEFAULT;

#if OSAPI_ENABLE_LOG
    DDS_Boolean bretval;
#endif

    OSAPI_PRECONDITION(self == NULL || data == NULL || status == NULL,
            return DDS_RETCODE_BAD_PARAMETER,
            OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("data",data,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("status",status,RTI_TRUE);)

    *status = status_out;

    /* The algorithm here is somewhat complicated.
     *
     * If the GUID is unknown (i.e not set in data, e.g, DPSE), then the
     * remote_participant must have a name. However, if the GUID is known
     * (either static or dynamic discovery) then the participant does not
     * need to have a name.
     *
     * After a participant has been enabled, the GUID will be updated to the
     * real GUID. However, it could be that the QoS is changed even for static
     * discovery and we need to look up an existing participant. In that case
     * we will still use the name because the user does not know what the
     * real GUID is. Thus, the algorithm to lookup a participant is this:
     *
     * If the GUID passed in is AUTO/UNKNOWN we use the name. If the name
     * is empty an error is returned.
     *
     * If the GUID passed in is set, the name is ignored in the lookup
     */

    /* If the GUID is unknown, lookup the participant based on the name */
    if (DB_Database_lock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_BuiltinTopicKey_equals(&data->key, &DDS_BUILTINTOPICKEY_UNKNOWN))
    {
        if (data->participant_name.name[0] == 0)
        {
            DDSC_LOG_INVALID_PARTICIPANT_NAME(OSAPI_LOGKIND_ERROR)
            fn_retcode = DDS_RETCODE_BAD_PARAMETER;
            goto done;
        }
        record = NDDS_DomainParticipant_lookup_name(
                        self, data->participant_name.name);
        if (record == NULL)
        {
            assign_guid = DDS_BOOLEAN_TRUE;
            assigned_oid = DDS_DomainParticipant_get_next_objectid(self);
            if (assigned_oid == 0)
            {
                DDSC_LOG_GET_NEXT_OBJECT_ID(OSAPI_LOGKIND_ERROR)
                goto done;
            }
        }
        else
        {
            dbrc = DB_RETCODE_OK;
        }
    }
    else if (participant->qos.discovery.enable_participant_discovery_by_name)
    {
        record = NDDS_DomainParticipant_lookup_name(participant,
                                         data->participant_name.name);
        if (record != NULL)
        {
            DDS_ReturnCode_t ddsrc = DDS_RETCODE_OK;

            if (!DDS_BuiltinTopicKey_equals(&record->data.key,&data->key))
            {
                /* The participant already exists by name, but with a different
                 * GUID. Reset (static discovery/Remove it (dynamic discovery)
                 */
                if (DDS_RemoteParticipant_check_status(record,
                                        DDS_REMOTE_PARTICIPANT_STATUS_STATIC))
                {
                    ddsrc = NDDS_DomainParticipant_reset_remote_participant(
                                                participant,&record->data.key);
                    dbrc = DB_RETCODE_OK;
                    /* record != NULL and dbrc == DB_RETCODE_OK -> no change
                     */
                }
#ifndef RTI_CERT
                else
                {
                    ddsrc = NDDS_DomainParticipant_remove_remote_participant(
                                                participant,&record->data.key);
                    /* The remote participant has been removed, proceed
                     * as if the participant didn't exist.
                     */
                    dbrc = DB_RETCODE_NO_DATA;
                    record = NULL;
                }
#endif
                if (ddsrc != DDS_RETCODE_OK)
                {
                    /* Do not make any changes */
                    record = NULL;
                    fn_retcode = DDS_RETCODE_OK;
                    DDSC_LOG_REMOTE_PARTICIPANT_RESTART(OSAPI_LOGKIND_ERROR,
                                                data->participant_name.name)
                    goto done;
                }
            }
            else
            {
                /* The record existed, but there is no change in state */
                dbrc = DB_RETCODE_OK;
            }
           /* If the remote participant does not have a name proceed. If there
            * resources it will considered a new participant, otherwise it will
            * be dropped
            */
        }
    }
    else
    {
        /* If the GUID is known, lookup the participant based on the GUID */
        record = NULL;
        dbrc = DB_Table_select_match(participant->remote_participant_table,
                    DB_TABLE_DEFAULT_INDEX,(DB_Record_T*)&record,
                    (DB_Key_T)&data->key);

        if ((dbrc == DB_RETCODE_OK) &&
            (data->participant_name.name[0] != 0) &&
            DDS_RemoteParticipant_check_status(
                    record, DDS_REMOTE_PARTICIPANT_STATUS_STATIC))
        {
            record_test = NDDS_DomainParticipant_lookup_name(participant,
                                                data->participant_name.name);
            if (record_test == NULL)
            {
                DDSC_LOG_PARTICIPANT_DOES_NOT_EXIST(OSAPI_LOGKIND_ERROR,
                                                  data->participant_name.name)
                goto done;
            }

            if (!DDS_BuiltinTopicKey_equals(&record->data.key,
                                            &record_test->data.key))
            {
                DDSC_LOG_REMOTE_PARTICIPANT_KEY_NOT_EQUAL(OSAPI_LOGKIND_ERROR,
                                                  data->participant_name.name)
                fn_retcode = DDS_RETCODE_PRECONDITION_NOT_MET;
                goto done;
            }
        }
    }

    /* If the record already exists update it, otherwise create a new record */
    if ((record == NULL) && (dbrc == DB_RETCODE_NO_DATA))
    {
        retcode = NDDS_DomainParticipant_create_remote_participant(
                        participant,
                        data,
                        assign_guid,
                        assigned_oid,
                        &record);
        if (retcode != DDS_RETCODE_OK)
        {
            fn_retcode = retcode;
            goto done;
        }
    }
    else if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_ERROR(OSAPI_LOGKIND_ERROR,
                        DDSC_LOG_PARTICIPANT_RECORD,dbrc)
        goto done;
    }
    else
    {
        /* NOTE: If a participant already exists, any QoS changes are ignored.
         */
        /* This cannot stop communication since the participant already
         * has been discovered, instead continue
         */
#if OSAPI_ENABLE_LOG
        bretval = DDS_ParticipantBuiltinTopicData_is_equal(&record->data,data);
        if (!bretval)
        {
            DDSC_LOG_QOS_CHANGED(OSAPI_LOGKIND_WARNING,
                                DDSC_LOG_PARTICIPANT_QOS)
        }
#endif
    }

    fn_retcode = DDS_RETCODE_OK;

done:
    if (fn_retcode != DDS_RETCODE_OK && record != NULL)
    {
        if (!NDDS_DomainParticipant_delete_remote_participant_routes(
                        participant,record))
        {
            DDSC_LOG_OBJECT_FINALIZE(OSAPI_LOGKIND_ERROR,
                                     DDSC_LOG_PARTICIPANTDATA_OBJECT)
        }
#ifndef RTI_CERT
        if (!NDDS_RemoteParticipantRecord_finalize(record,participant))
        {
            DDSC_LOG_OBJECT_FINALIZE(OSAPI_LOGKIND_ERROR,
                                     DDSC_LOG_PARTICIPANTDATA_OBJECT)
        }
#endif
        dbrc = DB_Table_delete_record(participant->remote_participant_table,
                        (DB_Record_T)record);
#if OSAPI_ENABLE_LOG
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_RECORD_DELETE(OSAPI_LOGKIND_ERROR,
                                  DDSC_LOG_PARTICIPANT_RECORD,dbrc)
        }
#endif /* OSAPI_ENABLE_LOG */
        record = NULL;
    }

    if (fn_retcode == DDS_RETCODE_OK &&
        record != NULL &&
        !DDS_RemoteParticipant_check_status(
                record, DDS_REMOTE_PARTICIPANT_STATUS_ENABLED))
    {
        if (!DDS_DomainParticipant_after_remote_participant_ready(
                    participant, record))
        {
            return DDS_RETCODE_ERROR;
        }
    }

    if (record != NULL)
    {
        *status = record->status;
    }


    dbrc = DB_Database_unlock(participant->database);
    if (dbrc != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return fn_retcode;
}

DDS_ReturnCode_t
NDDS_DomainParticipant_refresh_remote_participant_liveliness(
                                DDS_DomainParticipant *const self,
                                const struct DDS_BuiltinTopicKey_t *const key)
{
    struct DDS_DomainParticipantImpl *participant =
                                    (struct DDS_DomainParticipantImpl *)self;
    struct DDS_RemoteParticipantImpl *remote_participant = NULL;
    DB_ReturnCode_T dbrc;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION(self == NULL || key == NULL,
            return DDS_RETCODE_PRECONDITION_NOT_MET,
            OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("key",key,RTI_TRUE);)

    if (DB_Database_lock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    remote_participant = NULL;
    dbrc = DB_Table_select_match(participant->remote_participant_table,
                                DB_TABLE_DEFAULT_INDEX,
                                (DB_Record_T*)&remote_participant,
                                (DB_Key_T) key);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_REFRESH_REM_PARTICIPANT(OSAPI_LOGKIND_ERROR,
                                         dbrc,
                                         key->value[0],key->value[1],
                                         key->value[2],key->value[3])
        goto done;
    }

    /* Nothing to do if lease duration is infinite */
    if (DDS_Duration_is_infinite(
                        &remote_participant->data.liveliness_lease_duration))
    {
        retcode = DDS_RETCODE_OK;
        goto done;
    }

    if (!OSAPI_Timer_update_timeout(remote_participant->timer,
            &remote_participant->lease_duration_event,
            remote_participant->data.liveliness_lease_duration.sec,
            (RTI_INT32)remote_participant->data.liveliness_lease_duration.nanosec))
    {
        DDSC_LOG_REFRESH_REM_PARTICIPANT_TIMEOUT(OSAPI_LOGKIND_ERROR,
                remote_participant->data.participant_name.name,
                    remote_participant->data.liveliness_lease_duration.sec,
                    remote_participant->data.liveliness_lease_duration.nanosec)

        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:

    if (DB_Database_unlock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}

/*ci
 * \brief Enable a remote participant
 *
 * \details
 * This function enables a remote participant and all its endpoints and
 * optionally assigns it a new key. When the remote endpoints are enabled
 * they are also matched with local endpoints.
 *
 * \param[in] self    The participant to enable the remote participant in
 * \param[in] entity  The remote participant to enable
 * \param[in] new_key If not NULL, update the key of the remote participant
 *                    with this key.
 *
 * \return DDS_RETCODE_OK on success,one of the standard error codes on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE DDS_ReturnCode_t
NDDS_RemoteParticipant_enable(DDS_DomainParticipant *const self,
                              NDDS_RemoteEntity *entity,
                              const DDS_BuiltinTopicKey_t *new_key)
{
    DB_ReturnCode_T dbrc;
    struct DDS_DomainParticipantImpl *participant =
                                    (struct DDS_DomainParticipantImpl *)self;
    struct DDS_RemoteParticipantImpl *remote_participant = NULL;
    struct OSAPI_TimeoutUserData storage;
    DB_Cursor_T cursor = NULL;
    struct DDS_RemotePublicationImpl *remote_pub;
    struct DDS_RemoteSubscriptionImpl *remote_sub;
    struct DDS_RemoteParticipantImpl *remote_participant2 = NULL;
    DDS_BuiltinTopicKey_t low_key = DDS_BuiltinTopicKey_t_INITIALIZER;
    DDS_BuiltinTopicKey_t high_key = DDS_BuiltinTopicKey_t_INITIALIZER;
    DDS_ReturnCode_t ddsrc;

    OSAPI_PRECONDITION((self == NULL) || (entity == NULL),
                       return DDS_RETCODE_BAD_PARAMETER,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("entity",entity,RTI_TRUE);)

    if (DB_Database_lock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (entity->state == RTIDDS_ENTITY_STATE_ENABLED)
    {
        ddsrc = DDS_RETCODE_OK;
        goto done;
    }

    remote_participant = (struct DDS_RemoteParticipantImpl*)entity;

    OSAPI_TRACE_DDS("enable remote participant",RTI_FALSE)
    OSAPI_TRACE_STRING("name",remote_participant->data.participant_name.name,RTI_FALSE)
    OSAPI_TRACE_GUID("GUID",&remote_participant->data.key,RTI_TRUE)

    /* We allow changing the GUID, e.g for static discovery */
    if ((new_key != NULL) &&
        DDS_BuiltinTopicKey_prefix_equals(&remote_participant->data.key,
                                          &DDS_BUILTINTOPICKEY_PREFIX_UNKNOWN))
    {
        dbrc = DB_Table_remove_record(participant->remote_participant_table,
                                      (DB_Record_T*)&remote_participant2,
                                      (DB_Key_T)&remote_participant->data.key);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_RECORD_REMOVE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_RECORD,dbrc)
            ddsrc = DDS_RETCODE_PRECONDITION_NOT_MET;
            goto done;
        }

        remote_participant->data.key = *new_key;
        dbrc = DB_Table_insert_record(participant->remote_participant_table,
                                     (DB_Record_T)remote_participant);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_RECORD_INSERT(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_RECORD,dbrc)
            ddsrc = DDS_RETCODE_PRECONDITION_NOT_MET;
            goto done;
        }
    }

    if (!DDS_DomainParticipant_register_matched_remote_participant(participant->config.trust,
                                                                   &remote_participant->data.key))
    {
        ddsrc = DDS_RETCODE_ERROR;
        goto done;
    }

    remote_participant->as_entity.state = RTIDDS_ENTITY_STATE_ENABLED;
    low_key = remote_participant->orig_key;
    low_key.value[3]=0;
    high_key = remote_participant->orig_key;
    high_key.value[3]=0xffffffff;

    /* Publisher */
    if (new_key)
    {
        dbrc = DB_Table_select_range(participant->remote_publisher_table,
                                    DB_TABLE_DEFAULT_INDEX,
                                    &cursor,&low_key,&high_key);
    }
    else
    {
        dbrc = DB_Table_select_all(participant->remote_publisher_table,
                                  DB_TABLE_DEFAULT_INDEX,&cursor);
    }

    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,participant->remote_publisher_table,dbrc)
        ddsrc = DDS_RETCODE_ERROR;
        goto done;
    }

    dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&remote_pub);
    while (dbrc == DB_RETCODE_OK)
    {
        if (DDS_BuiltinTopicKey_equals(&remote_participant->orig_key,
                                       &remote_pub->data.participant_key))
        {
            ddsrc = remote_pub->as_entity.enable_func(participant,
                                                      &remote_pub->as_entity,
                                                      new_key);
            if (ddsrc != DDS_RETCODE_OK)
            {
                DDSC_LOG_ENTITY_ENABLE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLICATION_ENTITY)
                DB_Cursor_finish(participant->remote_publisher_table,cursor);
                goto done;
            }
        }
        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&remote_pub);
    }
    DB_Cursor_finish(participant->remote_publisher_table,cursor);

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
        ddsrc = DDS_RETCODE_ERROR;
        goto done;
    }

    /* Subscriber */
    cursor = NULL;

    if (new_key)
    {
        dbrc = DB_Table_select_range(participant->remote_subscriber_table,
                                    DB_TABLE_DEFAULT_INDEX,
                                    &cursor,&low_key,&high_key);
    }
    else
    {
        dbrc = DB_Table_select_all(participant->remote_subscriber_table,
                                  DB_TABLE_DEFAULT_INDEX,&cursor);
    }

    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,participant->remote_subscriber_table,dbrc)
        ddsrc = DDS_RETCODE_ERROR;
        goto done;
    }

    dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&remote_sub);
    while (dbrc == DB_RETCODE_OK)
    {
        if (DDS_BuiltinTopicKey_equals(&remote_participant->orig_key,
                                       &remote_sub->data.participant_key))
        {
            ddsrc = remote_sub->as_entity.enable_func(participant,
                                                      &remote_sub->as_entity,
                                                      new_key);
            if (ddsrc != DDS_RETCODE_OK)
            {
                DDSC_LOG_ENTITY_ENABLE(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIPTION_ENTITY)
                DB_Cursor_finish(participant->remote_subscriber_table,cursor);
                goto done;
            }
        }
        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&remote_sub);
    }
    DB_Cursor_finish(participant->remote_subscriber_table,cursor);

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
        ddsrc = DDS_RETCODE_ERROR;
        goto done;
    }

    if (!DDS_Duration_is_infinite(
                          &remote_participant->data.liveliness_lease_duration))
    {
        storage.field[0] = (void *)remote_participant;
        storage.field[1] = (void *)participant;

        if (!OSAPI_Timer_create_timeout(remote_participant->timer,
                &remote_participant->lease_duration_event,
                remote_participant->data.liveliness_lease_duration.sec,
                (RTI_INT32)remote_participant->data.liveliness_lease_duration.nanosec,
                OSAPI_TIMER_ONE_SHOT,
                NDDS_RemoteParticipant_on_liveliness,
                &storage))
        {
            ddsrc = DDS_RETCODE_OUT_OF_RESOURCES;
            remote_participant->as_entity.state = RTIDDS_ENTITY_STATE_CREATED;
            DDSC_LOG_TIMER_CREATE_TIMEOUT(OSAPI_LOGKIND_ERROR)
            goto done;
        }
    }

    ddsrc = DDS_RETCODE_OK;

done:
    if (DB_Database_unlock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return ddsrc;
}

DDS_ReturnCode_t
NDDS_DomainParticipant_enable_remote_participant_name(
                    DDS_DomainParticipant *const self,
                    const struct DDS_ParticipantBuiltinTopicData *const data)
{
    struct DDS_DomainParticipantImpl *participant =
                                    (struct DDS_DomainParticipantImpl *)self;
    DDS_BuiltinTopicKey_t old_key;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    struct DDS_RemoteParticipantImpl *remote_participant = NULL;

    OSAPI_PRECONDITION((self == NULL) || (data == NULL),
                   return DDS_RETCODE_BAD_PARAMETER,
                   OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                   OSAPI_Log_entry_add_pointer("data",data,RTI_TRUE);)

    if (DB_Database_lock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    remote_participant = NDDS_DomainParticipant_lookup_name(participant,
                                                  data->participant_name.name);
    if (remote_participant == NULL)
    {
        DDSC_LOG_PARTICIPANT_LOOKUP(OSAPI_LOGKIND_ERROR,
                                    data->participant_name.name)
        goto done;
    }

    /* If the GUID is specified, it matches that of the record, otherwise
     * it is an inconsistency
     */
    if (!DDS_BuiltinTopicKey_prefix_equals(&remote_participant->data.key,
                                           &DDS_BUILTINTOPICKEY_PREFIX_UNKNOWN)
        && (!DDS_BuiltinTopicKey_equals(&remote_participant->data.key,
                                        &data->key) ||
             DDS_BuiltinTopicKey_suffix_equals(&data->key,
                                               &DDS_BUILTINTOPICKEY_UNKNOWN)))
    {
        DDSC_LOG_INVALID_PARTICIPANT_GUID_PREFIX(OSAPI_LOGKIND_ERROR)
        retcode = DDS_RETCODE_PRECONDITION_NOT_MET;
        goto done;
    }

    /* Ensure that the GUID is not unknown (or internal) when it is enabled */
    if (DDS_BuiltinTopicKey_prefix_equals(&remote_participant->data.key,
                                          &DDS_BUILTINTOPICKEY_UNKNOWN) &&
        (DDS_BuiltinTopicKey_prefix_equals(&data->key,
                                           &DDS_BUILTINTOPICKEY_UNKNOWN) ||
         DDS_BuiltinTopicKey_suffix_equals(&data->key,
                                           &DDS_BUILTINTOPICKEY_UNKNOWN)))
    {
        DDSC_LOG_INVALID_PARTICIPANT_GUID_PREFIX(OSAPI_LOGKIND_ERROR)
        retcode = DDS_RETCODE_PRECONDITION_NOT_MET;
        goto done;
    }

    old_key = remote_participant->data.key;
    if (!DDS_ParticipantBuiltinTopicData_set_from(&remote_participant->data, data,
                                                    DDS_BOOLEAN_TRUE, participant))
    {
        DDSC_LOG_OBJECT_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANTDATA_OBJECT)
        retcode = DDS_RETCODE_OUT_OF_RESOURCES;
        goto done;
    }

    remote_participant->data.key = old_key;
    retcode = remote_participant->as_entity.enable_func(participant,
                                    (NDDS_RemoteEntity *)remote_participant,
                                     &data->key);

done:

    if (DB_Database_unlock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}

DDS_ReturnCode_t
NDDS_DomainParticipant_enable_remote_participant_guid(
                DDS_DomainParticipant *const self,
                const struct DDS_ParticipantBuiltinTopicData *const data)
{
    struct DDS_DomainParticipantImpl *participant =
                                    (struct DDS_DomainParticipantImpl *)self;
    struct DDS_RemoteParticipantImpl *record = NULL;
    DB_ReturnCode_T dbrc;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION((self == NULL) || (data == NULL),
          return DDS_RETCODE_BAD_PARAMETER,
                  OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                  OSAPI_Log_entry_add_pointer("data",data,RTI_TRUE);)

    if (DB_Database_lock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    record = NULL;
    dbrc = DB_Table_select_match(participant->remote_participant_table,
                                DB_TABLE_DEFAULT_INDEX,
                                (DB_Record_T*)&record,
                                (DB_Key_T)&data->key);
    if (dbrc != DB_RETCODE_OK)
    {
        goto done;
    }
    if (&record->data != data)
    {
        if (!DDS_ParticipantBuiltinTopicData_set_from(&record->data, data,
                                                        DDS_BOOLEAN_TRUE, self))
        {
            DDSC_LOG_OBJECT_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANTDATA_OBJECT)
            retcode = DDS_RETCODE_OUT_OF_RESOURCES;
            goto done;
        }
    }

    retcode = record->as_entity.enable_func(
                             participant, (NDDS_RemoteEntity *) record,NULL);

#if OSAPI_ENABLE_LOG
    /* Enabling is based on discovery, that is this is an asynchronous event,
     * continue
     */
    if (retcode != DDS_RETCODE_OK)
    {
        DDSC_LOG_ENTITY_ENABLE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_ENTITY)
    }
#endif

done:
    if (DB_Database_unlock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}

#ifndef RTI_CERT
RTI_BOOL
NDDS_RemoteParticipantRecord_finalize(const void *record, void *param)
{
    RTI_BOOL retval = RTI_FALSE;
    struct DDS_RemoteParticipantImpl *rem_part =
        (struct DDS_RemoteParticipantImpl *)record;
    struct DDS_DomainParticipantImpl *participant =
            (struct DDS_DomainParticipantImpl*)param;

    OSAPI_PRECONDITION(record == NULL || param == NULL,
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("record",record,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("param",param,RTI_TRUE);)

    if (!DDS_ParticipantBuiltinTopicData_finalize_no_dealloc(&rem_part->data, participant))
    {
        goto done;
    }

    retval = RTI_TRUE;

done:
    return retval;
}
#endif

/*ci
 * \brief Reset/Remove a remote participant from a participant
 *
 * \details
 * This function removes or resets a remote participant and all its endpoints
 * in the participant. When the remote endpoints are reset/removed they are also
 * unmatched from local endpoints. However, the remote participants and remote
 * endpoints are not removed from the participant if a reset is performed.
 * This feature is typically used in static discovery where remote participants
 * and endpoints are statically asserted. After they have been reset they can be
 * re-enabled with one the remote participant enable functions
 * \ref NDDS_DomainParticipant_enable_remote_participant_guid or
 * \ref NDDS_DomainParticipant_enable_remote_participant_name
 *
 * \param[in] self        The participant to remove the remote participant from
 * \param[in] key         The key of the remote participant to remove
 * \param[in] reset_entry Whether the remote participant should be reset or
 *                        removed.
 *
 * \return DDS_RETCODE_OK on success,one of the standard error codes on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE DDS_ReturnCode_t
NDDS_RemoteParticipant_remove_internal(DDS_DomainParticipant *const self,
                                       const DDS_BuiltinTopicKey_t *const key,
                                       DDS_Boolean reset_entry)
{
    struct DDS_DomainParticipantImpl *participant =
                                    (struct DDS_DomainParticipantImpl *)self;
    struct DDS_RemoteParticipantImpl *remote_participant = NULL;
    DB_ReturnCode_T dbrc;
    struct DDS_RemotePublicationImpl *rem_publication;
    struct DDS_RemoteSubscriptionImpl *rem_subscription;
    DB_Cursor_T cursor = NULL;
    DDS_BuiltinTopicKey_t low_key = DDS_BuiltinTopicKey_t_INITIALIZER;
    DDS_BuiltinTopicKey_t high_key = DDS_BuiltinTopicKey_t_INITIALIZER;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION((self == NULL) || (key == NULL),
          return DDS_RETCODE_BAD_PARAMETER,
                  OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                  OSAPI_Log_entry_add_pointer("key",key,RTI_TRUE);)

    if (DB_Database_lock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    dbrc = DB_Table_select_match(participant->remote_participant_table,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&remote_participant,
                                 (DB_Key_T)key);
    if (dbrc != DB_RETCODE_OK)
    {
        /* This is not necessarily an error, for example we may receive multiple
         * dispose for the same participant and remove it on the first dispose
         */
        if (dbrc == DB_RETCODE_NO_DATA)
        {
            DDSC_LOG_RECORD_SELECT(OSAPI_LOGKIND_WARNING,DDSC_LOG_PARTICIPANT_RECORD,dbrc)
            retcode = DDS_RETCODE_OK;
        }
#if OSAPI_ENABLE_LOG
        else
        {
            DDSC_LOG_RECORD_SELECT(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_RECORD,dbrc)
        }
#endif
        goto done;
    }

    /* remove any trust state associated with this remote participant */
    if (!DDS_DomainParticipant_unregister_matched_remote_participant(participant->config.trust, key))
    {
        goto done;
    }

    low_key = *key;
    high_key = *key;
    low_key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] = 0;
    high_key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] = 0xffffffff;

    cursor = NULL;
    dbrc = DB_Table_select_range(participant->remote_publisher_table,
                                  DB_TABLE_DEFAULT_INDEX,&cursor,
                                  &low_key,&high_key);

    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,participant->remote_publisher_table,dbrc)
        goto done;
    }

    /* First remove all queued discovery messages for the removed participant
     * to avoid processing queued messages for the remove participant
     */
    DDS_DiscoveryQueue_purge_by_prefix(participant,
                   &participant->discovery_queue,&remote_participant->data.key);

    do
    {
        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*) &rem_publication);
        if (dbrc == DB_RETCODE_OK)
        {
            if (DDS_BuiltinTopicKey_equals(&remote_participant->data.key,
                                    &rem_publication->data.participant_key))
            {
                if (NDDS_RemotePublication_remove_internal(participant,
                        &rem_publication->data.key,reset_entry) != DDS_RETCODE_OK)
                {
                    DDSC_LOG_REMOVE_PUBLICATION(OSAPI_LOGKIND_ERROR)
                    retcode = DDS_RETCODE_ERROR;
                    DB_Cursor_finish(participant->remote_publisher_table,cursor);
                    goto done;
                }
            }
        }
    } while (dbrc == DB_RETCODE_OK);

    DB_Cursor_finish(participant->remote_publisher_table,cursor);

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    cursor = NULL;
    dbrc = DB_Table_select_range(participant->remote_subscriber_table,
                                DB_TABLE_DEFAULT_INDEX,&cursor,
                                &low_key,&high_key);

    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,participant->remote_subscriber_table,dbrc)
        goto done;
    }

    do
    {
        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*) &rem_subscription);
        if (dbrc == DB_RETCODE_OK)
        {
            if (DDS_BuiltinTopicKey_equals(&remote_participant->data.key,
                    &rem_subscription->data.participant_key))
            {
                if (NDDS_RemoteSubscription_remove_internal(participant,
                        &rem_subscription->data.key,reset_entry) != DDS_RETCODE_OK)
                {
                    DDSC_LOG_REMOVE_SUBSCRIPTION(OSAPI_LOGKIND_ERROR)
                    retcode = DDS_RETCODE_ERROR;
                    DB_Cursor_finish(participant->remote_subscriber_table,cursor);
                    goto done;
                }
            }
        }
    } while (dbrc == DB_RETCODE_OK);

    DB_Cursor_finish(participant->remote_subscriber_table,cursor);

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
        goto done;
    }


#ifndef RTI_CERT
    if (!NDDS_DomainParticipant_delete_remote_participant_routes(participant,
                                                           remote_participant))
    {
        goto done;
    }
#endif /*RTI_CERT*/

    if (participant->disc_plugin != NULL)
    {
        if (!NDDS_Discovery_Plugin_on_before_remote_participant_deleted(
                                                participant->disc_plugin,
                                                participant,
                                                &remote_participant->data,
                                                remote_participant->status))
        {
            DDSC_LOG_DISC_BEFORE_REMOTE_PARTICIPANT_DELETED(OSAPI_LOGKIND_WARNING,
                                 RT_ComponentFactoryId_get_name(
                                    &participant->qos.discovery.discovery.name))
            goto done;
        }
    }

    remote_participant = NULL;
    dbrc = DB_Table_remove_record(participant->remote_participant_table,
                                (DB_Record_T*)&remote_participant,
                                (DB_Key_T)key);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_REMOVE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_RECORD,dbrc)
        goto done;
    }

    if ((remote_participant->lease_duration_event.epoch > -1) &&
        !OSAPI_Timer_delete_timeout(remote_participant->timer,
                                   &remote_participant->lease_duration_event))
    {
        DDSC_LOG_OBJECT_DELETE(OSAPI_LOGKIND_ERROR,DDSC_LOG_TIMEROUT_OBJECT);
        goto done;
    }

/* reset_entry is always TRUE for RTI_CERT */
#ifndef RTI_CERT
    if (reset_entry)
    {
#endif /* !RTI_CERT */

        remote_participant->status = DDS_REMOTE_PARTICIPANT_STATUS_DEFAULT;
        DDS_RemoteParticipant_enable_status(remote_participant,
                                            DDS_REMOTE_PARTICIPANT_STATUS_STATIC);

        remote_participant->as_entity.state = RTIDDS_ENTITY_STATE_CREATED;
        remote_participant->data.key = remote_participant->orig_key;
        dbrc = DB_Table_insert_record(participant->remote_participant_table,
                                     (DB_Record_T)remote_participant);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_RECORD_INSERT(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_RECORD,dbrc)
            retcode = DDS_RETCODE_PRECONDITION_NOT_MET;
            goto done;
        }
#ifndef RTI_CERT
    }
    else
    {
        if (!NDDS_RemoteParticipantRecord_finalize(remote_participant,
                                                   participant))
        {
            goto done;
        }

        dbrc = DB_Table_delete_record(participant->remote_participant_table,
                                     remote_participant);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_RECORD_DELETE(OSAPI_LOGKIND_ERROR,
                    DDSC_LOG_PARTICIPANT_RECORD, dbrc)
            retcode = DDS_RETCODE_ERROR;
            goto done;
        }
    }
#endif /* !RTI_CERT */

    retcode = DDS_RETCODE_OK;

done:
    if (DB_Database_unlock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}

#ifndef RTI_CERT
DDS_ReturnCode_t
DDS_DataWriter_remove_all_peers(DDS_DataWriter *const datawriter,
                                DDS_UnsignedLong object_id)
{
    struct DDS_DomainParticipantImpl *participant;
    struct DDS_RemoteParticipantImpl *remote_participant = NULL;
    DB_ReturnCode_T dbrc;
    DB_Cursor_T cursor = NULL;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_BuiltinTopicKey_t dr_key = DDS_BuiltinTopicKey_t_INITIALIZER;
    RTI_BOOL route_existed = RTI_FALSE;

    OSAPI_PRECONDITION((datawriter == NULL),
          return DDS_RETCODE_BAD_PARAMETER,
              OSAPI_Log_entry_add_pointer("datawriter",datawriter,RTI_TRUE);)

    participant = DDS_Publisher_get_participant(
                                    DDS_DataWriter_get_publisher(datawriter));

    if (DB_Database_lock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    cursor = NULL;
    dbrc = DB_Table_select_all(participant->remote_participant_table,
                                 DB_TABLE_DEFAULT_INDEX,&cursor);

    if (dbrc != DB_RETCODE_OK)
    {
        /* This is not necessarily an error, for example we may receive multiple
         * dispose for the same participant and remove it on the first dispose
         */
        if (dbrc == DB_RETCODE_NO_DATA)
        {
            DDSC_LOG_RECORD_SELECT(OSAPI_LOGKIND_WARNING,DDSC_LOG_PARTICIPANT_RECORD,dbrc)
            retcode = DDS_RETCODE_OK;
        }
#if OSAPI_ENABLE_LOG
        else
        {
            DDSC_LOG_RECORD_SELECT(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_RECORD,dbrc)
        }
#endif
        goto done;
    }

    do
    {
        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&remote_participant);
        if (dbrc == DB_RETCODE_OK)
        {
            dr_key = remote_participant->data.key;
            dr_key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] = object_id;

            if (!DDS_DataWriter_delete_route(datawriter,&dr_key,
                    &remote_participant->data.metatraffic_unicast_locators,
                    &remote_participant->data.metatraffic_multicast_locators,
                    &route_existed))
            {
                goto done;
            }
        }
    } while (dbrc == DB_RETCODE_OK);

    DB_Cursor_finish(participant->remote_participant_table,cursor);

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    if (DB_Database_unlock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}
#endif

#ifndef RTI_CERT
DDS_ReturnCode_t
DDS_DataReader_remove_all_peers(DDS_DataReader *const datareader,
                                DDS_UnsignedLong object_id)
{
    struct DDS_DomainParticipantImpl *participant;
    struct DDS_RemoteParticipantImpl *remote_participant = NULL;
    DB_ReturnCode_T dbrc;
    DB_Cursor_T cursor = NULL;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_BuiltinTopicKey_t dw_key = DDS_BuiltinTopicKey_t_INITIALIZER;
    RTI_BOOL route_existed = RTI_FALSE;

    OSAPI_PRECONDITION((datareader == NULL),
              return DDS_RETCODE_BAD_PARAMETER,
              OSAPI_Log_entry_add_pointer("datareader",datareader,RTI_TRUE);)

    participant = DDS_Subscriber_get_participant(
                                    DDS_DataReader_get_subscriber(datareader));

    if (DB_Database_lock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    cursor = NULL;
    dbrc = DB_Table_select_all(participant->remote_participant_table,
                                 DB_TABLE_DEFAULT_INDEX,&cursor);

    if (dbrc != DB_RETCODE_OK)
    {
        /* This is not necessarily an error, for example we may receive multiple
         * dispose for the same participant and remove it on the first dispose
         */
        if (dbrc == DB_RETCODE_NO_DATA)
        {
            DDSC_LOG_RECORD_SELECT(OSAPI_LOGKIND_WARNING,DDSC_LOG_PARTICIPANT_RECORD,dbrc)
            retcode = DDS_RETCODE_OK;
        }
#if OSAPI_ENABLE_LOG
        else
        {
            DDSC_LOG_RECORD_SELECT(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_RECORD,dbrc)
        }
#endif
        goto done;
    }

    do
    {
        dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&remote_participant);
        if (dbrc == DB_RETCODE_OK)
        {
            dw_key = remote_participant->data.key;
            dw_key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] = object_id;

            if (!DDS_DataReader_delete_route(datareader,&dw_key,
                    &remote_participant->data.metatraffic_unicast_locators,
                    &remote_participant->data.metatraffic_multicast_locators,
                    &route_existed))
            {
                goto done;
            }
        }
    } while (dbrc == DB_RETCODE_OK);

    DB_Cursor_finish(participant->remote_participant_table,cursor);

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    if (DB_Database_unlock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}
#endif

DDS_ReturnCode_t
DDS_DataWriter_remove_peer(DDS_DataWriter *const datawriter,
                           const DDS_BuiltinTopicKey_t *const dp_key,
                           DDS_UnsignedLong object_id)
{
    struct DDS_DomainParticipantImpl *participant;
    struct DDS_RemoteParticipantImpl *remote_participant = NULL;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DB_ReturnCode_T dbrc;
    DDS_BuiltinTopicKey_t dr_key = DDS_BuiltinTopicKey_t_INITIALIZER;
    RTI_BOOL route_existed = RTI_FALSE;

    OSAPI_PRECONDITION((datawriter == NULL) || (dp_key == NULL),
              return DDS_RETCODE_BAD_PARAMETER,
              OSAPI_Log_entry_add_pointer("datawriter",datawriter,RTI_FALSE);
              OSAPI_Log_entry_add_pointer("dp_key",dp_key,RTI_TRUE);)

    participant = DDS_Publisher_get_participant(
                                    DDS_DataWriter_get_publisher(datawriter));

    if (DB_Database_lock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    dbrc = DB_Table_select_match(participant->remote_participant_table,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&remote_participant,
                                 (DB_Key_T)dp_key);
    if (dbrc != DB_RETCODE_OK)
    {
        /* This is not necessarily an error, for example we may receive multiple
         * dispose for the same participant and remove it on the first dispose
         */
        if (dbrc == DB_RETCODE_NO_DATA)
        {
            DDSC_LOG_RECORD_SELECT(OSAPI_LOGKIND_WARNING,DDSC_LOG_PARTICIPANT_RECORD,dbrc)
            retcode = DDS_RETCODE_OK;
        }
#if OSAPI_ENABLE_LOG
        else
        {
            DDSC_LOG_RECORD_SELECT(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_RECORD,dbrc)
        }
#endif
        goto done;
    }

    dr_key = remote_participant->data.key;
    dr_key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] = object_id;

    if (!DDS_DataWriter_delete_route(datawriter,&dr_key,
            &remote_participant->data.metatraffic_unicast_locators,
            &remote_participant->data.metatraffic_multicast_locators,
            &route_existed))
    {
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    if (DB_Database_unlock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}

DDS_ReturnCode_t
DDS_DataWriter_remove_participant_routes(DDS_DataWriter *const datawriter,
                                         const DDS_BuiltinTopicKey_t *const dp_key)
{
    struct DDS_DomainParticipantImpl *participant;
    struct DDS_RemoteParticipantImpl *remote_participant = NULL;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DB_ReturnCode_T dbrc;
    struct NETIO_Address dst_reader;

    OSAPI_PRECONDITION((datawriter == NULL) || (dp_key == NULL),
          return DDS_RETCODE_BAD_PARAMETER,
                  OSAPI_Log_entry_add_pointer("datawriter",datawriter,RTI_FALSE);
                  OSAPI_Log_entry_add_pointer("dp_key",dp_key,RTI_TRUE);)

    participant = DDS_Publisher_get_participant(
                                    DDS_DataWriter_get_publisher(datawriter));

    if (DB_Database_lock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    dbrc = DB_Table_select_match(participant->remote_participant_table,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&remote_participant,
                                 (DB_Key_T)dp_key);
    if (dbrc != DB_RETCODE_OK)
    {
        /* This is not necessarily an error, for example we may receive multiple
         * dispose for the same participant and remove it on the first dispose
         */
        if (dbrc == DB_RETCODE_NO_DATA)
        {
            DDSC_LOG_RECORD_SELECT(OSAPI_LOGKIND_WARNING,DDSC_LOG_PARTICIPANT_RECORD,dbrc)
            retcode = DDS_RETCODE_OK;
        }
#if OSAPI_ENABLE_LOG
        else
        {
            DDSC_LOG_RECORD_SELECT(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_RECORD,dbrc)
        }
#endif
        goto done;
    }

    NETIO_Address_init(&dst_reader,NETIO_ADDRESS_KIND_INTRA);
    dst_reader.value.rtps_guid.object_id =
                    NETIO_htonl(RTPS_OBJECT_ID_READER_SDP_PARTICIPANT);

    if (!DDS_DataWriter_delete_anonymous_route_from_seq(
                    datawriter,
                    &dst_reader,
                    &remote_participant->data.metatraffic_unicast_locators))
    {
        goto done;
    }

    if (!DDS_DataWriter_delete_anonymous_route_from_seq(
                    datawriter,
                    &dst_reader,
                    &remote_participant->data.metatraffic_multicast_locators))
    {
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:

    if (DB_Database_unlock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}

DDS_ReturnCode_t
DDS_DataReader_remove_peer(DDS_DataReader *const datareader,
                           const DDS_BuiltinTopicKey_t *const dp_key,
                           DDS_UnsignedLong object_id)
{
    struct DDS_DomainParticipantImpl *participant;
    struct DDS_RemoteParticipantImpl *remote_participant = NULL;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DB_ReturnCode_T dbrc;
    DDS_BuiltinTopicKey_t dw_key = DDS_BuiltinTopicKey_t_INITIALIZER;
    RTI_BOOL route_existed = RTI_FALSE;

    OSAPI_PRECONDITION((datareader == NULL) || (dp_key == NULL),
          return DDS_RETCODE_BAD_PARAMETER,
                  OSAPI_Log_entry_add_pointer("datareader",datareader,RTI_FALSE);
                  OSAPI_Log_entry_add_pointer("dp_key",dp_key,RTI_TRUE);)

    participant = DDS_Subscriber_get_participant(
                                    DDS_DataReader_get_subscriber(datareader));

    if (DB_Database_lock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    dbrc = DB_Table_select_match(participant->remote_participant_table,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&remote_participant,
                                 (DB_Key_T)dp_key);
    if (dbrc != DB_RETCODE_OK)
    {
        /* This is not necessarily an error, for example we may receive multiple
         * dispose for the same participant and remove it on the first dispose
         */
        if (dbrc == DB_RETCODE_NO_DATA)
        {
            DDSC_LOG_RECORD_SELECT(OSAPI_LOGKIND_WARNING,DDSC_LOG_PARTICIPANT_RECORD,dbrc)
            retcode = DDS_RETCODE_OK;
        }
#if OSAPI_ENABLE_LOG
        else
        {
            DDSC_LOG_RECORD_SELECT(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_RECORD,dbrc)
        }
#endif
        goto done;
    }

    dw_key = remote_participant->data.key;
    dw_key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] = object_id;

    if (!DDS_DataReader_delete_route(datareader,&dw_key,
            &remote_participant->data.metatraffic_unicast_locators,
            &remote_participant->data.metatraffic_multicast_locators,
            &route_existed))
    {
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    if (DB_Database_unlock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}

DDS_ReturnCode_t
DDS_DataWriter_add_peer(DDS_DataWriter *const datawriter,
                        const DDS_BuiltinTopicKey_t *const dp_key,
                        const struct DDS_DataReaderQos *const dr_qos,
                        DDS_UnsignedLong object_id)
{
    struct DDS_DomainParticipantImpl *participant;
    DDS_BuiltinTopicKey_t key;
    RTI_BOOL route_existed;
    struct DDS_RemoteParticipantImpl *remote_participant = NULL;
    DB_ReturnCode_T dbrc;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION((datawriter == NULL) || (dp_key == NULL) ||
                       (dr_qos == NULL),
          return DDS_RETCODE_BAD_PARAMETER,
                  OSAPI_Log_entry_add_pointer("datawriter",datawriter,RTI_FALSE);
                  OSAPI_Log_entry_add_pointer("dr_qos",dr_qos,RTI_FALSE);
                  OSAPI_Log_entry_add_pointer("dp_key",dp_key,RTI_TRUE);)

    participant = DDS_Publisher_get_participant(
                                    DDS_DataWriter_get_publisher(datawriter));

    if (DB_Database_lock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    dbrc = DB_Table_select_match(participant->remote_participant_table,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&remote_participant,
                                 (DB_Key_T)dp_key);
    if (dbrc != DB_RETCODE_OK)
    {
        /* This is not necessarily an error, for example we may receive multiple
         * dispose for the same participant and remove it on the first dispose
         */
        if (dbrc == DB_RETCODE_NO_DATA)
        {
            DDSC_LOG_RECORD_SELECT(OSAPI_LOGKIND_WARNING,DDSC_LOG_PARTICIPANT_RECORD,dbrc)
            retcode = DDS_RETCODE_OK;
        }
#if OSAPI_ENABLE_LOG
        else
        {
            DDSC_LOG_RECORD_SELECT(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_RECORD,dbrc)
        }
#endif
        goto done;
    }

    key = remote_participant->data.key;
    key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] = object_id;

    if (!DDS_DataWriter_add_route(datawriter,
                             &key,dr_qos,
                             &remote_participant->data.metatraffic_unicast_locators,
                             &remote_participant->data.metatraffic_multicast_locators,
                             DDS_ENCAPSULATION_ID_RESOLVE,
                             DDS_XCDR_DATA_REPRESENTATION,
                             &route_existed))
    {
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:

    if (DB_Database_unlock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;

}

DDS_ReturnCode_t
DDS_DataReader_add_peer(DDS_DataReader *const datareader,
                        const DDS_BuiltinTopicKey_t *const dp_key,
                        const struct DDS_DataWriterQos *const dw_qos,
                        DDS_UnsignedLong object_id)
{
    struct DDS_DomainParticipantImpl *participant;
    DDS_BuiltinTopicKey_t key;
    RTI_BOOL route_existed;
    struct DDS_RemoteParticipantImpl *remote_participant = NULL;
    DB_ReturnCode_T dbrc;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION((datareader == NULL) || (dp_key == NULL) ||
                       (dw_qos == NULL),
          return DDS_RETCODE_BAD_PARAMETER,
              OSAPI_Log_entry_add_pointer("datareader",datareader,RTI_FALSE);
              OSAPI_Log_entry_add_pointer("dw_qos",dw_qos,RTI_FALSE);
              OSAPI_Log_entry_add_pointer("dp_key",dp_key,RTI_TRUE);)

    participant = DDS_Subscriber_get_participant(
                                    DDS_DataReader_get_subscriber(datareader));

    if (DB_Database_lock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    dbrc = DB_Table_select_match(participant->remote_participant_table,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&remote_participant,
                                 (DB_Key_T)dp_key);
    if (dbrc != DB_RETCODE_OK)
    {
        /* This is not necessarily an error, for example we may receive multiple
         * dispose for the same participant and remove it on the first dispose
         */
        if (dbrc == DB_RETCODE_NO_DATA)
        {
            DDSC_LOG_RECORD_SELECT(OSAPI_LOGKIND_WARNING,DDSC_LOG_PARTICIPANT_RECORD,dbrc)
            retcode = DDS_RETCODE_OK;
        }
#if OSAPI_ENABLE_LOG
        else
        {
            DDSC_LOG_RECORD_SELECT(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_RECORD,dbrc)
        }
#endif
        goto done;
    }

    key = remote_participant->data.key;
    key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] = object_id;

    if (!DDS_DataReader_add_route(datareader,
                             &key,dw_qos,
                             &remote_participant->data.metatraffic_unicast_locators,
                             &remote_participant->data.metatraffic_multicast_locators,
                             &route_existed))
    {
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:

    if (DB_Database_unlock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}

#ifndef RTI_CERT
DDS_ReturnCode_t
NDDS_DomainParticipant_remove_remote_participant(
                                DDS_DomainParticipant *const participant,
                                const DDS_BuiltinTopicKey_t *const key)
{
    OSAPI_TRACE_TRUST_LOG_GUIDS(
                "DP_REMOVE",
                &participant->rtps_intf->local_address.value.guid,
                &key)
    return NDDS_RemoteParticipant_remove_internal(participant,
                                                  key, DDS_BOOLEAN_FALSE);
}
#endif /* !RTI_CERT */

DDS_ReturnCode_t
NDDS_DomainParticipant_reset_remote_participant(
                                DDS_DomainParticipant *const participant,
                                const struct DDS_BuiltinTopicKey_t *const key)
{
    return NDDS_RemoteParticipant_remove_internal(participant,
                                                  key, DDS_BOOLEAN_TRUE);
}


RTI_BOOL
DDS_DomainParticipant_after_remote_participant_ready(
        struct DDS_DomainParticipantImpl *participant,
        struct DDS_RemoteParticipantImpl *remote_dp)
{
    OSAPI_PRECONDITION(participant == NULL || remote_dp == NULL ||
        DDS_RemoteParticipant_check_status(remote_dp,
            DDS_REMOTE_PARTICIPANT_STATUS_ENABLED),
        return RTI_FALSE,
        OSAPI_Log_entry_add_pointer("participant",participant,RTI_FALSE);
        OSAPI_Log_entry_add_uint("status",remote_dp != NULL ? remote_dp->status : 0,RTI_FALSE);
        OSAPI_Log_entry_add_pointer("remote_dp",remote_dp,RTI_TRUE);)

    OSAPI_TRACE_TRUST_DP_READY(
            &participant->rtps_intf->local_address.value.guid,
            &remote_dp->data.key)

#if DDS_LIVELINESS_CHANNEL_ENABLED
    /* assert channel routes if the remote participant
     * has bits participant message dw/dr set
     */
    if ((remote_dp->data.dds_builtin_endpoints &
        (DDS_BUILTIN_ENDPOINT_MESSAGE_DATA_ANNOUNCER |
         DDS_BUILTIN_ENDPOINT_MESSAGE_DATA_DETECTOR)) &&
        (!remote_dp->ch_partmsgdata_asserted))
    {
        if (DDS_DomainParticipant_is_ipc_liveliness_enabled(participant) &&
            !DDS_IpcLiveliness_assert_routes(participant->ipc_liveliness,
                                             remote_dp))
        {
            DDS_LOG_IPC_ASSERT_ROUTES(OSAPI_LOGKIND_ERROR)
            return RTI_FALSE;
        }
        remote_dp->ch_partmsgdata_asserted = RTI_TRUE;
    }
#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */

    if (participant->disc_plugin != NULL)
    {
        /* Assert discovery endpoints */
        NDDS_Discovery_Plugin_assert_remote_participant(
                participant->disc_plugin,
                participant,
                &remote_dp->data);
    }

    DDS_RemoteParticipant_enable_status(
                remote_dp, DDS_REMOTE_PARTICIPANT_STATUS_ENABLED);


    return RTI_TRUE;
}

/*ci @}  */
