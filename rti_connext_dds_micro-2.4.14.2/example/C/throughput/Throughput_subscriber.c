/*
 (c) Copyright, Real-Time Innovations, 2009-2015.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.
*/

#include <stdio.h>
#include <stdlib.h>
#ifndef rti_me_c_h
#include "rti_me_c.h"
#endif
#ifndef netio_udp_h
#include "netio/netio_udp.h"
#endif
#ifndef ThroughputPlugin_h
#include "ThroughputPlugin.h"
#endif
#ifndef ThroughputSupport_h
#include "ThroughputSupport.h"
#endif
#ifndef ThroughputArgs_h
#include "ThroughputArgs.h"
#endif
#ifndef ThroughputCommon_h
#include "ThroughputCommon.h"
#endif
#ifndef ThroughputQos_h
#include "ThroughputQos.h"
#endif
#ifndef PerfMon_h
#include "PerfMon.h"
#endif
#ifndef TimeManager_h
#include "TimeManager.h"
#endif

typedef struct ThroughputCommandListener
{
    struct DDS_DataReaderListener parent;
    ThroughputCommand packet;
    int _matching_writer;
    int _samples_lost;
} ThroughputCommandListener;

void
ThroughputCommandListener_reset(ThroughputCommandListener * self)
{
    self->packet.command = THROUGHPUT_COMMAND_IDLE;
}

void
ThroughputCommandListener_delete(ThroughputCommandListener * self)
{
};

void
ThroughputCommandListener_on_data_available(void *listener_data,
                                            DDS_DataReader * reader)
{
    ThroughputCommandListener *self =
        *(ThroughputCommandListener **) listener_data;
    DDS_ReturnCode_t return_code;
    struct DDS_SampleInfo info;

    return_code = DDS_DataReader_take_next_sample(reader, &self->packet, &info);
    if (return_code != DDS_RETCODE_OK)
    {
        AppLog_exception("\nError in Throughput Command Reader\n");
        return;
    }

    if (!info.valid_data)
    {
        ThroughputCommand_initialize(&self->packet);
    }
}

void
on_liveliness_changed(void *listener_data, DDS_DataReader * reader,
                      const struct DDS_LivelinessChangedStatus *status)
{
}

void
on_requested_deadline_missed(void *listener_data, DDS_DataReader * reader,
                             const struct DDS_RequestedDeadlineMissedStatus
                             *status)
{
}

void
on_requested_incompatible_qos(void *listener_data, DDS_DataReader * reader,
                              const struct DDS_RequestedIncompatibleQosStatus
                              *status)
{
}

void
ThroughputCommandListener_on_sample_lost(void *listener_data,
                                         DDS_DataReader * reader,
                                         const struct DDS_SampleLostStatus
                                         *status)
{
    ThroughputCommandListener *self =
        *(ThroughputCommandListener **) listener_data;
    self->_samples_lost = status->total_count;
}

void
on_sample_rejected(void *listener_data, DDS_DataReader * reader,
                   const struct DDS_SampleRejectedStatus *status)
{
}


void
on_instance_replaced(void *listener_data, DDS_DataReader * reader,
                   const struct DDS_DataReaderInstanceReplacedStatus *status)
{
}

void
ThroughputCommandListener_on_subscription_matched(void *listener_data,
                                                  DDS_DataReader * reader,
                                                  const struct
                                                  DDS_SubscriptionMatchedStatus
                                                  *status)
{
    ThroughputCommandListener *self =
        *(ThroughputCommandListener **) listener_data;
    self->_matching_writer = status->current_count;
}

void
ThroughputCommandListener_create(ThroughputCommandListener *self)
{
    self->parent.on_data_available =
        ThroughputCommandListener_on_data_available;
    self->parent.on_liveliness_changed = on_liveliness_changed;
    self->parent.on_requested_deadline_missed = on_requested_deadline_missed;
    self->parent.on_requested_incompatible_qos = on_requested_incompatible_qos;
    self->parent.on_sample_lost = ThroughputCommandListener_on_sample_lost;
    self->parent.on_sample_rejected = on_sample_rejected;
    self->parent.on_instance_replaced = on_instance_replaced;
    self->parent.on_subscription_matched =
        ThroughputCommandListener_on_subscription_matched;
    self->parent.on_before_sample_commit = NULL;
    self->parent.on_before_sample_deserialize = NULL;
    self->packet.data_length = 0;
    self->_matching_writer = 0;
    self->_samples_lost = 0;
    ThroughputCommandListener_reset(self);
    self->parent.as_listener.listener_data = (void *)self;
}

