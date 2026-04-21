/*
 * FILE: Bitmap.c - RTPS bitmap handling functions
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
 * 20feb2021,tk  MICRO-2876/PR#28773
 *   - Removed the redundant bit_array_size in
 *     RTPS_Bitmap_get_unreserved_count().
 * 06jul2015,eh  MICRO-1388/PR#15217 Fix coding standard violations
 * 13jun2015,eh  MICRO-1235/PR#14804 Check exact bit array length when
 *               deserializing
 * 23feb2015,eh  MICRO-1081: fix function/var names to conform to coding std
 * 23feb2015,eh  MICRO-1075: remove and replace macros
 * 27apr2008,cc Written
 */
/*ci 
 *  
 * \brief Functions for manipulating RTPS bitmaps 
 *  
 * \details 
 * Operations to manipulate bitmaps are necessary to send and receive RTPS 
 * submessages that contain them (e.g. ACKNACK, GAP). 
 * 
 */
#include "rtps/rtps_rtps.h"
#ifndef rtps_log_h
#include "rtps/rtps_log.h"
#endif
#ifndef cdr_serialize_h
#include "cdr/cdr_serialize.h"
#endif

/* Convenience macros representing bit patterns as specified in their names
 */
#define RTPS_BITMAP_32_BITS_MSB (0x80000000U)
#define RTPS_BITMAP_32_BITS_LSB (0x00000001U)
#define RTPS_BITMAP_32_ONES (0xFFFFFFFFU)
#define RTPS_BITMAP_24_ONES (0xFFFFFFU)
#define RTPS_BITMAP_16_ONES (0xFFFFU)
#define RTPS_BITMAP_8_ONES  (0xFFU)

/*** SOURCE_BEGIN ***/

/*ci 
 * \brief Get the size of a bitmap's internal array of bits 
 *  
 * \param[in] me Bitmap 
 *  
 * \return Minimum number of 32-bit integers to contain bitmap's bit_count   
 */
RTI_PRIVATE RTI_INT32
RTPS_Bitmap_get_array_size(struct RTPS_Bitmap *me)
{
    return ((me->bit_count + 31) >> 5);
}


/*ci
 * \brief
 * Resets bitmap to specified lead sequence number and bit count
 * 
 * \details
 * Wraps macro, implemented and valid only for debug mode.
 * 
 * \param[in] me        Bitmap
 * \param[in] sn        New lead sequence number
 * \param[in] bit_count New bit count
 *
 */
void
RTPS_Bitmap_reset(struct RTPS_Bitmap *me,
                  const struct REDA_SequenceNumber *sn,
                  RTI_INT32 bit_count)
{
    int i = 0;                           
    me->lead = *sn;                                            
    me->bit_count = bit_count;                                      
    for (i = 0; i < RTPS_BITMAP_32BITS_ARRAY_SIZE_MAX; ++i) 
    {
        me->bits[i] = 0;                      
    }  
}

