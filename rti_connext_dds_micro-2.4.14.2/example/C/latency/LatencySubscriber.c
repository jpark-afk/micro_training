/*
 (c) Copyright, Real-Time Innovations, $Date: 2009-2015/05/17 03:01:09 $.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.
*/

/*=======================================================================*/

#include "LatencyExample.h"

int subscriber_main_no_args(void);

static DDS_Boolean
subscriber_main(int rti_dds_domain, int rti_dds_part_index, char *peer,
                int cookie, DDS_Boolean is_reliable, DDS_Boolean use_keys,
                int event_thread_priority, DDS_Boolean echo_disabled, char* udp_intf)
{
    RtiDdsReceiverCommunicator communicator;
    int sleep_time_ms = DEFAULT_SLEEP_TIME;
    Latency send_instance;

    RtiDdsReceiverCommunicator_initialize(&communicator, rti_dds_domain,
                                      rti_dds_part_index, peer, is_reliable,
                                      use_keys, event_thread_priority, cookie,
                                      echo_disabled, udp_intf);

    /* set up the data buffer for sending/receiving the message */
    if (!Latency_initialize(&send_instance))
    {
        AppLog_exception("LatencyTypeSupport_initialize_sample failed\n");
        return DDS_BOOLEAN_FALSE;
    }

    if (!RtiDdsReceiverCommunicator_prepare(&communicator, &send_instance, NULL))
    {
        AppLog_exception("failed to prepare communications\n");
        return DDS_BOOLEAN_FALSE;
    }

    /* Leave commented for Latency Test.
     ** Causes communicator to start a thread
     */
    /* communicator->runReceiverThread(); */

    /* passively receive issue and echo back */
    while (!RtiDdsReceiverCommunicator_has_received_sentinel(&communicator))
    {
        OSAPI_Thread_sleep(sleep_time_ms);
    }

    AppLog_warn("Test successful: %d messages received, %d replies sent.\n",
                RtiDdsReceiverCommunicator_recv_message_count(&communicator),
                RtiDdsReceiverCommunicator_sent_message_count(&communicator));

    if (!RtiDdsReceiverCommunicator_cleanup(&communicator))
    {
        AppLog_exception("failed to cleanup receiver\n");
        return DDS_BOOLEAN_FALSE;
    }

    if (!CDR_OctetSeq_finalize(&send_instance.data))
    {
        AppLog_exception("CDR_OctetSeq_finalize failed\n");
        return DDS_BOOLEAN_FALSE;
    }

    /* clean-up communicator */
    RtiDdsReceiverCommunicator_finalize(&communicator);

    return DDS_BOOLEAN_TRUE;
}

