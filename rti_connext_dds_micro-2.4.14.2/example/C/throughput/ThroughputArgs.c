/*:
 (c) Copyright, Real-Time Innovations, 2009-2015.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.
*/

/* This class is used to collect and validate the command line arguments for
   the NDDS Throughput tool */

#include <stdio.h>
#include <stdlib.h>
#ifndef rti_me_c_h
#include "rti_me_c.h"
#endif
#ifndef ThroughputArgs_h
#include "ThroughputArgs.h"
#endif
#ifndef ThroughputCommon_h
#include "ThroughputCommon.h"
#endif

#define IS_OPTION(str, option) (strcmp(str, option) == 0)

void
ThroughputArgs_create(ThroughputArgs * self)
{
    memset(self->_peer_hosts, 0, sizeof(self->_peer_hosts));
    memset(self->_error_string, 0, sizeof(self->_error_string));
    memset(self->_config_file_name, 0, sizeof(self->_config_file_name));
    memset(self->_arg_variables, 0, sizeof(self->_arg_variables));
}

void
ThroughputArgs_delete(ThroughputArgs * self)
{
    /* Free the arguments buffer if allocated */
    if (self->_arguments_buff != NULL)
    {
        free(self->_arguments_buff);
    }
}

void
ThroughputArgs_usage(ThroughputArgs * self)
{
const char *common_usage_str =
    "\t-help                         - Print this message and exit\n"
    "\t-domainId      <domainId>     - RTI DDS domain\n"
    /* We need to know the domain that the test is to operate on. */
    "\t-peer        <IP Address>     - Peer Host IP Address\n"
    "\t                                The parameter can be repeated ut to 16\n"
    "\t                                times to indicate multiple peers.\n"
    "\t                                Both unicast and multicast IP addresses\n"
    "\t                                can be used.\n"
    /* We need a list of peers */
    "\t-participantId        <id>    - Participant Id (range 0..4)\n"
    /* Participant Id */
    "\t-reliable                     - Use reliable service\n"
    "\t-file         <config file>   - Read arguments from the specified\n"
    "\t                                configuration file\n"
    /* We need to know if there are further options provided via a file */
    "\t-name         <program name>  - Change the name used in the output\n"
    "\t                                messages to identify this program\n"
    "\t                                execution\n";

    AppLog_exception("Usage: \n\t%s [baseoptions] [extraoptions]\n\n",
                     self->_command);
    AppLog_exception("Where [baseoptions] are:\n%s\n", common_usage_str);
}

char *
ThroughputArgs_get_peer_host(ThroughputArgs * self, int index)
{
    if (index > THROUGHPUT_TEST_MAX_NODES)
    {
        return NULL;
    }
    else
    {
        return self->_peer_hosts[index];
    }
}

int
ThroughputArgs_parse_common_args(ThroughputArgs * self, int *arg, char **argv)
{
    /* Assume it is a common argument */
    self->_args_valid = DDS_BOOLEAN_TRUE;

    if (IS_OPTION(argv[*arg], "-participantId"))
    {
        /* Peer host list */
        if (argv[++*arg] != NULL)
        {
            self->participant_id = strtol(argv[*arg], NULL, 10);
            if (self->participant_id > MAX_PEER_PART_IDX)
            {
                sprintf(self->_error_string,
                        "Only 5 participants permitted per node"
                        " (participant 0..4)\n");
                self->_args_valid = DDS_BOOLEAN_FALSE;
            }
        }
        else
        {
            self->_args_valid = DDS_BOOLEAN_FALSE;
        }
    }
    else if (IS_OPTION(argv[*arg], "-domainId"))
    {                           /* Test Domain */
        if (argv[++*arg] != NULL)
        {
            self->ndds_domain = strtol(argv[*arg], NULL, 10);
        }
        else
        {
            self->_args_valid = DDS_BOOLEAN_FALSE;
        }
    }
    else if (IS_OPTION(argv[*arg], "-peer"))
    {                           /* Peer host list */
        if ((argv[++*arg] != NULL) &&
            (self->max_peer_index < THROUGHPUT_TEST_MAX_NODES))
        {
            strncpy(self->_peer_hosts[self->max_peer_index++], argv[*arg],
                    MAX_PEER_LOCATOR_STR_LEN);
        }
        else
        {
            self->_args_valid = DDS_BOOLEAN_FALSE;
            if (self->max_peer_index >= THROUGHPUT_TEST_MAX_NODES)
            {
                sprintf(self->_error_string,
                        "Peer nodes (%d) > Maximum number of "
                        "peer nodes permitted (%d)\n", self->max_peer_index,
                        THROUGHPUT_TEST_MAX_NODES);
            }
        }
    }
    else if (IS_OPTION(argv[*arg], "-name"))
    {                           /* Executable name */
        if (argv[++*arg] != NULL)
        {
            self->exe_name = argv[*arg];
        }
        else
        {
            self->_args_valid = DDS_BOOLEAN_FALSE;
        }
    }
    else if (IS_OPTION(argv[*arg], "-reliable"))
    {
        /* Use Reliable Communications */
        self->_reliable = DDS_BOOLEAN_TRUE;
    }
    else if (IS_OPTION(argv[*arg], "-instance"))
    {
        if (argv[++*arg] != NULL)
        {
            self->mi_instance_count = strtol(argv[*arg], NULL, 10);
        }
        else
        {
            self->_args_valid = DDS_BOOLEAN_FALSE;
        }
    }
    else if (IS_OPTION(argv[*arg], "-help"))
    {
        /* Use Reliable Communications */
        self->_help_requested = DDS_BOOLEAN_TRUE;
    }
    else
    {                           /* return error */
        self->_args_valid = DDS_BOOLEAN_FALSE;
    }
    return self->_args_valid;
}

