/*
 * FILE: DomainParticipantTrust.c - Trust-related DomainParticipant operations
 *
 * (c) Copyright, Real-Time Innovations, 2017-2025.
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
/*ci
 * \file
 * \brief Trust support for DomainParticipant
 *
 * \details
 * This file contains functions implementing behavior related to trust plugins
 * in a DomainParticipant.
 */
/*ci
 * \addtogroup DDSDomainModule
 * @{
 */

#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif

#ifndef dds_c_trust_h
#include "dds_c/dds_c_trust.h"
#endif
#ifndef rtps_trust_plugin_h
#include "rtps/rtps_trust_plugin.h"
#endif

#include "dds_c/dds_c_domain_trust.h"

#include "DomainFactory.h"
#include "DomainParticipant.h"
#include "DomainParticipantTrust.h"
#include "RemoteParticipant.h"
#include "RemotePublication.h"
#include "RemoteSubscription.h"
#include "BuiltinCdr.h"

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Check whether a DomainParticipant is configured to use the
 * trust plugins.
 *
 * If a DomainParticipant has trust plugins passed to it during initialization,
 * it will interact with them to enforce any configured trust protocol.
 *
 * Even if the DomainParticipant's trust attributes do not include access
 * control, the DomainParticipant will still be required to invoke several of
 * the trust plugin interface operations, e.g. to validate its identity.
 *
 * If no trust plugins have been installed, the DomainParticipant will instead
 * behave like any normal, "untrusted" DDS DomainParticipant.
 *
 * \param[in]  self    The DomainParticipant to check.
 *
 * \return RTI_TRUE if the DomainParticipant has trust plugins installed and
 * follows the DDS trust protocols, RTI_FALSE otherwise.
 */

#define DDS_DomainParticipant_is_trust_enabled(self) \
    (((self) != NULL) && \
     ((self)->config.trust != (DDS_Trust_ParticipantTrustConfig_Null)))

#define DDS_DomainParticipant_trust_state(self) \
    ((self)->config.trust->state)


/*ci
 * \brief Check whether a DomainParticipant created with the specified QoS will
 * enforce the DDS trust protocols.
 *
 * This is true if the QoS specifies the name of Trust Plugin Suite
 *
 * \param[in]  qos    The DomainParticipantQos to check.
 *
 * \return RTI_TRUE if the DomainParticipantQos references any trust plugin,
 * RTI_FALSE otherwise.
 */
RTI_BOOL
DDS_DomainParticipantQos_is_trust_enabled(
            const struct DDS_DomainParticipantQos *qos)
{
    OSAPI_PRECONDITION(qos == NULL,
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("qos",qos,RTI_TRUE););

    return !(RT_ComponentFactoryId_is_nil(&qos->trust.suite));
}

/*ci
 * \brief Convert a DDS_BuiltinTopicKey_t to DDS_GUID_t
 *
 * This function converts a built-in topic key to a DDS GUID.
 *
 * \param[in]  key         The built-in topic key.
 * \param[out] remote_guid The resulting DDS GUID.
 */
RTI_PRIVATE void
DDS_DomainParticipant_builtin_key_to_dds_guid(const struct DDS_BuiltinTopicKey_t *key, DDS_GUID_t *remote_guid)
{
    struct RTPS_Guid rtps_guid = RTPS_GUID_UNKNOWN;

     /* key -> rtps_guid -> dds_guid */
    rtps_guid.prefix.host_id = key->value[0];
    rtps_guid.prefix.app_id = key->value[1];
    rtps_guid.prefix.instance_id = key->value[2];
    rtps_guid.object_id = key->value[3];

    DDS_GUID_from_rtps(remote_guid, &rtps_guid);
}



RTI_BOOL
DDS_DomainParticipant_register_matched_remote_participant(struct DDS_Trust_ParticipantTrustConfig *config,
                                                           struct DDS_BuiltinTopicKey_t *key)
{
    DDS_GUID_t remote_guid = DDS_GUID_INITIALIZER;
    RTPS_TrustPluginState trusted_state;

