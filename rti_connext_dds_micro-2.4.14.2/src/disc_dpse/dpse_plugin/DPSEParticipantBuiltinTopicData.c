/*
 * FILE: DPSEParticipantBuiltinTopicData.c - DPSE Participant builtin topic
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
 * 13sep2021,tk MICRO-3254/PR.29544
 * - Reset ParticipantBuiltinTopicData to default value before deserialization
 *   in DPSE_ParticipantBuiltinTopicDataTypePlugin_deserialize.
 * 20jul2021,tk MICRO-3127 Addressed MISRA-C 2021 and SEI CERT C warnings:
 *  - DPSE_ParticipantBuiltinTopicData_deserialize_parameter_value:
 *    - Added else branch for RTPS_PID_PARTICIPANT_GUID
 *    - Added else branch for RTPS_PID_PROTOCOL_VERSION
 *    - Added else branch for RTPS_PID_VENDOR_ID
 *    - Added else branch for RTPS_PID_LEASE_DURATION
 *  - Added coverity[cert_exp34_c_violation] in DPSE_ParticipantBuiltinTopicDataTypePlugin_get_parameters_max_size_serialized
 *  - Added coverity[dereference] in DPSE_ParticipantBuiltinTopicDataTypePlugin_get_parameters_max_size_serialized
 * 19jul2021,tk MICRO-3109/PR.29259
 * - Initialize the default value for the checksum property if it was received
 *   with a RTI CRC32 checksum and the vendor ID is RTI in
 *   DPSE_ParticipantBuiltinTopicDataTypePlugin_deserialize
 * 13jul2021,tk MICRO-3102/PR.29318
 *   - Assign ok before using it in a test in the following functions:
 *     DPSE_ParticipantBuiltinTopicDataTypePlugin_serialize_four_byte_parameter().
 *     DPSE_ParticipantBuiltinTopicDataTypePlugin_serialize_nonprimitive_parameter()
 *     DPSE_ParticipantBuiltinTopicDataTypePlugin_deserialize_parameter_sequence
 * 21jun2021,tk MICRO-3098/PR.29300
 *   - Use a local initialSize=size variable in get_header_max_size_serialized()
 *     and get_parameters_max_size_serialized() to prevent optimizing out size.
 * 21feb2021,tk MICRO-2872/PR#28755
 *   - Updated DPSE_ParticipantBuiltinTopicData_deserialize_locator() to
 *     explicitly ignore the result if the locator sequence is full to avoid
 *     implicitly ignoring the result.
 *   - Check the return from set_length() if a locator is dropped.
 *   - Return FALSE if deserialization fails, just in case it happened that
 *     a partially deserialized locator was valid.
 * 28jul2015,tk MICRO-1471/PR#15633  Use DDS_Duration_to_ntp_time as an additional
 *                                   robustness check
 * 29jun2015,tk  MICRO-1366/PR#15179 Check *sample, not sample,in create_sample()
 * 29jun2015,tk  MICRO-1362/PR#15169 Corrected function comment header
 * 28may2015,tk  MICRO-1248/PR#14839 Replaced magic number 254 with
 *                                   DDS_ENTITYNAME_QOS_NAME_MAX
 * 28may2015,tk  MICRO-1247/PR#14838 Refactored deserialize_parameter_value and
 *                                   fail on failed sequence operations (should
 *                                   be impossible).
 * 28may2015,tk  MICRO-1246/PR#14837 Removed RTI_BOOL parameter to
 *                                   deserialize_parameter_value
 * 28may2015,tk  MICRO-1245/PR#14836 Fixed comment in ipv6_from_ipv4
 * 28may2015,tk  MICRO-1244/PR#14835 Removed additional call to
 *                          CDR_Stream_get_encapsulation_size in
 *                          get_parameters_max_size_serialized()
 * 16apr2015,eh  MICRO-1142 Fix DPSE_fv_ParticipantBuiltinTopicDataTypePlugin
 *               for Cert
 * 25Mar2015,as  MICRO-908: Removed unused PluginHelper functions
 * 23mar2014,eh  MICRO-1040/PR#13555 add param check to instance_to_keyhash
 * 17mar2015,tk  MICRO-1119/PR#14255 Pass in instance handle when calling dispose
 * 23feb2015,eh  MICRO-1081: fix function/var names to conform to coding std
 * 20feb2015,eh  MICRO-813/PR#9172 Fix Lint warnings
 * 05jan2014,tk  Updated log-codes
 * 26jun2014,eh  MICRO-812: ignore deserialized RTPS_PID_PROPERTY_LIST
 * 20may2013,eg  MICRO-799: Unknown RTPS parameter as incompatible Qos 
 * 07may2014,eh  MICRO-313/VerocelPR 1439: CDR_Stream_Align() returns void
 * 07mar2014,tk  MICRO-735: Send properties for tools
 * 07aug2012,tk  MICRO-250: Send user-data multi-cast addresses
 * 07aug2012,tk  MICRO-486: Send discovery multi-cast addresses
 * 03jun2008,rmw Created.
 */
/*ce
 * \file
 * \brief DPSE Participant builtin topic
 *
 * \details
 * This is the type-plugin for the built-in participant topic data. It is
 * handwritten as the IDL compiler does not support code-generation for the
 * parameter list type. The data-type itself is defined by the DDS and RTPS
 * specifications.
 */
/*ci \addtogroup DPSEModule
 * @{
 */
#ifndef cdr_encapsulation_h
#include "cdr/cdr_encapsulation.h"
#endif
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef disc_dpse_dpsediscovery_h
#include "disc_dpse/disc_dpse_dpsediscovery.h"
#endif
#ifndef disc_dpse_log_h
#include "disc_dpse/disc_dpse_log.h"
#endif

#include "DPSECdr.h"
#include "DPSEParticipantBuiltinTopicData.h"
#include "DPSEDiscoveryPlugin.h"

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Determine the parameter length based on the start and end offset
 *
 * \details
 *
 * Calculate the parameter length based on the start and end offsets in the
 * serialization buffer. The offsets are signed 32 bits. This function tests
 * if the difference is larger than an unsigned short can hold. If that is the
 * case FALSE is returned. This function assumes end >= begin.
 *
 * \param[out] length The parameter length
 * \param[in]  begin  The beginning of the paraemeter
 * \param[in]  end    The end of the parameter
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
DPSE_ParticipantBuiltinTopicDataTypePlugin_get_parameter_length(
                                      DDS_UnsignedShort *length,
                                      DDS_UnsignedLong begin,
                                      DDS_UnsignedLong end)
{
    DDS_UnsignedLong diff = end - begin;

    if (diff > USHRT_MAX)
    {
        return RTI_FALSE;
    }

    *length = (DDS_UnsignedShort)diff;

    return RTI_TRUE;
}

