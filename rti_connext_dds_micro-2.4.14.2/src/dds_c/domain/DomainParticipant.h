/*
 * FILE: DomainParticipant.h - DomainParticipant implementation
 *
 * (c) Copyright, Real-Time Innovations, 2008-2020.
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
 * 31may2022,jh MICRO-3381/PR.30046
 * - Conditionally included definition of shared_lock when
 *   compiling without RTI_CERT
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 * 20feb2015,eh MICRO-813/PR#9172 Fix Lint warnings
 * 07mar2014,tk  MICRO-735: Send properties for tools
 * 08nov2013,as  MICRO-681 Complete implementation of WaitSets
 *               and support for StatusConditions
 * 27jun2012,tk  Major update
 * 25aug2011,yy  Set interpreter storage to NULL upon deletion
 * 30apr2008,tk  Written
 */
/*ci
 * \file
 */
/*ci \addtogroup DDSDomainModule
 * @{
 */
#ifndef DomainParticipant_h
#define DomainParticipant_h

#ifndef RTI_CERT
#include "dds_c/dds_c_string_manager.h"
#endif

#include "DataReaderImpl.h"
#include "SubscriberImpl.h"
#include "DataWriterImpl.h"
#include "PublisherImpl.h"
#include "TopicDescription.h"
#include "Topic.h"


/*ci
 * \brief DDS DomainParticipant configuration data
 *
 * \details
 *
 * When a domain participant (participant) is created by the factory. the
 * factory passes in this configuration data for the participant.
 */
struct NDDS_ParticipantConfig
{
    /*ci
     * \brief The registry the participant shall use to look up optional modules
     */
    RT_Registry_T *registry;

    /*ci
     * \brief The GUID the for this participant assigned by the factory
     */
    struct DDS_GUID_t guid;

    /*ci
     * \brief Unique id within the factory for the participant
     */
    DDS_Long instance;
};

/*ci
 * \brief Constant to initialize NDDS_ParticipantConfig
 */
#define NDDS_ParticipantConfig_INITIALIZER \
{ \
    NULL,\
    DDS_GUID_INITIALIZER,\
    0 \
}

/*ci
 * \brief The maximum number of properties which an be assigned to the
 *        participant.
 *
 * \details
 *
 * Properties are general name=value pairs. RTI uses pre-defined properties
 * to pass information to tools, but the properties are not assignable
 * by the user.
 */
#define DDS_DOMAINPARTICIPANT_MAX_PROPERTIES 4

/*ci
 * \brief The DDS DomainParticipant implementation
 *
 * \details
 *
 * This structure holds all state information for a single participant. The
 * memory for this structure is allocated by the factory and passed to the
 * initializer.
 */
struct DDS_DomainParticipantImpl
{
    /*ci
     * \brief A participant is a DDS entity, inherit from base-class
     */
    struct DDS_EntityImpl as_entity;

    /*ci
     * \brief The DDS qos policy the participant was created with
     */
    struct DDS_DomainParticipantQos qos;

    /*ci
     * \brief The DDS listener the participant was created with
     */
    struct DDS_DomainParticipantListener listener;

    /*ci
     * \brief The status mask the participant was created with
     */
    DDS_StatusMask mask;

    /*ci
     * \brief The domain id the participant was created in
     */
    DDS_DomainId_t domain_id;

    /*ci
     * \brief The current default DDS publisher Qos policy used when
     *        creating a DDS publisher using the default qos option
     */
    struct DDS_PublisherQos _default_publisher_qos;

    /*ci
     * \brief The current default DDS subscriber Qos policy used when
     *        creating a DDS subscriber using the default qos option
     */
    struct DDS_SubscriberQos _default_subscriber_qos;

    /*ci
     * \brief The current default DDS topic Qos policy used when
     *        creating a DDS topic using the default qos option
     */
    struct DDS_TopicQos _default_topic_qos;

    /*ci
     * \brief The database used to store entity's discovery information. It
     *        is also passed to optional modules and components
     */
    DB_Database_T database;

    /*ci
     * \brief Database table of locally created DDS publishers
     */
    DB_Table_T local_publisher_table;

    /*ci
     * \brief Database table of locally created DDS subscribers
     */
    DB_Table_T local_subscriber_table;

    /*ci
     * \brief Database table of locally created DDS datareaders
     */
    DB_Table_T local_reader_table;

    /*ci
     * \brief Database table of locally created DDS datawriters
     */
    DB_Table_T local_writer_table;

    /*ci
     * \brief Database table of discovered participants
     */
    DB_Table_T remote_participant_table;

    /*ci
     * \brief Database table of discovered publications
     */
    DB_Table_T remote_publisher_table;

    /*ci
     * \brief Database table of discovered subscriptions
     */
    DB_Table_T remote_subscriber_table;

    /*ci
     * \brief Database table of locally created topics
     */
    DB_Table_T topic_table;

