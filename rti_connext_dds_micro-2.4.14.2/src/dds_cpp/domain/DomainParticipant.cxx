/* 

 (c) Copyright, Real-Time Innovations, 2013-2015.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.

 modification history
---------------------
12feb2016,as  MICRO-1524: Pass status as reference to DDSDomainParticipantListener
16may2014,as  MICRO-795 Complete support for listener API in C++
19jul2013,as  Major C++ update
11jan2013,eh  Created
===================================================================== */

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

#include "Entity.hxx"
#include "Publisher.hxx"
#include "Subscriber.hxx"
#include "DataWriter.hxx"
#include "DataReader.hxx"
#include "Topic.hxx"
#include "TopicDescription.hxx"

#include "DomainParticipant.hxx"

/*** SOURCE_BEGIN ***/

// ---------------------------------------------------------------------
// Public Methods
// ---------------------------------------------------------------------

DDSDomainParticipant::DDSDomainParticipant(
        DDS_DomainParticipant* c_participant) :
        DDSEntity(DDS_DomainParticipant_as_entity(c_participant))
{
}

DDSDomainParticipant::~DDSDomainParticipant()
{
}


DDSPublisher* 
DDSDomainParticipant_impl::create_publisher(const DDS_PublisherQos& qos,
                                            DDSPublisherListener* listener,
                                            DDS_StatusMask mask)
{
    DDSPublisher_impl *publisher_impl = NULL;
    DDS_Publisher *c_publisher = NULL;
    DDS_PublisherListener c_listener = DDS_PublisherListener_INITIALIZER;
    DDS_PublisherQos modifiedQos;

    /* Publisher listener currently unpopulated, no assignement needed */
    if (listener != NULL)
    {
        DDSPublisherListener_INITIALIZE_C_LISTENER(listener, &c_listener);
    }

    /* If qos is the default qos, we will need to get the default value first
       before modifying it */
    if (&qos == &DDS_PUBLISHER_QOS_DEFAULT) 
    {
#ifndef RTI_CERT
        DDS_DomainParticipant_get_default_publisher_qos(
            (DDS_DomainParticipant*)this->_c_entity,
            &modifiedQos);
#endif /* !RTI_CERT */
    } 
    else
    {
        modifiedQos.copy(qos);
    }

    c_publisher = DDS_DomainParticipant_create_publisher(
            (DDS_DomainParticipant*)this->_c_entity, &modifiedQos,
            (listener != NULL)?&c_listener:NULL, mask);
    if (c_publisher == NULL) 
    {
        return NULL;
    }

    publisher_impl = new DDSPublisher_impl(c_publisher);
    if (publisher_impl == NULL)
    {
#ifndef RTI_CERT
        DDS_DomainParticipant_delete_publisher(
                    (DDS_DomainParticipant*)this->_c_entity, c_publisher);
#endif
        return NULL;
    }

    DDS_Entity_set_wrapper(
            DDS_Publisher_as_entity(c_publisher), (void *)publisher_impl);

    return (DDSPublisher*) publisher_impl;
}

DDS_ReturnCode_t 
DDSDomainParticipant_impl::delete_publisher(DDSPublisher* publisher)
{
#ifndef RTI_CERT
    DDS_ReturnCode_t result = DDS_RETCODE_ERROR;
    DDSPublisher_impl *publisher_impl = (DDSPublisher_impl *)publisher; 

    result = DDS_DomainParticipant_delete_publisher(
            (DDS_DomainParticipant*)this->_c_entity,
            (DDS_Publisher*)publisher_impl->_c_entity);
    if (result == DDS_RETCODE_OK)
    {
        delete publisher_impl;
    }
    return result;
#else
    UNUSED_ARG(publisher);
    return DDS_RETCODE_UNSUPPORTED;
#endif
}


