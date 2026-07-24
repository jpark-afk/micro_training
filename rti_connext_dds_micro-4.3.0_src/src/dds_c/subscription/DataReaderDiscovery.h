/*
 * FILE: DataReaderDiscovery.h - DDS DataReader Discovery implementation
 *
 * Copyright 2008-2024 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 04nov2015,tk  MICRO-1505 Reduce memory footprint
 * 19mar2014,tk  MICRO-74: Support endpoint specific transport
 * 18may2012,tk  Major rewrite
 * 26aug2011,yy  Fixed returning errors
 * 30apr2008,tk  Written
 */
/*ce
 * \file
 * \brief DDS DataReader Discovery implementation
 */
/*ci
 * \addtogroup DDSSubscriptionModule
 * @{
 */
#ifndef DataReaderDiscovery_h
#define DataReaderDiscovery_h

MUST_CHECK_RETURN extern DDS_Boolean
DDS_DataReader_writer_is_compatible(DDS_DataReader *datareader,
                                    const struct DDS_DataWriterQos *dw_qos);

extern void
DDS_DataReader_match_writer(DDS_DataReader *self,
                            const struct DDS_BuiltinTopicKey_t *key,
                            const struct DDS_DataWriterQos *const qos,
                            const struct DDS_LocatorSeq *uc_locator,
                            const struct DDS_LocatorSeq *mc_locator);

extern void
DDS_DataReader_unmatch_writer(DDS_DataReader *datareader,
                              const struct DDS_BuiltinTopicKey_t *key,
                              const struct DDS_LocatorSeq *uc_locator,
                              const struct DDS_LocatorSeq *mc_locator);

extern void
DDS_DataReader_match_local_writer(DDS_DataReader *self,
                                  struct DDS_BuiltinTopicKey_t *key,
                                  const char *topic_name,
                                  const char *type_name,
                                  const DDS_DataWriter *dw);

extern void
DDS_DataReader_match_remote_writer(DDS_DataReader *self,
        const struct DDS_ParticipantBuiltinTopicData *const parent_data,
        const struct DDS_PublicationBuiltinTopicData *const data);

#ifndef RTI_CERT
extern void
DDS_DataReader_unmatch_local_writer(DDS_DataReader *self,
                                    struct DDS_BuiltinTopicKey_t *key,
                                    const DDS_DataWriter *datawriter);
#endif /* !RTI_CERT */

extern void
DDS_DataReader_unmatch_remote_writer(DDS_DataReader *self,
                const struct DDS_ParticipantBuiltinTopicData *const parent_data,
                const struct DDS_PublicationBuiltinTopicData *const data);

#endif

/*ci @} */

