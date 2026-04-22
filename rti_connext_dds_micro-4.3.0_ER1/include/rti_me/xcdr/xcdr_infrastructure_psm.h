/*
 * FILE: xcd_infrastructure_psm.h
 *
 * (c) Copyright 2018-2026 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef xcdr_infrastructure_psm_h
#define xcdr_infrastructure_psm_h

#include "osapi/osapi_types.h"
#include "netio/netio_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* #define RTI_SIZE_T unsigned int */

typedef char RTIXCdrChar;
typedef RTI_UINT32 RTIXCdrLegacyWchar;
typedef RTI_UINT16 RTIXCdrWchar;
typedef RTI_UINT8 RTIXCdrOctet;
typedef RTI_INT16 RTIXCdrShort;
typedef RTI_UINT16 RTIXCdrUnsignedShort;
typedef RTI_INT32 RTIXCdrLong;
typedef RTI_UINT32 RTIXCdrUnsignedLong;
typedef RTI_INT64 RTIXCdrLongLong;
typedef RTI_UINT64 RTIXCdrUnsignedLongLong;
typedef RTI_FLOAT32 RTIXCdrFloat;
typedef RTI_DOUBLE64 RTIXCdrDouble;
typedef RTI_UINT8 RTIXCdrBoolean;
typedef RTI_UINT8 RTIXCdrExtendedBoolean;
typedef RTI_INT32 RTIXCdrEnum;
typedef RTI_DOUBLE128 RTIXCdrLongDouble;
typedef RTI_INT8 RTIXCdrInt8;
typedef RTI_UINT8 RTIXCdrUInt8;

typedef RTIXCdrOctet RTIXCdr1Byte;
typedef RTIXCdrUnsignedShort RTIXCdr2Byte;
typedef RTIXCdrUnsignedLong RTIXCdr4Byte;
typedef RTIXCdrUnsignedLongLong RTIXCdr8Byte;
typedef RTIXCdrLongDouble RTIXCdr16Byte;

#define RTI_XCDR_ONE_BYTE_SIZE 1
#define RTI_XCDR_TWO_BYTE_SIZE 2
#define RTI_XCDR_FOUR_BYTE_SIZE 4
#define RTI_XCDR_EIGHT_BYTE_SIZE 8
#define RTI_XCDR_SIXTEEN_BYTE_SIZE 16

#define RTI_XCDR_ENVIRONMENT_VARIABLE_MAX_LENGTH 1024

#define RTI_XCDR_COMPRESSION_CLASS_ID_NONE  0

#define RTI_FUNCTION_NAME RTIME_FUNCTION_NAME

/* ------------------------------------------------------------------------- */
/* ---- Logging ------------------------------------------------------------ */
/* ------------------------------------------------------------------------- */
#ifdef RTI_PRECONDITION_TEST
#define RTI_LOG_BIT_FATAL_ERROR   (0x00000001U)

#define RTI_XCDR_LOG_FATAL RTI_LOG_BIT_FATAL_ERROR
#endif

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif /* xcdr_infrastructure_psm_h */
