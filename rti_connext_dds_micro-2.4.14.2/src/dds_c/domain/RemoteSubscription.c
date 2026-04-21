/*
 * FILE: RemoteSubscription.c - Remote subscription implementation
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
 * 20feb2015,eh  MICRO-813/PR#9172 Fix Lint warnings
 * 10dec2014,tk MICRO-978/PR#12914 Consistently return records on failure
 * 31jul2014,tk MICRO-842/PR#9683 - Removed superfluous paramaters in DB
 *                                  compare function
 * 04mar2014,tk MICRO-211: Unique object id across systems not needed
 * 17may2012,tk Updated
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief Remote subscription implementation
 *
 * \details
 * This file implements functions to manage remote subscriptions, such
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
#include "RemoteSubscription.h"

#include "DataWriterDiscovery.h"

/*ci \brief OMG defined name for the SubscriptionBuiltinTopicData Type
 */
const char * const DDS_SUBSCRIPTION_BUILTIN_TOPIC_TYPE_NAME = 
                                      "DDS_SubscriptionBuiltinTopicData";

/*ci \brief OMG defined name for the SubscriptionBuiltinTopicData Type
 */                                        
const char * const DDS_SUBSCRIPTION_BUILTIN_TOPIC_NAME = "DCPSSubscription";

/*
 *                                 Internal API
 */
MUST_CHECK_RETURN RTI_PRIVATE DDS_ReturnCode_t
NDDS_RemoteSubscription_enable(DDS_DomainParticipant *const participant,
                               NDDS_RemoteEntity *entity,
                               const DDS_BuiltinTopicKey_t *key);

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Compare entries in the table of remote subscriptions. The function is
 *        compatible with \ref DB_IndexCompare_T
 *
 * \param[in] flags Used to distinguish between op2 being a key or record
 * \param[in] op1   A DDS_RemoteSubscriptionImpl already in the database
 * \param[in] op2   Either a DDS_RemoteSubscriptionImpl being added or a key
 *                  being searched for
 *
 * \return positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2,
 *         zero if op1 is equal to op2
 */
MUST_CHECK_RETURN RTI_INT32
DDS_RemoteSubscriptionImpl_compare(RTI_INT32 flags,const DB_Record_T op1, void *op2)
{
    struct DDS_RemoteSubscriptionImpl *record_left =
                                        (struct DDS_RemoteSubscriptionImpl*)op1;
    const DDS_BuiltinTopicKey_t *key_right;

    if (DB_SELECT_OP2_IS_KEY(flags))
    {
        key_right = (const DDS_BuiltinTopicKey_t*)op2;
    }
    else
    {
        key_right = &((struct DDS_RemoteSubscriptionImpl*)op2)->data.key;
    }

    return DDS_BuiltinTopicKey_compare(&record_left->data.key,key_right);
}

/*ci
 * \brief Perform matching with a specific local datawriter against all
 *        discovered subscriptions
 *
 * \details
 *
 * When a local writer is created/modified it is matched against all discovered
 * subscriptions.
 *
 * \param[in] self         The participant to perform matching in
 * \param[in] local_writer The local writer to match with
 */
void
NDDS_RemoteSubscription_match_with_local_writer(DDS_DomainParticipant *const self,
                                                struct DDS_DataWriterImpl *local_writer)
{
    struct DDS_DomainParticipantImpl *participant =
                                    (struct DDS_DomainParticipantImpl *)self;
    struct DDS_RemoteSubscriptionImpl *rem_sub = NULL;
    struct DDS_RemoteParticipantImpl *rem_dp = NULL;
    DB_Cursor_T remsub_cursor = NULL;
    DB_ReturnCode_T dbrc;

