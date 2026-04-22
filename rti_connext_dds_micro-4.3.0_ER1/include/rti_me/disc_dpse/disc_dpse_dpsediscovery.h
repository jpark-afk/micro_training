/*
 * FILE: disc_dpse_discovery.h - DPSE API
 *
 * (c) Copyright 2011-2015 Real-Time Innovations,
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
 * 09sep2008,tk  Refactored
 * 08aug2011,yy  Copied the loopback string so that it could be freed later
 * 04aug2011,yy  Fixed 14066
 * 09sep2008,rmw Created
 */
/*ce
 * \file
 * \brief DPSE API Dynamic Participant/Static Endpoint Discovery Plugin.
 * \details
 * The Plugin APIs are used to configure and instantiate the RTI
 * Connext Micro Dynamic Participant/Static Endpoint
 * Discovery Plugin.
 */
#ifndef disc_dpse_dpsediscovery_h
#define disc_dpse_dpsediscovery_h

#ifndef disc_dpse_dll_h
#include "disc_dpse/disc_dpse_dll.h"
#endif
#ifndef dds_c_discovery_plugin_h
#include "dds_c/dds_c_discovery_plugin.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

struct DPSE_DiscoveryPluginProperty;

/*ce \dref_DPSEDiscoveryPlugin_DiscoveryPluginProperty_initialize
*/
DISC_DPSEDllExport DDS_ReturnCode_t
DPSE_DiscoveryPluginProperty_initialize(
            struct DPSE_DiscoveryPluginProperty *dst);

#ifdef __cplusplus
}
#endif

/*e \dref_DPSE_DiscoveryPluginProperty
*/
struct DDSCPPDllExport DPSE_DiscoveryPluginProperty
{
    /*ci
     * \brief Inherit from base-class
     */
    struct RT_ComponentFactoryProperty _parent;

    /*e \dref_DPSE_DiscoveryPluginProperty_participant_liveliness_assert_period
     */
    struct DDS_Duration_t participant_liveliness_assert_period;

    /*e \dref_DPSE_DiscoveryPluginProperty_participant_liveliness_lease_duration
     */
    struct DDS_Duration_t participant_liveliness_lease_duration;

    /*e \dref_DPSE_DiscoveryPluginProperty_initial_participant_announcements
     */
    DDS_Long initial_participant_announcements;

    /*e \dref_DPSE_DiscoveryPluginProperty_initial_participant_announcement_period
     */
    struct DDS_Duration_t initial_participant_announcement_period;

    /*e \dref_DPSE_DiscoveryPluginProperty_max_participant_locators;
     */
    DDS_Long max_participant_locators;

    /*e \dref_DPSE_DiscoveryPluginProperty_max_locators_per_discovered_participant;
     */
    DDS_Long max_locators_per_discovered_participant;

#if DDS_LIVELINESS_CHANNEL_ENABLED
    /*e \dref_DPSE_DiscoveryPluginProperty_participant_message_reader_reliability_kind
     */
    DDS_ReliabilityQosPolicyKind participant_message_reader_reliability_kind;
#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */

#ifdef RTI_CPP
    public:
        DPSE_DiscoveryPluginProperty()
        {
            DPSE_DiscoveryPluginProperty_initialize(this);
        }
#endif
};

#ifdef __cplusplus
extern "C"
{
#endif

#define RTI_DPSEDISCOVERY_INTERFACE_ID \
  RT_MKINTERFACEID(RT_COMPONENT_CLASS_DISCOVERY,\
                   RT_COMPONENT_INSTANCE_DISCOVERY_DPSE)

/*e \dref_DPSE_DiscoveryPluginProperty_INITIALIZER
 */
#define DPSE_DiscoveryPluginProperty_INITIALIZER \
{ \
    RT_ComponentFactoryProperty_INITIALIZER,\
    {30,0},  \
    {100,0}, \
    5,       \
    {1,0},   \
    DDS_LENGTH_AUTO,\
    4 \
    DDS_PARTICIPANT_MESSAGE_READER_RELIABILITY_KIND_INITIALIZER \
}

/*ce \dref_DPSEDiscoveryPlugin_RemoteParticipant_assert
 */
MUST_CHECK_RETURN DISC_DPSEDllExport DDS_ReturnCode_t
DPSE_RemoteParticipant_assert(DDS_DomainParticipant *const participant,
                              const char *rem_participant_name);

/*ce \dref_DPSEDiscoveryPlugin_RemotePublication_assert
 */
MUST_CHECK_RETURN DISC_DPSEDllExport DDS_ReturnCode_t
DPSE_RemotePublication_assert(DDS_DomainParticipant * const participant,
                      const char *const rem_participant_name,
                      const struct DDS_PublicationBuiltinTopicData *const data,
                      NDDS_TypePluginKeyKind key_kind);

/*ce \dref_DPSEDiscoveryPlugin_RemoteSubscription_assert
 */
MUST_CHECK_RETURN DISC_DPSEDllExport DDS_ReturnCode_t
DPSE_RemoteSubscription_assert(DDS_DomainParticipant * const participant,
                   const char *const rem_participant_name,
                   const struct DDS_SubscriptionBuiltinTopicData *const data,
                   NDDS_TypePluginKeyKind key_kind);

/* -------------------------------------------------------------------------- */
/* Plugin factory
 */
/*ce \dref_DPSE_Plugin_Factory_get_interface
 */
MUST_CHECK_RETURN DISC_DPSEDllExport struct RT_ComponentFactoryI*
DPSE_DiscoveryFactory_get_interface(void);

/*ce \dref_DPSE_Plugin_Factory_get_version
 */
#ifndef RTI_CERT
DISC_DPSEDllExport const char*
DPSE_DiscoveryFactory_get_version(void);
#endif

MUST_CHECK_RETURN DISC_DPSEDllExport struct DDS_TypePlugin*
DPSE_ParticipantBuiltinTopicDataTypePlugin_create(
                                    DDS_DomainParticipant *participant,
                                    struct DDS_DomainParticipantQos *dp_qos,
                                    DDS_TypePluginMode_T endpoint_mode,
                                    DDS_TypePluginEndpoint *endpoint,
                                    DDS_TypePluginEndpointQos *qos,
                                    struct DDS_TypePluginProperty *property);

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif /* disc_dpse_dpsediscovery_h */
