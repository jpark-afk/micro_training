/*
 * FILE: RemotePublication.c - Remote publication implementation
 *
 * (c) Copyright, Real-Time Innovations, 2008-2021.
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
 * 13dec2021,tk MICRO-3226/PR.29479
 * - Updated comments to comply with coding standards.
 * 13apr2021,tk MICRO-3028/PR.28960
 * - Removed inclusion of "TopicQos.h" for CERT
 * 12mar2015,tk MICRO-1108 Return BAD_PARAMETER instead of PRECONDITION_NOT_MET
 * 20feb2015,eh MICRO-813/PR#9172 Fix Lint warnings
 * 02feb2015,tk MICRO-1000/PR#13242 Use different log message for select/remove
 * 21jan2015,tk MICRO-998/PR#13239  Return if a record cannot be removed
 * 21jan2015,tk MICRO-1000/PR#13242 Added log-message
 * 10dec2014,tk MICRO-978/PR#12914  Consistently return records on failure
 * 31jul2014,tk MICRO-842/PR#9683   Removed superfluous paramaters in DB
 *                                  compare function
 * 04mar2014,tk MICRO-211: Unique object id across systems not needed
 * 17may2012,tk Updated
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief Remote publication implementation
 *
 * \details
 * This file implements functions to manage remote publications, such
 * as discovery and matching with local endpoints.
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
#ifndef dds_c_log_h
#include "dds_c/dds_c_log.h"
#endif

#include "Entity.h"
#include "RemoteEntity.h"
#include "RemoteEndpoint.h"
#include "QosPolicy.h"
#include "BuiltinTopicKey.h"
#include "TopicDescription.h"
#include "Topic.h"
#include "Type.h"
#include "DomainParticipantQos.h"
#include "DomainParticipantEvent.h"
#include "DomainParticipant.h"
#include "RemoteParticipant.h"
#include "RemotePublication.h"
#include "DataReaderDiscovery.h"

/*ci \brief OMG defined name for the PublicationBuiltinTopicData Type
 */
const char *const DDS_PUBLICATION_BUILTIN_TOPIC_TYPE_NAME = 
                                        "DDS_PublicationBuiltinTopicData";
                                        
/*ci \brief OMG defined name for the PublicationBuiltinTopicData Topic
 */                                        
const char *const DDS_PUBLICATION_BUILTIN_TOPIC_NAME = "DCPSPublication";

/*
 *                                 Internal API
 */
MUST_CHECK_RETURN RTI_PRIVATE DDS_ReturnCode_t
NDDS_RemotePublication_enable(DDS_DomainParticipant *const participant,
                              NDDS_RemoteEntity *entity,
                              const DDS_BuiltinTopicKey_t *new_key);

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Compare entries in the table of remote publications. The function is
 *        compatible with \ref DB_IndexCompare_T
 *
 * \param[in] flags Used to distinguish between op2 being a key or record
 * \param[in] op1   A DDS_RemotePublicationImpl already in the database
 * \param[in] op2   Either a DDS_RemotePublicationImpl being added or a key
 *                  being searched for
 *
 * \return positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2,
 *         zero if op1 is equal to op2
 */
MUST_CHECK_RETURN RTI_INT32
DDS_RemotePublicationImpl_compare(RTI_INT32 flags,const DB_Record_T op1, void *op2)
{
    struct DDS_RemotePublicationImpl *record_left =
                                        (struct DDS_RemotePublicationImpl*)op1;
    const DDS_BuiltinTopicKey_t *key_right;

    if (DB_SELECT_OP2_IS_KEY(flags))
    {
        key_right = (const DDS_BuiltinTopicKey_t*)op2;
    }
    else
    {
        key_right = &((struct DDS_RemotePublicationImpl*)op2)->data.key;
    }

    return DDS_BuiltinTopicKey_compare(&record_left->data.key,key_right);
}

