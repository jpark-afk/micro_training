/*
 * FILE: AppGenPlugin.c - Application Generator implementation
 *
 * (c) Copyright, Real-Time Innovations, 2017-2024.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 */
/*ci
 * \file
 * \brief Micro Application Generator implementation
 */
/*ci \addtogroup AppGenModule
 * @{
 */
#include "AppGenPlugin.h"

#ifndef appgen_h
#include "app_gen/app_gen.h"
#endif

#ifndef appgen_log_h
#include "app_gen/app_gen_log.h"
#endif

#ifndef wh_sm_history_h
#include "wh_sm/wh_sm_history.h"
#endif

#ifndef rh_sm_history_h
#include "rh_sm/rh_sm_history.h"
#endif

#ifndef dds_c_type_h
#include "dds_c/dds_c_type.h"
#endif

#ifndef dds_c_content_filter_h
#include "dds_c/dds_c_content_filter.h"
#endif

#define T struct APPGEN_LibraryModel
#define TSeq APPGEN_LibraryModelSeq
#include "reda/reda_sequence_defn.h"

/*ci
 *\brief Forward declaration of the Application Generation interface implementation
 */
LINK_SECTION_DATA_SDRAM
RTI_PRIVATE struct DDS_AppGenI AppGen_fv_Intf;

#ifndef RTI_CERT
/*ci
 * \brief Finalize an App Generation instance
 *
 * \param[in] appgen Interface to finalize
 *
 * \sa \ref AppGen_initialize
 */
RTI_PRIVATE void
APPGEN_Plugin_finalize(struct APPGEN_Plugin *appgen)
{
    RT_Component_finalize(&appgen->_parent._parent);
}
#endif /* !RTI_CERT */

/*ci
 * \brief Initialize an App Generation instance
 *
 * \param[in] appgen      Interface to delete
 * \param[in] factory     App Generation factory that is creating the instance
 * \param[in] property    The property of the new App Generation
 * \param[in] listener    The listener for the new App Generation
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 *
 * \sa \ref APPGEN_Plugin_finalize
 *
 */
RTI_PRIVATE void
APPGEN_Plugin_initialize(struct APPGEN_Plugin *appgen,
                         struct APPGEN_Factory *factory,
                         const struct DDS_AppGen_ComponentProperty *property,
                         const struct RT_ComponentListener *const listener)
{
    UNUSED_ARG(listener);

    RT_Component_initialize(&appgen->_parent._parent,
                            &AppGen_fv_Intf._parent,
                            0,
                            NULL,
                            NULL);

    appgen->property.entity_manager = property->entity_manager;

    appgen->factory = factory;
}

/*ci
 * \brief Create a new App Generation instance
 *
 * \param[in] factory   Factory creating the new instance
 * \param[in] property  The property of the app generation interface
 * \param[in] listener  The listener for the app generation interface
 *
 * \return Pointer to new Application Create instance on success, NULL
 *         on failure
 *
 * \sa \ref APPGEN_Plugin_delete
 */
MUST_CHECK_RETURN RTI_PRIVATE struct APPGEN_Plugin*
APPGEN_Plugin_create(struct APPGEN_Factory *factory,
                     struct DDS_AppGen_ComponentProperty *property,
                     const struct RT_ComponentListener *const listener)
{
    struct APPGEN_Plugin *appgen = NULL;

    OSAPI_PRECONDITION((factory == NULL) || (property == NULL),
                       return NULL,
                   OSAPI_Log_entry_add_pointer("factory",factory,RTI_FALSE);
                   OSAPI_Log_entry_add_pointer("property",property,RTI_TRUE);)

    OSAPI_Heap_allocate_struct(&appgen, struct APPGEN_Plugin);
    if (appgen == NULL)
    {
        APPGEN_LOG_ALLOCATE(OSAPI_LOGKIND_ERROR)
        return NULL;
    }

    APPGEN_Plugin_initialize(appgen, factory, property, listener);

    return appgen;
}

#ifndef RTI_CERT
/*ci
 * \brief Delete an App Generation instance
 *
 * \param[in] appgen App Generation to delete
 *
 * \sa \ref APPGEN_Plugin_create
 */
RTI_PRIVATE void
APPGEN_Plugin_delete(struct APPGEN_Plugin *appgen)
{
    if (appgen != NULL)
    {
        APPGEN_Plugin_finalize(appgen);
        OSAPI_Heap_free_struct(appgen);
    }
}
#endif /* !RTI_CERT */

/*ci
 * \brief Verifies that the entity management configuration is correct.
 *
 * \param[in] entity_mgnt Pointer to structure with function pointers to
 *                        create/delete DDS entities.
 *
 * \return RTI_TRUE if configuration is correct. RTI_FALSE in case any of the
 * pointers is NULL.
 */
RTI_PRIVATE RTI_BOOL
APPGEN_EntityManager_consistent(const struct DDS_AppGen_EntityManager *entity_manager)
{
    RTI_BOOL ret_value = RTI_FALSE;

    if (entity_manager == NULL)
    {
        APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                          "pointer to DDS management functions cannot be NULL")
        goto done;
    }
    if (entity_manager->create_participant == NULL)
    {
        APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                   "create_participant pointer cannot be NULL")
        goto done;
    }
    if (entity_manager->create_publisher == NULL)
    {
        APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                     "create_publisher pointer cannot be NULL")
        goto done;
    }
    if (entity_manager->create_subscriber == NULL)
    {
        APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                    "create_subscriber pointer cannot be NULL")
        goto done;
    }
    if (entity_manager->create_datawriter == NULL)
    {
        APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                    "create_datawriter pointer cannot be NULL")
        goto done;
    }
    if (entity_manager->create_datareader == NULL)
    {
        APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                    "create_datareader pointer cannot be NULL")
        goto done;
    }
    if (entity_manager->create_topic == NULL)
    {
        APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                         "create_topic pointer cannot be NULL")
        goto done;
    }
#ifndef RTI_CERT
    if (entity_manager->delete_contained_entities == NULL)
    {
        APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                            "delete_contained_entities pointer cannot be NULL")
        goto done;
    }
    if (entity_manager->delete_participant == NULL)
    {
        APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                          "delete_participant cannot be NULL")
        goto done;
    }
#endif


    ret_value = RTI_TRUE;

done:

    return ret_value;
}

/*ci
 * \brief Verifies that the configuration in a RemoteParticipant model
 * is consistent and without errors
 *
 * \param[in] model RemoteParticipant to check for consistency
 *
 * \return RTI_TRUE in case RemoteParticipantModel is correct or RTI_FALSE
 * in case RemoteParticipantModel is not correct.
 */
RTI_PRIVATE RTI_BOOL
APPGEN_RemoteParticipantModel_consistent(
    const struct APPGEN_RemoteParticipantModel *model)
{
    RTI_BOOL ret_value = RTI_FALSE;
    DDS_UnsignedLong k;

    if (model == NULL)
    {
        APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                          "RemoteParticipantModel is NULL")
        goto done;
    }

    if (model->name == NULL)
    {
        APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                          "remote_participant name is NULL")
        goto done;
    }
    if (((model->remote_publisher_count == 0) &&
            (model->remote_publishers != NULL)) ||
        ((model->remote_publisher_count > 0) &&
            (model->remote_publishers == NULL)))
    {
        APPGEN_LOG_APP_CONFIG_POINTER_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                         "remote_publishers count not correct",
                                         model->remote_publisher_count,
                                         model->remote_publishers)
        goto done;
    }
    if (((model->remote_subscriber_count == 0) &&
            (model->remote_subscribers != NULL)) ||
        ((model->remote_subscriber_count > 0) &&
            (model->remote_subscribers == NULL)))
    {
        APPGEN_LOG_APP_CONFIG_POINTER_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                         "remote_subscribers count not correct",
                                         model->remote_subscriber_count,
                                         model->remote_subscribers)
        goto done;
    }

    for (k = 0; k < model->remote_publisher_count; k++)
    {
        if (model->remote_publishers[k].get_type_plugin == NULL)
        {
            APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                                "publishers.get_type_plugin")
            goto done;
        }
    }

    for (k = 0; k < model->remote_subscriber_count; k++)
    {
        if (model->remote_subscribers[k].get_type_plugin == NULL)
        {
            APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                                "subscribers.get_type_plugin")
            goto done;
        }
    }

    ret_value = RTI_TRUE;

done:

    return ret_value;
}

/*ci
 * \brief Verifies that the configuration in a DataReader model is consistent
 * and without errors
 *
 * \param[in] model DataReaderModel to check for consistency
 *
 * \return RTI_TRUE in case DataReaderModel is correct or RTI_FALSE in case
 * DataReaderModel is not correct.
 */
