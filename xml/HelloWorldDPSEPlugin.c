/*
WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

This file was generated from HelloWorldDPSE.idl using "rtiddsgen".
The rtiddsgen tool is part of the RTI Data Distribution Service distribution.
For more information, type 'rtiddsgen -help' at a command shell
or consult the RTI Data Distribution Service manual.
*/

#include "HelloWorldDPSE.h"
#include "reda/reda_bufferpool.h"
#if DDS_XTYPES_IS_ENABLED
#include "xcdr/xcdr_interpreter.h"
#include "xcdr/xcdr_dds_xcdr_type_plugin.h"
#include "xcdr/xcdr_dds_interpreter.h"
#endif
#include "HelloWorldDPSEPlugin.h"

/*** SOURCE_BEGIN ***/
#ifndef UNUSED_ARG
#define UNUSED_ARG(x) (void)(x)
#endif

/* --------------------------------------------------------------------------
(De)Serialize functions:
* -------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------
Key Management functions:
* -------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------
*  Sample Support functions:
* -------------------------------------------------------------------------- */
RTI_BOOL
HelloWorldPlugin_create_sample(
    struct DDS_TypePlugin *plugin,
    void **sample)
{
    UNUSED_ARG(plugin);

    *sample = (void *) HelloWorld_create();
    return (*sample != NULL);
}

#ifndef RTI_CERT
RTI_BOOL
HelloWorldPlugin_delete_sample(
    struct DDS_TypePlugin *plugin,
    void *sample)
{
    UNUSED_ARG(plugin);

    #ifndef RTI_CERT
    /* HelloWorld_delete() is a void function
    * which expects (sample != NULL). Since
    * HelloWorldPlugin_delete_sample
    * is an internal function, sample is assumed to be a valid pointer
    */
    HelloWorld_delete((HelloWorld *) sample);
    #endif

    return RTI_TRUE;
}
#endif

RTI_BOOL
HelloWorldPlugin_copy_sample(
    struct DDS_TypePlugin *plugin,
    void *dst,
    const void *src)
{
    UNUSED_ARG(plugin);

    return HelloWorld_copy(
        (HelloWorld*)dst,
        (const HelloWorld*)src);
}
/* --------------------------------------------------------------------------
*  Type HelloWorld Plugin Instantiation
* -------------------------------------------------------------------------- */

NDDSCDREncapsulation HelloWorldEncapsulationKind[] =
{
    {
        DDS_ENCAPSULATION_ID_CDR_LE,
        DDS_ENCAPSULATION_ID_CDR_BE,
        0
    }
};

NDDSCDREncapsulation HelloWorldV2EncapsulationKind[] =
{
    {

        DDS_ENCAPSULATION_ID_XCDR2_A_LE,
        DDS_ENCAPSULATION_ID_XCDR2_A_BE,
        0

    }
};

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
HelloWorldTypePlugin_initialize_sample(struct DDS_TypePlugin *plugin, void *buffer)
{
    UNUSED_ARG(plugin);
    return HelloWorld_initialize((HelloWorld*)buffer);
}

