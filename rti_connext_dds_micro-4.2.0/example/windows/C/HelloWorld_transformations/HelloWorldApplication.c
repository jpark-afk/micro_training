/*********************************************************************************************
Copyright (c) 2017-2025 Real-Time Innovations, Inc.  All rights reserved.    	
Permission to modify and use for internal purposes granted.   	
This software is provided "as is", without warranty, express or implied.                                                                            
**********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rti_me_c.h"
#include "disc_dpde/disc_dpde_discovery_plugin.h"
#include "HelloWorldApplication.h"
#include "HelloWorldUdpTransform.h"
#include "HelloWorldPlugin.h"
#include "HelloWorldSupport.h"
#include "wh_sm/wh_sm_history.h"
#include "rh_sm/rh_sm_history.h"
#include "netio/netio_udp.h"

static const char TRANSFORM_FACTORY_NAME[] = "openssl";

void
Application_help(char *appname)
{
    printf("%s [options]\n", appname);
    printf("options:\n");
    printf("-h                 - This text\n");
    printf("-domain <id>       - DomainId (default: 0)\n");
    printf("-udp_intf1 <intf>  - udp interface 1 (no default)\n");
    printf("-udp_intf2 <intf>  - udp interface 2 (no default)\n");
    printf("-peer <address>    - peer address (no default)\n");
    printf("-count <count>     - count (default -1)\n");
    printf("-sleep <ms>        - sleep between sends (default 1s)\n");
    printf("\n");
}

struct Application *
Application_create(const char *local_participant_name,
                   const char *remote_participant_name,
                   DDS_Long domain_id, char *udp_intf1,
                   char *udp_intf2, char *peer,
                   DDS_Long sleep_time, DDS_Long count)
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
    struct UDP_InterfaceFactoryProperty *udp_property1 = NULL;
    struct UDP_InterfaceFactoryProperty *udp_property2 = NULL;
    struct HelloWorldUdpTransformFactoryProperty *transform_property = NULL;

    struct DPDE_DiscoveryPluginProperty discovery_plugin_properties =
        DPDE_DiscoveryPluginProperty_INITIALIZER;

    UNUSED_ARG(local_participant_name);
    UNUSED_ARG(remote_participant_name);
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

    registry =
        DDS_DomainParticipantFactory_get_registry
        (DDS_DomainParticipantFactory_get_instance());

    if (!RT_Registry_register(registry, DDSHST_WRITER_DEFAULT_HISTORY_NAME,
                              WHSM_HistoryFactory_get_interface(), NULL, NULL))
    {
        printf("failed to register wh\n");
        goto done;
    }

    if (!RT_Registry_register(registry, DDSHST_READER_DEFAULT_HISTORY_NAME,
                              RHSM_HistoryFactory_get_interface(), NULL, NULL))
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

    /* there will always be at least one interface.
     * If there is only one interface it will be used for discovery and user-data
     * If there are two interfaces the first one will be used for discovery and
     * the second for user-data
     */
    udp_property1 = (struct UDP_InterfaceFactoryProperty *)
                           malloc(sizeof(struct UDP_InterfaceFactoryProperty));
    if (udp_property1 == NULL)
    {
        goto done;
    }
    *udp_property1 = UDP_INTERFACE_FACTORY_PROPERTY_DEFAULT;

    transform_property = (struct HelloWorldUdpTransformFactoryProperty *)
                  malloc(sizeof(struct HelloWorldUdpTransformFactoryProperty));
    if (transform_property == NULL)
    {
        goto done;
    }
    *transform_property = HELLOWORLD_UDP_TRANSFORM_FACTORY_PROPERTY_DEFAULT;

    if (udp_intf2 != NULL)
    {
        udp_property2 = (struct UDP_InterfaceFactoryProperty *)
                           malloc(sizeof(struct UDP_InterfaceFactoryProperty));
        if (udp_property2 == NULL)
        {
            goto done;
        }
        *udp_property2 = UDP_INTERFACE_FACTORY_PROPERTY_DEFAULT;
        udp_property2->is_default_interface = RTI_FALSE;
        /* UDP needs to be enabled as this transport sends plain data */
        udp_property2->transform_udp_mode = UDP_TRANSFORM_UDP_MODE_ENABLED;
    }

    /* For additional allowed interface(s), increase maximum and length, and
       set interface below:
    */
    if (!DDS_StringSeq_set_maximum(&udp_property1->allow_interface,1))
    {
        printf("failed to set allow_interface1 maximum\n");
        goto done;
    }
    if (!DDS_StringSeq_set_length(&udp_property1->allow_interface,1))
    {
        printf("failed to set allow_interface1 length\n");
        goto done;
    }

    if (udp_intf2 != NULL)
    {
        if (!DDS_StringSeq_set_maximum(&udp_property2->allow_interface,1))
        {
            printf("failed to set allow_interface2 maximum\n");
            goto done;
        }
        if (!DDS_StringSeq_set_length(&udp_property2->allow_interface,1))
        {
            printf("failed to set allow_interface2 length\n");
            goto done;
        }
    }

    /* first interface */
    if (udp_intf1 != NULL)
    {   /* use interface supplied on command line */
        *DDS_StringSeq_get_reference(&udp_property1->allow_interface,0) =
                                                      DDS_String_dup(udp_intf1);
    }
    else
    {   /* use hardcoded interface */
#if defined(RTI_DARWIN)
        *DDS_StringSeq_get_reference(&udp_property1->allow_interface,0) =
            DDS_String_dup("eth0");
#elif defined (RTI_LINUX)
        *DDS_StringSeq_get_reference(&udp_property1->allow_interface,0) =
            DDS_String_dup("en1");
#elif defined (RTI_VXWORKS)
        *DDS_StringSeq_get_reference(&udp_property1->allow_interface,0) =
            DDS_String_dup("geisc0");
#elif defined(RTI_WIN32)
        *DDS_StringSeq_get_reference(&udp_property1->allow_interface,0) =
            DDS_String_dup("Local Area Connection");
#else
        *DDS_StringSeq_get_reference(&udp_property1->allow_interface,0) =
            DDS_String_dup("lo");
#endif
    }

    if (udp_intf2 != NULL)
    {   /* use interface supplied on command line */
        *DDS_StringSeq_get_reference(&udp_property2->allow_interface,0) =
                                                     DDS_String_dup(udp_intf2);
    }

    if (!HelloWorldUdpTransformFactory_register(registry,
                                                TRANSFORM_FACTORY_NAME,
                                                transform_property))
    {
        printf("failed to register udp transformation\n");
        goto done;
    }

    /* use this transformation to receive from any address and any mask */
    if (!UDP_TransformRules_assert_source_rule(
                                               &udp_property1->source_rules,
                                               0, 0,
                                               TRANSFORM_FACTORY_NAME,
                                               NULL))
    {
        printf("failed to assert source transform\n");
        goto done;
    }

    /* use this transformation to send to any address and any mask */
    if (!UDP_TransformRules_assert_destination_rule(
                                            &udp_property1->destination_rules,
                                            0, 0,
                                            TRANSFORM_FACTORY_NAME,
                                            NULL))
    {
        printf("failed to assert source transform\n");
        goto done;
    }

    if (!RT_Registry_register(registry, "_udp1",
                            UDP_InterfaceFactory_get_interface(),
                            (struct RT_ComponentFactoryProperty*)udp_property1, NULL))
    {
        printf("failed to register udp\n");
        goto done;
    }

    if (udp_property2 != NULL)
    {
        if (!RT_Registry_register(registry, "_udp2",
                              UDP_InterfaceFactory_get_interface(),
                            (struct RT_ComponentFactoryProperty*)udp_property2, NULL))
        {
            printf("failed to register udp\n");
            goto done;
        }
    }

    DDS_DomainParticipantFactory_get_qos(factory, &dpf_qos);
    dpf_qos.entity_factory.autoenable_created_entities = DDS_BOOLEAN_FALSE;
    DDS_DomainParticipantFactory_set_qos(factory, &dpf_qos);

    if (!RT_Registry_register(registry,
                              "dpde",
                              DPDE_DiscoveryFactory_get_interface(),
                              &discovery_plugin_properties._parent,
                              NULL))
    {
        printf("failed to register dpde\n");
        goto done;
    }

    if (!RT_ComponentFactoryId_set_name(&dp_qos.discovery.discovery.name,"dpde"))
    {
        printf("failed to set discovery plugin name\n");
        goto done;
    }

    if (peer == NULL)
    {
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
            *DDS_StringSeq_get_reference(&dp_qos.discovery.initial_peers,0) =
                                                   DDS_String_dup("239.0.0.1");
    }
    else
    {
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
        *DDS_StringSeq_get_reference(&dp_qos.discovery.initial_peers,0) =
                                                          DDS_String_dup(peer);
    }


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

    if (udp_property2 != NULL)
    {
        if (!DDS_StringSeq_set_maximum(&dp_qos.transports.enabled_transports,2))
        {
            printf("failed to set enabled transports maximum\n");
            goto done;
        }
        if (!DDS_StringSeq_set_length(&dp_qos.transports.enabled_transports,2))
        {
            printf("failed to set enabled transports length\n");
            goto done;
        }
        *DDS_StringSeq_get_reference(&dp_qos.transports.enabled_transports,1) =
                                                       DDS_String_dup("_udp2");
    }
    else
    {
        if (!DDS_StringSeq_set_maximum(&dp_qos.transports.enabled_transports,1))
        {
            printf("failed to set enabled transports maximum\n");
            goto done;
        }
        if (!DDS_StringSeq_set_length(&dp_qos.transports.enabled_transports,1))
        {
            printf("failed to set enabled transports length\n");
            goto done;
        }
    }
    *DDS_StringSeq_get_reference(&dp_qos.transports.enabled_transports,0) =
                                                        DDS_String_dup("_udp1");

    if (!DDS_StringSeq_set_maximum(&dp_qos.discovery.enabled_transports,1))
    {
        printf("failed to set discovery transports maximum\n");
        goto done;
    }
    if (!DDS_StringSeq_set_length(&dp_qos.discovery.enabled_transports,1))
    {
        printf("failed to set discovery transports length\n");
        goto done;
    }
    *DDS_StringSeq_get_reference(&dp_qos.discovery.enabled_transports,0) =
                                                        DDS_String_dup("_udp1://");

    if (!DDS_StringSeq_set_maximum(&dp_qos.user_traffic.enabled_transports,1))
    {
        printf("failed to set user traffic transports maximum\n");
        goto done;
    }
    if (!DDS_StringSeq_set_length(&dp_qos.user_traffic.enabled_transports,1))
    {
        printf("failed to set user traffic transports length\n");
        goto done;
    }
    if (udp_property2 != NULL)
    {
        *DDS_StringSeq_get_reference(&dp_qos.user_traffic.enabled_transports,0) =
                                                        DDS_String_dup("_udp2://");
    }
    else
    {
        *DDS_StringSeq_get_reference(&dp_qos.user_traffic.enabled_transports,0) =
                                                        DDS_String_dup("_udp1://");
    }

    application->participant =
        DDS_DomainParticipantFactory_create_participant(factory, domain_id,
                                                        &dp_qos, NULL,
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
    application->topic =
        DDS_DomainParticipant_create_topic(application->participant,
                                           application->topic_name,
                                           application->type_name,
                                           &DDS_TOPIC_QOS_DEFAULT, NULL,
                                           DDS_STATUS_MASK_NONE);

    if (application->topic == NULL)
    {
        printf("topic == NULL\n");
        goto done;
    }

    success = DDS_BOOLEAN_TRUE;

  done:
    DDS_DomainParticipantQos_finalize(&dp_qos);

    if (!success)
    {
        if (udp_property1 != NULL)
        {
            UDP_InterfaceFactoryProperty_finalize(udp_property1);
            free(udp_property1);
        }
        if (udp_property2 != NULL)
        {
            UDP_InterfaceFactoryProperty_finalize(udp_property2);
            free(udp_property2);
        }
        if (transform_property != NULL)
        {
            free(transform_property);
        }
        free(application);
        application = NULL;
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

void
Application_delete(struct Application *application)
{
    DDS_ReturnCode_t retcode;
    RT_Registry_T *registry = NULL;
    struct UDP_InterfaceFactoryProperty *udp_property = NULL;
    struct HelloWorldUdpTransformFactoryProperty *transform_property = NULL;

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

    /* _udp1:// is always registered so unregister it */
    if (!RT_Registry_unregister(registry, "_udp1",
                                (struct RT_ComponentFactoryProperty**)&udp_property,
                                NULL))
    {
        printf("failed to unregister udp1\n");
    }
    if (udp_property != NULL)
    {
        UDP_InterfaceFactoryProperty_finalize(udp_property);
        free(udp_property);
    }
    /* _udp2:// is not always registered so unregister it only in case it
     *  is registered */
    if (RT_Registry_lookup(registry, "_udp2"))
    {
        if (!RT_Registry_unregister(registry, "_udp2",
                                (struct RT_ComponentFactoryProperty**)&udp_property,
                                NULL))
        {
            printf("failed to unregister udp2\n");
        }
        if (udp_property != NULL)
        {
            UDP_InterfaceFactoryProperty_finalize(udp_property);
            free(udp_property);
        }
    }
    if (!HelloWorldUdpTransformFactory_unregister(registry,
                                                  TRANSFORM_FACTORY_NAME,
                                                  &transform_property))
    {
        printf("failed to unregister udp transformation\n");
    }
    if (transform_property != NULL)
    {
        free(transform_property);
    }
    if (!RT_Registry_unregister(registry, "dpde", NULL, NULL))
    {
        printf("failed to unregister dpde\n");
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

    retcode = DDS_DomainParticipantFactory_finalize_instance();
    if (retcode != DDS_RETCODE_OK)
    {
        printf("failed to finalize instance %d\n", retcode);
        return;
    }
}
