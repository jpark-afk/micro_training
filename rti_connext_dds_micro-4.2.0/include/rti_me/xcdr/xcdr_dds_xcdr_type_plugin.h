/*
 * FILE: dds_c_xcdr_type_plugin.h
 *
 * Copyright (c) 2018-2025 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef dds_xcdr_type_plugin_h
#define dds_xcdr_type_plugin_h

#include "xcdr/xcdr_dll.h"

#ifndef cdr_encapsulation_h
#include "cdr/cdr_encapsulation.h"
#endif
#include  "reda/reda_bufferpool.h"
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#include "xcdr/xcdr_heapmgr.h"
#include "xcdr/xcdr_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct DDS_TypeInterfaceI XCdrTypeInterfaceI_gv_Intf;

struct DDS_XTypesHeapPlugin
{
    struct DDS_TypeMemoryPlugin _parent;
    struct XCDR_HeapMgr *heap_manager;
    REDA_BufferPool_T sample_holder_pool;
    DDS_TypePluginMode_T endpoint_mode;
    RTI_BOOL is_managing_flat_data_samples;
    RTI_BOOL initialize_sample;

};

struct XCDR_StreamPlugin
{
    struct DDS_TypeEncapsulationPlugin _parent;

    /*
     * This encapsulation/wire plugin will use this heap manager.
     * This heap manager will maintain a pool of sample holders
     * which will be used by the DataReaders
     */
    struct DDS_XTypesHeapPlugin* heap_memory_manager;
    REDA_BufferPool_T serialization_buffers_buf_pool;
    RTI_UINT32 size;
};

/******************************************************************************
 *                         XCDRv1 Stream Plugin Functions
 ******************************************************************************/

RTIXCdrDllExport void
XCDR_Interpreter_map_xcdrstream_to_micro_cdr_stream(
        RTIXCdrStream *pro_xcdr_stream,
        struct CDR_Stream_t *micro_cdr_stream);

RTIXCdrDllExport struct DDS_TypeEncapsulationPlugin*
XCDRv1_StreamPlugin_create(
        struct DDS_TypePlugin *tp,
        DDS_DomainParticipant *participant,
        struct DDS_DomainParticipantQos *dp_qos,
        DDS_TypePluginMode_T endpoint_mode,
        DDS_TypePluginEndpoint *endpoint,
        DDS_TypePluginEndpointQos *qos,
        struct DDS_TypeMemoryPlugin *mp);

RTIXCdrDllExport void
XCDRv1_StreamPlugin_delete(
        struct DDS_TypePlugin *p,
        struct DDS_TypeEncapsulationPlugin *mp);

RTIXCdrDllExport RTI_UINT32
XCDRv1_StreamPlugin_get_serialized_sample_size(
        struct DDS_TypePlugin *tp,
        struct DDS_TypeEncapsulationPlugin *ep,
        RTI_UINT32 alignment);

/******************************************************************************
 *                         XCDRv2 Stream Plugin Functions
 ******************************************************************************/

RTIXCdrDllExport struct DDS_TypeEncapsulationPlugin*
XCDRv2_StreamPlugin_create(
        struct DDS_TypePlugin *tp,
        DDS_DomainParticipant *participant,
        struct DDS_DomainParticipantQos *dp_qos,
        DDS_TypePluginMode_T endpoint_mode,
        DDS_TypePluginEndpoint *endpoint,
        DDS_TypePluginEndpointQos *qos,
        struct DDS_TypeMemoryPlugin *mp);

RTIXCdrDllExport void
XCDRv2_StreamPlugin_delete(
        struct DDS_TypePlugin *p,
        struct DDS_TypeEncapsulationPlugin *mp);

RTIXCdrDllExport RTI_UINT32
XCDRv2_StreamPlugin_get_serialized_sample_size(
        struct DDS_TypePlugin *tp,
        struct DDS_TypeEncapsulationPlugin *ep,
        RTI_UINT32 alignment);


/******************************************************************************
 *                         Generic Stream Plugin Functions
 ******************************************************************************/

RTIXCdrDllExport void*
XCDR_GenericStreamPlugin_get_buffer(struct DDS_TypePlugin *tp);

RTIXCdrDllExport void
XCDR_GenericStreamPlugin_return_buffer(
        struct DDS_TypePlugin *tp,
        void *buffer);

RTIXCdrDllExport struct DDS_TypePluginSampleHolder*
XCDR_GenericStreamPlugin_get_sample(
        struct DDS_TypePlugin *tp,
        struct CDR_Stream_t *stream);

RTIXCdrDllExport void
XCDR_GenericStreamPlugin_return_sample(
        struct DDS_TypePlugin *tp,
        struct DDS_TypePluginSampleHolder *sample);

MUST_CHECK_RETURN RTIXCdrDllExport RTI_BOOL
XCDR_GenericStreamPlugin_serialize(
        struct DDS_TypePlugin *plugin,
        struct CDR_Stream_t *stream,
        const void *data,
        DDS_InstanceHandle_t *destination);

MUST_CHECK_RETURN RTIXCdrDllExport RTI_BOOL
XCDR_GenericStreamPlugin_deserialize(
        struct DDS_TypePlugin *plugin,
        void *sample,
        struct CDR_Stream_t *stream,
        DDS_InstanceHandle_t *source);

