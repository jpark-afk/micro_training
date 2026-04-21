/*********************************************************************************************
Copyright (c) 2025-2025 Real-Time Innovations, Inc.  All rights reserved.    	
Permission to modify and use for internal purposes granted.   	
This software is provided "as is", without warranty, express or implied.                                                                            
**********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "rti_me_c.h"
#include "disc_dpse/disc_dpse_dpsediscovery.h"
#include "HelloWorldApplication.h"
#include "HelloWorldPlugin.h"
#include "HelloWorldSupport.h"
#include "wh_sm/wh_sm_history.h"
#include "rh_sm/rh_sm_history.h"
#include "HelloWorld_dgram_udpv4.h"

void
Application_help(char *appname)
{
    printf("%s [options]\n", appname);
    printf("options:\n");
    printf("-h                 - This text\n");
    printf("-domain <id>       - DomainId (default: 0)\n");
    printf("-count <count>     - count (default -1)\n");
    printf("-sleep <ms>        - sleep between sends (default 1s)\n");
    printf("\n");
}

struct Application *
Application_create(
    const char *local_participant_name,
    const char *remote_participant_name,
    DDS_Long domain_id,
    DDS_Long sleep_time,
    DDS_Long count)
{
    DDS_ReturnCode_t retcode;
    DDS_DomainParticipantFactory *factory = NULL;
    struct DDS_DomainParticipantFactoryQos dpf_qos =
        DDS_DomainParticipantFactoryQos_INITIALIZER;
    struct DDS_DomainParticipantQos dp_qos =
        DDS_DomainParticipantQos_INITIALIZER;
    DDS_Boolean success = DDS_BOOLEAN_FALSE;
    struct Application *application = NULL;
    RT_Registry_T *registry = NULL;
    struct DPSE_DiscoveryPluginProperty discovery_plugin_properties =
        DPSE_DiscoveryPluginProperty_INITIALIZER;

    /* Uncomment to increase verbosity level:
    OSAPI_Log_set_verbosity(OSAPI_LOG_VERBOSITY_WARNING);
    */
    application = (struct Application *)malloc(sizeof(struct Application));

    if (application == NULL)
    {
        printf("failed to allocate application\n");
        goto done;
    }

    application->sleep_time = sleep_time;
    application->count = count;

    factory = DDS_DomainParticipantFactory_get_instance();

    registry = DDS_DomainParticipantFactory_get_registry(factory);

    if (!RT_Registry_register(
        registry,
        DDSHST_WRITER_DEFAULT_HISTORY_NAME,
        WHSM_HistoryFactory_get_interface(),
        NULL,
        NULL))
    {
        printf("failed to register wh\n");
        goto done;
    }

    if (!RT_Registry_register(
        registry,
        DDSHST_READER_DEFAULT_HISTORY_NAME,
        RHSM_HistoryFactory_get_interface(),
        NULL,
        NULL))
    {
        printf("failed to register rh\n");
        goto done;
    }

    /* If the UDP transport has already been registered, unregister it to
     * set new properties.
     */
    if (RT_Registry_unregister(registry, NETIO_DEFAULT_UDP_NAME, NULL, NULL))
    {
        printf("Unregistered existing UDP transport.\n");
    }

    if (!HelloWorld_dgram_udpv4_Interface_register(registry,"_dgram"))
    {
        printf("failed to register dgram\n");
        goto done;
    }

    DDS_DomainParticipantFactory_get_qos(factory, &dpf_qos);
    dpf_qos.entity_factory.autoenable_created_entities = DDS_BOOLEAN_FALSE;
    DDS_DomainParticipantFactory_set_qos(factory, &dpf_qos);

    if (!RT_Registry_register(registry,
                              "dpse",
                              DPSE_DiscoveryFactory_get_interface(),
                              &discovery_plugin_properties._parent,
                              NULL))
    {
        printf("failed to register dpse\n");
        goto done;
    }

    if (!RT_ComponentFactoryId_set_name(&dp_qos.discovery.discovery.name,"dpse"))
    {
        printf("failed to set discovery plugin name\n");
        goto done;
    }

    if (!DDS_StringSeq_set_maximum(&dp_qos.discovery.initial_peers,1))
    {
        printf("failed to set initial peers maximum\n");
        goto done;
    }
    if (!DDS_StringSeq_set_length(&dp_qos.discovery.initial_peers,1))
    {
        printf("failed to set initial peers length\n");
        goto done;
    }

    DDS_StringSeq_set_maximum(&dp_qos.transports.enabled_transports,1);
    DDS_StringSeq_set_length(&dp_qos.transports.enabled_transports,1);
    *DDS_StringSeq_get_reference(&dp_qos.transports.enabled_transports,0) = DDS_String_dup("_dgram");

    *DDS_StringSeq_get_reference(&dp_qos.discovery.initial_peers,0) = DDS_String_dup("127.0.0.1");

    DDS_StringSeq_set_maximum(&dp_qos.discovery.enabled_transports,1);
    DDS_StringSeq_set_length(&dp_qos.discovery.enabled_transports,1);
    *DDS_StringSeq_get_reference(&dp_qos.discovery.enabled_transports,0) = DDS_String_dup("_dgram://");

    DDS_StringSeq_set_maximum(&dp_qos.user_traffic.enabled_transports,1);
    DDS_StringSeq_set_length(&dp_qos.user_traffic.enabled_transports,1);
    *DDS_StringSeq_get_reference(&dp_qos.user_traffic.enabled_transports,0) = DDS_String_dup("_dgram://");

    /* if there are more remote or local endpoints, you need to increase these limits */
    dp_qos.resource_limits.max_destination_ports = 32;
    dp_qos.resource_limits.max_receive_ports = 32;
    dp_qos.resource_limits.local_topic_allocation = 1;
    dp_qos.resource_limits.local_type_allocation = 1;
    dp_qos.resource_limits.local_reader_allocation = 1;
    dp_qos.resource_limits.local_writer_allocation = 1;
    dp_qos.resource_limits.remote_participant_allocation = 8;
    dp_qos.resource_limits.remote_reader_allocation = 8;
    dp_qos.resource_limits.remote_writer_allocation = 8;

    /* Must set the name of the domain participant in QoS for discovery */
    strcpy(dp_qos.participant_name.name,local_participant_name);

    application->participant = DDS_DomainParticipantFactory_create_participant(
        factory,
        domain_id,
        &dp_qos,
        NULL,
        DDS_STATUS_MASK_NONE);

    if (application->participant == NULL)
    {
        printf("failed to create participant\n");
        goto done;
    }

    sprintf(application->type_name, "HelloWorld");
    retcode = HelloWorldTypeSupport_register_type(application->participant,
                                                  application->type_name);
    if (retcode != DDS_RETCODE_OK)
    {
        printf("failed to register type: %s\n", "test_type");
        goto done;
    }

    sprintf(application->topic_name, "Example HelloWorld");
    application->topic = DDS_DomainParticipant_create_topic(
        application->participant,
        application->topic_name,
        application->type_name,
        &DDS_TOPIC_QOS_DEFAULT,
        NULL,
        DDS_STATUS_MASK_NONE);
    if (application->topic == NULL)
    {
        printf("topic == NULL\n");
        goto done;
    }

    retcode = DPSE_RemoteParticipant_assert(
        application->participant,
        remote_participant_name);
    if (retcode != DDS_RETCODE_OK)
    {
        printf("failed to assert remote participant\n");
        goto done;
    }

    success = DDS_BOOLEAN_TRUE;

  done:

  DDS_DomainParticipantQos_finalize(&dp_qos);
  
    if (!success)
    {
        if (application != NULL)
        {
            #ifndef RTI_CERT
            free(application);
            #endif
            application = NULL;
        }
    }

    return application;
}