/*ci
 * \brief
 * Sets a single bit of bitmap to specified value
 * 
 * \param[in]  me      Bitmap
 * \param[out] existed Flag whether bit already set to on-value.  Optional. 
 * \param[in]  id      Sequence number of bit to set
 * \param[in]  on      Value to set bit
 * 
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
RTPS_Bitmap_set_bit(struct RTPS_Bitmap *me,
                    RTI_BOOL *existed,
                    const struct REDA_SequenceNumber *id, 
                    RTI_BOOL on)
{
    RTI_INT32 distance, offset;
    RTI_UINT32 mask, before_value;

    if (existed != NULL)
    {
        *existed = RTI_FALSE;
    }

    distance = RTPS_SequenceNumber_get_distance(&me->lead, id);

    /* Check for out of range. */
    if (REDA_SequenceNumber_compare(id, &me->lead) < 0
        || distance >= me->bit_count)
    {
        RTPS_LOG_SEQ_NUM_OUT_OF_RANGE(OSAPI_LOGKIND_INFO, 
                                      distance, me->bit_count)
        return RTI_FALSE;
    }

    offset = (distance >> 5);
    mask = 1U << (31U - (distance & 0x1F));

    before_value = me->bits[offset];

    if (on)
    {
        me->bits[offset] |= mask;
    }
    else
    {
        me->bits[offset] &= ~mask;
    }

    if (existed != NULL)
    {
        *existed = (before_value == me->bits[offset]) ? RTI_TRUE : RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief
 * Get the value of a single bit of bitmap
 * 
 * \param[in]  me  Bitmap
 * \param[out] bit Returned bit 
 * \param[in]  id  Sequence number of bit to get
 * 
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
RTPS_Bitmap_get_bit(const struct RTPS_Bitmap *me,
                    RTI_BOOL *bit,
                    const struct REDA_SequenceNumber *id)
{
    RTI_INT32 byte_offset, bit_offset , distance;

    distance = RTPS_SequenceNumber_get_distance(&me->lead, id);

    /* Check for out of range. */
    if (REDA_SequenceNumber_compare(id, &me->lead) < 0 ||
        distance >= me->bit_count)
    {
        return RTI_FALSE;
    }

    byte_offset = (distance >> 5);
    bit_offset  = (distance & 0x1F);
    *bit = 0x01 & (me->bits[byte_offset] >> (31 - bit_offset ));

    return RTI_TRUE;
}

/*ci
 * \brief
 * Get the sequence number of the first bit in bitmap that matches the input 
 * value 
 * 
 * \param[in] me         Bitmap
 * \param[out] position  Sequence number of first bit matching search_bit.
 *                       If no match found, position is one greater than last
 *                       valid bit of bitmap.
 * \param[in] search_bit Value of bit for which to search 
 * 
 * \return RTI_TRUE on successfully finding matching bit, RTI_FALSE on failure
 */
RTI_BOOL
RTPS_Bitmap_get_first_bit(const struct RTPS_Bitmap *me,
                        struct REDA_SequenceNumber *position,
                        RTI_BOOL search_bit)
{
    RTI_BOOL found = RTI_FALSE;
    RTI_UINT32 value;
    RTI_INT32 bit_array_size, extra_bits, valid_bits;
    RTI_INT32 i, j = 0;

    bit_array_size = RTPS_Bitmap_get_array_size((struct RTPS_Bitmap*)me);
    extra_bits = (me->bit_count & 0x1F);
    for (i = 0; i < bit_array_size && !found; ++i)
    {
        /* If a full 32 bits, first check all bits at once. */
        if ((extra_bits == 0) || (i != (bit_array_size - 1)))
        {
            if ((search_bit && me->bits[i] == 0) ||
                (!search_bit && me->bits[i] == RTPS_BITMAP_32_ONES))
            {
                continue;
            }
            valid_bits = 32;
        }
        else
        {
            /* The last integer may not have a full 32 valid bits. */
            valid_bits = extra_bits;
        }

        /* Search bit by bit. */
        for (j = 0, value = me->bits[i];
             j < valid_bits && !found; ++j, value <<= 1)
        {
            if ((search_bit && ((value & RTPS_BITMAP_32_BITS_MSB) ==
                               RTPS_BITMAP_32_BITS_MSB)) ||
                (!search_bit && ((value & RTPS_BITMAP_32_BITS_MSB) == 0)))
            {
                found = RTI_TRUE;
            }
        }
    }

    if (position != NULL)
    {
        position->high = 0;
        position->low = found ? (32U * ((RTI_UINT32)i - 1U) + ((RTI_UINT32)j - 1U)) : (RTI_UINT32)me->bit_count;
        REDA_SequenceNumber_increment(position, &me->lead);
    }

    return found;
}


/*ci
 * \brief
 * Get the sequence number of the last bit in bitmap that matches the input 
 * value 
 * 
 * \param[in]  me        Bitmap
 * \param[out] position  Sequence number of last bit matching search_bit.
 *                       If no match found, position is zero.
 * \param[in] search_bit Value of bit for which to search 
 * 
 * \return RTI_TRUE on successfully finding matching bit, RTI_FALSE on failure
 */
RTI_BOOL
RTPS_Bitmap_get_last_bit(const struct RTPS_Bitmap *me,
                       struct REDA_SequenceNumber *position,
                       RTI_BOOL search_bit)
{
    RTI_BOOL found = RTI_FALSE;
    RTI_UINT32 value;
    RTI_INT32 bit_array_size, extra_bits, valid_bits;
    RTI_INT32 i, j = 0;

    bit_array_size = RTPS_Bitmap_get_array_size((struct RTPS_Bitmap*)me);
    extra_bits = (me->bit_count & 0x1F);
    for (i = bit_array_size-1; i >= 0 && !found; --i)
    {
        /* If a full 32 bits, first check all bits at once. */
        if ((extra_bits == 0) || (i != (bit_array_size - 1)))
        {
            if ((search_bit && me->bits[i] == 0) ||
                (!search_bit && me->bits[i] == RTPS_BITMAP_32_ONES))
            {
                continue;
            }
            valid_bits = 32;
        }
        else
        {
            /* The last integer may not have a full 32 valid bits. */
            valid_bits = extra_bits;
        }

        /* Search bit by bit. */
        value = me->bits[i];
        for (j = 32-valid_bits; j > 0; --j)
        {
            value >>= 1;
        }

        for (j = valid_bits-1; j >= 0 && !found; --j, value >>= 1)
        {
            if ((search_bit && ((value & RTPS_BITMAP_32_BITS_LSB) ==
                               RTPS_BITMAP_32_BITS_LSB)) ||
                (!search_bit && ((value & RTPS_BITMAP_32_BITS_LSB) == 0)))
            {
                found = RTI_TRUE;
            }
        }
    }

    if (position != NULL)
    {
        REDA_SequenceNumber_set_zero(position);
        if (found)
        {
            position->low = (32U * ((RTI_UINT32)i + 1) + ((RTI_UINT32)j + 1));
            REDA_SequenceNumber_increment(position, &me->lead);
        }
    }

    return found;
}

/*ci
 * \brief
 * Fill a range of bits in bitmap with specified value
 * 
 * \param[in] me            Bitmap
 * \param[in] first_seq_num First sequence number in range to fill
 * \param[in] last_seq_num  Last sequence number in range to fill
 * \param[in] bit           Value to set bits in range
 * 
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
RTPS_Bitmap_fill(struct RTPS_Bitmap *me,
                 const struct REDA_SequenceNumber *first_seq_num,
                 const struct REDA_SequenceNumber *last_seq_num, 
                 RTI_BOOL bit)
{
    RTI_INT32 first_bit, last_bit, first_32_bits, last_32_bits;
    RTI_INT32 start_bit, stop_bit;
    RTI_UINT32 mask;
    RTI_INT32 i, j;

    /* Guarantee first <= last ordering, otherwise fail. */
    if (REDA_SequenceNumber_compare(first_seq_num, last_seq_num) > 0)
    {
        return RTI_FALSE;
    }
    /* Given ordering, if range falls outside of bitmap, noop success. */
    if ((REDA_SequenceNumber_compare(last_seq_num, &me->lead) < 0) ||
        ((REDA_SequenceNumber_compare(first_seq_num, &me->lead) >= 0) &&
         (RTPS_SequenceNumber_get_distance(first_seq_num, &me->lead) >=
          me->bit_count)) || (me->bit_count == 0))
    {
        return RTI_TRUE;
    }

    /* Convert sequences to offsets and truncate to bitmap's boundaries. */
    first_bit = (REDA_SequenceNumber_compare(first_seq_num, &me->lead) <= 0) ?
        0 : RTPS_SequenceNumber_get_distance(first_seq_num, &me->lead);
    last_bit = RTPS_SequenceNumber_get_distance(last_seq_num, &me->lead);
    if (last_bit >= me->bit_count)
    {
        last_bit = me->bit_count - 1;
    }

    /* Iterate through the bitmap and set bits as needed. */
    first_32_bits = (first_bit >> 5);
    last_32_bits = (last_bit >> 5);

    for (i = first_32_bits; i <= last_32_bits; ++i)
    {
        if (i == first_32_bits)
        {
            start_bit = (first_bit & 0x1F);
            mask = 1U << (31U - (RTI_UINT32)start_bit);
        }
        else
        {
            start_bit = 0;
            mask = RTPS_BITMAP_32_BITS_MSB;
        }
        stop_bit = (i == last_32_bits) ? (last_bit & 0x1F) : 31;

        /* Try to set all 32 bits at once if possible. */
        if ((start_bit == 0) && (stop_bit == 31))
        {
            me->bits[i] = bit ? RTPS_BITMAP_32_ONES : 0;
            continue;
        }

        for (j = start_bit; j <= stop_bit; ++j, mask >>= 1)
        {
            if (bit)
            {
                me->bits[i] |= mask;
            }
            else
            {
                me->bits[i] &= ~mask;
            }
        }
    }

    return RTI_TRUE;
}

