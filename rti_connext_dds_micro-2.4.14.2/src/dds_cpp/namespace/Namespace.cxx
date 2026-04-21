/* 
 *
 * (c) Copyright, Real-Time Innovations, 2013-2015.
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification history
 * -------------------- 
 * 05may2020,tk MICRO-2364/PR#27563 Added missing SOURCE_BEGIN
 */


#ifndef dds_cpp_dll_hxx
  #include "dds_cpp/dds_cpp_dll.hxx"
#endif
#ifndef dds_cpp_infrastructure_hxx
  #include "dds_cpp/dds_cpp_infrastructure.hxx"
#endif
#ifndef dds_cpp_topic_hxx
  #include "dds_cpp/dds_cpp_topic.hxx"
#endif
#ifndef dds_cpp_publication_hxx
  #include "dds_cpp/dds_cpp_publication.hxx"
#endif
#ifndef dds_cpp_subscription_hxx
  #include "dds_cpp/dds_cpp_subscription.hxx"
#endif
#ifndef dds_cpp_domain_hxx
  #include "dds_cpp/dds_cpp_domain.hxx"
#endif
#ifndef dds_cpp_namespace_hxx
  #include "dds_cpp/dds_cpp_namespace.hxx"
#endif

/*** SOURCE_BEGIN ***/

    // ----------------------------------------------------------------------
    // Pre-defined values
    // ----------------------------------------------------------------------
    const DDS_InstanceHandle_t &DDS::HANDLE_NIL = DDS_HANDLE_NIL;

    const DDS_Long &DDS::LENGTH_UNLIMITED = DDS_LENGTH_UNLIMITED;

    const DDS_Long         &DDS::DURATION_INFINITE_SEC   = DDS_DURATION_INFINITE_SEC;
    const DDS_UnsignedLong &DDS::DURATION_INFINITE_NSEC  = DDS_DURATION_INFINITE_NSEC;
    const DDS_Duration_t   &DDS::DURATION_INFINITE       = DDS_DURATION_INFINITE;

    const DDS_Long         &DDS::DURATION_ZERO_SEC   = DDS_DURATION_ZERO_SEC;
    const DDS_UnsignedLong &DDS::DURATION_ZERO_NSEC  = DDS_DURATION_ZERO_NSEC;
    const DDS_Duration_t   &DDS::DURATION_ZERO       = DDS_DURATION_ZERO;

    const DDS_GUID_t &DDS::GUID_UNKNOWN = DDS_GUID_UNKNOWN;
    const DDS_GUID_t &DDS::GUID_AUTO = DDS_GUID_AUTO;

    const DDS_Locator_t        &DDS::LOCATOR_INVALID = DDS_LOCATOR_INVALID;    



    // ----------------------------------------------------------------------
    // Return codes
    // ----------------------------------------------------------------------
    const DDS_ReturnCode_t DDS::RETCODE_OK                    = DDS_RETCODE_OK;
    const DDS_ReturnCode_t DDS::RETCODE_ERROR                 = DDS_RETCODE_ERROR;
    const DDS_ReturnCode_t DDS::RETCODE_UNSUPPORTED           = DDS_RETCODE_UNSUPPORTED;
    const DDS_ReturnCode_t DDS::RETCODE_BAD_PARAMETER         = DDS_RETCODE_BAD_PARAMETER;
    const DDS_ReturnCode_t DDS::RETCODE_PRECONDITION_NOT_MET  = DDS_RETCODE_PRECONDITION_NOT_MET;
    const DDS_ReturnCode_t DDS::RETCODE_OUT_OF_RESOURCES      = DDS_RETCODE_OUT_OF_RESOURCES;
    const DDS_ReturnCode_t DDS::RETCODE_NOT_ENABLED           = DDS_RETCODE_NOT_ENABLED;
    const DDS_ReturnCode_t DDS::RETCODE_IMMUTABLE_POLICY      = DDS_RETCODE_IMMUTABLE_POLICY;
    const DDS_ReturnCode_t DDS::RETCODE_INCONSISTENT_POLICY   = DDS_RETCODE_INCONSISTENT_POLICY;
    const DDS_ReturnCode_t DDS::RETCODE_ALREADY_DELETED       = DDS_RETCODE_ALREADY_DELETED;
    const DDS_ReturnCode_t DDS::RETCODE_TIMEOUT               = DDS_RETCODE_TIMEOUT;
    const DDS_ReturnCode_t DDS::RETCODE_NO_DATA               = DDS_RETCODE_NO_DATA;
    const DDS_ReturnCode_t DDS::RETCODE_ILLEGAL_OPERATION     = DDS_RETCODE_ILLEGAL_OPERATION;

    // ----------------------------------------------------------------------
    // Status to support listeners and conditions
    // ----------------------------------------------------------------------

    const DDS_StatusKind DDS::DATA_AVAILABLE_STATUS              = DDS_DATA_AVAILABLE_STATUS;


    // ----------------------------------------------------------------------
    // Conditions
    // ----------------------------------------------------------------------

    const DDS_SampleStateKind        DDS::READ_SAMPLE_STATE                  = DDS_READ_SAMPLE_STATE;
    const DDS_SampleStateKind        DDS::NOT_READ_SAMPLE_STATE              = DDS_NOT_READ_SAMPLE_STATE;
    const DDS_ViewStateKind          DDS::NEW_VIEW_STATE                        = DDS_NEW_VIEW_STATE;
    const DDS_ViewStateKind          DDS::NOT_NEW_VIEW_STATE                    = DDS_NOT_NEW_VIEW_STATE;
    const DDS_InstanceStateKind      DDS::ALIVE_INSTANCE_STATE                  = DDS_ALIVE_INSTANCE_STATE;
    const DDS_InstanceStateKind      DDS::NOT_ALIVE_DISPOSED_INSTANCE_STATE     = DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE;
    const DDS_InstanceStateKind      DDS::NOT_ALIVE_NO_WRITERS_INSTANCE_STATE   = DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE;

    const DDS_SampleStateMask        &DDS::ANY_SAMPLE_STATE                      = DDS_ANY_SAMPLE_STATE;
    const DDS_ViewStateMask          &DDS::ANY_VIEW_STATE                        = DDS_ANY_VIEW_STATE;
    const DDS_InstanceStateMask      &DDS::ANY_INSTANCE_STATE                    = DDS_ANY_INSTANCE_STATE;
    const DDS_InstanceStateMask      &DDS::NOT_ALIVE_INSTANCE_STATE              = DDS_NOT_ALIVE_INSTANCE_STATE;

    // ----------------------------------------------------------------------
    // Qos
    // ----------------------------------------------------------------------

    const DDS_QosPolicyId_t DDS::DURABILITY_QOS_POLICY_ID           = DDS_DURABILITY_QOS_POLICY_ID           ;
    const DDS_QosPolicyId_t DDS::PRESENTATION_QOS_POLICY_ID         = DDS_PRESENTATION_QOS_POLICY_ID         ;
    const DDS_QosPolicyId_t DDS::DEADLINE_QOS_POLICY_ID             = DDS_DEADLINE_QOS_POLICY_ID             ;
    const DDS_QosPolicyId_t DDS::LATENCYBUDGET_QOS_POLICY_ID        = DDS_LATENCYBUDGET_QOS_POLICY_ID        ;
    const DDS_QosPolicyId_t DDS::OWNERSHIP_QOS_POLICY_ID            = DDS_OWNERSHIP_QOS_POLICY_ID            ;
    const DDS_QosPolicyId_t DDS::OWNERSHIPSTRENGTH_QOS_POLICY_ID    = DDS_OWNERSHIPSTRENGTH_QOS_POLICY_ID    ;
    const DDS_QosPolicyId_t DDS::LIVELINESS_QOS_POLICY_ID           = DDS_LIVELINESS_QOS_POLICY_ID           ;
    const DDS_QosPolicyId_t DDS::TIMEBASEDFILTER_QOS_POLICY_ID      = DDS_TIMEBASEDFILTER_QOS_POLICY_ID      ;
    const DDS_QosPolicyId_t DDS::PARTITION_QOS_POLICY_ID            = DDS_PARTITION_QOS_POLICY_ID            ;
    const DDS_QosPolicyId_t DDS::RELIABILITY_QOS_POLICY_ID          = DDS_RELIABILITY_QOS_POLICY_ID          ;
    const DDS_QosPolicyId_t DDS::DESTINATIONORDER_QOS_POLICY_ID     = DDS_DESTINATIONORDER_QOS_POLICY_ID     ;
    const DDS_QosPolicyId_t DDS::HISTORY_QOS_POLICY_ID              = DDS_HISTORY_QOS_POLICY_ID              ;
    const DDS_QosPolicyId_t DDS::RESOURCELIMITS_QOS_POLICY_ID       = DDS_RESOURCELIMITS_QOS_POLICY_ID       ;
    const DDS_QosPolicyId_t DDS::ENTITYFACTORY_QOS_POLICY_ID        = DDS_ENTITYFACTORY_QOS_POLICY_ID        ;
    const DDS_QosPolicyId_t DDS::WRITERDATALIFECYCLE_QOS_POLICY_ID  = DDS_WRITERDATALIFECYCLE_QOS_POLICY_ID  ;
    const DDS_QosPolicyId_t DDS::READERDATALIFECYCLE_QOS_POLICY_ID  = DDS_READERDATALIFECYCLE_QOS_POLICY_ID  ;
    const DDS_QosPolicyId_t DDS::TOPICDATA_QOS_POLICY_ID            = DDS_TOPICDATA_QOS_POLICY_ID            ; 
    const DDS_QosPolicyId_t DDS::GROUPDATA_QOS_POLICY_ID            = DDS_GROUPDATA_QOS_POLICY_ID            ; 
    const DDS_QosPolicyId_t DDS::TRANSPORTPRIORITY_QOS_POLICY_ID    = DDS_TRANSPORTPRIORITY_QOS_POLICY_ID    ; 
    const DDS_QosPolicyId_t DDS::LIFESPAN_QOS_POLICY_ID             = DDS_LIFESPAN_QOS_POLICY_ID             ; 
    const DDS_QosPolicyId_t DDS::DURABILITYSERVICE_QOS_POLICY_ID    = DDS_DURABILITYSERVICE_QOS_POLICY_ID    ; 
 
    const DDS_DurabilityQosPolicyKind  DDS::VOLATILE_DURABILITY_QOS        = DDS_VOLATILE_DURABILITY_QOS;
    const DDS_DurabilityQosPolicyKind  DDS::TRANSIENT_LOCAL_DURABILITY_QOS = DDS_TRANSIENT_LOCAL_DURABILITY_QOS;
    const DDS_OwnershipQosPolicyKind DDS::SHARED_OWNERSHIP_QOS    = DDS_SHARED_OWNERSHIP_QOS;
    const DDS_OwnershipQosPolicyKind DDS::EXCLUSIVE_OWNERSHIP_QOS = DDS_EXCLUSIVE_OWNERSHIP_QOS;
    const DDS_LivelinessQosPolicyKind  DDS::AUTOMATIC_LIVELINESS_QOS             = DDS_AUTOMATIC_LIVELINESS_QOS;
    const DDS_LivelinessQosPolicyKind  DDS::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS = DDS_MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
    const DDS_LivelinessQosPolicyKind  DDS::MANUAL_BY_TOPIC_LIVELINESS_QOS       = DDS_MANUAL_BY_TOPIC_LIVELINESS_QOS;
    const DDS_ReliabilityQosPolicyKind DDS::BEST_EFFORT_RELIABILITY_QOS   = DDS_BEST_EFFORT_RELIABILITY_QOS;
    const DDS_ReliabilityQosPolicyKind DDS::RELIABLE_RELIABILITY_QOS      = DDS_RELIABLE_RELIABILITY_QOS;
    const DDS_HistoryQosPolicyKind DDS::KEEP_LAST_HISTORY_QOS  = DDS_KEEP_LAST_HISTORY_QOS;
    const DDS_HistoryQosPolicyKind DDS::KEEP_ALL_HISTORY_QOS   = DDS_KEEP_ALL_HISTORY_QOS;
    const DDS_DestinationOrderQosPolicyKind  DDS::BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS = DDS_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS;
    const DDS_DestinationOrderQosPolicyKind  DDS::BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS = DDS_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;

    const int DDS::RTPS_AUTO_ID = DDS_RTPS_AUTO_ID;

    const DDS_DomainParticipantQos &DDS::PARTICIPANT_QOS_DEFAULT = DDS_PARTICIPANT_QOS_DEFAULT;
    const DDS_TopicQos             &DDS::TOPIC_QOS_DEFAULT = DDS_TOPIC_QOS_DEFAULT;
    const DDS_PublisherQos         &DDS::PUBLISHER_QOS_DEFAULT = DDS_PUBLISHER_QOS_DEFAULT;
    const DDS_SubscriberQos        &DDS::SUBSCRIBER_QOS_DEFAULT = DDS_SUBSCRIBER_QOS_DEFAULT;
    const DDS_DataWriterQos        &DDS::DATAWRITER_QOS_DEFAULT = DDS_DATAWRITER_QOS_DEFAULT;
    const DDS_DataReaderQos        &DDS::DATAREADER_QOS_DEFAULT = DDS_DATAREADER_QOS_DEFAULT;

    // ----------------------------------------------------------------------
    // DDS Macros -> constants
    // ----------------------------------------------------------------------

    /*infrastructure*/
    const DDS_StatusMask DDS::STATUS_MASK_NONE = DDS_STATUS_MASK_NONE;
    const DDS_StatusMask DDS::STATUS_MASK_ALL  = DDS_STATUS_MASK_ALL;

    /*common*/
    const DDS_Boolean DDS::BOOLEAN_TRUE = DDS_BOOLEAN_TRUE;
    const DDS_Boolean DDS::BOOLEAN_FALSE = DDS_BOOLEAN_FALSE;

    const DDS_ProtocolVersion_t DDS::PROTOCOLVERSION_1_0 = DDS_PROTOCOLVERSION_1_0;
    const DDS_ProtocolVersion_t DDS::PROTOCOLVERSION_1_1 = DDS_PROTOCOLVERSION_1_1;
    const DDS_ProtocolVersion_t DDS::PROTOCOLVERSION_1_2 = DDS_PROTOCOLVERSION_1_2;
    const DDS_ProtocolVersion_t DDS::PROTOCOLVERSION_2_0 = DDS_PROTOCOLVERSION_2_0;
    const DDS_ProtocolVersion_t DDS::PROTOCOLVERSION_2_1 = DDS_PROTOCOLVERSION_2_1;
    const DDS_ProtocolVersion_t DDS::PROTOCOLVERSION     = DDS_PROTOCOLVERSION;
    const DDS_VendorId_t        DDS::VENDORID_UNKNOWN    = DDS_VENDORID_UNKNOWN;

    //-------------------------------------------------------------------------
    // NETIO constants
    //-------------------------------------------------------------------------
#if !UDP_EXCLUDE_BUILTIN
    const char * NETIO::DEFAULT_UDP_NAME = NETIO_DEFAULT_UDP_NAME;
#endif /* !UDP_EXCLUDE_BUILTIN */
    const char * NETIO::DEFAULT_INTRA_NAME = NETIO_DEFAULT_INTRA_NAME;
    const char * NETIO::DEFAULT_RTPS_NAME = NETIO_DEFAULT_RTPS_NAME;