void
ThroughputArgs_initialize(ThroughputArgs * self, const char *command)
{

    strncpy(self->_command, command, MAX_COMMAND_EXEC_LEN - 1);
    self->_command[MAX_COMMAND_EXEC_LEN - 1] = '\0';

    self->mi_instance_count = 0;

    self->ndds_domain = THROUGHPUT_TEST_DOMAIN_DEFAULT;
    self->max_peer_index = 0;

    self->_arguments_buff = NULL;

    self->_reliable = DDS_BOOLEAN_FALSE;
    self->_help_requested = DDS_BOOLEAN_FALSE;

    sprintf(self->_config_file_name, "Not Specified");
    sprintf(self->_error_string, "None\n");
}

void
ThroughputArgs_print_arguments(ThroughputArgs * self)
{
int i;
    AppLog_report("\t-domainId      = %d\n"
                  "\t-participantId = %d\n",
                  self->ndds_domain, self->participant_id);
    i = 0;
    while (++i < self->max_peer_index)
    {
        AppLog_report("\t-peer[%d] = '%s' \n", i, self->_peer_hosts[i]);
    }

    AppLog_report("\t-reliable         = %s\n"
                  "\t-file             = %s\n",
                  ((self->_reliable == DDS_BOOLEAN_TRUE) ? "ON" : "OFF"),
                  self->_config_file_name);
}

int
ThroughputArgs_construct_args_from_file(ThroughputArgs * self,
                                        char *config_file_name, int *arg_count)
{

    FILE *p_config_file;
    long end_of_buffer, file_length;
    int args_valid;

    args_valid = DDS_BOOLEAN_TRUE;      /* Assume OK */

    p_config_file = fopen(config_file_name, "r");
    if (p_config_file == NULL)
    {
        sprintf(self->_error_string, "Unable to open %s\n", config_file_name);
        args_valid = DDS_BOOLEAN_FALSE;
        return args_valid;
    }
    /* File name is OK so save the name in a
     * class variable for printing arguments. */
    strncpy(self->_config_file_name, config_file_name,
            sizeof(config_file_name));

    /* Find the length of the file */
    fseek(p_config_file, 0, SEEK_END);
    file_length = ftell(p_config_file);
    fseek(p_config_file, 0, SEEK_SET);

    /* Free if it has been allocated prior to this */
    if (self->_arguments_buff != NULL)
    {
        free(self->_arguments_buff);
    }

    /* Allocate a buffer for the whole file + 1 in case the file is empty! */
    self->_arguments_buff = (char *)calloc(file_length + 1, sizeof(char));

    /* and read the file in.... */
    end_of_buffer = fread(self->_arguments_buff, sizeof(char), file_length,
                          p_config_file);
    self->_arguments_buff[end_of_buffer - 1] = '\0';

    /* Don't need the file an more ! */
    fclose(p_config_file);

    *arg_count = 0;
    self->_arg_variables[++*arg_count] = strtok(self->_arguments_buff, " ");
    while (self->_arg_variables[*arg_count] != NULL)
    {
        self->_arg_variables[++*arg_count] = strtok(NULL, " ");
    }
    return args_valid;
}

