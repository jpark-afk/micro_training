/*
 * Copyright (c) 2017-2024 Real-Time Innovations, Inc.  All rights reserved.
 * Permission to modify and use for internal purposes granted.
 * This software is provided "as is", without warranty, express or implied.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef rti_me_cpp_hxx
#include "rti_me_cpp.hxx"
#endif


#include "HelloWorldApplication.h"
#include "HelloWorldUdpTransform.h"
#include "HelloWorldPlugin.h"
#include "HelloWorldSupport.h"

#include "dds_cpp/dds_cpp_dpde.hxx"
#include "dds_cpp/dds_cpp_wh_sm.hxx"
#include "dds_cpp/dds_cpp_rh_sm.hxx"
#include "dds_cpp/dds_cpp_netio.hxx"

static const char TRANSFORM_FACTORY_NAME[] = "openssl";

void
Application::help(char *appname)
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

DDS_ReturnCode_t 
Application::initialize(const char *local_participant_name,
                        const char *remote_participant_name,
                        DDS_Long domain_id, 
                        char *udp_intf1, 
                        char *udp_intf2, 
                        char *peer,
                        DDS_Long sleep_time,
                        DDS_Long count)
{
    DDS_ReturnCode_t retcode;
    DDSDomainParticipantFactory *factory = NULL; 
    DDS_DomainParticipantFactoryQos dpf_qos;
    DDS_DomainParticipantQos dp_qos;
    DPDE_DiscoveryPluginProperty dpde_properties;
    DDS_Boolean success = DDS_BOOLEAN_FALSE;
    RTRegistry *registry = NULL;
    struct UDP_InterfaceFactoryProperty *udp_property1 = NULL;
    struct UDP_InterfaceFactoryProperty *udp_property2 = NULL;
    struct HelloWorldUdpTransformFactoryProperty *transform_property = NULL;

    /* Uncomment to increase verbosity level:
       OSAPI_Log_set_verbosity(OSAPI_LOG_VERBOSITY_WARNING);
    */

    this->sleep_time = sleep_time;
    this->count = count;
    
    factory = DDSDomainParticipantFactory::get_instance();
    registry = factory->get_registry();

    if (!registry->register_component("wh",
                              WHSMHistoryFactory::get_interface(),
                              NULL, NULL))
    {
        printf("failed to register wh\n");
        goto done;
    }

    if (!registry->register_component("rh",
                              RHSMHistoryFactory::get_interface(),
                              NULL, NULL))
    {
        printf("failed to register rh\n");
        goto done;
    }

    /* Configure UDP transport's allowed interfaces */
    if (!registry->unregister(NETIO_DEFAULT_UDP_NAME, NULL, NULL))
    {
        printf("failed to unregister udp\n");
        goto done;
    }

    /* there will always be at least one interface. 
     * If there is only one interface it will be used for discovery and user-data
     * If there are two interfaces the first one will be used for discovery and
     * the second for user-data
     */
    udp_property1 = new UDP_InterfaceFactoryProperty();
    if (udp_property1 == NULL)
    {
        printf("failed to allocate udp properties 1\n");
        goto done;
    }

    transform_property = new HelloWorldUdpTransformFactoryProperty();
    if (transform_property == NULL)
    {
        goto done;
    }

    if (udp_intf2 != NULL)
    {
        udp_property2 = new UDP_InterfaceFactoryProperty();
        if (udp_property2 == NULL)
        {
            printf("failed to allocate udp properties 2\n");
            goto done;
        }
        udp_property2->is_default_interface = RTI_FALSE;
        /* UDP needs to be enabled as this transport sends plain data */
        udp_property2->transform_udp_mode = UDP_TRANSFORM_UDP_MODE_ENABLED;
    }
    /* For additional allowed interface(s), increase maximum and length, and
       set interface below:
    */
    if (!udp_property1->allow_interface.maximum(1))
    {
        printf("failed to set allow_interface1 maximum\n");
        goto done;
    }
    if (!udp_property1->allow_interface.length(1))
    {
        printf("failed to set allow_interface1 length\n");
        goto done;
    }

    if (udp_intf2 != NULL)
    {
        if (!udp_property2->allow_interface.maximum(1))
        {
            printf("failed to set allow_interface2 maximum\n");
            goto done;
        }
        if (!udp_property2->allow_interface.length(1))
        {
            printf("failed to set allow_interface2 length\n");
            goto done;
        }
    }

    /* first interface */
    if (udp_intf1 != NULL)
    {   /* use interface supplied on command line */
        *udp_property1->allow_interface.get_reference(0) = DDS_String_dup(udp_intf1);
    } 
    else
    {   /* use hardcoded interface */
#if defined(RTI_DARWIN)
        *udp_property1->allow_interface.get_reference(0) = DDS_String_dup("lo0");
#elif defined (RTI_LINUX)
        *udp_property1->allow_interface.get_reference(0) = DDS_String_dup("lo");
#elif defined (RTI_VXWORKS)
        *udp_property1->allow_interface.get_reference(0) = DDS_String_dup("lo0");
#elif defined(RTI_WIN32)
        *udp_property1->allow_interface.get_reference(0) = 
                   DDS_String_dup("Loopback Pseudo-Interface 1");
#else
        *udp_property1->allow_interface.get_reference(0) = DDS_String_dup("lo");
#endif
    }

    if (udp_intf2 != NULL)
    {   /* use interface supplied on command line */
        *udp_property2->allow_interface.get_reference(0) = DDS_String_dup(udp_intf2);
    }

    if (!registry->register_component(TRANSFORM_FACTORY_NAME,
                                      HelloWorldUdpTransformFactory_get_interface(),
                                      &transform_property->_parent, 
                                      NULL))
    {
        printf("failed to register udp transformation\n");
        goto done;
    }

    /* use this transformation to receive from any address and any mask */
    if (!UDP_TransformRules_assert_source_rule(&udp_property1->source_rules, 
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

    if (!registry->register_component("_udp1",
                              UDPInterfaceFactory::get_interface(),
                              &udp_property1->_parent._parent,
                              NULL))
    {
        printf("failed to register udp\n");
        goto done;
    }

    if (udp_property2 != NULL)
    {
        if (!registry->register_component("_udp2",
                                 UDPInterfaceFactory::get_interface(),
                                 &udp_property2->_parent._parent,
                                 NULL))
        {
            printf("failed to register udp\n");
            goto done;
        }
    }

    factory->get_qos(dpf_qos);
    dpf_qos.entity_factory.autoenable_created_entities = DDS_BOOLEAN_FALSE;
    factory->set_qos(dpf_qos);

    if (!registry->register_component(
                              "dpde",
                              DPDEDiscoveryFactory::get_interface(),
                              &dpde_properties._parent,
                              NULL))
    {
        printf("failed to register dpde\n");
        goto done;
    }

    if (!dp_qos.discovery.discovery.name.set_name("dpde"))
    {
        printf("failed to set discovery plugin name\n");
        goto done;
    }

    if (peer == NULL)
    {
        if (!dp_qos.discovery.initial_peers.maximum(1))
        {
            printf("failed to set initial peers maximum\n");
            goto done;
        }
        if (!dp_qos.discovery.initial_peers.length(1))
        {
            printf("failed to set initial peers length\n");
            goto done;
        }
    }
    else
    {
        if (!dp_qos.discovery.initial_peers.maximum(2))
        {
            printf("failed to set initial peers maximum\n");
            goto done;
        }
        if (!dp_qos.discovery.initial_peers.length(2))
        {
            printf("failed to set initial peers length\n");
            goto done;
        }
        *dp_qos.discovery.initial_peers.get_reference(1) = DDS_String_dup(peer);
    }
    *dp_qos.discovery.initial_peers.get_reference(0) = DDS_String_dup("239.0.0.1");

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
        if (!dp_qos.transports.enabled_transports.maximum(2))
        {
            printf("failed to set enabled transports maximum\n");
            goto done;
        }
        if (!dp_qos.transports.enabled_transports.length(2))
        {
            printf("failed to set enabled transports length\n");
            goto done;
        }
        *dp_qos.transports.enabled_transports.get_reference(1) =
                                                       DDS_String_dup("_udp2");
    }
    else
    {
        if (!dp_qos.transports.enabled_transports.maximum(1))
        {
            printf("failed to set enabled transports maximum\n");
            goto done;
        }
        if (!dp_qos.transports.enabled_transports.length(1))
        {
            printf("failed to set enabled transports length\n");
            goto done;
        }
    }
    *dp_qos.transports.enabled_transports.get_reference(0) =
                                                        DDS_String_dup("_udp1");

    if (!dp_qos.discovery.enabled_transports.maximum(1))
    {
        printf("failed to set discovery transports maximum\n");
        goto done;
    }
    if (!dp_qos.discovery.enabled_transports.length(1))
    {
        printf("failed to set discovery transports length\n");
        goto done;
    }
    *dp_qos.discovery.enabled_transports.get_reference(0) = 
                                                    DDS_String_dup("_udp1://");

    if (!dp_qos.user_traffic.enabled_transports.maximum(1))
    {
        printf("failed to set user traffic transports maximum\n");
        goto done;
    }
    if (!dp_qos.user_traffic.enabled_transports.length(1))
    {
        printf("failed to set user traffic transports length\n");
        goto done;
    }
    if (udp_property2 != NULL)
    {
        *dp_qos.user_traffic.enabled_transports.get_reference(0) = 
                                                    DDS_String_dup("_udp2://");
    }
    else
    {
        *dp_qos.user_traffic.enabled_transports.get_reference(0) = 
                                                    DDS_String_dup("_udp1://");
    }

    this->participant = factory->create_participant(
                                    (DDS_DomainId_t)domain_id,
                                    dp_qos, 
                                    NULL,
                                    DDS_STATUS_MASK_NONE);

    if (this->participant == NULL)
    {
        printf("failed to create participant\n");
        goto done;
    }

    strcpy(this->type_name,"HelloWorld");

    retcode = HelloWorldTypeSupport::register_type(this->participant,
                                                   this->type_name);
    if (retcode != DDS_RETCODE_OK)
    {
        printf("failed to register type: %s\n", "test_type");
        goto done;
    }

    strcpy(this->topic_name, "Example HelloWorld");
    this->topic = this->participant->create_topic(
                                               this->topic_name,
                                               this->type_name,
                                               DDS_TOPIC_QOS_DEFAULT, 
                                               NULL,
                                               DDS_STATUS_MASK_NONE);
    if (this->topic == NULL)
    {
        printf("topic == NULL\n");
        goto done;
    }

    success = DDS_BOOLEAN_TRUE;

  done:
    
    if (!success)
    {
        if (udp_property1 != NULL)
        {
            delete udp_property1;
        }
        if (udp_property2 != NULL)
        {
            delete udp_property2;
        }
        if (transform_property != NULL)
        {
            delete transform_property;
        }
    }

    return (success ? DDS_RETCODE_OK : DDS_RETCODE_ERROR);
}

