/*

 (c) Copyright, Real-Time Innovations, 2006-2026.  All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.
*/
/*
modification history
--------------------
07apr2016,tk  MICRO-1539 Renamed DPSE API to avoid name collision with assert
12feb2016,tk  MICRO-1529 Added primitive sequences
22dec2015,as  MICRO-1515 expose Status types
              MICRO-1518 expose TheParticipantFactory
10dec2015,as  MICRO-1229 Expose ConditionSeq
16may2014,as  MICRO-794 Remove C++ TODO and commented out code
13feb2006,gpc Created
=========================================================================*/

#ifndef dds_cpp_namespace_hxx
#define dds_cpp_namespace_hxx

#ifndef dds_cpp_dll_hxx
#include "dds_cpp/dds_cpp_dll.hxx"
#endif
#ifndef dds_cpp_infrastructure_hxx
#include "dds_cpp/dds_cpp_infrastructure.hxx"
#endif
#ifndef dds_cpp_topic_hxx
#include "dds_cpp/dds_cpp_topic.hxx"
#endif
#ifndef dds_cpp_subscription_hxx
#include "dds_cpp/dds_cpp_subscription.hxx"
#endif
#ifndef dds_cpp_publication_hxx
#include "dds_cpp/dds_cpp_publication.hxx"
#endif
#ifndef dds_cpp_domain_hxx
#include "dds_cpp/dds_cpp_domain.hxx"
#endif
#ifndef dds_cpp_rt_hxx
#include "dds_cpp/dds_cpp_rt.hxx"
#endif
#ifndef dds_cpp_osapi_hxx
#include "dds_cpp/dds_cpp_osapi.hxx"
#endif
#if RTIME_CPP_ENABLE_UDP_NAMESPACE
#ifndef dds_cpp_netio_hxx
#include "dds_cpp/dds_cpp_netio.hxx"
#endif
#endif
#ifndef dds_cpp_netio_shmem_hxx
#include "dds_cpp/dds_cpp_netio_shmem.hxx"
#endif
#ifndef dds_cpp_dpse_hxx
#include "dds_cpp/dds_cpp_dpse.hxx"
#endif
#ifndef dds_cpp_dpde_hxx
#include "dds_cpp/dds_cpp_dpde.hxx"
#endif
#ifndef dds_cpp_wh_sm_hxx
#include "dds_cpp/dds_cpp_wh_sm.hxx"
#endif
#ifndef dds_cpp_rh_sm_hxx
#include "dds_cpp/dds_cpp_rh_sm.hxx"
#endif

#define TheParticipantFactory DDSDomainParticipantFactory::get_instance()

namespace DDS {
    // ----------------------------------------------------------------------
    // Primitive data types
    // ----------------------------------------------------------------------

    typedef DDS_Char Char;
    typedef DDS_Wchar Wchar;
    typedef DDS_Octet Octet;
    typedef DDS_Short Short;
    typedef DDS_UnsignedShort UnsignedShort;
    typedef DDS_Long Long;
    typedef DDS_UnsignedLong UnsignedLong;
    typedef DDS_LongLong LongLong;
    typedef DDS_UnsignedLongLong UnsignedLongLong;
    typedef DDS_Float Float;
    typedef DDS_Double Double;
    typedef DDS_LongDouble LongDouble;
    typedef DDS_Enum Enum;
    typedef DDS_Boolean Boolean;
    typedef DDS_String String;
    typedef DDS_Wstring Wstring;