typedef struct ThroughputListener
{
    struct DDS_DataReaderListener parent;
    int _packets_received;
    int _packets_lost;
    DDS_UnsignedLong _sequence_number;
    int _matching_writer;
    int _samples_lost;
    int _samples_rejected;
} ThroughputListener;

void
ThroughputListener_delete(ThroughputListener * self)
{
}

void
ThroughputListener_resetCounts(ThroughputListener * self)
{
    self->_samples_rejected = 0;
    self->_samples_lost = 0;
    self->_sequence_number = 0;
    self->_packets_received = 0;
    self->_packets_lost = 0;
}

void
ThroughputListener_reset(ThroughputListener * self)
{
    ThroughputListener_resetCounts(self);
    self->_matching_writer = 0;
}

void
ThroughputListener_on_sample_rejected(void *listener_data,
                                      DDS_DataReader * reader,
                                      const struct DDS_SampleRejectedStatus
                                      *status)
{
    ThroughputListener *self = *(ThroughputListener **) listener_data;
    self->_samples_rejected += status->total_count_change;
}

void
ThroughputListener_on_sample_lost(void *listener_data,
                                  DDS_DataReader * reader,
                                  const struct DDS_SampleLostStatus *status)
{
    ThroughputListener *self = *(ThroughputListener **) listener_data;
    self->_samples_lost += status->total_count_change;
}

void
ThroughputListener_on_subscription_matched(void *listener_data,
                                           DDS_DataReader * reader,
                                           const struct
                                           DDS_SubscriptionMatchedStatus
                                           *status)
{
    ThroughputListener *self = *(ThroughputListener **) listener_data;
    self->_matching_writer = status->current_count;
}

void
ThroughputListener_on_data_available(void *listener_data,
                                     DDS_DataReader * reader)
{
    ThroughputListener *self = *(ThroughputListener **) listener_data;
    /* Increment number of packets received */
    Throughput data;
    struct DDS_SampleInfo info;
    DDS_ReturnCode_t return_code = DDS_RETCODE_OK;

    if (!Throughput_initialize(&data))
    {
        AppLog_exception("Error in Throughput initialize_sample\n");
        return;
    }

    while (1)
    {
        return_code = DDS_DataReader_take_next_sample(reader, &data, &info);
        if (return_code != DDS_RETCODE_OK)
        {
            goto done;
        }
        if (info.valid_data)
        {
            if (data.sequence_number != self->_sequence_number)
            {
                /*if (self->_sequence_number != 0) {
                 * self->_packets_lost += (data.sequence_number -
                 * self->_sequence_number);
                 * if (data.sequence_number < self->_sequence_number) {
                 * AppLog_exception("data.sequence_number = %d < self->_sequence_number = %d\n", data.sequence_number, self->_sequence_number);
                 * }
                 * } */
                /* Reset sequence number */
                self->_sequence_number = data.sequence_number;
            }
            ++self->_packets_received;
            ++self->_sequence_number;
        }
    }
    done:
    if (!Throughput_finalize(&data))
    {
        AppLog_exception("Error in Throughput finalize_sample\n");
    }
}

void
ThroughputListener_create(ThroughputListener * self)
{
    self->parent.on_data_available = ThroughputListener_on_data_available;
    self->parent.on_liveliness_changed = on_liveliness_changed;
    self->parent.on_requested_deadline_missed = on_requested_deadline_missed;
    self->parent.on_requested_incompatible_qos = on_requested_incompatible_qos;
    self->parent.on_sample_lost = ThroughputListener_on_sample_lost;
    self->parent.on_sample_rejected = ThroughputListener_on_sample_rejected;
    self->parent.on_subscription_matched =
        ThroughputListener_on_subscription_matched;
    self->parent.on_instance_replaced = on_instance_replaced;
    self->parent.on_before_sample_commit = NULL;
    self->parent.on_before_sample_deserialize = NULL;
    ThroughputListener_reset(self);
    self->parent.as_listener.listener_data = (void *)self;
}

