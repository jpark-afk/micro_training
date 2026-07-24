/*********************************************************************************************
Copyright (c) 2017-2025 Real-Time Innovations, Inc.  All rights reserved.    	
Permission to modify and use for internal purposes granted.   	
This software is provided "as is", without warranty, express or implied.                                                                            
**********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rti_me_c.h"
#include "wh_sm/wh_sm_history.h"
#include "rh_sm/rh_sm_history.h"

#include "HelloWorld.h"
#include "HelloWorldSupport.h"
#include "HelloWorldApplication.h"

static void
HelloWorldPublisher_on_publication_matched(void *listener_data,
                                           DDS_DataWriter *writer,
                                           const struct
                                           DDS_PublicationMatchedStatus *status)
{
    UNUSED_ARG(listener_data);
    UNUSED_ARG(writer);

    if (status->current_count_change > 0)
    {
        printf("Matched a subscriber\n");
    }
    else if (status->current_count_change < 0)
    {
        printf("Unmatched a subscriber\n");
    }
}

static int
publisher_main_w_args(DDS_Long domain_id, char *udp_intf1, char *udp_intf2,
                      char *peer, DDS_Long sleep_time, DDS_Long count)
{
    DDS_Publisher *publisher;
    DDS_DataWriter *datawriter;
    HelloWorldDataWriter *hw_datawriter;
    struct DDS_DataWriterQos dw_qos = DDS_DataWriterQos_INITIALIZER;
    DDS_ReturnCode_t retcode;
    HelloWorld *sample = NULL;
    struct Application *application = NULL;
    DDS_Long i;
    struct DDS_DataWriterListener dw_listener =
        DDS_DataWriterListener_INITIALIZER;

    sample = HelloWorld_create();
    if (sample == NULL)
    {
        printf("failed HelloWorld_create\n");
        return 0;
    }

    application = Application_create("publisher", "subscriber", domain_id,
                                     udp_intf1, udp_intf2, peer, sleep_time, count);

    if (application == NULL)
    {
        printf("failed Application create\n");
        goto done;
    }

    publisher = DDS_DomainParticipant_create_publisher(application->participant,
                                                       &DDS_PUBLISHER_QOS_DEFAULT,
                                                       NULL,
                                                       DDS_STATUS_MASK_NONE);
    if (publisher == NULL)
    {
        printf("publisher == NULL\n");
        goto done;
    }

#ifdef USE_RELIABLE_QOS
    dw_qos.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
#else
    dw_qos.reliability.kind = DDS_BEST_EFFORT_RELIABILITY_QOS;
#endif
    dw_qos.resource_limits.max_samples_per_instance = 32;
    dw_qos.resource_limits.max_instances = 2;
    dw_qos.resource_limits.max_samples = dw_qos.resource_limits.max_instances *
                                dw_qos.resource_limits.max_samples_per_instance;
    dw_qos.history.depth = 32;
    dw_qos.protocol.rtps_reliable_writer.heartbeat_period.sec = 0;
    dw_qos.protocol.rtps_reliable_writer.heartbeat_period.nanosec = 250000000;

    dw_listener.on_publication_matched =
        HelloWorldPublisher_on_publication_matched;
    datawriter =
        DDS_Publisher_create_datawriter(publisher, application->topic, &dw_qos,
                                        &dw_listener,
                                        DDS_PUBLICATION_MATCHED_STATUS);

    if (datawriter == NULL)
    {
        printf("datawriter == NULL\n");
        goto done;
    }

    retcode = Application_enable(application);
    if (retcode != DDS_RETCODE_OK)
    {
        printf("failed to enable application\n");
        goto done;
    }

    hw_datawriter = HelloWorldDataWriter_narrow(datawriter);

    for (i = 0; (application->count > 0 && i < application->count) ||
         (application->count == 0); ++i)
    {
        /* two instances will be created, id = 0 and id = 1 */
        sample->id = (i % 2);
        sprintf(sample->msg, "Hello World! (%d).", i);
        printf("%s\n", sample->msg);

        retcode = HelloWorldDataWriter_write(hw_datawriter, sample, &DDS_HANDLE_NIL);
        if (retcode != DDS_RETCODE_OK)
        {
            printf("Failed to write to sample\n");
        }

        OSAPI_Thread_sleep((RTI_UINT32)application->sleep_time);
    }

  done:

    Application_delete(application);

    DDS_DataWriterQos_finalize(&dw_qos);

    if (sample != NULL)
    {
        HelloWorld_delete(sample);
    }

    return 0;
}

#if !(defined(RTI_VXWORKS) && !defined(__RTP__))
int
main(int argc, char **argv)
{

    DDS_Long i = 0;
    DDS_Long domain_id = 0;
    char *peer = NULL;
    char *udp_intf1 = NULL;
    char *udp_intf2 = NULL;
    DDS_Long sleep_time = 1000;
    DDS_Long count = 0;

    for (i = 1; i < argc; ++i)
    {
        if (!strcmp(argv[i], "-domain"))
        {
            ++i;
            if (i == argc)
            {
                printf("-domain <domain_id>\n");
                return -1;
            }
            domain_id = (DDS_Long)strtol(argv[i], NULL, 0);
        }
        else if (!strcmp(argv[i], "-udp_intf1"))
        {
            ++i;
            if (i == argc)
            {
                printf("-udp_intf1 <interface>\n");
                return -1;
            }
            udp_intf1 = argv[i];
        }
        else if (!strcmp(argv[i], "-udp_intf2"))
        {
            ++i;
            if (i == argc)
            {
                printf("-udp_intf2 <interface>\n");
                return -1;
            }
            udp_intf2 = argv[i];
        }
        else if (!strcmp(argv[i], "-peer"))
        {
            ++i;
            if (i == argc)
            {
                printf("-peer <address>\n");
                return -1;
            }
            peer = argv[i];
        }
        else if (!strcmp(argv[i], "-sleep"))
        {
            ++i;
            if (i == argc)
            {
                printf("-sleep_time <sleep_time>\n");
                return -1;
            }
            sleep_time = (DDS_Long)strtol(argv[i], NULL, 0);
        }
        else if (!strcmp(argv[i], "-count"))
        {
            ++i;
            if (i == argc)
            {
                printf("-count <count>\n");
                return -1;
            }
            count = (DDS_Long)strtol(argv[i], NULL, 0);
        }
        else if (!strcmp(argv[i], "-h"))
        {
            Application_help(argv[0]);
            return 0;
        }
        else
        {
            printf("unknown option: %s\n", argv[i]);
            return -1;
        }
    }

    return publisher_main_w_args(domain_id, udp_intf1, udp_intf2,
                                 peer, sleep_time, count);
}
#elif defined(RTI_VXWORKS)
static int
publisher_main(void)
{
    /* Explicitly configure args below */
    DDS_Long i = 0;
    DDS_Long domain_id = 44;
    char *peer = NULL; /* set correct peer, e.g. "10.10.30.101" */
    char *udp_intf1 = NULL;
    char *udp_intf2 = NULL;
    DDS_Long sleep_time = 1000;
    DDS_Long count = 0;

    return publisher_main_w_args(domain_id, udp_intf1, udp_intf2, peer, sleep_time, count);
}
#endif
