/*
WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

This file was generated from HelloWorldDPSE.idl using "rtiddsgen".
The rtiddsgen tool is part of the RTI Data Distribution Service distribution.
For more information, type 'rtiddsgen -help' at a command shell
or consult the RTI Data Distribution Service manual.
*/

#ifndef HelloWorldDPSE_2058707909_h
#define HelloWorldDPSE_2058707909_h

#ifndef rti_me_c_h
#include "rti_me_c.h"
#endif

#if DDS_XTYPES_IS_ENABLED
#include "dds_c/dds_c_typecode.h"
#include "xcdr/xcdr_dds_interpreter.h"
#endif

#if (defined(RTI_WIN32) || defined(RTI_WINCE)) && defined(NDDS_USER_DLL_EXPORT)
/* If the code is building on Windows, start exporting symbols. */
#undef NDDSUSERDllExport
#define NDDSUSERDllExport __declspec(dllexport)
#endif

#ifdef __cplusplus
extern "C" {
    #endif

    extern const char *HelloWorldTYPENAME;

    typedef struct HelloWorld {

        DDS_String   sender ;
        DDS_String   message ;
        DDS_Long   count ;

    } HelloWorld ;
    #if (defined(RTI_WIN32) || defined (RTI_WINCE) || defined(RTI_INTIME)) && defined(NDDS_USER_DLL_EXPORT)
    /* If the code is building on Windows, start exporting symbols.
    */
    #undef NDDSUSERDllExport
    #define NDDSUSERDllExport __declspec(dllexport)
    #endif

    #ifndef NDDS_STANDALONE_TYPE
    NDDSUSERDllExport DDS_TypeCode* HelloWorld_get_typecode(void); /* Type code */
    NDDSUSERDllExport RTIXCdrTypePlugin *HelloWorld_get_type_plugin_info(void);
    NDDSUSERDllExport RTIXCdrSampleAccessInfo *HelloWorld_get_sample_access_info(void);
    #endif

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

    NDDSUSERDllExport extern HelloWorld*
    HelloWorld_create(void);

    NDDSUSERDllExport extern void
    HelloWorld_delete(HelloWorld* sample);

    NDDSUSERDllExport
    RTIBool HelloWorld_initialize(
        HelloWorld* self);

    NDDSUSERDllExport
    RTIBool HelloWorld_initialize_ex(
        HelloWorld* self,
        RTIBool allocatePointers,
        RTIBool allocateMemory);

    NDDSUSERDllExport
    RTIBool HelloWorld_initialize_w_params(
        HelloWorld* self,
        const struct DDS_TypeAllocationParams_t * allocParams);

    NDDSUSERDllExport
    RTIBool HelloWorld_finalize(
        HelloWorld* self);

    NDDSUSERDllExport
    RTIBool HelloWorld_finalize_w_return(
        HelloWorld* self);

    NDDSUSERDllExport
    void HelloWorld_finalize_ex(
        HelloWorld* self,RTIBool deletePointers);

    NDDSUSERDllExport
    void HelloWorld_finalize_w_params(
        HelloWorld* self,
        const struct DDS_TypeDeallocationParams_t * deallocParams);

    NDDSUSERDllExport
    void HelloWorld_finalize_optional_members(
        HelloWorld* self, RTIBool deletePointers);

    NDDSUSERDllExport
    RTIBool HelloWorld_copy(
        HelloWorld* dst,
        const HelloWorld* src);

    #if (defined(RTI_WIN32) || defined (RTI_WINCE) || defined(RTI_INTIME)) && defined(NDDS_USER_DLL_EXPORT)
    /* If the code is building on Windows, stop exporting symbols.
    */
    #undef NDDSUSERDllExport
    #define NDDSUSERDllExport
    #endif

    #if (defined(RTI_WIN32) || defined(RTI_WINCE)) && defined(NDDS_USER_DLL_EXPORT)
    /* If the code is building on Windows, stop exporting symbols. */
    #undef NDDSUSERDllExport
    #define NDDSUSERDllExport
    #endif

    #ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* HelloWorldDPSE */