/*ci
 * \brief Perform matching with all datareaders in a participant
 *
 * \details
 *
 * When a remote publication is detected/modified it is matched against
 * all locally created datareaders. The participant data is used for
 * default values, such as which locators to use.
 *
 * \param[in] self        The participant
 * \param[in] parent_data The remote participant containing the publication
 * \param[in] data        The remote publication to match against
 */
RTI_PRIVATE void
NDDS_RemotePublication_match_local_reader(DDS_DomainParticipant *const self,
               const struct DDS_ParticipantBuiltinTopicData *const parent_data,
               const struct DDS_PublicationBuiltinTopicData *const data)
{
    struct DDS_DomainParticipantImpl *participant =
                                    (struct DDS_DomainParticipantImpl *)self;
    struct DDS_DataReaderImpl *local_reader;
    DB_Cursor_T dr_cursor = NULL;
    DB_ReturnCode_T dbrc;

    dbrc = DB_Table_select_all(participant->local_reader_table,
                               DB_TABLE_DEFAULT_INDEX,&dr_cursor);

    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,participant->local_reader_table,dbrc)
        return;
    }

    do
    {
        dbrc = DB_Cursor_get_next(dr_cursor,(DB_Record_T*)&local_reader);
        if (dbrc == DB_RETCODE_OK)
        {
            if (DDS_Entity_is_enabled(DDS_DataReader_as_entity(local_reader)))
            {
                DDS_DataReader_match_remote_writer(local_reader,parent_data,data);
            }
        }
    } while (dbrc == DB_RETCODE_OK);
    DB_Cursor_finish(participant->local_reader_table,dr_cursor);

#if OSAPI_ENABLE_LOG
    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
    }
#endif
}

/*ci
 * \brief Perform matching with a specific local datareader against all
 *        discovered publications
 *
 * \details
 *
 * When a local reader is created/modified it is matched against all discovered
 * publications.
 *
 * \param[in] self         The participant to perform matching in
 * \param[in] local_reader The local reader to match with
 */
void
NDDS_RemotePublication_match_with_local_reader(DDS_DomainParticipant *const self,
                                               DDS_DataReader *local_reader)
{
    struct DDS_DomainParticipantImpl *participant =
                                    (struct DDS_DomainParticipantImpl *)self;
    struct DDS_RemotePublicationImpl *rem_pub = NULL;
    struct DDS_RemoteParticipantImpl *rem_dp = NULL;
    DB_Cursor_T rempub_cursor = NULL;
    DB_ReturnCode_T dbrc;

    dbrc = DB_Table_select_all(participant->remote_publisher_table,
                               DB_TABLE_DEFAULT_INDEX,&rempub_cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,participant->remote_publisher_table,dbrc)
        return;
    }

    do
    {
        dbrc = DB_Cursor_get_next(rempub_cursor,(DB_Record_T*)&rem_pub);
        if ((dbrc == DB_RETCODE_OK) &&
            (rem_pub->as_entity.state == RTIDDS_ENTITY_STATE_ENABLED))
        {
            dbrc = DB_Table_select_match(participant->remote_participant_table,
                                     DB_TABLE_DEFAULT_INDEX,
                                     (DB_Record_T*)&rem_dp,
                                     (DB_Key_T)&rem_pub->data.participant_key);
            if (dbrc != DB_RETCODE_OK)
            {
                goto done;
            }

            DDS_DataReader_match_remote_writer(local_reader,
                                               &rem_dp->data,
                                               &rem_pub->data);
        }
    } while (dbrc == DB_RETCODE_OK);

done:

    DB_Cursor_finish(participant->remote_publisher_table,rempub_cursor);

#if OSAPI_ENABLE_LOG
    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
    }
#endif
}

/*ci
 * \brief Unmatch all local readers from a specific remote publication
 *
 * \details
 *
 * When a remote publication is removed it is unmatched from
 * all locally created datareaders.
 *
 * \param[in] self        The participant
 * \param[in] parent_data The remote participant containing the publication
 * \param[in] data        The remote publication to unmatch from
 */