DDSSubscriber* 
DDSDomainParticipant_impl::create_subscriber(const DDS_SubscriberQos& qos,
                                            DDSSubscriberListener* listener,
                                            DDS_StatusMask mask)
{
    DDSSubscriber_impl *subscriber_impl = NULL;
    DDS_Subscriber *c_subscriber = NULL;
    DDS_SubscriberListener c_listener = DDS_SubscriberListener_INITIALIZER;
    DDS_SubscriberQos modifiedQos;

    if (listener)
    {
        DDSSubscriberListener_INITIALIZE_C_LISTENER(listener, &c_listener);
    }

    /* If qos is the default qos, we will need to get the default value first
       before modifying it */
    if (&qos == &DDS_SUBSCRIBER_QOS_DEFAULT) 
    {
#ifndef RTI_CERT
        DDS_DomainParticipant_get_default_subscriber_qos(
                (DDS_DomainParticipant*)this->_c_entity, &modifiedQos);
#endif /* !RTI_CERT */
    } 
    else
    {
        modifiedQos.copy(qos);
    }

    c_subscriber = DDS_DomainParticipant_create_subscriber(
            (DDS_DomainParticipant*)this->_c_entity, &modifiedQos,
            (listener != NULL)?&c_listener:NULL, mask);
    if (c_subscriber == NULL) 
    {
        return NULL;
    }

    subscriber_impl = new DDSSubscriber_impl(c_subscriber);
    if (subscriber_impl == NULL)
    {
#ifndef RTI_CERT
        DDS_DomainParticipant_delete_subscriber(
                    (DDS_DomainParticipant*)this->_c_entity, c_subscriber);
#endif
        return NULL;
    }

    DDS_Entity_set_wrapper(
            DDS_Subscriber_as_entity(c_subscriber),(void *)subscriber_impl);

    return (DDSSubscriber*) subscriber_impl;
}

DDS_ReturnCode_t 
DDSDomainParticipant_impl::delete_subscriber(DDSSubscriber* subscriber)
{
#ifndef RTI_CERT
    DDS_ReturnCode_t result = DDS_RETCODE_ERROR;
    DDSSubscriber_impl *subscriber_impl = (DDSSubscriber_impl*) subscriber;

    result = DDS_DomainParticipant_delete_subscriber(
            (DDS_DomainParticipant*)this->_c_entity,
            (DDS_Subscriber*)subscriber_impl->_c_entity);
    if (result == DDS_RETCODE_OK)
    {
        delete subscriber_impl;
    }

    return result;
#else
    UNUSED_ARG(subscriber);
    return DDS_RETCODE_UNSUPPORTED;
#endif
}

DDSTopic* 
DDSDomainParticipant_impl::create_topic(const char* topic_name, 
                                        const char* type_name,
                                        const DDS_TopicQos& qos,
                                        DDSTopicListener* listener,
                                        DDS_StatusMask mask)
{
    DDSTopic *result = NULL;
    DDSTopic_impl* topic_impl = NULL;
    DDS_TopicListener c_listener = DDS_TopicListener_INITIALIZER;

    if (listener != NULL)
    {
        DDSTopicListener_INITIALIZE_C_LISTENER(listener, &c_listener);
    }

    /* call DDS C method */
    DDS_Topic* c_topic = DDS_DomainParticipant_create_topic(
            (DDS_DomainParticipant*)this->_c_entity,
            topic_name, type_name, &qos,
            (listener != NULL)?&c_listener:NULL, mask);
    if (c_topic == NULL) 
    {
        return NULL;
    }

    /* setup C++ wrapper */
    topic_impl = new DDSTopic_impl(c_topic);
    if (topic_impl == NULL)
    {
#ifndef RTI_CERT
        DDS_DomainParticipant_delete_topic(
                (DDS_DomainParticipant*)this->_c_entity, c_topic);
#endif
        return NULL;
    }

    DDS_Entity_set_wrapper(
            DDS_Topic_as_entity(c_topic), (void *)topic_impl);

    result = static_cast<DDSTopic*>(topic_impl);

    return result;
}

DDS_ReturnCode_t 
DDSDomainParticipant_impl::delete_topic(DDSTopic* topic)
{
#ifndef RTI_CERT
    DDSTopic_impl *topic_impl = (DDSTopic_impl *)topic; 

    /* The C++ object cannot be deleted unless the C object was deleted,
     * but the C object may not have been deleted if there was still
     * references to it.
     */
    return DDS_DomainParticipant_delete_topic_w_finalizer(
                (DDS_DomainParticipant*)this->_c_entity,
                (DDS_Topic*)topic_impl->_c_entity,
                DDSDomainParticipant_impl::finalize_topic);
#else
    UNUSED_ARG(topic);
    return DDS_RETCODE_UNSUPPORTED;
#endif
}

#if INCLUDE_API_LOOKUP
DDSTopic*
DDSDomainParticipant_impl::find_topic(
        const char* topic_name,
        const DDS_Duration_t& timeout)
{
    DDSTopic *topic_cpp = NULL;
    DDS_Topic *topic_c = NULL;

    topic_c = DDS_DomainParticipant_find_topic(
                (DDS_DomainParticipant*)this->_c_entity,
                topic_name,
                &timeout);

    if (topic_c != NULL)
    {
        DDSTopic_impl *topic_cpp_i = (DDSTopic_impl*) DDS_Entity_get_wrapper(
                                                    DDS_Topic_as_entity(topic_c));
        topic_cpp = static_cast<DDSTopic*>(topic_cpp_i);
    }

    return topic_cpp;
}
#endif

