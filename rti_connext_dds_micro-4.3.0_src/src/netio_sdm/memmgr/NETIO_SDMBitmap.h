/*
 * FILE: NETIO_SDMBitmap.h - Bitmap implementation
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
#ifndef netio_sdm_bitmap_h
#define netio_sdm_bitmap_h

/******************************************************************************
 *                             SDM_Bitmap
 ******************************************************************************/
#include "osapi/osapi_types.h"

typedef struct SDM_Bitmap
{
    RTI_UINT32 length;
    RTI_UINT32 num_bits;
    RTI_UINT32 *bitfield;
    RTI_UINT32 start;
    RTI_UINT32 end;
    RTI_UINT32 current;
    RTI_BOOL wrap;
} SDM_Bitmap_t;

RTI_BOOL
SDM_Bitmap_init(SDM_Bitmap_t *bitmap, RTI_UINT32 num_bits);

RTI_BOOL
SDM_Bitmap_finalize(SDM_Bitmap_t *bitmap);

void
SDM_Bitmap_set_search_range(
        SDM_Bitmap_t *bitmap,
        RTI_UINT32 start,
        RTI_UINT32 end,
        RTIBool reverse);

RTI_BOOL
SDM_Bitmap_get_prev_index(SDM_Bitmap_t *bitmap, RTI_INT32 *index);

RTI_BOOL
SDM_Bitmap_op_index_state(
        SDM_Bitmap_t *bitmap,
        RTI_INT32 index,
        RTI_UINT32 state,
        RTIBool clear);

#define SDM_BITMAP_ALL_FREE         (0x55555555)
#define SDM_BITMAP_ELEMENT_FREE     (0x1U)
#define SDM_BITMAP_ELEMENT_VALID    (0x2)
#define SDM_BITMAP_BITS_PER_ELEMENT (16)
#define SDM_BITMAP_BITS_PER_RECORD  (2)

#define SDM_Bitmap_free_index(__bitmap,__index) \
    SDM_Bitmap_op_index_state(__bitmap,__index,SDM_BITMAP_ELEMENT_FREE,0)

#define SDM_Bitmap_allocate_index(__bitmap,__index) \
    SDM_Bitmap_op_index_state(__bitmap,__index,SDM_BITMAP_ELEMENT_FREE,1)

#define SDM_Bitmap_validate_index(__bitmap,__index) \
    SDM_Bitmap_op_index_state(__bitmap,__index,SDM_BITMAP_ELEMENT_VALID,0)

#endif /* netio_sdm_bitmap_h */
