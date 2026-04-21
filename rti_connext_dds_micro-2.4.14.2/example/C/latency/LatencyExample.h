/*
 (c) Copyright, Real-Time Innovations, 2009-2015
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.
*/

/*=======================================================================*/

/*
**  Description
**  -----------
**
**  We use two pairs of best effort writers and best effort readers
**  to measure the latency. The basic algorithm is as follows:
**
**    LatencyWriter                          LatencyReader
**    Create a writer (topic dataTopic)      Create a reader (topic dataTopic)
**    Create a reader (topic echoTopic)      Create a writer (topic echoTopic)
**    Start timer                            Wait
**    Send one issue --(topic dataTopic)-->  ...
**    Wait                                   Receive the issue
**    ....           <--(topic echoTopic)--  Send one issue of the same size
**    Receive the issue
**    Stop timer
**    Calculate delay, minus clock overhead, and divide by 2
*/

#ifndef latency_example_h
#define latency_example_h

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* RTI DDS header file */
#include "rti_me_c.h"

/* Type header files */
#include "Latency.h"

/*** INTERNAL API: OSAPI, CDR, REDA, CLOCK Headers ***/
#include "osapi/osapi_types.h"
#include "osapi/osapi_semaphore.h"

#include "LatencyGlobalSettings.h"

#include "DataProcessor.h"
#include "Communicator.h"

/* Sentinel value indicating the absence of a cookie. The presence
** of a response cookie limits the response of reader. Only
** the reader with the cookie that matches will echo back to
** the writer. No cookie means all readers respond
*/
#define LATENCY_RESTRICT_RECV_COOKIE_NONE (0)
#define LATENCY_WAIT_RECV_COUNT_DEFAULT   (1)

#define IS_OPTION(str, option) (strncmp(str, option, strlen(str)) == 0)

#define AppLog_exception    printf
#define AppLog_warn          printf
#define AppLog_report         printf
#define AppLog_flush              fflush(stdout)

#define MAX_COMMAND_LINE_ARGUMENTS 80
#define MAX_COMMAND_EXEC_LEN 256

/* The final sequence number to indicate to terminate the application */
#define FINAL_SEQUENCE_NUMBER (-1)

/* ---------------------------------------------------------------------*/
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
                                               measure_individual_latencies);
DDS_Boolean LatencyEngine_execute_sending_loop(LatencyEngine * self,
                                               DDS_Boolean
                                               measure_individual_latencies);
DDS_Boolean LatencyEngine_finalize(LatencyEngine * self);
/*DDS_Boolean LatencyEngine_signal_data_received(LatencyEngine * self);*/

#if 0
/*functions to run the example with hard-coded arguments*/
int publisher_main_no_args(void);
int subscriber_main_no_args(void);
#endif

#endif /* #ifndef latency_example_h */
