/*
 * FILE: cdr_serialize.h - CDR serialize API
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
 * 13dec2020,tk
 *     - MICRO-2710/PR.28397
 *         - Make param as unused for CDR_Stream_serialize_property_sequence
 *         - Corrected description of return value for
 *           CDR_get_max_size_serialized_non_primitive_sequence
 * 10dec2020,tk
 *     - MICRO-2703/PR#28316
 *       - Removed CDR_Stream_serialize_2_octets_big_endian() and
 *         CDR_Stream_deserialize_2_octets_big_endian
 *     - MICRO-2712/PR#28305
 *       - Consistent use of in,out, and inout for parameter designations.
 * 20oct2020,tk  MICRO-2623/PR#28237 Replaced tabs with spaces
 * 09sep2020,tk  MICRO-2528/PR#28010 Renamed variables in
 *               CDR_get_max_size_serialized_primitive_sequence and
 *               CDR_get_max_size_serialized_primitive_array.
 * 30jun2015,eh  MICRO-1374/PR#15168 Rename CDR_Stream_has_free_space to
 *               CDR_Stream_has_remaining_space.
 * 09jun2015,eh  MICRO-1296/PR#14964 Make non-private CDR_Stream_has_free_space
 * 23feb2015,eh  MICRO-1075: remove and replace macros
 * 15sep2014,eh  Updated documentation
 * 24mar2012,kaj Written
 */

/*ci
 * \file 
 * \defgroup CDRSerializeClass CDR Serialize 
 * \ingroup CDRModule 
 * \brief Serialization to and deserialization from CDR streams 
 *  
 * \details 
 * Operations to serialize types, and sequences and arrays of types, 
 * into CDR streams. 
 * Operations to deserialize types, and sequences and arrays of types, 
 * from CDR streams. 
 * Operations to get sizes of serialized types in CDR streams. 
 */

/*ci \addtogroup CDRSerializeClass
 *   @{
 */
#ifndef cdr_serialize_h
#define cdr_serialize_h

#ifndef reda_sequence_h
#include "reda/reda_sequence.h"
#endif
#ifndef reda_sequenceNumber_h
#include "reda/reda_sequenceNumber.h"
#endif
#ifndef cdr_dll_h
#include "cdr/cdr_dll.h"
#endif
#ifndef cdr_cdr_type_h
#include "cdr/cdr_cdr_type.h"
#endif
#ifndef cdr_stream_h
#include "cdr/cdr_stream.h"
#endif
#ifndef cdr_serialize_impl_h
#include "cdr/cdr_serialize_impl.h"
#endif

/* In 2.4.14 the default behavior is to support variable sized (in memory)
 * enums, but in RTI_CERT this capability has not been certified. Thus, to
 * property support the RTI_CERT profile disable variable sized enums for
 * RTI_CERT.
 */
#ifdef RTI_CERT
#define CDR_VARIABLE_ENUM_ENABLED (0)
#else
#define CDR_VARIABLE_ENUM_ENABLED (1)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*ci \brief Standard CDR serialized parameter alignment */
#define CDR_DEFAULT_PARAMETER_ALIGNMENT (4)

/*ci \brief Get max serialized size of octet */
#define CDR_get_max_size_serialized_octet       CDR_get_1_byte_max_size_serialized

/*ci \brief Get max serialized size of char */
#define CDR_get_max_size_serialized_char        CDR_get_1_byte_max_size_serialized

/*ci \brief Get max serialized size of boolean */
#define CDR_get_max_size_serialized_boolean     CDR_get_1_byte_max_size_serialized

/*ci \brief Get max serialized size of short */
#define CDR_get_max_size_serialized_short       CDR_get_2_byte_max_size_serialized

/*ci \brief Get max serialized size of unsigned short */
#define CDR_get_max_size_serialized_unsigned_short \
                                                CDR_get_2_byte_max_size_serialized

/*ci \brief Get max serialized size of long */
#define CDR_get_max_size_serialized_long        CDR_get_4_byte_max_size_serialized

/*ci \brief Get max serialized size of unsigned long */
#define CDR_get_max_size_serialized_unsigned_long \
                                                CDR_get_4_byte_max_size_serialized

/*ci \brief Get max serialized size of wchar */
#define CDR_get_max_size_serialized_wchar       CDR_get_4_byte_max_size_serialized

/*ci \brief Get max serialized size of enum */
#define CDR_get_max_size_serialized_enum        CDR_get_4_byte_max_size_serialized

/*ci \brief Get max serialized size of float */
#define CDR_get_max_size_serialized_float       CDR_get_4_byte_max_size_serialized

/*ci \brief Get max serialized size of long long */
#define CDR_get_max_size_serialized_long_long   CDR_get_8_byte_max_size_serialized

/*ci \brief Get max serialized size of unsigned long long */
#define CDR_get_max_size_serialized_unsigned_long_long \
                                                CDR_get_8_byte_max_size_serialized

