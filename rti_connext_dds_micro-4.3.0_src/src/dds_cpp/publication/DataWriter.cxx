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
              Moved forwarding functions to DataWriterListener.cxx
07may2014,as  MICRO-784 Expose get_X_status API in C++
19jul2013,as  Major C++ update
21jan2013,eh  Created
===================================================================== */

#ifndef dds_cpp_publication_hxx
#include "dds_cpp/dds_cpp_publication.hxx"
#endif

#ifndef dds_c_publication_h
#include "dds_c/dds_c_publication.h"
#endif
#ifndef dds_c_infrastructure_h
#include "dds_c/dds_c_infrastructure.h"
#endif

#include "Entity.hxx"
#include "DataWriter.hxx"

/*** SOURCE_BEGIN ***/

// ---------------------------------------------------------------------
// Public Methods
// ---------------------------------------------------------------------
DDSDataWriter::DDSDataWriter(DDS_DataWriter *c_writer):
    DDSEntity(DDS_DataWriter_as_entity(c_writer))
{

}

DDSDataWriter::~DDSDataWriter()
{

}

DDS_ReturnCode_t
DDSDataWriter_impl::write_untyped(
                            const void* data,
                            const DDS_InstanceHandle_t& handle)
{
    return DDS_DataWriter_write(
                (DDS_DataWriter*)this->_c_entity, data, &handle);
}

DDS_ReturnCode_t
DDSDataWriter_impl::assert_liveliness()
{
    return DDS_DataWriter_assert_liveliness(
                (DDS_DataWriter*) this->_c_entity);
}

DDSTopic*
DDSDataWriter_impl::get_topic()
{
    DDS_Topic *c_topic = NULL;
    DDSTopic *topic = NULL;

    c_topic = DDS_DataWriter_get_topic((DDS_DataWriter*) this->_c_entity);
    if (c_topic != NULL)
    {
        topic = (DDSTopic*) DDS_Entity_get_wrapper(
                                DDS_Topic_as_entity(c_topic));
    }

    return topic;
}

DDSPublisher*
DDSDataWriter_impl::get_publisher()
{
    DDS_Publisher *c_publisher = NULL;
    DDSPublisher *publisher = NULL;

    c_publisher = DDS_DataWriter_get_publisher((DDS_DataWriter*) this->_c_entity);
    if (c_publisher != NULL)
    {
        publisher = (DDSPublisher*) DDS_Entity_get_wrapper(
                                DDS_Publisher_as_entity(c_publisher));
    }

    return publisher;
}

#if DDS_XTYPES_IS_ENABLED
DDS_TypeCode*
DDSDataWriter_impl::get_typecode()
{
    return DDS_DataWriter_get_typecode((DDS_DataWriter*) this->_c_entity);
}
#endif

DDS_ReturnCode_t
DDSDataWriter_impl::get_qos(
        DDS_DataWriterQos& qos)
{
#ifndef RTI_CERT
    return DDS_DataWriter_get_qos((DDS_DataWriter*) this->_c_entity, &qos);
#else
    UNUSED_ARG(qos);
    return DDS_RETCODE_UNSUPPORTED;
#endif /* !RTI_CERT */
}

DDS_ReturnCode_t
DDSDataWriter_impl::set_qos(const DDS_DataWriterQos& qos)
{
#ifndef RTI_CERT
    return DDS_DataWriter_set_qos((DDS_DataWriter*) this->_c_entity, &qos);
#else
    UNUSED_ARG(qos);
    return DDS_RETCODE_UNSUPPORTED;
#endif /* !RTI_CERT */
}

DDS_ReturnCode_t
DDSDataWriter_impl::set_listener(
        const DDSDataWriterListener *listener,
        DDS_StatusMask mask)
{
#ifndef RTI_CERT
    struct DDS_DataWriterListener c_listener = DDS_DataWriterListener_INITIALIZER,
                *installed_listener = NULL;

    if (listener != NULL)
    {
        DDSDataWriterListener_INITIALIZE_C_LISTENER(listener,&c_listener);
        installed_listener = &c_listener;
    }

    return DDS_DataWriter_set_listener(
                (DDS_DataWriter*) this->_c_entity,
                installed_listener, mask);
#else
    UNUSED_ARG(listener);
    UNUSED_ARG(mask);
    return DDS_RETCODE_UNSUPPORTED;
#endif
}



DDSDataWriterListener*
DDSDataWriter_impl::get_listener()
{
#ifndef RTI_CERT
    struct DDS_DataWriterListener c_listener = DDS_DataWriterListener_INITIALIZER;
    c_listener = DDS_DataWriter_get_listener((DDS_DataWriter*) this->_c_entity);
    return (DDSDataWriterListener*) c_listener.as_listener.listener_data;
#else
    return NULL;
#endif
}


DDS_InstanceHandle_t
DDSDataWriter_impl::register_instance_untyped(
        const void *instance_data)
{
    return DDS_DataWriter_register_instance((DDS_DataWriter*) this->_c_entity, instance_data);
}

