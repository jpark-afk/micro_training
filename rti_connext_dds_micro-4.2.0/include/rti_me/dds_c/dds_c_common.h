/*
 * FILE: dds_c_common.h - DDS_C common definitions
 *
 * (c) Copyright, Real-Time Innovations, 2008-2025.
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
 * 05jun2012,kaj Add DDS_LongLong, DDS_UnsignedLongLong, DDS_LongDouble
 * 30apr2008,tk  Created
 */
/*ce
 * \file
 * \brief DDS_C common definitions
 */

/*i @addtogroup DDSCommonModule Common Module
 *  @ingroup DDSCModule
 *
 * @brief Common facilities used by the \dds implementation. Defines
 * DDS_DEBUG and so on to facilitate code organization
 * and the development process.
 */
/* ----------------------------------------------------------------- */
/*i @file
  @ingroup DDSCommonModule

  @brief Defines the common facilities used by the \dds implementation.
*/

#ifndef dds_c_common_h
#define dds_c_common_h

#ifndef cdr_cdr_type_h
#include "cdr/cdr_cdr_type.h"
#endif

#include "cdr/cdr_serialize.h"
#include "dds_c/dds_c_dll.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* ================================================================= */
/*                       DDS Primitive Types                         */
/* ================================================================= */

/*e
 * \dref_CdrGroupDocs
 */
/*e
 * \dref_Char
 */
typedef CDR_Char DDS_Char;

/*e
 * \dref_Wchar
 */
#if DDS_ENABLE_WCHAR32
typedef CDR_Wchar DDS_Wchar;
#else
typedef RTI_UINT16 DDS_Wchar;
#endif

/*e
 * \dref_Octet
 */
typedef CDR_Octet DDS_Octet;

/*e
 * \dref_Short
 */
typedef CDR_Short DDS_Short;

/*e
 * \dref_UnsignedShort
 */
typedef CDR_UnsignedShort DDS_UnsignedShort;

/*e
 * \dref_Long
 */
typedef CDR_Long DDS_Long;

/*e
 * \dref_UnsignedLong
 */
typedef CDR_UnsignedLong DDS_UnsignedLong;

/*e
 * \dref_LongLong
 */
typedef CDR_LongLong DDS_LongLong;

/*e
 * \dref_UnsignedLongLong
 */
typedef CDR_UnsignedLongLong DDS_UnsignedLongLong;

/*e
 * \dref_Float
 */
typedef CDR_Float DDS_Float;

/*e
 * \dref_Double
 */
typedef CDR_Double DDS_Double;

/*e
 * \dref_LongDouble
 */
typedef CDR_LongDouble DDS_LongDouble;

/*e
 * \dref_Boolean
 */
typedef CDR_Boolean DDS_Boolean;

/*e
 * \dref_CdrEnum
 */
typedef CDR_Enum DDS_Enum;

/*e
 * \dref_String
 */
typedef CDR_String DDS_String;

/*e
 * \dref_Wstring
 */
typedef DDS_Wchar *DDS_Wstring;

/*e
 * \dref_BOOLEAN_TRUE
 */
#define DDS_BOOLEAN_TRUE    CDR_BOOLEAN_TRUE

/*e
 * \dref_BOOLEAN_FALSE
 */
#define DDS_BOOLEAN_FALSE   CDR_BOOLEAN_FALSE

/* ================================================================= */
/*               Type Specific Initialization & Copy                 */
/* ================================================================= */
/* not all types are atomic so we define macros to emulate as needed */

/*ci \brief Initialize Char
 * \param[inout] value Pointer to value to initialize
 */
#define DDS_Primitive_init(value)              *(value) = 0

/*ci \brief Initialize Long Double
 * \param[inout] value Pointer to value to initialize
 */
#ifdef RTI_HAVE_LONG_DOUBLE
#define DDS_Primitive_init_long_double(value)        *(value) = 0
#else
#define DDS_Primitive_init_long_double(value) \
    OSAPI_Memory_zero(value,sizeof(RTI_DOUBLE128))
#endif

/*ci \brief Initialize String
 * \param[inout] value Pointer to String to initialize
 * \param[in] maxsize Maximum string size
 */
#define DDS_Primitive_init_string(value,maxsize) \
    OSAPI_Memory_zero(value,maxsize)