/*ci
 * \brief
 * Merge another bitmap with self bitmap 
 *  
 * \details 
 * Copies '1' bits from source bitmap whose sequence numbers overlap with self 
 * bitmap.  Bitcount and lead SN of self bitmap are preserved. 
 * 
 * \param[in] me     Self bitmap
 * \param[in] source Bitmap to merge with self
 */
void
RTPS_Bitmap_merge(struct RTPS_Bitmap *me,
                  const struct RTPS_Bitmap *source)
{

    struct REDA_SequenceNumber first_seq_num, last_seq_num;
    RTI_INT32 first_bit, last_bit, first_32_bits, last_32_bits, start_bit,
        stop_bit, distance, i, j;
    RTI_UINT32 mask;

    first_seq_num = source->lead;

    /* last = first + source->bitCount - 1 */
    last_seq_num.high = 0;
    last_seq_num.low = (RTI_UINT32)source->bit_count;
    REDA_SequenceNumber_increment(&last_seq_num, &first_seq_num);
    REDA_SequenceNumber_minusminus(&last_seq_num);

    /* If source falls outside of me, noop. */
    if (REDA_SequenceNumber_compare(&last_seq_num, &me->lead) < 0 ||
        (REDA_SequenceNumber_compare(&first_seq_num, &me->lead) >= 0 &&
         RTPS_SequenceNumber_get_distance(&first_seq_num,
                                           &me->lead) >= me->bit_count)
        || me->bit_count == 0 || source->bit_count == 0)
    {
        return;
    }

    /* Truncate source sequence numbers to me's boundaries. */
    if (REDA_SequenceNumber_compare(&first_seq_num, &me->lead) < 0)
    {
        first_seq_num = me->lead;
    }
    if (RTPS_SequenceNumber_get_distance(&last_seq_num, &me->lead) >=
        me->bit_count)
    {
        last_seq_num.high = 0;
        last_seq_num.low = (RTI_UINT32)me->bit_count;
        REDA_SequenceNumber_increment(&last_seq_num, &me->lead);
        REDA_SequenceNumber_minusminus(&last_seq_num);
    }

    /* Get the bit indexes of the sequences into the source bitmap. */
    first_bit = RTPS_SequenceNumber_get_distance(&first_seq_num, &source->lead);
    last_bit = RTPS_SequenceNumber_get_distance(&last_seq_num, &source->lead);

    /* Iterate through source and set corresponding bits in dest as needed. */
    distance = RTPS_SequenceNumber_get_distance(&first_seq_num, &me->lead);
    first_32_bits = (first_bit >> 5);
    last_32_bits = (last_bit >> 5);
    for (i = first_32_bits; i <= last_32_bits; ++i)
    {
        if (i == first_32_bits)
        {
            start_bit = (first_bit & 0x1F);
            mask = 1U << (31 - (RTI_UINT32)start_bit);
        }
        else
        {
            start_bit = 0;
            mask = RTPS_BITMAP_32_BITS_MSB;
        }
        if (i == last_32_bits)
        {
            stop_bit = (last_bit & 0x1F);
        }
        else
        {
            stop_bit = 31;
        }

        /* If possible, check 32 bits at once to see if there are any 1's. */
        if ((start_bit == 0 && stop_bit == 31) && /* addressing all 32 bits */
            source->bits[i] == 0)
        {
            /* nothing in source */
            distance += 32;     /* skip to next 32 bits */
            continue;
        }

        /* Else we need to find the 1 bits in the source bitmap. */
        for (j = start_bit; j <= stop_bit; ++j, mask >>= 1, ++distance)
        {
            if ((source->bits[i] & mask) == mask)
            {
                /* Need to set the bit in the me bitmap. */
                me->bits[(distance >> 5)] |= 
                    (1U << (31U - (RTI_UINT32)(distance & 0x1F)));
            }
        }
    }
}

