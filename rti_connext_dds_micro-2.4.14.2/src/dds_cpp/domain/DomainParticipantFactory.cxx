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
19jul2013,as  Major C++ update
11jan2013,eh  Created
===================================================================== */

#ifndef dds_cpp_domain_hxx
#include "dds_cpp/dds_cpp_domain.hxx"
#endif
#ifndef dds_c_domain_h
#include "dds_c/dds_c_domain.h"
#endif
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif

#include "dds_cpp/dds_cpp_infrastructure.hxx"

#include "Entity.hxx"
#include "DomainParticipant.hxx"
#include "DomainParticipantFactory.hxx"

/*** SOURCE_BEGIN ***/

// ---------------------------------------------------------------------
// Initialize static members
// ---------------------------------------------------------------------
DDSDomainParticipantFactory* DDSDomainParticipantFactory::_instance = NULL;
volatile RTI_INT32 DDSDomainParticipantFactory::_instance_initialized = 0;


DDSDomainParticipantFactory* 
DDSDomainParticipantFactory::get_instance() 
{
    /* initialize DomainParticipantFactory's C-factory instance */
    if (!DDSDomainParticipantFactory::_instance_initialized)
    {
        DDSDomainParticipantFactory::_instance =
                    new DDSDomainParticipantFactory_impl();
        if (DDSDomainParticipantFactory::_instance == NULL)
        {
            return NULL;
        }
        DDSDomainParticipantFactory::_instance_initialized = 1;
    }

    return
        (DDSDomainParticipantFactory*)DDSDomainParticipantFactory::_instance;
}

DDS_ReturnCode_t
DDSDomainParticipantFactory::finalize_instance() 
{
#ifndef RTI_CERT
    DDS_ReturnCode_t result = DDS_RETCODE_ERROR;

       result = DDS_DomainParticipantFactory_finalize_instance();
    if (result == DDS_RETCODE_OK
            && DDSDomainParticipantFactory::_instance_initialized)
    {
        delete ((DDSDomainParticipantFactory_impl*)
                    DDSDomainParticipantFactory::_instance);
        DDSDomainParticipantFactory::_instance = NULL;
        DDSDomainParticipantFactory::_instance_initialized = 0;
    }

    return result;
#else
    return DDS_RETCODE_UNSUPPORTED;
#endif
}

DDSDomainParticipant*
DDSDomainParticipantFactory_impl::create_participant(
    DDS_DomainId_t domainId,
    const DDS_DomainParticipantQos& qos, 
    DDSDomainParticipantListener* listener,
    DDS_StatusMask mask) 
{
    DDSDomainParticipant_impl *participant_impl = NULL;
    DDS_DomainParticipant *c_participant = NULL;
    
    struct DDS_DomainParticipantListener c_listener =
        DDS_DomainParticipantListener_INITIALIZER;

    struct DDS_DomainParticipantQos modifiedQos;

    if (listener != NULL) 
    {
        DDSDomainParticipantListener_INITIALIZE_C_LISTENER(listener, &c_listener);
    }

    /* If qos is the default qos, we will need to get the default value first
       before modifying it */
    if (&qos == &DDS_PARTICIPANT_QOS_DEFAULT) 
    {
#ifndef RTI_CERT
        DDS_DomainParticipantFactory_get_default_participant_qos(
                this->_c_domain_part_factory, &modifiedQos);
#endif /* !RTI_CERT */
    } 
    else 
    {
        modifiedQos.copy(qos);
    }

    /* create C DomainParticipant */
    c_participant = DDS_DomainParticipantFactory_create_participant(
            this->_c_domain_part_factory, domainId, &modifiedQos,
            (listener != NULL)?&c_listener:NULL, mask);

    if (c_participant == NULL) 
    {
        return NULL;
    }

    participant_impl = new DDSDomainParticipant_impl(c_participant);

    /* Store C++ participant with C entity */
    DDS_Entity_set_wrapper((DDS_Entity *)c_participant,
                             (void *)participant_impl);

    return (DDSDomainParticipant *)participant_impl;
}

DDS_ReturnCode_t 
DDSDomainParticipantFactory_impl::delete_participant(
        DDSDomainParticipant* participant)
{
#ifndef RTI_CERT
    DDS_ReturnCode_t result = DDS_RETCODE_ERROR;

    DDSDomainParticipant_impl *participant_impl =
        (DDSDomainParticipant_impl *)participant; 

    result = DDS_DomainParticipantFactory_delete_participant(
                    this->_c_domain_part_factory,
                    (DDS_DomainParticipant*)participant_impl->_c_entity);
    if (result != DDS_RETCODE_OK)
    {
        return result;
    }

    delete participant_impl;

    return DDS_RETCODE_OK;
#else
    UNUSED_ARG(participant);
    return DDS_RETCODE_UNSUPPORTED;
#endif
}

