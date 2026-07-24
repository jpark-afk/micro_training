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
              MICRO-794 Remove C++ TODO and commented out code
19jul2013,as  Major C++ update
21jan2013,eh  Created
===================================================================== */

#ifndef dds_cpp_domain_hxx
#include "dds_cpp/dds_cpp_domain.hxx"
#endif
#ifndef dds_cpp_publication_hxx
#include "dds_cpp/dds_cpp_publication.hxx"
#endif

#include "Entity.hxx"
#include "Topic.hxx"
#include "DataWriter.hxx"
#include "Publisher.hxx"

/*** SOURCE_BEGIN ***/

// ---------------------------------------------------------------------
// C Methods
// ---------------------------------------------------------------------
#if DDS_ENABLE_APPGEN
DDS_DataWriter*
DDSPublisher_create_datawriter(
        DDS_Publisher *self,
        DDS_Topic *topic,
        const struct DDS_DataWriterQos *qos,
        const struct DDS_DataWriterListener *listener,
        DDS_StatusMask mask)
{
    DDS_DataWriter *ret_value = NULL;
    DDSDataWriter *writer = NULL;
    DDSPublisher *publisher;
    DDSTopic *cpp_topic;

    /* Note that this function can not have a non NULL listener */
    if ((self != NULL) && (topic != NULL) && (qos != NULL) &&
        (listener == NULL) && (mask == DDS_STATUS_MASK_NONE))
    {
        publisher = (DDSPublisher*)
                          DDS_Entity_get_wrapper(DDS_Publisher_as_entity(self));

        if (publisher != NULL)
        {
            cpp_topic = (DDSTopic*)DDS_Entity_get_wrapper(
                                                    DDS_Topic_as_entity(topic));

            writer = publisher->create_datawriter(cpp_topic, *qos, NULL,
                                                  DDS_STATUS_MASK_NONE);

            if (writer != NULL)
            {
                ret_value = (DDS_DataWriter*)writer->get_c_entity();
            }
        }
    }

    return ret_value;
}
#endif /* DDS_ENABLE_APPGEN */

// ---------------------------------------------------------------------
// Public Methods
// ---------------------------------------------------------------------

DDSPublisher::DDSPublisher(DDS_Publisher *c_publisher) :
            DDSEntity(DDS_Publisher_as_entity(c_publisher))
{

}

DDSPublisher::~DDSPublisher() { }