/*ci
 * \brief Count the number of ones in an integer.
 * 
 * \param[in] value    Integer to count 1s in
 * \param[in] max_bits The maximum number of bits to count  
 * 
 * \return The number of 1s in value
 */
RTI_PRIVATE RTI_INT32
RTPS_Bitmap_count_ones(RTI_UINT32 value, RTI_UINT32 max_bits)
{
    RTI_INT32 ones = 0;
    RTI_UINT32 one = 0x80000000;

    while ((value) && max_bits)
    {
        if (value & one)
        {
            ones++;
        }
        one >>= 1;
        --max_bits;
    }

    return ones;
}

/*ci
 * \brief
 * Shift lead sequence number of bitmap to new value
 *  
 * \details 
 * Preserves bitcount of bitmap, setting new shifted-in bits to zero.
 * 
 * \param[in] me      Self bitmap
 * \param[in] seq_num New lead sequence number
 * 
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
RTPS_Bitmap_shift(struct RTPS_Bitmap *me,
                  const struct REDA_SequenceNumber *seq_num)
{
    RTI_BOOL ok = RTI_FALSE;
    RTI_INT32 i, distance, bit_array_size, full_32_bits , extra_bits, valid_bits;
    RTI_UINT32 mask, value;

    i = REDA_SequenceNumber_compare(seq_num, &me->lead);

    if (i < 0)
    {
        RTPS_LOG_SHIFT_SEQ_NUM_OUT_OF_RANGE(OSAPI_LOGKIND_WARNING)
        goto finally;
    }
    if (i == 0)
    {
        /* noop */
        goto success;
    }

    distance = RTPS_SequenceNumber_get_distance(seq_num, &me->lead);
    if (distance >= me->bit_count)
    {
        /* 0 fill */
        RTPS_Bitmap_reset(me, seq_num, me->bit_count);
        goto success;
    }

    me->lead = *seq_num;
    bit_array_size = RTPS_Bitmap_get_array_size(me);
    full_32_bits  = (distance >> 5);

    if (full_32_bits  > 0)
    {
        /* have at least 1 full 32 bit in distance */
        valid_bits = (me->bit_count & 0x1F);

        /* Shift bits an integer (32 bits) at a time. */
        for (i = 0; (i + full_32_bits ) < bit_array_size; ++i)
        {
            /* The last integer may not have a full 32 valid bits to shift. */
            if (((i + full_32_bits ) == (bit_array_size - 1)) && (valid_bits != 0))
            {
                mask = RTPS_BITMAP_32_ONES << (32 - valid_bits);
                me->bits[i] = me->bits[i + full_32_bits ] & mask;
            }
            else
            {
                /* copy entire later 32 bits to earlier 32 bits */
                me->bits[i] = me->bits[i + full_32_bits ];
            }
        }

        /* The rest: fill with zeroes if nowhere to shift from. */
        for (; i < bit_array_size; ++i)
        {
            me->bits[i] = 0;
        }
    }

    extra_bits = (distance & 0x1F);
    if (extra_bits > 0)
    {
        valid_bits = (me->bit_count & 0x1F);
        /* Shift each integer in the integer array that was fully shifted
         * in the previous step by an additional extra_bits bits sucking in
         * bits from the next integer as necessary but don't handle the
         * last integer yet as it has nowhere to suck in bits if needed.
         */
        for (i = 0; (i + full_32_bits ) < (bit_array_size - 1); ++i)
        {
            me->bits[i] <<= extra_bits;

            /* Suck in bits from next integer as needed. */
            mask = RTPS_BITMAP_32_ONES << (32 - extra_bits);

            /* If next to last, we may not have enough bits to suck in from. */
            if (((i + 1 + full_32_bits ) == (bit_array_size - 1)) &&
                (extra_bits > valid_bits) && (valid_bits != 0))
            {
                mask <<= (extra_bits - valid_bits);
            }

            value = me->bits[i + 1] & mask;
            value >>= (32 - extra_bits);
            me->bits[i] |= value;
        }

        /* Now handle the last integer of the bitmap that was fully shifted. */
        me->bits[i] <<= extra_bits;

        /* Make sure newly shifted in bits of the last integer are zeroed. */
        mask = RTPS_BITMAP_32_ONES;
        if (valid_bits != 0)
        {
            mask <<= (32 - valid_bits);
        }
        mask <<= extra_bits;
        me->bits[i] &= mask;
    }