void
ThroughputPublisherArgs_create(ThroughputPublisherArgs * self)
{
    ThroughputArgs_create(&self->parent);
    AppLog_warn("WARNING: running ThroughputPublisherArgs() constructor\n"
                "          make sure the hard-coded arguments will\n"
                "         fit your system and scenario\n");


    self->parent._args_valid = DDS_BOOLEAN_TRUE;
    self->parent._arguments_buff = NULL;

    self->parent.mi_instance_count = 0;

    self->parent.ndds_domain = THROUGHPUT_TEST_DOMAIN_DEFAULT;

    self->parent.max_peer_index = 0;

    self->parent._reliable = DDS_BOOLEAN_FALSE;
    self->parent._help_requested = DDS_BOOLEAN_FALSE;

    sprintf(self->parent._config_file_name, "Not Specified");
    sprintf(self->parent._error_string, "None\n");

    self->parent.participant_id = DEFAULT_PUBLISHER_PARTICIPANT_ID;

    /* Size of the message */
    self->packet_size = DEFAULT_PACKET_SIZE;

    /* Number of seconds to run the test */
    self->test_duration_sec = DEFAULT_TEST_DURATION;

    /* Set the default message demand per 10ms iteration */
    self->recovery_time_msec = DEFAULT_RECOVERY_TIME_MS;        /* 10ms */
    self->demand_initial = DEFAULT_DEMAND;
    self->demand_increment = DEFAULT_DEMAND;
    self->demand_max = DEFAULT_DEMAND;

    /* Assume only one subscriber */
    self->subscribers = DEFAULT_NUMBER_OF_SUBSCRIBERS;

    self->strength = DEFAULT_TEST_STRENGTH;
    self->_max_blocking_time.sec = 0;
    self->_max_blocking_time.nanosec = DEFAULT_MAX_BLOCKING_TIME_NS;
}

void
ThroughputPublisherArgs_delete(ThroughputPublisherArgs * self)
{
    ThroughputArgs_delete(&self->parent);
}

void
ThroughputPublisherArgs_create_with_file(ThroughputPublisherArgs * self,
                                         char *config_file_name)
{
    int arg_count;

    ThroughputArgs_create(&self->parent);
    self->parent._args_valid = DDS_BOOLEAN_TRUE;        /* Assume that arguments are OK */
    arg_count = 0;

    ThroughputPublisherArgs_initialize(self, "");       /* Set the defaults */

    self->parent._args_valid =
        ThroughputArgs_construct_args_from_file(&self->parent, config_file_name,
                                                &arg_count);
    if (self->parent._args_valid)
    {
        self->parent._args_valid =
            ThroughputPublisherArgs_parse_arguments(self, arg_count,
                                                    self->parent.
                                                    _arg_variables);
        /* If there are no parse errors check for logical errors. */
        if (self->parent._args_valid)
        {
            self->parent._args_valid =
                ThroughputPublisherArgs_validate_args(self);
        }
    }
}

void
ThroughputPublisherArgs_create_with_string(ThroughputPublisherArgs * self,
                                           int argc, char **argv)
{
    ThroughputArgs_create(&self->parent);

    ThroughputPublisherArgs_initialize(self, argv[0]);  /* Set the defaults */

    self->parent._args_valid = DDS_BOOLEAN_FALSE;

    self->parent.exe_name = "Throughput_publisher";
    self->parent._args_valid =
        ThroughputPublisherArgs_parse_arguments(self, argc, argv);
    /* If there are no parse errors check for logical errors. */
    if (self->parent._args_valid)
    {
        /* Provide loop back as default if no Peers specified. */
        self->parent._args_valid = ThroughputPublisherArgs_validate_args(self);
    }
}

