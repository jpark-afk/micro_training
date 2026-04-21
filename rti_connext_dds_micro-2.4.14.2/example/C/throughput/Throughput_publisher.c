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

/* ---------------------------------------------------------------------*/
void
ThroughputListener_on_publication_matched(void *listener_data,
                                          DDS_DataWriter * writer,
                                          const struct
                                          DDS_PublicationMatchedStatus *status)
{
    DDS_Long *num_subscriptions = (DDS_Long *)listener_data;
    *num_subscriptions = status->current_count;
}

/* Delete all entities */
static int
publisher_shutdown(DDS_DomainParticipant *participant)
{
    DDS_ReturnCode_t return_code;
    RT_Registry_T *registry = NULL;

    if (participant != NULL)
    {
        return_code =
            DDS_DomainParticipant_delete_contained_entities(participant);
        if (return_code != DDS_RETCODE_OK)
        {
            AppLog_exception("delete_contained_entities error %d\n",
                             return_code);
            return -1;
        }

        return_code =
            DDS_DomainParticipantFactory_delete_participant
            (DDS_DomainParticipantFactory_get_instance(), participant);
        if (return_code != DDS_RETCODE_OK)
        {
            AppLog_exception("delete_participant error %d\n", return_code);
            return -1;
        }
    }

    registry = DDS_DomainParticipantFactory_get_registry(
       DDS_DomainParticipantFactory_get_instance());

#if PERF_TRANSFORMS_ENABLED
    /* register the performance transformation */
    if (!PerformanceUdpTransformFactory_unregister(registry,
                                                   "pt",
                                                   NULL))
    {
        AppLog_exception("unregister dpse error\n");
        return -1;
    }
#endif /* PERF_TRANSFORMS_ENABLED */

    if (!RT_Registry_unregister(registry, "dpse", NULL, NULL))
    {
        AppLog_exception("unregister dpse error\n");
        return -1;
    }

    if (!RT_Registry_unregister(registry, DDSHST_READER_DEFAULT_HISTORY_NAME, NULL, NULL))
    {
        AppLog_exception("unregister reader history error\n");
        return -1;
    }
    if (!RT_Registry_unregister(registry, DDSHST_WRITER_DEFAULT_HISTORY_NAME, NULL, NULL))
    {
        AppLog_exception("unregister writer history error\n");
        return -1;
    }

    return_code = DDS_DomainParticipantFactory_finalize_instance();
    if (return_code != DDS_RETCODE_OK)
    {
        AppLog_exception("finalize_instance error %d\n", return_code);
        return -1;
    }

    return 0;
}

