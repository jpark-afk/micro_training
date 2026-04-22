/*
 * FILE: osapi_string.h - Definition of string interface
 *
 * Copyright (c) 2012-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 15may2014,as MICRO-317 (Verocel PR#1443) Added OSAPI_String_ncmp
 * 12mar2012,tk Written
 */
/*ce
 * \file
 * \brief String interface definition
 */
#ifndef osapi_string_h
#define osapi_string_h

#ifndef osapi_dll_h
#include "osapi/osapi_dll.h"
#endif
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*e \dref_OSAPIMemoryGroupDocs
 */

 /*e \dref_OSAPIStringGroupDocs
 */

/*i \defgroup OSAPI_MemoryClass OSAPI Memory API
 *  \ingroup OSAPIModule
 */

/*e \dref_OSAPI_Memory_copy
*/
OSPSLDllExport void
OSAPI_Memory_copy(void *dest,const void *src, RTI_SIZE_T size);

/*e \dref_OSAPI_Memory_zero
 */
OSPSLDllExport void
OSAPI_Memory_zero(void *mem, RTI_SIZE_T size);

/*e \dref_OSAPI_Memory_compare
 */
MUST_CHECK_RETURN OSPSLDllExport RTI_INT32
OSAPI_Memory_compare(const void *left,const void *right, RTI_SIZE_T size);

/*e \dref_OSAPI_Memory_move
 */
OSPSLDllExport void
OSAPI_Memory_move(void *dest,const void *src, RTI_SIZE_T size);

/*e \dref_OSAPI_Memory_fndchr
 */
MUST_CHECK_RETURN OSPSLDllExport void*
OSAPI_Memory_fndchr(const void *s, RTI_INT32 c, RTI_SIZE_T n);

/*e \defgroup OSAPI_StringClass OSAPI String API
 *  \ingroup OSAPIModule
 */

/*e \dref_OSAPI_String_length
 */
MUST_CHECK_RETURN OSPSLDllExport RTI_SIZE_T
OSAPI_String_length(const char *s);

/*e \dref_OSAPI_String_cmp
 */
MUST_CHECK_RETURN OSPSLDllExport RTI_INT32
OSAPI_String_cmp(const char *left,const char *right);

/*e \dref_OSAPI_String_ncmp
 */
MUST_CHECK_RETURN OSPSLDllExport RTI_INT32
OSAPI_String_ncmp(const char *left,const char *right, RTI_SIZE_T num);

/*e \dref_OSAPI_String_parse_unsigned_long
 */
MUST_CHECK_RETURN OSPSLDllExport RTI_BOOL
OSAPI_String_parse_unsigned_long(const char *str, unsigned int *result);

/*e \dref_OSAPI_String_parse_unsigned_long_long
 */
MUST_CHECK_RETURN OSPSLDllExport RTI_BOOL
OSAPI_String_parse_unsigned_long_long(const char *str, unsigned long long int *result);

/*e \dref_OSAPI_String_char_to_lowercase
 */
MUST_CHECK_RETURN OSPSLDllExport int
OSAPI_String_char_to_lowercase(const int c);

/*e \dref_OSAPI_String_strchr
 */
MUST_CHECK_RETURN OSPSLDllExport char*
OSAPI_String_strchr(const char *s, int c);

/*e \dref_OSAPI_String_length_w_max
 */
MUST_CHECK_RETURN OSPSLDllExport RTI_SIZE_T
OSAPI_String_length_w_max(const char *s,RTI_SIZE_T max);

#if DDS_FILTERING_ENABLED
MUST_CHECK_RETURN OSPSLDllExport RTI_BOOL
OSAPI_String_parse_float(const char *str, RTI_FLOAT32 *result);

MUST_CHECK_RETURN OSPSLDllExport RTI_BOOL
OSAPI_String_parse_double(const char *str, RTI_DOUBLE64 *result);
#endif /* DDS_ENABLE_FILTERING */

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif /* osapi_string_h */
