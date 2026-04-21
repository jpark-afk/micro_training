/*
 * FILE: dds_c_type.h - DDS type module
 *
 * Copyright (c) 2011-2025, Real-Time Innovations, .inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
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

#ifndef osapi_hash_h
#include "osapi/osapi_hash.h"
#endif

#ifndef rt_rt_h
#include "rt/rt_rt.h"
#endif
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
#include "reda/reda_bufferpool.h"
#ifndef rtps_rtps_h
#include "rtps/rtps_rtps.h"
#endif
#ifndef dds_c_dll_h
#include "dds_c/dds_c_dll.h"
#endif
#ifndef dds_c_infrastructure_h
#include "dds_c/dds_c_infrastructure.h"
#endif
#if DDS_XTYPES_IS_ENABLED
#ifndef dds_c_typecode_h
#include "dds_c/dds_c_typecode.h"
#endif
#endif
#ifdef __cplusplus
extern "C"
{
#endif

/* ================================================================= */
/*                            Type Interface                         */
/* ================================================================= */
#if DDS_XTYPES_IS_ENABLED
typedef struct RTIXCdrInterpreterPrograms*
(*DDS_TypeInterfaceI_create_program_T)(
                const RTIXCdrTypeCode *tc,
                const struct RTIXCdrInterpreterProgramsGenProperty *property,
                RTIXCdrProgramMask mask);
typedef void
(*DDS_TypeInterfaceI_delete_program_T)(struct RTIXCdrInterpreterPrograms *prg);

typedef DDS_Boolean
(*DDS_TypeInterfaceI_set_padding_options_T)(const DDS_TypeCode *const tc,
                                            DDS_EncapsulationId_t eid);

typedef struct RTIXCdrTypePluginProgramContext*
(*DDS_TypeInterfaceI_create_execution_context_T)(const DDS_TypeCode *const tc);

typedef void
(*DDS_TypeInterfaceI_delete_execution_context_T)(
                                struct RTIXCdrTypePluginProgramContext *ctxt);

struct DDS_TypeInterfaceI
{
    DDS_TypeInterfaceI_create_program_T create_program;

    DDS_TypeInterfaceI_delete_program_T delete_program;

    DDS_TypeInterfaceI_create_execution_context_T create_execution_context;

    DDS_TypeInterfaceI_delete_execution_context_T delete_execution_context;

    DDS_TypeInterfaceI_set_padding_options_T set_padding_options;
};

#define DDS_TypeInterfaceI_create_program(self_,tc_,prop_,mask_) \
(self_)->create_program(tc_,prop_,mask_)

#define DDS_TypeInterfaceI_delete_program(self_,prog_) \
(self_)->delete_program(prog_)

#define DDS_TypeInterfaceI_set_padding_options(self_,tc_,id_) \
(self_)->set_padding_options(tc_,id_)

#define DDS_TypeInterfaceI_create_execution_context(self_,tc_) \
(self_)->create_execution_context(tc_)

#define DDS_TypeInterfaceI_delete_execution_context(self_,ctxt_) \
(self_)->delete_execution_context(ctxt_)

#endif

/************************************************************************* */

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

/*i \dref_NDDS_TypePluginEhdrLocation
*/
typedef enum
{
    /*i \dref_NDDS_TypePluginEhdrLocation_TYPEPLUGIN_EH_LOCATION_PAYLOAD
     */
    NDDS_TYPEPLUGIN_EH_LOCATION_PAYLOAD,

    /*i \dref_NDDS_TypePluginEhdrLocation_TYPEPLUGIN_EH_LOCATION_SAMPLE
     */
    NDDS_TYPEPLUGIN_EH_LOCATION_SAMPLE,

    /*i \dref_NDDS_TypePluginEhdrLocation_TYPEPLUGIN_EH_LOCATION_INLINE
     */
    NDDS_TYPEPLUGIN_EH_LOCATION_INLINE
} NDDS_TypePluginEhdrLocation;

/* **************************** end plugin function prototypes *********** */

/* anonymous declaration of TypeCode */
struct DDS_TypeCode;

/*i \dref_TypeCode_t
 */
typedef struct DDS_TypeCode DDS_TypeCode_t;

struct DDS_TypePluginProperty;
struct DDS_TypePlugin;
struct DDS_TypePluginFactory;
struct DDS_TypeRegistrationProperty;
typedef struct DDS_TypePlugin DDS_TypePlugin_T;
struct DDS_TypeEncapsulationPlugin;
struct DDS_TypeMemoryPlugin;
struct DDS_DomainParticipantQos;
struct DDS_SampleInfo;

struct DDS_TypePluginSampleHolder
{
    void *sample;
    struct DDS_TypeEncapsulationPlugin *owner;
};

FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*DDS_TypePlugin_add_peer_T)(struct DDS_TypePlugin *tp,
                             DDS_InstanceHandle_t *peer)
)

DDSCDllExport DDS_Boolean
DDS_TypePlugin_add_peer(struct DDS_TypePlugin *tp,
                        DDS_InstanceHandle_t *peer);

FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*DDS_TypePlugin_remove_peer_T)(struct DDS_TypePlugin *tp,
                                DDS_InstanceHandle_t *peer)
)

DDSCDllExport DDS_Boolean
DDS_TypePlugin_remove_peer(struct DDS_TypePlugin *tp,
                           DDS_InstanceHandle_t *peer);

FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*DDS_TypePlugin_serialize_sample_T)(struct DDS_TypePlugin *tp,
                                     struct CDR_Stream_t *stream,
                                     const void *sample,
                                     DDS_InstanceHandle_t *destination)
)

MUST_CHECK_RETURN DDSCDllExport DDS_Boolean
DDS_TypePlugin_serialize_sample(struct DDS_TypePlugin *tp,
                                struct CDR_Stream_t *stream,
                                const void *sample,
                                DDS_InstanceHandle_t *destination);

FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*DDS_TypePlugin_deserialize_sample_T)(struct DDS_TypePlugin *tp,
                                       void *sample,
                                       struct CDR_Stream_t *stream,
                                       DDS_InstanceHandle_t *source)
)

#define DDS_TypePlugin_deserialize_sample(self_,sample_,stream_,source_) \
((struct DDS_TypeEncapsulationI*)(\
(((struct DDS_TypePlugin*)(self_)))->wp_intf))->deserialize((struct DDS_TypePlugin*)(self_),sample_,stream_,source_)


FUNCTION_MUST_TYPEDEF(
struct DDS_TypePluginSampleHolder*
(*DDS_TypePlugin_get_sample_T)(struct DDS_TypePlugin *tp,
                               struct CDR_Stream_t *stream)
)

