/*
 * FILE: TopicDescription.hxx - TopicDescription header
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
 * 19jul2013,as  Major C++ update
 * 28jun2013,eh  Written
 */

#ifndef TopicDescription_hxx
#define TopicDescription_hxx


#ifndef dds_cpp_topic_hxx
  #include "dds_cpp/dds_cpp_topic.hxx"
#endif
#ifndef dds_cpp_domain_hxx
  #include "dds_cpp/dds_cpp_domain.hxx"
#endif

#ifndef dds_c_topic_h
  #include "dds_c/dds_c_topic.h"
#endif

typedef enum DDSTopicType
{
    DDS_TOPIC_TYPE_TOPIC = 0x01
} DDSTopicType_t;


class DDSSubscriber_impl;

class DDSTopicDescription_impl : public DDSTopicDescription
{
  friend class DDSTopic;
  friend class DDSSubscriber_impl;

  // --- <<interface>> DDSTopicDescription: ----------------------------
  public:

      const char* get_type_name();

      const char* get_name();

      DDSDomainParticipant* get_participant();

  // --- <<lifecycle>>: ------------------------------------------------
  public:
    virtual ~DDSTopicDescription_impl() ;
    DDSTopicDescription_impl(
            DDS_TopicDescription* c_topic_desc,
            DDSTopicType_t topic_type,
            void *base_topic);

  private:

    DDS_TopicDescription* _c_topic_desc;
    DDSTopic *_base_topic;
};

#endif /* TopicDescription_hxx */
