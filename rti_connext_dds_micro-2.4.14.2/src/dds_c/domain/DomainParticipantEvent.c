/*
 * FILE: DomainParticipantEvent.c - DomainParticipant event implementation
 *
 * (c) Copyright, Real-Time Innovations, 2008-2022.
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
 * 08sep2022,tk MICRO-3367/PR.30121
 * - NDDS_DomainParticipant_on_data_available:
 *   - Removed UNUSED_ARG(subscriber) since it is used.
 * 16apr2021,tk MICRO-2845/PR.28682
 *  - Removed empty statements (LOG statements ending in ;)
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 * 12mar2015,tk MICRO-1079/PR#14031 Removed redundant code
 *                                  Added return to enable() function
 * 16sep2014,as MICRO-903/PR#11236  Incorrect handling of status events and listeners
 * 19mar2014,tk MICRO-74: Support endpoint specific transport
 * 08nov2013,as  MICRO-681 Complete implementation of WaitSets
 *               and support for StatusConditions
 * 01aug2013,tk  MICRO-677: on_liveliness_callback on remote deletion
 * 31may2013,eh  Fixed MICRO-380: liveliness_changed mask
 * 27jun2012,tk  Major update
 * 30apr2008,tk  Written
 */
/*ci
 * \file
 * \brief DomainParticipant event implementation
 *
 * \details
 * This file implements functionality to manage various events related to a
 * DDS participant, such as handling propagated listener events from contained
 * entities and handle discovery events related to creation and deletion of
 * local entities.
 */
/*ci \addtogroup DDSDomainModule
 * @{
 */
#ifndef netio_address_h
#include "netio/netio_address.h"
#endif
#ifndef netio_route_h
#include "netio/netio_route.h"
#endif
#include "netio/netio_loopback.h"
#ifndef reda_bufferpool_h
#include "reda/reda_bufferpool.h"
#endif
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
#include "Conditions.h"
#include "QosPolicy.h"
#include "TopicDescription.h"
#include "Topic.h"
#include "Type.h"
#include "DataReaderImpl.h"
#include "DataReaderDiscovery.h"
#include "DataReaderInterface.h"
#include "SubscriberQos.h"
#include "SubscriberImpl.h"
#include "DataWriterImpl.h"
#include "DataWriterDiscovery.h"
#include "DataWriterInterface.h"
#include "PublisherQos.h"
#include "PublisherImpl.h"
#include "DomainParticipantQos.h"
#include "DomainParticipant.h"
#include "RemoteEntity.h"
#include "RemoteEndpoint.h"
#include "RemoteParticipant.h"
#include "RemotePublication.h"
#include "RemoteSubscription.h"

#include "DomainParticipantEvent.h"

/*** SOURCE_BEGIN ***/

/*******************************************************************************
 *                            Common Functions
 ******************************************************************************/
#ifndef RTI_CERT
/*ci
 * \brief Add a new user traffic route to the participant bind resolver for the
 * local writer passed as parameter
 *
 * \param[in] intf         INTRA transport netio interface
 * \param[in] id           The name of the component to assert routes for
 * \param[in] participant  The DomainParticipant to assert the routes for
 * \param[in] local_writer The DataWriter to assert routes for
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
DomainParticipantEvent_assert_datawriter_in_intra(
    NETIO_Interface_T *intf,
    RT_ComponentFactoryId_T *id,
    DDS_DomainParticipant *participant,
    struct DDS_DataWriterImpl *local_writer)
{
    struct NETIO_Address dw_address;
    DDS_InstanceHandle_t ih_dw;
    struct DDS_BuiltinTopicKey_t dw_key;

    NETIO_Address_init(&dw_address,0);

    ih_dw = DDS_Entity_get_instance_handle(
                            DDS_DataWriter_as_entity(local_writer));
    DDS_BuiltinTopicKey_from_guid(&dw_key,&ih_dw);

    NETIO_Address_set_guid_from_key(&dw_address,0,
                                    (struct NETIO_AddressInt32*)&dw_key);

    if (!NETIO_BindResolver_add_route(participant->bind_resolver,
                                      NETIO_ROUTEKIND_USER, id,
                                      &dw_address, intf, NULL))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief Remove user traffic route from the participant bind resolver for the
 * local writer passed as parameter
 *
 * \param[in] id           The name of the component to remove routes from
 * \param[in] participant  The DomainParticipant to remove the routes from
 * \param[in] local_writer The DataWriter to remove routes from
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
DomainParticipantEvent_remove_datawriter_from_intra(
    RT_ComponentFactoryId_T *id,
    DDS_DomainParticipant *participant,
    struct DDS_DataWriterImpl *local_writer)
{
    struct NETIO_Address dw_address;
    DDS_InstanceHandle_t ih_dw;
    struct DDS_BuiltinTopicKey_t dw_key;

    NETIO_Address_init(&dw_address, 0);

    ih_dw = DDS_Entity_get_instance_handle(
                    DDS_DataWriter_as_entity(local_writer));
    DDS_BuiltinTopicKey_from_guid(&dw_key, &ih_dw);

    NETIO_Address_set_guid_from_key(&dw_address, 0,
                                    (struct NETIO_AddressInt32*) &dw_key);

    if (!NETIO_BindResolver_delete_route(participant->bind_resolver,
                                         NETIO_ROUTEKIND_USER, id,
                                         &dw_address, NULL))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}
#endif

/*******************************************************************************
 *                            Entity Listeners
 ******************************************************************************/

/*ci
 * \brief Notification handler for when a local DDS DataReader is being created
 *
 * \details
 *
 * This function is called before a DDS DataReader is created, but after a
 * key has been assigned. The purpose of this function is to give other
 * parts of the system the ability to determine if the creation is ok, such
 * as checking for resources. This function must return DDS_RETCODE_OK if the
 * DDS DataReader can be created or one of the standard error code if
 * for some reason the DDS DataReader creation cannot proceed. It is up to the
 * caller to decide what to do next.
 *
 * \param[in] subscriber  The DDS Subscriber creating the datareader
 * \param[in] dr_key      The assigned key for the DDS datareader
 * \param[in] reservation DDS_BOOLEAN_TRUE if this is a reservation,
 *                        DDS_BOOLEAN_FALSE if this is releasing a
 *                        reservation.
 *
 * \return DDS_RETCODE_OK success, one of the standard return codes on failure
 */
DDS_ReturnCode_t
DomainParticipantEvent_on_before_datareader_created(
                                DDS_Subscriber *const subscriber,
                                struct DDS_BuiltinTopicKey_t *const dr_key,
                                DDS_Boolean reservation)
{
    DDS_DomainParticipant *participant;

    participant = DDS_Subscriber_get_participant(subscriber);

    if (participant->disc_plugin)
    {
        return NDDS_Discovery_Plugin_on_before_local_datareader_created(
                    participant->disc_plugin,participant,dr_key,reservation);
    }

    return DDS_RETCODE_OK;
}

