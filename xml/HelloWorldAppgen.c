
/*
WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

This file was generated from HelloWorld.xml using "rtiddsmag."
The rtiddsmag tool is part of the RTI Connext distribution.
For more information, type 'rtiddsmag -help' at a command shell
or consult the RTI Connext manual.
*/

#include "HelloWorldAppgen.h"

const char *const HelloWorldAppLibrary_HelloWorldDPDEPubDP_initial_peers[2] =
{
    "127.0.0.1",
    "239.255.0.1"
};

const char *const HelloWorldAppLibrary_HelloWorldDPDEPubDP_discovery_enabled_transports[3] =
{
    "udp1://",
    "udp1://239.255.0.1",
    "udp1://127.0.0.1"
};

const char *const HelloWorldAppLibrary_HelloWorldDPDEPubDP_transport_enabled_transports[1] =
{
    "udp1"
};

const char *const HelloWorldAppLibrary_HelloWorldDPDEPubDP_user_traffic_enabled_transports[2] =
{
    "udp1://",
    "udp1://127.0.0.1"
};

const char *const HelloWorldAppLibrary_HelloWorldDPDESubDP_initial_peers[2] =
{
    "127.0.0.1",
    "239.255.0.1"
};

const char *const HelloWorldAppLibrary_HelloWorldDPDESubDP_discovery_enabled_transports[3] =
{
    "udp1://",
    "udp1://239.255.0.1",
    "udp1://127.0.0.1"
};

const char *const HelloWorldAppLibrary_HelloWorldDPDESubDP_transport_enabled_transports[1] =
{
    "udp1"
};

const char *const HelloWorldAppLibrary_HelloWorldDPDESubDP_user_traffic_enabled_transports[2] =
{
    "udp1://",
    "udp1://127.0.0.1"
};

const struct ComponentFactoryUnregisterModel HelloWorldAppLibrary_HelloWorldDPDEPubDP_unregister_components[2] =
{
    {
        "_udp", /* NETIO_DEFAULT_UDP_NAME */
        NULL, /* udp struct RT_ComponentFactoryProperty** */
        NULL  /* udp struct RT_ComponentFactoryListener** */
    },
    {
        "_intra", /* NETIO_DEFAULT_INTRA_NAME */
        NULL, /* _intra struct RT_ComponentFactoryProperty** */
        NULL  /* _intra struct RT_ComponentFactoryListener** */
    }
};

struct DPDE_DiscoveryPluginProperty HelloWorldAppLibrary_HelloWorldDPDEPubDP_dpde[1] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldQos.xml, lineNumber=124, columnNumber=35 */
    RTI_APP_GEN___dpde__HelloWorldAppLibrary_HelloWorldDPDEPubDP_dpde1
};

struct UDP_InterfaceFactoryProperty HelloWorldAppLibrary_HelloWorldDPDEPubDP_udpv4[1] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldQos.xml, lineNumber=32, columnNumber=28 */
    RTI_APP_GEN___udpv4__HelloWorldAppLibrary_HelloWorldDPDEPubDP_udp1
};

const struct ComponentFactoryRegisterModel HelloWorldAppLibrary_HelloWorldDPDEPubDP_register_components[2] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldQos.xml, lineNumber=124, columnNumber=35 */
    {
        "dpde1", /* register_name */
        DPDE_DiscoveryFactory_get_interface, /* register_intf */
        &HelloWorldAppLibrary_HelloWorldDPDEPubDP_dpde[0]._parent, /* register_property */
        NULL /* register_listener */
    },
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldQos.xml, lineNumber=32, columnNumber=28 */
    {
        "udp1", /* register_name */
        UDP_InterfaceFactory_get_interface, /* register_intf */
        &HelloWorldAppLibrary_HelloWorldDPDEPubDP_udpv4[0]._parent._parent, /* register_property */
        NULL /* register_listener */
    }
};

const struct APPGEN_TypeRegistrationModel HelloWorldAppLibrary_HelloWorldDPDEPubDP_type_registrations[1] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorld.xml, lineNumber=7, columnNumber=68 */
    {
        "HelloWorld", /* registered_type_name */
        HelloWorldTypePlugin_get /* get_type_plugin */
    }
};

const struct APPGEN_TopicModel HelloWorldAppLibrary_HelloWorldDPDEPubDP_topics[1] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorld.xml, lineNumber=10, columnNumber=74 */
    {
        "HelloWorldTopic", /* topic_name */
        "HelloWorld", /* type_name */
        DDS_TopicQos_INITIALIZER /* topic_qos */
    }
};

const struct APPGEN_DataWriterModel HelloWorldAppLibrary_HelloWorldDPDEPubDP_publisher_HelloWorldDPDEPub_data_writers[1] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorld.xml, lineNumber=20, columnNumber=82 */
    {
        "HelloWorldDPDEDW", /* name */
        1UL, /* multiplicity */
        "HelloWorldTopic", /* topic_name */
        RTI_APP_GEN___DW_QOS_HelloWorldAppLibrary_HelloWorldDPDEPubDP_HelloWorldDPDEPub_HelloWorldDPDEDW /* writer_qos */
    }
};

