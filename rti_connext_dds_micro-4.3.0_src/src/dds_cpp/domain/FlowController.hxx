/*
 * FILE: DDSFlowController_impl.hxx - DDS FlowController APIs
 *
 * (c) Copyright, Real-Time Innovations, 2018-2018.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 */
#ifndef DDSFlowController_impl_hxx
#define DDSFlowController_impl_hxx

#if DDS_FLOW_CONTROLLER_ENABLED


class DDSFlowController_impl : public DDSFlowController
{
    // --- <<conversion>>: -----------------------------------------------
public:

    DDS_ReturnCode_t
    set_property(const DDS_FlowControllerProperty_t &property);

    DDS_ReturnCode_t
    get_property(DDS_FlowControllerProperty_t &property);

    DDS_ReturnCode_t
    trigger_flow();

    const char*
    get_name();

    DDSDomainParticipant*
    get_participant();

    // --- <<lifecycle>>: ------------------------------------------------
public:

    DDSFlowController_impl(DDS_FlowController *c_flow_controller)
    {
        _c_flow_controller = c_flow_controller;
    }

    ~DDSFlowController_impl() ;

    DDS_FlowController*
    get_c_flow_controller() { return _c_flow_controller; }

    /* This function follows the destructor naming conventions to indicate
     * it is class function to delete an instance.
     */
    static void
    DDSFlowController_impl_delete(void *fc_impl);

private:
    DDS_FlowController* _c_flow_controller;

};

#endif

#endif