/*ci
 * \brief Notification handler for when a local DDS datareader has been enabled
 *
 * \details
 *
 * This function is installed when a datareader is created and is called by
 * the datareader when it is enabled. When a datareader is enabled it is
 * matched with local datawriters and discovered (remote) datawriters
 * (publications) and its existence is advertised in the DDS domain via
 * the discovery plugin (if present).
 *
 * \param[in] reader DDS datareader that is enabled
 * \param[in] qos    Current DDS datareader Qos for the enabled datareader
 */
RTI_BOOL
DomainParticipantEvent_on_after_datareader_enabled(
                        DDS_DataReader *const reader,
                        const struct DDS_DataReaderQos *const qos)
{
    DDS_DomainParticipant *participant;
    struct DDS_DataWriterImpl *local_writer;
    DB_Cursor_T dw_cursor = NULL;
    DB_ReturnCode_T dbrc;
    DDS_InstanceHandle_t ih;
#ifdef RTI_CERT
    DDS_InstanceHandle_t ih_dw;
    struct DDS_BuiltinTopicKey_t dw_key;
#endif
    struct DDS_BuiltinTopicKey_t key;
    struct DDS_BuiltinTopicKey_t dr_key;
    RTI_BOOL retval = RTI_FALSE;
    RT_ComponentFactoryId_T id;
#ifdef RTI_CERT
    struct NETIO_Address dw_address;
#endif
    NETIO_Interface_T *intf = NULL;

    participant = DDS_Subscriber_get_participant(
                                        DDS_DataReader_get_subscriber(reader));

    ih = DDS_Entity_get_instance_handle(DDS_DataReader_as_entity(reader));
    DDS_BuiltinTopicKey_from_guid(&dr_key,&ih);


    if (DB_Database_lock(participant->database) != DB_RETCODE_OK)
    {
        return RTI_FALSE;
    }

    if (!RT_ComponentFactoryId_set_name(&id,NETIO_DEFAULT_INTRA_NAME))
    {
        goto done;
    }

    intf = NULL;
    if (!NETIO_AddressResolver_lookup_interface(
                 participant->address_resolver,NETIO_DEFAULT_INTRA_NAME,&intf))
    {
        goto done;
    }

    dbrc = DB_Table_select_all(participant->local_writer_table,
                               DB_TABLE_DEFAULT_INDEX,&dw_cursor);

    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,participant->local_writer_table,dbrc)
        goto done;
    }

    do
    {
        dbrc = DB_Cursor_get_next(dw_cursor,(DB_Record_T*)&local_writer);

        if (dbrc == DB_RETCODE_OK)
        {
#ifdef RTI_CERT
            /* Make sure the writer is asserted in order for the reader to bind.
             * This is needed because otherwise it will default to regular
             * communication which will cause problems. This ensures that
             * the INTRA transport has higher priority over RTPS (and thus any
             * transport).
             */
            if (intf != NULL)
            {
                NETIO_Address_init(&dw_address,0);

                ih_dw = DDS_Entity_get_instance_handle(
                                        DDS_DataWriter_as_entity(local_writer));
                DDS_BuiltinTopicKey_from_guid(&dw_key,&ih_dw);

                NETIO_Address_set_guid_from_key(&dw_address,0,
                                                (struct NETIO_AddressInt32*)&dw_key);

                if (!NETIO_BindResolver_add_route(participant->bind_resolver,
                                       NETIO_ROUTEKIND_USER,&id,&dw_address,intf,NULL))
                {
                    goto done;
                }
            }

            ih = DDS_Entity_get_instance_handle(
                                       DDS_DataWriter_as_entity(local_writer));
            DDS_BuiltinTopicKey_from_guid(&key,&ih);

            if (DDS_Entity_is_enabled(DDS_DataWriter_as_entity(local_writer)))
            {
                DDS_DataReader_match_local_writer(reader,&key,
                  DDS_TopicDescription_get_name(
                          DDS_Topic_as_topicdescription(local_writer->topic)),
                  DDS_TopicDescription_get_type_name(
                          DDS_Topic_as_topicdescription(local_writer->topic)),
                  &local_writer->qos);

                DDS_DataWriter_match_local_reader(local_writer,&dr_key,
                       DDS_TopicDescription_get_name(
                               DDS_Topic_as_topicdescription(reader->topic)),
                       DDS_TopicDescription_get_type_name(
                               DDS_Topic_as_topicdescription(reader->topic)),
                           qos);
            }
#else
            /* Ignore local data writers that cannot match.
             * DDS_Topic_is_compatible() must not be called since it includes
             * additional logic not applicable here. However, if the topic
             * names are different the reader and the writer cannot match and
             * thus it is guaranteed the route will not be needed and does not
             * need to be asserted.
             */
            if (REDA_String_compare(DDS_TopicDescription_get_name(
                        DDS_Topic_as_topicdescription(local_writer->topic)),
                   DDS_TopicDescription_get_name(
                        DDS_Topic_as_topicdescription(reader->topic))))
            {
                continue;
            }

            /* Make sure the writer is asserted in order for the reader to bind.
             * This is needed because otherwise it will default to regular
             * communication which will cause problems. This ensures that
             * the INTRA transport has higher priority over RTPS (and thus any
             * transport).
             */
            if (intf != NULL)
            {
                if (!DomainParticipantEvent_assert_datawriter_in_intra(
                         intf, &id, participant, local_writer))
                {
                    goto done;
                }
            }

            ih = DDS_Entity_get_instance_handle(
                                       DDS_DataWriter_as_entity(local_writer));
            DDS_BuiltinTopicKey_from_guid(&key,&ih);

            if (DDS_Entity_is_enabled(DDS_DataWriter_as_entity(local_writer)))
            {
                DDS_DataReader_match_local_writer(reader,&key,
                  DDS_TopicDescription_get_name(
                      DDS_Topic_as_topicdescription(local_writer->topic)),
                          DDS_TopicDescription_get_type_name(
                                  DDS_Topic_as_topicdescription(local_writer->topic)),
                                                                &local_writer->qos);

                DDS_DataWriter_match_local_reader(local_writer,&dr_key,
                      DDS_TopicDescription_get_name(
                          DDS_Topic_as_topicdescription(reader->topic)),
                              DDS_TopicDescription_get_type_name(
                                  DDS_Topic_as_topicdescription(reader->topic)),
                                                                qos);
            }
#endif
        }
    } while (dbrc == DB_RETCODE_OK);
    DB_Cursor_finish(participant->local_writer_table,dw_cursor);

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    NDDS_RemotePublication_match_with_local_reader(participant,reader);

    if (participant->disc_plugin)
    {
        if (!NDDS_Discovery_Plugin_on_after_local_datareader_enabled(
                participant->disc_plugin,participant,reader,qos))
        {
            DDSC_LOG_DISC_AFTER_LOCAL_DATAREADER_ENABLED(OSAPI_LOGKIND_WARNING,
                    RT_ComponentFactoryId_get_name(
                                &participant->qos.discovery.discovery.name))
            goto done;
        }
    }

    retval = RTI_TRUE;

done:

    if (DB_Database_unlock(participant->database) != DB_RETCODE_OK)
    {
        return RTI_FALSE;
    }

    return retval;
}

