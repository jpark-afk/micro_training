
/*
 * FILE: DiscoveryQueue.c - DDS Discovery Queue
 *
 * Copyright (c) 2025-2025 Real-Time Innovations,Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#include "DiscoveryQueue.h"
#include "DomainParticipant.h"

#ifndef RTI_CERT
#define RTI_MICRO_ONLY(x_) x_
#else
#define RTI_MICRO_ONLY(x_)
#endif

DDS_Boolean
DDS_DiscoveryQueue_initialize(
                    struct DDS_DiscoveryQueue *queue,
                    const struct DDS_DiscoveryQueueProperty *const property)
{
    struct REDA_BufferPoolProperty prop = REDA_BufferPoolProperty_INITIALIZER;

    queue->publication_pool = NULL;
    queue->subscription_pool= NULL;

    REDA_CircularList_init(&queue->publication_queue);
    REDA_CircularList_init(&queue->subscription_queue);

    if (property->max_publication_queue > 0)
    {
        prop.buffer_size = (RTI_SIZE_T)sizeof(struct DDS_DiscoveryQueueNode);
        prop.max_buffers = (RTI_SIZE_T)property->max_publication_queue;
        queue->publication_pool = REDA_BufferPool_new("discovery_pub_queue",
                                                    &prop,NULL,NULL,NULL,NULL);
        if (queue->publication_pool == NULL)
        {
            return DDS_BOOLEAN_FALSE;
        }
    }

    if (property->max_subscription_queue > 0)
    {
        prop.buffer_size = (RTI_SIZE_T)sizeof(struct DDS_DiscoveryQueueNode);
        prop.max_buffers = (RTI_SIZE_T)property->max_subscription_queue;
        queue->subscription_pool = REDA_BufferPool_new("discovery_sub_queue",
                                                    &prop,NULL,NULL,NULL,NULL);
        if (queue->subscription_pool == NULL)
        {
            DDS_DiscoveryQueue_finalize(queue);
            return DDS_BOOLEAN_FALSE;
        }
    }

    return DDS_BOOLEAN_TRUE;
}

DDS_Boolean
DDS_DiscoveryQueue_finalize(struct DDS_DiscoveryQueue *queue)
{
    if (queue->publication_pool != NULL)
    {
        RTI_MICRO_ONLY
        (
            if (!REDA_BufferPool_delete(queue->publication_pool))
            {
                return DDS_BOOLEAN_FALSE;
            }
        )
        queue->publication_pool = NULL;
    }

    if (queue->subscription_pool != NULL)
    {
        RTI_MICRO_ONLY
        (
            if (!REDA_BufferPool_delete(queue->subscription_pool))
            {
                return DDS_BOOLEAN_FALSE;
            }
        )
        queue->subscription_pool = NULL;
    }
    return DDS_BOOLEAN_TRUE;
}

DDS_Boolean
DDS_DiscoveryQueue_is_publication_empty(
            struct DDS_DiscoveryQueue *queue)
{
    return  REDA_CircularList_is_empty(&queue->publication_queue) ?
            DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

DDS_Boolean
DDS_DiscoveryQueue_is_subscription_empty(
            struct DDS_DiscoveryQueue *queue)
{
    return REDA_CircularList_is_empty(&queue->subscription_queue) ?
            DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

RTI_PRIVATE DDS_ReturnCode_t
DDS_DiscoveryQueue_queue_data(REDA_CircularList_T *queue,
                              REDA_BufferPool_T pool,
                              void *data,
                              struct DDS_SampleInfo *info)
{
    struct DDS_DiscoveryQueueNode *qn;

    if (pool == NULL)
    {
        return DDS_RETCODE_NOT_ENABLED;
    }

    qn = REDA_BufferPool_get_buffer(pool);
    if (qn == NULL)
    {
        return DDS_RETCODE_OUT_OF_RESOURCES;
    }
    qn->data = data;
    qn->info = info;
    REDA_CircularList_append(queue,&qn->_parent);

    return DDS_RETCODE_OK;
}

DDS_Boolean
DDS_DiscoveryQueue_is_publication_queue_enabled(struct DDS_DiscoveryQueue *queue)
{
    return queue->publication_pool != NULL ?
            DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

DDS_Boolean
DDS_DiscoveryQueue_is_subscription_queue_enabled(struct DDS_DiscoveryQueue *queue)
{
    return queue->subscription_pool != NULL ?
            DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

DDS_ReturnCode_t
DDS_DiscoveryQueue_queue_publication(
                    struct DDS_DiscoveryQueue *queue,
                    struct DDS_PublicationBuiltinTopicData *data,
                    struct DDS_SampleInfo *info)
{
    return DDS_DiscoveryQueue_queue_data(
                 &queue->publication_queue,queue->publication_pool,data,info);
}

DDS_ReturnCode_t
DDS_DiscoveryQueue_queue_subscription(
                    struct DDS_DiscoveryQueue *queue,
                    struct DDS_SubscriptionBuiltinTopicData *data,
                    struct DDS_SampleInfo *info)
{
    return DDS_DiscoveryQueue_queue_data(
                 &queue->subscription_queue,queue->subscription_pool,data,info);
}

void
DDS_DiscoveryQueue_assert_queued_publication(
                                DDS_DomainParticipant *const participant,
                                struct DDS_DiscoveryQueue *queue)
{
    struct DDS_DiscoveryQueueNode *qn;
    DDS_ReturnCode_t retcode;

    if (REDA_CircularList_is_empty(&queue->publication_queue))
    {
        /* Nothing to, considered success */
        return;
    }

    qn = (struct DDS_DiscoveryQueueNode*)
                REDA_CircularList_get_first(&queue->publication_queue);

    REDA_CircularList_unlink_node(&qn->_parent);

   retcode = NDDS_DomainParticipant_assert_remote_publication(
                        participant,NULL,qn->data,NDDS_TYPEPLUGIN_GUID_KEY);

    if (retcode == DDS_RETCODE_OUT_OF_RESOURCES)
    {
        REDA_CircularList_prepend(&queue->publication_queue,&qn->_parent);
    }
    else
    {
        /* Either OK or anohter error. In both cases the sample loan is
         * returned.
         */
        if (retcode != DDS_RETCODE_OK)
        {
            DDSC_LOG_QUEUE_PUBLICATION_DISCOVERY(OSAPI_LOGKIND_ERROR)
        }
        if ((participant->disc_plugin != NULL)
            && NDDS_Discovery_Plugin_has_on_publication_data_return_loan(
                        participant->disc_plugin))
        {
            NDDS_Discovery_Plugin_on_publication_data_return_loan(
                    participant->disc_plugin,participant,qn->data,qn->info);
        }
        REDA_BufferPool_return_buffer(queue->publication_pool,qn);
    }
}

