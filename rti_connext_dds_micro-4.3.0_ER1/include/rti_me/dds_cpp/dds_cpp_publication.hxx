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
              Removed unused types DDSDataWriter_ptr and DDSPublisher_ptr
07may2014,as  MICRO-784 Expose get_X_status API in C++
19jul2013,as  Major C++ update
16jan2013,eh  Created
===================================================================== */

#ifndef dds_cpp_publication_hxx
#define dds_cpp_publication_hxx

/*ci
 * \file
 * \brief DDS publication module
 */
#ifndef dds_cpp_dll_hxx
  #include "dds_cpp/dds_cpp_dll.hxx"
#endif
#ifndef dds_cpp_infrastructure_hxx
  #include "dds_cpp/dds_cpp_infrastructure.hxx"
#endif
#ifndef dds_cpp_topic_hxx
  #include "dds_cpp/dds_cpp_topic.hxx"
#endif
#ifndef dds_c_publication_h
  #include "dds_c/dds_c_publication.h"
#endif


/* ================================================================= */
/*                       Listeners                                   */
/* ================================================================= */

/*e @ingroup DDSWriterModule
 * This type is only an implementation detail of the DDSDataWriterSeq type.
 * It should not be used elsewhere.
 */
class DDSDataWriter;

/* -------------------------------*/
/* Data Writer Listener interface */
/* -------------------------------*/

/* ----------------------------------------------------------------- */
/*e \dref_DataWriterListener
 */
class DDSCPPDllExport DDSDataWriterListener : public virtual DDSListener 
{
  public:
    /*e \dref_DataWriterListener_on_offered_deadline_missed
     */
    virtual void on_offered_deadline_missed(
            DDSDataWriter *writer,
            const DDS_OfferedDeadlineMissedStatus& status) {UNUSED_ARG(writer); UNUSED_ARG(status); }

    /*e \dref_DataWriterListener_on_liveliness_lost
     */
    virtual void on_liveliness_lost(
            DDSDataWriter *writer,
            const DDS_LivelinessLostStatus& status) {UNUSED_ARG(writer);UNUSED_ARG(status); }

    /*e \dref_DataWriterListener_on_offered_incompatible_qos
     */
    virtual void on_offered_incompatible_qos(
            DDSDataWriter *writer,
            const DDS_OfferedIncompatibleQosStatus& status) {UNUSED_ARG(writer);UNUSED_ARG(status); }

    /*e \dref_DataWriterListener_on_publication_matched
     */
    virtual void on_publication_matched(
            DDSDataWriter *writer,
            const DDS_PublicationMatchedStatus& status) {UNUSED_ARG(writer);UNUSED_ARG(status); }

    /*e \dref_DataWriterListener_on_reliable_reader_activity_changed
     */
    virtual void on_reliable_reader_activity_changed(
            DDSDataWriter *writer,
            const DDS_ReliableReaderActivityChangedStatus& status) {UNUSED_ARG(writer);UNUSED_ARG(status); }

    /*e \dref_DataWriterListener_on_reliable_sample_unacknowledged
     */
    virtual void on_reliable_sample_unacknowledged(
            DDSDataWriter *writer,
            const DDS_ReliableSampleUnacknowledgedStatus& status) {UNUSED_ARG(writer);UNUSED_ARG(status); }

    /*e \dref_DataWriterListener_on_sample_removed
     */
    virtual void on_sample_removed(
            DDSDataWriter *writer,
            const DDS_Cookie_t& cookie){ UNUSED_ARG(writer);UNUSED_ARG(cookie);}

  public:
    DDSDataWriterListener() { }
    virtual ~DDSDataWriterListener() { }

};

class DDSPublisher;

/* -------------------------------*/
/* Publisher Listener interface   */
/* -------------------------------*/

/* ----------------------------------------------------------------- */
/*e \dref_PublisherListener
 */
