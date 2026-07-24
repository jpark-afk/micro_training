/*
 * FILE: DDS_ParticipantMessageDataPlugin.h
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

#ifndef DDS_ParticipantMessageDataPlugin_1033596382_h
#define DDS_ParticipantMessageDataPlugin_1033596382_h

#include "DDS_ParticipantMessageData.h"

#if (defined(RTI_WIN32) || defined(RTI_WINCE)) && defined(NDDS_USER_DLL_EXPORT)
/* If the code is building on Windows, start exporting symbols. */
#undef NDDSUSERDllExport
#define NDDSUSERDllExport __declspec(dllexport)
#endif

#ifdef __cplusplus
extern "C" {
    #endif

    /* --------------------------------------------------------------------------
    (De)Serialize functions:
    * -------------------------------------------------------------------------- */
    NDDSUSERDllExport extern RTI_BOOL 
    DDS_GuidPrefix_t_cdr_serialize(struct DDS_TypePlugin *plugin, 
    struct CDR_Stream_t *stream, 
    const void *void_sample,
    DDS_InstanceHandle_t *destination);

    NDDSUSERDllExport extern RTI_BOOL 
    DDS_GuidPrefix_t_cdr_deserialize(struct DDS_TypePlugin *plugin,
    void *void_sample,
    struct CDR_Stream_t *stream,
    DDS_InstanceHandle_t *source); 

    NDDSUSERDllExport extern RTI_UINT32
    DDS_GuidPrefix_t_get_serialized_sample_size(
        struct DDS_TypePlugin *plugin,
        RTI_UINT32 current_alignment);
    /* Unkeyed type key serialization equivalent to sample serialization */
    #define DDS_GuidPrefix_t_cdr_serialize_key DDS_GuidPrefix_t_cdr_serialize
    #define DDS_GuidPrefix_t_cdr_deserialize_key DDS_GuidPrefix_t_cdr_deserialize
    #define DDS_GuidPrefix_t_get_serialized_key_size DDS_GuidPrefix_t_get_serialized_sample_size
    #ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
    #endif

    /* --------------------------------------------------------------------------
    (De)Serialize functions:
    * -------------------------------------------------------------------------- */
    NDDSUSERDllExport extern RTI_BOOL 
    DDS_OctetArray4_cdr_serialize(struct DDS_TypePlugin *plugin, 
    struct CDR_Stream_t *stream, 
    const void *void_sample,
    DDS_InstanceHandle_t *destination);

    NDDSUSERDllExport extern RTI_BOOL 
    DDS_OctetArray4_cdr_deserialize(struct DDS_TypePlugin *plugin,
    void *void_sample,
    struct CDR_Stream_t *stream,
    DDS_InstanceHandle_t *source); 

    NDDSUSERDllExport extern RTI_UINT32
    DDS_OctetArray4_get_serialized_sample_size(
        struct DDS_TypePlugin *plugin,
        RTI_UINT32 current_alignment);
    /* Unkeyed type key serialization equivalent to sample serialization */
    #define DDS_OctetArray4_cdr_serialize_key DDS_OctetArray4_cdr_serialize
    #define DDS_OctetArray4_cdr_deserialize_key DDS_OctetArray4_cdr_deserialize
    #define DDS_OctetArray4_get_serialized_key_size DDS_OctetArray4_get_serialized_sample_size
    #ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
    #endif

    /* --------------------------------------------------------------------------
    (De)Serialize functions:
    * -------------------------------------------------------------------------- */
    NDDSUSERDllExport extern RTI_BOOL 
    DDS_Liveliness_ParticipantMessageDataKey_cdr_serialize(struct DDS_TypePlugin *plugin, 
    struct CDR_Stream_t *stream, 
    const void *void_sample,
    DDS_InstanceHandle_t *destination);

    NDDSUSERDllExport extern RTI_BOOL 
    DDS_Liveliness_ParticipantMessageDataKey_cdr_deserialize(struct DDS_TypePlugin *plugin,
    void *void_sample,
    struct CDR_Stream_t *stream,
    DDS_InstanceHandle_t *source); 

    NDDSUSERDllExport extern RTI_UINT32
    DDS_Liveliness_ParticipantMessageDataKey_get_serialized_sample_size(
        struct DDS_TypePlugin *plugin,
        RTI_UINT32 current_alignment);
    /* Unkeyed type key serialization equivalent to sample serialization */
    #define DDS_Liveliness_ParticipantMessageDataKey_cdr_serialize_key DDS_Liveliness_ParticipantMessageDataKey_cdr_serialize
    #define DDS_Liveliness_ParticipantMessageDataKey_cdr_deserialize_key DDS_Liveliness_ParticipantMessageDataKey_cdr_deserialize
    #define DDS_Liveliness_ParticipantMessageDataKey_get_serialized_key_size DDS_Liveliness_ParticipantMessageDataKey_get_serialized_sample_size
    #ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
    #endif

    struct DDS_Liveliness_ParticipantMessageDataTypePlugin;

    NDDSUSERDllExport extern RTI_BOOL
    DDS_Liveliness_ParticipantMessageDataTypePlugin_delete(struct DDS_TypePlugin *self);

    NDDSUSERDllExport extern struct DDS_TypePlugin*
    DDS_Liveliness_ParticipantMessageDataWriterTypePlugin_create(
        DDS_DomainParticipant *participant,
        struct DDS_DomainParticipantQos *dp_qos,
        DDS_DataWriter *writer,
        struct DDS_DataWriterQos *qos,
        struct DDS_TypePluginProperty *property);

    NDDSUSERDllExport extern struct DDS_TypePlugin*
    DDS_Liveliness_ParticipantMessageDataReaderTypePlugin_create(
        DDS_DomainParticipant *participant,
        struct DDS_DomainParticipantQos *dp_qos,
        DDS_DataReader *reader,
        struct DDS_DataReaderQos *qos,
        struct DDS_TypePluginProperty *property);

    NDDSUSERDllExport extern struct DDS_TypePluginI*
    DDS_Liveliness_ParticipantMessageDataTypePlugin_get(void);
    NDDSUSERDllExport extern const char*
    DDS_Liveliness_ParticipantMessageDataTypePlugin_get_default_type_name(void);

    /* --------------------------------------------------------------------------
    Untyped interfaces to the typed sample management functions
    * -------------------------------------------------------------------------- */
    NDDSUSERDllExport extern RTI_BOOL
    DDS_Liveliness_ParticipantMessageDataPlugin_create_sample(
        struct DDS_TypePlugin *plugin, void **sample);

    #ifndef RTI_CERT
    NDDSUSERDllExport extern RTI_BOOL 
    DDS_Liveliness_ParticipantMessageDataPlugin_delete_sample(
        struct DDS_TypePlugin *plugin, void *sample);
    #endif

    NDDSUSERDllExport extern RTI_BOOL 
    DDS_Liveliness_ParticipantMessageDataPlugin_copy_sample(
        struct DDS_TypePlugin *plugin, void *dst, const void *src);

    /* --------------------------------------------------------------------------
    (De)Serialize functions:
    * -------------------------------------------------------------------------- */
    NDDSUSERDllExport extern RTI_BOOL 
    DDS_Liveliness_ParticipantMessageData_cdr_serialize(struct DDS_TypePlugin *plugin, 
    struct CDR_Stream_t *stream, 
    const void *void_sample,
    DDS_InstanceHandle_t *destination);

    NDDSUSERDllExport extern RTI_BOOL 
    DDS_Liveliness_ParticipantMessageData_cdr_deserialize(struct DDS_TypePlugin *plugin,
    void *void_sample,
    struct CDR_Stream_t *stream,
    DDS_InstanceHandle_t *source); 

    NDDSUSERDllExport extern RTI_UINT32
    DDS_Liveliness_ParticipantMessageData_get_serialized_sample_size(
        struct DDS_TypePlugin *plugin,
        RTI_UINT32 current_alignment);
    /* --------------------------------------------------------------------------
    Key Management functions:
    * -------------------------------------------------------------------------- */
    NDDSUSERDllExport extern RTI_BOOL 
    DDS_Liveliness_ParticipantMessageData_cdr_serialize_key(struct DDS_TypePlugin *plugin,
    struct CDR_Stream_t *keystream, 
    const void *sample,
    DDS_InstanceHandle_t *destination);

    NDDSUSERDllExport extern RTI_BOOL 
    DDS_Liveliness_ParticipantMessageData_cdr_deserialize_key(
        struct DDS_TypePlugin *plugin,
        void *sample,
        struct CDR_Stream_t *keystream,
        DDS_InstanceHandle_t *source);

    NDDSUSERDllExport extern RTI_UINT32
    DDS_Liveliness_ParticipantMessageData_get_serialized_key_size(
        struct DDS_TypePlugin *plugin,
        RTI_UINT32 current_alignment);

    NDDSUSERDllExport extern RTI_BOOL 
    DDS_Liveliness_ParticipantMessageData_instance_to_keyhash(
        struct DDS_TypePlugin *plugin,
        struct CDR_Stream_t *stream, 
        DDS_KeyHash_t *keyHash, 
        const void *instance);
    #ifdef __cplusplus
}
#endif

#if (defined(RTI_WIN32) || defined(RTI_WINCE)) && defined(NDDS_USER_DLL_EXPORT)
/* If the code is building on Windows, stop exporting symbols. */
#undef NDDSUSERDllExport
#define NDDSUSERDllExport
#endif

#endif /* DDS_ParticipantMessageDataPlugin_1033596382_h */