#ifndef RTI_CERT
/*ci
 * \brief Notification handler for when a local DDS datareader is deleted
 *
 * \details
 *
 * This function is installed when a datareader is created and is called by
 * the datareader when it is deleted. When a datareader is deleted it is
 * unmatched from local datawriters and remote publications as well as
 * disposed of in the DDS domain. The disposal in the DDS domain is handled
 * by a discovery plugin (if present)
 *
 * \param[in] reader DDS datareader
 */
void
DomainParticipantEvent_on_before_datareader_deleted(DDS_DataReader *const reader)
{
    DDS_DomainParticipant *participant;
    DDS_InstanceHandle_t ih;
    struct DDS_DataWriterImpl *local_writer;
    DB_Cursor_T dw_cursor = NULL;
    DB_ReturnCode_T dbrc;
    struct DDS_BuiltinTopicKey_t dr_key;
    struct DDS_BuiltinTopicKey_t dw_key;
    struct DataReaderBindEntry *drio_be;
#ifndef RTI_CERT
    RT_ComponentFactoryId_T id;
#endif
    NETIO_Interface_T *drio;
    DDS_Boolean bretval;

    participant = DDS_Subscriber_get_participant(
                                        DDS_DataReader_get_subscriber(reader));

    ih = DDS_Entity_get_instance_handle(DDS_DataReader_as_entity(reader));
    DDS_BuiltinTopicKey_from_guid(&dr_key,&ih);

    if (DB_Database_lock(participant->database) != DB_RETCODE_OK)
    {
        return;
    }

#ifndef RTI_CERT
    if (!RT_ComponentFactoryId_set_name(&id, NETIO_DEFAULT_INTRA_NAME))
    {
        goto done;
    }
#endif

    dbrc = DB_Table_select_all(participant->local_writer_table,
                             DB_TABLE_DEFAULT_INDEX,&dw_cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,participant->local_writer_table,dbrc)
        goto done;
    }

    do
    {
        dbrc = DB_Cursor_get_next(dw_cursor,(DB_Record_T*)&local_writer);
        if (dbrc == DB_RETCODE_OK)
        {
#ifdef RTI_CERT
            ih = DDS_Entity_get_instance_handle(
                    DDS_DataWriter_as_entity(local_writer));
            DDS_BuiltinTopicKey_from_guid(&dw_key,&ih);

            DDS_DataWriter_unmatch_local_reader(local_writer,
                                                   &dr_key,&reader->qos);

            DDS_DataReader_unmatch_local_writer(reader,&dw_key,&local_writer->qos);
#else
            /* Ignore local data writers that cannot match.
             * DDS_Topic_is_compatible() must not be called since it includes
             * additional logic not applicable here. However, if the topic
             * names are different the reader and the writer cannot match and
             * thus it is guaranteed the route will not be needed and does not
             * need to be asserted.
             */
            if (REDA_String_compare(DDS_TopicDescription_get_name(
                        DDS_Topic_as_topicdescription(local_writer->topic)),
                   DDS_TopicDescription_get_name(
                        DDS_Topic_as_topicdescription(reader->topic))))
            {
                continue;
            }

            /* Remove routes added for a local data-writer.
             * If the data-writer has already been deleted then this operation
             * does not change anything as the local data-writer has deleted
             * all the routes itself.
             */
            if (!DomainParticipantEvent_remove_datawriter_from_intra(
                    &id, participant, local_writer))
            {
                goto done;
            }

            ih = DDS_Entity_get_instance_handle(
                    DDS_DataWriter_as_entity(local_writer));
            DDS_BuiltinTopicKey_from_guid(&dw_key,&ih);

            DDS_DataWriter_unmatch_local_reader(local_writer,
                                                   &dr_key,&reader->qos);

            DDS_DataReader_unmatch_local_writer(reader,&dw_key,&local_writer->qos);
#endif
        }
    } while (dbrc == DB_RETCODE_OK);

    DB_Cursor_finish(participant->local_writer_table,dw_cursor);

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    drio = reader->dr_intf;
    if ((drio != NULL) && (!reader->qos.management.is_anonymous))
    {
        dw_cursor = NULL;
        dbrc = DB_Table_select_all(drio->_btable,DB_TABLE_DEFAULT_INDEX,&dw_cursor);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,drio->_btable,dbrc)
            goto done;
        }

        do
        {
            dbrc = DB_Cursor_get_next(dw_cursor,(DB_Record_T*)&drio_be);
            if (dbrc == DB_RETCODE_OK)
            {
                DDS_BuiltinTopicKey_from_guid(&dw_key,
                        (DDS_InstanceHandle_t*)&drio_be->_parent.source.value.guid);
                NDDS_RemotePublication_unmatch_local_reader_from_key(
                        participant,&dw_key,reader);
            }
        } while (dbrc == DB_RETCODE_OK);
        DB_Cursor_finish(drio->_btable,dw_cursor);

        if (dbrc != DB_RETCODE_NO_DATA)
        {
            DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
            goto done;
        }
    }

    if (participant->disc_plugin)
    {
        bretval = NDDS_Discovery_Plugin_on_after_local_datareader_deleted(
                participant->disc_plugin,participant,&dr_key);
#if OSAPI_ENABLE_LOG
        if (!bretval)
        {
            DDSC_LOG_DISC_AFTER_LOCAL_DATAREADER_DELETED(OSAPI_LOGKIND_WARNING,
                    RT_ComponentFactoryId_get_name(
                                &participant->qos.discovery.discovery.name));
        }
#else
        IGNORE_RETVAL(bretval);
#endif
    }

done:
    if (DB_Database_unlock(participant->database) != DB_RETCODE_OK)
    {
        return;
    }
}
#endif /* !RTI_CERT */

/*ci
 * \brief Notification handler for when a local DDS DataWriter is being created
 */
/*ci
 * \brief Notification handler for when a local DDS DataWriter is being created
 *
 * \details
 *
 * This function is called before a DDS DataWriter is created, but after a
 * key has been assigned. The purpose of this function is to give other
 * parts of the system the the ability determine if the creation is ok,
 * such as checking for resources. This function must return DDS_RETCODE_OK
 * if the DDS DataWriter can be created or one of the standard error code if
 * for some reason the DDS DataWriter creation should not proceed. It is
 * up to the caller to decide what to do next.
 *
 * \param[in] publisher   The DDS Publisher creating the datawriter
 * \param[in] dw_key      The assigned key for the DDS datawriter
 * \param[in] reservation DDS_BOOLEAN_TRUE if this is a reservation,
 *                        DDS_BOOLEAN_FALSE if this is releasing a
 *                        reservation.
 *
 * \return DDS_RETCODE_OK success, one of the standard return codes on failure
 */
DDS_ReturnCode_t
DomainParticipantEvent_on_before_datawriter_created(
                                DDS_Publisher *const publisher,
                                struct DDS_BuiltinTopicKey_t *const dw_key,
                                DDS_Boolean reservation)
{
    DDS_DomainParticipant *participant;

    participant = DDS_Publisher_get_participant(publisher);
    if (participant == NULL)
    {
        return DDS_RETCODE_ERROR;
    }

    if (participant->disc_plugin)
    {
        return NDDS_Discovery_Plugin_on_before_local_datawriter_created(
                    participant->disc_plugin,participant,dw_key,reservation);
    }

    return DDS_RETCODE_OK;
}

