/* 

 (c) Copyright, Real-Time Innovations, 2013-2015.
 All rights reserved.

 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.

modification history
--------------------
12feb2016,as  MICRO-1524: Pass status as reference to DDSDomainParticipantListener
21may2014,as  MICRO-754: No Multiple Inheritance for C++ DomainParticipantListener
16may2014,as  MICRO-795: Complete support for listener API in C++
19jul2013,as  Major C++ update
11jan2013,eh  Created. 
===================================================================== */

#ifndef dds_cpp_domain_hxx
#define dds_cpp_domain_hxx

/*i
 * \file
 * \brief DDS Domain Module definitions
 */
/*e
  @addtogroup DDSDomainModule Domain Module

  @brief Defines the \dds domain package
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
#ifndef dds_cpp_rt_hxx
#include "dds_cpp/dds_cpp_rt.hxx"
#endif
#ifndef dds_c_domain_h
  #include "dds_c/dds_c_domain.h"
#endif
#ifndef dds_cpp_flowcontroller_hxx
#include "dds_cpp/dds_cpp_flowcontroller.hxx"
#endif


/* ================================================================= */
/*                       Listeners                                   */
/* ================================================================= */

/*e \dref_DomainParticipantListener
 *
 *  \ndds C++ API avoids the use of multiple inheritance for certification
 *  purposes. For this reason, DDSDomainParticipantListener is not
 *  declared as a sub-class of DDSPublisherListener, DDSSubscriberListener,
 *  and DDSTopicListener, but it explicitly declares all supported
 *  call-backs.
 */
class DDSCPPDllExport DDSDomainParticipantListener
{
  public:
    /*e \dref_DataReaderListener_on_data_available
     */
    virtual void on_data_available(DDSDataReader*) { }

    /*e \dref_DataReaderListener_on_requested_deadline_missed
     */
    virtual void on_requested_deadline_missed(
            DDSDataReader *reader,
            const DDS_RequestedDeadlineMissedStatus& status) {UNUSED_ARG(reader); UNUSED_ARG(status);}

    /*e \dref_DataReaderListener_on_liveliness_changed
     */
    virtual void on_liveliness_changed(
            DDSDataReader *reader,
            const DDS_LivelinessChangedStatus& status) {UNUSED_ARG(reader);UNUSED_ARG(status); }

    /*e \dref_DataReaderListener_on_requested_incompatible_qos
     */
    virtual void on_requested_incompatible_qos(
            DDSDataReader *reader,
            const DDS_RequestedIncompatibleQosStatus& status) {UNUSED_ARG(reader);UNUSED_ARG(status); }

    /*e \dref_DataReaderListener_on_sample_rejected
     */
    virtual void on_sample_rejected(
            DDSDataReader *reader,
            const DDS_SampleRejectedStatus& status) {UNUSED_ARG(reader); UNUSED_ARG(status);}

    /*e \dref_DataReaderListener_on_subscription_matched
     */
    virtual void on_subscription_matched(
            DDSDataReader *reader,
            const DDS_SubscriptionMatchedStatus& status) {UNUSED_ARG(reader);UNUSED_ARG(status); }

    /*e \dref_DataReaderListener_on_sample_lost
     */
    virtual void on_sample_lost(
            DDSDataReader *reader,
            const DDS_SampleLostStatus& status) {UNUSED_ARG(reader);UNUSED_ARG(status); }

    /*e \dref_DataReaderListener_on_instance_replaced
     */
    virtual void on_instance_replaced(
            DDSDataReader *reader,
            const DDS_DataReaderInstanceReplacedStatus& status) {UNUSED_ARG(reader);UNUSED_ARG(status); }

    /*e \dref_DataReaderListener_on_before_sample_deserialize
     */
    virtual DDS_Boolean on_before_sample_deserialize(
            DDSDataReader *reader,
            NDDS_Type_Plugin *,
            CDR_Stream_t *stream,
            DDS_Boolean *)
    {
        UNUSED_ARG(reader);
        UNUSED_ARG(stream);
        return RTI_TRUE;
    }

    /*e \dref_DataReaderListener_on_before_sample_commit
     */
    virtual DDS_Boolean on_before_sample_commit(
            DDSDataReader *,
            const void *const,
            const struct DDS_SampleInfo *const,
            DDS_Boolean *)
    {
        return RTI_TRUE;
    }

    /*e \dref_SubscriberListener_on_data_on_readers
     */
    virtual void on_data_on_readers(DDSSubscriber*) { }

