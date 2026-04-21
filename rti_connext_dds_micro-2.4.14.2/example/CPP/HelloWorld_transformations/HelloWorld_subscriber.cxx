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
#include "HelloWorldPlugin.h"
#include "HelloWorldApplication.h"


class HelloWorldReaderListener : public DDSDataReaderListener
{

  public:

    virtual void on_data_available(DDSDataReader *reader);
};


template <typename T>
void take_and_print(typename T::DataReader* reader)
{
    DDS_ReturnCode_t retcode;
    struct DDS_SampleInfo sample_info;
    T *sample = NULL;

    sample = T::TypeSupport::create_data();
    if (sample == NULL)
    {
        printf("Failed sample initialize\n");
        return;
    }

    retcode = reader->take_next_sample(*sample, sample_info);
    while (retcode == DDS_RETCODE_OK)
    {
        if (sample_info.valid_data)
        {
            printf("\nSample received\n\tmsg: '%s'\n", sample->msg);
        }
        else
        {
            printf("\nSample received\n\tINVALID DATA\n");
        }
        retcode = reader->take_next_sample(*sample, sample_info);
    }

    T::TypeSupport::delete_data(sample);
}

void
HelloWorldReaderListener::on_data_available(DDSDataReader * reader)
{
    HelloWorldDataReader *hw_reader = HelloWorldDataReader::narrow(reader);
    take_and_print<HelloWorld>(hw_reader);
}


int
subscriber_main_w_args(DDS_Long domain_id, char *udp_intf1, char *udp_intf2, 
                       char *peer, DDS_Long sleep_time, DDS_Long count)
{
    DDSSubscriber *subscriber = NULL;
    DDSDataReader *datareader = NULL;
    HelloWorldReaderListener *listener  = new HelloWorldReaderListener();
    DDS_DataReaderQos dr_qos;
    DDS_ReturnCode_t retcode;
    Application *application = NULL;
    RT_Registry_T *registry = NULL;


    application = new Application();
    if (application == NULL)
    {
        printf("failed Application new\n");
        goto done;
    }

    retcode = application->initialize("subscriber", 
                                      "publisher", 
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

    subscriber = application->participant->create_subscriber(
                        DDS_SUBSCRIBER_QOS_DEFAULT,NULL,
                        DDS_STATUS_MASK_NONE);
    if (subscriber == NULL)
    {
        printf("subscriber == NULL\n");
        goto done;
    }

    retcode = subscriber->get_default_datareader_qos(dr_qos);
    if (retcode != DDS_RETCODE_OK)
    {
        printf("failed get_default_datareader_qos\n");
        goto done;
    }

    dr_qos.resource_limits.max_samples = 32;
    dr_qos.resource_limits.max_instances = 1;
    dr_qos.resource_limits.max_samples_per_instance = 32;
    /* if there are more remote writers, you need to increase these limits */
    dr_qos.reader_resource_limits.max_remote_writers = 10;
    dr_qos.reader_resource_limits.max_remote_writers_per_instance = 10;
    dr_qos.history.depth = 32;

    /* Reliability QoS */
#ifdef USE_RELIABLE_QOS
    dr_qos.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
#else
    dr_qos.reliability.kind = DDS_BEST_EFFORT_RELIABILITY_QOS;
#endif

    datareader = subscriber->create_datareader(
                           application->topic,
                           dr_qos,
                           listener,
                           DDS_DATA_AVAILABLE_STATUS);

    if (datareader == NULL)
    {
        printf("datareader == NULL\n");
        goto done;
    }

    retcode = application->enable();
    if (retcode != DDS_RETCODE_OK)
    {
        printf("failed to enable application\n");
        goto done;
    }

    if (application->count != 0)
    {
        printf("Running for %d seconds, press Ctrl-C to exit\n",
               application->count);
        OSAPI_Thread_sleep(application->count * 1000);
    }
    else
    {
        int sleep_loop_count =  (24 * 60 * 60) / 2000;
        int sleep_loop_left = (24 * 60 * 60) % 2000;

        printf("Running for 24 hours, press Ctrl-C to exit\n");

        while (sleep_loop_count)
        {
            OSAPI_Thread_sleep(2000  * 1000); /* sleep is in ms */
            --sleep_loop_count;
        }

        OSAPI_Thread_sleep(sleep_loop_left * 1000);
    }

    done:

    if (application != NULL)
    {
        delete application;
    }
   
    if (listener != NULL)
    {
        delete listener;
    }

    //DataReaderQos is automatically disposed by destructor

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

    return subscriber_main_w_args(domain_id, udp_intf1, udp_intf2, 
                                  peer, sleep_time, count);
}
#elif defined(RTI_VXWORKS)
int
subscriber_main(void)
{
    /* Explicitly configure args below */
    DDS_Long domain_id = 44;
    char *peer = "10.10.65.103";
    char *udp_intf1 = NULL;
    char *udp_intf2 = NULL;
    DDS_Long sleep_time = 1000;
    DDS_Long count = 0;

    return subscriber_main_w_args(domain_id, udp_intf1, udp_intf2, peer, sleep_time, count);
}
#endif

