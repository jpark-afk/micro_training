/*
 (c) Copyright, Real-Time Innovations, 2009-2015.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.
*/

/*=======================================================================*/

#include "LatencyExample.h"

#include "LatencyPlugin.h"
#include "LatencySupport.h"

#include "PerformanceUdpTransform.h"

#include "disc_dpse/disc_dpse_dpsediscovery.h"
#include "wh_sm/wh_sm_history.h"
#include "rh_sm/rh_sm_history.h"
#include "netio/netio_udp.h"

#define REPLY_REGARDLESS_OF_COOKIE (255)

DDS_Boolean latency_plugin_use_key_in_message = DDS_BOOLEAN_FALSE;

void Communicator_initialize(Communicator *self, DDS_Boolean is_ready);
void RtiDdsCommunicator_initialize(RtiDdsCommunicator *self,
                       int domain_id, int index,
                       char* peer,
                       DDS_Boolean is_reliable,
                       DDS_Boolean use_key_in_topic,
                       int event_thread_priority,
                       DDS_Boolean is_ready,
                       char* udp_intf);
DDS_Boolean RtiDdsCommunicator_create_dds_entities(RtiDdsCommunicator *self,
                       const struct Latency *instance,
                       const char *send_topic_name,
                       const char *recv_topic_name);
void SenderEchoListener_initialize(SenderEchoListener *self,
                       LatencyDataProcessor *dp,
                       struct OSAPI_Semaphore* sem);
void SenderEchoListener_finalize(SenderEchoListener *self);
void SenderEchoListener_on_data_available(void* listener_data,
    DDS_DataReader* reader);
void SenderWriterListener_initialize(SenderWriterListener *self);
void SenderWriterListener_on_publication_matched(void *listener_data,
    DDS_DataWriter* writer, const struct DDS_PublicationMatchedStatus *status);
DDS_InstanceHandle_t SenderWriterListener_get_discovered_subscription_handle(
    SenderWriterListener *self, int order);

void ReceiverDataListener_initialize(ReceiverDataListener *self, int cookie);
void ReceiverDataListener_finalize(ReceiverDataListener *self);

void ReceiverDataListener_on_data_available(void* listener_data,
    DDS_DataReader* reader);

void ReceiverDataListener_set_writer(ReceiverDataListener *self,
    DDS_DataWriter* writer);

int ReceiverDataListener_num_messages(ReceiverDataListener *self);
int ReceiverDataListener_num_replies(ReceiverDataListener *self);

/* test is finished if the sn received is FINAL */
DDS_Boolean ReceiverDataListener_has_received_sentinel(
    ReceiverDataListener *self);


void
latency_plugin_use_keyed_data(DDS_Boolean useKey)
{
    latency_plugin_use_key_in_message = useKey;
}

void
Communicator_initialize(Communicator * self, DDS_Boolean is_ready)
{
    self->_is_ready = is_ready;
}

void
SenderEchoListener_initialize(SenderEchoListener * self,
                          LatencyDataProcessor * dp, struct OSAPI_Semaphore *sem)
{
    self->parent.on_before_sample_commit = NULL;
    self->parent.on_before_sample_deserialize = NULL;

    self->_data_processor = dp;
    self->_sem = sem;
    self->parent.as_listener.listener_data = (void *)self;
    Latency_initialize(&self->data);
}

void
SenderEchoListener_finalize(SenderEchoListener * self)
{
    Latency_finalize(&self->data);
}

void
SenderWriterListener_initialize(SenderWriterListener * self)
{
    self->_matched_subs_count = 0;
    self->parent.as_listener.listener_data = (void *)self;
}

void
ReceiverDataListener_initialize(ReceiverDataListener * self, int cookie)
{
    self->parent.on_before_sample_commit = NULL;
    self->parent.on_before_sample_deserialize = NULL;

    self->_instance_handle = DDS_HANDLE_NIL;
    self->_writer = NULL;
    self->_sequence_number = 0;
    self->_cookie = cookie;
    self->_num_messages = 0;
    self->_num_replies = 0;
    self->parent.as_listener.listener_data = (void *)self;
    Latency_initialize(&self->data);
    Latency_initialize(&self->_instance);
}

void
ReceiverDataListener_finalize(ReceiverDataListener * self)
{
    Latency_finalize(&self->data);
    Latency_finalize(&self->_instance);
}

int
ReceiverDataListener_num_messages(ReceiverDataListener * self)
{
    return self->_num_messages;
}

int
ReceiverDataListener_num_replies(ReceiverDataListener * self)
{
    return self->_num_replies;
}

DDS_Boolean
ReceiverDataListener_has_received_sentinel(ReceiverDataListener * self)
{
    return self->_sequence_number == FINAL_SEQUENCE_NUMBER;
}

void
on_subscription_matched(void *listener_data,
                        DDS_DataReader * reader,
                        const struct DDS_SubscriptionMatchedStatus *status)
{
}

void
on_offered_incompatible_qos(void *listener_data, DDS_DataWriter * writer,
                            const struct DDS_OfferedIncompatibleQosStatus
                            *status)
{
}

void
ReceiverDataListener_set_writer(ReceiverDataListener * self,
                                DDS_DataWriter * writer)
{
    self->_writer = writer;
}

