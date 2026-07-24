/*
 * FILE: SubscriberEvent.h - Subscriber event implementation
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
 * 08nov2013,as MICRO-681 Complete implementation of WaitSets
 *              and support for StatusConditions
 * 05jul2012,tk Updated
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief Subscriber event implementation
 */
/*ci \addtogroup DDSSubscriptionModule
 * @{
 */
#ifndef SubscriberEvent_h
#define SubscriberEvent_h

MUST_CHECK_RETURN extern DDS_Boolean
DDS_SubscriberEvent_on_requested_incompatible_qos(DDS_Subscriber *subscriber,
                   DDS_DataReader *reader,
                   const struct DDS_RequestedIncompatibleQosStatus*status);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_SubscriberEvent_on_subscription_matched(DDS_Subscriber *subscriber,
                            DDS_DataReader *reader,
                            const struct DDS_SubscriptionMatchedStatus *status);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_SubscriberEvent_on_data_on_readers(DDS_Subscriber *subscriber);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_SubscriberEvent_on_data_available(DDS_Subscriber *subscriber,
                                      DDS_DataReader *reader);

MUST_CHECK_RETURN extern  DDS_Boolean
DDS_SubscriberEvent_on_sample_rejected(DDS_Subscriber *subscriber,
                           DDS_DataReader *reader,
                           const struct DDS_SampleRejectedStatus *status);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_SubscriberEvent_on_requested_deadline_missed(DDS_Subscriber *subscriber,
                     DDS_DataReader *reader,
                     const struct DDS_RequestedDeadlineMissedStatus *status);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_SubscriberEvent_on_liveliness_changed(DDS_Subscriber *subscriber,
                   DDS_DataReader *reader,
                   const struct DDS_LivelinessChangedStatus *status);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_SubscriberEvent_on_sample_lost(DDS_Subscriber *subscriber,
                                   DDS_DataReader *reader,
                                   const struct DDS_SampleLostStatus *status);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_SubscriberEvent_on_instance_replaced(
                    DDS_Subscriber *subscriber,
                    DDS_DataReader *reader,
                    const struct DDS_DataReaderInstanceReplacedStatus *status);

#endif

/*ci @} */

