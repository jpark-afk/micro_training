/*
 (c) Copyright, Real-Time Innovations, 2009-2015.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.
*/

#ifndef ThroughputCommon_h
#include "ThroughputCommon.h"
#endif

const struct DDS_Duration_t ten_millisec = { 0, NANOSEC_PER_MILLISEC * 10 };
const struct DDS_Duration_t twenty_millisec = { 0, NANOSEC_PER_MILLISEC * 20 };
const struct DDS_Duration_t hundred_millisec =
    { 0, NANOSEC_PER_MILLISEC * 100 };
const struct DDS_Duration_t one_second = { 1, 0 };
const struct DDS_Duration_t three_second = { 3, 0 };