/* ---------------------------------------------------------------------*/
void
on_requested_incompatible_qos(void *listener_data,
                              DDS_DataReader * reader,
                              const struct DDS_RequestedIncompatibleQosStatus
                              *status)
{
    AppLog_warn("Detected writer with incompatible QoS policy_id = %d\n",
                status->last_policy_id);
    AppLog_warn("            Please verify that you used "
                "the \"-reliable\" flag " "consistently in the subscriber\n");
}

/* ---------------------------------------------------------------------*/
/*i called when received echo */
void
SenderEchoListener_on_data_available(void *listener_data,
                                     DDS_DataReader * reader)
{
    SenderEchoListener *self = *(SenderEchoListener **) listener_data;
    DDS_ReturnCode_t ret_code;

    /* get all the data that the reader has received since the last 'take' */
    ret_code = DDS_DataReader_take_next_sample(reader, &self->data,
                                               &self->info);

    if (!LatencyDataProcessor_echo_received(self->_data_processor))
    {
        AppLog_exception("failed to call echo_received\n");
    }

    if (ret_code != DDS_RETCODE_OK)
    {
        AppLog_exception("failed to access data from the reader\n");
    }
    else
    {
        /* Must check the 'valid_data' because for some
         * samples the data will be NULL */
        if (self->info.valid_data &&
            (self->data.sequence_number ==
             self->_data_processor->_sequence_number))
        {
            LatencyDataProcessor_finish_one_issue_recv_thread
                (self->_data_processor);
        }
    }

    if (!OSAPI_Semaphore_give(self->_sem))
    {
        AppLog_exception("failed to give semaphore\n");
    }
}

/* ---------------------------------------------------------------------*/
/*i called when received echo and taking average latency */
void
SenderEchoListener_on_data_available_no_timestamp(void *listener_data,
                                                  DDS_DataReader * reader)
{
    SenderEchoListener *self = *(SenderEchoListener **) listener_data;
    DDS_ReturnCode_t ret_code;
    LatencyDataProcessor *data_processor;

    /* get all the data that the reader has received since the last 'take' */
    ret_code = DDS_DataReader_take_next_sample(reader, &self->data,
                                               &self->info);

    if (ret_code != DDS_RETCODE_OK)
    {
        AppLog_exception("failed to access data from the reader\n");
    }
    else
    {
        data_processor = self->_data_processor;
        /* Must check the 'valid_data' because for some
         * samples the data will be NULL */
        if (self->info.valid_data &&
            (self->data.sequence_number == data_processor->_sequence_number))
        {
            ++data_processor->_sequence_number;
            ++data_processor->_message_count;
            data_processor->_got_valid_echo = DDS_BOOLEAN_TRUE;
        }
    }

    if (!OSAPI_Semaphore_give(self->_sem))
    {
        AppLog_exception("failed to give semaphore\n");
    }
}

/* ---------------------------------------------------------------------*/
void
ReceiverDataListener_on_data_available(void *listener_data,
                                       DDS_DataReader * reader)
{
    ReceiverDataListener *self = *(ReceiverDataListener **) listener_data;
    DDS_ReturnCode_t ret_code;
    DDS_Octet *data0;

    /* get all the data that the reader has received since the last 'take' */
    ret_code = DDS_DataReader_take_next_sample(reader, &self->data,
                                               &self->info);

    if (ret_code != DDS_RETCODE_OK)
    {
        AppLog_exception("failed to access data from the reader\n");
    }
    else
    {
        /* Must check the 'valid_data' because for some
         * samples the data_seq[i] will be NULL */
        if (self->info.valid_data)
        {
            self->_sequence_number = self->data.sequence_number;
            Latency_copy(&self->_instance, &self->data);
            data0 = CDR_OctetSeq_get_reference(&self->data.data, 0);
            if ((self->_writer != NULL) &&
                ((*data0
                  == LATENCY_RESTRICT_RECV_COOKIE_NONE) ||
                 (*data0 == self->_cookie)))
            {
                /* echo back to the originator */
                ret_code = DDS_DataWriter_write(self->_writer, &self->_instance,
                                                &self->_instance_handle);
                if (ret_code != DDS_RETCODE_OK)
                {
                    AppLog_exception("failed to send data\n");
                }
                ++self->_num_replies;
            }
            ++self->_num_messages;
        }
    }
}

/* ---------------------------------------------------------------------*/
DDS_InstanceHandle_t
SenderWriterListener_get_discovered_subscription_handle(SenderWriterListener *
                                                        self, int order)
{
    if ((order <= 0) || (order > self->_matched_subs_count))
    {
        return DDS_HANDLE_NIL;
    }

    return self->_subscription_handle[order];
}

/* ---------------------------------------------------------------------*/
void
SenderWriterListener_on_publication_matched(void *listener_data,
                                            DDS_DataWriter * writer,
                                            const struct
                                            DDS_PublicationMatchedStatus
                                            *status)
{
    SenderWriterListener *self = *(SenderWriterListener **) listener_data;

    if (status->total_count_change <= 0)
    {
        /* unmatched */
        return;
    }

    AppLog_warn("[%d]", status->current_count);

    /* Save the order in which it was discovered as this indicates
     ** the order in which data will be sent
     */

    self->_subscription_handle[++self->_matched_subs_count] =
        status->last_subscription_handle;
}

