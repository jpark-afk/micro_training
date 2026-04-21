/*
 * FILE: dds_c_type.h - DDS type module
 *
 * (c) Copyright, Real-Time Innovations, 2011-2020.
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
 * 13apr2021,tk MICRO-3028/PR.28960
 * - Hide unsupported APIs from public documentation
 * 25Mar2015,as MICRO-908: Removed unused PluginHelper functions
 * 19jul2013,as Added support for C++
 * 11jun2011  Created
 */
/*ce
 * \file
 * \brief DDS type module
 */
/*e
*/
#ifndef dds_c_type_h
#define dds_c_type_h

#ifndef cdr_cdr_type_h
#include "cdr/cdr_cdr_type.h"
#endif
#ifndef cdr_stream_h
#include "cdr/cdr_stream.h"
#endif
#ifndef cdr_encapsulation_h
#include "cdr/cdr_encapsulation.h"
#endif
#ifndef reda_buffer_h
#include "reda/reda_buffer.h"
#endif
#ifndef rtps_rtps_h
#include "rtps/rtps_rtps.h"
#endif
#ifndef dds_c_dll_h
#include "dds_c/dds_c_dll.h"
#endif
#ifndef dds_c_infrastructure_h
#include "dds_c/dds_c_infrastructure.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/* ================================================================= */
/*                       Type Plugin Interface                       */
/* ================================================================= */
/*ce @ingroup DDSMicroTypesModule
 */

/*i \dref_Type
 */
typedef struct DDS_TypeImpl DDS_Type;

struct NDDS_Type_Plugin;

/*i \dref_UserDataKeyHolder_t
 */
typedef void *UserDataKeyHolder_t;

/*i \dref_KeyHash_t
 */
typedef struct RTPS_KeyHash DDS_KeyHash_t;

/*i \dref_KeyHash_DEFAULT
 */
#define DDS_KEY_HASH_DEFAULT  RTPS_KEY_HASH_DEFAULT

/*e \dref_NDDS_Type_PluginVersion
 */
typedef struct NDDS_Type_PluginVersion
{
    /*e \dref_NDDS_Type_PluginVersion_majorRev */
    char majorRev;

    /*e \dref_NDDS_Type_PluginVersion_minorRev */
    char minorRev;
} NDDS_Type_PluginVersion;

/*e \dref_NDDS_TypePluginKeyKind
*/
typedef enum
{
    /*e \dref_NDDS_TypePluginKeyKind_TYPEPLUGIN_NO_KEY
     */
    NDDS_TYPEPLUGIN_NO_KEY,

    /*i \dref_NDDS_TypePluginKeyKind_TYPEPLUGIN_GUID_KEY
     */
    NDDS_TYPEPLUGIN_GUID_KEY,

    /*e \dref_NDDS_TypePluginKeyKind_TYPEPLUGIN_USER_KEY
     */
    NDDS_TYPEPLUGIN_USER_KEY
} NDDS_TypePluginKeyKind;

/*ce \dref_NDDS_Type_GetSerializedSizeFunc
 */
FUNCTION_MUST_TYPEDEF(
RTI_UINT32
(*NDDS_Type_GetSerializedSizeFunc)(
        struct NDDS_Type_Plugin *plugin,
        RTI_UINT32 current_alignment,
        void *param)
)

/*ce \dref_NDDS_Type_CreateSampleFunc
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*NDDS_Type_CreateSampleFunc)(
        struct NDDS_Type_Plugin *plugin,
        void **sample,
        void *param)
)

/*ce \dref_NDDS_Type_DeleteSampleFunc
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*NDDS_Type_DeleteSampleFunc) (
        struct NDDS_Type_Plugin *plugin,
        void *sample,
        void *param)
)

/*ce \dref_NDDS_Type_CopySampleFunc
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*NDDS_Type_CopySampleFunc)(
        struct NDDS_Type_Plugin *plugin,
        void *dst,
        const void *src,
        void *param)
)

/*ce \dref_NDDS_Type_GetKeyKindFunc
 */
FUNCTION_MUST_TYPEDEF(
NDDS_TypePluginKeyKind
(*NDDS_PluginHelper_GetKeyKindFunc)(
        struct NDDS_Type_Plugin *plugin,
        void *param)
)

/*ce \dref_NDDS_Type_InstanceToKeyHashFunc
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*NDDS_PluginHelper_InstanceToKeyHashFunc)(
        struct NDDS_Type_Plugin *plugin,
        struct CDR_Stream_t *stream,
        DDS_KeyHash_t *keyHash,
        const void *instance,
        void *param)
)

/*ce \dref_NDDS_Type_CreateTypedDataWriterFunc
 */
typedef void*
(*NDDS_PluginHelper_CreateTypedDataWriterFunc)(void *writer);

