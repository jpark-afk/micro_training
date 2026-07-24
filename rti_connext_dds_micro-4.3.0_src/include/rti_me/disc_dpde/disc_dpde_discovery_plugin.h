/*

 (c) Copyright 2008-2015 Real-Time Innovations
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.

modification history
------------ -------
1.0b,09sep12,tk Updated
1.0a,09sep08,tk Created
=========================================================================*/

#ifndef disc_dpde_discovery_plugin_h
#define disc_dpde_discovery_plugin_h


#ifndef disc_dpde_dll_h
#include "disc_dpde/disc_dpde_dll.h"
#endif
#ifndef dds_c_discovery_h
#include "dds_c/dds_c_discovery.h"
#endif
#ifndef dds_c_discovery_plugin_h
#include "dds_c/dds_c_discovery_plugin.h"
#endif

struct DPDE_DiscoveryPlugin;
typedef struct DPDE_DiscoveryPlugin DPDE_DiscoveryPlugin_T;

/*i \file
    \brief Dynamic Participant/Static Endpoint Discovery Plugin.
*/

/*i \ingroup DPDEModule
  \brief Dynamic Participant Dynamic Endpoint Discovery Plugin

  The Plugin APIs are used to configure and instantiate the RTI
  Connext Micro Dynamic Participant Dynamic Endpoint Discovery Plugin.
*/

/*i \ingroup DPDEModule

  \brief Properties structure to tune the workings of the DPDE Discovery Plugin.
*/

#ifdef __cplusplus
extern "C"
{
#endif

struct DPDE_DiscoveryPluginProperty;

/*ce \dref_DPDE_DiscoveryPluginProperty_initialize
*/
DISC_DPDEDllExport DDS_ReturnCode_t
DPDE_DiscoveryPluginProperty_initialize(
            struct DPDE_DiscoveryPluginProperty* self);

#ifdef __cplusplus
}
#endif

/*e \dref_DPDE_DiscoveryPluginProperty
*/
struct DDSCPPDllExport DPDE_DiscoveryPluginProperty
{
    struct RT_ComponentFactoryProperty _parent;

    /*e \dref_DPDE_DiscoveryPluginProperty_participant_liveliness_assert_period
     */
    struct DDS_Duration_t participant_liveliness_assert_period;

    /*e \dref_DPDE_DiscoveryPluginProperty_participant_liveliness_lease_duration
     */
    struct DDS_Duration_t participant_liveliness_lease_duration;

    /*e \dref_DPDE_DiscoveryPluginProperty_initial_participant_announcements
     */
    DDS_Long initial_participant_announcements;

    /*e \dref_DPDE_DiscoveryPluginProperty_initial_participant_announcement_period
     */
    struct DDS_Duration_t initial_participant_announcement_period;

    /*e \dref_DPDE_DiscoveryPluginProperty_cache_serialized_samples
     */
    DDS_Boolean cache_serialized_samples;

    /*e \dref_DPDE_DiscoveryPluginProperty_max_participant_locators
     */
    DDS_Long max_participant_locators;

    /*e \dref_DPDE_DiscoveryPluginProperty_max_locators_per_discovered_participant
     */
    DDS_Long max_locators_per_discovered_participant;

    /*e \dref_DPDE_DiscoveryPluginProperty_max_samples_per_builtin_endpoint_reader
     */
    DDS_Long max_samples_per_builtin_endpoint_reader;

    /*e \dref_DPDE_DiscoveryPluginProperty_max_samples_per_remote_builtin_endpoint_writer
     */
    DDS_Long max_samples_per_remote_builtin_endpoint_writer;

    /*e \dref_DPDE_DiscoveryPluginProperty_builtin_writer_max_heartbeat_retries
     */
    DDS_Long builtin_writer_max_heartbeat_retries;

    /*e \dref_DPDE_DiscoveryPluginProperty_builtin_writer_heartbeat_period
     */
    struct DDS_Duration_t builtin_writer_heartbeat_period;

    /*e \dref_DPDE_DiscoveryPluginProperty_builtin_writer_heartbeats_per_max_samples
     */
    DDS_Long builtin_writer_heartbeats_per_max_samples;

    /*e \dref_DPDE_DiscoveryPluginProperty_builtin_endpoint_reader_nack_period
     */
    struct DDS_Duration_t builtin_endpoint_reader_nack_period;

#if DDS_LIVELINESS_CHANNEL_ENABLED
    /*e \dref_DPDE_DiscoveryPluginProperty_participant_message_reader_reliability_kind
     */
    DDS_ReliabilityQosPolicyKind participant_message_reader_reliability_kind;
#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */

#ifdef RTI_CPP
    public:
        DPDE_DiscoveryPluginProperty()
        {
            DPDE_DiscoveryPluginProperty_initialize(this);
        }
#endif
};

#ifdef __cplusplus
extern "C"
{
#endif

#define DPDE_DISCOVERY_INTERFACE_ID \
  RT_MKINTERFACEID(RT_COMPONENT_CLASS_DISCOVERY,\
                   RT_COMPONENT_INSTANCE_DISCOVERY_DPDE)

/*e \dref_DPDE_DiscoveryPluginProperty_INITIALIZER
 */
#define DPDE_DiscoveryPluginProperty_INITIALIZER \
{\
    RT_ComponentFactoryProperty_INITIALIZER,\
    {30,0},  \
    {100,0}, \
    5,       \
    {1,0},   \
    DDS_BOOLEAN_FALSE, \
    DDS_LENGTH_AUTO,\
    4,\
    8,\
    DDS_MAX_UNLIMITED,\
    DDS_LENGTH_UNLIMITED,\
    {0,100000000},\
    DDS_LENGTH_UNLIMITED,\
    DDS_RTPS_NACK_PERIOD_DEFAULT \
    DDS_PARTICIPANT_MESSAGE_READER_RELIABILITY_KIND_INITIALIZER \
}

/* -------------------------------------------------------------------------- */
/* Plugin factory
 */
/*ce \dref_DPDE_DiscoveryFactory_get_interface
 */
MUST_CHECK_RETURN DISC_DPDEDllExport struct RT_ComponentFactoryI*
DPDE_DiscoveryFactory_get_interface(void);

/*ce \dref_DPDE_DiscoveryFactory_get_version
 */
#ifndef RTI_CERT
DISC_DPDEDllExport const char*
DPDE_DiscoveryFactory_get_version(void);
#endif

#ifdef __cplusplus
}                               /* extern "C" */
#endif


#endif /* disc_dpde_discovery_plugin_h */
