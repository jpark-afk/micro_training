/*
 * FILE: autosarString.c - AutoSAR string functions
 *
 * Copyright 2019-2026 Real-Time Innovations, Inc.
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
 * 13mar2019,fmt Written, based on osapi_string_impl.h
 *
 */
/*ce
 * \file
 * \brief AutoSAR implementation of OSAPI string routines
 */
#include "rti_me_psl.h"

#include <string.h>
#include <ctype.h>
#include <stdlib.h>


/*** SOURCE_BEGIN ***/

FUNC(void, SOAD_CODE)
OSAPI_Memory_copy(P2VAR(void, AUTOMATIC, SOAD_APPL_DATA) dest,
                  P2CONST(void, AUTOMATIC, SOAD_APPL_DATA) src,
                  RTI_SIZE_T size)
{
    (void)memcpy(dest,src,size);
}

FUNC(void, SOAD_CODE)
OSAPI_Memory_zero(P2VAR(void, AUTOMATIC, SOAD_APPL_DATA) mem, RTI_SIZE_T size)
{
    (void)memset(mem,0,size);
}

FUNC(RTI_INT32, SOAD_CODE)
OSAPI_Memory_compare(P2CONST(void, AUTOMATIC, SOAD_APPL_DATA) left,
                     P2CONST(void, AUTOMATIC, SOAD_APPL_DATA) right,
                     RTI_SIZE_T size)
{
    return memcmp(left,right,size);
}

FUNC(void, SOAD_CODE)
OSAPI_Memory_move(P2VAR(void, AUTOMATIC, SOAD_APPL_DATA) dest, 
                  P2CONST(void, AUTOMATIC, SOAD_APPL_DATA) src,
                  RTI_SIZE_T size)
{
    (void)memmove(dest,src,size);
}

FUNC(P2VAR(void, AUTOMATIC, SOAD_APPL_DATA), SOAD_CODE)
OSAPI_Memory_fndchr(P2CONST(void, AUTOMATIC, SOAD_APPL_DATA) s,
                    RTI_INT32 c, RTI_SIZE_T n)
{
    return memchr(s, c, n);
}

FUNC(RTI_SIZE_T, SOAD_CODE)
OSAPI_String_length(P2CONST(char, AUTOMATIC, SOAD_APPL_DATA) s)
{
    return (RTI_SIZE_T)strlen(s);
}

FUNC(RTI_INT32, SOAD_CODE)
OSAPI_String_cmp(P2CONST(char, AUTOMATIC, SOAD_APPL_DATA) left,
                 P2CONST(char, AUTOMATIC, SOAD_APPL_DATA) right)
{
    return strcmp(left,right);
}

FUNC(RTI_INT32, SOAD_CODE)
OSAPI_String_ncmp(P2CONST(char, AUTOMATIC, SOAD_APPL_DATA) left,
                  P2CONST(char, AUTOMATIC, SOAD_APPL_DATA) right, 
                  RTI_SIZE_T num)
{
    return strncmp(left,right,num);
}

RTI_BOOL
OSAPI_String_parse_unsigned_long_long(const char *str, unsigned long long int *result)
{
    char *endptr;
    unsigned long long int value;

    value = strtoull(str, &endptr, 0);
    if (endptr == str)
    {
        return RTI_FALSE;
    }
    *result = value;
    return RTI_TRUE;
}

int
OSAPI_String_char_to_lowercase(const int c)
{
    return tolower((unsigned char)c);
}

char*
OSAPI_String_strchr(const char *s, int c)
{
    return strchr(s,c);
}

FUNC(RTI_SIZE_T, SOAD_CODE)
OSAPI_String_length_w_max(P2CONST(char, AUTOMATIC, SOAD_APPL_DATA) s, RTI_SIZE_T max)
{
    RTI_SIZE_T len = strlen(s);
    
    if (len > max)
    {
        len = max;
    }
    return len;
}