DDS_ReturnCode_t
DDSDomainParticipant_impl::get_qos(DDS_DomainParticipantQos& qos)
{
#ifndef RTI_CERT
    return DDS_DomainParticipant_get_qos(
                    (DDS_DomainParticipant*)this->_c_entity, &qos);
#else
    UNUSED_ARG(qos);
    return DDS_RETCODE_UNSUPPORTED;
#endif /* !RTI_CERT */
}

DDS_ReturnCode_t
DDSDomainParticipant_impl::set_qos(const DDS_DomainParticipantQos& qos)
{
#ifndef RTI_CERT
    return DDS_DomainParticipant_set_qos(
                    (DDS_DomainParticipant*)this->_c_entity, &qos);
#else
    UNUSED_ARG(qos);
    return DDS_RETCODE_UNSUPPORTED;
#endif /* !RTI_CERT */
}

DDS_ReturnCode_t
DDSDomainParticipant_impl::get_default_publisher_qos(
        DDS_PublisherQos& qos)
{
#ifndef RTI_CERT
    return DDS_DomainParticipant_get_default_publisher_qos(
            (DDS_DomainParticipant*)this->_c_entity, &qos);
#else
    UNUSED_ARG(qos);
    return DDS_RETCODE_UNSUPPORTED;
#endif /* !RTI_CERT */
}

DDS_ReturnCode_t
DDSDomainParticipant_impl::set_default_publisher_qos(
        const DDS_PublisherQos& qos)
{
#ifndef RTI_CERT
    return DDS_DomainParticipant_set_default_publisher_qos(
                (DDS_DomainParticipant*)this->_c_entity, &qos);
#else
    UNUSED_ARG(qos);
    return DDS_RETCODE_UNSUPPORTED;
#endif /* !RTI_CERT */
}

DDS_ReturnCode_t
DDSDomainParticipant_impl::get_default_subscriber_qos(
        DDS_SubscriberQos& qos)
{
#ifndef RTI_CERT
    return DDS_DomainParticipant_get_default_subscriber_qos(
                (DDS_DomainParticipant*)this->_c_entity, &qos);
#else
    UNUSED_ARG(qos);
    return DDS_RETCODE_UNSUPPORTED;
#endif /* !RTI_CERT */
}

DDS_ReturnCode_t
DDSDomainParticipant_impl::set_default_subscriber_qos(
        const DDS_SubscriberQos& qos)
{
#ifndef RTI_CERT
    return DDS_DomainParticipant_set_default_subscriber_qos(
                (DDS_DomainParticipant*)this->_c_entity, &qos);
#else
    UNUSED_ARG(qos);
    return DDS_RETCODE_UNSUPPORTED;
#endif /* !RTI_CERT */
}

DDS_ReturnCode_t
DDSDomainParticipant_impl::get_default_topic_qos(
        DDS_TopicQos& qos)
{
#ifndef RTI_CERT
    return DDS_DomainParticipant_get_default_topic_qos(
                (DDS_DomainParticipant*)this->_c_entity, &qos);
#else
    UNUSED_ARG(qos);
    return DDS_RETCODE_UNSUPPORTED;
#endif /* !RTI_CERT */
}

/* virtual DDSEntity method */
DDS_ReturnCode_t
DDSDomainParticipant_impl::set_default_topic_qos(
        const DDS_TopicQos& qos)
{
#ifndef RTI_CERT
    return DDS_DomainParticipant_set_default_topic_qos(
                (DDS_DomainParticipant*)this->_c_entity, &qos);
#else
    UNUSED_ARG(qos);
    return DDS_RETCODE_UNSUPPORTED;
#endif /* !RTI_CERT */
}

DDSTopicDescription*
DDSDomainParticipant_impl::lookup_topicdescription(
        const char *topic_name)
{
    DDSTopicDescription *result = NULL;
    DDS_TopicDescription *c_topicdesc = NULL;
    DDS_Topic* c_topic = NULL;
    DDSTopic *topic = NULL;
    DDSTopic_impl *topic_impl = NULL;

    c_topicdesc = DDS_DomainParticipant_lookup_topicdescription(
                    (DDS_DomainParticipant*)this->_c_entity, topic_name);
    if (c_topicdesc != NULL)
    {
        c_topic = DDS_Topic_narrow(c_topicdesc);
        if (c_topic != NULL)
        {
            topic = (DDSTopic*) DDS_Entity_get_wrapper(
                                    DDS_Topic_as_entity(c_topic));
            if (topic != NULL)
            {
                topic_impl = static_cast<DDSTopic_impl*>(topic);
                result = topic_impl->_topic_desc_impl;
            }
        }
    }

    return result;
}