RTI_PRIVATE void
NDDS_RemotePublication_unmatch_local_reader(DDS_DomainParticipant *const self,
        const struct DDS_ParticipantBuiltinTopicData *const parent_data,
        const struct DDS_PublicationBuiltinTopicData *const data)
{
    struct DDS_DomainParticipantImpl *participant =
                                    (struct DDS_DomainParticipantImpl *)self;
    struct DDS_DataReaderImpl *local_reader;
    DB_Cursor_T dr_cursor = NULL;
    DB_ReturnCode_T dbrc;

    dbrc = DB_Table_select_all(participant->local_reader_table,
                               DB_TABLE_DEFAULT_INDEX,&dr_cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,participant->local_reader_table,dbrc)
        return;
    }

    do
    {
        dbrc = DB_Cursor_get_next(dr_cursor,(DB_Record_T*)&local_reader);
        if (dbrc == DB_RETCODE_OK)
        {
                DDS_DataReader_unmatch_remote_writer(
                                                local_reader,parent_data,data);
        }
    } while (dbrc == DB_RETCODE_OK);
    DB_Cursor_finish(participant->local_reader_table,dr_cursor);

#if OSAPI_ENABLE_LOG
    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
    }
#endif
}

#ifndef RTI_CERT
/*ci
 * \brief Unmatch a local reader from a remote publication
 *
 * \param[in] participant  The participant
 * \param[in] dw_key       The key of the remote publication to unmatch
 * \param[in] local_reader The local_reader to unmatch
 */
void
NDDS_RemotePublication_unmatch_local_reader_from_key(
            DDS_DomainParticipant *const participant,
            DDS_BuiltinTopicKey_t *dw_key,
            struct DDS_DataReaderImpl *local_reader)
{
    struct DDS_RemotePublicationImpl *rem_pub = NULL;
    struct DDS_RemoteParticipantImpl *rem_dp = NULL;
    DB_ReturnCode_T dbrc;


    dbrc = DB_Table_select_match(participant->remote_publisher_table,
                                DB_TABLE_DEFAULT_INDEX,
                                (DB_Record_T*)&rem_pub,
                                (DB_Key_T)dw_key);
    if (dbrc != DB_RETCODE_OK)
    {
        return;
    }

    dbrc = DB_Table_select_match(participant->remote_participant_table,
                                DB_TABLE_DEFAULT_INDEX,
                                (DB_Record_T*)&rem_dp,
                                (DB_Key_T)&rem_pub->data.participant_key);
    if (dbrc != DB_RETCODE_OK)
    {
        return;
    }

    DDS_DataReader_unmatch_remote_writer(local_reader,
                                         &rem_dp->data,&rem_pub->data);
}
#endif /* !RTI_CERT */

/*
 *                                 Public API
 */