/*ci
 * \brief Notification handler for when a local DDS datawriter has been enabled
 *
 * \details
 *
 * This function is installed when a datawriter is created and is called by
 * the datawriter when it is enabled. When a datawriter is enabled it is
 * matched with local datareaders and discovered (remote) datareaders
 * (subscriptions) and its existence is advertised in the DDS domain via
 * the discovery plugin (if present).
 *
 * \param[in] writer DDS datawriter that is enabled
 * \param[in] qos    Current DDS datawriter Qos for enabled datawriter
 */
RTI_BOOL
DomainParticipantEvent_on_after_datawriter_enabled(
                                    DDS_DataWriter *const writer,
                                    const struct DDS_DataWriterQos *const qos)
{
    DDS_DomainParticipant *participant;
    struct DDS_DataReaderImpl *local_reader;
    DB_Cursor_T dr_cursor = NULL;
    DB_ReturnCode_T dbrc;
    DDS_InstanceHandle_t ih;
    struct DDS_BuiltinTopicKey_t key;
    struct DDS_BuiltinTopicKey_t dw_key;
    RT_ComponentFactoryId_T id;
#ifdef RTI_CERT
    struct NETIO_Address dw_address;
#endif
    NETIO_Interface_T *intf = NULL;
    RTI_BOOL retval = RTI_FALSE;

    participant = DDS_Publisher_get_participant(
                                        DDS_DataWriter_get_publisher(writer));
    if (participant == NULL)
    {
        return RTI_FALSE;
    }

    ih = DDS_Entity_get_instance_handle(DDS_DataWriter_as_entity(writer));
    DDS_BuiltinTopicKey_from_guid(&dw_key,&ih);

    if (DB_Database_lock(participant->database) != DB_RETCODE_OK)
    {
        return RTI_FALSE;
    }

    if (!RT_ComponentFactoryId_set_name(&id,NETIO_DEFAULT_INTRA_NAME))
    {
        goto done;
    }

#ifdef RTI_CERT
    NETIO_Address_init(&dw_address,0);
#endif
    intf = NULL;
    if (!NETIO_AddressResolver_lookup_interface(
                  participant->address_resolver,NETIO_DEFAULT_INTRA_NAME,&intf))
    {
        goto done;
    }

#ifdef RTI_CERT
    if (intf != NULL)
    {
        NETIO_Address_set_guid_from_key(&dw_address,0,
                                        (struct NETIO_AddressInt32*)&dw_key);
        if (!NETIO_BindResolver_add_route(participant->bind_resolver,
                                NETIO_ROUTEKIND_USER,&id,&dw_address,intf,NULL))
        {
            goto done;
        }
    }

#else
    /* Assert intra route for this writer for the scenario where
     * this writer is created after a matching reader
     */
    if (intf != NULL)
    {
        if (!DomainParticipantEvent_assert_datawriter_in_intra(intf, &id,
                                                               participant,
                                                               writer))
        {
            goto done;
        }
    }
#endif

    dbrc = DB_Table_select_all(participant->local_reader_table,
                               DB_TABLE_DEFAULT_INDEX,&dr_cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,participant->local_reader_table,dbrc)
        goto done;
    }

    do
    {
        dbrc = DB_Cursor_get_next(dr_cursor,(DB_Record_T*)&local_reader);
        if (dbrc == DB_RETCODE_OK)
        {
            ih = DDS_Entity_get_instance_handle(
                    DDS_DataReader_as_entity(local_reader));
            DDS_BuiltinTopicKey_from_guid(&key,&ih);

            if (DDS_Entity_is_enabled(DDS_DataReader_as_entity(local_reader)))
            {

                DDS_DataWriter_match_local_reader(writer,&key,
                      DDS_TopicDescription_get_name(
                          DDS_Topic_as_topicdescription(local_reader->topic)),
                      DDS_TopicDescription_get_type_name(
                          DDS_Topic_as_topicdescription(local_reader->topic)),
                          &local_reader->qos);

                DDS_DataReader_match_local_writer(local_reader,&dw_key,
                          DDS_TopicDescription_get_name(
                          DDS_Topic_as_topicdescription(writer->topic)),
                          DDS_TopicDescription_get_type_name(
                          DDS_Topic_as_topicdescription(writer->topic)),qos);
            }
        }
    } while (dbrc == DB_RETCODE_OK);
    DB_Cursor_finish(participant->local_reader_table,dr_cursor);

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    NDDS_RemoteSubscription_match_with_local_writer(participant,writer);

    if (participant->disc_plugin)
    {
        if (!NDDS_Discovery_Plugin_on_after_local_datawriter_enabled(
                            participant->disc_plugin,participant,writer,qos))
        {
            DDSC_LOG_DISC_AFTER_LOCAL_DATAWRITER_ENABLED(OSAPI_LOGKIND_WARNING,
                    RT_ComponentFactoryId_get_name(
                            &participant->qos.discovery.discovery.name))
            goto done;
        }
    }

    retval = RTI_TRUE;

done:
    if (DB_Database_unlock(participant->database) != DB_RETCODE_OK)
    {
        return RTI_FALSE;
    }

    return retval;
}

#ifndef RTI_CERT
/*ci
 * \brief Notification handler for when a local DDS datawriter is deleted
 *
 * \details
 *
 * This function is installed when a datawriter is created and is called
 * by a datawriter when it is deleted. When a datawriter is deleted it is
 * unmatched from local datareaders and remote subscriptions as well as
 * disposed of in the DDS domain. The disposal in the DDS domain is handled
 * by a discovery plugin (if present)
 *
 * \param[in] writer DDS datawriter
 */
