/*
 (c) Copyright, Real-Time Innovations, 2009-2015.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.
*/

/*=========================================================================*/

#include "LatencyExample.h"

int publisher_main_no_args(void);

/* ---------------------------------------------------------------------*/
#if 0
typedef struct LatencyEngine
{
    RtiDdsSenderCommunicator *_communicator;
    int _min_message_size, _max_message_size;
    DDS_Boolean _is_reliable;
    int _num_iter;
    int _subscribers;

    struct OSAPI_System *_clock;
    LatencyDataProcessor _data_processor;
    Latency _send_instance;
} LatencyEngine;

DDS_Boolean LatencyEngine_initialize(LatencyEngine * self,
                                     const char *command_line_string,
                                     int min_message_size, int max_message_size,
                                     DDS_Boolean is_reliable, int num_iter,
                                     int subscribers);
DDS_Boolean LatencyEngine_prepare_communicator(LatencyEngine * self,
                                               RtiDdsSenderCommunicator *
                                               communicator,
                                               DDS_Boolean
                                               measure_individual_latencies,
                                               const char* udp_intf);
DDS_Boolean LatencyEngine_execute_sending_loop(LatencyEngine * self,
                                               DDS_Boolean
                                               measure_individual_latencies);
DDS_Boolean LatencyEngine_finalize(LatencyEngine * self);

/* ---------------------------------------------------------------------*/
DDS_Boolean
LatencyEngine_prepare_communicator(LatencyEngine * self,
                                   RtiDdsSenderCommunicator * communicator,
                                   DDS_Boolean measure_individual_latencies,
                                   const char* udp_intf)
{
    self->_communicator = communicator;
    return RtiDdsSenderCommunicator_prepare(self->_communicator,
                                            &self->_send_instance,
                                            &self->_data_processor,
                                            measure_individual_latencies);
}

