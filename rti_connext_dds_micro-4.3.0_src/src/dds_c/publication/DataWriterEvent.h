/*
 * FILE: DataWriterEvent.h - DataWriter events functionality
 *
 * (c) Copyright 2008-2015 Real-Time Innovations,
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
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief DataWriter events functionality
 */
/*ci
 * \addtogroup DDSPublicationModule
 * @{
 */
#ifndef DataWriterEvent_h
#define DataWriterEvent_h

extern void
DDS_DataWriterEvent_on_incompatible_qos(struct DDS_DataWriterImpl *dw,
                                        const DDS_BuiltinTopicKey_t *key);

extern void
DDS_DataWriterEvent_on_publication_matched(struct DDS_DataWriterImpl *dw,
                                           const DDS_BuiltinTopicKey_t *key,
                                           DDS_Boolean route_existed,
                                           DDS_Boolean matched);

extern OSAPI_TimeoutOp_t
DDS_DataWriterEvent_on_liveliness(struct OSAPI_TimeoutUserData *storage);

extern void
DDS_DataWriterEvent_on_sample_removed(void *config,
                                      DDS_InstanceHandle_t *key,
                                      struct DDSHST_WriterSample *sample,
                                      struct REDA_SequenceNumber *sn,
                                      DDSHST_WriterSampleRemovedKind_T kind,
                                      DDS_Long ack_count);

#ifndef RTI_CERT
extern void
DDS_DataWriterEvent_on_key_removed(void *config, DDS_InstanceHandle_t * key,
                                   DDSHST_WriterKeyRemovedKind_T kind);
#endif

extern void
DDS_DataWriterEvent_on_deadline_missed(void *listener_data,
                                        DDS_InstanceHandle_t *key);

extern OSAPI_TimeoutOp_t
DDS_DataWriterEvent_on_deadline_expired(struct OSAPI_TimeoutUserData *storage);

extern void
DDS_DataWriterEvent_on_reliable_reader_activity_changed(
                                                struct DDS_DataWriterImpl *dw);

#endif

/*ci @} */