/*ci \brief Get max serialized size of double */
#define CDR_get_max_size_serialized_double      CDR_get_8_byte_max_size_serialized

/*ci \brief Get max serialized size of long double */
#define CDR_get_max_size_serialized_long_double CDR_get_16_byte_max_size_serialized

#if CDR_VARIABLE_ENUM_ENABLED
/*ci \brief Size of an enum with a 1-byte memory representation */
#define ENUM_1BYTE 1

/*ci \brief Size of an enum with a 2-byte memory representation */
#define ENUM_2BYTE 2

/*ci \brief Size of an enum with a 4-byte memory representation */
#define ENUM_4BYTE 4
#endif

/*******************************************************************************
 * Declare serialization / deserialization for base primitive types:
 * UnsignedShort (2 bytes), UnsignedLong (4 bytes), UnsignedLongLong (8 bytes),
 * LongDouble (16 bytes)
 ******************************************************************************/

/*ci 
 * \brief 
 * Serialize an unsigned short 
 *  
 * \param[inout] dest_buffer Serialization buffer
 * \param[in] instance Value to serialize 
 * \param[in] byte_swap Flag whether to byte swap when serializing
 *   
 */
CDRDllExport void
CDR_serialize_unsigned_short(char **dest_buffer, const RTI_UINT16 *instance,
                           RTI_BOOL byte_swap);

/*ci 
 * \brief 
 * Deserialize an unsigned short 
 *  
 * \param[inout] src_buffer Deserialization buffer
 * \param[out] instance Deserialized instance 
 * \param[in] byte_swap Flag whether to byte swap when deserializing
 *   
 */
CDRDllExport void
CDR_deserialize_unsigned_short(char **src_buffer, RTI_UINT16 *instance,
                             RTI_BOOL byte_swap);

/*ci 
 * \brief 
 * Deserialize an unsigned short from big endian byte order
 *
 * \param[inout] src_buffer Deserialization buffer
 * \param[out] instance Deserialized instance
 *
 */
CDRDllExport void
CDR_deserialize_unsigned_short_from_big_endian(char **src_buffer,
                                               RTI_UINT16 *instance);

/*ci
 * \brief
 * Serialize an unsigned long
 *
 * \param[inout] dest_buffer Serialization buffer
 * \param[in] instance Value to serialize 
 * \param[in] byte_swap Flag whether to byte swap when serializing
 *   
 */
CDRDllExport void
CDR_serialize_unsigned_long(char **dest_buffer, const RTI_UINT32 *instance,
                          RTI_BOOL byte_swap);

/*ci 
 * \brief 
 * Deserialize an unsigned long 
 *  
 * \param[inout] src_buffer Deserialization buffer
 * \param[out] instance Deserialized instance 
 * \param[in] byte_swap Flag whether to byte swap when deserializing
 *   
 */
CDRDllExport void
CDR_deserialize_unsigned_long(char **src_buffer, RTI_UINT32 *instance,
                            RTI_BOOL byte_swap);

/*ci 
 * \brief 
 * Serialize an unsigned long to big endian byte order
 *  
 * \param[inout] dest_buffer Serialization buffer
 * \param[in] instance Value to serialize 
 *   
 */
CDRDllExport void
CDR_serialize_unsigned_long_to_big_endian(char **dest_buffer,
                                          const RTI_UINT32 *instance);

/*ci 
 * \brief 
 * Deserialize an unsigned long from big endian byte order 
 *  
 * \param[inout] src_buffer Deserialization buffer
 * \param[out] instance Deserialized instance 
 *   
 */
CDRDllExport void
CDR_deserialize_unsigned_long_from_big_endian(char **src_buffer,
                                              RTI_UINT32 *instance);

/*ci 
 * \brief 
 * Serialize an unsigned long long 
 *  
 * \param[inout] dest_buffer Serialization buffer
 * \param[in] instance Value to serialize 
 * \param[in] byte_swap Flag whether to byte swap when serializing
 *   
 */
CDRDllExport void
CDR_serialize_unsigned_long_long(char **dest_buffer, const RTI_UINT64 *instance,
                                 RTI_BOOL byte_swap);

/*ci 
 * \brief 
 * Deserialize an unsigned long long 
 *  
 * \param[inout] src_buffer Deserialization buffer
 * \param[out] instance Deserialized instance 
 * \param[in] byte_swap Flag whether to byte swap when deserializing
 *   
 */
CDRDllExport void
CDR_deserialize_unsigned_long_long(char **src_buffer, RTI_UINT64 *instance,
                                   RTI_BOOL byte_swap);

/*ci
 * \brief
 * Deserialize an unsigned long long from big endian byte order
 *
 * \param[inout] src_buffer Deserialization buffer
 * \param[out] instance Deserialized instance
 *
 */
