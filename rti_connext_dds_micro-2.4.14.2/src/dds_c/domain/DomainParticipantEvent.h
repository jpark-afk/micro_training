/*
 * FILE: DomainParticipantEvent.h - DomainParticipant event implementation
 *
 * (c) Copyright, Real-Time Innovations, 2008-2020.
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
 * 08nov2013,as  MICRO-681 Complete implementation of WaitSets
 *               and support for StatusConditions
 * 27jun2012,tk  Major update
 * 30apr2008,tk  Written
 */
/*ce
 * \file
 * \brief DomainParticipant event implementation
 */
/*ci \addtogroup DDSDomainModule
 * @{
 */
#ifndef DomainParticipantEvent_h
#define DomainParticipantEvent_h

extern DDS_ReturnCode_t
DomainParticipantEvent_on_before_datareader_created(
                                DDS_Subscriber *const subscriber,
                                struct DDS_BuiltinTopicKey_t *const dr_key,
                                DDS_Boolean reservation);


extern RTI_BOOL
DomainParticipantEvent_on_after_datareader_enabled(
                        DDS_DataReader *const reader,
                        const struct DDS_DataReaderQos *const qos);

#ifndef RTI_CERT
extern void
DomainParticipantEvent_on_before_datareader_deleted(DDS_DataReader *const reader);
#endif /* !RTI_CERT */

extern DDS_ReturnCode_t
DomainParticipantEvent_on_before_datawriter_created(
                                DDS_Publisher *const publisher,
                                struct DDS_BuiltinTopicKey_t *const dw_key,
                                DDS_Boolean reservation);

extern RTI_BOOL
DomainParticipantEvent_on_after_datawriter_enabled(
                        DDS_DataWriter *const writer,
                        const struct DDS_DataWriterQos *const qos);

#ifndef RTI_CERT
extern void
DomainParticipantEvent_on_before_datawriter_deleted(DDS_DataWriter *const writer);
#endif /* !RTI_CERT */

extern DDS_Boolean
NDDS_DomainParticipant_on_offered_incompatible_qos(DDS_DomainParticipant * self,
                        DDS_DataWriter * writer,
                        const struct DDS_OfferedIncompatibleQosStatus *status);

extern DDS_Boolean
NDDS_DomainParticipant_on_publication_matched(DDS_DomainParticipant * self,
                        DDS_DataWriter * writer,
                        const struct DDS_PublicationMatchedStatus *status);

extern DDS_Boolean
NDDS_DomainParticipant_on_reliable_reader_activity_changed(
   DDS_DomainParticipant *self,
   DDS_DataWriter *writer,
   const struct DDS_ReliableReaderActivityChangedStatus *status);

extern DDS_Boolean
NDDS_DomainParticipant_on_requested_incompatible_qos(
                        DDS_DomainParticipant *self,
                        DDS_DataReader * reader,
                        const struct DDS_RequestedIncompatibleQosStatus *status);

extern DDS_Boolean
NDDS_DomainParticipant_on_subscription_matched(DDS_DomainParticipant *self,
                        DDS_DataReader *reader,
                        const struct DDS_SubscriptionMatchedStatus *status);

extern DDS_Boolean
NDDS_DomainParticipant_on_data_available(DDS_DomainParticipant *self,
                                         DDS_Subscriber *subscriber,
                                         DDS_DataReader *reader);

extern DDS_Boolean
NDDS_DomainParticipant_on_data_on_readers(DDS_DomainParticipant *self,
                                         DDS_Subscriber *subscriber);

extern DDS_Boolean
NDDS_DomainParticipant_on_requested_deadline_missed(DDS_DomainParticipant *dp,
                        DDS_DataReader *reader,
                        const struct DDS_RequestedDeadlineMissedStatus *status);

extern DDS_Boolean
NDDS_DomainParticipant_on_liveliness_changed(DDS_DomainParticipant *dp,
                            DDS_DataReader * reader,
                            const struct DDS_LivelinessChangedStatus *status);

extern DDS_Boolean
NDDS_DomainParticipant_on_sample_lost(DDS_DomainParticipant * dp,
                           DDS_DataReader *reader,
                           const struct DDS_SampleLostStatus *status);

extern DDS_Boolean
NDDS_DomainParticipant_on_sample_rejected(DDS_DomainParticipant *self,
                            DDS_DataReader *reader,
                            const struct DDS_SampleRejectedStatus *status);

extern DDS_Boolean
NDDS_DomainParticipant_on_inconsistent_topic(DDS_DomainParticipant * self,
                              DDS_Topic * topic,
                              const struct DDS_InconsistentTopicStatus *status);

extern DDS_Boolean
NDDS_DomainParticipant_on_liveliness_lost(DDS_DomainParticipant * self,
                        DDS_DataWriter *writer,
                        const struct DDS_LivelinessLostStatus *status);

extern DDS_Boolean
NDDS_DomainParticipant_on_offered_deadline_missed(DDS_DomainParticipant * self,
                        DDS_DataWriter * writer,
                        const struct DDS_OfferedDeadlineMissedStatus *status);

extern DDS_Boolean
NDDS_DomainParticipantListener_on_instance_replaced(
        DDS_DomainParticipant *self,
        DDS_DataReader *reader,
        const struct DDS_DataReaderInstanceReplacedStatus *status);

#endif

/*ci @} */