    /* a null config mean the participant is configured for trust. However this is not an error */
    if (config == NULL)
    {
        return RTI_TRUE;
    }
    else
    {
        trusted_state = config->state;
        if (!trusted_state.is_rtps_transform_enabled)
        {
            /* nothing to do for remote participants */
            return RTI_TRUE;
        }
    }
    DDS_DomainParticipant_builtin_key_to_dds_guid(key, &remote_guid);
    return RTPS_TrustPlugin_register_matched_remote_participant(
            config->plugin, &remote_guid);
}

RTI_BOOL
DDS_DomainParticipant_unregister_matched_remote_participant(struct DDS_Trust_ParticipantTrustConfig *config,
                                                           const struct DDS_BuiltinTopicKey_t *key)
{
    DDS_GUID_t remote_guid = DDS_GUID_INITIALIZER;
    RTPS_TrustPluginState trusted_state;

    /* a null config mean the participant is configured for trust. However this is not an error */
    if (config == NULL)
    {
        return RTI_TRUE;
    }
    else
    {
        trusted_state = config->state;
        if (!trusted_state.is_rtps_transform_enabled)
        {
            /* nothing to do for remote participants */
            return RTI_TRUE;
        }
    }

    DDS_DomainParticipant_builtin_key_to_dds_guid(key, &remote_guid);
    return RTPS_TrustPlugin_unregister_matched_remote_participant(
            config->plugin, &remote_guid);
}



RTI_BOOL
DDS_DomainParticipant_validate_local_participant_trust(struct DDS_Trust_ParticipantTrustConfig *config,
                                                            DDS_DomainId_t domain_id,
                                                            const struct DDS_DomainParticipantQos *qos,
                                                            DDS_GUID_t *candidate_guid_inout)
{
    /* a null config mean the participant is configured for trust. HOwever this is not an error */
    if (config == NULL)
    {
        return RTI_TRUE;
    }
    return RTPS_TrustPlugin_validate_participant_trust(
            config->plugin, domain_id, qos, candidate_guid_inout, &config->state);
}

void
DDS_Trust_DomainParticipant_invalidate_local_participant_trust(struct DDS_Trust_ParticipantTrustConfig *config)
{

    if ((config != NULL) && (config->plugin != NULL))
    {
        RTPS_TrustPlugin_invalidate_participant_trust(
            config->plugin);
    }
}

/*ci
 * \brief Tries to Create a trust configuration for the DomainParticipant if needed.
 *
 *  \details. This function creates a DDS_Trust_ParticipantTrustConfig
 *            if the DomainParticipantQos has enabled trust.
 * \param[in]  registry  The registry to use to create the trust plugin.
 * \param[in]  qos       The DomainParticipantQos to use to create the trust
 *                       plugin.
 * \param[out] config    The trust configuration created.
 *
 * \return RTI_TRUE if the trust configuration was created RTI_FALSE if the trust configuration could not be created.
 */

RTI_BOOL
DDS_DomainParticipantFactory_assert_trust_config(
        RT_Registry_T *registry,
        const struct DDS_DomainParticipantQos *qos,
        struct DDS_Trust_ParticipantTrustConfig  **config)
{
    RTI_BOOL result = RTI_FALSE;
    RT_ComponentFactory_T *c_factory = NULL;
    struct DDS_Trust_ParticipantTrustConfig default_config =
        DDS_Trust_ParticipantTrustConfig_INITIALIZER;
    struct DDS_Trust_ParticipantTrustConfig *config_ptr = NULL;
    RTPS_TrustPluginProperty plugin_property = RTPS_TrustPluginProperty_INITIALIZER;

    if (!DDS_DomainParticipantQos_is_trust_enabled(qos))
    {
        *config = DDS_Trust_ParticipantTrustConfig_Null;
        return RTI_TRUE;
    }

    OSAPI_Heap_allocate_struct(&config_ptr, struct DDS_Trust_ParticipantTrustConfig);
    if (config_ptr == NULL)
    {
        DDSC_LOG_TRUST_CREATE_TRUST_PLUGIN(OSAPI_LOGKIND_ERROR,
            RT_ComponentFactoryId_get_name(&qos->trust.suite));
        goto done;
    }