    // ----------------------------------------------------------------------
    // Primitive sequence types
    // ----------------------------------------------------------------------
    typedef DDS_CharSeq CharSeq;
    typedef DDS_WcharSeq WcharSeq;
    typedef DDS_OctetSeq OctetSeq;
    typedef DDS_ShortSeq ShortSeq;
    typedef DDS_UnsignedShortSeq UnsignedShortSeq;
    typedef DDS_LongSeq LongSeq;
    typedef DDS_UnsignedLongSeq UnsignedLongSeq;
    typedef DDS_LongLongSeq LongLongSeq;
    typedef DDS_UnsignedLongLongSeq UnsignedLongLongSeq;
    typedef DDS_FloatSeq FloatSeq;
    typedef DDS_DoubleSeq DoubleSeq;
    typedef DDS_LongDoubleSeq LongDoubleSeq;
    typedef DDS_EnumSeq EnumSeq;
    typedef DDS_BooleanSeq BooleanSeq;
    typedef DDS_StringSeq  StringSeq;
    typedef DDS_WstringSeq WstringSeq;

    // ----------------------------------------------------------------------
    // Basic types
    // ----------------------------------------------------------------------
    typedef DDS_DomainId_t            DomainId_t;
    typedef DDS_KeyHash_t             KeyHash_t;
    typedef DDS_InstanceHandle_t      InstanceHandle_t;
    typedef DDS_InstanceHandleSeq     InstanceHandleSeq;
    typedef DDS_BuiltinTopicKey_t     BuiltinTopicKey_t;
    typedef DDS_SampleIdentity_t      SampleIdentity_t;
    typedef DDS_WriteParams_t         WriteParams_t;

    typedef DDS_ReturnCode_t          ReturnCode_t;
    typedef DDS_QosPolicyId_t         QosPolicyId_t;

    typedef DDS_Duration_t            Duration_t;
    typedef DDS_Time_t                Time_t;

    typedef DDS_SequenceNumber_t      SequenceNumber_t;
    typedef DDS_GUID_t                GUID_t;

    typedef DDS_Locator_t             Locator_t;
    typedef DDS_LocatorSeq            LocatorSeq;
    typedef DDS_ProtocolVersion_t     ProtocolVersion_t;
    typedef DDS_VendorId_t            VendorId_t;
    typedef DDS_ProductVersion_t      ProductVersion_t;

    typedef DDS_RtpsWellKnownPorts_t             RtpsWellKnownPorts_t;

    typedef DDS_ChecksumKindMask_t    ChecksumKindMask_t;
    typedef DDS_ChecksumKind_t        ChecksumKind_t;

    typedef DDS_Property_t                  Property_t;
    typedef DDS_PropertySeq                 PropertySeq;
    typedef DDS_PropertyQosPolicy           PropertyQosPolicy;
    typedef DDSPropertyQosPolicyHelper      PropertyQosPolicyHelper;

    // ----------------------------------------------------------------------
    // Pre-defined values
    // ----------------------------------------------------------------------

    extern DDSCPPDllVariable const DDS_InstanceHandle_t      &HANDLE_NIL;

    extern DDSCPPDllVariable const DDS_Long         &LENGTH_UNLIMITED;

    extern DDSCPPDllVariable const DDS_Long         &DURATION_INFINITE_SEC;
    extern DDSCPPDllVariable const DDS_UnsignedLong &DURATION_INFINITE_NSEC;
    extern DDSCPPDllVariable const DDS_Duration_t   &DURATION_INFINITE;

    extern DDSCPPDllVariable const DDS_Long         &DURATION_ZERO_SEC;
    extern DDSCPPDllVariable const DDS_UnsignedLong &DURATION_ZERO_NSEC;
    extern DDSCPPDllVariable const DDS_Duration_t   &DURATION_ZERO;

    extern DDSCPPDllVariable const DDS_Long         &DURATION_AUTO_SEC;
    extern DDSCPPDllVariable const DDS_UnsignedLong &DURATION_AUTO_NSEC;
    extern DDSCPPDllVariable const DDS_Duration_t   &DURATION_AUTO;

    extern DDSCPPDllVariable const DDS_Time_t       &TIME_INVALID;

