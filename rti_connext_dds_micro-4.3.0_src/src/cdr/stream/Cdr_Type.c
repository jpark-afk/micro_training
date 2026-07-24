/*
 * FILE: Cdr_Type.h - CDR types
 *
 * (c) Copyright, Real-Time Innovations, 2012-2026.
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 10feb2016,tk  MICRO-1530 Unified REDA_StringSeq, CDR_StringSeq, and DDS_StringSeq
 * 19may2015,as  MICRO-1193 Re-factoring of Sequence API levels
 * 23feb2015,eh  MICRO-1075: remove and replace macros
 * 15jan2015,eh  MICRO-1012/PR#13353 - String_compare return values
 * 15jan2015,eh  MICRO-1013/PR#13354 - NULL each array entry of _initialize
 * 04aug2014,tk  MICRO-846/PR#10203 - Limit exposure of PropertySeq API
 *               MICRO-848/PR#10206 - Limit exposure of PropertySeq API
 * 19aug2013,tk  Additional return code checking
 * 03jun2013,kaj Fix following:
 *               MICRO-502 (use CDR_String_copy in place of REDA_String_replace)
 *               MICRO-192 / MICRO-363 (missing /wrong string compare functions)
 *               MICRO-361 / MICRO-362 (CDR string / Wstring copy doesn't do
 *                   deep copy, doesn't distinguish between empty / NULL string)
 *               MICRO-344 (CDR_Wstring_replace length comparison wrong)
 *               MICRO-166 (Unneeded NULL assign in CDR_StringArray_initialize)
 *               MICRO-167 (Inconsistent definition of string length for string
 *                   and Wstring in CDR_StringArray_initialize)
 *               MICRO-249 (NULL checks needed in CDR W/String external API)
 * 14aug2012,kaj Written
 */
/*ci @ingroup CDRModule
 * \file
 *
 * \brief Functions of CDR Types
 *
 * \details
 * - Declarations of sequences of CDR types
 * - CDR String operations
 */
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif
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

/*** SOURCE_BEGIN ***/

/*==============================================================================
 * Built-In Sequences
 * ===========================================================================*/

/* 1 byte: CDR_Octet */
#define T    CDR_Octet
#define TSeq CDR_OctetSeq
#define TSeq_is_equal
#define TSeq_get_contiguous_buffer
#define TSeq_loan_contiguous
#define REDA_SEQUENCE_USER_API
#include "reda/reda_sequence_defn.h"

#define T CDR_Char
#define TSeq CDR_CharSeq
#define REDA_SEQUENCE_USER_API
#include <reda/reda_sequence_defn.h>

#define T    CDR_Boolean
#define TSeq CDR_BooleanSeq
#define REDA_SEQUENCE_USER_API
#include <reda/reda_sequence_defn.h>

#define T    CDR_Short
#define TSeq CDR_ShortSeq
#define REDA_SEQUENCE_USER_API
#include <reda/reda_sequence_defn.h>

#define T    CDR_Long
#define TSeq CDR_LongSeq
#define REDA_SEQUENCE_USER_API
#include <reda/reda_sequence_defn.h>

#define T    CDR_Enum
#define TSeq CDR_EnumSeq
#define REDA_SEQUENCE_USER_API
#include <reda/reda_sequence_defn.h>

#define T    CDR_Wchar
#define TSeq CDR_WcharSeq
#define REDA_SEQUENCE_USER_API
#include <reda/reda_sequence_defn.h>

#define T    CDR_LongLong
#define TSeq CDR_LongLongSeq
#define REDA_SEQUENCE_USER_API
#include <reda/reda_sequence_defn.h>


#define T    CDR_Float
#define TSeq CDR_FloatSeq
#define REDA_SEQUENCE_USER_API
#include <reda/reda_sequence_defn.h>

#define T    CDR_Double
#define TSeq CDR_DoubleSeq
#define REDA_SEQUENCE_USER_API
#include <reda/reda_sequence_defn.h>

/* 2 bytes: CDR_UnsignedShort */
#define T    CDR_UnsignedShort
#define TSeq CDR_UnsignedShortSeq
#define REDA_SEQUENCE_USER_API
#include "reda/reda_sequence_defn.h"

/* 4 bytes: CDR_UnsignedLong */
#define T    CDR_UnsignedLong
#define TSeq CDR_UnsignedLongSeq
#if DDS_FILTERING_ENABLED
#define TSeq_loan_contiguous
#endif
#define REDA_SEQUENCE_USER_API
#include "reda/reda_sequence_defn.h"

