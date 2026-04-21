/*
WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

This file was generated from HelloWorld.idl using "rtiddsgen".
The rtiddsgen tool is part of the RTI Data Distribution Service distribution.
For more information, type 'rtiddsgen -help' at a command shell
or consult the RTI Data Distribution Service manual.
*/

#include "HelloWorldSupport.h"

/*** SOURCE_BEGIN ***/
/* =========================================================================== */

/* Requires */
#define TTYPENAME   HelloWorldTYPENAME

/* 
HelloWorldDataWriter (DDS_DataWriter)   
*/

/* Defines */
#define TDataWriter HelloWorldDataWriter
#define TData       HelloWorld

#include "dds_c/dds_c_tdatawriter_gen.h"

#undef TDataWriter
#undef TData

/* =========================================================================== */
/* 
HelloWorldDataReader (DDS_DataReader)   
*/

/* Defines */
#define TDataReader HelloWorldDataReader
#define TDataSeq    HelloWorldSeq
#define TData       HelloWorld
#include "dds_c/dds_c_tdatareader_gen.h"
#undef TDataReader
#undef TDataSeq
#undef TData

DDS_ReturnCode_t
HelloWorldTypeSupport_register_type(
    DDS_DomainParticipant* participant,
    const char* type_name)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    if (participant == NULL) 
    {
        goto done;
    }

    if (type_name == NULL) 
    {
        type_name = HelloWorldTypePlugin_get_default_type_name();
        if (type_name == NULL)
        {
            goto done;
        }
    }

    retcode = DDS_DomainParticipant_register_type(
        participant,
        type_name,
        HelloWorldTypePlugin_get());

    if (retcode != DDS_RETCODE_OK)
    {
        goto done;
    }

    retcode = DDS_RETCODE_OK;

    done:

    return retcode;
}

#ifndef RTI_CERT
DDS_ReturnCode_t
HelloWorldTypeSupport_unregister_type(
    DDS_DomainParticipant* participant,
    const char* type_name)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    if (participant == NULL) 
    {
        goto done;
    }

    if (type_name == NULL) 
    {
        type_name = HelloWorldTypePlugin_get_default_type_name();
        if (type_name == NULL)
        {
            goto done;
        }
    }

    if (HelloWorldTypePlugin_get() !=
    DDS_DomainParticipant_unregister_type(participant,type_name))
    {
        goto done;
    }

    retcode = DDS_RETCODE_OK;

    done:

    return retcode;
}
#endif
const char*
HelloWorldTypeSupport_get_type_name(void)
{
    return HelloWorldTYPENAME;
}
HelloWorld *
HelloWorldTypeSupport_create_data(void)
{
    HelloWorld *data = NULL;

    data = HelloWorld_create();

    return data;
}

#ifndef RTI_CERT
DDS_ReturnCode_t
HelloWorldTypeSupport_delete_data(
    HelloWorld *data)
{
    HelloWorld_delete(data);
    return DDS_RETCODE_OK; 
}
#endif

static struct DDS_TypeProgramNode HelloWorld_gv_ProgramNode = DDS_TypeProgramNode_INITIALIZER;

static DDS_DataRepresentationId_t HelloWorld_gv_AutoRepresentation = DDS_XCDR_DATA_REPRESENTATION;
static DDS_DataRepresentationId_t HelloWorld_gv_XCDR1 = DDS_XCDR_DATA_REPRESENTATION;
static DDS_DataRepresentationId_t HelloWorld_gv_XCDR2 = DDS_XCDR2_DATA_REPRESENTATION;

DDS_ReturnCode_t 
HelloWorldTypeSupport_serialize_data_to_cdr_buffer_ex(
    char *buffer,                                
    unsigned int *length,                        
    const HelloWorld *a_data,                         
    DDS_DataRepresentationId_t  representation)
{
    struct DDS_TypePlugin tp;
    DDS_EncapsulationId_t encapsulation;
    const struct RTIXCdrInterpreterPrograms *programs = NULL;

    DDS_TypePlugin_initialize(&tp);

    HelloWorld_gv_ProgramNode.type_intf = HelloWorldTypePlugin_get();

    programs = DDS_DomainParticipantFactory_assert_program(
        DDS_TheParticipantFactory,
        HelloWorldTypePlugin_get(),
        &HelloWorld_gv_ProgramNode,
        HelloWorld_get_typecode());

    if (programs == NULL)
    {
        return DDS_RETCODE_ERROR;
    }

    tp.property.program_context = HelloWorld_gv_ProgramNode.context;

    DDS_TypePlugin_initialize_static(
        &tp,
        HelloWorldTypePlugin_get(),
        programs);

    if (DDS_RETCODE_OK != DDS_TypeSupport_resolve_representation(
        &representation,
        HelloWorld_gv_AutoRepresentation,
        HelloWorld_gv_XCDR1,
        HelloWorld_gv_XCDR2,
        &encapsulation))
    {
        return DDS_RETCODE_ERROR;
    }

    return XCDR_Interpreter_serialized_sample_to_buffer(
        &tp,
        buffer,
        length,
        (const void *)a_data,
        representation,
        encapsulation);
}

DDS_ReturnCode_t 
HelloWorldTypeSupport_serialize_data_to_cdr_buffer(
    char *buffer,
    unsigned int *length,
    const HelloWorld *a_data)
{
    return HelloWorldTypeSupport_serialize_data_to_cdr_buffer_ex(
        buffer,
        length,
        a_data,
        DDS_AUTO_DATA_REPRESENTATION);

}

DDS_ReturnCode_t 
HelloWorldTypeSupport_deserialize_data_from_cdr_buffer(
    HelloWorld *a_data,
    const char *buffer,
    unsigned int length)
{
    struct CDR_Stream_t stream;
    struct DDS_TypePlugin tp;
    const struct RTIXCdrInterpreterPrograms *programs = NULL;

    DDS_TypePlugin_initialize(&tp);

    if (!CDR_Stream_initialize_w_buffer(&stream,buffer,length))
    {
        return DDS_RETCODE_ERROR;
    }

    programs = DDS_DomainParticipantFactory_assert_program(
        DDS_TheParticipantFactory,
        HelloWorldTypePlugin_get(),
        &HelloWorld_gv_ProgramNode,
        HelloWorld_get_typecode());

    if (programs == NULL)
    {
        return DDS_RETCODE_ERROR;
    }

    tp.property.program_context = HelloWorld_gv_ProgramNode.context;

    DDS_TypePlugin_initialize_static(
        &tp,
        HelloWorldTypePlugin_get(),
        programs);

    if (!XCDR_Interpreter_deserialize(&tp,a_data,&stream,RTI_TRUE,RTI_TRUE,0))
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}

#undef TTYPENAME