class DDSCPPDllExport DDSPublisherListener : public virtual DDSDataWriterListener 
{
  public:
    DDSPublisherListener() { }
    virtual ~DDSPublisherListener() { }
};

/* ================================================================= */
/*                            Entity                                 */
/* ================================================================= */

class DDSPublisher;

class DDSDomainParticipant;
class DDSTopic;
/* ----------------------------------------------------------------- */
/*e \dref_Publisher
 */
class DDSCPPDllExport DDSPublisher : public DDSEntity 
{
  // --- <<interface>> DDSPublisher: -----------------------------------
  public:
    /*ce \dref_Publisher_as_entity
     */
    virtual DDSEntity* as_entity() = 0;

    /*e \dref_Publisher_create_datawriter
     */
    virtual DDSDataWriter* create_datawriter(
            DDSTopic* topic,
            const DDS_DataWriterQos& qos,
            DDSDataWriterListener* listener,
            DDS_StatusMask mask) = 0;

    /*e \dref_Publisher_delete_datawriter
     */
    virtual DDS_ReturnCode_t delete_datawriter(
            DDSDataWriter* a_datawriter) = 0;

    /*e \dref_Publisher_get_default_datawriter_qos
     */
    virtual DDS_ReturnCode_t get_default_datawriter_qos(
            DDS_DataWriterQos& qos) = 0;

    /*e \dref_Publisher_set_default_datawriter_qos
     */
    virtual DDS_ReturnCode_t set_default_datawriter_qos(
            const DDS_DataWriterQos& qos) = 0;

    /*e \dref_Publisher_lookup_datawriter
     */
    virtual DDSDataWriter* lookup_datawriter(
            const char* topic_name) = 0;

#if DDS_ENABLE_APPGEN
    /*e \dref_Publisher_lookup_datawriter_by_name
     */
    virtual DDSDataWriter* lookup_datawriter_by_name(
            const char* writer_name) = 0;
#endif /* DDS_ENABLE_APPGEN */

    /*e \dref_Publisher_get_participant
     */
    virtual DDSDomainParticipant* get_participant() = 0;

    /*e \dref_Publisher_get_qos
     */
    virtual DDS_ReturnCode_t get_qos(
            DDS_PublisherQos& qos) = 0;

    /*e \dref_Publisher_set_qos
     */
    virtual DDS_ReturnCode_t set_qos(
            const DDS_PublisherQos& qos) = 0;

    /*e \dref_Publisher_delete_contained_entities
     */
    virtual DDS_ReturnCode_t delete_contained_entities() = 0;

    /*e \dref_Publisher_set_listener
     */
    virtual DDS_ReturnCode_t set_listener(
            const DDSPublisherListener *listener,
            DDS_StatusMask mask) = 0;

    /*e \dref_Publisher_get_listener
     */
    virtual DDSPublisherListener* get_listener() = 0;

  protected: 

    // --- <<lifecycle>>: ------------------------------------------------
    DDSPublisher(DDS_Publisher *c_publisher);

    ~DDSPublisher();

};


struct DDS_SubscriptionBuiltinTopicData;
/* ----------------------------------------------------------------- */
/*e \dref_DataWriter
 */
class DDSCPPDllExport DDSDataWriter : public DDSEntity 
{

  public:
    /*ce \dref_DataWriter_as_entity
     */
    virtual DDSEntity* as_entity() = 0;

    // --- Untyped endpoint methods ------------------------------------------

    /*e \dref_DataWriter_write_untyped
     */
    virtual DDS_ReturnCode_t write_untyped(
            const void* instance_data,
            const DDS_InstanceHandle_t& handle) = 0;

    /*e \dref_DataWriter_assert_liveliness
     */
    virtual DDS_ReturnCode_t assert_liveliness() = 0;

    /*e \dref_DataWriter_get_topic
     */
    virtual DDSTopic* get_topic() = 0;

