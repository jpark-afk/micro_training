/*
 * FILE: Type.c - DDS Type implementation
 *
 * (c) Copyright 2008-2015 Real-Time Innovations,
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
 * 15apr2021,tk MICRO-2979/PR.27549
 * - Removed unused functions from CERT:
 *   NDDS_TypePlugin_is_valid
 * 10jan2021,tk MICRO-2807 / PR.27549  Removed unused functions for CERT
 * 03oct2016,tk MICRO-1569 Improved type registration/unregistration API
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
DDS_TypeImpl_initalize(struct DDS_TypeImpl *type,
                        const char *type_name,
                        struct NDDS_Type_Plugin *plugin)
{
    DDS_UnsignedLong type_length;

    type->plugin = plugin;
    type->topic_count = 0;
    type->ref_count = 1;

    type_length = DDS_String_length(type_name);
    if (type_length > RTPS_PATHNAME_LEN_MAX)
    {
        DDSC_LOG_TYPE_TOO_LONG(OSAPI_LOGKIND_ERROR,type_length)
        return DDS_BOOLEAN_FALSE;
    }

#ifndef RTI_CERT
    type->name = (char*)type_name;
#else
    type->name = REDA_String_dup(type_name);
#endif
    if (type->name == NULL)
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
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

#ifndef RTI_CERT
    type->name = NULL;
#else
    if (type->name != NULL)
    {
        REDA_String_free(type->name);
        type->name = NULL;
    }
#endif

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

#ifndef RTI_CERT
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
#endif

#ifndef RTI_CERT
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
#endif

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

#ifndef RTI_CERT
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
#endif

#ifndef RTI_CERT
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
#endif

#ifndef RTI_CERT
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
#endif

/*****/

/*ci
 * \brief Return pointer to the type-plugin for a type
 *
 * \param[in] self Type to return the plugin for
 *
 * \return Pointer to type-plugin on success, NULL on failure
 */
struct NDDS_Type_Plugin*
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

#ifndef RTI_CERT
/*ci
 * \brief Validate a type-plugin structure
 *
 * \param[in] type_plugin Type-plugin to validate
 *
 * \return DDS_BOOLEAN_TRUE if the plugin is valid, DDS_BOOLEAN_FALSE if not
 */
DDS_Boolean
NDDS_TypePlugin_is_valid(struct NDDS_Type_Plugin *type_plugin)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;

    if (type_plugin->get_serialized_sample_max_size == NULL)
    {
        DDSC_LOG_type_function_null(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_TYPE_FUNCTION_GET_SERIALIZED_SAMPLE_MAX_SIZE);
        goto done;
    }

    if (type_plugin->serialize_data == NULL)
    {
        DDSC_LOG_type_function_null(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_TYPE_FUNCTION_SERIALIZE_DATA);
        goto done;
    }

    if (type_plugin->deserialize_data == NULL)
    {
        DDSC_LOG_type_function_null(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_TYPE_FUNCTION_DESERIALIZE_DATA);
        goto done;
    }

    if (type_plugin->create_sample == NULL)
    {
        DDSC_LOG_type_function_null(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_TYPE_FUNCTION_CREATE_SAMPLE);
        goto done;
    }

    if (type_plugin->copy_sample == NULL)
    {
        DDSC_LOG_type_function_null(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_TYPE_FUNCTION_COPY_SAMPLE);
        goto done;
    }

    if (type_plugin->delete_sample == NULL)
    {
        DDSC_LOG_type_function_null(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_TYPE_FUNCTION_DELETE_SAMPLE);
        goto done;
    }

    if (type_plugin->get_key_kind == NULL)
    {
        DDSC_LOG_type_function_null(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_TYPE_FUNCTION_GET_KEY_KIND);
        goto done;
    }

    if (type_plugin->instance_to_keyhash == NULL)
    {
        DDSC_LOG_type_function_null(OSAPI_LOGKIND_ERROR,
                DDSC_LOG_TYPE_FUNCTION_INSTANCE_TO_KEYHASH);
        goto done;
    }

    retval = DDS_BOOLEAN_TRUE;

done:

    return retval;
}
#endif

/*ci @} */