DDS_DomainId_t
DDSDomainParticipant_impl::get_domain_id()
{
    return DDS_DomainParticipant_get_domain_id(
                (DDS_DomainParticipant*)this->_c_entity);
}

void
DDSDomainParticipant_impl::finalize_publisher(DDS_Publisher *publisher)
{
#ifndef RTI_CERT
    DDSPublisher_impl *cpp_publisher = (DDSPublisher_impl*)
            DDS_Entity_get_wrapper(DDS_Publisher_as_entity(publisher));
    delete cpp_publisher;
#else
    UNUSED_ARG(publisher);
#endif
}

void
DDSDomainParticipant_impl::finalize_subscriber(DDS_Subscriber *subscriber)
{
#ifndef RTI_CERT
    DDSSubscriber_impl *cpp_subscriber = (DDSSubscriber_impl*)
            DDS_Entity_get_wrapper(DDS_Subscriber_as_entity(subscriber));
    delete cpp_subscriber;
#else
    UNUSED_ARG(subscriber);
#endif
}

void
DDSDomainParticipant_impl::finalize_topic(DDS_Topic *topic)
{
#ifndef RTI_CERT
    DDSTopic_impl *cpp_topic = (DDSTopic_impl*)
            DDS_Entity_get_wrapper(DDS_Topic_as_entity(topic));
    delete cpp_topic;
#else
    UNUSED_ARG(topic);
#endif
}

DDS_ReturnCode_t
DDSDomainParticipant_impl::delete_contained_entities()
{
#ifndef RTI_CERT
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_DomainParticipant_EntityFinalizer finalizer =
                DDS_DomainParticipant_EntityFinalizer_INITIALIZER;

    finalizer.finalize_publisher = DDSDomainParticipant_impl::finalize_publisher;
    finalizer.finalize_subscriber = DDSDomainParticipant_impl::finalize_subscriber;
    finalizer.finalize_topic = DDSDomainParticipant_impl::finalize_topic;

    retcode = DDS_DomainParticipant_delete_contained_entities_w_finalizerI(
                        (DDS_DomainParticipant*)this->_c_entity, &finalizer);

    return retcode;
#else
    return DDS_RETCODE_OK;
#endif
}

DDS_ReturnCode_t
DDSDomainParticipant_impl::register_type(
        const char *type_name,
        NDDS_Type_Plugin *plugin)
{
    return DDS_DomainParticipant_register_type(
                (DDS_DomainParticipant*)this->_c_entity, type_name, plugin);
}

NDDS_Type_Plugin*
DDSDomainParticipant_impl::unregister_type(
           const char *type_name)
{
#ifndef RTI_CERT
    return DDS_DomainParticipant_unregister_type(
                (DDS_DomainParticipant*)this->_c_entity, type_name);
#else
    UNUSED_ARG(type_name);
    return NULL;
#endif
}

DDS_ReturnCode_t
DDSDomainParticipant_impl::set_listener(
        const DDSDomainParticipantListener *listener,
        DDS_StatusMask mask)
{
#ifndef RTI_CERT
    struct DDS_DomainParticipantListener
        c_listener = DDS_DomainParticipantListener_INITIALIZER,
        *installed_listener = NULL;

    if (listener != NULL)
    {
        DDSDomainParticipantListener_INITIALIZE_C_LISTENER(listener, &c_listener);
        installed_listener = &c_listener;
    }

    return DDS_DomainParticipant_set_listener(
                (DDS_DomainParticipant*) this->_c_entity,
                installed_listener, mask);
#else
    UNUSED_ARG(listener);
    UNUSED_ARG(mask);
    return DDS_RETCODE_UNSUPPORTED;
#endif
}



DDSDomainParticipantListener*
DDSDomainParticipant_impl::get_listener()
{
#ifndef RTI_CERT 
    struct DDS_DomainParticipantListener c_listener =
                DDS_DomainParticipantListener_INITIALIZER;

    c_listener = DDS_DomainParticipant_get_listener(
                    (DDS_DomainParticipant*) this->_c_entity);

    return (DDSDomainParticipantListener*)
                c_listener.as_subscriberlistener.as_datareaderlistener.as_listener.listener_data;
#else
    return NULL;
#endif
}


DDSEntity*
DDSDomainParticipant_impl::as_entity()
{
    return this;
}

DDS_ReturnCode_t
DDSDomainParticipant_impl::get_current_time(struct DDS_Time_t &current_time)
{
    return DDS_DomainParticipant_get_current_time(
                (DDS_DomainParticipant*)this->_c_entity, &current_time);
}

DDS_ReturnCode_t
DDSDomainParticipant_impl::add_peer(const char *peer)
{
    return DDS_DomainParticipant_add_peer(
                        (DDS_DomainParticipant*)this->_c_entity,peer);
}

