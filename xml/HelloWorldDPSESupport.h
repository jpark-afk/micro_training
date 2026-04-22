/*
WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

This file was generated from HelloWorldDPSE.idl using "rtiddsgen".
The rtiddsgen tool is part of the RTI Data Distribution Service distribution.
For more information, type 'rtiddsgen -help' at a command shell
or consult the RTI Data Distribution Service manual.
*/

#ifndef HelloWorldDPSESupport_2058707909_h
#define HelloWorldDPSESupport_2058707909_h

/* Uses */
#include "HelloWorldDPSE.h"
/* Requires */
#include "HelloWorldDPSEPlugin.h"

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

    DDS_TYPESUPPORT_C(HelloWorldTypeSupport, HelloWorld);
    DDS_DATAWRITER_C(HelloWorldDataWriter, HelloWorld);

    DDS_DATAREADER_C(HelloWorldDataReader, HelloWorldSeq, HelloWorld);

    #ifdef __cplusplus
} /* extern "C" */
#endif

#if (defined(RTI_WIN32) || defined(RTI_WINCE)) && defined(NDDS_USER_DLL_EXPORT)
/* If the code is building on Windows, stop exporting symbols. */
#undef NDDSUSERDllExport
#define NDDSUSERDllExport
#endif

#endif  /* HelloWorldDPSESupport_2058707909_h */

