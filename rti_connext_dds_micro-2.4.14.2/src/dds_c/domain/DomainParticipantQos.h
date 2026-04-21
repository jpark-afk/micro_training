/*
 * FILE: DomainParticipantQos.h - DomainParticipantQos implementation
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

MUST_CHECK_RETURN extern DDS_Boolean
DDS_DomainParticipantQos_immutable_is_equal(
                                const struct DDS_DomainParticipantQos *left,
                                const struct DDS_DomainParticipantQos *right);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_DomainParticipantQos_is_consistent(
                                const struct DDS_DomainParticipantQos *self);

#endif

/*ci @} */
