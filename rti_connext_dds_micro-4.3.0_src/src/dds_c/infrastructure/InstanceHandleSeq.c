/*
 * FILE: InstanceHandleSeq.c - InstanceHandle sequence implementation
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
 * 19may2015,as MICRO-1193 Refactoring of Sequence API levels
 * 24oct2008,tk Created
 */
/*ce
 * \file
 * \brief InstanceHandle sequence implementation
 */
/*ci \addtogroup DDSInfrastructureModule
 * @{
 */
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_common_h
#include "dds_c/dds_c_common.h"
#endif
#ifndef dds_c_infrastructure_h
#include "dds_c/dds_c_infrastructure.h"
#endif

/*** SOURCE_BEGIN ***/

#define T DDS_InstanceHandle_t
#define TSeq DDS_InstanceHandleSeq
#include "reda/reda_sequence_defn.h"
#undef T
#undef TSeq

/*ci @} */

