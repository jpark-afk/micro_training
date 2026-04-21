/*
WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

This file was generated from HelloWorld.idl using "rtiddsgen".
The rtiddsgen tool is part of the RTI Data Distribution Service distribution.
For more information, type 'rtiddsgen -help' at a command shell
or consult the RTI Data Distribution Service manual.
*/

#ifndef HelloWorld_1436885537_h
#define HelloWorld_1436885537_h

#ifndef rti_me_c_h
#include "rti_me_c.h"
#endif

#if (defined(RTI_WIN32) || defined(RTI_WIN64) || defined(RTI_WINCE)) && defined(NDDS_USER_DLL_EXPORT)
/* If the code is building on Windows, start exporting symbols. */
#undef NDDSUSERDllExport
#define NDDSUSERDllExport __declspec(dllexport)
#endif

#ifdef __cplusplus
extern "C" {
    #endif

    typedef struct HelloWorld

    {

        DDS_String msg;

    } HelloWorld ;

    NDDSUSERDllExport extern const char *HelloWorldTYPENAME;

    #define REDA_SEQUENCE_USER_API
    #define T HelloWorld
    #define TSeq HelloWorldSeq
    #define REDA_SEQUENCE_EXCLUDE_C_METHODS
    #define REDA_SEQUENCE_USER_CPP
    #include <reda/reda_sequence_decl.h>

    #define REDA_SEQUENCE_USER_API
    #define T HelloWorld
    #define TSeq HelloWorldSeq
    #define REDA_SEQUENCE_EXCLUDE_STRUCT
    #define REDA_SEQUENCE_USER_CPP
    #include <reda/reda_sequence_decl.h>

    NDDSUSERDllExport extern RTI_BOOL
    HelloWorld_initialize(HelloWorld* sample);

    NDDSUSERDllExport extern HelloWorld*
    HelloWorld_create(void);

    #ifndef RTI_CERT
    NDDSUSERDllExport extern RTI_BOOL
    HelloWorld_finalize(HelloWorld* sample);

    NDDSUSERDllExport extern void
    HelloWorld_delete(HelloWorld* sample);
    #endif

    NDDSUSERDllExport extern RTI_BOOL
    HelloWorld_copy(HelloWorld* dst, const HelloWorld* src);

    #if (defined(RTI_WIN32) || defined(RTI_WIN64) || defined(RTI_WINCE)) && defined(NDDS_USER_DLL_EXPORT)
    /* If the code is building on Windows, stop exporting symbols. */
    #undef NDDSUSERDllExport
    #define NDDSUSERDllExport
    #endif

    #ifdef __cplusplus
}
#endif

#endif /* HelloWorld */