DDS_ReturnCode_t
Application::enable()
{
    DDS_ReturnCode_t retcode;

    retcode = this->participant->enable();
    if (retcode != DDS_RETCODE_OK)
    {
        printf("failed to enable entity\n");
    }

    return retcode;
}

Application::Application()
{
    this->participant = NULL;
    this->topic = NULL;
    this->topic_name[0] = '\0';
    this->type_name[0] = '\0';
    this->sleep_time = 1000;
    this->count = 0;
}


Application::~Application()
{
    DDS_ReturnCode_t retcode;
    RTRegistry *registry = NULL;
    struct UDP_InterfaceFactoryProperty *udp_property = NULL;
    struct HelloWorldUdpTransformFactoryProperty *transform_property = NULL;

    if (this->participant != NULL)
    {
        retcode = this->participant->delete_contained_entities();
        if (retcode != DDS_RETCODE_OK)
        {
            printf("failed to delete contained entities (retcode=%d)\n",retcode);
        }

        retcode =
            DDSTheParticipantFactory->delete_participant(this->participant);
        if (retcode != DDS_RETCODE_OK)
        {
            printf("failed to delete participant: %d\n", retcode);
            return;
        }
        this->participant = NULL;
    }

    registry = (DDSDomainParticipantFactory::get_instance())->get_registry();

    /* _udp1:// is always registered so unregister it */
    if (!registry->unregister("_udp1", 
                              (struct RT_ComponentFactoryProperty**)&udp_property, 
                              NULL))
    {
        printf("failed to unregister udp\n");
    }
    if (udp_property != NULL)
    {
        REDA_StringSeq_finalize(&udp_property->allow_interface);
        REDA_StringSeq_finalize(&udp_property->deny_interface);
        UDP_TransformRuleSeq_finalize(&udp_property->destination_rules);
        UDP_TransformRuleSeq_finalize(&udp_property->source_rules);
        delete udp_property;
    }
    /* _udp2:// is not always registered so unregister it only 
     * in case it is registered 
     */
    if (RT_Registry_lookup(DDS_DomainParticipantFactory_get_registry(
               DDS_DomainParticipantFactory_get_instance()), "_udp2"))
    {
        if (!registry->unregister("_udp2",
                                  (struct RT_ComponentFactoryProperty**)&udp_property, 
                                  NULL))
        {
            printf("failed to unregister udp\n");
        }
        if (udp_property != NULL)
        {
            delete udp_property;
        }
    }
    if (!registry->unregister(TRANSFORM_FACTORY_NAME, 
                              (struct RT_ComponentFactoryProperty**)&transform_property, 
                              NULL))
    {
        printf("failed to unregister udp transformation\n");
    }
    if (transform_property != NULL)
    {
        delete transform_property;
    }

    if (!registry->unregister("dpde", NULL, NULL))
    {
        printf("failed to unregister dpde\n");
        return;
    }
    if (!registry->unregister("rh", NULL, NULL))
    {
        printf("failed to unregister rh\n");
        return;
    }
    if (!registry->unregister("wh", NULL, NULL))
    {
        printf("failed to unregister wh\n");
        return;
    }

    retcode = DDSTheParticipantFactory->finalize_instance();
    if (retcode != DDS_RETCODE_OK)
    {
        printf("failed to finalize instance (retcode=%d)\n",retcode);
        return;
    }
}
