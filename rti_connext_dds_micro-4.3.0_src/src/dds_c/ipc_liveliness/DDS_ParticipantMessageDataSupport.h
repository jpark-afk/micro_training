/*
 * FILE: DDS_ParticipantMessageDataSupport.h
 *
 * (c) Copyright 2018-2024 Real-Time Innovations,
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

#ifndef DDS_ParticipantMessageDataSupport_1033596382_h
#define DDS_ParticipantMessageDataSupport_1033596382_h

/* Uses */
#include "DDS_ParticipantMessageData.h"
/* Requires */
#include "DDS_ParticipantMessageDataPlugin.h"

/* ========================================================================== */
/**
Uses:     T
Defines:  TTypeSupport, TDataWriter, TDataReader*/

#if (defined(RTI_WIN32) || defined(RTI_WINCE)) && defined(NDDS_USER_DLL_EXPORT)
/* If the code is building on Windows, start exporting symbols. */
#undef NDDSUSERDllExport
#define NDDSUSERDllExport __declspec(dllexport)
#endif

#ifdef __cplusplus
extern "C" {
    #endif

    NDDSUSERDllExport extern DDS_ReturnCode_t
    DDS_Liveliness_ParticipantMessageDataTypeSupport_register_type(
        DDS_DomainParticipant* participant,
        const char* type_name);

    #ifndef RTI_CERT
    NDDSUSERDllExport extern DDS_ReturnCode_t
    DDS_Liveliness_ParticipantMessageDataTypeSupport_unregister_type(
        DDS_DomainParticipant* participant,
        const char* type_name);
    #endif

    NDDSUSERDllExport extern const char*
    DDS_Liveliness_ParticipantMessageDataTypeSupport_get_type_name(void);

    NDDSUSERDllExport extern DDS_Liveliness_ParticipantMessageData *
    DDS_Liveliness_ParticipantMessageDataTypeSupport_create_data(void);

    #ifndef RTI_CERT
    NDDSUSERDllExport extern void
    DDS_Liveliness_ParticipantMessageDataTypeSupport_delete_data(
        DDS_Liveliness_ParticipantMessageData *data);
    #endif

    DDS_DATAWRITER_C(DDS_Liveliness_ParticipantMessageDataDataWriter, DDS_Liveliness_ParticipantMessageData);

    DDS_DATAREADER_C(DDS_Liveliness_ParticipantMessageDataDataReader, DDS_Liveliness_ParticipantMessageDataSeq, DDS_Liveliness_ParticipantMessageData);

    #ifdef __cplusplus
}
#endif

#if (defined(RTI_WIN32) || defined(RTI_WINCE)) && defined(NDDS_USER_DLL_EXPORT)
/* If the code is building on Windows, stop exporting symbols. */
#undef NDDSUSERDllExport
#define NDDSUSERDllExport
#endif

#endif  /* DDS_ParticipantMessageDataSupport_1033596382_h */

