/*
 * FILE: Duration.h - Duration implementation
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
 * 15apr2021,tk MICRO-2979/PR.27549
 * - Removed unused functions from CERT:
 *   DDS_Duration_from_ntp_time
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief Duration implementation
 * \ingroup DDSInfrastructureModule
 */
#ifndef Duration_h
#define Duration_h

#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif

MUST_CHECK_RETURN extern void
DDS_Duration_to_ntp_time(const struct DDS_Duration_t *self,
                         struct OSAPI_NtpTime *dst);


#ifndef RTI_CERT
MUST_CHECK_RETURN extern void
DDS_Duration_from_ntp_time(struct DDS_Duration_t *self,
                           const struct OSAPI_NtpTime *src);
#endif

#endif

