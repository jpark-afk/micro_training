/*
 * FILE: dds_c_common.h - DDS_C common definitions
 *
 * (c) Copyright, Real-Time Innovations, 2008-2015.
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
typedef CDR_Wchar DDS_Wchar;

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
typedef CDR_Wstring DDS_Wstring;

/*e
 * \dref_BOOLEAN_TRUE
 */
#define DDS_BOOLEAN_TRUE    ((DDS_Boolean) 1)

/*e
 * \dref_BOOLEAN_FALSE
 */
#define DDS_BOOLEAN_FALSE   ((DDS_Boolean) 0)

/* ================================================================= */
/*                    NATIVE Representation                          */
/* ================================================================= */

/*i @ingroup DDSInfrastructureModule
 *  @brief Defines the native language representation of a
 *         DDS_DomainId_t
 */
#define DDS_DOMAINID_TYPE_NATIVE    DDS_Long

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

#ifdef __cplusplus
}                               /* extern "C" */
#endif


#endif /* dds_c_common_h */
