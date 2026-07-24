/*
 * FILE: FlowController.c - FlowController Implementation
 *
 * (c) Copyright, Real-Time Innovations, 2018-2018.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef dds_cpp_dll_hxx
#include "dds_cpp/dds_cpp_dll.hxx"
#endif
#ifndef dds_cpp_flowcontroller_hxx
#include "dds_cpp/dds_cpp_flowcontroller.hxx"
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

#include "DomainParticipant.hxx"

#if DDS_FLOW_CONTROLLER_ENABLED

#include "FlowController.hxx"

/*** SOURCE_BEGIN */
DDS_FlowControllerProperty_t::DDS_FlowControllerProperty_t()
{
    DDS_FlowControllerProperty_t_initialize(this);
}

DDS_FlowControllerProperty_t::~DDS_FlowControllerProperty_t()
{
}

bool
DDS_FlowControllerProperty_t::operator==(const DDS_FlowControllerProperty_t& other)
{
    return DDS_FlowControllerProperty_t_is_equal(this,&other) ? true : false;
}

bool
DDS_FlowControllerProperty_t::operator!=(const DDS_FlowControllerProperty_t& other)
{
    return !DDS_FlowControllerProperty_t_is_equal(this,&other) ? true : false;
}

/*e \dref_DomainParticipant_get_default_flowcontroller_property
 */
DDS_ReturnCode_t
DDSDomainParticipant_impl::get_default_flowcontroller_property(
                                    DDS_FlowControllerProperty_t &prop)
{
    return DDS_DomainParticipant_get_default_flowcontroller_property(
                                    (DDS_DomainParticipant*)this->_c_entity,
                                    &prop);
}

/*e \dref_DomainParticipant_set_default_flowcontroller_property
 */
DDS_ReturnCode_t
DDSDomainParticipant_impl::set_default_flowcontroller_property(
                                    const DDS_FlowControllerProperty_t &prop)
{
    return DDS_DomainParticipant_set_default_flowcontroller_property(
                                    (DDS_DomainParticipant*)this->_c_entity,
                                    &prop);
}

/*e \dref_DomainParticipant_create_flowcontroller
 */
DDSFlowController*
DDSDomainParticipant_impl::create_flowcontroller(const char *name,
                                    const DDS_FlowControllerProperty_t &prop)
{
    DDS_FlowController *c_fc;
    DDSFlowController *retval = NULL;

    c_fc = DDS_DomainParticipant_create_flowcontroller(
                                (DDS_DomainParticipant*)this->_c_entity,
                                name,&prop);

    if (c_fc == NULL)
    {
        return NULL;
    }

    DDSFlowController_impl *fc_impl = new DDSFlowController_impl(c_fc);

    if (fc_impl == NULL)
    {
        DDS_DomainParticipant_delete_flowcontroller(
                                (DDS_DomainParticipant*)this->_c_entity,c_fc);
        return NULL;
    }

    DDS_FlowController_set_wrapper(c_fc,(void*)fc_impl);

    retval = static_cast<DDSFlowController*>(fc_impl);

    return retval;
}

/*e \dref_DomainParticipant_delete_flowcontroller
 */
DDS_ReturnCode_t
DDSDomainParticipant_impl::delete_flowcontroller(DDSFlowController *fc)
{
    DDSFlowController_impl *fc_impl = (DDSFlowController_impl *)fc;
    DDS_ReturnCode_t retcode;

    retcode = DDS_DomainParticipant_delete_flowcontroller(
                                (DDS_DomainParticipant*)this->_c_entity,
                                fc_impl->get_c_flow_controller());

    return retcode;
}

/*e \dref_DomainParticipant_lookup_flowcontroller
 */
DDSFlowController*
DDSDomainParticipant_impl::lookup_flowcontroller(const char *name)
{
    DDSFlowController *retval = NULL;
    DDS_FlowController *c_fc = NULL;

    c_fc = DDS_DomainParticipant_lookup_flowcontroller(
                                (DDS_DomainParticipant*)this->_c_entity,
                                name);

    if (c_fc == NULL)
    {
        return NULL;
    }

    DDSFlowController_impl *fc_impl = (DDSFlowController_impl*)
                                         DDS_FlowController_get_wrapper(c_fc);

    retval = static_cast<DDSFlowController*>(fc_impl);

    return retval;
}

DDS_ReturnCode_t
DDSFlowController_impl::set_property(const DDS_FlowControllerProperty_t &property)
{

    return DDS_FlowController_set_property(this->_c_flow_controller,
                                           &property);
}

DDS_ReturnCode_t
DDSFlowController_impl::get_property(DDS_FlowControllerProperty_t &property)
{
    return DDS_FlowController_get_property(this->_c_flow_controller,
                                           &property);
}

DDS_ReturnCode_t
DDSFlowController_impl::trigger_flow()
{
    return DDS_FlowController_trigger_flow(this->_c_flow_controller);
}

const char*
DDSFlowController_impl::get_name()
{
    return DDS_FlowController_get_name(this->_c_flow_controller);
}

DDSDomainParticipant*
DDSFlowController_impl::get_participant()
{
    DDS_DomainParticipant *c_participant = NULL;
    DDSDomainParticipant_impl *participant_impl = NULL;

    c_participant = DDS_FlowController_get_participant(
                                this->_c_flow_controller);

    participant_impl = (DDSDomainParticipant_impl*)
                                    DDS_Entity_get_wrapper(
                                            DDS_DomainParticipant_as_entity(
                                                                c_participant));

    return static_cast<DDSDomainParticipant*>(participant_impl);
}

void
DDSFlowController_impl::DDSFlowController_impl_delete(void *object)
{
    DDSFlowController_impl *fc_impl = (DDSFlowController_impl *)object;

    delete fc_impl;
}

DDSFlowController_impl::~DDSFlowController_impl()
{

}

DDSFlowController::~DDSFlowController()
{

}

#endif /* DDS_FLOW_CONTROLLER_ENABLED */
