/* 

 (c) Copyright, Real-Time Innovations, 2013-2015.
 All rights reserved.

 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.

modification history
--------------------
22dec2015,as  MICRO-1514 Pass Status objects to callbacks by reference
16may2014,as  MICRO-795 Complete support for listener API in C++;
              MICRO-794 Remove C++ TODO and commented out code;
              Removed unused types DDSDataReader_ptr and DDSSubscriber_ptr
07may2014,as  MICRO-784 Expose get_X_status API in C++
19jul2013,as  Major C++ update
11jan2013,eh  Created
===================================================================== */

#ifndef dds_cpp_subscription_hxx
#define dds_cpp_subscription_hxx

/*ce
 * \file
 * \brief DDS subscription module
 */
/*e
  @addtogroup DDSSubscriptionModule
  @ingroup DDSCModule
  @brief Defines the \dds subscription package.
*/

#ifndef dds_cpp_dll_hxx
#include "dds_cpp/dds_cpp_dll.hxx"
#endif
#ifndef dds_cpp_infrastructure_hxx
#include "dds_cpp/dds_cpp_infrastructure.hxx"
#endif
#ifndef dds_c_subscription_h
#include "dds_c/dds_c_subscription.h"
#endif

/* ================================================================= */
/*                       Listeners                                   */
/* ================================================================= */

class DDSDataReader;

/* -------------------------------*/
/* Data Reader Listener interface */
/* -------------------------------*/

/*e \dref_DataReaderListener
  */
class DDSCPPDllExport DDSDataReaderListener : 
    public virtual DDSListener 
{
  // --- <<interface>> DDSDataReaderListener: -----------------------
  public:

    /*e \dref_DataReaderListener_on_data_available
     */
    virtual void on_data_available(DDSDataReader*) { }

    /*e \dref_DataReaderListener_on_requested_deadline_missed
     */
    virtual void on_requested_deadline_missed(
            DDSDataReader*,
            const DDS_RequestedDeadlineMissedStatus&) { }

    /*e \dref_DataReaderListener_on_liveliness_changed
     */
    virtual void on_liveliness_changed(
            DDSDataReader *,
            const DDS_LivelinessChangedStatus&) { }

    /*e \dref_DataReaderListener_on_requested_incompatible_qos
     */
    virtual void on_requested_incompatible_qos(
            DDSDataReader *,
            const DDS_RequestedIncompatibleQosStatus&) { }

    /*e \dref_DataReaderListener_on_sample_rejected
     */
    virtual void on_sample_rejected(
            DDSDataReader *,
            const DDS_SampleRejectedStatus&) { }

    /*e \dref_DataReaderListener_on_subscription_matched
     */
    virtual void on_subscription_matched(
            DDSDataReader *,
            const DDS_SubscriptionMatchedStatus&) { }

    /*e \dref_DataReaderListener_on_sample_lost
     */
    virtual void on_sample_lost(
            DDSDataReader *,
            const DDS_SampleLostStatus&) { }

    /*e \dref_DataReaderListener_on_instance_replaced
     */
    virtual void on_instance_replaced(
            DDSDataReader *,
            const DDS_DataReaderInstanceReplacedStatus&) { }

    /*e \dref_DataReaderListener_on_before_sample_deserialize
     */
    virtual DDS_Boolean on_before_sample_deserialize(
            DDSDataReader *,
            NDDS_Type_Plugin *,
            CDR_Stream_t *,
            DDS_Boolean *)
    {
        return RTI_TRUE;
    }

    /*e \dref_DataReaderListener_on_before_sample_commit
     */
    virtual DDS_Boolean on_before_sample_commit(
            DDSDataReader *,
            const void *const,
            const struct DDS_SampleInfo *const,
            DDS_Boolean *)
    {
        return RTI_TRUE;
    }

  public:
    DDSDataReaderListener() { }
    virtual ~DDSDataReaderListener() { }
};  


class DDSSubscriber;

/* -------------------------------*/
/*  Subscriber Listener interface */
/* -------------------------------*/

/*e \dref_SubscriberListener
 */