/*ci
 * \brief Insert a parameter header for a PID of the specified length
 *
 * \details
 * When a PID is added to the stream the contents are first serialized then
 * the PID header is updated. If the parameter is successfully added the
 * stream is updated with the PID header, otherwise the PID is ignored.
 *
 * \param[in] stream            Stream to add header to
 * \param[in] param_begin_pos   The beginning of the parameter
 * \param[in] parameter_length  The length of the new PID
 * \param[in] parameter_success Whether the new PID was serialized successfully
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_TRUE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE DDS_Boolean
DPSE_ParticipantBuiltinTopicDataTypePlugin_insert_parameter_length(
                                            struct CDR_Stream_t *stream,
                                            DDS_UnsignedLong param_begin_pos,
                                            DDS_UnsignedShort parameter_length,
                                            RTI_BOOL parameter_success)
{
    RTI_UINT32 current_position;
    RTI_BOOL ok = RTI_TRUE;

    if (parameter_success)
    {
        current_position = CDR_Stream_get_current_position_offset(stream);

        ok = CDR_Stream_set_current_position_offset(stream,
                param_begin_pos + CDR_SHORT_SIZE)
               && CDR_Stream_serialize_unsigned_short(stream, &parameter_length);

        if (ok)
        {
            if (!CDR_Stream_set_current_position_offset(stream, current_position))
            {
                DPSE_LOG_CDR_SET_POSITION(OSAPI_LOGKIND_ERROR,current_position)
                return DDS_BOOLEAN_FALSE;
            }
            return DDS_BOOLEAN_TRUE;
        }
        else
        {
            DPSE_LOG_CDR_SET_POSITION(OSAPI_LOGKIND_ERROR,
                                      param_begin_pos + CDR_SHORT_SIZE)
            return DDS_BOOLEAN_FALSE;
        }
    }

    if (!CDR_Stream_set_current_position_offset(stream, param_begin_pos))
    {
        DPSE_LOG_CDR_SET_POSITION(OSAPI_LOGKIND_ERROR,param_begin_pos)
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Serialize a 4 byte PID to a CDR stream
 *
 * \param[in] stream       The stream to add the PID to
 * \param[in] in           The data to serialize
 * \param[in] parameter_id The PID
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPSE_ParticipantBuiltinTopicDataTypePlugin_serialize_four_byte_parameter(
                                                struct CDR_Stream_t *stream,
                                                const void *in,
                                                DDS_UnsignedShort parameter_id)
{
    RTI_BOOL ok;
    DDS_UnsignedShort parameter_length;
    RTI_UINT32 parameter_begin_position;
    RTI_UINT32 value_begin_position;
    RTI_UINT32 value_end_position;

    parameter_begin_position = CDR_Stream_get_current_position_offset(stream);

    /* parameter Id */
    ok = CDR_Stream_serialize_unsigned_short(stream, &parameter_id);

    /* skip parameter length */
    ok = ok && CDR_Stream_increment_current_position(stream, CDR_SHORT_SIZE);

    /* parameter value parameter has to be 4-byte aligned */
    value_begin_position = CDR_Stream_get_current_position_offset(stream);

    if (ok)
    {
        if (!CDR_Stream_serialize_long(stream, (long *)in))
        {
            DPSE_LOG_CDR_SERIALIZE(OSAPI_LOGKIND_ERROR,DPSE_LOG_CDR_LONG_KIND,0)
            return RTI_FALSE;
        }
    }
    else
    {
        DPSE_LOG_CDR_SERIALIZE(OSAPI_LOGKIND_ERROR,
                               DPSE_LOG_CDR_PID_KIND,parameter_id)
        return RTI_FALSE;
    }

    CDR_Stream_align(stream, CDR_DEFAULT_PARAMETER_ALIGNMENT);
    value_end_position = CDR_Stream_get_current_position_offset(stream);

    if (!DPSE_ParticipantBuiltinTopicDataTypePlugin_get_parameter_length(
                &parameter_length,value_begin_position,value_end_position))
    {
        return RTI_FALSE;
    }

    return DPSE_ParticipantBuiltinTopicDataTypePlugin_insert_parameter_length(
                stream,parameter_begin_position,parameter_length,ok);
}

/*ci
 * \brief Helper function to serialize a non-primitive parameter type
 *
 * \details
 * Non-primitive parameters are types that themselves contain types,
 * such as arrays, sequences, structures etc.
 *
 * \param[in] stream         The stream to serialize to
 * \param[in] in             The data to serialize
 * \param[in] serialize_func A function that can serialize the data
 * \param[in] parameter_id   The parameter ID for the data
 *
 * \return RTI_TRUE on failure, RTI_FALSE on success
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPSE_ParticipantBuiltinTopicDataTypePlugin_serialize_nonprimitive_parameter(
                                struct CDR_Stream_t *stream,
                                const void *in,
                                CDR_Stream_SerializeFunction serialize_func,
                                DDS_UnsignedShort parameter_id)
{
    RTI_BOOL ok;
    DDS_UnsignedShort parameter_length;
    RTI_UINT32 parameter_begin_position;
    RTI_UINT32 value_begin_position;
    RTI_UINT32 value_end_position;

    parameter_begin_position = CDR_Stream_get_current_position_offset(stream);

    /* parameter Id */
    ok = CDR_Stream_serialize_unsigned_short(stream, &parameter_id);

    /* skip parameter length */
    ok = ok && CDR_Stream_increment_current_position(stream, CDR_SHORT_SIZE);

    /* parameter value parameter has to be 4-byte aligned */
    value_begin_position = CDR_Stream_get_current_position_offset(stream);
    ok = ok && serialize_func(stream, in, NULL);

    if (!ok)
    {
        DPSE_LOG_CDR_SERIALIZE(OSAPI_LOGKIND_ERROR,
                               DPSE_LOG_CDR_PID_KIND,parameter_id)
        return RTI_FALSE;
    }

    CDR_Stream_align(stream, CDR_DEFAULT_PARAMETER_ALIGNMENT);
    value_end_position = CDR_Stream_get_current_position_offset(stream);

    if (!DPSE_ParticipantBuiltinTopicDataTypePlugin_get_parameter_length(
                &parameter_length,value_begin_position,value_end_position))
    {
        return RTI_FALSE;
    }

    return DPSE_ParticipantBuiltinTopicDataTypePlugin_insert_parameter_length(
                stream,parameter_begin_position,parameter_length,ok);
}

/*ci
 * \brief Calculate the size of a parameter including padding
 *
 * \param[in] size The current size of all the preceding parameters
 *
 * \return The size of the parameter, including any padding
 */
MUST_CHECK_RETURN RTI_PRIVATE DDS_UnsignedLong
DPSE_ParticipantBuiltinTopicDataTypePlugin_get_parameter_header_max_size_serialized(
                                                    DDS_UnsignedLong size)
{
    DDS_UnsignedLong origSize = size;

    /* align to 4 bytes boundary */
    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    /* paramterId */
    size += CDR_get_max_size_serialized_unsigned_short(size);

    /* paramterLength */
    size += CDR_get_max_size_serialized_unsigned_short(size);

    return (size - origSize);
}

/*ci
 * \brief Calculate the maximum serialized size of all parameters serialized
 *        by this plugin
 *
 * \param[in] size        The current logical size of all preceding parameters
 * \param[in] disc_plugin The discovery plugin
 *
 * \return The size of the parameters, including any padding
 */
MUST_CHECK_RETURN RTI_PRIVATE DDS_UnsignedLong
DPSE_ParticipantBuiltinTopicDataTypePlugin_get_parameters_max_size_serialized(
                                    DDS_UnsignedLong size,
                                    struct DPSE_DiscoveryPlugin *disc_plugin)
{
    DDS_UnsignedLong origSize = size;
    DDS_UnsignedLong initialSize = size;
    DDS_Long i = 0;
    struct DDS_Property *a_property;
    RTI_SIZE_T s_len;

    /*
     * initialSize is used to prevent the TASKING compiler from optimizing
     * out the size value which prevents function level testing.
     *
     * Call get max size serialized on each parameter header and field
     * according to the type. We assume there is a logical reset before
     * serializing each value.
     *
     * getParameterHeaderMaxSizeSerialize() take care of 4-byte alignment.
     * Sentinel parameter size is not included in the calculation.
     */

    /* Guid */
    initialSize += DPSE_ParticipantBuiltinTopicDataTypePlugin_get_parameter_header_max_size_serialized(initialSize);
    initialSize += RTPS_Guid_get_max_size_serialized(0);

    /* Builtin endpoint mask */
    initialSize += DPSE_ParticipantBuiltinTopicDataTypePlugin_get_parameter_header_max_size_serialized(initialSize);
    initialSize += CDR_get_max_size_serialized_unsigned_long(0);

    /* Protocol version */
    initialSize += DPSE_ParticipantBuiltinTopicDataTypePlugin_get_parameter_header_max_size_serialized(initialSize);
    initialSize += RTPS_get_2_octets_max_size_serialized(0);

    /* Vendor ID */
    initialSize += DPSE_ParticipantBuiltinTopicDataTypePlugin_get_parameter_header_max_size_serialized(initialSize);
    initialSize += RTPS_get_2_octets_max_size_serialized(0);

    /* User Unicast */
    for (i = 0; i < RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX; ++i)
    {
        initialSize += DPSE_ParticipantBuiltinTopicDataTypePlugin_get_parameter_header_max_size_serialized(initialSize);
        initialSize += RTPS_get_ipv6_locator_max_size_serialized(0);
    }

    /* User Multicast */
    for (i = 0; i < RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX; ++i)
    {
        initialSize += DPSE_ParticipantBuiltinTopicDataTypePlugin_get_parameter_header_max_size_serialized(initialSize);
        initialSize += RTPS_get_ipv6_locator_max_size_serialized(0);
    }

