/* Duration.cxx

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

DDS_Boolean
DDS_Duration_t::is_infinite()
{
    return DDS_Duration_is_infinite(this);
}

int
DDS_Duration_t::compare(const DDS_Duration_t *other)
{
    return DDS_Duration_compare(this, other);
}

DDS_Boolean
DDS_Duration_t::equal(const DDS_Duration_t *other)
{
    return DDS_Duration_equal(this, other);
}

DDS_Boolean
DDS_Duration_t::is_zero()
{
    return DDS_Duration_is_zero(this);
}

DDS_Duration_t
DDS_Duration_t::subtract(const DDS_Duration_t &other)
{
    DDS_Duration_t result;

    if (DDS_Duration_is_infinite(this))
    {
        result.sec = DDS_DURATION_INFINITE_SEC;
        result.nanosec = DDS_DURATION_INFINITE_NSEC;
    }
    else if (DDS_Duration_is_infinite(&other))
    {
        result.sec = 0;
        result.nanosec = 0;
    }
    else
    {
        result.sec  = this->sec - other.sec;
        result.nanosec = this->nanosec - other.nanosec;

        if (result.nanosec > this->nanosec)
        {
            result.sec--;
        }
    }
    
    return result;
}