class DDSCPPDllExport DDSSubscriberListener : 
    public virtual DDSDataReaderListener 
{
  public:
    /*e \dref_SubscriberListener_on_data_on_readers
     */
    virtual void on_data_on_readers(DDSSubscriber*) { }

  public:
    DDSSubscriberListener() { }
    virtual ~DDSSubscriberListener() { }
};

/* ================================================================= */
/*                         Conditions                                */
/* ================================================================= */


/* ====================================================== */
/*                   Subscriber                              */
/* ====================================================== */

class DDSSubscriber;
class DDSTopicDescription;
class DDSDomainParticipant;

/* ---------------------*/
/* Subscriber interface */
/* ---------------------*/

/*e \dref_Subscriber
 */  
class DDSCPPDllExport DDSSubscriber : public DDSEntity
{
  public:
    // --- <<interface>> DDSSubscriber: ----------------------------------
    /*e \dref_Subscriber_create_datareader
     */
    virtual DDSDataReader* create_datareader(
            DDSTopicDescription* topic,
            const DDS_DataReaderQos& qos,
            DDSDataReaderListener* listener,
            DDS_StatusMask mask) = 0;

    /*e \dref_Subscriber_delete_datareader
     */
    virtual DDS_ReturnCode_t delete_datareader(
            DDSDataReader* a_datareader) = 0;

    /*e \dref_Subscriber_get_default_datareader_qos
     */
    virtual DDS_ReturnCode_t get_default_datareader_qos(
            DDS_DataReaderQos& qos) = 0;

    /*e \dref_Subscriber_set_default_datareader_qos
     */
    virtual DDS_ReturnCode_t set_default_datareader_qos(
            const DDS_DataReaderQos& qos) = 0;

    /*e \dref_Subscriber_delete_contained_entities
     */
    virtual DDS_ReturnCode_t delete_contained_entities() = 0;

    /*e \dref_Subscriber_lookup_datareader
     */
    virtual DDSDataReader* lookup_datareader(const char *topic_name) = 0;

    /*e \dref_Subscriber_get_participant
     */
    virtual DDSDomainParticipant* get_participant() = 0;

    /*e \dref_Subscriber_set_qos
     */
    virtual DDS_ReturnCode_t set_qos(const DDS_SubscriberQos& qos) = 0;

    /*e \dref_Subscriber_get_qos
     */
    virtual DDS_ReturnCode_t get_qos(DDS_SubscriberQos& qos) = 0;
    
    /*e \dref_Subscriber_set_listener
     */
    virtual DDS_ReturnCode_t set_listener(
            const DDSSubscriberListener *listener,
            DDS_StatusMask mask) = 0;

    /*e \dref_Subscriber_get_listener
     */
    virtual DDSSubscriberListener* get_listener() = 0;

  protected: 

    // --- <<lifecycle>>: ----------------------------------------------
    DDSSubscriber(DDS_Subscriber *c_subscriber);

    ~DDSSubscriber();
};



/* ================================================================= */
/*                                    Reader                         */
/* ================================================================= */

/*e \dref_DataReader
 */  
class DDSCPPDllExport DDSDataReader : public DDSEntity
{

  // --- <<interface>> DDSDataReader: ----------------------------------
  public:
    /*e \dref_DataReader_as_entity
    */
    virtual DDSEntity* as_entity() = 0;

    /*e \dref_DataReader_get_topicdescription
    */
    virtual DDSTopicDescription* get_topicdescription() = 0;

    /*e \dref_DataReader_get_subscriber
    */
    virtual DDSSubscriber* get_subscriber() = 0;

    /*e \dref_DataReader_set_qos
    */
    virtual DDS_ReturnCode_t set_qos(const DDS_DataReaderQos& qos) = 0;

    /*e \dref_DataReader_get_qos
    */
    virtual DDS_ReturnCode_t get_qos(DDS_DataReaderQos& qos) = 0;

    /*e \dref_DataReader_set_listener
    */
    virtual DDS_ReturnCode_t set_listener(
            const DDSDataReaderListener *listener,
            DDS_StatusMask mask) = 0;