CDRDllExport void
CDR_deserialize_unsigned_long_long_from_big_endian(char **src_buffer,
                                                   RTI_UINT64 *instance);

/*ci
 * \brief
 * Serialize an unsigned long long to big endian byte order
 *
 * \param[inout] dest_buffer Serialization buffer
 * \param[in] instance Value to serialize
 *
 */
CDRDllExport void
CDR_serialize_unsigned_long_long_to_big_endian(char **dest_buffer,
                                               const RTI_UINT64 *instance);
/*ci 
 * \brief 
 * Serialize a long double 
 *  
 * \param[inout] dest_buffer Serialization buffer
 * \param[in] instance Value to serialize 
 * \param[in] byte_swap Flag whether to byte swap when serializing
 *   
 */
CDRDllExport void
CDR_serialize_long_double(char **dest_buffer, const RTI_DOUBLE128 *instance,
                          RTI_BOOL byte_swap);

/*ci 
 * \brief 
 * Deserialize a long double 
 *  
 * \param[inout] src_buffer Deserialization buffer
 * \param[out] instance Deserialized instance 
 * \param[in] byte_swap Flag whether to byte swap when deserializing
 *   
 */
CDRDllExport void
CDR_deserialize_long_double(char **src_buffer, RTI_DOUBLE128 *instance,
                            RTI_BOOL byte_swap);

/******************************************************************************
 * Define serialization / deserialization macros for primitive types that are
 * implemented by equivalent sized base primitive types
 ******************************************************************************/

/*ci \brief Deserialize Long */
#define CDR_deserialize_long(__buf,__inst,_b) \
    CDR_deserialize_unsigned_long(__buf,((RTI_UINT32*)__inst),_b)

/*******************************************************************************
 * Declare stream serialization / deserialization for base primitive types:
 * Char (1 byte), UnsignedShort (2 bytes), UnsignedLong (4 bytes),
 * UnsignedLongLong (8 bytes), LongDouble (16 bytes)
 ******************************************************************************/

