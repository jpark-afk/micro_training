/*
 * FILE: FlowController.h - Internal FlowController APIs
 *
 * (c) Copyright, Real-Time Innovations, 2018-2024.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef FlowController_h
#define FlowController_h

#include "dds_c/dds_c_config.h"

#if DDS_FLOW_CONTROLLER_ENABLED
#ifndef osapi_task_h
#include "osapi/osapi_task.h"
#endif
#ifndef netio_address_h
#include "netio/netio_address.h"
#endif
#ifndef netio_route_h
#include "netio/netio_route.h"
#endif
#ifndef netio_loopback_h
#include "netio/netio_loopback.h"
#endif
#ifndef netio_rtps_h
#include "netio/netio_rtps.h"
#endif
#ifndef reda_sequence_h
#include "reda/reda_sequence.h"
#endif
#ifndef reda_string_h
#include "reda/reda_string.h"
#endif
#ifndef reda_indexer_h
#include "reda/reda_indexer.h"
#endif
#ifndef reda_bufferpool_h
#include "reda/reda_bufferpool.h"
#endif
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_discovery_plugin_h
#include "dds_c/dds_c_discovery_plugin.h"
#endif
#include "dds_c/dds_c_flowcontroller.h"

/*ci
 * \brief The DDS FlowController base-class based on the FlowController
 *        in Connext Pro
 */
struct DDS_FlowController
{
    /*ci
     * \brief base-class
     */
    struct NETIO_FlowController _parent;

    /*ci
     * \brief Flow controller properties
     */
    struct DDS_FlowControllerProperty_t property;

    /*ci
     * \brief Pointer to another object wrapping this, normally a language
     *        binding object;
     */
    void *wrapper;

    /*ci
     * \brief Reference count for the number of DataWriters using this flow controller.
     * This is used to determine when to delete the flow controller.
     */
    RTI_UINT32 ref_count;
};

/*ci
 * \brief Structure to maintain DDS Flow Control information.
 */
struct DDS_FlowControl
{
    /*ci
     * \brief index of named flow-controllers
     */
    struct REDA_Indexer *index;

    /*ci
     * \brief The default flow-controller (leaky bucket)
     */
    struct RT_ComponentFactory *default_factory;

    /*ci
     * \brief Default flow-controller properties for the leaky bucket
     */
    struct DDS_FlowControllerProperty_t default_property;

    /*ci
     * \brief
     */
    OSAPI_TaskScheduler_T task_scheduler;

    /*ci
     * \brief Mutex for the task scheduler
     */
    struct OSAPI_Mutex *sched_lock;

    /*ci
     * \brief Finalizer method, typically used by language bindings
     */
    DDS_FlowController_Finalizer_T wrapper_finalizer;
};

#define DDS_FlowControl_INITIALIZER \
{\
    NULL,\
    NULL,\
    DDS_FlowControllerProperty_t_INITIALIZER,\
    OSAPI_TaskScheduler_INITIALIZER,\
    NULL\
}

extern void
DDS_FlowControl_initialize(struct DDS_FlowControl *fc,
                           struct DDS_DomainParticipantImpl *self);

DDS_Boolean
DDS_FlowControl_assert(struct DDS_FlowControl *self,
                       struct DDS_DomainParticipantImpl *dp);

#ifndef RTI_CERT
extern DDS_Boolean
DDS_FlowControl_finalize(struct DDS_FlowControl *fc,
                         DDS_Boolean is_delete_contained);
#endif

extern DDS_Boolean
DDS_FlowController_initialize(struct DDS_FlowController *fc,
                              struct NETIO_FlowControllerI *intf,
                              struct DDS_FlowControllerProperty_t *prop);

#ifndef RTI_CERT
extern void
DDS_FlowController_finalize(struct DDS_FlowController *fc);
#endif

#endif /* DDS_FLOW_CONTROLLER_ENABLED */

#endif /* FlowController_h */
