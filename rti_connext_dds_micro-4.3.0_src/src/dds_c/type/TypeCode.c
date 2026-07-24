/*
 * FILE: TypeCode.c
 *
 * (c) Copyright 2018-2018 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#include "dds_c/dds_c_typecode.h"

DDS_TypeCode DDS_g_tc_null            = DDS_INITIALIZE_PRIMITIVE_TYPECODE(DDS_TK_NULL, DDS_BOOLEAN_FALSE);
DDS_TypeCode DDS_g_tc_short           = DDS_INITIALIZE_PRIMITIVE_TYPECODE(DDS_TK_SHORT, DDS_BOOLEAN_FALSE);
DDS_TypeCode DDS_g_tc_long            = DDS_INITIALIZE_PRIMITIVE_TYPECODE(DDS_TK_LONG, DDS_BOOLEAN_FALSE);
DDS_TypeCode DDS_g_tc_ushort          = DDS_INITIALIZE_PRIMITIVE_TYPECODE(DDS_TK_USHORT, DDS_BOOLEAN_FALSE);
DDS_TypeCode DDS_g_tc_ulong           = DDS_INITIALIZE_PRIMITIVE_TYPECODE(DDS_TK_ULONG, DDS_BOOLEAN_FALSE);
DDS_TypeCode DDS_g_tc_float           = DDS_INITIALIZE_PRIMITIVE_TYPECODE(DDS_TK_FLOAT, DDS_BOOLEAN_FALSE);
DDS_TypeCode DDS_g_tc_double          = DDS_INITIALIZE_PRIMITIVE_TYPECODE(DDS_TK_DOUBLE, DDS_BOOLEAN_FALSE);
DDS_TypeCode DDS_g_tc_boolean         = DDS_INITIALIZE_PRIMITIVE_TYPECODE(DDS_TK_BOOLEAN, DDS_BOOLEAN_FALSE);
DDS_TypeCode DDS_g_tc_char            = DDS_INITIALIZE_PRIMITIVE_TYPECODE(DDS_TK_CHAR, DDS_BOOLEAN_FALSE);
DDS_TypeCode DDS_g_tc_octet           = DDS_INITIALIZE_PRIMITIVE_TYPECODE(DDS_TK_OCTET, DDS_BOOLEAN_FALSE);
DDS_TypeCode DDS_g_tc_longlong        = DDS_INITIALIZE_PRIMITIVE_TYPECODE(DDS_TK_LONGLONG, DDS_BOOLEAN_FALSE);
DDS_TypeCode DDS_g_tc_ulonglong       = DDS_INITIALIZE_PRIMITIVE_TYPECODE(DDS_TK_ULONGLONG, DDS_BOOLEAN_FALSE);
DDS_TypeCode DDS_g_tc_longdouble      = DDS_INITIALIZE_PRIMITIVE_TYPECODE(DDS_TK_LONGDOUBLE, DDS_BOOLEAN_FALSE);
DDS_TypeCode DDS_g_tc_wchar           = DDS_INITIALIZE_PRIMITIVE_TYPECODE(DDS_TK_WCHAR, DDS_BOOLEAN_FALSE);

const RTIXCdrUnsignedLong
DDS_TCKind_g_primitiveSizes[RTI_XCDR_TK_NUM_PRIMITIVE_SIZES] =
{
    0,                                  /* TK_NULL */
    sizeof(DDS_Short),                  /* TK_SHORT */
    sizeof(DDS_Long),                   /* TK_LONG */
    sizeof(DDS_UnsignedShort),          /* TK_USHORT */
    sizeof(DDS_UnsignedLong),           /* TK_ULONG */
    sizeof(DDS_Float),                  /* TK_FLOAT */
    sizeof(DDS_Double),                 /* TK_DOUBLE */
    sizeof(DDS_Boolean),                /* TK_BOOLEAN */
    sizeof(DDS_Char),                   /* TK_CHAR */
    sizeof(DDS_Octet),                  /* TK_OCTET */
    0,                                  /* TK_STRUCT */
    0,                                  /* TK_UNION */
    sizeof(DDS_Long),                   /* TK_ENUM */
    sizeof(DDS_Char*),                  /* TK_STRING */
    0,                                  /* TK_SEQUENCE */
    0,                                  /* TK_ARRAY */
    0,                                  /* TK_ALIAS */
    sizeof(DDS_LongLong),               /* TK_LONGLONG */
    sizeof(DDS_UnsignedLongLong),       /* TK_ULONGLONG */
    sizeof(DDS_LongDouble),             /* TK_LONGDOUBLE */
    sizeof(DDS_Wchar),                  /* TK_WCHAR */
    sizeof(DDS_Wchar*),                 /* TK_WSTRING */
};


DDS_Boolean
DDS_TypeCode_is_flat_data_language_binding(
        const DDS_TypeCode *self,
        DDS_ExceptionCode_t *ex)
{
    DDS_ExceptionCode_t exception = DDS_NO_EXCEPTION_CODE;
    DDS_Boolean result = DDS_BOOLEAN_FALSE;

    if (self == NULL)
    {
        exception = DDS_BAD_PARAM_SYSTEM_EXCEPTION_CODE;
        goto done;
    }

    if (self->_data._kind & DDS_TK_FLAT_DATA_LANGUAGE_BINDING)
    {
        result = DDS_BOOLEAN_TRUE;
    }

    done:

    if (ex != NULL)
    {
        *ex = exception;
    }

    return result;

}

DDS_Boolean
DDS_TypeCode_is_shmem_ref_transfer_mode(
        const DDS_TypeCode *self,
        DDS_ExceptionCode_t *ex)
{
    DDS_ExceptionCode_t exception = DDS_NO_EXCEPTION_CODE;
    DDS_Boolean result = DDS_BOOLEAN_FALSE;

    if (self == NULL)
    {
        exception = DDS_BAD_PARAM_SYSTEM_EXCEPTION_CODE;
        goto done;
    }

    if (self->_data._kind & DDS_TK_SHMEM_REF_TRANSFER_MODE)
    {
        result = DDS_BOOLEAN_TRUE;
    }

done:

    if (ex != NULL)
    {
        *ex = exception;
    }

    return result;
}
