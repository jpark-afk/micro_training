/*
 * (c) Copyright, Real-Time Innovations, 2018-2018
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*e
 * \addtogroup DDSFlowControllerModule
 * \ingroup DDSPublicationModule
 */
#if DDS_FLOW_CONTROLLER_ENABLED

#ifndef dds_cpp_flowcontroller_hxx
#define dds_cpp_flowcontroller_hxx

#ifndef dds_cpp_dll_hxx
  #include "dds_cpp/dds_cpp_dll.hxx"
#endif
#ifndef dds_cpp_infrastructure_hxx
  #include "dds_cpp/dds_cpp_infrastructure.hxx"
#endif
#ifndef dds_cpp_topic_hxx
  #include "dds_cpp/dds_cpp_topic.hxx"
#endif
#ifndef dds_cpp_publication_hxx
  #include "dds_cpp/dds_cpp_publication.hxx"
#endif
#ifndef dds_cpp_subscription_hxx
  #include "dds_cpp/dds_cpp_subscription.hxx"
#endif
#ifndef dds_cpp_rt_hxx
#include "dds_cpp/dds_cpp_rt.hxx"
#endif
#ifndef dds_c_domain_h
  #include "dds_c/dds_c_domain.h"
#endif

class DDSFlowController_impl;
class DDSDomainParticipant;
class DDSDomainParticipant_impl;

/*e \dref_FlowController
 */
class DDSCPPDllExport DDSFlowController
{
  public:

    /*e \dref_FlowController_set_property
     */
    virtual DDS_ReturnCode_t set_property(
        const struct DDS_FlowControllerProperty_t &prop) = 0;

    /*e \dref_FlowController_get_property
     */
    virtual DDS_ReturnCode_t get_property(
        struct DDS_FlowControllerProperty_t &prop) = 0;

    /*e \dref_FlowController_trigger_flow
     */
    virtual DDS_ReturnCode_t trigger_flow() = 0;

    /*e \dref_FlowController_get_name
     */
    virtual const char* get_name() = 0;

    /*e \dref_FlowController_get_participant
     */
    virtual DDSDomainParticipant* get_participant() = 0;

  protected:
    virtual ~DDSFlowController();
};

#endif

#endif