DDS_ReturnCode_t
NDDS_DomainParticipant_assert_remote_publication(
                    DDS_DomainParticipant *const participant,
                    const char *const participant_name,
                    const struct DDS_PublicationBuiltinTopicData *const data,
                    NDDS_TypePluginKeyKind key_kind)
{
    struct DDS_DomainParticipantImpl *self =
                        (struct DDS_DomainParticipantImpl *)participant;
    struct DDS_RemotePublicationImpl *rem_publication = NULL;
    DB_ReturnCode_T dbrc;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    struct DDS_RemoteParticipantImpl *rem_participant = NULL;
    struct DDS_BuiltinTopicKey_t tmp_key;

    OSAPI_PRECONDITION((participant == NULL) || (data == NULL),
                        return DDS_RETCODE_BAD_PARAMETER,
                        OSAPI_Log_entry_add_pointer("participant",participant,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("data",data,RTI_TRUE);)

    OSAPI_TRACE_DDS("assert publication:",RTI_FALSE)
    OSAPI_TRACE_GUID("key",&data->key,RTI_FALSE)
    OSAPI_TRACE_GUID("parent",&data->participant_key,RTI_TRUE)

    if (DDS_BuiltinTopicKey_suffix_equals(&data->key,
                                          &DDS_BUILTINTOPICKEY_UNKNOWN))
    {
        DDSC_LOG_INVALID_ENDPOINT_GUID(OSAPI_LOGKIND_ERROR)
        return DDS_RETCODE_PRECONDITION_NOT_MET;
    }

    if (DB_Database_lock(self->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    rem_participant = NDDS_RemoteEndpoint_find_parent(participant,
                           &data->participant_key,participant_name,&data->key);

    if (rem_participant == NULL)
    {
        DDSC_LOG_FIND_PUBLICATION_PARENT(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    rem_publication = NULL;
    tmp_key = data->key;

    if (key_kind == NDDS_TYPEPLUGIN_USER_KEY)
    {
        tmp_key.value[3] =
              (tmp_key.value[3] << 8) | RTPS_OBJECT_NORMAL_USER_CST_WRITER;
    }
    else if (key_kind == NDDS_TYPEPLUGIN_NO_KEY)
    {
        tmp_key.value[3] =
              (tmp_key.value[3] << 8) | RTPS_OBJECT_NORMAL_USER_PUBLICATION;
    }

    dbrc = DB_Table_select_match(self->remote_publisher_table,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&rem_publication,
                                 (DB_Key_T)&tmp_key);

    if (dbrc == DB_RETCODE_NO_DATA)
    {
        dbrc = DB_Table_create_record(self->remote_publisher_table,
                                      (DB_Record_T*)&rem_publication);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_RECORD_CREATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLICATION_RECORD,dbrc)
            retcode = DDS_RETCODE_OUT_OF_RESOURCES;
            goto done;
        }
#ifdef RTI_CERT
        /* if all is ok, update the record */
        if (!DDS_PublicationBuiltinTopicData_initialize(&rem_publication->data))
        {
            DDSC_LOG_OBJECT_INITIALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLICATIONDATA_OBJECT)
            (void)DB_Table_delete_record(participant->remote_publisher_table,
                                         (DB_Record_T)rem_publication);
            retcode = DDS_RETCODE_OUT_OF_RESOURCES;
            goto done;
        }
#endif

#ifndef RTI_CERT
        if (!DDS_PublicationBuiltinTopicData_set_from(participant,
                                                  &rem_publication->data,data))
#else
        if (!DDS_PublicationBuiltinTopicData_copy(&rem_publication->data,data))
#endif
        {
            DDSC_LOG_OBJECT_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLICATIONDATA_OBJECT)
#ifndef RTI_CERT
            if (!DDS_PublicationBuiltinTopicData_finalize_no_dealloc(participant,
                                                        &rem_publication->data))
            {
            }
#endif
            (void)DB_Table_delete_record(participant->remote_publisher_table,
                                         (DB_Record_T)rem_publication);
            retcode = DDS_RETCODE_OUT_OF_RESOURCES;
            goto done;
        }

        rem_publication->data.key = tmp_key;
        rem_publication->data.participant_key = rem_participant->data.key;
        rem_publication->as_entity.enable_func = NDDS_RemotePublication_enable;
        rem_publication->as_entity.kind = DDS_PUBLISHER_ENTITY_KIND;
        rem_publication->as_entity.state = RTIDDS_ENTITY_STATE_CREATED;
        rem_publication->orig_key = rem_publication->data.key;

        /* Insert into the table */
        dbrc = DB_Table_insert_record(self->remote_publisher_table,
                                      rem_publication);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_RECORD_INSERT(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLICATION_RECORD,dbrc)
#ifndef RTI_CERT
            if (!DDS_PublicationBuiltinTopicData_finalize_no_dealloc(participant,
                                                        &rem_publication->data))
            {
            }
#endif
            (void)DB_Table_delete_record(participant->remote_publisher_table,
                                         (DB_Record_T)rem_publication);
            goto done;
        }

        if (NDDS_RemoteEntity_is_enabled((NDDS_RemoteEntity *)rem_participant))
        {
            retcode = NDDS_RemotePublication_enable(participant,
                                                  &rem_publication->as_entity,
                                                  &rem_participant->data.key);
            if (retcode != DDS_RETCODE_OK)
            {
                DDSC_LOG_ENTITY_ENABLE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLICATION_ENTITY)
                goto done;
            }
        }
    }
    else if (dbrc == DB_RETCODE_OK)
    {
        if (rem_publication->as_entity.state == RTIDDS_ENTITY_STATE_ENABLED)
        {
            NDDS_RemotePublication_match_local_reader(participant,
                                                      &rem_participant->data,
                                                      &rem_publication->data);
        }
    }
    else
    {
        DDSC_LOG_RECORD_LOOKUP(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLICATION_RECORD)
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:

    if (DB_Database_unlock(self->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}

/*ci
 * \brief Enable a remote publication
 *
 * \details
 * This function enables a remote publication and optionally assigns it a
 * new key (typically used for static discovery). When the remote publication
 * is enabled it is matched with local endpoints.
 *
 * \param[in] self    The participant to enable the remote publication in
 * \param[in] entity  The remote publication to enable
 * \param[in] new_key If not NULL, update the key of the remote publication
 *                    with this key.
 *
 * \return DDS_RETCODE_OK on success,one of the standard error codes on failure
 */
DDS_ReturnCode_t
NDDS_RemotePublication_enable(DDS_DomainParticipant *const self,
                              NDDS_RemoteEntity *entity,
                              const DDS_BuiltinTopicKey_t *new_key)
{
    struct DDS_DomainParticipantImpl *participant =
                        (struct DDS_DomainParticipantImpl *)self;
    struct DDS_RemotePublicationImpl *remote_publication = NULL;
    struct DDS_RemotePublicationImpl *remote_publication2 = NULL;
    struct DDS_RemoteParticipantImpl *remote_participant = NULL;
    DB_ReturnCode_T dbrc;
    DDS_BuiltinTopicKey_t new_pub_key;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION(entity == NULL || self == NULL,
                    return DDS_RETCODE_BAD_PARAMETER,
                   OSAPI_Log_entry_add_pointer("entity",entity,RTI_FALSE);
                   OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    remote_publication = (struct DDS_RemotePublicationImpl *)entity;

    if (DB_Database_lock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    OSAPI_TRACE_DDS("enable remote publication",RTI_TRUE)

    if (remote_publication->as_entity.state == RTIDDS_ENTITY_STATE_ENABLED)
    {
        retcode = DDS_RETCODE_OK;
        goto done;
    }

    remote_participant = NULL;
    if (new_key == NULL)
    {
        dbrc = DB_Table_select_match(participant->remote_participant_table,
                    DB_TABLE_DEFAULT_INDEX,
                    (DB_Record_T*)&remote_participant,
                    (DB_Key_T)&remote_publication->data.participant_key);
    }
    else
    {
        dbrc = DB_Table_select_match(participant->remote_participant_table,
                                    DB_TABLE_DEFAULT_INDEX,
                                    (DB_Record_T*)&remote_participant,
                                    (DB_Key_T)new_key);
    }

    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_LOOKUP(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_RECORD)
        goto done;
    }

    if (!NDDS_RemoteEntity_is_enabled(&remote_participant->as_entity))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_ENTITY)
        retcode = DDS_RETCODE_NOT_ENABLED;
        goto done;
    }

    /* We allow changing the host and app id part of the GUID, e.g for static
     * discovery.
     */
    if ((new_key != NULL) &&
        !DDS_BuiltinTopicKey_prefix_equals(new_key,
                                           &DDS_BUILTINTOPICKEY_UNKNOWN))
    {

        /* If the GUID is specified, it matches that of the record, otherwise 
         * it is an inconsistency
         */
        if (!DDS_BuiltinTopicKey_prefix_equals(&remote_participant->data.key,
                                               &DDS_BUILTINTOPICKEY_UNKNOWN) &&
            !DDS_BuiltinTopicKey_equals(&remote_participant->data.key, new_key))
        {
            DDSC_LOG_REMOTE_PARTICIPANT_KEY_NOT_EQUAL(OSAPI_LOGKIND_ERROR,
                            remote_participant->data.participant_name.name)
            retcode = DDS_RETCODE_PRECONDITION_NOT_MET;
            goto done;
        }

        /* new_key is parent key, can only update the GUID prefix */
        new_pub_key = *new_key;
        DDS_BuiltinTopicKey_copy_suffix(&new_pub_key,
                (DDS_BuiltinTopicKey_t*)&remote_publication->data.key.value);

        dbrc = DB_Table_remove_record(participant->remote_publisher_table,
                                     (DB_Record_T*)&remote_publication2,
                                     (DB_Key_T)&remote_publication->data.key);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_RECORD_REMOVE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLICATION_RECORD,dbrc)
            goto done;
        }

        remote_publication2->data.participant_key = *new_key;
        remote_publication2->data.key = new_pub_key;

        dbrc = DB_Table_insert_record(participant->remote_publisher_table,
                                     (DB_Record_T)remote_publication2);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_RECORD_INSERT(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLICATION_RECORD,dbrc)
            goto done;
        }
    }

    remote_publication->as_entity.state = RTIDDS_ENTITY_STATE_ENABLED;

    NDDS_RemotePublication_match_local_reader(participant,
                                              &remote_participant->data,
                                              &remote_publication->data);

    OSAPI_TRACE_DDS("successfully enabled remote publication",RTI_TRUE)

    retcode = DDS_RETCODE_OK;

