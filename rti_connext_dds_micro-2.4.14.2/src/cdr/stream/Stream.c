/*
 * FILE: Stream.c - CDR Stream API
 *
 * (c) Copyright, Real-Time Innovations, 2012-2015.
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 14sep2021,tk MICRO-3147/PR.29483
 * - Call CDR_Stream_has_remaining_space in CDR_Stream_check_size to check
 *   for underflow.
 * 15jun2021,tk MICRO-3062/PR.29176
 * - Corrected precondition checks in CDR_Stream_endian_and_byteswap_get
 *   (replaced && with ||).
 * 10jan2021,tk  MICRO-2807 / PR.27549  Removed unused functions for CERT
 * 13dec2020,tk
 *     - MICRO-2710/PR.28397
 *       - Changed CDR_Stream_is_checksum_protected to return RTI_TRUE or RTI_FALSE
 *       - Changed CDR_Stream_is_checksum_crc32 to return RTI_TRUE or RTI_FALSE
 *       - Changed CDR_Stream_is_checksum_hdrext to return RTI_TRUE or RTI_FALSE
 * 10dec2020,tk  MICRO-2712/PR#28305
 *     - Clarify function header comments for arguments
 * 30jun2015,eh  MICRO-1371/PR#15109 Fix comment for set_buffer()
 * 23feb2015,eh  MICRO-1075: remove and replace macros
 * 07sep2014,eh  MICRO-906: fix CDR_Stream_checkSize()
 * 29jul2014,tk  MICRO-859/PR#10221 - Fixed Source code comment
 *               MICRO-860/PR#10222 - Check CDR options
 *               MICRO-861/PR#10224 - Removed redundant checks
 *               MICRO-862/PR#10226 - Do not realign buffer if not enough space
 * 08may2014,eh  MICRO-194/Verocel PR 1084: commentary on buff_ptr >= buffer
 * 24mar2012,kaj Written
 */
/*ci @ingroup CDRModule
 * \file
 * \brief CDR Stream API 
 *  
 * \details 
 * Operations to manage a CDR stream
 */
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef cdr_stream_h
#include "cdr/cdr_stream.h"
#endif
#ifndef cdr_serialize_h
#include "cdr/cdr_serialize.h"
#endif
#ifndef cdr_log_h
#include "cdr/cdr_log.h"
#endif

/*** SOURCE_BEGIN ***/

/* A CDR_Stream_t's buff_ptr is always >= its buffer:
 * 
 * - buff_ptr is initialized to buffer
 * 
 * - buff_ptr is reset to buffer
 * 
 * - buff_ptr can be changed by CDR_Stream_set_current_position_offset(), and
 *   there the "num" offset is ensured to be non-negative before addit it
 *   to buff_ptr.
 * 
 * - buff_ptr can be changed by CDR_Stream_IncrementCurrentPosition(), and
 *   there the amount is ensured to be positive before adding it to buff_ptr.
 * 
 * - buff_ptr is set by CDR_Stream_Alilgn(), and because align_base is equal
 *   to the initialized or reset buffer and buff_ptr, the updated buff_ptr is
 *   no less than buffer.
 *
 */
/*ci 
 * \brief 
 * Free resources of a stream 
 *  
 * \param[in] cdrs Stream to free 
 *   
 */
#ifndef RTI_CERT
void
CDR_Stream_free(struct CDR_Stream_t *cdrs)
{
    if (cdrs)
    {
        if (cdrs->real_buff)
        {
            OSAPI_Heap_free_array(cdrs->real_buff);
        }
        OSAPI_Heap_free_struct(cdrs);
    }
}
#endif /* !RTI_CERT */