/*ci 
 * \brief 
 * Serialize a char with a stream
 *  
 * \param[in] cdrs Serialization stream 
 * \param[in] instance Value to serialize 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_serialize_char(struct CDR_Stream_t *cdrs, const RTI_INT8 *instance);

/*ci 
 * \brief 
 * Deserialize a char with a stream 
 *  
 * \param[in] cdrs Deserialization buffer 
 * \param[out] instance Deserialized instance 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_deserialize_char(struct CDR_Stream_t *cdrs, RTI_INT8 *instance);

/*ci 
 * \brief 
 * Serialize an unsigned short with a CDR stream
 *  
 * \param[in] cdrs Serialization stream 
 * \param[in] instance Value to serialize 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure    
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_serialize_unsigned_short(struct CDR_Stream_t *cdrs,
                                    const RTI_UINT16 *instance);

/*ci 
 * \brief 
 * Deserialize an unsigned short with a stream 
 *  
 * \param[in] cdrs Deserialization buffer 
 * \param[out] instance Deserialized instance 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_deserialize_unsigned_short(struct CDR_Stream_t *cdrs,
                                      RTI_UINT16 *instance);

/*ci 
 * \brief 
 * Serialize an unsigned short to big endian byte order with a CDR stream
 *  
 * \param[in] cdrs Serialization stream 
 * \param[in] instance Value to serialize 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure    
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_serialize_unsigned_short_to_big_endian(struct CDR_Stream_t *cdrs,
                                                  const RTI_UINT16 *instance);

/*ci 
 * \brief 
 * Deserialize an unsigned short of big endian byte order with a stream 
 *  
 * \param[in] cdrs Deserialization buffer 
 * \param[out] instance Deserialized instance 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_deserialize_unsigned_short_from_big_endian(struct CDR_Stream_t *cdrs,
                                                      RTI_UINT16 *instance);

/*ci 
 * \brief 
 * Serialize an unsigned long with a CDR stream
 *  
 * \param[in] cdrs Serialization stream 
 * \param[in] instance Value to serialize 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure    
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_serialize_unsigned_long(struct CDR_Stream_t *cdrs,
                                   const RTI_UINT32 *instance);

/*ci 
 * \brief 
 * Deserialize an unsigned long with a stream 
 *  
 * \param[in] cdrs Deserialization buffer 
 * \param[out] instance Deserialized instance 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_deserialize_unsigned_long(struct CDR_Stream_t *cdrs,
                                     RTI_UINT32 *instance);

/*ci 
 * \brief 
 * Serialize an unsigned long to big endian byte order with a CDR stream
 *  
 * \param[in] cdrs Serialization stream 
 * \param[in] instance Value to serialize 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure    
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_serialize_unsigned_long_to_big_endian(struct CDR_Stream_t *cdrs,
                                                 const RTI_UINT32 *instance);

/*ci 
 * \brief 
 * Deserialize an unsigned long with big endian byte order from a stream 
 *  
 * \param[in] cdrs Deserialization buffer 
 * \param[out] instance Deserialized instance 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_deserialize_unsigned_long_from_big_endian(struct CDR_Stream_t *cdrs,
                                                     RTI_UINT32 *instance);

/*ci 
 * \brief 
 * Serialize an unsigned long long with a CDR stream
 *  
 * \param[in] cdrs Serialization stream 
 * \param[in] instance Value to serialize 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure    
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_serialize_unsigned_long_long(struct CDR_Stream_t *cdrs,
                                        const RTI_UINT64 *instance);

/*ci 
 * \brief 
 * Deserialize an unsigned long long from a stream 
 *  
 * \param[in] cdrs Deserialization buffer 
 * \param[out] instance Deserialized instance 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_deserialize_unsigned_long_long(struct CDR_Stream_t *cdrs,
                                          RTI_UINT64 *instance);

/*ci 
 * \brief 
 * Serialize a long double with a CDR stream
 *  
 * \param[in] cdrs Serialization stream 
 * \param[in] in Value to serialize 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure    
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_serialize_long_double(struct CDR_Stream_t *cdrs,
                                 const RTI_DOUBLE128 *in);

/*ci 
 * \brief 
 * Deserialize a long double from a stream 
 *  
 * \param[in] cdrs Deserialization buffer 
 * \param[out] out Deserialized instance 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_deserialize_long_double(struct CDR_Stream_t *cdrs,
                                   RTI_DOUBLE128 *out);

/*ci 
 * \brief 
 * Serialize a string with a CDR stream
 *  
 * \param[in] cdrs Serialization stream 
 * \param[in] in Pointer to string 
 * \param[in] max_length Maximum length of string
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure    
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_serialize_string(struct CDR_Stream_t *cdrs,
                            const char *in, RTI_UINT32 max_length);

/*ci 
 * \brief 
 * Deserialize a string from a stream 
 *  
 * \param[in] cdrs Deserialization buffer 
 * \param[in] out Deserialized instance
 * \param[in] max_length Maximum length of string
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_deserialize_string(struct CDR_Stream_t *cdrs,
                              char *out, RTI_UINT32 max_length);

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
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_serialize_wstring(struct CDR_Stream_t *cdrs,
                           const RTI_UINT32 *in, RTI_UINT32 max_length);

/*ci 
 * \brief 
 * Deserialize a wstring from a stream 
 *  
 * \param[in] cdrs Deserialization buffer 
 * \param[in] out Deserialized wstring
 * \param[in] max_length Maximum wstring length
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_deserialize_wstring(struct CDR_Stream_t *cdrs,
                             RTI_UINT32 *out, RTI_UINT32 max_length);

#if CDR_VARIABLE_ENUM_ENABLED
/*******************************************************************************
 * Declare stream serialization / deserialization for primitive types with 
 * variable memory representation size:
 * enum (variable size)
 ******************************************************************************/

/*ci 
 * \brief Serialize an enum by value to a CDR stream
 *  
 * \param[in] cdrs Serialization stream 
 * \param[in] instance Value to serialize 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure    
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_serialize_enum_value(struct CDR_Stream_t *cdrs, RTI_UINT32 instance);

/*ci 
 * \brief Serialize an enum by reference to a CDR stream
 *  
 * \param[in] cdrs Serialization stream 
 * \param[in] instance Address of enum to serialize 
 * \param[in] size Size of instance in bytes
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure    
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_serialize_enum(
        struct CDR_Stream_t *cdrs,
        void *instance,
        RTI_UINT32 size);

/*ci 
 * \brief Deserialize an enum from a CDR stream
 *  
 * \param[in] cdrs Serialization stream 
 * \param[out] instance Enum to deserialize
 * \param[in] size Size of instance in bytes
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure    
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_deserialize_enum(
        struct CDR_Stream_t *cdrs,
        void *instance,
        RTI_UINT32 size);

#endif

/******************************************************************************
 * Define serialization / deserialization stream macros for primitive types that
 * are implemented by equivalent sized base primitive types
 ******************************************************************************/

/*ci \brief Serialize boolean in stream */
#define CDR_Stream_serialize_boolean(__s,__i) \
    CDR_Stream_serialize_char(__s,((RTI_INT8*)__i))

/*ci \brief Deserialize boolean from stream */
#define CDR_Stream_deserialize_boolean(__s,__i) \
    CDR_Stream_deserialize_char(__s,((RTI_INT8*)__i))

/*ci \brief Serialize octet into stream */
#define CDR_Stream_serialize_octet(__s,__i) \
    CDR_Stream_serialize_char(__s,((RTI_INT8*)__i))

/*ci \brief Deserialize octet from stream */
#define CDR_Stream_deserialize_octet(__s,__i) \
    CDR_Stream_deserialize_char(__s,((RTI_INT8*)__i))

