/*
 (c) Copyright, Real-Time Innovations, 2009-2015.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.
*/

#ifndef ThroughputQos_h
#include "ThroughputQos.h"
#endif
#ifndef ThroughputCommon_h
#include "ThroughputCommon.h"
#endif
#ifndef PerformanceUdpTransform_h
#include "PerformanceUdpTransform.h"
#endif
#include "wh_sm/wh_sm_history.h"
#include "rh_sm/rh_sm_history.h"

void
DataWriterQos_setImpatientReliable(struct DDS_DataWriterQos *qos,
                                   struct DDS_Duration_t
                                   time_to_go_from_full_to_low_water_mark_ms)
{
    /* assert(block_for_this_ms > alert_reader_within_this_ms); */
    /* assert(send_period_ms > 2 * block_for_this_ms); */
    /* only use for assertion */

    qos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;

    qos->history.kind = DDS_KEEP_LAST_HISTORY_QOS;
    qos->history.depth = 4;

    /* limit the queue, because we don't need to use the queue */
    qos->resource_limits.max_samples = 4;

    /* use these hard coded value until you use key */
    qos->resource_limits.max_samples_per_instance =
        qos->resource_limits.max_samples;
    qos->resource_limits.max_instances = 1;
    qos->protocol.rtps_reliable_writer.heartbeats_per_max_samples = 4;
}

void
DataWriterQos_setReliableBursty(struct DDS_DataWriterQos *qos,
                                int worst_burst_in_samples,
                                struct DDS_Duration_t
                                time_to_go_from_full_to_low_water_mark_ms)
{
    qos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;

    qos->history.kind = DDS_KEEP_LAST_HISTORY_QOS;
    qos->history.depth = 256;

    /* avoid malloc and pay memory;
     * might have to change policy for large type */
    /*qos->resource_limits.max_samples = worst_burst_in_samples; */
    qos->resource_limits.max_samples = 256;
    /* if worst burst == expected burst */
    qos->resource_limits.max_samples_per_instance =
        qos->resource_limits.max_samples;
    qos->protocol.rtps_reliable_writer.heartbeat_period.sec = 9990;
    qos->protocol.rtps_reliable_writer.heartbeat_period.nanosec = 1000000;
    qos->protocol.rtps_reliable_writer.heartbeats_per_max_samples = 32;
}

void
DataReaderQos_setImpatientReliable(struct DDS_DataReaderQos *qos)
{
    /* limit the queue, because we don't need to use the queue */
    qos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    qos->history.kind = DDS_KEEP_LAST_HISTORY_QOS;
    qos->history.depth = 2;
    /* limit the queue, because we don't need to use the queue */
    qos->resource_limits.max_samples = 2;

    /* use these hard coded value until you use key */
    qos->resource_limits.max_samples_per_instance =
        qos->resource_limits.max_samples;
    qos->resource_limits.max_instances = 1;
}

void
DataReaderQos_setReliableBursty(struct DDS_DataReaderQos *qos,
                                int remote_writer_count_max)
{
    /* reader queue can be constant regardless of rate */
    const int UNRESOLVED_SAMPLE_PER_REMOTE_WRITER_MAX = 256;

    qos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    qos->history.kind = DDS_KEEP_LAST_HISTORY_QOS;
    qos->history.depth =
        remote_writer_count_max * UNRESOLVED_SAMPLE_PER_REMOTE_WRITER_MAX;

    qos->resource_limits.max_samples =
        remote_writer_count_max * UNRESOLVED_SAMPLE_PER_REMOTE_WRITER_MAX;
    qos->resource_limits.max_samples_per_instance =
        qos->resource_limits.max_samples;
}

