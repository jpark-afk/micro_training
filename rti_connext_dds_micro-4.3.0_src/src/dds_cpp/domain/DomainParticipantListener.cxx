/* 

 (c) Copyright, Real-Time Innovations, 2013-2015.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.

 modification history
--------------------
 12feb2016,as  Created
===================================================================== */
#ifndef dds_cpp_domain_hxx
  #include "dds_cpp/dds_cpp_domain.hxx"
#endif

#include "DomainParticipant.hxx"

/*** SOURCE_BEGIN ***/

void
DDSDomainParticipantListener_forward_on_data_available(
   void* listener_data,
   DDS_DataReader* c_datareader)
{
    DDSDomainParticipantListener* reader_listener =
        (DDSDomainParticipantListener*) listener_data;

    if (reader_listener != NULL)
    {
        DDSDataReader* datareader =
                (DDSDataReader *)DDS_Entity_get_wrapper(
                        DDS_DataReader_as_entity(c_datareader));
        reader_listener->on_data_available(datareader);
    }
}

void
DDSDomainParticipantListener_forward_on_requested_deadline_missed(
        void *listener_data,
        DDS_DataReader* c_reader,
        const DDS_RequestedDeadlineMissedStatus *status)
{
    DDSDomainParticipantListener* reader_listener =
            (DDSDomainParticipantListener*) listener_data;


    if (reader_listener != NULL)
    {
        DDSDataReader* datareader =
            (DDSDataReader *)DDS_Entity_get_wrapper(
                    DDS_DataReader_as_entity(c_reader));
        reader_listener->on_requested_deadline_missed(datareader, *status);
    }
}


void
DDSDomainParticipantListener_forward_on_liveliness_changed(
        void *listener_data,
        DDS_DataReader *c_reader,
        const DDS_LivelinessChangedStatus *status)
{
    DDSDomainParticipantListener* reader_listener =
                (DDSDomainParticipantListener*) listener_data;


    if (reader_listener != NULL)
    {
        DDSDataReader* datareader =
                (DDSDataReader *)DDS_Entity_get_wrapper(
                        DDS_DataReader_as_entity(c_reader));
        reader_listener->on_liveliness_changed(datareader, *status);
    }
}

void
DDSDomainParticipantListener_forward_on_requested_incompatible_qos(
        void *listener_data,
        DDS_DataReader *c_reader,
        const DDS_RequestedIncompatibleQosStatus *status)
{
    DDSDomainParticipantListener* reader_listener =
                    (DDSDomainParticipantListener*) listener_data;

    DDSDataReader* datareader =
                    (DDSDataReader *)DDS_Entity_get_wrapper(
                            DDS_DataReader_as_entity(c_reader));

    if (reader_listener != NULL)
    {
        reader_listener->on_requested_incompatible_qos(datareader, *status);
    }
}

void
DDSDomainParticipantListener_forward_on_sample_rejected(
        void *listener_data,
        DDS_DataReader *c_reader,
        const DDS_SampleRejectedStatus *status)
{
    DDSDomainParticipantListener* reader_listener =
                    (DDSDomainParticipantListener*) listener_data;

    if (reader_listener != NULL)
    {
        DDSDataReader* datareader =
                    (DDSDataReader *)DDS_Entity_get_wrapper(
                            DDS_DataReader_as_entity(c_reader));
        reader_listener->on_sample_rejected(datareader, *status);
    }
}

void
DDSDomainParticipantListener_forward_on_subscription_matched(
        void *listener_data,
        DDS_DataReader *c_reader,
        const DDS_SubscriptionMatchedStatus *status)
{
    DDSDomainParticipantListener* reader_listener =
                    (DDSDomainParticipantListener*) listener_data;

    if (reader_listener != NULL)
    {
        DDSDataReader* datareader =
                    (DDSDataReader *)DDS_Entity_get_wrapper(
                            DDS_DataReader_as_entity(c_reader));

        reader_listener->on_subscription_matched(datareader, *status);
    }
}

