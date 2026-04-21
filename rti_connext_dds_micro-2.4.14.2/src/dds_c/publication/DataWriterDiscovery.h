/*
 * FILE: DataWriterDiscovery.h - DataWriter discovery implementation
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
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 * 30apr2008,tk  Written
 */
/*ce
 * \file
 * \brief DataWriter discovery implementation
 */
/*ci
 * \addtogroup DDSPublicationModule
 * @{
 */
#ifndef DataWriterDiscovery_h
#define DataWriterDiscovery_h

MUST_CHECK_RETURN extern DDS_Boolean
DDS_DataWriter_reader_is_compatible(DDS_DataWriter *datawriter,
                                    const struct DDS_DataReaderQos *dr_qos);

extern void
DDS_DataWriter_match_reader(DDS_DataWriter *datawriter,
                            const struct DDS_BuiltinTopicKey_t *key,
                            const struct DDS_DataReaderQos *const qos,
                            const struct DDS_LocatorSeq *uc_locator,
                            const struct DDS_LocatorSeq *mc_locator);

extern void
DDS_DataWriter_unmatch_reader(DDS_DataWriter *datawriter,
                            const struct DDS_BuiltinTopicKey_t *key,
                            const struct DDS_LocatorSeq *uc_locator,
                            const struct DDS_LocatorSeq *mc_locator);

extern void
DDS_DataWriter_match_local_reader(DDS_DataWriter *datawriter,
                                  struct DDS_BuiltinTopicKey_t *key,
                                  const char *topic_name,
                                  const char *type_name,
                                  const struct DDS_DataReaderQos *const qos);

extern void
DDS_DataWriter_match_remote_reader(DDS_DataWriter *self,
        const struct DDS_ParticipantBuiltinTopicData *const parent_data,
        const struct DDS_SubscriptionBuiltinTopicData *const data);

#ifndef RTI_CERT
extern void
DDS_DataWriter_unmatch_local_reader(DDS_DataWriter *self,
                                    struct DDS_BuiltinTopicKey_t *key,
                                    struct DDS_DataReaderQos *const qos);
#endif /* !RTI_CERT */

extern void
DDS_DataWriter_unmatch_remote_reader(DDS_DataWriter *self,
        const struct DDS_ParticipantBuiltinTopicData *const parent_data,
        const struct DDS_SubscriptionBuiltinTopicData *const data);

#endif

/*ci @} */

