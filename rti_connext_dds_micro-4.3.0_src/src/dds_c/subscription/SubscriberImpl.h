/*
 * FILE: SubscriberImpl.h - Exported SubscriberImpl functions
 *
 * (c) Copyright, Real-Time Innovations, 2010-2026
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
 * 04nov2015,tk  MICRO-1505 Reduce memory footprint
 * 31jul2014,tk  MICRO-842/PR#9683 - Removed superfluous parameters in DB
 *                                   compare function
 * 19mar2014,tk  MICRO-74: Support endpoint specific transport
 * 08nov2013,as  MICRO-681 Complete implementation of WaitSets
 *               and support for StatusConditions
 *
 */
/*ci \addtogroup DDSSubscriptionModule
 * @{
 */
#ifndef SubscriberImpl_pkg_h
#define SubscriberImpl_pkg_h

#include "DataReaderImpl.h"

typedef DDS_Boolean
(*DomainParticipantListener_on_requested_deadline_missed)(
                        DDS_DomainParticipant *self,
                        DDS_DataReader *reader,
                        const struct DDS_RequestedDeadlineMissedStatus *status);

typedef DDS_Boolean
(*DomainParticipantListener_on_requested_incompatible_qos)(
                        DDS_DomainParticipant *self,
                        DDS_DataReader *reader,
                        const struct DDS_RequestedIncompatibleQosStatus *status);

typedef DDS_Boolean
(*DomainParticipantListener_on_sample_rejected)(
                        DDS_DomainParticipant *self,
                        DDS_DataReader *reader,
                        const struct DDS_SampleRejectedStatus *status);

typedef DDS_Boolean
(*DomainParticipantListener_on_liveliness_changed)(
                        DDS_DomainParticipant *self,
                        DDS_DataReader *reader,
                        const struct DDS_LivelinessChangedStatus *status);

typedef DDS_Boolean
(*DomainParticipantListener_on_data_available)(
                        DDS_DomainParticipant *self,
                        DDS_Subscriber *subscriber,
                        DDS_DataReader *reader);

typedef DDS_Boolean
(*DomainParticipantListener_on_data_on_readers)(
                        DDS_DomainParticipant *self,
                        DDS_Subscriber *subscriber);

typedef DDS_Boolean
(*DomainParticipantListener_on_subscription_matched)(
                           DDS_DomainParticipant *self,
                           DDS_DataReader *reader,
                           const struct DDS_SubscriptionMatchedStatus *status);

typedef DDS_Boolean
(*DomainParticipantListener_on_sample_lost)(
                           DDS_DomainParticipant *self,
                           DDS_DataReader *reader,
                           const struct DDS_SampleLostStatus *status);

typedef DDS_Boolean
(*DomainParticipantListener_on_instance_replaced)(
        DDS_DomainParticipant *self,
        DDS_DataReader *reader,
        const struct DDS_DataReaderInstanceReplacedStatus *status);

/*ci
 * \brief Configuration data for a subscriber
 */