/*ci \brief Serialize short into stream */
#define CDR_Stream_serialize_short(__s,__i) \
    CDR_Stream_serialize_unsigned_short(__s,((RTI_UINT16*)__i))

/*ci \brief Deserialize short from stream */
#define CDR_Stream_deserialize_short(__s,__i) \
    CDR_Stream_deserialize_unsigned_short(__s,((RTI_UINT16*)__i))

/*ci \brief Serialize wchar into stream */
#define CDR_Stream_serialize_wchar(__s,__i) \
    CDR_Stream_serialize_unsigned_long(__s,(RTI_UINT32*)__i)

/*ci \brief Deserialize wchar from stream */
#define CDR_Stream_deserialize_wchar(__s,__i) \
    CDR_Stream_deserialize_unsigned_long(__s,(RTI_UINT32*)__i)

#if !CDR_VARIABLE_ENUM_ENABLED
/*ci \brief Serialize enum into stream */
#define CDR_Stream_serialize_enum(__s,__i) \
    CDR_Stream_serialize_unsigned_long(__s,(RTI_UINT32*)__i)

/*ci \brief Deserialize enum from stream */
#define CDR_Stream_deserialize_enum(__s,__i) \
    CDR_Stream_deserialize_unsigned_long(__s,(RTI_UINT32*)__i)
#endif

/*ci \brief Serialize long into stream */
#define CDR_Stream_serialize_long(__s,__i) \
    CDR_Stream_serialize_unsigned_long(__s,((RTI_UINT32*)__i))

/*ci \brief Deserialize long from stream */
#define CDR_Stream_deserialize_long(__s,__i) \
    CDR_Stream_deserialize_unsigned_long(__s,((RTI_UINT32*)__i))

/*ci \brief Serialize float into stream */
#define CDR_Stream_serialize_float(__s,__i) \
    CDR_Stream_serialize_unsigned_long(__s,((RTI_UINT32*)__i))

/*ci \brief Deserialize float from stream */
#define CDR_Stream_deserialize_float(__s,__i) \
    CDR_Stream_deserialize_unsigned_long(__s,((RTI_UINT32*)__i))

/*ci \brief Serialize long long into stream */
#define CDR_Stream_serialize_long_long(__s,__i) \
    CDR_Stream_serialize_unsigned_long_long(__s,((RTI_UINT64*)__i))

/*ci \brief Deserialize long long from stream */
#define CDR_Stream_deserialize_long_long(__s,__i) \
    CDR_Stream_deserialize_unsigned_long_long(__s,((RTI_UINT64*)__i))

/*ci \brief Serialize double into stream */
#define CDR_Stream_serialize_double(__s,__i) \
    CDR_Stream_serialize_unsigned_long_long(__s,((RTI_UINT64*)__i))

/*ci \brief Deserialize double from stream */
#define CDR_Stream_deserialize_double(__s,__i) \
    CDR_Stream_deserialize_unsigned_long_long(__s,((RTI_UINT64*)__i))

/*ci \brief Serialization function for an arbitrary type 
 *
 * \param[in] stream Serialization stream
 * \param[in] sample Pointer to type to serialize
 * \param[in] param Serialization function parameter
 * 
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
typedef RTI_BOOL
(*CDR_Stream_SerializeFunction) (struct CDR_Stream_t *stream,
                                 const void *sample,
                                 void *param);
   
/*ci \brief Deserialization function for an arbitrary type 
 *
 * \param[in] stream Deserialization stream
 * \param[inout] sample Deserialized type from stream 
 * \param[in] param Deserialization function parameter
 * 
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */                                                   
typedef RTI_BOOL
(*CDR_Stream_DeserializeFunction) (struct CDR_Stream_t *stream,
                                   void *sample,
                                   void *param);

struct NDDS_Type_Plugin;            /* forward declaration */

/*ci \brief Get serialized size of stream buffer with a sample of a custom type 
 *   with a type-plugin 
 *  
 * \param[in] plugin Custom type's type-plugin 
 * \param[in] current_alignment Stream buffer's current alignment 
 * \param[in] param Optional function parameter
 *  
 * \return Number of bytes to serialize one sample of plugin's type 
 */
typedef RTI_UINT32
(*CDR_Stream_GetSerializedSizeFunction)(struct NDDS_Type_Plugin *plugin,
                                        RTI_UINT32 current_alignment,
                                        void *param);

/******************************************************************************/
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
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_serialize_byte_array(struct CDR_Stream_t *cdrs,
                                const unsigned char *in_array,
                                RTI_UINT32 length);