done:

    if (DB_Database_unlock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}

/*ci
 * \brief Reset/Remove a remote publication from a participant
 *
 * \details
 * This function removes or resets a remote publication from the participant.
 * When the remote publication is reset/removed it is also unmatched from
 * local datareaders. However, the remote publication is not removed from the
 * participant if a reset is performed. This feature is typically used in
 * static discovery where remote participants and endpoints are statically
 * asserted. After they have been reset they can be
 * re-enabled with \ref NDDS_RemotePublication_enable.
 *
 * \param[in] self        The participant to remove the remote publication from
 * \param[in] key         The key of the remote publication to remove
 * \param[in] reset_entry Whether the remote publication should be reset or
 *                        removed.
 *
 * \return DDS_RETCODE_OK on success,one of the standard error codes on failure
 */
DDS_ReturnCode_t
NDDS_RemotePublication_remove_internal(DDS_DomainParticipant *const self,
                                       const DDS_BuiltinTopicKey_t *const key,
                                       DDS_Boolean reset_entry)
{
    struct DDS_DomainParticipantImpl *participant =
                        (struct DDS_DomainParticipantImpl *)self;
    struct DDS_RemotePublicationImpl *rem_publication = NULL;
    struct DDS_RemotePublicationImpl *rem_publication2 = NULL;
    DB_ReturnCode_T dbrc;
    struct DDS_RemoteParticipantImpl *remote_participant = NULL;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
#ifdef RTI_CERT
    UNUSED_ARG(reset_entry);
#endif /* !RTI_CERT */

    OSAPI_PRECONDITION(self == NULL || key == NULL,
                    return DDS_RETCODE_BAD_PARAMETER,
                    OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("key",key,RTI_TRUE);)

    OSAPI_TRACE_DDS("remove remote publication",RTI_TRUE)

    if (DB_Database_lock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    /* reset_entry is always TRUE for RTI_CERT */
#ifndef RTI_CERT
    if (reset_entry)
    {
#endif /* !RTI_CERT */
        dbrc = DB_Table_select_match(participant->remote_publisher_table,
                                     DB_TABLE_DEFAULT_INDEX,
                                     (DB_Record_T*)&rem_publication,
                                     (DB_Key_T) key);
#ifndef RTI_CERT
    }
    else
    {
        dbrc = DB_Table_remove_record(participant->remote_publisher_table,
                                      (DB_Record_T*)&rem_publication,
                                      (DB_Key_T) key);
    }
#endif /* !RTI_CERT */

    if (dbrc != DB_RETCODE_OK)
    {
        /* This could happen if we get a dispose for something that has not
         * been discovered, this is a valid case
         */
        if (dbrc == DB_RETCODE_NO_DATA)
        {
            retcode = DDS_RETCODE_OK;
        }
#if OSAPI_ENABLE_LOG
        else
        {
            DDSC_LOG_RECORD_REMOVE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLICATION_RECORD,dbrc)
        }
#endif
        goto done;
    }

    remote_participant = NULL;
    dbrc = DB_Table_select_match(participant->remote_participant_table,
                              DB_TABLE_DEFAULT_INDEX,
                              (DB_Record_T*)&remote_participant,
                              (DB_Key_T)&rem_publication->data.participant_key);

    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_LOOKUP(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_RECORD)
        goto done;
    }

    NDDS_RemotePublication_unmatch_local_reader(participant,
                                                &remote_participant->data,
                                                &rem_publication->data);

    /* reset_entry is always TRUE for RTI_CERT */
