/*
 * FILE: dds_c_string.h - DDS string definitions
 *
 * (c) Copyright, Real-Time Innovations, 2012-2020.
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
 * 31may2022,tk MICRO-3529/PR.30312
 * - Replaced DDS_Wstring_cmp with DDS_Wstring_compare. The function signature
 *   for DDS_Wstring_compare is kept to keep type information in documentation.
 * 15apr2021,tk MICRO-2979/PR.27549
 * - Removed unused functions from CERT:
 *   DDS_String_replace
 * 13apr2021,tk MICRO-3028/PR.28960
 * - Added DDS_Wstring_cmp as macro for CDR_Wstring_compare.
 * - Moved documentation to external documentation for DDS_Wstring_cmp.
 * - Moved all String documentation to external documentation
 * 15may2014,as MICRO-317 (Verocel PR#1443) Added DDS_String_ncmp
 * 08jun2012,tk Written
 */
/*ce
 * \file
 * \brief DDS string definitions
 */

/*e \dref_StringGroupDocs
 */

/*i @file
  @ingroup String
  @brief Defines the String facilities.
*/
#ifndef dds_c_string_h
#define dds_c_string_h

#ifndef dds_c_dll_h
#include "dds_c/dds_c_dll.h"
#endif
#ifndef dds_c_common_h
#include "dds_c/dds_c_common.h"
#endif
#ifndef dds_c_sequence_h
#include "dds_c/dds_c_sequence.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*e \dref_DDS_String_alloc
 */
DDSCDllExport char*
DDS_String_alloc(DDS_UnsignedLong length);

#define DDS_String_alloc(l_) REDA_String_alloc(l_)

/*e \dref_DDS_String_dup
 */
DDSCDllExport char*
DDS_String_dup(const char *str);

#define DDS_String_dup(str_) REDA_String_dup(str_)

#ifndef RTI_CERT
/*ci \dref_DDS_String_replace
 */
DDSCDllExport char*
DDS_String_replace(char **string_ptr, const char *new_value);
#define DDS_String_replace(str_,val_) REDA_String_replace(str_,val_)
#endif

#ifndef RTI_CERT
/*e \dref_DDS_String_free
 */
DDSCDllExport void
DDS_String_free(char *str);
#endif /* !RTI_CERT */

#ifndef RTI_CERT
#define DDS_String_free(str_) REDA_String_free(str_)
#else
#define DDS_String_free(str_) 
#endif /* !RTI_CERT */

/*e \dref_DDS_String_cmp
 */
DDSCDllExport int
DDS_String_cmp(const char *s1, const char *s2);
#define DDS_String_cmp(str1_,str2_) REDA_String_compare(str1_,str2_)

/*e \dref_DDS_String_ncmp
  */
DDSCDllExport int
DDS_String_ncmp(const char *s1, const char *s2, DDS_UnsignedLong num);
#define DDS_String_ncmp(str1_,str2_,num_) REDA_String_ncompare(str1_,str2_,num_)

/*e \dref_DDS_String_length
 */
DDSCDllExport int
DDS_String_length(const char *string);
#define DDS_String_length(str1_) REDA_String_length(str1_)

/*e \dref_DDS_Wstring_compare
 */
DDSCDllExport int
DDS_Wstring_compare(const DDS_Wstring *s1, const DDS_Wstring *s2);
#define DDS_Wstring_compare(s1_,s2_) CDR_Wstring_compare((s1_),(s2_))


#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif /* dds_c_string_h */
