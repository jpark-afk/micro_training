/*
 * FILE: Serialize.c - CDR serialize API
 *
 * (c) Copyright, Real-Time Innovations, 2012-2024.
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
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

/*ci
 * \brief
 * Whether the stream has enough space for additional bytes
 *
 * \param[in] cdrs Stream
 * \param[in] needed_space Additional bytes needed
 *
 * \return RTI_TRUE if stream has enough remaining space to serialize or
 *         deserialize needed_space more bytes, otherwise RTI_FALSE
 */
RTI_BOOL
CDR_Stream_has_remaining_space(struct CDR_Stream_t *cdrs,
                               RTI_UINT32 needed_space)
{
    RTI_UINT32 remaining_space;

    remaining_space = (RTI_UINT32)(cdrs->length - (RTI_UINT32)(cdrs->buff_ptr - cdrs->buffer));

    /* length is unsigned, need to check for underflow from subtraction */
    if ((remaining_space > cdrs->length) || (remaining_space < needed_space))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci \brief Get alignment of a given type
 *
 * \param[in] type Type to get alignment
 *
 * \return Alignment in bytes of input type
 */
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

/*ci \brief Get size of a given type
 *
 * \param[in] type Type to get size
 *
 * \return Size in bytes of input type
 */
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


/*ci
 * \brief
 * Serialize an unsigned short
 *
 * \param[in] dest_buffer Serialization buffer
 * \param[in] instance Value to serialize
 * \param[in] byte_swap Flag whether to byte swap when serializing
 *
 */
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
        *OSAPI_Compiler_reinterpret_cast(RTI_UINT16*,*dest_buffer) = *OSAPI_Compiler_reinterpret_cast(RTI_UINT16*,instance);
        (*dest_buffer) += CDR_SHORT_SIZE;
    }
}

/*ci
 * \brief
 * Deserialize an unsigned short
 *
 * \param[in] src_buffer Deserialization buffer
 * \param[out] instance Deserialized instance
 * \param[in] byte_swap Flag whether to byte swap when deserializing
 *
 */
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
        *instance = *OSAPI_Compiler_reinterpret_cast(RTI_UINT16*,*src_buffer);
        (*src_buffer) += CDR_SHORT_SIZE;
    }
}

/*ci
 * \brief
 * Serialize an unsigned long
 *
 * \param[in] dest_buffer Serialization buffer
 * \param[in] instance Value to serialize
 * \param[in] byte_swap Flag whether to byte swap when serializing
 *
 */
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
        *OSAPI_Compiler_reinterpret_cast(RTI_UINT32*,*dest_buffer) = *OSAPI_Compiler_reinterpret_cast(RTI_UINT32*,instance);
        (*dest_buffer) += CDR_LONG_SIZE;
    }
}

void
CDR_serialize_unsigned_short_to_big_endian(char **dest_buffer,
                                          const RTI_UINT16 *instance)
{
    OSAPI_PRECONDITION(dest_buffer == NULL || instance == NULL,
               return,
               OSAPI_Log_entry_add_pointer("dest_buffer",dest_buffer,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("instance",instance,RTI_TRUE);)

#ifdef RTI_ENDIAN_LITTLE
    CDR_serialize_unsigned_short(dest_buffer, instance, RTI_TRUE);
#else
    CDR_serialize_unsigned_short(dest_buffer, instance, RTI_FALSE);
#endif
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


/*ci
 * \brief
 * Deserialize an unsigned long
 *
 * \param[in] src_buffer Deserialization buffer
 * \param[out] instance Deserialized instance
 * \param[in] byte_swap Flag whether to byte swap when deserializing
 *
 */
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
        *instance = *OSAPI_Compiler_reinterpret_cast(RTI_UINT32*,*src_buffer);
        (*src_buffer) += CDR_LONG_SIZE;
    }
}

/*ci
 * \brief
 * Serialize an unsigned long to big endian byte order
 *
 * \param[in] dest_buffer Serialization buffer
 * \param[in] instance Value to serialize
 *
 */
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

/*ci
 * \brief
 * Deserialize an unsigned long from big endian byte order
 *
 * \param[in] src_buffer Deserialization buffer
 * \param[out] instance Deserialized instance
 *
 */
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


/*ci
 * \brief
 * Serialize an unsigned long long
 *
 * \param[in] dest_buffer Serialization buffer
 * \param[in] instance Value to serialize
 * \param[in] byte_swap Flag whether to byte swap when serializing
 *
 */
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
        *OSAPI_Compiler_reinterpret_cast(RTI_UINT64*,*dest_buffer) = *OSAPI_Compiler_reinterpret_cast(RTI_UINT64*,instance);
        (*dest_buffer) += CDR_LONG_LONG_SIZE;
    }
}

/*ci
 * \brief
 * Deserialize an unsigned long long
 *
 * \param[in] src_buffer Deserialization buffer
 * \param[out] instance Deserialized instance
 * \param[in] byte_swap Flag whether to byte swap when deserializing
 *
 */
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
        *instance = *OSAPI_Compiler_reinterpret_cast(RTI_UINT64*,*src_buffer);
        (*src_buffer) += CDR_LONG_LONG_SIZE;
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

/*ci
 * \brief
 * Serialize a long double
 *
 * \param[in] dest_buffer Serialization buffer
 * \param[in] instance Value to serialize
 * \param[in] byte_swap Flag whether to byte swap when serializing
 *
 */
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
        *OSAPI_Compiler_reinterpret_cast(RTI_DOUBLE128*,*dest_buffer) = *OSAPI_Compiler_reinterpret_cast(RTI_DOUBLE128*,instance);
        (*dest_buffer) += CDR_LONG_DOUBLE_SIZE;
    }
}

/*ci
 * \brief
 * Deserialize a long double
 *
 * \param[in] src_buffer Deserialization buffer
 * \param[out] instance Deserialized instance
 * \param[in] byte_swap Flag whether to byte swap when deserializing
 *
 */
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
        *instance = *OSAPI_Compiler_reinterpret_cast(RTI_DOUBLE128*,*src_buffer);
        (*src_buffer) += CDR_LONG_DOUBLE_SIZE;
    }
}

/*ci \brief Get serialized size of primitive array
 *
 * \param[in] alignment Current alignment
 * \param[in] length Array length
 * \param[in] type Array element type
 *
 * \return Serialized size of primitive array, in bytes.
 */
RTI_UINT32
CDR_get_max_size_serialized_primitive_array(RTI_UINT32 alignment,
                                            RTI_UINT32 length,
                                            CdrPrimitiveType type)
{
    RTI_UINT8 type_size;
    RTI_UINT8 type_align;
    RTI_UINT32 pad_size;
    RTI_UINT32 array_size;

    if (length == 0)
    {
        return 0;
    }

    type_size = (RTI_UINT8)CDR_Primitive_get_size(type);
    type_align = CDR_Primitive_get_alignment(type);

    /* Check for overflow in multiplication */
    if (length > (UINT_MAX / type_size))
    {
        return 0;
    }
    array_size = type_size * length;

    pad_size = CDR_get_pad_size(alignment, type_align);

    /* Check for overflow in addition */
    if (pad_size > (UINT_MAX - array_size))
    {
        return 0;
    }

    return pad_size + array_size;
}

/*ci
 * \brief
 * Return length in bytes of serialized primitive sequence
 *
 * \param[in] current_alignment Alignment of serialization buffer pointer
 * \param[in] length Length of sequence
 * \param[in] type Type of sequenece element
 *
 * \return Number of bytes to serialize sequence of primitives
 */
RTI_UINT32
CDR_get_max_size_serialized_primitive_sequence(RTI_UINT32 current_alignment,
                                               RTI_UINT32 length,
                                               CdrPrimitiveType type)
{
    RTI_UINT32 add_size;
    RTI_UINT32 array_size;
    RTI_UINT32 alignment;

    add_size = CDR_get_max_size_serialized_unsigned_long(current_alignment);
    alignment = current_alignment + add_size;

    array_size = CDR_get_max_size_serialized_primitive_array(alignment, length, type);

    /* Check if array size calculation detected overflow (returns 0) */
    /* Length zero is valid for sequences, a zero length array will
     * return 0 but we want to distinguish it from overflow
     */
    if ((array_size == 0) && (length != 0))
    {
        return 0;
    }

    /* Check for overflow in addition */
    if (add_size > (UINT_MAX - array_size))
    {
        return 0;
    }

    return add_size + array_size;
}

