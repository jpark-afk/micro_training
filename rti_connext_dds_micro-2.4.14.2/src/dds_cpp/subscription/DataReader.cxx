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
              Moved forwarding functions from DataReaderListener.cxx
07may2014,as  MICRO-784 Expose get_X_status API in C++
08nov2013,as  MICRO-681 Complete implementation of WaitSets
              and support for StatusConditions
19jul2013,as  Major C++ update
21jan2013,eh  Created
===================================================================== */

#ifndef dds_cpp_subscription_hxx
  #include "dds_cpp/dds_cpp_subscription.hxx"
#endif
#ifndef dds_c_subscription_h
  #include "dds_c/dds_c_subscription.h"
#endif

#include "Entity.hxx"
#include "DataReader.hxx"
#include "Topic.hxx"
/*** SOURCE_BEGIN ***/

// ---------------------------------------------------------------------
// Public Methods
// ---------------------------------------------------------------------

DDSTopicDescription*
DDSDataReader_impl::get_topicdescription()
{
    DDS_TopicDescription *c_desc = NULL;
    DDS_Topic *c_topic = NULL;
    DDSTopic *topic = NULL;
    DDSTopic_impl *topic_impl = NULL;
    DDSTopicDescription* result = NULL;
    c_desc = DDS_DataReader_get_topicdescription(
                        (DDS_DataReader*) this->_c_entity);
    if (c_desc != NULL)
    {
        c_topic = DDS_Topic_narrow(c_desc);
        if (c_topic != NULL)
        {
            topic = (DDSTopic*)
                    DDS_Entity_get_wrapper(DDS_Topic_as_entity(c_topic));
            if (topic != NULL)
            {
                topic_impl = static_cast<DDSTopic_impl*>(topic);
                result = topic_impl;
            }
        }
    }
    return result;
}

DDSSubscriber*
DDSDataReader_impl::get_subscriber()
{
    DDS_Subscriber *c_subscriber = NULL;
    DDSSubscriber *result = NULL;
    c_subscriber = DDS_DataReader_get_subscriber(
                                (DDS_DataReader*) this->_c_entity);
    if (c_subscriber != NULL)
    {
        result = (DDSSubscriber*)
                DDS_Entity_get_wrapper(
                    DDS_Subscriber_as_entity(c_subscriber));
    }
    return result;
}


DDS_ReturnCode_t
DDSDataReader_impl::set_qos(
        const DDS_DataReaderQos& qos)
{
#ifndef RTI_CERT
    return DDS_DataReader_set_qos((DDS_DataReader*) this->_c_entity, &qos);
#else
    UNUSED_ARG(qos);
    return DDS_RETCODE_UNSUPPORTED;
#endif /* !RTI_CERT */
}

DDS_ReturnCode_t
DDSDataReader_impl::get_qos(DDS_DataReaderQos& qos)
{
#ifndef RTI_CERT
    return DDS_DataReader_get_qos((DDS_DataReader*) this->_c_entity, &qos);
#else
    UNUSED_ARG(qos);
    return DDS_RETCODE_UNSUPPORTED;
#endif /* !RTI_CERT */
}


DDS_ReturnCode_t
DDSDataReader_impl::set_listener(
        const DDSDataReaderListener *listener,
        DDS_StatusMask mask)
{
    struct DDS_DataReaderListener c_listener = DDS_DataReaderListener_INITIALIZER,
                *installed_listener = NULL;

    if (listener != NULL)
    {
        DDSDataReaderListener_INITIALIZE_C_LISTENER(listener,&c_listener);
        installed_listener = &c_listener;
    }

    return DDS_DataReader_set_listener(
                (DDS_DataReader*) this->_c_entity,
                installed_listener, mask);
}



DDSDataReaderListener*
DDSDataReader_impl::get_listener()
{
#ifndef RTI_CERT
    struct DDS_DataReaderListener c_listener =  DDS_DataReaderListener_INITIALIZER;

    c_listener = DDS_DataReader_get_listener((DDS_DataReader*) this->_c_entity);

    return (DDSDataReaderListener*) c_listener.as_listener.listener_data;
#else
    return NULL;
#endif 
}


DDS_ReturnCode_t
DDSDataReader_impl::read_untyped(
        DDS_UntypedSampleSeq *received_data,
        DDS_SampleInfoSeq& info_seq,
        DDS_Long max_samples,
        DDS_SampleStateMask sample_states,
        DDS_ViewStateMask view_states,
        DDS_InstanceStateMask instance_states)
{
    return DDS_DataReader_read(
                (DDS_DataReader*) this->_c_entity,
                received_data, &info_seq, max_samples,
                sample_states, view_states, instance_states);
}

DDS_ReturnCode_t
DDSDataReader_impl::take_untyped(
        DDS_UntypedSampleSeq *received_data,
        DDS_SampleInfoSeq& info_seq,
        DDS_Long max_samples,
        DDS_SampleStateMask sample_states,
        DDS_ViewStateMask view_states,
        DDS_InstanceStateMask instance_states)
{
    return DDS_DataReader_take(
                (DDS_DataReader*) this->_c_entity,
                received_data, &info_seq, max_samples,
                sample_states, view_states, instance_states);
}