struct CDR_Stream_t*
CDR_Stream_alloc(RTI_UINT32 buff_size)
{
    struct CDR_Stream_t *cdrs = NULL;
    struct CDR_Stream_t *return_cdrs = NULL;
    RTI_UINT8 lsb = 0;
    RTI_UINT8 align = CDR_MAX_ALIGNMENT - 1;

    OSAPI_PRECONDITION(buff_size <= 0,
                   return NULL,
                   OSAPI_Log_entry_add_uint("bufsize<=0",buff_size,RTI_TRUE);)

    OSAPI_Heap_allocate_struct(&cdrs, struct CDR_Stream_t);
    if (cdrs == NULL)
    {
        CDR_LOG_STREAM_ALLOC(OSAPI_LOGKIND_ERROR,
                             sizeof(struct CDR_Stream_t))
        goto finally;
    }

    /* allocate buffer memory (requested buffsize plus alignment overhead) */
    OSAPI_Heap_allocate_array(&cdrs->real_buff,
                              buff_size + (CDR_MAX_ALIGNMENT - 1),char);
    if (cdrs->real_buff == NULL)
    {
        CDR_LOG_STREAM_ALLOC(OSAPI_LOGKIND_ERROR,
                             buff_size + (CDR_MAX_ALIGNMENT - 1))
        goto finally;
    }

    /* align allocated buffer to CDR_MAX_ALIGNMENT */
    cdrs->buffer = cdrs->real_buff;
    lsb = (RTI_UINT8)(cdrs->buffer - OSAPI_ADDRESS_ZERO);
    lsb = lsb & align;
    if (lsb != 0)
    {
        cdrs->buffer += (CDR_MAX_ALIGNMENT - lsb);
    }

    /* set buffer length to requested size */
    cdrs->length = buff_size;

    /* reset the stream pointer */
    cdrs->buff_ptr = cdrs->buffer;
    cdrs->align_base = cdrs->buffer;
    cdrs->need_byte_swap = CDR_BYTESWAP_INVALID;
    cdrs->checksum_info = CDR_STREAM_CHECKSUM_NONE;
    CDR_Stream_set_vendor(cdrs,CDR_VENDOR_ID_MAJOR_RTI,CDR_VENDOR_ID_MINOR_MICRO);

    return_cdrs = cdrs;

finally:

    return return_cdrs;
}

void
CDR_Stream_reset(struct CDR_Stream_t *cdrs)
{
    OSAPI_PRECONDITION(cdrs == NULL,
                           return,
                           OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_TRUE);)

    (cdrs)->buff_ptr = (cdrs)->buffer;
    (cdrs)->align_base = (cdrs)->buffer;
}

void
CDR_Stream_byteswap_set(struct CDR_Stream_t *cdrs, RTI_BOOL little_endian)
{
    OSAPI_PRECONDITION_ALWAYS(cdrs == NULL,
                           return,
                           OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_TRUE);)
#ifdef RTI_ENDIAN_LITTLE
    cdrs->need_byte_swap = little_endian ? RTI_FALSE : RTI_TRUE;
    cdrs->endian = RTI_CDR_ENDIAN_LITTLE;
#else
    cdrs->need_byte_swap = little_endian ? RTI_TRUE : RTI_FALSE;
    cdrs->endian = RTI_CDR_ENDIAN_BIG;
#endif
}

/* documented in cdr_stream.h */
void
CDR_Stream_endian_and_byteswap_get(struct CDR_Stream_t *cdrs, 
                                   RTI_BOOL *need_byte_swap, 
                                   RTI_UINT16 *endian)
{
    OSAPI_PRECONDITION_ALWAYS(
      (cdrs == NULL) ||
      (need_byte_swap == NULL) ||
      (endian == NULL),
      return,
      OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
      OSAPI_Log_entry_add_pointer("need_byte_swap",need_byte_swap,RTI_FALSE);
      OSAPI_Log_entry_add_pointer("endian",endian,RTI_TRUE);)

    *need_byte_swap = cdrs->need_byte_swap;
    *endian = cdrs->endian;
}

/* documented in cdr_stream.h */
void
CDR_Stream_endian_and_byteswap_set(struct CDR_Stream_t *cdrs, 
                                   RTI_BOOL need_byte_swap, 
                                   RTI_UINT16 endian)
{
    OSAPI_PRECONDITION_ALWAYS(cdrs == NULL,
                            return,
                            OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_TRUE);)

    cdrs->need_byte_swap = need_byte_swap;
    cdrs->endian = endian;
}

RTI_UINT32
CDR_Stream_get_current_position_offset(struct CDR_Stream_t *cdrs)
{
    OSAPI_PRECONDITION_ALWAYS(cdrs == NULL,
                           return 0,
                           OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_TRUE);)

    return (RTI_UINT32)((cdrs)->buff_ptr - (cdrs)->buffer);
}

RTI_BOOL
CDR_Stream_set_current_position_offset(struct CDR_Stream_t *cdrs,
                                      RTI_UINT32 num)
{
    OSAPI_PRECONDITION_ALWAYS(cdrs == NULL,
                            return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_TRUE);)

    /* unsigned num cannot be less than zero */
    if (num <= cdrs->length)
    {
        cdrs->buff_ptr = cdrs->buffer + num;
        return RTI_TRUE;
    }

    CDR_LOG_SET_OFFSET(OSAPI_LOGKIND_ERROR,num)
    return RTI_FALSE;
}

