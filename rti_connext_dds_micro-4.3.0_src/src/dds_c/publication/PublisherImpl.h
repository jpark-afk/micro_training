/*
 * FILE: PublisherImpl.h - Exported PublisherImpl functions
 *
 * Copyright (c) 2010-2026 Real-Time Innovations, Inc.  All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 04nov2015,tk  MICRO-1505 Reduce memory footprint
 * 31jul2014,tk  MICRO-842/PR#9683 - Removed superfluous parameters in DB
 *                                   compare function
 * 19mar2014,tk  MICRO-74: Support endpoint specific transport
 * 08nov2013,as  MICRO-681 Complete implementation of WaitSets
 *               and support for StatusConditions
 *
 */
/*ci \addtogroup DDSPublicationModule
 * @{
 */
#ifndef PublisherImpl_pkg_h
#define PublisherImpl_pkg_h

#include "DataWriterImpl.h"

typedef DDS_Boolean
(*DomainParticipantListener_on_offered_deadline_missed)(
                       DDS_DomainParticipant *self,
                       DDS_DataWriter* writer,
                       const struct DDS_OfferedDeadlineMissedStatus* status);

typedef DDS_Boolean
(*DomainParticipantListener_on_liveliness_lost)(
                       DDS_DomainParticipant *self,
                       DDS_DataWriter *writer,
                       const struct DDS_LivelinessLostStatus *status);

typedef DDS_Boolean
(*DomainParticipantListener_on_offered_incompatible_qos)(
        DDS_DomainParticipant *self,
        DDS_DataWriter* writer,
        const struct DDS_OfferedIncompatibleQosStatus *status);

typedef DDS_Boolean
(*DomainParticipantListener_on_publication_matched)(
        DDS_DomainParticipant *self,
        DDS_DataWriter *writer,
        const struct DDS_PublicationMatchedStatus *status);

typedef DDS_Boolean
(*DomainParticipantListener_on_reliable_reader_activity_changed)(
        DDS_DomainParticipant *self,
        DDS_DataWriter *writer,
        const struct DDS_ReliableReaderActivityChangedStatus *status);

/*ci
 * \brief Configuration data for a publisher
 */
struct NDDS_PublisherConfig
{
    /*ci
     * \brief The database to create tables in
     */
    DB_Database_T db;

    /*ci
     * \brief The timer to create timeouts from
     */
    OSAPI_Timer_T timer;

    /*ci
     * \brief run-time registry to look for components in
     */
    RT_Registry_T *registry;

    /*ci
     * \brief A table to store locally created datawriters in
     */
    DB_Table_T local_writer_table;

    /*ci
     * \brief Callback to call after the datawriter is enabled
     */
    DataWriterLifeCycleListener_on_after_enabled on_after_datawriter_enabled;

    /*ci
     * \brief Callback to call before the datawriter is deleted
     */
    DataWriterLifeCycleListener_on_before_deleted on_before_datawriter_deleted;

    /*ci
     * \brief Callback to call after the datawriter is deleted
     */
    DataWriterLifeCycleListener_on_after_deleted on_after_datawriter_deleted;

    /*ci
     *\brief Function to retrieve the parent instance handle
     */
    RTIDDS_EntityGetInstanceHandleFunction get_parent_handle;

    /*ci
     *\brief Function to generate object-ids
     */
    RTIDDS_ObjectIdGenerator object_id_generator;

    /*ci
     * \brief Callback to forward on_offered_deadline_missed events if not
     *        handled by the publisher
     */
    DomainParticipantListener_on_offered_deadline_missed on_offered_deadline_missed;

    /*ci
     * \brief Callback to forward on_liveliness_lost events if not
     *        handled by the publisher
     */
    DomainParticipantListener_on_liveliness_lost on_liveliness_lost;

    /*ci
     * \brief Callback to forward on_offered_incompatible_qos events if not
     *        handled by the publisher
     */
    DomainParticipantListener_on_offered_incompatible_qos on_offered_incompatible_qos;

    /*ci
     * \brief Callback to forward on_publication_matched events if not
     *        handled by the publisher
     */
    DomainParticipantListener_on_publication_matched on_publication_matched;

    /*ci
     * \brief Callback to forward on_reliable_reader_activity_changed events if not
     *        handled by the publisher
     */
    DomainParticipantListener_on_reliable_reader_activity_changed
        on_reliable_reader_activity_changed;

    /*ci
     * \brief The default discovery unicast locators this publisher
     *        should listen on if none are specified for the publisher
     */
    struct DDS_LocatorSeq *default_unicast;

    /*ci
     * \brief The default multicast locators this publisher should listen on
     *        if none are specified for the publisher
     */
    struct DDS_LocatorSeq *default_multicast;

    /*ci
     * \brief The default discovery unicast locators this publisher
     *        should listen on if none are specified for the publisher
     */
    struct DDS_LocatorSeq *default_meta_unicast;

    /*ci
     * \brief The default discovery multicast locators this publisher
     *        should listen on if none are specified for the publisher
     */
    struct DDS_LocatorSeq *default_meta_multicast;

    /*ci
     * \brief A bind-resolver to find NETIO interface to listen for data on
     */
    NETIO_BindResolver_T *bind_resolver;

    /*ci
     * \brief The transports that are enabled for this publisher
     */
    struct REDA_StringSeq *enabled_transports;

    /*ci
     * \brief A route resolver to find NETIO interface to route to peers
     */
    NETIO_RouteResolver_T *route_resolver;

    NETIO_AddressResolver_T *addr_resolver;

    /*ci
     * \brief Pointer to a participant_id in case none has yet been assigned
     *        by the participant due to no unicast addresses specified
     */
    DDS_Long *participant_id;