    *config_ptr = default_config;

    c_factory = RT_Registry_lookup(registry,
        RT_ComponentFactoryId_get_name(
                &qos->trust.suite));
    if (c_factory == NULL)
    {
        DDSC_LOG_TRUST_LOOKUP_FACTORY_FAILED(OSAPI_LOGKIND_ERROR,
            RT_ComponentFactoryId_get_name(&qos->trust.suite))
        goto done;
    }

    plugin_property.qos = qos;
    config_ptr->plugin = RTPS_TrustPlugin_create_component(c_factory,&plugin_property._parent,NULL);

    if (config_ptr->plugin == NULL)
    {
        DDSC_LOG_TRUST_CREATE_TRUST_PLUGIN(OSAPI_LOGKIND_ERROR,
            RT_ComponentFactoryId_get_name(&qos->trust.suite));
        goto done;
    }
    *config = config_ptr;
    result = RTI_TRUE;

done:
#ifndef RTI_CERT
    if (!result)
    {
        if (config_ptr != NULL && config_ptr->plugin != NULL)
        {
            RTPS_TrustPlugin_delete_component(c_factory,&config_ptr->plugin->_parent);
        }

        if (config_ptr != NULL)
        {
            OSAPI_Heap_free(config_ptr);
            config_ptr = NULL;
        }

    }
#endif
    return result;
}

/*ci
 * \brief Tries to delete a trust configuration for the DomainParticipant if needed.
 *
 *  \details. This function selectively deletes a DDS_Trust_ParticipantTrustConfig
 *            if the DomainParticipantQos has enabled trust and previously created a trust
 *           configuration.
 * \param[in]  registry  The registry to use to delete the trust plugin.
 * \param[in]  qos       The DomainParticipantQos to use to delete the trust
 *                       plugin.
 * \param[in]  config    The trust configuration created.
 *
 * \return RTI_TRUE - if the trust configuration was sucessfully deleted
 *                  - if the config pased in was DDS_Trust_ParticipantTrustConfig_Null.
 *         RTI_FALSE if the trust configuration could not be deleted.
 */

#ifndef RTI_CERT
RTI_BOOL
DDS_DomainParticipantFactory_delete_trust_config(
        RT_Registry_T *registry,
        const struct DDS_DomainParticipantQos *qos,
        struct DDS_Trust_ParticipantTrustConfig *config)
{
    RTI_BOOL result = RTI_FALSE;
    RT_ComponentFactory_T *c_factory = NULL;

    if (config == DDS_Trust_ParticipantTrustConfig_Null)
    {
        return RTI_TRUE;
    }

    c_factory = RT_Registry_lookup(registry,
                    RT_ComponentFactoryId_get_name(
                    &qos->trust.suite));
    if (c_factory == NULL)
    {
        DDSC_LOG_TRUST_LOOKUP_FACTORY_FAILED(OSAPI_LOGKIND_ERROR,
            RT_ComponentFactoryId_get_name(&qos->trust.suite));
        goto done;
    }

    if (config->plugin != NULL)
    {
        RTPS_TrustPlugin_delete_component(c_factory,&config->plugin->_parent);
        config->plugin = NULL;
    }

    OSAPI_Heap_free(config);
    config = NULL;
    result = RTI_TRUE;

done:

    return result;
}
#endif /* RTI_CERT */

RTI_UINT32
DDS_DomainParticipant_get_max_serialized_trust_param_size(const DDS_DomainParticipant *self, RTI_UINT32 size)
{
    struct DDS_Trust_ParticipantTrustConfig *config = self->config.trust;
    RTI_UINT32 orig_size = size;
    if (DDS_DomainParticipant_is_trust_enabled(self))
    {
        return (RTPS_TrustPlugin_max_serialized_size(config->plugin, size) - orig_size);
    }
    return 0;
}

