/*
 * FILE: SubscriberQos.h - Subscriber Qos implementation
 *
 * (c) Copyright 2008-2024 Real-Time Innovations,
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

extern DDS_ReturnCode_t
DDS_SubscriberQos_set_from(
        struct DDS_SubscriberQos *out,
        const struct DDS_SubscriberQos *in,
        DDS_Boolean shallow_copy,
        DDS_DomainParticipant *participant);

#ifndef RTI_CERT
extern DDS_ReturnCode_t
DDS_SubscriberQos_finalize_managed(
        struct DDS_SubscriberQos *self,
        DDS_DomainParticipant *participant);
#endif

#endif

/*ci @} */