/* ---------------------------------------------------------------------*/
/* ---------------------------------------------------------------------*/
void
RtiDdsCommunicator_initialize(RtiDdsCommunicator * self,
                          int domain_id, int index,
                          char *peer,
                          DDS_Boolean is_reliable,
                          DDS_Boolean use_key_in_topic,
                          int event_thread_priority,
                          DDS_Boolean is_ready,
                          char* udp_intf)
{
    Communicator_initialize(&self->parent, is_ready);
    self->_domain_id = domain_id;
    self->_index = index;
    self->_peer = peer;
    self->_is_reliable = is_reliable;
    self->_use_key_in_topic = use_key_in_topic;
    self->_event_thread_priority = event_thread_priority;
    self->_udp_intf = DDS_String_dup(udp_intf);
}

void
RtiDdsCommunicator_finalize(RtiDdsCommunicator * self)
{
    if (self->_udp_intf != NULL)
    {
        DDS_String_free(self->_udp_intf);
        self->_udp_intf = NULL;
    }
}

/* ---------------------------------------------------------------------*/
DDS_Boolean
RtiDdsCommunicator_send_data(RtiDdsCommunicator * self)
{
DDS_ReturnCode_t ret_code;

    ret_code =
        DDS_DataWriter_write(self->_writer, self->_send_instance,
                             &self->_instance_handle);
    return (ret_code == DDS_RETCODE_OK);
}

/* ---------------------------------------------------------------------*/
DDS_Boolean
RtiDdsCommunicator_needs_receiver_thread(RtiDdsCommunicator * self)
{
    return DDS_BOOLEAN_FALSE;
}

