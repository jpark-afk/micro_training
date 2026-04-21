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
void
HelloWorldTypeSupport_delete_data(
    HelloWorld *data)
{
    HelloWorld_delete(data);
}
#endif

#undef TTYPENAME