void
ThroughputPublisherArgs_usage(ThroughputPublisherArgs * self)
{
    ThroughputArgs_usage(&self->parent);

    AppLog_exception(
    "Where [extraoptions] are:\n"
    "\t-subscribers <num>            - Number of subscribers (range: 1..16)\n"
    /* We need to know how many  of the peers are subscribers. */
    "\t-size        <numBytes>       - Packet size (range 8..240000)\n"
    /* We need to know how  big the packet should be */
    "\t-duration    <numSeconds>     - Test duration in seconds\n"
    /* We need to know how long the test should last. */
    "\t-demand     <first:incr:last> - Controls messages sent per write loop\n"
    "\t                                increasing demand increases throughput\n"
    "\t                                and also the processor load\n"
    "\t                                Must be specified as a range using\n"
    "\t                                three numbers (first, incr, last)\n"
    "\t                                separated by a ':'\n"
    "\t                                The values must verify 1<= first <= last\n"
    "\t-recoveryTime  <ms>           - sleep time between 2 write loops\n"
    "\t-strength    <value>          - Sets DDS Writer Ownership Strength\n"
    /* We need to know what the strength of this publisher. */
    "\t-maxBlockingTime  <ms>        - Sets the DDS Reliable writer\n"
    "\t                                max blocking time (range: 20..MAX_INT)\n");
}

void
ThroughputPublisherArgs_print_error(ThroughputPublisherArgs * self)
{
    AppLog_exception("\nLast error detected: %s", self->parent._error_string);

    /* Explain where the error occurred if possible. */
}

void
ThroughputPublisherArgs_initialize(ThroughputPublisherArgs * self,
                                   const char *command)
{
    ThroughputArgs_initialize(&self->parent, command);

    self->parent.participant_id = DEFAULT_PUBLISHER_PARTICIPANT_ID;

    /* Size of the message, can be up 63K user bytes +
     * 4 bytes for the sequence number; */
    self->packet_size = DEFAULT_PACKET_SIZE;

    /* Number of seconds to run the test */
    self->test_duration_sec = DEFAULT_TEST_DURATION;

    /* Set the default message demand per 10ms iteration */
    self->recovery_time_msec = DEFAULT_RECOVERY_TIME_MS;        /* 10ms */
    self->demand_initial = DEFAULT_DEMAND;
    self->demand_increment = DEFAULT_DEMAND;
    self->demand_max = DEFAULT_DEMAND;

    /* Assume only one subscriber */
    self->subscribers = DEFAULT_NUMBER_OF_SUBSCRIBERS;

    self->strength = DEFAULT_TEST_STRENGTH;
    self->_max_blocking_time.sec = 0;
    self->_max_blocking_time.nanosec = DEFAULT_MAX_BLOCKING_TIME_NS;
}

DDS_Boolean
ThroughputPublisherArgs_validate_args(ThroughputPublisherArgs * self)
{

    /* Is the packet size wrong ? */
    if (self->packet_size < 0)
    {
        sprintf(self->parent._error_string, "Packet Size = %d < 0\n",
                self->packet_size);
        return DDS_BOOLEAN_FALSE;
    }

    if (self->packet_size > THROUGHPUT_TEST_PACKET_DATA_SIZE_MAX)
    {
        sprintf(self->parent._error_string, "Packet Size (%d) > "
                "THROUGHPUT_TEST_PACKET_DATA_SIZE_MAX (%d) \n",
                self->packet_size, THROUGHPUT_TEST_PACKET_DATA_SIZE_MAX);
        return DDS_BOOLEAN_FALSE;
    }

    /* Is the duration too small ? */
    if (self->test_duration_sec < MIN_TEST_DURATION_SEC)
    {
        sprintf(self->parent._error_string,
                "Duration (%d) < Minimum permitted (%d) ?\n",
                self->test_duration_sec, MIN_TEST_DURATION_SEC);
        return DDS_BOOLEAN_FALSE;
    }
    /* Have too many subscribers been specified ? */
    if (self->subscribers > MAX_TEST_SUBSCRIBERS)
    {
        sprintf(self->parent._error_string,
                "Subscribers (%d) > Max permitted (%d)?\n",
                self->subscribers, MAX_TEST_SUBSCRIBERS);
        return DDS_BOOLEAN_FALSE;
    }

    if (self->demand_initial <= 0 ||
        self->demand_increment <= 0 || self->demand_max < self->demand_initial)
    {
        sprintf(self->parent._error_string,
                "Demand (%d) must be > 1 message per write loop\n",
                self->demand_initial);
        return DDS_BOOLEAN_FALSE;
    }

    if (self->parent._reliable)
    {
        if (((self->_max_blocking_time.sec == 0) &&
             (self->_max_blocking_time.nanosec < DEFAULT_MAX_BLOCKING_TIME_NS))
            ||
            ((self->_max_blocking_time.nanosec %
              DEFAULT_FAST_HEARTBEAT_TIME_NS) != 0))
        {
            sprintf(self->parent._error_string,
                    "Max blocking time must be > 2 * "
                    "Fast Heartbeat period(10ms), and a multiple of 10 ms\n");
            return DDS_BOOLEAN_FALSE;
        }
    }

    return DDS_BOOLEAN_TRUE;
}

