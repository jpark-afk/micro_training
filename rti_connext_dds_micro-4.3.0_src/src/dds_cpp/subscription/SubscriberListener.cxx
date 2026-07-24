/* 

 (c) Copyright, Real-Time Innovations, 2013-2015.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.

 modification history
--------------------
16may2014,as  Moved forwarding functions from Subscriber.cxx
19jul2013,as  Major C++ update
16jan2013,eh  Created
===================================================================== */
#ifndef dds_cpp_subscription_hxx
  #include "dds_cpp/dds_cpp_subscription.hxx"
#endif

#include "Subscriber.hxx"
/*** SOURCE_BEGIN ***/

void
DDSSubscriberListener_forward_on_data_on_readers(
   void* listener_data,
   DDS_Subscriber* c_subscriber)
{
    DDSSubscriber* subscriber = NULL;
    DDSSubscriberListener* sub_listener =
        (DDSSubscriberListener*) listener_data;

    if (sub_listener != NULL)
    {
        subscriber =
                (DDSSubscriber *)DDS_Entity_get_wrapper(
                        DDS_Subscriber_as_entity(c_subscriber));
        sub_listener->on_data_on_readers(subscriber);
    }
}