success:
        ok = RTI_TRUE;

finally:
        return ok;
}

RTI_INT32
RTPS_Bitmap_get_unreserved_count(struct RTPS_Bitmap *me,
                                 const struct REDA_SequenceNumber *seq_num)
{
    RTI_INT32 i, element_count,max_bits,max_distance;
    RTI_INT32 count = 0;

    i = REDA_SequenceNumber_compare(seq_num, &me->lead);

    if (i <= 0)
    {
        return 0;
    }

    /* Do not count outside the receive window */
    max_distance = RTPS_SequenceNumber_get_distance(seq_num, &me->lead);
    if (max_distance > me->bit_count)
    {
        max_distance = me->bit_count;
    }

    element_count = (max_distance + 31) >> 5;

    max_bits = max_distance;

    for (i = 0; (i < element_count); ++i)
    {
        if (max_bits > 32)
        {
            count += RTPS_Bitmap_count_ones(me->bits[i],32);
        }
        else
        {
            count += RTPS_Bitmap_count_ones(me->bits[i],(RTI_UINT32)max_bits);
        }
        max_bits -= 32;
    }

    return max_distance - count;
}

/*ci
 * \brief
 * Truncates bitmap to end at specified sequence number
 *  
 * \details 
 * Updates bitcount of bitmap to reflect number of valid bits up to truncated 
 * sequence number.  If truncation sequence number is greater than current last 
 * sequence number of bitmap, no truncation is done and bitcount stays the same.
 * 
 * \param[in] me      Self bitmap
 * \param[in] seq_num Sequence number after which bitmap is truncated.
 * 
 */