void
DDS_DiscoveryQueue_assert_queued_subscription(
                                DDS_DomainParticipant *const participant,
                                struct DDS_DiscoveryQueue *queue)
{
    struct DDS_DiscoveryQueueNode *qn;
    DDS_ReturnCode_t retcode;

    if (REDA_CircularList_is_empty(&queue->subscription_queue))
    {
        /* Nothing to do, considered success */
        return;
    }

    qn = (struct DDS_DiscoveryQueueNode*)
                REDA_CircularList_get_first(&queue->subscription_queue);
    REDA_CircularList_unlink_node(&qn->_parent);

   retcode = NDDS_DomainParticipant_assert_remote_subscription(
                        participant,NULL,qn->data,NDDS_TYPEPLUGIN_GUID_KEY);

    if (retcode == DDS_RETCODE_OUT_OF_RESOURCES)
    {
            REDA_CircularList_prepend(&queue->subscription_queue,&qn->_parent);
    }
    else
    {
        /* Either OK or anohter error. In both cases the sample loan is
         * returned.
         */

        if (retcode != DDS_RETCODE_OK)
        {
            DDSC_LOG_QUEUE_SUBSCRIPTION_DISCOVERY(OSAPI_LOGKIND_ERROR)
        }

        if ((participant->disc_plugin != NULL)
            && NDDS_Discovery_Plugin_has_on_subscription_data_return_loan(
                        participant->disc_plugin))
        {
            NDDS_Discovery_Plugin_on_subscription_data_return_loan(
                    participant->disc_plugin,participant,qn->data,qn->info);
        }
        REDA_BufferPool_return_buffer(queue->subscription_pool,qn);
    }
}