struct NDDS_SubscriberConfig
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
      * \brief A table to store locally created datareaders in
      */
    DB_Table_T local_reader_table;

    /*ci
     * \brief Callback to call after a datareader is enabled
     */
    DataReaderLifeCycleListener_on_after_enabled on_after_datareader_enabled;

    /*ci
     * \brief Callback to call before a datawriter is deleted
     */
    DataReaderLifeCycleListener_on_before_deleted on_before_datareader_deleted;

    /*ci
     *\brief Function to retrieve the parent instance handle
     */
    RTIDDS_EntityGetInstanceHandleFunction get_parent_handle;

    /*ci
     *\brief Function to generate object-ids
     */
    RTIDDS_ObjectIdGenerator object_id_generator;

    /*ci
     * \brief Callback to forward on_requested_deadline_missed events if not
     *        handled by the subscriber
     */
    DomainParticipantListener_on_requested_deadline_missed on_requested_deadline_missed;

    /*ci
     * \brief Callback to forward on_requested_incompatible_qos events if not
     *        handled by the subscriber
     */
    DomainParticipantListener_on_requested_incompatible_qos on_requested_incompatible_qos;

    /*ci
     * \brief Callback to forward on_sample_rejected events if not
     *        handled by the subscriber
     */
    DomainParticipantListener_on_sample_rejected on_sample_rejected;

    /*ci
     * \brief Callback to forward on_liveliness_changed events if not
     *        handled by the subscriber
     */
    DomainParticipantListener_on_liveliness_changed on_liveliness_changed;

    /*ci
     * \brief Callback to forward on_data_available events if not
     *        handled by the subscriber
     */
    DomainParticipantListener_on_data_available on_data_available;

    /*ci
     * \brief Callback to forward on_data_on_readers events if not
     *        handled by the subscriber
     */
    DomainParticipantListener_on_data_on_readers on_data_on_readers;

    /*ci
     * \brief Callback to forward on_subscription_matched events if not
     *        handled by the subscriber
     */
    DomainParticipantListener_on_subscription_matched on_subscription_matched;

    /*ci
     * \brief Callback to forward on_sample_lost events if not
     *        handled by the subscriber
     */
    DomainParticipantListener_on_sample_lost on_sample_lost;

    /*ci
     * \brief Callback to forward on_instance_replaced events if not
     *        handled by the subscriber
     */
    DomainParticipantListener_on_instance_replaced on_instance_replaced;

    /*ci
     * \brief The default discovery unicast locators this subscriber
     *        should listen on if none are specified for the subscriber
     */
    struct DDS_LocatorSeq *default_unicast;

    /*ci
     * \brief The default multicast locators this subscriber should listen on
     *        if none are specified for the subscriber
     */
    struct DDS_LocatorSeq *default_multicast;

    /*ci
     * \brief The default discovery unicast locators this subscriber
     *        should listen on if none are specified for the subscriber
     */
    struct DDS_LocatorSeq *default_meta_unicast;

    /*ci
     * \brief The default discovery multicast locators this subscriber
     *        should listen on if none are specified for the subscriber
     */
    struct DDS_LocatorSeq *default_meta_multicast;

    /*ci
     * \brief A bind-resolver to find NETIO interface to listen for data on
     */
    NETIO_BindResolver_T *bind_resolver;

    /*ci
     * \brief The transports that are enabled for this subscriber
     */
    struct REDA_StringSeq *enabled_transports;

    /*ci
     * \brief A route resolver to find NETIO interface to route to peers
     */
    NETIO_RouteResolver_T *route_resolver;

    /*ci
     * \brief A route resolver to find NETIO interface to route to peers
     */
    NETIO_AddressResolver_T *address_resolver;

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
     *\brief Configuration data for all datareaders created by this subscriber
     */
    struct NDDS_DataReaderConfig *dr_config;

    /*ci
     */
    DataReaderLifeCycleListener_on_before_datareader_created
                                            on_before_datareader_created;

    /*ci \brief The external interface created by the parent Participant
     */
    NETIO_Interface_T *ext_rtps_intf;

    /*ci \brief Shared packet pool
     */
    REDA_BufferPool_T packet_pool;

    struct DDS_LocatorSeq *resolved_unicast_seq;

    struct DDS_LocatorSeq *resolved_multicast_seq;

#if DDS_FILTERING_ENABLED
    struct DDS_FilterPlugin *filter_plugin;
#endif /* DDS_FILTERING_ENABLED */
};

#if DDS_FILTERING_ENABLED
#define NDDS_SubscriberConfig_Filter_INITIALIZER \
    ,\
    NULL
#else
#define NDDS_SubscriberConfig_Filter_INITIALIZER
#endif

/*ci
 * \def NDDS_SubscriberConfig_INITIALIZER
 * \brief Constant to initialize NDDS_SubscriberConfig
 */
#define NDDS_SubscriberConfig_INITIALIZER \
{ \
    NULL /* db */,\
    NULL /* timer */,\
    NULL /* registry */,\
    NULL /* local_reader_table */,\
    NULL /* on_after_datareader_enabled */,\
    NULL /* on_before_datareader_deleted */,\
    NULL /* get_parent_handle */,\
    NULL /* object_id_generator */,\
    NULL /* on_requested_deadline_missed */,\
    NULL /* on_requested_incompatible_qos */,\
    NULL /* on_sample_rejected */,\
    NULL /* on_liveliness_changed */,\
    NULL /* on_data_available */,\
    NULL /* on_data_on_readers */,\
    NULL /* on_subscription_matched */,\
    NULL /* on_sample_lost */,\
    NULL /* on_instance_replaced */,\
    NULL /* default_unicast */,\
    NULL /* default_multicast */,\
    NULL /* default_meta_unicast */,\
    NULL /* default_meta_multicast */,\
    NULL /* bind_resolver */,\
    NULL /* enabled_transports */,\
    NULL /* route_resolver */,\
    NULL /* address_resolver */,\
    NULL /* participant_id */,\
    NULL /* domain_id */,\
    NULL /* dr_config */,\
    NULL /* on_before_datareader_created */,\
    NULL /* ext_rtps_intf */,\
    NULL /* netio_packetpool*/,\
    NULL,\
    NULL\
    NDDS_SubscriberConfig_Filter_INITIALIZER\
}