/*ci 
 * \brief 
 * Deserialize an array of bytes from a stream 
 *  
 * \param[in] cdrs Deserialization stream 
 * \param[in] out Array of deserialized elements
 * \param[in] length Length of array 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_deserialize_byte_array(struct CDR_Stream_t *cdrs,
                                  unsigned char *out,
                                  RTI_UINT32 length);

/*ci \brief Get serialized size of byte array */
#define CDR_get_max_size_serialized_byte_array(current_alignment, length, type) \
  (length)

/*ci 
 * \brief 
 * Serialize array of strings with stream 
 *  
 * \param[in] cdrs Serialization stream 
 * \param[in] in Array containing elements to serialize 
 * \param[in] length Length of array 
 * \param[in] max_string_length Maximum length of string
 * \param[in] type Type of string, char or wide char 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_serialize_string_array(struct CDR_Stream_t *cdrs,
                                  const void *in, 
                                  RTI_UINT32 length,
                                  RTI_UINT32 max_string_length,
                                  CdrPrimitiveType type);

/*ci 
 * \brief 
 * Deserialize an array of strings from a stream 
 *  
 * \param[in] cdrs Deserialization stream 
 * \param[out] out Array of deserialized strings 
 * \param[in] length Length of array 
 * \param[in] max_string_length Maximum length of string
 * \param[in] type Type of string, char or wide char 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_deserialize_string_array(struct CDR_Stream_t *cdrs,
                                    void *out,
                                    RTI_UINT32 length,
                                    RTI_UINT32 max_string_length,
                                    CdrPrimitiveType type);

/*ci 
 * \brief 
 * Return length in bytes of serialized string array 
 *  
 * \param[in] current_alignment Alignment of serialization buffer pointer
 * \param[in] length Length of array 
 * \param[in] max_string_length Maximum length of string
 * \param[in] type Type of string, char or wide char 
 *  
 * \return Number of bytes of serialized string array
 */
MUST_CHECK_RETURN CDRDllExport RTI_UINT32
CDR_get_max_size_serialized_string_array(RTI_UINT32 current_alignment,
                                         RTI_UINT32 length,
                                         RTI_UINT32 max_string_length,
                                         CdrPrimitiveType type);

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
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_serialize_primitive_array(struct CDR_Stream_t *cdrs,
                                     const void *in,
                                     RTI_UINT32 length,
                                     CdrPrimitiveType type);

/*ci 
 * \brief 
 * Deserialize an array of primitive type elements from a stream 
 *  
 * \param[in] cdrs Deserialization stream 
 * \param[in] out Array of deserialized elements
 * \param[in] length Length of array 
 * \param[in] type Type of array element 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_deserialize_primitive_array(struct CDR_Stream_t *cdrs,
                                       void *out,
                                       RTI_UINT32 length,
                                       CdrPrimitiveType type);

/*ci \brief Get serialized size of primitive array 
 *
 * \details
 *
 * current_pos is the current position within a buffer. The buffer itself
 * is not important. current_pos is used to determine the total size
 * of the input type, including any alignment adjustments based on
 * current_pos.
 *
 * \param[in] current_pos Current logical position within a buffer
 * \param[in] length Array length
 * \param[in] type Array element type
 * 
 * \return Serialized size of primitive array, in bytes.
 */
MUST_CHECK_RETURN CDRDllExport RTI_UINT32
CDR_get_max_size_serialized_primitive_array(RTI_UINT32 current_pos,
                                            RTI_UINT32 length, 
                                            CdrPrimitiveType type);

/*ci 
 * \brief 
 * Serialize array of non-primitive type elements with stream 
 *  
 * \param[in] stream Serialization stream 
 * \param[in] in Array pointer 
 * \param[in] length Length of array 
 * \param[in] element_size Size in bytes of an array element
 * \param[in] serialize_function Serialization function per array element
 * \param[in] param Parameter for element serialization function
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_serialize_non_primitive_array(struct CDR_Stream_t *stream,
                                         const void *in,
                                         RTI_UINT32 length,
                                         RTI_UINT32 element_size,
                                         CDR_Stream_SerializeFunction serialize_function,
                                         void *param);

/*ci 
 * \brief 
 * Deserialize an array of non-primitive type elements from a stream 
 *  
 * \param[in] stream Deserialization stream 
 * \param[in] out Array of deserialized elements
 * \param[in] length Length of array 
 * \param[in] element_size Size in bytes of an array element
 * \param[in] deserialize_function Deserialization function per array element
 * \param[in] param Parameter for element deserialization function
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_deserialize_non_primitive_array(struct CDR_Stream_t *stream,
                                           void *out,
                                           RTI_UINT32 length,
                                           RTI_UINT32 element_size,
                                           CDR_Stream_DeserializeFunction deserialize_function,
                                           void *param);

/*ci 
 * \brief 
 * Return length in bytes of serialized non-primitive array 
 *  
 * \param[in] current_pos Current logical position within buffer
 * \param[in] length Length of array 
 * \param[in] get_serialized_size_func Function returning serialized size of one
 * element 
 * \param[in] param Parameter for element serialized size function 
 *  
 * \return Number of bytes to serialize array of non-primitive elements 
 */
