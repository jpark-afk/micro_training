/* 

 (c) Copyright, Real-Time Innovations, 2013-2015.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.

 modification history
--------------------
22dec2015,as  MICRO-1514 Pass Status objects to callbacks by reference
16may2014,as  Created with functions from Topic.cxx
===================================================================== */
#ifndef dds_cpp_topic_hxx
  #include "dds_cpp/dds_cpp_topic.hxx"
#endif

#include "Topic.hxx"
/*** SOURCE_BEGIN ***/

void DDSTopicListener_forward_on_inconsistent_topic(
                            void *listener_data,
                            DDS_Topic *c_topic,
                            const DDS_InconsistentTopicStatus *status)
{
    DDSTopic* topic = NULL;
    DDSTopicListener *topic_listener = NULL;

    topic_listener = (DDSTopicListener*) listener_data;
    if (topic_listener != NULL)
    {
        topic = (DDSTopic*) DDS_Entity_get_wrapper(
                                DDS_Topic_as_entity(c_topic));
        topic_listener->on_inconsistent_topic(topic, *status);
    }
}