RTI_BOOL
DDS_DomainParticipant_serialize_trust_param(const DDS_DomainParticipant *self, struct CDR_Stream_t *stream)
{
    RTI_BOOL ok = RTI_TRUE;
    struct DDS_Trust_ParticipantTrustConfig *config = self->config.trust;
    if (!DDS_DomainParticipant_is_trust_enabled(self))
    {
        /* return true but no information is serialized */
        return ok;
    }

    ok = RTPS_TrustPlugin_serialize(config->plugin,stream);

    return ok;
}


RTI_BOOL
DDS_DomainParticipant_initialize_rtps_trust_property(
        struct DDS_DomainParticipantImpl *self,
        struct RTPS_InterfaceProperty  *rtps_property,
        struct RT_ComponentFactory *rtps_factory)
{
    RTI_INT32 max_mtu = 0;
    RTPS_TrustPluginState trusted_state;
    struct RTPS_InterfaceTrustProperty *rtps_trust_property = &rtps_property->trust_property;
    struct RTPS_InterfaceFactoryProperty *rtps_fprop = NULL;

    NETIO_InterfaceFactory_get_property(rtps_factory,&rtps_fprop);
    if (rtps_fprop == NULL)
    {
        return RTI_FALSE;
    }
    if ((DDS_DomainParticipant_is_trust_enabled(self)))
    {
        trusted_state = DDS_DomainParticipant_trust_state(self);
        rtps_trust_property->enabled = trusted_state.is_rtps_transform_enabled ? RTI_TRUE : RTI_FALSE;
    }

    if (!rtps_trust_property->enabled)
    {
        return RTI_TRUE;
    }

    trusted_state = DDS_DomainParticipant_trust_state(self);

    /* Legacy CRC32 is not allowed when psk is enabled */
    if (rtps_fprop->checksum.checksum_tx_mode == RTPS_CHECKSUM_TXMODE_RTICRC32)
    {
        return RTI_FALSE;
    }

    rtps_trust_property->service_plugin = self->config.trust->plugin;
    rtps_trust_property->aad_enabled = trusted_state.is_aad_enabled;
    rtps_trust_property->transform_buffer_count = DDS_TRUST_MAX_TRANSFORM_BUFFERS;

    max_mtu = NETIO_RouteResolver_get_maximum_mtu(self->route_resolver);
    if (max_mtu < 0)
    {
        return RTI_FALSE;
    }
    if (max_mtu > DEFAULT_MAX_TRANSFORMED_SERIALIZED_PAYLOAD_SIZE)
    {
        max_mtu = DEFAULT_MAX_TRANSFORMED_SERIALIZED_PAYLOAD_SIZE;
    }
    rtps_trust_property->transform_buffer_size = (RTI_SIZE_T)max_mtu;
    return RTI_TRUE;
}


RTI_BOOL
DDS_DomainParticipant_set_trust_property(
        struct DDS_DomainParticipantImpl *self,
        const struct DDS_DomainParticipantQos *qos)
{
    RTI_BOOL ok = RTI_TRUE;
    struct DDS_Trust_ParticipantTrustConfig *config = self->config.trust;
    struct CDR_Property *passphrase_property = NULL;
    char* bretval = NULL;
    if (!DDS_DomainParticipant_is_trust_enabled(self))
    {
        /* trust not enabled. Nothing to do.*/
        return ok;
    }

    ok = RTPS_TrustPlugin_set_qos(config->plugin,qos);
    if (ok)
    {
        /* if we are here we know the passphrase is is successfully updated.
         * We free up the passphrase property since that is the only one allowed to be modified.
         * This is done so that the next call upstream which copies the qos is successfull
         */

        passphrase_property = DDS_PropertyQosPolicyHelper_lookup_property(
                                &self->qos.property,
                                DDS_TRUST_RTPS_PSK_PASSPHRASE_PROPERTY);
        if (passphrase_property != NULL)
        {
            bretval = REDA_String_replace(&passphrase_property->value, NULL);
            IGNORE_RETVAL(bretval);
        }

    }
    return ok;
}

/*ci @}  */
