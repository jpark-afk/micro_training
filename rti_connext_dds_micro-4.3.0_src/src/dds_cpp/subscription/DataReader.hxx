/*
 * FILE: DataReader.hxx - DataReader header
 *
 * (c) Copyright 2013-2015 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 19jul2013,as  Major C++ update
 * 28jun2013,eh  Written
 */

#ifndef DataReader_hxx
#define DataReader_hxx


#ifndef dds_cpp_subscription_hxx
  #include "dds_cpp/dds_cpp_subscription.hxx"
#endif

#ifndef dds_c_subscription_h
  #include "dds_c/dds_c_subscription.h"
#endif


extern "C" {

    void
    DDSDataReaderListener_forward_on_data_available(
       void* listener_data,
       DDS_DataReader* reader);

    void
    DDSDataReaderListener_forward_on_requested_deadline_missed(
            void *listener_data,
            DDS_DataReader* c_reader,
            const struct DDS_RequestedDeadlineMissedStatus *status);

    void
    DDSDataReaderListener_forward_on_liveliness_changed(
            void *listener_data,
            DDS_DataReader *c_reader,
            const struct DDS_LivelinessChangedStatus *status);

    void
    DDSDataReaderListener_forward_on_requested_incompatible_qos(
            void *listener_data,
            DDS_DataReader *c_reader,
            const struct DDS_RequestedIncompatibleQosStatus *status);

    void
    DDSDataReaderListener_forward_on_sample_rejected(
            void *listener_data,
            DDS_DataReader *c_reader,
            const struct DDS_SampleRejectedStatus *status);

    void
    DDSDataReaderListener_forward_on_subscription_matched(
            void *listener_data,
            DDS_DataReader *c_reader,
            const struct DDS_SubscriptionMatchedStatus *status);

    void
    DDSDataReaderListener_forward_on_sample_lost(
            void *listener_data,
            DDS_DataReader *c_reader,
            const struct DDS_SampleLostStatus *status);

    void
    DDSDataReaderListener_forward_on_instance_replaced(
        void *listener_data,
        DDS_DataReader *c_reader,
        const struct DDS_DataReaderInstanceReplacedStatus *status);

    DDS_Boolean
    DDSDataReaderListener_forward_on_before_sample_deserialize(
            void *listener_data,
            DDS_DataReader *c_reader,
            struct NDDS_Type_Plugin *plugin,
            struct CDR_Stream_t *stream,
            DDS_Boolean *dropped);

    DDS_Boolean
    DDSDataReaderListener_forward_on_before_sample_commit(
            void *listener_data,
            DDS_DataReader *c_reader,
            const void *const sample,
            const struct DDS_SampleInfo *const sample_info,
            DDS_Boolean *dropped);
}



#define DDSDataReaderListener_INITIALIZE_C_LISTENER(listener_, c_listener_)\
{\
        (c_listener_)->as_listener.listener_data = (void*) (listener_);\
        (c_listener_)->on_data_available =\
                DDSDataReaderListener_forward_on_data_available;\
        (c_listener_)->on_requested_deadline_missed =\
                DDSDataReaderListener_forward_on_requested_deadline_missed;\
        (c_listener_)->on_liveliness_changed =\
                DDSDataReaderListener_forward_on_liveliness_changed;\
        (c_listener_)->on_requested_incompatible_qos =\
                DDSDataReaderListener_forward_on_requested_incompatible_qos;\
        (c_listener_)->on_sample_rejected =\
                DDSDataReaderListener_forward_on_sample_rejected;\
        (c_listener_)->on_subscription_matched =\
                DDSDataReaderListener_forward_on_subscription_matched;\
        (c_listener_)->on_sample_lost =\
                DDSDataReaderListener_forward_on_sample_lost;\
        (c_listener_)->on_instance_replaced =\
                DDSDataReaderListener_forward_on_instance_replaced;\
        (c_listener_)->on_before_sample_deserialize =\
                DDSDataReaderListener_forward_on_before_sample_deserialize;\
        (c_listener_)->on_before_sample_commit =\
                DDSDataReaderListener_forward_on_before_sample_commit;\
}


#endif /* DataReader_hxx */
