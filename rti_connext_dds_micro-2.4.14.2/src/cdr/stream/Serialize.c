/*
 * FILE: Serialize.c - CDR serialize API
 *
 * (c) Copyright, Real-Time Innovations, 2012-2022.
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
 * 08aug2022,tk MICRO-3367/PR.30121
 * - Placed { on its own line in:
 *   - CDR_get_max_size_serialized_string_array
 *   - CDR_get_max_size_serialized_string_sequence
 *   - CDR_Stream_serialize_non_primitive_sequence
 * 18feb2022,jh MICRO-3455
 * - Fixed compiler warning in CDR_Stream_has_remaining_space by moving
 *   the cast to RTI_UINT32.
 * 13dec2021,tk MICRO-3226/PR.29479
 * - General updates to comply with coding standards
 *   - Braces in individual lines
 *   - Fixed switch statements.
 * 14sep2021,tk MICRO-3147/PR.29483
 * - Call CDR_Stream_has_remaining_space instead CDR_Stream_check_size, not
 *   necessary with a precondition check.
 * - Use the CDR_serialize_<type> and CDR_serialize_deserialize_<type> APIs
 *   directly since space has already been checked.
 * - Removed unsupported test (OSAPI_PLATFORM == OSAPI_PLATFORM_VXWORKS) && (CPU_FAMILY==ARM).
 *
 * 13dec2020,tk
 *     - MICRO-2710/PR.28397
 *         - Change argument name from crs to stream in function comment for
 *           CDR_Stream_serialize_property()
 * 10dec2020,tk
 *     - MICRO-2703/PR#28316
 *       - Removed CDR_Stream_serialize_2_octets_big_endian() and
 *         CDR_Stream_deserialize_2_octets_big_endian
 *     - MICRO-2712/PR#28305
 *       - Removed duplicate function header comments already in the public
 *         header-files.
 *       - Consistent use of in,out, and inout for parameter designations.
 * 09sep2020,tk  MICRO-2528/PR#28010 Renamed variables in
 *               CDR_get_max_size_serialized_primitive_sequence and
 *               CDR_get_max_size_serialized_primitive_array.
 * 30jun2015,eh  MICRO-1374/PR#15168 Rename CDR_Stream_has_free_space to
 *               CDR_Stream_has_remaining_space
 * 09jun2015,eh  MICRO-1296/PR#14964 Make non-private CDR_Stream_has_free_space 
 * 27mar2014,eh  MICRO-899: add has_free_space() to check underflow
 * 23feb2015,eh  MICRO-1075: remove and replace macros
 * 13oct2014,eh  MICRO-950: precondition check
 * 29jul2014,tk  MICRO-859/PR#10221 - Fixed Source code comment
 *               MICRO-860/PR#10222 - Check CDR options
 *               MICRO-861/PR#10224 - Removed redundant checks
 *               MICRO-862/PR#10226 - Do not realign buffer if not enough space
 * 07may2014,eh  MICRO-313/VerocelPR 1439: CDR_Stream_Align() returns void
 * 03jun2013,kaj Fix MICRO-634 (CDR_Stream_deserialize_string check for NULL)
 *               Fix MICRO-635 (NULL string check in generated code moved here)
 * 24mar2012,kaj  Written
 */
/*ce @ingroup CDRModule
 * \file
 * \brief CDR serialize API 
 *  
 * \details 
 * Serialization and deserialization operations for CDR types 
 */
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef osapi_string_h
#include "osapi/osapi_string.h"
#endif
#ifndef reda_string_h
#include "reda/reda_string.h"
#endif
#ifndef reda_sequence_h
#include "reda/reda_sequence.h"
#endif
#ifndef cdr_cdr_type_h
#include "cdr/cdr_cdr_type.h"
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

