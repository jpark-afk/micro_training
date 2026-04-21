/*
 * FILE: DomainParticipantFactory.hxx - DomainParticipantFactory header
 *
 * (c) Copyright 2008-2015 Real-Time Innovations,
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
 * 16may2014,as  MICRO-794 Remove C++ TODO and commented out code
 * 19jul2013,as  Major C++ update
 * 28jun2013,eh  Written
 */

#ifndef DomainParticipantFactory_hxx
#define DomainParticipantFactory_hxx

#ifndef dds_cpp_domain_hxx
#include "dds_cpp/dds_cpp_domain.hxx"
#endif
#ifndef dds_c_domain_h
#include "dds_c/dds_c_domain.h"
#endif

#include "DataWriter.hxx"
#include "DomainParticipant.hxx"

/* ----------------------------------------------------------------- */
/*e \dref_DomainParticipantFactory
 */
class DDSDomainParticipantFactory_impl :
        public DDSDomainParticipantFactory
{

  // --- <<interface>> DDSDomainParticipantFactory ---------------------
  public:
    DDSDomainParticipant* create_participant(
            DDS_DomainId_t domainId,
            const DDS_DomainParticipantQos& qos,
            DDSDomainParticipantListener* listener,
            DDS_StatusMask mask);

    DDS_ReturnCode_t delete_participant(
            DDSDomainParticipant* a_participant);

    DDSDomainParticipant* lookup_participant(
            DDS_DomainId_t domainId);

    DDS_ReturnCode_t set_default_participant_qos(
            const struct DDS_DomainParticipantQos& qos);

    DDS_ReturnCode_t get_default_participant_qos(
            struct DDS_DomainParticipantQos& qos);

    DDS_ReturnCode_t get_qos(
            struct DDS_DomainParticipantFactoryQos& qos);

    DDS_ReturnCode_t set_qos(
            const struct DDS_DomainParticipantFactoryQos& qos);

    RTRegistry* get_registry();

 
  public:
    // --- <<lifecycle>>: ------------------------------------------------
    ~DDSDomainParticipantFactory_impl();

    DDSDomainParticipantFactory_impl();

  private:
    DDS_DomainParticipantFactory* _c_domain_part_factory;
};

#endif /* DomainParticipantFactory_hxx */