/*ci
 * \brief  Implementation of the DDS Subscriber entity
 */
struct DDS_SubscriberImpl
{
    /*ci
     * \brief Inherit from DDS_Entity
     */
    struct DDS_EntityImpl as_entity;

    /*----- From create call -----*/

    /*ci \dref_SubscriberQos_entity_factory
     */
    struct DDS_EntityFactoryQosPolicy entity_factory;

    /*ci \dref_SubscriberQos_partition
     */
    struct DDS_PartitionQosPolicy *partition;

    /*ci \dref_SubscriberQos_management
     */
    struct RTI_ManagementQosPolicy management;

    /*ci \dref_SubscriberQos_group_data
     */
    struct DDS_GroupDataQosPolicy *group_data;

    /*ci \dref_SubscriberQos_subscriber_name
     */
    char *subscriber_name;

    /*ci
     * \brief The current Subscriber listener, as specified by the user
     */
    struct DDS_SubscriberListener listener;

    /*ci
     * \brief The currently status' the Subscriber is interested in
     *        as specified by the user
     */
    DDS_StatusMask mask;

    /*ci
     * \brief Pointer to participant that created this subscriber
     */
    DDS_DomainParticipant *participant;

    /*----- Support for API -----*/
#if INCLUDE_API_QOS
    /*ci
     * \brief The default datareader qos if the user creates a datareader
     *        with the the default qos short-cut.
     */
    struct DDS_DataReaderQos *default_qos;
#endif

    /*----- Internal management -----*/
    /*ci
     * \brief The current number of datareaders in this subscriber
     */
    DDS_Long dr_count;

    /*----- Properties -----*/
    /*ci
      * \brief Copy of the configuration data received during initialization
      */
    struct NDDS_SubscriberConfig *config;
};

MUST_CHECK_RETURN extern DDS_Boolean
DDS_SubscriberListener_is_consistent(const struct DDS_SubscriberListener *l, DDS_StatusMask m);

MUST_CHECK_RETURN extern RTI_INT32
DDS_SubscriberImpl_compare(RTI_INT32 flags,const DB_Record_T op1, void *op2);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_SubscriberImpl_is_hidden(struct DDS_SubscriberImpl * self);

#ifndef RTI_CERT
SHOULD_CHECK_RETURN extern DDS_Boolean
DDS_SubscriberImpl_finalize(struct DDS_SubscriberImpl * self);
#endif

MUST_CHECK_RETURN extern DDS_InstanceHandle_t
DDS_SubscriberImpl_get_instance_handle(DDS_Entity *entity);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_SubscriberImpl_initialize(struct DDS_SubscriberImpl *subscriber,
                              DDS_DomainParticipant *participant,
                              const struct DDS_SubscriberQos *qos,
                              const struct DDS_SubscriberListener *listener,
                              DDS_StatusMask mask,
                              DDS_UnsignedLong object_id,
                              struct NDDS_SubscriberConfig *config);

extern DDS_ReturnCode_t
DDS_Subscriber_get_qos_from(const DDS_Subscriber *self,
                            struct DDS_SubscriberQos *out);

MUST_CHECK_RETURN extern DDS_ReturnCode_t
DDS_Subscriber_set_qos_from(
        DDS_Subscriber *out,
        const struct DDS_SubscriberQos *in,
        DDS_DomainParticipant *participant);

MUST_CHECK_RETURN extern DDS_ReturnCode_t
DDS_Subscriber_delete_contained_entities_no_lock(DDS_Subscriber *self);

MUST_CHECK_RETURN extern DDS_ReturnCode_t
DDS_Subscriber_delete_datareader_no_lock(DDS_Subscriber *self,
                                         DDS_DataReader *datareader);

#endif /* SubscriberImpl_pkg_h */

/*ci @} */
