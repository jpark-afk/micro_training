/*
 TopicQos.cxx
 
 (c) Copyright, Real-Time Innovations, Sep 20, 2014-2015.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.

 modification history
---------------------
20sep2014,as  Created
===================================================================== */

#ifndef dds_cpp_topic_hxx
  #include "dds_cpp/dds_cpp_topic.hxx"
#endif
#ifndef dds_c_topic_h
  #include "dds_c/dds_c_topic.h"
#endif
/*** SOURCE_BEGIN ***/

#define T DDS_TopicQos
#define T_generate_extended
#include "SupportMethodsGen.hxx"
#undef T_generate_extended
#undef T
