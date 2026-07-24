/*
 * FILE: DiscoveryQueue.h - DDS Discovery Queue
 *
 * Copyright (c) 2025-2025 Real-Time Innovations,Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef DiscoveryQueue_h
#define DiscoveryQueue_h

#include "reda/reda_bufferpool.h"

#include "dds_c/dds_c_config.h"
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#include "dds_c/dds_c_discovery.h"

#ifdef __cplusplus
extern "C"
{
#endif

struct DDS_DiscoveryQueueNode
{
    struct REDA_CircularListNode _parent;

    /*ci \brief queued sample
     */
    void *data;

    struct DDS_SampleInfo *info;
};

struct DDS_DiscoveryQueueProperty
{
    /*ci \brief max publication queue size
     */
    DDS_Long max_publication_queue;

    /*ci \brief max subscription queue size
     */
    DDS_Long max_subscription_queue;
};

#define DDS_DiscoveryQueueProperty_INITIALIZER \
{\
    0,\
    0\
}

struct DDS_DiscoveryQueue
{
    /*ci buffer-pool for publications
     */
    REDA_BufferPool_T publication_pool;

    /*ci buffer-pool for subscriptions
     */
    REDA_BufferPool_T subscription_pool;

    /*ci queue of publication samples
     */
    REDA_CircularList_T publication_queue;

    /*ci queue of subscription samples
     */
    REDA_CircularList_T subscription_queue;
};

extern DDS_Boolean
DDS_DiscoveryQueue_initialize(
                    struct DDS_DiscoveryQueue *queue,
                    const struct DDS_DiscoveryQueueProperty *const property);

extern DDS_Boolean
DDS_DiscoveryQueue_finalize(struct DDS_DiscoveryQueue *queue);

extern DDS_Boolean
DDS_DiscoveryQueue_is_publication_empty(
            struct DDS_DiscoveryQueue *queue);

extern DDS_Boolean
DDS_DiscoveryQueue_is_subscription_empty(
            struct DDS_DiscoveryQueue *queue);

extern DDS_Boolean
DDS_DiscoveryQueue_is_publication_queue_enabled(struct DDS_DiscoveryQueue *queue);

extern DDS_Boolean
DDS_DiscoveryQueue_is_subscription_queue_enabled(struct DDS_DiscoveryQueue *queue);

extern DDS_ReturnCode_t
DDS_DiscoveryQueue_queue_publication(
                    struct DDS_DiscoveryQueue *queue,
                    struct DDS_PublicationBuiltinTopicData *data,
                    struct DDS_SampleInfo *info);

extern DDS_ReturnCode_t
DDS_DiscoveryQueue_queue_subscription(
                    struct DDS_DiscoveryQueue *queue,
                    struct DDS_SubscriptionBuiltinTopicData *data,
                    struct DDS_SampleInfo *info);

extern void
DDS_DiscoveryQueue_assert_queued_publication(
                                DDS_DomainParticipant *const participant,
                                struct DDS_DiscoveryQueue *queue);

extern void
DDS_DiscoveryQueue_assert_queued_subscription(
                                DDS_DomainParticipant *const participant,
                                struct DDS_DiscoveryQueue *queue);

extern void
DDS_DiscoveryQueue_purge_by_prefix(
                    DDS_DomainParticipant *const participant,
                    struct DDS_DiscoveryQueue *queue,
                    DDS_BuiltinTopicKey_t *key);

#ifdef __cplusplus
}
#endif

#endif