    /* Meta Unicast */
    for (i = 0; i < RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX; ++i)
    {
        initialSize += DPSE_ParticipantBuiltinTopicDataTypePlugin_get_parameter_header_max_size_serialized(initialSize);
        initialSize += RTPS_get_ipv6_locator_max_size_serialized(0);
    }

    /* Meta Multicast */
    for (i = 0; i < RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX; ++i)
    {
        initialSize += DPSE_ParticipantBuiltinTopicDataTypePlugin_get_parameter_header_max_size_serialized(initialSize);
        initialSize += RTPS_get_ipv6_locator_max_size_serialized(0);
    }

    /* Lease duration */
    initialSize += DPSE_ParticipantBuiltinTopicDataTypePlugin_get_parameter_header_max_size_serialized(initialSize);
    initialSize += RTPS_get_ntp_time_max_size_serialized(0);

    /* Product Version */
    initialSize += DPSE_ParticipantBuiltinTopicDataTypePlugin_get_parameter_header_max_size_serialized(initialSize);
    initialSize += DPSE_Builtin_get_product_version_max_size_serialized(initialSize);

    /* CRC properties */
    initialSize += DPSE_ParticipantBuiltinTopicDataTypePlugin_get_parameter_header_max_size_serialized(initialSize);
    initialSize += DPSE_Builtin_get_checksum_property_max_size_serialized(initialSize);

    /* Participant Name */
    initialSize += DPSE_ParticipantBuiltinTopicDataTypePlugin_get_parameter_header_max_size_serialized(initialSize);
    initialSize += DPSE_Builtin_get_entity_name_qos_policy_max_size_serialized(initialSize);

