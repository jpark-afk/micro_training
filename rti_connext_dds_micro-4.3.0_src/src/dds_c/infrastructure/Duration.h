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

extern void
DDS_Duration_to_ntp_format(const struct DDS_Duration_t *self,
    RTI_INT32 *sec_out, RTI_UINT32 *frac_out);
    
#endif