DDSDataWriter* 
DDSPublisher_impl::create_datawriter(
        DDSTopic* topic,
        const DDS_DataWriterQos& qos,
        DDSDataWriterListener* listener,
        DDS_StatusMask mask)
{
    DDSDataWriter *writer = NULL;
    DDS_DataWriter *c_writer = NULL;
    DDSTopic_impl *topic_impl = NULL;
    DDS_DataWriterListener c_listener = DDS_DataWriterListener_INITIALIZER;
    struct DDS_DataWriterQos modifiedQos;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    if (topic == NULL)
    {
        return NULL;
    }

    topic_impl = static_cast<DDSTopic_impl*>(topic);

    if (listener != NULL)
    {
        DDSDataWriterListener_INITIALIZE_C_LISTENER(listener, &c_listener);
    }

    /* If qos is the default qos, we will need to get the default value first
       before modifying it */
    if (&qos == &DDS_DATAWRITER_QOS_DEFAULT) 
    {
#ifndef RTI_CERT
        retcode = DDS_Publisher_get_default_datawriter_qos(
                    (DDS_Publisher*)this->_c_entity, &modifiedQos);
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

    c_writer = DDS_Publisher_create_datawriter(
                    (DDS_Publisher*)this->_c_entity,
                    (DDS_Topic*)topic_impl->_c_entity, &modifiedQos,
                    (listener != NULL)?&c_listener:NULL, mask);
    if (c_writer == NULL) 
    {
        return NULL;
    }

    writer = (DDSDataWriter*)
            DDS_Entity_get_wrapper(DDS_DataWriter_as_entity(c_writer));
    if (writer == NULL)
    {
#ifndef RTI_CERT
        DDS_Publisher_delete_datawriter(
                    (DDS_Publisher*)this->_c_entity,c_writer);
#endif
    }

    return writer;
}



DDS_ReturnCode_t 
DDSPublisher_impl::delete_datawriter(DDSDataWriter* datawriter)
{
#ifndef RTI_CERT
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDSDataWriter_impl *datawriter_impl = NULL;

    if (datawriter == NULL)
    {
        return DDS_RETCODE_BAD_PARAMETER;
    }

    datawriter_impl = static_cast<DDSDataWriter_impl*>(datawriter);
    retcode = DDS_Publisher_delete_datawriter(
                (DDS_Publisher*) this->_c_entity,
                (DDS_DataWriter*)datawriter_impl->_c_entity);

    return retcode;
#else
    UNUSED_ARG(datawriter);
    return DDS_RETCODE_OK;
#endif
}

DDS_ReturnCode_t
DDSPublisher_impl::get_default_datawriter_qos(
        DDS_DataWriterQos& qos)
{
#ifndef RTI_CERT
    return DDS_Publisher_get_default_datawriter_qos(
                    (DDS_Publisher*)this->_c_entity, &qos);
#else
    UNUSED_ARG(qos);
    return DDS_RETCODE_UNSUPPORTED;
#endif /* !RTI_CERT */
}

DDS_ReturnCode_t
DDSPublisher_impl::set_default_datawriter_qos(
        const DDS_DataWriterQos& qos)
{
#ifndef RTI_CERT
    return DDS_Publisher_set_default_datawriter_qos(
                    (DDS_Publisher*)this->_c_entity, &qos);
#else
    UNUSED_ARG(qos);
    return DDS_RETCODE_UNSUPPORTED;
#endif /* !RTI_CERT */
}

DDSDataWriter*
DDSPublisher_impl::lookup_datawriter(const char* topic_name)
{
    DDS_DataWriter *c_writer = NULL;
    DDSDataWriter *result = NULL;
    c_writer = DDS_Publisher_lookup_datawriter(
                    (DDS_Publisher*)this->_c_entity, topic_name);
    if (c_writer != NULL)
    {
        result = (DDSDataWriter*) DDS_Entity_get_wrapper(
                    DDS_DataWriter_as_entity(c_writer));
    }

    return result;
}

#if DDS_ENABLE_APPGEN
DDSDataWriter*
DDSPublisher_impl::lookup_datawriter_by_name(
        const char * datawriter_name)
{
    DDS_DataWriter *c_writer = NULL;
    DDSDataWriter *result = NULL;

    c_writer = DDS_Publisher_lookup_datawriter_by_name(
                    (DDS_Publisher*)this->_c_entity, datawriter_name);
    if (c_writer != NULL)
    {
        result = (DDSDataWriter*) DDS_Entity_get_wrapper(
                    DDS_DataWriter_as_entity(c_writer));
    }

    return result;
}
#endif /* DDS_ENABLE_APPGEN */

DDSDomainParticipant*
DDSPublisher_impl::get_participant()
{
    DDS_DomainParticipant *c_participant = NULL;
    DDSDomainParticipant *result = NULL;
    c_participant = DDS_Publisher_get_participant(
                        (DDS_Publisher*)this->_c_entity);
    if (c_participant != NULL)
    {
        result = (DDSDomainParticipant*) DDS_Entity_get_wrapper(
                    DDS_DomainParticipant_as_entity(c_participant));
    }

    return result;
}

DDS_ReturnCode_t
DDSPublisher_impl::get_qos(DDS_PublisherQos& qos)
{
#ifndef RTI_CERT
    return DDS_Publisher_get_qos((DDS_Publisher*)this->_c_entity, &qos);
#else
    UNUSED_ARG(qos);
    return DDS_RETCODE_UNSUPPORTED;
#endif /* !RTI_CERT */
}

DDS_ReturnCode_t
DDSPublisher_impl::set_qos(const DDS_PublisherQos& qos)
{
#ifndef RTI_CERT
    return DDS_Publisher_set_qos((DDS_Publisher*)this->_c_entity, &qos);
#else
    UNUSED_ARG(qos);
    return DDS_RETCODE_UNSUPPORTED;
#endif /* !RTI_CERT */
}

DDS_ReturnCode_t
DDSPublisher_impl::delete_contained_entities()
{
#ifndef RTI_CERT
    return DDS_Publisher_delete_contained_entities(
                        (DDS_Publisher*)this->_c_entity);
#else
    return DDS_RETCODE_OK;
#endif
}

DDS_ReturnCode_t
DDSPublisher_impl::set_listener(
        const DDSPublisherListener *listener,
        DDS_StatusMask mask)
{
#ifndef RTI_CERT
    struct DDS_PublisherListener
        c_listener = DDS_PublisherListener_INITIALIZER,
        *installed_listener = NULL;

    if (listener != NULL)
    {
        DDSPublisherListener_INITIALIZE_C_LISTENER(listener,&c_listener);
        installed_listener = &c_listener;
    }

    return DDS_Publisher_set_listener(
                (DDS_Publisher*) this->_c_entity,
                installed_listener, mask);
#else
    UNUSED_ARG(listener);
    UNUSED_ARG(mask);
    return DDS_RETCODE_UNSUPPORTED;
#endif
}


DDSPublisherListener*
DDSPublisher_impl::get_listener()
{
#ifndef RTI_CERT
    struct DDS_PublisherListener c_listener = DDS_PublisherListener_INITIALIZER;
    c_listener = DDS_Publisher_get_listener((DDS_Publisher*) this->_c_entity);
    return (DDSPublisherListener*)
                c_listener.as_datawriterlistener.as_listener.listener_data;
#else
    return NULL;
#endif
}


DDSEntity*
DDSPublisher_impl::as_entity()
{
    return this;
}
