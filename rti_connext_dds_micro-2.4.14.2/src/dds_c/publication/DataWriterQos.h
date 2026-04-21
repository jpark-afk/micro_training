/*
 * FILE: DataWriterQos.h - DataWriter QoS implementation
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
 * 30jun2015,tk MICRO-1378/PR#15203 Updated comments
 * 20sep2014,as Exposed is_equal within module dds_c
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \ingroup DDSDomainModule
 * \brief DataWriter QoS implementation
 */
/*ci \addtogroup DDSPublicationModule
 * @{
 */
#ifndef DataWriterQos_h
#define DataWriterQos_h

MUST_CHECK_RETURN extern DDS_Boolean
DDS_DataWriterQos_immutable_is_equal(const struct DDS_DataWriterQos *left,
                                     const struct DDS_DataWriterQos *right);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_DataWriterQos_is_consistent(const struct DDS_DataWriterQos *self);

#endif

/*ci @} */

