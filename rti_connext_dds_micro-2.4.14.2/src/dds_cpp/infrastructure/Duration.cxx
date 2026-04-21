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

void
DDS_Duration_t::to_ntp_time(OSAPI_NtpTime *dst)
{
    DDS_Duration_to_ntp_time(this, dst);
}

#ifndef RTI_CERT
void
DDS_Duration_t::from_ntp_time(const OSAPI_NtpTime *src)
{
    DDS_Duration_from_ntp_time(this, src);
}
#endif

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