/* Delete all entities */
static int
subscriber_shutdown(DDS_DomainParticipant *participant)
{
    DDS_ReturnCode_t return_code;
    int status = 0;
    RT_Registry_T *registry = NULL;

    if (participant != NULL)
    {
        return_code =
            DDS_DomainParticipant_delete_contained_entities(participant);
        if (return_code != DDS_RETCODE_OK)
        {
            AppLog_exception("delete_contained_entities error %d\n",
                             return_code);
            status = -1;
        }

        return_code =
            DDS_DomainParticipantFactory_delete_participant
            (DDS_DomainParticipantFactory_get_instance(), participant);
        if (return_code != DDS_RETCODE_OK)
        {
            AppLog_exception("delete_participant error %d\n", return_code);
            status = -1;
        }
    }

    registry = DDS_DomainParticipantFactory_get_registry(
       DDS_DomainParticipantFactory_get_instance());

#if PERF_TRANSFORMS_ENABLED
    /* unregister the performance transformation */
    if (!PerformanceUdpTransformFactory_unregister(registry,
                                                   "pt",
                                                   NULL))
    {
        AppLog_exception("unregister transformations error\n");
        status = -1;
    }
#endif /* PERF_TRANSFORMS_ENABLED */

    if (!RT_Registry_unregister(registry, "dpse", NULL, NULL))
    {
        AppLog_exception("unregister dpse error\n");
        status = -1;
    }

    if (!RT_Registry_unregister(registry, DDSHST_READER_DEFAULT_HISTORY_NAME, NULL, NULL))
    {
        AppLog_exception("unregister reader history error\n");
        status = -1;
    }
    if (!RT_Registry_unregister(registry, DDSHST_WRITER_DEFAULT_HISTORY_NAME, NULL, NULL))
    {
        AppLog_exception("unregister writer history error\n");
        status = -1;
    }

    return_code = DDS_DomainParticipantFactory_finalize_instance();
    if (return_code != DDS_RETCODE_OK)
    {
        AppLog_exception("finalize_instance error %d\n", return_code);
        status = -1;
    }

    return status;
}

