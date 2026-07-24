/*
 * FILE: FlowControllerBuiltin.h - The builtin leaky-bucket FlowController
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
#ifndef FlowControllerLeakyBucket_h
#define FlowControllerLeakyBucket_h


#include "FlowControl.h"

#if DDS_FLOW_CONTROLLER_ENABLED

#define DDS_BITS_PER_BYTE (8)

struct DDS_LeakyBucketFlowController;


struct DDS_LeakyBucketFlowControllerTask
{
    REDA_CircularListNode_T _parent;

    RTI_INT32 min_bits_per_run;

    NETIO_FlowControllerFlowSendData_T task_entry;

    NETIO_FlowControllerFlowFinalize_T task_finalize;

    void *task_param;

    struct DDS_LeakyBucketFlowController *fc;

    RTI_INT32 priority;

    struct DDS_Duration_t max_latency;

    RTI_INT32 group_count;

    volatile NETIO_FlowControllerFlowState_T state;

    RTI_BOOL is_deleted;
};

#define DDS_LeakyBucketFlowControllerTask_INITIALIZER \
{\
    {NULL,NULL},\
    0,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    0,\
    DDS_DURATION_INFINITE_INITIALIZER,\
    0,\
    NETIO_FLOW_CONTROLLER_FLOW_STATE_INVALID,\
    RTI_FALSE\
}

typedef RTI_INT32
(*DDS_LeakyBucketFlowController_compare_task)(
                const struct DDS_LeakyBucketFlowControllerTask *const ltask,
                const struct DDS_LeakyBucketFlowControllerTask *const rtask);

struct DDS_LeakyBucketFlowController
{
    struct DDS_FlowController _parent;

    REDA_CircularList_T tasks;

    REDA_CircularList_T run_list;

    REDA_BufferPool_T task_pool;

    RTI_INT32 accumulated_bits;

    RTI_INT32 added_bits_per_period;

    RTI_INT32 leaked_bits_per_period;

    RTI_INT32 bits_per_max_token;

    struct OSAPI_Task task;

    DDS_LeakyBucketFlowController_compare_task task_cmp;

    struct DDS_LeakyBucketFlowControllerTask *next_task;

    OSAPI_Mutex_T *flow_lock;
};

extern DDS_ReturnCode_t
DDS_LeakyBucketFlowControllerFactory_register(RT_Registry_T *registry,
                                              const char* const name);

extern DDS_ReturnCode_t
DDS_LeakyBucketFlowControllerFactory_unregister(RT_Registry_T *registry,
                                                const char* const name);

#endif

#endif

