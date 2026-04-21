/*
 * FILE: DomainParticipantChecksum.c - DomainParticipant checksum related functions
 *
 * (c) Copyright, Real-Time Innovations, 2020 - 2020.
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
 * 04jan2021,tk MICRO-2800/PR#28566
 *   - Do not allow a NULL RTPS property in DDS_DomainParticipant_checksum_configure()
 *     even if no checksum functionality is required by DDS.
 * 19oct2020,tk MICRO-2575/PR#28172 Removed support for custom checksums.
 * 05may2020,tk MICRO-2364/PR#27563 Added missing SOURCE_BEGIN
 */
#include "dds_c/dds_c_config.h"
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_discovery_plugin_h
#include "dds_c/dds_c_discovery_plugin.h"
#endif

#include "Entity.h"
#include "DomainParticipant.h"
#include "DomainParticipantChecksum.h"

/*** SOURCE_BEGIN ***/

/*ci \brief Configure the checksum support for a participant
 *          based on what is supported by RTPS and the Qos policies.
 *
 * \param[in] participant The participant
 * \param[in] rtps_factory The RTPS component
 * \param[inout] rtps_property The updated rtps property
 *
 * \return TRUE on successful configuration, FALSE on failure.
 */
DDS_Boolean
DDS_DomainParticipant_checksum_configure(struct DDS_DomainParticipantImpl *participant,
                                    struct RT_ComponentFactory *rtps_factory,
                                    struct RTPS_InterfaceProperty *rtps_property)
{
    struct RTPS_InterfaceFactoryProperty *rtps_fprop = NULL;
    DDS_ChecksumKindMask_t supported_crc = DDS_CHECKSUM_NONE;

    NETIO_InterfaceFactory_get_property(rtps_factory,&rtps_fprop);

    if (rtps_fprop == NULL)
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (RTPS_ChecksumClass_is_supported(&rtps_fprop->checksum.builtin_checksum32_class))
    {
        supported_crc |= DDS_CHECKSUM_BUILTIN32;
    }

    if (RTPS_ChecksumClass_is_supported(&rtps_fprop->checksum.builtin_checksum64_class))
    {
        supported_crc |= DDS_CHECKSUM_BUILTIN64;
    }

    if (RTPS_ChecksumClass_is_supported(&rtps_fprop->checksum.builtin_checksum128_class))
    {
        supported_crc |= DDS_CHECKSUM_BUILTIN128;
    }

    if (participant->qos.protocol.compute_crc)
    {
        participant->builtin_data.checksum.computed_crc_kind =
                                participant->qos.protocol.computed_crc_kind;

        if (participant->builtin_data.checksum.computed_crc_kind == DDS_CHECKSUM_AUTO)
        {
            participant->builtin_data.checksum.computed_crc_kind = DDS_CHECKSUM_BUILTIN32;
        }
        else if ((rtps_fprop->checksum.checksum_tx_mode == RTPS_CHECKSUM_TXMODE_RTICRC32) &&
                 (participant->builtin_data.checksum.computed_crc_kind != DDS_CHECKSUM_BUILTIN32))
        {
            /* If the transmit mode is CRC32 it is only possible to send
             * BUILTIN32 CRC.
             */
            DDSC_LOG_CHECKSUM_INCONSISTENT_COMPUTE(OSAPI_LOGKIND_ERROR,
                              participant->builtin_data.checksum.computed_crc_kind,
                              DDS_CHECKSUM_BUILTIN32)
            return DDS_BOOLEAN_FALSE;
        }

        if ((supported_crc & participant->builtin_data.checksum.computed_crc_kind)
             != participant->builtin_data.checksum.computed_crc_kind)
        {
            /* The computed_crc_kind  must be one that is supported */
            DDSC_LOG_CHECKSUM_INCONSISTENT_COMPUTE(OSAPI_LOGKIND_ERROR,
                                              participant->builtin_data.checksum.computed_crc_kind,
                                              supported_crc)
            return DDS_BOOLEAN_FALSE;
        }
    }

    participant->builtin_data.checksum.require_crc =
                                        participant->qos.protocol.require_crc;