/*ci \brief Initialize WString
 * \param[inout] value Pointer to WString to initialize
 * \param[in] maxsize Maximum string size
 */
#define DDS_Primitive_init_wstring(value,maxsize) \
    OSAPI_Memory_zero(value,((maxsize)*sizeof(DDS_Wchar)))

/*ci \brief Initialize Array
 * \param[inout] value Pointer to array to initialize
 * \param[in] maxsize Maximum Array length
 */
#define DDS_Primitive_init_array(value,maxsize) \
    OSAPI_Memory_zero(value,maxsize)

/******************************************************************************/
/*ci \brief Copy Char
 * \param[inout] dst Destination of copy
 * \param[in] src Source to copy
 */
#define DDS_Primitive_copy(dst,src)            *(dst) = *(src)

/*ci \brief Copy Long Double
 * \param[inout] dst Destination of copy
 * \param[in] src Source to copy
 */
#ifdef RTI_HAVE_LONG_DOUBLE
#define DDS_Primitive_copy_long_double(dst,src)      *(dst) = *(src)
#else
#define DDS_Primitive_copy_long_double(dst,src) \
    OSAPI_Memory_copy((void*)dst,(void*)src,sizeof(RTI_DOUBLE128))
#endif

/*ci \brief Copy String
 * \param[inout] dst Destination of copy
 * \param[in] src Source to copy
 */
#define DDS_Primitive_copy_string(dst,src,maxsize) \
    OSAPI_Memory_copy((void*)dst,(void*)src,maxsize)

/*ci \brief Copy WString
 * \param[inout] dst Destination of copy
 * \param[in] src Source to copy
 */
#define DDS_Primitive_copy_wstring(dst,src,maxsize) \
    OSAPI_Memory_copy((void*)dst,(void*)src,((maxsize)*CDR_WCHAR_SIZE))

/*ci \brief Copy Array
 * \param[inout] dst Destination of copy
 * \param[in] src Source to copy
 */
#define DDS_Primitive_copy_array(dst,src,maxsize) \
    OSAPI_Memory_copy((void*)dst,(void*)src,maxsize)

/* ================================================================= */
/*                    NATIVE Representation                          */
/* ================================================================= */

/*i @ingroup DDSInfrastructureModule
 *  @brief Defines the native language representation of a
 *         DDS_DomainId_t
 */
#define DDS_DOMAINID_TYPE_NATIVE    DDS_Long

/*e \dref_DomainId_t
 */
typedef DDS_DOMAINID_TYPE_NATIVE DDS_DomainId_t;

/* ----------------------------------------------------------------- */

/*i @ingroup DDSInfrastructureModule
 * @brief Defines the implementation-specific representation of a
 *        DDS_InstanceHandle_t.
 */
typedef struct DDS_HANDLE_TYPE_NATIVE_
{
    DDS_Octet octet[16];
    DDS_Boolean is_valid;
} DDS_HANDLE_TYPE_NATIVE;

/*i @ingroup DDSInfrastructureModule
 *
 * @brief Defines the implementation-specific representation of a
 *        nil instance handle (DDS_HANDLE_NIL).
 *
 */
#define DDS_HANDLE_NIL_NATIVE {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},DDS_BOOLEAN_FALSE}

/*i @ingroup DDSInfrastructureModule
  @brief Defines the native language representation of a
         DDS_BuiltinTopicKey.
*/
#define DDS_BUILTIN_TOPIC_KEY_TYPE_NATIVE    DDS_UnsignedLong



/* BuiltinCDR function */
DDSCDllExport RTI_BOOL
DDS_CdrStream_serialize_non_primitive_parameter(struct CDR_Stream_t *stream,
                                                const void *in,
                                                CDR_Stream_SerializeFunction
                                                serializeFunction,
                                                DDS_UnsignedShort parameterId,
                                                RTI_BOOL serializeEncapsulation,
                                                RTI_BOOL serializeSample);

DDSCDllExport DDS_UnsignedLong
DDS_Cdr_get_parameter_header_max_size_serialized(DDS_UnsignedLong size);

#ifdef __cplusplus
}                               /* extern "C" */
#endif


#endif /* dds_c_common_h */