MUST_CHECK_RETURN CDRDllExport RTI_UINT32
CDR_get_max_size_serialized_non_primitive_array(RTI_UINT32 current_pos,
                                                RTI_UINT32 length,
                                                CDR_Stream_GetSerializedSizeFunction get_serialized_size_func,
                                                void *param);

/*ci 
 * \brief 
 * Serialize a sequence of char or wide char strings with a stream 
 *  
 * \param[in] cdrs Serialization stream 
 * \param[in] in Sequence of strings 
 * \param[in] max_string_length Maximum length of a string
 * \param[in] type Type of string, either char or wide char 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_serialize_string_sequence(struct CDR_Stream_t *cdrs,
                                     const struct REDA_Sequence *in,
                                     RTI_UINT32 max_string_length,
                                     CdrPrimitiveType type);

/*ci 
 * \brief 
 * Deserialize a sequence of char or wide char strings from a stream 
 *  
 * \param[in] cdrs Deserialization stream 
 * \param[inout] out Sequence of deserialized strings
 * \param[in] max_string_length Maximum length of a string
 * \param[in] type Type of string, either char or wide char 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_deserialize_string_sequence(struct CDR_Stream_t *cdrs,
                                       struct REDA_Sequence *out,
                                       RTI_UINT32 max_string_length,
                                       CdrPrimitiveType type);

/*ci 
 * \brief 
 * Return length in bytes of serialized string sequence  
 *  
 * \param[in] current_alignment Alignment of serialization buffer pointer
 * \param[in] length Length of array 
 * \param[in] max_string_length Maximum length of a string
 * \param[in] Type of string, either char or wide char 
 *  
 * \return Number of bytes to serialize sequence of strings
 */
MUST_CHECK_RETURN CDRDllExport RTI_UINT32
CDR_get_max_size_serialized_string_sequence(RTI_UINT32 current_alignment,
                                            RTI_UINT32 length,
                                            RTI_UINT32 max_string_length,
                                            CdrPrimitiveType type);

/*ci 
 * \brief 
 * Serialize a sequence of primitive type elements with a stream 
 *  
 * \param[in] stream Serialization stream 
 * \param[in] in Sequence of primitive elements 
 * \param[in] type Type of sequence element  
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_serialize_primitive_sequence(struct CDR_Stream_t *stream,
                                        const struct REDA_Sequence *in,
                                        CdrPrimitiveType type);

/*ci 
 * \brief 
 * Deserialize a sequence of primitive type elements with a stream 
 *  
 * \param[in] stream Deserialization stream 
 * \param[inout] out Sequence of primitive elements
 * \param[in] type Type of sequence element  
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_deserialize_primitive_sequence(struct CDR_Stream_t *stream,
                                          struct REDA_Sequence *out,
                                          CdrPrimitiveType type);

/*ci 
 * \brief 
 * Return length in bytes of serialized primitive sequence
 *
 * \details
 *
 * current_pos is the current position within a buffer. The buffer itself
 * is not important. current_pos is used to determine the total size
 * of the input type, including any alignment adjustments based on
 * current_pos.
 *
 * \param[in] current_pos Current logical position within a buffer
 * \param[in] length Length of sequence 
 * \param[in] type Type of sequence element
 *  
 * \return Number of bytes to serialize sequence of primitives
 */
MUST_CHECK_RETURN CDRDllExport RTI_UINT32
CDR_get_max_size_serialized_primitive_sequence(RTI_UINT32 current_alignment,
                                               RTI_UINT32 length,
                                               CdrPrimitiveType type);

/*ci 
 * \brief 
 * Serialize a sequence of non-primitive type elements with a stream 
 *  
 * \param[in] stream Serialization stream 
 * \param[in] in Sequence of non-primitive elements 
 * \param[in] serialize_function Serialization function of the non-primitive type
 * \param[in] param Parameter for serialize function of non-primitive type 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_serialize_non_primitive_sequence(struct CDR_Stream_t *stream,
                                            const struct REDA_Sequence *in,
                                            CDR_Stream_SerializeFunction serialize_function,
                                            void *param);

/*ci 
 * \brief 
 * Deserialize a sequence of non-primitive type elements with a stream 
 *  
 * \param[in] stream Deserialization stream 
 * \param[out] out Sequence of non-primitive elements 
 * \param[in] deserialize_function Deserialization function of a non-primitive
 * type  
 * \param[in] param Parameter for deserialize function of non-primitive type 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_deserialize_non_primitive_sequence(struct CDR_Stream_t *stream,
                                              struct REDA_Sequence *out,
                                              CDR_Stream_DeserializeFunction deserialize_function,
                                              void *param);

/*ci 
 * \brief 
 * Return the length in bytes to serialize a sequence of non-primitive type 
 * elements 
 *  
 * \param[in] current_alignment Alignment of serialization buffer
 * \param[in] length Length of sequence
 * \param[in] get_serialized_size_func Function returning the length in bytes of
 *            one serialized sequence element type  
 * \param[in] param Parameter for serialized size function of non-primitive type 
 *  
 * \return The maximum number of bytes required to serialize the sequence
 *         based on a current alignment.
 */
