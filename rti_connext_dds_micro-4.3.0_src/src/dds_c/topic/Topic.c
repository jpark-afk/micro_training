/*
 * FILE: Topic.c - DDS Topic implementation
 *
 * (c) Copyright 2008-2025 Real-Time Innovations,
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
 * 12dec2016,tk MICRO-1574 Added check for key consistency in is_comptaible()
 * 27jun2016,tk MICRO-1545 Added support APIs for delete_topic() and find_topic()
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 * 12mar2015,tk MICRO-1104/PR#14175 Removed redundant code
 * 27jan2014,tk MICRO-1025/PR#13452 Removed redundant code
 *              MICRO-1026/PR#13453 Added comments
 * 05dec2014,as Additional fixes for MICRO-969
 * 20sep2014,as Moved implementation of support functions for Status types to
 *              TopicStatus.c
 * 16sep2014,tk MICRO-870 Check that participant is enabled in enable()
 * 16sep2014,as MICRO-903/PR#11236 - Incorrect handling of status events and listeners
 * 31jul2014,tk MICRO-842/PR#9683 - Removed superfluous parameters in DB
 *                                  compare function
 * 15may2014,as MICRO-317 (Verocel PR#1443) Replaced String_cmp with String_ncmp
 * 07may2014,as MICRO-784 Expose get_X_status API in C++ (implementation of
 *              support functions for status types);
 *              MICRO-783 Implement DDS_Topic_get_inconsistent_topic_status
 * 05may2014,as MICRO-270 Always enable precondition
 *              checks for public API operations
 * 08nov2013,as MICRO-681 Complete implementation of WaitSets
 *              and support for StatusConditions
 * 02aug2013,tk MICRO-319 / PR#1449: Call DDS_Topic_on_inconsistent_topic
 * 19jul2013,as Fixed MICRO-668 (DDS_Topic_set_qos)
 * 11jun2013,tk MICRO-632: max topic length is 255 excluding NUL
 * 06jul2012,tk Updated
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief DDS Topic implementation
 *
 * \details
 * This file implements the DDS Topic API.
 */
/*ci \addtogroup DDSDomainModule
 * @{
 */
#include "Topic.h"
#include "QosPolicy.h"
#include "UserDataQosPolicy.h"
#include "BuiltinCdr.h"

#if INCLUDE_API_QOS
#include "DomainParticipant.h"
#endif

RTI_PRIVATE struct DDS_TopicDataQosPolicy DDS_TOPIC_DATA_DEFAULT = DDS_TOPIC_DATA_QOS_POLICY_DEFAULT;

/*******************************************************************************
 *                              Internal/Peer API
 ******************************************************************************/

/*** SOURCE_BEGIN ***/