    /*i deprecated */
    extern DDSCPPDllVariable const DDS_Long         &DURATION_INFINITY_SEC;
    extern DDSCPPDllVariable const DDS_UnsignedLong &DURATION_INFINITY_NSEC;
    extern DDSCPPDllVariable const DDS_Long         &TIMESTAMP_INVALID_SEC;
    extern DDSCPPDllVariable const DDS_UnsignedLong &TIMESTAMP_INVALID_NSEC;

    extern DDSCPPDllVariable const DDS_SequenceNumber_t &SEQUENCE_NUMBER_UNKNOWN;
    extern DDSCPPDllVariable const DDS_SequenceNumber_t &SEQUENCE_NUMBER_ZERO;
    extern DDSCPPDllVariable const DDS_SequenceNumber_t &SEQUENCE_NUMBER_MAX;
    extern DDSCPPDllVariable const DDS_SequenceNumber_t &AUTO_SEQUENCE_NUMBER;

    extern DDSCPPDllVariable const DDS_SampleIdentity_t &AUTO_SAMPLE_IDENTITY;
    extern DDSCPPDllVariable const DDS_SampleIdentity_t &UNKNOWN_SAMPLE_IDENTITY;

    extern DDSCPPDllVariable const DDS_WriteParams_t &WRITEPARAMS_DEFAULT;

    extern DDSCPPDllVariable const DDS_GUID_t &GUID_UNKNOWN;

    extern DDSCPPDllVariable const DDS_Locator_t         &LOCATOR_INVALID;

    extern DDSCPPDllVariable const DDS_RtpsWellKnownPorts_t &RTI_BACKWARDS_COMPATIBLE_RTPS_WELL_KNOWN_PORTS;

    extern DDSCPPDllVariable const DDS_RtpsWellKnownPorts_t &INTEROPERABLE_RTPS_WELL_KNOWN_PORTS;


    // ----------------------------------------------------------------------
    // Return codes
    // ----------------------------------------------------------------------
    extern DDSCPPDllVariable const DDS_ReturnCode_t RETCODE_OK;
    extern DDSCPPDllVariable const DDS_ReturnCode_t RETCODE_ERROR;
    extern DDSCPPDllVariable const DDS_ReturnCode_t RETCODE_UNSUPPORTED;
    extern DDSCPPDllVariable const DDS_ReturnCode_t RETCODE_BAD_PARAMETER;
    extern DDSCPPDllVariable const DDS_ReturnCode_t RETCODE_PRECONDITION_NOT_MET;
    extern DDSCPPDllVariable const DDS_ReturnCode_t RETCODE_OUT_OF_RESOURCES;
    extern DDSCPPDllVariable const DDS_ReturnCode_t RETCODE_NOT_ENABLED;
    extern DDSCPPDllVariable const DDS_ReturnCode_t RETCODE_IMMUTABLE_POLICY;
    extern DDSCPPDllVariable const DDS_ReturnCode_t RETCODE_INCONSISTENT_POLICY;
    extern DDSCPPDllVariable const DDS_ReturnCode_t RETCODE_ALREADY_DELETED;
    extern DDSCPPDllVariable const DDS_ReturnCode_t RETCODE_TIMEOUT;
    extern DDSCPPDllVariable const DDS_ReturnCode_t RETCODE_NO_DATA;
    extern DDSCPPDllVariable const DDS_ReturnCode_t RETCODE_ILLEGAL_OPERATION;

    // ----------------------------------------------------------------------
    // Status to support listeners and conditions
    // ----------------------------------------------------------------------

    typedef DDS_StatusKind StatusKind;
    typedef DDS_StatusMask StatusMask;

    extern DDSCPPDllVariable const DDS_StatusKind DATA_AVAILABLE_STATUS;

    // ----------------------------------------------------------------------
    // Listeners
    // ----------------------------------------------------------------------
    typedef DDSListener                   Listener;
    typedef DDSDataWriterListener         DataWriterListener;
    typedef DDSPublisherListener          PublisherListener;
    typedef DDSDataReaderListener         DataReaderListener;
    typedef DDSSubscriberListener         SubscriberListener;
    typedef DDSDomainParticipantListener  DomainParticipantListener;

