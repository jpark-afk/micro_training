/*
 * FILE: DDS_ParticipantMessageData.h
 *
 * (c) Copyright 2018-2019 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */


/*
WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

This file was generated from DDS_ParticipantMessageData.idl using "rtiddsgen".
The rtiddsgen tool is part of the RTI Data Distribution Service distribution.
For more information, type 'rtiddsgen -help' at a command shell
or consult the RTI Data Distribution Service manual.
*/

#ifndef DDS_ParticipantMessageData_1033596382_h
#define DDS_ParticipantMessageData_1033596382_h

#ifndef rti_me_c_h
#include "rti_me_c.h"
#endif

#if (defined(RTI_WIN32) || defined(RTI_WINCE)) && defined(NDDS_USER_DLL_EXPORT)
/* If the code is building on Windows, start exporting symbols. */
#undef NDDSUSERDllExport
#define NDDSUSERDllExport __declspec(dllexport)
#endif

typedef CDR_Octet DDS_GuidPrefix_t[12];

#define REDA_SEQUENCE_USER_API
#define T DDS_GuidPrefix_t
#define TSeq DDS_GuidPrefix_tSeq
#define REDA_SEQUENCE_EXCLUDE_C_METHODS
#define REDA_SEQUENCE_USER_CPP
#include <reda/reda_sequence_decl.h>

#ifdef __cplusplus
extern "C" {
    #endif

    #define REDA_SEQUENCE_USER_API
    #define T DDS_GuidPrefix_t
    #define TSeq DDS_GuidPrefix_tSeq
    #define REDA_SEQUENCE_EXCLUDE_STRUCT
    #define REDA_SEQUENCE_USER_CPP
    #include <reda/reda_sequence_decl.h>

    NDDSUSERDllExport extern RTI_BOOL
    DDS_GuidPrefix_t_initialize(DDS_GuidPrefix_t* sample);

    NDDSUSERDllExport extern DDS_GuidPrefix_t*
    DDS_GuidPrefix_t_create(void);

    #ifndef RTI_CERT
    NDDSUSERDllExport extern RTI_BOOL
    DDS_GuidPrefix_t_finalize(DDS_GuidPrefix_t* sample);

    NDDSUSERDllExport extern void
    DDS_GuidPrefix_t_delete(DDS_GuidPrefix_t* sample);
    #endif

    NDDSUSERDllExport extern RTI_BOOL
    DDS_GuidPrefix_t_copy(DDS_GuidPrefix_t* dst, const DDS_GuidPrefix_t* src);
    #ifdef __cplusplus
}
#endif

typedef CDR_Octet DDS_OctetArray4[4];

#define REDA_SEQUENCE_USER_API
#define T DDS_OctetArray4
#define TSeq DDS_OctetArray4Seq
#define REDA_SEQUENCE_EXCLUDE_C_METHODS
#define REDA_SEQUENCE_USER_CPP
#include <reda/reda_sequence_decl.h>

#ifdef __cplusplus
extern "C" {
    #endif

    #define REDA_SEQUENCE_USER_API
    #define T DDS_OctetArray4
    #define TSeq DDS_OctetArray4Seq
    #define REDA_SEQUENCE_EXCLUDE_STRUCT
    #define REDA_SEQUENCE_USER_CPP
    #include <reda/reda_sequence_decl.h>

    NDDSUSERDllExport extern RTI_BOOL
    DDS_OctetArray4_initialize(DDS_OctetArray4* sample);

    NDDSUSERDllExport extern DDS_OctetArray4*
    DDS_OctetArray4_create(void);

    #ifndef RTI_CERT
    NDDSUSERDllExport extern RTI_BOOL
    DDS_OctetArray4_finalize(DDS_OctetArray4* sample);

    NDDSUSERDllExport extern void
    DDS_OctetArray4_delete(DDS_OctetArray4* sample);
    #endif

    NDDSUSERDllExport extern RTI_BOOL
    DDS_OctetArray4_copy(DDS_OctetArray4* dst, const DDS_OctetArray4* src);
    #ifdef __cplusplus
}
#endif

typedef struct DDS_Liveliness_ParticipantMessageDataKey

{

    DDS_GuidPrefix_t participant_guid_prefix;
    DDS_OctetArray4 kind;

} DDS_Liveliness_ParticipantMessageDataKey ;