DDS_ReturnCode_t
DDSDataReader_impl::read_instance_untyped(
        DDS_UntypedSampleSeq *received_data,
        DDS_SampleInfoSeq& info_seq,
        DDS_Long max_samples,
        const DDS_InstanceHandle_t& a_handle,
        DDS_SampleStateMask sample_states,
        DDS_ViewStateMask view_states,
        DDS_InstanceStateMask instance_states)
{
    return DDS_DataReader_read_instance(
                (DDS_DataReader*) this->_c_entity,
                received_data, &info_seq, max_samples,
                &a_handle, sample_states, view_states, instance_states);
}

DDS_ReturnCode_t
DDSDataReader_impl::take_instance_untyped(
        DDS_UntypedSampleSeq *received_data,
        DDS_SampleInfoSeq& info_seq,
        DDS_Long max_samples,
        const DDS_InstanceHandle_t& a_handle,
        DDS_SampleStateMask sample_states,
        DDS_ViewStateMask view_states,
        DDS_InstanceStateMask instance_states)
{
    return DDS_DataReader_take_instance(
                (DDS_DataReader*) this->_c_entity,
                received_data, &info_seq, max_samples,
                &a_handle, sample_states, view_states, instance_states);
}

DDS_ReturnCode_t
DDSDataReader_impl::read_next_sample_untyped(
        void* received_data,
        DDS_SampleInfo& sample_info)
{
    return DDS_DataReader_read_next_sample(
                (DDS_DataReader*) this->_c_entity, received_data, &sample_info);
}   

DDS_ReturnCode_t
DDSDataReader_impl::take_next_sample_untyped(
        void* received_data,
        DDS_SampleInfo& sample_info)
{
    return DDS_DataReader_take_next_sample(
                (DDS_DataReader*) this->_c_entity, received_data, &sample_info);
}

DDS_InstanceHandle_t
DDSDataReader_impl::lookup_instance_untyped(const void *key_holder)
{
    return DDS_DataReader_lookup_instance(
                (DDS_DataReader*) this->_c_entity, key_holder);
}

DDS_ReturnCode_t
DDSDataReader_impl::return_loan_untyped(
            DDS_UntypedSampleSeq *received_data,
            DDS_SampleInfoSeq& info_seq)
{
    return DDS_DataReader_return_loan(
                (DDS_DataReader*) this->_c_entity, received_data, &info_seq);
}

DDS_ReturnCode_t
DDSDataReader_impl::get_subscription_matched_status(
            DDS_SubscriptionMatchedStatus& status)
{
    return DDS_DataReader_get_subscription_matched_status(
                (DDS_DataReader*) this->_c_entity, &status);
}

DDS_ReturnCode_t
DDSDataReader_impl::get_liveliness_changed_status(
            DDS_LivelinessChangedStatus& status)
{
    return DDS_DataReader_get_liveliness_changed_status(
                (DDS_DataReader*) this->_c_entity, &status);
}

DDS_ReturnCode_t
DDSDataReader_impl::get_sample_rejected_status(
            DDS_SampleRejectedStatus& status)
{
    return DDS_DataReader_get_sample_rejected_status(
                (DDS_DataReader*) this->_c_entity, &status);
}

DDS_ReturnCode_t
DDSDataReader_impl::get_sample_lost_status(
            DDS_SampleLostStatus& status)
{
    return DDS_DataReader_get_sample_lost_status(
                (DDS_DataReader*) this->_c_entity, &status);
}

DDS_ReturnCode_t
DDSDataReader_impl::get_requested_deadline_missed_status(
            DDS_RequestedDeadlineMissedStatus& status)
{
    return DDS_DataReader_get_requested_deadline_missed_status(
                (DDS_DataReader*) this->_c_entity, &status);
}

DDS_ReturnCode_t
DDSDataReader_impl::get_requested_incompatible_qos_status(
            DDS_RequestedIncompatibleQosStatus& status)
{
    return DDS_DataReader_get_requested_incompatible_qos_status(
                (DDS_DataReader*) this->_c_entity, &status);
}

DDS_ReturnCode_t
DDSDataReader_impl::get_instance_replaced_missed_status(
            DDS_DataReaderInstanceReplacedStatus& status)
{
    return DDS_DataReader_get_instance_replaced_status(
                (DDS_DataReader*) this->_c_entity, &status);
}

DDSEntity*
DDSDataReader_impl::as_entity()
{
    return this;
}

// --- Constructors & destructors: -------------------------------------

DDSDataReader::DDSDataReader(DDS_DataReader* c_reader) :
        DDSEntity(DDS_DataReader_as_entity(c_reader))
{

}

DDSDataReader::~DDSDataReader()
{

}

DDSDataReader_impl::DDSDataReader_impl(DDS_DataReader* c_reader) :
        DDSDataReader(c_reader)
{

}

DDSDataReader_impl::~DDSDataReader_impl()
{

}
