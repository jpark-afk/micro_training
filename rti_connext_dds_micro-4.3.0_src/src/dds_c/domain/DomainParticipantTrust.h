/*
 * FILE: DomainParticipantTrust.h - Trust support for DomainParticipant
 *
 * (c) Copyright, Real-Time Innovations, 2008-2025.
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
 * 13dec2017,as created
 */
/*ce
 * \file
 * \brief Trust support for DomainParticipant
 */
/*ci
 * \addtogroup DDSDomainModule
 * @{
 */
#ifndef DomainParticipantTrust_h
#define DomainParticipantTrust_h

#include "dds_c/dds_c_trust.h"

#define DDS_Trust_ParticipantTrustConfig_Null ((void*)0)

struct DDS_Trust_ParticipantTrustConfig
{
    /*ci
     * \brief Trust plugin.
     */
    RTPS_TrustPlugin *plugin;

    /*ci
     * \brief An opaque handle to the trust state by the Trust Plugin
     */
    RTPS_TrustPluginState state;
};

#define DDS_Trust_ParticipantTrustConfig_INITIALIZER \
{\
    NULL,/* psk plugin */\
    RTPS_TrustPluginState_INITIALIZER    /* state */\
}

extern RTI_BOOL
DDS_DomainParticipant_validate_local_participant_trust(
    struct DDS_Trust_ParticipantTrustConfig *config,
    DDS_DomainId_t domain_id,
    const struct DDS_DomainParticipantQos *qos,
    DDS_GUID_t *candidate_guid_inout);

extern RTI_BOOL
DDS_DomainParticipantFactory_assert_trust_config(
    RT_Registry_T *registry,
    const struct DDS_DomainParticipantQos *qos,
    struct DDS_Trust_ParticipantTrustConfig **config);

#ifndef RTI_CERT
extern RTI_BOOL
DDS_DomainParticipantFactory_delete_trust_config(
    RT_Registry_T *registry,
    const struct DDS_DomainParticipantQos *qos,
    struct DDS_Trust_ParticipantTrustConfig *config);
#endif

extern RTI_UINT32
DDS_DomainParticipant_get_max_serialized_trust_param_size(const DDS_DomainParticipant *self, RTI_UINT32 size);

extern RTI_BOOL
DDS_DomainParticipant_serialize_trust_param(const DDS_DomainParticipant *self, struct CDR_Stream_t *stream);

extern void
DDS_Trust_DomainParticipant_invalidate_local_participant_trust(
    struct DDS_Trust_ParticipantTrustConfig *config);

extern RTI_BOOL
DDS_DomainParticipant_initialize_rtps_trust_property(
    struct DDS_DomainParticipantImpl *self,
    struct RTPS_InterfaceProperty  *rtps_property,
    struct RT_ComponentFactory *rtps_factory);

extern RTI_BOOL
DDS_DomainParticipant_set_trust_property(
        struct DDS_DomainParticipantImpl *self,
        const struct DDS_DomainParticipantQos *qos);

extern RTI_BOOL
DDS_DomainParticipant_register_matched_remote_participant(
                    struct DDS_Trust_ParticipantTrustConfig *config,
                    struct DDS_BuiltinTopicKey_t *key);

extern RTI_BOOL
DDS_DomainParticipant_unregister_matched_remote_participant(
                struct DDS_Trust_ParticipantTrustConfig *config,
                const struct DDS_BuiltinTopicKey_t *key);

#endif

/*ci @} */