DDS_InstanceHandle_t
DDSDataWriter_impl::register_instance_w_timestamp_untyped(
        const void *instance_data,
        const DDS_Time_t& source_timestamp)
{
    return DDS_DataWriter_register_instance_w_timestamp(
                (DDS_DataWriter*) this->_c_entity,
                instance_data, &source_timestamp);
}

DDS_ReturnCode_t
DDSDataWriter_impl::unregister_instance_untyped(
        const void *instance_data,
        const DDS_InstanceHandle_t& handle)
{
    return DDS_DataWriter_unregister_instance(
                (DDS_DataWriter*) this->_c_entity, instance_data, &handle);
}

DDS_ReturnCode_t
DDSDataWriter_impl::unregister_instance_w_timestamp_untyped(
        const void *instance_data,
        const DDS_InstanceHandle_t& handle,
        const DDS_Time_t& source_timestamp)
{
    return DDS_DataWriter_unregister_instance_w_timestamp(
                (DDS_DataWriter*) this->_c_entity,
                instance_data, &handle, &source_timestamp);
}

DDS_ReturnCode_t
DDSDataWriter_impl::dispose_untyped(
        const void *instance_data,
        const DDS_InstanceHandle_t &handle)
{
    return DDS_DataWriter_dispose(
                (DDS_DataWriter*) this->_c_entity, instance_data, &handle);
}

DDS_ReturnCode_t
DDSDataWriter_impl::dispose_w_timestamp_untyped(
        const void *instance_data,
        const DDS_InstanceHandle_t& handle,
        const DDS_Time_t& source_timestamp)
{
    return DDS_DataWriter_dispose_w_timestamp(
                (DDS_DataWriter*) this->_c_entity,
                instance_data, &handle, &source_timestamp);
}

DDS_ReturnCode_t
DDSDataWriter_impl::write_w_timestamp_untyped(
        const void *instance_data,
        const DDS_InstanceHandle_t& handle,
        const DDS_Time_t& source_timestamp)
{
    return DDS_DataWriter_write_w_timestamp(
                (DDS_DataWriter*) this->_c_entity,
                instance_data, &handle, &source_timestamp);
}

DDS_ReturnCode_t
DDSDataWriter_impl::get_loan_untyped(void **sample)
{
    return DDS_DataWriter_get_loan((DDS_DataWriter*)this->_c_entity, sample);
}

DDS_ReturnCode_t
DDSDataWriter_impl::discard_loan_untyped(void *sample)
{
    return DDS_DataWriter_discard_loan((DDS_DataWriter*)this->_c_entity, sample);
}

DDS_ReturnCode_t
DDSDataWriter_impl::get_publication_matched_status(
            DDS_PublicationMatchedStatus& status)
{
    return DDS_DataWriter_get_publication_matched_status(
                (DDS_DataWriter*) this->_c_entity, &status);
}

DDS_ReturnCode_t
DDSDataWriter_impl::get_reliable_reader_activity_changed_status(
            DDS_ReliableReaderActivityChangedStatus& status)
{
    return DDS_DataWriter_get_reliable_reader_activity_changed_status(
                (DDS_DataWriter*) this->_c_entity, &status);
}

DDS_ReturnCode_t
DDSDataWriter_impl::get_liveliness_lost_status(
            DDS_LivelinessLostStatus& status)
{
    return DDS_DataWriter_get_liveliness_lost_status(
                (DDS_DataWriter*) this->_c_entity, &status);
}

#if INCLUDE_API_LOOKUP
DDS_ReturnCode_t
DDSDataWriter_impl::get_matched_subscriptions(
                DDS_InstanceHandleSeq &subscription_handles)
{
    return DDS_DataWriter_get_matched_subscriptions(
                (DDS_DataWriter*) this->_c_entity,
                &subscription_handles);
}

DDS_ReturnCode_t
DDSDataWriter_impl::get_matched_subscription_data(
                DDS_SubscriptionBuiltinTopicData &subscription_data,
                const DDS_InstanceHandle_t &subscription_handle)
{
    return DDS_DataWriter_get_matched_subscription_data(
                (DDS_DataWriter*) this->_c_entity, &subscription_data,
                &subscription_handle);
}
#endif /* INCLUDE_API_LOOKUP */

DDS_ReturnCode_t
DDSDataWriter_impl::get_offered_deadline_missed_status(
            DDS_OfferedDeadlineMissedStatus& status)
{
    return DDS_DataWriter_get_offered_deadline_missed_status(
                (DDS_DataWriter*) this->_c_entity, &status);
}

DDS_ReturnCode_t
DDSDataWriter_impl::get_offered_incompatible_qos_status(
            DDS_OfferedIncompatibleQosStatus& status)
{
    return DDS_DataWriter_get_offered_incompatible_qos_status(
                (DDS_DataWriter*) this->_c_entity, &status);
}

DDSEntity*
DDSDataWriter_impl::as_entity()
{
    return this;
}

DDSDataWriter_impl::DDSDataWriter_impl(DDS_DataWriter *c_writer) :
        DDSDataWriter(c_writer)
{

}

DDSDataWriter_impl::~DDSDataWriter_impl()
{

}
