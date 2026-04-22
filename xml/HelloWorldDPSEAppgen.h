/*
WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

This file was generated from HelloWorldDPSE.xml using "rtiddsmag."
The rtiddsmag tool is part of the RTI Connext distribution.
For more information, type 'rtiddsmag -help' at a command shell
or consult the RTI Connext manual.
*/

#include "C:/Users/jpark/Documents/rti_workspace/micro_4_training/xml/HelloWorldDPSEPlugin.h"
#include "app_gen/app_gen.h"
#include "netio/netio_udp.h"
#include "disc_dpse/disc_dpse_dpsediscovery.h"

/* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldDPSEQos.xml, lineNumber=32, columnNumber=28 */

/**
* NOTE: Do not add a comma (,) after UDP_NAT_INITIALIZER, it is
* part of the definition based on the RTI_CERT definition.
*/
#define RTI_APP_GEN___udpv4__HelloWorldAppLibrary_HelloWorldDPSEPubDP_udp1 \
{ \
    NETIO_InterfaceFactoryProperty_INITIALIZER, \
    REDA_StringSeq_INITIALIZER, /* allow_interface */ \
    REDA_StringSeq_INITIALIZER, /* deny_interface */ \
    65535, /* max_send_buffer_size */ \
    65535, /* max_receive_buffer_size */ \
    65507, /* max_message_size */ \
    -1, /* max_send_message_size */ \
    1, /* multicast_ttl */ \
    UDP_NAT_INITIALIZER \
    UDP_InterfaceTableEntrySeq_INITIALIZER, /* if_table */ \
    NULL, /* multicast_interface */ \
    DDS_BOOLEAN_TRUE, /* is_default_interface */ \
    DDS_BOOLEAN_FALSE, /* disable_auto_interface_config */ \
    DDS_BOOLEAN_FALSE, /* multicast_loopback_disabled */ \
    {   /* recv_thread */ \
        OSAPI_THREAD_USE_OSDEFAULT_STACKSIZE, /* stack_size */ \
        OSAPI_THREAD_PRIORITY_NORMAL, /* priority */ \
        OSAPI_THREAD_SUSPEND_ENABLE /* options */ \
    },  \
    RTI_FALSE, /* enable_interface_bind */ \
    DDS_BOOLEAN_FALSE, /* disable_multicast_bind */ \
    DDS_BOOLEAN_FALSE /* disable_multicast_interface_select */ \
    UDP_TRANSFORMS_INITIALIZER \
}

/* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldDPSEQos.xml, lineNumber=124, columnNumber=35 */
#define RTI_APP_GEN___dpse__HelloWorldAppLibrary_HelloWorldDPSEPubDP_dpse1 \
{ \
    RT_ComponentFactoryProperty_INITIALIZER, /* _parent */ \
    {   /*participant_liveliness_assert_period */ \
        30L, /* sec */ \
        0L /* nanosec */ \
    }, \
    {   /*participant_liveliness_lease_duration */ \
        100L, /* sec */ \
        0L /* nanosec */ \
    }, \
    5, /* initial_participant_announcements */ \
    {   /*initial_participant_announcement_period */ \
        1L, /* sec */ \
        0L /* nanosec */ \
    }, \
    DDS_LENGTH_AUTO, /* max_participant_locators */ \
    4 /* max_locators_per_discovered_participant */ \
    DDS_PARTICIPANT_MESSAGE_READER_RELIABILITY_KIND_INITIALIZER \
}

/* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldDPSE.xml, lineNumber=21, columnNumber=74 */