void
DomainParticipantEvent_on_before_datawriter_deleted(DDS_DataWriter *const writer)
{
    DDS_DomainParticipant *participant;
    DDS_InstanceHandle_t ih;
    struct DDS_BuiltinTopicKey_t dw_key;
    DB_Cursor_T dr_cursor = NULL;
    DB_ReturnCode_T dbrc;
    struct DDS_DataReaderImpl *local_reader;
    RT_ComponentFactoryId_T id;
    struct NETIO_Address dw_address;
    NETIO_Interface_T *intf = NULL;
    NETIO_Interface_T *dwio;
    DB_Cursor_T cursor = NULL;
    struct DDS_DataWriterBindEntry *dwio_be;
    DDS_BuiltinTopicKey_t dr_key;
    DDS_Boolean bretval;

    participant = DDS_Publisher_get_participant(
                                        DDS_DataWriter_get_publisher(writer));
    if (participant == NULL)
    {
        return;
    }

    ih = DDS_Entity_get_instance_handle(DDS_DataWriter_as_entity(writer));
    DDS_BuiltinTopicKey_from_guid(&dw_key,&ih);

    if (DB_Database_lock(participant->database) != DB_RETCODE_OK)
    {
        return;
    }

    if (!RT_ComponentFactoryId_set_name(&id,NETIO_DEFAULT_INTRA_NAME))
    {
        goto done;
    }

    NETIO_Address_init(&dw_address,0);

    intf = NULL;
    if (!NETIO_AddressResolver_lookup_interface(
                  participant->address_resolver,NETIO_DEFAULT_INTRA_NAME,&intf))
    {
        goto done;
    }

    /* Delete route for intra added by the writer itself at
     * DomainParticipantEvent_on_after_datawriter_enabled
     */
    if (intf != NULL)
    {
        if (!DomainParticipantEvent_remove_datawriter_from_intra(&id, participant, writer))
        {
            goto done;
        }
    }

    dbrc = DB_Table_select_all(participant->local_reader_table,
                               DB_TABLE_DEFAULT_INDEX,&dr_cursor);
    if (dbrc != DB_RETCODE_OK)
    {
        DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,participant->local_reader_table,dbrc)
        goto done;
    }

    do
    {
        dbrc = DB_Cursor_get_next(dr_cursor,(DB_Record_T*)&local_reader);
        if (dbrc == DB_RETCODE_OK)
        {
            /* Ignore local data readers that cannot match.
             * DDS_Topic_is_compatible() must not be called since it includes
             * additional logic not applicable here. However, if the topic
             * names are different the reader and the writer cannot match and
             * thus it is guaranteed the route will not be needed and does not
             * need to be asserted.
             */
            if (REDA_String_compare(DDS_TopicDescription_get_name(
                        DDS_Topic_as_topicdescription(local_reader->topic)),
                   DDS_TopicDescription_get_name(
                        DDS_Topic_as_topicdescription(writer->topic))))
            {
                continue;
            }

            /* Remove routes added for this data-writer by a local data-reader.
             * If the data-reader has already been deleted then this operation
             * does not change anything as the local data-reader has deleted
             * the route it created for this data-writer.
             */
            if (intf != NULL)
            {
                if (!DomainParticipantEvent_remove_datawriter_from_intra(&id, participant, writer))
                {
                    goto done;
                }
            }

            ih = DDS_Entity_get_instance_handle(
                    DDS_DataReader_as_entity(local_reader));
            DDS_BuiltinTopicKey_from_guid(&dr_key,&ih);

            DDS_DataReader_unmatch_local_writer(local_reader,
                                                &dw_key,&writer->qos);

            DDS_DataWriter_unmatch_local_reader(writer,
                                                &dr_key,&local_reader->qos);
        }
    } while (dbrc == DB_RETCODE_OK);
    DB_Cursor_finish(participant->local_reader_table,dr_cursor);

    if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    dwio = writer->dw_intf;
    if ((dwio != NULL) && (!writer->qos.management.is_anonymous))
    {
        cursor = NULL;
        dbrc = DB_Table_select_all(dwio->_btable,DB_TABLE_DEFAULT_INDEX,&cursor);
        if (dbrc != DB_RETCODE_OK)
        {
            DDSC_LOG_TABLE_SELECT(OSAPI_LOGKIND_ERROR,dwio->_btable,dbrc)
            goto done;
        }

        do
        {
            dbrc = DB_Cursor_get_next(cursor,(DB_Record_T*)&dwio_be);
            if (dbrc == DB_RETCODE_OK)
            {
                DDS_BuiltinTopicKey_from_guid(&dr_key,
                                    (DDS_InstanceHandle_t*)&dwio_be->source);
                NDDS_RemoteSubscription_unmatch_local_writer_from_key(
                        participant,&dr_key,writer);
            }
        } while (dbrc == DB_RETCODE_OK);

        DB_Cursor_finish(dwio->_btable,cursor);

        if (dbrc != DB_RETCODE_NO_DATA)
        {
            DDSC_LOG_DB_CURSOR_INVALIDATED(OSAPI_LOGKIND_ERROR)
            goto done;
        }
    }

    if (participant->disc_plugin)
    {
        bretval = NDDS_Discovery_Plugin_on_after_local_datawriter_deleted(
                participant->disc_plugin,participant,&dw_key);
#if OSAPI_ENABLE_LOG
        if (!bretval)
        {
            DDSC_LOG_DISC_AFTER_LOCAL_DATAWRITER_DELETED(OSAPI_LOGKIND_WARNING,
                    RT_ComponentFactoryId_get_name(&
                            participant->qos.discovery.discovery.name));
        }
#else
        IGNORE_RETVAL(bretval);
#endif
    }

done:
    if (DB_Database_unlock(participant->database) != DB_RETCODE_OK)
    {
        return;
    }
}
#endif /* !RTI_CERT */

/*******************************************************************************
 *                            LISTENER FORWARDERS APIs
 ******************************************************************************/
/*ci
 * \brief Listener called by a publisher for the OFFERED_INCOMPATIBLE_QOS_STATUS
 *        change if not handled by the datawriter or publisher
 *
 * \details
 *
 * A listener is said to consume the event if a listener is installed for the
 * event. If the event was not consumed by a contained datawriter or publisher
 * (checked in that order) it is propagated to the participant which may or
 * may not consume the event. This function checks if the event is consumed
 * by the participant.
 *
 * \param[in] self   The participant forwarded to
 * \param[in] writer The DDS datawriter the notification originated in
 * \param[in] status The listener status data
 *
 * \return DDS_BOOLEAN_TRUE if the event was consumed, DDS_BOOLEAN_FALSE if it
 *         was not consumed
 */
