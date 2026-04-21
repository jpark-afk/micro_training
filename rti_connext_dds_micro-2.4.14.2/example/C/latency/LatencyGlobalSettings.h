/*
 (c) Copyright, Real-Time Innovations, 2009-2015
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.
*/

/*=======================================================================*/

#ifndef latency_global_settings_h
#define latency_global_settings_h

/*-------------- General Test Settings --------------*/
/* Max number of different issue sizes tested */
#define LATENCY_ROUND_MAX (16)
/* Max number of remote subscriptions when using a transport plug-in */
#define LATENCY_MAX_SUBSCRIPTIONS (32)
/* Number of loops to calculate clock overhead */
#define NUM_OF_LOOPS_CLOCK (320)

/*-------------- DDS Topics Settings --------------*/
/* DataTopic: from LatencyWriter to LatencyReader */
#define LATENCY_DATA_TOPIC_NAME    "DataTopic"
/* EchoTopic: from LatencyReader to LatencyWriter */
#define LATENCY_ECHO_TOPIC_NAME    "EchoTopic"
/* Type name */
#define LATENCY_TYPE_NAME LatencyTYPENAME

/* UDP Interfaces to be registered, one local and one external */
#if defined(RTI_DARWIN)
#define DEFAULT_UDP_ALLOWED_INTERFACE_LOCAL "lo0"
#define DEFAULT_UDP_ALLOWED_INTERFACE_EXT     "en1"
#elif defined (RTI_LINUX)
#define DEFAULT_UDP_ALLOWED_INTERFACE_LOCAL "lo"
#define DEFAULT_UDP_ALLOWED_INTERFACE_EXT     "eth0"
#elif defined (RTI_VXWORKS)
#define DEFAULT_UDP_ALLOWED_INTERFACE_LOCAL "lo0"
#define DEFAULT_UDP_ALLOWED_INTERFACE_EXT     "geisc0"
#elif defined(RTI_WIN32)
#define DEFAULT_UDP_ALLOWED_INTERFACE_LOCAL "Loopback Pseudo-Interface 1"
#define DEFAULT_UDP_ALLOWED_INTERFACE_EXT     "Local Area Connection"
#else
#define DEFAULT_UDP_ALLOWED_INTERFACE_LOCAL "lo"
#define DEFAULT_UDP_ALLOWED_INTERFACE_EXT     "ce0"
#endif

/* Max size of send and receive buffers in the UDP transport */
#define DEFAULT_UDP_MAX_SEND_BUFFER_SIZE    64*1024
#define DEFAULT_UDP_MAX_RECEIVE_BUFFER_SIZE    64*1024

/* Initial peer to be contacted by LatencyPublisher */
#define DEFAULT_INITIAL_PEER_PUB    "127.0.0.1"
/* Initial peer to be contacted by LatencySubscriber */
#define DEFAULT_INITIAL_PEER_SUB    "127.0.0.1"

/* DDS Domain ID to use */
#define DEFAULT_DOMAIN_ID             12
/* Time in ms between multiple operations */
#define DEFAULT_SLEEP_TIME             2000

#endif /* #ifndef latency_global_settings_h */