DDSDomainParticipant*
DDSDomainParticipantFactory_impl::lookup_participant(
        DDS_DomainId_t domainId)
{
    DDSDomainParticipant *result = NULL;
    DDS_DomainParticipant *c_participant = NULL;

    c_participant =
        DDS_DomainParticipantFactory_lookup_participant(
                this->_c_domain_part_factory, domainId);


    if (c_participant != NULL)
    {
        result = (DDSDomainParticipant*) DDS_Entity_get_wrapper(
                    DDS_DomainParticipant_as_entity(c_participant));
    }

    return result;
}

DDS_ReturnCode_t
DDSDomainParticipantFactory_impl::set_default_participant_qos(
        const struct DDS_DomainParticipantQos& qos)
{
#ifndef RTI_CERT
    return
        DDS_DomainParticipantFactory_set_default_participant_qos(
                this->_c_domain_part_factory, &qos);
#else
    UNUSED_ARG(qos);
    return DDS_RETCODE_UNSUPPORTED;
#endif /* !RTI_CERT */
}

DDS_ReturnCode_t
DDSDomainParticipantFactory_impl::get_default_participant_qos(
        struct DDS_DomainParticipantQos& qos)
{
#ifndef RTI_CERT
    return
        DDS_DomainParticipantFactory_get_default_participant_qos(
                this->_c_domain_part_factory, &qos);
#else
    UNUSED_ARG(qos);
    return DDS_RETCODE_UNSUPPORTED;
#endif /* !RTI_CERT */
}

DDS_ReturnCode_t
DDSDomainParticipantFactory_impl::get_qos(
        struct DDS_DomainParticipantFactoryQos& qos)
{
    return
        DDS_DomainParticipantFactory_get_qos(
                this->_c_domain_part_factory, &qos);
}

DDS_ReturnCode_t
DDSDomainParticipantFactory_impl::set_qos(
        const struct DDS_DomainParticipantFactoryQos& qos)
{
    return
        DDS_DomainParticipantFactory_set_qos(
                this->_c_domain_part_factory, &qos);
}

RTRegistry*
DDSDomainParticipantFactory_impl::get_registry()
{
    return RTRegistry::get_instance();
}

/* ---------------------------------------------------------------------- */

// --- Constructors & destructors: -------------------------------------

DDSDomainParticipantFactory_impl::DDSDomainParticipantFactory_impl()
{
    this->_c_domain_part_factory = DDS_DomainParticipantFactory_get_instance();

#ifndef RTI_CERT
    OSAPI_TRACE_DDS("RTI Connext Micro C++ library",RTI_FALSE)
    OSAPI_TRACE_INT32("major",RTIME_DDS_VERSION_MAJOR,RTI_FALSE)
    OSAPI_TRACE_INT32("minor",RTIME_DDS_VERSION_MINOR,RTI_FALSE)
    OSAPI_TRACE_INT32("revision",RTIME_DDS_VERSION_REVISION,RTI_FALSE)
#ifndef RTI_CERT
    OSAPI_TRACE_INT32("release",RTIME_DDS_VERSION_RELEASE,RTI_FALSE)
    OSAPI_TRACE_STRING("buildid",DDSCPP_Library_get_version(),RTI_TRUE)
#else
    OSAPI_TRACE_INT32("release",RTIME_DDS_VERSION_RELEASE,RTI_TRUE)
#endif
#endif /* !RTI_CERT */
}

DDSDomainParticipantFactory_impl::~DDSDomainParticipantFactory_impl() 
{
    /* The C DPF is already finalized using
     * DDS_DomainParticipantFactory_finalize_instance by
     * DDSDomainParticipantFactory::finalize_instance */
    this->_c_domain_part_factory = NULL;

    /* Explicitly finalize the RTRegistry singleton instance.
     *
     * RTRegistry is automatically initialized by RTRegistry::get_instance(),
     * and it is always initialized *after* the DPF by construction: users must
     * first access the DPF (thus initializing it) in order to call
     * DPF::get_registry() to access the RTRegistry singleton (because
     * RTRegistry::get_instance() is not public).
     *
     * Once an application calls DPF::finalize_instance() (which then triggers
     * this destructor), we assume we can release RTRegistry too.
     *
     * If the application needs access to DDS services again, both DPF and
     * RTRegistry will be re-initialized by their respective "get_instance" 
     * operations. */
    RTRegistry::delete_instance();
}

/* ---------------------------------------------------------------------- */
