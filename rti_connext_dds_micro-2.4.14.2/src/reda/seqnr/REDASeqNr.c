/*
 * FILE: REDASeqNr.c - Sequence Number implementation
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
 * 15apr2021,tk MICRO-2979/PR.27549
 * - Removed unused functions from CERT:
 *   REDA_SequenceNumber_set_maximum
 *   REDA_SequenceNumber_decrement
 * 06apr2021,tk MICRO-2968/PR.28504
 *  - Simplified overflow test in REDA_SequenceNumber_add and
 *    REDA_SequenceNumber_increment for MC/DC coverage.
 * 05may2020,tk MICRO-2364/PR#27563 Added missing SOURCE_BEGIN
 * 25feb2015,tk Written, Refactored from reda_buffer.h
 */
/*ce
 * \file
 * \brief Implementation of the REDA SeqNr API
 */
#ifndef osapi_config_h
#include "osapi/osapi_config.h"
#endif
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif
#ifndef reda_sequenceNumber_h
#include "reda/reda_sequenceNumber.h"
#endif

/*** SOURCE_BEGIN ***/

void
REDA_SequenceNumber_set_zero(struct REDA_SequenceNumber *seqnr)
{
    seqnr->high = 0;
    seqnr->low = 0;
}

#ifndef RTI_CERT
void
REDA_SequenceNumber_set_maximum(struct REDA_SequenceNumber *seqnr)
{
    seqnr->high = REDA_SEQUENCE_NUMBER_HIGH_MAX;
    seqnr->low = REDA_SEQUENCE_NUMBER_LOW_MAX;
}
#endif

RTI_INT32
REDA_SequenceNumber_compare(const struct REDA_SequenceNumber *left,
                            const struct REDA_SequenceNumber *right)
{
    return ((((left)->high) > ((right)->high)) ? 1 :
   ((((left)->high) < ((right)->high)) ? -1 :
    ((((left)->low) > ((right)->low)) ? 1 :
     ((((left)->low) < ((right)->low)) ? -1 : 0))));
}

RTI_BOOL
REDA_SequenceNumber_is_unknown(const struct REDA_SequenceNumber *seqnr)
{
    return ((seqnr->high == (RTI_INT32)0xffffffff) &&
            (seqnr->low == (RTI_UINT32)0xffffffff));
}

RTI_BOOL
REDA_SequenceNumber_is_zero(const struct REDA_SequenceNumber *seqnr)
{
    return (seqnr->high == 0) && (seqnr->low == 0) ? RTI_TRUE : RTI_FALSE;
}

void
REDA_SequenceNumber_add(struct REDA_SequenceNumber *answer,
                       const struct REDA_SequenceNumber *sn1,
                       const struct REDA_SequenceNumber *sn2)
{
    answer->high = sn1->high + sn2->high;

    if (sn1->low > (UINT_MAX - sn2->low))
    {
        ++answer->high;
    }

    /* Because low is an unsigned integer, the overflow is well-defined
     * by the C standard and it wraps around.
     */
    answer->low = sn1->low + sn2->low;
}

void
REDA_SequenceNumber_subtract(struct REDA_SequenceNumber *answer,
                             const struct REDA_SequenceNumber *sn1,
                             const struct REDA_SequenceNumber *sn2)
{
    answer->high  = sn1->high - sn2->high;
    answer->low = sn1->low - sn2->low;

    if (answer->low > sn1->low)
    {
        --answer->high;
    }
}

void
REDA_SequenceNumber_plusplus(struct REDA_SequenceNumber *seqnr)
{
  (++seqnr->low == 0 ? ++seqnr->high : 0);
}

void
REDA_SequenceNumber_minusminus(struct REDA_SequenceNumber *seqnr)
{
    RTI_UINT32 low = seqnr->low;

    --seqnr->low;
    if (seqnr->low > low)
    {
        --seqnr->high;
    }
}

void
REDA_SequenceNumber_increment(struct REDA_SequenceNumber *answer,
                             const struct REDA_SequenceNumber *seqnr)
{
    answer->high += seqnr->high;

    if (answer->low > (UINT_MAX - seqnr->low))
    {
        ++answer->high;
    }

    /* Because low is an unsigned integer, the overflow is well-defined
     * by the C standard and it wraps around.
     */
    answer->low += seqnr->low;
}

#ifndef RTI_CERT
void
REDA_SequenceNumber_decrement(struct REDA_SequenceNumber *answer,
                             const struct REDA_SequenceNumber *seqnr)
{
    RTI_UINT32 low = answer->low;

    answer->high -= seqnr->high;
    answer->low -= seqnr->low;

    if (answer->low > low)
    {
        --answer->high;
    }
}
#endif

void
REDA_SequenceNumber_max(struct REDA_SequenceNumber *answer,
                       const struct REDA_SequenceNumber *sn1,
                       const struct REDA_SequenceNumber *sn2)
{
    if (REDA_SequenceNumber_compare(sn1,sn2) > 0)
    {
        *answer = *sn1;
    }
    else
    {
        *answer = *sn2;
    }
}

void
REDA_SequenceNumber_min(struct REDA_SequenceNumber *answer,
                       const struct REDA_SequenceNumber *sn1,
                       const struct REDA_SequenceNumber *sn2)
{
    if (REDA_SequenceNumber_compare(sn1,sn2) < 0)
    {
        *answer = *sn1;
    }
    else
    {
        *answer = *sn2;
    }
}