/*ci
 * \brief
 * Serialize a char with a stream
 *
 * \param[in] cdrs Serialization stream
 * \param[in] instance Value to serialize
 *
 * \return RTI_TRUE on success, otherwise RTI_FALSE
 *
 */
RTI_BOOL
CDR_Stream_serialize_char(struct CDR_Stream_t *cdrs, const CDR_Char *instance)
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

    CDR_Stream_serialize_1_byte(cdrs, (RTI_INT8*)instance);

    return RTI_TRUE;
}

/*ci
 * \brief
 * Deserialize a char with a stream
 *
 * \param[in] cdrs Deserialization buffer
 * \param[out] instance Deserialized instance
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
CDR_Stream_deserialize_char(struct CDR_Stream_t * cdrs, CDR_Char *instance)
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

    CDR_Stream_deserialize_1_byte(cdrs, (RTI_INT8*)instance);
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

/*ci
 * \brief
 * Serialize an unsigned short with a CDR stream
 *
 * \param[in] cdrs Serialization stream
 * \param[in] instance Value to serialize
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
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

/*ci
 * \brief
 * Serialize an unsigned short to big endian byte order with a CDR stream
 *
 * \param[in] me Serialization stream
 * \param[in] in Value to serialize
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
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

/*ci
 * \brief
 * Deserialize an unsigned short with a stream
 *
 * \param[in] cdrs Deserialization buffer
 * \param[out] instance Deserialized instance
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
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

/*ci
 * \brief
 * Deserialize an unsigned short of big endian byte order with a stream
 *
 * \param[in] cdrs Deserialization buffer
 * \param[out] instance Deserialized instance
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
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

/*ci
 * \brief
 * Serialize an unsigned long with a CDR stream
 *
 * \param[in] cdrs Serialization stream
 * \param[in] instance Value to serialize
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
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

/*ci
 * \brief
 * Serialize an unsigned long to big endian byte order with a CDR stream
 *
 * \param[in] cdrs Serialization stream
 * \param[in] instance Value to serialize
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
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

RTI_BOOL
CDR_Stream_serialize_wchar32_from_unsigned_short(struct CDR_Stream_t *cdrs,
                                                 const RTI_UINT16 *instance)
{
    RTI_UINT32 x;

    x = (RTI_UINT32)*instance;

    return CDR_Stream_serialize_unsigned_long(cdrs,&x);
}

RTI_BOOL
CDR_Stream_deserialize_wchar32_to_unsigned_short(struct CDR_Stream_t *cdrs,
                                                 RTI_UINT16 *instance)
{
    RTI_UINT32 x;

    if (!CDR_Stream_deserialize_unsigned_long(cdrs,&x))
    {
        return RTI_FALSE;
    }

    *instance = (RTI_UINT16)x;

    return RTI_TRUE;
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

/*ci
 * \brief
 * Deserialize an unsigned long with a stream
 *
 * \param[in] cdrs Deserialization buffer
 * \param[out] instance Deserialized instance
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
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

/*ci
 * \brief
 * Deserialize an unsigned long with big endian byte order from a stream
 *
 * \param[in] cdrs Deserialization buffer
 * \param[out] instance Deserialized instance
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
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

/*ci
 * \brief
 * Serialize an unsigned long long with a CDR stream
 *
 * \param[in] cdrs Serialization stream
 * \param[in] instance Value to serialize
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
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

/*ci
 * \brief
 * Deserialize an unsigned long long from a stream
 *
 * \param[in] cdrs Deserialization buffer
 * \param[out] instance Deserialized instance
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
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

/*ci
 * \brief
 * Serialize a long double with a CDR stream
 *
 * \param[in] cdrs Serialization stream
 * \param[in] instance Value to serialize
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
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

/*ci
 * \brief
 * Deserialize a long double from a stream
 *
 * \param[in] cdrs Deserialization buffer
 * \param[out] instance Deserialized instance
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
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

/*ci
 * \brief
 * Serialize a string with a CDR stream
 *
 * \param[in] cdrs       Serialization stream
 * \param[in] in         Pointer to string
 * \param[in] max_length Maximum length of string excluding the terminating
 *                       NUL character
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
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
CDR_Stream_deserialize_string(struct CDR_Stream_t * cdrs,
                              char *out,
                              RTI_UINT32 max_length)
{
    RTI_UINT32 length;

    OSAPI_PRECONDITION(cdrs == NULL || out == NULL || max_length == 0,
                        return RTI_FALSE,
                        OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("out",out,RTI_FALSE);
                        OSAPI_Log_entry_add_uint("max",max_length,RTI_TRUE);)

    /* verify string pointer - valid instance might still contain NULL ptr */
    if (out == NULL)
    {
        return RTI_FALSE;
    }

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
CDR_Stream_deserialize_get_string(struct CDR_Stream_t *cdrs,
                        char **string,
                        RTI_UINT32 *str_length)
{
    RTI_UINT32 bytes_length;

    OSAPI_PRECONDITION(cdrs == NULL || string == NULL || str_length == NULL,
                           return RTI_FALSE,
                           OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("string",string,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("str_length",str_length,RTI_TRUE);)

    /* Get the string length including the null-terminator */
    if (!CDR_Stream_deserialize_unsigned_long(cdrs, &bytes_length))
    {
        return RTI_FALSE;
    }

    if (bytes_length == 0)
    {
        return RTI_FALSE;
    }

    /* Verify string fits within stream */
    if (!CDR_Stream_has_remaining_space(cdrs, bytes_length))
    {
        return RTI_FALSE;
    }

    /* Verify string is null-terminated */
    if (cdrs->buff_ptr[bytes_length - 1] != '\0')
    {
        return RTI_FALSE;
    }

    *string = cdrs->buff_ptr;
    *str_length = bytes_length - 1;

    cdrs->buff_ptr += bytes_length;

    return RTI_TRUE;
}

/*ci
 * \brief
 * Serialize a wstring with a CDR stream
 *
 * \param[in] cdrs Serialization stream
 * \param[in] in Pointer to wstring
 * \param[in] max_length Maximum length of wstring
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
CDR_Stream_serialize_wstring32_from_unsigned_long(struct CDR_Stream_t * cdrs,
                                                const RTI_UINT32 *in,
                                                RTI_UINT32 max_length)
{
    RTI_UINT32 length = 0;

    OSAPI_PRECONDITION(cdrs == NULL || in == NULL,
                            return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("in",in,RTI_TRUE);)

    while (in[length] != 0)
    {
        ++length;
    }

    ++length;

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
                                                CDR_UNSIGNED_LONG_TYPE);
}

RTI_BOOL
CDR_Stream_serialize_wstring32_from_unsigned_short(struct CDR_Stream_t * cdrs,
                                                 const RTI_UINT16 *in,
                                                 RTI_UINT32 max_length)
{
    RTI_UINT32 length = 0,index = 0, value = 0;

    OSAPI_PRECONDITION(cdrs == NULL || in == NULL,
                            return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("in",in,RTI_TRUE);)

    while (in[length] != 0)
    {
        ++length;
    }

    ++length;

    /* maximum length is EXCLUSIVE of terminating NUL character */
    if ((max_length < 1) || (length > max_length + 1))
    {
        return RTI_FALSE;
    }

    if (!CDR_Stream_serialize_unsigned_long(cdrs, &length))
    {
        return RTI_FALSE;
    }

    for (index = 0; index < length; ++index)
    {
        value = in[index];
        if (!CDR_Stream_serialize_unsigned_long(cdrs,&value))
        {
            return RTI_FALSE;
        }
    }

    return RTI_TRUE;
}