    // ----------------------------------------------------------------------
    // Conditions
    // ----------------------------------------------------------------------
    typedef DDSCondition        Condition;
    typedef DDSConditionSeq     ConditionSeq;

    typedef DDSWaitSet          WaitSet;

    typedef DDSGuardCondition   GuardCondition;
    typedef DDSStatusCondition  StatusCondition;

    // Sample states to support reads
    typedef DDS_SampleStateKind  SampleStateKind;
    extern DDSCPPDllVariable const DDS_SampleStateKind        READ_SAMPLE_STATE;
    extern DDSCPPDllVariable const DDS_SampleStateKind        NOT_READ_SAMPLE_STATE;

    // This is a bit-mask SampleStateKind
    typedef DDS_SampleStateMask  SampleStateMask;

    // View states to support reads
    typedef DDS_ViewStateKind    ViewStateKind;
    extern DDSCPPDllVariable const DDS_ViewStateKind          NEW_VIEW_STATE;
    extern DDSCPPDllVariable const DDS_ViewStateKind          NOT_NEW_VIEW_STATE;

    // This is a bit-mask ViewStateKind
    typedef DDS_ViewStateMask    ViewStateMask;

    // Instance states to support reads
    typedef DDS_InstanceStateKind InstanceStateKind;
    extern DDSCPPDllVariable const DDS_InstanceStateKind       ALIVE_INSTANCE_STATE;
    extern DDSCPPDllVariable const DDS_InstanceStateKind       NOT_ALIVE_DISPOSED_INSTANCE_STATE;
    extern DDSCPPDllVariable const DDS_InstanceStateKind       NOT_ALIVE_NO_WRITERS_INSTANCE_STATE;

    // This is a bit-mask InstanceStateKind
    typedef DDS_InstanceStateMask InstanceStateMask;

    typedef DDS_SampleInfo               SampleInfo;
    typedef DDS_SampleInfoSeq            SampleInfoSeq;

    extern DDSCPPDllVariable const DDS_SampleStateMask        &ANY_SAMPLE_STATE;
    extern DDSCPPDllVariable const DDS_ViewStateMask          &ANY_VIEW_STATE;
    extern DDSCPPDllVariable const DDS_InstanceStateMask      &ANY_INSTANCE_STATE;
    extern DDSCPPDllVariable const DDS_InstanceStateMask      &NOT_ALIVE_INSTANCE_STATE;


    // ----------------------------------------------------------------------
    // Qos
    // ----------------------------------------------------------------------

    extern DDSCPPDllVariable const DDS_QosPolicyId_t DURABILITY_QOS_POLICY_ID;
    extern DDSCPPDllVariable const DDS_QosPolicyId_t PRESENTATION_QOS_POLICY_ID;
    extern DDSCPPDllVariable const DDS_QosPolicyId_t DEADLINE_QOS_POLICY_ID;
    extern DDSCPPDllVariable const DDS_QosPolicyId_t LATENCYBUDGET_QOS_POLICY_ID;
    extern DDSCPPDllVariable const DDS_QosPolicyId_t OWNERSHIP_QOS_POLICY_ID;
    extern DDSCPPDllVariable const DDS_QosPolicyId_t OWNERSHIPSTRENGTH_QOS_POLICY_ID;
    extern DDSCPPDllVariable const DDS_QosPolicyId_t LIVELINESS_QOS_POLICY_ID;
    extern DDSCPPDllVariable const DDS_QosPolicyId_t TIMEBASEDFILTER_QOS_POLICY_ID;
    extern DDSCPPDllVariable const DDS_QosPolicyId_t PARTITION_QOS_POLICY_ID;
    extern DDSCPPDllVariable const DDS_QosPolicyId_t RELIABILITY_QOS_POLICY_ID;
    extern DDSCPPDllVariable const DDS_QosPolicyId_t DESTINATIONORDER_QOS_POLICY_ID;
    extern DDSCPPDllVariable const DDS_QosPolicyId_t HISTORY_QOS_POLICY_ID;
    extern DDSCPPDllVariable const DDS_QosPolicyId_t RESOURCELIMITS_QOS_POLICY_ID;
    extern DDSCPPDllVariable const DDS_QosPolicyId_t ENTITYFACTORY_QOS_POLICY_ID;
    extern DDSCPPDllVariable const DDS_QosPolicyId_t WRITERDATALIFECYCLE_QOS_POLICY_ID;
    extern DDSCPPDllVariable const DDS_QosPolicyId_t READERDATALIFECYCLE_QOS_POLICY_ID;
    extern DDSCPPDllVariable const DDS_QosPolicyId_t TOPICDATA_QOS_POLICY_ID;
    extern DDSCPPDllVariable const DDS_QosPolicyId_t GROUPDATA_QOS_POLICY_ID;
    extern DDSCPPDllVariable const DDS_QosPolicyId_t TRANSPORTPRIORITY_QOS_POLICY_ID;
    extern DDSCPPDllVariable const DDS_QosPolicyId_t LIFESPAN_QOS_POLICY_ID;
    extern DDSCPPDllVariable const DDS_QosPolicyId_t DURABILITYSERVICE_QOS_POLICY_ID;