void
RTPS_Bitmap_truncate(struct RTPS_Bitmap *me,
                     const struct REDA_SequenceNumber *seq_num)
{
    RTI_INT32 distance;

    if (REDA_SequenceNumber_compare(seq_num, &me->lead) < 0)
    {
        me->bit_count = 0;
        return;
    }

    distance = RTPS_SequenceNumber_get_distance(&me->lead, seq_num);
    if (distance < me->bit_count)
    {
        me->bit_count = distance + 1;
    }
}

/*ci
 * \brief
 * Toggle bits of bitmap 
 *  
 * \details 
 * Change '1' to '0' and vice versa. 
 *  
 * \param[inout] me Self bitmap
 * 
 */
void
RTPS_Bitmap_invert(struct RTPS_Bitmap *me)
{
    RTI_INT32 bit_array_size, i;

    bit_array_size = RTPS_Bitmap_get_array_size(me);
    for (i = 0; i < bit_array_size; ++i)
    {
        me->bits[i] = ~(me->bits[i]);
    }
}

/*ci
 * \brief
 * Deserialize from buffer a bitmap 
 *  
 * \details 
 * Change '1' to '0' and vice versa. 
 *  
 * \param[in] me           Self bitmap
 * \param[in] stream_ptr   Pointer to serialized buffer
 * \param[in] max_bits_len Remaining space of submessage to deserialize 
 *                         the variable length bits of bitmap
 * \param[in] need_byte_swap Flag whether byte swap is necessary for
 * deserialization 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 * 
 */
RTI_BOOL
RTPS_Bitmap_deserialize(struct RTPS_Bitmap *me,
                        const char **stream_ptr, 
                        RTI_UINT32 max_bits_len,
                        RTI_BOOL need_byte_swap)
{
    RTI_INT32 i, bit_array_size;
    char *stream = (char *)(*stream_ptr);

    RTPS_SequenceNumber_deserialize(&stream, &(me->lead), need_byte_swap);
    CDR_deserialize_long(&stream, &me->bit_count, need_byte_swap);

    /* Make sure that bit_count does not get set to an invalid value.
     * Assume that the maximum bit_count is RTPS_BITMAP_SIZE_MAX. */
    if ((me->bit_count > RTPS_BITMAP_SIZE_MAX) || (me->bit_count < 0))
    {
        /* Set bit_count to 0 so that we don't interpret the bitmap array. */
        RTPS_LOG_BITCOUNT_OUT_OF_BOUNDS(OSAPI_LOGKIND_ERROR, me->bit_count)
        me->bit_count = 0;
        return RTI_FALSE;
    }

    /* Calculate only once for efficiency */
    bit_array_size = RTPS_Bitmap_get_array_size(me);

    /* Must have enough space to deserialize bits */
    if (((RTI_UINT32)bit_array_size*sizeof(CDR_UnsignedLong)) != max_bits_len)
    {
        RTPS_LOG_BITCOUNT_OUT_OF_BOUNDS(OSAPI_LOGKIND_ERROR, me->bit_count)
        return RTI_FALSE;
    }

    for (i = 0; i < bit_array_size; ++i)
    {
        CDR_deserialize_unsigned_long(&stream, &me->bits[i], need_byte_swap);
    }

    /* Atomically update the stream position */
    *stream_ptr = stream;

    return RTI_TRUE;
}