/*ci
 * \brief
 * Deserialize a wstring from a stream
 *
 * \param[in] cdrs Deserialization buffer
 * \param[inout] out Deserialized wstring
 * \param[in] max_length Maximum wstring length
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
CDR_Stream_deserialize_wstring32_to_unsigned_long(struct CDR_Stream_t * cdrs,
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
                                                  CDR_UNSIGNED_LONG_TYPE);
}

RTI_BOOL
CDR_Stream_deserialize_wstring32_to_unsigned_short(struct CDR_Stream_t * cdrs,
                                                RTI_UINT16 *out,
                                                RTI_UINT32 max_length)
{
    RTI_UINT32 length,index;
    RTI_UINT32 value;

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

    for (index = 0; index < length; ++index)
    {
        if (!CDR_Stream_deserialize_unsigned_long(cdrs,&value))
        {
            return RTI_FALSE;
        }
        out[index] = (RTI_UINT16)value;
    }

    return RTI_TRUE;
}

/*ci
 * \brief
 * Serialize unsigned short to big endian byte order with stream
 *
 * \param[in] Serialization stream
 * \param[in] Value to serialize
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
CDR_Stream_serialize_2_octets_big_endian(struct CDR_Stream_t *cdrs,
                                      const RTI_UINT16 *in)
{
    OSAPI_PRECONDITION(cdrs == NULL || in == NULL,
                            return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("in",in,RTI_TRUE);)

    CDR_Stream_align(cdrs, CDR_SHORT_ALIGN);

    /* check to make sure the CDR stream has enough space for the data type */
    if (!CDR_Stream_has_remaining_space(cdrs, CDR_UNSIGNED_SHORT_SIZE))
    {
        return RTI_FALSE;
    }

    *(cdrs->buff_ptr++) = *((char *)(in));
    *(cdrs->buff_ptr++) = *((char *)(in) + 1);

    return RTI_TRUE;
}

/*ci
 * \brief
 * Deserialize an unsigned short with big endian byte order from a stream
 *
 * \param[in] cdrs Deserialization buffer
 * \param[out] out Deserialized instance
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
CDR_Stream_deserialize_2_octets_big_endian(struct CDR_Stream_t *cdrs,
                                        RTI_UINT16 *out)
{
    char *ptr = (char *)out;

    OSAPI_PRECONDITION(cdrs == NULL || out == NULL,
                            return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("out",out,RTI_TRUE);)

    CDR_Stream_align(cdrs, CDR_SHORT_ALIGN);

    /* check to make sure the CDR stream has enough space for the data type */
    if (!CDR_Stream_has_remaining_space(cdrs, CDR_UNSIGNED_SHORT_SIZE))
    {
        return RTI_FALSE;
    }

    ptr[0] = *(cdrs->buff_ptr++);
    ptr[1] = *(cdrs->buff_ptr++);

    return RTI_TRUE;
}

/*ci
 * \brief
 * Serialize array of bytes with stream
 *
 * \param[in] cdrs Serialization stream
 * \param[in] in_array Array containing elements to serialize
 * \param[in] length Length of array
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
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

/*ci
 * \brief
 * Deserialize an array of bytes from a stream
 *
 * \param[in] cdrs Deserialization stream
 * \param[out] out Array of deserialized elements
 * \param[in] length Length of array
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
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

/*ci
 * \brief
 * Serialize array of strings with stream
 *
 * \param[in] cdrs Serialization stream
 * \param[in] in Array containing elements to serialize
 * \param[in] length Length of array
 * \param[in] max_str_len Maximum length of string
 * \param[in] type Type of string, char or wide char
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
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
            if (!CDR_Stream_serialize_wstring(cdrs,
                                              ((CDR_WCHAR_MEM_TYPE **)in)[i],
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

/*ci
 * \brief
 * Deserialize an array of strings from a stream
 *
 * \param[in] cdrs Deserialization stream
 * \param[out] out Array of deserialized strings
 * \param[in] length Length of array
 * \param[in] max_str_len Maximum length of string
 * \param[in] type Type of string, char or wide char
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
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
            if (!CDR_Stream_deserialize_wstring(cdrs,
                                                ((CDR_WCHAR_MEM_TYPE **)out)[i],
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

/*ci
 * \brief
 * Return length in bytes of serialized string array
 *
 * \param[in] current_alignment Alignment of serialization buffer pointer
 * \param[in] length Length of array
 * \param[in] max_str_len Maximum length of string
 * \param[in] type Type of string, char or wide char
 *
 * \return Number of bytes of serialized string array
 */
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
            RTI_UINT32 first_wstring_size;

            /* Calculate size of first wstring */
            first_wstring_size = CDR_get_max_size_serialized_wstring(
                                            current_alignment,
                                            (max_str_len+1));

            add_size += first_wstring_size;

            /* Calculate size of remaining wstrings only if there are more than one */
            if (length > 1)
            {
                RTI_UINT32 remaining_wstring_size;
                RTI_UINT32 mult_result;

                /* Calculate size of remaining wstrings with updated alignment */
                remaining_wstring_size = CDR_get_max_size_serialized_wstring(
                                                current_alignment + add_size,
                                                (max_str_len+1));

                /* Check for overflow in multiplication */
                if (remaining_wstring_size > (UINT_MAX / (length - 1)))
                {
                    /* Overflow detected - return 0 to indicate error */
                    return 0;
                }
                mult_result = remaining_wstring_size * (length - 1);

                /* Check for overflow in second addition */
                if (add_size > (UINT_MAX - mult_result))
                {
                    return 0;
                }
                add_size += mult_result;
            }
        }
        else
        {
            RTI_UINT32 first_string_size;

            /* Calculate size of first string */
            first_string_size = CDR_get_max_size_serialized_string(
                                        current_alignment,(max_str_len+1));

            add_size += first_string_size;

            /* Calculate size of remaining strings only if there are more than one */
            if (length > 1)
            {
                RTI_UINT32 remaining_string_size;
                RTI_UINT32 mult_result;

                /* Calculate size of remaining strings with updated alignment */
                remaining_string_size = CDR_get_max_size_serialized_string(
                                                current_alignment + add_size,
                                                (max_str_len+1));

                /* Check for overflow in multiplication */
                if (remaining_string_size > (UINT_MAX / (length - 1)))
                {
                    /* Overflow detected - return 0 to indicate error */
                    return 0;
                }
                mult_result = remaining_string_size * (length - 1);

                /* Check for overflow in second addition */
                if (add_size > (UINT_MAX - mult_result))
                {
                    return 0;
                }
                add_size += mult_result;
            }
        }
    }

    return add_size;
}

/*ci
 * \brief
 * Serialize array of primitive type elements with stream
 *
 * \param[in] cdrs Serialization stream
 * \param[in] in Array pointer
 * \param[in] length Length of array
 * \param[in] type Type of array element
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
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

    if (type > CDR_ENUM_TYPE)
    {
        return RTI_FALSE;
    }

    /* no need to check for alignment or swap order for 1-byte types:
       CDR_CHAR_TYPE, CDR_OCTET_TYPE, CDR_BOOLEAN_TYPE */
    if (CDR_Primitive_get_size(type) == 1)
    {
        if (!CDR_Stream_check_size(cdrs, length))
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
    if (!CDR_Stream_check_size(cdrs, element_size * length))
    {
        return RTI_FALSE;
    }

    /* if not byte swapping then can just copy memory */
#if (OSAPI_PLATFORM == OSAPI_PLATFORM_VXWORKS) && (CPU_FAMILY==ARM)
    if ((!cdrs->need_byte_swap) && (type != CDR_DOUBLE_TYPE))
#else
    if (!cdrs->need_byte_swap)
#endif
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
            in_2 = (RTI_UINT16 *)in;
            for (i = 0; i < length; ++i)
            {
                if (!CDR_Stream_serialize_unsigned_short(cdrs, in_2))
                {
                    return RTI_FALSE;
                }
                ++in_2;
            }
            break;
          case CDR_WCHAR_TYPE:
          case CDR_LONG_TYPE:
          case CDR_UNSIGNED_LONG_TYPE:
          case CDR_FLOAT_TYPE:
          case CDR_ENUM_TYPE:
            in_4 = (RTI_UINT32 *)in;
            for (i = 0; i < length; ++i)
            {
                if (!CDR_Stream_serialize_unsigned_long(cdrs, in_4))
                {
                    return RTI_FALSE;
                }
                ++in_4;
            }
            break;
          case CDR_LONG_LONG_TYPE:
          case CDR_UNSIGNED_LONG_LONG_TYPE:
            in_8 = (RTI_UINT64 *)in;
            for (i = 0; i < length; ++i)
            {
                if (!CDR_Stream_serialize_unsigned_long_long(cdrs, in_8))
                {
                    return RTI_FALSE;
                }
                ++in_8;
            }
            break;
          case CDR_LONG_DOUBLE_TYPE:
            in_long_double = (RTI_DOUBLE128 *)in;
            for (i = 0; i < length; ++i)
            {
                if (!CDR_Stream_serialize_long_double(cdrs, in_long_double))
                {
                    return RTI_FALSE;
                }
                ++in_long_double;
            }
            break;
          case CDR_DOUBLE_TYPE:
            in_double = (RTI_DOUBLE64 *)in;
            for (i = 0; i < length; ++i)
            {
                if (!CDR_Stream_serialize_double(cdrs, in_double))
                {
                    return RTI_FALSE;
                }
                ++in_double;
            }
            break;

          default:
            return RTI_FALSE; /* otherwise, ERROR */
        }
    }

    return RTI_TRUE;
}