    /*e \dref_DataWriterListener_on_offered_deadline_missed
     */
    virtual void on_offered_deadline_missed(
            DDSDataWriter *writer,
            const struct DDS_OfferedDeadlineMissedStatus& status) {UNUSED_ARG(writer);UNUSED_ARG(status); }

    /*e \dref_DataWriterListener_on_liveliness_lost
     */
    virtual void on_liveliness_lost(
            DDSDataWriter *writer,
            const struct DDS_LivelinessLostStatus& status) {UNUSED_ARG(writer);UNUSED_ARG(status); }

    /*e \dref_DataWriterListener_on_offered_incompatible_qos
     */
    virtual void on_offered_incompatible_qos(
            DDSDataWriter *writer,
            const struct DDS_OfferedIncompatibleQosStatus& status) { UNUSED_ARG(writer);UNUSED_ARG(status);}

    /*e \dref_DataWriterListener_on_publication_matched
     */
    virtual void on_publication_matched(
            DDSDataWriter *writer,
            const struct DDS_PublicationMatchedStatus& status) { UNUSED_ARG(writer);UNUSED_ARG(status);}

    /*e \dref_DataWriterListener_on_reliable_reader_activity_changed
     */
    virtual void on_reliable_reader_activity_changed(
            DDSDataWriter *writer,
             const struct DDS_ReliableReaderActivityChangedStatus& status) {UNUSED_ARG(writer);UNUSED_ARG(status); }

    /*i \dref_DataWriterListener_on_reliable_sample_unacknowledged
     */
    virtual void on_reliable_sample_unacknowledged(
            DDSDataWriter *writer,
            const DDS_ReliableSampleUnacknowledgedStatus& status) {UNUSED_ARG(writer);UNUSED_ARG(status); }

    /*e \dref_TopicListener_on_inconsistent_topic
     */
    virtual void on_inconsistent_topic(
            DDSTopic *topic,
            const DDS_InconsistentTopicStatus& status) { UNUSED_ARG(topic);UNUSED_ARG(status);}

  public:
    DDSDomainParticipantListener() { }
    virtual ~DDSDomainParticipantListener() { }
};

/* ================================================================= */
/*                       DDSDomainParticipant                        */
/* ================================================================= */

class DDSDomainParticipantFactory;
class DDSPublisherListener;
class DDSSubscriberListener;


/* ----------------------------------------------------------------- */
/*e \dref_DomainParticipant
 */
class DDSCPPDllExport DDSDomainParticipant : public DDSEntity
{

  // --- <<interface>> DDSDomainParticipant: --------------------------
  public:

    /*e \dref_DomainParticipant_as_entity
     */
    virtual DDSEntity* as_entity() = 0;

    /*e \dref_DomainParticipant_create_publisher
     */
    virtual DDSPublisher* create_publisher(
            const DDS_PublisherQos& qos,
            DDSPublisherListener* listener,
            DDS_StatusMask mask) = 0;

    /*e \dref_DomainParticipant_delete_publisher
     */
    virtual DDS_ReturnCode_t delete_publisher(
            DDSPublisher* p) = 0;

    /*e \dref_DomainParticipant_create_subscriber
     */
    virtual DDSSubscriber* create_subscriber(
            const DDS_SubscriberQos& qos,
            DDSSubscriberListener* listener,
            DDS_StatusMask mask) = 0;

    /*e \dref_DomainParticipant_delete_subscriber
     */
    virtual DDS_ReturnCode_t delete_subscriber(
            DDSSubscriber* s) = 0;

    /*e \dref_DomainParticipant_create_topic
     */
    virtual DDSTopic* create_topic(
            const char* topic_name,
            const char* type_name,
            const DDS_TopicQos& qos,
            DDSTopicListener* listener,
            DDS_StatusMask mask) = 0;

     /*e \dref_DomainParticipant_delete_topic
     */
     virtual DDS_ReturnCode_t delete_topic(
            DDSTopic* topic) = 0;

#if INCLUDE_API_LOOKUP
     /*e \dref_DomainParticipant_find_topic
     */
     virtual DDSTopic* find_topic(
            const char* topic_name,
            const DDS_Duration_t& timeout) = 0;
#endif

     /*e \dref_DomainParticipant_get_qos
     */
     virtual DDS_ReturnCode_t get_qos(
             DDS_DomainParticipantQos& qos) = 0;

     /*e \dref_DomainParticipant_set_qos
     */
     virtual DDS_ReturnCode_t set_qos(
             const DDS_DomainParticipantQos& qos) = 0;