RTI_BOOL
CDR_Stream_increment_current_position(struct CDR_Stream_t *me,
                                      RTI_INT32 amount)
{
    OSAPI_PRECONDITION_ALWAYS(me == NULL || me->buff_ptr == NULL,
                            return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("me",me,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("me->buff_ptr",
                                   (me != NULL ? me->buff_ptr : NULL),RTI_TRUE);)

    if (amount != 0)
    {
        if (amount > 0)
        {
            if (amount > (RTI_INT32)(me->length -
                                    CDR_Stream_get_current_position_offset(me)))
            {
                CDR_LOG_INCR_OFFSET(OSAPI_LOGKIND_ERROR,(RTI_INT32)amount)
                return RTI_FALSE;
            }
        }
        else
        {
            if (-(amount) > (RTI_INT32)CDR_Stream_get_current_position_offset(me))
            {
                CDR_LOG_INCR_OFFSET(OSAPI_LOGKIND_ERROR,(RTI_INT32)amount)
                return RTI_FALSE;
            }
        }
    
        me->buff_ptr += amount;
    }

    return RTI_TRUE;
}

RTI_BOOL
CDR_Stream_set_buffer(struct CDR_Stream_t *me, char *buf, RTI_UINT32 length)
{
    OSAPI_PRECONDITION(me == NULL,
                           return RTI_FALSE,
                           OSAPI_Log_entry_add_pointer("me",me,RTI_TRUE);)

    me->real_buff = buf;
    me->buffer = buf;
    me->align_base = buf;
    me->buff_ptr = buf;
    me->length = length;

    return RTI_TRUE;
}

RTI_BOOL
CDR_Stream_check_size(struct CDR_Stream_t *me,RTI_UINT32 size)
{
    OSAPI_PRECONDITION_ALWAYS(me == NULL,
                           return RTI_FALSE,
                           OSAPI_Log_entry_add_pointer("me",me,RTI_TRUE);)

    return CDR_Stream_has_remaining_space(me,size);
}

char*
CDR_Stream_get_current_position_ptr(struct CDR_Stream_t *me)
{
    OSAPI_PRECONDITION_ALWAYS(me == NULL,
                              return NULL,
                              OSAPI_Log_entry_add_pointer("me",me,RTI_TRUE);)

    return me->buff_ptr;
}

void
CDR_Stream_align(struct CDR_Stream_t *stream, 
                 RTI_UINT8 align)
{
    stream->buff_ptr = stream->align_base + 
        (((stream->buff_ptr - stream->align_base) + (align - 1)) & ~(align - 1));
}

RTI_BOOL
CDR_Stream_is_byte_swapped(struct CDR_Stream_t *me)
{
    OSAPI_PRECONDITION_ALWAYS(me == NULL,
                              return RTI_FALSE,
                              OSAPI_Log_entry_add_pointer("me",me,RTI_TRUE);)

    return me->need_byte_swap;
}

void
CDR_Stream_set_vendor(struct CDR_Stream_t *me,
                      RTI_UINT8 vendor_id_major,
                      RTI_UINT8 vendor_id_minor)
{
    me->vendor_id_major = vendor_id_major;
    me->vendor_id_minor = vendor_id_minor;
}

RTI_BOOL
CDR_Stream_is_vendor_rti(struct CDR_Stream_t *me)
{
    return (me->vendor_id_major == CDR_VENDOR_ID_MAJOR_RTI) &&
                ((me->vendor_id_minor == CDR_VENDOR_ID_MINOR_MICRO) ||
                 (me->vendor_id_minor == CDR_VENDOR_ID_MINOR_CORE));
}

void
CDR_Stream_set_checksum_info(struct CDR_Stream_t *me,RTI_UINT32 checksum_info)
{
    me->checksum_info = checksum_info;
}

#ifndef RTI_CERT
RTI_BOOL
CDR_Stream_is_checksum_protected(struct CDR_Stream_t *me)
{
    return (me->checksum_info & CDR_STREAM_CHECKSUM_MASK) ? RTI_TRUE : RTI_FALSE;
}
#endif

RTI_BOOL
CDR_Stream_is_checksum_crc32(struct CDR_Stream_t *me)
{
    return (me->checksum_info & CDR_STREAM_CHECKSUM_CRC32) ? RTI_TRUE : RTI_FALSE;
}

#ifndef RTI_CERT
RTI_BOOL
CDR_Stream_is_checksum_hdrext(struct CDR_Stream_t *me)
{
    return (me->checksum_info & CDR_STREAM_CHECKSUM_HDREXT) ? RTI_TRUE : RTI_FALSE;
}
#endif