/* 8 bytes: CDR_LongLong */
#define T    CDR_UnsignedLongLong
#define TSeq CDR_UnsignedLongLongSeq
#define REDA_SEQUENCE_USER_API
#include "reda/reda_sequence_defn.h"

/* 16 bytes: CDR_LongDouble */
#define T    CDR_LongDouble
#define TSeq CDR_LongDoubleSeq
#define REDA_SEQUENCE_USER_API
#include "reda/reda_sequence_defn.h"

/*==============================================================================
 * String Sequences
 * ===========================================================================*/
#define T    CDR_Wstring
#define TSeq CDR_WstringSeq
#define T_initialize CDR_Wstring_initialize
#define T_finalize   CDR_Wstring_finalize
#define T_copy       CDR_Wstring_copy
#define TSeq_isCDRStringType
#define TSeq_isCDRStringType_no_max
#define REDA_SEQUENCE_USER_API
#ifndef RTI_CERT
#define T_compare    CDR_Wstring_compare
#define TSeq_is_equal
#endif
#include "reda/reda_sequence_defn.h"

/*==============================================================================
 * String helper functions
 * ===========================================================================*/

/*ci
 * \brief
 * Initialize memory for string
 *
 * \param[inout] string String to initialize
 * \param[in] max_str_len Maximum string length
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
CDR_String_initialize(CDR_String * string, RTI_UINT32 max_str_len)
{
    return REDA_String_initialize_ptr(string, max_str_len);
}

#ifndef RTI_CERT
/*ci
 * \brief
 * Finalize string
 *
 * \details
 * Note, CDR_String_finalize is required to return an RTI_BOOL to comply with
 * the expected interface for finalize functions needed by sequence methods
 *
 * \param[inout] string String to finalize
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
CDR_String_finalize(CDR_String *string)
{
    return REDA_String_finalize_ptr(string);
}
#endif /* !RTI_CERT */

/*ci
 * \brief
 * Copy from source to destination string
 *
 * \param[inout] dst Destination string
 * \param[in] src Source string
 * \param[in] max_str_len Maximum length of destination string
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
CDR_String_copy(CDR_String *dst, const CDR_String *src, RTI_UINT32 max_str_len)
{
    return REDA_String_copy_ptr(dst,
                                       (const char **)src,
                                       max_str_len);
}

/*ci
 * \brief
 * Compare two strings
 *
 * \param[in] left Left string
 * \param[in] right Right string
 *
 * \return 1, 0, or -1 if the left string is greater than, equal to, or less
 * than the right string, respectively.
 */
RTI_INT32
CDR_String_compare(const CDR_String * left, const CDR_String * right)
{
    return REDA_String_compare_ptr((const char **)left,
                                          (const char **)right);
}


/*==============================================================================
 * Wstring helper functions
 * ===========================================================================*/

/*ci
 * \brief
 * Returns length in Wchars of a Wstring
 *
 * \param[in] wstring
 *
 * \return Length of Wstring in Wchars
 */
RTI_SIZE_T
CDR_Wstring_length(const CDR_Wchar * wstring)
{
    RTI_UINT32 i = 0;

    if (wstring == NULL)
    {
        return 0;
    }

    while ((*wstring++) != 0) i++;

    return (RTI_SIZE_T) i;
}

RTI_SIZE_T
CDR_Wstring16_length(const RTI_UINT16 *wstring)
{
    RTI_UINT32 i = 0;

    if (wstring == NULL)
    {
        return 0;
    }

    while ((*wstring++) != 0) i++;

    return (RTI_SIZE_T) i;
}

/*ci
 * \brief
 * Allocates and initializes memory for a wide char string
 *
 * \param[inout] wstring Wstring to initialize
 * \param[in] max_str_len Maximum length of string
 *
 * \return RTI_BOOL on success with wstring initialized, RTI_FALSE on failure.
 */
RTI_BOOL
CDR_Wstring_initialize(CDR_Wstring * wstring, RTI_UINT32 max_str_len)
{
    if (max_str_len == REDA_SEQUENCE_ELEMENT_REPLACE)
    {
        return RTI_TRUE;
    }

    if (max_str_len == REDA_SEQUENCE_ELEMENT_ALLOC)
    {
        *wstring = NULL;
        return RTI_TRUE;
    }

    if (*wstring != NULL)
    {
        return RTI_FALSE;
    }

    OSAPI_Heap_allocate_array(wstring, max_str_len + 1, RTI_UINT32);
    if (*wstring == NULL)
    {
        return RTI_FALSE;
    }

    **wstring = 0;
    return RTI_TRUE;
}

