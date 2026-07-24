/* 

 (c) Copyright, Real-Time Innovations, 2013-2015.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.

 modification history
--------------------
22dec2015,as  MICRO-1514 Pass Status objects to callbacks by reference
16may2014,as  Moved forwarding functions from DataReader.cxx
19jul2013,as  Major C++ update
16jan2013,eh  Created
===================================================================== */
#ifndef dds_cpp_subscription_hxx
  #include "dds_cpp/dds_cpp_subscription.hxx"
#endif

#include "DataReader.hxx"
/*** SOURCE_BEGIN ***/

void
DDSDataReaderListener_forward_on_data_available(
   void* listener_data,
   DDS_DataReader* c_datareader)
{
    DDSDataReaderListener* reader_listener =
        (DDSDataReaderListener*) listener_data;

    if (reader_listener != NULL)
    {
        DDSDataReader* datareader =
                (DDSDataReader *)DDS_Entity_get_wrapper(
                        DDS_DataReader_as_entity(c_datareader));
        reader_listener->on_data_available(datareader);
    }
}

void
DDSDataReaderListener_forward_on_requested_deadline_missed(
        void *listener_data,
        DDS_DataReader* c_reader,
        const DDS_RequestedDeadlineMissedStatus *status)
{
    DDSDataReaderListener* reader_listener =
            (DDSDataReaderListener*) listener_data;


    if (reader_listener != NULL)
    {
        DDSDataReader* datareader =
            (DDSDataReader *)DDS_Entity_get_wrapper(
                    DDS_DataReader_as_entity(c_reader));
        reader_listener->on_requested_deadline_missed(datareader, *status);
    }
}


void
DDSDataReaderListener_forward_on_liveliness_changed(
        void *listener_data,
        DDS_DataReader *c_reader,
        const DDS_LivelinessChangedStatus *status)
{
    DDSDataReaderListener* reader_listener =
                (DDSDataReaderListener*) listener_data;


    if (reader_listener != NULL)
    {
        DDSDataReader* datareader =
                (DDSDataReader *)DDS_Entity_get_wrapper(
                        DDS_DataReader_as_entity(c_reader));
        reader_listener->on_liveliness_changed(datareader, *status);
    }
}

void
DDSDataReaderListener_forward_on_requested_incompatible_qos(
        void *listener_data,
        DDS_DataReader *c_reader,
        const DDS_RequestedIncompatibleQosStatus *status)
{
    DDSDataReaderListener* reader_listener =
                    (DDSDataReaderListener*) listener_data;

    DDSDataReader* datareader =
                    (DDSDataReader *)DDS_Entity_get_wrapper(
                            DDS_DataReader_as_entity(c_reader));

    if (reader_listener != NULL)
    {
        reader_listener->on_requested_incompatible_qos(datareader, *status);
    }
}

void
DDSDataReaderListener_forward_on_sample_rejected(
        void *listener_data,
        DDS_DataReader *c_reader,
        const DDS_SampleRejectedStatus *status)
{
    DDSDataReaderListener* reader_listener =
                    (DDSDataReaderListener*) listener_data;

    if (reader_listener != NULL)
    {
        DDSDataReader* datareader =
                    (DDSDataReader *)DDS_Entity_get_wrapper(
                            DDS_DataReader_as_entity(c_reader));
        reader_listener->on_sample_rejected(datareader, *status);
    }
}

void
DDSDataReaderListener_forward_on_subscription_matched(
        void *listener_data,
        DDS_DataReader *c_reader,
        const DDS_SubscriptionMatchedStatus *status)
{
    DDSDataReaderListener* reader_listener =
                    (DDSDataReaderListener*) listener_data;

    if (reader_listener != NULL)
    {
        DDSDataReader* datareader =
                    (DDSDataReader *)DDS_Entity_get_wrapper(
                            DDS_DataReader_as_entity(c_reader));

        reader_listener->on_subscription_matched(datareader, *status);
    }
}

void
DDSDataReaderListener_forward_on_sample_lost(
        void *listener_data,
        DDS_DataReader *c_reader,
        const DDS_SampleLostStatus *status)
{
    DDSDataReaderListener* reader_listener =
                    (DDSDataReaderListener*) listener_data;

    if (reader_listener != NULL)
    {
        DDSDataReader* datareader =
                    (DDSDataReader *)DDS_Entity_get_wrapper(
                            DDS_DataReader_as_entity(c_reader));

        reader_listener->on_sample_lost(datareader, *status);
    }
}

void
DDSDataReaderListener_forward_on_instance_replaced(
        void *listener_data,
        DDS_DataReader *c_reader,
        const DDS_DataReaderInstanceReplacedStatus *status)
{
    DDSDataReaderListener* reader_listener =
                    (DDSDataReaderListener*) listener_data;

    if (reader_listener != NULL)
    {
        DDSDataReader* datareader =
                    (DDSDataReader *)DDS_Entity_get_wrapper(
                            DDS_DataReader_as_entity(c_reader));

        reader_listener->on_instance_replaced(datareader, *status);
    }
}

DDS_Boolean
DDSDataReaderListener_forward_on_before_sample_deserialize(
        void *listener_data,
        DDS_DataReader *c_reader,
        NDDS_Type_Plugin *plugin,
        CDR_Stream_t *stream,
        DDS_Boolean *dropped)
{
    DDSDataReaderListener* reader_listener =
                    (DDSDataReaderListener*) listener_data;

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
DDSDataReaderListener_forward_on_before_sample_commit(
        void *listener_data,
        DDS_DataReader *c_reader,
        const void *const sample,
        const DDS_SampleInfo *const sample_info,
        DDS_Boolean *dropped)
{
    DDSDataReaderListener* reader_listener =
                    (DDSDataReaderListener*) listener_data;

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