#ifndef RTI_CERT
    if (reset_entry)
    {
#endif /* !RTI_CERT */
        rem_publication->as_entity.state = RTIDDS_ENTITY_STATE_CREATED;
        if (!DDS_BuiltinTopicKey_prefix_equals(&rem_publication->data.key,
                &DDS_BUILTINTOPICKEY_PREFIX_UNKNOWN))
        {
            dbrc = DB_Table_remove_record(participant->remote_publisher_table,
                    (DB_Record_T*)&rem_publication2,
                    (DB_Key_T)&rem_publication->data.key);
            if (dbrc != DB_RETCODE_OK)
            {
                DDSC_LOG_RECORD_REMOVE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLICATION_RECORD,dbrc)
                goto done;
            }
            rem_publication2->data.key = rem_publication->orig_key;
            rem_publication2->data.participant_key = remote_participant->orig_key;

            dbrc = DB_Table_insert_record(participant->remote_publisher_table,
                                          (DB_Record_T)rem_publication2);
            if (dbrc != DB_RETCODE_OK)
            {
                DDSC_LOG_RECORD_INSERT(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLICATION_RECORD,dbrc)
                goto done;
            }
        }
#ifndef RTI_CERT
    }
    else
    {
#ifndef RTI_CERT
        if (!DDS_PublicationBuiltinTopicData_finalize_no_dealloc(participant,
                                                        &rem_publication->data))
#else
        if (!DDS_PublicationBuiltinTopicData_finalize(&rem_publication->data))
#endif
        {
            DDSC_LOG_OBJECT_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLICATIONDATA_OBJECT)
            retcode = DDS_RETCODE_PRECONDITION_NOT_MET;
            goto done;
        }
        dbrc = DB_Table_delete_record(participant->remote_publisher_table,
                                     (DB_Record_T)rem_publication);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_RECORD_DELETE(OSAPI_LOGKIND_ERROR,DDSC_LOG_PUBLICATION_RECORD,dbrc)
            goto done;
        }

        OSAPI_TRACE_DDS("deleted remote publication:",RTI_FALSE)
        OSAPI_TRACE_GUID("key",&rem_publication->data.key,RTI_TRUE)

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
NDDS_DomainParticipant_remove_remote_publication(DDS_DomainParticipant *
                                                 const participant,
                                                 const DDS_BuiltinTopicKey_t *
                                                 const key)
{
    return NDDS_RemotePublication_remove_internal(participant, key,
                                                  DDS_BOOLEAN_FALSE);
}
#endif /* !RTI_CERT */

/*ci @} */