int
subscriber_test_throughput(ThroughputSubscriberArgs * args)
{
    DDS_DomainParticipantFactory *factory;
    struct DDS_DomainParticipantFactoryQos factory_qos =
        DDS_DomainParticipantFactoryQos_INITIALIZER;
    DDS_DomainParticipant *participant = NULL;
    struct DDS_DomainParticipantQos participant_qos =
        DDS_DomainParticipantQos_INITIALIZER;
    DDS_Subscriber *subscriber;
    struct DDS_SubscriberQos subscriber_qos = DDS_SubscriberQos_INITIALIZER;
    DDS_DataReader *throughput_reader;
    struct DDS_DataReaderQos throughput_reader_qos = DDS_DataReaderQos_INITIALIZER;
    ThroughputListener throughput_reader_listener;
    DDS_DataReader *throughput_command_reader;
    struct DDS_DataReaderQos throughput_command_reader_qos =
        DDS_DataReaderQos_INITIALIZER;
    ThroughputCommandListener throughput_command_reader_listener;
    DDS_Topic *throughput_topic;
    DDS_Topic *throughput_command_topic;
    struct DDS_PublicationBuiltinTopicData rem_publication_data =
        DDS_PublicationBuiltinTopicData_INITIALIZER;
    struct DDS_PublicationBuiltinTopicData rem_command_publication_data =
        DDS_PublicationBuiltinTopicData_INITIALIZER;
    struct UDP_InterfaceFactoryProperty udp_property = 
        UDP_INTERFACE_FACTORY_PROPERTY_DEFAULT;
    char buffer[40];

    /***************************************************
     * Other data
     ***************************************************/
    DDS_ReturnCode_t return_code = DDS_RETCODE_OK;
    int result = -1;
    /* We use a manager to handle all of the timing */
    TimeManager time_manager;
    double bytes_read = 0.0;
    double delta_time = 0.0;
    double subscriber_throughput = 0.0;
    float appCpuLoad;
    unsigned long memory;
    int packets_received = 0;
    
    PerfMon subsMonitor;
    struct DPSE_DiscoveryPluginProperty discovery_plugin_properties =
        DPSE_DiscoveryPluginProperty_INITIALIZER;
    int i;
    RT_Registry_T* registry;

    PerfMon_create(&subsMonitor);

    factory = DDS_DomainParticipantFactory_get_instance();
    if (factory == NULL)
    {
        AppLog_exception("Error in getting a participant factory\n");
        goto done;
    }

    /* Configure the Factory Quality of Service */
    return_code = configure_factory_qos(&factory_qos, factory);
    if (return_code != DDS_RETCODE_OK)
    {
        AppLog_exception("Error in configure_factory_qos\n");
        goto done;
    }

    /* Configure the transport */
    return_code = configure_transport_qos(&udp_property, factory);
    if (return_code != DDS_RETCODE_OK)
    {
        AppLog_exception("Error in configure_factory_qos\n");
        goto done;
    }

    /* Configure the Participant Quality of Service */
    get_subscriber_name(buffer, args->subscriberId);

    return_code =
        configure_participant_qos(&participant_qos, factory, &args->parent,
                                  buffer, 
                                  1 /* subscriber only needs to 
                                     * communicate with 1 publisher 
                                     */
                                  );

    if (return_code != DDS_RETCODE_OK)
    {
        AppLog_exception("Error in configure_participant_qos\n");
        goto done;
    }

    if (!RT_ComponentFactoryId_set_name(&participant_qos.discovery.discovery.name,"dpse"))
    {
        AppLog_exception("failed to set discovery plugin name\n");
        goto done;
    }

    if (args->parent.max_peer_index > 0)
    {
        const char *initial_peer_array[THROUGHPUT_TEST_MAX_NODES];

        if (!DDS_StringSeq_set_maximum(&participant_qos.discovery.initial_peers,
                                       args->parent.max_peer_index))
        {
            printf("failed to set initial peers maximum\n");
            goto done;
        }
        if (!DDS_StringSeq_set_length(&participant_qos.discovery.initial_peers,
                                      args->parent.max_peer_index))
        {
            printf("failed to set initial peers length\n");
            goto done;
        }

        for (i = 0; i < args->parent.max_peer_index; ++i)
        {
            initial_peer_array[i] =
                ThroughputArgs_get_peer_host(&args->parent, i);
            if (initial_peer_array[i] == NULL)
            {
                AppLog_exception("ThroughputArgs_get_peer_host failed\n");
                return_code = DDS_RETCODE_ERROR;
                goto done;
            }

            *DDS_StringSeq_get_reference(&participant_qos.discovery.initial_peers,i) = 
                DDS_String_dup(initial_peer_array[i]);
        }
    }
    else 
    {
        /* default loopback peer */
        if (!DDS_StringSeq_set_maximum(&participant_qos.discovery.initial_peers,1))
        {
            printf("failed to set initial peers maximum\n");
            goto done;
        }
        if (!DDS_StringSeq_set_length(&participant_qos.discovery.initial_peers,1))
        {
            printf("failed to set initial peers length\n");
            goto done;
        }
        *DDS_StringSeq_get_reference(&participant_qos.discovery.initial_peers,0) = 
                DDS_String_dup("127.0.0.1");
    }

    registry = DDS_DomainParticipantFactory_get_registry(factory);

    if (!RT_Registry_register(
        registry, "dpse",
        DPSE_DiscoveryFactory_get_interface(),
        &discovery_plugin_properties._parent,
        NULL))
    {
        AppLog_exception("Failed to register DPSE\n");
        goto done;
    }

    /* Now we can create the 'disabled' participant. */
    participant = DDS_DomainParticipantFactory_create_participant(factory,
                                                                  args->parent.
                                                                  ndds_domain,
                                                                  &participant_qos,
                                                                  NULL,
                                                                  DDS_STATUS_MASK_NONE);
    if (participant == NULL)
    {
        AppLog_exception("Error in creating Domain Participant\n");
        goto done;
    }

    subscriber =
        DDS_DomainParticipant_create_subscriber(participant, &subscriber_qos,
                                                NULL, DDS_STATUS_MASK_NONE);
    if (subscriber == NULL)
    {
        AppLog_exception("Error in creating Subscriber\n");
        goto done;
    }

    /*********************************************************
     * Set up Throughput topic and reader.
     **********************************************************/

    /* Now we register the data topic type with the participant. */
    return_code =
        ThroughputTypeSupport_register_type(participant,
                                            ThroughputTYPENAME);
    if (return_code != DDS_RETCODE_OK)
    {
        AppLog_exception("Error in registering %s\n", ThroughputTYPENAME);
        goto done;
    }

    /* And create a Topic with Default QoS and default listener.
     * Note: Ownership is Shared by default. */
    throughput_topic =
        DDS_DomainParticipant_create_topic(participant, "Throughput",
                                           ThroughputTYPENAME,
                                           &DDS_TOPIC_QOS_DEFAULT, NULL,
                                           DDS_STATUS_MASK_NONE);
    if (throughput_topic == NULL)
    {
        AppLog_exception("Error in creating data Topic\n");
        goto done;
    }
    ThroughputListener_create(&throughput_reader_listener);

    /* Configure throughput reader Quality of service */
    return_code = configure_throughput_reader_qos(&throughput_reader_qos,
                                                  subscriber, args);
    if (return_code != DDS_RETCODE_OK)
    {
        AppLog_exception("Error in configure_throughput_reader_qos\n");
        goto done;
    }

    /* Now create the throughput reader */
    throughput_reader = DDS_Subscriber_create_datareader(subscriber,
                                                         DDS_Topic_as_topicdescription
                                                         (throughput_topic),
                                                         &throughput_reader_qos,
                                                         &throughput_reader_listener.
                                                         parent,
                                                         DDS_STATUS_MASK_ALL);
    if (throughput_reader == NULL)
    {
        AppLog_exception("Error in creating Data Reader\n");
        goto done;
    }

    rem_publication_data.key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] =
        DATA_WRITER_OBJECT_ID;
    rem_publication_data.topic_name = DDS_String_dup("Throughput");
    rem_publication_data.type_name = DDS_String_dup(ThroughputTYPENAME);
    rem_publication_data.ownership.kind = DDS_SHARED_OWNERSHIP_QOS;
    if (args->parent._reliable)
    {
        rem_publication_data.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    }

    /*************************************************************************
     * Set up Command topic and reader.
     *************************************************************************/

    /* Now we register the command topic type with the participant */
    return_code =
        ThroughputCommandTypeSupport_register_type(participant,
                                                   ThroughputCommandTYPENAME);
    if (return_code != DDS_RETCODE_OK)
    {
        AppLog_exception("Error in registering %s\n",
                         ThroughputCommandTYPENAME);
        goto done;
    }

    /* and create the command topic and reader */
    throughput_command_topic =
        DDS_DomainParticipant_create_topic(participant, "ThroughputCommand",
                                           ThroughputCommandTYPENAME,
                                           &DDS_TOPIC_QOS_DEFAULT, NULL,
                                           DDS_STATUS_MASK_NONE);
    if (throughput_command_topic == NULL)
    {
        AppLog_exception("Error in creating Command Topic\n");
        goto done;
    }

    ThroughputCommandListener_create(&throughput_command_reader_listener);

    return_code =
        configure_command_reader_qos(&throughput_command_reader_qos,
                                     subscriber);
    if (return_code != DDS_RETCODE_OK)
    {
        AppLog_exception("Error in configuring command reader qos\n");
        goto done;
    }

    /* create and enable command reader */
    throughput_command_reader = DDS_Subscriber_create_datareader(subscriber,
                                                                 DDS_Topic_as_topicdescription
                                                                 (throughput_command_topic),
                                                                 &throughput_command_reader_qos,
                                                                 &throughput_command_reader_listener.
                                                                 parent,
                                                                 DDS_STATUS_MASK_ALL);
    if (throughput_command_reader == NULL)
    {
        AppLog_exception("Error in creating Command Reader\n");
        goto done;
    }

    rem_command_publication_data.key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] =
        COMMAND_WRITER_OBJECT_ID;
    rem_command_publication_data.topic_name =
        DDS_String_dup("ThroughputCommand");
    rem_command_publication_data.type_name =
        DDS_String_dup(ThroughputCommandTYPENAME);
    rem_command_publication_data.ownership.kind = DDS_SHARED_OWNERSHIP_QOS;
    rem_command_publication_data.ownership_strength.value = 5;  /* Hard-coded! */
    rem_command_publication_data.reliability.kind =
        DDS_RELIABLE_RELIABILITY_QOS;

    return_code = DPSE_RemoteParticipant_assert(participant, "publisher");
    if (return_code != DDS_RETCODE_OK)
    {
        AppLog_exception("failed to assert remote participant\n");
        goto done;
    }

    return_code = DPSE_RemotePublication_assert(participant,
                                                "publisher",
                                                &rem_publication_data,
                                                NDDS_TYPEPLUGIN_USER_KEY);
    if (return_code != DDS_RETCODE_OK)
    {
        AppLog_exception("failed to assert remote publication\n");
        goto done;
    }

    return_code = DPSE_RemotePublication_assert(participant,
                                                "publisher",
                                                &rem_command_publication_data,
                                                NDDS_TYPEPLUGIN_NO_KEY);
    if (return_code != DDS_RETCODE_OK)
    {
        AppLog_exception("failed to assert remote publication\n");
        goto done;
    }

    /* Now we enable the participant */
    return_code =
        DDS_Entity_enable(DDS_DomainParticipant_as_entity(participant));
    if (return_code != DDS_RETCODE_OK)
    {
        AppLog_exception("Error in enabling the domain participant\n");
        goto done;
    }

    /*************************************************************************
     * Start test
     ************************************************************************/

    if (!TimeManager_initialize(&time_manager, 0))
    {
        AppLog_exception("Error in creating clock\n");
        goto done;
    }

    if (!TimeManager_calculate_clock_overhead(&time_manager))
    {
        AppLog_exception("Error in calculating clock overhead. Results may not"
                         " be accurate\n");
    }

    AppLog_report("Starting test...\n");
    AppLog_report("with the following parameters...\n");
    ThroughputSubscriberArgs_print_arguments(args);

    AppLog_report
        (" bytes, demand, samples, sample/s, Mbit/s, Nreject, lost(N), CPU %%, memory(kB)\n"
         "------, ------, -------, --------, ------, -------, -------, -----, ----------\n");
    fflush(stdout);

    do
    {
        ThroughputCommandListener_reset(&throughput_command_reader_listener);
        /* Reset Data Reader Listener counts prior to starting test */
        ThroughputListener_resetCounts(&throughput_reader_listener);

        AppLog_warn("Waiting for test start command\n");

        /* wait for the start command */
        while (throughput_command_reader_listener.packet.command !=
               THROUGHPUT_COMMAND_START)
        {
            TimeManager_sleep(&time_manager,
                              (struct DDS_Duration_t *)&ten_millisec);
        }
        appCpuLoad = PerfMon_getCpu(&subsMonitor);
        /* Command contains writer packet size */
        /* throughput_command_reader_listener->packet.packet_size =
         * throughput_command_reader_listener->getPacketSize(); */
        if (!TimeManager_get_start_time(&time_manager))
        {
            AppLog_exception("Error in setting StartTime.\n");
            goto done;
        }

        /* Run until the test complete command is received */
        while (throughput_command_reader_listener.packet.command !=
               THROUGHPUT_COMMAND_COMPLETE)
        {
            TimeManager_sleep(&time_manager,
                              (struct DDS_Duration_t *)&ten_millisec);
        }

        if (!TimeManager_get_stop_time(&time_manager))
        {
            AppLog_exception("Error in getting Stop Time."
                             " Results may not be accurate\n");
            goto done;
        }
        packets_received = throughput_reader_listener._packets_received;
        appCpuLoad = PerfMon_getCpu(&subsMonitor);
        delta_time = TimeManager_get_delta_time(&time_manager);

        bytes_read = (double)packets_received *(THROUGHPUT_PACKET_OVERHEAD +
                                        throughput_command_reader_listener.
                                        packet.data_length);
        if (delta_time > 0)
        {
            subscriber_throughput = 8.0 * bytes_read / delta_time / 1000000.0;
        }
        memory = PerfMon_getMemory(&subsMonitor);

        AppLog_report("%6d, %6d, %7d, %8.2f, %6.1f, %7d, %7d, %5.2f, %10.1f\n",
                      (throughput_command_reader_listener.packet.data_length +
                       THROUGHPUT_PACKET_OVERHEAD),
                      throughput_command_reader_listener.packet.
                      current_publisher_effort, packets_received,
                      (float)packets_received / delta_time,
                      subscriber_throughput,
                      /*throughput_reader_listener._packets_lost, this doesn't work yet...needs wait_for_acknowledgements */
                      throughput_reader_listener._samples_rejected,
                      throughput_reader_listener._samples_lost,
                      appCpuLoad,
                      (float)(memory - subsMonitor.memBase) / (1024.0 * 1024.0));

        fflush(stdout);

        /* Receive remaining of data */
        TimeManager_sleep(&time_manager,
                          (struct DDS_Duration_t *)&three_second);

    }
    while (throughput_command_reader_listener.packet.current_publisher_effort
           < throughput_command_reader_listener.packet.final_publisher_effort);

    AppLog_report("Test Completed.\n");
    result = 0;

    done:
    if (participant != NULL)
    {
        subscriber_shutdown(participant);
    }
    ThroughputCommandListener_delete(&throughput_command_reader_listener);
    ThroughputListener_delete(&throughput_reader_listener);
    TimeManager_delete(&time_manager);

    DDS_DomainParticipantFactoryQos_finalize(&factory_qos);
    DDS_DomainParticipantQos_finalize(&participant_qos);
    DDS_SubscriberQos_finalize(&subscriber_qos);
    DDS_DataReaderQos_finalize(&throughput_reader_qos);
    DDS_DataReaderQos_finalize(&throughput_command_reader_qos);
    DDS_PublicationBuiltinTopicData_finalize(&rem_publication_data);
    DDS_PublicationBuiltinTopicData_finalize(&rem_command_publication_data);

    return result;
}

