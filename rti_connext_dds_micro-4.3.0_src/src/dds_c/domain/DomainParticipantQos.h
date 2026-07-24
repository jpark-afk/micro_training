/*
 * FILE: DomainParticipantQos.h - DomainParticipantQos implementation
 *
 * (c) Copyright, Real-Time Innovations, 2008-2024.
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
 * 27jun2012,tk  Major update
 * 30apr2008,tk  Written
 */
/*ce
 * \file
 * \brief  DomainParticipantQos implementation
 */
/*ci \addtogroup DDSDomainModule
 * @{
 */
#ifndef DomainParticipantQos_h
#define DomainParticipantQos_h

/*i
 * \brief Deep copy only the fields that are not managed by a manager
 *
 * \long
 * This function is used to copy the fields of a DomainParticipantQos that are
 * not managed by a manager. This function is necessary for initializing a
 * DomainParticipant because these fields must be copied before the
 * DomainParticipant is fully initialized. After it is initialized, the remaining
 * fields can be copied with \dref_DomainParticipantQos_copy_managed_fields.
 *
 * \param out The destination DomainParticipantQos
 * \param in The source DomainParticipantQos
 */
extern DDS_Boolean
DDS_DomainParticipantQos_copy_unmanaged_fields(
        struct DDS_DomainParticipantQos *out,
        const struct DDS_DomainParticipantQos *in);

/*i
 * \brief Shallow copy only the fields that are managed by a manager
 *
 * \long
 * This function is used to shallow copy the fields of a DomainParticipantQos that
 * are managed by a manager. This function must be passed a fully initialized
 * DomainParticipant. The fields that are unmanaged can be copied without an
 * initialized DomainParticipant \dref_DomainParticipantQos_copy_unmanaged_fields.
 *
 * \param out The destination DomainParticipantQos
 * \param in The source DomainParticipantQos
 * \param participant The DomainParticipant that will managed the fields
 */
extern DDS_Boolean
DDS_DomainParticipantQos_copy_managed_fields(
        struct DDS_DomainParticipantQos *out,
        const struct DDS_DomainParticipantQos *in,
        DDS_DomainParticipant *participant);

#ifndef RTI_CERT
extern DDS_ReturnCode_t
DDS_DomainParticipantQos_finalize_managed(
        struct DDS_DomainParticipantQos *self,
        DDS_DomainParticipant *participant);

#endif


#if INCLUDE_API_QOS
MUST_CHECK_RETURN extern DDS_Boolean
DDS_DomainParticipantQos_immutable_is_equal(
                                const struct DDS_DomainParticipantQos *left,
                                const struct DDS_DomainParticipantQos *right);
#endif

MUST_CHECK_RETURN extern DDS_Boolean
DDS_DomainParticipantQos_is_consistent(
                                const struct DDS_DomainParticipantQos *self);

#endif

/*ci @} */