     /*e \dref_DomainParticipant_get_default_publisher_qos
      */
     virtual DDS_ReturnCode_t get_default_publisher_qos(
            DDS_PublisherQos& qos) = 0;

     /*e \dref_DomainParticipant_set_default_publisher_qos
      */
     virtual DDS_ReturnCode_t set_default_publisher_qos(
            const DDS_PublisherQos& qos) = 0;

     /*e \dref_DomainParticipant_get_default_subscriber_qos
      */
     virtual DDS_ReturnCode_t get_default_subscriber_qos(
            DDS_SubscriberQos& qos) = 0;

     /*e \dref_DomainParticipant_set_default_subscriber_qos
      */
     virtual DDS_ReturnCode_t set_default_subscriber_qos(
            const DDS_SubscriberQos& qos) = 0;

     /*e \dref_DomainParticipant_get_default_topic_qos
      */
     virtual DDS_ReturnCode_t get_default_topic_qos(
            DDS_TopicQos& qos) = 0;

     /*e \dref_DomainParticipant_set_default_topic_qos
      */
     virtual DDS_ReturnCode_t set_default_topic_qos(
            const DDS_TopicQos& qos) = 0;

     /*e \dref_DomainParticipant_lookup_topicdescription
      */
     virtual DDSTopicDescription* lookup_topicdescription(
            const char *topic_name) = 0;

#if DDS_ENABLE_APPGEN
     /*e \dref_DomainParticipant_lookup_publisher_by_name
      */
     virtual DDSPublisher* lookup_publisher_by_name(
            const char *publisher_name) = 0;

     /*e \dref_DomainParticipant_lookup_subscriber_by_name
      */
     virtual DDSSubscriber* lookup_subscriber_by_name(
            const char *subscriber_name) = 0;

     /*e \dref_DomainParticipant_lookup_datawriter_by_name
      */
     virtual DDSDataWriter* lookup_datawriter_by_name(
            const char *datawriter_full_name) = 0;

     /*e \dref_DomainParticipant_lookup_datareader_by_name
      */
     virtual DDSDataReader* lookup_datareader_by_name(
            const char *datareader_full_name) = 0;
#endif /* DDS_ENABLE_APPGEN */

     /*i \dref_DomainParticipant_get_domain_id
      */
     virtual DDS_DomainId_t get_domain_id() = 0;

#if DDS_LIVELINESS_CHANNEL_ENABLED
     /*e \dref_DomainParticipant_assert_liveliness
     */
     virtual DDS_ReturnCode_t assert_liveliness() = 0;
#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */

     /*e \dref_DomainParticipant_delete_contained_entities
      */
     virtual DDS_ReturnCode_t delete_contained_entities() = 0;

     /*e \dref_DomainParticipant_register_type
      */
     virtual DDS_ReturnCode_t register_type(
            const char *type_name,
            DDS_TypePluginI *plugin) = 0;

     /*e \dref_DomainParticipant_unregister_type
      */
     virtual DDS_TypePluginI* unregister_type(
            const char *type_name) = 0;

    /*e \dref_DomainParticipant_set_listener
     */
    virtual DDS_ReturnCode_t set_listener(
            const DDSDomainParticipantListener *listener,
            DDS_StatusMask mask) = 0;

    /*e \dref_DomainParticipant_get_listener
     */
    virtual DDSDomainParticipantListener* get_listener() = 0;

#if INCLUDE_API_LOOKUP
    /*e \dref_DomainParticipant_get_discovered_participants
     */
    virtual DDS_ReturnCode_t get_discovered_participants(
        DDS_InstanceHandleSeq &participant_handles) = 0;

    /*e \dref_DomainParticipant_get_discovered_participant_data
     */
    virtual DDS_ReturnCode_t get_discovered_participant_data(
            DDS_ParticipantBuiltinTopicData &participant_data,
            const DDS_InstanceHandle_t &participant_handle) = 0;
#endif /* INCLUDE_API_LOOKUP */

    /*e \dref_DomainParticipant_get_current_time
     */
    virtual DDS_ReturnCode_t get_current_time(DDS_Time_t &current_time) = 0;

    /*e \dref_DomainParticipant_add_peer
     */
    virtual DDS_ReturnCode_t add_peer(const char* peer) = 0;

#if DDS_FLOW_CONTROLLER_ENABLED
    /*e \dref_DomainParticipant_get_default_flowcontroller_property
     */
    virtual DDS_ReturnCode_t
    get_default_flowcontroller_property(DDS_FlowControllerProperty_t &prop) = 0;

