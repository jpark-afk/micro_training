/*
 (c) Copyright, Real-Time Innovations, $Date: 2009-2015/05/17 01:14:45 $.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.
*/

/*
 * ThroughputArgs.h
 *
 * This is a class for parsing and validating arguments to the
 * Throughput test publisher/subscriber
 *
 */

#ifndef ThroughputArgs_h
#define ThroughputArgs_h

#ifndef Throughput_h
#include "Throughput.h"
#endif

#define THROUGHPUT_TEST_MAX_NODES 16
#define THROUGHPUT_TEST_DOMAIN_DEFAULT 0
#define MAX_COMMAND_LINE_ARGUMENTS 80
#define MAX_COMMAND_EXEC_LEN 256
#define MAX_PEER_LOCATOR_STR_LEN 128
#define MAX_PEER_PART_IDX 4
/* Allow a maximum of 5 participants (0..4) per node. */

#define DEFAULT_TEST_VERBOSITY TEST_VERBOSITY_ERRORS
#define DEFAULT_NDDS_VERBOSITY 1

#define _MAX_FILENAME_LENGTH 260


/* Parent class */
typedef struct ThroughputArgs {
    int mi_instance_count, mi_inactive_count;
    char _peer_hosts[THROUGHPUT_TEST_MAX_NODES][MAX_PEER_LOCATOR_STR_LEN];
    char _error_string[128];

    /* Name of configuration file */
    char _config_file_name[_MAX_FILENAME_LENGTH];

    char *_arguments_buff; /* Pointer to buffer to hold arguments from file. */
    char *_arg_variables[MAX_COMMAND_LINE_ARGUMENTS];
    /* Arguments parsed from file buffer */

    DDS_Boolean _args_valid;
    DDS_Boolean _reliable;
    DDS_Boolean _help_requested;
    char _command[MAX_COMMAND_EXEC_LEN];
    int ndds_domain; /* Test domain */
    int max_peer_index; /* Number of peers */
    int participant_id; /* the participant Id which must be supplied */
    int ndds_verbosity; /* NDDS verbosity */
    int test_verbosity; /* Test verbosity */
    const char* exe_name;
} ThroughputArgs;

void ThroughputArgs_create(ThroughputArgs *self);
void ThroughputArgs_delete(ThroughputArgs *self);
void ThroughputArgs_usage(ThroughputArgs *self);
char* ThroughputArgs_get_peer_host(ThroughputArgs *self, int index);
void ThroughputArgs_print_error(ThroughputArgs *self);
void ThroughputArgs_initialize(ThroughputArgs *self, const char *command);
void ThroughputArgs_print_arguments(ThroughputArgs *self);
/* utility function in case the derived class uses
   constructor with a file name. */
int ThroughputArgs_construct_args_from_file(
    ThroughputArgs *self, char* config_file_name, int *arg_count);
/* Parse common arguments */
int ThroughputArgs_parse_common_args(
    ThroughputArgs *self, int *arg, char ** argv);

typedef struct ThroughputPublisherArgs {
    ThroughputArgs parent;
    /* Defines the different packet sizes that are used
       for testing when in auto mode. */
    /* Maximum blocking time for a reliable writer */
    struct DDS_Duration_t _max_blocking_time;
    /* Hold user input in ms before conversion to DDS_Duration_t */
    unsigned long _blocking_time;
    int packet_size; /* Size of the message sequence */
    int test_duration_sec; /* Duration of test */
    int subscribers; /* Number of subscribers IP addresses supplied */
    int strength; /* For the reliable writers */
    int recovery_time_msec; /*sleep time / write loop*/
    int demand_initial; /* Demand messages / write loop */
    int demand_increment;
    int demand_max;
} ThroughputPublisherArgs;

void ThroughputPublisherArgs_create(ThroughputPublisherArgs *self);
void ThroughputPublisherArgs_delete(ThroughputPublisherArgs *self);
void ThroughputPublisherArgs_create_with_string(
    ThroughputPublisherArgs *self, int argc, char **argv);
void ThroughputPublisherArgs_create_with_file(
    ThroughputPublisherArgs *self, char* config_file_name);
/* Functions that override base class pure virtual public functions
   If arguments are not valid then the Usage method can be called */
void ThroughputPublisherArgs_usage(ThroughputPublisherArgs *self);
/* Arguments used for the test can be printed */
void ThroughputPublisherArgs_print_arguments(ThroughputPublisherArgs *self);
/* the error string can be printed */
void ThroughputPublisherArgs_print_error(ThroughputPublisherArgs *self);
/* With defaults for the Publisher */
void ThroughputPublisherArgs_initialize(
    ThroughputPublisherArgs *self, const char *);
/* Check that parameters provided are valid for the publisher */
DDS_Boolean ThroughputPublisherArgs_validate_args(
    ThroughputPublisherArgs *self);
/* Parse the command line or constructed command line arguments */
DDS_Boolean ThroughputPublisherArgs_parse_arguments(
    ThroughputPublisherArgs *self, int, char**);
void ThroughputPublisherArgs_set_max_blocking_time(
    ThroughputPublisherArgs *self, unsigned long msec);

/* Subscriber arguments */
/* Derived classes for Publisher arguments or Subscriber arguments */

typedef struct ThroughputSubscriberArgs {
    ThroughputArgs parent;
    int subscriberId;
} ThroughputSubscriberArgs;

void ThroughputSubscriberArgs_create(ThroughputSubscriberArgs *self);
void ThroughputSubscriberArgs_delete(ThroughputSubscriberArgs *self);
void ThroughputSubscriberArgs_create_with_string(
    ThroughputSubscriberArgs *self, int argc, char **argv);
void ThroughputSubscriberArgs_create_with_file(
    ThroughputSubscriberArgs *self, char* argumentsFile);
/* Functions that override base class pure virtual public functions
   If arguments are not valid then the Usage method can be called */
void ThroughputSubscriberArgs_usage(ThroughputSubscriberArgs *self);
/* Arguments used for the test can be printed */
void ThroughputSubscriberArgs_print_arguments(ThroughputSubscriberArgs *self);
/* the error string can be printed */
void ThroughputSubscriberArgs_print_error(ThroughputSubscriberArgs *self);
/* With defaults for the Subscriber */
void ThroughputSubscriberArgs_initialize(
    ThroughputSubscriberArgs *self, const char *);
/* Parse the command line or constructed command line arguments */
DDS_Boolean ThroughputSubscriberArgs_parse_arguments(
    ThroughputSubscriberArgs *self, int, char**);
  
#endif

