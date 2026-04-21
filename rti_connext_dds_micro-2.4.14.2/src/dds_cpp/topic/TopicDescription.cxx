/* 

 (c) Copyright, Real-Time Innovations, 2013-2016.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.

 modification history
---------------------
27jun2016,tk  MICRO-1544 Properly initialize the 2nd (internal) TopicDescription 
                         to point to point to the implementation object
19jul2013,as  Major C++  update
21jan2013,eh  Created
===================================================================== */

#ifndef dds_cpp_topic_hxx
  #include "dds_cpp/dds_cpp_topic.hxx"
#endif
#ifndef dds_cpp_domain_hxx
  #include "dds_cpp/dds_cpp_domain.hxx"
#endif

#ifndef dds_c_topic_h
  #include "dds_c/dds_c_topic.h"
#endif

#include "TopicDescription.hxx"
/*** SOURCE_BEGIN ***/

// ---------------------------------------------------------------------
// Public Methods
// ---------------------------------------------------------------------

DDSTopicDescription_impl*
DDSTopicDescription::get_impl()
{
    return _topic_desc_impl;
}

DDSTopicDescription_impl::DDSTopicDescription_impl(
   DDS_TopicDescription* c_topic_desc,
   DDSTopicType_t topic_type,
   void* base_topic)
{
    this->_c_topic_desc = c_topic_desc;
    this->_topic_desc_impl = this;

    switch (topic_type) {
        case DDS_TOPIC_TYPE_TOPIC:
        {
            this->_base_topic = (DDSTopic*) base_topic;
            break;
        }
        default:
        {
            /* Will never get here, since this is used
             * only internally. Otherwise, log exception
             */
            break;
        }
    }
}

DDSTopicDescription_impl::~DDSTopicDescription_impl() 
{
    this->_c_topic_desc = NULL;
    this->_base_topic = NULL;
}

const char*
DDSTopicDescription_impl::get_type_name()
{
    return DDS_TopicDescription_get_type_name(this->_c_topic_desc);
}

const char*
DDSTopicDescription_impl::get_name()
{
    return DDS_TopicDescription_get_name(this->_c_topic_desc);
}

DDSDomainParticipant*
DDSTopicDescription_impl::get_participant()
{
    DDS_DomainParticipant *c_participant = NULL;
    DDSDomainParticipant* participant = NULL;

    c_participant = DDS_TopicDescription_get_participant(this->_c_topic_desc);
    if (c_participant != NULL)
    {
        participant = (DDSDomainParticipant*) DDS_Entity_get_wrapper(
                            DDS_DomainParticipant_as_entity(c_participant));
    }

    return participant;
}
