/*
 * FILE: DataReaderHistory.c - DataReader history sequence implementation
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
 * 19may2015,as  MICRO-1193 Refactoring of Sequence API levels
 * 21aug2012,eh  Created
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

/*ci
 * \brief Sequence to manage untyped sample sequences
 */
#define T DDSHST_ReaderSample_T
#define TSeq DDSHST_ReaderSample_TSeq
#include "reda/reda_sequence_defn.h"

const char *const DDSHST_READER_DEFAULT_HISTORY_NAME = "rh";

/*ci @} */