int
publisher_test_throughput(ThroughputPublisherArgs * args)
{
    DDS_DomainParticipantFactory *factory;
    struct DDS_DomainParticipantFactoryQos factory_qos =
        DDS_DomainParticipantFactoryQos_INITIALIZER;
    DDS_DomainParticipant *participant = NULL;
    struct DDS_DomainParticipantQos participant_qos =
        DDS_DomainParticipantQos_INITIALIZER;
    DDS_Publisher *publisher;
    /* And a data writer to do the work */
    DDS_DataWriter *throughput_writer;
    struct DDS_DataWriterQos throughput_writer_qos = DDS_DataWriterQos_INITIALIZER;
    struct DDS_InstanceHandleSeq subscription_handles;
    /* And a writer to communicate commands */
    DDS_DataWriter *throughput_command_writer;
    struct DDS_DataWriterQos throughput_command_writer_qos =
        DDS_DataWriterQos_INITIALIZER;
    struct DDS_InstanceHandleSeq command_subscription_handles;
    DDS_Topic *throughput_topic;
    Throughput *throughput_instance = NULL;
    DDS_Topic *throughput_command_topic;
    ThroughputCommand *throughput_command_instance = NULL;
    struct DDS_SubscriptionBuiltinTopicData rem_subscription_data =
        DDS_SubscriptionBuiltinTopicData_INITIALIZER;
    struct DDS_SubscriptionBuiltinTopicData rem_command_subscription_data =
        DDS_SubscriptionBuiltinTopicData_INITIALIZER;
    struct DDS_DataWriterListener data_listener =
        DDS_DataWriterListener_INITIALIZER;
    struct DDS_DataWriterListener command_listener =
        DDS_DataWriterListener_INITIALIZER;
    struct UDP_InterfaceFactoryProperty udp_property = 
        UDP_INTERFACE_FACTORY_PROPERTY_DEFAULT;

    /***************************************************
     * Other data
     ***************************************************/

    DDS_ReturnCode_t return_code = DDS_RETCODE_OK;
    int result = -1;
    unsigned long packets_written = 0L;
    double bytes_written = 0.0;
    double delta_time = 0.0;
    double publisher_throughput = 0.0;

    /*for CPU usage */
    PerfMon app_monitor;

    /* We use a manager to handle all of the timing */
    TimeManager time_manager;
    int i, load;
    DDS_Boolean test_complete = DDS_BOOLEAN_FALSE;
    int current_load;
    float appCpuLoad;
    struct DDS_Duration_t ack_wait_time = { 60, 0 };        /* 1 minute */
    struct DDS_Duration_t recovery_time =
        { 0, NANOSEC_PER_MILLISEC * args->recovery_time_msec };
    struct DPSE_DiscoveryPluginProperty discovery_plugin_properties =
        DPSE_DiscoveryPluginProperty_INITIALIZER;
    DDS_Long num_data_subscriptions = 0, num_command_subscriptions = 0;
    RT_Registry_T* registry;

    PerfMon_create(&app_monitor);

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

    /* Configure participant Quality of Service */
    return_code =
        configure_participant_qos(&participant_qos, factory, &args->parent,
                                  "publisher", args->subscribers);
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
        DDS_StringSeq_set_maximum(&participant_qos.discovery.initial_peers,1);
        DDS_StringSeq_set_length(&participant_qos.discovery.initial_peers,1);
        *DDS_StringSeq_get_reference(&participant_qos.discovery.initial_peers,0) = 
                DDS_String_dup("127.0.0.1");
    }

    registry = DDS_DomainParticipantFactory_get_registry(factory);
    if (!RT_Registry_register(registry, "dpse",
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

    /* And create the publisher with new QoS */
    publisher =
        DDS_DomainParticipant_create_publisher(participant,
                                               &DDS_PUBLISHER_QOS_DEFAULT, NULL,
                                               DDS_STATUS_MASK_NONE);
    if (publisher == NULL)
    {
        AppLog_exception("Error in creating Publisher\n");
        goto done;
    }

    /***********************************************************************
     * Set up Throughput topic and writer.
     ***********************************************************************/

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

    /* set up user data */
    throughput_instance = ThroughputTypeSupport_create_data();
    if (throughput_instance == NULL)
    {
        AppLog_exception("Error in creating Data Instance\n");
        goto done;
    }

    /* Configure throughput writer Quality of service */
    return_code =
        configure_throughput_writer_qos(&throughput_writer_qos, publisher,
                                        args);
    if (return_code != DDS_RETCODE_OK)
    {
        AppLog_exception("Error in configure_throughput_writer_qos\n");
        goto done;
    }

    data_listener.on_publication_matched =
        ThroughputListener_on_publication_matched;
    data_listener.as_listener.listener_data = &num_data_subscriptions;
    throughput_writer = DDS_Publisher_create_datawriter(publisher,
                                                        throughput_topic,
                                                        &throughput_writer_qos,
                                                        &data_listener,
                                                        DDS_PUBLICATION_MATCHED_STATUS);
    if (throughput_writer == NULL)
    {
        AppLog_exception("Error in creating Data Writer\n");
        goto done;
    }

    rem_subscription_data.key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] =
        DATA_READER_OBJECT_ID;
    rem_subscription_data.topic_name = DDS_String_dup("Throughput");
    rem_subscription_data.type_name = DDS_String_dup(ThroughputTYPENAME);
    rem_subscription_data.ownership.kind = DDS_SHARED_OWNERSHIP_QOS;
    if (args->parent._reliable)
    {
        rem_subscription_data.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    }

    /************************************************************************
     * Set up Command topic and writer.
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

    /* Configure command writer Quality of Service */
    return_code =
        configure_command_writer_qos(&throughput_command_writer_qos, publisher,
                                     args);
    if (return_code != DDS_RETCODE_OK)
    {
        AppLog_exception("Error in configuring command writer qos\n");
        goto done;
    }

    /*Create command writer */
    command_listener.on_publication_matched =
        ThroughputListener_on_publication_matched;
    command_listener.as_listener.listener_data = &num_command_subscriptions;
    throughput_command_writer = DDS_Publisher_create_datawriter(publisher,
                                                                throughput_command_topic,
                                                                &throughput_command_writer_qos,
                                                                &command_listener,
                                                                DDS_PUBLICATION_MATCHED_STATUS);
    if (throughput_command_writer == NULL)
    {
        AppLog_exception("Error in creating Command Writer\n");
        goto done;
    }

    rem_command_subscription_data.key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] =
        COMMAND_READER_OBJECT_ID;
    rem_command_subscription_data.topic_name =
        DDS_String_dup("ThroughputCommand");
    rem_command_subscription_data.type_name =
        DDS_String_dup(ThroughputCommandTYPENAME);
    rem_command_subscription_data.ownership.kind = DDS_SHARED_OWNERSHIP_QOS;
    rem_command_subscription_data.reliability.kind =
        DDS_RELIABLE_RELIABILITY_QOS;

    /* and create an initialized instance of command topic */
    throughput_command_instance = ThroughputCommandTypeSupport_create_data();
    if (throughput_command_instance == NULL)
    {
        AppLog_exception("Error in creating Command Instance\n");
        goto done;
    }

    for (i = 0; i < args->subscribers; i++)
    {
        char buffer[40];

        get_subscriber_name(buffer, i);

        return_code = DPSE_RemoteParticipant_assert(participant, buffer);
        if (return_code != DDS_RETCODE_OK)
        {
            AppLog_exception("failed to assert remote participant\n");
            goto done;
        }

        rem_subscription_data.key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] =
                                   DATA_READER_OBJECT_ID + i;
        return_code = DPSE_RemoteSubscription_assert(participant,
                                                     buffer,
                                                     &rem_subscription_data,
                                                     NDDS_TYPEPLUGIN_USER_KEY);
        if (return_code != DDS_RETCODE_OK)
        {
            AppLog_exception("failed to assert remote subscription\n");
            goto done;
        }

        rem_command_subscription_data.key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] =
                                   DATA_READER_OBJECT_ID + i;
        return_code = DPSE_RemoteSubscription_assert(participant,
                                                     buffer,
                                                     &rem_command_subscription_data,
                                                     NDDS_TYPEPLUGIN_NO_KEY);
        if (return_code != DDS_RETCODE_OK)
        {
            AppLog_exception("failed to assert remote subscription\n");
            goto done;
        }
    }

    /* Now we are free to enable the participant */
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

    /* Initialize the timer for the test */
    if (!TimeManager_initialize(&time_manager, args->test_duration_sec))
    {
        AppLog_exception("Error in initializing time manager\n");
        goto done;
    }

    if (!TimeManager_calculate_clock_overhead(&time_manager))
    {
        AppLog_exception("Error in calculating clock overhead\n");
    }

    /* Ensure we are sending the correct packet size */
    if (!CDR_OctetSeq_set_length(&throughput_instance->data, args->packet_size))
    {
        AppLog_exception("Error in setting correct data packet length\n");
        goto done;
    }

    AppLog_report("Starting test...\n");
    AppLog_report("with the following parameters...\n");
    ThroughputPublisherArgs_print_arguments(args);

    if (!DDS_InstanceHandleSeq_initialize(&subscription_handles))
    {
        AppLog_exception("Error in DDS_InstanceHandleSeq_initialize\n");
        goto done;
    }
    if (!DDS_InstanceHandleSeq_initialize(&command_subscription_handles))
    {
        AppLog_exception("Error in DDS_InstanceHandleSeq_initialize\n");
        goto done;
    }

    /* Wait until we have all subscribers matching command topics
     * and data topics */
    do
    {
        TimeManager_sleep(&time_manager,
                          (struct DDS_Duration_t *)&twenty_millisec);
    }
    while ((num_data_subscriptions < args->subscribers) ||
           (num_command_subscriptions < args->subscribers));

    AppLog_report
        (" bytes, demand,  samples, sample/s,  Mbit/s, duration, CPU %%, memory(kB)\n"
         "------, ------, --------, --------, -------, --------, -----, ----------\n");
    fflush(stdout);

    /* Allow time for subscribers to initialise */
    TimeManager_sleep(&time_manager, (struct DDS_Duration_t *)&one_second);

    for (load = args->demand_initial;
         load <= args->demand_max; load += args->demand_increment)
    {

        test_complete = DDS_BOOLEAN_FALSE;

        /* Set up initial Data */
        throughput_instance->sequence_number = 0;

        /* Setup command */
        throughput_command_instance->signature[0] = 0x43;       /* signature */
        throughput_command_instance->signature[1] = 0x21;
        throughput_command_instance->signature[2] = 0x43;
        throughput_command_instance->signature[3] = 0x21;

        throughput_command_instance->command = THROUGHPUT_COMMAND_START;
        throughput_command_instance->data_length = args->packet_size;
        throughput_command_instance->current_publisher_effort = load;
        throughput_command_instance->final_publisher_effort = args->demand_max;

        return_code = DDS_DataWriter_write(throughput_command_writer,
                                           throughput_command_instance,
                                           &DDS_HANDLE_NIL);
        if (return_code != DDS_RETCODE_OK)
        {
            AppLog_exception("\nCommand writer error, "
                             "TEST_START_COMMAND not issued!\n");
            goto done;
        }

        appCpuLoad = PerfMon_getCpu(&app_monitor);

        if (!TimeManager_get_start_time(&time_manager))
        {
            AppLog_exception("Error in getting Start Time. "
                             "Results may not be accurate\n");
            goto done;
        }

        while (!test_complete)
        {
            for (current_load = 0;
                 current_load < load && !test_complete;
                 ++current_load, ++throughput_instance->sequence_number)
            {

                return_code = DDS_DataWriter_write(throughput_writer,
                                                   throughput_instance,
                                                   &DDS_HANDLE_NIL);
                if (return_code != DDS_RETCODE_OK)
                {
                    if (return_code == DDS_RETCODE_TIMEOUT)
                    {
                        AppLog_exception("\nWrite Timeout please increase "
                                         "-maxBlockingTime parameter for "
                                         "test\n");
                        TimeManager_sleep(&time_manager,
                                          (struct DDS_Duration_t
                                           *)&ten_millisec);
                        --throughput_instance->sequence_number;
                        /* expected case so write the same packet again */
                    }
                    else
                    {
                        AppLog_exception("\nError while writing\n");
                        test_complete = DDS_BOOLEAN_TRUE;
                        break;
                    }
                }
            }
            test_complete = TimeManager_is_test_complete(&time_manager);

            if (!test_complete)
            {
                TimeManager_sleep(&time_manager, &recovery_time);
            }
        }

        appCpuLoad = PerfMon_getCpu(&app_monitor);
        throughput_command_instance->publisher_cpu_usage =
            (DDS_Float) appCpuLoad;

        throughput_command_instance->command = THROUGHPUT_COMMAND_COMPLETE;
        return_code = DDS_DataWriter_write(throughput_command_writer,
                                           throughput_command_instance,
                                           &DDS_HANDLE_NIL);
        if (return_code != DDS_RETCODE_OK)
        {
            AppLog_exception("\nCommand writer error, "
                             "TEST_COMPLETE_COMMAND not issued!\n");
            goto done;
        }

        if (!TimeManager_get_stop_time(&time_manager))
        {
            AppLog_exception("Error in getting Stop Time. "
                             "Results may not be accurate\n");
            goto done;
        }
        delta_time = TimeManager_get_delta_time(&time_manager);

        packets_written = throughput_instance->sequence_number;
        bytes_written = ((double)packets_written *
                         (THROUGHPUT_PACKET_OVERHEAD + args->packet_size));
        if (delta_time > 0)
        {
            publisher_throughput = ((8.0 * (double)bytes_written / delta_time) /
                                    1000000.0);
        }

        AppLog_report("%6d, %6d, %8lu, %8.1f, %7.1f, %8.2f, %5.2f, %10.1f\n",
                      (args->packet_size + THROUGHPUT_PACKET_OVERHEAD),
                      load,
                      packets_written,
                      (float)packets_written / delta_time,
                      publisher_throughput,
                      delta_time,
                      throughput_command_instance->publisher_cpu_usage,
                      (float)PerfMon_getMemory(&app_monitor) / (1024.0 *
                                                                1024.0));
        fflush(stdout);

        /* Let subscriber poll for COMPLETE command */
        TimeManager_sleep(&time_manager, (struct DDS_Duration_t *)&one_second);

        /* Let subscriber receive remaining of data */
        TimeManager_sleep(&time_manager,
                          (struct DDS_Duration_t *)&three_second);

    }                           /* for (load < maxLoad) */

    AppLog_report("Test Completed.\n");
    result = 0;

    done:

    if (participant != NULL)
    {
        publisher_shutdown(participant);
    }
    if (throughput_command_instance != NULL)
    {
        ThroughputCommandTypeSupport_delete_data(throughput_command_instance);
    }
    if (throughput_instance != NULL)
    {
        ThroughputTypeSupport_delete_data(throughput_instance);
    }
    TimeManager_delete(&time_manager);

    DDS_DomainParticipantFactoryQos_finalize(&factory_qos);
    DDS_DomainParticipantQos_finalize(&participant_qos);
    DDS_DataWriterQos_finalize(&throughput_writer_qos);
    DDS_DataWriterQos_finalize(&throughput_command_writer_qos);
    DDS_SubscriptionBuiltinTopicData_finalize(&rem_subscription_data);
    DDS_SubscriptionBuiltinTopicData_finalize(&rem_command_subscription_data);

    return result;
}




