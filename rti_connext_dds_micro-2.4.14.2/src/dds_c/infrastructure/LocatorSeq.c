/*
 * FILE: LocatorSeq.c - Locator sequence implementation
 *
 * (c) Copyright 2008-2021 Real-Time Innovations,
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
 * 13dec2021,tk MICRO-3226/PR.29479
 * - Fixed description following FILE to state Locator sequence
 * 19may2015,as MICRO-1193 Refactoring of Sequence API levels
 * 24oct2008,tk Created
 */
/*ce
 * \file
 * \brief Locator sequence implementation
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

/*** SOURCE_BEGIN ***/

#define T struct RTPS_Locator
#define TSeq DDS_LocatorSeq
#define TSeq_is_equal
#define REDA_SEQUENCE_USER_API
#include "reda/reda_sequence_defn.h"

/*ci @} */
