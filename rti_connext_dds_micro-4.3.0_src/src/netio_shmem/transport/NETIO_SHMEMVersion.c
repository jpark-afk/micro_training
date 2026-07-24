/*
 * FILE: NETIO_SHMEMVersion.c - Product Version
 *
 * (c) Copyright 2018-2024 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
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

#include "netio_shmem/netio_shmem.h"

#ifndef RTI_CERT

RTI_PRIVATE const char* const NETIO_SHMEM_Version_fv_String =
                                RTIME_BUILD_STRING_BUILDER("rti_me_netioshmem");

const char*
NETIO_SHMEMInterfaceFactory_get_version(void)
{
    return NETIO_SHMEM_Version_fv_String;
}

#endif

/*ci @} */