    typedef DDS_DurabilityQosPolicyKind     DurabilityQosPolicyKind;
    extern DDSCPPDllVariable const DDS_DurabilityQosPolicyKind  VOLATILE_DURABILITY_QOS;
    extern DDSCPPDllVariable const DDS_DurabilityQosPolicyKind  TRANSIENT_LOCAL_DURABILITY_QOS;
    typedef DDS_DurabilityQosPolicy                         DurabilityQosPolicy;

    typedef DDS_DestinationOrderQosPolicyKind  DestinationOrderQosPolicyKind;
    extern DDSCPPDllVariable const DDS_DestinationOrderQosPolicyKind  BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS;
    extern DDSCPPDllVariable const DDS_DestinationOrderQosPolicyKind  BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
    typedef DDS_DestinationOrderQosPolicy      DestinationOrderQosPolicy;

    typedef DDS_DeadlineQosPolicy                           DeadlineQosPolicy;

    typedef DDS_OwnershipQosPolicyKind      OwnershipQosPolicyKind;
    extern DDSCPPDllVariable const DDS_OwnershipQosPolicyKind SHARED_OWNERSHIP_QOS;
    extern DDSCPPDllVariable const DDS_OwnershipQosPolicyKind EXCLUSIVE_OWNERSHIP_QOS;
    typedef DDS_OwnershipQosPolicy                          OwnershipQosPolicy;
    typedef DDS_OwnershipStrengthQosPolicy                  OwnershipStrengthQosPolicy;

    typedef DDS_LivelinessQosPolicyKind     LivelinessQosPolicyKind;
    extern DDSCPPDllVariable const DDS_LivelinessQosPolicyKind  AUTOMATIC_LIVELINESS_QOS;
    extern DDSCPPDllVariable const DDS_LivelinessQosPolicyKind  MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
    extern DDSCPPDllVariable const DDS_LivelinessQosPolicyKind  MANUAL_BY_TOPIC_LIVELINESS_QOS;
    typedef DDS_LivelinessQosPolicy                         LivelinessQosPolicy;

    typedef DDS_ReliabilityQosPolicyKind    ReliabilityQosPolicyKind;
    extern DDSCPPDllVariable const DDS_ReliabilityQosPolicyKind BEST_EFFORT_RELIABILITY_QOS;
    extern DDSCPPDllVariable const DDS_ReliabilityQosPolicyKind RELIABLE_RELIABILITY_QOS;
    typedef DDS_ReliabilityQosPolicy                        ReliabilityQosPolicy;