DDS_Boolean
NDDS_DomainParticipant_on_offered_incompatible_qos(
            DDS_DomainParticipant *self,
            DDS_DataWriter *writer,
            const struct DDS_OfferedIncompatibleQosStatus *status)
{
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;

    if (self->mask & DDS_OFFERED_INCOMPATIBLE_QOS_STATUS)
    {
        if (self->listener.as_publisherlistener.as_datawriterlistener.
                on_offered_incompatible_qos != NULL)
        {
            DDS_EntityImpl_disable_status(&writer->as_entity,
                DDS_OFFERED_INCOMPATIBLE_QOS_STATUS);
            DDS_OfferedIncompatibleQosStatus_reset(
                        &writer->off_incompatible_qos_status);
            self->listener.as_publisherlistener.as_datawriterlistener.
                on_offered_incompatible_qos(self->listener.as_publisherlistener.
                                        as_datawriterlistener.as_listener.
                                        listener_data, writer, status);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }

    return event_consumed;
}

/*ci
 * \brief Listener called by a publisher for the PUBLICATION_MATCHED_STATUS
 *        change if not handled by the datawriter or publisher
 *
 * \details
 *
 * A listener is said to consume the event if a listener is installed for the
 * event. If the event was not consumed by a contained datawriter or publisher
 * (checked in that order) it is propagated to the participant which may or
 * may not consume the event. This function checks if the event is consumed
 * by the participant.
 *
 * \param[in] self   The participant forwarded to
 * \param[in] writer The DDS datawriter the notification originated in
 * \param[in] status The listener status data
 *
 * \return DDS_BOOLEAN_TRUE if the event was consumed, DDS_BOOLEAN_FALSE if it
 *         was not consumed
 */
DDS_Boolean
NDDS_DomainParticipant_on_publication_matched(
        DDS_DomainParticipant *self,
        DDS_DataWriter *writer,
        const struct DDS_PublicationMatchedStatus *status)
{
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;

    if (self->mask & DDS_PUBLICATION_MATCHED_STATUS)
    {
        if (self->listener.as_publisherlistener.as_datawriterlistener.
                on_publication_matched != NULL)
        {
            DDS_EntityImpl_disable_status(&writer->as_entity,
                    DDS_PUBLICATION_MATCHED_STATUS);
            DDS_PublicationMatchedStatus_reset(
                    &writer->publication_matched_status);
            self->listener.as_publisherlistener.as_datawriterlistener.
                on_publication_matched(self->listener.as_publisherlistener.
                                   as_datawriterlistener.as_listener.
                                   listener_data, writer, status);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }

    return event_consumed;
}

/*ci
 * \brief Listener called by a publisher for the
 *        RELIABLE_READER_ACTIVITY_CHANGED_STATUS change if not handled by the
 *        datawriter or publisher
 *
 * \details
 *
 * A listener is said to consume the event if a listener is installed for the
 * event. If the event was not consumed by a contained datawriter or publisher
 * (checked in that order) it is propagated to the participant which may or
 * may not consume the event. This function checks if the event is consumed
 * by the participant.
 *
 * \param[in] self   The participant forwarded to
 * \param[in] writer The DDS datawriter the notification originated in
 * \param[in] status The listener status data
 *
 * \return DDS_BOOLEAN_TRUE if the event was consumed, DDS_BOOLEAN_FALSE if it
 *         was not consumed
 */
DDS_Boolean
NDDS_DomainParticipant_on_reliable_reader_activity_changed(
        DDS_DomainParticipant *self,
        DDS_DataWriter *writer,
        const struct DDS_ReliableReaderActivityChangedStatus *status)
{
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;

    if (self->mask & DDS_RELIABLE_READER_ACTIVITY_CHANGED_STATUS)
    {
        if (self->listener.as_publisherlistener.as_datawriterlistener.
                on_reliable_reader_activity_changed != NULL)
        {
            DDS_EntityImpl_disable_status(&writer->as_entity,
                DDS_RELIABLE_READER_ACTIVITY_CHANGED_STATUS);
            DDS_ReliableReaderActivityChangedStatus_reset(
                     &writer->reliable_reader_activity_changed_status);
            self->listener.as_publisherlistener.as_datawriterlistener.
                on_reliable_reader_activity_changed(
                   self->listener.as_publisherlistener.as_datawriterlistener.
                   as_listener.listener_data, writer, status);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }

    return event_consumed;
}

/*ci
 * \brief Listener called by a subscriber for the
 *        REQUESTED_INCOMPATIBLE_QOS_STATUS change if not handled by the
 *        datareader or subscriber
 *
 * \details
 *
 * A listener is said to consume the event if a listener is installed for the
 * event. If the event was not consumed by a contained datareader or subscriber
 * (checked in that order) it is propagated to the participant which may or
 * may not consume the event. This function checks if the event is consumed
 * by the participant.
 *
 * \param[in] self   The participant forwarded to
 * \param[in] reader The DDS datareader the notification originated in
 * \param[in] status The listener status data
 *
 * \return DDS_BOOLEAN_TRUE if the event was consumed, DDS_BOOLEAN_FALSE if it
 *         was not consumed
 */
DDS_Boolean
NDDS_DomainParticipant_on_requested_incompatible_qos(
        DDS_DomainParticipant *self,
        DDS_DataReader *reader,
        const struct DDS_RequestedIncompatibleQosStatus *status)
{
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;

    if (self->mask & DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS)
    {
        if (self->listener.as_subscriberlistener.as_datareaderlistener.
                on_requested_incompatible_qos != NULL)
        {
            DDS_EntityImpl_disable_status(&reader->as_entity,
                DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS);
            DDS_RequestedIncompatibleQosStatus_reset(
                    &reader->req_incompatible_qos_status);
            self->listener.as_subscriberlistener.as_datareaderlistener.
                on_requested_incompatible_qos(
                        self->listener.as_subscriberlistener.
                        as_datareaderlistener.as_listener.
                        listener_data, reader, status);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }

    return event_consumed;
}

/*ci
 * \brief Listener called by a subscriber for the SUBSCRIPTION_MATCHED_STATUS
 *        change if not handled by the datareader or subscriber
 *
 * \details
 *
 * A listener is said to consume the event if a listener is installed for the
 * event. If the event was not consumed by a contained datareader or subscriber
 * (checked in that order) it is propagated to the participant which may or
 * may not consume the event. This function checks if the event is consumed
 * by the participant.
 *
 * \param[in] self   The participant forwarded to
 * \param[in] reader The DDS datareader the notification originated in
 * \param[in] status The listener status data
 *
 * \return DDS_BOOLEAN_TRUE if the event was consumed, DDS_BOOLEAN_FALSE if it
 *         was not consumed
 */
DDS_Boolean
NDDS_DomainParticipant_on_subscription_matched(
        DDS_DomainParticipant *self,
        DDS_DataReader *reader,
        const struct DDS_SubscriptionMatchedStatus *status)
{
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;

    if (self->mask & DDS_SUBSCRIPTION_MATCHED_STATUS)
    {
        if (self->listener.as_subscriberlistener.as_datareaderlistener.
                on_subscription_matched != NULL)
        {
            DDS_EntityImpl_disable_status(&reader->as_entity,
                    DDS_SUBSCRIPTION_MATCHED_STATUS);
            DDS_SubscriptionMatchedStatus_reset(
                    &reader->subscription_matched_status);
            self->listener.as_subscriberlistener.as_datareaderlistener.
                on_subscription_matched(self->listener.as_subscriberlistener.
                                    as_datareaderlistener.as_listener.
                                    listener_data, reader, status);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }

    return event_consumed;
}

/*ci
 * \brief Listener called by a subscriber for the DATA_AVAILABLE_STATUS
 *        change if not handled by the datareader or subscriber
 *
 * \details
 *
 * A listener is said to consume the event if a listener is installed for the
 * event. If the event was not consumed by a contained datareader or subscriber
 * (checked in that order) it is propagated to the participant which may or
 * may not consume the event. This function checks if the event is consumed
 * by the participant.
 *
 * \param[in] self       The participant the listener is forwarded to
 * \param[in] subscriber The DDS subscriber the notification originated in
 * \param[in] reader     The DDS datareader the notification originated in
 *
 * \return DDS_BOOLEAN_TRUE if the event was consumed, DDS_BOOLEAN_FALSE if it
 *         was not consumed
 */
DDS_Boolean
NDDS_DomainParticipant_on_data_available(DDS_DomainParticipant *self,
                                         DDS_Subscriber *subscriber,
                                         DDS_DataReader *reader)
{
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;

    if (self->mask & DDS_DATA_AVAILABLE_STATUS)
    {
        if (self->listener.as_subscriberlistener.as_datareaderlistener.
                on_data_available != NULL)
        {
            DDS_EntityImpl_disable_status(&reader->as_entity,
                    DDS_DATA_AVAILABLE_STATUS);
            DDS_EntityImpl_disable_status(&subscriber->as_entity,
                    DDS_DATA_ON_READERS_STATUS);
            self->listener.as_subscriberlistener.as_datareaderlistener.
                on_data_available(self->listener.as_subscriberlistener.
                              as_datareaderlistener.as_listener.listener_data,
                              reader);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }

    return event_consumed;
}

/*ci
 * \brief Listener called by a subscriber for the DATA_ON_READERS_STATUS
 *        change if not handled by the subscriber
 *
 * \details
 *
 * A listener is said to consume the event if a listener is installed for the
 * event. If the event was not consumed by a contained subscriber
 * it is propagated to the participant which may or may not consume the event.
 * This function checks if the event is consumed by the participant.
 *
 * \param[in] self       The participant forwarded to
 * \param[in] subscriber The DDS subscriber the notification originated in
 *
 * \return DDS_BOOLEAN_TRUE if the event was consumed, DDS_BOOLEAN_FALSE if it
 *         was not consumed
 */
DDS_Boolean
NDDS_DomainParticipant_on_data_on_readers(DDS_DomainParticipant *self,
                                         DDS_Subscriber *subscriber)
{
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;

    if (self->mask & DDS_DATA_ON_READERS_STATUS)
    {
        if (self->listener.as_subscriberlistener.on_data_on_readers != NULL)
        {
            DDS_EntityImpl_disable_status(&subscriber->as_entity,
                    DDS_DATA_ON_READERS_STATUS);
            self->listener.as_subscriberlistener.on_data_on_readers(
                    self->listener.as_subscriberlistener.
                    as_datareaderlistener.as_listener.listener_data,
                    subscriber);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }

    return event_consumed;
}

/*ci
 * \brief Listener called by a subscriber for the REQUESTED_DEADLINE_MISSED_STATUS
 *        change if not handled by a subscriber or datareader
 *
 * \details
 *
 * A listener is said to consume the event if a listener is installed for the
 * event. If the event was not consumed by a contained datareader or subscriber
 * (checked in that order) it is propagated to the participant which may or
 * may not consume the event. This function checks if the event is consumed
 * by the participant.
 *
 * \param[in] self   The participant forwarded to
 * \param[in] reader The DDS datareader the notification originated in
 * \param[in] status The status associated with the change
 *
 * \return DDS_BOOLEAN_TRUE if the event was consumed, DDS_BOOLEAN_FALSE if it
 *         was not consumed
 */
DDS_Boolean
NDDS_DomainParticipant_on_requested_deadline_missed(
        DDS_DomainParticipant *self,
        DDS_DataReader *reader,
        const struct DDS_RequestedDeadlineMissedStatus *status)
{
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;

    if (self->mask & DDS_REQUESTED_DEADLINE_MISSED_STATUS)
    {
        if (self->listener.as_subscriberlistener.as_datareaderlistener.
                on_requested_deadline_missed != NULL)
        {
            DDS_EntityImpl_disable_status(&reader->as_entity,
                DDS_REQUESTED_DEADLINE_MISSED_STATUS);
            DDS_RequestedDeadlineMissedStatus_reset(
                &reader->req_deadline_missed_status);
            self->listener.as_subscriberlistener.as_datareaderlistener.
                on_requested_deadline_missed(self->listener.as_subscriberlistener.
                                         as_datareaderlistener.as_listener.
                                         listener_data, reader, status);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }

    return event_consumed;
}

/*ci
 * \brief Listener called by a subscriber for the LIVELINESS_CHANGED_STATUS
 *        change if not handled by a subscriber or datareader
 *
 * \details
 *
 * A listener is said to consume the event if a listener is installed for the
 * event. If the event was not consumed by a contained datareader or subscriber
 * (checked in that order) it is propagated to the participant which may or
 * may not consume the event. This function checks if the event is consumed
 * by the participant.
 *
 * \param[in] self   The participant forwarded to
 * \param[in] reader The DDS datareader the notification originated in
 * \param[in] status The status associated with the change
 *
 * \return DDS_BOOLEAN_TRUE if the event was consumed, DDS_BOOLEAN_FALSE if it
 *         was not consumed
 */
DDS_Boolean
NDDS_DomainParticipant_on_liveliness_changed(
        DDS_DomainParticipant *self,
        DDS_DataReader *reader,
        const struct DDS_LivelinessChangedStatus *status)
{
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;

    if (self->mask & DDS_LIVELINESS_CHANGED_STATUS)
    {
        if (self->listener.as_subscriberlistener.as_datareaderlistener.
                on_liveliness_changed != NULL)
        {
            DDS_EntityImpl_disable_status(&reader->as_entity,
                    DDS_LIVELINESS_CHANGED_STATUS);
            DDS_LivelinessChangedStatus_reset(
                &reader->liveliness_changed_status);
            self->listener.as_subscriberlistener.as_datareaderlistener.
                on_liveliness_changed(self->listener.as_subscriberlistener.
                                  as_datareaderlistener.as_listener.
                                  listener_data, reader, status);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }

    return event_consumed;
}

/*ci
 * \brief Listener called by a subscriber for the SAMPLE_LOST_STATUS
 *        change if not handled by a subscriber or datareader
 *
 * \details
 *
 * A listener is said to consume the event if a listener is installed for the
 * event. If the event was not consumed by a contained datareader or subscriber
 * (checked in that order) it is propagated to the participant which may or
 * may not consume the event. This function checks if the event is consumed
 * by the participant.
 *
 * \param[in] self   The participant forwarded to
 * \param[in] reader The DDS datareader the notification originated in
 * \param[in] status The status associated with the change
 *
 * \return DDS_BOOLEAN_TRUE if the event was consumed, DDS_BOOLEAN_FALSE if it
 *         was not consumed
 */
DDS_Boolean
NDDS_DomainParticipant_on_sample_lost(
        DDS_DomainParticipant *self,
        DDS_DataReader *reader,
        const struct DDS_SampleLostStatus *status)
{
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;

    if (self->mask & DDS_SAMPLE_LOST_STATUS)
    {
        if (self->listener.as_subscriberlistener.as_datareaderlistener.
                on_sample_lost != NULL)
        {
            DDS_EntityImpl_disable_status(&reader->as_entity,
                    DDS_SAMPLE_LOST_STATUS);
            DDS_SampleLostStatus_reset(&reader->sample_lost_status);
            self->listener.as_subscriberlistener.as_datareaderlistener.
                on_sample_lost(self->listener.as_subscriberlistener.
                           as_datareaderlistener.as_listener.listener_data,
                           reader, status);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }

    return event_consumed;
}

/*ci
 * \brief Listener called by a subscriber for the SAMPLE_REJECTED_STATUS
 *        change if not handled by a subscriber or datareader
 *
 * \details
 *
 * A listener is said to consume the event if a listener is installed for the
 * event. If the event was not consumed by a contained datareader or subscriber
 * (checked in that order) it is propagated to the participant which may or
 * may not consume the event. This function checks if the event is consumed
 * by the participant.
 *
 * \param[in] self   The participant forwarded to
 * \param[in] reader The DDS datareader the notification originated in
 * \param[in] status The status associated with the change
 *
 * \return DDS_BOOLEAN_TRUE if the event was consumed, DDS_BOOLEAN_FALSE if it
 *         was not consumed
 */
DDS_Boolean
NDDS_DomainParticipant_on_sample_rejected(
        DDS_DomainParticipant *self,
        DDS_DataReader *reader,
        const struct DDS_SampleRejectedStatus *status)
{
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;

    if (self->mask & DDS_SAMPLE_REJECTED_STATUS)
    {
        if (self->listener.as_subscriberlistener.as_datareaderlistener.
                on_sample_rejected != NULL)
        {
            DDS_EntityImpl_disable_status(&reader->as_entity,
                    DDS_SAMPLE_REJECTED_STATUS);
            DDS_SampleRejectedStatus_reset(&reader->sample_rejected_status);
            self->listener.as_subscriberlistener.as_datareaderlistener.
                on_sample_rejected(self->listener.as_subscriberlistener.
                               as_datareaderlistener.as_listener.listener_data,
                               reader, status);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }

    return event_consumed;
}

/*ci
 * \brief Listener called by a topic for the INCONSISTENT_TOPIC_STATUS
 *        change if not handled by the topic
 *
 * \details
 *
 * A listener is said to consume the event if a listener is installed for the
 * event. If the event was not consumed by the topic it is propagated to the
 * participant which may or may not consume the event. This function checks if
 * the event is consumed by the participant.
 *
 * \param[in] self   The participant forwarded to
 * \param[in] topic  The DDS topic the notification originated in
 * \param[in] status The status associated with the change
 *
 * \return DDS_BOOLEAN_TRUE if the event was consumed, DDS_BOOLEAN_FALSE if it
 *         was not consumed
 */
DDS_Boolean
NDDS_DomainParticipant_on_inconsistent_topic(
        DDS_DomainParticipant *self,
        DDS_Topic *topic,
        const struct DDS_InconsistentTopicStatus *status)
{
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;

    if (self->mask & DDS_INCONSISTENT_TOPIC_STATUS)
    {
        if (self->listener.as_topiclistener.on_inconsistent_topic != NULL)
        {
            DDS_EntityImpl_disable_status(&topic->as_entity,
                    DDS_INCONSISTENT_TOPIC_STATUS);
            DDS_InconsistentTopicStatus_reset(&topic->inconsistent_status);
            self->listener.as_topiclistener.on_inconsistent_topic(
                    self->listener.as_topiclistener.
                    as_listener.listener_data,
                    topic, status);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }

    return event_consumed;
}

/*ci
 * \brief Listener called by a publisher for the LIVELINESS_LOST_STATUS
 *        change if not handled by a publisher or datawriter
 *
 * \details
 *
 * A listener is said to consume the event if a listener is installed for the
 * event. If the event was not consumed by a contained datawriter or publisher
 * (checked in that order) it is propagated to the participant which may or
 * may not consume the event. This function checks if the event is consumed
 * by the participant.
 *
 * \param[in] self   The participant forwarded to
 * \param[in] writer The DDS datawriter the notification originated in
 * \param[in] status The status associated with the change
 *
 * \return DDS_BOOLEAN_TRUE if the event was consumed, DDS_BOOLEAN_FALSE if it
 *         was not consumed
 */
DDS_Boolean
NDDS_DomainParticipant_on_liveliness_lost(
        DDS_DomainParticipant *self,
        DDS_DataWriter *writer,
        const struct DDS_LivelinessLostStatus *status)
{
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;

    if (self->mask & DDS_LIVELINESS_LOST_STATUS)
    {
        if (self->listener.as_publisherlistener.as_datawriterlistener.
                on_liveliness_lost != NULL)
        {
            DDS_EntityImpl_disable_status(&writer->as_entity,
                    DDS_LIVELINESS_LOST_STATUS);
            DDS_LivelinessLostStatus_reset(&writer->liveliness_lost_status);
            self->listener.as_publisherlistener.as_datawriterlistener.
                on_liveliness_lost(self->listener.as_publisherlistener.
                               as_datawriterlistener.as_listener.listener_data,
                               writer, status);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }

    return event_consumed;
}

/*ci
 * \brief Listener called by a publisher for the OFFERED_DEADLINE_MISSED_STATUS
 *        change if not handled by a publisher or datawriter
 *
 * \details
 *
 * A listener is said to consume the event if a listener is installed for the
 * event. If the event was not consumed by a contained datawriter or publisher
 * (checked in that order) it is propagated to the participant which may or
 * may not consume the event. This function checks if the event is consumed
 * by the participant.
 *
 * \param[in] self   The participant forwarded to
 * \param[in] writer The DDS datawriter the notification originated in
 * \param[in] status The status associated with the change
 *
 * \return DDS_BOOLEAN_TRUE if the event was consumed, DDS_BOOLEAN_FALSE if it
 *         was not consumed
 */
DDS_Boolean
NDDS_DomainParticipant_on_offered_deadline_missed(
        DDS_DomainParticipant *self,
        DDS_DataWriter *writer,
        const struct DDS_OfferedDeadlineMissedStatus *status)
{
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;

    if (self->mask & DDS_OFFERED_DEADLINE_MISSED_STATUS)
    {
        if (self->listener.as_publisherlistener.as_datawriterlistener.
                on_offered_deadline_missed != NULL)
        {
            DDS_EntityImpl_disable_status(&writer->as_entity,
                    DDS_OFFERED_DEADLINE_MISSED_STATUS);
            DDS_OfferedDeadlineMissedStatus_reset(
                    &writer->off_deadline_missed_status);
            self->listener.as_publisherlistener.as_datawriterlistener.
                on_offered_deadline_missed(self->listener.as_publisherlistener.
                                       as_datawriterlistener.as_listener.
                                       listener_data, writer, status);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }

    return event_consumed;
}

/*ci
 * \brief Listener called by a subscriber for the INSTANCE_REPLACED_STATUS
 *        change if not handled by a subscriber or datareader
 *
 * \details
 *
 * A listener is said to consume the event if a listener is installed for the
 * event. If the event was not consumed by a contained datareader or subscriber
 * (checked in that order) it is propagated to the participant which may or
 * may not consume the event. This function checks if the event is consumed
 * by the participant.
 *
 * \param[in] self   The participant forwarded to
 * \param[in] reader The DDS datareader the notification originated in
 * \param[in] status The status associated with the change
 *
 * \return DDS_BOOLEAN_TRUE if the event was consumed, DDS_BOOLEAN_FALSE if it
 *         was not consumed
 */
DDS_Boolean
NDDS_DomainParticipantListener_on_instance_replaced(
        DDS_DomainParticipant *self,
        DDS_DataReader *reader,
        const struct DDS_DataReaderInstanceReplacedStatus *status)
{
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;

    if (self->mask & DDS_INSTANCE_REPLACED_STATUS)
    {
        if (self->listener.as_subscriberlistener.
                as_datareaderlistener.on_instance_replaced != NULL)
        {
            DDS_EntityImpl_disable_status(&reader->as_entity,
                    DDS_INSTANCE_REPLACED_STATUS);
            DDS_DataReaderInstanceReplacedStatus_reset(
                &reader->instance_replaced_status);
            self->listener.as_subscriberlistener.as_datareaderlistener.
                on_instance_replaced(self->listener.as_subscriberlistener.
                                       as_datareaderlistener.as_listener.
                                       listener_data, reader, status);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }

    return event_consumed;
}

/*ci @} */

