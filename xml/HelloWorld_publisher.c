
#include "rti_me_c.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wh_sm/wh_sm_history.h"
#include "rh_sm/rh_sm_history.h"

#include "HelloWorld.h"
#include "HelloWorldSupport.h"
#include "HelloWorldPlugin.h"

#include "HelloWorldApplication.h"

static void
HelloWorldPublisher_on_publication_matched(
    void *listener_data,
    DDS_DataWriter *writer,
    const struct DDS_PublicationMatchedStatus *status)
{
    (void)listener_data;
    (void)writer;

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
publisher_main_w_args(DDS_Long sleep_time, DDS_Long count)
{
    DDS_DataWriter *datawriter;
    HelloWorldDataWriter *hw_datawriter;
    DDS_ReturnCode_t retcode;
    HelloWorld *sample = NULL;
    struct Application *application = NULL;
    DDS_Long i;
    struct DDS_DataWriterListener dw_listener =
    DDS_DataWriterListener_INITIALIZER;
    int ret_value = -1;

    sample = HelloWorldTypeSupport_create_data();
    if (sample == NULL)
    {
        printf("failed HelloWorldTypeSupport_create_data\n");
        return -1;
    }

    application = Application_create(PUB_PARTICIPANT_NAME, sleep_time, count);

    if (application == NULL)
    {
        printf("failed Application create\n");
        goto done;
    }

    datawriter = DDS_DomainParticipant_lookup_datawriter_by_name(
        application->participant,
        PUB_DATAWRITER_NAME);
    if (datawriter == NULL)
    {
        printf("datawriter == NULL\n");
        goto done;
    }

    dw_listener.on_publication_matched =
    HelloWorldPublisher_on_publication_matched;

    retcode = DDS_DataWriter_set_listener(
        datawriter,
        &dw_listener,
        DDS_PUBLICATION_MATCHED_STATUS);
    if (retcode != DDS_RETCODE_OK)
    {
        printf("failed to set writer listener\n");
        goto done;
    }

    hw_datawriter = HelloWorldDataWriter_narrow(datawriter);

    #ifdef RTI_CERT
    #ifdef RTI_VXWORKS
    /** End initialization, disable further dynamic memory allocation ***/
    memAllocDisable();
    #endif
    #endif

    for (i = 0; (application->count <= 0) || (i < application->count); ++i)
    {

        /* TODO set sample attributes here */
		snprintf(sample->message,64,"Hello World(%d)!\0",i);
		sample->count = i;

        retcode = HelloWorldDataWriter_write(
            hw_datawriter,
            sample,
            &DDS_HANDLE_NIL);
        if (retcode != DDS_RETCODE_OK)
        {
            printf("Failed to write sample\n");
        }
        else
        {
            printf("Wrote sample #%d\n",(i+1));
        }

        OSAPI_Thread_sleep((RTI_UINT32)application->sleep_time);
    }

    ret_value = 0;

    done:

    #ifndef RTI_CERT
    if (application != NULL)
    {
        Application_delete(application);
    }

    if (sample != NULL)
    {
        HelloWorldTypeSupport_delete_data(sample);
    }

    #endif

    return ret_value;
}

#if !(defined(RTI_VXWORKS) && !defined(__RTP__))
int
main(int argc, char **argv)
{
    DDS_Long i = 0;
    DDS_Long sleep_time = 1000;
    DDS_Long count = 0;

    for (i = 1; i < argc; ++i)
    {
        if (!strcmp(argv[i], "-sleep"))
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

    return publisher_main_w_args(sleep_time, count);
}
#elif defined(RTI_VXWORKS)
int
publisher_main(void)
{
    DDS_Long sleep_time = 1000;
    DDS_Long count = 0;

    return publisher_main_w_args(sleep_time, count);
}
#endif
