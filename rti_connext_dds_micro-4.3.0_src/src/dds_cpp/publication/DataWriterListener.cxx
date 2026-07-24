/*

 (c) Copyright, Real-Time Innovations 2013-2015

 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.

 modification history
 ---------------------
22dec2015,as  MICRO-1514 Pass Status objects to callbacks by reference
16may2014,as  Moved forwarding functions from DataWriter.cxx
19jul2013,as  Major C++ update
21jan2013,eh  Created
===================================================================== */

#ifndef dds_cpp_publication_hxx
#include "dds_cpp/dds_cpp_publication.hxx"
#endif

#include "DataWriter.hxx"

/*** SOURCE_BEGIN ***/

void
DDSDataWriterListener_forward_on_offered_deadline_missed(
        void *listener_data,
        DDS_DataWriter* c_writer,
        const DDS_OfferedDeadlineMissedStatus* status)
{
    DDSDataWriter *writer = NULL;
    DDSDataWriterListener *writer_listener = NULL;

    writer_listener = (DDSDataWriterListener*) listener_data;
    if (writer_listener != NULL)
    {
        writer = (DDSDataWriter*)
            DDS_Entity_get_wrapper(DDS_DataWriter_as_entity(c_writer));
        writer_listener->on_offered_deadline_missed(writer, *status);
    }
}

void DDSDataWriterListener_forward_on_liveliness_lost(
        void *listener_data,
        DDS_DataWriter *c_writer,
        const DDS_LivelinessLostStatus *status)
{
    DDSDataWriter *writer = NULL;
    DDSDataWriterListener *writer_listener = NULL;
    writer_listener = (DDSDataWriterListener*) listener_data;
    if (writer_listener != NULL)
    {
        writer = (DDSDataWriter*)
                DDS_Entity_get_wrapper(DDS_DataWriter_as_entity(c_writer));
        writer_listener->on_liveliness_lost(writer, *status);
    }
}

void DDSDataWriterListener_forward_on_offered_incompatible_qos(
        void *listener_data,
        DDS_DataWriter* c_writer,
        const DDS_OfferedIncompatibleQosStatus *status)
{
    DDSDataWriter *writer = NULL;
    DDSDataWriterListener *writer_listener = NULL;
    writer_listener = (DDSDataWriterListener*) listener_data;
    if (writer_listener != NULL)
    {
        writer = (DDSDataWriter*)
                DDS_Entity_get_wrapper(DDS_DataWriter_as_entity(c_writer));
        writer_listener->on_offered_incompatible_qos(writer, *status);
    }
}

void DDSDataWriterListener_forward_on_publication_matched(
        void *listener_data,
        DDS_DataWriter *c_writer,
        const DDS_PublicationMatchedStatus *status)
{
    DDSDataWriter *writer = NULL;
    DDSDataWriterListener *writer_listener = NULL;
    writer_listener = (DDSDataWriterListener*) listener_data;
    if (writer_listener != NULL)
    {
        writer = (DDSDataWriter*)
                DDS_Entity_get_wrapper(DDS_DataWriter_as_entity(c_writer));
        writer_listener->on_publication_matched(writer, *status);
    }
}

void DDSDataWriterListener_forward_on_sample_removed(
        void *listener_data,
        DDS_DataWriter *c_writer,
        const struct DDS_Cookie_t* cookie)
{
    DDSDataWriter *writer = NULL;
    DDSDataWriterListener *writer_listener = NULL;
    writer_listener = (DDSDataWriterListener*) listener_data;
    if (writer_listener != NULL)
    {
        writer = (DDSDataWriter*) DDS_Entity_get_wrapper(
                DDS_DataWriter_as_entity(c_writer));
        writer_listener->on_sample_removed(writer, *cookie);
    }
}

void DDSDataWriterListener_forward_on_reliable_reader_activity_changed(
        void *listener_data,
        DDS_DataWriter * c_writer,
        const DDS_ReliableReaderActivityChangedStatus * status)
{
    DDSDataWriter *writer = NULL;
    DDSDataWriterListener *writer_listener = NULL;
    writer_listener = (DDSDataWriterListener*) listener_data;
    if (writer_listener != NULL)
    {
        writer = (DDSDataWriter*)
                DDS_Entity_get_wrapper(DDS_DataWriter_as_entity(c_writer));
        writer_listener->on_reliable_reader_activity_changed(writer, *status);
    }
}

void DDSDataWriterListener_forward_on_reliable_sample_unacknowledged(
        void *listener_data,
        DDS_DataWriter * c_writer,
        const DDS_ReliableSampleUnacknowledgedStatus * status)
{
    DDSDataWriter *writer = NULL;
    DDSDataWriterListener *writer_listener = NULL;
    writer_listener = (DDSDataWriterListener*) listener_data;
    if (writer_listener != NULL)
    {
        writer = (DDSDataWriter*)
                DDS_Entity_get_wrapper(DDS_DataWriter_as_entity(c_writer));
        writer_listener->on_reliable_sample_unacknowledged(writer, *status);
    }
}