DDS_Boolean
ThroughputPublisherArgs_parse_arguments(ThroughputPublisherArgs * self,
                                        int argc, char **argv)
{
    /* This function is called recursively if there is a configuration file
     * specified on the command line. */

    char config_file_name[_MAX_FILENAME_LENGTH];
    char *tmp1, *tmp2;
    int i;

    self->parent._args_valid = DDS_BOOLEAN_TRUE;        /* Assume all is OK */
    *config_file_name = 0;      /* Ensure that file name is not hanging around */

    /* Parse the arguments */
    for (i = 1; i < argc; ++i)
    {
        if (IS_OPTION(argv[i], "-subscribers"))
        {
            /* number of subscribers */
            if (argv[++i] != NULL)
            {
                self->subscribers = strtol(argv[i], NULL, 10);
            }
            else
            {
                self->parent._args_valid = DDS_BOOLEAN_FALSE;
                return DDS_BOOLEAN_FALSE;
            }
        }
        else if (IS_OPTION(argv[i], "-size"))
        {
            /* Packet size */
            if (argv[++i] != NULL)
            {
                self->packet_size = strtol(argv[i], NULL, 10);
                self->packet_size -= THROUGHPUT_PACKET_OVERHEAD;
            }
            else
            {
                self->parent._args_valid = DDS_BOOLEAN_FALSE;
                return DDS_BOOLEAN_FALSE;
            }
        }
        else if (IS_OPTION(argv[i], "-duration"))
        {                       /* Test duration */
            if (argv[++i] != NULL)
            {
                self->test_duration_sec = strtol(argv[i], NULL, 10);
            }
            else
            {
                self->parent._args_valid = DDS_BOOLEAN_FALSE;
                return DDS_BOOLEAN_FALSE;
            }
        }
        else if (IS_OPTION(argv[i], "-demand"))
        {
            if (argv[++i] == NULL)
            {
                self->parent._args_valid = DDS_BOOLEAN_FALSE;
                return DDS_BOOLEAN_FALSE;
            }
            tmp1 = strchr(argv[i], ':');
            if (!tmp1)
            {
                self->parent._args_valid = DDS_BOOLEAN_FALSE;
                return DDS_BOOLEAN_FALSE;
            }
            *tmp1 = 0;
            ++tmp1;
            self->demand_initial = strtol(argv[i], NULL, 10);
            tmp2 = strchr(tmp1, ':');
            if (!tmp2)
            {
                self->parent._args_valid = DDS_BOOLEAN_FALSE;
                return DDS_BOOLEAN_FALSE;
            }
            *tmp2 = 0;
            ++tmp2;
            self->demand_increment = strtol(tmp1, NULL, 10);
            self->demand_max = strtol(tmp2, NULL, 10);
        }
        else if (IS_OPTION(argv[i], "-recoveryTime"))
        {
            /* sleep time in the send loop */
            if (argv[++i] != NULL)
            {
                self->recovery_time_msec = strtol(argv[i], NULL, 10);
            }
            else
            {
                self->parent._args_valid = DDS_BOOLEAN_FALSE;
                return DDS_BOOLEAN_FALSE;
            }
        }
        else if (IS_OPTION(argv[i], "-strength"))
        {
            /* Ownership strength */
            if (argv[++i] != NULL)
            {
                self->strength = strtol(argv[i], NULL, 10);
            }
            else
            {
                self->parent._args_valid = DDS_BOOLEAN_FALSE;
                return DDS_BOOLEAN_FALSE;
            }
        }
        else if (IS_OPTION(argv[i], "-maxBlockingTime"))
        {
            /* Maximum Blocking Time */
            if (argv[++i] != NULL)
            {
                self->_blocking_time = strtol(argv[i], NULL, 10);
                ThroughputPublisherArgs_set_max_blocking_time(self,
                                                              self->
                                                              _blocking_time);
            }
            else
            {
                self->parent._args_valid = DDS_BOOLEAN_FALSE;
                return DDS_BOOLEAN_FALSE;
            }
        }
        else if (IS_OPTION(argv[i], "-file"))
        {
            /* Configuration file name  do here because of recursion */
            if (argv[++i] != NULL)
            {
                strncpy(config_file_name, argv[i], sizeof(config_file_name));
                /* Get local copy to use (recursion) */
            }
            else
            {
                self->parent._args_valid = DDS_BOOLEAN_FALSE;
                return DDS_BOOLEAN_FALSE;
            }
        }
        else
        {
            /* Check to see if it is one of the common arguments */
            if (ThroughputArgs_parse_common_args(&self->parent, &i, argv) ==
                DDS_BOOLEAN_FALSE)
            {
                return DDS_BOOLEAN_FALSE;
            }
        }
    }

    if (strlen(config_file_name) > 0)
    {
        int arg_count;
        self->parent._args_valid =
            ThroughputArgs_construct_args_from_file(&self->parent,
                                                    config_file_name,
                                                    &arg_count);
        if (self->parent._args_valid)
        {
            /* Recurse using file arguments which take precedent */
            self->parent._args_valid =
                ThroughputPublisherArgs_parse_arguments(self, arg_count,
                                                        self->parent.
                                                        _arg_variables);
        }
    }
    return self->parent._args_valid;
}

