/* 

 (c) Copyright, Real-Time Innovations, 2013-2015.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.

 modification history
---------------------
16may2014,as  MICRO-795 Complete support for listener API in C++;
              MICRO-794 Remove C++ TODO and commented out code;
              Moved forwarding functions to SubscriberListener.cxx
19jul2013,as  Major C++ update
21jan2013,eh  Created
===================================================================== */

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
#include "Subscriber.hxx"

/*** SOURCE_BEGIN ***/

// ---------------------------------------------------------------------
// C Methods
// ---------------------------------------------------------------------
#if DDS_ENABLE_APPGEN
DDS_DataReader*
DDSSubscriber_create_datareader(
       DDS_Subscriber *self,
       DDS_TopicDescription *topic_desc,
       const struct DDS_DataReaderQos *qos,
       const struct DDS_DataReaderListener *listener,
       DDS_StatusMask mask)
{
    DDS_DataReader *ret_value = NULL;
    DDSDataReader *reader = NULL;
    DDSSubscriber *subscriber;
    DDSTopic *topic;

    /* Note that this function can not have a non NULL listener */
    if ((self != NULL) && (topic_desc != NULL) && (qos != NULL) &&
        (listener == NULL) && (mask == DDS_STATUS_MASK_NONE))
    {
        subscriber = (DDSSubscriber*)
            DDS_Entity_get_wrapper(DDS_Subscriber_as_entity(self));

        if (subscriber != NULL)
        {
            topic = (DDSTopic*)DDS_Entity_get_wrapper(
                                   DDS_Topic_as_entity(
                                       DDS_Topic_narrow(topic_desc)));

            reader = subscriber->create_datareader(topic, *qos, NULL, 
                                                   DDS_STATUS_MASK_NONE);

            if (reader != NULL)
            {
                ret_value = (DDS_DataReader*)reader->get_c_entity();
            }
        }
    }

    return ret_value;
}
#endif /* DDS_ENABLE_APPGEN */

// ---------------------------------------------------------------------
// Public Methods
// ---------------------------------------------------------------------

DDSSubscriber::DDSSubscriber(DDS_Subscriber *c_subscriber) :
    DDSEntity(DDS_Subscriber_as_entity(c_subscriber))
{

}

DDSSubscriber::~DDSSubscriber()
{

}

DDSDataReader* 
DDSSubscriber_impl::create_datareader(DDSTopicDescription* topic_desc,
                                      const DDS_DataReaderQos& qos,
                                      DDSDataReaderListener* listener,
                                      DDS_StatusMask mask)
{
    DDSDataReader *reader = NULL;
    DDS_DataReader *c_reader = NULL;
    DDS_DataReaderListener c_listener = DDS_DataReaderListener_INITIALIZER;
    struct DDS_DataReaderQos modifiedQos;
    DDSTopicDescription_impl *topicdesc_impl = NULL;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    if (topic_desc == NULL)
    {
        return NULL;
    }

    topicdesc_impl = topic_desc->_topic_desc_impl;

    /* setup listener wrapper if necessary */
    if (listener != NULL) 
    {
        DDSDataReaderListener_INITIALIZE_C_LISTENER(listener, &c_listener);
    }

    /* If qos is the default qos, we will need to get the default value first
       before modifying it */
    if (&qos == &DDS_DATAREADER_QOS_DEFAULT) 
    {
#ifndef RTI_CERT
        retcode = DDS_Subscriber_get_default_datareader_qos(
                    (DDS_Subscriber*)this->_c_entity, &modifiedQos);
#endif /* !RTI_CERT */
    } 
    else
    {
        retcode = modifiedQos.copy(qos);
    }
    if (retcode != DDS_RETCODE_OK)
    {
        return NULL;
    }

    c_reader = DDS_Subscriber_create_datareader(
                    (DDS_Subscriber*)this->_c_entity,
                    topicdesc_impl->_c_topic_desc,
                    &modifiedQos,
                    (listener != NULL)?&c_listener:NULL,
                    mask);
    if (c_reader == NULL) 
    {
        return NULL;
    }

    reader = (DDSDataReader*)
            DDS_Entity_get_wrapper(DDS_DataReader_as_entity(c_reader));

    if (reader == NULL)
    {
#ifndef RTI_CERT
        DDS_Subscriber_delete_datareader(
                (DDS_Subscriber*)this->_c_entity,c_reader);
#endif
    }

    return reader;
}



DDS_ReturnCode_t 
DDSSubscriber_impl::delete_datareader(DDSDataReader* datareader)
{
#ifndef RTI_CERT
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDSDataReader_impl *datareader_impl = NULL;

    if (datareader == NULL)
    {
        return DDS_RETCODE_BAD_PARAMETER;
    }

    datareader_impl = static_cast<DDSDataReader_impl*>(datareader);
    retcode = DDS_Subscriber_delete_datareader(
                    (DDS_Subscriber*)this->_c_entity,
                    (DDS_DataReader*)datareader_impl->_c_entity);

    return retcode;
#else
    UNUSED_ARG(datareader);
    return DDS_RETCODE_OK;
#endif
}