    /*e \dref_DomainParticipant_set_default_flowcontroller_property
     */
    virtual DDS_ReturnCode_t
    set_default_flowcontroller_property(
                            const DDS_FlowControllerProperty_t &prop) = 0;

    /*e \dref_DomainParticipant_create_flowcontroller
     */
    virtual DDSFlowController*
    create_flowcontroller(const char *name,
                           const DDS_FlowControllerProperty_t &prop) = 0;

    /*e \dref_DomainParticipant_delete_flowcontroller
     */
    virtual DDS_ReturnCode_t
    delete_flowcontroller( DDSFlowController *fc) = 0;

    /*e \dref_DomainParticipant_lookup_flowcontroller
     */
    virtual DDSFlowController*
    lookup_flowcontroller(const char *name) = 0;

#endif
  // --- <<lifecycle>>: ------------------------------------------------
  protected: 

    DDSDomainParticipant(DDS_DomainParticipant* c_participant);

    ~DDSDomainParticipant();
};



/* ================================================================= */
/*                       Factory                                     */
/* ================================================================= */

class DDSDomainParticipantFactory_impl;

/* ----------------------------------------------------------------- */
/*e \dref_DomainParticipantFactory
 */
class DDSCPPDllExport DDSDomainParticipantFactory
{

  // --- <<interface>> DDSDomainParticipantFactory ---------------------
  public: 
    /*e \dref_DomainParticipantFactory_get_instance
     */
    static DDSDomainParticipantFactory* get_instance();

    /*e \dref_DomainParticipantFactory_finalize_instance
     */
    static DDS_ReturnCode_t finalize_instance();

    /*e \dref_DomainParticipantFactory_create_participant
     */
    virtual DDSDomainParticipant* create_participant(
            DDS_DomainId_t domainId,
            const DDS_DomainParticipantQos& qos,
            DDSDomainParticipantListener* listener,
            DDS_StatusMask mask) = 0;

    /*e \dref_DomainParticipantFactory_delete_participant
     */
    virtual DDS_ReturnCode_t delete_participant(
            DDSDomainParticipant* a_participant) = 0;

#if DDS_ENABLE_APPGEN
    /*e \dref_DomainParticipantFactory_create_participant_from_config
     */
    virtual DDSDomainParticipant* create_participant_from_config(
            const char* configuration_name) = 0;
#endif /* DDS_ENABLE_APPGEN */

    /*e \dref_DomainParticipantFactory_lookup_participant
     */
    virtual DDSDomainParticipant* lookup_participant(
            DDS_DomainId_t domainId) = 0;

    /*e \dref_DomainParticipantFactory_lookup_participant_by_name
     */
#if DDS_ENABLE_APPGEN
    virtual DDSDomainParticipant* lookup_participant_by_name(
            const char* participant_name) = 0;
#endif /* DDS_ENABLE_APPGEN */

    /*e \dref_DomainParticipantFactory_set_default_participant_qos
     */
    virtual DDS_ReturnCode_t set_default_participant_qos(
            const DDS_DomainParticipantQos& qos) = 0;

    /*e \dref_DomainParticipantFactory_get_default_participant_qos
     */
    virtual DDS_ReturnCode_t get_default_participant_qos(
            DDS_DomainParticipantQos& qos) = 0;

    /*e \dref_DomainParticipantFactory_get_qos
     */
    virtual DDS_ReturnCode_t get_qos(
            DDS_DomainParticipantFactoryQos& qos) = 0;

    /*e \dref_DomainParticipantFactory_set_qos
     */
    virtual DDS_ReturnCode_t set_qos(
            const DDS_DomainParticipantFactoryQos& qos) = 0;

    /*e \dref_DomainParticipantFactory_get_registry
     */
    virtual RTRegistry* get_registry() = 0;
 
  // --- <<eXtension>> methods: ---------------------------------------


  // --- <<lifecycle>>: ------------------------------------------------
  protected:

    virtual ~DDSDomainParticipantFactory() { }

  // --- The <<singleton>> instance: ----------------------------------
  private:
    /*i
      @brief \st_singleton The singleton instance.
     */
    static DDSDomainParticipantFactory* _instance;
    /*i
    @brief \st_singleton The singleton instance.
    */
    static volatile RTI_INT32 _instance_initialized;
   
};

/* ----------------------------------------------------------------- */

/*e \dref_TheParticipantFactory
 */
#define DDSTheParticipantFactory DDSDomainParticipantFactory::get_instance()

#endif /* dds_cpp_domain_hxx */

/* ----------------------------------------------------------------- */

