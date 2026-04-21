/*
 * FILE: RemoteParticipant.h - Remote participant implementation
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
 * 31jul2014,tk MICRO-842/PR#9683 - Removed superfluous parameters in DB
 *                                  compare function
 * 17may2012,tk Updated
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief Remote participant implementation
 */
/*ci
 * \addtogroup DDSDomainModule
 * @{
 */
#ifndef RemoteParticipant_h
#define RemoteParticipant_h

/*ci
 * \brief Implementation of a remote participant
 */
struct DDS_RemoteParticipantImpl
{
    /*ci
     * \brief Inherited from the base-class
     */
    struct NDDS_RemoteEntityImpl as_entity;

    /*ci
     * \brief The discovery data for for the participant
     */
    struct DDS_ParticipantBuiltinTopicData data;

    /*ci
     * \brief The lease-duration event used to detect if a participant maintain
     *        its lease
     */
    OSAPI_TimeoutHandle_T lease_duration_event;

    /*ci
     * \brief The timer to create lease-duration events from
     */
    OSAPI_Timer_T timer;

    /*ci
     * \brief The original key of a participant, used for static discovery
     *        when the key is not yet known
     */
    DDS_BuiltinTopicKey_t orig_key;

    /*ci
     * \brief Flag to indicate if a remote participant was asserted statically
     *        or not. If this flag is false the participant name cannot be
     *        trusted.
     */
    DDS_Boolean is_static;
};

MUST_CHECK_RETURN extern RTI_INT32
DDS_RemoteParticipantImpl_compare(RTI_INT32 flags,
                                  const DB_Record_T op1, void *op2);

MUST_CHECK_RETURN struct DDS_RemoteParticipantImpl*
NDDS_DomainParticipant_lookup_name(DDS_DomainParticipant *const participant,
                                   const char *name);

#ifndef RTI_CERT
SHOULD_CHECK_RETURN extern RTI_BOOL
NDDS_RemoteParticipantRecord_finalize(const void *record, void *param);
#endif

#endif

/*ci @} */