DDS_ReturnCode_t
configure_factory_qos(struct DDS_DomainParticipantFactoryQos *factory_qos,
                      DDS_DomainParticipantFactory * factory)
{
    DDS_ReturnCode_t return_code = DDS_RETCODE_OK;
    RT_Registry_T* registry = NULL;

    registry = DDS_DomainParticipantFactory_get_registry(factory);

    if (!RT_Registry_register(registry, DDSHST_WRITER_DEFAULT_HISTORY_NAME,
                              WHSM_HistoryFactory_get_interface(),NULL,NULL))
    {
        return DDS_RETCODE_ERROR;
    }

    if (!RT_Registry_register(registry, DDSHST_READER_DEFAULT_HISTORY_NAME,
                              RHSM_HistoryFactory_get_interface(),NULL,NULL))
    {
        return DDS_RETCODE_ERROR;
    }

    /* We need to disable participants so that we can plug in a new/modified
     * transport */
    return_code = DDS_DomainParticipantFactory_get_qos(factory, factory_qos);
    if (return_code != DDS_RETCODE_OK)
    {
        AppLog_exception("Error get factory QoS\n");
        return return_code;
    }

    factory_qos->entity_factory.autoenable_created_entities = DDS_BOOLEAN_FALSE;
    return_code = DDS_DomainParticipantFactory_set_qos(factory, factory_qos);
    if (return_code != DDS_RETCODE_OK)
    {
        AppLog_exception("Error set factory QoS\n");
        return return_code;
    }
    return return_code;
}

DDS_ReturnCode_t
configure_transport_qos(struct UDP_InterfaceFactoryProperty *udp_property,
                        DDS_DomainParticipantFactory * factory)
{
    DDS_ReturnCode_t return_code = DDS_RETCODE_OK;

    /* default transport can be used in case no transformations are 
     * performed, so nothing to do
     */
#if PERF_TRANSFORMS_ENABLED
    RT_Registry_T* registry = NULL;

    registry = DDS_DomainParticipantFactory_get_registry(factory);

    printf("Using UDP transformations.\n");

    /* unregister UDP transport to change its properties */
    if (!RT_Registry_unregister(registry, NETIO_DEFAULT_UDP_NAME, NULL, NULL))
    {
        printf("failed to unregister udp\n");
        return DDS_RETCODE_ERROR;
    }

    /* register the performance transformation */
    if (!PerformanceUdpTransformFactory_register(registry,
                                                 "pt",
                                                 NULL))
    {
        AppLog_exception("failed to register udp transformation\n");
        return DDS_RETCODE_ERROR;
    }

    /* use this transformation to receive from any address and any mask */
    if (!UDP_TransformRules_assert_source_rule(&udp_property->source_rules, 
                                               0, 0, 
                                               "pt", 
                                               NULL))
    {
        printf("failed to assert source transform\n");
        return DDS_RETCODE_ERROR;
    }

    /* use this transformation to send to any address and any mask */
    if (!UDP_TransformRules_assert_destination_rule(
                                            &udp_property->destination_rules, 
                                            0, 0, 
                                            "pt", 
                                            NULL))
    {
        printf("failed to assert source transform\n");
        return DDS_RETCODE_ERROR;
    }

    /* register UDP transport with the modified properties */
    if (!RT_Registry_register(registry, NETIO_DEFAULT_UDP_NAME,
                              UDP_InterfaceFactory_get_interface(),
                              (struct RT_ComponentFactoryProperty*)udp_property, 
                              NULL))
    {
       printf("failed to register udp\n");
       return DDS_RETCODE_ERROR;
    }
#endif /* PERF_TRANSFORMS_ENABLED */

    return return_code;
}

DDS_ReturnCode_t
configure_participant_qos(struct DDS_DomainParticipantQos * participant_qos,
                          DDS_DomainParticipantFactory * factory,
                          ThroughputArgs * args, const char *participant_name,
                          DDS_Long remote_app)
{
    DDS_ReturnCode_t return_code = DDS_RETCODE_OK;

    /* Someone may decide to start another publisher
     * so take participant index from arguments */
    participant_qos->protocol.participant_id = args->participant_id;
    strcpy(participant_qos->participant_name.name, participant_name);
    participant_qos->resource_limits.remote_participant_allocation = remote_app * 2;
    participant_qos->resource_limits.max_destination_ports = 32;
    participant_qos->resource_limits.max_receive_ports = 32;
    participant_qos->resource_limits.local_topic_allocation = 2;
    participant_qos->resource_limits.local_type_allocation = 10;
    participant_qos->resource_limits.local_writer_allocation = 2;
    participant_qos->resource_limits.local_reader_allocation = 2;
    participant_qos->resource_limits.remote_writer_allocation = 2;
    participant_qos->resource_limits.remote_reader_allocation = remote_app * 2;

    return return_code;
}