/*ci
 * \brief
 * Deserialize an array of primitive type elements from a stream
 *
 * \param[in] cdrs Deserialization stream
 * \param[out] out Array of deserialized elements
 * \param[in] length Length of array
 * \param[in] type Type of array element
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
CDR_Stream_deserialize_primitive_array(struct CDR_Stream_t *cdrs,
                                      void* out,
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

    if (type > CDR_ENUM_TYPE)
    {
        return RTI_FALSE;
    }

    /* no need to check for alignment or swap order for 1-byte types:
       CDR_CHAR_TYPE, CDR_OCTET_TYPE, CDR_BOOLEAN_TYPE */
    if (CDR_Primitive_get_size(type) == 1)
    {
        if (!CDR_Stream_check_size(cdrs, length))
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

    /* if not byte swapping then can just copy memory */
#if (OSAPI_PLATFORM == OSAPI_PLATFORM_VXWORKS) && (CPU_FAMILY==ARM)
    if ((!cdrs->need_byte_swap) && (type != CDR_DOUBLE_TYPE))
#else
    if (!cdrs->need_byte_swap)
#endif
    {
        if (!CDR_Stream_check_size(cdrs, element_size * length))
        {
            return RTI_FALSE;
        }

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
                if (!CDR_Stream_deserialize_unsigned_short(cdrs, out_2))
                {
                    return RTI_FALSE;
                }
                ++out_2;
            }
            break;
          case CDR_WCHAR_TYPE:
          case CDR_LONG_TYPE:
          case CDR_UNSIGNED_LONG_TYPE:
          case CDR_FLOAT_TYPE:
          case CDR_ENUM_TYPE:
            out_4 = (RTI_UINT32 *)out;
            for (i = 0; i < length; ++i)
            {
                if (!CDR_Stream_deserialize_unsigned_long(cdrs, out_4))
                {
                    return RTI_FALSE;
                }
                ++out_4;
            }
            break;
          case CDR_LONG_LONG_TYPE:
          case CDR_UNSIGNED_LONG_LONG_TYPE:
            out_8 = (RTI_UINT64 *)out;
            for (i = 0; i < length; ++i)
            {
                if (!CDR_Stream_deserialize_unsigned_long_long(cdrs, out_8))
                {
                    return RTI_FALSE;
                }
                ++out_8;
            }
            break;
          case CDR_LONG_DOUBLE_TYPE:
            out_long_double = (RTI_DOUBLE128 *)out;
            for (i = 0; i < length; ++i)
            {
                if (!CDR_Stream_deserialize_long_double(cdrs, out_long_double))
                {
                    return RTI_FALSE;
                }
                ++out_long_double;
            }
            break;
          case CDR_DOUBLE_TYPE:
            out_double = (RTI_DOUBLE64 *)out;
            for (i = 0; i < length; ++i)
            {
                if (!CDR_Stream_deserialize_double(cdrs, out_double))
                {
                    return RTI_FALSE;
                }
                ++out_double;
            }
            break;

          default:
            return RTI_FALSE; /* otherwise, ERROR */
        }
    }

    return RTI_TRUE;
}

/*ci
 * \brief
 * Serialize array of non-primitive type elements with stream
 *
 * \param[in] cdrs Serialization stream
 * \param[in] in Array pointer
 * \param[in] length Length of array
 * \param[in] element_size Size in bytes of an array element
 * \param[in] serialize_function Serialization function per array element
 * \param[in] param Parameter for element serialization function
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
CDR_Stream_serialize_non_primitive_array(
                               struct CDR_Stream_t *cdrs,
                               const void* in,
                               RTI_UINT32 length,
                               RTI_UINT32 element_size,
                               CDR_Stream_SerializeFunction serialize_function)
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
             OSAPI_Log_entry_add_pointer("func",
                     serialize_function != NULL ? (void*)1 : NULL,RTI_TRUE);)

    array_element = (const char *)in;
    for (i = 0; i < length; ++i)
    {
        if (!serialize_function(cdrs, array_element,NULL))
        {
            return RTI_FALSE;
        }
        array_element += element_size;
    }
    return RTI_TRUE;
}

/*ci
 * \brief
 * Deserialize an array of non-primitive type elements from a stream
 *
 * \param[in] cdrs Deserialization stream
 * \param[out] out Array of deserialized elements
 * \param[in] length Length of array
 * \param[in] element_size Size in bytes of an array element
 * \param[in] deserialize_function Deserialization function per array element
 * \param[in] param Parameter for element deserialization function
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
CDR_Stream_deserialize_non_primitive_array(struct CDR_Stream_t *cdrs,
                                           void* out,
                                           RTI_UINT32 length,
                                           RTI_UINT32 element_size,
                                           CDR_Stream_DeserializeFunction
                                           deserialize_function)
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
                 OSAPI_Log_entry_add_pointer("func",
                     deserialize_function != NULL ? (void*)1 : NULL,
                     RTI_TRUE);)

    array_element = (char *)out;
    for (i = 0; i < length; ++i)
    {
        if (!deserialize_function(cdrs,array_element,NULL))
        {
            return RTI_FALSE;
        }
        array_element += element_size;
    }
    return RTI_TRUE;
}

/*ci
 * \brief
 * Return length in bytes of serialized non-primitive array
 *
 * \param[in] current_alignment Alignment of serialization buffer pointer
 * \param[in] length Length of array
 * \param[in] get_serialized_size_func Function returning serialized size of one
 * element
 * \param[in] param Parameter for element serialized size function
 *
 * \return Number of bytes to serialize array of non-primitive elements
 */
RTI_UINT32
CDR_get_max_size_serialized_non_primitive_array(
                void *data,
                RTI_UINT32 current_alignment,
                RTI_UINT32 ulength,
                CDR_Stream_GetSerializedSizeFunction get_serialized_size_func)
{
    RTI_UINT32 add_size = 0;
    RTI_INT32 i = 0,samples_in_loop, loop_count;
    RTI_UINT32 loop_size = 0;
    RTI_INT32 alignment, align[8];
    RTI_UINT32 size_align[8];
    RTI_INT32 length;

    OSAPI_PRECONDITION(get_serialized_size_func == NULL,
           return 0,
           OSAPI_Log_entry_add_pointer("func",
                               get_serialized_size_func != NULL ?
                                       (void*)1 : NULL,RTI_TRUE);)

    if ((ulength > INT_MAX) || (ulength == 0))
    {
        return 0;
    }

    length = (RTI_INT32)ulength;
    for (i = 0; i < 8; i++)
    {
        align[i] = -1;
        size_align[i] = 0;
    }

    i = 0;
    alignment = (current_alignment % 8);

    while ((align[alignment] < 0) && (i < length))
    {
        RTI_UINT32 element_size;
        /* Can handle array sizes up to 2^31 */
        align[alignment] = i;
        size_align[alignment] = add_size;
        element_size = get_serialized_size_func(
                            data,current_alignment + add_size,NULL);

        /* Check if nested call detected overflow (returns 0) */
        if (element_size == 0)
        {
            return 0;
        }

        /* Check for overflow in addition */
        if (add_size > (UINT_MAX - element_size))
        {
            return 0;
        }

        add_size += element_size;
        i++;
        alignment = (RTI_INT32)((current_alignment + add_size) % 8);
    }

    if (i < length)
    {
        samples_in_loop = i - align[alignment];
        loop_size = add_size - size_align[alignment];
        loop_count = (length - i)/samples_in_loop;

        /* Check for overflow in multiplication */
        if ((loop_count != 0) && (loop_size > (UINT_MAX / (RTI_UINT32)loop_count)))
        {
            /* Overflow detected - return 0 to indicate error */
            return 0;
        }

        /* Check for overflow in addition */
        if (add_size > (UINT_MAX - (loop_size * (RTI_UINT32)loop_count)))
        {
            return 0;
        }

        add_size += (loop_size * (RTI_UINT32)loop_count);
        i += samples_in_loop * loop_count;
    }

    for(;i < length; ++i)
    {
        RTI_UINT32 element_size;
        element_size = get_serialized_size_func(
                data,current_alignment + add_size,NULL);

        /* Check if nested call detected overflow (returns 0) */
        if (element_size == 0)
        {
            return 0;
        }

        /* Check for overflow in addition */
        if (add_size > (UINT_MAX - element_size))
        {
            return 0;
        }

        add_size += element_size;
    }
    return add_size;
}