    if (participant->qos.protocol.require_crc ||
        participant->qos.protocol.check_crc)
    {
        participant->builtin_data.checksum.allowed_crc_mask =
                                participant->qos.protocol.allowed_crc_mask;

        if (participant->builtin_data.checksum.allowed_crc_mask == DDS_CHECKSUM_AUTO)
        {
            participant->builtin_data.checksum.allowed_crc_mask = supported_crc;
        }

        if ((supported_crc & participant->builtin_data.checksum.allowed_crc_mask)
            != participant->builtin_data.checksum.allowed_crc_mask)
        {
            /* Specified allowed_crc_mask must be a subset of what is supported_crc */
            DDSC_LOG_CHECKSUM_INCONSISTENT_ALLOW(OSAPI_LOGKIND_ERROR,
                                            participant->builtin_data.checksum.allowed_crc_mask,
                                            supported_crc)
            return DDS_BOOLEAN_FALSE;
        }
    }

    if (participant->qos.protocol.compute_crc)
    {
        rtps_property->computed_crc_kind =
                            participant->builtin_data.checksum.computed_crc_kind;
    }
    else
    {
        rtps_property->computed_crc_kind = DDS_CHECKSUM_NONE;
    }

    if (participant->qos.protocol.require_crc ||
        participant->qos.protocol.check_crc)
    {
        rtps_property->allowed_crc_mask =
                            participant->builtin_data.checksum.allowed_crc_mask;
    }
    else
    {
        rtps_property->allowed_crc_mask = DDS_CHECKSUM_NONE;
    }

    rtps_property->require_crc = participant->qos.protocol.require_crc;
    rtps_property->check_crc = participant->qos.protocol.check_crc;

    return DDS_BOOLEAN_TRUE;
}

/*ci \brief Determine if a participant is compatible with a discovered
 *          participant's CRC configuration.
 *
 * \param[in] participant The participant
 * \param[in] remote  The remote participants configuration
 *
 * \return TRUE if they are compatible, FALSE if they are not compatible.
 */
DDS_Boolean
DDS_DomainParticipant_is_checksum_compatible(DDS_DomainParticipant *self,
                        const struct DDS_ParticipantBuiltinTopicData *remote)
{
    struct DDS_DomainParticipantImpl *participant =
                                    (struct DDS_DomainParticipantImpl *)self;

    if (!remote->checksum.require_crc &&
        !participant->qos.protocol.compute_crc &&
        !participant->qos.protocol.check_crc &&
        !participant->qos.protocol.require_crc)
    {
        /* The remote does not require CRC and the local does send or
         * process CRCs.
         */
        return DDS_BOOLEAN_TRUE;
    }

    if ((remote->checksum.require_crc && !participant->qos.protocol.compute_crc) ||
        (participant->qos.protocol.require_crc && (remote->checksum.computed_crc_kind == DDS_CHECKSUM_NONE)))
    {
        DDSC_LOG_CHECKSUM_REQUIRED(OSAPI_LOGKIND_WARNING,
                              remote->checksum.require_crc,
                              participant->qos.protocol.require_crc)
        /* One side requires, but the other is not sending, ignore */
        return DDS_BOOLEAN_FALSE;
    }

    /* NOTE: If remote require is TRUE, but an empty allowed_mask is received,
     *       it is considered compatible. This could be a mis-configuration.
     *       Instead of not passing, let the other side determine if there
     *       is a match or not.
     */
    if (participant->qos.protocol.compute_crc
        && (remote->checksum.allowed_crc_mask != DDS_CHECKSUM_NONE)
        && !(participant->builtin_data.checksum.computed_crc_kind & remote->checksum.allowed_crc_mask))
    {
        DDSC_LOG_CHECKSUM_INCOMPATIBLE(OSAPI_LOGKIND_WARNING,
                                  participant->builtin_data.checksum.computed_crc_kind,
                                  remote->checksum.allowed_crc_mask)
        /* Local is sending a CRC that is not understood by the remote */
        return DDS_BOOLEAN_FALSE;
    }

    /* If the remote sends more than 1 CRC it is ok as long as _all_ are
     * understood.
     */
    if ((participant->builtin_data.checksum.allowed_crc_mask != 0) &&
        (remote->checksum.computed_crc_kind != DDS_CHECKSUM_NONE) &&
        ((participant->builtin_data.checksum.allowed_crc_mask & remote->checksum.computed_crc_kind)
         != remote->checksum.computed_crc_kind))
    {
        DDSC_LOG_CHECKSUM_INCOMPATIBLE(OSAPI_LOGKIND_WARNING,
                                  remote->checksum.computed_crc_kind,
                                  participant->builtin_data.checksum.allowed_crc_mask)
        /* The remote is sending a CRC that is not understood */
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}
