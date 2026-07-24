/*
 * FILE: Type.c - DDS Type implementation
 *
 * Copyright (c) 2008-2025 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 03oct2016,tk  MICRO-1569 Improved type registration/unregistration API
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 * 09feb2015,tk MICRO-1062/PR#13614 Use DDS_BOOLEAN_FALSE instead of RTI_FALSE
 * 31jul2014,tk MICRO-842/PR#9683   Removed superfluous parameters in DB
 *                                  compare function
 * 15may2014,as MICRO-317 (Verocel PR#1443) Replaced String_cmp with String_ncmp
 * 11jun2013,tk MICRO-633: max type length is 255 excluding NUL
 * 06jul2012,tk Updated
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief DDS Type implementation
 *
 * \details
 * This file implements the API to manage DDS data-types.
 */
/*ci \addtogroup DDSDomainModule
 * @{
 */
#include "Type.h"

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Compare entries in the table of type entries. The function is
 *        compatible with \ref DB_IndexCompare_T
 *
 * \param[in] flags Used to distinguish between op2 being a key or record
 * \param[in] op1   A DDS_TypeImpl already in the database
 * \param[in] op2   Either a DDS_TypeImpl being added or a key
 *                  being searched for
 *
 * \return positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2,
 *         zero if op1 is equal to op2
 */
RTI_INT32
DDS_TypeImpl_compare(RTI_INT32 flags,const DB_Record_T op1, void *op2)
{
    struct DDS_TypeImpl *record_left = (struct DDS_TypeImpl*)op1;
    const char *name_right;

    if (DB_SELECT_OP2_IS_KEY(flags))
    {
        name_right = (const char*)op2;
    }
    else
    {
        name_right = ((struct DDS_TypeImpl*)op2)->name;
    }

    return REDA_String_ncompare(record_left->name,name_right,
                RTPS_PATHNAME_LEN_MAX);
}

/*ci
 * \brief Initialize a type
 *
 * \details
 *
 * The participant allocates memory to store the type data and passes
 * it to the type for initialization. The type allocates all its
 * internal resources. The type is passed a pointer to the type-plugin
 * associated with the type-name. NOTE: A initialized type automatically
 * gets a reference count of 1, thus it is necessary to call \ref
 * DDS_TypeImpl_dereference before it is finalized.
 *
 * \param[in] type       A type structure to initialize
 * \param[in] type_name  The name of the type
 * \param[in] plugin     The type-plugin associated with the type-name

 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DDS_TypeImpl_initalize(DDS_DomainParticipant *participant,
                       struct DDS_TypeImpl *type,
                       const char *type_name,
                       struct DDS_TypePluginI *plugin)
{
    DDS_UnsignedLong type_length;
    UNUSED_ARG(participant);

    type->plugin = plugin;
    type->topic_count = 0;
    type->ref_count = 1;

    type_length = DDS_String_length(type_name);
    if (type_length > RTPS_PATHNAME_LEN_MAX)
    {
        DDSC_LOG_TYPE_TOO_LONG(OSAPI_LOGKIND_ERROR,type_length)
        return DDS_BOOLEAN_FALSE;
    }
    type->name = (char*)type_name;

#if DDS_XTYPES_IS_ENABLED
    type->flat_data_plain_sample_helper = NULL;
    type->programs = NULL;
    type->typecode = NULL;
#endif
    return DDS_BOOLEAN_TRUE;
}

/* The qos parameter has been deprecated and is no longer used */
struct DDS_TypePlugin*
DDS_TypeImpl_create_plugin(struct DDS_TypeImpl *type,
                      DDS_DomainParticipant *participant,
                      struct DDS_DomainParticipantQos *dp_qos,
                      DDS_TypePluginMode_T endpoint_mode,
                      DDS_TypePluginEndpoint *endpoint,
                      DDS_TypePluginEndpointQos *qos_deprecated,
                      struct DDS_TypePluginProperty *property)
{
    struct DDS_TypePlugin *plugin = NULL;
    UNUSED_ARG(qos_deprecated);

    plugin = DDS_TypePlugin_create(type->plugin,participant,dp_qos,
                                   endpoint_mode,endpoint,qos_deprecated,
                                   property);
    if (plugin == NULL)
    {
        return NULL;
    }

#if DDS_XTYPES_IS_ENABLED
    plugin->property.type_code = type->typecode;
    plugin->property.programs = type->programs;
    plugin->property.flat_data_plain_helper = type->flat_data_plain_sample_helper;
    plugin->property.program_context = NULL;
#endif

