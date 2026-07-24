/*
 * FILE: SampleInfoSeq.c - SampleInfoSeq implementation
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
 * \brief SampleInfoSeq implementation
 */
/*ci \addtogroup DDSInfrastructureModule
 * @{
 */
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif

/*** SOURCE_BEGIN ***/

#define T struct DDS_SampleInfo
#define TSeq DDS_SampleInfoSeq
#define REDA_SEQUENCE_API REDA_SEQUENCE_API_FULL
#include "reda/reda_sequence_defn.h"


/*ci @} */