/* ---------------------------------------------------------------------*/
DDS_Boolean
RtiDdsCommunicator_create_dds_entities(RtiDdsCommunicator * self,
                                       const struct Latency * instance,
                                       const char *send_topic_name,
                                       const char *recv_topic_name)
{
    DDS_Boolean ok = DDS_BOOLEAN_FALSE;
    struct DDS_DomainParticipantFactoryQos factory_qos =
        DDS_DomainParticipantFactoryQos_INITIALIZER;
    struct DDS_DomainParticipantQos participant_qos =
        DDS_DomainParticipantQos_INITIALIZER;
    struct DDS_SubscriberQos subscriber_qos = DDS_SubscriberQos_INITIALIZER;
    struct DDS_PublisherQos publisher_qos = DDS_PublisherQos_INITIALIZER;
    struct DDS_DataReaderQos reader_qos = DDS_DataReaderQos_INITIALIZER;
    struct DDS_DataWriterQos writer_qos = DDS_DataWriterQos_INITIALIZER;
    struct DDS_TopicQos topic_qos = DDS_TopicQos_INITIALIZER;
    struct DPSE_DiscoveryPluginProperty discovery_plugin_properties =
        DPSE_DiscoveryPluginProperty_INITIALIZER;
    struct DDS_SubscriptionBuiltinTopicData rem_subscription_data =
        DDS_SubscriptionBuiltinTopicData_INITIALIZER;
    struct DDS_PublicationBuiltinTopicData rem_publication_data =
        DDS_PublicationBuiltinTopicData_INITIALIZER;
    RT_Registry_T* registry = NULL;
    struct UDP_InterfaceFactoryProperty *udp_property = NULL;

    /* DomainParticipantFactory declarations */
    DDS_DomainParticipantFactory *factory = NULL;

    /* Data declarations */
    self->_send_instance = instance;

    /*-------------------------- create dds entities ---------------------*/

    /* get handle to participant factory */
    factory = DDS_DomainParticipantFactory_get_instance();
    if (factory == NULL)
    {
        AppLog_exception("failed to get domain participant factory\n");
        goto done;
    }

    registry = DDS_DomainParticipantFactory_get_registry(factory);

    if (!RT_Registry_register(registry, DDSHST_WRITER_DEFAULT_HISTORY_NAME,
                              WHSM_HistoryFactory_get_interface(),NULL,NULL))
    {
        AppLog_exception("failed to register writer history\n");
        goto done;
    }

    if (!RT_Registry_register(registry, DDSHST_READER_DEFAULT_HISTORY_NAME,
                              RHSM_HistoryFactory_get_interface(),NULL,NULL))
    {
        AppLog_exception("failed to register reader history\n");
        goto done;
    }
    
    udp_property = (struct UDP_InterfaceFactoryProperty *)
        malloc(sizeof(struct UDP_InterfaceFactoryProperty));
    if (udp_property == NULL)
    {
        printf("failed to allocate udp properties\n");
        goto done;
    }
    *udp_property = UDP_INTERFACE_FACTORY_PROPERTY_DEFAULT;

    /* Configure UDP transport's allowed interfaces */
    if (!RT_Registry_unregister(registry, NETIO_DEFAULT_UDP_NAME, NULL, NULL))
    {
        AppLog_exception("failed to unregister udp\n");
        goto done;
    }


    /* For additional allowed interface(s), increase maximum and length, and
       set interface below:
    */
    if (!DDS_StringSeq_set_maximum(&udp_property->allow_interface,2))
    {
        printf("failed to set allow_interface maximum\n");
        goto done;
    }
    if (!DDS_StringSeq_set_length(&udp_property->allow_interface,2))
    {
        printf("failed to set allow_interface length\n");
        goto done;
    }

    /* loopback interface */
    *DDS_StringSeq_get_reference(&udp_property->allow_interface,0) =
            DDS_String_dup(DEFAULT_UDP_ALLOWED_INTERFACE_LOCAL);

    *DDS_StringSeq_get_reference(&udp_property->allow_interface,1) =
                DDS_String_dup(self->_udp_intf);

    udp_property->max_send_buffer_size = DEFAULT_UDP_MAX_SEND_BUFFER_SIZE;
    udp_property->max_receive_buffer_size = DEFAULT_UDP_MAX_RECEIVE_BUFFER_SIZE;

#ifdef UDP_TRANSFORMS_ENABLED
    /* register performance transformations */
    printf("Using UDP transformations.\n");
    if (!PerformanceUdpTransformFactory_register(registry,
                                                 "pt",
                                                 NULL))
    {
        AppLog_exception("failed to register udp transformation\n");
        goto done;
    }

    /* use this transformation to receive from any address and any mask */
    if (!UDP_TransformRules_assert_source_rule(&udp_property->source_rules, 
                                               0, 0, 
                                               "pt", 
                                               NULL))
    {
        AppLog_exception("failed to assert source transform\n");
        goto done;
    }

    /* use this transformation to send to any address and any mask */
    if (!UDP_TransformRules_assert_destination_rule(
                                            &udp_property->destination_rules, 
                                            0, 0, 
                                            "pt", 
                                            NULL))
    {
        AppLog_exception("failed to assert source transform\n");
        goto done;
    }
#endif /* UDP_TRANSFORMS_ENABLED */

    if (!RT_Registry_register(registry, NETIO_DEFAULT_UDP_NAME,
                         UDP_InterfaceFactory_get_interface(),
                        (struct RT_ComponentFactoryProperty*)udp_property, NULL))
    {
       printf("failed to register udp\n");
       goto done;
    }

    /* Do not auto enable participants upon creation since we want to register
     * transport plugins */
    if (DDS_DomainParticipantFactory_get_qos(factory, &factory_qos) !=
        DDS_RETCODE_OK)
    {
        AppLog_exception("failed to get factory QoS\n");
        goto done;
    }
    factory_qos.entity_factory.autoenable_created_entities = DDS_BOOLEAN_FALSE;

    if (DDS_DomainParticipantFactory_set_qos(factory, &factory_qos) !=
        DDS_RETCODE_OK)
    {
        AppLog_exception("failed to set factory QoS\n");
        goto done;
    }

    if (!RT_Registry_register(registry, "dpse", 
                              DPSE_DiscoveryFactory_get_interface(),
                              &discovery_plugin_properties._parent, NULL))
    {
        AppLog_exception("failed to register DPSE discovery\n");
        goto done;
    }

    if (!RT_ComponentFactoryId_set_name(&participant_qos.discovery.discovery.name,"dpse"))
    {
        AppLog_exception("failed to set discovery plugin name\n");
        goto done;
    }

    if (!DDS_StringSeq_set_maximum(&participant_qos.discovery.initial_peers,1))
    {
        printf("failed to set initial peers maximum\n");
        goto done;
    }
    if (!DDS_StringSeq_set_length(&participant_qos.discovery.initial_peers,1))
    {
        printf("failed to set initial peers length\n");
        goto done;
    }
    *DDS_StringSeq_get_reference(&participant_qos.discovery.initial_peers,0) = 
        DDS_String_dup(self->_peer);

    strcpy(participant_qos.participant_name.name, send_topic_name);
    participant_qos.resource_limits.remote_participant_allocation = 2;
    participant_qos.resource_limits.max_destination_ports = 32;
    participant_qos.resource_limits.max_receive_ports = 32;
    participant_qos.resource_limits.local_topic_allocation = 2;
    participant_qos.resource_limits.local_type_allocation = 10;
    participant_qos.resource_limits.local_writer_allocation = 2;
    participant_qos.resource_limits.local_reader_allocation = 2;
    participant_qos.resource_limits.remote_writer_allocation = 2;
    participant_qos.resource_limits.remote_reader_allocation = 2;

    /* create participant. 
     ** Defer enable till builtin transports are configured 
     */
    self->_participant =
        DDS_DomainParticipantFactory_create_participant(factory,
                                                        self->_domain_id,
                                                        &participant_qos,
                                                        NULL /* listener */ ,
                                                        DDS_STATUS_MASK_NONE);
    if (self->_participant == NULL)
    {
        AppLog_exception("failed to create domain participant\n");
        goto done;
    }

    /*----------------------------------------------------------------------
      Create a publisher and a subsciber, both with default QoSes.
      ----------------------------------------------------------------------*/

    /* optionally modify subscriber qos values */

    /* create and enable subscriber */
    self->_subscriber =
        DDS_DomainParticipant_create_subscriber(self->_participant,
                                                &subscriber_qos,
                                                NULL /* listener */ ,
                                                DDS_STATUS_MASK_NONE);
    if (self->_subscriber == NULL)
    {
        AppLog_exception("failed to create subscriber\n");
        goto done;
    }

    /* optionally modify publisher qos values here */

    /* create and enable publisher */
    self->_publisher =
        DDS_DomainParticipant_create_publisher(self->_participant,
                                               &publisher_qos,
                                               NULL /* listener */ ,
                                               DDS_STATUS_MASK_NONE);
    if (self->_publisher == NULL)
    {
        AppLog_exception("failed to create publisher\n");
        goto done;
    }

   /*--------------------------------------------------------------------
     Register data types, and create topics: recvTopic and sendTopic
     --------------------------------------------------------------------*/
    latency_plugin_use_keyed_data(self->_use_key_in_topic);
    if (LatencyTypeSupport_register_type(self->_participant, 
                                         LATENCY_TYPE_NAME) !=
        DDS_RETCODE_OK)
    {
        AppLog_exception("failed to register type: %s\n", LATENCY_TYPE_NAME);
        goto done;
    }

    /* Optionally create the topic to send. In the one-to-many
     * test all receivers except for one do not reply to the messages */
    self->_send_topic = NULL;
    if (send_topic_name != NULL)
    {
        self->_send_topic =
            DDS_DomainParticipant_create_topic(self->_participant,
                                               send_topic_name,
                                               LATENCY_TYPE_NAME,
                                               &DDS_TOPIC_QOS_DEFAULT,
                                               NULL /* listener */ ,
                                               DDS_STATUS_MASK_NONE);
        if (self->_send_topic == NULL)
        {
            AppLog_exception("failed to create data topic\n");
            goto done;
        }
    }

    self->_recv_topic =
        DDS_DomainParticipant_create_topic(self->_participant, recv_topic_name,
                                           LATENCY_TYPE_NAME,
                                           &DDS_TOPIC_QOS_DEFAULT,
                                           NULL /* listener */ ,
                                           DDS_STATUS_MASK_NONE);
    if (self->_recv_topic == NULL)
    {
        AppLog_exception("failed to create echo topic\n");
        goto done;
    }

    if (DPSE_RemoteParticipant_assert(self->_participant, recv_topic_name)
        != DDS_RETCODE_OK)
    {
        AppLog_exception("failed to assert remote participant\n");
        goto done;
    }

    /*---------------------------------------------------------------------
      Create one data writer: 
    -----------------------------------------------------------------------*/
    self->_writer = NULL;
    writer_qos.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    if (self->_send_topic != NULL)
    {
        /* writer_qos has DDS default values */
        writer_qos.history.kind = DDS_KEEP_LAST_HISTORY_QOS;
        writer_qos.history.depth = 1;

        if (!self->_is_reliable)
        {                       /* writer is reliable by default */
            writer_qos.reliability.kind = DDS_BEST_EFFORT_RELIABILITY_QOS;
        }
        else
        {
            writer_qos.resource_limits.max_samples =
                writer_qos.resource_limits.max_samples_per_instance = 3;
            writer_qos.protocol.rtps_reliable_writer.
                heartbeats_per_max_samples = 3;
            writer_qos.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
        }

        /* 1-to-1 for now */
        writer_qos.protocol.rtps_object_id = 100;

        /* create and enable writer. use sendTopic. */
        self->_writer =
            DDS_Publisher_create_datawriter(self->_publisher, self->_send_topic,
                                            &writer_qos, NULL /* listener */ ,
                                            DDS_STATUS_MASK_NONE);
        if (self->_writer == NULL)
        {
            AppLog_exception("failed to create writer\n");
            goto done;
        }

        /* For better performance register the instance to get the handle */
        self->_instance_handle = DDS_HANDLE_NIL;

        rem_subscription_data.key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] = 200;
        rem_subscription_data.topic_name = DDS_String_dup(send_topic_name);
        rem_subscription_data.type_name = DDS_String_dup(LATENCY_TYPE_NAME);

        if (DDS_RETCODE_OK !=
            DPSE_RemoteSubscription_assert(self->_participant,
                                           recv_topic_name,
                                           &rem_subscription_data,
                                           NDDS_TYPEPLUGIN_NO_KEY))
        {
            AppLog_exception("failed to assert remote subscription\n");
            goto done;
        }
    }

    /*----------------------------------------------------------------------
      Always create a data-reader
      ----------------------------------------------------------------------*/
    /* reader_qos already has DDS default values */

    if (self->_is_reliable)
    {                           /* reader is BE by default */
        reader_qos.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    }

    reader_qos.history.kind = DDS_KEEP_LAST_HISTORY_QOS;
    reader_qos.history.depth = 1;

    /* 1-to-1 for now */
    reader_qos.protocol.rtps_object_id = 200;

    /* create and enable reader. use recvTopic. */
    self->_reader =
        DDS_Subscriber_create_datareader(self->_subscriber,
                                         DDS_Topic_as_topicdescription
                                         (self->_recv_topic), &reader_qos, NULL,
                                         DDS_STATUS_MASK_NONE);
    if (self->_reader == NULL)
    {
        AppLog_exception("failed to create reader\n");
        goto done;
    }

    rem_publication_data.key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] = 100;
    rem_publication_data.topic_name = DDS_String_dup(recv_topic_name);
    rem_publication_data.type_name = DDS_String_dup(LATENCY_TYPE_NAME);
    if (self->_is_reliable)
    {
        rem_publication_data.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    }

    if (DDS_RETCODE_OK !=
        DPSE_RemotePublication_assert(self->_participant,
                                      recv_topic_name,
                                      &rem_publication_data,
                                      NDDS_TYPEPLUGIN_NO_KEY))
    {
        AppLog_exception("failed to assert remote publication\n");
        goto done;
    }

    ok = DDS_BOOLEAN_TRUE;

    done:
    DDS_DomainParticipantFactoryQos_finalize(&factory_qos);
    DDS_DomainParticipantQos_finalize(&participant_qos);
    DDS_SubscriberQos_finalize(&subscriber_qos);
    DDS_PublisherQos_finalize(&publisher_qos);
    DDS_DataReaderQos_finalize(&reader_qos);
    DDS_DataWriterQos_finalize(&writer_qos);
    DDS_SubscriptionBuiltinTopicData_finalize(&rem_subscription_data);
    DDS_PublicationBuiltinTopicData_finalize(&rem_publication_data);

    return ok;
}

