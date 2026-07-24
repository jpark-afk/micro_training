/*
 * FILE: DDS_ParticipantMessageDataSupport.c
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

#include "DDS_ParticipantMessageDataSupport.h"

/*** SOURCE_BEGIN ***/

/* =========================================================================== */

/* Requires */
#define TTYPENAME   DDS_Liveliness_ParticipantMessageDataTYPENAME

/* 
DDS_Liveliness_ParticipantMessageDataDataWriter (DDS_DataWriter)   
*/

/* Defines */
#define TDataWriter DDS_Liveliness_ParticipantMessageDataDataWriter
#define TData       DDS_Liveliness_ParticipantMessageData

#include "dds_c/dds_c_tdatawriter_gen.h"

#undef TDataWriter
#undef TData

/* =========================================================================== */
/* 
DDS_Liveliness_ParticipantMessageDataDataReader (DDS_DataReader)   
*/

/* Defines */
#define TDataReader DDS_Liveliness_ParticipantMessageDataDataReader
#define TDataSeq    DDS_Liveliness_ParticipantMessageDataSeq
#define TData       DDS_Liveliness_ParticipantMessageData
#include "dds_c/dds_c_tdatareader_gen.h"
#undef TDataReader
#undef TDataSeq
#undef TData

DDS_ReturnCode_t
DDS_Liveliness_ParticipantMessageDataTypeSupport_register_type(
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
        type_name = DDS_Liveliness_ParticipantMessageDataTypePlugin_get_default_type_name();
        if (type_name == NULL)
        {
            goto done;
        }
    }

    retcode = DDS_DomainParticipant_register_type(
        participant,
        type_name,
        DDS_Liveliness_ParticipantMessageDataTypePlugin_get());

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
DDS_Liveliness_ParticipantMessageDataTypeSupport_unregister_type(
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
        type_name = DDS_Liveliness_ParticipantMessageDataTypePlugin_get_default_type_name();
        if (type_name == NULL)
        {
            goto done;
        }
    }

    if (DDS_Liveliness_ParticipantMessageDataTypePlugin_get() !=
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
DDS_Liveliness_ParticipantMessageDataTypeSupport_get_type_name(void)
{
    return DDS_Liveliness_ParticipantMessageDataTYPENAME;
}
DDS_Liveliness_ParticipantMessageData *
DDS_Liveliness_ParticipantMessageDataTypeSupport_create_data(void)
{
    DDS_Liveliness_ParticipantMessageData *data = NULL;

    data = DDS_Liveliness_ParticipantMessageData_create();

    return data;
}

#ifndef RTI_CERT
void
DDS_Liveliness_ParticipantMessageDataTypeSupport_delete_data(
    DDS_Liveliness_ParticipantMessageData *data)
{
    DDS_Liveliness_ParticipantMessageData_delete(data);
}
#endif

#undef TTYPENAME