RTI_PRIVATE DDS_ReturnCode_t
DDS_Topic_set_qos_from(
        DDS_Topic *out,
        const struct DDS_TopicQos *in,
        DDS_Boolean shallow_copy,
        DDS_DomainParticipant *participant)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    PRECOND_ARG(shallow_copy)
    PRECOND_ARG(participant)

    OSAPI_PRECONDITION((out == NULL) || (in == NULL) ||
                            (shallow_copy && (participant == NULL)),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("out",out,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("in",in,RTI_FALSE);
                        OSAPI_Log_entry_add_uint("shallow_copy",shallow_copy,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("participant",participant,RTI_TRUE);)


    if (!DDS_TopicDataQosPolicy_is_equal(out->topic_data,&in->topic_data))
    {
        if (out->topic_data == &DDS_TOPIC_DATA_DEFAULT)
        {
            OSAPI_Heap_allocate_struct(&out->topic_data,
                                       struct DDS_TopicDataQosPolicy);

            if (out->topic_data == NULL)
            {
                retcode = DDS_RETCODE_OUT_OF_RESOURCES;
                goto done;
            }

            retcode = DDS_TopicDataQosPolicy_initialize(out->topic_data);
            if (retcode != DDS_RETCODE_OK)
            {
                goto done;
            }
        }

        if (shallow_copy)
        {
            if (!DDS_UserDataManager_assert_user_data(
                    DDS_DomainParticipant_get_user_data_manager(participant),
                    DDS_USER_DATA_TOPIC_TYPE,
                    &in->topic_data.value,
                    &out->topic_data->value))
            {
                retcode = DDS_RETCODE_ERROR;
                goto done;
            }
        }
        else
        {
            if (DDS_TopicDataQosPolicy_copy(out->topic_data,&in->topic_data) != DDS_RETCODE_OK)
            {
                retcode = DDS_RETCODE_ERROR;
                goto done;
            }
        }
    }

    out->management = in->management;
    retcode = DDS_RETCODE_OK;

done:

    if ((retcode != DDS_RETCODE_OK) &&
        (out->topic_data != &DDS_TOPIC_DATA_DEFAULT) &&
        (out->topic_data != NULL))
    {
        OSAPI_Heap_free_struct(out->topic_data);
        out->topic_data = &DDS_TOPIC_DATA_DEFAULT;
    }

    return retcode;
}

#ifndef RTI_CERT
RTI_PRIVATE DDS_ReturnCode_t
DDS_Topic_get_qos_from(const DDS_Topic *self,
                       struct DDS_TopicQos *out)
{
    OSAPI_PRECONDITION((self == NULL) || (out == NULL),
                        return DDS_BOOLEAN_FALSE,
                        OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("out",out,RTI_TRUE);)

    if (DDS_RETCODE_OK != DDS_TopicDataQosPolicy_copy(&out->topic_data,
                                                      self->topic_data))
    {
        return DDS_RETCODE_ERROR;
    }

    out->management = self->management;

    return DDS_RETCODE_OK;
}

/*ci
 * \brief Check that the immutable part of two topic qos policies are valid
 *
 * \param[in] left  Left side of comparison
 * \param[in] right Right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
RTI_PRIVATE DDS_Boolean
DDS_TopicImpl_is_immutable_equal(const DDS_Topic *self,
                                 const struct DDS_TopicQos *right)
{
    OSAPI_PRECONDITION(self == NULL || right == NULL,
                           return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    if (!RTI_ManagementQosPolicy_is_equal(&self->management,&right->management))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_TopicDataQosPolicy_is_equal(self->topic_data,&right->topic_data))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}
#endif

DDS_ReturnCode_t
DDS_Topic_get_inconsistent_topic_status(DDS_Topic *self,
                                     struct DDS_InconsistentTopicStatus *status)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (status == NULL),
               return DDS_RETCODE_BAD_PARAMETER,
               OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("status",status,RTI_TRUE);)

    if (DB_Database_lock(self->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    *status = self->inconsistent_status;

    DDS_InconsistentTopicStatus_reset(&self->inconsistent_status);
    if (!DDS_EntityImpl_disable_status(DDS_Topic_as_entity(self),
                                       DDS_INCONSISTENT_TOPIC_STATUS))
    {
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    if (DB_Database_unlock(self->config->db) != DB_RETCODE_OK)
    {
        retcode = DDS_RETCODE_ERROR;
    }

    return retcode;
}

/*ci
 * \brief Compare entries in the table of topic entries. The function is
 *        compatible with \ref DB_IndexCompare_T
 *
 * \param[in] flags Used to distinguish between op2 being a key or record
 * \param[in] op1   A DDS_TopicImpl already in the database
 * \param[in] op2   Either a DDS_TopicImpl being added or a key
 *                  being searched for
 *
 * \return positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2
 *         zero if op1 is equal to op2
 */
RTI_INT32
DDS_TopicImpl_compare(RTI_INT32 flags,const DB_Record_T op1, void *op2)
{
    struct DDS_TopicImpl *record_left = (struct DDS_TopicImpl*)op1;
    const char *name_right;

    if (DB_SELECT_OP2_IS_KEY(flags))
    {
        name_right = (const char*)op2;
    }
    else
    {
        name_right = ((struct DDS_TopicImpl*)op2)->topic_name;
    }

    return REDA_String_ncompare(record_left->topic_name,name_right,
                    RTPS_PATHNAME_LEN_MAX);
}

/*ci
 * \brief Check if a topic is hidden.
 *
 * \details
 *
 * A topic that is hidden is not included in functions that operate on
 * an entire factory, such as delete_containted_entities. Entities that
 * are hidden must be explicitly deleted.
 *
 * \param[in] self Topic to test
 *
 * \return DDS_BOOLEAN_TRUE if hidden, DDS_BOOLEAN_FALSE if not hidden
 */
DDS_Boolean
DDS_TopicImpl_is_hidden(DDS_Topic *self)
{
    return self->management.is_hidden;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize a Topic
 *
 * \details
 * Free up all resources used by a topic. Note that this function does
 * not free the memory used to hold the topic itself, this memory is
 * freed by the participant because the participant is the factory.
 *
 * \param[in] topic The topic to finalize
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DDS_TopicImpl_finalize(struct DDS_TopicImpl *topic)
{
    if (!DDS_EntityImpl_finalize(&topic->as_entity))
    {
        DDSC_LOG_ENTITY_FINALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_TOPIC_ENTITY)
        return DDS_BOOLEAN_FALSE;
    }

    if (topic->topic_data != &DDS_TOPIC_DATA_DEFAULT)
    {
        if (DDS_TopicDataQosPolicy_finalize_no_dealloc(
                topic->topic_data,
                DDS_DomainParticipant_get_user_data_manager(
                    topic->as_topicdescription.participant),
                DDS_USER_DATA_TOPIC_TYPE) != DDS_RETCODE_OK)
        {
            return DDS_RETCODE_ERROR;
        }

        OSAPI_Heap_free_struct(topic->topic_data);
    }

    topic->topic_name = NULL;

    DDS_TopicDescriptionImpl_finalize(&topic->as_topicdescription);
    DDS_TypeImpl_detach_topic(topic->type);

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Finalize a Topic record
 *
 * \param[in] topic_record Topic record to finalize
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
DDS_Topic_dtor(DB_Record_T topic_record)
{
    return DDS_TopicImpl_finalize((struct DDS_TopicImpl*)topic_record) ?
                                RTI_TRUE : RTI_FALSE;
}
#endif

/*ci
 * \brief Return the instance handle of a topic
 *
 * \details
 * This function overrides the DDS entity get_instance_handle function.
 *
 * \param[in] entity The base-class for the topic
 *
 * \return The instance handle
 */
MUST_CHECK_RETURN RTI_PRIVATE DDS_InstanceHandle_t
DDS_TopicImpl_get_instance_handle(DDS_Entity *entity)
{
    struct DDS_TopicImpl *topic = (struct DDS_TopicImpl*)entity;
    DDS_InstanceHandle_t retval;

    retval = topic->config->get_parent_handle((DDS_Entity*)topic->as_topicdescription.participant);

    DDS_InstanceHandle_set_suffix(&retval,topic->as_entity.entity_id);

    return retval;
}

/*ci
 * \brief Enable a topic
 *
 * \details
 *
 * This function overrides the DDS entity enable function.
 *
 * \param[in] self The base-class for the topic
 *
 * \return DDS_RETCODE_OK on success, one of \ref DDS_ReturnCode_t on error
 */
MUST_CHECK_RETURN RTI_PRIVATE DDS_ReturnCode_t
DDS_Topic_enable(DDS_Entity *self)
{
    struct DDS_TopicImpl *topic = (struct DDS_TopicImpl*)self;

    if (!DDS_Entity_is_enabled(DDS_DomainParticipant_as_entity(
                                      topic->as_topicdescription.participant)))
    {
        DDSC_LOG_ENTITY_NOT_ENABLED(OSAPI_LOGKIND_ERROR,DDSC_LOG_TOPIC_ENTITY)
        return DDS_RETCODE_PRECONDITION_NOT_MET;
    }

    topic->as_entity.state = RTIDDS_ENTITY_STATE_ENABLED;

    return DDS_RETCODE_OK;
}

/*ci
 * \brief Initialize a topic
 *
 * \details
 *
 * The participant allocates memory to store the topic data and passes
 * it to the publisher for initialization. The topic allocates all its
 * internal resources. The topic is passed shared resources in the
 * config structure, such as database, timers resolvers etc. The resources
 * are typically managed by the domain participant.
 *
 * \param[in] topic       A topic structure to initialize
 * \param[in] participant The participant creating the publisher
 * \param[in] topic_name  The topic name for the topic
 * \param[in] type        A reference to the type for the topic
 * \param[in] qos         The topic qos policy
 * \param[in] listener    The topic listener
 * \param[in] mask        Mask with enabled statuses on the topic
 * \param[in] object_id   The topic object id generated by the factory
 * \param[in] config      General topic configuration

 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DDS_TopicImpl_initialize(struct DDS_TopicImpl *topic,
                         DDS_DomainParticipant *participant,
                         const char *topic_name,
                         DDS_Type *type,
                         const struct DDS_TopicQos *qos,
                         const struct DDS_TopicListener *listener,
                         DDS_StatusMask mask,
                         DDS_UnsignedLong object_id,
                         struct NDDS_TopicConfig *config)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    RTI_SIZE_T topic_length;
    struct DDS_TopicListener nil_listener = DDS_TopicListener_INITIALIZER;
    struct DDS_DomainParticipantQos *dp_qos = NULL;

    topic_length = DDS_String_length(topic_name);
    if (topic_length > RTPS_PATHNAME_LEN_MAX)
    {
        DDSC_LOG_TOPIC_TOO_LONG(OSAPI_LOGKIND_ERROR,topic_length)
        goto done;
    }

    /* Verify that TopicQoS is consistent with PartcipantQoS */
    dp_qos = DDS_DomainParticipant_get_qos_ref(participant);
    if (!DDS_TopicDataQosPolicy_is_consistent(
            &qos->topic_data, dp_qos->resource_limits.topic_data_max_length))
    {
        DDSC_LOG_QOS_INCONSISTENT(OSAPI_LOGKIND_ERROR,DDSC_LOG_TOPIC_QOS)
        goto done;
    }

    if ((listener != NULL) &&
        !DDS_TopicListener_is_consistent(listener, mask))
    {
        DDSC_LOG_LISTENER_INCONSISTENT(OSAPI_LOGKIND_ERROR,
                                       DDSC_LOG_TOPIC_LISTENER,mask)
        goto done;
    }

    /* The length of topic_name has already been verified */
    topic->topic_name = (char*)topic_name;
    if (topic->topic_name == NULL)
    {
        goto done;
    }
    DDS_TopicDescriptionImpl_initialize(&topic->as_topicdescription,
                        (const char *)topic->topic_name,
                        DDS_TypeImpl_get_type_name_reference(type),
                        topic,participant);

    topic->topic_data = &DDS_TOPIC_DATA_DEFAULT;

#if INCLUDE_API_QOS
    if (qos == &DDS_TOPIC_QOS_DEFAULT)
    {
        if (DDS_Topic_set_qos_from(topic, participant->_default_topic_qos,
                DDS_BOOLEAN_TRUE, participant) != DDS_RETCODE_OK)
        {
            DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_TOPIC_QOS)
            goto done;
        }
    }
    else
#endif
    {
        if (DDS_Topic_set_qos_from(topic, qos,
                DDS_BOOLEAN_TRUE, participant) != DDS_RETCODE_OK)
        {
            DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_TOPIC_QOS)
            goto done;
        }
    }

    if (!DDS_EntityImpl_initialize(&topic->as_entity,
                          DDS_TOPIC_ENTITY_KIND,
                          object_id,
                          DDS_Topic_enable,
                          DDS_TopicImpl_get_instance_handle))
    {
        DDSC_LOG_ENTITY_INITIALIZE(OSAPI_LOGKIND_ERROR,DDSC_LOG_TOPIC_ENTITY)
        goto done;
    }

    DDS_TypeImpl_attach_topic(type);
    topic->config = config;
    topic->type = type;

    topic->inconsistent_status.total_count = 0;
    topic->inconsistent_status.total_count_change = 0;
    topic->ref_count = 1;
    topic->inuse_count = 0;

    if (listener == NULL)
    {
        topic->listener = nil_listener;
    }
    else
    {
        topic->listener = *listener;
    }
    topic->mask = mask;

    retval = DDS_BOOLEAN_TRUE;