#if !(defined(RTI_VXWORKS) && !defined(__RTP__)) && !defined(RTI_PSOS)
int
main(int argc, char *argv[])
{
    int rti_dds_domain = DEFAULT_DOMAIN_ID;
    /* Use default => (cookie != 0)?(cookie %50):1 */
    int rti_dds_part_index = -1;
    int cookie = 1, i;
    char *peer = NULL, *nic = NULL, *udp_intf = NULL;
    DDS_Boolean echo_disabled = DDS_BOOLEAN_FALSE;
    DDS_Boolean is_reliable = DDS_BOOLEAN_FALSE;
    DDS_Boolean use_keys = DDS_BOOLEAN_FALSE;
    char *colon_pos = NULL;

    /*** INTERNAL API: RTI_OSAPI_XXX ***/
    int event_thread_priority = OSAPI_THREAD_PRIORITY_BELOW_NORMAL;

    const char *usage_str =
        "Usage:\n"
        "       %s  [commonoptions] [rtiddsoptions]\n"
        "\nWhere [commonoptions] are:\n"
        "\t-help                 "
        "- Print this usage message and exit\n"
        "\t-cookie      <cookie> "
        "- Globally-unique number among subscribers: 1..255\n"
        "\t                        "
        "Allows publisher to restrict reply to a single\n"
        "\t                        subscriber. "
        "(cookie==0) ==> do not restrict\n"
        "\t-noecho               "
        "- Do not echo. Overrides the use of the cookie\n"
        "\nWhere [rtiddsoptions] are:\n"
        "\t-domainId              <id>    "
        "- RTI DDS domain (range 0..100)\n"
        "\t-peer                <peer>  "
        "- Peer Host IP Address\n"
        "\t-index               <index> "
        "- RTI DDS Participant index (range 0..50)\n"
        "\t-reliable                    "
        "- Use reliable service\n"
        "\t-eventThreadPriority <prio>  "
        "- Set priority of the RTI DDS event thread\n"
        "\t-udp_intf <intf>             - udp interface (no default)\n";

    AppLog_warn("\nRTI DDS Latency Test - Subscriber\n");
    AppLog_warn("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

    if (argc <= 1)
    {
        return subscriber_main_no_args();
    }
    else
    {

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
            else if (IS_OPTION(argv[i], "-index"))
            {
                if ((i == (argc - 1)) || *argv[++i] == '-')
                {
                    AppLog_exception("Missing <index> after -index\n");
                    return 0;
                }
                rti_dds_part_index = strtol(argv[i], NULL, 10);
            }
            else if (IS_OPTION(argv[i], "-cookie"))
            {
                if ((i == (argc - 1)) || *argv[++i] == '-')
                {
                    AppLog_exception("Missing <cookie> after -cookie\n");
                    return 0;
                }
                cookie = strtol(argv[i], NULL, 10);
            }
            else if (IS_OPTION(argv[i], "-noecho"))
            {
                echo_disabled = DDS_BOOLEAN_TRUE;
            }
            else if (IS_OPTION(argv[i], "-reliable"))
            {
                is_reliable = DDS_BOOLEAN_TRUE;
            }
            else if (IS_OPTION(argv[i], "-use_keys"))
            {
                use_keys = DDS_BOOLEAN_TRUE;
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

        if (rti_dds_part_index == -1)
        {
            rti_dds_part_index = (cookie != 0) ? (cookie % 50) : 1;
        }

        if ((rti_dds_part_index < 1) || (rti_dds_part_index > 50))
        {
            AppLog_exception("Out of range value specified for option -index.\n"
                             "           Specified value was %d, valid range is "
                             "between 1 and 50\n", rti_dds_part_index);
            return 0;
        }

        if ((cookie < 0) || (cookie > 255))
        {
            AppLog_exception
                ("Out of range value specified for option -cookie.\n"
                 "           Specified value was %d, valid range is "
                 "between 0 and 255\n", cookie);
            return 0;
        }

        return subscriber_main(rti_dds_domain, rti_dds_part_index, peer, cookie,
                               is_reliable, use_keys, event_thread_priority,
                               echo_disabled, udp_intf);
    }                           /*if (argc <= 1) */
}
#endif

/*This function will allow you to run the example by hard-coding the arguments*/
int
subscriber_main_no_args(void)
{

    char *peer = DEFAULT_INITIAL_PEER_SUB;

    int rti_dds_part_index = -1;
    int cookie = 1;
    int domainId = DEFAULT_DOMAIN_ID;

    DDS_Boolean echo_disabled = DDS_BOOLEAN_FALSE;
    DDS_Boolean is_reliable = DDS_BOOLEAN_FALSE;
    DDS_Boolean use_keys = DDS_BOOLEAN_FALSE;
    char *colon_pos = NULL;

    /*** INTERNAL API: RTI_OSAPI_XXX ***/
    int event_thread_priority = OSAPI_THREAD_PRIORITY_BELOW_NORMAL;

    AppLog_warn("WARNING: running subscriber_main_no_args function\n"
                "      make sure the hard-coded arguments will\n"
                "         fit your system and scenario\n");


    return subscriber_main(domainId, rti_dds_part_index, peer, cookie,
                           is_reliable, use_keys, event_thread_priority,
                           echo_disabled, NULL);
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
        return subscriber_main_no_args();
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