#define REDA_SEQUENCE_USER_API
#define T DDS_Liveliness_ParticipantMessageDataKey
#define TSeq DDS_Liveliness_ParticipantMessageDataKeySeq
#define REDA_SEQUENCE_EXCLUDE_C_METHODS
#define REDA_SEQUENCE_USER_CPP
#include <reda/reda_sequence_decl.h>

#ifdef __cplusplus
extern "C" {
    #endif

    #define REDA_SEQUENCE_USER_API
    #define T DDS_Liveliness_ParticipantMessageDataKey
    #define TSeq DDS_Liveliness_ParticipantMessageDataKeySeq
    #define REDA_SEQUENCE_EXCLUDE_STRUCT
    #define REDA_SEQUENCE_USER_CPP
    #include <reda/reda_sequence_decl.h>

    NDDSUSERDllExport extern RTI_BOOL
    DDS_Liveliness_ParticipantMessageDataKey_initialize(DDS_Liveliness_ParticipantMessageDataKey* sample);

    NDDSUSERDllExport extern DDS_Liveliness_ParticipantMessageDataKey*
    DDS_Liveliness_ParticipantMessageDataKey_create(void);

    #ifndef RTI_CERT
    NDDSUSERDllExport extern RTI_BOOL
    DDS_Liveliness_ParticipantMessageDataKey_finalize(DDS_Liveliness_ParticipantMessageDataKey* sample);

    NDDSUSERDllExport extern void
    DDS_Liveliness_ParticipantMessageDataKey_delete(DDS_Liveliness_ParticipantMessageDataKey* sample);
    #endif

    NDDSUSERDllExport extern RTI_BOOL
    DDS_Liveliness_ParticipantMessageDataKey_copy(DDS_Liveliness_ParticipantMessageDataKey* dst, const DDS_Liveliness_ParticipantMessageDataKey* src);
    #ifdef __cplusplus
}
#endif

typedef struct DDS_Liveliness_ParticipantMessageData

{

    DDS_Liveliness_ParticipantMessageDataKey key;
    struct CDR_OctetSeq data;

} DDS_Liveliness_ParticipantMessageData ;

NDDSUSERDllExport extern const char *DDS_Liveliness_ParticipantMessageDataTYPENAME;

#define REDA_SEQUENCE_USER_API
#define T DDS_Liveliness_ParticipantMessageData
#define TSeq DDS_Liveliness_ParticipantMessageDataSeq
#define REDA_SEQUENCE_EXCLUDE_C_METHODS
#define REDA_SEQUENCE_USER_CPP
#include <reda/reda_sequence_decl.h>

#ifdef __cplusplus
extern "C" {
    #endif

    #define REDA_SEQUENCE_USER_API
    #define T DDS_Liveliness_ParticipantMessageData
    #define TSeq DDS_Liveliness_ParticipantMessageDataSeq
    #define REDA_SEQUENCE_EXCLUDE_STRUCT
    #define REDA_SEQUENCE_USER_CPP
    #include <reda/reda_sequence_decl.h>

    NDDSUSERDllExport extern RTI_BOOL
    DDS_Liveliness_ParticipantMessageData_initialize(DDS_Liveliness_ParticipantMessageData* sample);

    NDDSUSERDllExport extern DDS_Liveliness_ParticipantMessageData*
    DDS_Liveliness_ParticipantMessageData_create(void);

    #ifndef RTI_CERT
    NDDSUSERDllExport extern RTI_BOOL
    DDS_Liveliness_ParticipantMessageData_finalize(DDS_Liveliness_ParticipantMessageData* sample);

    NDDSUSERDllExport extern void
    DDS_Liveliness_ParticipantMessageData_delete(DDS_Liveliness_ParticipantMessageData* sample);
    #endif

    NDDSUSERDllExport extern RTI_BOOL
    DDS_Liveliness_ParticipantMessageData_copy(DDS_Liveliness_ParticipantMessageData* dst, const DDS_Liveliness_ParticipantMessageData* src);
    #ifdef __cplusplus
}
#endif

#if (defined(RTI_WIN32) || defined(RTI_WINCE)) && defined(NDDS_USER_DLL_EXPORT)
/* If the code is building on Windows, stop exporting symbols. */
#undef NDDSUSERDllExport
#define NDDSUSERDllExport
#endif

#endif /* DDS_ParticipantMessageData */