RTI_PRIVATE RTI_UINT32
HelloWorld_get_user_sample_size(
    struct DDS_TypePlugin *tp)
{
    UNUSED_ARG(tp);
    return sizeof(struct HelloWorld);
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
HelloWorld_cdr_initialize(void *init_config, void *buffer)
{
    struct DDS_TypePluginDefault *plugin = (struct DDS_TypePluginDefault*)init_config;
    void *sample;
    struct DDS_TypePluginSampleHolder *sh = (struct DDS_TypePluginSampleHolder*)buffer;

    if (!HelloWorldPlugin_create_sample(&plugin->_parent,&sample))
    {
        return RTI_FALSE;
    }

    sh->sample = sample;

    return RTI_TRUE;
}

#ifndef RTI_CERT
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
HelloWorld_cdr_finalize(void *finalize_config, void *buffer)
{
    struct DDS_TypePluginDefault *plugin = (struct DDS_TypePluginDefault*)finalize_config;
    struct DDS_TypePluginSampleHolder *sh = (struct DDS_TypePluginSampleHolder*)buffer;

    if (!HelloWorldPlugin_delete_sample(&plugin->_parent,sh->sample))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}
#endif

RTI_PRIVATE void
HelloWorldTypePlugin_return_sample(
    struct DDS_TypePlugin *tp,
    struct DDS_TypePluginSampleHolder *sample)
{
    HelloWorld_finalize_optional_members((HelloWorld*)sample->sample, RTI_FALSE);
    XCDR_GenericStreamPlugin_return_sample(tp, sample);
}

RTI_PRIVATE struct DDS_TypeMemoryPlugin*
HelloWorldXTypesHeapPlugin_create(
    struct DDS_TypePlugin *tp,
    DDS_DomainParticipant *participant,
    struct DDS_DomainParticipantQos *dp_qos,
    DDS_TypePluginMode_T endpoint_mode,
    DDS_TypePluginEndpoint *endpoint,
    DDS_TypePluginEndpointQos *qos)
{
    return DDS_XTypesHeapPlugin_create(
        tp,
        participant,
        dp_qos,
        endpoint_mode,
        endpoint,
        qos,
        NULL,
        NULL,
        HelloWorld_cdr_initialize,
        #ifndef RTI_CERT
        HelloWorld_cdr_finalize,
        #else
        NULL,
        #endif
        sizeof(HelloWorld),
        RTI_FALSE
        );
}

RTI_PRIVATE struct DDS_TypeMemoryI HelloWorld_fv_XTypesHeapPluginI =
{
    RTI_MEMORY_MANAGER_HEAP,
    RTI_MEMORY_TYPE_HEAP,
    HelloWorldPlugin_create_sample,
    #ifndef RTI_CERT
    HelloWorldPlugin_delete_sample,
    #else
    NULL,
    #endif
    NULL, /* get_address */
    NULL, /* return_address */
    NULL,
    DDS_XTypesHeapPlugin_get_sample_state,
    DDS_XTypesHeapPlugin_set_sample_state,
    DDS_XTypesHeapPlugin_is_owner,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    HelloWorldXTypesHeapPlugin_create,
    DDS_XTypesHeapPlugin_delete
};

RTI_PRIVATE DDS_Boolean
HelloWorld_on_type_registered(struct DDS_TypeImpl* type_impl)
{
    struct RTIXCdrInterpreterPrograms *programs = NULL;
    struct RTIXCdrInterpreterProgramsGenProperty programProperty =
    RTIXCdrInterpreterProgramsGenProperty_INITIALIZER;

    programProperty.resolveAlias = RTI_XCDR_TRUE;
    programProperty.inlineStruct = RTI_XCDR_TRUE;
    programProperty.optimizeEnum = RTI_XCDR_TRUE;

    programs = RTIXCdrInterpreterPrograms_new(
        (RTIXCdrTypeCode *)HelloWorld_get_typecode(),
        &programProperty,
        RTI_XCDR_PROGRAM_MASK_TYPEPLUGIN);

    if (programs == NULL)
    {
        return DDS_BOOLEAN_FALSE;
    }

    DDS_TypeImpl_set_programs(type_impl, programs);
    DDS_TypeImpl_set_typecode(type_impl,  HelloWorld_get_typecode());

    return DDS_BOOLEAN_TRUE;
}
RTI_PRIVATE DDS_Boolean
HelloWorld_on_type_unregistered(struct DDS_TypeImpl* type_impl)
{
    struct RTIXCdrInterpreterPrograms *programs = NULL;
    programs = DDS_TypeImpl_get_programs(type_impl);

    if (programs != NULL)
    {
        RTIXCdrInterpreterPrograms_delete(programs);
    }
    DDS_TypeImpl_set_programs(type_impl, NULL);

    return DDS_BOOLEAN_TRUE;
}

RTI_PRIVATE
struct DDS_TypeEncapsulationI HelloWorld_fv_XCDRv1PluginI =
{
    DDS_XCDR_DATA_REPRESENTATION,
    NULL,
    HelloWorldEncapsulationKind,
    RTI_MEMORY_TYPE_HEAP,
    RTI_MEMORY_MANAGER_HEAP,
    NULL,
    NULL,
    XCDR_GenericStreamPlugin_get_buffer,
    XCDR_GenericStreamPlugin_return_buffer,
    XCDR_GenericStreamPlugin_get_sample,
    HelloWorldTypePlugin_return_sample,
    XCDR_GenericStreamPlugin_serialize,
    XCDR_GenericStreamPlugin_deserialize,
    XCDRv1_StreamPlugin_get_serialized_sample_size,
    XCDRv1_StreamPlugin_create,
    XCDRv1_StreamPlugin_delete
};

RTI_PRIVATE struct DDS_TypeEncapsulationI HelloWorld_fv_XCDRv2PluginI =
{
    DDS_XCDR2_DATA_REPRESENTATION,
    NULL,
    HelloWorldV2EncapsulationKind,
    RTI_MEMORY_TYPE_HEAP,
    RTI_MEMORY_MANAGER_HEAP,
    NULL,
    NULL,
    XCDR_GenericStreamPlugin_get_buffer,
    XCDR_GenericStreamPlugin_return_buffer,
    XCDR_GenericStreamPlugin_get_sample,

    HelloWorldTypePlugin_return_sample,
    XCDR_GenericStreamPlugin_serialize,
    XCDR_GenericStreamPlugin_deserialize,
    XCDRv2_StreamPlugin_get_serialized_sample_size,
    XCDRv2_StreamPlugin_create,
    XCDRv2_StreamPlugin_delete
};

RTI_PRIVATE struct DDS_TypeEncapsulationI *HelloWorld_fv_WirePlugins[] =
{
    &HelloWorld_fv_XCDRv2PluginI,
    &HelloWorld_fv_XCDRv1PluginI,
    NULL
};

RTI_PRIVATE struct DDS_TypeMemoryI *HelloWorld_fv_MemoryPlugins[] =
{
    &HelloWorld_fv_XTypesHeapPluginI,
    NULL
};

RTI_PRIVATE const struct DDS_TypeInterfaceI HelloWorld_fv_XCdrIntf =
{
    XCdrTypeInterfaceI_create_program,
    XCdrTypeInterfaceI_delete_program,
    XCdrTypeInterfaceI_create_execution_context,
    XCdrTypeInterfaceI_delete_execution_context,
    XCdrTypeInterfaceI_set_padding_options
};

RTI_PRIVATE struct DDS_TypePlugin*
HelloWorldTypePlugin_create_plugin(
    DDS_DomainParticipant *participant,
    struct DDS_DomainParticipantQos *dp_qos,
    DDS_TypePluginMode_T endpoint_mode,
    DDS_TypePluginEndpoint *endpoint,
    DDS_TypePluginEndpointQos *qos,
    struct DDS_TypePluginProperty *const property);

RTI_PRIVATE RTI_BOOL
HelloWorldTypePlugin_delete_plugin(struct DDS_TypePlugin *plugin);

RTI_PRIVATE struct DDS_TypePluginI HelloWorld_fv_TypePluginI =
{
    /**************************************************************************
    *                   Type information functions
    **************************************************************************/

    NULL,                       /* DDS_TypeCode_t* */
    NDDS_TYPEPLUGIN_USER_KEY,   /* NDDS_TypePluginKeyKind */

    NDDS_TYPEPLUGIN_EH_LOCATION_PAYLOAD,
    HelloWorld_get_user_sample_size,
    RTI_MEMORY_TYPE_HEAP,
    XCDR_GenericTypePlugin_instance_to_keyhash,
    HelloWorldPlugin_copy_sample,

    HelloWorldTypePlugin_initialize_sample,

    XCDR_GenericTypePlugin_serialize_key,
    XCDR_GenericTypePlugin_deserialize_key,
    XCDR_GenericTypelugin_get_serialized_key_size,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    HelloWorld_fv_MemoryPlugins,
    HelloWorld_fv_WirePlugins,

    /**************************************************************************
    *       Helper APIs to create language binding wrapper Functions
    **************************************************************************/

    NULL, NULL, NULL, NULL,  /* endpoint wrappers not used in C */
    HelloWorldTypePlugin_create_plugin,
    HelloWorldTypePlugin_delete_plugin,
    HelloWorld_on_type_registered,
    HelloWorld_on_type_unregistered,
    &HelloWorld_fv_XCdrIntf
};

/* --------------------------------------------------------------------------
*  Type HelloWorld Plugin Methods
* -------------------------------------------------------------------------- */

struct DDS_TypePluginI*
HelloWorldTypePlugin_get(void)
{
    return &HelloWorld_fv_TypePluginI;
}

RTI_PRIVATE struct DDS_TypePlugin*
HelloWorldTypePlugin_create_plugin(
    DDS_DomainParticipant *participant,
    struct DDS_DomainParticipantQos *dp_qos,
    DDS_TypePluginMode_T endpoint_mode,
    DDS_TypePluginEndpoint *endpoint,
    DDS_TypePluginEndpointQos *qos,
    struct DDS_TypePluginProperty *const property)
{
    return DDS_TypePluginDefault_create(&HelloWorld_fv_TypePluginI,
    participant,dp_qos,
    endpoint_mode,endpoint,qos,
    property);
}

RTI_PRIVATE RTI_BOOL
HelloWorldTypePlugin_delete_plugin(struct DDS_TypePlugin *plugin)
{
    return DDS_TypePluginDefault_delete(plugin);
}

struct DDS_TypePlugin*
HelloWorldWriterTypePlugin_create(
    DDS_DomainParticipant *participant,
    struct DDS_DomainParticipantQos *dp_qos,
    DDS_DataWriter *writer,
    struct DDS_DataWriterQos *qos,
    struct DDS_TypePluginProperty *property)
{
    return DDS_TypePlugin_create_w_intf(
        &HelloWorld_fv_TypePluginI,
        participant,dp_qos,
        DDS_TYPEPLUGIN_MODE_WRITER,
        (DDS_TypePluginEndpoint*)writer,
        (DDS_TypePluginEndpointQos*)qos,
        property);
}

struct DDS_TypePlugin*
HelloWorldReaderTypePlugin_create(
    DDS_DomainParticipant *participant,
    struct DDS_DomainParticipantQos *dp_qos,
    DDS_DataReader *reader,
    struct DDS_DataReaderQos *qos,
    struct DDS_TypePluginProperty *property)
{
    return DDS_TypePlugin_create_w_intf(
        &HelloWorld_fv_TypePluginI,
        participant,dp_qos,
        DDS_TYPEPLUGIN_MODE_READER,
        (DDS_TypePluginEndpoint*)reader,
        (DDS_TypePluginEndpointQos*)qos,
        property);
}

const char*
HelloWorldTypePlugin_get_default_type_name(void)
{
    return HelloWorldTYPENAME;
}

NDDS_TypePluginKeyKind
HelloWorldI_get_key_kind(void)
{
    return HelloWorld_fv_TypePluginI.key_kind;
}

