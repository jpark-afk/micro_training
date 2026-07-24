/*
 * FILE: DPSECdr.c - DPSE CDR functionality
 *
 * Copyright (c) 2011-2024 Real-Time Innovations, Inc.
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
 * 29jun2015,tk MICRO-1347/PR#15098 Fixed comment errors
 * 16mar2015,tk MICRO-1129/PR#14274 Correctly pass DDS_ENTITYNAME_QOS_NAME_MAX
 *                                  to CDR serialize and de-serialize functions
 *                                  for entity name
 * 16mar2015,tk MICRO-1130/PR#14275 Fixed comment
 * 20feb2015,eh MICRO-813/PR#9172 Fix Lint warnings
 * 05jan2014,tk Updated log-codes
 * 25jul2011,tk Written
 */
/*ce
 * \file
 * \brief DPSE CDR functionality
 *
 * \details
 * This file implements helper function to serialize and deserialize CDR
 * datatypes for the participant announcements related to the static discovery
 * plugin.
 *
 */
/*ci \addtogroup DPSEModule
 * @{
 */
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif

#include "DPSECdr.h"

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Serialize the product version PID
 *
 * \param[in] stream          Stream to serialize to
 * \param[in] product_version Product version to serialize
 * \param[in] param           Opaque param passed from generated type-plugin
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
RTI_BOOL
DPSE_Builtin_serialize_product_version(struct CDR_Stream_t *stream,
                            const struct DDS_ProductVersion_t *product_version,
                            void *param)
{
    UNUSED_ARG(param);

    if (!CDR_Stream_serialize_octet(stream,&product_version->major))
    {
        return RTI_FALSE;
    }

    if (!CDR_Stream_serialize_octet(stream,&product_version->minor))
    {
        return RTI_FALSE;
    }

    if (!CDR_Stream_serialize_octet(stream,&product_version->release))
    {
        return RTI_FALSE;
    }

    if (!CDR_Stream_serialize_octet(stream,&product_version->revision))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief Deserialize the product version PID
 *
 * \param[in] stream          Stream to deserialize from
 * \param[in] product_version Product version to deserialize into
 * \param[in] param           Opaque param passed from generated type-plugin
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
RTI_BOOL
DPSE_Builtin_deserialize_product_version(struct CDR_Stream_t *stream,
                                       struct DDS_ProductVersion_t *product_version,
                                       void *param)
{
    UNUSED_ARG(param);

    if (!CDR_Stream_deserialize_octet(stream,&product_version->major))
    {
        return RTI_FALSE;
    }

    if (!CDR_Stream_deserialize_octet(stream,&product_version->minor))
    {
        return RTI_FALSE;
    }

    if (!CDR_Stream_deserialize_octet(stream,&product_version->release))
    {
        return RTI_FALSE;
    }

    if (!CDR_Stream_deserialize_octet(stream,&product_version->revision))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief Calculate the maximum size of the serialized product version
 *        given the input size.
 *
 * \details
 * The input size is the current logical size of all the fields preceding the
 * product version. The current size can then also be viewed as the logical
 * offset into a stream and must be aligned properly.
 *
 * \param[in] size current logical size of the all preceding fields
 *
 * \return Maximum size of the serialized product version, including any
 *         padding
 */
RTI_UINT32
DPSE_Builtin_get_product_version_max_size_serialized(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += CDR_get_1_byte_max_size_serialized(size);
    size += CDR_get_1_byte_max_size_serialized(size);
    size += CDR_get_1_byte_max_size_serialized(size);
    size += CDR_get_1_byte_max_size_serialized(size);

    return (size - orig_size);
}

/* ----------------------------------------------------------------- */
/*ci
 * \brief Serialize the entity name PID
 *
 * \param[in] stream        Stream to serialize to
 * \param[in] entity_name   Entity name to serialize
 * \param[in] param         Opaque param passed from generated type-plugin
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
RTI_BOOL
DPSE_Builtin_serialize_entity_name_qos_policy(struct CDR_Stream_t *stream,
                            const struct DDS_EntityNameQosPolicy *entity_name,
                            void *param)
{
    UNUSED_ARG(param);

    /* entity_name holds DDS_ENTITYNAME_QOS_NAME_MAX + 1 byte,
     * and CDR_Stream_serialize_string adds 1 to include the NUL termintaion
     * byte.
     */
    if (!CDR_Stream_serialize_string(stream, entity_name->name,
                                     DDS_ENTITYNAME_QOS_NAME_MAX))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief Deserialize the entity name PID
 *
 * \param[in] stream        Stream to deserialize from
 * \param[in] entity_name   Entity name to deserialize into
 * \param[in] param         Opaque param passed from generated type-plugin
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure.
 */
RTI_BOOL
DPSE_Builtin_deserialize_entity_name_qos_policy(struct CDR_Stream_t *stream,
                            struct DDS_EntityNameQosPolicy *entity_name,
                            void *param)
{
    UNUSED_ARG(param);

    /* Entity name is preallocated with size of DDS_ENTITYNAME_QOS_NAME_MAX+1
     * NOTE: The maximum length passed to CDR_Stream_deserialize_string
     * (3rd parameter) does _not_ include the NUL termination character. The
     * maximum number of bytes available including the NUL termination is
     * DDS_ENTITYNAME_QOS_NAME_MAX + 1.
     */
    if (!CDR_Stream_deserialize_string(stream, entity_name->name,
                                       DDS_ENTITYNAME_QOS_NAME_MAX))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief Calculate the maximum size of the serialized entity name
 *        given the the input size.
 *
 * \details
 * The input size is the current logical size of all the fields preceding the
 * entity name. The current size can also then be viewed as the logical
 * offset into a stream and must then be aligned properly.
 *
 * \param[in] size current logical size of the all preceding fields
 *
 * \return Maximum size of the serialized entity name, including any
 *         padding
 */
RTI_UINT32
DPSE_Builtin_get_entity_name_qos_policy_max_size_serialized(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    /* The maximum number of bytes that can be stored in the entity name is
     * DDS_ENTITYNAME_QOS_NAME_MAX excluding the NUL termination character,
     * thus add 1 for the maximum length.
     */
    size += CDR_get_max_size_serialized_string(size, DDS_ENTITYNAME_QOS_NAME_MAX + 1);

    return (size - orig_size);
}

/*ci \brief Serialize the CRC properties structure
 *
 * \param[in] stream The stream to serialize to
 * \param[in] crc_property The CRC property to serialize
 * \param[in] param Used defined parameter, not used
 *
 * \return TRUE if the CRC was successfully serialized, FALSE if not.
 */
RTI_BOOL
DPSE_Builtin_serialize_checksum_property(struct CDR_Stream_t *stream,
                     const struct DDS_ChecksumProperty *checksum_property,
                     void *param)
{
    RTI_BOOL ok = RTI_TRUE;
    UNUSED_ARG(param);

    ok = CDR_Stream_serialize_unsigned_short(stream,
                            (RTI_UINT16*)&checksum_property->computed_crc_kind);

    ok = ok && CDR_Stream_serialize_unsigned_short(stream,
                            (RTI_UINT16*)&checksum_property->allowed_crc_mask);

    ok = ok && CDR_Stream_serialize_boolean(stream,&checksum_property->require_crc);

    return ok;
}

/*ci \brief Deserialize the CRC properties
 *
 * \param[in] stream The stream to serialize from
 * \param[in] crc_property The CRC property fill in
 * \param[in] param Used defined parameter, not used
 *
 * \return TRUE if the CRC was successfully serialized, FALSE if not.
 */
RTI_BOOL
DPSE_Builtin_deserialize_checksum_property(struct CDR_Stream_t *stream,
                                        struct DDS_ChecksumProperty *checksum_property,
                                        void *param)
{
    UNUSED_ARG(param);

    if (!CDR_Stream_deserialize_unsigned_short(stream,
                     (RTI_UINT16*)&checksum_property->computed_crc_kind))
    {
        return RTI_FALSE;
    }

    if (!CDR_Stream_deserialize_unsigned_short(stream,
                    (RTI_UINT16*)&checksum_property->allowed_crc_mask))
    {
        return RTI_FALSE;
    }

    if (!CDR_Stream_deserialize_boolean(stream,&checksum_property->require_crc))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci \brief Get the maximum size of a serialized CRC property
 *
 * \param[in] size The current size of the stream
 *
 * \return maximum size of the CRC property aligned to the minimum CDR alignment.
 */
RTI_UINT32
DPSE_Builtin_get_checksum_property_max_size_serialized(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += 2 * CDR_get_max_size_serialized_short(0) +
                CDR_get_max_size_serialized_boolean(0);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return (size - orig_size);
}

/*ci @} */