RTI_PRIVATE RTI_BOOL
APPGEN_DataReaderModel_consistent(
    const struct APPGEN_DataReaderModel *model)
{
    RTI_BOOL ret_value = RTI_FALSE;

    if (model == NULL)
    {
        APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                          "DataReaderModel is NULL")
        goto done;
    }

    if (model->multiplicity == 0)
    {
        APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                          "datareader multiplicity not correct")
        goto done;
    }
    if (model->name == NULL)
    {
        APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                          "datareader name is NULL")
        goto done;
    }
    if (model->topic_name == NULL)
    {
        APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                          "datareader topic_name is NULL")
        goto done;
    }

    ret_value = RTI_TRUE;

done:

    return ret_value;
}
/*ci
 * \brief Verifies that the configuration in a Subscriber model is consistent
 * and without errors
 *
 * \param[in] model SubscriberModel to check for consistency
 *
 * \return RTI_TRUE in case SubscriberModel is correct or RTI_FALSE in case
 * SubscriberModel is not correct.
 */
RTI_PRIVATE RTI_BOOL
APPGEN_SubscriberModel_consistent(
    const struct APPGEN_SubscriberModel *model)
{
    RTI_BOOL ret_value = RTI_FALSE;
    DDS_UnsignedLong j;

    if (model == NULL)
    {
        APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                          "DataReaderModel not correct")
        goto done;
    }

    if (model->multiplicity == 0)
    {
        APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                          "datareader multiplicity is 0")
        goto done;
    }
    if (model->name == NULL)
    {
        APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                          "subscriber name is NULL")
        goto done;
    }
    if (((model->reader_count == 0) && (model->data_readers != NULL)) ||
        ((model->reader_count > 0) && (model->data_readers == NULL)))
    {
        APPGEN_LOG_APP_CONFIG_POINTER_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                          "datareader count not correct",
                                          model->reader_count,
                                          model->data_readers)
        goto done;
    }

    /* writer model correct? */
    for (j = 0; j < model->reader_count; j++)
    {
        if (!APPGEN_DataReaderModel_consistent(&model->data_readers[j]))
        {
            goto done;
        }
    }

    ret_value = RTI_TRUE;

done:

    return ret_value;
}

/*ci
 * \brief Verifies that the configuration in a DataWriter model is consistent
 * and without errors
 *
 * \param[in] model DataWriterModel to check for consistency
 *
 * \return RTI_TRUE in case DataWriterModel is correct or RTI_FALSE in case
 * DataWriterModel is not correct.
 */
RTI_PRIVATE RTI_BOOL
APPGEN_DataWriterModel_consistent(
    const struct APPGEN_DataWriterModel *model)
{
    RTI_BOOL ret_value = RTI_FALSE;

    if (model == NULL)
    {
        APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                          "DataWriterModel is NULL")
        goto done;
    }

    if (model->multiplicity == 0)
    {
        APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                            "datawriter multiplicity == 0")
        goto done;
    }
    if (model->name == NULL)
    {
        APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                          "datawriter name is NULL")
        goto done;
    }
    if (model->topic_name == NULL)
    {
        APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                          "datawriter topic_name is NULL")
        goto done;
    }

    ret_value = RTI_TRUE;

done:

    return ret_value;
}

/*ci
 * \brief Verifies that the configuration in a Publisher model is consistent
 * and without errors
 *
 * \param[in] model PublisherModel to check for consistency
 *
 * \return RTI_TRUE in case PublisherModel is correct or RTI_FALSE in case
 * PublisherModel is not correct.
 */
RTI_PRIVATE RTI_BOOL
APPGEN_PublisherModel_consistent(
    const struct APPGEN_PublisherModel *model)
{
    RTI_BOOL ret_value = RTI_FALSE;
    DDS_UnsignedLong j;

    if (model == NULL)
    {
        APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                          "DomainPublisherModel not correct")
        goto done;
    }

    if (model->multiplicity == 0)
    {
        APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                          "publisher multiplicity is 0")
        goto done;
    }
    if (model->name == NULL)
    {
        APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                          "publisher name is NULL")
        goto done;
    }
    if (((model->writer_count == 0) && (model->data_writers != NULL)) ||
        ((model->writer_count > 0) && (model->data_writers == NULL)))
    {
        APPGEN_LOG_APP_CONFIG_POINTER_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                          "datawriter count not correct",
                                          model->writer_count,
                                          model->data_writers)
        goto done;
    }

    /* datawriter model correct? */
    for (j = 0; j < model->writer_count; j++)
    {
        if (!APPGEN_DataWriterModel_consistent(&model->data_writers[j]))
        {
            goto done;
        }
    }

    ret_value = RTI_TRUE;

done:

    return ret_value;
}

/*ci
 * \brief Verifies that the configuration in a FlowControler model is
 * consistent and without errors
 *
 * \param[in] model FlowControllerModel to check for consistency
 *
 * \return RTI_TRUE in case FlowControllerModel is correct or RTI_FALSE in
 * case FlowControllerModel is not correct.
 */
RTI_PRIVATE RTI_BOOL
APPGEN_CustomFlowControllerModel_consistent(
    const struct APPGEN_CustomFlowControllerModel *model)
{
    RTI_BOOL ret_value = RTI_FALSE;

    if (model == NULL)
    {
        APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                          "FlowControllerModel not correct")
        goto done;
    }

    if (model->name == NULL)
    {
        APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                          "flowcontroller name is NULL")
        goto done;
    }

    ret_value = RTI_TRUE;

done:

    return ret_value;
}

/*ci
 * \brief Verifies that the configuration in a DomainParticipant model is consistent
 * and without errors
 *
 * \param[in] model DomainParticipantModel to check for consistency
 *
 * \return RTI_TRUE in case DomainParticipantModel is correct or RTI_FALSE in case
 * DomainParticipant model is not correct.
 */
RTI_PRIVATE RTI_BOOL
APPGEN_DomainParticipantModel_consistent(
    const struct APPGEN_DomainParticipantModel *model)
{
    DDS_UnsignedLong j;
    const struct ComponentFactoryRegisterModel *register_model;
    const struct ComponentFactoryUnregisterModel *unregister_model;
    RTI_BOOL ret_value = RTI_FALSE;

    if (model == NULL)
    {
        APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                          "DomainParticipantModel not correct")
        goto done;
    }

    /* domain participant factory model correct? */
    if ((model->domain_participant_factory.register_count > 0) &&
        (model->domain_participant_factory.register_components == NULL))
    {
        APPGEN_LOG_APP_CONFIG_POINTER_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                     "register_count not correct",
                     model->domain_participant_factory.register_count,
                     model->domain_participant_factory.register_components)
        goto done;
    }

    if ((model->domain_participant_factory.register_count == 0) &&
        ((model->domain_participant_factory.register_components != NULL)))
    {
        APPGEN_LOG_APP_CONFIG_POINTER_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                     "register_components pointer not correct",
                     model->domain_participant_factory.register_count,
                     model->domain_participant_factory.register_components)
        goto done;
    }

    register_model = model->domain_participant_factory.register_components;
    for (j = 0;
         j < model->domain_participant_factory.register_count;
         j++, register_model++)
    {
        /* listener and properties to register can be NULL */
        if ((register_model->register_intf == NULL) ||
            (register_model->register_name == NULL))
        {
            APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                           "register intf or name not correct")
            goto done;
        }
    }

    if ((model->domain_participant_factory.unregister_count == 0) &&
        ((model->domain_participant_factory.unregister_components != NULL)))
    {
        APPGEN_LOG_APP_CONFIG_POINTER_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                     "unregister_components pointer not correct",
                     model->domain_participant_factory.unregister_count,
                     model->domain_participant_factory.unregister_components)
        goto done;
    }

    unregister_model = model->domain_participant_factory.unregister_components;
    for (j = 0;
         j < model->domain_participant_factory.unregister_count;
         j++, unregister_model++)
    {
        /* listener and properties to unregister can be NULL */
        if (unregister_model->unregister_name == NULL)
        {
            APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                           "register name is NULL")
            goto done;
        }
    }

    if (model->name == NULL)
    {
        APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                          "participant name not corect")
        goto done;
    }
    if (((model->topic_count == 0) && (model->topics != NULL)) ||
        ((model->topic_count > 0) && (model->topics == NULL)))
    {
        APPGEN_LOG_APP_CONFIG_POINTER_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                     "topic_count not correct",
                     model->topic_count,
                     model->topics)
        goto done;
    }
    if (((model->type_registration_count == 0) &&
         (model->type_registrations != NULL)) ||
        ((model->type_registration_count > 0) &&
         (model->type_registrations == NULL)))
    {
        APPGEN_LOG_APP_CONFIG_POINTER_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                     "type_registration_count not correct",
                     model->type_registration_count,
                     model->type_registrations)
        goto done;
    }
    if (((model->remote_participant_count == 0) &&
         (model->remote_participants != NULL)) ||
        ((model->remote_participant_count > 0) &&
         (model->remote_participants == NULL)))
    {
        APPGEN_LOG_APP_CONFIG_POINTER_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                     "remote_participant_count not correct",
                     model->remote_participant_count,
                     model->remote_participants)
        goto done;
    }
    if (((model->publisher_count == 0) && (model->publishers != NULL)) ||
        ((model->publisher_count > 0) && (model->publishers == NULL)))
    {
        APPGEN_LOG_APP_CONFIG_POINTER_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                     "publisher_count not correct",
                     model->publisher_count,
                     model->publishers)
        goto done;
    }
    if (((model->subscriber_count == 0) && (model->subscribers != NULL)) ||
        ((model->subscriber_count > 0) && (model->subscribers == NULL)))
    {
        APPGEN_LOG_APP_CONFIG_POINTER_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                     "subscriber_count not correct",
                     model->subscriber_count,
                     model->subscribers)
        goto done;
    }

    if (((model->custom_flow_controller_count == 0) &&
         (model->custom_flow_controllers != NULL)) ||
        ((model->custom_flow_controller_count > 0) &&
         (model->custom_flow_controllers == NULL)))
    {
        APPGEN_LOG_APP_CONFIG_POINTER_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                     "custom_flow_controller_count not correct",
                     model->custom_flow_controller_count,
                     model->custom_flow_controllers)
        goto done;
    }

    /* type registration model correct? */
    for (j = 0; j < model->type_registration_count; j++)
    {
        const struct APPGEN_TypeRegistrationModel *type_registration;

        type_registration = model->type_registrations + j;

        if ((type_registration->get_type_plugin == NULL) ||
            (type_registration->type_name == NULL))
        {
            APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                              "type_registrations")
            goto done;
        }
    }

    /* topic model correct? */
    for (j = 0; j < model->topic_count; j++)
    {
        const struct APPGEN_TopicModel *topic;

        topic = model->topics + j;

        if ((topic->name == NULL) || (topic->type_name == NULL))
        {
            APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,"topics")
            goto done;
        }
    }

    /* publisher model correct? */
    for (j = 0; j < model->publisher_count; j++)
    {
        if (!APPGEN_PublisherModel_consistent(&model->publishers[j]))
        {
            goto done;
        }
    }

    /* subscriber model correct? */
    for (j = 0; j < model->subscriber_count; j++)
    {
        if (!APPGEN_SubscriberModel_consistent(&model->subscribers[j]))
        {
            goto done;
        }
    }

    /* remote participants model correct? */
    for (j = 0; j < model->remote_participant_count; j++)
    {
        if (!APPGEN_RemoteParticipantModel_consistent(
                 &model->remote_participants[j]))
        {
            goto done;
        }
    }

    /* flow controller model correct? */
    for (j = 0; j < model->custom_flow_controller_count; j++)
    {
        if (!APPGEN_CustomFlowControllerModel_consistent(
                 &model->custom_flow_controllers[j]))
        {
            goto done;
        }
    }

    ret_value = RTI_TRUE;

