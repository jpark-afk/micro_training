/*
 * FILE: dds_c_flowcontroller.h - DDS FlowController APIs
 *
 * (c) Copyright, Real-Time Innovations, 2018-2024.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 */
/*e
 * \file
 * \brief DDS Flow Controller
 */

/*e
 * \addtogroup DDSFlowControllerModule
 * \ingroup DDSPublicationModule
 */
#ifndef dds_c_flowcontroller_h
#define dds_c_flowcontroller_h

#include "netio/netio_flowcontroller.h"

#include "dds_c/dds_c_common.h"
#include "dds_c/dds_c_infrastructure.h"

#ifdef __cplusplus
extern "C" {
#endif

struct DDS_FlowController;

/*ce
 * \dref_FlowController
 */
typedef struct DDS_FlowController DDS_FlowController;

/*e \dref_DEFAULT_FLOW_CONTROLLER_NAME
 */
extern DDSCDllVariable const char *const DDS_DEFAULT_FLOW_CONTROLLER_NAME;

/*e \dref_FIXED_RATE_FLOW_CONTROLLER_NAME
 */
extern DDSCDllVariable const char *const DDS_FIXED_RATE_FLOW_CONTROLLER_NAME;

/*e \dref_ON_DEMAND_FLOW_CONTROLLER_NAME;
 */
extern DDSCDllVariable const char *const DDS_ON_DEMAND_FLOW_CONTROLLER_NAME;

#define DDS_DOMAINPARTICIPANTRESOURCELIMITSQSPOLICY_FLOW_CONTROLLER_ALLOCATION (32L)

/*e \dref_FlowControllerSchedulingPolicy
 */
typedef enum
{
    /*e \dref_FlowControllerSchedulingPolicy_RR_FLOW_CONTROLLER_SCHED_POLICY
     */
    DDS_RR_FLOW_CONTROLLER_SCHED_POLICY,

    /*e \dref_FlowControllerSchedulingPolicy_EDF_FLOW_CONTROLLER_SCHED_POLICY
     */
    DDS_EDF_FLOW_CONTROLLER_SCHED_POLICY,

    /*e \dref_FlowControllerSchedulingPolicy_HPF_FLOW_CONTROLLER_SCHED_POLICY
     */
    DDS_HPF_FLOW_CONTROLLER_SCHED_POLICY
} DDS_FlowControllerSchedulingPolicy;

/*e \dref_FlowControllerTokenBucketProperty_t
 */
struct DDSCPPDllExport DDS_FlowControllerTokenBucketProperty_t
{
    /*e \dref_FlowControllerTokenBucketProperty_t_max_tokens
     */
    DDS_Long max_tokens;

    /*e \dref_FlowControllerTokenBucketProperty_t_tokens_added_per_period
     */
    DDS_Long tokens_added_per_period;

    /*e \dref_FlowControllerTokenBucketProperty_t_tokens_leaked_per_period
     */
    DDS_Long tokens_leaked_per_period;

    /*e \dref_FlowControllerTokenBucketProperty_t_period
     */
    struct DDS_Duration_t period;

    /*e \dref_FlowControllerTokenBucketProperty_t_bytes_per_token
     */
    DDS_Long bytes_per_token;
};

/*e \dref_FlowControllerProperty_t
 */
struct DDSCPPDllExport DDS_FlowControllerProperty_t
{
    struct NETIO_FlowControllerProperty _parent;

    /*e \dref_FlowControllerProperty_t_scheduling_policy
     */
    DDS_FlowControllerSchedulingPolicy scheduling_policy;

    /*e \dref_FlowControllerProperty_t_token_bucket
     */
    struct DDS_FlowControllerTokenBucketProperty_t token_bucket;

    DDS_Boolean is_vendor_specific;

    DDSC_CPP_QOS_METHODS(DDS_FlowControllerProperty_t)
};

#define DDS_FlowControllerTokenBucketProperty_t_INITIALIZER \
{  \
    -1L,  \
    -1L,  \
    0,  \
    { 1, 0 }, \
    -1L,\
}

/* Note: Connext Pro uses DDS_EDF_FLOW_CONTROLLER_SCHED_POLICY as the
 * default. However, Micro does not support the latency budget and in this
 * case DDS_EDF_FLOW_CONTROLLER_SCHED_POLICY is effectively round-robin.
 */
#define DDS_FlowControllerProperty_t_INITIALIZER   \
{ \
    NETIO_FlowControllerProperty_INITIALIZER,\
    DDS_EDF_FLOW_CONTROLLER_SCHED_POLICY,\
    DDS_FlowControllerTokenBucketProperty_t_INITIALIZER,\
    DDS_BOOLEAN_FALSE \
}

DDSCDllExport void
DDS_FlowControllerProperty_t_initialize(struct DDS_FlowControllerProperty_t* out);

DDSCDllExport RTI_INT32
DDS_FlowControllerProperty_t_is_equal(const struct DDS_FlowControllerProperty_t *left,
                                      const struct DDS_FlowControllerProperty_t *right);

DDSCDllExport DDS_ReturnCode_t
DDS_FlowControllerProperty_copy(struct DDS_FlowControllerProperty_t* out,
                                const struct DDS_FlowControllerProperty_t* in);

/*ce \dref_FlowController_get_name
 */
DDSCDllExport const char*
DDS_FlowController_get_name(DDS_FlowController* self);

/*ce \dref_FlowController_get_participant
 */
DDSCDllExport DDS_DomainParticipant*
DDS_FlowController_get_participant(DDS_FlowController* self);

/*ce \dref_FlowController_set_property
 */
DDSCDllExport DDS_ReturnCode_t
DDS_FlowController_set_property(DDS_FlowController *self,
                            const struct DDS_FlowControllerProperty_t *prop);

/*ce \dref_FlowController_get_property
 */
DDSCDllExport DDS_ReturnCode_t
DDS_FlowController_get_property(DDS_FlowController *self,
                                struct DDS_FlowControllerProperty_t *prop);

/*ce \dref_FlowController_trigger_flow
 */
DDSCDllExport DDS_ReturnCode_t
DDS_FlowController_trigger_flow(DDS_FlowController *self);

DDSCDllExport void
DDS_FlowController_set_wrapper(struct DDS_FlowController *fc,
                               void *wrapper);

DDSCDllExport void*
DDS_FlowController_get_wrapper(struct DDS_FlowController *fc);

/*ce \dref_DomainParticipant_get_default_flowcontroller_property
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_get_default_flowcontroller_property(
                                    DDS_DomainParticipant *self,
                                    struct DDS_FlowControllerProperty_t *prop);

/*ce \dref_DomainParticipant_set_default_flowcontroller_property
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_set_default_flowcontroller_property(
                            DDS_DomainParticipant *self,
                            const struct DDS_FlowControllerProperty_t *prop);

/*e \dref_FLOW_CONTROLLER_PROPERTY_DEFAULT
 */
extern DDSCDllVariable const struct DDS_FlowControllerProperty_t
                                        DDS_FLOW_CONTROLLER_PROPERTY_DEFAULT;

/*ce \dref_DomainParticipant_create_flowcontroller
 */
DDSCDllExport DDS_FlowController*
DDS_DomainParticipant_create_flowcontroller(DDS_DomainParticipant *self,
                            const char *name,
                            const struct DDS_FlowControllerProperty_t *prop);

/*ce \dref_DomainParticipant_delete_flowcontroller
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_delete_flowcontroller(DDS_DomainParticipant *self,
                                            DDS_FlowController *fc);

/*ce \dref_DomainParticipant_lookup_flowcontroller
 */
DDSCDllExport DDS_FlowController*
DDS_DomainParticipant_lookup_flowcontroller(DDS_DomainParticipant *self,
                                            const char *name);

typedef DDS_FlowController*
(*DDS_DomainParticipant_lookup_flowcontroller_T)(DDS_DomainParticipant *self,
                                            const char *name);

typedef void
(*DDS_FlowController_Finalizer_T)(void *);

DDSCDllExport void
DDS_DomainParticipant_set_flowcontroller_finalizer(DDS_DomainParticipant *self,
                                   DDS_FlowController_Finalizer_T finalizer);

/* FlowController API NETIO level */

#ifdef __cplusplus
    }   /* extern "C" */
#endif

#endif /* dds_c_flowcontroller_h */