MUST_CHECK_RETURN CDRDllExport CDR_Wstring
CDR_Wstring_initialize_ex(RTI_UINT32 max_str_len)
{
    CDR_Wstring new_string = NULL;
    if (!CDR_Wstring_initialize(&new_string, max_str_len))
    {
        return NULL;
    }
    return new_string;
}

CDRDllExport void
CDR_Wstring_finalize_ex(CDR_Wstring wstring)
{
#ifndef RTI_CERT
    if (!CDR_Wstring_finalize(&wstring))
    {

    }
#else
    UNUSED_ARG(wstring);
#endif
}

#ifndef RTI_CERT
/*ci
 * \brief
 * Finalize a wide char string
 *
 * \details
 * Note, CDR_Wstring_finalize is required to return an RTI_BOOL to comply with
 * the expected interface for finalize functions needed by sequence methods
 *
 * \param[in] wstring Wstring to finalize
 *
 * \return RTI_TRUE
 */
RTI_BOOL
CDR_Wstring_finalize(CDR_Wstring * wstring)
{
#ifndef RTI_CERT
    OSAPI_Heap_free_array(*wstring);
#endif /* !RTI_CERT */
    *wstring = NULL;
    return RTI_TRUE;
}
#endif /* !RTI_CERT */

/*ci
 * \brief
 * Copy from source to destination wide char string
 *
 * \param[inout] dst Destination string
 * \param[in] src Source string
 * \param[in] max_str_len Maximum length of source string
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
CDR_Wstring_copy(CDR_Wstring * dst, const CDR_Wstring * src,
                 RTI_UINT32 max_str_len)
{
    const CDR_Wchar * string;
    RTI_SIZE_T len = 0;

    OSAPI_PRECONDITION((*src == NULL) ||
                    ((max_str_len != REDA_SEQUENCE_ELEMENT_ALLOC) &&
                     (*dst == NULL)),
                    return RTI_FALSE,
                    OSAPI_Log_entry_add_pointer("*dst",*dst,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("*src",*src,RTI_TRUE);)

    if (max_str_len == REDA_SEQUENCE_ELEMENT_ALLOC)
    {
#ifndef RTI_CERT
        if (*dst != NULL)
        {
            OSAPI_Heap_free_array(*dst);
        }
#endif
        len = CDR_Wstring_length(*src);
        OSAPI_Heap_allocate_array(dst, len + 1, RTI_UINT32);
        if (*dst == NULL)
        {
            return RTI_FALSE;
        }
    }
    else if (max_str_len < REDA_SEQUENCE_ELEMENT_ALLOC)
    {
        /* calculate string length, or max_str_len+1 if unterminated */
        string = *src;
        while ((len < max_str_len+1) && ((*string++) != 0)) len++;

        /* max_str_len is IDL length, which does not include terminating NUL
       all strings in CDR samples have been allocated at mxStrLen + 1 */
        if (len >  max_str_len)
        {
            return RTI_FALSE;
        }
    }
    else if (max_str_len == REDA_SEQUENCE_ELEMENT_REPLACE)
    {
        len = CDR_Wstring_length(*src);
    }
    else
    {
        return RTI_FALSE;
    }

    CDR_Primitive_copy_wstring(*dst, *src, len+1);

    return RTI_TRUE;
}

/*ci
 * \brief
 * Compare two wide char strings
 *
 * \param[in] left Left string
 * \param[in] right Right string
 *
 * \return 1, 0, or -1 if the left string is greater than, equal to, or less
 * than the right string, respectively.
 */
RTI_INT32
CDR_Wstring_compare(const CDR_Wstring * left, const CDR_Wstring * right)
{
    CDR_Wstring left_str, right_str;

    if ((left == NULL) || (right == NULL))
    {
        return 0;
    }

    if ((*left == NULL) && (*right == NULL))
    {
        return 0;
    }

    if (*left == NULL)
    {
        /* Non-NULL right is greater than the NULL left */
        return -1;
    }

    if (*right == NULL)
    {
        /* Non-NULL left is greater than the NULL right */
        return 1;
    }

    left_str = *left;
    right_str = *right;

    while (*left_str == *right_str)
    {
       if (*left_str == 0)
       {
           return 0;
       }

       left_str++;
       right_str++;
    }

    return (*left_str < *right_str) ? -1 : 1;
}