/*ce \dref_NDDS_Type_DeleteTypedDataWriterFunc
 */
typedef void
(*NDDS_PluginHelper_DeleteTypedDataWriterFunc)(void *wrapper);

/*ce \dref_NDDS_Type_CreateTypedDataReaderFunc
 */
typedef void*
(*NDDS_PluginHelper_CreateTypedDataReaderFunc)(void *reader);

/*ce \dref_NDDS_Type_DeleteTypedDataReaderFunc
 */
typedef void
(*NDDS_PluginHelper_DeleteTypedDataReaderFunc)(void *wrapper);

/* **************************** end plugin function prototypes *********** */

/* anonymous declaration of TypeCode */
struct DDS_TypeCode;

/*i \dref_TypeCode_t
 */
typedef struct DDS_TypeCode DDS_TypeCode_t;

/*e \dref_NDDS_Type_Plugin
 */
struct NDDS_Type_Plugin
{
    /* constants */

    /*ci \dref_NDDS_Type_Plugin_version
     */
    NDDS_Type_PluginVersion version;

    /*ci \dref_NDDS_Type_Plugin_type_code
     */
    DDS_TypeCode_t* type_code;

    /*ci \dref_NDDS_Type_Plugin_encapsulationKind
     */
    NDDSCDREncapsulation* encapsulationKind;

    /*ci \dref_NDDS_Type_Plugin_key_kind
     */
    NDDS_TypePluginKeyKind  key_kind;

    /* function pointers for type specific functions */

    /*ce \dref_NDDS_Type_Plugin_serialize_data
     */
    CDR_Stream_SerializeFunction serialize_data;

    /*ce \dref_NDDS_Type_Plugin_deserialize_data
     */
    CDR_Stream_DeserializeFunction deserialize_data;

    /*ce \dref_NDDS_Type_Plugin_get_serialized_sample_max_size
     */
    NDDS_Type_GetSerializedSizeFunc get_serialized_sample_max_size;

    /*i \dref_NDDS_Type_Plugin_serialize_key
     */
    CDR_Stream_SerializeFunction serialize_key;

    /*i \dref_NDDS_Type_Plugin_deserialize_key
     */
    CDR_Stream_DeserializeFunction deserialize_key;

    /*ce \dref_NDDS_Type_Plugin_get_serializedKeyMaxSize
     */
    NDDS_Type_GetSerializedSizeFunc get_serialized_key_max_size;

    /*ce \dref_NDDS_Type_Plugin_create_sample
     */
    NDDS_Type_CreateSampleFunc create_sample;

    /*ce \dref_NDDS_Type_Plugin_delete_sample
     */
    NDDS_Type_DeleteSampleFunc delete_sample;

    /*ce \dref_NDDS_Type_Plugin_copy_sample
     */
    NDDS_Type_CopySampleFunc copy_sample;

    /*ce \dref_NDDS_Type_Plugin_get_key_kind
     */
    NDDS_PluginHelper_GetKeyKindFunc get_key_kind;

    /*ce \dref_NDDS_Type_Plugin_instance_to_keyhash
     */
    NDDS_PluginHelper_InstanceToKeyHashFunc instance_to_keyhash;
    
    /*ci \dref_NDDS_Type_Plugin_create_typed_datawriter
     */
    NDDS_PluginHelper_CreateTypedDataWriterFunc create_typed_datawriter;

    /*ci \dref_NDDS_Type_Plugin_delete_typed_datawriter
     */
    NDDS_PluginHelper_DeleteTypedDataWriterFunc delete_typed_datawriter;

    /*ci \dref_NDDS_Type_Plugin_create_typed_datareader
     */
    NDDS_PluginHelper_CreateTypedDataReaderFunc create_typed_datareader;

    /*ci \dref_NDDS_Type_Plugin_delete_typed_datareader
     */
    NDDS_PluginHelper_DeleteTypedDataReaderFunc delete_typed_datareader;
};

/* ================================================================= */
/*              Non-Type Specific Type Support Functions             */
/* ================================================================= */

/*ce \dref_PluginHelper_instance_to_keyhash
 */
MUST_CHECK_RETURN DDSCDllExport RTI_BOOL
PluginHelper_instance_to_keyhash(struct NDDS_Type_Plugin *plugin,
                                 struct CDR_Stream_t *stream,
                                 DDS_KeyHash_t *keyHash,
                                 const void *instance,
                                 void *param);

/*ce \dref_PluginHelper_get_key_kind
 */
MUST_CHECK_RETURN DDSCDllExport NDDS_TypePluginKeyKind
PluginHelper_get_key_kind(struct NDDS_Type_Plugin *plugin,void *param);

#ifdef __cplusplus
}                               /* extern "C" */
#endif


#endif /* dds_c_type_h */