    typedef DDS_HistoryQosPolicyKind HistoryQosPolicyKind;
    extern DDSCPPDllVariable const DDS_HistoryQosPolicyKind KEEP_LAST_HISTORY_QOS;
    extern DDSCPPDllVariable const DDS_HistoryQosPolicyKind KEEP_ALL_HISTORY_QOS;
    typedef DDS_HistoryQosPolicy                            HistoryQosPolicy;

    typedef DDS_ResourceLimitsQosPolicy                     ResourceLimitsQosPolicy;
    typedef DDS_EntityFactoryQosPolicy                      EntityFactoryQosPolicy;
    typedef DDS_DiscoveryQosPolicy                          DiscoveryQosPolicy;
    typedef DDS_EntityNameQosPolicy                         EntityNameQosPolicy;

    typedef DDS_WireProtocolQosPolicy                       WireProtocolQosPolicy;
    extern DDSCPPDllVariable const int RTPS_AUTO_ID;

    typedef DDS_DataReaderResourceLimitsQosPolicy           DataReaderResourceLimitsQosPolicy;
    typedef DDS_DataReaderProtocolQosPolicy                 DataReaderProtocolQosPolicy;
    typedef DDS_DataWriterProtocolQosPolicy                 DataWriterProtocolQosPolicy;
    typedef DDS_SystemResourceLimitsQosPolicy               SystemResourceLimitsQosPolicy;
    typedef DDS_DomainParticipantResourceLimitsQosPolicy    DomainParticipantResourceLimitsQosPolicy;

    typedef DDS_TypeSupportQosPolicy                        TypeSupportQosPolicy;

    typedef DDS_HANDLE_TYPE_NATIVE         HANDLE_TYPE_NATIVE;

#if DDS_FLOW_CONTROLLER_ENABLED
    typedef DDS_FlowControllerSchedulingPolicy FlowControllerSchedulingPolicy;
    extern DDSCPPDllVariable const DDS_FlowControllerSchedulingPolicy RR_FLOW_CONTROLLER_SCHED_POLICY;
    extern DDSCPPDllVariable const DDS_FlowControllerSchedulingPolicy EDF_FLOW_CONTROLLER_SCHED_POLICY;
    extern DDSCPPDllVariable const DDS_FlowControllerSchedulingPolicy HPF_FLOW_CONTROLLER_SCHED_POLICY;

    typedef DDS_PublishModeQosPolicy PublishModeQosPolicy;
    typedef DDS_PublishModeQosPolicyKind PublishModeQosPolicyKind;
    extern DDSCPPDllVariable const DDS_PublishModeQosPolicyKind SYNCHRONOUS_PUBLISH_MODE_QOS;
    extern DDSCPPDllVariable const DDS_PublishModeQosPolicyKind ASYNCHRONOUS_PUBLISH_MODE_QOS;
    extern DDSCPPDllVariable const DDS_PublishModeQosPolicyKind AUTOMATIC_PUBLISH_MODE_QOS;

    // ----------------------------------------------------------------------
    // Flow-Controller names
    // ----------------------------------------------------------------------
    extern DDSCPPDllVariable const char *const &DEFAULT_FLOW_CONTROLLER_NAME;
    extern DDSCPPDllVariable const char *const &FIXED_RATE_FLOW_CONTROLLER_NAME;
    extern DDSCPPDllVariable const char *const &ON_DEMAND_FLOW_CONTROLLER_NAME;

#endif

    // ----------------------------------------------------------------------
    typedef DDS_DomainParticipantFactoryQos  DomainParticipantFactoryQos;
    typedef DDS_DomainParticipantQos         DomainParticipantQos;
    typedef DDS_TopicQos                     TopicQos;
    typedef DDS_DataWriterQos                DataWriterQos;
    typedef DDS_PublisherQos                 PublisherQos;
    typedef DDS_DataReaderQos                DataReaderQos;
    typedef DDS_SubscriberQos                SubscriberQos;

    // ----------------------------------------------------------------------