#if defined(RTI_WINCE)
int
wmain(int argc, wchar_t ** argv)
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

    ThroughputPublisherArgs args;
    if (argc <= 1)
    {
        ThroughputPublisherArgs_create(&args);
    }
    else
    {
        ThroughputPublisherArgs_create_with_string(&args, argc, argv_c);
    }

#elif !(defined(RTI_VXWORKS) && !defined(__RTP__)) && !defined(RTI_PSOS)
int
main(int argc, char *argv[])
{
    ThroughputPublisherArgs args;
    int ret_value;

    if (argc <= 1)
    {
        ThroughputPublisherArgs_create(&args);
    }
    else
    {
        ThroughputPublisherArgs_create_with_string(&args, argc, argv);
    }
#elif defined(RTI_VXWORKS)
int
publisher_main(void)
{
    ThroughputPublisherArgs args;
    int ret_value;
    ThroughputPublisherArgs_create(&args);
    /* add your peers here: */
    strncpy(args.parent._peer_hosts[0], "10.45.1.105",
            MAX_PEER_LOCATOR_STR_LEN);
    args.parent.max_peer_index = 1;
    args.parent.ndds_domain = 12;
    args.subscribers = 1;
    args.test_duration_sec = 10;
    args.packet_size = 32;
    args.demand_initial = 1000;
    args.demand_increment = 1000;
    args.demand_max = 9000;
    args.parent._reliable = RTI_TRUE;   /* set to RTI_TRUE to turn on reliable reliability */
#endif

    if (args.parent._help_requested)
    {
        ThroughputPublisherArgs_usage(&args);
        ThroughputPublisherArgs_delete(&args);
        return -1;
    }
    else if (!args.parent._args_valid)
    {
        ThroughputPublisherArgs_print_error(&args);
        ThroughputPublisherArgs_delete(&args);
        return -1;
    }

    ret_value = publisher_test_throughput(&args);
    ThroughputPublisherArgs_delete(&args);
    return ret_value;
}