    dbrc = DB_Table_select_all(participant->remote_subscriber_table,
                               DB_TABLE_DEFAULT_INDEX,&remsub_cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,participant->remote_subscriber_table,dbrc)
        return;
    }

    do
    {
        dbrc = DB_Cursor_get_next(remsub_cursor,(DB_Record_T*)&rem_sub);
        if ((dbrc == DB_RETCODE_OK) &&
            (rem_sub->as_entity.state == RTIDDS_ENTITY_STATE_ENABLED))
        {
            dbrc = DB_Table_select_match(participant->remote_participant_table,
                                     DB_TABLE_DEFAULT_INDEX,
                                     (DB_Record_T*)&rem_dp,
                                     (DB_Key_T)&rem_sub->data.participant_key);
            if (dbrc != DB_RETCODE_OK)
            {
                goto done;
            }

            DDS_DataWriter_match_remote_reader(local_writer,
                                               &rem_dp->data,
                                               &rem_sub->data);
        }
    } while (dbrc == DB_RETCODE_OK);

done:

    DB_Cursor_finish(participant->remote_subscriber_table,remsub_cursor);

#if OSAPI_ENABLE_LOG
    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
    }
#endif

}

/*ci
 * \brief Perform matching with all datawriters in a participant
 *
 * \details
 *
 * When a remote subscription is detected/modified it is matched against
 * all locally created datawriters. The participant data is used for
 * default values, such as which locators to use.
 *
 * \param[in] self        The participant
 * \param[in] parent_data The remote participant containing the subscription
 * \param[in] data        The remote subscription to match against
 */
RTI_PRIVATE void
NDDS_RemoteSubscription_match_local_writer(DDS_DomainParticipant *const self,
        const struct DDS_ParticipantBuiltinTopicData *const parent_data,
        const struct DDS_SubscriptionBuiltinTopicData *const data)
{
    struct DDS_DomainParticipantImpl *participant =
                                    (struct DDS_DomainParticipantImpl *)self;
    struct DDS_DataWriterImpl *local_writer;
    DB_Cursor_T dw_cursor = NULL;
    DB_ReturnCode_T dbrc;

    dbrc = DB_Table_select_all(participant->local_writer_table,
                               DB_TABLE_DEFAULT_INDEX,&dw_cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,participant->local_writer_table,dbrc)
        return;
    }

    do
    {
        dbrc = DB_Cursor_get_next(dw_cursor,(DB_Record_T*)&local_writer);
        if (dbrc == DB_RETCODE_OK)
        {
            if (DDS_Entity_is_enabled(DDS_DataWriter_as_entity(local_writer)))
            {
                DDS_DataWriter_match_remote_reader(local_writer,parent_data,data);
            }
        }
    } while (dbrc == DB_RETCODE_OK);
    DB_Cursor_finish(participant->local_writer_table,dw_cursor);

#if OSAPI_ENABLE_LOG
    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
    }
#endif
}

/*ci
 * \brief Unmatch all local writers from a specific remote subscription
 *
 * \details
 *
 * When a remote subscription is removed it is unmatched from
 * all locally created datawriters.
 *
 * \param[in] self        The participant
 * \param[in] parent_data The remote participant containing the subscription
 * \param[in] data        The remote subscription to unmatch from
 */
RTI_PRIVATE void
NDDS_RemoteSubscription_unmatch_local_writer(DDS_DomainParticipant *const self,
        const struct DDS_ParticipantBuiltinTopicData *const parent_data,
        const struct DDS_SubscriptionBuiltinTopicData *const data)
{
    struct DDS_DomainParticipantImpl *participant =
                                    (struct DDS_DomainParticipantImpl *)self;
    struct DDS_DataWriterImpl *local_writer;
    DB_Cursor_T dw_cursor = NULL;
    DB_ReturnCode_T dbrc;

    dbrc = DB_Table_select_all(participant->local_writer_table,
                               DB_TABLE_DEFAULT_INDEX,&dw_cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,participant->local_writer_table,dbrc)
        return;
    }

    do
    {
        dbrc = DB_Cursor_get_next(dw_cursor,(DB_Record_T*)&local_writer);
        if (dbrc == DB_RETCODE_OK)
        {
            DDS_DataWriter_unmatch_remote_reader(local_writer,
                                                 parent_data,data);
        }
    } while (dbrc == DB_RETCODE_OK);

    DB_Cursor_finish(participant->local_writer_table,dw_cursor);

#if OSAPI_ENABLE_LOG
    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
    }
#endif
}