#define RTI_APP_GEN___DW_QOS_HelloWorldAppLibrary_HelloWorldDPSEPubDP_HelloWorldDPSEPub_HelloWorldDPSEDW \
{ \
    DDS_DEADLINE_QOS_POLICY_DEFAULT, \
    DDS_LIVELINESS_QOS_POLICY_DEFAULT, \
    {   /* history */ \
        DDS_KEEP_LAST_HISTORY_QOS, /* kind */ \
        32L /* depth */ \
    }, \
    {   /* resource_limits */ \
        64L, /* max_samples */ \
        2L, /* max_instances */ \
        32L /* max_samples_per_instance */ \
    }, \
    DDS_OWNERSHIP_QOS_POLICY_DEFAULT, \
    DDS_OWNERSHIP_STRENGTH_QOS_POLICY_DEFAULT, \
    DDS_LATENCY_BUDGET_QOS_POLICY_DEFAULT, \
    DDS_DATAWRITER_RELIABILITY_QOS_POLICY_DEFAULT, \
    DDS_DURABILITY_QOS_POLICY_DEFAULT, \
    DDS_DESTINATION_ORDER_QOS_POLICY_DEFAULT, \
    DDS_TRANSPORT_ENCAPSULATION_QOS_POLICY_DEFAULT, \
    DDS_DATA_REPRESENTATION_QOS_POLICY_DEFAULT, \
    {   /* protocol */ \
        1UL, /* rtps_object_id */ \
        { /* rtps_reliable_writer */ \
            {   /* heartbeat_period */ \
                0L, /* sec */ \
                250000000L /* nanosec */ \
            },  \
            1L, /* heartbeats_per_max_samples */ \
            DDS_LENGTH_UNLIMITED, /* max_send_window */ \
            DDS_LENGTH_UNLIMITED, /* max_heartbeat_retries */ \
            {   /* first_write_sequence_number */ \
                0, /* high */ \
                1  /* low */ \
            } \
        }, \
        DDS_BOOLEAN_TRUE /* serialize_on_write */ \
    }, \
    DDS_TYPESUPPORT_QOS_POLICY_DEFAULT, \
    DDS_TRANSPORT_QOS_POLICY_DEFAULT, \
    RTI_MANAGEMENT_QOS_POLICY_DEFAULT, \
    DDS_DATAWRITERRESOURCE_LIMITS_QOS_POLICY_DEFAULT, \
    DDS_PUBLISH_MODE_QOS_POLICY_DEFAULT, \
    DDS_USER_DATA_QOS_POLICY_DEFAULT, \
    DDS_PROPERTY_QOS_POLICY_DEFAULT, \
    DDS_DATAWRITERQOS_TRUST_INITIALIZER \
    DDS_DATAWRITERQOS_APPGEN_INITIALIZER \
    NULL, \
    DDS_DataWriterTransferModeQosPolicy_INITIALIZER \
}

/* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldDPSE.xml, lineNumber=24, columnNumber=67 */
extern const char *const HelloWorldAppLibrary_HelloWorldDPSEPubDP_initial_peers[2];
extern const char *const HelloWorldAppLibrary_HelloWorldDPSEPubDP_discovery_enabled_transports[3];
extern const char *const HelloWorldAppLibrary_HelloWorldDPSEPubDP_transport_enabled_transports[1];
extern const char *const HelloWorldAppLibrary_HelloWorldDPSEPubDP_user_traffic_enabled_transports[2];