done:
    return retval;
}

#ifdef ENABLE_STATUS_LISTENER
/*ci
 * \brief Notify a listener on the on_inconsistent_topic event
 *
 * \param[in] self The DDS topic the notification occurred on
 */
void
DDS_Topic_on_inconsistent_topic(DDS_Topic *self)
{
    DDS_Boolean event_consumed = DDS_BOOLEAN_FALSE;
    struct DDS_InconsistentTopicStatus status;

    ++self->inconsistent_status.total_count;
    ++self->inconsistent_status.total_count_change;

    status = self->inconsistent_status;

    if (self->mask & DDS_INCONSISTENT_TOPIC_STATUS)
    {
        if (self->listener.on_inconsistent_topic != NULL)
        {
            DDS_EntityImpl_disable_status(&self->as_entity,
                    DDS_INCONSISTENT_TOPIC_STATUS);
            DDS_InconsistentTopicStatus_reset(&self->inconsistent_status);
            self->listener.on_inconsistent_topic(self->listener.as_listener.
                                              listener_data, self,
                                              &status);
            event_consumed = DDS_BOOLEAN_TRUE;
        }
    }
    else
    {
        if (self->config->on_inconsistent_topic != NULL)
        {
            event_consumed = self->config->on_inconsistent_topic(
                                    self->as_topicdescription.participant,
                                    self,
                                    &status);
        }
    }

    DDS_EntityImpl_enable_status(&self->as_entity,
            DDS_INCONSISTENT_TOPIC_STATUS, event_consumed);

}
#endif