#define DDS_TypePlugin_get_sample(self_,stream_) \
((struct DDS_TypeEncapsulationI*)(\
(((struct DDS_TypePlugin*)(self_)))->wp_intf))->get_sample((struct DDS_TypePlugin*)(self_),stream_)

typedef void
(*DDS_TypePlugin_return_sample_T)(struct DDS_TypePlugin *tp,struct DDS_TypePluginSampleHolder *sample);

#define DDS_TypePlugin_return_sample(self_,sample_) \
(sample_)->owner->_intf->return_sample((struct DDS_TypePlugin*)(self_),sample_)

/*************/


FUNCTION_MUST_TYPEDEF(
void*
(*DDS_TypePlugin_get_buffer_T)(struct DDS_TypePlugin *tp)
)

#define DDS_TypePlugin_get_buffer(self_) \
((struct DDS_TypeEncapsulationI*)(\
(((struct DDS_TypePlugin*)(self_)))->wp_intf))->get_buffer((struct DDS_TypePlugin*)(self_))

typedef void
(*DDS_TypePlugin_return_buffer_T)(struct DDS_TypePlugin *tp,void *buffer);

DDSCDllExport void
DDS_TypePlugin_return_buffer(struct DDS_TypePlugin *tp,void *buffer);

/*******************/
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*DDS_TypePlugin_serialize_key_T)(struct DDS_TypePlugin *tp,
                                     struct CDR_Stream_t *stream,
                                     const void *sample,
                                     DDS_InstanceHandle_t *destination)
)

#define DDS_TypePlugin_serialize_key(self_,stream_,sample_,dst_) \
((struct DDS_TypePluginI*)(\
(((struct DDS_TypePlugin*)(self_)))->_intf))->serialize_key(self_,stream_,sample_,dst_)

FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*DDS_TypePlugin_deserialize_key_T)(struct DDS_TypePlugin *tp,
                                       void *sample,
                                       struct CDR_Stream_t *stream,
                                       DDS_InstanceHandle_t *source)
)

#define DDS_TypePlugin_deserialize_key(self_,sample_,stream_,src_) \
((struct DDS_TypePluginI*)(\
(((struct DDS_TypePlugin*)(self_)))->_intf))->deserialize_key(self_,sample_,stream_,src_)

typedef RTI_UINT32
(*DDS_TypePlugin_get_serialized_sample_size_T)(struct DDS_TypePlugin *tp,
                                               RTI_UINT32 alignment);

DDSCDllExport RTI_UINT32
DDS_TypePlugin_get_serialized_sample_size_max(struct DDS_TypePlugin *tp);

/*ci \dref_NDDS_Type_InstanceToKeyHashFunc
 */
typedef RTI_UINT32
(*DDS_TypePlugin_get_serialized_key_size_T)(struct DDS_TypePlugin *plugin,
                                            RTI_UINT32 current_alignment);

#define DDS_TypePlugin_get_serialized_key_size(self_,alignment_) \
((struct DDS_TypePluginI*)(\
(((struct DDS_TypePlugin*)(self_)))->_intf))->get_serialized_key_size(self_,alignment_)

/*ci \dref_NDDS_Type_CreateSampleFunc
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*DDS_TypePlugin_create_sample_T)(struct DDS_TypePlugin *plugin,
                                  void **sample)
)

#define DDS_TypeEncapsulationPlugin_has_remove_peer(mp_) \
  (((struct DDS_TypeEncapsulationI*)(mp_)->_intf)->remove_peer != NULL)

#define DDS_TypePlugin_has_create_sample(self_) \
        ((self_)->allocator_plugin->_intf->create_sample != NULL)

#define DDS_TypePlugin_get_sample_state(self_,sample_, state_out_) \
((struct DDS_TypeMemoryI*)(\
(((struct DDS_TypePlugin*)(self_)))->allocator_plugin->_intf))->get_sample_state(self_,sample_, state_out_)

#define DDS_TypePlugin_set_sample_state(self_,sample_, new_state_, revert_to_previous_) \
((struct DDS_TypeMemoryI*)(\
(((struct DDS_TypePlugin*)(self_)))->allocator_plugin->_intf))->set_sample_state(self_,sample_, new_state_, revert_to_previous_)

#define DDS_TypePlugin_create_sample(self_,sample_) \
((struct DDS_TypeMemoryI*)(\
(((struct DDS_TypePlugin*)(self_)))->allocator_plugin->_intf))->create_sample(self_,sample_)

/*ci \dref_NDDS_Type_DeleteSampleFunc
 */
FUNCTION_MUST_TYPEDEF(
RTI_UINT32
(*DDS_TypePlugin_user_sample_size_T)(struct DDS_TypePlugin *plugin))

#define DDS_TypePlugin_initialize_sample(self_,sample_) \
(((struct DDS_TypePluginI*)(self_))->_intf)->initialize_sample(self_,sample_)

/*ci \dref_NDDS_Type_DeleteSampleFunc
 */
FUNCTION_MUST_TYPEDEF(
RTI_INT32
(*DDS_TypePlugin_initialize_sample_T)(struct DDS_TypePlugin *plugin,
                                      void *sample)
)

#define DDS_TypePlugin_initialize_sample(self_,sample_) \
(((struct DDS_TypePluginI*)(self_))->_intf)->initialize_sample(self_,sample_)

/*ci
 * This returns the memory size of the sample
 */
#define DDS_TypePlugin_get_sample_size(self_) \
((struct DDS_TypePluginI*)((self_)->_intf))->sample_size(self_)

