/*
 (c) Copyright, Real-Time Innovations, $Date: 2009-2015/05/17 02:59:54 $.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.
*/

/*=======================================================================*/
#ifndef Communicator_h
#define Communicator_h

/* ---------------------------------------------------------------------*/

typedef struct Communicator_t {
    DDS_Boolean _is_ready;
} Communicator;

typedef struct RtiDdsCommunicator_t {
    Communicator parent;
    DDS_DomainId_t _domain_id;
    int _index;
    char* _peer;
    char* _udp_intf;
    DDS_Boolean _is_reliable;
    DDS_Boolean _use_key_in_topic;
    int  _event_thread_priority;

    DDS_DomainParticipant* _participant;
    DDS_Subscriber* _subscriber;
    DDS_Publisher* _publisher;
    DDS_Topic* _send_topic;
    DDS_Topic* _recv_topic;

    DDS_DataReader* _reader;
    DDS_DataWriter* _writer;

    DDS_InstanceHandle_t _instance_handle;
    const struct Latency *_send_instance;
} RtiDdsCommunicator;

typedef struct SenderEchoListener_t {
    struct DDS_DataReaderListener parent;
    LatencyDataProcessor *_data_processor;
    Latency data;
    struct DDS_SampleInfo info;

    /*** INTERNAL API: OsapiSemaphore ***/
    struct OSAPI_Semaphore* _sem;
} SenderEchoListener;

typedef struct SenderWriterListener_t {
    struct DDS_DataWriterListener parent;
    DDS_InstanceHandle_t _subscription_handle[LATENCY_MAX_SUBSCRIPTIONS];
    int _matched_subs_count;
} SenderWriterListener;

typedef struct RtiDdsSenderCommunicator_t {
    RtiDdsCommunicator parent;
    SenderEchoListener _echo_reader_listener;
    SenderWriterListener _data_writer_listener;
    int _subscribers;
    struct OSAPI_Semaphore* sem;
} RtiDdsSenderCommunicator;

typedef struct ReceiverDataListener_t {
    struct DDS_DataReaderListener parent;
    Latency _instance;
    Latency data;
    struct DDS_SampleInfo info;
    DDS_InstanceHandle_t _instance_handle;
    DDS_DataWriter* _writer;
    DDS_Long _sequence_number;
    int _cookie;
    int _num_messages;
    int _num_replies;
} ReceiverDataListener;

typedef struct RtiDdsReceiverCommunicator_t {
    RtiDdsCommunicator parent;
    ReceiverDataListener _data_listener;
    int _cookie;
    DDS_Boolean _echo_disabled;
} RtiDdsReceiverCommunicator;

/* ---------------------------------------------------------------------*/

void Communicator_delete(Communicator *self);

/* ---------------------------------------------------------------------*/

DDS_Boolean RtiDdsCommunicator_send_data(RtiDdsCommunicator *self);
DDS_Boolean RtiDdsCommunicator_needs_receiver_thread(RtiDdsCommunicator *self);

/* ---------------------------------------------------------------------*/

DDS_Boolean RtiDdsSenderCommunicator_prepare(RtiDdsSenderCommunicator *self,
                                struct Latency *instance,
                                LatencyDataProcessor *data_processor,
                                DDS_Boolean measure_individual_latencies);
DDS_Boolean RtiDdsSenderCommunicator_cleanup(RtiDdsSenderCommunicator *self);
DDS_Boolean RtiDdsSenderCommunicator_is_ready(RtiDdsSenderCommunicator *self);
int RtiDdsSenderCommunicator_get_echoer_cookie(RtiDdsSenderCommunicator *self);
void RtiDdsSenderCommunicator_finalize(RtiDdsSenderCommunicator *self);
void RtiDdsSenderCommunicator_initialize(RtiDdsSenderCommunicator *self,
                       int domain_id,
                       char* peer,
                       DDS_Boolean is_reliable,
                       DDS_Boolean use_key_in_topic,
                       int event_thread_priority,
                       DDS_Boolean is_ready,
                       int subscribers,
                       char *udp_intf);


/* ---------------------------------------------------------------------*/

void RtiDdsReceiverCommunicator_initialize(RtiDdsReceiverCommunicator *self,
                               int domain_id,
                               int index,
                               char* peer,
                               DDS_Boolean is_reliable,
                               DDS_Boolean use_key_in_topic,
                               int event_thread_priority,
                               int cookie,
                               DDS_Boolean echo_disabled,
                               char* udp_intf);

DDS_Boolean RtiDdsReceiverCommunicator_prepare(RtiDdsReceiverCommunicator *self,
                                struct Latency *instance,
                                LatencyDataProcessor *data_processor);
DDS_Boolean RtiDdsReceiverCommunicator_has_received_sentinel(
    RtiDdsReceiverCommunicator *self);
int RtiDdsReceiverCommunicator_recv_message_count(RtiDdsReceiverCommunicator *self);
int RtiDdsReceiverCommunicator_sent_message_count(RtiDdsReceiverCommunicator *self);
DDS_Boolean RtiDdsReceiverCommunicator_cleanup(RtiDdsReceiverCommunicator *self);
void RtiDdsReceiverCommunicator_finalize(RtiDdsReceiverCommunicator *self);

#endif/* Communicator_h*/

