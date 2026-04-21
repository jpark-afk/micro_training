/*
 * FILE: Topic.hxx - Topic header
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
 * 07may2014,as  MICRO-784 Expose get_X_status API in C++
 * 19jul2013,as  Major C++ update
 * 28jun2013,eh  Written
 */

#ifndef Topic_hxx
#define Topic_hxx



#ifndef dds_cpp_topic_hxx
  #include "dds_cpp/dds_cpp_topic.hxx"
#endif
#ifndef dds_cpp_domain_hxx
  #include "dds_cpp/dds_cpp_domain.hxx"
#endif

#ifndef dds_c_topic_h
  #include "dds_c/dds_c_topic.h"
#endif

#include "Entity.hxx"
#include "TopicDescription.hxx"

class DDSDataReader_impl;
class DDSPublisher_impl;
class DDSDomainParticipant_impl;

class DDSTopic_impl : public DDSTopic
{
  friend class DDSDataReader_impl;
  friend class DDSPublisher_impl;
  friend class DDSDomainParticipant_impl;

  // --- <<interface>> DDSTopic: ---------------------------------------
  public:

      DDSTopicDescription* as_topicdescription();

      DDSEntity* as_entity();

      DDS_ReturnCode_t set_qos(const DDS_TopicQos& qos);

      DDS_ReturnCode_t get_qos(DDS_TopicQos& qos);

      DDS_ReturnCode_t set_listener(
                const DDSTopicListener *listener,
                DDS_StatusMask mask);

      DDSTopicListener* get_listener();

      /* TopicDescription methods */

      const char* get_type_name();

      const char* get_name();

      DDSDomainParticipant* get_participant();

      DDS_ReturnCode_t get_inconsistent_topic_status(
                DDS_InconsistentTopicStatus& status);

  // --- <<lifecycle>>: ------------------------------------------------
  public:

    virtual ~DDSTopic_impl();

    DDSTopic_impl(DDS_Topic *cTopic);
};

extern "C" {
    void DDSTopicListener_forward_on_inconsistent_topic(
                            void *listener_data,
                            DDS_Topic *topic,
                            const DDS_InconsistentTopicStatus *status);
}

#define DDSTopicListener_INITIALIZE_C_LISTENER(listener_, c_listener_)\
{\
    (c_listener_)->as_listener.listener_data = (void*) (listener);\
    (c_listener_)->on_inconsistent_topic = \
        DDSTopicListener_forward_on_inconsistent_topic;\
}

#endif /* Topic_hxx */