void
ThroughputPublisherArgs_set_max_blocking_time(ThroughputPublisherArgs * self,
                                              unsigned long msec)
{
    while (msec >= 1000)
    {
        self->_max_blocking_time.sec++;
        msec -= 1000;
    }
    self->_max_blocking_time.nanosec = msec * 1000000;
}

void
ThroughputPublisherArgs_print_arguments(ThroughputPublisherArgs * self)
{
    ThroughputArgs_print_arguments(&self->parent);

    AppLog_report("\t-subscribers     = %d\n"
                  "\t-size            = %d bytes\n"
                  "\t-duration        = %d seconds\n"
                  "\t-demand          = %d:%d:%d message(s) per write loop\n"
                  "\t-strength        = %d\n"
                  "\t-maxBlockingTime = %lu ms\n",
                  self->subscribers,
                  self->packet_size,
                  self->test_duration_sec,
                  self->demand_initial, self->demand_increment,
                  self->demand_max, self->strength, self->_blocking_time);
}


void
ThroughputSubscriberArgs_create(ThroughputSubscriberArgs * self)
{
    ThroughputArgs_create(&self->parent);
    AppLog_warn("WARNING: running ThroughputSubscriberArgs() constructor\n"
                "         make sure the hard-coded arguments will\n"
                "         fit your system and scenario\n");


    self->parent._args_valid = DDS_BOOLEAN_TRUE;
    self->parent._arguments_buff = NULL;

    self->parent.mi_instance_count = 0;

    self->parent.ndds_domain = THROUGHPUT_TEST_DOMAIN_DEFAULT;

    self->parent.max_peer_index = 0;

    self->parent._reliable = DDS_BOOLEAN_FALSE;
    self->parent._help_requested = DDS_BOOLEAN_FALSE;

    sprintf(self->parent._config_file_name, "Not Specified");
    sprintf(self->parent._error_string, "None\n");

    self->parent.participant_id = DEFAULT_SUBSCRIBER_PARTICIPANT_ID;

    self->subscriberId = 0;
}

void
ThroughputSubscriberArgs_delete(ThroughputSubscriberArgs * self)
{
    ThroughputArgs_delete(&self->parent);
}