RTI_PRIVATE void
DDS_DiscoveryQueue_purge_publication_by_prefix(
                    DDS_DomainParticipant *const participant,
                    struct DDS_DiscoveryQueue *queue,
                    DDS_BuiltinTopicKey_t *key)
{
    struct DDS_DiscoveryQueueNode *qn,*qn_next;
    struct DDS_PublicationBuiltinTopicData *pub_data;

    qn = (struct DDS_DiscoveryQueueNode*)
                REDA_CircularList_get_first(&queue->publication_queue);

    if (qn == NULL)
    {
        /* will only happen if the queue is not initialized */
        return;
    }

    while (!REDA_CircularList_node_at_head(&queue->publication_queue,
                                           &qn->_parent))
    {
        qn_next = (struct DDS_DiscoveryQueueNode*)
                               REDA_CircularListNode_get_next(&qn->_parent);
        pub_data =  (struct DDS_PublicationBuiltinTopicData*)qn->data;

        if ((key == NULL)
            || DDS_BuiltinTopicKey_prefix_equals(key,&pub_data->participant_key))
        {
            REDA_CircularList_unlink_node(&qn->_parent);
            if ((participant->disc_plugin != NULL)
                && NDDS_Discovery_Plugin_has_on_publication_data_return_loan(
                            participant->disc_plugin))
            {
                NDDS_Discovery_Plugin_on_publication_data_return_loan(
                        participant->disc_plugin,participant,pub_data,qn->info);
            }
            REDA_BufferPool_return_buffer(queue->publication_pool,qn);
        }
        qn = qn_next;
    }
}

RTI_PRIVATE void
DDS_DiscoveryQueue_purge_subscription_by_prefix(
                    DDS_DomainParticipant *const participant,
                    struct DDS_DiscoveryQueue *queue,
                    DDS_BuiltinTopicKey_t *key)
{
    struct DDS_DiscoveryQueueNode *qn,*qn_next;
    struct DDS_SubscriptionBuiltinTopicData *sub_data;

    qn = (struct DDS_DiscoveryQueueNode*)
                REDA_CircularList_get_first(&queue->subscription_queue);

    if (qn == NULL)
    {
        /* will only happen if the queue is not initialized */
        return;
    }

    while (!REDA_CircularList_node_at_head(&queue->subscription_queue,
                                           &qn->_parent))
    {
        qn_next = (struct DDS_DiscoveryQueueNode*)
                    REDA_CircularListNode_get_next(&qn->_parent);
        sub_data =  (struct DDS_SubscriptionBuiltinTopicData*)qn->data;

        if ((key == NULL)
            || DDS_BuiltinTopicKey_prefix_equals(key,&sub_data->participant_key))
        {
            REDA_CircularList_unlink_node(&qn->_parent);
            if ((participant->disc_plugin != NULL)
                && NDDS_Discovery_Plugin_has_on_subscription_data_return_loan(
                            participant->disc_plugin))
            {
                NDDS_Discovery_Plugin_on_subscription_data_return_loan(
                    participant->disc_plugin,participant,sub_data,qn->info);
            }
            REDA_BufferPool_return_buffer(queue->subscription_pool,qn);
        }
        qn = qn_next;
    }
}

void
DDS_DiscoveryQueue_purge_by_prefix(
                    DDS_DomainParticipant *const participant,
                    struct DDS_DiscoveryQueue *queue,
                    DDS_BuiltinTopicKey_t *key)
{
    DDS_DiscoveryQueue_purge_publication_by_prefix(participant,queue,key);
    DDS_DiscoveryQueue_purge_subscription_by_prefix(participant,queue,key);
}
