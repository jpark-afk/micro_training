/*
 * Copyright (c) 2017-2024 Real-Time Innovations, Inc.  All rights reserved.
 * Permission to modify and use for internal purposes granted.
 * This software is provided "as is", without warranty, express or implied.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef rti_me_cpp_hxx
#include "rti_me_cpp.hxx"
#endif

#include "wh_sm/wh_sm_history.h"
#include "rh_sm/rh_sm_history.h"

#include "HelloWorld.h"
#include "HelloWorldSupport.h"
#include "HelloWorldApplication.h"


class HelloWorldDataWriterListener : public DDSDataWriterListener
{
public:
    HelloWorldDataWriterListener() : DDSDataWriterListener() { }
    ~HelloWorldDataWriterListener() { }
};


int
publisher_main_w_args(DDS_Long domain_id, char *udp_intf1, char *udp_intf2, 
                      char *peer, DDS_Long sleep_time, DDS_Long count)
{
    Application *application = NULL;

    DDSPublisher *publisher = NULL;
    DDSDataWriter *datawriter = NULL;
    HelloWorldDataWriter *hw_writer = NULL;
    DDS_DataWriterQos dw_qos;
    DDS_ReturnCode_t retcode;
    HelloWorld *sample = NULL;
    DDS_Long i;
    DDSDataWriterListener *dw_listener = NULL;

    sample = HelloWorldTypeSupport::create_data();
    if (sample == NULL)
    {
        printf("failed HelloWorldTypeSupport::create_data\n");
        return 0;
    }

    application = new Application();
    if (application == NULL)
    {
        printf("failed Application new\n");
        goto done;
    }

    retcode = application->initialize("publisher", 
                                      "subscriber", 
                                      domain_id,
                                      udp_intf1, 
                                      udp_intf2, 
                                      peer, 
                                      sleep_time, 
                                      count);
    if (retcode != DDS_RETCODE_OK)
    {
        printf("failed Application initialize\n");
        goto done;
    }


    publisher = application->participant->create_publisher(
                        DDS_PUBLISHER_QOS_DEFAULT,NULL,DDS_STATUS_MASK_NONE);
    if (publisher == NULL)
    {
        printf("publisher == NULL\n");
        goto done;
    }

    retcode = publisher->get_default_datawriter_qos(dw_qos);
    if (retcode != DDS_RETCODE_OK)
    {
        printf("failed get_default_datawriter_qos\n");
        goto done;
    }

#ifdef USE_RELIABLE_QOS
    dw_qos.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
#else
    dw_qos.reliability.kind = DDS_BEST_EFFORT_RELIABILITY_QOS;
#endif
    dw_qos.resource_limits.max_samples = 32;
    dw_qos.resource_limits.max_samples_per_instance = 32;
    dw_qos.resource_limits.max_instances = 1;
    dw_qos.history.depth = 32;

    /* INTEROPERABILITY NOTE:
       Non-Connext Micro DDS readers will have default LivelinessQoS kind
       set to DDS_AUTOMATIC_LIVELINESS_QOS.
     
       Because Connext Micro currently only supports
       DDS_MANUAL_BY_TOPIC_LIVELINESS_QOS, the non-Connext Micro reader will need
       to set its liveliness kind to DDS_MANUAL_BY_TOPIC_LIVELINESS_QOS as well,
       in order for Requested-Offered (RxO) semantics to be compatible.
    */
 
    datawriter = publisher->create_datawriter(application->topic,
                                              dw_qos,NULL,DDS_STATUS_MASK_NONE);
    if (datawriter == NULL)
    {
        printf("datawriter == NULL\n");
        goto done;
    }

    retcode = application->enable();
    if (retcode != DDS_RETCODE_OK)
    {
        printf("failed to enable application\n");
        goto done;
    }

    hw_writer = HelloWorldDataWriter::narrow(datawriter);
    if (hw_writer == NULL)
    {
        printf("failed datawriter narrow\n");
        goto done;
    }

    for (i = 0;
        (application->count > 0 && i < application->count) ||
        (application->count == 0);
        ++i)
    {
    sprintf(sample->msg, "Hello World (%d)", i);
        printf("%s\n", sample->msg);

        retcode = hw_writer->write(*sample, DDS_HANDLE_NIL);
        if (retcode != DDS_RETCODE_OK)
        {
            printf("Failed to write to sample\n");
        }

        OSAPI_Thread_sleep(application->sleep_time);
    }

  done:

    if (application != NULL)
    {
        delete application;
    }

    if (sample != NULL)
    {
        HelloWorldTypeSupport::delete_data(sample);
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
            domain_id = strtol(argv[i], NULL, 0);
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
            sleep_time = strtol(argv[i], NULL, 0);
        }
        else if (!strcmp(argv[i], "-count"))
        {
            ++i;
            if (i == argc)
            {
                printf("-count <count>\n");
                return -1;
            }
            count = strtol(argv[i], NULL, 0);
        }
        else if (!strcmp(argv[i], "-h"))
        {
            Application::help(argv[0]);
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
int
publisher_main(void)
{
    /* Explicitly configure args below */
    DDS_Long i = 0;
    DDS_Long domain_id = 44;
    char *peer = "10.10.65.104";
    char *udp_intf1 = NULL;
    char *udp_intf2 = NULL;
    DDS_Long sleep_time = 1000;
    DDS_Long count = 0;

    return publisher_main_w_args(domain_id, udp_intf, peer, sleep_time, count);
}
#endif
