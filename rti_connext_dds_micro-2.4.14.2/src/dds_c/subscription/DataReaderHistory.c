/*
 * FILE: DataReaderHistory.c - DataReader history sequence implementation
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
 * - Include DataReaderHistory.h
 * 15apr2021,tk MICRO-2979/PR.27549
 * - Removed unused functions from CERT:
 *   DDSHST_ReaderSample_TSeq
 * 05may2020,tk MICRO-2364/PR#27563 Added missing SOURCE_BEGIN
 * 19may2015,as MICRO-1193 Refactoring of Sequence API levels
 * 21aug2012,eh Created
 */
/*ce
 * \file
 * \brief DataReader history sequence implementation
 */
/*ci \addtogroup DDSSubscriptionModule
 * @{
 */
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_subscription_h
#include "dds_c/dds_c_subscription.h"
#endif

#include "DataReaderHistory.h"

/*** SOURCE_BEGIN ***/

#ifndef RTI_CERT
/*ci
 * \brief Sequence to manage untyped sample sequences
 */
#define T DDSHST_ReaderSample_T
#define TSeq DDSHST_ReaderSample_TSeq
#include "reda/reda_sequence_defn.h"
#endif

const char *const DDSHST_READER_DEFAULT_HISTORY_NAME = "rh";

/*ci @} */
