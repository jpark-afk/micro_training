/*
 (c) Copyright, Real-Time Innovations, $Date: 2009-2015/05/09 14:17:40 $.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.
*/

#ifndef ThroughputQos_h
#define ThroughputQos_h

#ifndef ThroughputArgs_h
#include "ThroughputArgs.h"
#endif
#ifndef PerformanceUdpTransform_h
#include "PerformanceUdpTransform.h"
#endif

#include "disc_dpse/disc_dpse_dpsediscovery.h"

DDS_ReturnCode_t configure_factory_qos(
    struct DDS_DomainParticipantFactoryQos *factory_qos,
    DDS_DomainParticipantFactory *factory);

DDS_ReturnCode_t
configure_transport_qos(struct UDP_InterfaceFactoryProperty *udp_property,
                        DDS_DomainParticipantFactory * factory);

DDS_ReturnCode_t configure_participant_qos(
    struct DDS_DomainParticipantQos *participant_qos,
    DDS_DomainParticipantFactory *factory, ThroughputArgs *args,
    const char *participant_name, DDS_Long remote_app);

DDS_ReturnCode_t configure_throughput_writer_qos(
    struct DDS_DataWriterQos *throughput_writer_qos,
    DDS_Publisher *publisher, ThroughputPublisherArgs *args);

DDS_ReturnCode_t configure_throughput_reader_qos(
    struct DDS_DataReaderQos *throughput_reader_qos, DDS_Subscriber *subscriber,
    ThroughputSubscriberArgs *args);


DDS_ReturnCode_t configure_command_reader_qos(
    struct DDS_DataReaderQos *throughput_command_reader_qos, 
    DDS_Subscriber *subscriber);

DDS_ReturnCode_t configure_command_writer_qos(
    struct DDS_DataWriterQos *throughput_command_writer_qos, 
    DDS_Publisher *publisher, ThroughputPublisherArgs *args);

void get_subscriber_name(char *buffer, int subscriberId);

#endif /* ThroughputQos_h */
