/*
 * FILE: ProductVersion.c - ProductVersion implementation
 *
 * (c) Copyright, Real-Time Innovations, 2017-2018.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 */
/*ce
 * \file
 * \brief ProductVersion implementation
 */
/*ci \addtogroup AppGenModule
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

#ifndef dds_c_discovery_plugin_h
#include "dds_c/dds_c_profile_plugin.h"
#endif
#ifndef appgen_plugin_h
#include "app_gen/app_gen_plugin.h"
#endif
#ifndef appgen_log_h
#include "app_gen/app_gen_log.h"
#endif

#ifndef RTI_CERT

RTI_PRIVATE const char* const APPGEN_Version_fv_String=
                            RTIME_BUILD_STRING_BUILDER("rti_me_appgen");

const char*
APPGEN_Factory_get_version(void)
{
    return APPGEN_Version_fv_String;
}

#endif

/*ci @} */
