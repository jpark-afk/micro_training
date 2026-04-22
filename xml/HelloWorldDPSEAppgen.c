
/*
WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

This file was generated from HelloWorldDPSE.xml using "rtiddsmag."
The rtiddsmag tool is part of the RTI Connext distribution.
For more information, type 'rtiddsmag -help' at a command shell
or consult the RTI Connext manual.
*/

#include "HelloWorldDPSEAppgen.h"

const char *const HelloWorldAppLibrary_HelloWorldDPSEPubDP_initial_peers[2] =
{
    "127.0.0.1",
    "239.255.0.1"
};

const char *const HelloWorldAppLibrary_HelloWorldDPSEPubDP_discovery_enabled_transports[3] =
{
    "udp1://",
    "udp1://239.255.0.1",
    "udp1://127.0.0.1"
};

const char *const HelloWorldAppLibrary_HelloWorldDPSEPubDP_transport_enabled_transports[1] =
{
    "udp1"
};

const char *const HelloWorldAppLibrary_HelloWorldDPSEPubDP_user_traffic_enabled_transports[2] =
{
    "udp1://",
    "udp1://127.0.0.1"
};

const char *const HelloWorldAppLibrary_HelloWorldDPSESubDP_initial_peers[2] =
{
    "127.0.0.1",
    "239.255.0.1"
};

const char *const HelloWorldAppLibrary_HelloWorldDPSESubDP_discovery_enabled_transports[3] =
{
    "udp1://",
    "udp1://239.255.0.1",
    "udp1://127.0.0.1"
};

const char *const HelloWorldAppLibrary_HelloWorldDPSESubDP_transport_enabled_transports[1] =
{
    "udp1"
};

const char *const HelloWorldAppLibrary_HelloWorldDPSESubDP_user_traffic_enabled_transports[2] =
{
    "udp1://",
    "udp1://127.0.0.1"
};

const struct ComponentFactoryUnregisterModel HelloWorldAppLibrary_HelloWorldDPSEPubDP_unregister_components[2] =
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

struct DPSE_DiscoveryPluginProperty HelloWorldAppLibrary_HelloWorldDPSEPubDP_dpse[1] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldDPSEQos.xml, lineNumber=124, columnNumber=35 */
    RTI_APP_GEN___dpse__HelloWorldAppLibrary_HelloWorldDPSEPubDP_dpse1
};

struct UDP_InterfaceFactoryProperty HelloWorldAppLibrary_HelloWorldDPSEPubDP_udpv4[1] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldDPSEQos.xml, lineNumber=32, columnNumber=28 */
    RTI_APP_GEN___udpv4__HelloWorldAppLibrary_HelloWorldDPSEPubDP_udp1
};

const struct ComponentFactoryRegisterModel HelloWorldAppLibrary_HelloWorldDPSEPubDP_register_components[2] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldDPSEQos.xml, lineNumber=124, columnNumber=35 */
    {
        "dpse1", /* register_name */
        DPSE_DiscoveryFactory_get_interface, /* register_intf */
        &HelloWorldAppLibrary_HelloWorldDPSEPubDP_dpse[0]._parent, /* register_property */
        NULL /* register_listener */
    },
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldDPSEQos.xml, lineNumber=32, columnNumber=28 */
    {
        "udp1", /* register_name */
        UDP_InterfaceFactory_get_interface, /* register_intf */
        &HelloWorldAppLibrary_HelloWorldDPSEPubDP_udpv4[0]._parent._parent, /* register_property */
        NULL /* register_listener */
    }
};

const struct APPGEN_TypeRegistrationModel HelloWorldAppLibrary_HelloWorldDPSEPubDP_type_registrations[1] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldDPSE.xml, lineNumber=7, columnNumber=68 */
    {
        "HelloWorld", /* registered_type_name */
        HelloWorldTypePlugin_get /* get_type_plugin */
    }
};

const struct APPGEN_TopicModel HelloWorldAppLibrary_HelloWorldDPSEPubDP_topics[1] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldDPSE.xml, lineNumber=10, columnNumber=74 */
    {
        "HelloWorldTopic", /* topic_name */
        "HelloWorld", /* type_name */
        DDS_TopicQos_INITIALIZER /* topic_qos */
    }
};