done:

    return ret_value;
}

/*ci
 * \brief Verifies that the configuration in a library model is consistent
 * and without errors
 *
 * \param[in] model Library model to check for consistency
 *
 * \return RTI_TRUE in case library model is correct or RTI_FALSE in case
 * library model is not correct.
 */
RTI_PRIVATE RTI_BOOL
APPGEN_LibraryModel_consistent(const struct APPGEN_LibraryModel *model)
{
    RTI_BOOL ret_value = RTI_FALSE;
    DDS_UnsignedLong i;

    /* model and names correct? */
    if (model == NULL)
    {
        APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                          "library model cannot be NULL")
        goto done;
    }

    if (model->library_name == NULL)
    {
        APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                         "library_name cannot be NULL")
        goto done;
    }

    /* it is ok to have 0 participants as long as the pointer to participant
     * model is also null. the same will apply for publishers, subscribers, etc.
     */
    if (((model->participant_count == 0) && (model->participants != NULL)) ||
        ((model->participant_count > 0) && (model->participants == NULL)))
    {
        APPGEN_LOG_APP_CONFIG_POINTER_NOT_CORRECT(OSAPI_LOGKIND_ERROR,
                                  "DomainParticipant not configured correctly",
                                  model->participant_count,
                                  model->participants)
        goto done;
    }

    /* participant model correct? */
    for (i = 0; i < model->participant_count; i++)
    {
        if (!APPGEN_DomainParticipantModel_consistent(&model->participants[i]))
        {
            goto done;
        }
    }

    ret_value = RTI_TRUE;

done:

    return ret_value;
}

/*ci
 * \brief Verifies that the configuration in an application model is consistent
 * and without errors
 *
 * \param[in] model Library model sequence to check for consistency
 *
 * \return RTI_TRUE in case application model is correct or RTI_FALSE in case
 * appliation model is not correct.
 */
RTI_PRIVATE RTI_BOOL
APPGEN_LibraryModelSeq_consistent(const struct APPGEN_LibraryModelSeq *model)
{
    RTI_BOOL ret_value = RTI_FALSE;
    RTI_INT32 i;
    const struct APPGEN_LibraryModel *library_model = NULL;
    DDS_Long library_length;

    /* model and names correct? */
    if (model == NULL)
    {
        APPGEN_LOG_APP_CONFIG_NOT_CORRECT(OSAPI_LOGKIND_ERROR,"model == NULL")
        goto done;
    }

    library_length = APPGEN_LibraryModelSeq_get_length(model);
    for (i = 0; i < library_length; i++)
    {
        library_model = APPGEN_LibraryModelSeq_get_reference(model, i);

        if (!APPGEN_LibraryModel_consistent(library_model))
        {
            goto done;
        }
    }

    ret_value = RTI_TRUE;

done:

    return ret_value;
}

/*ci
 * \brief Gets two names in a fully qualified name
 *
 * \param[in] qualified_name Fully qualified name, e.g. "Name1::Name2"
 *            This will be usually a library name, followed by two ':'
 *            and finished by and entity name.
 * \param[out] Length of the library name (the name before the first ':'
 *             character. Has valid value only if function returns RTI_TRUE.
 * \param[out] Pointer to the entity name (the name after the second ':'
 *             character. This string is terminated by character '\0'.
 *             Has valid value only if function returns RTI_TRUE.
 *
 * \return RTI_TRUE in case two names separated by "::" are found in the input
 *         parameter 'qualified_name'. RTI_FALSE in case of error.
 */
RTI_PRIVATE RTI_BOOL
APPGEN_Plugin_split_qualified_name(const char *qualified_name,
                                   RTI_SIZE_T *lib_name_len,
                                   const char **entity_name)
{
    RTI_SIZE_T len = OSAPI_String_length(qualified_name);
    const char *aux = NULL;

    aux = OSAPI_Memory_fndchr((const void *)qualified_name,
                              (RTI_INT32)':', len);
    if (aux != NULL)
    {
        *lib_name_len = (RTI_SIZE_T)(aux - qualified_name);
        aux++;
        if ((*aux) == ':')
        {
            aux++;
        }
    }

    *entity_name = aux;

    return (aux == NULL) ? RTI_FALSE : RTI_TRUE;
}

/*ci
 * \brief Generates an entity name according to rules:
 *         - Name inside parameter 'entity_name' is used in case
 *         it is not an empty string. In case 'entity_name' contains
 *         and empty string, 'model_name' is copied in 'entity_name'
 *         - Name from previous rule is concatenated with string "#i", being i
 *         the value of input parameter 'qualified_name', but this is done
 *         only case this value is greater than 1.
 *
 * \param[in/out] entity_name Entity name. As input it has an entity name.
 *                As output is has the generated entity name using all
 *                input parameters.
 * \param[in] model_name Name to copy into 'entity_name' in case 'entity_name'
 *            is the empty string.
 * \param[in] multiplicity_index Multiplicity index of the entity name to
 *            generate. Starts at 0.
 *
 * \return RTI_TRUE in case the name can be generated or RTI_FALSE in case of
 *         error. Error might happen in case the name to generate does not fit
 *         inside an 'DDS_EntityNameQosPolicy' structure.
 */
