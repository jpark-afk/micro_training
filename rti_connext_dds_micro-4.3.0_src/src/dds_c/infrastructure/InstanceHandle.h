/*
 * FILE: InstanceHandle.h - InstanceHandle implementation
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
 * \ingroup DDSDomainModule
 * \brief InstanceHandle implementation
 */
/*ci \addtogroup DDSInfrastructureModule
 * @{
 */
#ifndef InstanceHandle_h
#define InstanceHandle_h

#include "dds_c/dds_c_infrastructure.h"

extern void
DDS_InstanceHandle_set_suffix(DDS_InstanceHandle_t *self,DDS_UnsignedLong suffix);

#endif

/*ci @} */