const struct APPGEN_DataWriterModel HelloWorldAppLibrary_HelloWorldDPSEPubDP_publisher_HelloWorldDPSEPub_data_writers[1] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldDPSE.xml, lineNumber=20, columnNumber=82 */
    {
        "HelloWorldDPSEDW", /* name */
        1UL, /* multiplicity */
        "HelloWorldTopic", /* topic_name */
        RTI_APP_GEN___DW_QOS_HelloWorldAppLibrary_HelloWorldDPSEPubDP_HelloWorldDPSEPub_HelloWorldDPSEDW /* writer_qos */
    }
};

const struct APPGEN_PublisherModel HelloWorldAppLibrary_HelloWorldDPSEPubDP_publishers[1] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldDPSE.xml, lineNumber=19, columnNumber=49 */
    {
        "HelloWorldDPSEPub", /* name */
        1UL, /* multiplicity */
        DDS_PublisherQos_INITIALIZER, /* publisher_qos */
        1UL, /* writer_count */
        HelloWorldAppLibrary_HelloWorldDPSEPubDP_publisher_HelloWorldDPSEPub_data_writers /* data_writers */
    }
};

const struct APPGEN_RemoteSubscriptionModel HelloWorldAppLibrary_HelloWorldDPSEPubDP_HelloWorldAppLibrary_HelloWorldDPSESubDP_remote_subscribers[1] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldDPSE.xml, lineNumber=30, columnNumber=82 */
    RTI_APP_GEN__RSD_HelloWorldAppLibrary_HelloWorldDPSEPubDP_HelloWorldAppLibrary_HelloWorldDPSESubDP_HelloWorldDPSESub_HelloWorldDPSEDR
};

const struct APPGEN_RemoteParticipantModel HelloWorldAppLibrary_HelloWorldDPSEPubDP_remote_participants[1] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldDPSE.xml, lineNumber=28, columnNumber=62 */
    {
        "HelloWorldDPSESubDP", /* name */
        0UL, /* remote_publisher_count */
        NULL, /* remote_publishers */
        1UL, /* remote_subscriber_count */
        HelloWorldAppLibrary_HelloWorldDPSEPubDP_HelloWorldAppLibrary_HelloWorldDPSESubDP_remote_subscribers /* remote_subscribers */
    }
};

const struct ComponentFactoryUnregisterModel HelloWorldAppLibrary_HelloWorldDPSESubDP_unregister_components[2] =
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

struct DPSE_DiscoveryPluginProperty HelloWorldAppLibrary_HelloWorldDPSESubDP_dpse[1] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldDPSEQos.xml, lineNumber=124, columnNumber=35 */
    RTI_APP_GEN___dpse__HelloWorldAppLibrary_HelloWorldDPSEPubDP_dpse1
};

struct UDP_InterfaceFactoryProperty HelloWorldAppLibrary_HelloWorldDPSESubDP_udpv4[1] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldDPSEQos.xml, lineNumber=32, columnNumber=28 */
    RTI_APP_GEN___udpv4__HelloWorldAppLibrary_HelloWorldDPSEPubDP_udp1
};

const struct ComponentFactoryRegisterModel HelloWorldAppLibrary_HelloWorldDPSESubDP_register_components[2] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldDPSEQos.xml, lineNumber=124, columnNumber=35 */
    {
        "dpse1", /* register_name */
        DPSE_DiscoveryFactory_get_interface, /* register_intf */
        &HelloWorldAppLibrary_HelloWorldDPSESubDP_dpse[0]._parent, /* register_property */
        NULL /* register_listener */
    },
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldDPSEQos.xml, lineNumber=32, columnNumber=28 */
    {
        "udp1", /* register_name */
        UDP_InterfaceFactory_get_interface, /* register_intf */
        &HelloWorldAppLibrary_HelloWorldDPSESubDP_udpv4[0]._parent._parent, /* register_property */
        NULL /* register_listener */
    }
};