void
ThroughputSubscriberArgs_create_with_file(ThroughputSubscriberArgs * self,
                                          char *config_file_name)
{
    int arg_count;

    ThroughputArgs_create(&self->parent);
    self->parent._args_valid = DDS_BOOLEAN_TRUE;        /* Assume that arguments are OK */
    self->parent.exe_name = "Throughput_subscriber";
    arg_count = 0;

    ThroughputSubscriberArgs_initialize(self, "");      /* Set the defaults */

    self->parent._args_valid =
        ThroughputArgs_construct_args_from_file(&self->parent, config_file_name,
                                                &arg_count);
    if (self->parent._args_valid)
    {
        self->parent._args_valid =
            ThroughputSubscriberArgs_parse_arguments(self, arg_count,
                                                     self->parent.
                                                     _arg_variables);
    }
}

void
ThroughputSubscriberArgs_create_with_string(ThroughputSubscriberArgs * self,
                                            int argc, char **argv)
{
    ThroughputArgs_create(&self->parent);

    ThroughputSubscriberArgs_initialize(self, argv[0]); /* Set the defaults */

    self->parent._args_valid = DDS_BOOLEAN_TRUE;        /* Assume that arguments are OK */
    self->parent.exe_name = "Throughput_subscriber";
    self->parent._args_valid =
        ThroughputSubscriberArgs_parse_arguments(self, argc, argv);
}

void
ThroughputSubscriberArgs_usage(ThroughputSubscriberArgs * self)
{
    ThroughputArgs_usage(&self->parent);

    AppLog_exception(
    "Where [extraoptions] are:\n"
    "\t-subscriberId <id>           - Subscriber Id. Used when running a "
                                       "test with several subscribers\n");
}

void
ThroughputSubscriberArgs_print_error(ThroughputSubscriberArgs * self)
{
    AppLog_exception("\nLast error detected: %s", self->parent._error_string);
    /* Explain where the error occurred if possible. */
}

void
ThroughputSubscriberArgs_initialize(ThroughputSubscriberArgs * self,
                                    const char *command)
{
    ThroughputArgs_initialize(&self->parent, command);
    self->parent.participant_id = DEFAULT_SUBSCRIBER_PARTICIPANT_ID;
    
    self->subscriberId = 0;
}

DDS_Boolean
ThroughputSubscriberArgs_parse_arguments(ThroughputSubscriberArgs * self,
                                         int argc, char **argv)
{
    /* This function is called recursively if there is a configuration file
     * specified on the command line. */

    char config_file_name[_MAX_FILENAME_LENGTH];
    int i;

    self->parent._args_valid = DDS_BOOLEAN_TRUE;        /* Assume all is OK */
    *config_file_name = 0;      /* Ensure that file name is not hanging around */

    /* Parse the arguments */
    for (i = 1; i < argc; ++i)
    {
        if (IS_OPTION(argv[i], "-file"))
        {
            /* Configuration file name */
            if (argv[++i] != NULL)
            {
                strncpy(config_file_name, argv[i], sizeof(config_file_name));
                /* Get local copy to use (recursion) */
            }
            else
            {
                self->parent._args_valid = DDS_BOOLEAN_FALSE;
                return DDS_BOOLEAN_FALSE;
            }
        }
        else if (IS_OPTION(argv[i], "-subscriberId"))
        {
            /* Packet size */
            if (argv[++i] != NULL)
            {
                self->subscriberId = strtol(argv[i], NULL, 10);
            }
            else
            {
                self->parent._args_valid = DDS_BOOLEAN_FALSE;
                return DDS_BOOLEAN_FALSE;
            }
        }
        else
        {                       /* Check to see if it is one of the common arguments */
            if (!ThroughputArgs_parse_common_args(&self->parent, &i, argv))
            {
                return DDS_BOOLEAN_FALSE;
            }
        }
    }

    if (strlen(config_file_name) > 0)
    {
    int arg_count;

        self->parent._args_valid =
            ThroughputArgs_construct_args_from_file(&self->parent,
                                                    config_file_name,
                                                    &arg_count);
        if (self->parent._args_valid)
        {
            /* Recurse using file arguments which take precedent */
            self->parent._args_valid =
                ThroughputSubscriberArgs_parse_arguments(self, arg_count,
                                                         self->parent.
                                                         _arg_variables);
        }
    }
    return self->parent._args_valid;
}

void
ThroughputSubscriberArgs_print_arguments(ThroughputSubscriberArgs * self)
{
    ThroughputArgs_print_arguments(&self->parent);

    AppLog_report("\t-subscriberId     = %d\n",
                  self->subscriberId);
}