/*==============================================================================
 * String Array Functions - support both Strings and Wstrings
 * ===========================================================================*/

/*ci
 * \brief
 * Initialize an array of char or wide char strings
 *
 * \param[inout] value Preallocated buffer for array
 * \param[in] length Length of array
 * \param[in] max_str_len Maximum length of each string
 * \param[in] type Char or Wide Char type of string
 *
 * \return RTI_TRUE on success with each string of array initialized,
 * RTI_FALSE on failure
 */
RTI_BOOL
CDR_StringArray_initialize(void *value,
                           RTI_UINT32 length,
                           RTI_UINT32 max_str_len,
                           CdrPrimitiveType type)
{
    RTI_UINT32 i;
    char **value_char = NULL;
    RTI_UINT32 **value_wchar = NULL;

    OSAPI_PRECONDITION(value == NULL,
                            return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("value",value,RTI_TRUE);)

    if (type == CDR_CHAR_TYPE)
    {
        value_char = (char **)value;
        for (i = 0; i < length; i++)
        {
            value_char[i] = NULL;
            if (!CDR_String_initialize(&value_char[i], max_str_len))
            {
                return RTI_FALSE;
            }
        }
        return RTI_TRUE;
    }
    else if (type==CDR_WCHAR_TYPE)
    {
        value_wchar = (RTI_UINT32 **)value;
        for (i = 0; i < length; i++)
        {
            value_wchar[i] = NULL;
            if (!CDR_Wstring_initialize(&value_wchar[i], max_str_len))
            {
                return RTI_FALSE;
            }
        }
        return RTI_TRUE;
    }

    return RTI_FALSE;
}

#ifndef RTI_CERT
/*ci
 * \brief
 * Finalize an array of char or wide char strings
 *
 * \param[inout] value Array pointer
 * \param[in] length Length of array
 * \param[in] type Char or Wide Char type of string
 *
 * \return RTI_TRUE on success with each string of array finalized,
 * RTI_FALSE on failure
 */
RTI_BOOL
CDR_StringArray_finalize(void *value,
                         RTI_UINT32 length,
                         CdrPrimitiveType type)
{
    RTI_UINT32 i;
    char  **value_char = NULL;
    RTI_UINT32 **value_wchar = NULL;

    OSAPI_PRECONDITION(value == NULL,
                           return RTI_FALSE,
                           OSAPI_Log_entry_add_pointer("value",value,RTI_TRUE);)

    if (type == CDR_CHAR_TYPE)
    {
        value_char = (char **)value;
        for (i = 0; i < length; i++)
        {
            if (!CDR_String_finalize(&value_char[i]))
            {
                return RTI_FALSE;
            }
        }
        return RTI_TRUE;
    }

    if (type == CDR_WCHAR_TYPE)
    {
        value_wchar = (RTI_UINT32 **)value;
        for (i = 0; i < length; i++)
        {
            if (!CDR_Wstring_finalize(&value_wchar[i]))
            {
                return RTI_FALSE;
            }
        }
        return RTI_TRUE;
    }

    return RTI_FALSE;
}
#endif /* !RTI_CERT */

/*ci
 * \brief
 * Copy an array of char or wide char strings to another array
 *
 * \param[out] out Destination array
 * \param[in] in Source array
 * \param[in] max_str_len Maximum string length of source
 * \param[in] type Char or Wide Char type of string
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
CDR_StringArray_copy(void *out, const void* in,
                     RTI_UINT32 length,
                     RTI_UINT32 max_str_len,
                     CdrPrimitiveType type)
{
    RTI_UINT32 i;

    OSAPI_PRECONDITION(out == NULL || in == NULL,
                           return RTI_FALSE,
                           OSAPI_Log_entry_add_pointer("out",out,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("in",in,RTI_TRUE);)

    if (type == CDR_CHAR_TYPE)
    {
        for (i=0; i<length; i++)
        {
            if (!CDR_String_copy(&((char **)out)[i],
                                 &((char **)in)[i],max_str_len))
            {
                return RTI_FALSE;
            }
        }
        return RTI_TRUE;
    }

    if (type == CDR_WCHAR_TYPE)
    {
        for (i=0; i<length; i++)
        {
            if (!CDR_Wstring_copy(&((RTI_UINT32 **)out)[i],
                                  &((RTI_UINT32 **)in)[i],max_str_len))
            {
                return RTI_FALSE;
            }
        }
        return RTI_TRUE;
    }

    return RTI_FALSE;
}