#define RTI_APP_GEN___DP_QOS_HelloWorldAppLibrary_HelloWorldDPSEPubDP \
{ \
    DDS_ENTITY_FACTORY_QOS_POLICY_DEFAULT, \
    {   /* discovery */ \
        REDA_StringSeq_INITIALIZER_W_LOAN(HelloWorldAppLibrary_HelloWorldDPSEPubDP_initial_peers, 2, 2), /* initial_peers */ \
        REDA_StringSeq_INITIALIZER_W_LOAN(HelloWorldAppLibrary_HelloWorldDPSEPubDP_discovery_enabled_transports, 3, 3), /* enabled_transports */ \
        { \
            { { "dpse1" } }, /* RT_ComponentFactoryId_INITIALIZER */ \
            NDDS_Discovery_Property_INITIALIZER \
        }, /* discovery_component */ \
        DDS_BOOLEAN_TRUE, /* accept_unknown_peers */ \
        DDS_BOOLEAN_FALSE, /* enable_participant_discovery_by_name */ \
        DDS_BOOLEAN_FALSE /* enable_endpoint_discovery_queue */ \
    }, \
    {   /* resource_limits  */ \
        1L, /* local_writer_allocation */ \
        1L, /* local_reader_allocation */ \
        1L, /* local_publisher_allocation */ \
        1L, /* local_subscriber_allocation */ \
        1L, /* local_topic_allocation */ \
        1L, /* local_type_allocation */ \
        8L, /* remote_participant_allocation */ \
        32L, /* remote_writer_allocation */ \
        32L, /* remote_reader_allocation */ \
        128L, /* matching_writer_reader_pair_allocation */ \
        128L, /* matching_reader_writer_pair_allocation */ \
        32L, /* max_receive_ports */ \
        32L, /* max_destination_ports */ \
        65536, /* unbound_data_buffer_size */ \
        500UL, /* shmem_ref_transfer_mode_max_segments */ \
        0L, /* participant_user_data_max_length */ \
        DDS_SIZE_AUTO, /* participant_user_data_max_count */ \
        0L, /* topic_data_max_length */ \
        DDS_SIZE_AUTO, /* topic_data_max_count */ \
        0L, /* publisher_group_data_max_length */ \
        DDS_SIZE_AUTO, /* publisher_group_data_max_count */ \
        0L, /* subscriber_group_data_max_length */ \
        DDS_SIZE_AUTO, /* subscriber_group_data_max_count */ \
        0L, /* writer_user_data_max_length */ \
        DDS_SIZE_AUTO, /* writer_user_data_max_count */ \
        0L, /* reader_user_data_max_length */ \
        DDS_SIZE_AUTO, /* reader_user_data_max_count */ \
        64L, /* max_partitions */ \
        256L, /* max_partition_cumulative_characters */ \
        DDS_LENGTH_UNLIMITED, /* max_partition_string_size */ \
        DDS_LENGTH_UNLIMITED, /* max_partition_string_allocation */ \
        1L, /* participant_property_list_max_length */ \
        32L, /* participant_property_string_max_length */ \
        1L, /* writer_property_list_max_length */ \
        32L, /* writer_property_string_max_length */ \
        0L, /* reader_property_list_max_length */ \
        0L /* reader_property_string_max_length */ \
    }, \
    DDS_ENTITY_NAME_QOS_POLICY_DEFAULT, \
    DDS_WIRE_PROTOCOL_QOS_POLICY_DEFAULT, \
    {   /* transports */ \
        REDA_StringSeq_INITIALIZER_W_LOAN(HelloWorldAppLibrary_HelloWorldDPSEPubDP_transport_enabled_transports, 1, 1) /* enabled_transports */ \
    }, \
    {   /* user_traffic */ \
        REDA_StringSeq_INITIALIZER_W_LOAN(HelloWorldAppLibrary_HelloWorldDPSEPubDP_user_traffic_enabled_transports, 2, 2) /* enabled_transports */ \
    }, \
    DDS_TRUST_QOS_POLICY_DEFAULT, \
    DDS_PROPERTY_QOS_POLICY_DEFAULT, \
    DDS_USER_DATA_QOS_POLICY_DEFAULT \
    ,DDS_FilterQosPolicy_INITIALIZER \
}

/* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldDPSE.xml, lineNumber=31, columnNumber=74 */

#define RTI_APP_GEN___DR_QOS_HelloWorldAppLibrary_HelloWorldDPSESubDP_HelloWorldDPSESub_HelloWorldDPSEDR \
{ \
    DDS_DEADLINE_QOS_POLICY_DEFAULT, \
    DDS_LIVELINESS_QOS_POLICY_DEFAULT, \
    {   /* history */ \
        DDS_KEEP_LAST_HISTORY_QOS, /* kind */ \
        32L /* depth */ \
    }, \
    {   /* resource_limits */ \
        64L, /* max_samples */ \
        2L, /* max_instances */ \
        32L /* max_samples_per_instance */ \
    }, \
    DDS_OWNERSHIP_QOS_POLICY_DEFAULT, \
    DDS_LATENCY_BUDGET_QOS_POLICY_DEFAULT, \
    {   /* reliability */ \
        DDS_RELIABLE_RELIABILITY_QOS, /* kind */ \
        {   /* max_blocking_time */ \
            0L, /* sec */ \
            0L /* nanosec */ \
        } \
    }, \
    DDS_DURABILITY_QOS_POLICY_DEFAULT, \
    DDS_DESTINATION_ORDER_QOS_POLICY_DEFAULT, \
    DDS_TRANSPORT_ENCAPSULATION_QOS_POLICY_DEFAULT, \
    DDS_DATA_REPRESENTATION_QOS_POLICY_DEFAULT, \
    DDS_TYPESUPPORT_QOS_POLICY_DEFAULT, \
    {   /* protocol */ \
        2UL, /* rtps_object_id */ \
        DDS_RTPS_RELIABLE_READER_PROTOCOL_DEFAULT \
    }, \
    DDS_TRANSPORT_QOS_POLICY_DEFAULT, \
    {   /* reader_resource_limits */ \
        10L, /* max_remote_writers */ \
        10L, /* max_remote_writers_per_instance */ \
        1L, /* max_samples_per_remote_writer */ \
        1L, /* max_outstanding_reads */ \
        DDS_NO_INSTANCE_REPLACEMENT_QOS, /* instance_replacement */ \
        4L, /* max_routes_per_writer */ \
        DDS_MAX_AUTO, /* max_fragmented_samples */ \
        DDS_MAX_AUTO, /* max_fragmented_samples_per_remote_writer */ \
        DDS_SIZE_AUTO /* shmem_ref_transfer_mode_attached_segment_allocation */ \
    }, \
    RTI_MANAGEMENT_QOS_POLICY_DEFAULT, \
    DDS_USER_DATA_QOS_POLICY_DEFAULT, \
    DDS_PROPERTY_QOS_POLICY_DEFAULT, \
    DDS_CONTENT_FILTER_QOS_POLICY_DEFAULT, \
    DDS_DATAREADERQOS_TRUST_INITIALIZER \
    DDS_DATAREADERQOS_APPGEN_INITIALIZER \
    NULL \
}

/* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldDPSE.xml, lineNumber=34, columnNumber=67 */
extern const char *const HelloWorldAppLibrary_HelloWorldDPSESubDP_initial_peers[2];
extern const char *const HelloWorldAppLibrary_HelloWorldDPSESubDP_discovery_enabled_transports[3];
extern const char *const HelloWorldAppLibrary_HelloWorldDPSESubDP_transport_enabled_transports[1];
extern const char *const HelloWorldAppLibrary_HelloWorldDPSESubDP_user_traffic_enabled_transports[2];

#define RTI_APP_GEN___DP_QOS_HelloWorldAppLibrary_HelloWorldDPSESubDP \
{ \
    DDS_ENTITY_FACTORY_QOS_POLICY_DEFAULT, \
    {   /* discovery */ \
        REDA_StringSeq_INITIALIZER_W_LOAN(HelloWorldAppLibrary_HelloWorldDPSESubDP_initial_peers, 2, 2), /* initial_peers */ \
        REDA_StringSeq_INITIALIZER_W_LOAN(HelloWorldAppLibrary_HelloWorldDPSESubDP_discovery_enabled_transports, 3, 3), /* enabled_transports */ \
        { \
            { { "dpse1" } }, /* RT_ComponentFactoryId_INITIALIZER */ \
            NDDS_Discovery_Property_INITIALIZER \
        }, /* discovery_component */ \
        DDS_BOOLEAN_TRUE, /* accept_unknown_peers */ \
        DDS_BOOLEAN_FALSE, /* enable_participant_discovery_by_name */ \
        DDS_BOOLEAN_FALSE /* enable_endpoint_discovery_queue */ \
    }, \
    {   /* resource_limits  */ \
        1L, /* local_writer_allocation */ \
        1L, /* local_reader_allocation */ \
        1L, /* local_publisher_allocation */ \
        1L, /* local_subscriber_allocation */ \
        1L, /* local_topic_allocation */ \
        1L, /* local_type_allocation */ \
        8L, /* remote_participant_allocation */ \
        32L, /* remote_writer_allocation */ \
        32L, /* remote_reader_allocation */ \
        128L, /* matching_writer_reader_pair_allocation */ \
        128L, /* matching_reader_writer_pair_allocation */ \
        32L, /* max_receive_ports */ \
        32L, /* max_destination_ports */ \
        65536, /* unbound_data_buffer_size */ \
        500UL, /* shmem_ref_transfer_mode_max_segments */ \
        0L, /* participant_user_data_max_length */ \
        DDS_SIZE_AUTO, /* participant_user_data_max_count */ \
        0L, /* topic_data_max_length */ \
        DDS_SIZE_AUTO, /* topic_data_max_count */ \
        0L, /* publisher_group_data_max_length */ \
        DDS_SIZE_AUTO, /* publisher_group_data_max_count */ \
        0L, /* subscriber_group_data_max_length */ \
        DDS_SIZE_AUTO, /* subscriber_group_data_max_count */ \
        0L, /* writer_user_data_max_length */ \
        DDS_SIZE_AUTO, /* writer_user_data_max_count */ \
        0L, /* reader_user_data_max_length */ \
        DDS_SIZE_AUTO, /* reader_user_data_max_count */ \
        64L, /* max_partitions */ \
        256L, /* max_partition_cumulative_characters */ \
        DDS_LENGTH_UNLIMITED, /* max_partition_string_size */ \
        DDS_LENGTH_UNLIMITED, /* max_partition_string_allocation */ \
        1L, /* participant_property_list_max_length */ \
        32L, /* participant_property_string_max_length */ \
        1L, /* writer_property_list_max_length */ \
        32L, /* writer_property_string_max_length */ \
        0L, /* reader_property_list_max_length */ \
        0L /* reader_property_string_max_length */ \
    }, \
    DDS_ENTITY_NAME_QOS_POLICY_DEFAULT, \
    DDS_WIRE_PROTOCOL_QOS_POLICY_DEFAULT, \
    {   /* transports */ \
        REDA_StringSeq_INITIALIZER_W_LOAN(HelloWorldAppLibrary_HelloWorldDPSESubDP_transport_enabled_transports, 1, 1) /* enabled_transports */ \
    }, \
    {   /* user_traffic */ \
        REDA_StringSeq_INITIALIZER_W_LOAN(HelloWorldAppLibrary_HelloWorldDPSESubDP_user_traffic_enabled_transports, 2, 2) /* enabled_transports */ \
    }, \
    DDS_TRUST_QOS_POLICY_DEFAULT, \
    DDS_PROPERTY_QOS_POLICY_DEFAULT, \
    DDS_USER_DATA_QOS_POLICY_DEFAULT \
    ,DDS_FilterQosPolicy_INITIALIZER \
}

