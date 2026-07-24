
/* DataWriter.hxx

 (c) Copyright, Real-Time Innovations, 2013-2015.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.

 modification history
---------------------
19jul2013,as  Created.
===================================================================== */

#ifndef DataWriter_hxx
#define DataWriter_hxx

#ifndef dds_cpp_publication_hxx
  #include "dds_cpp/dds_cpp_publication.hxx"
#endif
#ifndef dds_c_publication_h
  #include "dds_c/dds_c_publication.h"
#endif

extern "C" {
    void DDSDataWriterListener_forward_on_offered_deadline_missed(
        void *listener_data,
        DDS_DataWriter* writer,
        const struct DDS_OfferedDeadlineMissedStatus* status);

    void DDSDataWriterListener_forward_on_liveliness_lost(
        void *listener_data,
        DDS_DataWriter *writer,
        const struct DDS_LivelinessLostStatus *status);

    void DDSDataWriterListener_forward_on_offered_incompatible_qos(
        void *listener_data,
        DDS_DataWriter* writer,
        const struct DDS_OfferedIncompatibleQosStatus *status);

    void DDSDataWriterListener_forward_on_publication_matched(
        void *listener_data,
        DDS_DataWriter *writer,
        const struct DDS_PublicationMatchedStatus *status);

    void DDSDataWriterListener_forward_on_reliable_reader_activity_changed(
        void *listener_data,
        DDS_DataWriter * writer,
        const struct DDS_ReliableReaderActivityChangedStatus * status);

    void DDSDataWriterListener_forward_on_reliable_sample_unacknowledged(
            void *listener_data,
            DDS_DataWriter * writer,
            const struct DDS_ReliableSampleUnacknowledgedStatus * status);

    void DDSDataWriterListener_forward_on_sample_removed(
            void *listener_data,
            DDS_DataWriter *writer,
            const struct DDS_Cookie_t* cookie);
}

#define DDSDataWriterListener_INITIALIZE_C_LISTENER(listener_, c_listener_)\
{\
    (c_listener_)->as_listener.listener_data = (void*) (listener_);\
    (c_listener_)->on_liveliness_lost =\
                DDSDataWriterListener_forward_on_liveliness_lost;\
    (c_listener_)->on_offered_deadline_missed =\
                DDSDataWriterListener_forward_on_offered_deadline_missed;\
    (c_listener_)->on_offered_incompatible_qos =\
                DDSDataWriterListener_forward_on_offered_incompatible_qos;\
    (c_listener_)->on_publication_matched =\
                DDSDataWriterListener_forward_on_publication_matched;\
    (c_listener_)->on_reliable_reader_activity_changed =\
                DDSDataWriterListener_forward_on_reliable_reader_activity_changed;\
    (c_listener_)->on_reliable_sample_unacknowledged =\
                DDSDataWriterListener_forward_on_reliable_sample_unacknowledged;\
    (c_listener_)->on_sample_removed =\
                DDSDataWriterListener_forward_on_sample_removed;\
}


#endif /* DATAWRITER_HXX_ */