    /*ci
     * \brief Database table of locally registered types
     */
    DB_Table_T type_table;

#ifndef RTI_CERT
    DDS_StringManager_T *string_manager;
#endif

    /*ci
     * \brief Pointer to a discovery plugin if one is specified, NULL is allowed
     */
    struct NDDS_Discovery_Plugin *disc_plugin;

#ifndef RTI_CERT
    /*ci
     * \brief Mutex to use for calls which must be thread-safe at the
     *        participant level
     */
    struct OSAPI_Mutex *shared_lock;
#endif

    /*ci
     * \brief Mutex shared with a database to provide thread-safety for
     *        accessing entities within a participant.
     */
    struct OSAPI_Mutex *db_lock;

    /*ci
     * \brief OSAPI timer shared between all entities within the participant.
     */
    OSAPI_Timer_T timer;

    /*ci
     * \brief The participants object_id.
     */
    RTI_UINT32 object_id;

    /*ci
     * \brief The participant_id assigned to the domain participant.
     */
    DDS_Long participant_id;
    
    /*ci
     *  \brief Current object-id. Participant tracks the last object id generated 
     * by the participant and is incrememtned as new enitities show up
     */
    RTI_UINT32 current_object_id;

    /*ci
     * \brief The participant's topic-data published for participant discovery.
     */
    struct DDS_ParticipantBuiltinTopicData builtin_data;

    /*ci
     * \brief bind resolver to keep track what the participant can listen to.
     */
    NETIO_BindResolver_T *bind_resolver;

    /*ci
     * \brief route resolver to keep track what the participant can reach.
     */
    NETIO_RouteResolver_T *route_resolver;

    /*ci
     * \brief address resolver to convert string addresses to NETIO_Address
     *        structures
     */
    NETIO_AddressResolver_T *address_resolver;

    /*ci
     * \brief The RTPS interface uses one interface to parse received
     *        messages to determine which local RTPS session it is for. Each
     *        participant has one such external RTPS interface.
     */
    NETIO_Interface_T *rtps_intf;

    /*ci
     * \brief Copy of the configuration data passed in from the participant
     *       factory.
     */
    struct NDDS_ParticipantConfig config;

    /*ci
     * \brief Configuration data passed to the address resolver to resolve
     *        NETIO ports.
     */
    struct NETIO_RtpsPortData rtps_port_data;

    /*ci
     * \brief DDS properties sent as part of discovery. These properties are
     *        not configurable by the user and are hard-coded during
     *        initialization.
     */
    char hostname_property[OSAPI_SYSTEM_MAX_HOSTNAME];
    char process_property[OSAPI_SYSTEM_MAX_HOSTNAME];
    char version_property[OSAPI_SYSTEM_MAX_HOSTNAME];

    /*ci
     * \brief Array for the hard-coded properties. These are loaned to the
     *        property sequence below.
     */
    struct DDS_Property dds_sys_property_values[DDS_DOMAINPARTICIPANT_MAX_PROPERTIES];

    /*ci
     * \brief The DDS property sequence with values sent as part of the DDS
     *        discovery.
     */
    struct DDS_PropertySeq dds_sys_property;

    /*ci
     *\brief Configuration data for all publishers created by this participant
     */
    struct NDDS_PublisherConfig pub_config;

    /*ci
     *\brief Configuration data for all subscribers created by this participant
     */
    struct NDDS_SubscriberConfig sub_config;

    /*ci
     *\brief Configuration data for all topics created by this participant
     */
    struct NDDS_TopicConfig topic_config;

    /*ci
     *\brief Configuration data for all datawriters created by this participant
     */
    struct NDDS_DataWriterConfig dw_config;

    /*ci
     *\brief Configuration data for all datareaders created by this participant
     */
    struct NDDS_DataReaderConfig dr_config;

    /*ci
     * \brief Sequence to hold routes per enabled transports
     */
    struct NETIO_AddressSeq routes;

    /*ci
     * \brief Sequence to hold netmask for per enabled transports
     */
    struct NETIO_NetmaskSeq netmasks;
};

#ifndef RTI_CERT
MUST_CHECK_RETURN extern DDS_ReturnCode_t
DDS_DomainParticipant_finalize(DDS_DomainParticipant *self);
#endif

MUST_CHECK_RETURN extern DDS_DomainParticipant*
DDS_DomainParticipant_initialize(
                          struct DDS_DomainParticipantImpl *participant,
                          DDS_DomainParticipantFactory *factory,
                          DDS_DomainId_t domain_id,
                          const struct DDS_DomainParticipantQos *qos,
                          const struct DDS_DomainParticipantListener *listener,
                          DDS_StatusMask mask,
                          struct NDDS_ParticipantConfig *config,
                          struct OSAPI_Mutex *factory_lock);

extern RTI_UINT32
DDS_DomainParticipant_get_next_objectid(void *self);

#endif

/*ci @} */

