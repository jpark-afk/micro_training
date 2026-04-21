/*
 * FILE: RemotePublication.h - Remote publication definitions
 *
 * (c) Copyright, Real-Time Innovations, 2008-2021.
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
 * 13dec2021,tk MICRO-3226/PR.29479
 * - Fixed filename in header.
 * - Replaced with RemotePublication_pkg_h RemotePublication_h
 * 20feb2015,eh MICRO-813/PR#9172 Fix Lint warnings
 * 31jul2014,tk MICRO-842/PR#9683 - Removed superfluous paramaters in DB
 *                                  compare function
 * 17may2012,tk Updated
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief Remote publication implementation
 */
/*ci
 * \addtogroup DDSDomainModule
 * @{
 */
#ifndef RemotePublication_h
#define RemotePublication_h

/*ci
 * \brief Implementation of a remote publication
 */
struct DDS_RemotePublicationImpl
{
    /*ci
     * \brief base-class
     */
    struct NDDS_RemoteEntityImpl as_entity;

    /*ci
     * \brief Current discovered publication data for the remote publication.
     *        The first field in the builtin topic data is the GUID
     *        which is the key.
     */
    struct DDS_PublicationBuiltinTopicData data;

    /*ci
     * \brief The key the remote publication was asserted with, used in case
     *        of reset
     */
    DDS_BuiltinTopicKey_t orig_key;
};

MUST_CHECK_RETURN extern RTI_INT32
DDS_RemotePublicationImpl_compare(RTI_INT32 flags,const DB_Record_T op1, void *op2);

SHOULD_CHECK_RETURN extern DDS_ReturnCode_t
NDDS_RemotePublication_remove_internal(DDS_DomainParticipant *const self,
                                       const DDS_BuiltinTopicKey_t *const key,
                                       DDS_Boolean reset_entry);

extern void
NDDS_RemotePublication_match_with_local_reader(DDS_DomainParticipant *const self,
                                               DDS_DataReader *local_reader);

#ifndef RTI_CERT
extern void
NDDS_RemotePublication_unmatch_local_reader_from_key(
                                     DDS_DomainParticipant *const participant,
                                     DDS_BuiltinTopicKey_t *dw_key,
                                     struct DDS_DataReaderImpl *local_reader);
#endif /* !RTI_CERT */

#endif /* RemotePublication_pkg_h */

/*ci @} */