const struct APPGEN_TypeRegistrationModel HelloWorldAppLibrary_HelloWorldDPSESubDP_type_registrations[1] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldDPSE.xml, lineNumber=7, columnNumber=68 */
    {
        "HelloWorld", /* registered_type_name */
        HelloWorldTypePlugin_get /* get_type_plugin */
    }
};

const struct APPGEN_TopicModel HelloWorldAppLibrary_HelloWorldDPSESubDP_topics[1] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldDPSE.xml, lineNumber=10, columnNumber=74 */
    {
        "HelloWorldTopic", /* topic_name */
        "HelloWorld", /* type_name */
        DDS_TopicQos_INITIALIZER /* topic_qos */
    }
};

const struct APPGEN_DataReaderModel HelloWorldAppLibrary_HelloWorldDPSESubDP_subscriber_HelloWorldDPSESub_data_readers[1] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldDPSE.xml, lineNumber=30, columnNumber=82 */
    {
        "HelloWorldDPSEDR", /* name */
        1UL, /* multiplicity */
        "HelloWorldTopic", /* topic_name */
        RTI_APP_GEN___DR_QOS_HelloWorldAppLibrary_HelloWorldDPSESubDP_HelloWorldDPSESub_HelloWorldDPSEDR /* reader_qos */
    }
};

const struct APPGEN_SubscriberModel HelloWorldAppLibrary_HelloWorldDPSESubDP_subscribers[1] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldDPSE.xml, lineNumber=29, columnNumber=50 */
    {
        "HelloWorldDPSESub", /* name */
        1UL, /* multiplicity */
        DDS_SubscriberQos_INITIALIZER, /* subscriber_qos */
        1UL, /* reader_count */
        HelloWorldAppLibrary_HelloWorldDPSESubDP_subscriber_HelloWorldDPSESub_data_readers /* data_readers */
    }
};

const struct APPGEN_RemotePublicationModel HelloWorldAppLibrary_HelloWorldDPSESubDP_HelloWorldAppLibrary_HelloWorldDPSEPubDP_remote_publishers[1] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldDPSE.xml, lineNumber=20, columnNumber=82 */
    RTI_APP_GEN__RPD_HelloWorldAppLibrary_HelloWorldDPSESubDP_HelloWorldAppLibrary_HelloWorldDPSEPubDP_HelloWorldDPSEPub_HelloWorldDPSEDW
};

const struct APPGEN_RemoteParticipantModel HelloWorldAppLibrary_HelloWorldDPSESubDP_remote_participants[1] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldDPSE.xml, lineNumber=18, columnNumber=62 */
    {
        "HelloWorldDPSEPubDP", /* name */
        1UL, /* remote_publisher_count */
        HelloWorldAppLibrary_HelloWorldDPSESubDP_HelloWorldAppLibrary_HelloWorldDPSEPubDP_remote_publishers, /* remote_publishers */
        0UL, /* remote_subscriber_count */
        NULL /* remote_subscribers */
    }
};

const struct APPGEN_DomainParticipantModel HelloWorldAppLibrary_participants[2] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldDPSE.xml, lineNumber=18, columnNumber=62 */
    RTI_APP_GEN__DP_HelloWorldAppLibrary_HelloWorldDPSEPubDP,
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldDPSE.xml, lineNumber=28, columnNumber=62 */
    RTI_APP_GEN__DP_HelloWorldAppLibrary_HelloWorldDPSESubDP
};

const struct APPGEN_LibraryModel HelloWorldDPSE_libraries[1] =
{
    /* XML Source Location: file=C:\Users\jpark\Documents\rti_workspace\micro_4_training\xml\HelloWorldDPSE.xml, lineNumber=16, columnNumber=61 */
    RTI_APP_GEN__LIB_HelloWorldAppLibrary
};

const struct APPGEN_LibraryModelSeq HelloWorldDPSE_libraries_sequence =
        REDA_DEFINE_SEQUENCE_INITIALIZER_W_LOAN(
                HelloWorldDPSE_libraries,
                1,
                1,
                struct APPGEN_LibraryModel);

APPGENDllExport const struct APPGEN_LibraryModelSeq*
        APPGEN_get_library_seq(void)
{
    return &HelloWorldDPSE_libraries_sequence;
}
