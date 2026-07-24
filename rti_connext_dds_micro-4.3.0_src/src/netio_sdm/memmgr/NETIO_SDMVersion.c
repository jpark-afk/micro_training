/*
 * FILE: NETIO_SDMVersion.c - Product Version
 *
 * (c) Copyright 2018-2018 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
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

#include "netio_sdm/netio_sdm.h"

#ifndef RTI_CERT

RTI_PRIVATE const char* const NETIO_SDM_Version_fv_String =
                                RTIME_BUILD_STRING_BUILDER("rti_me_netiosdm");

const char*
NETIO_SDM_get_version(void)
{
    return NETIO_SDM_Version_fv_String;
}

#endif