/*ci \dref_NDDS_Type_DeleteSampleFunc
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*DDS_TypePlugin_is_sample_consistent_T)(struct DDS_TypePlugin *plugin,
                                         DDS_Boolean *is_data_consistent,
                                         const void *sample,
                                         const struct DDS_SampleInfo *sample_info)
)

typedef enum
{
    TYPEPLUGIN_SAMPLE_STATE_FREE            = 0x1,
    TYPEPLUGIN_SAMPLE_STATE_UNCOMMITTED     = 0x2,
    TYPEPLUGIN_SAMPLE_STATE_COMMITTED       = 0x4
} DDS_LoanedSampleState_T;

FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*DDS_TypePlugin_serialize_inline_qos_T)(struct DDS_TypePlugin *plugin,
                                         struct CDR_Stream_t *stream,
                                         const void *const sample,
                                         DDS_InstanceHandle_t *destination)
)

FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*DDS_TypePlugin_deserialize_inline_qos_T)(struct DDS_TypePlugin *plugin,
                                           struct CDR_Stream_t *stream,
                                           const void *const sample,
                                           DDS_InstanceHandle_t *source)
)

/*ci \dref_NDDS_Type_DeleteSampleFunc
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*DDS_TypePlugin_delete_sample_T)(struct DDS_TypePlugin *plugin,
                                  void *sample)
)

#define DDS_TypePlugin_has_delete_sample(self_) \
        ((self_)->allocator_plugin->_intf->delete_sample != NULL)

MUST_CHECK_RETURN DDSCDllExport DDS_Boolean
DDS_TypePlugin_delete_sample(struct DDS_TypePlugin *tp,void *sample);

/*ci \dref_NDDS_Type_CopySampleFunc
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*DDS_TypePlugin_copy_sample_T)(struct DDS_TypePlugin *plugin,
                                void *dst,
                                const void *src)
)

#define DDS_TypePlugin_copy_sample(self_,dst_,src_) \
((struct DDS_TypePluginI*)(\
(((struct DDS_TypePlugin*)(self_)))->_intf))->copy_sample(self_,dst_,src_)

/*ci \dref_NDDS_Type_InstanceToKeyHashFunc
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*DDS_TypePlugin_instance_to_keyhash_T)(struct DDS_TypePlugin *plugin,
                                        struct CDR_Stream_t *stream,
                                        DDS_KeyHash_t *keyHash,
                                        const void *instance,
                                        DDS_EncapsulationId_t id)
)

#define DDS_TypePlugin_instance_to_keyhash(self_,stream_,kh_,instance_,id_) \
((struct DDS_TypePluginI*)(\
(((struct DDS_TypePlugin*)(self_)))->_intf))->instance_to_keyhash(self_,\
                                                  stream_,kh_,instance_,id_)

/*ci \dref_NDDS_Type_GetKeyKindFunc
 */
FUNCTION_MUST_TYPEDEF(
NDDS_TypePluginKeyKind
(*DDS_TypePlugin_get_key_kind_T)(struct DDS_TypePlugin *plugin)
)

#define DDS_TypePlugin_get_key_kind(self_) \
((struct DDS_TypePluginI*)(\
    (self_)->_intf))->key_kind

/*ci \dref_NDDS_Type_GetKeyKindFunc
 */
FUNCTION_MUST_TYPEDEF(
NDDS_TypePluginKeyKind
(*DDS_TypePluginI_get_key_kind_T)(void)
)

/*i \dref_DDS_TypePluginI_get_interface_T
 */
FUNCTION_MUST_TYPEDEF(
struct DDS_TypePluginI*
(*DDS_TypePluginI_get_interface_T)(void)
)

#define DDS_TypePlugin_get_key_kind(self_) \
((struct DDS_TypePluginI*)(\
    (self_)->_intf))->key_kind

#define DDS_TypePlugin_get_eh_location(self_) \
((struct DDS_TypePluginI*)((self_)->_intf))->eh_location

/*ci \dref_NDDS_Type_CreateTypedDataWriterFunc
 */
typedef void*
(*DDS_TypePlugin_create_typed_datawriter_T)(void *writer);

#define DDS_TypePlugin_has_create_typed_datawriter(self_) \
(((struct DDS_TypePluginI*)(\
    (self_)->_intf))->create_typed_datawriter != NULL)

#define DDS_TypePlugin_create_typed_datawriter(self_,writer_) \
((struct DDS_TypePluginI*)(\
    (self_)->_intf))->create_typed_datawriter(writer_)

/*ci \dref_NDDS_Type_DeleteTypedDataWriterFunc
 */
typedef void
(*DDS_TypePlugin_delete_typed_datawriter_T)(void *wrapper);

#define DDS_TypePlugin_has_delete_typed_datawriter(self_) \
(((struct DDS_TypePluginI*)(\
    (self_)->_intf))->delete_typed_datawriter != NULL)

#define DDS_TypePlugin_delete_typed_datawriter(self_,writer_) \
((struct DDS_TypePluginI*)(\
    (self_)->_intf))->delete_typed_datawriter(writer_)

/*ci \dref_NDDS_Type_CreateTypedDataReaderFunc
 */
typedef void*
(*DDS_TypePlugin_create_typed_datareader_T)(void *reader);

#define DDS_TypePlugin_has_create_typed_datareader(self_) \
(((struct DDS_TypePluginI*)(\
    (self_)->_intf))->create_typed_datareader != NULL)

#define DDS_TypePlugin_create_typed_datareader(self_,reader_) \
((struct DDS_TypePluginI*)(\
    (self_)->_intf))->create_typed_datareader(reader_)

/*ci \dref_NDDS_Type_DeleteTypedDataReaderFunc
 */
typedef void
(*DDS_TypePlugin_delete_typed_datareader_T)(void *wrapper);

#define DDS_TypePlugin_has_delete_typed_datareader(self_) \
(((struct DDS_TypePluginI*)(\
    (self_)->_intf))->delete_typed_datareader != NULL)

#define DDS_TypePlugin_delete_typed_datareader(self_,reader_) \
((struct DDS_TypePluginI*)(\
    (self_)->_intf))->delete_typed_datareader(reader_)


typedef enum
{
    DDS_TYPEPLUGIN_MODE_READER,
    DDS_TYPEPLUGIN_MODE_WRITER
} DDS_TypePluginMode_T;

typedef void DDS_TypePluginEndpoint;
typedef void DDS_TypePluginEndpointQos;
struct DDS_TypePluginI;

typedef struct DDS_TypePlugin*
(*DDS_TypePlugin_create_T)(
        DDS_DomainParticipant *participant,
        struct DDS_DomainParticipantQos *dp_qos,
        DDS_TypePluginMode_T endpoint_mode,
        DDS_TypePluginEndpoint *endpoint,
        DDS_TypePluginEndpointQos *qos,
        struct DDS_TypePluginProperty *property);

#define DDS_TypePlugin_create(intf_,dp_,q_,em_,e_,eq_,p_) \
    (intf_)->create_plugin(dp_,q_,em_,e_,eq_,p_)

typedef RTI_BOOL
(*DDS_TypePlugin_delete_T)(struct DDS_TypePlugin *plugin);


typedef DDS_Boolean
(*DDS_TypePlugin_on_type_registered_T)(struct DDS_TypeImpl* type_impl);

typedef DDS_Boolean
(*DDS_TypePlugin_on_type_unregistered_T)(struct DDS_TypeImpl* type_impl);

DDSCDllExport void
DDS_TypePlugin_delete(struct DDS_TypePlugin *plugin);

struct DDS_TypeEncapsulationPlugin;
struct DDS_TypeMemoryPlugin;