RTI_PRIVATE RTI_BOOL
APPGEN_Plugin_generate_entity_name(
        struct DDS_EntityNameQosPolicy *entity_name,
        const char *model_name,
        DDS_UnsignedLong multiplicity_index)
{
    RTI_BOOL ret_value = RTI_FALSE;

    /* fist step : if the name in the qos is empty string and
     * the name in the model name is not an empty string, use
     * the name in the model
     */
    RTI_SIZE_T len = OSAPI_String_length(entity_name->name);
    RTI_SIZE_T len2 = OSAPI_String_length(model_name);
    RTI_SIZE_T max_len = (RTI_SIZE_T)sizeof(entity_name->name) /
                         (RTI_SIZE_T)sizeof(entity_name->name[0]);

    if ((len == 0) && (len2 > 0))
    {
        if (len2 >= max_len)
        {
            APPGEN_LOG_COPY_ENTITY_NAME(OSAPI_LOGKIND_ERROR, model_name)
            return RTI_FALSE;
        }
        else
        {
            OSAPI_Memory_copy(entity_name->name, model_name, len2 + 1);
            len = len2;
        }
    }

    /* second step : add #i (with i = multiplicity_index) in case
     * multiplicity_index is greater than 0.
     */
    if (multiplicity_index > 0)
    {
        /* enough space to add '#' ? */
        if ((len + 1) < max_len)
        {
            if (OSAPI_Log_itoa(
                    entity_name->name + len + 1,
                    max_len - (len + 1),
                    (RTI_INT32)multiplicity_index) < (max_len - (len + 1)))
            {
                entity_name->name[len] = '#';
                ret_value = RTI_TRUE;
            }
            else
            {
                APPGEN_LOG_CREATE_ENTITY_NAME(OSAPI_LOGKIND_ERROR,
                                              model_name,
                                              multiplicity_index)
            }
        }
        else
        {
            APPGEN_LOG_CREATE_ENTITY_NAME(OSAPI_LOGKIND_ERROR,
                                          model_name,
                                          multiplicity_index)
        }
    }
    else if (multiplicity_index == 0)
    {
        /* for the first entity leave name as it is */
        ret_value = RTI_TRUE;
    }
    else
    {
        /* multiplicity cannot be negative */
        APPGEN_MULTIPLICITY(OSAPI_LOGKIND_ERROR, multiplicity_index)
        ret_value = RTI_FALSE;
    }

    return ret_value;
}

/*ci
 * \brief Unregisters all factories as indicated in the model configuration;
 *        registers all factories as indicated in the model configuration;
 *        finally set the domain participant factory QoS using the properties
 *        in the model configuration.
 *
 * \param[in] appgen App Generation pointer
 * \param[in] factory Pointer to Domain Participant Factory.
 * \param[in] model Pointer to the model with the domain participant
 *            configuration.
 *
 * \return RTI_TRUE in case no error.
 *
 * \sa \ref APPGEN_Plugin_finalize_component_factories
 */
RTI_PRIVATE RTI_BOOL
APPGEN_Plugin_initialize_component_factories(
        struct APPGEN_Plugin *appgen,
        DDS_DomainParticipantFactory *factory,
        const struct APPGEN_DomainParticipantFactoryModel *model)
{
    DDS_UnsignedLong i;
    RT_Registry_T *registry = NULL;
    DDS_ReturnCode_t ret_code = DDS_RETCODE_OK;
    const struct ComponentFactoryRegisterModel *register_model;
    const struct ComponentFactoryUnregisterModel *unregister_model;
    RTI_BOOL ret_value = RTI_FALSE;
    struct DDS_DomainParticipantFactoryQos factory_qos;

    registry = DDS_DomainParticipantFactory_get_registry(factory);

    /* The unregister names is used only to unregister the udp transport
     * that the participant factory is registering by default. So this
     * needs to be done only if we did not create any domain participant
     * already.
     */
    if (appgen->factory->number_participants == 0)
    {
        unregister_model = model->unregister_components;

        for (i = 0; i < model->unregister_count; i++, unregister_model++)
        {
            /* Do not be too strict and unregister a factory only
             * if it is already registered */
            if ((RT_Registry_lookup(registry,
                                    unregister_model->unregister_name) != NULL))
            {
                if (!RT_Registry_unregister(
                         registry,
                         unregister_model->unregister_name,
                         (struct RT_ComponentFactoryProperty **)unregister_model->unregister_property,
                         (struct RT_ComponentFactoryListener **)unregister_model->unregister_listener))
                {
                    APPGEN_FACTORY_UNREGISTER(OSAPI_LOGKIND_ERROR,
                                              unregister_model->unregister_name)
                    goto done;
                }
            }
        }
    }

    register_model = model->register_components;
    for (i = 0; i < model->register_count; i++, register_model++)
    {
        /* it is allowed that different domain participants share the
         * same factories, so factories are registered only in case that
         * a factory with the same name is not registered already
         */
        if ((RT_Registry_lookup(registry,
                                register_model->register_name) == NULL))
        {
            if (!RT_Registry_register(registry,
                                      register_model->register_name,
                                      register_model->register_intf(),
                                      (struct RT_ComponentFactoryProperty *)register_model->register_property,
                                      (struct RT_ComponentFactoryListener *)register_model->register_listener))
            {
                APPGEN_FACTORY_REGISTER(OSAPI_LOGKIND_ERROR,
                                        register_model->register_name)
                goto done;
            }
        }
    }

    /* At the moment it is possible to configure only by XML
     * autoenable_created_entities. So, we need to get the factory QoS,
     * change autoenable_created_entities and set it again
     */
    ret_code = DDS_DomainParticipantFactory_get_qos(factory, &factory_qos);
    if (ret_code != DDS_RETCODE_OK)
    {
        APPGEN_LOG_API_ERROR(OSAPI_LOGKIND_ERROR, ret_code)
        goto done;
    }

    factory_qos.entity_factory.autoenable_created_entities =
        model->factory_qos.entity_factory.autoenable_created_entities;

    ret_code =
        DDS_DomainParticipantFactory_set_qos(factory, &factory_qos);

    if (ret_code != DDS_RETCODE_OK)
    {
        APPGEN_LOG_API_ERROR(OSAPI_LOGKIND_ERROR, ret_code)
        goto done;
    }

    /* Try to register reader and writer history. They may be already
     * registered if a participant was already created, etc.
     */
    if ((RT_Registry_lookup(registry,
                            DDSHST_WRITER_DEFAULT_HISTORY_NAME) == NULL))
    {
        if (!RT_Registry_register(registry, DDSHST_WRITER_DEFAULT_HISTORY_NAME,
                                  WHSM_HistoryFactory_get_interface(), NULL, NULL))
        {
            APPGEN_FACTORY_REGISTER(OSAPI_LOGKIND_ERROR,
                                    DDSHST_WRITER_DEFAULT_HISTORY_NAME)
            goto done;
        }

        appgen->factory->registered_writer_history = RTI_TRUE;
    }

    if (RT_Registry_lookup(registry,
                           DDSHST_READER_DEFAULT_HISTORY_NAME) == NULL)
    {
        if (!RT_Registry_register(registry, DDSHST_READER_DEFAULT_HISTORY_NAME,
                                  RHSM_HistoryFactory_get_interface(), NULL, NULL))
        {
            APPGEN_FACTORY_REGISTER(OSAPI_LOGKIND_ERROR,
                                    DDSHST_READER_DEFAULT_HISTORY_NAME)
            goto done;
        }

        appgen->factory->registered_reader_history = RTI_TRUE;
    }

    ret_value = RTI_TRUE;

done:

    return ret_value;
}

#ifndef RTI_CERT
/*ci
 * \brief Unregisters all factories in the model configuration.
 * It is ok if some of the factories in the model configuration
 * have not been previously registered, e.g. in case of error.
 *
 * \param[in] factory Pointer to Domain Participant Factory.
 * \param[in] model Pointer to the model with the domain participant
 *            configuration.
 *
 * \return RTI_TRUE in case no error.
 *
 * \sa \ref APPGEN_Plugin_initialize_component_factories
 */
RTI_PRIVATE RTI_BOOL
APPGEN_Plugin_finalize_component_factories(
    DDS_DomainParticipantFactory *factory,
    const struct APPGEN_DomainParticipantFactoryModel *model)
{
    DDS_UnsignedLong i;
    RT_Registry_T *registry;
    const char *component_name;
    const struct ComponentFactoryRegisterModel *register_model;
    RTI_BOOL ret_value = RTI_FALSE;

    registry = DDS_DomainParticipantFactory_get_registry(factory);

    register_model = model->register_components;
    for (i = 0; i < model->register_count; i++, register_model++)
    {
        /* ensure that the factory is registered before unregistering it
         * to avoid false error/warning logs
         */
        component_name = register_model->register_name;

        if (RT_Registry_lookup(registry, component_name) == NULL)
        {
            continue;
        }

        if (!RT_Registry_unregister(registry, component_name, NULL, NULL))
        {
            APPGEN_FACTORY_UNREGISTER(OSAPI_LOGKIND_ERROR, component_name)
            goto done;
        }
    }

    ret_value = RTI_TRUE;

done:
    return ret_value;
}
#endif