    /*ci
     * \brief Pointer to the DDS domain ID of the owning DomainParticipant.
     */
    DDS_DomainId_t *domain_id;

    /*ci
     * \brief Pointer to datawriter configuration structure
     */
    struct NDDS_DataWriterConfig *dw_config;

    /*ci
     */
    DataWriterLifeCycleListener_on_before_datawriter_created
    on_before_datawriter_created;

    /*ci \brief The parent participant's DDS_PropertyQosPolicy
     *
     * This property is propagated because some properties may have
     * a value on the participant that can be overridden at a lower
     * level.
     */
    struct DDS_PropertyQosPolicy *participant_property_qos_policy;

    /*ci \brief The external interface created by the parent Participant
     */
    NETIO_Interface_T *ext_rtps_intf;

    /*ci \brief Shared packet pool
     */
    REDA_BufferPool_T packet_pool;

    /*ci \brief The resolved unicast locators for this publisher. This is used
     *          to keep track of the resolved unicast locators for this
     *          publisher so they can be used for discovery and to add routes
     *          to matched datareaders.
     */
    struct DDS_LocatorSeq *resolved_unicast_seq;

    /*ci \brief The resolved multicast locators for this publisher. This is
     *          used to keep track of the resolved multicast locators for this
     *          publisher so they can be used for discovery and to add routes
     *          to matched datareaders.
     */
    struct DDS_LocatorSeq *resolved_multicast_seq;

#if DDS_FILTERING_ENABLED
    struct DDS_FilterPlugin *filter_plugin;
#endif /* DDS_FILTERING_ENABLED */
};

#if DDS_FILTERING_ENABLED
#define NDDS_PublisherConfig_Filter_INITIALIZER \
    ,\
    NULL
#else
#define NDDS_PublisherConfig_Filter_INITIALIZER
#endif

/*ci
 * \def NDDS_PublisherConfig_INITIALIZER
 * \brief Constant to initialize NDDS_PublisherConfig
 */
#define NDDS_PublisherConfig_INITIALIZER \
{ \
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL\
    NDDS_PublisherConfig_Filter_INITIALIZER \
}

/*ci
 * \brief  Implementation of the DDS Publisher entity
 */
struct DDS_PublisherImpl
{
    /*ci
     * \brief Inherit from DDS_Entity
     */
    struct DDS_EntityImpl as_entity;

    /*----- From create call -----*/

    /*ci \dref_PublisherQos_entity_factory
     */
    struct DDS_EntityFactoryQosPolicy entity_factory;

    /*ci \dref_PublisherQos_partition
     */
    struct DDS_PartitionQosPolicy *partition;

    /*ci \dref_PublisherQos_management
     */
    struct RTI_ManagementQosPolicy management;

    /*ci \dref_PublisherQos_group_data
     */
    struct DDS_GroupDataQosPolicy *group_data;

    /*e \dref_PublisherQos_publisher_name
     */
    char *publisher_name;

    /*ci
     * \brief The current Publisher listener, as specified by the user
     */
    struct DDS_PublisherListener listener;

    /*ci
     * \brief The current status' the Publisher is interested in
     *        as specified by the user
     */
    DDS_StatusMask mask;

    /*ci
     * \brief Pointer to participant that created this publisher
     */
    DDS_DomainParticipant *participant;

    /*----- Support for API -----*/

#if INCLUDE_API_QOS
    /*ci
     * \brief The default datawriter qos if the user creates a datawriter
     *        with the default qos short-cut.
     */
   struct DDS_DataWriterQos *default_qos;
#endif

    /*----- Internal management -----*/
    /*ci
     * \brief The current number of datawriters in this publisher
     */
    DDS_Long dw_count;

    /*----- Properties ------*/
    /*ci
      * \brief Copy of the configuration data received during initialization
      */
    struct NDDS_PublisherConfig *config;
};

MUST_CHECK_RETURN extern DDS_Boolean
DDS_PublisherListener_is_consistent(const struct DDS_PublisherListener *l,
                                    DDS_StatusMask m);

MUST_CHECK_RETURN extern RTI_INT32
DDS_PublisherImpl_compare(RTI_INT32 flags,const DB_Record_T op1, void *op2);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_PublisherImpl_is_hidden(DDS_Publisher *self);

#ifndef RTI_CERT
SHOULD_CHECK_RETURN extern DDS_Boolean
DDS_PublisherImpl_finalize(struct DDS_PublisherImpl *self);
#endif

MUST_CHECK_RETURN extern DDS_InstanceHandle_t
DDS_PublisherImpl_get_instance_handle(DDS_Entity *entity);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_PublisherImpl_initialize(struct DDS_PublisherImpl *publisher,
                             DDS_DomainParticipant *participant,
                             const struct DDS_PublisherQos *qos,
                             const struct DDS_PublisherListener *listener,
                             DDS_StatusMask mask,
                             DDS_UnsignedLong object_id,
                             struct NDDS_PublisherConfig *config);

extern DDS_ReturnCode_t
DDS_Publisher_get_qos_from(const DDS_Publisher *self,
                               struct DDS_PublisherQos *out);

extern DDS_ReturnCode_t
DDS_Publisher_set_qos_from(DDS_Publisher *out,
                               const struct DDS_PublisherQos *in,
                               DDS_DomainParticipant *participant);

MUST_CHECK_RETURN extern DDS_ReturnCode_t
DDS_Publisher_delete_contained_entities_no_lock(DDS_Publisher *self);

MUST_CHECK_RETURN extern DDS_ReturnCode_t
DDS_Publisher_delete_datawriter_no_lock(DDS_Publisher *self,
                                        DDS_DataWriter *datawriter);

#endif /* PublisherImpl_pkg_h */

/*ci @} */