DDS_ReturnCode_t
DDSSubscriber_impl::get_default_datareader_qos(
        DDS_DataReaderQos& qos)
{
#ifndef RTI_CERT
    return DDS_Subscriber_get_default_datareader_qos(
                    (DDS_Subscriber*)this->_c_entity, &qos);
#else
    UNUSED_ARG(qos);
    return DDS_RETCODE_UNSUPPORTED;
#endif /* !RTI_CERT */
}

DDS_ReturnCode_t
DDSSubscriber_impl::set_default_datareader_qos(
        const DDS_DataReaderQos& qos)
{
#ifndef RTI_CERT
    return DDS_Subscriber_set_default_datareader_qos(
                    (DDS_Subscriber*)this->_c_entity, &qos);
#else
    UNUSED_ARG(qos);
    return DDS_RETCODE_UNSUPPORTED;
#endif /* !RTI_CERT */
}

DDS_ReturnCode_t
DDSSubscriber_impl::delete_contained_entities()
{
#ifndef RTI_CERT
    return DDS_Subscriber_delete_contained_entities(
                            (DDS_Subscriber*)this->_c_entity);
#else
    return DDS_RETCODE_OK;
#endif
}

DDSDataReader*
DDSSubscriber_impl::lookup_datareader(
        const char *topic_name)
{
    DDS_DataReader *c_reader = NULL;
    DDSDataReader *result = NULL;
    c_reader = DDS_Subscriber_lookup_datareader(
                    (DDS_Subscriber*)this->_c_entity, topic_name);
    if (c_reader != NULL) {
        result = (DDSDataReader*)
                DDS_Entity_get_wrapper(
                    DDS_DataReader_as_entity(c_reader));
    }
    return result;
}

#if DDS_ENABLE_APPGEN
DDSDataReader*
DDSSubscriber_impl::lookup_datareader_by_name(
        const char * datareader_name)
{
    DDS_DataReader *c_reader = NULL;
    DDSDataReader *result = NULL;

    c_reader = DDS_Subscriber_lookup_datareader_by_name(
                    (DDS_Subscriber*)this->_c_entity, datareader_name);
    if (c_reader != NULL)
    {
        result = (DDSDataReader*) DDS_Entity_get_wrapper(
                    DDS_DataReader_as_entity(c_reader));
    }

    return result;
}
#endif /* DDS_ENABLE_APPGEN */

DDSDomainParticipant*
DDSSubscriber_impl::get_participant()
{
    DDS_DomainParticipant *c_participant = NULL;
    DDSDomainParticipant *result = NULL;
    c_participant = DDS_Subscriber_get_participant(
                        (DDS_Subscriber*)this->_c_entity);
    if (c_participant != NULL) {
        result = (DDSDomainParticipant*)
                DDS_Entity_get_wrapper(
                        DDS_DomainParticipant_as_entity(c_participant));
    }
    return result;
}

DDS_ReturnCode_t
DDSSubscriber_impl::set_qos(
        const DDS_SubscriberQos& qos)
{
#ifndef RTI_CERT
    return DDS_Subscriber_set_qos((DDS_Subscriber*)this->_c_entity, &qos);
#else
    UNUSED_ARG(qos);
    return DDS_RETCODE_UNSUPPORTED;
#endif /* !RTI_CERT */
}

DDS_ReturnCode_t
DDSSubscriber_impl::get_qos(
        DDS_SubscriberQos& qos)
{
#ifndef RTI_CERT
    return DDS_Subscriber_get_qos((DDS_Subscriber*)this->_c_entity, &qos);
#else
    UNUSED_ARG(qos);
    return DDS_RETCODE_UNSUPPORTED;
#endif /* !RTI_CERT */
}

DDSEntity*
DDSSubscriber_impl::as_entity()
{
    return this;
}

DDS_ReturnCode_t
DDSSubscriber_impl::set_listener(
        const DDSSubscriberListener *listener,
        DDS_StatusMask mask)
{
#ifndef RTI_CERT
    DDS_SubscriberListener c_listener = DDS_SubscriberListener_INITIALIZER,
            *installed_listener = NULL;

    if (listener != NULL)
    {
        DDSSubscriberListener_INITIALIZE_C_LISTENER(listener,&c_listener);
        installed_listener = &c_listener;
    }

    return DDS_Subscriber_set_listener(
                (DDS_Subscriber*) this->_c_entity,
                installed_listener, mask);
#else
    UNUSED_ARG(listener);
    UNUSED_ARG(mask);
    return DDS_RETCODE_UNSUPPORTED;
#endif
}

DDSSubscriberListener*
DDSSubscriber_impl::get_listener()
{
#ifndef RTI_CERT
    struct DDS_SubscriberListener c_listener =  DDS_SubscriberListener_INITIALIZER;
    c_listener = DDS_Subscriber_get_listener((DDS_Subscriber*) this->_c_entity);
    return (DDSSubscriberListener*)
                c_listener.as_datareaderlistener.as_listener.listener_data;
#else
    return NULL;
#endif
}

