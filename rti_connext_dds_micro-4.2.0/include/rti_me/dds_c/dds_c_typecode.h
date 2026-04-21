/*
 * FILE: dds_c_typecode - TypeCode definitions
 *
 * Copyright (c) 2018-2025 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef dds_c_typecode_h
#define dds_c_typecode_h

#include "dds_c/dds_c_infrastructure.h"
#include "xcdr/xcdr_typeCode.h"
#include "dds_c/dds_c_dll.h"

#ifdef __cplusplus
    extern "C" {
#endif

#define DDS_TK_NULL             RTI_XCDR_TK_NULL
#define DDS_TK_SHORT            RTI_XCDR_TK_SHORT
#define DDS_TK_LONG             RTI_XCDR_TK_LONG
#define DDS_TK_USHORT           RTI_XCDR_TK_USHORT
#define DDS_TK_ULONG            RTI_XCDR_TK_ULONG
#define DDS_TK_FLOAT            RTI_XCDR_TK_FLOAT
#define DDS_TK_DOUBLE           RTI_XCDR_TK_DOUBLE
#define DDS_TK_BOOLEAN          RTI_XCDR_TK_BOOLEAN
#define DDS_TK_CHAR             RTI_XCDR_TK_CHAR
#define DDS_TK_OCTET            RTI_XCDR_TK_OCTET
#define DDS_TK_STRUCT           RTI_XCDR_TK_STRUCT
#define DDS_TK_UNION            RTI_XCDR_TK_UNION
#define DDS_TK_ENUM             RTI_XCDR_TK_ENUM
#define DDS_TK_STRING           RTI_XCDR_TK_STRING
#define DDS_TK_SEQUENCE         RTI_XCDR_TK_SEQUENCE
#define DDS_TK_ARRAY            RTI_XCDR_TK_ARRAY
#define DDS_TK_ALIAS            RTI_XCDR_TK_ALIAS
#define DDS_TK_LONGLONG         RTI_XCDR_TK_LONGLONG
#define DDS_TK_ULONGLONG        RTI_XCDR_TK_ULONGLONG
#define DDS_TK_LONGDOUBLE       RTI_XCDR_TK_LONGDOUBLE
#define DDS_TK_WCHAR            RTI_XCDR_TK_WCHAR
#define DDS_TK_WSTRING          RTI_XCDR_TK_WSTRING
#define DDS_TK_VALUE            RTI_XCDR_TK_VALUE
#define DDS_TK_SPARSE           RTI_XCDR_TK_SPARSE
#define DDS_TK_RAW_BYTES        RTI_XCDR_TK_RAW_BYTES
#define DDS_TK_RAW_BYTES_KEYED  RTI_XCDR_TK_RAW_BYTES_KEYED

#define DDS_TK_FINAL_EXTENSIBILITY         RTI_XCDR_TK_FLAGS_IS_FINAL
#define DDS_TK_MUTABLE_EXTENSIBILITY       RTI_XCDR_TK_FLAGS_IS_MUTABLE
#define DDS_TK_FLAT_DATA_LANGUAGE_BINDING  RTI_XCDR_TK_FLAGS_IS_FLAT_DATA
#define DDS_TK_SHMEM_REF_TRANSFER_MODE     RTI_XCDR_TK_FLAGS_IS_SHMEM_REF

#define DDS_TK_FLAG_IS_INDEXED  RTI_XCDR_TK_FLAG_IS_INDEXED
#define DDS_TK_FLAGS_ALL        RTI_XCDR_TK_FLAGS_ALL
#define DDS_TK_FLAGS_IS_FINAL   RTI_XCDR_TK_FLAGS_IS_FINAL
#define DDS_TK_FLAGS_IS_MUTABLE RTI_XCDR_TK_FLAGS_IS_MUTABLE

#define DDS_VM_NONE        RTI_XCDR_VM_NONE
#define DDS_VM_CUSTOM      RTI_XCDR_VM_CUSTOM
#define DDS_VM_ABSTRACT    RTI_XCDR_VM_ABSTRACT
#define DDS_VM_TRUNCATABLE RTI_XCDR_VM_TRUNCATABLE
#define DDS_VM_MUTABLE     RTI_XCDR_VM_MUTABLE

#define RTI_CDR_NONKEY_MEMBER   RTI_XCDR_NONKEY_MEMBER
#define RTI_CDR_KEY_MEMBER      RTI_XCDR_KEY_MEMBER
#define RTI_CDR_REQUIRED_MEMBER RTI_XCDR_REQUIRED_MEMBER

#define DDS_PRIVATE_MEMBER RTI_XCDR_PRIVATE_MEMBER
#define DDS_PUBLIC_MEMBER  RTI_XCDR_PUBLIC_MEMBER

#define RTI_CDR_TYPE_CODE_UNION_DEFAULT_LABEL RTI_XCDR_TYPE_CODE_UNION_DEFAULT_LABEL

#define RTICdrAnnotationParameterValue                  RTIXCdrAnnotationParameterValue
#define RTICdrTypeCodeMemberAnnotations                 RTIXCdrTypeCodeMemberAnnotations
#define RTICdrTypeCodeMemberAnnotations_INITIALIZER     RTIXCdrTypeCodeMemberAnnotations_INITIALIZER
#define RTICdrTypeCodeAnnotations_INITIALIZER           RTIXCdrTypeCodeAnnotations_INITIALIZER
#define RTICdrTypeCodeMemberAnnotations_initialize      RTIXCdrTypeCodeMemberAnnotations_initialize
#define RTICdrTypeCodeMemberAnnotations_copy            RTIXCdrTypeCodeMemberAnnotations_copy
#define RTICdrTypeCodeMemberAnnotations_equals          RTIXCdrTypeCodeMemberAnnotations_equals
#define RTICdrTypeCodeMemberAnnotations_finalize        RTIXCdrTypeCodeMemberAnnotations_finalize

struct DDSCDllExport DDS_TypeCode 
{
    RTIXCdrTypeCode _data;
};

typedef struct RTICdrAnnotationParameterValue DDS_TypeCode_Annotation;
typedef struct DDS_TypeCode                   DDS_TypeCode;
typedef RTIXCdrTypeCodeMember                 DDS_TypeCode_Member;
typedef RTIXCdrTypeCode                       RTICdrTypeCode;

#define DDS_INITIALIZE_PRIMITIVE_TYPECODE(kind, copyable)\
    {{kind,DDS_BOOLEAN_FALSE,-1,NULL,NULL,0,0,NULL,0,NULL,DDS_VM_NONE, RTICdrTypeCodeAnnotations_INITIALIZER, copyable, NULL, NULL}}
#define DDS_INITIALIZE_PRIMITIVE_TYPECODE_W_SAMPLE_ACCESS_INFO(kind, copyable, sample_access)\
    {{kind,DDS_BOOLEAN_FALSE,-1,NULL,NULL,0,0,NULL,0,NULL,DDS_VM_NONE, RTICdrTypeCodeAnnotations_INITIALIZER, copyable, sample_access, NULL}}
#define DDS_INITIALIZE_STRING_TYPECODE(maximum)\
    {{DDS_TK_STRING,DDS_BOOLEAN_FALSE,-1,NULL,NULL,maximum,0,NULL,0,NULL,DDS_VM_NONE, RTICdrTypeCodeAnnotations_INITIALIZER, DDS_BOOLEAN_TRUE, NULL, NULL}}
#define DDS_INITIALIZE_STRING_TYPECODE_W_SAMPLE_ACCESS_INFO(maximum, sample_access)\
    {{DDS_TK_STRING,DDS_BOOLEAN_FALSE,-1,NULL,NULL,maximum,0,NULL,0,NULL,DDS_VM_NONE, RTICdrTypeCodeAnnotations_INITIALIZER, DDS_BOOLEAN_TRUE, sample_access, NULL}}
#define DDS_INITIALIZE_WSTRING_TYPECODE(maximum)\
    {{DDS_TK_WSTRING,DDS_BOOLEAN_FALSE,-1,NULL,NULL,maximum,0,NULL,0,NULL,DDS_VM_NONE, RTICdrTypeCodeAnnotations_INITIALIZER, DDS_BOOLEAN_TRUE, NULL, NULL}}
#define DDS_INITIALIZE_SEQUENCE_TYPECODE(maximum,typecode)\
    {{DDS_TK_SEQUENCE,DDS_BOOLEAN_FALSE,-1,NULL,typecode,maximum,0,NULL,0,NULL,DDS_VM_NONE, RTICdrTypeCodeAnnotations_INITIALIZER, DDS_BOOLEAN_TRUE, NULL, NULL}}
#define DDS_INITIALIZE_ARRAY_TYPECODE(dimensionsCount,dimension_1,dimensions,typecode)\
    {{DDS_TK_ARRAY,DDS_BOOLEAN_FALSE,-1,NULL,typecode,dimension_1,dimensionsCount,dimensions,0,NULL,DDS_VM_NONE, RTICdrTypeCodeAnnotations_INITIALIZER, DDS_BOOLEAN_TRUE, NULL, NULL}}
#define DDS_INITIALIZE_ALIAS_TYPECODE(dimension,typecode,pointer)\
    {{DDS_TK_ALIAS,pointer,-1,NULL,typecode,dimension,0,NULL,0,NULL,DDS_VM_NONE, RTICdrTypeCodeAnnotations_INITIALIZER, DDS_BOOLEAN_TRUE, NULL, NULL}}

extern DDSCDllVariable const
RTIXCdrUnsignedLong DDS_TCKind_g_primitiveSizes[RTI_XCDR_TK_NUM_PRIMITIVE_SIZES];

extern DDSCDllVariable DDS_TypeCode DDS_g_tc_null;
extern DDSCDllVariable DDS_TypeCode DDS_g_tc_short;
extern DDSCDllVariable DDS_TypeCode DDS_g_tc_long;
extern DDSCDllVariable DDS_TypeCode DDS_g_tc_ushort;
extern DDSCDllVariable DDS_TypeCode DDS_g_tc_ulong;
extern DDSCDllVariable DDS_TypeCode DDS_g_tc_float;
extern DDSCDllVariable DDS_TypeCode DDS_g_tc_double;
extern DDSCDllVariable DDS_TypeCode DDS_g_tc_boolean;
extern DDSCDllVariable DDS_TypeCode DDS_g_tc_char;
extern DDSCDllVariable DDS_TypeCode DDS_g_tc_octet;
extern DDSCDllVariable DDS_TypeCode DDS_g_tc_longlong;
extern DDSCDllVariable DDS_TypeCode DDS_g_tc_ulonglong;
extern DDSCDllVariable DDS_TypeCode DDS_g_tc_longdouble;
extern DDSCDllVariable DDS_TypeCode DDS_g_tc_wchar;

extern DDSCDllExport DDS_Boolean
DDS_TypeCode_is_flat_data_language_binding(
        const DDS_TypeCode *self,
        DDS_ExceptionCode_t *ex);

extern DDSCDllExport DDS_Boolean
DDS_TypeCode_is_shmem_ref_transfer_mode(
        const DDS_TypeCode *self,
        DDS_ExceptionCode_t *ex);

#ifdef __cplusplus
    }	/* extern "C" */
#endif

#endif /* dds_c_typecode_h */
