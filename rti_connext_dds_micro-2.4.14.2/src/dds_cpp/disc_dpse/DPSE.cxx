/* 
 (c) Copyright, Real-Time Innovations, 2013-2016.
 All rights reserved.

 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.

modification history
-------------------- 
07apr2016,tk  MICRO-1539 Renamed DPSE API to avoid name collision with assert
22dec2015,as  Created
===================================================================== */

#ifndef dds_cpp_dpse_hxx
#include "dds_cpp/dds_cpp_dpse.hxx"
#endif

/*** SOURCE_BEGIN ***/

struct RT_ComponentFactoryI*
DPSEDiscoveryFactory::get_interface()
{
    return DPSE_DiscoveryFactory_get_interface();
}

DDS_ReturnCode_t
DPSEDiscoveryPlugin::RemoteParticipant_assert(DDSDomainParticipant *const participant,
                                const char *rem_participant_name)
{
    return DPSE_RemoteParticipant_assert(
            (DDS_DomainParticipant*)participant->get_c_entity(),
            rem_participant_name);
}

DDS_ReturnCode_t
DPSEDiscoveryPlugin::RemotePublication_assert(DDSDomainParticipant * const participant,
                      const char *const rem_participant_name,
                      const struct DDS_PublicationBuiltinTopicData *const data,
                      NDDS_TypePluginKeyKind key_kind)
{
    return DPSE_RemotePublication_assert(
                (DDS_DomainParticipant*)participant->get_c_entity(),
                rem_participant_name, data, key_kind);
}

DDS_ReturnCode_t
DPSEDiscoveryPlugin::RemoteSubscription_assert(DDSDomainParticipant * const participant,
                   const char *const rem_participant_name,
                   const struct DDS_SubscriptionBuiltinTopicData *const data,
                   NDDS_TypePluginKeyKind key_kind)
{
    return DPSE_RemoteSubscription_assert(
                (DDS_DomainParticipant*)participant->get_c_entity(),
                rem_participant_name, data, key_kind);
}