typedef struct DDS_TypeEncapsulationPlugin*
(*DDS_TypeEncapsulationPlugin_create_T)(struct DDS_TypePlugin *tp,
                                       DDS_DomainParticipant *participant,
                                       struct DDS_DomainParticipantQos *dp_qos,
                                       DDS_TypePluginMode_T endpoint_mode,
                                       DDS_TypePluginEndpoint *endpoint,
                                       DDS_TypePluginEndpointQos *qos,
                                       struct DDS_TypeMemoryPlugin *mp);

typedef void
(*DDS_TypeEncapsulationPlugin_delete_T)(struct DDS_TypePlugin *tp,
                                        struct DDS_TypeEncapsulationPlugin *plugin);

#define DDS_TypeEncapsulationPlugin_get_kind(p_) \
    (struct DDS_TypeEncapsulationI*)((p_)->_intf)->encapsulation_kind

typedef struct NDDSCDREncapsulation*
(*DDS_TypeEncapsulationPlugin_get_kind_T)(void);

typedef struct DDS_TypeMemory*
(*DDS_TypeEncapsulationPlugin_get_memory_kind_T)(void);

typedef RTI_BOOL
(*DDS_TypeEncapsulationPlugin_add_peer_T)(struct DDS_TypeEncapsulationPlugin *ep,
                                          DDS_InstanceHandle_t *peer);

#define DDS_TypeEncapsulationPlugin_has_add_peer(mp_) \
  (((struct DDS_TypeEncapsulationI*)(mp_)->_intf)->add_peer != NULL)

typedef RTI_BOOL
(*DDS_TypeEncapsulationPlugin_remove_peer_T)(struct DDS_TypeEncapsulationPlugin *ep,
                                             DDS_InstanceHandle_t *peer);

typedef RTI_UINT32
(*DDS_TypeEncapsulationPlugin_get_serialized_sample_size_T)(
                                        struct DDS_TypePlugin *tp,
                                        struct DDS_TypeEncapsulationPlugin *ep,
                                        RTI_UINT32 alignment);

#define DDS_TypeEncapsulationPlugin_has_remove_peer(mp_) \
  (((struct DDS_TypeEncapsulationI*)(mp_)->_intf)->remove_peer != NULL)

struct DDS_TypeEncapsulationI
{
    DDS_DataRepresentationId_t representation_id;

    DDS_DataRepresentationId_t *representation_id_aliases;

    NDDSCDREncapsulation* encapsulation_kind;

    NDDSMemoryType memory_type;

    NDDSMemoryManager memory_id;

    DDS_TypeEncapsulationPlugin_add_peer_T add_peer;

    DDS_TypeEncapsulationPlugin_remove_peer_T remove_peer;

    DDS_TypePlugin_get_buffer_T get_buffer;

    DDS_TypePlugin_return_buffer_T return_buffer;

    DDS_TypePlugin_get_sample_T get_sample;

    DDS_TypePlugin_return_sample_T return_sample;

    /*ci \dref_NDDS_Type_Plugin_serialize_data
     */
    DDS_TypePlugin_serialize_sample_T serialize;

    /*ci \dref_NDDS_Type_Plugin_deserialize_data
     */
    DDS_TypePlugin_deserialize_sample_T deserialize;

    DDS_TypeEncapsulationPlugin_get_serialized_sample_size_T get_serialized_sample_size;

    DDS_TypeEncapsulationPlugin_create_T create_plugin;

    DDS_TypeEncapsulationPlugin_delete_T delete_plugin;
};

typedef struct DDS_TypeMemoryPlugin*
(*DDS_TypeMemoryPlugin_create_T)(struct DDS_TypePlugin *tp,
                                 DDS_DomainParticipant *participant,
                                 struct DDS_DomainParticipantQos *dp_qos,
                                 DDS_TypePluginMode_T endpoint_mode,
                                 DDS_TypePluginEndpoint *endpoint,
                                 DDS_TypePluginEndpointQos *qos);

typedef void
(*DDS_TypeMemoryPlugin_delete_T)(struct DDS_TypePlugin *tp,
                                 struct DDS_TypeMemoryPlugin*);

typedef struct DDS_TypeMemory*
(*DDS_TypeMemoryPlugin_get_kind_T)(void);

typedef RTI_BOOL
(*DDS_TypeMemoryPlugin_get_reference_T)(struct DDS_TypeMemoryPlugin *mp,
                                        const void *address,
                                        void *reference);


FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*DDS_TypeMemoryPlugin_serialize_inline_qos_T)(struct DDS_TypePlugin *tp,
                                               struct DDS_TypeMemoryPlugin *plugin,
                                               struct CDR_Stream_t *stream,
                                               const void *const sample,
                                               DDS_InstanceHandle_t *destination)
)

#define DDS_TypeMemoryPlugin_has_serialize_inline_qos(mp_) \
  (((struct DDS_TypeMemoryI*)(mp_)->_intf)->serialize_inline_qos != NULL)

FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*DDS_TypeMemoryPlugin_deserialize_inline_qos_T)(
                                           struct DDS_TypePlugin *tp,
                                           struct DDS_TypeMemoryPlugin *plugin,
                                           struct CDR_Stream_t *stream,
                                           const void *const sample,
                                           DDS_InstanceHandle_t *source)
)

#define DDS_TypeMemoryPlugin_has_deserialize_inline_qos(mp_) \
  (((struct DDS_TypeMemoryI*)(mp_)->_intf)->deserialize_inline_qos != NULL)

FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*DDS_TypeMemoryPlugin_is_sample_consistent_T)(struct DDS_TypePlugin *tp,
                                         struct DDS_TypeMemoryPlugin *mp,
                                         DDS_Boolean *is_data_consistent,
                                         const void *sample,
                                         const struct DDS_SampleInfo *sample_info)
)

#define DDS_TypeMemoryPlugin_has_is_sample_consistent(mp_) \
  (((struct DDS_TypeMemoryI*)(mp_)->_intf)->is_sample_consistent != NULL)

#define DDS_TypeMemoryPlugin_has_get_reference(mp_) \
 (((struct DDS_TypeMemoryI*)(mp_)->_intf)->get_reference != NULL)

#define DDS_TypeMemoryPlugin_get_reference(mp_,addr_,ref_) \
 ((struct DDS_TypeMemoryI*)(mp_)->_intf)->get_reference((mp_),(addr_),(ref_))

typedef void*
(*DDS_TypeMemoryPlugin_get_address_T)(struct DDS_TypeMemoryPlugin *mp,
                                      void *reference);