#if defined(RTI_WINCE)
int
wmain(int argc, wchar_t * argv[])
{
    char arg_array[MAX_COMMAND_LINE_ARGUMENTS][MAX_COMMAND_EXEC_LEN];
    char *argv_c[MAX_COMMAND_LINE_ARGUMENTS];
    int i;

    for (i = 0; i < argc; ++i)
    {
        wcstombs(arg_array[i], argv[i], wcslen(argv[i]) + 1);
        arg_array[i][wcslen(argv[i])] = '\0';
        argv_c[i] = &arg_array[i][0];
    }
    argv_c[argc] = '\0';

    ThroughputSubscriberArgs args;

    if (argc <= 1)
    {
        ThroughputSubscriberArgs_create(&args);
    }
    else
    {
        ThroughputSubscriberArgs_create_with_string(&args, argc, argv_c);
    }

#elif !(defined(RTI_VXWORKS) && !defined(__RTP__)) && !defined(RTI_PSOS)
int
main(int argc, char *argv[])
{
    ThroughputSubscriberArgs args;
    int ret_value;

    if (argc <= 1)
    {
        ThroughputSubscriberArgs_create(&args);
    }
    else
    {
        ThroughputSubscriberArgs_create_with_string(&args, argc, argv);
    }
#elif defined(RTI_VXWORKS)
int
subscriber_main(void)
{
    ThroughputSubscriberArgs args;
    int ret_value;
    ThroughputSubscriberArgs_create(&args);
    /* add your peers here: */
    strncpy(args.parent._peer_hosts[0], "10.45.1.104",
            MAX_PEER_LOCATOR_STR_LEN);
    args.parent.max_peer_index = 1;
    args.parent.ndds_domain = 12;
    args.parent.participant_id = 1;
    args.parent._reliable = RTI_TRUE;   /* set to RTI_TRUE to turn on reliable reliability */
#endif

    if (args.parent._help_requested)
    {
        ThroughputSubscriberArgs_usage(&args);
        ThroughputSubscriberArgs_delete(&args);
        return -1;
    }
    else if (!args.parent._args_valid)
    {
        ThroughputSubscriberArgs_print_error(&args);
        ThroughputSubscriberArgs_delete(&args);
        return -1;
    }
    ret_value = subscriber_test_throughput(&args);
    ThroughputSubscriberArgs_delete(&args);
    return ret_value;
}
