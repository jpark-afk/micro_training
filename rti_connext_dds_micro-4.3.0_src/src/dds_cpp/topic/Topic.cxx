/* 

 (c) Copyright, Real-Time Innovations, 2013-2015.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.

 modification history
---------------------
16may2014,as  MICRO-795 Complete support for listener API in C++
07may2014,as  MICRO-784 Expose get_X_status API in C++
19jul2013,as  Major C++ update
21jan2013,eh  Created
===================================================================== */

#ifndef dds_cpp_topic_hxx
  #include "dds_cpp/dds_cpp_topic.hxx"
#endif
#ifndef dds_cpp_domain_hxx
  #include "dds_cpp/dds_cpp_domain.hxx"
#endif

#ifndef dds_c_config_h
  #include "dds_c/dds_c_config.h"
#endif

#ifndef dds_c_topic_h
  #include "dds_c/dds_c_topic.h"
#endif

#include "Entity.hxx"
#include "TopicDescription.hxx"
#include "Topic.hxx"
/*** SOURCE_BEGIN ***/

// ---------------------------------------------------------------------
// Abstract base class Methods
// ---------------------------------------------------------------------

DDSTopic* DDSTopic::narrow(DDSTopicDescription* topic_description) 
{
    DDSTopicDescription_impl *topic_desc_impl = NULL;
    if (topic_description == NULL)
    {
        return NULL;
    }

    topic_desc_impl = topic_description->get_impl();

    return topic_desc_impl->_base_topic;
}

DDS_ReturnCode_t
DDSTopic_impl::set_qos(const DDS_TopicQos& qos)
{
#ifndef RTI_CERT
    return DDS_Topic_set_qos((DDS_Topic*)this->_c_entity, &qos);
#else
    UNUSED_ARG(qos);
    return DDS_RETCODE_UNSUPPORTED;
#endif /* !RTI_CERT */
}

DDS_ReturnCode_t
DDSTopic_impl::get_qos(DDS_TopicQos& qos)
{
#ifndef RTI_CERT
    return DDS_Topic_get_qos((DDS_Topic*)this->_c_entity, &qos);
#else
    UNUSED_ARG(qos);
    return DDS_RETCODE_UNSUPPORTED;
#endif /* !RTI_CERT */
}


DDS_ReturnCode_t
DDSTopic_impl::set_listener(
                const DDSTopicListener *listener,
                DDS_StatusMask mask)
{
#ifndef RTI_CERT 
    struct DDS_TopicListener c_listener = DDS_TopicListener_INITIALIZER,
                *installed_listener = NULL;

    if (listener != NULL)
    {
        DDSTopicListener_INITIALIZE_C_LISTENER(listener,&c_listener);
        installed_listener = &c_listener;
    }
    return DDS_Topic_set_listener(
                (DDS_Topic*) this->_c_entity,
                installed_listener, mask);
#else
    UNUSED_ARG(listener);
    UNUSED_ARG(mask);
    return DDS_RETCODE_UNSUPPORTED;
#endif
}



DDSTopicListener*
DDSTopic_impl::get_listener()
{
#ifndef RTI_CERT 
    struct DDS_TopicListener c_listener =  DDS_TopicListener_INITIALIZER;
    c_listener = DDS_Topic_get_listener((DDS_Topic*) this->_c_entity);
    return (DDSTopicListener*) c_listener.as_listener.listener_data;
#else
    return NULL;
#endif 
}



// --- Constructors & destructors: -------------------------------------


DDSTopicDescription*
DDSTopic_impl::as_topicdescription()
{
    return this;
}

DDSEntity*
DDSTopic_impl::as_entity()
{
    return this;
}


/* TopicDescription methods */

const char*
DDSTopic_impl::get_type_name()
{
    return this->_topic_desc_impl->get_type_name();
}

const char*
DDSTopic_impl::get_name()
{
    return this->_topic_desc_impl->get_name();
}

DDSDomainParticipant*
DDSTopic_impl::get_participant()
{
    return this->_topic_desc_impl->get_participant();
}

DDS_ReturnCode_t
DDSTopic_impl::get_inconsistent_topic_status(
                DDS_InconsistentTopicStatus& status)
{
    return DDS_Topic_get_inconsistent_topic_status(
                (DDS_Topic*)this->_c_entity, &status);
}

DDSTopic::DDSTopic(DDS_Topic* c_topic) :
        DDSEntity(DDS_Topic_as_entity(c_topic))
{

}

DDSTopic::~DDSTopic()
{

}

DDSTopic_impl::DDSTopic_impl(DDS_Topic* c_topic) :
        DDSTopic(c_topic)
{
    this->_topic_desc_impl = new DDSTopicDescription_impl(
                                    DDS_Topic_as_topicdescription(c_topic),
                                    DDS_TOPIC_TYPE_TOPIC, this);
}

DDSTopic_impl::~DDSTopic_impl()
{
    if (this->_topic_desc_impl != NULL)
    {
        delete this->_topic_desc_impl;
        this->_topic_desc_impl = NULL;
    }
}