#define DDS_TypeMemoryPlugin_get_address(mp_,ref_) \
 ((struct DDS_TypeMemoryI*)(mp_)->_intf)->get_address((mp_),(ref_))

typedef RTI_BOOL
(*DDS_TypeMemoryPlugin_is_owner_T)(struct DDS_TypeMemoryPlugin *mp,
                                   const void *sample);

#define DDS_TypeMemoryPlugin_is_owner(mp_,sample_) \
 ((struct DDS_TypeMemoryI*)(mp_)->_intf)->is_owner((mp_),(sample_))

#define DDS_TypePlugin_is_managed_samples(p_) \
(((p_)->allocator_plugin->_intf->id == RTI_MEMORY_MANAGER_SHMEM) || \
((p_)->allocator_plugin->_intf->id == RTI_MEMORY_MANAGER_SHMEMV2) || \
 ((p_)->allocator_plugin->_intf->id == RTI_MEMORY_MANAGER_HEAP_MANAGED))

#define DDS_TypeMemoryPlugin_has_owner(mp_) \
  (((struct DDS_TypeMemoryI*)(mp_)->_intf)->is_owner != NULL)

#define DDS_TypePlugin_get_allocator_plugin_memory_kind(tp_) \
        (tp_)->allocator_plugin->_intf->id

#define DDS_TypePlugin_allocator_plugin_is_v2(tp_) \
        ((tp_)->allocator_plugin->_intf->id == RTI_MEMORY_MANAGER_SHMEMV2)

#define DDS_TypePlugin_allocator_plugin_is_v1(tp_) \
        ((tp_)->allocator_plugin->_intf->id == RTI_MEMORY_MANAGER_SHMEM)


typedef RTI_BOOL
(*DDS_TypeMemoryPlugin_add_peer_T)(struct DDS_TypeMemoryPlugin *mp,
                                          DDS_InstanceHandle_t *peer);

#define DDS_TypeMemoryPlugin_has_add_peer(mp_) \
  (((struct DDS_TypeMemoryI*)(mp_)->_intf)->add_peer != NULL)

typedef RTI_BOOL
(*DDS_TypeMemoryPlugin_remove_peer_T)(struct DDS_TypeMemoryPlugin *mp,
                                      DDS_InstanceHandle_t *peer);

#define DDS_TypeMemoryPlugin_has_remove_peer(mp_) \
  (((struct DDS_TypeMemoryI*)(mp_)->_intf)->remove_peer != NULL)

FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*DDS_TypeMemoryPlugin_return_address_T)(struct DDS_TypeMemoryPlugin *mp,
                                         void *address)
)

#define DDS_TypeMemoryPlugin_has_return_address(mp_) \
  (((struct DDS_TypeMemoryI*)(mp_)->_intf)->return_address != NULL)


typedef RTI_BOOL
(*DDS_TypeMemoryPlugin_get_sample_state_T)(struct DDS_TypePlugin *mp,
                                           const void *sample,
                                           DDS_LoanedSampleState_T *state_out);
typedef RTI_BOOL
(*DDS_TypeMemoryPlugin_set_sample_state_T)(struct DDS_TypePlugin *mp,
                                           const void *sample,
                                           DDS_LoanedSampleState_T new_state,
                                           DDS_Boolean revert_to_previous_state);

struct DDS_TypeMemoryI
{
    NDDSMemoryManager id;

    NDDSMemoryType type;

    /*ci \dref_NDDS_Type_Plugin_create_sample
     */
    DDS_TypePlugin_create_sample_T create_sample;

    /*ci \dref_NDDS_Type_Plugin_delete_sample
     */
    DDS_TypePlugin_delete_sample_T delete_sample;

    DDS_TypeMemoryPlugin_get_address_T get_address;

    DDS_TypeMemoryPlugin_return_address_T return_address;

    DDS_TypeMemoryPlugin_get_reference_T get_reference;

    DDS_TypeMemoryPlugin_get_sample_state_T get_sample_state;

    DDS_TypeMemoryPlugin_set_sample_state_T set_sample_state;

    DDS_TypeMemoryPlugin_is_owner_T is_owner;

    DDS_TypeMemoryPlugin_add_peer_T add_peer;

    DDS_TypeMemoryPlugin_remove_peer_T remove_peer;

    DDS_TypeMemoryPlugin_serialize_inline_qos_T serialize_inline_qos;

    DDS_TypeMemoryPlugin_deserialize_inline_qos_T deserialize_inline_qos;

    DDS_TypeMemoryPlugin_is_sample_consistent_T is_sample_consistent;

    DDS_TypeMemoryPlugin_create_T create_plugin;

    DDS_TypeMemoryPlugin_delete_T delete_plugin;
};

#define DDS_TYPE_ENCAPSULATION_MAX_IDS (2)

struct DDS_TypeEncapsulationPlugin
{
    struct REDA_CircularListNode _node;

    struct DDS_TypeEncapsulationI *_intf;

    struct DDS_TypeMemoryPlugin *memory_plugin;
};

struct DDS_TypeMemoryPlugin
{
    struct REDA_CircularListNode _node;

    struct DDS_TypeMemoryI *_intf;
};

/*e \dref_NDDS_Type_Plugin
 */