    if (!DDS_TypePlugin_create_wire_plugin(plugin,
                                           participant,dp_qos,
                                           endpoint_mode,endpoint,
                                           qos_deprecated))
    {
        return NULL;
    }

    return plugin;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize a Type
 *
 * \details
 * Free up all resources used by a type. Note that this function does
 * not free the memory used to hold the type itself, this memory is
 * freed by the participant because the participant is the factory.
 *
 * \param[in] type Type to finalize
 *
 * \return This function returns RTI_TRUE if the type is successfully finalized
 *         and RTI_FALSE if the type cannot be finalized.
 */
DDS_Boolean
DDS_TypeImpl_finalize(struct DDS_TypeImpl *type)
{

    if (type->ref_count != 0)
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (type->topic_count != 0)
    {
        return DDS_BOOLEAN_FALSE;
    }

    type->name = NULL;
    type->plugin = NULL;
    type->topic_count = 0;
    type->ref_count = 0;

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Finalize a Type record
 *
 * \param[in] type_record Type record to finalize
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
DDS_Type_dtor(DB_Record_T type_record)
{
    return DDS_TypeImpl_finalize((struct DDS_TypeImpl*)type_record) ?
                                RTI_TRUE : RTI_FALSE;
}

#endif /* !RTI_CERT */

/*ci
 * \brief Increment topics attached to a type
 *
 * \details
 *
 * A type can only be registered once, but different topics can be created
 * of the same type. A topic counter is incremented for each topic attached
 * to a type, and a type can only be deleted if the topic count is 0.
 *
 * \param[in] type Type to attach topic to
 *
 * \sa \ref DDS_TypeImpl_detach_topic, \ref DDS_TypeImpl_is_attached
 */
void
DDS_TypeImpl_attach_topic(struct DDS_TypeImpl *type)
{
    ++type->topic_count;
}

/*ci
 * \brief Decrement topics attached to a type
 *
 * \param[in] type Type to detach topic from
 *
 * \sa \ref DDS_TypeImpl_attach_topic, \ref DDS_TypeImpl_is_attached
 */
void
DDS_TypeImpl_detach_topic(struct DDS_TypeImpl *type)
{
    --type->topic_count;
}

/*ci
 * \brief Check is any topics are attache to the type
 *
 * \param[in] type Type to check if any topics are attached to
 *
 * \return
 * \sa \ref DDS_TypeImpl_attach_topic, \ref DDS_TypeImpl_detach_topic
 */
DDS_Boolean
DDS_TypeImpl_is_attached(struct DDS_TypeImpl *type)
{
    return (type->topic_count > 0) ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/****/

/*ci
 * \brief Increment the number of references to a type
 *
 * \details
 *
 * The same type can registered multiple times, but must also be
 * unregistered an equal number of times to release the resources
 * associated with it. This function add a reference to the type
 *
 * \param[in] type Type to increment reference count for
 *
 * \sa \ref DDS_TypeImpl_dereference, \ref DDS_TypeImpl_is_referenced,
 *     DDS_TypeImpl_reset_reference
 */
void
DDS_TypeImpl_reference(struct DDS_TypeImpl *type)
{
    ++type->ref_count;
}

/*ci
 * \brief Decrement the number of references to a type
 *
 * \param[in] type Type to decrement reference count for
 *
 * \sa \ref DDS_TypeImpl_reference, \ref DDS_TypeImpl_is_referenced,
 *     \ref DDS_TypeImpl_reset_reference
 */
void
DDS_TypeImpl_dereference(struct DDS_TypeImpl *type)
{
    --type->ref_count;
}

/*ci
 * \brief Check if there are any references to a type
 *
 * \param[in] type Type to check for references
 *
 * \return
 * \sa \ref DDS_TypeImpl_dereference, \ref DDS_TypeImpl_reference,
 *     \ref DDS_TypeImpl_reset_reference
 */
DDS_Boolean
DDS_TypeImpl_is_referenced(struct DDS_TypeImpl *type)
{
    return (type->ref_count > 0) ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/*ci
 * \brief Reset the reference count
 *
 * \details
 *
 * This function assumes that a type has a reference count of 1 the first
 * time it is registered/initialized successfully.
 *
 * \param[in] type Type to reset the reference count for
 *
 * \return
 * \sa \ref DDS_TypeImpl_is_referenced, \ref DDS_TypeImpl_dereference,
 *     \ref DDS_TypeImpl_reference
 */
void
DDS_TypeImpl_reset_reference(struct DDS_TypeImpl *type)
{
    type->ref_count = 1;
    type->topic_count = 0;
}

/*****/

/*ci
 * \brief Return pointer to the type-plugin for a type
 *
 * \param[in] self Type to return the plugin for
 *
 * \return Pointer to type-plugin on success, NULL on failure
 */
struct DDS_TypePluginI*
DDS_TypeImpl_get_plugin(DDS_Type *self)
{
    struct DDS_TypeImpl *type = (struct DDS_TypeImpl *)self;

    OSAPI_PRECONDITION(type == NULL,
                           return NULL,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return type->plugin;
}

/*ci
 * \brief Return pointer to the type-name for a type
 *
 * \param[in] self Type to return type-name for
 *
 * \return Pointer to the type-name for a type on success, NULL on failure
 */
const char*
DDS_TypeImpl_get_type_name_reference(DDS_Type *self)
{
    struct DDS_TypeImpl *type = (struct DDS_TypeImpl *)self;

    OSAPI_PRECONDITION(type == NULL,
                            return NULL,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return type->name;
}

DDSCDllExport DDS_Boolean
DDS_Type_representation_is_supported(struct DDS_TypePluginI *tp,
                                     DDS_DataRepresentationId_t r)
{
    struct DDS_TypeEncapsulationI *wire_intf;
    RTI_UINT32 count = 0;

    while (tp->wire_intf[count] != NULL)
    {
        wire_intf = tp->wire_intf[count];

        if (r == wire_intf->representation_id)
        {
            return DDS_BOOLEAN_TRUE;
        }

        ++count;
    }

    return DDS_BOOLEAN_FALSE;
}

DDS_Boolean
DDS_Type_valid_representations(struct DDS_TypePluginI *tp,
                               struct DDS_DataRepresentationIdSeq *seq)
{
    RTI_INT32 seq_length;
    RTI_INT32 seq_index;

    seq_length = DDS_DataRepresentationIdSeq_get_length(seq);
    for (seq_index = 0; seq_index < seq_length; ++seq_index)
    {
        if (!DDS_Type_representation_is_supported(tp,
              *DDS_DataRepresentationIdSeq_get_reference(seq,seq_index)))
        {
            return DDS_BOOLEAN_FALSE;
        }
    }

    return DDS_BOOLEAN_TRUE;
}

#if DDS_XTYPES_IS_ENABLED
/*ci
 * \brief Get the programs needed for the interpreted.
 *
 * \details
 * These programs are the interpreted programs used for the XTypes feature
 *
 * \param[in] type Type
 *
 * \return interpreter programs
 */
struct RTIXCdrInterpreterPrograms*
DDS_TypeImpl_get_programs(struct DDS_TypeImpl *type)
{
    return type->programs;
}

/*ci
 * \brief Get the plain helper sample needed to serialize keys
 *
 * \param[in] type Type
 *
 * \return
 */
void*
DDS_TypeImpl_get_sample(struct DDS_TypeImpl *type)
{
    return type->flat_data_plain_sample_helper;
}

/*ci
 * \brief Sets the programs of the type
 *
 * \param[in] type Type
 */
void
DDS_TypeImpl_set_programs(struct DDS_TypeImpl *type,
                          struct RTIXCdrInterpreterPrograms *programs)
{
    type->programs = programs;
}

/*ci
 * \brief Sets the a plain sample associated with the type
 *
 * \param[in] type Type
 */
void
DDS_TypeImpl_set_sample(struct DDS_TypeImpl *type, void *plain_sample)
{
    type->flat_data_plain_sample_helper = plain_sample;
}

/*ci
 * \brief Sets the typecode associated with the type
 *
 * \param[in] type Type
 */
void
DDS_TypeImpl_set_typecode(struct DDS_TypeImpl *type, DDS_TypeCode *typecode)
{
    type->typecode = typecode;
}

/*ci
 * \brief Gets the typecode associated with the type
 *
 * \param[in] type Type
 *
 * \return The typecode for the type, may be NULL
 */
DDS_TypeCode*
DDS_TypeImpl_get_typecode(struct DDS_TypeImpl *type)
{
    return type->typecode;
}
#endif

/*ci @} */