/*ci
 * \brief Creates all data writers as configured in the data model
 *
 * \param[in] appgen Pointer to application generation plugin
 * \param[in] participant Participant where the data writers will be created
 * \param[in] publisher Publisher where the data writers will be created
 * \param[in] model Data model with configuration needed to create the data
 *                  writers
 *
 * \warning This function has a local variable of type struct DDS_DataWriterQos.
 * This structure can use quite a lot of stack memory, e.g. it contains an
 * entity name which consists of 255 chars.
 * \warning Do not call this function from another function which also has
 * local variables which use big amount of memory.
 *
 * \return RTI_TRUE in case no error.
 */
RTI_PRIVATE RTI_BOOL
APPGEN_Plugin_create_datawriters_from_model(
    struct APPGEN_Plugin *appgen,
    DDS_DomainParticipant *participant,
    DDS_Publisher *publisher,
    const struct APPGEN_PublisherModel *model)
{
    DDS_UnsignedLong i, j;
    RTI_BOOL ret_val = RTI_FALSE;

    for (i = 0; i < model->writer_count; i++)
    {
        const struct APPGEN_DataWriterModel *datawriter_model =
            &model->data_writers[i];
        DDS_Topic *topic;

        topic = DDS_Topic_narrow(
                    DDS_DomainParticipant_lookup_topicdescription(
                        participant, datawriter_model->topic_name));
        if (topic == NULL)
        {
            APPGEN_LOG_FACTORY_TOPIC_NOT_FOUND(OSAPI_LOGKIND_ERROR,
                                               model->data_writers[i].topic_name)
            goto done;
        }

        for (j = 0; j < datawriter_model->multiplicity; j++)
        {
            struct DDS_DataWriterQos qos;
            DDS_DataWriter *writer;

            /* we do not need to call DomainParticipantQos_copy() because
             * we just need an exact copy with the correct participant name.
             * Calling copy() function would allocated memory for all
             * sequences, etc.
             */
            qos = datawriter_model->writer_qos;

            if (!APPGEN_Plugin_generate_entity_name(&qos.publication_name,
                                                    datawriter_model->name,
                                                    j))
            {
                /* log error added in function which returned the error */
                goto done;
            }

            writer = appgen->property.entity_manager.create_datawriter
                                                     (publisher,
                                                      topic,
                                                      &qos,
                                                      NULL,
                                                      DDS_STATUS_MASK_NONE);
            if (writer == NULL)
            {
                APPGEN_LOG_DDS_ENTITY(OSAPI_LOGKIND_ERROR)
                goto done;
            }
        }
    }

    ret_val = RTI_TRUE;

done:

    return ret_val;
}

/*ci
 * \brief Creates all data readers as configured in the data model
 *
 * \param[in] appgen Pointer to application generation plugin
 * \param[in] participant Participant where the data writers will be created
 * \param[in] subscriber Subscriber where the data writers will be created
 * \param[in] model Data model with configuration needed to create the data
 *                  writers
 *
 * \warning This function has a local variable of type struct DDS_DataReaderQos.
 * This structure can use quite a lot of stack memory, e.g. it contains an
 * entity name which consists of 255 chars.
 * \warning Do not call this function from another function which also has
 * local variables which use big amount of memory.
 *
 * \return RTI_TRUE in case no error.
 */
RTI_PRIVATE RTI_BOOL
APPGEN_Plugin_create_datareaders_from_model(
    struct APPGEN_Plugin *appgen,
    DDS_DomainParticipant *participant,
    DDS_Subscriber *subscriber,
    const struct APPGEN_SubscriberModel *model)
{
    DDS_UnsignedLong i, j;
    RTI_BOOL ret_val = RTI_FALSE;

    for (i = 0; i < model->reader_count; i++)
    {
        const struct APPGEN_DataReaderModel *datareader_model =
            &model->data_readers[i];
        DDS_TopicDescription *topic;

        topic = DDS_DomainParticipant_lookup_topicdescription(
                                            participant,
                                            datareader_model->topic_name);
        if (topic == NULL)
        {
            APPGEN_LOG_FACTORY_TOPIC_NOT_FOUND(OSAPI_LOGKIND_ERROR,
                                               datareader_model->topic_name)
            goto done;
        }

        for (j = 0; j < datareader_model->multiplicity; j++)
        {
            struct DDS_DataReaderQos qos;
            DDS_DataReader *reader;

            /* we do not need to call DomainParticipantQos_copy() because
             * we just need an exact copy with the correct participant name.
             * Calling copy() function would allocated memory for all
             * sequences, etc.
             */
            qos = datareader_model->reader_qos;

            if (!APPGEN_Plugin_generate_entity_name(&qos.subscription_name,
                                                    datareader_model->name,
                                                    j))
            {
                /* log error added in function which returned the error */
                goto done;
            }

            reader = appgen->property.entity_manager.create_datareader
                                                     (subscriber,
                                                      topic,
                                                      &qos,
                                                      NULL,
                                                      DDS_STATUS_MASK_NONE);
            if (reader == NULL)
            {
                APPGEN_LOG_DDS_ENTITY(OSAPI_LOGKIND_ERROR)
                goto done;
            }
        }
    }

    ret_val = RTI_TRUE;

done:

    return ret_val;
}

/*ci
 * \brief Register all types in a participant as configured in the model
 *
 * \param[in] appgen Pointer to application generation plugin
 * \param[in] participant Local participant where types will be registered
 * \param[in] model Data model with configuration needed to register all types
 *
 * \return DDS_RETCODE_OK if no error, or error code in case there was an error.
 */
RTI_PRIVATE DDS_ReturnCode_t
APPGEN_Plugin_register_types_from_model(
    DDS_DomainParticipant *participant,
    const struct APPGEN_DomainParticipantModel *model)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;
    DDS_UnsignedLong i;

    for (i = 0; i < model->type_registration_count; i++)
    {
        retcode = DDS_DomainParticipant_register_type(participant,
                         model->type_registrations[i].type_name,
                         model->type_registrations[i].get_type_plugin());
        if (retcode != DDS_RETCODE_OK)
        {
            APPGEN_LOG_API_ERROR(OSAPI_LOGKIND_ERROR, retcode)
            retcode = DDS_RETCODE_ERROR;
            goto done;
        }
    }

done:

    return retcode;
}

/*ci
 * \brief Creates all flowcontrollers as configured in the model
 *
 * \param[in] participant Local participant where flow controllers will
 *                        be created
 * \param[in] model Data model with configuration needed to create all flow
 *                  controllers
 *
 * \return DDS_RETCODE_OK if no error, or error code in case there was an error.
 */
RTI_PRIVATE DDS_ReturnCode_t
APPGEN_Plugin_create_flowcontrollers_from_model
    (DDS_DomainParticipant *participant,
     const struct APPGEN_DomainParticipantModel *model)
{
#if DDS_FLOW_CONTROLLER_ENABLED

    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;
    DDS_UnsignedLong i;

    for (i = 0; i < model->custom_flow_controller_count; i++)
    {
        DDS_FlowController *flow_controller;

        flow_controller =
            DDS_DomainParticipant_create_flowcontroller(
                participant,
                model->custom_flow_controllers[i].name,
                &model->custom_flow_controllers[i].flow_controller_property);
        if (flow_controller == NULL)
        {
            APPGEN_LOG_DDS_ENTITY(OSAPI_LOGKIND_ERROR)
            retcode = DDS_RETCODE_ERROR;
            goto done;
        }
    }

done:

    return retcode;
#else
    UNUSED_ARG(participant);
    UNUSED_ARG(model);

    return DDS_RETCODE_UNSUPPORTED;
#endif
}

/*ci
 * \brief Registers content filters as configured in
 * the model
 *
 * \param[in] participant Local participant where
 *     content filters will be registered
 * \param[in] model Data model with configuration
 *     needed to register all content filters
 *
 * \return DDS_RETCODE_OK if no error, or error code
 * in case there was an error.
 */
RTI_PRIVATE DDS_ReturnCode_t
APPGEN_Plugin_register_contentfilters_from_model(
    DDS_DomainParticipant *participant,
    const struct APPGEN_DomainParticipantModel *model)
{
#if DDS_FILTERING_ENABLED
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;
    DDS_UnsignedLong i;

    for (i = 0; i < model->content_filter_registration_count; i++)
    {
        const struct APPGEN_ContentFilterRegistration *reg =
                &model->content_filter_registrations[i];
        const struct DDS_ContentFilterI *intf = reg->get_filter_intf();

        retcode = DDS_DomainParticipant_register_contentfilter(
                participant,
                reg->filter_class_name,
                intf,
                reg->filter_property);
        if (retcode != DDS_RETCODE_OK)
        {
            APPGEN_LOG_DDS_ENTITY(OSAPI_LOGKIND_ERROR)
            goto done;
        }
    }

done:
    return retcode;
#else
    UNUSED_ARG(participant);
    UNUSED_ARG(model);
    return DDS_RETCODE_OK;
#endif
}