/*ci
 * \brief
 * Serialize a sequence of char or wide char strings with a stream
 *
 * \param[in] cdrs Serialization stream
 * \param[in] in Sequence of strings
 * \param[in] max_str_len Maximum length of a string
 * \param[in] type Type of string, either char or wide char
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
CDR_Stream_serialize_string_sequence(struct CDR_Stream_t *cdrs,
                                    const struct REDA_Sequence* in,
                                    RTI_UINT32 max_str_len,
                                    CdrPrimitiveType type)
{
    RTI_INT32 i, length;
    RTI_UINT32 ulength = 0;
    const char *in_str;
    const CDR_WCHAR_MEM_TYPE *in_wstr;

    OSAPI_PRECONDITION((cdrs == NULL) || (cdrs->buff_ptr == NULL) ||
                           (in == NULL),
                           return RTI_FALSE,
                           OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("cdrs->buff_ptr",
                                 (cdrs != NULL ? cdrs->buff_ptr : NULL),RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("in",in,RTI_TRUE);)

    length = REDA_Sequence_get_length(in);

    /* must have at least one element for non-zero sequence */
    if ((length != 0) && (REDA_Sequence_get_reference(in, 0) == NULL))
    {
        return RTI_FALSE;
    }

    /* serialize sequence length */
    ulength = (RTI_UINT32)length;
    if (!CDR_Stream_serialize_unsigned_long(cdrs, &ulength))
    {
        return RTI_FALSE;
    }

    if (length == 0)
    {
        return RTI_TRUE;
    }

    /* A sequence cannot be larger than an int can */
    length  = (RTI_INT32)ulength;
    if (type == CDR_CHAR_TYPE)
    {
        for (i = 0 ; i < length ;i++)
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
        for (i = 0; i < length; i++)
        {
            in_wstr = *(const CDR_WCHAR_MEM_TYPE **)
                                            REDA_Sequence_get_reference(in, i);
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

/*ci
 * \brief
 * Deserialize a sequence of char or wide char strings from a stream
 *
 * \param[in] cdrs Deserialization stream
 * \param[out] out Sequence of deserialized strings
 * \param[in] max_str_len Maximum length of a string
 * \param[in] type Type of string, either char or wide char
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
CDR_Stream_deserialize_string_sequence(struct CDR_Stream_t *cdrs,
                                      struct REDA_Sequence* out,
                                      RTI_UINT32 max_str_len,
                                      CdrPrimitiveType type)
{
    RTI_INT32 i, length;
    char *out_str;
    CDR_WCHAR_MEM_TYPE *out_wstr;

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
            out_wstr = *(CDR_WCHAR_MEM_TYPE **) REDA_Sequence_get_reference(out, i);
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

/*ci
 * \brief
 * Return length in bytes of serialized string sequence
 *
 * \param[in] current_alignment Alignment of serialization buffer pointer
 * \param[in] length Length of array
 * \param[in] max_str_len Maximum length of a string
 * \param[in] Type of string, either char or wide char
 *
 * \return Number of bytes to serialize sequence of strings
 */
RTI_UINT32
CDR_get_max_size_serialized_string_sequence(RTI_UINT32 current_alignment,
                                         RTI_UINT32 length,
                                         RTI_UINT32 max_str_len,
                                         CdrPrimitiveType type)
{
    RTI_UINT32 add_size = 0;

    add_size = CDR_get_max_size_serialized_unsigned_long(current_alignment);

    /* maximum string length is EXCLUSIVE of terminating NUL character so add 1 */
    if (length > 0) {
        if (type == CDR_WCHAR_TYPE)
        {
            RTI_UINT32 first_wstring_size;

            /* Calculate size of first wstring */
            first_wstring_size = CDR_get_max_size_serialized_wstring(current_alignment + add_size,(max_str_len+1));

            /* Check for overflow in first addition */
            if (add_size > (UINT_MAX - first_wstring_size))
            {
                return 0;
            }
            add_size += first_wstring_size;

            /* Calculate size of remaining wstrings only if there are more than one */
            if (length > 1)
            {
                RTI_UINT32 remaining_wstring_size;
                RTI_UINT32 mult_result;

                /* Calculate size of remaining wstrings with updated alignment */
                remaining_wstring_size = CDR_get_max_size_serialized_wstring(
                                    current_alignment + add_size,
                                    (max_str_len+1));

                /* Check for overflow in multiplication */
                if (remaining_wstring_size > (UINT_MAX / (length - 1)))
                {
                    /* Overflow detected - return 0 to indicate error */
                    return 0;
                }
                mult_result = remaining_wstring_size * (length - 1);

                /* Check for overflow in second addition */
                if (add_size > (UINT_MAX - mult_result))
                {
                    return 0;
                }
                add_size += mult_result;
            }
        }
        else
        {
            RTI_UINT32 first_string_size;

            /* Calculate size of first string */
            first_string_size = CDR_get_max_size_serialized_string(
                                    current_alignment + add_size,
                                    (max_str_len+1));

            /* Check for overflow in first addition */
            if (add_size > (UINT_MAX - first_string_size))
            {
                return 0;
            }
            add_size += first_string_size;

            /* Calculate size of remaining strings only if there are more than one */
            if (length > 1)
            {
                RTI_UINT32 remaining_string_size;
                RTI_UINT32 mult_result;

                /* Calculate size of remaining strings with updated alignment */
                remaining_string_size = CDR_get_max_size_serialized_string(
                                            current_alignment + add_size,
                                            (max_str_len+1));

                /* Check for overflow in multiplication */
                if (remaining_string_size > (UINT_MAX / (length - 1)))
                {
                    /* Overflow detected - return 0 to indicate error */
                    return 0;
                }
                mult_result = remaining_string_size * (length - 1);

                /* Check for overflow in second addition */
                if (add_size > (UINT_MAX - mult_result))
                {
                    return 0;
                }
                add_size += mult_result;
            }
        }
    }

    return add_size;
}

/*ci
 * \brief
 * Serialize a sequence of primitive type elements with a stream
 *
 * \param[in] cdrs Serialization stream
 * \param[in] in Sequence of primitive elements
 * \param[in] type Type of sequence element
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
CDR_Stream_serialize_primitive_sequence(struct CDR_Stream_t *cdrs,
                                       const struct REDA_Sequence* in,
                                       CdrPrimitiveType type)
{
    RTI_INT32 si,slength;
    RTI_UINT32 i,length,element_size;
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

    if (type > CDR_ENUM_TYPE)
    {
        return RTI_FALSE;
    }

    /* A sequence can never have a negative length */
    slength = REDA_Sequence_get_length(in);

    /* PrimitiveSequences always use contiguous buffer */
    buffer = (char *)REDA_Sequence_get_buffer(in);

    /* must have at least one element for non-zero sequence */
    if ((slength != 0) && (buffer == NULL))
    {
        return RTI_FALSE;
    }

    /* serialize sequence length */
    length = (RTI_UINT32)slength;

    if (!CDR_Stream_serialize_unsigned_long(cdrs, &length))
    {
        return RTI_FALSE;
    }

    if (slength == 0)
    {
        return RTI_TRUE;
    }

    /* no need to check for alignment or swap order for 1-byte types:
       CDR_CHAR_TYPE, CDR_OCTET_TYPE, CDR_BOOLEAN_TYPE */
    if ((CDR_Primitive_get_size(type) == 1) &&
        !REDA_Sequence_has_discontiguous_buffer(in))
    {
        if (!CDR_Stream_check_size(cdrs, length))
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
    if (!CDR_Stream_check_size(cdrs, element_size * length))
    {
        return RTI_FALSE;
    }

    /* if not byte swapping and contiguous buffer then can just copy memory */
#if (OSAPI_PLATFORM == OSAPI_PLATFORM_VXWORKS) && (CPU_FAMILY==ARM)
    if ((!cdrs->need_byte_swap) && (type != CDR_DOUBLE_TYPE) &&
        !REDA_Sequence_has_discontiguous_buffer(in))
#else
    if ((!cdrs->need_byte_swap) && !REDA_Sequence_has_discontiguous_buffer(in))
#endif
    {
        OSAPI_Memory_copy(cdrs->buff_ptr, buffer, element_size * length);
        cdrs->buff_ptr += element_size * length;
    }
    else
    {
        /* Every value within the switch has been accommodated by the collection
         * of case clauses. A check was performed to validate that <type> is
         * less then CDR_ENUM_TYPE. A default case is not required.
         */
        switch (type)
        {
            case CDR_CHAR_TYPE:
            case CDR_OCTET_TYPE:
            case CDR_BOOLEAN_TYPE:
                for (i = 0, si = 0; i < length; ++i,++si)
                {
                    in_1 = (RTI_INT8 *) REDA_Sequence_get_reference(in, si);
                    if (!CDR_Stream_serialize_char(cdrs, (CDR_Char *)in_1))
                    {
                        return RTI_FALSE;
                    }
                }
                break;
            case CDR_SHORT_TYPE:
            case CDR_UNSIGNED_SHORT_TYPE:
                for (i = 0, si = 0; i < length; ++i,++si)
                {
                    in_2 = (RTI_UINT16 *) REDA_Sequence_get_reference(in, si);
                    if (!CDR_Stream_serialize_unsigned_short(cdrs, in_2))
                    {
                        return RTI_FALSE;
                    }
                }
                break;
            case CDR_WCHAR_TYPE:
            case CDR_LONG_TYPE:
            case CDR_UNSIGNED_LONG_TYPE:
            case CDR_FLOAT_TYPE:
            case CDR_ENUM_TYPE:
                for (i = 0, si = 0; i < length; ++i,++si)
                {
                    in_4 = (RTI_UINT32 *) REDA_Sequence_get_reference(in, si);
                    if (!CDR_Stream_serialize_unsigned_long(cdrs, in_4))
                    {
                        return RTI_FALSE;
                    }
                }
                break;
            case CDR_LONG_LONG_TYPE:
            case CDR_UNSIGNED_LONG_LONG_TYPE:
                for (i = 0, si = 0; i < length; ++i,++si)
                {
                    in_8 = (RTI_UINT64 *) REDA_Sequence_get_reference(in, si);
                    if (!CDR_Stream_serialize_unsigned_long_long(cdrs, in_8))
                    {
                        return RTI_FALSE;
                    }
                }
                break;
            case CDR_LONG_DOUBLE_TYPE:
                for (i = 0, si = 0; i < length; ++i,++si)
                {
                    in_long_double = (RTI_DOUBLE128 *) REDA_Sequence_get_reference(in, si);
                    if (!CDR_Stream_serialize_long_double(cdrs, in_long_double))
                    {
                        return RTI_FALSE;
                    }
                }
                break;
            case CDR_DOUBLE_TYPE:
                for (i = 0, si = 0; i < length; ++i,++si)
                {
                    in_double = (RTI_DOUBLE64 *) REDA_Sequence_get_reference(in, si);
                    if (!CDR_Stream_serialize_double(cdrs, in_double))
                    {
                        return  RTI_FALSE;
                    }
                }
                break;
        }
    }

    return RTI_TRUE;
}

/*ci
 * \brief
 * Deserialize a sequence of primitive type elements with a stream
 *
 * \param[in] cdrs Deserialization stream
 * \param[out] out Sequence of primitive elements
 * \param[in] type Type of sequence element
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
CDR_Stream_deserialize_primitive_sequence(struct CDR_Stream_t *cdrs,
                                         struct REDA_Sequence* out,
                                         CdrPrimitiveType type)
{
    RTI_UINT32 i, element_size,length;
    RTI_INT32 slength,si;
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

    if (type > CDR_ENUM_TYPE)
    {
        return RTI_FALSE;
    }

    /* deserialize sequence length as unsigned int */
    if (!CDR_Stream_deserialize_long(cdrs, &length))
    {
        return RTI_FALSE;
    }

    if (length >= 0x80000000)
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

    /* PrimitiveSequences always use contiguous buffer */
    buffer = (char *) REDA_Sequence_get_buffer(out);

    /* no need to check for alignment or swap order for 1-byte types:
       CDR_CHAR_TYPE, CDR_OCTET_TYPE, CDR_BOOLEAN_TYPE */
    if ((CDR_Primitive_get_size(type) == 1) &&
            !REDA_Sequence_has_discontiguous_buffer(out))
    {
        if (!CDR_Stream_check_size(cdrs, length))
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

    /* if not byte swapping and contiguous buffer then can just copy memory */
#if (OSAPI_PLATFORM == OSAPI_PLATFORM_VXWORKS) && (CPU_FAMILY==ARM)
    if ((!cdrs->need_byte_swap) && (type != CDR_DOUBLE_TYPE) &&
            !REDA_Sequence_has_discontiguous_buffer(out))
#else
    if ((!cdrs->need_byte_swap) && !REDA_Sequence_has_discontiguous_buffer(out))
#endif
    {
        if (!CDR_Stream_check_size(cdrs, element_size * length))
        {
            return RTI_FALSE;
        }
        OSAPI_Memory_copy(buffer, cdrs->buff_ptr, element_size*length);
        cdrs->buff_ptr += element_size * length;
    }
    else
    {
        /* Every value within the switch has been accommodated by the collection
         * of case clauses. A check was performed to validate that <type> is
         * less then CDR_ENUM_TYPE. A default case is not required.
         */
        switch (type)
        {
        case CDR_CHAR_TYPE:
        case CDR_OCTET_TYPE:
        case CDR_BOOLEAN_TYPE:
            for (i = 0, si = 0; i < length; ++i,++si)
            {
                out_1 = (RTI_INT8 *) REDA_Sequence_get_reference(out, si);
                if (!CDR_Stream_deserialize_char(cdrs, (CDR_Char *)out_1))
                {
                    return RTI_FALSE;
                }
            }
            break;
        case CDR_SHORT_TYPE:
        case CDR_UNSIGNED_SHORT_TYPE:
            for (i = 0, si = 0; i < length; ++i,++si)
            {
                out_2 = (RTI_UINT16 *) REDA_Sequence_get_reference(out, si);
                if (!CDR_Stream_deserialize_unsigned_short(cdrs, out_2))
                {
                    return RTI_FALSE;
                }
            }
            break;
        case CDR_WCHAR_TYPE:
        case CDR_LONG_TYPE:
        case CDR_UNSIGNED_LONG_TYPE:
        case CDR_FLOAT_TYPE:
        case CDR_ENUM_TYPE:
            for (i = 0, si = 0; i < length; ++i,++si)
            {
                out_4 = (RTI_UINT32 *) REDA_Sequence_get_reference(out, si);
                if (!CDR_Stream_deserialize_unsigned_long(cdrs, out_4))
                {
                    return RTI_FALSE;
                }
            }
            break;
        case CDR_LONG_LONG_TYPE:
        case CDR_UNSIGNED_LONG_LONG_TYPE:
            for (i = 0, si = 0; i < length; ++i,++si)
            {
                out_8 = (RTI_UINT64 *) REDA_Sequence_get_reference(out, si);
                if (!CDR_Stream_deserialize_unsigned_long_long(cdrs, out_8))
                {
                    return RTI_FALSE;
                }
            }
            break;
        case CDR_LONG_DOUBLE_TYPE:
            for (i = 0, si = 0; i < length; ++i,++si)
            {
                out_long_double = (RTI_DOUBLE128 *) REDA_Sequence_get_reference(out, si);
                if (!CDR_Stream_deserialize_long_double(cdrs, out_long_double))
                {
                    return RTI_FALSE;
                }
            }
            break;
        case CDR_DOUBLE_TYPE:
            for (i = 0, si = 0; i < length; ++i,++si)
            {
                out_double = (RTI_DOUBLE64 *) REDA_Sequence_get_reference(out, si);
                if (!CDR_Stream_deserialize_double(cdrs, out_double))
                {
                    return RTI_FALSE;
                }
            }
            break;
        }
    }

    return RTI_TRUE;
}

RTI_BOOL
CDR_Stream_deserialize_byte_sequence_without_copy(struct CDR_Stream_t *cdrs,
                                                       struct REDA_Sequence* out,
                                                       CdrPrimitiveType type)
{
    RTI_INT32 length;

    PRECOND_ARG(type)

    OSAPI_PRECONDITION((cdrs == NULL) || (cdrs->buff_ptr == NULL) ||
                           (out == NULL) ||
                           (CDR_Primitive_get_size(type) != CDR_OCTET_SIZE),
                           return RTI_FALSE,
                           OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("cdrs->buff_ptr",
                                 (cdrs != NULL ? cdrs->buff_ptr : NULL),RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("out",out,RTI_TRUE);)

    /* Deserialize sequence length as unsigned int */
    if (!CDR_Stream_deserialize_long(cdrs, &length))
    {
        return RTI_FALSE;
    }

    if (length < 0)
    {
        return RTI_FALSE;
    }

    /* Loan bytes to output sequence */
    if (!REDA_Sequence_loan_contiguous(out, cdrs->buff_ptr, length, length))
    {
        return RTI_FALSE;
    }
    cdrs->buff_ptr += (RTI_SIZE_T)length;

    return RTI_TRUE;
}

/*ci
 * \brief
 * Serialize a sequence of non-primitive type elements with a stream
 *
 * \param[in] cdrs Serialization stream
 * \param[in] in Sequence of non-primitive elements
 * \param[in] serialize_function Serialization function of the non-primitive type
 * \param[in] param Parameter for serialize function of non-primitive type
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
CDR_Stream_serialize_non_primitive_sequence(struct CDR_Stream_t *cdrs,
                                            const struct REDA_Sequence* in,
                                            CDR_Stream_SerializeFunction
                                            serialize_function)
{
    RTI_INT32 i, length;
    RTI_UINT32 ulength;

    void * buffer;

    OSAPI_PRECONDITION((cdrs == NULL) || (cdrs->buff_ptr == NULL) ||
                           (serialize_function == NULL),
               return RTI_FALSE,
               OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("cdrs->buff_ptr",
                       (cdrs != NULL ? cdrs->buff_ptr : NULL),RTI_FALSE);
               OSAPI_Log_entry_add_pointer("serialize_function",
                       serialize_function != NULL ? (void*)0x1 : NULL,RTI_TRUE);)

    length = REDA_Sequence_get_length(in);

    /* slength cannot < 0) */
    ulength = (RTI_UINT32)length;

    if (!CDR_Stream_serialize_unsigned_long(cdrs, &ulength))
    {
        return RTI_FALSE;
    }

    for (i = 0; i < length; ++i)
    {
        buffer = REDA_Sequence_get_reference(in, i);
        if (buffer == NULL)
        {
            return RTI_FALSE;
        }

        if (!serialize_function(cdrs, buffer, NULL))
        {
            return RTI_FALSE;
        }
    }

    return RTI_TRUE;
}

/*ci
 * \brief
 * Deserialize a sequence of non-primitive type elements with a stream
 *
 * \param[in] cdrs Deserialization stream
 * \param[out] out Sequence of non-primitive elements
 * \param[in] deserialize_function Deserialization function of a non-primitive
 * type
 * \param[in] param Parameter for deserialize function of non-primitive type
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
CDR_Stream_deserialize_non_primitive_sequence(struct CDR_Stream_t *cdrs,
                                              struct REDA_Sequence* out,
                                              CDR_Stream_DeserializeFunction
                                              deserialize_function)
{
    RTI_UINT32 length;
    RTI_INT32 i,slength;
    void * buffer;

    OSAPI_PRECONDITION((cdrs == NULL) || (cdrs->buff_ptr == NULL) ||
                           (deserialize_function == NULL) || (out == NULL),
                           return RTI_FALSE,
                           OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("cdrs->buff_ptr",
                                   (cdrs != NULL ? cdrs->buff_ptr : NULL),RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("deserialize_function",
                                   deserialize_function != NULL ? (void*)1 : NULL,RTI_TRUE);)

    if (!CDR_Stream_deserialize_unsigned_long(cdrs, &length))
    {
        return RTI_FALSE;
    }

    if (length >= 0x80000000)
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

    for (i = 0; i < slength; ++i)
    {
        buffer = REDA_Sequence_get_reference(out, i);

        if (buffer == NULL)
        {
            return RTI_FALSE;
        }

        if (!deserialize_function(cdrs,buffer, NULL))
        {
            return RTI_FALSE;
        }
    }

    return RTI_TRUE;
}

/*ci
 * \brief
 * Return the length in bytes to serialize a sequence of non-primitive type
 * elements
 *
 * \param[in] current_alignment Alignment of serialization buffer
 * \param[in] length Length of sequence
 * \param[in] get_serialized_size_func Function returning the length in bytes of
 * one serialized sequence element
 * type
 * \param[in] param Parameter for serialized size function of non-primitive type
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_UINT32
CDR_get_max_size_serialized_non_primitive_sequence(
                void *data,
                RTI_UINT32 current_alignment,
                RTI_UINT32 length,
                CDR_Stream_GetSerializedSizeFunction get_serialized_size_func)
{
    RTI_UINT32 add_size;
    RTI_UINT32 seq_size = 0;


    add_size = CDR_get_max_size_serialized_unsigned_long(current_alignment);

    seq_size = CDR_get_max_size_serialized_non_primitive_array(
                                        data,
                                        current_alignment + add_size,
                                        length,get_serialized_size_func);
    /* Check if seq size calculation detected overflow (returns 0) */
    if ((seq_size == 0) && (length != 0))
    {
        return 0;
    }

    if (add_size > (UINT_MAX - seq_size))
    {
        return 0;
    }

    add_size += seq_size;

    return add_size;
}
typedef RTI_BOOL
(*CDR_Stream_FilterFunction)(struct CDR_Stream_t *stream,const void *data);
/*ci
 * \brief
 * Serialize a sequence of non-primitive type elements with a stream, possibly
 * filtering some elements out.
 *
 * The specified filter_function is used to filter each elements during serialization
 * and calculate the size of the serialized sequence.
 *
 * The serialize_function is expected to silently skip over elements that the
 * filter_function filtered out.
 *
 * \param[in] cdrs Serialization stream
 * \param[in] in Sequence of non-primitive elements
 * \param[in] serialize_function Serialization function of the non-primitive type
 * \param[in] filter_function Filtering function of the non-primitive type
 * \param[in] param Parameter for serialize function of non-primitive type
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
CDR_Stream_filter_serialize_non_primitive_sequence(
        struct CDR_Stream_t *cdrs,
        const struct REDA_Sequence* in,
        CDR_Stream_SerializeFunction serialize_function,
        CDR_Stream_FilterFunction filter_function)
{
    RTI_UINT32 serial_length = 0,
               length_offset = 0,
               final_offset = 0;
    RTI_INT32 i = 0,length;

    void * buffer;

    OSAPI_PRECONDITION((cdrs == NULL) || (cdrs->buff_ptr == NULL) ||
                           (serialize_function == NULL) ||
                           (filter_function == NULL),
                   return RTI_FALSE,
                   OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_FALSE);
                   OSAPI_Log_entry_add_pointer("cdrs->buff_ptr",
                           (cdrs != NULL ? cdrs->buff_ptr : NULL),RTI_FALSE);
                   OSAPI_Log_entry_add_pointer("serialize_function",
                           serialize_function != NULL ? (void*)1 : NULL,RTI_TRUE);
                   OSAPI_Log_entry_add_pointer("filter_function",
                           filter_function != NULL ? (void*)1 : NULL,RTI_TRUE);)

    length_offset = CDR_Stream_get_current_position_offset(cdrs);

    if (!CDR_Stream_increment_current_position(cdrs, CDR_LONG_SIZE))
    {
        return RTI_FALSE;
    }

    length = REDA_Sequence_get_length(in);

    /* Sequence API prevents length < 0 */
    if (length < 0)
    {
        return RTI_FALSE;
    }

    serial_length = (RTI_UINT32)length;

    for (i = 0; i < length; ++i)
    {
        buffer = REDA_Sequence_get_reference(in, i);
        if (buffer == NULL)
        {
            return RTI_FALSE;
        }
        if (!filter_function(cdrs,buffer))
        {
            --serial_length;
        }
        else if (!serialize_function(cdrs, buffer,NULL))
        {
            return RTI_FALSE;
        }
    }

    final_offset = CDR_Stream_get_current_position_offset(cdrs);

    if (!CDR_Stream_set_current_position_offset(cdrs,length_offset))
    {
        return RTI_FALSE;
    }

    if (!CDR_Stream_serialize_unsigned_long(cdrs, &serial_length))
    {
        return RTI_FALSE;
    }

    if (!CDR_Stream_set_current_position_offset(cdrs,final_offset))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief
 * Serialize a property with a stream
 *
 * \param[inout] cdrs Serialization stream
 * \param[in] in property Property to serialize
 * \param[in] param Unused
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
CDR_Stream_serialize_property(struct CDR_Stream_t *stream,
                              const void *data,
                              void *param)
{
    const struct CDR_Property *property = (const struct CDR_Property*) data;
    UNUSED_ARG(param);

    OSAPI_PRECONDITION(stream == NULL ||
            property == NULL || !property->propagate,
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("stream",stream,RTI_FALSE);
            OSAPI_Log_entry_add_pointer("property",property,RTI_FALSE);
            OSAPI_Log_entry_add_int("property->propagate",
                    (property != NULL)?property->propagate:0,RTI_TRUE);)

    if (!CDR_Stream_serialize_string(
            stream, property->name, CDR_PROPERTY_NAME_MAX_LEN))
    {
        return RTI_FALSE;
    }

    if (!CDR_Stream_serialize_string(
            stream, property->value, CDR_PROPERTY_VALUE_MAX_LEN))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_UINT32
CDR_get_max_size_serialized_property(
        struct NDDS_Type_Plugin *plugin,
        RTI_UINT32 current_alignment,
        void *param)
{
    RTI_UINT32 initial_alignment = current_alignment;
    UNUSED_ARG(plugin);
    UNUSED_ARG(param);

    current_alignment += CDR_get_max_size_serialized_string(
        current_alignment, CDR_PROPERTY_NAME_MAX_SERIALIZED_LEN+1);

    current_alignment += CDR_get_max_size_serialized_string(
        current_alignment, CDR_PROPERTY_VALUE_MAX_SERIALIZED_LEN+1);

    return  current_alignment - initial_alignment;
}

RTI_PRIVATE RTI_BOOL
CDR_Stream_filter_property(struct CDR_Stream_t *stream,
                           const void *data)
{
    const struct CDR_Property *property = (const struct CDR_Property *) data;
    UNUSED_ARG(stream);

    return (property->propagate == CDR_BOOLEAN_TRUE);
}

/*ci
 * \brief
 * Serialize a sequence of properties with a stream
 *
 * \param[inout] stream Serialization stream
 * \param[in] data Sequence of properties to serialize
 * \param[in] param Parameter for property serialization function
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
CDR_Stream_serialize_property_sequence(struct CDR_Stream_t *stream,
                                       const void *data,
                                       void *param)
{
    UNUSED_ARG(param);

    return CDR_Stream_filter_serialize_non_primitive_sequence(
                   stream,
                   data,
                   CDR_Stream_serialize_property,
                   CDR_Stream_filter_property);
}

/*ci
 * \brief Align current location to specified alignment
 *
 * \param[in] location Current position
 * \param[in] alignment Desired alignment
 *
 * \return New position, at or greater than input location, at the desired
 *         alignment
 */
RTI_UINT32
CDR_align_upwards(RTI_UINT32 location,
                  RTI_UINT8 alignment)
{
    return ((location + (alignment - 1U)) & ~(alignment - 1U));
}


/*ci
 * \brief
 * Return the size of additional padding in bytes to a buffer to get desired
 * alignment
 *
 * \param[in] current_size Current position of buffer
 * \param[in] align Desired alignment
 *
 * \return Number of bytes to add to current buffer to be at desired alignment
 */
RTI_UINT32
CDR_get_pad_size(RTI_UINT32 current_size,
                RTI_UINT8 align)
{
    align = (RTI_UINT8)((align <= CDR_MAX_ALIGNMENT) ? align : CDR_MAX_ALIGNMENT);
    return CDR_align_upwards(current_size, align) - current_size;
}

/*ci \brief Serialize a byte into a stream
 * \param[inout] me Serialization stream
 * \param[in] in Byte to serialize
 *
 */
void
CDR_Stream_serialize_1_byte(struct CDR_Stream_t *me,
                            const RTI_INT8 *in)
{
    *(me->buff_ptr++) = *(char*)in;
}

/*ci \brief Deserialize a byte from a stream
 * \param[in] me Deserialization stream
 * \param[inout] out Deserialized byte
 *
 */
void
CDR_Stream_deserialize_1_byte(struct CDR_Stream_t *me,
                              RTI_INT8 *out)
{
    *out = *(char*)(me->buff_ptr++);
}

/*ci \brief Get size of serialized and aligned short
 * \param[in] currentSize Current position
 *
 * \return Size of serialized and aligned short
 */
RTI_UINT32
CDR_get_2_byte_max_size_serialized(RTI_UINT32 current_size)
{
    return (CDR_get_pad_size(current_size, CDR_SHORT_ALIGN) +
            CDR_SHORT_SIZE);
}


/*ci \brief Get size of serialized and aligned long
 * \param[in] currentSize Current position
 *
 * \return Size of serialized and aligned long
 */
RTI_UINT32
CDR_get_4_byte_max_size_serialized(RTI_UINT32 current_size)
{
    return (CDR_get_pad_size(current_size, CDR_LONG_ALIGN) +
            CDR_LONG_SIZE);
}


/*ci \brief Get size of serialized and aligned long long
 * \param[in] currentSize Current position
 *
 * \return Size of serialized and aligned long long
 */
RTI_UINT32
CDR_get_8_byte_max_size_serialized(RTI_UINT32 current_size)
{
    return (CDR_get_pad_size(current_size, CDR_LONG_LONG_ALIGN) +
            CDR_LONG_LONG_SIZE);
}


/*ci \brief Get size of serialized and aligned long double
 * \param[in] currentSize Current position
 *
 * \return Size of serialized and aligned long double
 */
RTI_UINT32
CDR_get_16_byte_max_size_serialized(RTI_UINT32 current_size)
{
    return (CDR_get_pad_size(current_size, CDR_LONG_DOUBLE_ALIGN) +
            CDR_LONG_DOUBLE_SIZE);
}

/*ci \brief Get size of serialized and aligned string
 * \param[in] current_size Current position
 * \param[in] length String length
 *
 * \return Size of serialized and aligned string
 */
RTI_UINT32
CDR_get_max_size_serialized_string(RTI_UINT32 current_size,
                                   RTI_UINT32 length)
{
    return (CDR_get_4_byte_max_size_serialized(current_size) +
            (CDR_CHAR_SIZE * length));
}


/*ci \brief Get size of serialized and aligned wstring
 * \param[in] current_size Current position
 * \param[in] length Wstring length, in Wchars
 *
 * \return Size of serialized and aligned wstring
 */
RTI_UINT32
CDR_get_max_size_serialized_wstring(RTI_UINT32 current_size,
                                    RTI_UINT32 length)
{
    return (CDR_get_4_byte_max_size_serialized(current_size) +
            (CDR_WCHAR_SIZE * length));
}