#define RTI_APP_GEN___DPF_QOS_HelloWorldAppLibrary_HelloWorldDPSEPubDP \
{ \
    DDS_ENTITY_FACTORY_QOS_POLICY_DEFAULT, \
    {   /* resource_limits */ \
        2L, /* max_participants  */ \
        16L /* max_components */ \
    } \
}

#define RTI_APP_GEN___DPF_QOS_HelloWorldAppLibrary_HelloWorldDPSEPubDP_HelloWorldDPSESubDP \
{ \
    DDS_ENTITY_FACTORY_QOS_POLICY_DEFAULT, \
    {   /* resource_limits */ \
        2L, /* max_participants  */ \
        16L /* max_components */ \
    } \
}

/* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldDPSE.xml, lineNumber=30, columnNumber=82 */
#define RTI_APP_GEN__RSD_HelloWorldAppLibrary_HelloWorldDPSEPubDP_HelloWorldAppLibrary_HelloWorldDPSESubDP_HelloWorldDPSESub_HelloWorldDPSEDR \
{ \
    { /* subscription_data */ \
        { \
            { 0, 0, 0, 2 } /* key */ \
        }, \
        { \
            { 0, 0, 0, 0 } /* participant_key */ \
        }, \
        "HelloWorldTopic", /* topic_name */ \
        "HelloWorld", /* type_name */ \
        DDS_DEADLINE_QOS_POLICY_DEFAULT, \
        DDS_OWNERSHIP_QOS_POLICY_DEFAULT, \
        DDS_LATENCY_BUDGET_QOS_POLICY_DEFAULT, \
        {   /* reliability */ \
            DDS_RELIABLE_RELIABILITY_QOS, /* kind */ \
            {   /* max_blocking_time */ \
                0L, /* sec */ \
                0L /* nanosec */ \
            } \
        }, \
        DDS_LIVELINESS_QOS_POLICY_DEFAULT, \
        DDS_DURABILITY_QOS_POLICY_DEFAULT, \
        DDS_DESTINATION_ORDER_QOS_POLICY_DEFAULT, \
        DDS_SEQUENCE_INITIALIZER, \
        DDS_SEQUENCE_INITIALIZER, \
        DDS_PRESENTATION_QOS_POLICY_DEFAULT, \
        DDS_DATA_REPRESENTATION_QOS_POLICY_DEFAULT, \
        DDS_PARTITION_QOS_POLICY_DEFAULT, \
        DDS_USER_DATA_QOS_POLICY_DEFAULT, \
        DDS_GROUP_DATA_QOS_POLICY_DEFAULT, \
        DDS_TOPIC_DATA_QOS_POLICY_DEFAULT \
        ,DDS_CONTENT_FILTER_PROPERTY_DEFAULT \
        DDS_TRUST_SUBSCRIPTION_DATA_INITIALIZER \
    }, \
    HelloWorldTypePlugin_get /* get_type_plugin */ \
}