/*ci
 * \brief Creates all topics a participant as configured in the model
 *
 * \param[in] appgen Pointer to application generation plugin
 * \param[in] participant Local participant where topics will be created
 * \param[in] model Data model with configuration needed to create all topcis
 *
 * \return DDS_RETCODE_OK if no error, or error code in case there was an error.
 */
RTI_PRIVATE DDS_ReturnCode_t
APPGEN_Plugin_create_topics_from_model
    (struct APPGEN_Plugin *appgen,
     DDS_DomainParticipant *participant,
     const struct APPGEN_DomainParticipantModel *model)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;
    DDS_UnsignedLong i;

    for (i = 0; i < model->topic_count; i++)
    {
        DDS_Topic *topic = appgen->property.entity_manager.create_topic(
                                    participant,
                                    model->topics[i].name,
                                    model->topics[i].type_name,
                                    &model->topics[i].topic_qos,
                                    NULL,
                                    DDS_STATUS_MASK_NONE);
        if (topic == NULL)
        {
            APPGEN_LOG_DDS_ENTITY(OSAPI_LOGKIND_ERROR)
            retcode = DDS_RETCODE_ERROR;
            goto done;
        }
    }

done:

    return retcode;
}

/*ci
 * \brief Creates a publisher with QoS specified as parameter
 *
 * \param[in] appgen Pointer to application generation plugin
 * \param[in] participant Local participant where the publisher will be created
 * \param[in] model Data model with configuration needed to create the publisher
 *
 * \warning This function has a local variable of type struct DDS_PublisherQos.
 * This structure can use quite a lot of stack memory, e.g. it contains an
 * entity name which consists of 255 chars.
 * \warning Do not call this function from another function which also has
 * local variables which use big amount of memory.
 *
 * \return Pointer to publisher or NULL in case of error.
 */
RTI_PRIVATE DDS_Publisher*
APPGEN_Plugin_create_publisher_with_qos
    (struct APPGEN_Plugin *appgen,
     DDS_DomainParticipant *participant,
     const struct APPGEN_PublisherModel *model,
     DDS_UnsignedLong multiplicity)
{
    struct DDS_PublisherQos qos;
    DDS_Publisher *publisher = NULL;

    /* we do not need to call DomainParticipantQos_copy() because we just need
     * an exact copy with the correct participant name. Calling copy() function
     * would allocated memory for all sequences, etc.
     */
    qos = model->publisher_qos;

    if (!APPGEN_Plugin_generate_entity_name(&qos.publisher_name,
                                            model->name,
                                            multiplicity))
    {
        goto done;
    }

    publisher = appgen->property.entity_manager.create_publisher(
                    participant, &qos, NULL, DDS_STATUS_MASK_NONE);

done:

    return publisher;
}

/*ci
 * \brief Creates a subscriber with QoS specified as parameter
 *
 * \param[in] appgen Pointer to application generation plugin
 * \param[in] participant Local participant where the subscriber will be created
 * \param[in] model Data model with configuration needed to create the subscriber
 *
 * \warning This function has a local variable of type struct DDS_SubscriberQos.
 * This structure can use quite a lot of stack memory, e.g. it contains an
 * entity name which consists of 255 chars.
 * \warning Do not call this function from another function which also has
 * local variables which use big amount of memory.
 *
 * \return Pointer to subscriber or NULL in case of error.
 */
RTI_PRIVATE DDS_Subscriber*
APPGEN_Plugin_create_subscriber_with_qos
    (struct APPGEN_Plugin *appgen,
     DDS_DomainParticipant *participant,
     const struct APPGEN_SubscriberModel *model,
     DDS_UnsignedLong multiplicity)
{
    struct DDS_SubscriberQos qos;
    DDS_Subscriber *subscriber = NULL;

    /* we do not need to call DomainParticipantQos_copy() because we just need
     * an exact copy with the correct participant name. Calling copy() function
     * would allocated memory for all sequences, etc.
     */
    qos = model->subscriber_qos;

    if (!APPGEN_Plugin_generate_entity_name(&qos.subscriber_name,
                                            model->name,
                                            multiplicity))
    {
        goto done;
    }

    subscriber = appgen->property.entity_manager.create_subscriber(
                     participant, &qos, NULL, DDS_STATUS_MASK_NONE);

done:

    return subscriber;
}

/*ci
 * \brief Assert remote participants, writers and readers as configured in the
 * model.
 *
 * \param[in] participant Local participant that performs the assertions
 * \param[in] model Data model with configuration needed to assert all DDS
 *            entities
 *
 * \return DDS_RETCODE_OK if no error, or error code in case there was an error.
 */
RTI_PRIVATE DDS_ReturnCode_t
APPGEN_Plugin_assert_remote_entities_from_model(
    DDS_DomainParticipant *participant,
    const struct APPGEN_DomainParticipantModel *model)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_UnsignedLong i, j;
    const struct APPGEN_RemoteParticipantModel *remote_participant;

    /* assert remote participants, writers and readers */
    remote_participant = model->remote_participants;
    for (i = 0; i < model->remote_participant_count; i++, remote_participant++)
    {
        retcode = DPSE_RemoteParticipant_assert(
                      participant, remote_participant->name);
        if (retcode != DDS_RETCODE_OK)
        {
            APPGEN_LOG_API_ERROR(OSAPI_LOGKIND_ERROR, retcode)
            goto done;
        }

        for (j = 0; j < remote_participant->remote_publisher_count; j++)
        {
            retcode = DPSE_RemotePublication_assert(participant,
                    remote_participant->name,
                    &remote_participant->remote_publishers[j].publication_data,
                    remote_participant->remote_publishers[j].get_type_plugin()->key_kind);
            if (retcode != DDS_RETCODE_OK)
            {
                APPGEN_LOG_API_ERROR(OSAPI_LOGKIND_ERROR, retcode)
                goto done;
            }
        }

        for (j = 0; j < remote_participant->remote_subscriber_count; j++)
        {
            retcode = DPSE_RemoteSubscription_assert(participant,
                    remote_participant->name,
                    &remote_participant->remote_subscribers[j].subscription_data,
                    remote_participant->remote_subscribers[j].get_type_plugin()->key_kind);
            if (retcode != DDS_RETCODE_OK)
            {
                APPGEN_LOG_API_ERROR(OSAPI_LOGKIND_ERROR, retcode)
                goto done;
            }
        }
    }

    retcode = DDS_RETCODE_OK;

done:

    return retcode;
}

/*ci
 * \brief Create a domain participant with the QoS specified as parameter.
 *
 * \param[in] dpf Domain participant factory
 * \param[in] appgen Pointer to appliation generation plugin
 * \param[in] model Data model with configuration needed to create all DDS
 *            entities
 *
 * \note The main reason to create this function is because it has a variable
 * in the stack which uses a big amount of memory. There a few functions which
 * the same characteristic in this module. This function make possible to not
 * chain several calls to those functions.
 *
 * \warning This function has a local variable of type struct DDS_DomainParticipantQos.
 * This structure can use quite a lot of stack memory, e.g. it contains an
 * entity name which consists of 255 chars.
 *
 * \warning Do not call this function from another function which also has
 * local variables which use big amount of memory.
 *
 * \return Pointer to the domain participant created or NULL if failure.
 */
RTI_PRIVATE DDS_DomainParticipant*
APPGEN_Plugin_create_participant_with_qos(
    DDS_DomainParticipantFactory *dpf,
    struct APPGEN_Plugin *appgen,
    const struct APPGEN_DomainParticipantModel *model)
{
    struct DDS_DomainParticipantQos participant_qos;
    DDS_DomainParticipant *participant = NULL;

    /* we do not need to call DomainParticipantQos_copy() because we just need
     * an exact copy with the correct participant name. Calling copy() function
     * would allocated memory for all sequences, etc.
     */
    participant_qos = model->participant_qos;

    if (!APPGEN_Plugin_generate_entity_name(&participant_qos.participant_name,
                                            model->name, 0))
    {
        goto done;
    }

    /* create participant */
    participant = appgen->property.entity_manager.create_participant(
                                                        dpf,
                                                        model->domain_id,
                                                        &participant_qos,
                                                        NULL,
                                                        DDS_STATUS_MASK_NONE);

done:

    return participant;
}

/*ci
 * \brief Creates all participants, topics, publishers, subscribers, writers
 *  readers and registers all data types as configured in the data model.
 *
 * \param[in] appgen Pointer to appliation generation plugin
 * \param[in] model Data model with configuration needed to create all DDS
 *            entities
 *
 * \return A pointer to the last participant created. In case multiplicity is
 * 1 it will be the only participant created. NULL in case of any error.
 */