void
DDSDomainParticipantListener_forward_on_sample_lost(
        void *listener_data,
        DDS_DataReader *c_reader,
        const DDS_SampleLostStatus *status)
{
    DDSDomainParticipantListener* reader_listener =
                    (DDSDomainParticipantListener*) listener_data;

    if (reader_listener != NULL)
    {
        DDSDataReader* datareader =
                    (DDSDataReader *)DDS_Entity_get_wrapper(
                            DDS_DataReader_as_entity(c_reader));

        reader_listener->on_sample_lost(datareader, *status);
    }
}

void
DDSDomainParticipantListener_forward_on_instance_replaced(
        void *listener_data,
        DDS_DataReader *c_reader,
        const DDS_DataReaderInstanceReplacedStatus *status)
{
    DDSDomainParticipantListener* reader_listener =
                    (DDSDomainParticipantListener*) listener_data;

    if (reader_listener != NULL)
    {
        DDSDataReader* datareader =
                    (DDSDataReader *)DDS_Entity_get_wrapper(
                            DDS_DataReader_as_entity(c_reader));

        reader_listener->on_instance_replaced(datareader, *status);
    }
}

DDS_Boolean
DDSDomainParticipantListener_forward_on_before_sample_deserialize(
        void *listener_data,
        DDS_DataReader *c_reader,
        NDDS_Type_Plugin *plugin,
        CDR_Stream_t *stream,
        DDS_Boolean *dropped)
{
    DDSDomainParticipantListener* reader_listener =
                    (DDSDomainParticipantListener*) listener_data;

    if (reader_listener != NULL)
    {
        DDSDataReader* datareader =
                    (DDSDataReader *)DDS_Entity_get_wrapper(
                            DDS_DataReader_as_entity(c_reader));

        return reader_listener->on_before_sample_deserialize(
                                    datareader,
                                    plugin,
                                    stream,
                                    dropped);
    }
    else
    {
        return RTI_TRUE;
    }
}

DDS_Boolean
DDSDomainParticipantListener_forward_on_before_sample_commit(
        void *listener_data,
        DDS_DataReader *c_reader,
        const void *const sample,
        const DDS_SampleInfo *const sample_info,
        DDS_Boolean *dropped)
{
    DDSDomainParticipantListener* reader_listener =
                    (DDSDomainParticipantListener*) listener_data;

    if (reader_listener != NULL)
    {
        DDSDataReader* datareader =
                    (DDSDataReader *)DDS_Entity_get_wrapper(
                            DDS_DataReader_as_entity(c_reader));

        return reader_listener->on_before_sample_commit(
                                            datareader,
                                            sample,
                                            sample_info,
                                            dropped);
    }
    else
    {
        return RTI_TRUE;
    }
}

void
DDSDomainParticipantListener_forward_on_offered_deadline_missed(
        void *listener_data,
        DDS_DataWriter* c_writer,
        const DDS_OfferedDeadlineMissedStatus* status)
{
    DDSDataWriter *writer = NULL;
    DDSDomainParticipantListener *writer_listener = NULL;

    writer_listener = (DDSDomainParticipantListener*) listener_data;
    if (writer_listener != NULL)
    {
        writer = (DDSDataWriter*)
            DDS_Entity_get_wrapper(DDS_DataWriter_as_entity(c_writer));
        writer_listener->on_offered_deadline_missed(writer, *status);
    }
}

void DDSDomainParticipantListener_forward_on_liveliness_lost(
        void *listener_data,
        DDS_DataWriter *c_writer,
        const DDS_LivelinessLostStatus *status)
{
    DDSDataWriter *writer = NULL;
    DDSDomainParticipantListener *writer_listener = NULL;
    writer_listener = (DDSDomainParticipantListener*) listener_data;
    if (writer_listener != NULL)
    {
        writer = (DDSDataWriter*)
                DDS_Entity_get_wrapper(DDS_DataWriter_as_entity(c_writer));
        writer_listener->on_liveliness_lost(writer, *status);
    }
}