    extern DDSCPPDllVariable const DDS_DomainParticipantQos &PARTICIPANT_QOS_DEFAULT;
    extern DDSCPPDllVariable const DDS_TopicQos             &TOPIC_QOS_DEFAULT;
    extern DDSCPPDllVariable const DDS_PublisherQos         &PUBLISHER_QOS_DEFAULT;
    extern DDSCPPDllVariable const DDS_SubscriberQos        &SUBSCRIBER_QOS_DEFAULT;
    extern DDSCPPDllVariable const DDS_DataWriterQos        &DATAWRITER_QOS_DEFAULT;
    extern DDSCPPDllVariable const DDS_DataReaderQos        &DATAREADER_QOS_DEFAULT;

    extern DDSCPPDllVariable const DDS_DataWriterQos        &DATAWRITER_QOS_USE_TOPIC_QOS;
    extern DDSCPPDllVariable const DDS_DataReaderQos        &DATAREADER_QOS_USE_TOPIC_QOS;

    // ----------------------------------------------------------------------
    // Builtin Topic type definition
    // ----------------------------------------------------------------------
    typedef DDS_ParticipantBuiltinTopicData  ParticipantBuiltinTopicData;
    typedef DDS_PublicationBuiltinTopicData  PublicationBuiltinTopicData;
    typedef DDS_SubscriptionBuiltinTopicData SubscriptionBuiltinTopicData;

    extern DDSCPPDllVariable const char * &PARTICIPANT_TOPIC_NAME;
    extern DDSCPPDllVariable const char * &PUBLICATION_TOPIC_NAME;
    extern DDSCPPDllVariable const char * &SUBSCRIPTION_TOPIC_NAME;

    // ----------------------------------------------------------------------
    // Entities
    // ----------------------------------------------------------------------
    typedef DDSEntity                    Entity;
    typedef DDSDomainParticipant         DomainParticipant;
    typedef DDSDomainParticipantFactory  DomainParticipantFactory;
    typedef DDSTopicDescription          TopicDescription;
    typedef DDSTopic                     Topic;
    typedef DDSPublisher                 Publisher;
    typedef DDSDataWriter                DataWriter;
    typedef DDSSubscriber                Subscriber;
    typedef DDSDataReader                DataReader;

    // ----------------------------------------------------------------------
    // DDS Statuses
    // ----------------------------------------------------------------------
    typedef struct DDS_SubscriptionMatchedStatus            SubscriptionMatchedStatus;
    typedef struct DDS_LivelinessChangedStatus              LivelinessChangedStatus;
    typedef struct DDS_RequestedDeadlineMissedStatus        RequestedDeadlineMissedStatus;
    typedef struct DDS_RequestedIncompatibleQosStatus       RequestedIncompatibleQosStatus;
    typedef struct DDS_SampleRejectedStatus                 SampleRejectedStatus;
    typedef struct DDS_SampleLostStatus                     SampleLostStatus;
    typedef struct DDS_DataReaderInstanceReplacedStatus     DataReaderInstanceReplacedStatus;

    typedef struct DDS_PublicationMatchedStatus             PublicationMatchedStatus;
    typedef struct DDS_OfferedDeadlineMissedStatus          OfferedDeadlineMissedStatus;
    typedef struct DDS_OfferedIncompatibleQosStatus         OfferedIncompatibleQosStatus;
    typedef struct DDS_LivelinessLostStatus                 LivelinessLostStatus;
    typedef struct DDS_ReliableReaderActivityChangedStatus  ReliableReaderActivityChangedStatus;
    typedef struct DDS_ReliableSampleUnacknowledgedStatus   ReliableSampleUnacknowledgedStatus;

    typedef struct DDS_InconsistentTopicStatus              InconsistentTopicStatus;

    // ----------------------------------------------------------------------
    // DDS Macros -> constants
    // ----------------------------------------------------------------------

    /*infrastructure*/
    extern DDSCPPDllVariable const StatusMask STATUS_MASK_NONE;
    extern DDSCPPDllVariable const StatusMask STATUS_MASK_ALL;

