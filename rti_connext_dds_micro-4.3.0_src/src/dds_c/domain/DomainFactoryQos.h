/*
 * FILE: DomainFactoryQos.c - DomainFactoryQos implementation
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
 * 30apr2008,tk Created
 */
/*ci
 * \file
 */
/*ci \addtogroup DDSDomainModule
 * @{
 */
#ifndef DomainFactoryQos_h
#define DomainFactoryQos_h

/*ci
 * \brief Test if the immutable part of the DDS_DomainParticipantFactoryQos
 *        is the equal
 *
 * \param[in] left The left side of the comparison
 * \param[in] right The right side of the comparison
 *
 * \return DDS_BOOLEAN_TRUE if the immutable parts are equal,
 *         DDS_BOOLEAN_FALSE if the immutable parts are not equal
 *
 * \sa \ref DDS_DomainParticipantFactoryQos_is_equal
 */
MUST_CHECK_RETURN extern DDS_Boolean
DDS_DomainParticipantFactoryQos_immutable_is_equal(
                     const struct DDS_DomainParticipantFactoryQos *left,
                     const struct DDS_DomainParticipantFactoryQos *right);

/*ci
 * \brief Test if DDS_DomainParticipantFactoryQos has consistent, legal values
 *
 * \param[in] self Qos policy to test
 *
 * \return DDS_BOOLEAN_TRUE if the Qos policy has valid consistent values,
 *         DDS_BOOLEAN_FALSE if the Qos is not valid
 */
MUST_CHECK_RETURN extern DDS_Boolean
DDS_DomainParticipantFactoryQos_is_consistent(
                          const struct DDS_DomainParticipantFactoryQos *self);

#endif

/*ci @} */