void DDSDomainParticipantListener_forward_on_offered_incompatible_qos(
        void *listener_data,
        DDS_DataWriter* c_writer,
        const DDS_OfferedIncompatibleQosStatus *status)
{
    DDSDataWriter *writer = NULL;
    DDSDomainParticipantListener *writer_listener = NULL;
    writer_listener = (DDSDomainParticipantListener*) listener_data;
    if (writer_listener != NULL)
    {
        writer = (DDSDataWriter*)
                DDS_Entity_get_wrapper(DDS_DataWriter_as_entity(c_writer));
        writer_listener->on_offered_incompatible_qos(writer, *status);
    }
}

void DDSDomainParticipantListener_forward_on_publication_matched(
        void *listener_data,
        DDS_DataWriter *c_writer,
        const DDS_PublicationMatchedStatus *status)
{
    DDSDataWriter *writer = NULL;
    DDSDomainParticipantListener *writer_listener = NULL;
    writer_listener = (DDSDomainParticipantListener*) listener_data;
    if (writer_listener != NULL)
    {
        writer = (DDSDataWriter*)
                DDS_Entity_get_wrapper(DDS_DataWriter_as_entity(c_writer));
        writer_listener->on_publication_matched(writer, *status);
    }
}

void DDSDomainParticipantListener_forward_on_reliable_reader_activity_changed(
        void *listener_data,
        DDS_DataWriter * c_writer,
        const DDS_ReliableReaderActivityChangedStatus * status)
{
    DDSDataWriter *writer = NULL;
    DDSDomainParticipantListener *writer_listener = NULL;
    writer_listener = (DDSDomainParticipantListener*) listener_data;
    if (writer_listener != NULL)
    {
        writer = (DDSDataWriter*)
                DDS_Entity_get_wrapper(DDS_DataWriter_as_entity(c_writer));
        writer_listener->on_reliable_reader_activity_changed(writer, *status);
    }
}

void DDSDomainParticipantListener_forward_on_reliable_sample_unacknowledged(
        void *listener_data,
        DDS_DataWriter * c_writer,
        const DDS_ReliableSampleUnacknowledgedStatus * status)
{
    DDSDataWriter *writer = NULL;
    DDSDomainParticipantListener *writer_listener = NULL;
    writer_listener = (DDSDomainParticipantListener*) listener_data;
    if (writer_listener != NULL)
    {
        writer = (DDSDataWriter*)
                DDS_Entity_get_wrapper(DDS_DataWriter_as_entity(c_writer));
        writer_listener->on_reliable_sample_unacknowledged(writer, *status);
    }
}

void DDSDomainParticipantListener_forward_on_inconsistent_topic(
                            void *listener_data,
                            DDS_Topic *c_topic,
                            const DDS_InconsistentTopicStatus *status)
{
    DDSTopic* topic = NULL;
    DDSDomainParticipantListener *topic_listener = NULL;

    topic_listener = (DDSDomainParticipantListener*) listener_data;
    if (topic_listener != NULL)
    {
        topic = (DDSTopic*) DDS_Entity_get_wrapper(
                                DDS_Topic_as_entity(c_topic));
        topic_listener->on_inconsistent_topic(topic, *status);
    }
}

void
DDSDomainParticipantListener_forward_on_data_on_readers(
   void* listener_data,
   DDS_Subscriber* c_subscriber)
{
    DDSSubscriber* subscriber = NULL;
    DDSDomainParticipantListener* sub_listener =
        (DDSDomainParticipantListener*) listener_data;

    if (sub_listener != NULL)
    {
        subscriber =
                (DDSSubscriber *)DDS_Entity_get_wrapper(
                        DDS_Subscriber_as_entity(c_subscriber));
        sub_listener->on_data_on_readers(subscriber);
    }
}