RTI_PRIVATE DDS_DomainParticipant*
APPGEN_Plugin_create_domainparticipant_from_model(
    struct APPGEN_Plugin *appgen,
    const struct APPGEN_DomainParticipantModel *model)
{
    DDS_DomainParticipant *participant;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_DomainParticipantFactory *dpf;
    DDS_UnsignedLong i, j;

    dpf = DDS_DomainParticipantFactory_get_instance();

    /* Note: functions APPGEN_Plugin_create_domainparticipant_with_qos(),
     * APPGEN_Plugin_create_publisher_with_qos(),
     * APPGEN_Plugin_create_subscriber_with_qos()
     * APPGEN_Plugin_create_writers_from_model() and
     * APPGEN_Plugin_create_readers_from_model()
     * have large variables in the stack. Those function call should not be
     * chained. Those large variables are the DomainParticipant QoS,
     * Publisher QoS, etc. Note that all of them have an entity name which
     * is 256 bytes long.
     */
    participant = APPGEN_Plugin_create_participant_with_qos(dpf, appgen, model);
    if (participant == NULL)
    {
        APPGEN_LOG_DDS_ENTITY(OSAPI_LOGKIND_ERROR)
        retcode = DDS_RETCODE_ERROR;
        goto done;
    }

    /* Create flow controllers */
    retcode = APPGEN_Plugin_create_flowcontrollers_from_model(participant, model);
    if (retcode != DDS_RETCODE_OK)
    {
        /* no need to log the error. called function already did it */
        goto done;
    }

    /* Register content filters */
    retcode = APPGEN_Plugin_register_contentfilters_from_model(participant, model);
    if (retcode != DDS_RETCODE_OK)
    {
        goto done;
    }

    /* Register types */
    retcode = APPGEN_Plugin_register_types_from_model(participant, model);
    if (retcode != DDS_RETCODE_OK)
    {
        /* no need to log the error. called function already did it */
        goto done;
    }

    /* Create topics */
    retcode = APPGEN_Plugin_create_topics_from_model(appgen, participant, model);
    if (retcode != DDS_RETCODE_OK)
    {
        /* no need to log the error. called function already did it */
        goto done;
    }

    /* create publishers and writers */
    for (i = 0; i < model->publisher_count; i++)
    {
        for (j = 0; j < model->publishers[i].multiplicity; j++)
        {
            DDS_Publisher *publisher =
                        APPGEN_Plugin_create_publisher_with_qos(
                            appgen, participant, &model->publishers[i], j);
            if (publisher == NULL)
            {
                APPGEN_LOG_DDS_ENTITY(OSAPI_LOGKIND_ERROR)
                retcode = DDS_RETCODE_ERROR;
                goto done;
            }

            if (!APPGEN_Plugin_create_datawriters_from_model(
                     appgen, participant, publisher, &model->publishers[i]))
            {
                /* log error added in function which returned the error */
                retcode = DDS_RETCODE_ERROR;
                goto done;
            }
        }
    }

    /* create subscribers and readers */
    for (i = 0; i < model->subscriber_count; i++)
    {
        for (j = 0; j < model->subscribers[i].multiplicity; j++)
        {
            DDS_Subscriber *subscriber =
                        APPGEN_Plugin_create_subscriber_with_qos(
                            appgen, participant, &model->subscribers[i], j);
            if (subscriber == NULL)
            {
                APPGEN_LOG_DDS_ENTITY(OSAPI_LOGKIND_ERROR)
                retcode = DDS_RETCODE_ERROR;
                goto done;
            }

            if (!APPGEN_Plugin_create_datareaders_from_model(
                     appgen, participant, subscriber, &model->subscribers[i]))
            {
                /* log error added in function which returned the error */
                retcode = DDS_RETCODE_ERROR;
                goto done;
            }
        }
    }

    /* assert remote entities if any */
    retcode = APPGEN_Plugin_assert_remote_entities_from_model(participant,
                                                              model);
    if (retcode != DDS_RETCODE_OK)
    {
        /* log error added in function which returned the error */
        goto done;
    }

done:

#ifndef RTI_CERT
    if (retcode != DDS_RETCODE_OK)
    {
        /* Delete participant if it was created
         */
        if (participant != NULL)
        {
            retcode = appgen->property.entity_manager.delete_contained_entities(
                                                                  participant);
#if OSAPI_ENABLE_LOG
            if (retcode != DDS_RETCODE_OK)
            {
                APPGEN_LOG_API_ERROR(OSAPI_LOGKIND_ERROR,retcode)
            }
#else
            IGNORE_RETVAL(retcode);
#endif

            retcode = appgen->property.entity_manager.delete_participant(
                                                             dpf, participant);
#if OSAPI_ENABLE_LOG
            if (retcode != DDS_RETCODE_OK)
            {
                APPGEN_LOG_API_ERROR(OSAPI_LOGKIND_ERROR,retcode)
            }
#else
            IGNORE_RETVAL(retcode);
#endif

            participant = NULL;
        }
    }
#endif /* !RTI_CERT */

    return participant;
}

/* ------------------------------------------------------------------------ */
/*                   Plugin interface                                       */
/* ------------------------------------------------------------------------ */

/*ci
 * \brief Creates a Domain Participant configured in the data model.
 *
 * \param[in] self Pointer to plugin
 * \param[in] configuration_name Fully qualified name of the DomainParticipant to
 *  create. E.g. "LibraryName::ParticipantName"
 *
 * \return Pointer to the DomainParticipant created or NULL in case of error.
 */
RTI_PRIVATE DDS_DomainParticipant*
APPGEN_Plugin_create_participant_from_config(struct DDS_AppGenPlugin *self,
                                             const char *configuration_name)
{
    struct APPGEN_Plugin *appgen = (struct APPGEN_Plugin *)self;
    const struct APPGEN_LibraryModelSeq *model = NULL;
    const struct APPGEN_LibraryModel *library_model = NULL;
    const struct APPGEN_DomainParticipantModel *participant_model = NULL;
    DDS_UnsignedLong i;
    RTI_INT32 j;
    RTI_SIZE_T len;
    const char *part_name;
    DDS_DomainParticipant *participant = NULL;
    DDS_Long library_length;

    OSAPI_PRECONDITION_ALWAYS((self==NULL) || (configuration_name==NULL),
               return NULL,
               OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("name",configuration_name,RTI_TRUE);)

    model = appgen->factory->_model;

    if ((!APPGEN_LibraryModelSeq_consistent(model)) ||
        (!APPGEN_EntityManager_consistent(&appgen->property.entity_manager)))
    {
        goto done;
    }

    if (!APPGEN_Plugin_split_qualified_name(configuration_name,
                                            &len, &part_name))
    {
        APPGEN_LOG_APP_NAME_ERROR(OSAPI_LOGKIND_ERROR, configuration_name)
        goto done;
    }

    /* Search the library model */
    library_length = APPGEN_LibraryModelSeq_get_length(model);
    for (j = 0; j < library_length; j++)
    {
        library_model = APPGEN_LibraryModelSeq_get_reference(model, j);

        if ((library_model != NULL) &&
            (OSAPI_String_length(library_model->library_name) == len) &&
             !OSAPI_String_ncmp(library_model->library_name,
                                configuration_name, len))
        {
            break;
        }
        library_model = NULL;
    }

    if (library_model == NULL)
    {
        APPGEN_LOG_LIB_NOT_FOUND(OSAPI_LOGKIND_ERROR, configuration_name)
        goto done;
    }

    /* Search the participant model */
    for (i = 0; i < library_model->participant_count; ++i)
    {
        if (!OSAPI_String_cmp(library_model->participants[i].name, part_name))
        {
            participant_model = &library_model->participants[i];
            break;
        }
    }

    /* participant model found? */
    if (participant_model == NULL)
    {
        APPGEN_APP_PARTICIPANT_NOT_FOUND(OSAPI_LOGKIND_ERROR, part_name)
        goto done;
    }

    if (!APPGEN_Plugin_initialize_component_factories(
             appgen,
             DDS_DomainParticipantFactory_get_instance(),
             &participant_model->domain_participant_factory))
    {
        goto done;
    }

    /* create participant */
    participant = APPGEN_Plugin_create_domainparticipant_from_model(
                      appgen, participant_model);

    if (participant != NULL)
    {
        appgen->factory->number_participants++;
    }

done:

    /* If participant creation failed, do not call
     * APPGEN_Plugin_finalize_component_factories(). Participants
     * can share factories, so in case participant 2 fails to be created
     * and we unregister its fatories, participant 1 might still need
     * to use them.
     * In any APPGEN_Plugin_finalize_component_factories() is called
     * when this factory is finalized and this will ensure that any factory
     * that might be registered will be unregistered
     */

    return participant;
}

/******************************************************************************
 *
 * AppGen Component Interface
 */
/*ci
 * \brief The AppGen implementation
 */
RTI_PRIVATE struct DDS_AppGenI AppGen_fv_Intf =
{
    RT_COMPONENTI_BASE,
    APPGEN_Plugin_create_participant_from_config
};

/* ------------------------------------------------------------------------ */
/*                   Plugin factory                                         */
/* ------------------------------------------------------------------------ */

