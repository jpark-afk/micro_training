/*
 * FILE: netio_flowcontroller.h - NETIO FlowController
 *
 * (c) Copyright, Real-Time Innovations, 2018-2023.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef netio_flowcontroller_h
#define netio_flowcontroller_h

typedef struct NETIO_FlowControllerProperty
{
    struct RT_ComponentProperty _parent;

    /*ci
     * \brief Internal variables
     */
    const char *name;

    /*ci
     * \brief Internal variables
     */
    const void *owner;
} NETIO_FlowControllerProperty;

#define  NETIO_FlowControllerProperty_INITIALIZER \
{\
    RT_ComponentProperty_INITIALIZER,\
    NULL,\
    NULL\
}

typedef struct NETIO_FlowController
{
    struct RT_Component _parent;

    const char *name;

    const void *owner;
} NETIO_FlowController;

struct NETIO_FlowControllerFactoryProperty
{
    struct RT_ComponentFactoryProperty _parent;
};

struct NETIO_FlowControllerFactory
{
    /*ci
     * \brief Base-class
     */
    struct RT_ComponentFactory _parent;
};

typedef enum
{
    NETIO_FLOW_CONTROLLER_FLOW_STATE_INVALID = 0,
    NETIO_FLOW_CONTROLLER_FLOW_STATE_READY = 1,
    NETIO_FLOW_CONTROLLER_FLOW_STATE_COMPLETE,
    NETIO_FLOW_CONTROLLER_FLOW_STATE_RUNNING,
    NETIO_FLOW_CONTROLLER_FLOW_STATE_RESCHEDULED
} NETIO_FlowControllerFlowState_T;

typedef void* NETIO_FlowControllerFlowHandle;

struct NETIO_FlowProperty
{
    /*ci
     * \brief The relative priority of a flow in a flow control group.
     */
    RTI_INT32 priority;

    /*ci
     * \brief The minimum number of bits required to send any data.
     */
    RTI_INT32 min_bits_required;
};

#define NETIO_FlowProperty_INITIALIZER \
{\
    0,\
    0\
}

typedef void
(*NETIO_FlowController_send_T)(NETIO_FlowController *fc,
                               struct OSAPI_SystemTime *time);

typedef char*
(*NETIO_FlowController_get_name_T)(NETIO_FlowController *fc);

#define NETIO_FlowController_get_name(fc_) \
    ((struct NETIO_FlowController*)fc_)->name

typedef char*
(*NETIO_FlowController_get_owner_T)(NETIO_FlowController *fc);

#define NETIO_FlowController_get_owner(fc_) \
    ((struct NETIO_FlowController*)fc_)->owner

typedef NETIO_FlowControllerFlowState_T
(*NETIO_FlowControllerFlowSendData_T)(void *param,
                                      RTI_INT32 bits_recvd,
                                      RTI_INT32 *bits_used);

typedef void
(*NETIO_FlowControllerFlowFinalize_T)(void *param);


FUNCTION_MUST_TYPEDEF(
NETIO_FlowControllerFlowHandle
(*NETIO_FlowController_add_flow_T)(NETIO_FlowController *fc,
                                   struct NETIO_Guid *netio_guid,
                                   NETIO_FlowControllerFlowSendData_T send_data,
                                   NETIO_FlowControllerFlowFinalize_T flow_finalize,
                                   void *flow_param,
                                   struct NETIO_FlowProperty *property)
)

FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*NETIO_FlowController_remove_flow_T)(NETIO_FlowController *fc,
                                      NETIO_FlowControllerFlowHandle handle)
)

FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*NETIO_FlowController_reschedule_flow_T)(NETIO_FlowController *fc,
                                          NETIO_FlowControllerFlowHandle handle)
)

typedef void
(*NETIO_FlowController_clock_tick_T)(NETIO_FlowController *fc,RTI_INT32 periods);

struct NETIO_FlowControllerI
{
    struct RT_ComponentI _parent;

    /*ci
     * \brief Flow for specified time
     */
    NETIO_FlowController_send_T send;

    /*ci
     * \brief Add a task to this flow-controller
     */
    NETIO_FlowController_add_flow_T add_flow;

    /*ci
     * \brief Remove a task to this flow-controller
     */
    NETIO_FlowController_remove_flow_T remove_flow;

    /*ci
     * \brief Reschedule a task the flow-controller
     */
    NETIO_FlowController_reschedule_flow_T reschedule_flow;

    /*ci
     * \brief Tick the flow-controller for N clock periods
     */
    NETIO_FlowController_clock_tick_T clock_tick;
};

#define NETIO_FlowControllerFactory_create_flowcontroller(f_,p_,l_) \
    (struct NETIO_FlowControllerI*)((f_)->intf)->create_component(f_,p_,l_)

#define NETIO_FlowControllerFactory_delete_flowcontroller(f_,fc_) \
    ((f_)->intf)->delete_component(f_,(RT_Component_T*)fc_)

#define NETIO_FlowController_add_flow(fc_,n_,send_,finalize_,p_,fp_) \
    ((struct NETIO_FlowControllerI*)((fc_)->_parent._intf))->add_flow(\
                                              fc_,n_,send_,finalize_,p_,fp_)

#define NETIO_FlowController_remove_flow(fc_,h_) \
    ((struct NETIO_FlowControllerI*)((fc_)->_parent._intf))->remove_flow(fc_,h_)

#define NETIO_FlowController_reschedule_flow(fc_,h_) \
    ((struct NETIO_FlowControllerI*)((fc_)->_parent._intf))->reschedule_flow(fc_,h_)

#define NETIO_FlowController_clock_tick(fc_,t_) \
    ((struct NETIO_FlowControllerI*)((fc_)->_parent._intf))->clock_tick(fc_,t_)


#define NETIO_FlowController_send(fc_,t_) \
    ((struct NETIO_FlowControllerI*)((fc_)->_parent._intf))->send(fc_,t_)
#endif