const struct APPGEN_PublisherModel HelloWorldAppLibrary_HelloWorldDPDEPubDP_publishers[1] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorld.xml, lineNumber=19, columnNumber=49 */
    {
        "HelloWorldDPDEPub", /* name */
        1UL, /* multiplicity */
        DDS_PublisherQos_INITIALIZER, /* publisher_qos */
        1UL, /* writer_count */
        HelloWorldAppLibrary_HelloWorldDPDEPubDP_publisher_HelloWorldDPDEPub_data_writers /* data_writers */
    }
};

const struct ComponentFactoryUnregisterModel HelloWorldAppLibrary_HelloWorldDPDESubDP_unregister_components[2] =
{
    {
        "_udp", /* NETIO_DEFAULT_UDP_NAME */
        NULL, /* udp struct RT_ComponentFactoryProperty** */
        NULL  /* udp struct RT_ComponentFactoryListener** */
    },
    {
        "_intra", /* NETIO_DEFAULT_INTRA_NAME */
        NULL, /* _intra struct RT_ComponentFactoryProperty** */
        NULL  /* _intra struct RT_ComponentFactoryListener** */
    }
};

struct DPDE_DiscoveryPluginProperty HelloWorldAppLibrary_HelloWorldDPDESubDP_dpde[1] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldQos.xml, lineNumber=124, columnNumber=35 */
    RTI_APP_GEN___dpde__HelloWorldAppLibrary_HelloWorldDPDEPubDP_dpde1
};

struct UDP_InterfaceFactoryProperty HelloWorldAppLibrary_HelloWorldDPDESubDP_udpv4[1] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldQos.xml, lineNumber=32, columnNumber=28 */
    RTI_APP_GEN___udpv4__HelloWorldAppLibrary_HelloWorldDPDEPubDP_udp1
};

const struct ComponentFactoryRegisterModel HelloWorldAppLibrary_HelloWorldDPDESubDP_register_components[2] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldQos.xml, lineNumber=124, columnNumber=35 */
    {
        "dpde1", /* register_name */
        DPDE_DiscoveryFactory_get_interface, /* register_intf */
        &HelloWorldAppLibrary_HelloWorldDPDESubDP_dpde[0]._parent, /* register_property */
        NULL /* register_listener */
    },
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldQos.xml, lineNumber=32, columnNumber=28 */
    {
        "udp1", /* register_name */
        UDP_InterfaceFactory_get_interface, /* register_intf */
        &HelloWorldAppLibrary_HelloWorldDPDESubDP_udpv4[0]._parent._parent, /* register_property */
        NULL /* register_listener */
    }
};

const struct APPGEN_TypeRegistrationModel HelloWorldAppLibrary_HelloWorldDPDESubDP_type_registrations[1] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorld.xml, lineNumber=7, columnNumber=68 */
    {
        "HelloWorld", /* registered_type_name */
        HelloWorldTypePlugin_get /* get_type_plugin */
    }
};

const struct APPGEN_TopicModel HelloWorldAppLibrary_HelloWorldDPDESubDP_topics[1] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorld.xml, lineNumber=10, columnNumber=74 */
    {
        "HelloWorldTopic", /* topic_name */
        "HelloWorld", /* type_name */
        DDS_TopicQos_INITIALIZER /* topic_qos */
    }
};

const struct APPGEN_DataReaderModel HelloWorldAppLibrary_HelloWorldDPDESubDP_subscriber_HelloWorldDPDESub_data_readers[1] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorld.xml, lineNumber=30, columnNumber=82 */
    {
        "HelloWorldDPDEDR", /* name */
        1UL, /* multiplicity */
        "HelloWorldTopic", /* topic_name */
        RTI_APP_GEN___DR_QOS_HelloWorldAppLibrary_HelloWorldDPDESubDP_HelloWorldDPDESub_HelloWorldDPDEDR /* reader_qos */
    }
};

const struct APPGEN_SubscriberModel HelloWorldAppLibrary_HelloWorldDPDESubDP_subscribers[1] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorld.xml, lineNumber=29, columnNumber=50 */
    {
        "HelloWorldDPDESub", /* name */
        1UL, /* multiplicity */
        DDS_SubscriberQos_INITIALIZER, /* subscriber_qos */
        1UL, /* reader_count */
        HelloWorldAppLibrary_HelloWorldDPDESubDP_subscriber_HelloWorldDPDESub_data_readers /* data_readers */
    }
};

const struct APPGEN_DomainParticipantModel HelloWorldAppLibrary_participants[2] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorld.xml, lineNumber=18, columnNumber=62 */
    RTI_APP_GEN__DP_HelloWorldAppLibrary_HelloWorldDPDEPubDP,
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorld.xml, lineNumber=28, columnNumber=62 */
    RTI_APP_GEN__DP_HelloWorldAppLibrary_HelloWorldDPDESubDP
};

const struct APPGEN_LibraryModel HelloWorld_libraries[1] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorld.xml, lineNumber=16, columnNumber=61 */
    RTI_APP_GEN__LIB_HelloWorldAppLibrary
};

const struct APPGEN_LibraryModelSeq HelloWorld_libraries_sequence =
        REDA_DEFINE_SEQUENCE_INITIALIZER_W_LOAN(
                HelloWorld_libraries,
                1,
                1,
                struct APPGEN_LibraryModel);

APPGENDllExport const struct APPGEN_LibraryModelSeq*
        APPGEN_get_library_seq(void)
{
    return &HelloWorld_libraries_sequence;
}