/*ci
 * \brief Create a new instance of the App Generation
 *
 * \details
 * Implementation of the AppGen ComponentFactory create component method. This
 * method is not called directly, only via the factory interface.
 *
 * \param[in] factory   The factory creating the component
 * \param[in] property  The component property
 * \param[in] listener  The component listener
 *
 * \return A new component on success, NULL on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RT_Component_T*
APPGEN_Plugin_create_component(struct RT_ComponentFactory *factory,
                               struct RT_ComponentProperty *property,
                               struct RT_ComponentListener *listener)
{
    struct APPGEN_Plugin *retval = NULL;
    struct APPGEN_Factory *appgen = (struct APPGEN_Factory*)factory;
    struct DDS_AppGen_ComponentProperty *component_property =
                                (struct DDS_AppGen_ComponentProperty*)property;

    UNUSED_ARG(listener);

    OSAPI_PRECONDITION_ALWAYS(property==NULL,
                              return NULL,
                     OSAPI_Log_entry_add_pointer("property",property,RTI_TRUE);)

    retval = APPGEN_Plugin_create(appgen, component_property, NULL);

    if (retval == NULL)
    {
        return NULL;
    }

    return &retval->_parent._parent;
}

#ifndef RTI_CERT
/*ci
 * \brief Delete an instance of the Application Generation
 *
 * \details
 * Implementation of the AppGen ComponentFactory delete method. This method is
 * not called directly, only via the factory interface.
 *
 * \param[in] factory   The factory that created the component
 * \param[in] property  The component to be deleted
 *
 * \sa \ref AppGenFactory_create_component
 */
RTI_PRIVATE void
APPGEN_Plugin_delete_component(struct RT_ComponentFactory *factory,
                               RT_Component_T *component)
{
    struct APPGEN_Plugin *self = (struct APPGEN_Plugin*)component;

    UNUSED_ARG(factory);

    APPGEN_Plugin_delete(self);
}
#endif /* !RTI_CERT */

MUST_CHECK_RETURN RTI_PRIVATE struct RT_ComponentFactory*
APPGEN_Factory_initialize(struct RT_ComponentFactoryProperty *property,
                          struct RT_ComponentFactoryListener *listener);

#ifndef RTI_CERT
RTI_PRIVATE void
APPGEN_Factory_finalize(struct RT_ComponentFactory *factory,
                        struct RT_ComponentFactoryProperty **property,
                        struct RT_ComponentFactoryListener **listener);
#endif /* !RTI_CERT */

/*ci
 * \brief Implementation of the RT ComponentFactoryI interface
 */
#ifndef RTI_CERT
LINK_SECTION_DATA_SDRAM
RTI_PRIVATE struct RT_ComponentFactoryI AppGenFactory_fv_Intf =
{
    APPGEN_INTERFACE_ID,
    APPGEN_Factory_initialize,
    APPGEN_Factory_finalize,
    APPGEN_Plugin_create_component,
    APPGEN_Plugin_delete_component,
    NULL,
    NULL
};
#else
LINK_SECTION_DATA_SDRAM
RTI_PRIVATE struct RT_ComponentFactoryI AppGenFactory_fv_Intf =
{
    APPGEN_INTERFACE_ID,
    APPGEN_Factory_initialize,
    NULL, /* AppGenFactory_finalize, */
    APPGEN_Plugin_create_component,
    NULL, /* AppGenPlugin_delete_component, */
    NULL,
    NULL
};
#endif /* !RTI_CERT */

/*ci
 * \brief Initialize the Application Generation factory
 *
 * \details
 * Application generation specific implementation of the RT ComponentFactory
 * initialize method. This method is called when the UDP interface factory
 * is registered with the RT.
 *
 * \param[in] property The properties registered with the Application Generation
 * \param[in] listener The listener registered with the Application Generation
 *
 * \return A fully initialized factory on success, NULL on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE struct RT_ComponentFactory*
APPGEN_Factory_initialize(struct RT_ComponentFactoryProperty *property,
                          struct RT_ComponentFactoryListener *listener)
{
    struct APPGEN_Factory *factory = NULL;
    const struct APPGEN_LibraryModelSeq *model;

    UNUSED_ARG(listener);

    OSAPI_PRECONDITION_ALWAYS(property==NULL,
                              return NULL,
                     OSAPI_Log_entry_add_pointer("property",property,RTI_TRUE);)

    model = ((struct APPGEN_FactoryProperty*)property)->_model;

    OSAPI_PRECONDITION_ALWAYS(model==NULL,
                              return NULL,
                     OSAPI_Log_entry_add_pointer("model",model,RTI_TRUE);)

    OSAPI_Heap_allocate_struct(&factory,struct APPGEN_Factory);
    if (factory == NULL)
    {
        APPGEN_LOG_ALLOCATE(OSAPI_LOGKIND_ERROR)
        return NULL;
    }

    factory->_parent._factory = &factory->_parent;
    factory->_parent.intf = &AppGenFactory_fv_Intf;
    factory->_model = model;
    factory->registered_reader_history = RTI_FALSE;
    factory->registered_writer_history = RTI_FALSE;
    factory->number_participants = 0;

    return &factory->_parent;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize the UDP interface factory
 *
 * \details
 * Implementation of the AppGen ComponentFactory finalize method. This method
 * is called when the Application Generation interface factory is unregistered
 * from the RT.
 *
 * \param[in]  factory  The factory to finalize
 * \param[out] property The property the factory was registered with
 * \param[out] listener The listener the factory was registered with
 *
 * \sa \ref UDP_InterfaceFactory_initialize
 */
RTI_PRIVATE void
APPGEN_Factory_finalize(struct RT_ComponentFactory *factory,
                        struct RT_ComponentFactoryProperty **property,
                        struct RT_ComponentFactoryListener **listener)
{
    struct APPGEN_Factory *appgen_factory = (struct APPGEN_Factory*)factory;
    RTI_INT32 i;
    DDS_UnsignedLong j;
    RTI_BOOL ret_value;
    RT_Registry_T *registry;
    DDS_DomainParticipantFactory *participant_factory;
    const struct APPGEN_LibraryModel *library_model = NULL;
    DDS_Long library_length;

    if (property != NULL)
    {
        *property = NULL;
    }

    if (listener != NULL)
    {
        *listener = NULL;
    }

    participant_factory = DDS_DomainParticipantFactory_get_instance();
    registry = DDS_DomainParticipantFactory_get_registry(participant_factory);

    /* unregister reader and writer history if needed */
    if (appgen_factory->registered_reader_history)
    {
        ret_value = RT_Registry_unregister(registry,
                                           DDSHST_READER_DEFAULT_HISTORY_NAME,
                                           NULL, NULL);
        IGNORE_RETVAL(ret_value);
    }
    if (appgen_factory->registered_writer_history)
    {
        ret_value = RT_Registry_unregister(registry,
                                           DDSHST_WRITER_DEFAULT_HISTORY_NAME,
                                           NULL, NULL);
        IGNORE_RETVAL(ret_value);
    }

    /* We do not know what domain participants have been created so try to
     * unregister all factories in all domain participants in the model
     * configuration. Function AppGen_finalize_factories() checks that a
     * factory is registered before trying to unregister it.
     */
    library_length = APPGEN_LibraryModelSeq_get_length(appgen_factory->_model);
    for (i = 0; i < library_length; i++)
    {
        library_model =
            APPGEN_LibraryModelSeq_get_reference(appgen_factory->_model, i);
        if (library_model == NULL)
        {
            break;
        }

        for (j = 0; j < library_model->participant_count; j++)
        {
            const struct APPGEN_DomainParticipantModel *participant_model =
                    &library_model->participants[j];

            ret_value = APPGEN_Plugin_finalize_component_factories(
                            participant_factory,
                            &participant_model->domain_participant_factory);

            IGNORE_RETVAL(ret_value);
        }
    }

    OSAPI_Heap_free_struct(appgen_factory);
}
#endif /* !RTI_CERT */

struct RT_ComponentFactoryI*
APPGEN_Factory_get_interface(void)
{
    return &AppGenFactory_fv_Intf;
}

RTI_BOOL
APPGEN_Factory_register(RT_Registry_T *registry,
                        struct APPGEN_FactoryProperty *property)
{
    return RT_Registry_register(registry, PROFILE_DEFAULT_APPGEN_NAME,
                                APPGEN_Factory_get_interface(),
                                &property->_parent, NULL);
}

RTI_BOOL
APPGEN_Factory_unregister(RT_Registry_T *registry,
                          struct APPGEN_FactoryProperty **property)
{
    return RT_Registry_unregister(registry,
                                  PROFILE_DEFAULT_APPGEN_NAME,
                                  (struct RT_ComponentFactoryProperty**)property,
                                  NULL);
}

/*ci @} */
