/*
 * FILE: Cdr_Type.h - CDR types
 *
 * (c) Copyright, Real-Time Innovations, 2012-2021.
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
 * 21feb2021,tk MICRO-2825/PR#28600
 *   - Removed dead code in CDR_Wstring_copy().
 * 10dec2020,tk
 *     - MICRO-2737/PR#28432 Use correct return type and names in function
 *       header comments.
 *     - Minor updates for coding standard compliance.
 *     - MICRO-2712/PR#28305
 *       - Removed duplicate function header comments already in the public
 *         header-files.
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

/*************************************************/

/* NOTE:
 * This is an unmanaged sequence, it is not a public sequence
 * meaning that the content of the structure is 100% managed by
 * the middle-ware. Only a limited number of functions are exposed.
 */
#define T    struct CDR_Property
#define TSeq CDR_PropertySeq
#define REDA_SEQUENCE_API   REDA_SEQUENCE_API_UNTYPED
#define TSeq_initialize
#define TSeq_get_length
#define TSeq_set_length
#define TSeq_get_reference
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

RTI_BOOL
CDR_String_initialize(CDR_String *string, RTI_UINT32 max_str_len)
{
    if (*string != NULL)
    {
        return RTI_FALSE;
    }

    *string = REDA_String_alloc(max_str_len);
    if (*string == NULL)
    {
        return RTI_FALSE;
    }

    **string = '\0';
    return RTI_TRUE;
}

#ifndef RTI_CERT
RTI_BOOL
CDR_String_finalize(CDR_String *string)
{
    REDA_String_free(*string);
    *string = NULL;

    return RTI_TRUE;
}
#endif /* !RTI_CERT */

RTI_BOOL
CDR_String_copy(CDR_String *dst, const CDR_String *src, RTI_UINT32 max_str_len)
{
    char *lst_chr;
    RTI_UINT32 len = 0;

    OSAPI_PRECONDITION(*dst == NULL || *src == NULL,
                            return RTI_FALSE,
                             OSAPI_Log_entry_add_pointer("*dst",*dst,RTI_FALSE);
                             OSAPI_Log_entry_add_pointer("*src",*src,RTI_TRUE);)

    /* calculate string length, or max_str_len+1 if unterminated */
    lst_chr = OSAPI_Memory_fndchr(*src, '\0', max_str_len+1);
    len = (lst_chr ? (RTI_UINT32)(lst_chr - *src) : max_str_len + 1U);

    /* max_str_len is IDL length, which does not include terminating NUL
       all strings in CDR samples have been allocated at mxStrLen + 1 */
    if (len >  max_str_len)
    {
        return RTI_FALSE;
    }

    CDR_Primitive_copy_string(*dst, *src, len+1);

    return RTI_TRUE;
}

RTI_INT32
CDR_String_compare(const CDR_String *left, const CDR_String *right)
{
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

    return OSAPI_String_cmp(*left, *right);
}


/*==============================================================================
 * Wstring helper functions
 * ===========================================================================*/

RTI_SIZE_T
CDR_Wstring_length(const CDR_Wchar *wstring)
{      
    RTI_UINT32 i = 0;

    if (wstring == NULL)
    {
        return 0;
    }

    while ((*wstring++) != 0)
    {
        i++;
    }

    return (RTI_SIZE_T) i;
}


RTI_BOOL
CDR_Wstring_initialize(CDR_Wstring *wstring, RTI_UINT32 max_str_len)
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

#ifndef RTI_CERT
RTI_BOOL
CDR_Wstring_finalize(CDR_Wstring *wstring)
{
    OSAPI_Heap_free_array(*wstring);
    *wstring = NULL;

    return RTI_TRUE;
}
#endif /* !RTI_CERT */

RTI_BOOL
CDR_Wstring_copy(CDR_Wstring *dst, const CDR_Wstring *src,
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
    else if (max_str_len == REDA_SEQUENCE_ELEMENT_REPLACE)
    {
        len = CDR_Wstring_length(*src);
    }
    else
    {
        /* calculate string length, or max_str_len+1 if unterminated */
        string = *src;
        while ((len < max_str_len+1) && ((*string++) != 0))
        {
            len++;
        }

        /* max_str_len is IDL length, which does not include terminating NUL
         * all strings in CDR samples have been allocated at mxStrLen + 1 
         */
        if (len > max_str_len)
        {
            return RTI_FALSE;
        }
    }

    CDR_Primitive_copy_wstring(*dst, *src, len+1);

    return RTI_TRUE;
}

RTI_INT32
CDR_Wstring_compare(const CDR_Wstring *left, const CDR_Wstring *right)
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
 *         RTI_FALSE on failure
 */
RTI_BOOL
CDR_StringArray_initialize(void *value,
                           RTI_UINT32 length,
                           RTI_UINT32 max_str_len,
                           CdrPrimitiveType type)
{
    RTI_UINT32 i;
    CDR_String *a_string = NULL;
    CDR_Wstring *a_wstring = NULL;
     
    OSAPI_PRECONDITION(value == NULL,
                            return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("value",value,RTI_TRUE);)

    if (type == CDR_CHAR_TYPE)
    {
        /* value is an array of CDR_String, a_string points to the first
         * in the array.
         */
        a_string = (CDR_String*)value;
        for (i = 0; i < length; i++)
        {
            a_string[i] = NULL;
            if (!CDR_String_initialize(&a_string[i], max_str_len))
            {
                return RTI_FALSE;
            }
        }
        return RTI_TRUE;
    } 

    if (type == CDR_WCHAR_TYPE)
    {
        /* value is an array of CDR_Wstring, a_wstring points to the first
         * in the array.
         */
        a_wstring = (CDR_Wstring*)value;
        for (i = 0; i < length; i++)
        {
            a_wstring[i] = NULL;
            if (!CDR_Wstring_initialize(&a_wstring[i], max_str_len))
            {
                return RTI_FALSE;
            }
        }
        return RTI_TRUE;
    }

    return RTI_FALSE;
}

#ifndef RTI_CERT
RTI_BOOL
CDR_StringArray_finalize(void *value,
                         RTI_UINT32 length,
                         CdrPrimitiveType type)
{
    RTI_UINT32 i;
    CDR_String *a_string = NULL;
    CDR_Wstring *a_wstring = NULL;
    
    OSAPI_PRECONDITION(value == NULL,
                       return RTI_FALSE,
                       OSAPI_Log_entry_add_pointer("value",value,RTI_TRUE);)

    if (type == CDR_CHAR_TYPE)
    {
        /* value is an array of CDR_String, a_string points to the first
         * in the array.
         */
        a_string = (CDR_String*)value;
        for (i = 0; i < length; i++)
        {
            if (!CDR_String_finalize(&a_string[i]))
            {
                return RTI_FALSE;
            }
        }
        return RTI_TRUE;
    }

    if (type == CDR_WCHAR_TYPE)
    {
        /* value is an array of CDR_Wstring, a_wstring points to the first
         * in the array.
         */
        a_wstring = (CDR_Wstring*)value;
        for (i = 0; i < length; i++)
        {
            if (!CDR_Wstring_finalize(&a_wstring[i]))
            {
                return RTI_FALSE;
            }
        }
        return RTI_TRUE;
    }

    return RTI_FALSE;
}
#endif /* !RTI_CERT */
 
RTI_BOOL
CDR_StringArray_copy(void *out, const void *in,
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