/*ci
 * \brief Attach object/entity to the topic
 *
 * \details
 * A topic keeps track of how many entities have been created with this
 * it. It is not possible to delete a topic that is in use by an entity
 * or object. Note that this counter is different from the ref_count,
 * which is only used to keep track of references to a topic.
 *
 * \param[in] self Topic to attach to
 */
void
DDS_TopicImpl_attach(DDS_Topic *self)
{
    ++self->inuse_count;
}

/*ci
 * \brief Detach object/entity from the topic
 *
 * \details
 * A topic keeps track of how many entities have been created with this
 * it. It is not possible to delete a topic that is in use by an entity
 * or object. Note that this counter is different from the ref_count,
 * which is only used to keep track of references to a topic.
 *
 * \param[in] self Topic to detach from
 */
void
DDS_TopicImpl_detach(DDS_Topic *self)
{
    --self->inuse_count;
}

/*ci
 * \brief Test if a topic is attached to an object/entity
 * *
 * \param[in] self Topic to test
 */
DDS_Boolean
DDS_TopicImpl_is_attached(DDS_Topic *self)
{
    return (self->inuse_count > 0) ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/*ci
 * \brief Add a reference to the topic
 *
 * \details
 * A topic keeps track of how many times it is being referenced by
 * create_topic and find_topic. It is not allowed to delete a topic
 * that is being referenced.
 *
 * \param[in] self Topic to reference
 */
void
DDS_TopicImpl_reference(DDS_Topic *self)
{
    ++self->ref_count;
}

/*ci
 * \brief Remove a reference to the topic
 *
 * \details
 * A topic keeps track of how many times it is being referenced by
 * create_topic and find_topic. It is not allowed to delete a topic
 * that is being referenced.
 *
 * \param[in] self Topic to dereference
 */
void
DDS_TopicImpl_dereference(DDS_Topic *self)
{
    --self->ref_count;
}

/*ci
 * \brief Test if a topic is being referenced
 *
 * \param[in] self Topic to test
 */
DDS_Boolean
DDS_TopicImpl_is_referenced(DDS_Topic *self)
{
    return (self->ref_count > 0) ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/*ci
 * \brief Reset topic references
 *
 * \details
 * The primary use of this function is as part of delete_contained_entities to
 * force the topic to have a reference count of 1 (created) to that is can be
 * properly deleted.
 *
 * \param[in] self Topic to reset
 */
void
DDS_TopicImpl_reset_reference(DDS_Topic *self)
{
    self->ref_count = 1;
}

/*******************************************************************************
 *                                Public API
 ******************************************************************************/
DDS_Entity*
DDS_Topic_as_entity(DDS_Topic *self)
{
    struct DDS_TopicImpl *topic = (struct DDS_TopicImpl*)self;

    OSAPI_PRECONDITION_ALWAYS(topic == NULL,
                           return NULL,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return &topic->as_entity;
}

DDS_TopicDescription*
DDS_Topic_as_topicdescription(DDS_Topic *self)
{
    struct DDS_TopicImpl *topic = (struct DDS_TopicImpl*)self;

    OSAPI_PRECONDITION_ALWAYS(topic == NULL,
                           return NULL,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return &topic->as_topicdescription;
}

DDS_Topic*
DDS_Topic_narrow(DDS_TopicDescription *self)
{
    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                           return NULL,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return (DDS_Topic*)self->owner;
}

#ifndef RTI_CERT
DDS_ReturnCode_t
DDS_Topic_set_listener(DDS_Topic *self,
                       const struct DDS_TopicListener *listener,
                       DDS_StatusMask mask)
{
    struct DDS_TopicListener nil_listener =
                    DDS_TopicListener_INITIALIZER;

    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if ((listener != NULL) &&
            !DDS_TopicListener_is_consistent(listener, mask))
    {
        DDSC_LOG_LISTENER_INCONSISTENT(OSAPI_LOGKIND_ERROR,
                                       DDSC_LOG_TOPIC_LISTENER,mask)
        return DDS_RETCODE_BAD_PARAMETER;
    }

    if (DB_Database_lock(self->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (listener == NULL)
    {
        self->listener = nil_listener;
    }
    else
    {
        self->listener = *listener;
    }

    self->mask = mask;

    if (DB_Database_unlock(self->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}
#endif

/*ci
 * \brief Check if two topics are compatible
 *
 * \details
 *
 * The compatibility check is according to the DDS specification. Two topics
 * cannot be compatible if their topic names are different. In this case
 * FALSE is returned. If the topic names are equal but the types are different
 * they are considered to inconsistent. In this case the inconsistent_topic
 * status is triggered and the user application notified (if the event
 * is handled) and FALSE is returned. If both the  topic-name and
 * type-name are compatible the function returns TRUE.
 *
 * Two DDS object-id's are compatible if they are both are keyed
 * or both are unkeyed. They are not compatible if they one if keyed and the
 * other not. Note that the type of key, GUID or USER, is not relevant. Also,
 * it is not relevant if one is built-in and the other not.
 *
 * Note that this function does not perform a Qos compatibility check. This
 * must be done in addition to this test.
 *
 * \param[in] left             The left side of the comparison
 * \param[in] right_topic_name The right side topic-name
 * \param[in] right_type_name  The right side type-name
 * \param[in] dr_id            The datareader object id
 * \param[in] dw_id            The datawriter object id
 *
 * \return DDS_BOOLEAN_TRUE if the topics are compatible, DDS_BOOLEAN_FALSE
 *         otherwise.
 */
DDS_Boolean
DDS_Topic_is_compatible(DDS_Topic *left,
                        const char *right_topic_name,
                        const char *right_type_name,
                        DDS_UnsignedLong dr_id,
                        DDS_UnsignedLong dw_id)
{
    const char *left_topic_name;
    const char *left_type_name;

    left_topic_name =
        DDS_TopicDescription_get_name(DDS_Topic_as_topicdescription(left));

    left_type_name =
        DDS_TopicDescription_get_type_name(DDS_Topic_as_topicdescription(left));

    if (left_topic_name == NULL || right_topic_name == NULL)
    {
        DDSC_LOG_TOPIC_NAME_CMP(OSAPI_LOGKIND_ERROR,
                                 left_topic_name,right_topic_name)
        return DDS_BOOLEAN_FALSE;
    }
    if (left_type_name == NULL || right_type_name == NULL)
    {
        DDSC_LOG_TYPE_NAME_CMP(OSAPI_LOGKIND_ERROR,
                                left_type_name,right_type_name)
        return DDS_BOOLEAN_FALSE;
    }

    /* This function tests if two topics are compatible. Thus,
     * it is not considered an error if they have different names and
     * an error is not logged.
     */
    if (DDS_String_ncmp(left_topic_name,
            right_topic_name,RTPS_PATHNAME_LEN_MAX))
    {
        return DDS_BOOLEAN_FALSE;
    }

    /* However, if the topics have the same name but are of
     * different types it is considered an error. Log an error message.
     */
    if (DDS_String_ncmp(left_type_name,
            right_type_name,RTPS_PATHNAME_LEN_MAX))
    {
        DDSC_LOG_TYPE_NAME_CMP(OSAPI_LOGKIND_ERROR,
                                left_type_name,right_type_name)
#ifdef ENABLE_STATUS_LISTENER
        DDS_Topic_on_inconsistent_topic(left);
#endif
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_ObjectId_is_compatible(dr_id,dw_id))
    {
        DDSC_LOG_TYPE_KEY_TYPE(OSAPI_LOGKIND_ERROR,
                               right_topic_name,right_type_name);
#ifdef ENABLE_STATUS_LISTENER
        DDS_Topic_on_inconsistent_topic(left);
#endif
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Calculate the object if suffix for a datareader/datawriter given an
 *        integer
 *
 * \param[in] self        The topic to generate a topic id for
 * \param[in] oid         The initial object id, an unsigned integer
 * \param[in] is_writer   DDS_BOOLEAN_TRUE if the object to generate the suffix
 *                        for is a datawriter, otherwise datareader is assumed
 *
 * \return The generated object id, -1 on failure
 */
DDS_UnsignedLong
DDS_Topic_get_object_suffix(DDS_Topic *self,
                            DDS_UnsignedLong oid,
                            DDS_Boolean is_writer)
{
     NDDS_TypePluginKeyKind key_kind;
     DDS_UnsignedLong rval = 0;

     key_kind = DDS_TypeImpl_get_plugin(DDS_Topic_get_type(self))->key_kind;

     if (key_kind == NDDS_TYPEPLUGIN_USER_KEY)
     {
         if (is_writer)
         {
             rval = (oid << 8) | RTPS_OBJECT_NORMAL_USER_CST_WRITER;
         }
         else

         {
             rval = (oid << 8) | RTPS_OBJECT_NORMAL_USER_CST_READER;
         }
     }
     else if (key_kind == NDDS_TYPEPLUGIN_NO_KEY)
     {
         if (is_writer)
         {
             rval = (oid << 8) | RTPS_OBJECT_NORMAL_USER_PUBLICATION;
         }
         else
         {
             rval = (oid << 8) | RTPS_OBJECT_NORMAL_USER_SUBSCRIPTION;
         }
     }

     return rval;
}

#if INCLUDE_API_QOS
DDS_ReturnCode_t
DDS_Topic_get_qos(DDS_Topic *self, struct DDS_TopicQos *qos)
{
    struct DDS_TopicImpl *topic = (struct DDS_TopicImpl*)self;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (qos == NULL),
                    return DDS_RETCODE_BAD_PARAMETER,
                    OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                    OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE);)


    if (DB_Database_lock(topic->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (DDS_Topic_get_qos_from(self,qos) != DDS_RETCODE_OK)
    {
        DDSC_LOG_QOS_COPY(OSAPI_LOGKIND_ERROR,DDSC_LOG_TOPIC_QOS)
        return DDS_RETCODE_ERROR;
    }

    if (DB_Database_unlock(topic->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}
#endif

#if INCLUDE_API_QOS
DDS_ReturnCode_t
DDS_Topic_set_qos(DDS_Topic *self, const struct DDS_TopicQos *qos)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    struct DDS_TopicImpl *topic = (struct DDS_TopicImpl*)self;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (qos == NULL),
                        return DDS_RETCODE_BAD_PARAMETER,
                        OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE);)

    if (DB_Database_lock(topic->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    if (!DDS_TopicImpl_is_immutable_equal(topic,qos))
    {
        DDSC_LOG_QOS_IMMUTABLE(OSAPI_LOGKIND_ERROR,DDSC_LOG_TOPIC_QOS)
        retcode = DDS_RETCODE_IMMUTABLE_POLICY;
        goto done;
    }

    retcode = DDS_Topic_set_qos_from(self, qos, DDS_BOOLEAN_TRUE,
                                     self->as_topicdescription.participant);

done:

    if (DB_Database_unlock(topic->config->db) != DB_RETCODE_OK)
    {
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}
#endif

/*ci
 * \brief Return a pointer to the type-object for the topic
 *
 * \details
 * Return a pointer to the type used by a DDS topic. Note that the
 * type is not automatically reference counted and care must be taken to
 * not access this pointer on a invalid object. It is up to the caller to
 * determine if the pointer is valid or not.
 *
 * \param[in] topic The topic to return the type object for
 *
 * \return Pointer to the type-object for the topic
 */
DDS_Type*
DDS_Topic_get_type(DDS_Topic *topic)
{
    OSAPI_PRECONDITION(topic == NULL,
                       return NULL,
                       OSAPI_Log_entry_add_pointer("topic",topic,RTI_TRUE);)

    return topic->type;
}

#ifndef RTI_CERT
struct DDS_TopicListener
DDS_Topic_get_listener(DDS_Topic *topic)
{
    struct DDS_TopicListener retval = DDS_TopicListener_INITIALIZER;
    struct DDS_TopicListener nil_retval = DDS_TopicListener_INITIALIZER;

    OSAPI_PRECONDITION_ALWAYS(topic == NULL,
                           return retval,
                           OSAPI_Log_entry_add_pointer("topic",topic,RTI_TRUE);)

    if (DB_Database_lock(topic->config->db) != DB_RETCODE_OK)
    {
        return nil_retval;
    }

    retval = topic->listener;

    if (DB_Database_unlock(topic->config->db) != DB_RETCODE_OK)
    {
        return nil_retval;
    }

    return retval;
}
#endif

void
DDS_InconsistentTopicStatus_reset(struct DDS_InconsistentTopicStatus *s)
{
    s->total_count_change = 0;
}

/*ci
 * \brief Check if a DDS_TopicListener is consistent
 *
 * \param[in] l DDS_TopicListener to test for consistency
 * \param[in] m The current status mask to test
 *
 * \return evaluates to logical true on success, false on failure
 */
DDS_Boolean
DDS_TopicListener_is_consistent(const struct DDS_TopicListener *l,
                                DDS_StatusMask m)
{
    return ((!((m) & DDS_INCONSISTENT_TOPIC_STATUS)) ||
            (l->on_inconsistent_topic != NULL)) ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

DDS_ReturnCode_t
DDS_Topic_get_topic_data(DDS_Topic *self,
                         struct DDS_TopicDataQosPolicy *topic_data)
{
    return DDS_TopicDataQosPolicy_copy(topic_data,self->topic_data);
}

RTI_BOOL
DDS_Topic_serialize(DDS_Topic *self,struct CDR_Stream_t *stream)
{
    return DDS_CdrQosPolicy_serialize_topic_data(stream,self->topic_data);
}

/*ci @} */