    /*e \dref_DataWriter_get_publisher
     */
    virtual DDSPublisher* get_publisher() = 0;

#if DDS_XTYPES_IS_ENABLED
    virtual DDS_TypeCode* get_typecode() = 0;
#endif

    /*e \dref_DataWriter_get_qos
     */
    virtual DDS_ReturnCode_t get_qos(
            DDS_DataWriterQos& qos) = 0;

    /*e \dref_DataWriter_set_qos
     */
    virtual DDS_ReturnCode_t set_qos(
            const DDS_DataWriterQos& qos) = 0;

    /*e \dref_DataWriter_set_listener
     */
    virtual DDS_ReturnCode_t set_listener(
            const DDSDataWriterListener *listener,
            DDS_StatusMask mask) = 0;

    /*e \dref_DataWriter_get_listener
     */
    virtual DDSDataWriterListener* get_listener() = 0;

    /*e \dref_DataWriter_register_instance_untyped
     */
    virtual DDS_InstanceHandle_t register_instance_untyped(
            const void *instance_data) = 0;

    /*e \dref_DataWriter_register_instance_w_timestamp_untyped
     */
    virtual DDS_InstanceHandle_t register_instance_w_timestamp_untyped(
            const void *instance_data,
            const DDS_Time_t& source_timestamp) = 0;

    /*e \dref_DataWriter_unregister_instance_untyped
     */
    virtual DDS_ReturnCode_t unregister_instance_untyped(
            const void *instance_data,
            const DDS_InstanceHandle_t& handle) = 0;

    /*e \dref_DataWriter_unregister_instance_w_timestamp_untyped
     */
    virtual DDS_ReturnCode_t unregister_instance_w_timestamp_untyped(
            const void *instance_data,
            const DDS_InstanceHandle_t& handle,
            const DDS_Time_t& source_timestamp) = 0;

    /*e \dref_DataWriter_dispose_untyped
     */
    virtual DDS_ReturnCode_t dispose_untyped(
            const void *instance_data,
            const DDS_InstanceHandle_t& handle) = 0;

    /*e \dref_DataWriter_dispose_w_timestamp_untyped
     */
    virtual DDS_ReturnCode_t dispose_w_timestamp_untyped(
            const void *instance_data,
            const DDS_InstanceHandle_t& handle,
            const DDS_Time_t& source_timestamp) = 0;

    /*e \dref_DataWriter_write_w_timestamp_untyped
     */
    virtual DDS_ReturnCode_t write_w_timestamp_untyped(
            const void *instance_data,
            const DDS_InstanceHandle_t& handle,
            const DDS_Time_t& source_timestamp) = 0;

    virtual DDS_ReturnCode_t get_loan_untyped(void **sample) = 0;

    virtual DDS_ReturnCode_t discard_loan_untyped(void *sample) = 0;

     /*e \dref_DataWriter_get_publication_matched_status
     */
    virtual DDS_ReturnCode_t get_publication_matched_status(
            DDS_PublicationMatchedStatus& status) = 0;

    /*e \dref_DataWriter_get_liveliness_lost_status
     */
    virtual DDS_ReturnCode_t get_liveliness_lost_status(
            DDS_LivelinessLostStatus& status) = 0;

#if INCLUDE_API_LOOKUP
    /*e \dref_DataWriter_get_matched_subscriptions
    */
    virtual DDS_ReturnCode_t get_matched_subscriptions(
                DDS_InstanceHandleSeq &subscription_handles) = 0;

    /*e \dref_DataWriter_get_matched_subscription_data
     */
    virtual DDS_ReturnCode_t get_matched_subscription_data(
                DDS_SubscriptionBuiltinTopicData &subscription_data,
                const DDS_InstanceHandle_t &subscription_handle) = 0;
#endif /* INCLUDE_API_LOOKUP */

    /*e \dref_DataWriter_get_offered_deadline_missed_status
     */
    virtual DDS_ReturnCode_t get_offered_deadline_missed_status(
            DDS_OfferedDeadlineMissedStatus& status) = 0;

