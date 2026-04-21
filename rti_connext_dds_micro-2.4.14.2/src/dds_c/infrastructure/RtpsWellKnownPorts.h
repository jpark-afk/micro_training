/*
 * FILE: RtpsWellKnownPorts.h - RtpsWellKnown ports implementation
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
 * 20sep2014,as Exposed is_equal within module dds_c
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief RtpsWellKnown ports implementation
 */
/*ci \addtogroup DDSInfrastructureModule
 * @{
 */
#ifndef RtpsWellKnownPorts_h
#define RtpsWellKnownPorts_h

#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif

DDS_Boolean
DDS_RtpsWellKnownPorts_is_equal(const struct DDS_RtpsWellKnownPorts *left,
                                const struct DDS_RtpsWellKnownPorts *right);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_RtpsWellKnownPorts_is_consistent(
                    const struct DDS_RtpsWellKnownPorts_t *self);

MUST_CHECK_RETURN extern DDS_Long
DDS_RtpsWellKnownPorts_get_max_participant_index(
                    const struct DDS_RtpsWellKnownPorts_t *rtpsWellKnownPorts);

#endif

/*ci @} */