/* ---------------------------------------------------------------------*/
DDS_Boolean
LatencyEngine_initialize(LatencyEngine * self,
                         const char *command_line_string, int min_message_size,
                         int max_message_size, DDS_Boolean is_reliable,
                         int num_iter, int subscribers)
{
    /* check if user option greater than maximum message size
     ** of data + sequencenumber
     */
    if (max_message_size > (MAX_DATA_SEQUENCE_LENGTH + (int)sizeof(DDS_Long)))
    {
        AppLog_exception("invalid parameter; "
                         "max_message_size > max sequence length\n");
        return DDS_BOOLEAN_FALSE;
    }

    self->_min_message_size = min_message_size;
    self->_max_message_size = max_message_size;
    self->_is_reliable = is_reliable;
    self->_num_iter = num_iter;
    self->_subscribers = subscribers;

    if (!LatencyDataProcessor_initialize(
       &self->_data_processor, NULL /*self->_clock*/, self->_num_iter))
    {
        AppLog_exception("failed to initiate data processor\n");
        return DDS_BOOLEAN_FALSE;
    }

    /* calculate the overhead for clock->getTime() call */
    if (!LatencyDataProcessor_calculate_clock_overhead(&self->_data_processor))
    {
        AppLog_exception("failed to calculate clock overhead\n");
        return DDS_BOOLEAN_FALSE;
    }

    if (!CDR_OctetSeq_initialize(&self->_send_instance.data))
    {
        AppLog_exception("CDR_OctetSeq_initialize failed\n");
        return DDS_BOOLEAN_FALSE;
    }

    /* Size sequence to hold largest message */
    if (!CDR_OctetSeq_set_maximum
        (&self->_send_instance.data, (DDS_Long) self->_max_message_size))
    {
        AppLog_exception("CDR_OctetSeq_set_maximum failed\n");
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

/* ---------------------------------------------------------------------*/
DDS_Boolean
LatencyEngine_finalize(LatencyEngine * self)
{
    if (!RtiDdsSenderCommunicator_cleanup(self->_communicator))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!CDR_OctetSeq_finalize(&self->_send_instance.data))
    {
        AppLog_exception("CDR_OctetSeq_finalize failed\n");
        return DDS_BOOLEAN_FALSE;
    }

    LatencyDataProcessor_delete(&self->_data_processor);

    /* Close all files */
    return DDS_BOOLEAN_TRUE;
}

/* ---------------------------------------------------------------------*/
DDS_Boolean
LatencyEngine_execute_sending_loop(LatencyEngine * self,
                                   DDS_Boolean measure_individual_latencies)
{
    DDS_Boolean ok = DDS_BOOLEAN_FALSE;
    int sleep_time_ms = DEFAULT_SLEEP_TIME;
    int i = 0;
    int echoer_cookie;
    int m_size;
    int round_number;
    int fail_reason;
    DDS_Octet *data0;
    LatencyDataProcessor *data_processor;

    /* Wait for communicator to be ready */
    AppLog_warn("Waiting for %d receivers.", self->_subscribers);
    while (!RtiDdsSenderCommunicator_is_ready(self->_communicator))
    {
        AppLog_warn(".");
        /*** INTERNAL API: RTIOsapiSemaphore_XXX ***/
        if (!OSAPI_Semaphore_take(self->_communicator->sem, sleep_time_ms,
                                  &fail_reason))
        {
            /* wait loop */
        }
    }
    AppLog_warn("\n");
    echoer_cookie =
        RtiDdsSenderCommunicator_get_echoer_cookie(self->_communicator);
    if (echoer_cookie != LATENCY_RESTRICT_RECV_COOKIE_NONE)
    {
        AppLog_warn("Echoer restricted to the one in place #%d, identified to "
                    "have cookie %d.\n", self->_subscribers, echoer_cookie);
    }

    /* Echo to stdout the banner for the report */
    AppLog_report("Collecting statistics on %d samples per message size.\n"
                  "This is the roundtrip time, *not* the one-way-latency\n",
                  self->_num_iter);
    if (measure_individual_latencies)
    {
        AppLog_report
            ("bytes ,stdev us,ave us, min us, 50%% us, 90%% us, 99%% us, 99.99%%, max us\n");
        AppLog_report
            ("------,-------,-------,-------,-------,-------,-------,-------,-------\n");
    }
    else
    {
        AppLog_report("bytes , ave us\n");
        AppLog_report("------,-------\n");
    }


    data_processor = &self->_data_processor;
    for (m_size = self->_min_message_size, round_number = 0;
         m_size <= self->_max_message_size; m_size *= 2, ++round_number)
    {                           /* each round doubles payload */

        OSAPI_Thread_sleep(sleep_time_ms);
        if (!CDR_OctetSeq_set_length
            (&self->_send_instance.data, m_size - sizeof(DDS_Long)))
        {
            AppLog_exception("CDR_OctetSeq_set_length failed\n");
            return DDS_BOOLEAN_FALSE;
        }

        LatencyDataProcessor_start_one_round(data_processor, m_size);
        data0 = CDR_OctetSeq_get_reference(&self->_send_instance.data, 0);
        *data0 = echoer_cookie;
        if (measure_individual_latencies)
        {
            for (i = 0; i <= self->_num_iter;)
            {
                self->_send_instance.sequence_number =
                    data_processor->_sequence_number;
                LatencyDataProcessor_start_one_issue(data_processor);

                /* send the raw data to all interested parties */
                ok = RtiDdsCommunicator_send_data(&self->_communicator->parent);

                if (!ok)
                {
                    AppLog_exception("failed to send data\n");
                    return DDS_BOOLEAN_FALSE;
                }

                /* wait until the echo message is received 
                 ** from the LatencyReader 
                 */
                /*** INTERNAL API: RTIOsapiSemaphore_XXX ***/
                if (!OSAPI_Semaphore_take(self->_communicator->sem, 
                                          sleep_time_ms,
                                          &fail_reason))
                {
                    /* wait loop */
                }
                if (LatencyDataProcessor_is_finished(data_processor))
                {               /* this packet exchange succeeded */
                    ++i;
                }
                else
                {               /* warn, sleep, and retry rather than stopping */
                    AppLog_exception("STOPPING SEND: did not receive echo "
                                     "in reasonable time "
                                     "(messageSize = %d, loop = %d)\n",
                                     m_size, i);
                    OSAPI_Thread_sleep(sleep_time_ms); 
                }
            }                   /* end of for (i) */
            /* one round (m_size) finished. print out the result */
            LatencyDataProcessor_finish_one_round(data_processor);
        }
        else
        {                       /* !measure_individual_latencies */
            if (!OSAPI_System_get_time(&data_processor->_start_time))
            {
                AppLog_exception("Failed to get time\n");
            }
            for (i = 0; i <= self->_num_iter;)
            {
                self->_send_instance.sequence_number =
                    data_processor->_sequence_number;

                /* send the raw data to all interested parties */
                ok = RtiDdsCommunicator_send_data(&self->_communicator->parent);

                if (!ok)
                {
                    AppLog_exception("failed to send data\n");
                    return DDS_BOOLEAN_FALSE;
                }

                /* wait until the echo message is received 
                 ** from the LatencyReader 
                 */
                /*** INTERNAL API: RTIOsapiSemaphore_XXX ***/
                if (!OSAPI_Semaphore_take(self->_communicator->sem, 
                                          sleep_time_ms,
                                          &fail_reason))
                {
                    /* wait loop */
                }
                if (data_processor->_got_valid_echo)
                {               /* this packet exchange succeeded */
                    ++i;
                }
                else
                {               /* warn, sleep, and retry rather than stopping */
                    AppLog_exception("STOPPING SEND: did not receive echo "
                                     "in reasonable time "
                                     "(messageSize = %d, loop = %d)\n",
                                     m_size, i);
                    OSAPI_Thread_sleep(sleep_time_ms);
                }
            }                   /* end of for (i) */
            if (!OSAPI_System_get_time(&data_processor->_finish_time))
            {
                AppLog_exception("Failed to get time\n");
            }
            /* one round (m_size) finished. print out the result */
            LatencyDataProcessor_finish_one_round_average_only(data_processor);
        }
    }                           /* end of for (m_size) */

    /* Set to final sequence number */
    self->_send_instance.sequence_number = FINAL_SEQUENCE_NUMBER;
    if (!CDR_OctetSeq_set_length(&self->_send_instance.data, 16))
    {
        AppLog_exception("CDR_OctetSeq_set_length failed\n");
        return DDS_BOOLEAN_FALSE;
    }

    /* send to reader to terminate the reader */
    ok = RtiDdsCommunicator_send_data(&self->_communicator->parent);

    if (!ok)
    {
        AppLog_exception("failed to send data\n");
    }

    return ok;
}
#endif


/* ---------------------------------------------------------------------*/
static DDS_Boolean
publisher_main(int rti_dds_domain, char *peer,
               int min_message_size, int max_message_size,
               DDS_Boolean is_reliable,
               DDS_Boolean measure_individual_latencies_in,
               DDS_Boolean use_key_in_topic,
               int num_iter,
               int event_thread_priority,
               int subscribers,
               const char *command_line_string,
               char* udp_intf)
{
    int sleep_time_ms = DEFAULT_SLEEP_TIME;
    DDS_Boolean ok = DDS_BOOLEAN_FALSE;
    LatencyEngine latency_engine;
    RtiDdsSenderCommunicator communicator;
    int transport_priority = 0;
    struct OSAPI_NtpTime resolution;
    long resolution_sec;
    unsigned long resolution_usec;
    DDS_Boolean measure_individual_latencies = DDS_BOOLEAN_TRUE;

    ok = LatencyEngine_initialize(&latency_engine, command_line_string,
                                  min_message_size, max_message_size,
                                  is_reliable, num_iter, subscribers);
    if (!ok)
    {
        AppLog_exception("failed to initialize the engine\n");
        return DDS_BOOLEAN_FALSE;
    }

    /* take multiple measurements to determine latency */
     measure_individual_latencies = DDS_BOOLEAN_FALSE;
#if 0
    if (!measure_individual_latencies_in)
    {
        measure_individual_latencies = DDS_BOOLEAN_FALSE;
    }
    else
    {
        if (!OSAPI_System_getResolution(latency_engine._clock, &resolution))
        {
            AppLog_exception("failed to get clock resolution\n");
            return DDS_BOOLEAN_FALSE;
        }
        OSAPI_NtpTime_unpackToMicrosec(&resolution_sec, &resolution_usec,
                                      &resolution);
        if (resolution_usec > 1)
        {
            AppLog_report("clock resolution = %u us, too coarse for measuring "
                          "individual latencies, so measuring many latencies and taking "
                          "an average.\n", resolution_usec);
            measure_individual_latencies = DDS_BOOLEAN_FALSE;
        }
    }
#endif

    RtiDdsSenderCommunicator_initialize(&communicator, rti_dds_domain, peer,
                                    is_reliable, use_key_in_topic,
                                    event_thread_priority, DDS_BOOLEAN_FALSE
                                    /* start in not-ready state */ ,
                                    subscribers, udp_intf);

    if (!LatencyEngine_prepare_communicator
        (&latency_engine, &communicator, measure_individual_latencies))
    {
        AppLog_exception("failed to prepare communicator\n");
        return DDS_BOOLEAN_FALSE;
    }

    /* --------------------------------------------------------------------- */
    ok = LatencyEngine_execute_sending_loop(&latency_engine,
                                            measure_individual_latencies);
    if (!ok)
    {
        AppLog_exception("failed to execute sending of data\n");
        return DDS_BOOLEAN_FALSE;
    }

    /* Give any started threads a chance to exit before cleaning up things */
    OSAPI_Thread_sleep(sleep_time_ms);

    /* Cleanup the engine */
    ok = LatencyEngine_finalize(&latency_engine);
    if (!ok)
    {
        AppLog_exception("failed to finalize data structures\n");
        return DDS_BOOLEAN_FALSE;
    }

    if (ok)
    {
        AppLog_warn("Test successful!\n");
    }
    else
    {
        AppLog_exception("Test NOT successful!\n");
    }


    /* clean-up communicator */
    RtiDdsSenderCommunicator_finalize(&communicator);


    return DDS_BOOLEAN_TRUE;
}


/* ---------------------------------------------------------------------*/
#if !(defined(RTI_VXWORKS) && !defined(__RTP__)) && !defined(RTI_PSOS)
int
main(int argc, char **argv)
{
    int min_message_size = 16;
    int max_message_size = MAX_DATA_SEQUENCE_LENGTH;
    int rti_dds_domain = 0;
    char *peer = NULL;
    char *udp_intf = NULL;
    DDS_Boolean is_reliable = DDS_BOOLEAN_FALSE;
    DDS_Boolean measure_individual_latencies = DDS_BOOLEAN_TRUE;
    DDS_Boolean use_keys = DDS_BOOLEAN_FALSE;
    int num_iter = 10000;
    char *colon_pos = NULL;

    /*** INTERNAL API: RTI_OSAPI_XXX ***/
    int event_thread_priority = OSAPI_THREAD_PRIORITY_BELOW_NORMAL;
    char command_line[1024];
    int i;

    /* Assign default number of receivers to wait for */
    int subscribers = LATENCY_WAIT_RECV_COUNT_DEFAULT;

    const char *usage_str =
        "Usage:\n"
        "       %s  [commonoptions] [rtiddsoptions]\n"
        "\nWhere [commonoptions] are:\n"
        "\t-help                           "
        "- Print this usage message and exit\n"
        "\t-minSize       <size>           "
        "- Set minimum payload size (def. 16B)\n"
        "\t-maxSize       <size>           "
        "- Set maximum payload size (def. 8KB)\n"
        "\t-numIter       <count>          "
        "- Number of iterations per size.\n"
        "\t-subscribers   <recvCount>      "
        "- Wait for the presence of receivers\n"
        "\nWhere [rtiddsoptions] are:\n"
        "\t-domainId      <domainId>      "
        "- RTI DDS domain\n"
        "\t-peer        <peer>          "
        "- Peer Host IP Address\n"
        "\t-reliable                    "
        "- Use reliable service\n"
        "\t-average                    "
        "- Only measure the average latency\n"
        "\t-eventThreadPriority <prio>  "
        "- Set priority of the RTI DDS event thread\n"
        "\t-udp_intf <intf>             - udp interface (no default)\n";

    AppLog_warn("\nRTI DDS Latency Test - Publisher\n"
                "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

#if defined(__RTP__)
    AppLog_exception
        ("You cannot run the Latency Publisher in RTP mode on VxWorks\n"
         "Please read the Instructions for further details\n");
    exit(1);
#endif

    if (argc <= 1)
    {
        return publisher_main_no_args();
    }
    else
    {

        strcpy(command_line, argv[0]);
        for (i = 1; i < argc; i++)
        {
            strcat(command_line, " ");
            strcat(command_line, argv[i]);
        }

        for (i = 1; i < argc; i++)
        {
            if (IS_OPTION(argv[i], "-help"))
            {
                AppLog_exception(usage_str, argv[0]);
                return 0;
            }
            else if (IS_OPTION(argv[i], "-domainId"))
            {
                if ((i == (argc - 1)) || *argv[++i] == '-')
                {
                    AppLog_exception("Missing <id> after -domainId\n");
                    return 0;
                }
                rti_dds_domain = strtol(argv[i], NULL, 10);
            }
            else if (IS_OPTION(argv[i], "-minSize"))
            {
                if ((i == (argc - 1)) || *argv[++i] == '-')
                {
                    AppLog_exception("Missing <size> after -minSize\n");
                    return 0;
                }
                min_message_size = strtol(argv[i], NULL, 10);
            }
            else if (IS_OPTION(argv[i], "-maxSize"))
            {
                if ((i == (argc - 1)) || *argv[++i] == '-')
                {
                    AppLog_exception("Missing <size> after -maxSize\n");
                    return 0;
                }
                max_message_size = strtol(argv[i], NULL, 10);
            }
            else if (IS_OPTION(argv[i], "-reliable"))
            {
                is_reliable = DDS_BOOLEAN_TRUE;
            }
            else if (IS_OPTION(argv[i], "-average"))
            {
                measure_individual_latencies = DDS_BOOLEAN_FALSE;
            }
            else if (IS_OPTION(argv[i], "-use_keys"))
            {
                use_keys = DDS_BOOLEAN_TRUE;
            }
            else if (IS_OPTION(argv[i], "-numIter"))
            {
                if ((i == (argc - 1)) || *argv[++i] == '-')
                {
                    AppLog_exception("Missing <iter> after -numIter\n");
                    return 0;
                }
                num_iter = strtol(argv[i], NULL, 10);
            }
            else if (IS_OPTION(argv[i], "-eventThreadPriority"))
            {
                if ((i == (argc - 1)) || *argv[++i] == '-')
                {
                    AppLog_exception("Missing <prio> after "
                                     "-eventThreadPriority\n");
                    return 0;
                }
                event_thread_priority = strtol(argv[i], NULL, 10);
            }
            else if (IS_OPTION(argv[i], "-subscribers"))
            {
                if ((i == (argc - 1)) || *argv[++i] == '-')
                {
                    AppLog_exception("Missing <count> after -subscribers\n");
                    return 0;
                }
                subscribers = strtol(argv[i], NULL, 10);
            }
            else if (IS_OPTION(argv[i], "-peer"))
            {
                if ((i == (argc - 1)) || *argv[++i] == '-')
                {
                    AppLog_exception("Missing value after -peer\n");
                    return 0;
                }
                peer = argv[i];
            }
            else if (IS_OPTION(argv[i], "-udp_intf"))
            {
                if ((i == (argc - 1)) || *argv[++i] == '-')
                {
                    AppLog_exception("Missing value after -udp_intf\n");
                    return 0;
                }
                udp_intf = argv[i];
            }
            else
            {
                AppLog_exception("Unrecognized option \"%s\"\n", argv[i]);
                AppLog_exception("           use the '-help' option "
                                 "to see the usage\n");
                return 0;
            }
        }

        return publisher_main(rti_dds_domain, peer, min_message_size,
                              max_message_size, is_reliable,
                              measure_individual_latencies, use_keys, num_iter,
                              event_thread_priority, subscribers, command_line, udp_intf);

    }                           /*if (argc <= 1) */
}
#endif

/*This function will allow you to run the example by hard-coding the arguments*/
int
publisher_main_no_args(void)
{

    char *peer = DEFAULT_INITIAL_PEER_SUB;
    int domainId = DEFAULT_DOMAIN_ID;

    int rti_dds_part_index = -1;

    int min_message_size = MIN_DATA_SEQUENCE_LENGTH;
    int max_message_size = MAX_DATA_SEQUENCE_LENGTH;
    int num_iter = 100000;

    DDS_Boolean echo_disabled = DDS_BOOLEAN_FALSE;
    DDS_Boolean is_reliable = DDS_BOOLEAN_FALSE;
    DDS_Boolean measure_individual_latencies = DDS_BOOLEAN_TRUE;
    DDS_Boolean use_keys = DDS_BOOLEAN_FALSE;
    char *colon_pos = NULL;
    int subscribers = 1;

    /*** INTERNAL API: RTI_OSAPI_XXX ***/
    int event_thread_priority = OSAPI_THREAD_PRIORITY_BELOW_NORMAL;

    AppLog_warn("WARNING: running publisher_main_no_args function\n"
                    "      make sure the hard-coded arguments will\n"
                    "         fit your system and scenario\n");


    return publisher_main(domainId, peer, min_message_size, max_message_size,
                          is_reliable, measure_individual_latencies, use_keys,
                          num_iter, event_thread_priority, subscribers, NULL, NULL);
}

#if defined(RTI_WINCE)
int
wmain(int argc, wchar_t * argv[])
{
    char arg_array[MAX_COMMAND_LINE_ARGUMENTS][MAX_COMMAND_EXEC_LEN];
    char *argv_c[MAX_COMMAND_LINE_ARGUMENTS];
    int i;

    if (argc <= 1)
    {
        return publisher_main_no_args();
    }
    else
    {
        for (i = 0; i < argc; ++i)
        {
            wcstombs(arg_array[i], argv[i], wcslen(argv[i]) + 1);
            arg_array[i][wcslen(argv[i])] = '\0';
            argv_c[i] = arg_array[i];
        }
        argv_c[argc] = '\0';
        return main(argc, argv_c);
    }
}
#endif
