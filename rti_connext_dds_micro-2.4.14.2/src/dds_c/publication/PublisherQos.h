/*
 * FILE: PublisherQos.h - Publisher Qos implementation
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
 * 20sep2014,as Made DDS_PublisherQos_finalize public
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief Publisher Qos implementation
 * \ingroup DDSDomainModule
 */
/*ci \addtogroup DDSPublicationModule
 * @{
 */
#ifndef PublisherQos_h
#define PublisherQos_h

MUST_CHECK_RETURN extern DDS_Boolean
DDS_PublisherQos_is_consistent(const struct DDS_PublisherQos *self);

#endif

/*ci @} */

