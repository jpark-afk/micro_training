/*
 * FILE: ProductVersion.c - ProductVersion implementation
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
 * 20sep2014,as Removed use of deprecated header dds_c_tpolicy_gen.h
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief ProductVersion implementation
 */
/*ci \addtogroup DDSInfrastructureModule
 * @{
 */
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif

/* in case cmake is not used to compile use a hardcoded buildid */
#if RTIME_ENABLE_BUILDID
#include "dds_c_buildid.h"
#else
#define RTIME_BUILD_ID "NOT_GENERATED"
#endif

#ifndef RTI_CERT

RTI_PRIVATE const char* const DDS_Version_fv_String=
                                        RTIME_BUILD_STRING_BUILDER("rti_me");

const char*
DDSC_Library_get_version(void)
{
    return DDS_Version_fv_String;
}

#endif

/*ci @} */
