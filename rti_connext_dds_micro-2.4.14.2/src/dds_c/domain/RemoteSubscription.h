/*
 * FILE: RemoteSubscription.h - Remote subscription implementation
 *
 * (c) Copyright, Real-Time Innovations, 2008-2015.
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
 * 20feb2015,eh MICRO-813/PR#9172 Fix Lint warnings
 * 31jul2014,tk MICRO-842/PR#9683 - Removed superfluous paramaters in DB
 *                                  compare function
 * 17may2012,tk Updated
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief Remote subscription implementation
 */
/*ci \addtogroup DDSDomainModule
 * @{
 */
#ifndef RemoteSubscription_h
#define RemoteSubscription_h

/*ci
 * \brief Implementation of a remote subscription
 */
struct DDS_RemoteSubscriptionImpl
{
    /*ci
     * \brief base-class
     */
    struct NDDS_RemoteEntityImpl as_entity;

    /*ci
     * \brief  Current discovered subscription data for the remote subscription.
     *         The first field in the builtin topic data is the GUID
     *         which is the key.
     */
    struct DDS_SubscriptionBuiltinTopicData data;

    /*ci
     * \brief The key the remote subscription was asserted with, used in case
     *        of reset
     */
    DDS_BuiltinTopicKey_t orig_key;
};

MUST_CHECK_RETURN extern RTI_INT32
DDS_RemoteSubscriptionImpl_compare(RTI_INT32 flags,
                                   const DB_Record_T op1, void *op2);

SHOULD_CHECK_RETURN extern DDS_ReturnCode_t
NDDS_RemoteSubscription_remove_internal(DDS_DomainParticipant *const self,
                                        const DDS_BuiltinTopicKey_t *key,
                                        DDS_Boolean reset_entry);

extern void
NDDS_RemoteSubscription_match_with_local_writer(DDS_DomainParticipant *const self,
                                                struct DDS_DataWriterImpl *local_writer);

#ifndef RTI_CERT
extern void
NDDS_RemoteSubscription_unmatch_local_writer_from_key(
                                    DDS_DomainParticipant *const participant,
                                    DDS_BuiltinTopicKey_t *dr_key,
                                    struct DDS_DataWriterImpl *local_writer);
#endif /* !RTI_CERT */

#endif

/*ci @} */