RTI_BOOL
CDR_Stream_has_remaining_space(struct CDR_Stream_t *cdrs,
                          RTI_UINT32 needed_space)
{
    RTI_UINT32 remaining_space;

    remaining_space = cdrs->length - (RTI_UINT32)(cdrs->buff_ptr - cdrs->buffer);

    /* length is unsigned, need to check for underflow from subtraction */
    if ((remaining_space > cdrs->length) || (remaining_space < needed_space))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_UINT8
CDR_Primitive_get_alignment(CdrPrimitiveType type)
{
    switch (type)
    {
        case CDR_CHAR_TYPE:
        case CDR_OCTET_TYPE:
        case CDR_BOOLEAN_TYPE:
            return 1;

        case CDR_SHORT_TYPE:
        case CDR_UNSIGNED_SHORT_TYPE:
            return 2;

        case CDR_WCHAR_TYPE:
        case CDR_LONG_TYPE:
        case CDR_UNSIGNED_LONG_TYPE:
        case CDR_FLOAT_TYPE:
        case CDR_ENUM_TYPE:
            return 4;

        case CDR_LONG_LONG_TYPE:
        case CDR_UNSIGNED_LONG_LONG_TYPE:
        case CDR_DOUBLE_TYPE:
        case CDR_LONG_DOUBLE_TYPE:
            return 8;

        default:
            return 0;
    }
}

RTI_UINT32 
CDR_Primitive_get_size(CdrPrimitiveType type)
{
    switch (type)
    {
        case CDR_CHAR_TYPE:
        case CDR_OCTET_TYPE:
        case CDR_BOOLEAN_TYPE:
            return 1U;

        case CDR_SHORT_TYPE:
        case CDR_UNSIGNED_SHORT_TYPE:
            return 2U;

        case CDR_WCHAR_TYPE:
        case CDR_LONG_TYPE:
        case CDR_UNSIGNED_LONG_TYPE:
        case CDR_FLOAT_TYPE:
        case CDR_ENUM_TYPE:
            return 4U;

        case CDR_LONG_LONG_TYPE:
        case CDR_UNSIGNED_LONG_LONG_TYPE:
        case CDR_DOUBLE_TYPE:
            return 8U;

        case CDR_LONG_DOUBLE_TYPE:
            return 16U;

        default:
            return 0U;
    }
}

void
CDR_serialize_unsigned_short(char **dest_buffer,
                            const RTI_UINT16 *instance,
                            RTI_BOOL byte_swap)
{
    if (byte_swap) 
    {
        *((*dest_buffer)++) = *((char*)instance + 1);
        *((*dest_buffer)++) = *((char*)instance    );
    } 
    else 
    {
        *((*dest_buffer)++) = *((char*)instance );
        *((*dest_buffer)++) = *((char*)instance +1);
    }
}

void
CDR_deserialize_unsigned_short(char **src_buffer,
                              RTI_UINT16 *instance,
                              RTI_BOOL byte_swap)
{
    if (byte_swap)
    {
        *((char*)instance + 1) = *((*src_buffer)++);
        *((char*)instance    ) = *((*src_buffer)++);
    } 
    else 
    {
        *((char*)instance) = *((*src_buffer)++);
        *((char*)instance + 1) = *((*src_buffer)++);
    }
}

void
CDR_deserialize_unsigned_short_from_big_endian(char **src_buffer,
                                               RTI_UINT16 *instance)
{
    OSAPI_PRECONDITION(src_buffer == NULL || instance == NULL,
               return,
               OSAPI_Log_entry_add_pointer("src_buffer",src_buffer,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("instance",instance,RTI_TRUE);)

#ifdef RTI_ENDIAN_LITTLE
    CDR_deserialize_unsigned_short(src_buffer, instance, RTI_TRUE);
#else
    CDR_deserialize_unsigned_short(src_buffer, instance, RTI_FALSE);
#endif
}

void
CDR_serialize_unsigned_long(char **dest_buffer,
                           const RTI_UINT32 *instance,
                           RTI_BOOL byte_swap)
{
    RTI_INT32 i;
    if (byte_swap)
    {
        for (i = 3; i >= 0; --i)
        {
            *((*dest_buffer)++) = *((char*)instance + i);
        }
    } 
    else 
    {
        *((*dest_buffer)++) = *((char*)instance );
        *((*dest_buffer)++) = *((char*)instance + 1);
        *((*dest_buffer)++) = *((char*)instance + 2);
        *((*dest_buffer)++) = *((char*)instance + 3);
    }
}

void
CDR_deserialize_unsigned_long(char **src_buffer,
                             RTI_UINT32 *instance,
                             RTI_BOOL byte_swap)
{
    RTI_INT32 i;
    if (byte_swap) 
    {
        for (i = 3; i >= 0; --i)
        {
            *((char*)instance + i) = *((*src_buffer)++);
        }                            
    } 
    else 
    {
        *((char*)instance ) = *((*src_buffer)++);
        *((char*)instance + 1) = *((*src_buffer)++);
        *((char*)instance + 2) = *((*src_buffer)++);
        *((char*)instance + 3) = *((*src_buffer)++);
    }
}

void
CDR_serialize_unsigned_long_to_big_endian(char **dest_buffer,
                                      const RTI_UINT32 *instance)
{
    OSAPI_PRECONDITION(dest_buffer == NULL || instance == NULL,
               return,
               OSAPI_Log_entry_add_pointer("dest_buffer",dest_buffer,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("instance",instance,RTI_TRUE);)

#ifdef RTI_ENDIAN_LITTLE
    CDR_serialize_unsigned_long(dest_buffer, instance, RTI_TRUE);
#else
    CDR_serialize_unsigned_long(dest_buffer, instance, RTI_FALSE);
#endif
}

void
CDR_deserialize_unsigned_long_from_big_endian(char **src_buffer,
                                          RTI_UINT32 *instance)
{
    OSAPI_PRECONDITION(src_buffer == NULL || instance == NULL,
                           return,
               OSAPI_Log_entry_add_pointer("src_buffer",src_buffer,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("instance",instance,RTI_TRUE);)

#ifdef RTI_ENDIAN_LITTLE
    CDR_deserialize_unsigned_long(src_buffer, instance, RTI_TRUE);
#else
    CDR_deserialize_unsigned_long(src_buffer, instance, RTI_FALSE);
#endif
}

void
CDR_serialize_unsigned_long_long(char **dest_buffer,
                               const RTI_UINT64 *instance,
                               RTI_BOOL byte_swap)
{
    RTI_INT32 i; 
    if (byte_swap)
    { 
        for (i = 7; i >= 0; --i) 
        { 
            *((*dest_buffer)++) = *((char*)instance + i);
        } 
    } 
    else 
    {                                            
        for (i = 0; i < 8; i++)
        {
            *((*dest_buffer)++) = *((char*)instance + i);
        }
    }
}

void
CDR_deserialize_unsigned_long_long(char **src_buffer,
                                 RTI_UINT64 *instance,
                                 RTI_BOOL byte_swap)
{
    RTI_INT32 i; 
    if (byte_swap) 
    {
        for (i = 7; i >= 0; --i) 
        { 
            *((char*)instance + i) = *((*src_buffer)++);
        } 
    } 
    else 
    {
        for (i = 0; i < 8; i++)
        {
            *((char*)instance + i) = *((*src_buffer)++);
        }
    }
}

void
CDR_deserialize_unsigned_long_long_from_big_endian(char **src_buffer,
                                                   RTI_UINT64 *instance)
{
    OSAPI_PRECONDITION(src_buffer == NULL || instance == NULL,
                           return,
               OSAPI_Log_entry_add_pointer("src_buffer",src_buffer,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("instance",instance,RTI_TRUE);)

#ifdef RTI_ENDIAN_LITTLE
    CDR_deserialize_unsigned_long_long(src_buffer, instance, RTI_TRUE);
#else
    CDR_deserialize_unsigned_long_long(src_buffer, instance, RTI_FALSE);
#endif
}

void
CDR_serialize_unsigned_long_long_to_big_endian(char **dest_buffer,
                                               const RTI_UINT64 *instance)
{
    OSAPI_PRECONDITION(dest_buffer == NULL || instance == NULL,
               return,
               OSAPI_Log_entry_add_pointer("dest_buffer",dest_buffer,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("instance",instance,RTI_TRUE);)

#ifdef RTI_ENDIAN_LITTLE
    CDR_serialize_unsigned_long_long(dest_buffer, instance, RTI_TRUE);
#else
    CDR_serialize_unsigned_long_long(dest_buffer, instance, RTI_FALSE);
#endif
}

void
CDR_serialize_long_double(char **dest_buffer,
                         const RTI_DOUBLE128 *instance,
                         RTI_BOOL byte_swap)
{
    RTI_INT32 i; 
    if (byte_swap) 
    { 
        for (i = 15; i >= 0; --i) 
        { 
            *((*dest_buffer)++) = *((char*)instance + i);
        } 
    } 
    else 
    {                                            
        for (i = 0; i < 16; i++)
        {
            *((*dest_buffer)++) = *((char*)instance + i);
        }
    }
}

void
CDR_deserialize_long_double(char **src_buffer,
                           RTI_DOUBLE128 *instance,
                           RTI_BOOL byte_swap)
{
    RTI_INT32 i; 
    if (byte_swap) 
    {
        for (i = 15; i >= 0; --i) 
        { 
            *((char*)instance + i) = *((*src_buffer)++);
        } 
    } 
    else 
    {
        for (i = 0; i < 16; i++)
        {
            *((char*)instance + i) = *((*src_buffer)++);
        }
    }
}

RTI_UINT32
CDR_get_max_size_serialized_primitive_array(RTI_UINT32 current_pos,
                                            RTI_UINT32 length, 
                                            CdrPrimitiveType type)
{
    RTI_UINT8 type_size;

    if (length == 0)
    {
        return 0;
    }

    type_size = (RTI_UINT8)CDR_Primitive_get_size(type);

    return CDR_get_pad_size(current_pos, type_size) + (type_size * length);
}

RTI_UINT32
CDR_get_max_size_serialized_primitive_sequence(RTI_UINT32 current_pos,
                                               RTI_UINT32 length,
                                               CdrPrimitiveType type)
{
    RTI_UINT32 current_aligned_pos = current_pos +
        CDR_get_max_size_serialized_unsigned_long(current_pos);

    return CDR_get_max_size_serialized_unsigned_long(current_pos) +
        CDR_get_max_size_serialized_primitive_array(current_aligned_pos,
                                                    length, type);
}

RTI_BOOL
CDR_Stream_serialize_char(struct CDR_Stream_t *cdrs, const RTI_INT8 *instance)
{
    OSAPI_PRECONDITION(cdrs == NULL || instance == NULL,
                    return RTI_FALSE,
                    OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("instance",instance,RTI_TRUE);)

    /* check to make sure the CDR stream has enough space for the data type */
    if (!CDR_Stream_has_remaining_space(cdrs, CDR_OCTET_SIZE))
    {
        return RTI_FALSE;
    }

    CDR_Stream_serialize_1_byte(cdrs, instance);

    return RTI_TRUE;
}

RTI_BOOL
CDR_Stream_deserialize_char(struct CDR_Stream_t * cdrs, RTI_INT8 *instance)
{
    OSAPI_PRECONDITION(cdrs == NULL || instance == NULL,
                    return RTI_FALSE,
                    OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("instance",instance,RTI_TRUE);)

    /* check to make sure the CDR stream has enough space for the data type */
    if (!CDR_Stream_has_remaining_space(cdrs, CDR_OCTET_SIZE))
    {
        return RTI_FALSE;
    }

    CDR_Stream_deserialize_1_byte(cdrs, instance);
    return RTI_TRUE;
}

/*ci 
 * \brief 
 * Internal function to serialize an unsigned short with a CDR stream
 *  
 * \param[in] cdrs Serialization stream 
 * \param[in] instance Value to serialize 
 * \param[in] do_byte_swap Flag whether toe byte swap when serializing 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure    
 */
RTI_PRIVATE RTI_BOOL
CDR_Stream_serialize_unsigned_short_I(struct CDR_Stream_t *cdrs,
                                   const RTI_UINT16 *instance,
                                   RTI_BOOL do_byte_swap)
{
    CDR_Stream_align(cdrs, CDR_SHORT_ALIGN);

    /* check to make sure the CDR stream has enough space for the data type */
    if (!CDR_Stream_has_remaining_space(cdrs, CDR_UNSIGNED_SHORT_SIZE))
    {
        return RTI_FALSE;
    }

    CDR_serialize_unsigned_short(&cdrs->buff_ptr, instance, do_byte_swap);

    return RTI_TRUE;
}

RTI_BOOL
CDR_Stream_serialize_unsigned_short(struct CDR_Stream_t *cdrs,
                                   const RTI_UINT16 *instance)
{
    OSAPI_PRECONDITION(cdrs == NULL || instance == NULL,
                    return RTI_FALSE,
                    OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("instance",instance,RTI_TRUE);)

    return CDR_Stream_serialize_unsigned_short_I(cdrs, instance, cdrs->need_byte_swap);
}

RTI_BOOL
CDR_Stream_serialize_unsigned_short_to_big_endian(struct CDR_Stream_t *me, 
                                              const RTI_UINT16 *in) 
{
    OSAPI_PRECONDITION(me == NULL || in == NULL,
                           return RTI_FALSE,
                           OSAPI_Log_entry_add_pointer("me",me,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("in",in,RTI_TRUE);)

#ifdef RTI_ENDIAN_LITTLE
    /* LE: always swap */
    return CDR_Stream_serialize_unsigned_short_I(me, in, RTI_TRUE);
#else
    /* BE: never swap */
    return CDR_Stream_serialize_unsigned_short_I(me, in, RTI_FALSE);
#endif
}


/*ci 
 * \brief 
 * Deserialize an unsigned short with a stream 
 *  
 * \param[in] cdrs Deserialization stream 
 * \param[out] instance Deserialized instance 
 * \param[in] do_byte_swap Whether to byte swap on deserialization 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
RTI_PRIVATE RTI_BOOL
CDR_Stream_deserialize_unsigned_short_I(struct CDR_Stream_t *cdrs,
                                      RTI_UINT16 *instance,
                                      RTI_BOOL do_byte_swap)
{
    CDR_Stream_align(cdrs, CDR_SHORT_ALIGN);

    /* check to make sure the CDR stream has enough space for the data type */
    if (!CDR_Stream_has_remaining_space(cdrs, CDR_UNSIGNED_SHORT_SIZE))
    {
        return RTI_FALSE;
    }

    CDR_deserialize_unsigned_short(&cdrs->buff_ptr, instance, do_byte_swap);

    return RTI_TRUE;
}

RTI_BOOL
CDR_Stream_deserialize_unsigned_short(struct CDR_Stream_t *cdrs,
                                      RTI_UINT16 *instance)
{
    OSAPI_PRECONDITION(cdrs == NULL || instance == NULL,
                    return RTI_FALSE,
                    OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("instance",instance,RTI_TRUE);)

    return CDR_Stream_deserialize_unsigned_short_I(cdrs, instance, cdrs->need_byte_swap);
}

RTI_BOOL
CDR_Stream_deserialize_unsigned_short_from_big_endian(struct CDR_Stream_t *cdrs,
                                                  RTI_UINT16 *instance) 
{
    OSAPI_PRECONDITION(cdrs == NULL || instance == NULL,
                   return RTI_FALSE,
                   OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                   OSAPI_Log_entry_add_pointer("instance",instance,RTI_TRUE);)
#ifdef RTI_ENDIAN_LITTLE
    /* LE: always swap */
    return CDR_Stream_deserialize_unsigned_short_I(cdrs, instance, RTI_TRUE);
#else
    /* BE: always swap */
    return CDR_Stream_deserialize_unsigned_short_I(cdrs, instance, RTI_FALSE);
#endif
}

/*ci 
 * \brief 
 * Internal function to serialize an unsigned long with a CDR stream
 *  
 * \param[in] cdrs Serialization stream 
 * \param[in] instance Value to serialize 
 * \param[in] do_byte_swap Flag whether toe byte swap when serializing 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure    
 */
RTI_PRIVATE RTI_BOOL
CDR_Stream_serialize_unsigned_long_I(struct CDR_Stream_t *cdrs,
                                   const RTI_UINT32 *instance,
                                   RTI_BOOL do_byte_swap)
{
    CDR_Stream_align(cdrs, CDR_LONG_ALIGN);

    /* check to make sure the CDR stream has enough space for the data type */
    if (!CDR_Stream_has_remaining_space(cdrs, CDR_UNSIGNED_LONG_SIZE))
    {
        return RTI_FALSE;
    }

    CDR_serialize_unsigned_long(&cdrs->buff_ptr, instance, do_byte_swap);

    return RTI_TRUE;
}

RTI_BOOL
CDR_Stream_serialize_unsigned_long(struct CDR_Stream_t *cdrs,
                                  const RTI_UINT32 *instance)
{
    OSAPI_PRECONDITION(cdrs == NULL || instance == NULL,
                    return RTI_FALSE,
                    OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("instance",instance,RTI_TRUE);)

    return CDR_Stream_serialize_unsigned_long_I(cdrs,instance,cdrs->need_byte_swap);
}

RTI_BOOL
CDR_Stream_serialize_unsigned_long_to_big_endian(struct CDR_Stream_t *cdrs, 
                                              const RTI_UINT32 *instance) 
{
    OSAPI_PRECONDITION(cdrs == NULL || instance == NULL,
                    return RTI_FALSE,
                    OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("instance",instance,RTI_TRUE);)

#ifdef RTI_ENDIAN_LITTLE
    /* LE always byteswaps to BE */
    return CDR_Stream_serialize_unsigned_long_I(cdrs, instance, RTI_TRUE);
#else
     /* BE never byteswaps to BE */
    return CDR_Stream_serialize_unsigned_long_I(cdrs, instance, RTI_FALSE);
#endif
}


/*ci 
 * \brief 
 * Internal function to deserialize an unsigned long with a stream 
 *  
 * \param[in] cdrs Deserialization buffer 
 * \param[out] instance Deserialized instance 
 * \param[in] do_byte_swap Flag whether to byte swap when deserializing 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
RTI_PRIVATE RTI_BOOL
CDR_Stream_deserialize_unsigned_long_I(struct CDR_Stream_t *cdrs,
                                       RTI_UINT32 *instance,
                                       RTI_BOOL do_byte_swap)
{
    CDR_Stream_align(cdrs, CDR_LONG_ALIGN);

    /* check to make sure the CDR stream has enough space for the data type */
    /* cdrs->buff_ptr guaranteed to be >= cdrs->buffer.
     *
     */
    if (!CDR_Stream_has_remaining_space(cdrs, CDR_UNSIGNED_LONG_SIZE))
    {
        return RTI_FALSE;
    }

    CDR_deserialize_unsigned_long(&cdrs->buff_ptr, instance, do_byte_swap);

    return RTI_TRUE;
}

RTI_BOOL
CDR_Stream_deserialize_unsigned_long(struct CDR_Stream_t *cdrs,
                                    RTI_UINT32 *instance)
{
    OSAPI_PRECONDITION(cdrs == NULL || instance == NULL,
                    return RTI_FALSE,
                    OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("instance",instance,RTI_TRUE);)

    return CDR_Stream_deserialize_unsigned_long_I(cdrs, instance,
                                                  cdrs->need_byte_swap);
}

RTI_BOOL
CDR_Stream_deserialize_unsigned_long_from_big_endian(struct CDR_Stream_t *cdrs,
                                                  RTI_UINT32 *instance) 
{
    OSAPI_PRECONDITION(cdrs == NULL || instance == NULL,
                    return RTI_FALSE,
                    OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("instance",instance,RTI_TRUE);)

#ifdef RTI_ENDIAN_LITTLE
    /* LE always byteswaps from BE */
    return CDR_Stream_deserialize_unsigned_long_I(cdrs, instance, RTI_TRUE);
#else
    /* BE never byteswaps from BE */
    return CDR_Stream_deserialize_unsigned_long_I(cdrs, instance, RTI_FALSE);
#endif                                                  
}

RTI_BOOL
CDR_Stream_serialize_unsigned_long_long(struct CDR_Stream_t *cdrs,
                                      const RTI_UINT64 *instance)
{
    OSAPI_PRECONDITION(cdrs == NULL || instance == NULL,
                    return RTI_FALSE,
                    OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("instance",instance,RTI_TRUE);)

    CDR_Stream_align(cdrs, CDR_LONG_LONG_ALIGN);

    /* check to make sure the CDR stream has enough space for the data type */
    if (!CDR_Stream_has_remaining_space(cdrs, CDR_UNSIGNED_LONG_LONG_SIZE))
    {
        return RTI_FALSE;
    }

    CDR_serialize_unsigned_long_long(&cdrs->buff_ptr, instance, cdrs->need_byte_swap);

    return RTI_TRUE;
}

RTI_BOOL
CDR_Stream_deserialize_unsigned_long_long(struct CDR_Stream_t *cdrs,
                                        RTI_UINT64 *instance)
{
    OSAPI_PRECONDITION(cdrs == NULL || instance == NULL,
                    return RTI_FALSE,
                    OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("instance",instance,RTI_TRUE);)

    CDR_Stream_align(cdrs, CDR_LONG_LONG_ALIGN);

    /* check to make sure the CDR stream has enough space for the data type */
    if (!CDR_Stream_has_remaining_space(cdrs, CDR_UNSIGNED_LONG_LONG_SIZE))
    {
        return RTI_FALSE;
    }

    CDR_deserialize_unsigned_long_long(&cdrs->buff_ptr, instance, cdrs->need_byte_swap);

    return RTI_TRUE;
}

RTI_BOOL
CDR_Stream_serialize_long_double(struct CDR_Stream_t * cdrs,
                                const RTI_DOUBLE128 * instance)
{
    OSAPI_PRECONDITION(cdrs == NULL || instance == NULL,
                    return RTI_FALSE,
                    OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("instance",instance,RTI_TRUE);)

    CDR_Stream_align(cdrs, CDR_LONG_DOUBLE_ALIGN);

    /* check to make sure the CDR stream has enough space for the data type */
    if (!CDR_Stream_has_remaining_space(cdrs, CDR_LONG_DOUBLE_SIZE))
    {
        return RTI_FALSE;
    }

    CDR_serialize_long_double(&cdrs->buff_ptr, instance, cdrs->need_byte_swap);

    return RTI_TRUE;
}

RTI_BOOL
CDR_Stream_deserialize_long_double(struct CDR_Stream_t * cdrs,
                                  RTI_DOUBLE128 *instance)
{
    OSAPI_PRECONDITION(cdrs == NULL || instance == NULL,
                    return RTI_FALSE,
                    OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("instance",instance,RTI_TRUE);)

    CDR_Stream_align(cdrs, CDR_LONG_DOUBLE_ALIGN);

    /* check to make sure the CDR stream has enough space for the data type */
    if (!CDR_Stream_has_remaining_space(cdrs, CDR_LONG_DOUBLE_SIZE))
    {
        return RTI_FALSE;
    }

    CDR_deserialize_long_double(&cdrs->buff_ptr, instance, cdrs->need_byte_swap);

    return RTI_TRUE;
}

RTI_BOOL
CDR_Stream_serialize_string(struct CDR_Stream_t * cdrs,
                            const char *in,
                            RTI_UINT32 max_length)
{
    RTI_UINT32 length = 0;

    OSAPI_PRECONDITION(cdrs == NULL,
                           return RTI_FALSE,
                           OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_TRUE);)

    /* verify string pointer - valid instance might still contain NULL ptr */
    if (in == NULL)
    {
        return RTI_FALSE;
    }

    /* string length must include the terminating NUL character  */
    length = REDA_String_length(in) + 1;

    /* max_length is EXCLUSIVE of terminating NUL character */
    if (length > max_length + 1)
    {
        return RTI_FALSE;
    }

    if (!CDR_Stream_serialize_unsigned_long(cdrs, &length))
    {
        return RTI_FALSE;
    }

    /* check to make sure the CDR stream has enough space for the correct
     * length of string
     */
    if (!CDR_Stream_has_remaining_space(cdrs, length))
    {
        return RTI_FALSE;
    }

    /* copy the string to stream buffer and increment stream location */
    OSAPI_Memory_copy(cdrs->buff_ptr, in, length);

    cdrs->buff_ptr += length;

    return RTI_TRUE;
}

RTI_BOOL
CDR_Stream_deserialize_string(struct CDR_Stream_t *cdrs,
                              char *out,
                              RTI_UINT32 max_length)
{
    RTI_UINT32 length;

    OSAPI_PRECONDITION(cdrs == NULL || max_length == 0,
                        return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                        OSAPI_Log_entry_add_uint("max",max_length,RTI_TRUE);)

#if !OSAPI_ENABLE_PRECONDITION
    /* verify string pointer - valid instance might still contain NULL ptr */
    if (out == NULL)
    {
        return RTI_FALSE;
    }
#endif

    /* get the string length */
    if (!CDR_Stream_deserialize_unsigned_long(cdrs, &length))
    {
        return RTI_FALSE;
    }

    /* max_length is EXCLUSIVE of terminating NUL character */
    if (length > max_length + 1)
    {
        return RTI_FALSE;
    }

    if (length == 0)
    {
        *out = '\0';
        return RTI_TRUE;
    }

    /* check to make sure the CDR stream has enough space for the correct
     * length of string
     */
    if (!CDR_Stream_has_remaining_space(cdrs, length))
    {
        return RTI_FALSE;
    }

    /* copy the string to user buffer and increment stream location */
    OSAPI_Memory_copy(out, cdrs->buff_ptr, length);

    cdrs->buff_ptr += length;

    return RTI_TRUE;
}

RTI_BOOL
CDR_Stream_serialize_wstring(struct CDR_Stream_t * cdrs,
                             const RTI_UINT32 *in,
                             RTI_UINT32 max_length)
{
    RTI_UINT32 length;

    OSAPI_PRECONDITION(cdrs == NULL || in == NULL,
                            return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("in",in,RTI_TRUE);)

    length = CDR_Wstring_length(in) + 1;

    /* maximum length is EXCLUSIVE of terminating NUL character */
    if ((max_length < 1) || (length > max_length + 1))
    {
        return RTI_FALSE;    
    }

    if (!CDR_Stream_serialize_unsigned_long(cdrs, &length))
    {
        return RTI_FALSE;    
    }

    return CDR_Stream_serialize_primitive_array(cdrs, (void *)in, length,
                                               CDR_WCHAR_TYPE);
}

RTI_BOOL
CDR_Stream_deserialize_wstring(struct CDR_Stream_t * cdrs,
                               RTI_UINT32 *out,
                               RTI_UINT32 max_length)
{
    RTI_UINT32 length;

    OSAPI_PRECONDITION(cdrs == NULL || out == NULL || max_length == 0,
                        return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("out",out,RTI_FALSE);
                        OSAPI_Log_entry_add_uint("max",max_length,RTI_TRUE);)

    if (!CDR_Stream_deserialize_unsigned_long(cdrs, &length))
    {
        return RTI_FALSE;
    }

    /* maximum length is EXCLUSIVE of terminating NUL character */
    if (length > max_length + 1)
    {
        return RTI_FALSE;
    }

    if (length == 0)
    {
        return RTI_TRUE;
    }

    return CDR_Stream_deserialize_primitive_array(cdrs, (void *)out, length,
            CDR_WCHAR_TYPE);
}

#if CDR_VARIABLE_ENUM_ENABLED

MUST_CHECK_RETURN RTI_BOOL
CDR_Stream_serialize_enum_value(
        struct CDR_Stream_t *cdrs,
        RTI_UINT32 instance)
{
    return CDR_Stream_serialize_unsigned_long(cdrs, &instance);
}

MUST_CHECK_RETURN RTI_BOOL
CDR_Stream_serialize_enum(
        struct CDR_Stream_t *cdrs,
        void *instance,
        RTI_UINT32 size)
{
    RTI_UINT32 wireValue;

    switch (size)
    {
        case ENUM_1BYTE:
            wireValue = *(RTI_UINT8 *)instance;
            break;
        case ENUM_2BYTE:
            wireValue = *(RTI_UINT16 *)instance;
            break;
        case ENUM_4BYTE:
            wireValue = *(RTI_UINT32 *)instance;
            break;
        default:
            CDR_LOG_INVALID_ENUM(OSAPI_LOGKIND_ERROR,size);
            return RTI_FALSE;
    }
    return CDR_Stream_serialize_unsigned_long(cdrs, &wireValue);
}

MUST_CHECK_RETURN RTI_BOOL
CDR_Stream_deserialize_enum(
        struct CDR_Stream_t *cdrs,
        void *instance,
        RTI_UINT32 size)
{
    RTI_UINT32 wireValue;
    
    if(!CDR_Stream_deserialize_unsigned_long(cdrs, &wireValue))
    {
        return RTI_FALSE;
    }

    switch (size)
    {
        case ENUM_1BYTE:
            if (wireValue > 0xff)
            {
                return RTI_FALSE;
            }
            *(RTI_UINT8 *)instance = (RTI_UINT8)wireValue;
            break;
        case ENUM_2BYTE:
            if (wireValue > 0xffff)
            {
                return RTI_FALSE;
            }
            *(RTI_UINT16 *)instance = (RTI_UINT16)wireValue;
            break;
        case ENUM_4BYTE:
            *(RTI_UINT32 *)instance = (RTI_UINT32)wireValue;
            break;
        default:
            CDR_LOG_INVALID_ENUM(OSAPI_LOGKIND_ERROR,size);
            return RTI_FALSE;
    }
    return RTI_TRUE;
}
#endif

RTI_BOOL
CDR_Stream_serialize_byte_array(struct CDR_Stream_t *cdrs,
                               const unsigned char *in_array,
                               RTI_UINT32 length)
{
    OSAPI_PRECONDITION(cdrs == NULL || in_array == NULL,
                        return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("inArray",in_array,RTI_TRUE);)

    if (length == 0)
    {
        return RTI_TRUE;
    }

    /* check to make sure the CDR stream has enough space for
     * the correct length of string
     */
    if (!CDR_Stream_has_remaining_space(cdrs, length))
    {
        return RTI_FALSE;
    }

    /* copy the array to stream buffer and increment stream location */
    OSAPI_Memory_copy(cdrs->buff_ptr, in_array, length);
    cdrs->buff_ptr += length;

    return RTI_TRUE;
}

RTI_BOOL
CDR_Stream_deserialize_byte_array(struct CDR_Stream_t * cdrs,
                                 unsigned char *out,
                                 RTI_UINT32 length)
{
    OSAPI_PRECONDITION(cdrs == NULL || out == NULL,
                            return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("out",out,RTI_TRUE);)

    if (length == 0)
    {
        return RTI_TRUE;
    }

    /* check to make sure the CDR stream has enough space for the
     * correct length of string
     */
    if (!CDR_Stream_has_remaining_space(cdrs, length))
    {
        return RTI_FALSE;
    }

    /* copy the string to user buffer and increment stream location */
    OSAPI_Memory_copy(out, cdrs->buff_ptr, length);
    cdrs->buff_ptr += length;

    return RTI_TRUE;
}

RTI_BOOL
CDR_Stream_serialize_string_array(struct CDR_Stream_t *cdrs,
                                 const void* in, 
                                 RTI_UINT32 length,
                                 RTI_UINT32 max_str_len,
                                 CdrPrimitiveType type)
{
    RTI_UINT32 i;

    OSAPI_PRECONDITION(cdrs == NULL || in == NULL || max_str_len == 0,
                        return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("in",in,RTI_FALSE);
                        OSAPI_Log_entry_add_uint("max",max_str_len,RTI_TRUE);)

    /* maximum length is EXCLUSIVE of terminating NUL character */
    if (type == CDR_CHAR_TYPE)
    {
        for (i=0; i<length ;i++)
        {
            if (!CDR_Stream_serialize_string(cdrs, ((char **)in)[i], max_str_len))
            {
                return RTI_FALSE;
            }
        }
    }
    else if (type == CDR_WCHAR_TYPE)
    {
        for (i=0; i<length; i++)
        {
            if (!CDR_Stream_serialize_wstring(cdrs, ((RTI_UINT32 **)in)[i],
                                              max_str_len))
            {
                return RTI_FALSE;
            }
        }
    }
    else
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
CDR_Stream_deserialize_string_array(struct CDR_Stream_t *cdrs,
                                   void *out,
                                   RTI_UINT32 length,
                                   RTI_UINT32 max_str_len,
                                   CdrPrimitiveType type)
{
    RTI_UINT32 i;

    OSAPI_PRECONDITION(cdrs == NULL || out == NULL || max_str_len == 0,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("out",out,RTI_FALSE);
                       OSAPI_Log_entry_add_uint("max",max_str_len,RTI_TRUE);)

    /* maximum length is EXCLUSIVE of terminating NUL character */
    if (type == CDR_CHAR_TYPE)
    {
        for (i=0; i<length; i++)
        {
            if (!CDR_Stream_deserialize_string(cdrs, ((char **)out)[i],
                                               max_str_len))
            {
                return RTI_FALSE;
            }            
        }     
    }
    else if (type == CDR_WCHAR_TYPE)
    {
        for (i=0; i<length; i++)
        {
            if (!CDR_Stream_deserialize_wstring(cdrs, ((RTI_UINT32 **)out)[i],
                                                max_str_len))
            {
                return RTI_FALSE;
            }
        }
    }
    else
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;      
}

RTI_UINT32
CDR_get_max_size_serialized_string_array(RTI_UINT32 current_alignment,
                                      RTI_UINT32 length,
                                      RTI_UINT32 max_str_len,
                                      CdrPrimitiveType type)
{
    RTI_UINT32 add_size = 0;

    /* maximum string length is EXCLUSIVE of terminating NUL character so add 1 */
    if (length > 0)
    {
        if (type==CDR_WCHAR_TYPE)
        {
            add_size = CDR_get_max_size_serialized_wstring(current_alignment,(max_str_len+1));
            add_size += CDR_get_max_size_serialized_wstring(current_alignment + add_size,(max_str_len+1))*(length-1);
        }
        else
        {
            add_size = CDR_get_max_size_serialized_string(current_alignment,(max_str_len+1));
            add_size += CDR_get_max_size_serialized_string(current_alignment + add_size,(max_str_len+1))*(length-1);
        }
    }

    return add_size;      
}

RTI_BOOL
CDR_Stream_serialize_primitive_array(struct CDR_Stream_t *cdrs,
                                    const void* in,
                                    RTI_UINT32 length,
                                    CdrPrimitiveType type)
{
    RTI_UINT32 i, element_size;
    RTI_UINT16 *in_2 = NULL;
    RTI_UINT32 *in_4 = NULL;
    RTI_UINT64 *in_8 = NULL;
    RTI_DOUBLE64 *in_double = NULL;
    RTI_DOUBLE128 *in_long_double = NULL;
    RTI_UINT8 align;

    OSAPI_PRECONDITION((cdrs == NULL) || (cdrs->buff_ptr == NULL) ||
                           (in == NULL),
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("cdrs->buff_ptr",
                             (cdrs != NULL ? cdrs->buff_ptr : NULL),RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("in",in,RTI_TRUE);)

    /* no need to check for alignment or swap order for 1-byte types:
       CDR_CHAR_TYPE, CDR_OCTET_TYPE, CDR_BOOLEAN_TYPE */
    if (CDR_Primitive_get_size(type) == 1)
    {
        if (!CDR_Stream_has_remaining_space(cdrs, length))
        {
            return RTI_FALSE;
        }
        OSAPI_Memory_copy(cdrs->buff_ptr, in, length);
        cdrs->buff_ptr += length;
        return RTI_TRUE;
    }

    /* multi-byte types */
    align = CDR_Primitive_get_alignment(type);
    element_size = CDR_Primitive_get_size(type);

    /* all multi-byte types have alignment greater than one */
    CDR_Stream_align(cdrs, align);
    if (!CDR_Stream_has_remaining_space(cdrs, element_size * length))
    {
        return RTI_FALSE;
    }

    if (!cdrs->need_byte_swap)
    {
        OSAPI_Memory_copy(cdrs->buff_ptr, in, element_size * length);
        cdrs->buff_ptr += element_size * length;
    }
    else
    {
        switch (type)
        {
            case CDR_SHORT_TYPE:
            case CDR_UNSIGNED_SHORT_TYPE:
                in_2 = (RTI_UINT16*)in;
                for (i = 0; i < length; ++i)
                {
                    CDR_serialize_unsigned_short(&cdrs->buff_ptr, in_2,
                                         cdrs->need_byte_swap);
                    ++in_2;
                }
                break;
            case CDR_WCHAR_TYPE:
            case CDR_LONG_TYPE:
            case CDR_UNSIGNED_LONG_TYPE:
            case CDR_FLOAT_TYPE:
#if !CDR_VARIABLE_ENUM_ENABLED
            case CDR_ENUM_TYPE:
#endif
                in_4 = (RTI_UINT32*)in;
                for (i = 0; i < length; ++i)
                {
                    CDR_serialize_unsigned_long(&cdrs->buff_ptr,
                                        in_4,
                                        cdrs->need_byte_swap);
                    ++in_4;
                }
                break;
            case CDR_LONG_LONG_TYPE:
            case CDR_UNSIGNED_LONG_LONG_TYPE:
                in_8 = (RTI_UINT64*)in;
                for (i = 0; i < length; ++i)
                {
                    CDR_serialize_unsigned_long_long(&cdrs->buff_ptr, in_8,
                                             cdrs->need_byte_swap);
                    ++in_8;
                }
                break;
            case CDR_LONG_DOUBLE_TYPE:
                in_long_double = (RTI_DOUBLE128 *)in;
                for (i = 0; i < length; ++i)
                {
                    CDR_serialize_long_double(&cdrs->buff_ptr,
                                      in_long_double,
                                      cdrs->need_byte_swap);
                    ++in_long_double;
                }
                break;
            case CDR_DOUBLE_TYPE:
                in_double = (RTI_DOUBLE64*)in;
                for (i = 0; i < length; ++i)
                {
                    CDR_serialize_unsigned_long_long(&cdrs->buff_ptr,
                                             (RTI_UINT64*)in_double,
                                             cdrs->need_byte_swap);
                    ++in_double;
                }
                break;

            default:
                return RTI_FALSE; /* otherwise, ERROR */
        }
    }

    return RTI_TRUE;
}

RTI_BOOL
CDR_Stream_deserialize_primitive_array(struct CDR_Stream_t *cdrs,
                                      void *out,
                                      RTI_UINT32 length,
                                      CdrPrimitiveType type)
{
    RTI_UINT32 i, element_size;
    RTI_UINT16 *out_2 = NULL;
    RTI_UINT32 *out_4 = NULL;
    RTI_UINT64 *out_8 = NULL;
    RTI_DOUBLE64 *out_double = NULL;
    RTI_DOUBLE128 *out_long_double = NULL;
    RTI_UINT8 align;

    OSAPI_PRECONDITION((cdrs == NULL) || (cdrs->buff_ptr == NULL) ||
                           (out == NULL),
                           return RTI_FALSE,
                           OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("cdrs->buff_ptr",
                                 (cdrs != NULL ? cdrs->buff_ptr : NULL),RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("out",out,RTI_TRUE);)

    /* no need to check for alignment or swap order for 1-byte types:
       CDR_CHAR_TYPE, CDR_OCTET_TYPE, CDR_BOOLEAN_TYPE */
    if (CDR_Primitive_get_size(type) == 1)
    {
        if (!CDR_Stream_has_remaining_space(cdrs, length))
        {
            return RTI_FALSE;
        }
        OSAPI_Memory_copy(out, cdrs->buff_ptr, length);
        cdrs->buff_ptr += length;
        return RTI_TRUE;
    }

    /* multi-byte types */
    align = CDR_Primitive_get_alignment(type);
    element_size = CDR_Primitive_get_size(type);

    /* all multi-byte types have alignment greater than one */
    CDR_Stream_align(cdrs, align);
    if (!CDR_Stream_has_remaining_space(cdrs, element_size * length))
    {
        return RTI_FALSE;
    }

    /* if not byte swapping then can just copy memory */
    if (!cdrs->need_byte_swap)
    {
        OSAPI_Memory_copy(out, cdrs->buff_ptr, element_size*length);
        cdrs->buff_ptr += element_size * length;
    }
    else
    {
        switch (type)
        {
            case CDR_SHORT_TYPE:
            case CDR_UNSIGNED_SHORT_TYPE:
                out_2 = (RTI_UINT16 *)out;
                for (i = 0; i < length; ++i)
                {
                    CDR_deserialize_unsigned_short(&cdrs->buff_ptr, out_2,
                                                   cdrs->need_byte_swap);
                    ++out_2;
                }
                break;
            case CDR_WCHAR_TYPE:
            case CDR_LONG_TYPE:
            case CDR_UNSIGNED_LONG_TYPE:
            case CDR_FLOAT_TYPE:
#if !CDR_VARIABLE_ENUM_ENABLED
            case CDR_ENUM_TYPE:
#endif
                out_4 = (RTI_UINT32 *)out;
                for (i = 0; i < length; ++i)
                {
                    CDR_deserialize_unsigned_long(&cdrs->buff_ptr, out_4,
                                                  cdrs->need_byte_swap);
                    ++out_4;
                }
                break;
            case CDR_LONG_LONG_TYPE:
            case CDR_UNSIGNED_LONG_LONG_TYPE:
                out_8 = (RTI_UINT64 *)out;
                for (i = 0; i < length; ++i)
                {
                    CDR_deserialize_unsigned_long_long(&cdrs->buff_ptr, out_8,
                                                       cdrs->need_byte_swap);
                    ++out_8;
                }
                break;
            case CDR_LONG_DOUBLE_TYPE:
                out_long_double = (RTI_DOUBLE128 *)out;
                for (i = 0; i < length; ++i)
                {
                    CDR_deserialize_long_double(&cdrs->buff_ptr,
                                                out_long_double,
                                                cdrs->need_byte_swap);
                    ++out_long_double;
                }
                break;
            case CDR_DOUBLE_TYPE:
                out_double = (RTI_DOUBLE64 *)out;
                for (i = 0; i < length; ++i)
                {
                    CDR_deserialize_unsigned_long_long(&cdrs->buff_ptr,
                                                       (RTI_UINT64*)out_double,
                                                       cdrs->need_byte_swap);
                    ++out_double;
                }
                break;

            default:
                return RTI_FALSE; /* otherwise, ERROR */
        }
    }

    return RTI_TRUE;
}

RTI_BOOL
CDR_Stream_serialize_non_primitive_array(struct CDR_Stream_t *cdrs,
                               const void* in,
                               RTI_UINT32 length,
                               RTI_UINT32 element_size,
                               CDR_Stream_SerializeFunction serialize_function,
                               void *param)
{
    RTI_UINT32 i;
    const char *array_element;

    OSAPI_PRECONDITION(cdrs == NULL || cdrs->buff_ptr == NULL || in == NULL ||
            serialize_function == NULL,
             return RTI_FALSE,
             OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
             OSAPI_Log_entry_add_pointer("cdrs->buff_ptr",
                   (cdrs != NULL ? cdrs->buff_ptr : NULL),RTI_FALSE);
             OSAPI_Log_entry_add_pointer("in",in,RTI_FALSE);
             OSAPI_Log_entry_add_uint("size",element_size,RTI_FALSE);
             OSAPI_Log_entry_add_pointer("func",serialize_function == NULL ? NULL : (void*)0x1,RTI_TRUE);)

    array_element = (const char *)in;
    for (i = 0; i < length; ++i)
    {
        if (!serialize_function(cdrs, array_element, param))
        {
            return RTI_FALSE;
        }
        array_element += element_size;
    }
    return RTI_TRUE;
}

RTI_BOOL
CDR_Stream_deserialize_non_primitive_array(struct CDR_Stream_t *cdrs,
                                         void* out,
                                         RTI_UINT32 length,
                                         RTI_UINT32 element_size,
                                         CDR_Stream_DeserializeFunction
                                           deserialize_function,
                                         void *param)
{
    RTI_UINT32 i;
    char *array_element;

    OSAPI_PRECONDITION(cdrs == NULL || cdrs->buff_ptr == NULL ||
                          out == NULL ||
                          deserialize_function == NULL,
                 return RTI_FALSE,
                 OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                 OSAPI_Log_entry_add_pointer("cdrs->buff_ptr",
                       (cdrs != NULL ? cdrs->buff_ptr : NULL),RTI_FALSE);
                 OSAPI_Log_entry_add_pointer("out",out,RTI_FALSE);
                 OSAPI_Log_entry_add_uint("size",element_size,RTI_FALSE);
                 OSAPI_Log_entry_add_pointer("func",deserialize_function  == NULL ? NULL : (void*)0x1,RTI_TRUE);)

    array_element = (char *)out;
    for (i = 0; i < length; ++i)
    {
        if (!deserialize_function(cdrs, array_element, param))
        {
            return RTI_FALSE;
        }
        array_element += element_size;
    }
    return RTI_TRUE;
}

RTI_UINT32
CDR_get_max_size_serialized_non_primitive_array(RTI_UINT32 current_alignment,
                    RTI_UINT32 length,
                    CDR_Stream_GetSerializedSizeFunction get_serialized_size_func,
                    void *param)
{
    RTI_UINT32 i = 0, add_size = 0, samples_in_loop, loop_size, loop_count;
    RTI_INT32 si,alignment, align[8];
    RTI_UINT32 size_align[8];
    void *plugin = NULL;
    UNUSED_ARG(param);

    OSAPI_PRECONDITION(get_serialized_size_func == NULL,
           return RTI_FALSE,
           OSAPI_Log_entry_add_pointer("func",
                                       get_serialized_size_func  == NULL ?
                                               NULL : (void*)0x1,RTI_TRUE);)

    for (i = 0; i < 8; i++)
    {
        align[i] = -1;
        size_align[i] = 0;
    }

    i = 0;
    si = 0;
    alignment = (current_alignment % 8);

    while ((align[alignment] < 0) && (i < length))
    {
        align[alignment] = si;
        size_align[alignment] = add_size;
        add_size += 
            get_serialized_size_func(plugin, current_alignment + add_size, NULL);
        i++;
        si++;
        alignment = (int)((current_alignment + add_size)% 8);
    }

    if (i < length)
    {
        /* align[alignment] >= 0 */
        samples_in_loop = i - (RTI_UINT32)align[alignment];
        loop_size = add_size - size_align[alignment];
        loop_count = (length - i)/samples_in_loop;
        add_size += (loop_size* loop_count);
        i += samples_in_loop*loop_count;
    }

    for(;i < length; ++i)
    {
        add_size += 
            get_serialized_size_func(plugin, current_alignment + add_size, NULL);
    }
    return add_size;
}

RTI_BOOL
CDR_Stream_serialize_string_sequence(struct CDR_Stream_t *cdrs,
                                    const struct REDA_Sequence* in,
                                    RTI_UINT32 max_str_len,
                                    CdrPrimitiveType type)
{
    RTI_UINT32 length;
    RTI_INT32 i,slength;
    const char * in_str;
    const RTI_UINT32 * in_wstr;

    OSAPI_PRECONDITION((cdrs == NULL) || (cdrs->buff_ptr == NULL) ||
                           (in == NULL),
                           return RTI_FALSE,
                           OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("cdrs->buff_ptr",
                                 (cdrs != NULL ? cdrs->buff_ptr : NULL),RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("in",in,RTI_TRUE);)

    slength = REDA_Sequence_get_length(in);

    if (slength < 0)
    {
        return RTI_FALSE;
    }

    /* must have at least one element for non-zero sequence */
    if ((slength > 0) && (REDA_Sequence_get_reference(in, 0) == NULL))
    {
        return RTI_FALSE;
    }

    length = (RTI_UINT32)slength;

    /* serialize sequence length */
    if (!CDR_Stream_serialize_unsigned_long(cdrs, &length))
    {
        return RTI_FALSE;
    }

    if (length == 0)
    {
        return RTI_TRUE;
    }

    if (type == CDR_CHAR_TYPE)
    {
        for (i = 0; i < slength; i++)
        {
            in_str = *(const char **) REDA_Sequence_get_reference(in, i);
            if (!CDR_Stream_serialize_string(cdrs, in_str, max_str_len))
            {
                return RTI_FALSE;
            }
        }
    }
    else if (type == CDR_WCHAR_TYPE)
    {
        for (i = 0; i < slength; i++)
        {
            in_wstr = *(const RTI_UINT32 **) REDA_Sequence_get_reference(in, i);
            if (!CDR_Stream_serialize_wstring(cdrs, in_wstr, max_str_len))
            {
                return RTI_FALSE;
            }
        }
    }
    else
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
CDR_Stream_deserialize_string_sequence(struct CDR_Stream_t *cdrs,
                                      struct REDA_Sequence* out,
                                      RTI_UINT32 max_str_len,
                                      CdrPrimitiveType type)
{
    RTI_INT32 i, length;
    char * out_str;
    RTI_UINT32 *out_wstr;

    OSAPI_PRECONDITION((cdrs == NULL) || (cdrs->buff_ptr == NULL) ||
                           (out == NULL),
                           return RTI_FALSE,
                           OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("cdrs->buff_ptr",
                                 (cdrs != NULL ? cdrs->buff_ptr : NULL),RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("out",out,RTI_TRUE);)

    /* deserialize sequence length */
    if (!CDR_Stream_deserialize_unsigned_long(cdrs, (RTI_UINT32*)&length))
    {
        return RTI_FALSE;
    }

    if (length > REDA_Sequence_get_maximum(out))
    {
        return RTI_FALSE;
    }

    if (!REDA_Sequence_set_length(out, length))
    {
        return RTI_FALSE;
    }

    if (length == 0)
    {
        return RTI_TRUE;
    }

    if (type == CDR_CHAR_TYPE)
    {
        for (i=0; i<length; i++)
        {
            out_str = *(char **) REDA_Sequence_get_reference(out, i);
            if (!CDR_Stream_deserialize_string(cdrs, out_str, max_str_len))
            {
                return RTI_FALSE;
            }            
        }     
    }
    else if (type == CDR_WCHAR_TYPE)
    {
        for (i=0; i<length; i++)
        {
            out_wstr = *(RTI_UINT32 **) REDA_Sequence_get_reference(out, i);
            if (!CDR_Stream_deserialize_wstring(cdrs, out_wstr, max_str_len))
            {
                return RTI_FALSE;
            }
        }
    }
    else
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;      
}

RTI_UINT32
CDR_get_max_size_serialized_string_sequence(RTI_UINT32 current_alignment,
                                         RTI_UINT32 length,
                                         RTI_UINT32 max_str_len,
                                         CdrPrimitiveType type)
{
    RTI_UINT32 add_size;

    add_size = CDR_get_max_size_serialized_unsigned_long(current_alignment);

    /* maximum string length is EXCLUSIVE of terminating NUL character so add 1 */
    if (length > 0)
    {
        if (type == CDR_WCHAR_TYPE)
        {
            add_size += CDR_get_max_size_serialized_wstring(current_alignment + add_size,(max_str_len+1));
            add_size += CDR_get_max_size_serialized_wstring(current_alignment + add_size,(max_str_len+1))*(length-1);
        }
        else
        {
            add_size += CDR_get_max_size_serialized_string(current_alignment + add_size,(max_str_len+1));
            add_size += CDR_get_max_size_serialized_string(current_alignment + add_size,(max_str_len+1))*(length-1);
        }
    }

    return add_size;      
}

RTI_BOOL
CDR_Stream_serialize_primitive_sequence(struct CDR_Stream_t *cdrs,
                                       const struct REDA_Sequence* in,
                                       CdrPrimitiveType type)
{
    RTI_UINT32 element_size, length;
    RTI_INT32 i,slength;
    RTI_INT8 *in_1 = NULL;
    RTI_UINT16 *in_2 = NULL;
    RTI_UINT32 *in_4 = NULL;
    RTI_UINT64 *in_8 = NULL;
    RTI_DOUBLE64 *in_double = NULL;
    RTI_DOUBLE128 *in_long_double = NULL;
    char * buffer;
    RTI_UINT8 align;

    OSAPI_PRECONDITION((cdrs == NULL) || (cdrs->buff_ptr == NULL) ||
                           (in == NULL),
                           return RTI_FALSE,
                           OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("cdrs->buff_ptr",
                                 (cdrs != NULL ? cdrs->buff_ptr : NULL),RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("in",in,RTI_TRUE);)

    slength = REDA_Sequence_get_length(in);
    if (slength < 0)
    {
        return RTI_FALSE;
    }

    /* PrimitiveSequences always use contiguous buffer */
    buffer = (char *)REDA_Sequence_get_buffer(in);

    /* must have at least one element for non-zero sequence */
    if ((slength > 0) && (buffer == NULL))
    {
        return RTI_FALSE;
    }

    /* serialize sequence length */
    length = (RTI_UINT32)slength;
    if (!CDR_Stream_serialize_unsigned_long(cdrs, &length))
    {
        return RTI_FALSE;
    }

    if (length == 0)
    {
        return RTI_TRUE;
    }

    /* no need to check for alignment or swap order for 1-byte types:
       CDR_CHAR_TYPE, CDR_OCTET_TYPE, CDR_BOOLEAN_TYPE */
    if ((CDR_Primitive_get_size(type) == 1) &&
        !REDA_Sequence_has_discontiguous_buffer(in))
    {
        if (!CDR_Stream_has_remaining_space(cdrs, length))
        {
            return RTI_FALSE;
        }
        OSAPI_Memory_copy(cdrs->buff_ptr, buffer, length);
        cdrs->buff_ptr += length;
        return RTI_TRUE;
    }

    align = CDR_Primitive_get_alignment(type);
    element_size = CDR_Primitive_get_size(type);

    CDR_Stream_align(cdrs, align);
    if (!CDR_Stream_has_remaining_space(cdrs, element_size * length))
    {
        return RTI_FALSE;
    }

    /* if not byte swapping and contiguous buffer then can just copy memory */
    if ((!cdrs->need_byte_swap) && !REDA_Sequence_has_discontiguous_buffer(in))
    {
        OSAPI_Memory_copy(cdrs->buff_ptr, buffer, element_size * length);
        cdrs->buff_ptr += element_size * length;
    }
    else
    {
        switch (type)
        {
            case CDR_CHAR_TYPE:
            case CDR_OCTET_TYPE:
            case CDR_BOOLEAN_TYPE:
                for (i = 0; i < slength; ++i)
                {
                    in_1 = (RTI_INT8*)REDA_Sequence_get_reference(in, i);
                    CDR_Stream_serialize_1_byte(cdrs, in_1);
                }
                break;
            case CDR_SHORT_TYPE:
            case CDR_UNSIGNED_SHORT_TYPE:
                for (i = 0; i < slength; ++i)
                {
                    in_2 = (RTI_UINT16*)REDA_Sequence_get_reference(in, i);
                    CDR_serialize_unsigned_short(&cdrs->buff_ptr, in_2,
                                                 cdrs->need_byte_swap);
                }
                break;
            case CDR_WCHAR_TYPE:
            case CDR_LONG_TYPE:
            case CDR_UNSIGNED_LONG_TYPE:
            case CDR_FLOAT_TYPE:
#if !CDR_VARIABLE_ENUM_ENABLED
            case CDR_ENUM_TYPE:
#endif
                for (i = 0; i < slength; ++i)
                {
                    in_4 = (RTI_UINT32*)REDA_Sequence_get_reference(in, i);
                    CDR_serialize_unsigned_long(&cdrs->buff_ptr, in_4,
                                                cdrs->need_byte_swap);
                }
                break;
            case CDR_LONG_LONG_TYPE:
            case CDR_UNSIGNED_LONG_LONG_TYPE:
                for (i = 0; i < slength; ++i)
                {
                    in_8 = (RTI_UINT64*)REDA_Sequence_get_reference(in, i);
                    CDR_serialize_unsigned_long_long(&cdrs->buff_ptr, in_8,
                                                     cdrs->need_byte_swap);
                }
                break;
            case CDR_LONG_DOUBLE_TYPE:
                for (i = 0; i < slength; ++i)
                {
                    in_long_double = (RTI_DOUBLE128*)REDA_Sequence_get_reference(in, i);
                    CDR_serialize_long_double(&cdrs->buff_ptr,in_long_double,
                                              cdrs->need_byte_swap);
                }
                break;
            case CDR_DOUBLE_TYPE:
                for (i = 0; i < slength; ++i)
                {
                    in_double = (RTI_DOUBLE64*)REDA_Sequence_get_reference(in, i);
                    CDR_serialize_unsigned_long_long(&cdrs->buff_ptr,
                                                     (RTI_UINT64*)in_double,
                                                     cdrs->need_byte_swap);
                }
                break;
            default:
                return RTI_FALSE; /* otherwise, ERROR */
        }
    }

    return RTI_TRUE;
}

RTI_BOOL
CDR_Stream_deserialize_primitive_sequence(struct CDR_Stream_t *cdrs,
                                         struct REDA_Sequence* out,
                                         CdrPrimitiveType type)
{
    RTI_INT32 i, slength;
    RTI_UINT32 length,element_size;
    RTI_INT8 *out_1 = NULL;
    RTI_UINT16 *out_2 = NULL;
    RTI_UINT32 *out_4 = NULL;
    RTI_UINT64 *out_8 = NULL;
    RTI_DOUBLE64 *out_double = NULL;
    RTI_DOUBLE128 *out_long_double = NULL;
    char * buffer;
    RTI_UINT8 align;

    OSAPI_PRECONDITION((cdrs == NULL) || (cdrs->buff_ptr == NULL) ||
                           (out == NULL),
                           return RTI_FALSE,
                           OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("cdrs->buff_ptr",
                                 (cdrs != NULL ? cdrs->buff_ptr : NULL),RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("out",out,RTI_TRUE);)

    /* deserialize sequence length */
    if (!CDR_Stream_deserialize_unsigned_long(cdrs, &length))
    {
        return RTI_FALSE;
    }

    if (length > INT_MAX)
    {
        return RTI_FALSE;
    }

    slength = (RTI_INT32)length;
    if (slength > REDA_Sequence_get_maximum(out))
    {
        return RTI_FALSE;
    }

    if (!REDA_Sequence_set_length(out, slength))
    {
        return RTI_FALSE;
    }

    if (slength == 0)
    {
        return RTI_TRUE;
    }

    /* PrimitiveSequences always use contiguous buffer */
    buffer = (char *) REDA_Sequence_get_buffer(out);

    /* no need to check for alignment or swap order for 1-byte types:
       CDR_CHAR_TYPE, CDR_OCTET_TYPE, CDR_BOOLEAN_TYPE */
    if ((CDR_Primitive_get_size(type) == 1) &&
         !REDA_Sequence_has_discontiguous_buffer(out))
    {
        if (!CDR_Stream_has_remaining_space(cdrs, length))
        {
            return RTI_FALSE;
        }
        OSAPI_Memory_copy(buffer, cdrs->buff_ptr, length);
        cdrs->buff_ptr += length;
        return RTI_TRUE;
    }

    align = CDR_Primitive_get_alignment(type);
    element_size = CDR_Primitive_get_size(type);

    CDR_Stream_align(cdrs, align);
    if (!CDR_Stream_has_remaining_space(cdrs, element_size * length))
    {
        return RTI_FALSE;
    }

    /* if not byte swapping and contiguous buffer then can just copy memory */
    if ((!cdrs->need_byte_swap) && !REDA_Sequence_has_discontiguous_buffer(out))
    {
        OSAPI_Memory_copy(buffer, cdrs->buff_ptr, element_size*length);
        cdrs->buff_ptr += element_size * length;
    }
    else
    {
        switch (type)
        {
            case CDR_CHAR_TYPE:
            case CDR_OCTET_TYPE:
            case CDR_BOOLEAN_TYPE:
                for (i = 0; i < slength; ++i)
                {
                    out_1 = (RTI_INT8 *) REDA_Sequence_get_reference(out, i);
                    CDR_Stream_deserialize_1_byte(cdrs, out_1);
                }
                break;
            case CDR_SHORT_TYPE:
            case CDR_UNSIGNED_SHORT_TYPE:
                for (i = 0; i < slength; ++i)
                {
                    out_2 = (RTI_UINT16*)REDA_Sequence_get_reference(out, i);
                    CDR_deserialize_unsigned_short(&cdrs->buff_ptr,out_2,
                                                   cdrs->need_byte_swap);
                }
                break;
            case CDR_WCHAR_TYPE:
            case CDR_LONG_TYPE:
            case CDR_UNSIGNED_LONG_TYPE:
            case CDR_FLOAT_TYPE:
#if !CDR_VARIABLE_ENUM_ENABLED
            case CDR_ENUM_TYPE:
#endif
                for (i = 0; i < slength; ++i)
                {
                    out_4 = (RTI_UINT32*)REDA_Sequence_get_reference(out, i);
                    CDR_deserialize_unsigned_long(&cdrs->buff_ptr, out_4,
                                                  cdrs->need_byte_swap);
                }
                break;
            case CDR_LONG_LONG_TYPE:
            case CDR_UNSIGNED_LONG_LONG_TYPE:
                for (i = 0; i < slength; ++i)
                {
                    out_8 = (RTI_UINT64*)REDA_Sequence_get_reference(out, i);
                    CDR_deserialize_unsigned_long_long(&cdrs->buff_ptr, out_8,
                                                       cdrs->need_byte_swap);
                }
                break;
            case CDR_LONG_DOUBLE_TYPE:
                for (i = 0; i < slength; ++i)
                {
                    out_long_double = (RTI_DOUBLE128*)REDA_Sequence_get_reference(out, i);
                    CDR_deserialize_long_double(&cdrs->buff_ptr,
                                                out_long_double,
                                                cdrs->need_byte_swap);
                }
                break;
            case CDR_DOUBLE_TYPE:
                for (i = 0; i < slength; ++i)
                {
                    out_double = (RTI_DOUBLE64*)REDA_Sequence_get_reference(out, i);
                    CDR_deserialize_unsigned_long_long(&cdrs->buff_ptr,
                                                       (RTI_UINT64*)out_double,
                                                       cdrs->need_byte_swap);
                }
                break;
            default:

                return RTI_FALSE; /* otherwise, ERROR */
        }
    }

    return RTI_TRUE;
}

RTI_BOOL
CDR_Stream_serialize_non_primitive_sequence(struct CDR_Stream_t *cdrs,
                                          const struct REDA_Sequence* in,
                                          CDR_Stream_SerializeFunction
                                            serialize_function,
                                          void * param)
{
    RTI_INT32 i, slength;
    RTI_UINT32 length;
    void * buffer;

    OSAPI_PRECONDITION((cdrs == NULL) || (cdrs->buff_ptr == NULL) ||
                           (serialize_function == NULL),
                   return RTI_FALSE,
                   OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                   OSAPI_Log_entry_add_pointer("cdrs->buff_ptr",
                           (cdrs != NULL ? cdrs->buff_ptr : NULL),RTI_FALSE);
                   OSAPI_Log_entry_add_pointer("serialize_function",
                           serialize_function == NULL ? NULL : (void*)0x1,RTI_TRUE);)


    slength = REDA_Sequence_get_length(in);
    if (slength < 0)
    {
        return RTI_FALSE;
    }

    length = (RTI_UINT32)slength;
    if (!CDR_Stream_serialize_unsigned_long(cdrs, &length))
    {
        return RTI_FALSE;
    }

    for (i = 0; i < slength; ++i)
    {
        buffer = REDA_Sequence_get_reference(in, i);
        if (buffer == NULL)
        {
            return RTI_FALSE;
        }
        if (!serialize_function(cdrs, buffer, param))
        {
            return RTI_FALSE;
        }
    }

    return RTI_TRUE;
}

RTI_BOOL
CDR_Stream_deserialize_non_primitive_sequence(struct CDR_Stream_t *cdrs,
                                            struct REDA_Sequence* out,
                                            CDR_Stream_DeserializeFunction
                                              deserialize_function,
                                            void * param)
{
    RTI_UINT32 length;
    RTI_INT32 i,slength;
    void *buffer;

    OSAPI_PRECONDITION((cdrs == NULL) || (cdrs->buff_ptr == NULL) ||
                           (deserialize_function == NULL) || (out == NULL),
                           return RTI_FALSE,
                           OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("cdrs->buff_ptr",
                                   (cdrs != NULL ? cdrs->buff_ptr : NULL),RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("deserialize_function",
                                   deserialize_function  == NULL ? NULL : (void*)0x1,RTI_TRUE);)

    if (!CDR_Stream_deserialize_unsigned_long(cdrs, &length))
    {
        return RTI_FALSE;
    }

    if (length > INT_MAX)
    {
        return RTI_FALSE;
    }

    slength = (RTI_INT32)length;

    if (slength > REDA_Sequence_get_maximum(out))
    {
        return RTI_FALSE;
    }

    if (!REDA_Sequence_set_length(out, slength))
    {
        return RTI_FALSE;
    }

    if (length == 0)
    {
        return RTI_TRUE;
    }

    for (i = 0; i < slength; ++i)
    {
        buffer = REDA_Sequence_get_reference(out, i);
        if (buffer == NULL)
        {
            return RTI_FALSE;
        }
        if (!deserialize_function(cdrs, buffer, param))
        {
            return RTI_FALSE;
        }
    }

    return RTI_TRUE;
}


RTI_UINT32
CDR_get_max_size_serialized_non_primitive_sequence(RTI_UINT32 current_alignment,
                   RTI_UINT32 length,
                   CDR_Stream_GetSerializedSizeFunction get_serialized_size_func,
                   void * param)
{
    RTI_UINT32 add_size;

    add_size = CDR_get_max_size_serialized_unsigned_long(current_alignment);

    add_size += CDR_get_max_size_serialized_non_primitive_array(
                                        current_alignment + add_size,
                                        length,get_serialized_size_func,param);

    return add_size;
}


/*ci 
 * \brief 
 * Serialize a property with a stream 
 *  
 * \param[in] stream Serialization stream
 * \param[in] in property Property to serialize
 * \param[in] param Unused
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
RTI_PRIVATE RTI_BOOL
CDR_Stream_serialize_property(struct CDR_Stream_t *stream,
                              struct CDR_Property *property,
                              void *param)
{
    RTI_SIZE_T str_length;
    UNUSED_ARG(param);

    str_length = REDA_String_length(property->name);
    if (!CDR_Stream_serialize_string(stream,property->name,str_length))
    {
        return RTI_FALSE;
    }

    str_length = REDA_String_length(property->value);
    if (!CDR_Stream_serialize_string(stream,property->value,str_length))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
CDR_Stream_serialize_property_sequence(struct CDR_Stream_t *stream,
                                       const void *data,
                                       void *param)
{
    return CDR_Stream_serialize_non_primitive_sequence(stream,data,
                   (CDR_Stream_SerializeFunction)CDR_Stream_serialize_property,
                   param);
}

RTI_UINT32 
CDR_align_upwards(RTI_UINT32 location, 
                  RTI_UINT8 alignment)
{
    return ((location + (alignment - 1U)) & ~(alignment - 1U));
}


RTI_UINT32
CDR_get_pad_size(RTI_UINT32 current_size,
                RTI_UINT8 align)
{
    align = (RTI_UINT8)((align <= CDR_MAX_ALIGNMENT) ? align : CDR_MAX_ALIGNMENT);
    return CDR_align_upwards(current_size, align) - current_size;
}

void 
CDR_Stream_serialize_1_byte(struct CDR_Stream_t *me, 
                            const RTI_INT8 *in)
{
    *(me->buff_ptr++) = *(char*)in;
}

void 
CDR_Stream_deserialize_1_byte(struct CDR_Stream_t *me, 
                              RTI_INT8 *out)
{
    *out = *(RTI_INT8 *)(me->buff_ptr++);
}

RTI_UINT32 
CDR_get_2_byte_max_size_serialized(RTI_UINT32 current_size)
{
    return (CDR_get_pad_size(current_size, CDR_SHORT_ALIGN) + 
            CDR_SHORT_SIZE);
}


RTI_UINT32 
CDR_get_4_byte_max_size_serialized(RTI_UINT32 current_size)
{
    return (CDR_get_pad_size(current_size, CDR_LONG_ALIGN) + 
            CDR_LONG_SIZE);
}

RTI_UINT32 
CDR_get_8_byte_max_size_serialized(RTI_UINT32 current_size)
{
    return (CDR_get_pad_size(current_size, CDR_LONG_LONG_ALIGN) + 
            CDR_LONG_LONG_SIZE);
}
 

RTI_UINT32 
CDR_get_16_byte_max_size_serialized(RTI_UINT32 current_size)
{
    return (CDR_get_pad_size(current_size, CDR_LONG_DOUBLE_ALIGN) + 
            CDR_LONG_DOUBLE_SIZE);
}
  
RTI_UINT32 
CDR_get_max_size_serialized_string(RTI_UINT32 current_size, 
                                   RTI_UINT32 length)
{
    return (CDR_get_4_byte_max_size_serialized(current_size) + 
            (CDR_CHAR_SIZE * length));
}
  
RTI_UINT32 
CDR_get_max_size_serialized_wstring(RTI_UINT32 current_size, 
                                    RTI_UINT32 length)
{
    return (CDR_get_4_byte_max_size_serialized(current_size) + 
            (CDR_WCHAR_SIZE * length));
}
  