    /*e \dref_DataReader_get_listener
    */
    virtual DDSDataReaderListener* get_listener() = 0;

    /*e \dref_DataReader_read_untyped
    */
    virtual DDS_ReturnCode_t read_untyped(
            DDS_UntypedSampleSeq *received_data,
            DDS_SampleInfoSeq& info_seq,
            DDS_Long max_samples,
            DDS_SampleStateMask sample_states,
            DDS_ViewStateMask view_states,
            DDS_InstanceStateMask instance_states) = 0;

    /*e \dref_DataReader_take_untyped
    */
    virtual DDS_ReturnCode_t take_untyped(
            DDS_UntypedSampleSeq *received_data,
            DDS_SampleInfoSeq& info_seq,
            DDS_Long max_samples,
            DDS_SampleStateMask sample_states,
            DDS_ViewStateMask view_states,
            DDS_InstanceStateMask instance_states) = 0;

    /*e \dref_DataReader_read_instance_untyped
    */
    virtual DDS_ReturnCode_t read_instance_untyped(
            DDS_UntypedSampleSeq *received_data,
            DDS_SampleInfoSeq& info_seq,
            DDS_Long max_samples,
            const DDS_InstanceHandle_t& a_handle,
            DDS_SampleStateMask sample_states,
            DDS_ViewStateMask view_states,
            DDS_InstanceStateMask instance_states) = 0;

    /*e \dref_DataReader_take_instance_untyped
    */
    virtual DDS_ReturnCode_t take_instance_untyped(
            DDS_UntypedSampleSeq *received_data,
            DDS_SampleInfoSeq& info_seq,
            DDS_Long max_samples,
            const DDS_InstanceHandle_t& a_handle,
            DDS_SampleStateMask sample_states,
            DDS_ViewStateMask view_states,
            DDS_InstanceStateMask instance_states) = 0;

    /*e \dref_DataReader_read_next_sample_untyped
    */
    virtual DDS_ReturnCode_t read_next_sample_untyped(
            void* received_data,
            DDS_SampleInfo& sample_info) = 0;

    /*e \dref_DataReader_take_next_sample_untyped
    */
    virtual DDS_ReturnCode_t take_next_sample_untyped(
            void* received_data,
            DDS_SampleInfo& sample_info) = 0;

    /*e \dref_DataReader_lookup_instance_untyped
    */
    virtual DDS_InstanceHandle_t lookup_instance_untyped(
            const void *key_holder) = 0;

    /*e \dref_DataReader_return_loan_untyped
    */
    virtual DDS_ReturnCode_t return_loan_untyped(
            DDS_UntypedSampleSeq *received_data,
            DDS_SampleInfoSeq& info_seq) = 0;

    /*e \dref_DataReader_get_subscription_matched_status
     */
    virtual DDS_ReturnCode_t get_subscription_matched_status(
            DDS_SubscriptionMatchedStatus& status) = 0;

    /*e \dref_DataReader_get_liveliness_changed_status
     */
    virtual DDS_ReturnCode_t get_liveliness_changed_status(
            DDS_LivelinessChangedStatus& status) = 0;

    /*e \dref_DataReader_get_sample_rejected_status
     */
    virtual DDS_ReturnCode_t get_sample_rejected_status(
            DDS_SampleRejectedStatus& status) = 0;

    /*e \dref_DataReader_get_sample_lost_status
     */
    virtual DDS_ReturnCode_t get_sample_lost_status(
            DDS_SampleLostStatus& status) = 0;

    /*e \dref_DataReader_get_requested_deadline_missed_status
     */
    virtual DDS_ReturnCode_t get_requested_deadline_missed_status(
            DDS_RequestedDeadlineMissedStatus& status) = 0;

    /*e \dref_DataReader_get_requested_incompatible_qos_status
     */
    virtual DDS_ReturnCode_t get_requested_incompatible_qos_status(
            DDS_RequestedIncompatibleQosStatus& status) = 0;

