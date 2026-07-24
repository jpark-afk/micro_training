/*
 * FILE: Sequence.c - DDS Native Sequences types
 *
 * (c) Copyright, Real-Time Innovations, 2018 - 2018
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#include "osapi/osapi_config.h"
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
#ifndef dds_c_dll_h
#include "dds_c/dds_c_dll.h"
#endif
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_common_h
#include "dds_c/dds_c_common.h"
#endif
#include "dds_c/dds_c_sequence.h"
#include "dds_c/dds_c_string.h"


#define T    DDS_Wchar
#define TSeq DDS_WcharSeq
#define REDA_SEQUENCE_USER_API
#include <reda/reda_sequence_defn.h>

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
DDS_Wstring_initialize(DDS_Wstring *wstring, RTI_UINT32 max_str_len)
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

    OSAPI_Heap_allocate_array(wstring, max_str_len + 1, DDS_Wchar);
    if (*wstring == NULL)
    {
        return RTI_FALSE;
    }

    **wstring = 0;

    return RTI_TRUE;
}

DDS_Wstring
DDS_Wstring_initialize_ex(RTI_UINT32 max_str_len)
{
    DDS_Wstring new_string = NULL;

    if (!DDS_Wstring_initialize(&new_string, max_str_len))
    {
        return NULL;
    }

    return new_string;
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
DDS_Wstring_finalize(DDS_Wstring *wstring)
{
#ifndef RTI_CERT
    OSAPI_Heap_free_array(*wstring);
#endif /* !RTI_CERT */
    *wstring = NULL;
    return RTI_TRUE;
}

void
DDS_Wstring_finalize_ex(DDS_Wstring wstring)
{
    if (!DDS_Wstring_finalize(&wstring))
    {

    }
}
#endif /* !RTI_CERT */


/*ci
 * \brief
 * Returns length in Wchars of a Wstring
 *
 * \param[in] wstring
 *
 * \return Length of Wstring in Wchars
 */
RTI_SIZE_T
DDS_Wstring_length(const DDS_Wchar * wstring)
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
 * Copy from source to destination wide char string
 *
 * \param[inout] dst Destination string
 * \param[in] src Source string
 * \param[in] max_str_len Maximum length of source string
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
DDS_Wstring_copy(DDS_Wstring *dst, const DDS_Wstring *src,
                 RTI_UINT32 max_str_len)
{
    const DDS_Wchar *string;
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
        len = DDS_Wstring_length(*src);
        OSAPI_Heap_allocate_array(dst, len + 1, DDS_Wchar);
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
        len = DDS_Wstring_length(*src);
    }
    else
    {
        return RTI_FALSE;
    }

    OSAPI_Memory_copy((void*)*dst,(void*)*src,(len + 1) * (RTI_SIZE_T)sizeof(DDS_Wchar));

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
DDS_Wstring_compare(const DDS_Wstring * left, const DDS_Wstring * right)
{
    DDS_Wstring left_str, right_str;

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
DDS_StringArray_initialize(void *value,
                           RTI_UINT32 length,
                           RTI_UINT32 max_str_len,
                           CdrPrimitiveType type)
{
    RTI_UINT32 i;
    char **value_char = NULL;
    DDS_Wchar **value_wchar = NULL;

    OSAPI_PRECONDITION(value == NULL,
                            return RTI_FALSE,
                            OSAPI_Log_entry_add_pointer("value",value,RTI_TRUE);)

    if (type == CDR_CHAR_TYPE)
    {
        value_char = (char **)value;
        for (i = 0; i < length; i++)
        {
            value_char[i] = NULL;
            if (!DDS_String_initialize(&value_char[i], max_str_len))
            {
                return RTI_FALSE;
            }
        }
        return RTI_TRUE;
    }
    else if (type==CDR_WCHAR_TYPE)
    {
        value_wchar = (DDS_Wchar **)value;
        for (i = 0; i < length; i++)
        {
            value_wchar[i] = NULL;
            if (!DDS_Wstring_initialize(&value_wchar[i], max_str_len))
            {
                return RTI_FALSE;
            }
        }
        return RTI_TRUE;
    }

    return RTI_FALSE;
}

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
DDS_StringArray_finalize(void *value,
                         RTI_UINT32 length,
                         CdrPrimitiveType type)
{
#ifndef RTI_CERT
    RTI_UINT32 i;
    char  **value_char = NULL;
    DDS_Wchar **value_wchar = NULL;

    OSAPI_PRECONDITION(value == NULL,
                           return RTI_FALSE,
                           OSAPI_Log_entry_add_pointer("value",value,RTI_TRUE);)

    if (type == CDR_CHAR_TYPE)
    {
        value_char = (char **)value;
        for (i = 0; i < length; i++)
        {
            if (!DDS_String_finalize(&value_char[i]))
            {
                return RTI_FALSE;
            }
        }
        return RTI_TRUE;
    }

    if (type == CDR_WCHAR_TYPE)
    {
        value_wchar = (DDS_Wchar **)value;
        for (i = 0; i < length; i++)
        {
            if (!DDS_Wstring_finalize(&value_wchar[i]))
            {
                return RTI_FALSE;
            }
        }
        return RTI_TRUE;
    }

    return RTI_FALSE;

#else /* RTI_CERT */

    UNUSED_ARG(value);
    UNUSED_ARG(length);
    UNUSED_ARG(type);

    return RTI_TRUE;

#endif /* !RTI_CERT*/
}

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
DDS_StringArray_copy(void *out, const void* in,
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
            if (!DDS_String_copy(&((char **)out)[i],
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
            if (!DDS_Wstring_copy(&((DDS_Wchar **)out)[i],
                                  &((DDS_Wchar **)in)[i],max_str_len))
            {
                return RTI_FALSE;
            }
        }
        return RTI_TRUE;
    }

    return RTI_FALSE;
}

#define T            DDS_Wstring
#define TSeq         DDS_WstringSeq
#define T_initialize DDS_Wstring_initialize
#define T_finalize   DDS_Wstring_finalize
#define T_copy       DDS_Wstring_copy
#define TSeq_isCDRStringType
#define TSeq_isCDRStringType_no_max
#define REDA_SEQUENCE_USER_API
#ifndef RTI_CERT
#define T_compare    DDS_Wstring_compare
#define TSeq_is_equal
#endif
#include "reda/reda_sequence_defn.h"
