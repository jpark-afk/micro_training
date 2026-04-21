/*
 * FILE: reda_sequenceNumber_impl.h - Sequence Number API
 *
 * Copyright 2012-2015 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 06oct2014,eh MICRO-881: Set SEQUENCE_NUMBER_UNKNOWN as {-1,0}
 * 20apr2012,tk Written
 */
/*ci
 * \file
 *
 * \brief The REDA Sequence number provides an API to manipulate 64 bit
 *        sequence numbers
 *
 * \defgroup REDASequenceNumberClass REDA SequenceNumber
 * \ingroup REDAModule
 *
 * \details
 *
 * The REDA Sequence number provides an API to manipulate 64 bit
 * sequence numbers, including comparison and arithmetic operations.
 *
 *  The implementation does not assume a 64-bit processor architecture,
 *  but rather relies on 32-bit logic on the "low" and "high" part of the
 *  sequence number.
 */
/*ci
 * \addtogroup REDASequenceNumberClass
 * @{
 */
#ifndef reda_sequenceNumber_h
#define reda_sequenceNumber_h

#ifndef reda_dll_h
#include "reda/reda_dll.h"
#endif
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

struct REDA_Buffer;

/*ci
 * \brief internal representation of a 64-bit signed number.
 */
struct REDA_SequenceNumber
{
    /*ci
     * \brief The most significant part of the sequence number
     */
    RTI_INT32 high;

    /*ci
     * \brief The least significant part of the sequence number
     */
    RTI_UINT32 low;
};

/*ci
 * \def REDA_SEQUENCE_NUMBER_HIGH_MAX
 * \brief Highest, most positive value for the high part of the seq num
 */
#define REDA_SEQUENCE_NUMBER_HIGH_MAX ((RTI_INT32)0x7fffffff)

/*ci
 * \def REDA_SEQUENCE_NUMBER_LOW_MAX
 * \brief Highest, most positive value for the low part of the seq num
 */
#define REDA_SEQUENCE_NUMBER_LOW_MAX ((RTI_UINT32)0xffffffff)

/*ci
 * \def REDA_SEQUENCE_NUMBER_MAX
 * \brief Highest, most positive value for the 64-bit sequence number
 *
 * \details
 * Intended to be used as:
 * \code
 *  struct REDA_SequenceNumber seqNum = REDA_SEQUENCE_NUMBER_MAX;
 * \endcode
 */
#define REDA_SEQUENCE_NUMBER_MAX {REDA_SEQUENCE_NUMBER_HIGH_MAX, \
                                  REDA_SEQUENCE_NUMBER_LOW_MAX}

/*ci
 * \def REDA_SEQUENCE_NUMBER_ZERO
 *
 * \brief Zero value for the 64-bit sequence number
 *
 * \details
 *
 * Intended to be used as:
 * \code
 *     struct REDA_SequenceNumber seqNum = REDA_SEQUENCE_NUMBER_ZERO;
 * \endcode
 */
#define REDA_SEQUENCE_NUMBER_ZERO {0, 0}

/*ci
 * \def  REDA_SEQUENCE_NUMBER_UNKNOWN
 *
 * \brief Unknown value (-1) for the 64-bit sequence number
 *
 * \details
 *
 * Intended to be used as:
 * \code
 *     struct REDA_SequenceNumber seqNum = REDA_SEQUENCE_NUMBER_UNKNOWN;
 * \endcode
 *
 */
#define REDA_SEQUENCE_NUMBER_UNKNOWN \
  {(RTI_INT32)0xffffffff, (RTI_UINT32)0}

/*ci
 *\brief Reset a sequence number to zero.
 *
 *\details
 *
 * Reset a sequence number to zero. It is safer and more portable than setting
 * the individual fields.
 *
 * \param[in] me Sequence number to set to zero
 */
REDADllExport void
REDA_SequenceNumber_set_zero(struct REDA_SequenceNumber *me);

/*ci
 *\brief Set a sequence number to the maximum value.
 *
 *\details
 *
 * Set a sequence number to the maximum value. It is safer and more
 * portable than setting the individual fields.
 *
 * \param[in] me Sequence number to set to max
 */
REDADllExport void
REDA_SequenceNumber_set_maximum(struct REDA_SequenceNumber *me);

/*ci
 *\brief Compare two sequence numbers.
 *
 * \details
 *
 * Compare two sequences
 *
 * \param[in] me Left side of comparison
 * \param[in] sn Right side of comparison
 *
 *\return
 *  <UL>
 *      <LI>If me > sn then return 1.
 *      <LI>If me < sn then return -1.
 *      <LI>If me == sn then return 0.
 *  </UL>
 */
MUST_CHECK_RETURN REDADllExport RTI_INT32
REDA_SequenceNumber_compare(const struct REDA_SequenceNumber *me,
                           const struct REDA_SequenceNumber *sn);