#ifndef RTI_CERT
/*ci
 * \brief Unmatch a local writer from a remote subscription
 *
 * \param[in] participant  The participant
 * \param[in] dr_key       The key of the remote subscription to unmatch
 * \param[in] local_writer The local writer to unmatch
 */
void
NDDS_RemoteSubscription_unmatch_local_writer_from_key(
            DDS_DomainParticipant *const participant,
            DDS_BuiltinTopicKey_t *dr_key,
            struct DDS_DataWriterImpl *local_writer)
{
    struct DDS_RemoteSubscriptionImpl *rem_sub = NULL;
    struct DDS_RemoteParticipantImpl *rem_dp = NULL;
    DB_ReturnCode_T dbrc;

    dbrc = DB_Table_select_match(participant->remote_subscriber_table,
                                DB_TABLE_DEFAULT_INDEX,
                                (DB_Record_T*)&rem_sub,
                                (DB_Key_T)dr_key);
    if (dbrc != DB_RETCODE_OK)
    {
        return;
    }

    dbrc = DB_Table_select_match(participant->remote_participant_table,
                                DB_TABLE_DEFAULT_INDEX,
                                (DB_Record_T*)&rem_dp,
                                (DB_Key_T)&rem_sub->data.participant_key);
    if (dbrc != DB_RETCODE_OK)
    {
        return;
    }

    DDS_DataWriter_unmatch_remote_reader(local_writer,
                                          &rem_dp->data,&rem_sub->data);
}
#endif /* !RTI_CERT */

/*
 *                                 Public API
 */
DDS_ReturnCode_t
NDDS_DomainParticipant_assert_remote_subscription(
                    DDS_DomainParticipant *const participant,
                    const char *const participant_name,
                    const struct DDS_SubscriptionBuiltinTopicData *const data,
                    NDDS_TypePluginKeyKind key_kind)
{
    struct DDS_DomainParticipantImpl *self =
                        (struct DDS_DomainParticipantImpl *)participant;
    struct DDS_RemoteSubscriptionImpl *rem_subscription = NULL;
    DB_ReturnCode_T dbrc;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    struct DDS_RemoteParticipantImpl *rem_participant = NULL;
    struct DDS_BuiltinTopicKey_t tmp_key;

    OSAPI_PRECONDITION((participant == NULL) || (data == NULL),
                            return DDS_RETCODE_BAD_PARAMETER,
                            OSAPI_Log_entry_add_pointer("participant",participant,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("data",data,RTI_TRUE);)

    OSAPI_TRACE_DDS("assert subscription:",RTI_FALSE)
    OSAPI_TRACE_GUID("key",&data->key,RTI_FALSE)
    OSAPI_TRACE_GUID("parent",&data->participant_key,RTI_TRUE)

    /* Note that in RTI DDS, Subscriptions (Publisher + DataWriter) are 
     * published. This makes the data-model a little cluttered. We could
     * keep the data for a publication in one table, at the expense of
     * duplicating data for the parent.
     */

    /* NOTE: We allow participant_name to be NULL. This means the data
     * contains the GUID of the parent participant. If name != NULL, it means
     * we explicitly specify the parent name.
     */
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
        DDSC_LOG_FIND_SUBSCRIPTION_PARENT(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    rem_subscription = NULL;
    tmp_key = data->key;

    if (key_kind == NDDS_TYPEPLUGIN_USER_KEY)
    {
        tmp_key.value[3] =
              (tmp_key.value[3] << 8) | RTPS_OBJECT_NORMAL_USER_CST_READER;
    }
    else if (key_kind == NDDS_TYPEPLUGIN_NO_KEY)
    {
        tmp_key.value[3] =
              (tmp_key.value[3] << 8) | RTPS_OBJECT_NORMAL_USER_SUBSCRIPTION;
    }

    dbrc = DB_Table_select_match(self->remote_subscriber_table,
                        DB_TABLE_DEFAULT_INDEX,(DB_Record_T*)&rem_subscription,
                        (DB_Key_T)&tmp_key);

    if (dbrc == DB_RETCODE_NO_DATA)
    {
        dbrc = DB_Table_create_record(self->remote_subscriber_table,
                                     (DB_Record_T*)&rem_subscription);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_RECORD_CREATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIPTIONDATA_OBJECT,dbrc)
            retcode = DDS_RETCODE_OUT_OF_RESOURCES;
            goto done;
        }

#ifdef RTI_CERT
        if (!DDS_SubscriptionBuiltinTopicData_initialize(&rem_subscription->data))
        {
            DDSC_LOG_OBJECT_INITIALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIPTIONDATA_OBJECT)
            (void)DB_Table_delete_record(participant->remote_subscriber_table,
                                         (DB_Record_T)rem_subscription);
            retcode = DDS_RETCODE_OUT_OF_RESOURCES;
            goto done;
        }
#endif

#ifndef RTI_CERT
        if (!DDS_SubscriptionBuiltinTopicData_set_from(participant,
                                                   &rem_subscription->data,data))
#else
        if (!DDS_SubscriptionBuiltinTopicData_copy(&rem_subscription->data,data))
#endif
        {
            DDSC_LOG_OBJECT_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIPTIONDATA_OBJECT)
#ifndef RTI_CERT
            if (!DDS_SubscriptionBuiltinTopicData_finalize_no_dealloc(participant,            
                                                        &rem_subscription->data))
            {
            }
#endif
            (void)DB_Table_delete_record(participant->remote_subscriber_table,
                                         (DB_Record_T)rem_subscription);
            retcode = DDS_RETCODE_OUT_OF_RESOURCES;
            goto done;
        }

        /* Update the participant_key field as it may have changed from the 
         * based on the input parameters
         */
        rem_subscription->data.key = tmp_key;
        rem_subscription->data.participant_key = rem_participant->data.key;
        rem_subscription->as_entity.enable_func = NDDS_RemoteSubscription_enable;
        rem_subscription->as_entity.kind = DDS_SUBSCRIBER_ENTITY_KIND;
        rem_subscription->as_entity.state = RTIDDS_ENTITY_STATE_CREATED;
        rem_subscription->orig_key = rem_subscription->data.key;

        /* Insert into the table */
        dbrc = DB_Table_insert_record(self->remote_subscriber_table,
                                      rem_subscription);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_RECORD_INSERT(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIPTION_RECORD,dbrc)
