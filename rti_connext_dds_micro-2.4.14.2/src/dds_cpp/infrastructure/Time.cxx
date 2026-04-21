/* Time.cxx

 (c) Copyright, Real-Time Innovations, 2013-2016.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.

 modification history
 ---------------------
 07apr2016,tk MICRO-1541 Fixed assignment operator issues in C++
 30dec2015,as Created
 ===================================================================== */

#ifndef osapi_config_h
#include "osapi/osapi_config.h"
#endif
#ifndef dds_cpp_infrastructure_h
#include "dds_cpp/dds_cpp_infrastructure.hxx"
#endif
/*** SOURCE_BEGIN ***/

bool
DDS_Time_t::greater_than(const DDS_Time_t& other)
{
    return DDS_Time_t_greater_than(*this, other);
}

bool
DDS_Time_t::is_zero()
{
    return DDS_Time_is_zero(this);
}
