/*
WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

This file was generated from HelloWorld.idl using "rtiddsgen".
The rtiddsgen tool is part of the RTI Data Distribution Service distribution.
For more information, type 'rtiddsgen -help' at a command shell
or consult the RTI Data Distribution Service manual.
*/

#ifndef HelloWorldSupport_1436885537_h
#define HelloWorldSupport_1436885537_h

#include <stdlib.h>

/* Uses */
#include "HelloWorld.h"
/* Requires */
#include "HelloWorldPlugin.h"

/* ========================================================================== */
/**
Uses:     T
Defines:  TTypeSupport, TDataWriter, TDataReader*/

#if (defined(RTI_WIN32) || defined(RTI_WIN64) || defined(RTI_WINCE)) && defined(NDDS_USER_DLL_EXPORT)
/* If the code is building on Windows, start exporting symbols. */
#undef NDDSUSERDllExport
#define NDDSUSERDllExport __declspec(dllexport)
#endif

#ifdef __cplusplus
extern "C" {
    #endif

    NDDSUSERDllExport extern DDS_ReturnCode_t
    HelloWorldTypeSupport_register_type(
        DDS_DomainParticipant* participant,
        const char* type_name);

    #ifndef RTI_CERT
    NDDSUSERDllExport extern DDS_ReturnCode_t
    HelloWorldTypeSupport_unregister_type(
        DDS_DomainParticipant* participant,
        const char* type_name);
    #endif

    NDDSUSERDllExport extern const char*
    HelloWorldTypeSupport_get_type_name(void);

    NDDSUSERDllExport extern HelloWorld *
    HelloWorldTypeSupport_create_data(void);

    #ifndef RTI_CERT
    NDDSUSERDllExport extern void
    HelloWorldTypeSupport_delete_data(
        HelloWorld *data);
    #endif

    DDS_DATAWRITER_C(HelloWorldDataWriter, HelloWorld);

    DDS_DATAREADER_C(HelloWorldDataReader, HelloWorldSeq, HelloWorld);

    #ifdef __cplusplus
}
#endif

#if (defined(RTI_WIN32) || defined(RTI_WIN64) || defined(RTI_WINCE)) && defined(NDDS_USER_DLL_EXPORT)
/* If the code is building on Windows, stop exporting symbols. */
#undef NDDSUSERDllExport
#define NDDSUSERDllExport
#endif

#endif  /* HelloWorldSupport_1436885537_h */