#ifndef RTI_CERT
            if (!DDS_SubscriptionBuiltinTopicData_finalize_no_dealloc(participant,            
                                                        &rem_subscription->data))
            {
            }
#endif
            (void)DB_Table_delete_record(participant->remote_subscriber_table,
                                         (DB_Record_T)rem_subscription);
            goto done;
        }

        if (NDDS_RemoteEntity_is_enabled((NDDS_RemoteEntity *)rem_participant))
        {
            retcode = NDDS_RemoteSubscription_enable(participant,
                                                &rem_subscription->as_entity,
                                                &rem_participant->data.key);
            if (retcode != DDS_RETCODE_OK)
            {
                goto done;
            }
        }
    }
    else if (dbrc == DB_RETCODE_OK)
    {
        if (rem_subscription->as_entity.state == RTIDDS_ENTITY_STATE_ENABLED)
        {
            NDDS_RemoteSubscription_match_local_writer(participant,
                                                       &rem_participant->data,
                                                       &rem_subscription->data);
        }
    }
    else
    {
        DDSC_LOG_RECORD_LOOKUP(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIPTION_RECORD)
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
 * \brief Enable a remote subscription
 *
 * \details
 * This function enables a remote subscription and optionally assigns it a
 * new key (typically used for static discovery). When the remote subscription
 * is enabled it is matched with local endpoints.
 *
 * \param[in] self    The participant to enable the remote subscription in
 * \param[in] entity  The remote subscription to enable
 * \param[in] new_key If not NULL, update the key of the remote subscription
 *                    with this key.
 *
 * \return DDS_RETCODE_OK on success,one of the standard error codes on failure
 */
DDS_ReturnCode_t
NDDS_RemoteSubscription_enable(DDS_DomainParticipant *const self,
                               NDDS_RemoteEntity *entity,
                               const DDS_BuiltinTopicKey_t *new_key)
{
    struct DDS_DomainParticipantImpl *participant =
                        (struct DDS_DomainParticipantImpl *)self;
    struct DDS_RemoteSubscriptionImpl *rem_subscription = NULL;
    struct DDS_RemoteSubscriptionImpl *rem_subscription2 = NULL;
    struct DDS_RemoteParticipantImpl *remote_participant = NULL;
    DB_ReturnCode_T dbrc;
    DDS_BuiltinTopicKey_t new_sub_key;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION(entity == NULL || self == NULL,
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("entity",entity,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    rem_subscription = (struct DDS_RemoteSubscriptionImpl *)entity;

    if (DB_Database_lock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (rem_subscription->as_entity.state == RTIDDS_ENTITY_STATE_ENABLED)
    {
        if (DB_Database_unlock(participant->database) != DB_RETCODE_OK)
        {
            return DDS_RETCODE_ERROR;
        }
        return DDS_RETCODE_OK;
    }

    remote_participant = NULL;
    if (new_key == NULL)
    {
        dbrc = DB_Table_select_match(participant->remote_participant_table,
                    DB_TABLE_DEFAULT_INDEX,
                    (DB_Record_T*)&remote_participant,
                    (DB_Key_T)&rem_subscription->data.participant_key);
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
        new_sub_key = *new_key;
        DDS_BuiltinTopicKey_copy_suffix(&new_sub_key,
                                        &rem_subscription->data.key);

        dbrc = DB_Table_remove_record(participant->remote_subscriber_table,
                                     (DB_Record_T*)&rem_subscription2,
                                     (DB_Key_T)&rem_subscription->data.key);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_RECORD_REMOVE(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIPTION_RECORD,dbrc)
            goto done;
        }

        rem_subscription2->data.participant_key = *new_key;
        rem_subscription2->data.key = new_sub_key;

        dbrc = DB_Table_insert_record(participant->remote_subscriber_table,
                                     (DB_Record_T)rem_subscription2);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_RECORD_INSERT(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIPTION_RECORD,dbrc)
            goto done;
        }
    }

    OSAPI_TRACE_DDS("enable remote subscription",RTI_TRUE)

    rem_subscription->as_entity.state = RTIDDS_ENTITY_STATE_ENABLED;

    NDDS_RemoteSubscription_match_local_writer(participant,
                                               &remote_participant->data,
                                               &rem_subscription->data);

    retcode = DDS_RETCODE_OK;

done:
    if (DB_Database_unlock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}

/*ci
 * \brief Reset/Remove a remote subscription from a participant
 *
 * \details
 * This function removes or resets a remote subscription from the participant.
 * When the remote subscription is reset/removed it is also unmatched from
 * local datawriters. However, the remote subscription is not removed from the
 * participant if a reset is performed. This feature is typically used in
 * static discovery where remote participants and endpoints are statically
 * asserted. After they have been reset they can be
 * re-enabled with \ref NDDS_RemoteSubscription_enable.
 *
 * \param[in] participant The participant to remove the remote subscription from
 * \param[in] key         The key of the remote subscription to remove
 * \param[in] reset       Whether the remote subscription should be reset or
 *                        removed.
 *
 * \return DDS_RETCODE_OK on success,one of the standard error codes on failure
 */
DDS_ReturnCode_t
NDDS_RemoteSubscription_remove_internal(DDS_DomainParticipant *const self,
                                        const DDS_BuiltinTopicKey_t *key,
                                        DDS_Boolean reset_entry)
{
    struct DDS_DomainParticipantImpl *participant =
                                    (struct DDS_DomainParticipantImpl *)self;
    struct DDS_RemoteSubscriptionImpl *rem_subscription = NULL;
    struct DDS_RemoteSubscriptionImpl *rem_subscription2 = NULL;
    DB_ReturnCode_T dbrc;
    struct DDS_RemoteParticipantImpl *remote_participant = NULL;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
#ifdef RTI_CERT
    UNUSED_ARG(reset_entry);
#endif

    OSAPI_PRECONDITION(participant == NULL || key == NULL,
            return DDS_RETCODE_BAD_PARAMETER,
            OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("key",key,RTI_TRUE);)

    OSAPI_TRACE_DDS("remove remote subscription",RTI_TRUE)

    if (DB_Database_lock(participant->database) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

#ifndef RTI_CERT
    if (reset_entry)
    {
#endif /* !RTI_CERT */
        dbrc = DB_Table_select_match(participant->remote_subscriber_table,
                                    DB_TABLE_DEFAULT_INDEX,
                                    (DB_Record_T*)&rem_subscription,
                                    (DB_Key_T)key);
#ifndef RTI_CERT
    }
    else
    {
        dbrc = DB_Table_remove_record(participant->remote_subscriber_table,
                                    (DB_Record_T*)&rem_subscription,
                                     (DB_Key_T)key);
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
            DDSC_LOG_RECORD_REMOVE(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIPTION_RECORD,dbrc)
        }
#endif
        goto done;
    }

    remote_participant = NULL;
    dbrc = DB_Table_select_match(participant->remote_participant_table,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T*)&remote_participant,
                                 (DB_Key_T)&rem_subscription->data.participant_key);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_RECORD_LOOKUP(OSAPI_LOGKIND_ERROR,DDSC_LOG_PARTICIPANT_RECORD)
        goto done;
    }

    NDDS_RemoteSubscription_unmatch_local_writer(participant,
                                                 &remote_participant->data,
                                                 &rem_subscription->data);

    /* reset_entry is always TRUE for RTI_CERT */
#ifndef RTI_CERT
    if (reset_entry)
    {
#endif /* !RTI_CERT */
        rem_subscription->as_entity.state = RTIDDS_ENTITY_STATE_CREATED;
        if (!DDS_BuiltinTopicKey_prefix_equals(&rem_subscription->data.key,
                &DDS_BUILTINTOPICKEY_PREFIX_UNKNOWN))
        {
            dbrc = DB_Table_remove_record(participant->remote_subscriber_table,
                                          (DB_Record_T*)&rem_subscription2,
                                          (DB_Key_T)&rem_subscription->data.key);
            if (dbrc != DB_RETCODE_OK)
            {
                DDSC_LOG_RECORD_REMOVE(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIPTION_RECORD,dbrc)
                goto done;
            }

            rem_subscription2->data.key = rem_subscription->orig_key;
            rem_subscription2->data.participant_key = remote_participant->orig_key;

            dbrc = DB_Table_insert_record(participant->remote_subscriber_table,
                                          (DB_Record_T)rem_subscription2);
            if (dbrc != DB_RETCODE_OK)
            {
                DDSC_LOG_RECORD_INSERT(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIPTION_RECORD,dbrc)
                goto done;
            }
        }
#ifndef RTI_CERT
    }
    else
    {
#ifndef RTI_CERT
        if (!DDS_SubscriptionBuiltinTopicData_finalize_no_dealloc(participant,
                                                       &rem_subscription->data))
#else
        if (!DDS_SubscriptionBuiltinTopicData_finalize(&rem_subscription->data))
#endif
        {
            DDSC_LOG_OBJECT_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIPTIONDATA_OBJECT)
            retcode = DDS_RETCODE_PRECONDITION_NOT_MET;
            goto done;
        }

        dbrc = DB_Table_delete_record(participant->remote_subscriber_table,
                                     (DB_Record_T)rem_subscription);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_RECORD_DELETE(OSAPI_LOGKIND_ERROR,DDSC_LOG_SUBSCRIPTION_RECORD,dbrc)
            goto done;
        }

        OSAPI_TRACE_DDS("deleted remote subscription",RTI_FALSE)
        OSAPI_TRACE_GUID("key",&rem_subscription->data.key,RTI_FALSE)
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
NDDS_DomainParticipant_remove_remote_subscription(
                                    DDS_DomainParticipant *const participant,
                                    const DDS_BuiltinTopicKey_t *const key)
{
    return NDDS_RemoteSubscription_remove_internal(participant, key,
                                                   DDS_BOOLEAN_FALSE);
}
#endif /* !RTI_CERT */

/*ci @} */