DDS_ReturnCode_t
configure_throughput_writer_qos(struct DDS_DataWriterQos *
                                throughput_writer_qos,
                                DDS_Publisher * publisher,
                                ThroughputPublisherArgs * args)
{
    const struct DDS_Duration_t three_second = { 3, 0 };
    DDS_ReturnCode_t return_code = DDS_RETCODE_OK;

    /* We will own the topic so set the strength as determined by the user. */
    throughput_writer_qos->ownership.kind = DDS_SHARED_OWNERSHIP_QOS;
    throughput_writer_qos->ownership_strength.value = args->strength;

    /* Create as many instances as we need */
    throughput_writer_qos->resource_limits.max_instances =
        (args->parent.mi_instance_count >
         0) ? args->parent.mi_instance_count : 1;
    /* and override as necessary */
    if (!args->parent._reliable)
    {
        throughput_writer_qos->reliability.kind =
            DDS_BEST_EFFORT_RELIABILITY_QOS;
    }
    else
    {
        DataWriterQos_setReliableBursty(throughput_writer_qos, QUEUE_SIZE,
                                        args->_max_blocking_time);
    }
    throughput_writer_qos->protocol.rtps_object_id = DATA_WRITER_OBJECT_ID;

    return return_code;
}

DDS_ReturnCode_t
configure_throughput_reader_qos(struct DDS_DataReaderQos *
                                throughput_reader_qos,
                                DDS_Subscriber * subscriber,
                                ThroughputSubscriberArgs * args)
{
    DDS_ReturnCode_t return_code = DDS_RETCODE_OK;
    /* Get the default reader QoS and configure to our requirements */

    throughput_reader_qos->ownership.kind = DDS_SHARED_OWNERSHIP_QOS;
    throughput_reader_qos->resource_limits.max_instances =
        (args->parent.mi_instance_count >
         0) ? args->parent.mi_instance_count : 1;

    throughput_reader_qos->resource_limits.max_samples_per_instance = 1;
    throughput_reader_qos->resource_limits.max_samples =
        throughput_reader_qos->resource_limits.max_samples_per_instance;

    if (args->parent._reliable)
    {
        DataReaderQos_setReliableBursty(throughput_reader_qos, 1);
    }
    throughput_reader_qos->protocol.rtps_object_id = DATA_READER_OBJECT_ID + args->subscriberId;

    return return_code;
}

DDS_ReturnCode_t
configure_command_reader_qos(struct DDS_DataReaderQos *
                             throughput_command_reader_qos,
                             DDS_Subscriber * subscriber)
{
    DDS_ReturnCode_t return_code = DDS_RETCODE_OK;
    /* Get the default reader QoS and configure to
     * our requirements for command packets */

    throughput_command_reader_qos->ownership.kind = DDS_SHARED_OWNERSHIP_QOS;

    /* We will use reliable communications to communicate commands... */
    DataReaderQos_setImpatientReliable(throughput_command_reader_qos);
    throughput_command_reader_qos->protocol.rtps_object_id =
        COMMAND_READER_OBJECT_ID;
    return return_code;
}

DDS_ReturnCode_t
configure_command_writer_qos(struct DDS_DataWriterQos *
                             throughput_command_writer_qos,
                             DDS_Publisher * publisher,
                             ThroughputPublisherArgs * args)
{
    DDS_ReturnCode_t return_code = DDS_RETCODE_OK;

    /* We will own the topic so set the strength as determined by the user. */
    throughput_command_writer_qos->ownership.kind = DDS_SHARED_OWNERSHIP_QOS;
    throughput_command_writer_qos->ownership_strength.value = args->strength;
    /* We will use guaranteed reliable communications
     * to communicate commands... */
    DataWriterQos_setImpatientReliable(throughput_command_writer_qos,
                                       one_second);
    throughput_command_writer_qos->protocol.rtps_object_id =
        COMMAND_WRITER_OBJECT_ID;
    return return_code;
}

void get_subscriber_name(char *buffer, int subscriberId)
{
    const char subscriber_name[] = "subscriber";

    strcpy(buffer, subscriber_name);
    sprintf(&buffer[strlen(subscriber_name)], "%d", subscriberId);
}