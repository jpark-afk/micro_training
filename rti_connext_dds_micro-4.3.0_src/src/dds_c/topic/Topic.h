/*
 * FILE: Topic.h - DDS Topic implementation
 *
 * (c) Copyright 2008-2016 Real-Time Innovations,
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
 * 27jun2016,tk MICRO-1545 Added support APIs for delete_topic() and find_topic()
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 * 30jun2015,tk MICRO-1378/PR#15203 Updated comments
 * 31jul2014,tk MICRO-842/PR#9683 - Removed superfluous parameters in DB
 *                                  compare function
 * 08nov2013,as MICRO-681 Complete implementation of WaitSets
 *              and support for StatusConditions
 * 06jul2012,tk Updated
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief DDS Topic implementation
 */
/*ci \addtogroup DDSDomainModule
 * @{
 */
#ifndef Topic_h
#define Topic_h

#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_discovery_plugin_h
#include "dds_c/dds_c_discovery_plugin.h"
#endif

#include "Entity.h"
#include "Conditions.h"
#include "TopicDescription.h"
#include "InstanceHandle.h"
#include "TopicQos.h"
#include "Type.h"

typedef DDS_Boolean
(*DomainParticipantListener_on_inconsistent_topic)(
        DDS_DomainParticipant *participant,
        DDS_Topic *topic,
        const struct DDS_InconsistentTopicStatus *status);

/*ci
 * \brief Configuration data for a topic
 */
struct NDDS_TopicConfig
{
    /*ci
     * \brief The database to create tables in
     */
    DB_Database_T db;

    /*ci
     *\brief Function to retrieve the parent instance handle
     */
    RTIDDS_EntityGetInstanceHandleFunction get_parent_handle;

    /*ci
     * \brief Callback to forward on_inconsistent_topic events if not
     *        handled by the topic
     */
    DomainParticipantListener_on_inconsistent_topic on_inconsistent_topic;
};

/*ci
 * \def NDDS_TopicConfig_INITIALIZER
 * \brief Constant to initialize NDDS_TopicConfig
 */
#define NDDS_TopicConfig_INITIALIZER \
{\
     NULL,\
     NULL,\
     NULL\
}

/*ci
 * \brief  Implementation of the DDS Topic entity
 */
struct DDS_TopicImpl
{
    /*ci
     * \brief Inherit from DDS_Entity
     */
    struct DDS_EntityImpl as_entity;

    /*ci
     * \brief The topic name only copy the required size
     */
    char *topic_name;

    /*ci
     * \brief A topic is both an entity and a topic-description. This structure
     *        enables conversion between a topic and topic-description.
     */
    struct DDS_TopicDescriptionImpl as_topicdescription;

    /*i \dref_TopicQos_management
     */
    struct RTI_ManagementQosPolicy management;

    /*e \dref_TopicQos_topic_data
     */
    struct DDS_TopicDataQosPolicy *topic_data;

    /*ci
     * \brief The current Topic listener, as specified by the user
     */
    struct DDS_TopicListener listener;

    /*ci
     * \brief A reference to a DDS type for the topic.
     */
    DDS_Type *type;

    /*ci
     * \brief The current status' the Topic is interested in
     *        as specified by the user
     */
    DDS_StatusMask mask;

    /*ci
     * \brief Inconsistent topic status, updated on matching events
     */
    struct DDS_InconsistentTopicStatus inconsistent_status;

    /*ci
     * \brief Copy of the configuration data received during initialization
     */
    struct NDDS_TopicConfig *config;

    /*ci
     * \brief The number of time this topic is referenced to support
     *        \ref DDS_DomainParticipant_find_topic. A topic can only be deleted
     *        if the reference count is 0
     */
    DDS_Long ref_count;

    /*ci
     * \brief The number of entities that has been created using this topic.
     *        delete_topic() differentiates between a topic being referenced
     *        with find_topic() versus being in use by an entity such as
     *        data reader, data writer, multi topic etc.
     */
    DDS_Long inuse_count;
};

MUST_CHECK_RETURN extern DDS_Boolean
DDS_TopicListener_is_consistent(const struct DDS_TopicListener *l,
                                DDS_StatusMask m);

MUST_CHECK_RETURN extern RTI_INT32
DDS_TopicImpl_compare(RTI_INT32 flags,const DB_Record_T op1, void *op2);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_TopicImpl_is_hidden(DDS_Topic *self);

#ifndef RTI_CERT
SHOULD_CHECK_RETURN extern DDS_Boolean
DDS_TopicImpl_finalize(struct DDS_TopicImpl *topic);

SHOULD_CHECK_RETURN extern RTI_BOOL
DDS_Topic_dtor(DB_Record_T topic_record);

#endif

MUST_CHECK_RETURN extern DDS_Boolean
DDS_TopicImpl_initialize(struct DDS_TopicImpl *topic,
                         DDS_DomainParticipant *participant,
                         const char *topic_name,
                         DDS_Type *type,
                         const struct DDS_TopicQos *qos,
                         const struct DDS_TopicListener *listener,
                         DDS_StatusMask mask,
                         DDS_UnsignedLong object_id,
                         struct NDDS_TopicConfig *config);

extern void
DDS_TopicImpl_attach(DDS_Topic *topic);

extern void
DDS_TopicImpl_detach(DDS_Topic *topic);

extern DDS_Boolean
DDS_TopicImpl_is_attached(DDS_Topic *topic);

extern void
DDS_TopicImpl_reference(DDS_Topic *topic);

extern void
DDS_TopicImpl_dereference(DDS_Topic *topic);

extern DDS_Boolean
DDS_TopicImpl_is_referenced(DDS_Topic *topic);

extern void
DDS_TopicImpl_reset_reference(DDS_Topic *topic);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_Topic_is_compatible(DDS_Topic *left,
                        const char *right_topic_name,
                        const char *right_type_name,
                        DDS_UnsignedLong dr_id,
                        DDS_UnsignedLong dw_id);

extern DDS_UnsignedLong
DDS_Topic_get_object_suffix(DDS_Topic *self,
                            DDS_UnsignedLong oid,
                            DDS_Boolean is_writer);

MUST_CHECK_RETURN extern DDS_Type*
DDS_Topic_get_type(DDS_Topic *topic);

#ifdef ENABLE_STATUS_LISTENER
extern void
DDS_Topic_on_inconsistent_topic(DDS_Topic *self);
#endif

#endif

/*ci @} */