    /*e \dref_DataReader_get_instance_replaced_status
     */
    virtual DDS_ReturnCode_t get_instance_replaced_missed_status(
            DDS_DataReaderInstanceReplacedStatus& status) = 0;

  protected:
    // --- <<constructor/destructor>>: ------------------------------------
    DDSDataReader(DDS_DataReader *c_reader);

    ~DDSDataReader();
};

class DDSSubscriber_impl;

class DDSCPPDllExport DDSDataReader_impl : public DDSDataReader
{

  friend class DDSSubscriber_impl;

  // --- <<interface>> DDSDataReader: ----------------------------------
  public:
    DDSEntity* as_entity();

    DDSTopicDescription* get_topicdescription();

    DDSSubscriber* get_subscriber();

    DDS_ReturnCode_t set_qos(
            const DDS_DataReaderQos& qos);

    DDS_ReturnCode_t get_qos(
            DDS_DataReaderQos& qos);

    DDS_ReturnCode_t set_listener(
            const DDSDataReaderListener *listener,
            DDS_StatusMask mask);

    DDSDataReaderListener* get_listener();

    DDS_ReturnCode_t read_untyped(
            DDS_UntypedSampleSeq *received_data,
            DDS_SampleInfoSeq& info_seq,
            DDS_Long max_samples,
            DDS_SampleStateMask sample_states,
            DDS_ViewStateMask view_states,
            DDS_InstanceStateMask instance_states);

    DDS_ReturnCode_t take_untyped(
            DDS_UntypedSampleSeq *received_data,
            DDS_SampleInfoSeq& info_seq,
            DDS_Long max_samples,
            DDS_SampleStateMask sample_states,
            DDS_ViewStateMask view_states,
            DDS_InstanceStateMask instance_states);

    DDS_ReturnCode_t read_instance_untyped(
            DDS_UntypedSampleSeq *received_data,
            DDS_SampleInfoSeq& info_seq,
            DDS_Long max_samples,
            const DDS_InstanceHandle_t& a_handle,
            DDS_SampleStateMask sample_states,
            DDS_ViewStateMask view_states,
            DDS_InstanceStateMask instance_states);

    DDS_ReturnCode_t take_instance_untyped(
            DDS_UntypedSampleSeq *received_data,
            DDS_SampleInfoSeq& info_seq,
            DDS_Long max_samples,
            const DDS_InstanceHandle_t& a_handle,
            DDS_SampleStateMask sample_states,
            DDS_ViewStateMask view_states,
            DDS_InstanceStateMask instance_states);

    DDS_ReturnCode_t read_next_sample_untyped(
            void* received_data,
            DDS_SampleInfo& sample_info);

    DDS_ReturnCode_t take_next_sample_untyped(
            void* received_data,
            DDS_SampleInfo& sample_info);

    DDS_InstanceHandle_t lookup_instance_untyped(
            const void *key_holder);

    DDS_ReturnCode_t return_loan_untyped(
            DDS_UntypedSampleSeq *received_data,
            DDS_SampleInfoSeq& info_seq);

    DDS_ReturnCode_t get_subscription_matched_status(
            DDS_SubscriptionMatchedStatus& status);

    DDS_ReturnCode_t get_liveliness_changed_status(
            DDS_LivelinessChangedStatus& status);

    DDS_ReturnCode_t get_sample_rejected_status(
            DDS_SampleRejectedStatus& status);

    DDS_ReturnCode_t get_sample_lost_status(
            DDS_SampleLostStatus& status);

    DDS_ReturnCode_t get_requested_deadline_missed_status(
            DDS_RequestedDeadlineMissedStatus& status);

    DDS_ReturnCode_t get_requested_incompatible_qos_status(
            DDS_RequestedIncompatibleQosStatus& status);

    DDS_ReturnCode_t get_instance_replaced_missed_status(
            DDS_DataReaderInstanceReplacedStatus& status);

  protected:
    // --- <<constructor/destructor>>: ------------------------------------
    DDSDataReader_impl(DDS_DataReader* c_reader);

    ~DDSDataReader_impl();
};

#endif /* dds_cpp_subscription_hxx */
