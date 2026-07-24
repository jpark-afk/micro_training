/* 

 (c) Copyright, Real-Time Innovations, 2006-2016.  All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.
*/
/*
modification history
--------------------
07apr2016,tk  MICRO-1539 Renamed DPSE API to avoid name collision with assert
22dec2015,as  Created
=========================================================================*/

#ifndef dds_cpp_dpse_hxx
#define dds_cpp_dpse_hxx

#ifndef dds_cpp_dll_hxx
#include "dds_cpp/dds_cpp_dll.hxx"
#endif
#ifndef dds_cpp_infrastructure_hxx
#include "dds_cpp/dds_cpp_infrastructure.hxx"
#endif
#ifndef dds_cpp_domain_hxx
#include "dds_cpp/dds_cpp_domain.hxx"
#endif
#ifndef dds_cpp_subscription_hxx
#include "dds_cpp/dds_cpp_subscription.hxx"
#endif
#ifndef rt_rt_h
#include "rt/rt_rt.h"
#endif
#ifndef disc_dpse_dpsediscovery_h
#include "disc_dpse/disc_dpse_dpsediscovery.h"
#endif

/*e \ingroup DPSEModule
 */


/*e \dref_DPSE_Plugin_Factory
 */
class DDSCPPDllExport DPSEDiscoveryFactory
{
public:
    /*e
     * \dref_DPSE_Plugin_Factory_get_interface
     */
    static struct RT_ComponentFactoryI* get_interface();
};

/*e \dref_DPSEDiscoveryPlugin
 */
class DDSCPPDllExport DPSEDiscoveryPlugin
{
public:
    /*e
     * \dref_DPSEDiscoveryPlugin_RemoteParticipant_assert
     */
    static DDS_ReturnCode_t
    RemoteParticipant_assert(DDSDomainParticipant *const participant,
                                           const char *rem_participant_name);

    /*e
     * \dref_DPSEDiscoveryPlugin_RemotePublication_assert
     */
    static DDS_ReturnCode_t
    RemotePublication_assert(DDSDomainParticipant * const participant,
                      const char *const rem_participant_name,
                      const struct DDS_PublicationBuiltinTopicData *const data,
                      NDDS_TypePluginKeyKind key_kind);

    /*e
     * \dref_DPSEDiscoveryPlugin_RemoteSubscription_assert
     */
    static DDS_ReturnCode_t
    RemoteSubscription_assert(DDSDomainParticipant * const participant,
                   const char *const rem_participant_name,
                   const struct DDS_SubscriptionBuiltinTopicData *const data,
                   NDDS_TypePluginKeyKind key_kind);
};

#endif /* dds_cpp_dpse_hxx */