MUST_CHECK_RETURN CDRDllExport RTI_UINT32
CDR_get_max_size_serialized_non_primitive_sequence(RTI_UINT32 current_alignment,
               RTI_UINT32 length,
               CDR_Stream_GetSerializedSizeFunction get_serialized_size_func,
               void *param);

/*ci 
 * \brief 
 * Serialize a sequence of properties with a stream 
 *  
 * \param[in] stream Serialization stream
 * \param[in] data Sequence of properties to serialize
 * \param[in] param Unused
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_serialize_property_sequence(struct CDR_Stream_t *stream,
                                       const void *data,
                                       void *param);

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
MUST_CHECK_RETURN CDRDllExport RTI_UINT32
CDR_get_pad_size(RTI_UINT32 current_size, RTI_UINT8 align);


/*ci \brief Align current location to specified alignment 
 * \param[in] location Current position 
 * \param[in] alignment Desired alignment 
 *  
 * \return New position, at or greater than input location, at the desired 
 *         alignment 
 */
MUST_CHECK_RETURN CDRDllExport RTI_UINT32 
CDR_align_upwards(RTI_UINT32 location, 
                  RTI_UINT8 alignment);

/*ci \brief Serialize a byte into a stream
 *
 * \param[in] me Serialization stream
 * \param[in] in Byte to serialize 
 *  
 */
CDRDllExport void 
CDR_Stream_serialize_1_byte(struct CDR_Stream_t *me, 
                            const RTI_INT8 *in);     

/*ci \brief Deserialize a byte from a stream
 *
 * \param[in] me Deserialization stream 
 * \param[inout] out Deserialized byte 
 *  
 */
CDRDllExport void 
CDR_Stream_deserialize_1_byte(struct CDR_Stream_t *me, 
                              RTI_INT8 *out);  


/*ci \brief Get size of serialized and aligned short 
 *
 * \param[in] current_size Current position 
 *  
 * \return Size of serialized and aligned short 
 */
MUST_CHECK_RETURN CDRDllExport RTI_UINT32 
CDR_get_2_byte_max_size_serialized(RTI_UINT32 current_size);

/*ci \brief Get size of serialized and aligned long 
 *
 * \param[in] current_size Current position 
 *  
 * \return Size of serialized and aligned long 
 */
MUST_CHECK_RETURN CDRDllExport RTI_UINT32 
CDR_get_4_byte_max_size_serialized(RTI_UINT32 current_size);

/*ci \brief Get size of serialized and aligned long long 
 *
 * \param[in] current_size Current position 
 *  
 * \return Size of serialized and aligned long long 
 */
MUST_CHECK_RETURN CDRDllExport RTI_UINT32 
CDR_get_8_byte_max_size_serialized(RTI_UINT32 current_size); 

/*ci \brief Get size of serialized and aligned long double 
 *
 * \param[in] current_size Current position 
 *  
 * \return Size of serialized and aligned long double 
 */
MUST_CHECK_RETURN CDRDllExport RTI_UINT32 
CDR_get_16_byte_max_size_serialized(RTI_UINT32 current_size);

/*ci \brief Get size of serialized and aligned string 
 *
 * \param[in] current_size Current position 
 * \param[in] length String length  
 *  
 * \return Size of serialized and aligned string 
 */
MUST_CHECK_RETURN CDRDllExport RTI_UINT32 
CDR_get_max_size_serialized_string(RTI_UINT32 current_size, 
                                   RTI_UINT32 length);

/*ci \brief Get size of serialized and aligned wstring
 *
 * \param[in] current_size Current position 
 * \param[in] length Wstring length, in Wchars  
 *  
 * \return Size of serialized and aligned wstring 
 */
MUST_CHECK_RETURN CDRDllExport RTI_UINT32 
CDR_get_max_size_serialized_wstring(RTI_UINT32 current_size, 
                                    RTI_UINT32 length);


/*ci 
 * \brief 
 * Whether the stream has enough space for additional bytes
 *  
 * \param[in] cdrs Stream 
 * \param[in] needed_space Additional bytes needed 
 *  
 * \return RTI_TRUE if stream has enough free space to serialize or deserialize 
 * needed_space more bytes, otherwise RTI_FALSE 
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL 
CDR_Stream_has_remaining_space(struct CDR_Stream_t *cdrs,
                          RTI_UINT32 needed_space);

#ifdef __cplusplus
}                               /* extern "C" */
#endif


#endif /* cdr_serialize_h */

/*ci @} */
