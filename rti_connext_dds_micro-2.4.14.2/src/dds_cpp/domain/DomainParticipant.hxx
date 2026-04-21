/*
 * FILE: DomainParticipant.hxx - DomainParticipant header
 *
 * (c) Copyright 2008-2015 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 16may2014,as  MICRO-795 Complete support for listener API in C++
 * 19jul2013,as  Major C++ update
 * 28jun2013,eh  Written
 */

#ifndef DomainParticipant_hxx
#define DomainParticipant_hxx

#ifndef dds_cpp_dll_hxx
#include "dds_cpp/dds_cpp_dll.hxx"
#endif
#ifndef dds_cpp_domain_hxx
#include "dds_cpp/dds_cpp_domain.hxx"
#endif
#ifndef dds_cpp_publication_hxx
#include "dds_cpp/dds_cpp_publication.hxx"
#endif
#ifndef dds_cpp_subscription_hxx
#include "dds_cpp/dds_cpp_subscription.hxx"
#endif
#ifndef dds_cpp_topic_hxx
#include "dds_cpp/dds_cpp_topic.hxx"
#endif
#ifndef dds_c_domain_h
#include "dds_c/dds_c_domain.h"
#endif

#include "Publisher.hxx"
#include "Subscriber.hxx"
#include "DataWriter.hxx"
#include "DataReader.hxx"
#include "Topic.hxx"
#include "TopicDescription.hxx"

class DDSDomainParticipantFactory_impl;

/* ----------------------------------------------------------------- */
/*i @ingroup DDSParticipantModule
 * @brief \st_impl DDSDomainParticipant_impl
 */
class DDSDomainParticipant_impl : public DDSDomainParticipant
{
  friend class DDSDomainParticipantFactory_impl;

  public:
    DDSEntity* as_entity();

    // --- <<interface>> DDSDomainParticipant: --------------------------
    DDSPublisher* create_publisher(
        const DDS_PublisherQos& qos,
        DDSPublisherListener* listener,
        DDS_StatusMask mask);

    DDS_ReturnCode_t delete_publisher(DDSPublisher* p);

    DDSSubscriber* create_subscriber(
        const DDS_SubscriberQos& qos,
        DDSSubscriberListener* listener,
        DDS_StatusMask mask);

    DDS_ReturnCode_t delete_subscriber(DDSSubscriber* s);

    DDSTopic* create_topic(const char* topic_name,
        const char* type_name,
        const DDS_TopicQos& qos,
        DDSTopicListener* listener,
        DDS_StatusMask mask);

    DDS_ReturnCode_t delete_topic(DDSTopic* topic);

#if INCLUDE_API_LOOKUP
    DDSTopic* find_topic(
        const char* topic_name,
        const DDS_Duration_t& timeout);
#endif

    DDS_ReturnCode_t get_qos(
        DDS_DomainParticipantQos& qos);

    DDS_ReturnCode_t set_qos(
        const DDS_DomainParticipantQos& qos);

    DDS_ReturnCode_t get_default_publisher_qos(
        DDS_PublisherQos& qos);

    DDS_ReturnCode_t set_default_publisher_qos(
        const DDS_PublisherQos& qos);

    DDS_ReturnCode_t get_default_subscriber_qos(
        DDS_SubscriberQos& qos);

    DDS_ReturnCode_t set_default_subscriber_qos(
        const DDS_SubscriberQos& qos);

    DDS_ReturnCode_t get_default_topic_qos(
        DDS_TopicQos& qos);

    DDS_ReturnCode_t set_default_topic_qos(
        const DDS_TopicQos& qos);

    DDSTopicDescription* lookup_topicdescription(
        const char *topic_name);

    DDS_DomainId_t get_domain_id();

    DDS_ReturnCode_t delete_contained_entities();

    DDS_ReturnCode_t register_type(
        const char *type_name,
        NDDS_Type_Plugin *plugin);

    NDDS_Type_Plugin* unregister_type(
        const char *type_name);

    DDS_ReturnCode_t set_listener(
            const DDSDomainParticipantListener *listener,
            DDS_StatusMask mask);

    DDSDomainParticipantListener* get_listener();