/*ci
 * \brief Test if a sequence number is unknown.
 *
 * \param[in] me Sequence number to test
 *
 * \return RTI_TRUE if sequence number is unknown, RTI_FALSE otherwise
 */
REDADllExport RTI_BOOL
REDA_SequenceNumber_is_unknown(const struct REDA_SequenceNumber *me);

/*ci
 * \brief Test if a sequence number is zero.
 *
 * \param[in] me Sequence number to test
 *
 * \return RTI_TRUE if sequence number is zero, RTI_FALSE otherwise
 */
REDADllExport RTI_BOOL
REDA_SequenceNumber_is_zero(const struct REDA_SequenceNumber *me);

/*ci
 * \brief Add two sequence numbers.
 *
 * \details
 *
 * Add two sequence numbers as sn1 + sn2.
 *
 * \param[out] answer sn1 + sn2
 * \param[in]  sn1    Sequence number 1 to add
 * \param[in]  sn2    Sequence number 2 to add
 *
 * \return On return answer = sn1 + sn2
 */
REDADllExport void
REDA_SequenceNumber_add(struct REDA_SequenceNumber *answer,
                       const struct REDA_SequenceNumber *sn1,
                       const struct REDA_SequenceNumber *sn2);

/*ci
 * \brief Subtract two sequence numbers.
 *
 * \details
 *
 * Subtract two sequence numbers as sn1 - sn2.
 *
 * \param[out] answer sn1 - sn2
 * \param[in]  sn1    Sequence number 1 to subtract from
 * \param[in]  sn2    Sequence number to subtract
 *
 * \return On return answer = sn1 - sn2
 */
REDADllExport void
REDA_SequenceNumber_subtract(struct REDA_SequenceNumber *answer,
                             const struct REDA_SequenceNumber *sn1,
                             const struct REDA_SequenceNumber *sn2);

/*ci
 * \brief Increment a sequence number by one.
 *
 * \details
 *
 * Increment a sequence number by one.
 *
 * \param[in] me Sequence number to increment
 *
 * \sa \ref REDA_SequenceNumber_minusminus
 */
REDADllExport void
REDA_SequenceNumber_plusplus(struct REDA_SequenceNumber *me);

/*ci
 * \brief Decrement a sequence number by one.
 *
 * \details
 *
 * Decrement a sequence number by one.
 *
 * \param[in] me Sequence number to decrement by 1
 *
 * \sa \ref REDA_SequenceNumber_plusplus
 */
REDADllExport void
REDA_SequenceNumber_minusminus(struct REDA_SequenceNumber *me);

/*ci
 * \brief Increment a sequence number by another sequence number.
 *
 * \details
 *
 * Increment a sequence number with the value of another sequence number.
 *
 * \param[in] me Sequence number to increment
 * \param[in] sn Sequence number to increment by
 *
 * \sa \ref REDA_SequenceNumber_plusplus
 */
REDADllExport void
REDA_SequenceNumber_increment(struct REDA_SequenceNumber *me,
                             const struct REDA_SequenceNumber *sn);

/*ci
 * \brief Decrement a sequence number by another sequence number.
 *
 * \details
 *
 * Decrement a sequence number with the value of another sequence number.
 *
 * \param[in] me Sequence number to decrement
 * \param[in] sn Sequence number to decrement by
 *
 * \sa \ref REDA_SequenceNumber_increment
 */
REDADllExport void
REDA_SequenceNumber_decrement(struct REDA_SequenceNumber *me,
                             const struct REDA_SequenceNumber *sn);

/*ci
 * \brief Find the maximum of two sequence numbers
 *
 * \details
 *
 * Return the maximum of two sequence numbers in answer
 *
 * \param[out] answer max(sn1,sn2)
 * \param[in]  sn1    Sequence number 1
 * \param[in]  sn2    Sequence number 2 to compare against
 *
 * \return On return answer = max(sn1,sn2)
 *
 * \sa \ref REDA_SequenceNumber_min
 */
REDADllExport void
REDA_SequenceNumber_max(struct REDA_SequenceNumber *answer,
                       const struct REDA_SequenceNumber *sn1,
                       const struct REDA_SequenceNumber *sn2);

/*ci
 * \brief Find the minimum of two sequence numbers
 *
 * \details
 *
 * Return the minimum of two sequence numbers in answer
 *
 * \param[out] answer min(sn1,sn2)
 * \param[in]  sn1    Sequence number 1
 * \param[in]  sn2    Sequence number 2 to compare against
 *
 * \return On return answer = min(sn1,sn2)
 *
 * \sa \ref REDA_SequenceNumber_max
 */
REDADllExport void
REDA_SequenceNumber_min(struct REDA_SequenceNumber *answer,
                       const struct REDA_SequenceNumber *sn1,
                       const struct REDA_SequenceNumber *sn2);

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif /* reda_sequenceNumber_h */

/*ci @} */

