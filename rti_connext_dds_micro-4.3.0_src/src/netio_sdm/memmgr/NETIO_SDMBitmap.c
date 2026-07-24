/*
 * FILE: NETIO_SDMBitmap.c - Bitmap implementation
 *
 * (c) Copyright 2018-2018 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#include "osapi/osapi_heap.h"
#include "NETIO_SDMDataWriterMemMgr.h"

RTI_BOOL
SDM_Bitmap_init(SDM_Bitmap_t *bitmap, RTI_UINT32 num_bits)
{
    RTI_UINT32 i, j, k, l;
    RTIBool retcode = RTI_FALSE;

    if ((bitmap == NULL ) || (num_bits == 0))
    {
        goto done;
    }

    /* IMPROVEMENT: Reevaluate the need to store the length. Faster search for
     *              free index, but sizeof(RTI_UINT32) per bitmap
     */
    j = num_bits % SDM_BITMAP_BITS_PER_ELEMENT;
    bitmap->length = num_bits / SDM_BITMAP_BITS_PER_ELEMENT + (j ? 1 : 0);
    bitmap->num_bits = num_bits;
    bitmap->bitfield = (RTI_UINT32*) OSAPI_Heap_allocate(
            (RTI_SIZE_T)bitmap->length,
            sizeof(RTI_UINT32));

    if (bitmap->bitfield == NULL)
    {
        goto done;
    }
    l = bitmap->length - (j ? 1 : 0);
    for (i = 0; i < l; ++i)
    {
        bitmap->bitfield[i] = SDM_BITMAP_ALL_FREE;
    }

    if (j)
    {
        for (i = 0, k = 0; i < j; ++i)
        {
            k = (k << SDM_BITMAP_BITS_PER_RECORD) | SDM_BITMAP_ELEMENT_FREE;
        }
        bitmap->bitfield[l] = k;
    }

    retcode = RTI_TRUE;

done:
    return retcode;
}

RTI_BOOL
SDM_Bitmap_finalize(SDM_Bitmap_t *bitmap)
{
    RTI_BOOL retcode = RTI_FALSE;

    if (bitmap == NULL)
    {
        goto done;
    }

    if (bitmap->bitfield)
    {
        OSAPI_Heap_free_array(bitmap->bitfield);
        bitmap->bitfield = NULL;
    }

    bitmap->num_bits = 0;

    retcode = RTI_TRUE;

done:
    return retcode;
}

void
SDM_Bitmap_set_search_range(
        SDM_Bitmap_t *bitmap,
        RTI_UINT32 start,
        RTI_UINT32 end,
        RTIBool reverse)
{
    bitmap->start = start;
    if (reverse)
    {
        bitmap->current = end;
    }
    else
    {
        bitmap->current = start;
    }
    bitmap->end = end;
    bitmap->wrap = RTI_FALSE;
}

RTI_PRIVATE RTI_UINT32 SDM_Bitmap_freeBitMap[16] =
{
    0x00000001,
    0x00000005,
    0x00000015,
    0x00000055,
    0x00000155,
    0x00000555,
    0x00001555,
    0x00005555,
    0x00015555,
    0x00055555,
    0x00155555,
    0x00555555,
    0x01555555,
    0x05555555,
    0x15555555,
    0x55555555
};

RTI_BOOL
SDM_Bitmap_get_prev_index(SDM_Bitmap_t *bitmap, RTI_INT32 *index)
{
    RTI_UINT32 j,k;
    RTI_BOOL retcode = RTI_FALSE;
    RTI_UINT32 current_index,current_bit;

    if ((bitmap->current < bitmap->start) || (bitmap->current > bitmap->end))
    {
        goto done;
    }

    current_index =  bitmap->current / SDM_BITMAP_BITS_PER_ELEMENT;
    current_bit = bitmap->current % SDM_BITMAP_BITS_PER_ELEMENT;

    while (!bitmap->wrap) {
        if ((current_index == 0) ||
                (bitmap->start / SDM_BITMAP_BITS_PER_ELEMENT ==
                bitmap->end / SDM_BITMAP_BITS_PER_ELEMENT))
        {
            bitmap->wrap = RTI_TRUE;
        }
        if (bitmap->bitfield[current_index] &
                SDM_Bitmap_freeBitMap[current_bit])
        {
            break;
        }
        if (!bitmap->wrap)
        {
            current_index--;
            current_bit = 15;
        }
    }

    if (!(bitmap->bitfield[current_index] &
            SDM_Bitmap_freeBitMap[current_bit]))
    {
        goto done;
    }

    bitmap->current = (current_index * SDM_BITMAP_BITS_PER_ELEMENT) +
            current_bit;

    for (k = 0, j = SDM_BITMAP_ELEMENT_FREE << (current_bit *
            SDM_BITMAP_BITS_PER_RECORD);
            (k < SDM_BITMAP_BITS_PER_ELEMENT) &&
            !(bitmap->bitfield[current_index] & j);
            k++, j >>= SDM_BITMAP_BITS_PER_RECORD,--bitmap->current)
    {
    }

    if (k == SDM_BITMAP_BITS_PER_ELEMENT)
    {
        goto done;
    }

    *index = (RTI_INT32)bitmap->current;
    --bitmap->current;

    bitmap->bitfield[current_index] &= ~j;

    retcode = RTI_TRUE;

done:
    return retcode;
}

RTI_BOOL
SDM_Bitmap_op_index_state(
        SDM_Bitmap_t *bitmap,
        RTI_INT32 index,
        RTI_UINT32 state,
        RTIBool clear)
{
    RTI_INT32 i;
    RTIBool retcode = RTI_FALSE;
    RTI_UINT32 mask;

    if ((bitmap == NULL ) || ((RTI_UINT32)index > bitmap->num_bits - 1))
    {
        goto done;
    }

    i = index / SDM_BITMAP_BITS_PER_ELEMENT;
    mask = (RTI_UINT32)(state <<
            (SDM_BITMAP_BITS_PER_RECORD *
            (index % SDM_BITMAP_BITS_PER_ELEMENT))
            );

    if (clear)
    {
        bitmap->bitfield[i] &= ~mask;
    }
    else
    {
        bitmap->bitfield[i] |= mask;
    }

    retcode = RTI_TRUE;

done:
    return retcode;
}
