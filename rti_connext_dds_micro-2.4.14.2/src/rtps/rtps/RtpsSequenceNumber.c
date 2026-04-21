/*
 * FILE: RtpsSequenceNumber.c - RTPS sequence number handling
 *
 * Copyright 2008-2021 Real-Time Innovations, Inc.
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
 * 23feb2015,eh  MICRO-1081: fix function/var names to conform to coding std
 * 23feb2015,eh  MICRO-1075: remove and replace macros
 * 28apr2014,as MICRO-273 (Verocel PR#1426) Document upstream robustness checks
 *              of arguments to RTPS_SequenceNumber_get_distance
 * 12jun2008,tk Created
 */

/*ci \file
 *  
 * \brief Operations on sequence numbers used by RTPS
 *  
 */


#include "osapi/osapi_types.h"
#include "cdr/cdr_serialize.h"
#include "rtps/rtps_rtps.h"

/*** SOURCE_BEGIN ***/

/*ci 
 *\brief 
 * Computes the distance between two sequence numbers 
 * 
 * \details
 * Arguments are assumed to be valid and are not verified by the function.
 * It is up to the invoker to pass two non-NULL arguments.
 *
 * NOTE: This operation is meant for internal use only. It is leveraged by
 * operations of RTPS_Bitmap and it is always passed valid arguments;
 * typically these arguments are stack-allocated copies of REDA_SequenceNumber,
 * taken from internal objects and/or received messages by the RTPS interface;
 * preconditions checks, on arguments of calls upstream to this operations,
 * guarantee that only valid memory (i.e. non-NULL and of the appropriate type)
 * is eventually passed to this function.
 * 
 * \param[in] s1 First sequence number
 * \param[in] s2 Second sequence number
 * 
 * \return An integer between 0 and RTPS_BITMAP_DISTANCE_MAX
 */
RTI_INT32
RTPS_SequenceNumber_get_distance(const struct REDA_SequenceNumber *s1,
                                 const struct REDA_SequenceNumber *s2)
{
    struct REDA_SequenceNumber s;
    if (REDA_SequenceNumber_compare(s1, s2) >= 0)
    {
        REDA_SequenceNumber_subtract(&s, s1, s2);
    }
    else
    {
        REDA_SequenceNumber_subtract(&s, s2, s1);
    }

    if ((s.high > 0) || (s.low > RTPS_BITMAP_DISTANCE_MAX))
    {
        return RTPS_BITMAP_DISTANCE_MAX;
    }

    return (RTI_INT32)s.low;
}

/*ci \brief Deserialize sequence number from stream buffer 
 * 
 * \param[in] src_buffer Deserialization buffer pointer 
 * \param[inout] instance Deserialized sequence number 
 * \param[in] byte_swap Whether to byte swap on deserialization 
 *
 */
void
RTPS_SequenceNumber_deserialize(char **src_buffer,
                                struct REDA_SequenceNumber *instance,
                                RTI_BOOL byte_swap)
{
    CDR_deserialize_long(src_buffer, (RTI_UINT32*)&instance->high, byte_swap); 
    CDR_deserialize_unsigned_long(src_buffer, (RTI_UINT32*)&instance->low, 
                                  byte_swap);
}