    DDS_ReturnCode_t get_current_time(DDS_Time_t &current_time);
    
    DDS_ReturnCode_t add_peer(const char* peer);

  protected:

    /*i
     * Call-back for the deletion of a DDSPublisher from
     * DDS_DomainParticipant_delete_contained_entities_w_finalizer
     */
    static void finalize_publisher(DDS_Publisher *publisher);

    /*i
     * Call-back for the deletion of a DDSSubscriber from
     * DDS_DomainParticipant_delete_contained_entities_w_finalizer
     */
    static void finalize_subscriber(DDS_Subscriber *subscriber);

    /*i
     * Call-back for the deletion of a DDSTopic from
     * DDS_DomainParticipant_delete_contained_entities_w_finalizer
     */
    static void finalize_topic(DDS_Topic *topic);

    // --- <<lifecycle>>: ------------------------------------------------
    DDSDomainParticipant_impl(DDS_DomainParticipant *c_participant) :
            DDSDomainParticipant(c_participant) { }

    virtual ~DDSDomainParticipant_impl() { }
    
};

#define DDSDomainParticipantListener_INITIALIZE_C_LISTENER(\
            listener_, c_listener_)\
{\
        (c_listener_)->as_subscriberlistener.as_datareaderlistener.as_listener.listener_data = (void*) (listener_);\
        (c_listener_)->as_publisherlistener.as_datawriterlistener.as_listener.listener_data = (void*) (listener_);\
        (c_listener_)->as_topiclistener.as_listener.listener_data = (void*) (listener_);\
        (c_listener_)->as_subscriberlistener.on_data_on_readers =\
                DDSDomainParticipantListener_forward_on_data_on_readers;\
        (c_listener_)->as_subscriberlistener.as_datareaderlistener.on_data_available =\
                DDSDomainParticipantListener_forward_on_data_available;\
        (c_listener_)->as_subscriberlistener.as_datareaderlistener.on_requested_deadline_missed =\
                DDSDomainParticipantListener_forward_on_requested_deadline_missed;\
        (c_listener_)->as_subscriberlistener.as_datareaderlistener.on_liveliness_changed =\
                DDSDomainParticipantListener_forward_on_liveliness_changed;\
        (c_listener_)->as_subscriberlistener.as_datareaderlistener.on_requested_incompatible_qos =\
                DDSDomainParticipantListener_forward_on_requested_incompatible_qos;\
        (c_listener_)->as_subscriberlistener.as_datareaderlistener.on_sample_rejected =\
                DDSDomainParticipantListener_forward_on_sample_rejected;\
        (c_listener_)->as_subscriberlistener.as_datareaderlistener.on_subscription_matched =\
                DDSDomainParticipantListener_forward_on_subscription_matched;\
        (c_listener_)->as_subscriberlistener.as_datareaderlistener.on_sample_lost =\
                DDSDomainParticipantListener_forward_on_sample_lost;\
        (c_listener_)->as_subscriberlistener.as_datareaderlistener.on_instance_replaced =\
                DDSDomainParticipantListener_forward_on_instance_replaced;\
        (c_listener_)->as_subscriberlistener.as_datareaderlistener.on_before_sample_deserialize =\
                DDSDomainParticipantListener_forward_on_before_sample_deserialize;\
        (c_listener_)->as_subscriberlistener.as_datareaderlistener.on_before_sample_commit =\
                DDSDomainParticipantListener_forward_on_before_sample_commit;\
        (c_listener_)->as_publisherlistener.as_datawriterlistener.on_liveliness_lost =\
                DDSDomainParticipantListener_forward_on_liveliness_lost;\
        (c_listener_)->as_publisherlistener.as_datawriterlistener.on_offered_deadline_missed =\
                DDSDomainParticipantListener_forward_on_offered_deadline_missed;\
        (c_listener_)->as_publisherlistener.as_datawriterlistener.on_offered_incompatible_qos =\
                DDSDomainParticipantListener_forward_on_offered_incompatible_qos;\
        (c_listener_)->as_publisherlistener.as_datawriterlistener.on_publication_matched =\
                DDSDomainParticipantListener_forward_on_publication_matched;\
        (c_listener_)->as_publisherlistener.as_datawriterlistener.on_reliable_reader_activity_changed =\
                DDSDomainParticipantListener_forward_on_reliable_reader_activity_changed;\
        (c_listener_)->as_publisherlistener.as_datawriterlistener.on_reliable_sample_unacknowledged =\
                DDSDomainParticipantListener_forward_on_reliable_sample_unacknowledged;\
        (c_listener_)->as_topiclistener.on_inconsistent_topic = \
                DDSDomainParticipantListener_forward_on_inconsistent_topic;\
}