typedef struct DDS_TypePluginI
{
    /**************************************************************************
     *                   Type information functions
     **************************************************************************/

    /*ci \dref_NDDS_Type_Plugin_type_code
     */
    DDS_TypeCode_t* type_code;

    /*ci \dref_NDDS_Type_Plugin_key_kind
     */
    NDDS_TypePluginKeyKind key_kind;

    NDDS_TypePluginEhdrLocation eh_location;

    DDS_TypePlugin_user_sample_size_T sample_size;

    NDDSMemoryType memory_type;

    /*i \dref_NDDS_Type_Plugin_instance_to_keyhash
     */
    DDS_TypePlugin_instance_to_keyhash_T instance_to_keyhash;

    /*i \dref_NDDS_Type_Plugin_copy_sample
     */
    DDS_TypePlugin_copy_sample_T copy_sample;

    /*i \dref_NDDS_Type_Plugin_initialize_sample
     */
    DDS_TypePlugin_initialize_sample_T initialize_sample;

    /*i \dref_NDDS_Type_Plugin_serialize_key
     */
    DDS_TypePlugin_serialize_key_T serialize_key;

    /*i \dref_NDDS_Type_Plugin_deserialize_key
     */
    DDS_TypePlugin_deserialize_key_T deserialize_key;

    /*i \dref_NDDS_Type_Plugin_get_serializedKeyMaxSize
     */
    DDS_TypePlugin_get_serialized_key_size_T get_serialized_key_size;

    /*i \dref_NDDS_Type_Plugin__add_peer
     */
    DDS_TypePlugin_add_peer_T add_peer;

    /*i \dref_NDDS_Type_Plugin_remove_peer
     */
    DDS_TypePlugin_remove_peer_T remove_peer;

    /*i \dref_NDDS_Type_Plugin_remove_peer
     */
    DDS_TypePlugin_serialize_inline_qos_T serialize_inline_qos;

    /*i \dref_NDDS_Type_Plugin_remove_peer
     */
    DDS_TypePlugin_deserialize_inline_qos_T deserialize_inline_qos;

    DDS_TypePlugin_is_sample_consistent_T is_sample_consistent;

    /*i \brief Statically added memory interfaces
     */
    struct DDS_TypeMemoryI **memory_intf;

    /*i \brief Statically added wire interfaces
     */
    struct DDS_TypeEncapsulationI **wire_intf;

    /**************************************************************************
     *       Helper APIs to create language binding wrapper Functions
     **************************************************************************/

    /*i \dref_NDDS_Type_Plugin_create_typed_datawriter
     */
    DDS_TypePlugin_create_typed_datawriter_T create_typed_datawriter;

    /*i \dref_NDDS_Type_Plugin_delete_typed_datawriter
     */
    DDS_TypePlugin_delete_typed_datawriter_T delete_typed_datawriter;

    /*i \dref_NDDS_Type_Plugin_create_typed_datareader
     */
    DDS_TypePlugin_create_typed_datareader_T create_typed_datareader;

    /*i \dref_NDDS_Type_Plugin_delete_typed_datareader
     */
    DDS_TypePlugin_delete_typed_datareader_T delete_typed_datareader;

    DDS_TypePlugin_create_T create_plugin;

    DDS_TypePlugin_delete_T delete_plugin;

    DDS_TypePlugin_on_type_registered_T on_type_registered;

    DDS_TypePlugin_on_type_unregistered_T on_type_unregistered;

#if DDS_XTYPES_IS_ENABLED
    /*ci \brief Optional pointer to interface for compiled types
     */
    const struct DDS_TypeInterfaceI *type_factory;
#endif
} DDS_TypePluginI;

#if DDS_XTYPES_IS_ENABLED
#define DDS_TypePluginI_XTYPES_INITIALIZER  \
, NULL
#else
#define DDS_TypePluginI_XTYPES_INITIALIZER
#endif

#define DDS_TypePluginI_INITIALIZER \
{\
    NULL,\
    0,\
    0,\
    0,\
    NDDS_TYPEPLUGIN_EH_LOCATION_PAYLOAD,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,/*add_peer*/\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,/* memory_intf */\
    NULL,/*create_typed_datawriter */\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL\
    DDS_TypePluginI_XTYPES_INITIALIZER \
}

/*ce \dref_TypePlugin
 */
#define NDDS_Type_Plugin DDS_TypePlugin

struct DDS_TypePluginProperty
{
    DDS_Long head_padding;
    DDS_Long tail_padding;
    void *plugin_param;
    DDS_Long max_buffers;
#if DDS_XTYPES_IS_ENABLED
    struct RTIXCdrTypePluginProgramContext *program_context;
    struct RTIXCdrInterpreterPrograms *programs;
    struct DDS_TypeCode *type_code;
    void *flat_data_plain_helper;
#endif
};

#if DDS_XTYPES_IS_ENABLED
#define DDS_TypePluginProperty_XTYPES_INITIALIZER \
    ,NULL,\
    NULL,\
    NULL, \
    NULL
#else
#define DDS_TypePluginProperty_XTYPES_INITIALIZER
#endif

#define DDS_TypePluginProperty_INITIALIZER \
{\
    0,\
    0,\
    NULL,\
    0\
    DDS_TypePluginProperty_XTYPES_INITIALIZER\
}

typedef struct DDS_TypePlugin
{
    const struct DDS_TypePluginI *_intf;

    REDA_CircularList_T memory_plugins;

    REDA_CircularList_T wire_plugins;

    struct DDS_TypePluginProperty property;

    /* Current memory plugin */
    struct DDS_TypeMemoryPlugin *memory_plugin;

    /* Current wire plugin */
    struct DDS_TypeEncapsulationPlugin *wire_plugin;

    /* Current memory plugin */
    struct DDS_TypeMemoryPlugin *allocator_plugin;

    /* Current wire interface to */
    struct DDS_TypeEncapsulationI *wp_intf;

    struct DDS_TypeMemoryI *mp_intf;

    DDS_EncapsulationId_t current_encapsulation;

    RTI_BOOL big_endian;

    DDS_DomainParticipant *dp;

    DDS_EncapsulationId_t cdr_id;

    DDS_DataRepresentationId_t representation[DDS_TYPE_ENCAPSULATION_MAX_IDS];

    DDS_Long representation_count;

    /* Since the DDS_TypePlugin only has one allocator
     * This will be the maximum serialized size
     */
    RTI_UINT32 max_serialized_size;
    RTI_BOOL is_max_serialized_size_set;





} DDS_TypePlugin;







#define DDS_TypePlugin_TRUST_INITIALIZER


#if RTI_ENDIAN_LITTLE
#define DDS_TypePlugin_BIG_ENDIAN       RTI_FALSE
#else
#define DDS_TypePlugin_BIG_ENDIAN       RTI_TRUE
#endif /* RTI_ENDIAN_LITTLE */

#define DDS_TypePlugin_INITIALIZER \
{\
    NULL /* _intf */, \
    REDA_CircularList_INITIALIZER /* memory_plugins */, \
    REDA_CircularList_INITIALIZER /* wire_plugins */, \
    DDS_TypePluginProperty_INITIALIZER /* property */, \
    NULL /* memory_plugin */, \
    NULL /* wire_plugin */, \
    NULL /* allocator_plugin */, \
    NULL /* wp_intf */, \
    NULL /* mp_intf */, \
    DDS_ENCAPSULATION_ID_CDR /* current_encapsulation */,\
    DDS_TypePlugin_BIG_ENDIAN /* big_endian */,\
    NULL, /* dp */ \
    0,\
    {0},\
    0,\
    0,\
    RTI_FALSE \
    DDS_TypePlugin_TRUST_INITIALIZER \
}

DDSCDllExport DDS_Boolean
DDS_TypePlugin_set_stream_encapsulation(struct DDS_TypePlugin *tp,
                                        struct CDR_Stream_t *stream,
                                        DDS_EncapsulationId_t id);

DDSCDllExport DDS_Boolean
DDS_TypePlugin_set_encapsulation(struct DDS_TypePlugin *plugin,
                                 NDDSCDREncapsulationId id);