extern struct DPSE_DiscoveryPluginProperty HelloWorldAppLibrary_HelloWorldDPSEPubDP_dpse[1];
extern struct UDP_InterfaceFactoryProperty HelloWorldAppLibrary_HelloWorldDPSEPubDP_udpv4[1];

extern const struct ComponentFactoryUnregisterModel HelloWorldAppLibrary_HelloWorldDPSEPubDP_unregister_components[2];
extern const struct ComponentFactoryRegisterModel HelloWorldAppLibrary_HelloWorldDPSEPubDP_register_components[2];

#define RTI_APP_GEN__DPF_HelloWorldAppLibrary_HelloWorldDPSEPubDP \
{ \
    2UL, /* unregister_count */ \
    HelloWorldAppLibrary_HelloWorldDPSEPubDP_unregister_components, /* unregister_components */ \
    2UL, /* register_count */ \
    HelloWorldAppLibrary_HelloWorldDPSEPubDP_register_components, /* register_components */ \
    RTI_APP_GEN___DPF_QOS_HelloWorldAppLibrary_HelloWorldDPSEPubDP /* factory_qos */ \
}

/* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldDPSE.xml, lineNumber=18, columnNumber=62 */
extern const struct APPGEN_TypeRegistrationModel HelloWorldAppLibrary_HelloWorldDPSEPubDP_type_registrations[1];
extern const struct APPGEN_TopicModel HelloWorldAppLibrary_HelloWorldDPSEPubDP_topics[1];
extern const struct APPGEN_PublisherModel HelloWorldAppLibrary_HelloWorldDPSEPubDP_publishers[1];
extern const struct APPGEN_RemoteParticipantModel HelloWorldAppLibrary_HelloWorldDPSEPubDP_remote_participants[1];

#define RTI_APP_GEN__DP_HelloWorldAppLibrary_HelloWorldDPSEPubDP \
{ \
    "HelloWorldDPSEPubDP", /* name */ \
    RTI_APP_GEN__DPF_HelloWorldAppLibrary_HelloWorldDPSEPubDP, /* domain_participant_factory */ \
    RTI_APP_GEN___DP_QOS_HelloWorldAppLibrary_HelloWorldDPSEPubDP, /* participant_qos */ \
    0L, /* domain_id */ \
    1UL, /* type_registration_count */ \
    HelloWorldAppLibrary_HelloWorldDPSEPubDP_type_registrations, /* type_registrations */ \
    1UL, /* topic_count */ \
    HelloWorldAppLibrary_HelloWorldDPSEPubDP_topics, /* topics */ \
    1UL, /* publisher_count */ \
    HelloWorldAppLibrary_HelloWorldDPSEPubDP_publishers, /* publishers */ \
    0UL, /* subscriber_count */ \
    NULL, /* subscribers */ \
    1UL, /* remote_participant_count */ \
    HelloWorldAppLibrary_HelloWorldDPSEPubDP_remote_participants, /* remote_participants */ \
    0UL, /* custom_flow_controller_count */ \
    NULL, /* custom_flow_controllers */ \
}

/* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldDPSE.xml, lineNumber=20, columnNumber=82 */
#define RTI_APP_GEN__RPD_HelloWorldAppLibrary_HelloWorldDPSESubDP_HelloWorldAppLibrary_HelloWorldDPSEPubDP_HelloWorldDPSEPub_HelloWorldDPSEDW \
{ \
    { /* publication_data */ \
        { \
            { 0, 0, 0, 1 } /* key */ \
        }, \
        { \
            { 0, 0, 0, 0 } /* participant_key */ \
        }, \
        "HelloWorldTopic", /* topic_name */ \
        "HelloWorld", /* type_name */ \
        DDS_DEADLINE_QOS_POLICY_DEFAULT, \
        DDS_OWNERSHIP_QOS_POLICY_DEFAULT, \
        DDS_OWNERSHIP_STRENGTH_QOS_POLICY_DEFAULT, \
        DDS_LATENCY_BUDGET_QOS_POLICY_DEFAULT, \
        DDS_DATAWRITER_RELIABILITY_QOS_POLICY_DEFAULT, \
        DDS_LIVELINESS_QOS_POLICY_DEFAULT, \
        DDS_DURABILITY_QOS_POLICY_DEFAULT, \
        DDS_DESTINATION_ORDER_QOS_POLICY_DEFAULT, \
        DDS_SEQUENCE_INITIALIZER, \
        DDS_DATA_REPRESENTATION_QOS_POLICY_DEFAULT, \
        DDS_PARTITION_QOS_POLICY_DEFAULT, \
        DDS_USER_DATA_QOS_POLICY_DEFAULT, \
        DDS_GROUP_DATA_QOS_POLICY_DEFAULT, \
        DDS_TOPIC_DATA_QOS_POLICY_DEFAULT \
        DDS_TRUST_PUBLICATION_DATA_INITIALIZER \
    }, \
    HelloWorldTypePlugin_get /* get_type_plugin */ \
}