extern "C" {
void
DDSDomainParticipantListener_forward_on_data_available(
   void* listener_data,
   DDS_DataReader* c_datareader);

void
DDSDomainParticipantListener_forward_on_requested_deadline_missed(
        void *listener_data,
        DDS_DataReader* c_reader,
        const DDS_RequestedDeadlineMissedStatus *status);


void
DDSDomainParticipantListener_forward_on_liveliness_changed(
        void *listener_data,
        DDS_DataReader *c_reader,
        const DDS_LivelinessChangedStatus *status);

void
DDSDomainParticipantListener_forward_on_requested_incompatible_qos(
        void *listener_data,
        DDS_DataReader *c_reader,
        const DDS_RequestedIncompatibleQosStatus *status);

void
DDSDomainParticipantListener_forward_on_sample_rejected(
        void *listener_data,
        DDS_DataReader *c_reader,
        const DDS_SampleRejectedStatus *status);

void
DDSDomainParticipantListener_forward_on_subscription_matched(
        void *listener_data,
        DDS_DataReader *c_reader,
        const DDS_SubscriptionMatchedStatus *status);

void
DDSDomainParticipantListener_forward_on_sample_lost(
        void *listener_data,
        DDS_DataReader *c_reader,
        const DDS_SampleLostStatus *status);

void
DDSDomainParticipantListener_forward_on_instance_replaced(
        void *listener_data,
        DDS_DataReader *c_reader,
        const DDS_DataReaderInstanceReplacedStatus *status);

DDS_Boolean
DDSDomainParticipantListener_forward_on_before_sample_deserialize(
        void *listener_data,
        DDS_DataReader *c_reader,
        NDDS_Type_Plugin *plugin,
        CDR_Stream_t *stream,
        DDS_Boolean *dropped);

DDS_Boolean
DDSDomainParticipantListener_forward_on_before_sample_commit(
        void *listener_data,
        DDS_DataReader *c_reader,
        const void *const sample,
        const DDS_SampleInfo *const sample_info,
        DDS_Boolean *dropped);

void
DDSDomainParticipantListener_forward_on_offered_deadline_missed(
        void *listener_data,
        DDS_DataWriter* c_writer,
        const DDS_OfferedDeadlineMissedStatus* status);

void DDSDomainParticipantListener_forward_on_liveliness_lost(
        void *listener_data,
        DDS_DataWriter *c_writer,
        const DDS_LivelinessLostStatus *status);

void DDSDomainParticipantListener_forward_on_offered_incompatible_qos(
        void *listener_data,
        DDS_DataWriter* c_writer,
        const DDS_OfferedIncompatibleQosStatus *status);

void DDSDomainParticipantListener_forward_on_publication_matched(
        void *listener_data,
        DDS_DataWriter *c_writer,
        const DDS_PublicationMatchedStatus *status);

void DDSDomainParticipantListener_forward_on_reliable_reader_activity_changed(
        void *listener_data,
        DDS_DataWriter * c_writer,
        const DDS_ReliableReaderActivityChangedStatus * status);

void DDSDomainParticipantListener_forward_on_reliable_sample_unacknowledged(
        void *listener_data,
        DDS_DataWriter * c_writer,
        const DDS_ReliableSampleUnacknowledgedStatus * status);

void DDSDomainParticipantListener_forward_on_inconsistent_topic(
                            void *listener_data,
                            DDS_Topic *c_topic,
                            const DDS_InconsistentTopicStatus *status);

void
DDSDomainParticipantListener_forward_on_data_on_readers(
   void* listener_data,
   DDS_Subscriber* c_subscriber);
}

#endif /* DomainParticipant_hxx */

