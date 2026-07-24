/*
 * FILE: DataReaderEvent.h - DataReader event implementations
 *
 * Copyright (c) 2008-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 11may2015,eh MICRO-1195/PR#14747 Change parameter name to is_inactive
 * 20feb2015,eh MICRO-813/PR#9172 Fix Lint warnings
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief DataReader event implementations
 */
/*ci
 * \addtogroup DDSSubscriptionModule
 * @{
 */
#ifndef DataReaderEvent_h
#define DataReaderEvent_h

extern void
DDS_DataReaderEvent_on_incompatible_qos(struct DDS_DataReaderImpl *dr,
                                        const DDS_BuiltinTopicKey_t *key);

extern void
DDS_DataReaderEvent_on_subscription_matched(struct DDS_DataReaderImpl *dr,
                                           const DDS_BuiltinTopicKey_t *key,
                                           DDS_Boolean route_existed,
                                           DDS_Boolean matched);

extern void
DDS_DataReaderEvent_on_data_available(struct DDSHST_Reader *rh,
                                      void *listener_data,
                                      DDS_InstanceHandle_t * key,
                                      DDSHST_ReaderSample_T * sample);

extern void
DDS_DataReaderEvent_on_sample_removed(struct DDSHST_Reader *rh,
                                        void *listener_data,
                                        DDS_InstanceHandle_t *key,
                                        DDSHST_ReaderSample_T *sample);

extern void
DDS_DataReaderEvent_on_hst_sample_rejected(
                        struct DDSHST_Reader *rh,
                        void *listener_data,
                        DDS_InstanceHandle_t *key,
                        DDS_SampleRejectedStatusKind reason);

extern void
DDS_DataReaderEvent_on_hst_sample_lost(struct DDSHST_Reader *rh,
                                       void *listener_data,
                                       struct DDS_SampleInfo *sample_info,
                                       DDS_SampleLostStatusKind reason);
extern void
DDS_DataReaderEvent_on_sample_lost(struct DDS_DataReaderImpl *dr,
                                   struct NETIO_Address *source,
                                   struct REDA_SequenceNumber *first_sn,
                                   RTI_INT32 count);

extern void
DDS_DataReaderEvent_on_liveliness_changed(struct DDS_DataReaderImpl *datareader,
                                          DDS_InstanceHandle_t *instance);

extern void
DDS_DataReaderEvent_on_liveliness_detected(struct DDS_DataReaderImpl *datareader,
                                           struct NETIO_Guid *peer);

extern void
DDS_DataReaderEvent_on_liveliness_lost(struct DDS_DataReaderImpl *datareader,
                                       struct NETIO_Guid *peer);

extern void
DDS_DataReaderEvent_on_deadline_missed(struct DDSHST_Reader *rh,
                                        void *listener_data,
                                        DDS_InstanceHandle_t *key);

extern OSAPI_TimeoutOp_t
DDS_DataReaderEvent_on_deadline_timeout(struct OSAPI_TimeoutUserData *storage);

extern void
DDS_DataReaderEvent_on_instance_replaced(struct DDSHST_Reader *rh,
                                         void *listener_data,
                                         DDS_InstanceHandle_t *replaced_key,
                                         DDS_InstanceHandle_t *replaced_by_key,
                                         DDS_InstanceHandle_t *publisher,
                                         DDS_Long min_removed_samples);

extern void
DDS_DataReaderEvent_on_sample_committed(struct DDSHST_Reader *rh,
                                        void *listener_data,
                                        DDS_InstanceHandle_t *publisher,
                                        DDS_Long committed_samples);

extern void
DDS_DataReaderEvent_on_liveliness_lost(struct DDS_DataReaderImpl *datareader,
                                       struct NETIO_Guid *peer);

extern void
DDS_DataReaderEvent_on_remote_writer_deleted(struct DDS_DataReaderImpl *datareader,
                                             struct NETIO_Guid *peer,
                                             RTI_BOOL is_active);

extern void
DDS_DataReaderEvent_on_key_removed(struct DDSHST_Reader *rh,
                                   void *listener_data,
                                   DDS_InstanceHandle_t *key);

#endif

/*ci @} */