extern struct DPSE_DiscoveryPluginProperty HelloWorldAppLibrary_HelloWorldDPSESubDP_dpse[1];
extern struct UDP_InterfaceFactoryProperty HelloWorldAppLibrary_HelloWorldDPSESubDP_udpv4[1];

extern const struct ComponentFactoryUnregisterModel HelloWorldAppLibrary_HelloWorldDPSESubDP_unregister_components[2];
extern const struct ComponentFactoryRegisterModel HelloWorldAppLibrary_HelloWorldDPSESubDP_register_components[2];

#define RTI_APP_GEN__DPF_HelloWorldAppLibrary_HelloWorldDPSESubDP \
{ \
    2UL, /* unregister_count */ \
    HelloWorldAppLibrary_HelloWorldDPSESubDP_unregister_components, /* unregister_components */ \
    2UL, /* register_count */ \
    HelloWorldAppLibrary_HelloWorldDPSESubDP_register_components, /* register_components */ \
    RTI_APP_GEN___DPF_QOS_HelloWorldAppLibrary_HelloWorldDPSEPubDP_HelloWorldDPSESubDP /* factory_qos */ \
}

/* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldDPSE.xml, lineNumber=28, columnNumber=62 */
extern const struct APPGEN_TypeRegistrationModel HelloWorldAppLibrary_HelloWorldDPSESubDP_type_registrations[1];
extern const struct APPGEN_TopicModel HelloWorldAppLibrary_HelloWorldDPSESubDP_topics[1];
extern const struct APPGEN_SubscriberModel HelloWorldAppLibrary_HelloWorldDPSESubDP_subscribers[1];
extern const struct APPGEN_RemoteParticipantModel HelloWorldAppLibrary_HelloWorldDPSESubDP_remote_participants[1];

#define RTI_APP_GEN__DP_HelloWorldAppLibrary_HelloWorldDPSESubDP \
{ \
    "HelloWorldDPSESubDP", /* name */ \
    RTI_APP_GEN__DPF_HelloWorldAppLibrary_HelloWorldDPSESubDP, /* domain_participant_factory */ \
    RTI_APP_GEN___DP_QOS_HelloWorldAppLibrary_HelloWorldDPSESubDP, /* participant_qos */ \
    0L, /* domain_id */ \
    1UL, /* type_registration_count */ \
    HelloWorldAppLibrary_HelloWorldDPSESubDP_type_registrations, /* type_registrations */ \
    1UL, /* topic_count */ \
    HelloWorldAppLibrary_HelloWorldDPSESubDP_topics, /* topics */ \
    0UL, /* publisher_count */ \
    NULL, /* publishers */ \
    1UL, /* subscriber_count */ \
    HelloWorldAppLibrary_HelloWorldDPSESubDP_subscribers, /* subscribers */ \
    1UL, /* remote_participant_count */ \
    HelloWorldAppLibrary_HelloWorldDPSESubDP_remote_participants, /* remote_participants */ \
    0UL, /* custom_flow_controller_count */ \
    NULL, /* custom_flow_controllers */ \
}

extern const struct APPGEN_DomainParticipantModel HelloWorldAppLibrary_participants[2];

#define RTI_APP_GEN__LIB_HelloWorldAppLibrary \
{ \
    "HelloWorldAppLibrary", /* library_name */ \
    2UL, /* participant_count */ \
    HelloWorldAppLibrary_participants /* participants */ \
}

extern const struct APPGEN_LibraryModel HelloWorldDPSE_libraries[1];