DDSCDllExport NDDSCDREncapsulationId
DDS_TypePlugin_get_encapsulation(struct DDS_TypePlugin *plugin);

DDSCDllExport DDS_Boolean
DDS_TypePlugin_set_allocator(struct DDS_TypePlugin *plugin,
                             NDDSMemoryManager id);


DDSCDllExport struct DDS_TypePlugin*
DDS_TypePlugin_create_w_intf(struct DDS_TypePluginI *intf,
                             DDS_DomainParticipant *participant,
                             struct DDS_DomainParticipantQos *dp_qos,
                             DDS_TypePluginMode_T endpoint_mode,
                             DDS_TypePluginEndpoint *endpoint,
                             DDS_TypePluginEndpointQos *qos,
                             struct DDS_TypePluginProperty *property);

DDSCDllExport void
DDS_TypePlugin_initialize(struct DDS_TypePlugin *plugin);

struct DDS_TypePluginBuffer
{
    /*ci
     * \brief Link together multiple payload buffers
     */
    struct DDS_TypePluginBuffer *_next;
    struct DDS_TypeEncapsulationPlugin *wp;
    const void *data;
    DDS_EncapsulationId_t encapsulation;
    struct NETIO_PacketBuffer data_pbuf;
};

#define DDS_TypePluginBuffer_INITIALIZER \
{ \
    NULL,NULL,NULL,\
    DDS_ENCAPSULATION_ID_CDR_NATIVE,\
    NETIO_PacketBuffer_INITIALIZER \
}

DDSCDllExport DDS_Boolean
DDS_TypePlugin_set_stream(struct DDS_TypePlugin *tp,
                          struct CDR_Stream_t *stream,
                          struct DDS_TypePluginBuffer *tbuf);

DDSCDllExport void
DDS_TypePluginBuffer_initialize_static(struct DDS_TypePluginBuffer *tbuf,
                                       char *buffer,
                                       unsigned int length);

DDSCDllExport void*
DDS_TypePlugin_init_inline_buffer(struct DDS_TypeEncapsulationPlugin *wp,
                                  REDA_BufferPool_T pool,RTI_UINT32 size);

/* ================================================================= */
/*              Non-Type Specific Type Support Functions             */
/* ================================================================= */

/*ci \dref_PluginHelper_instance_to_keyhash
 */
MUST_CHECK_RETURN DDSCDllExport RTI_BOOL
PluginHelper_instance_to_keyhash(struct NDDS_Type_Plugin *plugin,
                                 struct CDR_Stream_t *stream,
                                 DDS_KeyHash_t *keyHash,
                                 const void *instance,
                                 DDS_EncapsulationId_t id);

/*ci \dref_PluginHelper_get_key_kind
 */
MUST_CHECK_RETURN DDSCDllExport NDDS_TypePluginKeyKind
PluginHelper_get_key_kind(struct NDDS_Type_Plugin *plugin,void *param);

MUST_CHECK_RETURN DDSCDllExport DDS_Boolean
PluginHelper_serialize_sequence(struct DDS_TypePlugin *plugin,
                                struct CDR_Stream_t *cdrs,
                                const struct REDA_Sequence *in,
                                DDS_InstanceHandle_t *destination,
                                DDS_TypePlugin_serialize_sample_T  serialize);

MUST_CHECK_RETURN DDSCDllExport DDS_Boolean
PluginHelper_deserialize_sequence(struct DDS_TypePlugin *plugin,
                                  struct REDA_Sequence *out,
                                  struct CDR_Stream_t *cdrs,
                                  DDS_InstanceHandle_t *source,
                                  DDS_TypePlugin_deserialize_sample_T  deserialize);

DDSCDllExport RTI_UINT32
PluginHelper_get_max_size_serialized_sequence(struct DDS_TypePlugin *plugin,
                                              RTI_UINT32 current_alignment,
                                              RTI_UINT32 length,
                                              DDS_TypePlugin_get_serialized_sample_size_T get_serialized_size_func);

MUST_CHECK_RETURN DDSCDllExport DDS_Boolean
PluginHelper_serialize_array(struct DDS_TypePlugin *plugin,
                             struct CDR_Stream_t *cdrs,
                             const void* in,
                             RTI_UINT32 length,
                             RTI_UINT32 element_size,
                             DDS_InstanceHandle_t *destination,
                             DDS_TypePlugin_serialize_sample_T serialize_function);

MUST_CHECK_RETURN DDSCDllExport DDS_Boolean
PluginHelper_deserialize_array(struct DDS_TypePlugin *plugin,
                               void* out,
                               struct CDR_Stream_t *cdrs,
                               RTI_UINT32 length,
                               RTI_UINT32 element_size,
                               DDS_InstanceHandle_t *source,
                               DDS_TypePlugin_deserialize_sample_T deserialize_function);

DDSCDllExport RTI_UINT32
PluginHelper_get_max_size_serialized_array(struct DDS_TypePlugin *plugin,
                                           RTI_UINT32 current_alignment,
                                           RTI_UINT32 length,
                                           DDS_TypePlugin_get_serialized_sample_size_T get_serialized_size_func);

/* ================================================================= */
/*              Default Type Plugin                                  */
/* ================================================================= */

struct DDS_DefaultHeapPlugin
{
    struct DDS_TypeMemoryPlugin _parent;
};

struct DDS_DefaultCdrPlugin
{
    struct DDS_TypeEncapsulationPlugin _parent;
    RTI_UINT32 size;
};

struct DDS_TypePluginDefault
{
    struct DDS_TypePlugin _parent;
    struct DDS_DefaultCdrPlugin cdr_plugin;
    struct DDS_DefaultHeapPlugin heap_plugin;
    REDA_BufferPool_T pool;
};

typedef RTI_BOOL
(*DDS_TypePlugin_initialize_pool_sample_T)(void *init_config, void *buffer);

typedef RTI_BOOL
(*DDS_TypePlugin_finalize_pool_sample_T)(void *finalize_config, void *buffer);

DDSCDllExport struct DDS_TypeMemoryPlugin*
DDS_TypePluginDefaultHeap_create(struct DDS_TypePlugin *tp,
                                DDS_DomainParticipant *participant,
                                struct DDS_DomainParticipantQos *dp_qos,
                                DDS_TypePluginMode_T endpoint_mode,
                                DDS_TypePluginEndpoint *endpoint,
                                DDS_TypePluginEndpointQos *qos,
                                DDS_TypePlugin_initialize_pool_sample_T init_sample,
                                DDS_TypePlugin_initialize_pool_sample_T finalize_sample);

