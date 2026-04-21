/*
 * FILE: autosarString.c - AutoSAR string functions
 *
 * Copyright 2019-2021 Real-Time Innovations, Inc.
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
#include "osapi/osapi_config.h"

#if OSAPI_INCLUDE_AUTOSAR

#include "osapi/osapi_types.h"
#include "osapi/osapi_string.h"
#include "osapi/osapi_log.h"

#include <string.h>

#endif

/*** SOURCE_BEGIN ***/

#if OSAPI_INCLUDE_AUTOSAR

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

#endif