DDS_ReturnCode_t
Application_enable(struct Application * application)
{
    DDS_Entity *entity;
    DDS_ReturnCode_t retcode;

    entity = DDS_DomainParticipant_as_entity(application->participant);

    retcode = DDS_Entity_enable(entity);
    if (retcode != DDS_RETCODE_OK)
    {
        printf("failed to enable entity\n");
    }

    return retcode;
}

#ifndef RTI_CERT
void
Application_delete(struct Application *application)
{
    DDS_ReturnCode_t retcode;
    RT_Registry_T *registry = NULL;

    retcode = DDS_DomainParticipant_delete_contained_entities(application->participant);
    if (retcode != DDS_RETCODE_OK)
    {
        printf("failed to delete contained entities (retcode=%d)\n",retcode);
    }

    retcode =
        DDS_DomainParticipantFactory_delete_participant
        (DDS_DomainParticipantFactory_get_instance(), application->participant);
    if (retcode != DDS_RETCODE_OK)
    {
        printf("failed to delete participant: %d\n", retcode);
        return;
    }
    
    registry = DDS_DomainParticipantFactory_get_registry
                (DDS_DomainParticipantFactory_get_instance());
    
    if (!NETIO_DGRAM_InterfaceFactory_unregister(registry,"_dgram"))
    {
        printf("failed to unregister dgram\n");
        return;
    }

    if (!RT_Registry_unregister(registry, "dpse", NULL, NULL))
    {
        printf("failed to unregister dpse\n");
        return;
    }
    if (!RT_Registry_unregister(registry, DDSHST_READER_DEFAULT_HISTORY_NAME, NULL, NULL))
    {
        printf("failed to unregister rh\n");
        return;
    }
    if (!RT_Registry_unregister(registry, DDSHST_WRITER_DEFAULT_HISTORY_NAME, NULL, NULL))
    {
        printf("failed to unregister wh\n");
        return;
    }

    free(application);
    DDS_DomainParticipantFactory_finalize_instance();
}
#endif