DDSCDllExport struct DDS_TypePlugin*
DDS_TypePluginDefault_create(struct DDS_TypePluginI *intf,
                           DDS_DomainParticipant *participant,
                           struct DDS_DomainParticipantQos *dp_qos,
                           DDS_TypePluginMode_T endpoint_mode,
                           DDS_TypePluginEndpoint *endpoint,
                           DDS_TypePluginEndpointQos *qos,
                           struct DDS_TypePluginProperty *const property);

DDSCDllExport DDS_Boolean
DDS_TypePluginDefault_delete(struct DDS_TypePlugin *plugin);

DDSCDllExport struct DDS_TypeEncapsulationPlugin*
DDS_TypePluginDefaultCdr_create(struct DDS_TypePlugin *tp,
                               DDS_DomainParticipant *participant,
                               struct DDS_DomainParticipantQos *dp_qos,
                               DDS_TypePluginMode_T endpoint_mode,
                               DDS_TypePluginEndpoint *endpoint,
                               DDS_TypePluginEndpointQos *qos,
                               struct DDS_TypeMemoryPlugin *mp,
                               RTI_UINT32 buf_size);

DDSCDllExport void*
DDS_TypePluginDefaultCdr_get_buffer(struct DDS_TypePlugin *tp);

DDSCDllExport void
DDS_TypePluginDefaultCdr_return_buffer(struct DDS_TypePlugin *tp,void *buffer);

DDSCDllExport struct DDS_TypePluginSampleHolder*
DDS_TypePluginDefaultCdr_get_sample(struct DDS_TypePlugin *tp,
                                struct CDR_Stream_t *stream);

DDSCDllExport void
DDS_TypePluginDefaultCdr_return_sample(struct DDS_TypePlugin *tp,
                                   struct DDS_TypePluginSampleHolder *sample);

DDSCDllExport DDS_Boolean
DDS_Type_representation_is_supported(struct DDS_TypePluginI *tp,
                                     DDS_DataRepresentationId_t id);

DDSCDllExport DDS_Boolean
DDS_Type_valid_representations(struct DDS_TypePluginI *tp,
                               struct DDS_DataRepresentationIdSeq *seq);


DDSCDllExport DDS_EncapsulationId_t
DDS_TypePlugin_match_representation(struct DDS_TypePlugin *tp,
                                    DDS_DataRepresentationId_t id);

DDSCDllExport DDS_Boolean
DDS_TypePlugin_is_sample_consistent(struct DDS_TypePlugin *tp,
                                    DDS_Boolean *is_data_consistent,
                                    const void *sample,
                                    const struct DDS_SampleInfo *sample_info);

DDSCDllExport DDS_Boolean
DDS_TypePlugin_serialize_inline_qos(struct DDS_TypePlugin *tp,
                                    struct CDR_Stream_t *stream,
                                    const void *const sample,
                                    DDS_InstanceHandle_t *destination);

DDSCDllExport DDS_Boolean
DDS_TypePlugin_is_representation_supported(struct DDS_TypePlugin *plugin,
                                           DDS_DataRepresentationId_t dr_id);

DDSCDllExport DDS_Boolean
DDS_TypePlugin_deserialize_inline_qos(struct DDS_TypePlugin *tp,
                                      struct CDR_Stream_t *stream,
                                      const void *const sample,
                                      DDS_InstanceHandle_t *source);

DDSCDllExport DDS_Boolean
DDS_TypePlugin_return_address(struct DDS_TypePlugin *tp,
                              void *address);

DDSCDllExport struct DDS_TypeEncapsulationPlugin*
DDS_TypePlugin_find_encapsulation_plugin(struct DDS_TypePlugin *plugin,
                                DDS_EncapsulationId_t enc_id,
                                DDS_DataRepresentationId_t dr_id);

DDSCDllExport DDS_EncapsulationId_t
DDS_TypePlugin_get_cdr_encapsulation(struct DDS_TypePlugin *plugin);

DDSCDllExport DDS_EncapsulationId_t
DDS_TypePlugin_resolve_encapsulation(DDS_EncapsulationId_t cdr_id,
                                     const struct DDS_DataRepresentationQosPolicy *dr);

DDSCDllExport DDS_Boolean
DDS_TypePlugin_is_v1_and_v2_enabled(struct DDS_TypePlugin *plugin);

DDSCDllExport DDS_Boolean
DDS_TypePlugin_is_representation_enabled(struct DDS_TypePlugin *plugin,
                          DDS_DataRepresentationId_t dr_id);

DDSCDllExport DDS_ReturnCode_t
DDS_TypeSupport_resolve_representation(
    DDS_DataRepresentationId_t *representation,
    DDS_DataRepresentationId_t auto_representation,
    DDS_DataRepresentationId_t xcdr1_representation,
    DDS_DataRepresentationId_t xcdr2_representation,
    DDS_EncapsulationId_t *encapsulation);

#if DDS_XTYPES_IS_ENABLED

DDSCDllExport void
DDS_TypePlugin_initialize_static(struct DDS_TypePlugin *plugin,
                                 const struct DDS_TypePluginI *intf,
                                 const struct RTIXCdrInterpreterPrograms *programs);

DDSCDllExport void
DDS_TypeImpl_set_programs(struct DDS_TypeImpl *type, struct RTIXCdrInterpreterPrograms *programs);

DDSCDllExport struct RTIXCdrInterpreterPrograms*
DDS_TypeImpl_get_programs(struct DDS_TypeImpl *type);

DDSCDllExport void
DDS_TypeImpl_set_sample(struct DDS_TypeImpl *type, void *plain_sample);

DDSCDllExport void*
DDS_TypeImpl_get_sample(struct DDS_TypeImpl *type);

DDSCDllExport void
DDS_TypeImpl_set_typecode(struct DDS_TypeImpl *type, DDS_TypeCode *programs);

DDSCDllExport DDS_TypeCode*
DDS_TypeImpl_get_typecode(struct DDS_TypeImpl *type);

typedef struct DDS_TypeProgramNode
{
    struct REDA_CircularListNode _node;
    struct RTIXCdrInterpreterPrograms *programs;
    struct DDS_TypePluginI *type_intf;
    struct RTIXCdrTypePluginProgramContext *context;
} DDS_TypeProgramNode;

#define DDS_TypeProgramNode_INITIALIZER \
{ \
    REDA_CircularList_INITIALIZER,\
    NULL,\
    NULL,\
    NULL\
}

#endif

#define DDS_TypePlugin_get_participant(p_) (p_)->dp

#ifdef __cplusplus
}                               /* extern "C" */
#endif


#endif /* dds_c_type_h */
