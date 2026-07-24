/*
 * FILE: DDS_ParticipantMessageData.c
 *
 * (c) Copyright 2018-2024 Real-Time Innovations,
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

#include "DDS_ParticipantMessageData.h"

#ifndef UNUSED_ARG
#define UNUSED_ARG(x) (void)(x)
#endif

/*** SOURCE_BEGIN ***/

/* ========================================================================= */

RTI_BOOL
DDS_GuidPrefix_t_initialize(DDS_GuidPrefix_t* sample)
{
    if (sample == NULL)
    {
        return RTI_FALSE;
    }

    CDR_Primitive_init_array((*sample), ((12)*CDR_OCTET_SIZE));

    return RTI_TRUE;
}

DDS_GuidPrefix_t *
DDS_GuidPrefix_t_create(void)
{
    DDS_GuidPrefix_t* sample;
    OSAPI_Heap_allocate_struct(&sample, DDS_GuidPrefix_t);

    if (sample != NULL)
    {
        if (!DDS_GuidPrefix_t_initialize(sample))
        {
            OSAPI_Heap_free_struct(sample);
            sample = NULL;
        }
    }

    return sample;
}

#ifndef RTI_CERT

RTI_BOOL
DDS_GuidPrefix_t_finalize(DDS_GuidPrefix_t* sample)
{
    if (sample == NULL)
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

void
DDS_GuidPrefix_t_delete(DDS_GuidPrefix_t*sample)
{
    RTI_BOOL bretval = RTI_TRUE;

    if (sample != NULL) {
        /* DDS_GuidPrefix_t_finalize() always
        returns RTI_TRUE when called with sample != NULL */
        bretval = DDS_GuidPrefix_t_finalize(sample);
        IGNORE_RETVAL(bretval);

        OSAPI_Heap_free_struct(sample);
    }
}
#endif

RTI_BOOL
DDS_GuidPrefix_t_copy(DDS_GuidPrefix_t* dst,const DDS_GuidPrefix_t* src)
{
    if ((dst == NULL) || (src == NULL))
    {
        return RTI_FALSE;
    }
    CDR_Primitive_copy_array( (*dst), (*src),((12)*CDR_OCTET_SIZE));
    return RTI_TRUE;
}

/**
* <<IMPLEMENTATION>>
*
* Defines:  TSeq, T
*
* Configure and implement 'DDS_GuidPrefix_t' sequence class.
*/
#define REDA_SEQUENCE_USER_API
#define T DDS_GuidPrefix_t
#define TSeq DDS_GuidPrefix_tSeq
#define T_initialize DDS_GuidPrefix_t_initialize
#define T_finalize   DDS_GuidPrefix_t_finalize
#define T_copy       DDS_GuidPrefix_t_copy
#include "reda/reda_sequence_defn.h"
#undef T_copy
#undef T_finalize
#undef T_initialize

/* ========================================================================= */

RTI_BOOL
DDS_OctetArray4_initialize(DDS_OctetArray4* sample)
{
    if (sample == NULL)
    {
        return RTI_FALSE;
    }

    CDR_Primitive_init_array(
        (*sample), ((4)*CDR_OCTET_SIZE));
    return RTI_TRUE;
}

DDS_OctetArray4 *
DDS_OctetArray4_create(void)
{
    DDS_OctetArray4* sample;
    OSAPI_Heap_allocate_struct(&sample, DDS_OctetArray4);
    if (sample != NULL) {
        if (!DDS_OctetArray4_initialize(sample)) {
            OSAPI_Heap_free_struct(sample);
            sample = NULL;
        }
    }
    return sample;
}

#ifndef RTI_CERT

RTI_BOOL
DDS_OctetArray4_finalize(DDS_OctetArray4* sample)
{
    if (sample == NULL)
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

void
DDS_OctetArray4_delete(DDS_OctetArray4*sample)
{
    RTI_BOOL bretval = RTI_TRUE;

    if (sample != NULL) {
        /* DDS_OctetArray4_finalize() always
        returns RTI_TRUE when called with sample != NULL */
        bretval = DDS_OctetArray4_finalize(sample);
        IGNORE_RETVAL(bretval);

        OSAPI_Heap_free_struct(sample);
    }
}
#endif

RTI_BOOL
DDS_OctetArray4_copy(DDS_OctetArray4* dst,const DDS_OctetArray4* src)
{
    if ((dst == NULL) || (src == NULL))
    {
        return RTI_FALSE;
    }
    CDR_Primitive_copy_array( (*dst), (*src),((4)*CDR_OCTET_SIZE));
    return RTI_TRUE;
}

/**
* <<IMPLEMENTATION>>
*
* Defines:  TSeq, T
*
* Configure and implement 'DDS_OctetArray4' sequence class.
*/
#define REDA_SEQUENCE_USER_API
#define T DDS_OctetArray4
#define TSeq DDS_OctetArray4Seq
#define T_initialize DDS_OctetArray4_initialize
#define T_finalize   DDS_OctetArray4_finalize
#define T_copy       DDS_OctetArray4_copy
#include "reda/reda_sequence_defn.h"
#undef T_copy
#undef T_finalize
#undef T_initialize

/* ========================================================================= */

RTI_BOOL
DDS_Liveliness_ParticipantMessageDataKey_initialize(DDS_Liveliness_ParticipantMessageDataKey* sample)
{
    if (sample == NULL)
    {
        return RTI_FALSE;
    }

    if (!DDS_GuidPrefix_t_initialize(&sample->participant_guid_prefix)) {
        return RTI_FALSE;
    }
    if (!DDS_OctetArray4_initialize(&sample->kind)) {
        return RTI_FALSE;
    }
    return RTI_TRUE;
}

DDS_Liveliness_ParticipantMessageDataKey *
DDS_Liveliness_ParticipantMessageDataKey_create(void)
{
    DDS_Liveliness_ParticipantMessageDataKey* sample;
    OSAPI_Heap_allocate_struct(&sample, DDS_Liveliness_ParticipantMessageDataKey);
    if (sample != NULL) {
        if (!DDS_Liveliness_ParticipantMessageDataKey_initialize(sample)) {
            OSAPI_Heap_free_struct(sample);
            sample = NULL;
        }
    }
    return sample;
}

#ifndef RTI_CERT

RTI_BOOL
DDS_Liveliness_ParticipantMessageDataKey_finalize(DDS_Liveliness_ParticipantMessageDataKey* sample)
{
    RTI_BOOL bretval = RTI_TRUE;
    if (sample == NULL)
    {
        return RTI_FALSE;
    }

    bretval = DDS_GuidPrefix_t_finalize(&sample->participant_guid_prefix);
    IGNORE_RETVAL(bretval);

    bretval = DDS_OctetArray4_finalize(&sample->kind);
    IGNORE_RETVAL(bretval);

    return RTI_TRUE;
}

void
DDS_Liveliness_ParticipantMessageDataKey_delete(DDS_Liveliness_ParticipantMessageDataKey*sample)
{
    RTI_BOOL bretval = RTI_TRUE;

    if (sample != NULL) {
        /* DDS_Liveliness_ParticipantMessageDataKey_finalize() always 
        returns RTI_TRUE when called with sample != NULL */
        bretval = DDS_Liveliness_ParticipantMessageDataKey_finalize(sample);
        IGNORE_RETVAL(bretval);

        OSAPI_Heap_free_struct(sample);
    }
}
#endif

RTI_BOOL
DDS_Liveliness_ParticipantMessageDataKey_copy(DDS_Liveliness_ParticipantMessageDataKey* dst,const DDS_Liveliness_ParticipantMessageDataKey* src)
{        
    if ((dst == NULL) || (src == NULL))
    {
        return RTI_FALSE;
    }
    if (!DDS_GuidPrefix_t_copy(
        &dst->participant_guid_prefix, &src->participant_guid_prefix)) {
        return RTI_FALSE;
    }
    if (!DDS_OctetArray4_copy(
        &dst->kind, &src->kind)) {
        return RTI_FALSE;
    }
    return RTI_TRUE;
}

/**
* <<IMPLEMENTATION>>
*
* Defines:  TSeq, T
*
* Configure and implement 'DDS_Liveliness_ParticipantMessageDataKey' sequence class.
*/
#define REDA_SEQUENCE_USER_API
#define T DDS_Liveliness_ParticipantMessageDataKey
#define TSeq DDS_Liveliness_ParticipantMessageDataKeySeq
#define T_initialize DDS_Liveliness_ParticipantMessageDataKey_initialize
#define T_finalize   DDS_Liveliness_ParticipantMessageDataKey_finalize
#define T_copy       DDS_Liveliness_ParticipantMessageDataKey_copy
#include "reda/reda_sequence_defn.h"
#undef T_copy
#undef T_finalize
#undef T_initialize

/* ========================================================================= */

const char *DDS_Liveliness_ParticipantMessageDataTYPENAME = "DDS::Liveliness::ParticipantMessageData";

RTI_BOOL
DDS_Liveliness_ParticipantMessageData_initialize(DDS_Liveliness_ParticipantMessageData* sample)
{
    if (sample == NULL)
    {
        return RTI_FALSE;
    }

    if (!DDS_Liveliness_ParticipantMessageDataKey_initialize(&sample->key)) {
        return RTI_FALSE;
    }
    if (!CDR_OctetSeq_initialize(&sample->data)) {
        return RTI_FALSE;
    }
    if (!CDR_OctetSeq_set_maximum(&sample->data,(1))) {
        return RTI_FALSE;
    }
    return RTI_TRUE;
}

DDS_Liveliness_ParticipantMessageData *
DDS_Liveliness_ParticipantMessageData_create(void)
{
    DDS_Liveliness_ParticipantMessageData* sample;
    OSAPI_Heap_allocate_struct(&sample, DDS_Liveliness_ParticipantMessageData);
    if (sample != NULL) {
        if (!DDS_Liveliness_ParticipantMessageData_initialize(sample)) {
            OSAPI_Heap_free_struct(sample);
            sample = NULL;
        }
    }
    return sample;
}

#ifndef RTI_CERT

RTI_BOOL
DDS_Liveliness_ParticipantMessageData_finalize(DDS_Liveliness_ParticipantMessageData* sample)
{
    RTI_BOOL bretval = RTI_TRUE;

    if (sample == NULL)
    {
        return RTI_FALSE;
    }

    bretval = DDS_Liveliness_ParticipantMessageDataKey_finalize(&sample->key);
    IGNORE_RETVAL(bretval);

    bretval = CDR_OctetSeq_finalize(&sample->data);
    IGNORE_RETVAL(bretval);

    return RTI_TRUE;
}

void
DDS_Liveliness_ParticipantMessageData_delete(DDS_Liveliness_ParticipantMessageData*sample)
{
    RTI_BOOL bretval = RTI_TRUE;

    if (sample != NULL) {
        /* DDS_Liveliness_ParticipantMessageData_finalize() always 
        returns RTI_TRUE when called with sample != NULL */
        bretval = DDS_Liveliness_ParticipantMessageData_finalize(sample);
        IGNORE_RETVAL(bretval);

        OSAPI_Heap_free_struct(sample);
    }
}
#endif

RTI_BOOL
DDS_Liveliness_ParticipantMessageData_copy(DDS_Liveliness_ParticipantMessageData* dst,const DDS_Liveliness_ParticipantMessageData* src)
{
    if ((dst == NULL) || (src == NULL))
    {
        return RTI_FALSE;
    }

    if (!DDS_Liveliness_ParticipantMessageDataKey_copy(
        &dst->key, &src->key))
    {
        return RTI_FALSE;
    }

    if (!CDR_OctetSeq_copy(&dst->data,&src->data))
    {
        return RTI_FALSE;
    }
    return RTI_TRUE;
}

/**
* <<IMPLEMENTATION>>
*
* Defines:  TSeq, T
*
* Configure and implement 'DDS_Liveliness_ParticipantMessageData' sequence class.
*/
#define REDA_SEQUENCE_USER_API
#define T DDS_Liveliness_ParticipantMessageData
#define TSeq DDS_Liveliness_ParticipantMessageDataSeq
#define T_initialize DDS_Liveliness_ParticipantMessageData_initialize
#define T_finalize   DDS_Liveliness_ParticipantMessageData_finalize
#define T_copy       DDS_Liveliness_ParticipantMessageData_copy
#include "reda/reda_sequence_defn.h"
#undef T_copy
#undef T_finalize
#undef T_initialize

