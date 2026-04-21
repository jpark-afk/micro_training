/*
 * FILE: reda_epoch.h - Epoch API
 *
 * Copyright 2012-2021 Real-Time Innovations, Inc.
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
 * 04jan2012,tk Written
 */
/*ce
 * \file
 */
#ifndef reda_epoch_h
#define reda_epoch_h

#ifndef reda_dll_h
#include "reda/reda_dll.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*ci
 * \file
 * \brief Defines the REDA_Epoch interface
 *
 * \defgroup REDAEpochClass REDA Epoch
 * \ingroup REDAModule
 *
 * \brief Generic epoch counter
 *
 * \details
 *
 * Used to determine staleness, version counting, and central to lockless tables.
 * That is, multiple threads can concurrently write the epoch.
 * Multiple concurrent readers will never see intermediate garbage values.
 * We assume writing an "unsigned int" is atomic on all architectures.
 *
 * All weakly referenced information has an epoch which is strictly
 * monotonically increasing with every change. If the referrer finds that
 * his idea of epoch is different than the current epoch, it means that
 * his information is stale and therefore should not be used.
 */

/*ci \addtogroup REDAEpochClass
 *  @{
 */

/*ci
 * \brief Definition of the epoch type
 */
typedef RTI_INT32 REDA_Epoch_T;

/*ci
 * \def REDA_EPOCH_UNKNOWN
 *
 * \brief Unknown REDA_Epoch
 *
 * \details
 *
 * Usually, when an epoch is unknown, the actor should assume the information
 * tied to the unknown epoch is stale.  Depending on the usage, it may also
 * mean that the caller wants to find the correct latest epoch.
*/
#define REDA_EPOCH_UNKNOWN (0)

/*ci
 * \def REDA_EPOCH_HALF_VALUE
 *
 * \brief Constant used for wrap detection. ASSUMPTION: an unsigned
 * int is at least 32 bits (it's OK
 * if it's bigger).
 */
#define REDA_EPOCH_HALF_VALUE ((REDA_Epoch)0x80000000)

/*ci
 * \brief Tests whether this epoch is greater than the value
 *
 * \param[in] me    Epoch to test
 * \param[in] value Epoch to value to test against
 *
 * \return 1 if true, 0 otherwise
 *
 * \sa REDA_Epoch_is_less
 */
MUST_CHECK_RETURN REDADllExport RTI_INT32
REDA_Epoch_is_greater(REDA_Epoch_T me, REDA_Epoch_T value);

#define REDA_Epoch_is_greater(me, value) (me > value)

/*ci
 * \brief Tests whether this epoch is less than the value
 *
 * \param[in] me    Epoch to test
 * \param[in] value Epoch to value to test against
 *
 * \return 1 if true, 0 otherwise
 *
 * \sa REDA_Epoch_is_greater
 */
MUST_CHECK_RETURN REDADllExport RTI_INT32
REDA_Epoch_is_less(REDA_Epoch_T me, REDA_Epoch_T value);

#define REDA_Epoch_is_less(me, value) (me < value)

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif /* reda_epoch_h */

/*ci @} */
