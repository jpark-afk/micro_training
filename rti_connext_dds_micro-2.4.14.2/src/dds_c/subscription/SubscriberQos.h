/*
 * FILE: SubscriberQos.h - Subscriber Qos implementation
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
 * 20sep2014,as Exposed DDS_SubscriberQos_is_equal within module dds_c
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief Subscriber Qos implementation
 */
/*ci \addtogroup DDSSubscriptionModule
 * @{
 */
#ifndef SubscriberQos_h
#define SubscriberQos_h

MUST_CHECK_RETURN extern DDS_Boolean
DDS_SubscriberQos_is_consistent(const struct DDS_SubscriberQos *self);

#endif

/*ci @} */