    /* since parameter can be serialized in any order, we need to make sure
     * that the next parameter is taken into account */
    initialSize = CDR_align_upwards(initialSize, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    /* NOTE: Participant properties are sent once when the participant is
     * created. Thus, it is ok to calculate exact space needed to serialize
     * only current/existing properties.
     */
    if (disc_plugin)
    {
        for (i = 0; i < DDS_PropertySeq_get_length(disc_plugin->dds_properties); ++i)
        {
            /* This sequence is statically initialized and not changeable
             * by an API, and is assumed to correct.
             */
            a_property = DDS_PropertySeq_get_reference(disc_plugin->dds_properties,i);

            /* coverity[cert_exp34_c_violation] */
            /* coverity[dereference] */
            s_len = REDA_String_length(a_property->name) + 1;
            initialSize += CDR_get_max_size_serialized_string(initialSize, s_len);

            initialSize = CDR_align_upwards(initialSize, CDR_DEFAULT_PARAMETER_ALIGNMENT);

            /* coverity[cert_exp34_c_violation] */
            /* coverity[dereference] */
            s_len = REDA_String_length(a_property->value) + 1;
            initialSize += CDR_get_max_size_serialized_string(initialSize, s_len);

            initialSize = CDR_align_upwards(initialSize, CDR_DEFAULT_PARAMETER_ALIGNMENT);
        }
    }

    /* sentinel parameter header */
    initialSize += DPSE_ParticipantBuiltinTopicDataTypePlugin_get_parameter_header_max_size_serialized(initialSize);

    return (initialSize - origSize);
}

/*ci
 * \brief  Get the max size of the serialization header after padding
 *
 * \param[in] size The size of all the preceding serialized fields
 *
 * \return The maximum serialized size of the header
 */
MUST_CHECK_RETURN RTI_PRIVATE DDS_UnsignedLong
DPSE_ParticipantBuiltinTopicDataTypePlugin_get_header_max_size_serialized(
                                                        DDS_UnsignedLong size)
{
    DDS_UnsignedLong origSize = size;
    DDS_UnsignedLong initialSize = size;

    /* initialSize is used to prevent the TASKING compiler from optimizing
     * out the size value which prevents function level testing.
     */
    initialSize = CDR_align_upwards(initialSize, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    initialSize += CDR_Stream_get_encapsulation_size(initialSize);

    return initialSize - origSize;

}

/*ci
 * \brief Calculate the maximum serialized size of the participant sample
 *
 * \param[in] plugin            The type-plugin
 * \param[in] current_alignment The current logical alignment in a stream
 * \param[in] param             Opaque parameter passed from the type-plugin
 *
 * \return The maximum serialized size of a participant sample
 */
MUST_CHECK_RETURN RTI_PRIVATE DDS_UnsignedLong
DPSE_ParticipantBuiltinTopicDataTypePlugin_get_serialized_sample_max_size(
                                            struct NDDS_Type_Plugin *plugin,
                                            DDS_UnsignedLong current_alignment,
                                            void *param)
{
    struct DPSE_DiscoveryPlugin *disc_plugin =
                        (struct DPSE_DiscoveryPlugin *)param;
    UNUSED_ARG(plugin);
    UNUSED_ARG(current_alignment);

    return DPSE_ParticipantBuiltinTopicDataTypePlugin_get_header_max_size_serialized(0) +
           DPSE_ParticipantBuiltinTopicDataTypePlugin_get_parameters_max_size_serialized(
            0,disc_plugin);

}

/*ci
 * \brief Create an IPv6 address from an IPv4 address
 *
 * \param[in]  ipv6_locator The IPv6 to create
 * \param[out] ipv4_locator The IPv4 address to convert
 */
RTI_PRIVATE void
DPSE_ParticipantBuiltinTopicDataTypePlugin_ipv6_from_ipv4(
                                        struct DDS_Locator_t *ipv6_locator,
                                        struct DDS_LocatorUdpv4_t *ipv4_locator)
{
    RTI_UINT32 tmp;

    OSAPI_Memory_copy(&tmp,&ipv4_locator->address,RTI_SIZEOF(tmp));

    OSAPI_Memory_zero(ipv6_locator->address, 12);

    /* This does not need to be byte swapped because we always store the locator
     * in network order.
     */
    ipv6_locator->address[12] = (RTI_UINT8)(tmp >> 24);
    ipv6_locator->address[13] = (RTI_UINT8)((tmp & 0x00FF0000) >> 16);
    ipv6_locator->address[14] = (RTI_UINT8)((tmp & 0x0000FF00) >> 8);
    ipv6_locator->address[15] = (RTI_UINT8)((tmp & 0x000000FF));
}

/*ci
 * \brief Serialize a DDS locater sequence to a stream
 *
 * \param[in] stream   The stream to serialize to locator sequence to
 * \param[in] loc_seq  The locator sequence to serialize
 * \param[in] param_id The PID for the locator sequence
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
DPSE_ParticipantBuiltinTopicData_serialize_locator(
                                               struct CDR_Stream_t *stream,
                                               struct DDS_LocatorSeq *loc_seq,
                                               DDS_UnsignedShort param_id)
{
    DDS_Long i;
    struct DDS_Locator_t generic_loc;

    for (i = 0; i < DDS_LocatorSeq_get_length(loc_seq); ++i)
    {
        generic_loc = *DDS_LocatorSeq_get_reference(loc_seq, i);

        if (NETIO_Address_get_kind((struct NETIO_Address*)&generic_loc) == RTPS_LOCATOR_KIND_UDPv4)
        {
            struct DDS_LocatorUdpv4_t *loc = NULL;
            OSAPI_Compiler_reinterpret_cast(loc,&generic_loc,ptr_ptr);
            DPSE_ParticipantBuiltinTopicDataTypePlugin_ipv6_from_ipv4(&generic_loc, loc);
        }

        if (!DPSE_ParticipantBuiltinTopicDataTypePlugin_serialize_nonprimitive_parameter(
            stream, &generic_loc,
            (CDR_Stream_SerializeFunction) RTPS_serialize_ipv6_locator, param_id))
        {
            return RTI_FALSE;
        }
    }
    return RTI_TRUE;
}

/*ci
 * \brief Serialize the participant sample to a stream
 *
 * \details
 * Implementation of the NDDS_TypePlugin_serialize_data method
 *
 * \param[in] stream The stream to serialize to
 * \param[in] data   The participant data to serialize
 * \param[in] param  Opaque pointer passed from the generated code
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPSE_ParticipantBuiltinTopicDataTypePlugin_serialize(
                                                struct CDR_Stream_t *stream,
                                                const void *data,
                                                void *param)
{
    DDS_UnsignedShort zeroLength = 0;
    struct RTPS_Guid guid;
    RTPS_ProtocolVersion_T protocolVersion;
    RTPS_VendorId vendorId;
    struct OSAPI_NtpTime serializeTime;
    RTPS_ParameterId sentinel;
   
    struct DDS_ParticipantBuiltinTopicData *topic_data =
                            (struct DDS_ParticipantBuiltinTopicData *)data;
    struct DPSE_DiscoveryPlugin *disc_plugin =
                        (struct DPSE_DiscoveryPlugin *)param;

    UNUSED_ARG(param);

    guid.prefix.host_id = topic_data->key.value[0];
    guid.prefix.app_id  = topic_data->key.value[1];
    guid.prefix.instance_id  = topic_data->key.value[2];
    guid.object_id = topic_data->key.value[3];

    /* Serialize the participant guid (the participant key) */
    if (!DPSE_ParticipantBuiltinTopicDataTypePlugin_serialize_nonprimitive_parameter(
            stream, &guid,(CDR_Stream_SerializeFunction) RTPS_Guid_serialize,
            RTPS_PID_PARTICIPANT_GUID))       
    {
        DPSE_LOG_SERIALIZE_GUID(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (!DPSE_ParticipantBuiltinTopicDataTypePlugin_serialize_four_byte_parameter(
        stream, &topic_data->dds_builtin_endpoints,
         RTPS_PID_BUILTIN_ENDPOINT_MASK))
    {
        DPSE_LOG_SERIALIZE_BUILTIN_ENDPOINTS(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    /* --- Protocol version --- */
    protocolVersion = (RTPS_ProtocolVersion_T)(((RTI_UINT16)topic_data->rtps_protocol_version.major << 8) |
                                              (RTI_UINT16)topic_data->rtps_protocol_version.minor);

    /* Serialize the participant parameter */
    if (!DPSE_ParticipantBuiltinTopicDataTypePlugin_serialize_nonprimitive_parameter(
            stream,&protocolVersion,
            (CDR_Stream_SerializeFunction) RTPS_serialize_2_octets,
            RTPS_PID_PROTOCOL_VERSION))
    {
        DPSE_LOG_SERIALIZE_PROTOCOL_VERSION(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    /* --- Vendor ID --- */
    vendorId = (RTPS_VendorId)(((RTI_UINT16)topic_data->rtps_vendor_id.vendorId[0] << 8) |
                             (RTI_UINT16)topic_data->rtps_vendor_id.vendorId[1]);

    if (!DPSE_ParticipantBuiltinTopicDataTypePlugin_serialize_nonprimitive_parameter(
                            stream,&vendorId,
                            (CDR_Stream_SerializeFunction)RTPS_serialize_2_octets,
            RTPS_PID_VENDOR_ID))
    {
        DPSE_LOG_SERIALIZE_VENDOR_ID(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    /* User Unicast */
    if (!DPSE_ParticipantBuiltinTopicData_serialize_locator(
                           stream, &topic_data->default_unicast_locators,
                           RTPS_PID_DEFAULT_UNICAST_LOCATOR6))
    {
        DPSE_LOG_SERIALIZE_DEFAULT_UNICAST(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    /* User Multicast */
    if (!DPSE_ParticipantBuiltinTopicData_serialize_locator(
       stream, &topic_data->default_multicast_locators, 
       RTPS_PID_MULTICAST_LOCATOR6))
    {
        DPSE_LOG_SERIALIZE_MULTICAST(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    /* Meta Unicast */
    if (!DPSE_ParticipantBuiltinTopicData_serialize_locator(
       stream, &topic_data->metatraffic_unicast_locators, 
       RTPS_PID_METATRAFFIC_UNICAST_LOCATOR6))
    {
        DPSE_LOG_SERIALIZE_META_UNICAST(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    /* Meta Multicast */
    if (!DPSE_ParticipantBuiltinTopicData_serialize_locator(
       stream, &topic_data->metatraffic_multicast_locators, 
       RTPS_PID_METATRAFFIC_MULTICAST_LOCATOR6))
    {
        DPSE_LOG_SERIALIZE_META_MULTICAST(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    /* Lease duration */
    DDS_Duration_to_ntp_time(&topic_data->liveliness_lease_duration,&serializeTime);

    if (!DPSE_ParticipantBuiltinTopicDataTypePlugin_serialize_nonprimitive_parameter(
            stream,&serializeTime,
            (CDR_Stream_SerializeFunction) RTPS_serialize_ntp_time,
            RTPS_PID_LEASE_DURATION))
    {
        DPSE_LOG_SERIALIZE_LEASE_DURATION(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    /* Product version */
    if (!DPSE_ParticipantBuiltinTopicDataTypePlugin_serialize_nonprimitive_parameter(
            stream,&topic_data->product_version,
            (CDR_Stream_SerializeFunction)DPSE_Builtin_serialize_product_version,
            DISC_RTPS_PID_PRODUCT_VERSION))
    {
        DPSE_LOG_SERIALIZE_PRODUCT_VERSION(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    /* CRC properties */
    if ((topic_data->checksum.allowed_crc_mask != 0) ||
        (topic_data->checksum.computed_crc_kind != 0))
    {
        if (!DPSE_ParticipantBuiltinTopicDataTypePlugin_serialize_nonprimitive_parameter
                (stream, &topic_data->checksum,
                 (CDR_Stream_SerializeFunction) DPSE_Builtin_serialize_checksum_property,
                 RTPS_PID_CHECKSUM_PROPERTY))
        {
            return RTI_FALSE;
        }
    }

    /* Participant name */
    if (!DPSE_ParticipantBuiltinTopicDataTypePlugin_serialize_nonprimitive_parameter(
       stream,
       &topic_data->participant_name,
       (CDR_Stream_SerializeFunction)DPSE_Builtin_serialize_entity_name_qos_policy,
       DISC_RTPS_PID_ENTITY_NAME))
    {
        DPSE_LOG_SERIALIZE_PARTICIPANT_NAME(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    /* Property List */
    if (disc_plugin && !DPSE_ParticipantBuiltinTopicDataTypePlugin_serialize_nonprimitive_parameter(stream,
            disc_plugin->dds_properties,
            (CDR_Stream_SerializeFunction)CDR_Stream_serialize_property_sequence,
            RTPS_PID_PROPERTY_LIST))
    {
        return RTI_FALSE;
    }

    sentinel = RTPS_PID_SENTINEL;
    if (DDS_BOOLEAN_TRUE != CDR_Stream_serialize_unsigned_short(stream, &sentinel))
    {
        DPSE_LOG_CDR_SERIALIZE(OSAPI_LOGKIND_ERROR,
                               DPSE_LOG_CDR_PID_KIND,RTPS_PID_SENTINEL)
        return RTI_FALSE;
    }

    if (DDS_BOOLEAN_TRUE !=
        CDR_Stream_serialize_unsigned_short(stream, &zeroLength))
    {
        DPSE_LOG_CDR_SERIALIZE(OSAPI_LOGKIND_ERROR,
                               DPSE_LOG_CDR_PID_KIND,RTPS_PID_SENTINEL)
        return RTI_FALSE;
    }


    return RTI_TRUE;
}

/*ci
 * \brief Deserialize a parameter value of the given PID
 *
 * \param[in] parameter               Pointer to the parameter to which value
 *                                    should be deserialized
 * \param[in] stream                  Stream for deserialization
 * \param[in] deserialize_param_value Deserialization function for the parameter
 *
 * \return Return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPSE_ParticipantBuiltinTopicDataTypePlugin_deserialize_parameter_sequence(
        void *parameter,
        struct CDR_Stream_t *stream,
        DDS_ParticipantBuiltinTopicData_TypePluginDeserializeParameterValueFunction
        deserialize_param_value,void *param)
{
    RTPS_ParameterId parameter_id;
    RTPS_ParameterId parameter_length;
    RTI_BOOL ok;
    RTI_UINT32 orig_pos;

    /* parameter id and length */
    ok = CDR_Stream_deserialize_unsigned_short(stream, &parameter_id);
    ok = ok && CDR_Stream_deserialize_unsigned_short(stream, &parameter_length);

    while (ok && parameter_id != RTPS_PID_SENTINEL)
    {
        orig_pos = CDR_Stream_get_current_position_offset(stream);

        /* logical reset of stream per parameter value */
        ok = deserialize_param_value(parameter,
                                 stream, parameter_id, parameter_length,param);

        /* skip to next parameter */
        ok = ok && CDR_Stream_set_current_position_offset(
                                        stream,orig_pos + parameter_length);

        /* parameter id and length */
        ok = ok && CDR_Stream_deserialize_unsigned_short(stream, &parameter_id);
        ok = ok && CDR_Stream_deserialize_unsigned_short(stream, &parameter_length);
    }

    return ok;
}

/*ci
 * \brief Helper function to deserialize a locator sequence
 *
 * \details
 * This function can only deserialize up to RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX
 * locators to the sequence. It is assumed that the sequence has been
 * successfully initialized to a maximum lenght of
 * RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX elements.
 *
 *
 * \param[in] locator_seq The locator sequence to deserialize
 * \param[in] stream      The CDR stream to deserialize from
 * \param[in] pid         The parameter ID for the locator
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
DPSE_ParticipantBuiltinTopicData_deserialize_locator(
                                            struct DDS_LocatorSeq *locator_seq,
                                            struct CDR_Stream_t *stream,
                                            DDS_UnsignedShort pid,
                                            void *param)
{
    RTI_BOOL result;
    struct DDS_Locator_t *locator;
    struct RTPS_Locator_t discard_loc;
    RTI_INT32 l, ml;
    struct DPSE_DiscoveryPlugin *plugin = (struct DPSE_DiscoveryPlugin *)param;

#if defined(RTI_CERT) || !OSAPI_ENABLE_LOG
    UNUSED_ARG(pid);
#endif /* !RTI_CERT */

    l = DDS_LocatorSeq_get_length(locator_seq);
    ml = DDS_LocatorSeq_get_maximum(locator_seq);

    if (l == ml)
    {
#ifndef RTI_CERT
        DPSE_LOG_DESERIALIZE_TOO_MANY_LOCATORS(OSAPI_LOGKIND_WARNING,l)
#else
        DPSE_LOG_DESERIALIZE_TOO_MANY_LOCATORS(OSAPI_LOGKIND_ERROR,l)
#endif
        result = RTPS_deserialize_ipv6_locator(stream, &discard_loc, NULL);
#ifndef RTI_CERT
        if (!result)
        {
            /* De-serialization error, return error to drop the sample. */
            DPSE_LOG_CDR_DESERIALIZE(OSAPI_LOGKIND_ERROR,
                                     DPSE_LOG_CDR_PID_KIND,pid)
            return RTI_FALSE;
        }
        return RTI_TRUE;
#else
        /* For CERT, RTI_FALSE is returned regardless of whether deserializing
         * the IPv6 locator failed or not, thus ignore the result.
         */
        IGNORE_RETVAL(result);
        return RTI_FALSE;
#endif
    }

    if (!DDS_LocatorSeq_set_length(locator_seq,l + 1))
    {
        return RTI_FALSE;
    }

    locator = DDS_LocatorSeq_get_reference(locator_seq,l);
    if (locator == NULL)
    {
        return RTI_FALSE;
    }

    NETIO_Address_init((struct NETIO_Address*)locator, 0);

    result = RTPS_deserialize_ipv6_locator(stream, locator, NULL);
    if (!result)
    {
        DPSE_LOG_CDR_DESERIALIZE(OSAPI_LOGKIND_ERROR,
                                 DPSE_LOG_CDR_PID_KIND,pid)
        return RTI_FALSE;
    }

    /* Check if this is a supported locator, if not drop it altogether.
     */
    if (!DDS_DomainParticipant_locator_is_supported(plugin->participant,locator))
    {
        /* Discard the locator */
        DPSE_LOG_UNSUPPORTED_LOCATOR(OSAPI_LOGKIND_WARNING,locator->kind)
        if (!DDS_LocatorSeq_set_length(locator_seq, l))
        {
            return RTI_FALSE;
        }
    }

    return RTI_TRUE;
}

/*ci
 * \brief Deserialize the specified parameter id (PID)
 *
 * \details
 * This is a helper function to deserialize one parameter value based on the
 * input arguments.
 *
 * \param[out] topic_data       Storage for deserialized data
 * \param[in]  stream           Stream to deserialize data from
 * \param[in]  parameter_id     The PID to deserialize
 * \param[in]  parameter_length The length of the PID
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure. Note that the return
 *         value does not indicate if the data is correct or not, only that
 *         the function itself completed.
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPSE_ParticipantBuiltinTopicData_deserialize_parameter_value(
                            struct DDS_ParticipantBuiltinTopicData *topic_data,
                            struct CDR_Stream_t *stream,
                            DDS_UnsignedShort parameter_id,
                            DDS_UnsignedShort parameter_length,
                            void *param)
{
    struct RTPS_Guid guid;
    DDS_UnsignedShort protocolVersion;
    RTPS_VendorId vendorId;
    struct OSAPI_NtpTime time;
    RTI_BOOL ok;

    UNUSED_ARG(parameter_length);

    ok = RTI_TRUE;

    /* check each field to see if the paramterId matches */
    switch (parameter_id)
    {
        case RTPS_PID_PARTICIPANT_GUID:
            if (!RTPS_Guid_deserialize(stream, &guid, NULL))
            {
                DPSE_LOG_DESERIALIZE_GUID(OSAPI_LOGKIND_ERROR)
                ok = RTI_FALSE;
            }
            else
            {
                topic_data->key.value[0] = guid.prefix.host_id;
                topic_data->key.value[1] = guid.prefix.app_id ;
                topic_data->key.value[2] = guid.prefix.instance_id ;
                topic_data->key.value[3] = guid.object_id;
            }
            break;
        case RTPS_PID_BUILTIN_ENDPOINT_MASK:
            if (!CDR_Stream_deserialize_unsigned_long
                (stream, &topic_data->dds_builtin_endpoints))
            {
                DPSE_LOG_DESERIALIZE_BUILTIN_ENDPOINTS(OSAPI_LOGKIND_ERROR)
                ok = RTI_FALSE;
            }
            break;
        case RTPS_PID_PROTOCOL_VERSION:
        {
            if (!RTPS_deserialize_2_octets(stream, &protocolVersion, NULL))
            {
                DPSE_LOG_DESERIALIZE_PROTOCOL_VERSION(OSAPI_LOGKIND_ERROR)
                ok = RTI_FALSE;
            }
            else
            {
                topic_data->rtps_protocol_version.major = (DDS_Octet)(protocolVersion >> 8);
                topic_data->rtps_protocol_version.minor = (DDS_Octet)(protocolVersion & 0x00ff);
            }
            break;
        }
        case RTPS_PID_VENDOR_ID:
        {
            if (!RTPS_deserialize_2_octets(stream, &vendorId, NULL))
            {
                DPSE_LOG_DESERIALIZE_VENDOR_ID(OSAPI_LOGKIND_ERROR)
                ok = RTI_FALSE;
            }
            else
            {
                topic_data->rtps_vendor_id.vendorId[0] =
                                            RTPS_VendorId_get_major(&vendorId);
                topic_data->rtps_vendor_id.vendorId[1] =
                                            RTPS_VendorId_get_minor(&vendorId);
            }
            break;
        }
        case RTPS_PID_DEFAULT_UNICAST_LOCATOR6:
            if (!DPSE_ParticipantBuiltinTopicData_deserialize_locator(
                    &topic_data->default_unicast_locators,
                    stream,RTPS_PID_DEFAULT_UNICAST_LOCATOR6,param))
            {
                ok = RTI_FALSE;
            }
            break;
        case RTPS_PID_MULTICAST_LOCATOR6:
            if (!DPSE_ParticipantBuiltinTopicData_deserialize_locator(
                    &topic_data->default_multicast_locators,
                    stream,RTPS_PID_MULTICAST_LOCATOR6,param))
            {
                ok = RTI_FALSE;
            }
            break;
        case RTPS_PID_METATRAFFIC_UNICAST_LOCATOR6:
            if (!DPSE_ParticipantBuiltinTopicData_deserialize_locator(
                    &topic_data->metatraffic_unicast_locators,
                    stream,RTPS_PID_METATRAFFIC_UNICAST_LOCATOR6,param))
            {
                ok = RTI_FALSE;
            }
            break;
        case RTPS_PID_METATRAFFIC_MULTICAST_LOCATOR6:
            if (!DPSE_ParticipantBuiltinTopicData_deserialize_locator(
                    &topic_data->metatraffic_multicast_locators,
                    stream,RTPS_PID_METATRAFFIC_UNICAST_LOCATOR6,param))
            {
                ok = RTI_FALSE;
            }
            break;
        case RTPS_PID_LEASE_DURATION:
            if (!RTPS_deserialize_ntp_time(stream, &time, NULL))
            {
                DPSE_LOG_CDR_DESERIALIZE(OSAPI_LOGKIND_ERROR,
                                         DPSE_LOG_CDR_PID_KIND,
                                         RTPS_PID_LEASE_DURATION)
                ok = RTI_FALSE;
            }
            else
            {
                OSAPI_NtpTime_to_nanosec(
                        &topic_data->liveliness_lease_duration.sec,
                        &topic_data->liveliness_lease_duration.nanosec, &time);
            }
            break;
        case DISC_RTPS_PID_PRODUCT_VERSION:
        {
            vendorId = (RTPS_VendorId)(((RTI_UINT16)topic_data->rtps_vendor_id.vendorId[0] << 8) |
                                        (RTI_UINT16)topic_data->rtps_vendor_id.vendorId[1]);

            if (vendorId == RTPS_VENDOR_ID_RTI_DDS || 
                vendorId == RTPS_VENDOR_ID_RTI_MICRO)
            {
                if (!DPSE_Builtin_deserialize_product_version(
                                    stream,&topic_data->product_version, NULL))
                {
                    DPSE_LOG_DESERIALIZE_PRODUCT_VERSION(OSAPI_LOGKIND_ERROR)
                    ok = RTI_FALSE;
                }
            }
            break;
        }
        case DISC_RTPS_PID_ENTITY_NAME:
            if (!DPSE_Builtin_deserialize_entity_name_qos_policy(
                                stream, &topic_data->participant_name,NULL))
            {
                DPSE_LOG_DESERIALIZE_PARTICIPANT_NAME(OSAPI_LOGKIND_ERROR)
                ok = RTI_FALSE;
            }
            break;

        case RTPS_PID_PROPERTY_LIST:
            /* Micro sends DDS_ParticipantBuiltinTopicData containing
               RTPS_PID_PROPERTY_LIST, but does not use any of its 
               current properties.  Thus, ignore this parameter quietly. */
            break;
        case RTPS_PID_CHECKSUM_PROPERTY:
            if (CDR_Stream_is_vendor_rti(stream))
            {
                ok = DPSE_Builtin_deserialize_checksum_property(
                                     stream, &topic_data->checksum, NULL);
            }
            else
            {
                ok = RTI_FALSE;
                DPSE_LOG_DESERIALIZE_UNKNOWN_PID(OSAPI_LOGKIND_WARNING,
                                                 (RTI_INT32)parameter_id,
                                                 (RTI_INT32)parameter_length)
            }
            break;
        default:
            /* Unknown parameter is either ignored or treated as
               incompatible QoS */
            if ((parameter_id & RTPS_PID_INCOMPATIBLE_MASK) != 0)
            {
                ok = RTI_FALSE;
                DPSE_LOG_DESERIALIZE_UNKNOWN_PID(OSAPI_LOGKIND_WARNING,
                                                 (RTI_INT32)parameter_id,
                                                 (RTI_INT32)parameter_length)
            }
#if OSAPI_ENABLE_LOG
            else
            {
                DPSE_LOG_DESERIALIZE_UNKNOWN_PID(OSAPI_LOGKIND_INFO,
                                                 (RTI_INT32)parameter_id,
                                                 (RTI_INT32)parameter_length)
            }
#endif
            return ok;
    }

    return ok;
}

/*ci
 * \brief Create a ParticipantBuiltinTopicData sample
 *
 * \details
 * Implementation of NDDS_Type_Plugin_create_sample
 *
 * \param[in]  plugin The type-plugin creating the sample
 * \param[out] sample Pointer to new and initialized participant sample
 * \param[in]  param  Opaque parameter passed from the type-plugin
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPSE_ParticipantBuiltinTopicDataTypePlugin_create_sample(
                                        struct NDDS_Type_Plugin* plugin,
                                        void **sample,
                                        void *param)
{
    struct DDS_ParticipantBuiltinTopicData *participant_builtin_data = NULL;
    UNUSED_ARG(plugin);
    UNUSED_ARG(sample);
    UNUSED_ARG(param);

    OSAPI_Heap_allocate_struct(sample, struct DDS_ParticipantBuiltinTopicData);

    if (*sample == NULL)
    {
        DPSE_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,
                                 DPSE_PARTICIPANTBUILTINTOPICDATA_OBJECT)

        return DDS_BOOLEAN_FALSE;
    }

    participant_builtin_data = (*sample);

    if (!DDS_ParticipantBuiltinTopicData_initialize(participant_builtin_data))
    {
        DPSE_LOG_OBJECT_INITIALIZE(OSAPI_LOGKIND_ERROR,
                                   DPSE_PARTICIPANTBUILTINTOPICDATA_OBJECT)
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_LocatorSeq_set_maximum(
                         &participant_builtin_data->default_unicast_locators,
                         DDSC_PARTICIPANT_ADDRESS_COUNT_MAX))
    {
        DPSE_LOG_SEQ_SETMAX(OSAPI_LOGKIND_ERROR,
                            DPSE_LOG_DEFAULT_UNICAST_LOCATOR_SEQUENCE,
                            DDSC_PARTICIPANT_ADDRESS_COUNT_MAX)
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_LocatorSeq_set_maximum(
                        &participant_builtin_data->default_multicast_locators,
                        DDSC_PARTICIPANT_ADDRESS_COUNT_MAX))
    {
        DPSE_LOG_SEQ_SETMAX(OSAPI_LOGKIND_ERROR,
                            DPSE_LOG_DEFAULT_MULTICAST_LOCATOR_SEQUENCE,
                            DDSC_PARTICIPANT_ADDRESS_COUNT_MAX)
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_LocatorSeq_set_maximum(
                    &participant_builtin_data->metatraffic_unicast_locators,
                    DDSC_PARTICIPANT_ADDRESS_COUNT_MAX))
    {
        DPSE_LOG_SEQ_SETMAX(OSAPI_LOGKIND_ERROR,
                            DPSE_LOG_META_UNICAST_LOCATOR_SEQUENCE,
                            DDSC_PARTICIPANT_ADDRESS_COUNT_MAX)
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_LocatorSeq_set_maximum(
                    &participant_builtin_data->metatraffic_multicast_locators,
                    DDSC_PARTICIPANT_ADDRESS_COUNT_MAX))
    {
        DPSE_LOG_SEQ_SETMAX(OSAPI_LOGKIND_ERROR,
                            DPSE_LOG_META_MULTICAST_LOCATOR_SEQUENCE,
                            DDSC_PARTICIPANT_ADDRESS_COUNT_MAX)
        return DDS_BOOLEAN_FALSE;
    }

    participant_builtin_data->dds_builtin_endpoints =
                                DDS_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER |
                                DDS_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR;

    return DDS_BOOLEAN_TRUE;
}

#ifndef RTI_CERT
/*ci
 * \brief Delete a ParticipantBuiltinTopicData sample
 *
 * \details
 * Implementation of NDDS_Type_Plugin_delete_sample
 *
 * \param[in] plugin The type-plugin the sample was allocated from
 * \param[in] sample Pointer to participant sample to delete
 * \param[in] param  Opaque parameter passed from the type-plugin
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPSE_ParticipantBuiltinTopicDataTypePlugin_delete_sample(
                                                struct NDDS_Type_Plugin *plugin,
                                                void *sample,
                                                void *param)
{
    struct DDS_ParticipantBuiltinTopicData *builtin_data =
                            (struct DDS_ParticipantBuiltinTopicData *)sample;

    UNUSED_ARG(plugin);
    UNUSED_ARG(sample);
    UNUSED_ARG(param);

    if (sample == NULL)
    {
        DPSE_LOG_OBJECT_DELETE(OSAPI_LOGKIND_ERROR,
                               DPSE_PARTICIPANTBUILTINTOPICDATA_OBJECT)
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_LocatorSeq_finalize(&builtin_data->default_unicast_locators))
    {
        DPSE_LOG_SEQ_FINALIZE(OSAPI_LOGKIND_ERROR,
                              DPSE_LOG_DEFAULT_UNICAST_LOCATOR_SEQUENCE)
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_LocatorSeq_finalize(&builtin_data->metatraffic_unicast_locators))
    {
        DPSE_LOG_SEQ_FINALIZE(OSAPI_LOGKIND_ERROR,
                              DPSE_LOG_META_UNICAST_LOCATOR_SEQUENCE)
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_LocatorSeq_finalize(&builtin_data->metatraffic_multicast_locators))
    {
        DPSE_LOG_SEQ_FINALIZE(OSAPI_LOGKIND_ERROR,
                              DPSE_LOG_META_MULTICAST_LOCATOR_SEQUENCE)
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_LocatorSeq_finalize(&builtin_data->default_multicast_locators))
    {
        DPSE_LOG_SEQ_FINALIZE(OSAPI_LOGKIND_ERROR,
                              DPSE_LOG_DEFAULT_MULTICAST_LOCATOR_SEQUENCE)
        return DDS_BOOLEAN_FALSE;
    }

    OSAPI_Heap_free_struct(builtin_data);

    return DDS_BOOLEAN_TRUE;
}
#endif /* !RTI_CERT */

/*ci
 * \brief Return the kind of key used by this type-plugin
 *
 * \details
 * Implementation of NDDS_Type_Plugin_get_key_kind
 *
 * \param[in] plugin The type-plugin
 * \param[in] param  Opaque parameter passed from the type-plugin
 *
 * \return The key type for the generated code
 */
MUST_CHECK_RETURN RTI_PRIVATE NDDS_TypePluginKeyKind
DPSE_ParticipantBuiltinTopicDataTypePlugin_get_key_kind(
                                              struct NDDS_Type_Plugin* plugin,
                                              void *param)
{
    UNUSED_ARG(plugin);
    UNUSED_ARG(param);

    return NDDS_TYPEPLUGIN_GUID_KEY;
}

/*ci
 * \brief Copy ParticipantBuiltinTopicData
 *
 * \details
 * Implementation of NDDS_Type_Plugin_copy_sample
 *
 * \param[in]    type  The type-plugin
 * \param[inout] dst   The preallocated destination for the sample
 * \param[in]    src   The source sample to copy from
 * \param[in]    param Opaque parameter passed from the type-plugin
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPSE_ParticipantBuiltinTopicDataTypePlugin_copy_sample(
                                                struct NDDS_Type_Plugin *type,
                                                void *dst,
                                                const void *src,
                                                void *param)
{
    struct DDS_ParticipantBuiltinTopicData *destination =
        (struct DDS_ParticipantBuiltinTopicData *)dst;
    struct DDS_ParticipantBuiltinTopicData *source =
        (struct DDS_ParticipantBuiltinTopicData *)src;
    DDS_Long i = 0;
    RTI_SIZE_T len;
    UNUSED_ARG(type);
    UNUSED_ARG(param);

    for (i = 0; i < DDS_BUILTIN_TOPIC_KEY_TYPE_NATIVE_LENGTH; i++)
    {
        destination->key.value[i] = source->key.value[i];
    }

    len = REDA_String_length(source->participant_name.name);
    if (len  > DDS_ENTITYNAME_QOS_NAME_MAX)
    {
        DPSE_LOG_OBJECT_INVALID(OSAPI_LOGKIND_ERROR,
                                DPSE_PARTICIPANTENTITYNAME_OBJECT)
        return DDS_BOOLEAN_FALSE;
    }

    OSAPI_Memory_copy(destination->participant_name.name,
                      source->participant_name.name, len+1);

    destination->dds_builtin_endpoints = source->dds_builtin_endpoints;

    destination->rtps_protocol_version.major =
                                source->rtps_protocol_version.major;
    destination->rtps_protocol_version.minor =
                                source->rtps_protocol_version.minor;

    for (i = 0; i < DDS_VENDOR_ID_LENGTH_MAX; i++)
    {
        destination->rtps_vendor_id.vendorId[i] =
                                        source->rtps_vendor_id.vendorId[i];
    }

    /* --- Default unicast locator list --- */
    if (!DDS_LocatorSeq_copy(&destination->default_unicast_locators,
                        &source->default_unicast_locators))
    {
        DPSE_LOG_SEQ_COPY(OSAPI_LOGKIND_ERROR,
                          DPSE_LOG_DEFAULT_UNICAST_LOCATOR_SEQUENCE)
        return DDS_BOOLEAN_FALSE;
    }

    /* --- Metatraffic unicast locator list --- */
    if (!DDS_LocatorSeq_copy(&destination->metatraffic_unicast_locators,
                             &source->metatraffic_unicast_locators))
    {
        DPSE_LOG_SEQ_COPY(OSAPI_LOGKIND_ERROR,
                          DPSE_LOG_META_UNICAST_LOCATOR_SEQUENCE)
        return DDS_BOOLEAN_FALSE;
    }

    /* --- Metatraffic multicast locator list --- */
    if (!DDS_LocatorSeq_copy(&destination->metatraffic_multicast_locators,
                             &source->metatraffic_multicast_locators))
    {
        DPSE_LOG_SEQ_COPY(OSAPI_LOGKIND_ERROR,
                          DPSE_LOG_META_MULTICAST_LOCATOR_SEQUENCE)
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_LocatorSeq_copy(&destination->default_multicast_locators,
                             &source->default_multicast_locators))
    {
        DPSE_LOG_SEQ_COPY(OSAPI_LOGKIND_ERROR,
                          DPSE_LOG_DEFAULT_MULTICAST_LOCATOR_SEQUENCE)
        return DDS_BOOLEAN_FALSE;
    }

    destination->liveliness_lease_duration.sec =
                                    source->liveliness_lease_duration.sec;
    destination->liveliness_lease_duration.nanosec =
                                    source->liveliness_lease_duration.nanosec;

    destination->product_version.major = source->product_version.major;
    destination->product_version.minor = source->product_version.minor;
    destination->product_version.release = source->product_version.release;
    destination->product_version.revision = source->product_version.revision;

    destination->checksum = source->checksum;

    return DDS_BOOLEAN_TRUE;
}

/*ci \brief Default value for the DDS_ParticipantBuiltinTopicData
 */
RTI_PRIVATE const struct DDS_ParticipantBuiltinTopicData
    DPSE_ParticipantBuiltinTopicData_fv_Default =
                                DDS_ParticipantBuiltinTopicData_INITIALIZER;
/*ci
 * \brief Deserialize a serialized participant sample
 *
 * \details
 * Implementation of NDDS_Type_Plugin_deserialize
 *
 * \param[in]    stream The stream to deserialize data from
 * \param[inout] sample The deserialized sample
 * \param[in]    param  Opaque parameter passed from the type-plugin
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
DPSE_ParticipantBuiltinTopicDataTypePlugin_deserialize(
                                                struct CDR_Stream_t *stream,
                                                void *sample,
                                                void *param)
{
    struct DDS_ParticipantBuiltinTopicData *topic_data =
        (struct DDS_ParticipantBuiltinTopicData *)sample;

    RTI_BOOL retval;

    /* Reset the sample to default values as it is being reused */
    topic_data->key = DPSE_ParticipantBuiltinTopicData_fv_Default.key;
    topic_data->participant_name = DPSE_ParticipantBuiltinTopicData_fv_Default.participant_name;
    topic_data->dds_builtin_endpoints = DPSE_ParticipantBuiltinTopicData_fv_Default.dds_builtin_endpoints;
    topic_data->rtps_protocol_version = DPSE_ParticipantBuiltinTopicData_fv_Default.rtps_protocol_version;
    topic_data->rtps_vendor_id = DPSE_ParticipantBuiltinTopicData_fv_Default.rtps_vendor_id;

    /* This is a new sample: empty out all sequences */
    if (!DDS_LocatorSeq_set_length(&topic_data->default_unicast_locators, 0))
    {
        DPSE_LOG_SEQ_SETLENGTH(OSAPI_LOGKIND_ERROR,
                               DPSE_LOG_DEFAULT_UNICAST_LOCATOR_SEQUENCE,0)
        return RTI_FALSE;
    }

    if (!DDS_LocatorSeq_set_length(&topic_data->default_multicast_locators, 0))
    {
        DPSE_LOG_SEQ_SETLENGTH(OSAPI_LOGKIND_ERROR,
                               DPSE_LOG_DEFAULT_MULTICAST_LOCATOR_SEQUENCE,0)
        return RTI_FALSE;
    }

    if (!DDS_LocatorSeq_set_length(&topic_data->metatraffic_unicast_locators, 0))
    {
        DPSE_LOG_SEQ_SETLENGTH(OSAPI_LOGKIND_ERROR,
                               DPSE_LOG_META_UNICAST_LOCATOR_SEQUENCE,0)
        return RTI_FALSE;
    }

    if (!DDS_LocatorSeq_set_length(&topic_data->metatraffic_multicast_locators, 0))
    {
        DPSE_LOG_SEQ_SETLENGTH(OSAPI_LOGKIND_ERROR,
                               DPSE_LOG_META_MULTICAST_LOCATOR_SEQUENCE,0)
        return RTI_FALSE;
    }

    topic_data->liveliness_lease_duration = DPSE_ParticipantBuiltinTopicData_fv_Default.liveliness_lease_duration;
    topic_data->product_version = DPSE_ParticipantBuiltinTopicData_fv_Default.product_version;

    /* If the stream was protected with a legacy RTI CRC32 checksum
     * assume that the sender does not require a checksum and will
     * accept any checksum. However, if a checksum property is received it
     * takes precedence.
     */
    if (CDR_Stream_is_checksum_crc32(stream) &&
            CDR_Stream_is_vendor_rti(stream))
    {
        topic_data->checksum.computed_crc_kind = DDS_CHECKSUM_BUILTIN32;
        topic_data->checksum.allowed_crc_mask = DDS_CHECKSUM_NONE;
        topic_data->checksum.require_crc = RTI_FALSE;
    }
    else
    {
        topic_data->checksum = DPSE_ParticipantBuiltinTopicData_fv_Default.checksum;
    }

    retval = DPSE_ParticipantBuiltinTopicDataTypePlugin_deserialize_parameter_sequence(
                topic_data, stream,
                (DDS_ParticipantBuiltinTopicData_TypePluginDeserializeParameterValueFunction)
                DPSE_ParticipantBuiltinTopicData_deserialize_parameter_value,param);

    return retval;
}

/*ci
 * \brief This function is only present to comply with the type-plugin interface
 *
 * \details
 *
 * Implementation of NDDS_TypePlugin_serialize_key. This method is not
 * implemented as the APIs that requires it are not support by Micro, such
 * as get_key_value.
 *
 * \param[in] stream     Stream to serialize key to
 * \param[in] sample_key A sample with the key fields filled in
 * \param[in] param      Opaque parameter passed from the type-plugin
 *
 * \return This function returns DDS_BOOLEAN_TRUE
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DPSE_ParticipantBuiltinTopicDataTypePlugin_serialize_key(
                                     struct CDR_Stream_t *stream,
                                     const void *sample_key,
                                     void *param)
{
    UNUSED_ARG(stream);
    UNUSED_ARG(sample_key);
    UNUSED_ARG(param);

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Deserialize a key. Only present to comply with the type-plugin interface
 *
 * \details
 * This function is used to deserialize a key from a serialized stream. However
 * the DPSE plugin does not support this feature.
 *
 * \param[in] stream     Stream to deserialize samples from
 * \param[in] sample_key A sample to hold the key fields
 * \param[in] param      Opaque parameter passed from the type-plugin
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
DPSE_ParticipantBuiltinTopicDataTypePlugin_deserialize_key(
                                    struct CDR_Stream_t *stream,
                                    const UserDataKeyHolder_t sample_key,
                                    void *param)
{
    UNUSED_ARG(stream);
    UNUSED_ARG(sample_key);
    UNUSED_ARG(param);

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Generate the keyhash from a sample
 *
 * \details
 * Implementation of the  NDDS_Type_Plugin_instance_to_keyhash method
 *
 * \param[in]  plugin   The type-plugin
 * \param[in]  stream   A stream to serialize data to
 * \param[out] key_hash The resulting key-hash
 * \param[in]  instance A sample with all the key field filled in
 * \param[in]  param    Opaque parameter passed from the type-plugin
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_PRIVATE RTI_BOOL
DPSE_ParticipantBuiltinTopicDataTypePlugin_instance_to_keyhash(
                                    struct NDDS_Type_Plugin *plugin,
                                    struct CDR_Stream_t *stream,
                                    DDS_KeyHash_t *key_hash,
                                    const void *instance,
                                    void *param)
{
    struct DDS_ParticipantBuiltinTopicData *topic_data =
        (struct DDS_ParticipantBuiltinTopicData *)instance;
    DDS_InstanceHandle_t ih;
    UNUSED_ARG(plugin);
    UNUSED_ARG(stream);
    UNUSED_ARG(param);

    if (topic_data == NULL)
    {
        return DDS_BOOLEAN_FALSE;
    }

    DDS_InstanceHandle_from_rtps(&ih,(struct RTPS_Guid*)&topic_data->key);
    OSAPI_Memory_copy(key_hash->value, ih.octet, RTPS_KEY_HASH_MAX_LENGTH);

    return DDS_BOOLEAN_TRUE;

}


MUST_CHECK_RETURN RTI_PRIVATE  RTI_UINT32
DPSE_ParticipantBuiltinTopicDataTypePlugin_get_serialized_key_size(
        struct NDDS_Type_Plugin *plugin,
        RTI_UINT32 current_alignment,
        void *param)
{
    UNUSED_ARG(plugin);
    UNUSED_ARG(current_alignment);
    UNUSED_ARG(param);

    return RTPS_KEY_HASH_MAX_LENGTH;
}

/*ci
 * \brief Implementation of the participant data type-plugin interface
 */
LINK_SECTION_DATA_SDRAM
RTI_PRIVATE struct NDDS_Type_Plugin DPSE_fv_ParticipantBuiltinTopicDataTypePlugin =
{
    {0,0},
    NULL,
    NULL,
    NDDS_TYPEPLUGIN_GUID_KEY,
    DPSE_ParticipantBuiltinTopicDataTypePlugin_serialize,
    DPSE_ParticipantBuiltinTopicDataTypePlugin_deserialize,
    DPSE_ParticipantBuiltinTopicDataTypePlugin_get_serialized_sample_max_size,
    DPSE_ParticipantBuiltinTopicDataTypePlugin_serialize_key,
    DPSE_ParticipantBuiltinTopicDataTypePlugin_deserialize_key,
    DPSE_ParticipantBuiltinTopicDataTypePlugin_get_serialized_key_size,
    DPSE_ParticipantBuiltinTopicDataTypePlugin_create_sample,
#ifndef RTI_CERT
    DPSE_ParticipantBuiltinTopicDataTypePlugin_delete_sample,
#else
    NULL,
#endif
    DPSE_ParticipantBuiltinTopicDataTypePlugin_copy_sample,
    DPSE_ParticipantBuiltinTopicDataTypePlugin_get_key_kind,
    DPSE_ParticipantBuiltinTopicDataTypePlugin_instance_to_keyhash,
    NULL,
    NULL,
    NULL,
    NULL
};


/*ci
 * \brief Get a pointer to the participant data type-plugin interface
 *
 * \details
 * This function is used to retrieve the type-plugin interface for
 * the ParticipantBuiltinTopicData and is registered with the
 * participant after the participant is enabled.
 *
 * \return A pointer to the type-plugin interface
 */
struct NDDS_Type_Plugin*
DPSE_ParticipantBuiltinTopicDataTypePlugin_get(void)
{
    return &DPSE_fv_ParticipantBuiltinTopicDataTypePlugin;
}

/*ci @} */