    /*common*/
    extern DDSCPPDllVariable const DDS_Boolean BOOLEAN_TRUE;
    extern DDSCPPDllVariable const DDS_Boolean BOOLEAN_FALSE;

    extern DDSCPPDllVariable const DDS_ProtocolVersion_t PROTOCOLVERSION_1_0;
    extern DDSCPPDllVariable const DDS_ProtocolVersion_t PROTOCOLVERSION_1_1;
    extern DDSCPPDllVariable const DDS_ProtocolVersion_t PROTOCOLVERSION_1_2;
    extern DDSCPPDllVariable const DDS_ProtocolVersion_t PROTOCOLVERSION_2_0;
    extern DDSCPPDllVariable const DDS_ProtocolVersion_t PROTOCOLVERSION_2_1;
    extern DDSCPPDllVariable const DDS_ProtocolVersion_t PROTOCOLVERSION;
    extern DDSCPPDllVariable const DDS_VendorId_t        VENDORID_UNKNOWN;

    // ----------------------------------------------------------------------
    // String manipulation functions
    // ----------------------------------------------------------------------

    inline char * String_alloc(DDS_UnsignedLong length)
    {
        return DDS_String_alloc(length);
    }

    inline char* String_dup(const char* str)
    {
        return DDS_String_dup(str);
    }

    inline char* String_replace(char** string_ptr, const char* new_value)
    {
        return DDS_String_replace(string_ptr, new_value);
    }

#ifndef RTI_CERT
    inline void String_free(char* str)
    {
        DDS_String_free(str);
    }
#endif
}

namespace RT {
    typedef RTRegistry                          Registry;
    typedef struct RT_ComponentFactoryI         ComponentFactoryI;
}

namespace OSAPI {
#if OSAPI_ENABLE_LOG
    typedef OSAPILog                            Log;
#endif
}

namespace NETIO {
    extern DDSCPPDllVariable const char*        DEFAULT_UDP_NAME;
    extern DDSCPPDllVariable const char*        DEFAULT_INTRA_NAME;
    extern DDSCPPDllVariable const char*        DEFAULT_RTPS_NAME;

    namespace SHMEM {
        typedef SHMEMInterfaceFactory                      InterfaceFactory;
        typedef struct NETIO_SHMEMInterfaceFactoryProperty InterfaceFactoryProperty;
    }
}


#if RTIME_CPP_ENABLE_UDP_NAMESPACE
namespace UDP {
    typedef UDPInterfaceFactory                 InterfaceFactory;
    typedef UDPInterfaceTable                   InterfaceTable;

    typedef struct UDP_InterfaceTableEntry      InterfaceTableEntry;
    typedef struct UDP_InterfaceFactoryProperty InterfaceFactoryProperty;
    typedef struct UDP_InterfaceTableEntrySeq   InterfaceTableEntrySeq;

#ifndef RTI_CERT
    typedef struct UDP_NatEntry                 NatEntry;
    typedef struct UDP_NatEntrySeq              NatEntrySeq;
#endif
}
#endif

namespace DPSE {
    typedef DPSEDiscoveryFactory                DiscoveryFactory;
    typedef DPSEDiscoveryPlugin                 DiscoveryPlugin;
    typedef DPSE_DiscoveryPluginProperty        DiscoveryPluginProperty;
}

namespace DPDE {
    typedef DPDEDiscoveryFactory                DiscoveryFactory;
    typedef DPDE_DiscoveryPluginProperty        DiscoveryPluginProperty;
}

namespace RHSM {
    typedef RHSMHistoryFactory                  HistoryFactory;
    typedef struct RHSM_HistoryProperty         HistoryProperty;
}

namespace WHSM {
    typedef WHSMHistoryFactory                  HistoryFactory;
    typedef struct WHSM_HistoryProperty         HistoryProperty;
}

#endif /* dds_cpp_namespace_hxx */