/* ---------------------------------------------------------------------*/
DDS_Boolean
RtiDdsSenderCommunicator_cleanup(RtiDdsSenderCommunicator * self)
{
    /* Delete all DDS entities created in the prepare */
    DDS_DomainParticipantFactory *factory =
        DDS_DomainParticipantFactory_get_instance();
    RT_Registry_T* registry = NULL;
    struct UDP_InterfaceFactoryProperty *udp_property = NULL;

    /*------------------------ tear down dds entities ----------------------*/
    SenderEchoListener_finalize(&self->_echo_reader_listener);

    if (DDS_DomainParticipant_delete_contained_entities
           (self->parent._participant) != DDS_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (DDS_DomainParticipantFactory_delete_participant
           (factory, self->parent._participant) != DDS_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    registry = DDS_DomainParticipantFactory_get_registry(factory);

#if PERF_TRANSFORMS_ENABLED
    /* unregister the performance transformation */
    if (!PerformanceUdpTransformFactory_unregister(registry,
                                                   "pt",
                                                   NULL))
    {
        AppLog_exception("unregister transformations error\n");
        return DDS_BOOLEAN_FALSE;
    }

#endif

    if (!RT_Registry_unregister(registry, NETIO_DEFAULT_UDP_NAME,
                        (struct RT_ComponentFactoryProperty**)&udp_property,
                        NULL))
    {
       printf("failed to unregister udp\n");
       return DDS_BOOLEAN_FALSE;
    }
    if (udp_property != NULL)
    {
        UDP_InterfaceFactoryProperty_finalize(udp_property);
        free(udp_property);
        udp_property = NULL;
    }

    if (!RT_Registry_unregister(registry, NETIO_DEFAULT_UDP_NAME,
                        (struct RT_ComponentFactoryProperty**)&udp_property,
                        NULL))
    {
       printf("failed to unregister udp\n");
       return DDS_BOOLEAN_FALSE;
    }
    if (udp_property != NULL)
    {
        UDP_InterfaceFactoryProperty_finalize(udp_property);
        free(udp_property);
        udp_property = NULL;
    }
    if (!RT_Registry_unregister(registry, "dpse", NULL, NULL))
    {
        return DDS_BOOLEAN_FALSE;
    }
    if (!RT_Registry_unregister(registry, DDSHST_READER_DEFAULT_HISTORY_NAME, 
                                NULL, NULL))
    {
        return DDS_BOOLEAN_FALSE;
    }
    if (!RT_Registry_unregister(registry, DDSHST_WRITER_DEFAULT_HISTORY_NAME, 
                                NULL, NULL))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (DDS_DomainParticipantFactory_finalize_instance() != DDS_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}


/* ---------------------------------------------------------------------*/
DDS_Boolean
RtiDdsSenderCommunicator_is_ready(RtiDdsSenderCommunicator * self)
{
    return (self->_data_writer_listener._matched_subs_count >=
            self->_subscribers);
}

/* ---------------------------------------------------------------------*/
int
RtiDdsSenderCommunicator_get_echoer_cookie(RtiDdsSenderCommunicator * self)
{
    return self->_subscribers;
}

/* ---------------------------------------------------------------------*/
DDS_Boolean
RtiDdsSenderCommunicator_prepare(RtiDdsSenderCommunicator * self,
                                 struct Latency * instance,
                                 LatencyDataProcessor * data_processor,
                                 DDS_Boolean measure_individual_latencies)
{
    DDS_ReturnCode_t ret_code = DDS_RETCODE_ERROR;

    /* Use participant index = 0 for sender */
    if (!RtiDdsCommunicator_create_dds_entities(&self->parent,
                                                instance,
                                                LATENCY_DATA_TOPIC_NAME,
                                                LATENCY_ECHO_TOPIC_NAME))
    {
        AppLog_exception("failed to create dds entities\n");
        return DDS_BOOLEAN_FALSE;
    }

    /*** INTERNAL API: OSAPI_Semaphore_XXX ***/
    self->sem = OSAPI_Semaphore_new();

    /* Listener declarations */
    SenderEchoListener_initialize(&self->_echo_reader_listener, data_processor,
                              self->sem);

    /* setup reader to access received data via a listener 
     * explicitly specify which operations in the listener are
     * activated for callback
     */
    self->_echo_reader_listener.parent.on_requested_incompatible_qos =
        on_requested_incompatible_qos;
    self->_echo_reader_listener.parent.on_subscription_matched =
        on_subscription_matched;
    if (measure_individual_latencies)
    {
        self->_echo_reader_listener.parent.on_data_available =
            SenderEchoListener_on_data_available;
    }
    else
    {
        self->_echo_reader_listener.parent.on_data_available =
            SenderEchoListener_on_data_available_no_timestamp;
    }
    ret_code =
        DDS_DataReader_set_listener(self->parent._reader,
                                    &self->_echo_reader_listener.parent,
                                    DDS_DATA_AVAILABLE_STATUS |
                                    DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS);
    if (ret_code != DDS_RETCODE_OK)
    {
        AppLog_exception("failed to set echo reader listener\n");
        return DDS_BOOLEAN_FALSE;
    }

    SenderWriterListener_initialize(&self->_data_writer_listener);

    /* Setup reader to access received data via a listener. 
     ** Explicitly specify which operations in the listener are
     ** activated for callback
     */
    self->_data_writer_listener.parent.on_offered_incompatible_qos =
        on_offered_incompatible_qos;
    self->_data_writer_listener.parent.on_publication_matched =
        SenderWriterListener_on_publication_matched;
    ret_code =
        DDS_DataWriter_set_listener(self->parent._writer,
                                    &self->_data_writer_listener.parent,
                                    DDS_PUBLICATION_MATCHED_STATUS |
                                    DDS_OFFERED_INCOMPATIBLE_QOS_STATUS);
    if (ret_code != DDS_RETCODE_OK)
    {
        AppLog_exception("failed to set data writer listener\n");
        return DDS_BOOLEAN_FALSE;
    }

    /* now bring up the participant */
    ret_code =
        DDS_Entity_enable(DDS_DomainParticipant_as_entity
                          (self->parent._participant));
    if (ret_code != DDS_RETCODE_OK)
    {
        AppLog_exception("failed to enable participant\n");
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}


void
RtiDdsSenderCommunicator_finalize(RtiDdsSenderCommunicator * self)
{
    RtiDdsCommunicator_finalize(&self->parent);

    /*** INTERNAL API: OSAPI_Semaphore_XXX ***/
    if (self->sem != NULL)
    {
        OSAPI_Semaphore_delete(self->sem);
        self->sem = NULL;
    }
}

/* ---------------------------------------------------------------------*/
void
RtiDdsSenderCommunicator_initialize(RtiDdsSenderCommunicator * self,
                                int domain_id,
                                char *peer,
                                DDS_Boolean is_reliable,
                                DDS_Boolean use_key_in_topic,
                                int event_thread_priority,
                                DDS_Boolean is_ready,
                                int subscribers,
                                char* udp_intf)
{
    RtiDdsCommunicator_initialize(&self->parent,
                    domain_id, 0,
                    (peer != NULL)? peer : DEFAULT_INITIAL_PEER_PUB,
                    is_reliable,
                    use_key_in_topic,
                    event_thread_priority,
                    is_ready,
                    (udp_intf != NULL)? udp_intf : DEFAULT_UDP_ALLOWED_INTERFACE_EXT);
    self->sem = NULL;
    self->_subscribers = subscribers;
}

/* ---------------------------------------------------------------------*/

void
RtiDdsReceiverCommunicator_finalize(RtiDdsReceiverCommunicator *self)
{
    RtiDdsCommunicator_finalize(&self->parent);
}

void
RtiDdsReceiverCommunicator_initialize(RtiDdsReceiverCommunicator * self,
                                  int domain_id,
                                  int index,
                                  char *peer,
                                  DDS_Boolean is_reliable,
                                  DDS_Boolean use_key_in_topic,
                                  int event_thread_priority,
                                  int cookie,
                                  DDS_Boolean echo_disabled,
                                  char* udp_intf)
{
    RtiDdsCommunicator_initialize(&self->parent,
                        domain_id, index,
                        (peer != NULL)? peer : DEFAULT_INITIAL_PEER_SUB,
                        is_reliable,
                        use_key_in_topic,
                        event_thread_priority,
                        DDS_BOOLEAN_TRUE /* receiver is always ready */,
                        (udp_intf != NULL)? udp_intf : DEFAULT_UDP_ALLOWED_INTERFACE_EXT);
    self->_cookie = cookie;
    self->_echo_disabled = echo_disabled;
}

/* ---------------------------------------------------------------------*/
DDS_Boolean
RtiDdsReceiverCommunicator_has_received_sentinel(RtiDdsReceiverCommunicator *
                                                 self)
{
    return ReceiverDataListener_has_received_sentinel(&self->_data_listener);
}

/* ---------------------------------------------------------------------*/
int
RtiDdsReceiverCommunicator_recv_message_count(RtiDdsReceiverCommunicator * self)
{
    return ReceiverDataListener_num_messages(&self->_data_listener);
}

/* ---------------------------------------------------------------------*/
int
RtiDdsReceiverCommunicator_sent_message_count(RtiDdsReceiverCommunicator * self)
{
    return ReceiverDataListener_num_replies(&self->_data_listener);
}

/* ---------------------------------------------------------------------*/
DDS_Boolean
RtiDdsReceiverCommunicator_prepare(RtiDdsReceiverCommunicator * self,
                                   struct Latency * instance,
                                   LatencyDataProcessor * data_processor)
{
DDS_ReturnCode_t ret_code = DDS_RETCODE_ERROR;
const char *send_topic_name = NULL;

    if (self->_echo_disabled == DDS_BOOLEAN_FALSE)
    {
        send_topic_name = LATENCY_ECHO_TOPIC_NAME;
    }

    /* Use participant index = _cookie for receiver */
    if (!RtiDdsCommunicator_create_dds_entities(&self->parent, instance,
                                                send_topic_name,
                                                LATENCY_DATA_TOPIC_NAME))
    {
        return DDS_BOOLEAN_FALSE;
    }

    /* Listener declarations */
    ReceiverDataListener_initialize(&self->_data_listener, self->_cookie);
    ReceiverDataListener_set_writer(&self->_data_listener,
                                    self->parent._writer);

    /* save the cookie as a cookie in the user data so that the
     ** writer can identify the reader it wants to reply 
     */

    /* Setup reader to access received data via a listener. 
     ** Explicitly specify which operations in the listener are
     ** activated for callback
     */
    self->_data_listener.parent.on_requested_incompatible_qos =
        on_requested_incompatible_qos;
    self->_data_listener.parent.on_subscription_matched =
        on_subscription_matched;
    self->_data_listener.parent.on_data_available =
        ReceiverDataListener_on_data_available;
    ret_code =
        DDS_DataReader_set_listener(self->parent._reader,
                                    &self->_data_listener.parent,
                                    DDS_DATA_AVAILABLE_STATUS |
                                    DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS |
                                    DDS_SUBSCRIPTION_MATCHED_STATUS);
    if (ret_code != DDS_RETCODE_OK)
    {
        AppLog_exception("failed to set listener\n");
        return DDS_BOOLEAN_FALSE;
    }

    /* bring up participant */
    ret_code =
        DDS_Entity_enable(DDS_DomainParticipant_as_entity
                          (self->parent._participant));
    if (ret_code != DDS_RETCODE_OK)
    {
        AppLog_exception("failed to enable participant\n");
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}


/* ---------------------------------------------------------------------*/
DDS_Boolean
RtiDdsReceiverCommunicator_cleanup(RtiDdsReceiverCommunicator * self)
{
    /* Delete all DDS entities created in the prepare */
    DDS_DomainParticipantFactory *factory =
        DDS_DomainParticipantFactory_get_instance();
    RT_Registry_T* registry = NULL;
    struct UDP_InterfaceFactoryProperty *udp_property = NULL;

    /*------------------------ tear down dds entities ----------------------*/
    ReceiverDataListener_finalize(&self->_data_listener);

    if (DDS_DomainParticipant_delete_contained_entities
           (self->parent._participant) != DDS_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (DDS_DomainParticipantFactory_delete_participant
           (factory, self->parent._participant) != DDS_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    registry = DDS_DomainParticipantFactory_get_registry(factory);

#if PERF_TRANSFORMS_ENABLED
    /* unregister the performance transformation */
    if (!PerformanceUdpTransformFactory_unregister(registry,
                                                   "pt",
                                                   NULL))
    {
        return DDS_BOOLEAN_FALSE;
    }
#endif

    if (!RT_Registry_unregister(registry, NETIO_DEFAULT_UDP_NAME,
                        (struct RT_ComponentFactoryProperty**)&udp_property,
                        NULL))
    {
        printf("failed to unregister udp\n");
        return DDS_BOOLEAN_FALSE;
    }
    if (udp_property != NULL)
    {
        UDP_InterfaceFactoryProperty_finalize(udp_property);
        free(udp_property);
        udp_property = NULL;
    }

    if (!RT_Registry_unregister(registry, NETIO_DEFAULT_UDP_NAME,
                        (struct RT_ComponentFactoryProperty**)&udp_property,
                        NULL))
    {
       printf("failed to unregister udp\n");
       return DDS_BOOLEAN_FALSE;
    }
    if (udp_property != NULL)
    {
        UDP_InterfaceFactoryProperty_finalize(udp_property);
        free(udp_property);
        udp_property = NULL;
    }
    if (!RT_Registry_unregister(registry, "dpse", NULL, NULL))
    {
        return DDS_BOOLEAN_FALSE;
    }
    if (!RT_Registry_unregister(registry, DDSHST_READER_DEFAULT_HISTORY_NAME, 
                                NULL, NULL))
    {
        return DDS_BOOLEAN_FALSE;
    }
    if (!RT_Registry_unregister(registry, DDSHST_WRITER_DEFAULT_HISTORY_NAME, 
                                NULL, NULL))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (DDS_DomainParticipantFactory_finalize_instance() != DDS_RETCODE_OK)
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}
