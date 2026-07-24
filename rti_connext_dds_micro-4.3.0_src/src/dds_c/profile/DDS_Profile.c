/*
 * FILE: DDS_Profile.c - Profile and Application Generation implementation
 *
 * (c) Copyright, Real-Time Innovations, 2017-2018.
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
 * \brief DomainFactory implementation
 */
/*ci \addtogroup DDSDomainModule
 * @{
 */
#include "DDS_Profile.h"
#include "DomainFactory.h"
#include "dds_c/dds_c_profile.h"

const char* const PROFILE_DEFAULT_APPGEN_NAME = "_mag";

#if DDS_ENABLE_APPGEN

/*** SOURCE_BEGIN ***/
#ifndef RTI_CERT
#define DDS_AppGen_EntityManager_C_DELETE_INITIALIZER  \
    , DDS_DomainParticipantFactory_delete_participant  \
    , DDS_DomainParticipant_delete_contained_entities
#else
#define DDS_AppGen_EntityManager_C_DELETE_INITIALIZER
#endif

#define DDS_AppGen_EntityManager_C_INITIALIZER        \
{                                                     \
    DDS_DomainParticipantFactory_create_participant,  \
    DDS_DomainParticipant_create_publisher,           \
    DDS_DomainParticipant_create_subscriber,          \
    DDS_Publisher_create_datawriter,                  \
    DDS_Subscriber_create_datareader,                 \
    DDS_DomainParticipant_create_topic                \
    DDS_AppGen_EntityManager_C_DELETE_INITIALIZER     \
}

DDS_DomainParticipant*
DDS_DomainParticipantFactory_create_participant_from_config_w_entity_manager(
                              DDS_DomainParticipantFactory *self,
                              const char *configuration_name,
                              const struct DDS_AppGen_EntityManager *entity_manager)
{
    DDS_DomainParticipant *ret_value = NULL;
    struct RT_ComponentFactory *f;

    OSAPI_PRECONDITION_ALWAYS(
        (self==NULL) || (configuration_name==NULL) || (entity_manager == NULL),
        return NULL,
        OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
        OSAPI_Log_entry_add_pointer("entity_manager",entity_manager,RTI_FALSE);
        OSAPI_Log_entry_add_pointer("name",configuration_name,RTI_TRUE);)

    f = RT_Registry_lookup(self->registry, PROFILE_DEFAULT_APPGEN_NAME);
    if (f != NULL)
    {
        struct DDS_AppGenPlugin *app;
        struct DDS_AppGen_ComponentProperty properties = 
                                      DDS_AppGen_ComponentProperty_INITIALIZER;

        properties.entity_manager = *entity_manager;

        app = DDS_AppGenFactory_create_component(f, &properties._parent, NULL);
        if (app != NULL)
        {
            ret_value =
              DDS_AppGen_create_participant_from_config(app, configuration_name);
#ifndef RTI_CERT
            DDS_AppGenFactory_delete_component(f, &app->_parent);
#endif
        }
        else
        {
            DDSC_LOG_COMPONENT_CREATE(OSAPI_LOGKIND_ERROR,
                                      DDSC_LOG_APPGEN_COMPONENT,
                                      PROFILE_DEFAULT_APPGEN_NAME)
        }
    }
    else
    {
        DDSC_LOG_COMPONENT_LOOKUP(OSAPI_LOGKIND_ERROR,
                                  PROFILE_DEFAULT_APPGEN_NAME)
    }

    return ret_value;
}

DDS_DomainParticipant*
DDS_DomainParticipantFactory_create_participant_from_config(
                                DDS_DomainParticipantFactory *self,
                                const char *configuration_name)
{
    struct DDS_AppGen_EntityManager entity_manager =
                                           DDS_AppGen_EntityManager_C_INITIALIZER;

    return
      DDS_DomainParticipantFactory_create_participant_from_config_w_entity_manager(
                              self,
                              configuration_name,
                              &entity_manager);
}

#endif /* DDS_ENABLE_APPGEN */

/*ci @} */
