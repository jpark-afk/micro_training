/*
 * FILE: Subscriber.hxx - Subscriber header
 *
 * (c) Copyright 2013-2015 Real-Time Innovations,
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
#ifndef Subscriber_hxx
#define Subscriber_hxx


#ifndef dds_cpp_topic_hxx
  #include "dds_cpp/dds_cpp_topic.hxx"
#endif
#ifndef dds_cpp_subscription_hxx
  #include "dds_cpp/dds_cpp_subscription.hxx"
#endif
#ifndef dds_c_subscription_h
  #include "dds_c/dds_c_subscription.h"
#endif

#include "TopicDescription.hxx"
#include "Topic.hxx"
#include "DataReader.hxx"


class DDSDomainParticipant_impl;

class DDSCPPDllExport DDSSubscriber_impl : public DDSSubscriber
{
  friend class DDSDomainParticipant_impl;

  public:
    // --- <<interface>> DDSSubscriber: -----------------------------------
    DDSEntity* as_entity();

    DDSDataReader* create_datareader(
            DDSTopicDescription* topic,
            const DDS_DataReaderQos& qos,
            DDSDataReaderListener* listener,
            DDS_StatusMask mask);

    DDS_ReturnCode_t get_default_datareader_qos(
            DDS_DataReaderQos& qos);

    DDS_ReturnCode_t set_default_datareader_qos(
            const DDS_DataReaderQos& qos);

    DDSDataReader* lookup_datareader(
            const char *topic_name);

#if DDS_ENABLE_APPGEN
    DDSDataReader* lookup_datareader_by_name(
            const char * datareader_name);
#endif /* DDS_ENABLE_APPGEN */

    DDSDomainParticipant* get_participant();

    DDS_ReturnCode_t set_qos(
            const DDS_SubscriberQos& qos);

    DDS_ReturnCode_t get_qos(
            DDS_SubscriberQos& qos);

    DDS_ReturnCode_t delete_datareader(
       DDSDataReader* a_datareader);

    DDS_ReturnCode_t delete_contained_entities();

    DDS_ReturnCode_t set_listener(
            const DDSSubscriberListener *listener,
            DDS_StatusMask mask);

    DDSSubscriberListener* get_listener();

  protected:

    // --- <<lifecycle>>: ------------------------------------------------
    DDSSubscriber_impl(DDS_Subscriber *c_subscriber):
        DDSSubscriber(c_subscriber) { }

    ~DDSSubscriber_impl() {}
};


extern "C" {
void DDSSubscriberListener_forward_on_data_on_readers(
       void* listener_data,
       DDS_Subscriber* subscriber);

#if DDS_ENABLE_APPGEN
DDS_DataReader*
DDSSubscriber_create_datareader(
       DDS_Subscriber *self,
       DDS_TopicDescription *topic_desc,
       const struct DDS_DataReaderQos *qos,
       const struct DDS_DataReaderListener *listener,
       DDS_StatusMask mask);
#endif /* DDS_ENABLE_APPGEN */
}

#define DDSSubscriberListener_INITIALIZE_C_LISTENER(listener_, c_listener_)\
{\
        (c_listener_)->on_data_on_readers =\
                DDSSubscriberListener_forward_on_data_on_readers;\
        DDSDataReaderListener_INITIALIZE_C_LISTENER(\
                (listener_), &((c_listener_)->as_datareaderlistener));\
}

#endif /* Subscriber_hxx */
