
#ifndef Application_h
#define Application_h

#define PUB_PARTICIPANT_NAME  "HelloWorldAppLibrary::HelloWorldDPDEPubDP"
#define PUB_DATAWRITER_NAME   "HelloWorldDPDEPub::HelloWorldDPDEDW"
#define SUB_PARTICIPANT_NAME  "HelloWorldAppLibrary::HelloWorldDPDESubDP"
#define SUB_DATAREADER_NAME   "HelloWorldDPDESub::HelloWorldDPDEDR"

#include "rti_me_c.h"

/*e \dref_Example_Config_UseSampleFilter
* Define USE_SAMPLE_FILTER to filter samples
* using call-backs on the DataReader's listener*/

/*#define USE_SAMPLE_FILTER*/

/*e \dref_Example_Config_FilterOnDeserialize
* Define FILTER_ON_DESERIALIZE to enable
* filtering on call-back on_before_sample_deserialize;
* otherwise use call-back on_before_sample_commit
* by default. */

/*#define FILTER_ON_DESERIALIZE*/

struct Application
{
    DDS_DomainParticipant *participant;
    DDS_Long sleep_time;
    DDS_Long count;
};

extern void
Application_help(char *appname);

extern struct Application*
Application_create(
    const char *participant_name,
    DDS_Long sleep_time,
    DDS_Long count);

#ifndef RTI_CERT
extern void
Application_delete(struct Application *application);

#endif
#endif