    /*e \dref_DataWriter_get_offered_incompatible_qos_status
     */
    virtual DDS_ReturnCode_t get_offered_incompatible_qos_status(
            DDS_OfferedIncompatibleQosStatus& status) = 0;

  protected:
    // --- <<constructor/destructor>> ---------------------------------------
    DDSDataWriter(DDS_DataWriter* c_writer);

    ~DDSDataWriter();
};

class DDSPublisher_impl;

class DDSCPPDllExport DDSDataWriter_impl : public DDSDataWriter
{

  friend class DDSPublisher_impl;

  public:

    DDSEntity* as_entity();

    // --- Untyped endpoint methods ------------------------------------------
    DDS_ReturnCode_t write_untyped(
            const void* instance_data,
            const DDS_InstanceHandle_t& handle);

    DDS_ReturnCode_t assert_liveliness();

    DDSTopic* get_topic();

    DDSPublisher* get_publisher();

    DDS_TypeCode* get_typecode();

    DDS_ReturnCode_t get_qos(DDS_DataWriterQos& qos);

    DDS_ReturnCode_t set_qos(const DDS_DataWriterQos& qos);

    DDS_ReturnCode_t set_listener(
            const DDSDataWriterListener *listener,
            DDS_StatusMask mask);

    DDSDataWriterListener* get_listener();

    DDS_InstanceHandle_t register_instance_untyped(
            const void *instance_data);

    DDS_InstanceHandle_t register_instance_w_timestamp_untyped(
            const void *instance_data,
            const DDS_Time_t& source_timestamp);

    DDS_ReturnCode_t unregister_instance_untyped(
            const void *instance_data,
            const DDS_InstanceHandle_t& handle);

    DDS_ReturnCode_t unregister_instance_w_timestamp_untyped(
            const void *instance_data,
            const DDS_InstanceHandle_t& handle,
            const DDS_Time_t& source_timestamp);

    DDS_ReturnCode_t dispose_untyped(
            const void *instance_data,
            const DDS_InstanceHandle_t& handle);

    DDS_ReturnCode_t dispose_w_timestamp_untyped(
            const void *instance_data,
            const DDS_InstanceHandle_t& handle,
            const DDS_Time_t& source_timestamp);

    DDS_ReturnCode_t get_loan_untyped(void **sample);

    DDS_ReturnCode_t discard_loan_untyped(void *sample);

    DDS_ReturnCode_t write_w_timestamp_untyped(
            const void *instance_data,
            const DDS_InstanceHandle_t& handle,
            const DDS_Time_t& source_timestamp);

    DDS_ReturnCode_t get_publication_matched_status(
            DDS_PublicationMatchedStatus& status);

    DDS_ReturnCode_t get_liveliness_lost_status(
            DDS_LivelinessLostStatus& status);

#if INCLUDE_API_LOOKUP
    DDS_ReturnCode_t get_matched_subscriptions(
                DDS_InstanceHandleSeq &subscription_handles);

    DDS_ReturnCode_t get_matched_subscription_data(
                DDS_SubscriptionBuiltinTopicData &subscription_data,
                const DDS_InstanceHandle_t &subscription_handle);
#endif /* INCLUDE_API_LOOKUP */

    DDS_ReturnCode_t get_offered_deadline_missed_status(
            DDS_OfferedDeadlineMissedStatus& status);

    DDS_ReturnCode_t get_offered_incompatible_qos_status(
            DDS_OfferedIncompatibleQosStatus& status);

    DDS_ReturnCode_t get_reliable_reader_activity_changed_status(
            DDS_ReliableReaderActivityChangedStatus& status);

  protected:
    // --- <<constructor/destructor>> ---------------------------------------
    DDSDataWriter_impl(DDS_DataWriter* c_writer);

    ~DDSDataWriter_impl();

};

#endif /* dds_cpp_publication_hxx */