RTIXCdrDllExport RTI_BOOL
XCDR_GenericTypePlugin_deserialize_key(
        struct DDS_TypePlugin *plugin,
        void *sample,
        struct CDR_Stream_t *stream,
        DDS_InstanceHandle_t *source);

RTIXCdrDllExport RTI_BOOL
XCDR_GenericTypePlugin_serialize_key(struct DDS_TypePlugin *plugin,
        struct CDR_Stream_t *stream,
        const void *sample,
        DDS_InstanceHandle_t *destination);

RTIXCdrDllExport RTI_UINT32
XCDR_GenericTypelugin_get_serialized_key_size(
        struct DDS_TypePlugin *plugin,
        RTI_UINT32 current_alignment);

MUST_CHECK_RETURN RTIXCdrDllExport RTI_BOOL
XCDR_GenericTypePlugin_instance_to_keyhash(
        struct NDDS_Type_Plugin *plugin,
        struct CDR_Stream_t *stream,
        DDS_KeyHash_t *keyHash,
        const void *instance,
        DDS_EncapsulationId_t id);

RTIXCdrDllExport RTI_BOOL
DDS_XTypesHeapPlugin_get_reference(
        struct DDS_TypeMemoryPlugin *mp,
        const void *in_address,
        void *reference);

RTIXCdrDllExport RTI_BOOL
DDS_XTypesHeapPlugin_get_sample_state(
        struct DDS_TypePlugin *mp,
        const void *in_address,
        DDS_LoanedSampleState_T *state_out);

RTIXCdrDllExport RTI_BOOL
DDS_XTypesHeapPlugin_set_sample_state(
        struct DDS_TypePlugin *mp,
        const void *in_address,
        DDS_LoanedSampleState_T new_state,
        DDS_Boolean revert_to_previous);

/******************************************************************************
 *                             XTypes Heap Plugin
 ******************************************************************************/

RTIXCdrDllExport RTI_BOOL
DDS_XTypesHeapPlugin_is_owner(struct DDS_TypeMemoryPlugin *mp,const void *sample);

MUST_CHECK_RETURN RTIXCdrDllExport RTI_BOOL
DDS_XTypesHeapPlugin_create_sample(struct DDS_TypePlugin *tp, void **sample);

MUST_CHECK_RETURN RTIXCdrDllExport RTI_BOOL
DDS_XTypesHeapPlugin_delete_sample(struct DDS_TypePlugin *tp, void *sample);

RTIXCdrDllExport struct DDS_TypeMemoryPlugin*
DDS_XTypesHeapPlugin_create(
        struct DDS_TypePlugin *tp,
        DDS_DomainParticipant *participant,
        struct DDS_DomainParticipantQos *dp_qos,
        DDS_TypePluginMode_T endpoint_mode,
        DDS_TypePluginEndpoint *endpoint,
        DDS_TypePluginEndpointQos *qos,
        XCDR_HeapMgr_gen_init initialize_sample_writer_pool,
        XCDR_HeapMgr_gen_init finalize_sample_writer_pool,
        DDS_TypePlugin_initialize_pool_sample_T initialize_sample_reader_pool,
        DDS_TypePlugin_finalize_pool_sample_T finalize_sample_reader_pool,
        DDS_UnsignedLong top_level_type_size,
        RTI_BOOL is_flat_data);

RTIXCdrDllExport void
DDS_XTypesHeapPlugin_delete(
        struct DDS_TypePlugin *p,
        struct DDS_TypeMemoryPlugin *mp);

/******************************************************************************
 *                           Interpreter Functions
 ******************************************************************************/

RTIXCdrDllExport RTI_BOOL
XCDR_Interpreter_serialized_sample_to_key(
        struct DDS_TypePlugin *plugin,
        void *sample,
        struct CDR_Stream_t *stream,
        RTIBool deserializeEncapsulation,
        RTIBool deserializeKey,
        DDS_EncapsulationId_t encapsulationId);

RTIXCdrDllExport RTIXCdrBoolean
XCDR_Interpreter_isEncapsulationCdrV2(DDS_EncapsulationId_t id);

RTIXCdrDllExport DDS_Boolean
XCdrTypeInterface_register(void);

RTIXCdrDllExport DDS_Boolean
XCdrTypeInterface_unregister(void);

RTIXCdrDllExport struct RTIXCdrInterpreterPrograms*
XCdrTypeInterfaceI_create_program(
    const RTIXCdrTypeCode *tc,
    const struct RTIXCdrInterpreterProgramsGenProperty *property,
    RTIXCdrProgramMask mask);

RTIXCdrDllExport void
XCdrTypeInterfaceI_delete_program(
            struct RTIXCdrInterpreterPrograms *programs);

RTIXCdrDllExport struct RTIXCdrTypePluginProgramContext*
XCdrTypeInterfaceI_create_execution_context(const DDS_TypeCode *const tc);

RTIXCdrDllExport void
XCdrTypeInterfaceI_delete_execution_context(
                            struct RTIXCdrTypePluginProgramContext *ctxt);

RTIXCdrDllExport DDS_Boolean
XCdrTypeInterfaceI_set_padding_options(const DDS_TypeCode *const tc,
                                       DDS_EncapsulationId_t eid);
                            
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* xcdr_dds_xcdr_type_plugin_h */
