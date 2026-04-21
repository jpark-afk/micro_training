/*
 * FILE: Publisher.hxx - Publisher header
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
 * 16may2014,as  MICRO-795 Complete support for listener API in C++
 * 19jul2013,as  Major C++ update
 * 28jun2013,eh  Written
 */


#ifndef Publisher_hxx
#define Publisher_hxx

#ifndef dds_cpp_domain_hxx
#include "dds_cpp/dds_cpp_domain.hxx"
#endif
#ifndef dds_cpp_publication_hxx
#include "dds_cpp/dds_cpp_publication.hxx"
#endif

#include "Entity.hxx"
#include "DataWriter.hxx"

class DDSDomainParticipant_impl;

class DDSCPPDllExport DDSPublisher_impl : public DDSPublisher
{
  friend class DDSDomainParticipant_impl;

  public:
    // --- <<interface>> DDSPublisher: -----------------------------------
    DDSEntity* as_entity();

    DDSDataWriter* create_datawriter(
            DDSTopic* topic,
            const DDS_DataWriterQos& qos,
            DDSDataWriterListener* listener,
            DDS_StatusMask mask);

    DDS_ReturnCode_t delete_datawriter(
            DDSDataWriter* a_datawriter);

    DDS_ReturnCode_t get_default_datawriter_qos(
            struct DDS_DataWriterQos& qos);

    DDS_ReturnCode_t set_default_datawriter_qos(
            const struct DDS_DataWriterQos& qos);

    DDSDataWriter* lookup_datawriter(
            const char* topic_name);

    DDSDomainParticipant* get_participant();

    DDS_ReturnCode_t get_qos(
            struct DDS_PublisherQos& qos);

    DDS_ReturnCode_t set_qos(
            const struct DDS_PublisherQos& qos);

    DDS_ReturnCode_t delete_contained_entities();
    
    DDS_ReturnCode_t set_listener(
            const DDSPublisherListener *listener,
            DDS_StatusMask mask);

    DDSPublisherListener* get_listener();

  protected:
    // --- <<lifecycle>>: ------------------------------------------------
    DDSPublisher_impl(DDS_Publisher *c_publisher) :
        DDSPublisher(c_publisher) { }

    virtual ~DDSPublisher_impl() { }

};

#define DDSPublisherListener_INITIALIZE_C_LISTENER(listener_, c_listener_)\
{\
    DDSDataWriterListener_INITIALIZE_C_LISTENER(\
            (listener_), &((c_listener_)->as_datawriterlistener));\
}

#endif /* Publisher_hxx */
