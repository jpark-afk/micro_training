/*
 * FILE: dds_c_domain_trust.h
 *
 * (c) Copyright  Real-Time Innovations, 2018-2025
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
 * 11Jun2018,asorbini  Created.
 */

#ifndef dds_c_domain_trust_h
#define dds_c_domain_trust_h

#ifndef dds_c_dll_h
#include "dds_c/dds_c_dll.h"
#endif

#ifndef dds_c_domain_h
#include "dds_c/dds_c_domain.h"
#endif

#ifndef rtps_trust_plugin_h
#include "rtps/rtps_trust_plugin.h"
#endif


#ifdef __cplusplus
extern "C"
{
#endif


MUST_CHECK_RETURN DDSCDllExport RTI_BOOL
DDS_DomainParticipant_is_trust_enabled(DDS_DomainParticipant *participant);

MUST_CHECK_RETURN DDSCDllExport RTI_BOOL
DDS_DomainParticipantQos_is_trust_enabled(
            const struct DDS_DomainParticipantQos *qos);












































































































































































































#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif /* dds_c_domain_trust_h */
