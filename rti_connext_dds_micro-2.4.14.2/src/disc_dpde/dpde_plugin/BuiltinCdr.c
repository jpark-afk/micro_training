/*
 * FILE: BuiltinCdr.c - DPDE CDR API
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
 * 23feb2015,eh MICRO-1075: remove and replace macros
 * 23feb2015,eh MICRO-1081: fix function/var names to conform to coding std
 * 07may2014,eh MICRO-313/VerocelPR 1439: CDR_Stream_Align() returns void
 * 25jul2011,tk Written.
 */
/*ce
 * \file
 * \brief DPDE CDR API
 */
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef disc_dpde_log_h
#include "disc_dpde/disc_dpde_log.h"
#endif
#ifndef dds_c_discovery_plugin_h
#include "dds_c/dds_c_discovery_plugin.h"
#endif
#ifndef disc_dpde_discovery_plugin_h
#include "disc_dpde/disc_dpde_discovery_plugin.h"
#endif
#include "DiscoveryPlugin.h"
#include "BuiltinCdr.h"

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
DDS_CdrQosPolicy_get_parameter_length(DDS_UnsignedShort *length,
                                      DDS_UnsignedLong begin, DDS_UnsignedLong end)
{
    DDS_UnsignedLong diff = end - begin;

    if (diff > USHRT_MAX)
    {
        return RTI_FALSE;
    }

    *length = (DDS_UnsignedShort)diff;

    return RTI_TRUE;
}

RTI_PRIVATE DDS_Boolean
DDS_Cdr_insertParameterLength(struct CDR_Stream_t *stream,
                              RTI_UINT32 parameterBeginPosition,
                              DDS_UnsignedShort parameterLength,
                              RTI_BOOL parameterSuccess)
{
    RTI_UINT32 currentPosition;
    RTI_BOOL ok = RTI_TRUE;

    if (parameterSuccess)
    {
        currentPosition = CDR_Stream_get_current_position_offset(stream);

        ok = CDR_Stream_set_current_position_offset(stream,
                parameterBeginPosition + CDR_SHORT_SIZE) &&
               CDR_Stream_serialize_unsigned_short(stream, &parameterLength);

        if (ok)
        {
            if (!CDR_Stream_set_current_position_offset(stream, currentPosition))
            {
                DPDE_LOG_CDR_SET_OFFSET(OSAPI_LOGKIND_ERROR,currentPosition)
                return DDS_BOOLEAN_FALSE;
            }
            return DDS_BOOLEAN_TRUE;
        }
    }

    if (!CDR_Stream_set_current_position_offset(stream, parameterBeginPosition))
    {
        DPDE_LOG_CDR_SET_OFFSET(OSAPI_LOGKIND_ERROR,parameterBeginPosition)
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

RTI_BOOL
DDS_Cdr_serializeFourByteParameter(struct CDR_Stream_t *stream,
                                   const void *in,
                                   DDS_UnsignedShort parameterId)
{
    RTI_BOOL ok = RTI_TRUE;
    DDS_UnsignedShort parameterLength;
    RTI_UINT32 parameterBeginPosition, valueBeginPosition, valueEndPosition;

    parameterBeginPosition = CDR_Stream_get_current_position_offset(stream);

    /* parameter Id */
    ok = ok && CDR_Stream_serialize_unsigned_short(stream, &parameterId);

    /* skip parameter length */
    ok = ok && CDR_Stream_increment_current_position(stream, CDR_SHORT_SIZE);

    /* parameter value parameter has to be 4-byte aligned */
    valueBeginPosition = CDR_Stream_get_current_position_offset(stream);

    if (ok)
    {
        if (!CDR_Stream_serialize_long(stream, (RTI_INT32 *) in))
        {
            return RTI_FALSE;
        }
    }

    CDR_Stream_align(stream, CDR_DEFAULT_PARAMETER_ALIGNMENT);
    valueEndPosition = CDR_Stream_get_current_position_offset(stream);

    if (!DDS_CdrQosPolicy_get_parameter_length(&parameterLength,
                                               valueBeginPosition,
                                               valueEndPosition))
    {
        return RTI_FALSE;
    }

    return DDS_Cdr_insertParameterLength(stream, parameterBeginPosition,
            parameterLength, ok);
}

RTI_PRIVATE RTI_BOOL
DDS_Cdr_serializeFourOctets(struct CDR_Stream_t *stream,
                            const char *in,
                            void *param)
{
    RTI_BOOL ok = RTI_TRUE;
    UNUSED_ARG(param);

    CDR_Stream_serialize_1_byte(stream, (RTI_INT8*)&in[0]);
    CDR_Stream_serialize_1_byte(stream, (RTI_INT8*)&in[1]);
    CDR_Stream_serialize_1_byte(stream, (RTI_INT8*)&in[2]);
    CDR_Stream_serialize_1_byte(stream, (RTI_INT8*)&in[3]);

    return ok;
}


RTI_BOOL
DDS_CdrQosPolicy_serializeNonPrimitiveParameter(struct CDR_Stream_t * stream,
        const void *in,
        CDR_Stream_SerializeFunction
        serializeFunction,
        DDS_UnsignedShort parameterId,
        RTI_BOOL serializeEncapsulation,
        RTI_BOOL serializeSample)
{
    RTI_BOOL ok = RTI_TRUE;
    DDS_UnsignedShort parameterLength;
    RTI_UINT32 parameterBeginPosition, valueBeginPosition, valueEndPosition;
    UNUSED_ARG(serializeEncapsulation);
    UNUSED_ARG(serializeSample);

    CDR_Stream_align(stream, CDR_DEFAULT_PARAMETER_ALIGNMENT);
    parameterBeginPosition = CDR_Stream_get_current_position_offset(stream);

    /* parameter Id */
    ok = ok && CDR_Stream_serialize_unsigned_short(stream, &parameterId);

    /* skip parameter length */
    ok = ok && CDR_Stream_increment_current_position(stream, CDR_SHORT_SIZE);

    /* parameter value parameter has to be 4-byte aligned */
    valueBeginPosition = CDR_Stream_get_current_position_offset(stream);
    ok = ok && serializeFunction(stream, in, NULL);
    CDR_Stream_align(stream, CDR_DEFAULT_PARAMETER_ALIGNMENT);
    valueEndPosition = CDR_Stream_get_current_position_offset(stream);

    if (!DDS_CdrQosPolicy_get_parameter_length(&parameterLength,
                                          valueBeginPosition,valueEndPosition))
    {
        return RTI_FALSE;
    }

    return DDS_Cdr_insertParameterLength(stream, parameterBeginPosition,
            parameterLength, ok);
}

DDS_UnsignedLong
DDS_CdrQosPolicy_get_parameter_header_max_size_serialized(DDS_UnsignedLong size)
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

DDS_UnsignedLong
DDS_CdrQosPolicy_get_header_max_size_serialized(DDS_UnsignedLong size)
{
    DDS_UnsignedLong origSize = size;

    /* align to 4 bytes boundary */
    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    size += CDR_Stream_get_encapsulation_size(size);

    return size - origSize;

}

RTI_PRIVATE void
DDS_Cdr_ipv6_from_ipv4(struct DDS_Locator_t *ipv6Locator,
        struct DDS_LocatorUdpv4_t *ipv4Locator)
{
    RTI_UINT32 tmp;

    OSAPI_Memory_copy(&tmp,&ipv4Locator->address,RTI_SIZEOF(tmp));

    OSAPI_Memory_zero(ipv6Locator->address, 12);

    /* This does not need to be byte swapped because we always store the locator
     * in network order.
     */
    ipv6Locator->address[12] = (unsigned char)(tmp >> 24);
    ipv6Locator->address[13] = (unsigned char)((tmp & 0x00FF0000) >> 16);
    ipv6Locator->address[14] = (unsigned char)((tmp & 0x0000FF00) >> 8);
    ipv6Locator->address[15] = (unsigned char)((tmp & 0x000000FF));
}

RTI_PRIVATE RTI_BOOL
DDS_Cdr_serializeString(struct CDR_Stream_t *stream,const char *in,void *param)
{
    UNUSED_ARG(param);

    return CDR_Stream_serialize_string(stream, in,
                                       RTPS_PATHNAME_LEN_MAX + 1);
}

/*i \ingroup DDS_PublicationBuiltinTopicData_TypePluginModule
  Prototype for deserializing parameter value of the given parameterId.

  @param ok \b Out. Indicate if parameter value is correctly deserialized.
  @param parameter \b Out. Pointer to the parameter to which value should be
  deserialized.
  @param stream \b InOut. Stream for deserialization.
  @param parameterId \b In. Parameter ID of the value to be deserialized.
  @param parameterLength \b In. Length of parameter value.
  @param deserializeOption \b In. deserialize option obtained from
  reader property.

  @return Return RTI_TRUE if there is a match in parameterId, and ok is set to
  RTI_TRUE if deserialization of the parameter value is successful.
  Otherwise, return RTI_FALSE.

  Returning RTI_TRUE does not mean that the parameter value is
  correctly deserialized.
 */
RTI_BOOL
DDS_Cdr_deserializeParameterSequence(void *parameter,
        struct CDR_Stream_t *stream,
        DDS_Cdr_SetDefaultParameterValuesFunction
        setDefaultParameterValuesFnc,
        DDS_Cdr_DeserializeParameterValueFunction
        deserializeParameterValueFnc, void *param)
{
    RTPS_ParameterId parameterId, parameterLength;
    RTI_BOOL ok = RTI_TRUE;
    DDS_UnsignedLong origPosition;

    /* set parameter to default value */
    setDefaultParameterValuesFnc(parameter);

    /* parameter id and length */
    ok = ok && CDR_Stream_deserialize_unsigned_short(stream, &parameterId);
    ok = ok && CDR_Stream_deserialize_unsigned_short(stream, &parameterLength);

    while (ok && parameterId != RTPS_PID_SENTINEL)
    {
        origPosition = CDR_Stream_get_current_position_offset(stream);

        /* logical reset of stream per parameter value */
        deserializeParameterValueFnc(&ok, parameter,
                stream, parameterId, parameterLength,param);

        /* skip to next parameter */
        ok = ok && CDR_Stream_set_current_position_offset(stream,
                origPosition +
                parameterLength);

        /* parameter id and length */
        ok = ok && CDR_Stream_deserialize_unsigned_short(stream, &parameterId);
        ok = ok && CDR_Stream_deserialize_unsigned_short(stream, &parameterLength);
    }

    return ok;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/*----------------------------------------------------------------------------*/

RTI_BOOL
DDS_CdrQosPolicy_serializeEntityNameQosPolicy(
        struct CDR_Stream_t *stream,
        const struct DDS_EntityNameQosPolicy *entityName,
        void *param)
{
    UNUSED_ARG(param);

    if (entityName != NULL)
    {
        if (!DDS_CdrQosPolicy_serializeNonPrimitiveParameter
                (stream, entityName->name,
                        (CDR_Stream_SerializeFunction) DDS_Cdr_serializeString,
                        DISC_RTPS_PID_ENTITY_NAME, RTI_FALSE, RTI_TRUE))
        {
            DPDE_LOG_SERIALIZE_ENTITY_NAME(OSAPI_LOGKIND_ERROR)
            return RTI_FALSE;
        }
    }

    return RTI_TRUE;
}

RTI_BOOL
DDS_CdrQosPolicy_deserializeEntityNameQosPolicy(
        struct CDR_Stream_t *stream,
        struct DDS_EntityNameQosPolicy *entityName,
        void *param)
{
    UNUSED_ARG(param);
    if (!CDR_Stream_deserialize_string(stream, entityName->name,
            RTPS_PATHNAME_LEN_MAX))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}


RTI_UINT32
DDS_CdrQosPolicy_getEntityNameQosPolicyMaxSizeSerialized(RTI_UINT32 size)
{
    RTI_UINT32 origSize = size;

    size += DDS_CdrQosPolicy_get_parameter_header_max_size_serialized(size);
    size += CDR_get_max_size_serialized_string(size, RTPS_PATHNAME_LEN_MAX + 1);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return (size - origSize);
}

/*----------------------------------------------------------------------------*/

RTI_BOOL
DDS_CdrQosPolicy_serializeTopicName(struct CDR_Stream_t *stream,
        const char *topic_name,
        void * param)
{
    RTI_BOOL retval = RTI_FALSE;
    UNUSED_ARG(param);

    if (!DDS_CdrQosPolicy_serializeNonPrimitiveParameter(
            stream, topic_name,
            (CDR_Stream_SerializeFunction) DDS_Cdr_serializeString,
            RTPS_PID_TOPIC_NAME, RTI_FALSE, RTI_TRUE))
    {
        DPDE_LOG_SERIALIZE_TOPIC_NAME(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    retval = RTI_TRUE;

    done:
    return retval;
}

RTI_BOOL
DDS_CdrQosPolicy_deserializeTopicName(struct CDR_Stream_t *stream,
        char *topic_name,
        void *param)
{
    UNUSED_ARG(param);
    return CDR_Stream_deserialize_string(stream, topic_name,
            RTPS_PATHNAME_LEN_MAX);
}

RTI_UINT32
DDS_CdrQosPolicy_getTopicNameMaxSerializedSize(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += CDR_get_max_size_serialized_string(size, RTPS_PATHNAME_LEN_MAX + 1);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return size - orig_size;
}

/*----------------------------------------------------------------------------*/

RTI_BOOL
DDS_CdrQosPolicy_serializeTypeName(struct CDR_Stream_t *stream,
        const char *type_name,
        void *param)
{
    RTI_BOOL retval = RTI_FALSE;
    UNUSED_ARG(param);

    if (!DDS_CdrQosPolicy_serializeNonPrimitiveParameter
            (stream, type_name,
                    (CDR_Stream_SerializeFunction) DDS_Cdr_serializeString,
                    RTPS_PID_TYPE_NAME, RTI_FALSE, RTI_TRUE))
    {
        DPDE_LOG_SERIALIZE_TYPE_NAME(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    retval = RTI_TRUE;

    done:
    return retval;
}

RTI_BOOL
DDS_CdrQosPolicy_deserializeTypeName(struct CDR_Stream_t *stream,
        char *type_name,
        void *param)
{
    UNUSED_ARG(param);
    return CDR_Stream_deserialize_string(stream, type_name,
            RTPS_PATHNAME_LEN_MAX);
}

RTI_UINT32
DDS_CdrQosPolicy_getTypeNameMaxSerializedSize(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += CDR_get_max_size_serialized_string(size, RTPS_PATHNAME_LEN_MAX + 1);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return size - orig_size;
}

RTI_PRIVATE RTI_BOOL
DDS_CdrQosPolicy_serializeParameter(struct CDR_Stream_t *stream,
                                    DDS_UnsignedShort param_id,
                                    DDS_UnsignedShort param_length)
{
    RTI_BOOL ok = RTI_TRUE;

    ok = CDR_Stream_serialize_unsigned_short(stream, &param_id);
    ok = ok && CDR_Stream_serialize_unsigned_short(stream, &param_length);

    return ok;
}

/*----------------------------------------------------------------------------*/

RTI_BOOL
DDS_CdrQosPolicy_serializeDeadline(struct CDR_Stream_t *stream,
        const struct DDS_DeadlineQosPolicy *deadline,
        void *param)
{
    RTI_BOOL ok = RTI_TRUE;
    struct OSAPI_NtpTime serializeTime;
    UNUSED_ARG(param);

    DDS_Duration_to_ntp_time(&deadline->period,&serializeTime);

    ok = DDS_CdrQosPolicy_serializeParameter(stream,RTPS_PID_DEADLINE,
                (DDS_UnsignedShort)(2 * CDR_get_max_size_serialized_long(0)));

    ok = ok && CDR_Stream_serialize_long(stream,
                                        (RTI_INT32*)&serializeTime.sec);

    ok = ok && CDR_Stream_serialize_unsigned_long(stream,
                                        (RTI_UINT32*)&serializeTime.frac);

    return ok;
}

RTI_BOOL
DDS_CdrQosPolicy_deserializeDeadline(struct CDR_Stream_t *stream,
        struct DDS_DeadlineQosPolicy *deadline,
        void *param)
{
    struct OSAPI_NtpTime time;
    UNUSED_ARG(param);

    if (!RTPS_deserialize_ntp_time(stream, &time, NULL))
    {
        return RTI_FALSE;
    }

    DDS_Duration_from_ntp_time(&deadline->period,&time);

    return RTI_TRUE;
}

RTI_UINT32
DDS_CdrQosPolicy_getDeadlineMaxSerializedSize(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += DDS_CdrQosPolicy_get_parameter_header_max_size_serialized(size);
    size += CDR_get_max_size_serialized_long(0);
    size += CDR_get_max_size_serialized_long(0);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return size - orig_size;
}

/*----------------------------------------------------------------------------*/

RTI_BOOL
DDS_CdrQosPolicy_serializeOwnership(struct CDR_Stream_t *stream,
        const struct DDS_OwnershipQosPolicy
        *ownership,
        void *param)
{
    RTI_BOOL ok = RTI_TRUE;
    UNUSED_ARG(param);

    ok = DDS_CdrQosPolicy_serializeParameter(stream,RTPS_PID_OWNERSHIP,
                    (DDS_UnsignedShort)CDR_get_max_size_serialized_long(0));

#if CDR_VARIABLE_ENUM_ENABLED
    ok = ok && CDR_Stream_serialize_enum_value(stream, ownership->kind);
#else
    ok = ok && CDR_Stream_serialize_unsigned_long(stream,
                                              (RTI_UINT32 *)&ownership->kind);
#endif
    return ok;
}

RTI_BOOL
DDS_CdrQosPolicy_deserializeOwnership(struct CDR_Stream_t *stream,
        struct DDS_OwnershipQosPolicy *ownership,
        void *param)
{
    UNUSED_ARG(param);
#if CDR_VARIABLE_ENUM_ENABLED
    return CDR_Stream_deserialize_enum(
            stream,
            &ownership->kind,
            sizeof(ownership->kind));
#else
    return CDR_Stream_deserialize_unsigned_long(stream,
                                               (RTI_UINT32*)&ownership->kind);
#endif
}

RTI_UINT32
DDS_CdrQosPolicy_getOwnershipMaxSerializedSize(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += DDS_CdrQosPolicy_get_parameter_header_max_size_serialized(size);
    size += CDR_get_max_size_serialized_long(0);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return size - orig_size;
}

/*----------------------------------------------------------------------------*/

RTI_BOOL
DDS_CdrQosPolicy_serializeOwnershipStrength(struct CDR_Stream_t *stream,
        const struct
        DDS_OwnershipStrengthQosPolicy
        *ownership_strength,
        void *param)
{
    RTI_BOOL ok = RTI_TRUE;
    UNUSED_ARG(param);

    ok = DDS_CdrQosPolicy_serializeParameter(stream,RTPS_PID_OWNERSHIP_STRENGTH,
            (DDS_UnsignedShort)CDR_get_max_size_serialized_long(0));

    ok = ok && CDR_Stream_serialize_unsigned_long(stream,
                                    (RTI_UINT32*)&ownership_strength->value);
    return ok;
}

RTI_BOOL
DDS_CdrQosPolicy_deserializeOwnershipStrength(struct CDR_Stream_t *stream,
        struct
        DDS_OwnershipStrengthQosPolicy
        *ownership_strength,
        void *param)
{
    UNUSED_ARG(param);
    return CDR_Stream_deserialize_unsigned_long(stream,
                                   (RTI_UINT32 *)&ownership_strength->value);
}

RTI_UINT32
DDS_CdrQosPolicy_getOwnershipStrengthMaxSerializedSize(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += DDS_CdrQosPolicy_get_parameter_header_max_size_serialized(size);
    size += CDR_get_max_size_serialized_long(0);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return size - orig_size;
}

/*----------------------------------------------------------------------------*/


RTI_BOOL
DDS_CdrQosPolicy_serializeReliability(struct CDR_Stream_t *stream,
        const struct DDS_ReliabilityQosPolicy
        *reliability,
        void *param)
{
    RTI_BOOL ok = RTI_TRUE;
    struct OSAPI_NtpTime serializeTime;
    UNUSED_ARG(param);

    DDS_Duration_to_ntp_time(&reliability->max_blocking_time,&serializeTime);

    ok = DDS_CdrQosPolicy_serializeParameter(stream,RTPS_PID_RELIABILITY,
            (DDS_UnsignedShort)(3 * CDR_get_max_size_serialized_long(0)));

#if CDR_VARIABLE_ENUM_ENABLED
    ok = ok && CDR_Stream_serialize_enum_value(stream, reliability->kind);
#else
    ok = ok && CDR_Stream_serialize_unsigned_long(stream,
                                (RTI_UINT32*)&reliability->kind);
#endif
    ok = ok && CDR_Stream_serialize_long(stream,
                        (RTI_INT32*)&serializeTime.sec);

    ok = ok && CDR_Stream_serialize_unsigned_long(stream,
                         (RTI_UINT32*)&serializeTime.frac);

    return ok;
}

RTI_BOOL
DDS_CdrQosPolicy_deserializeReliability(struct CDR_Stream_t *stream,
        struct DDS_ReliabilityQosPolicy
        *reliability,
        void *param)
{
    struct OSAPI_NtpTime time;
    UNUSED_ARG(param);

#if CDR_VARIABLE_ENUM_ENABLED
    if (!CDR_Stream_deserialize_enum(
            stream,
            &reliability->kind,
            sizeof(reliability->kind)))
    {
        return RTI_FALSE;
    }
#else
    if (!CDR_Stream_deserialize_unsigned_long(stream,
                                            (RTI_UINT32*)&reliability->kind))
    {
        return RTI_FALSE;
    }
#endif

    if (!RTPS_deserialize_ntp_time(stream, &time, NULL))
    {
        return RTI_FALSE;
    }

    DDS_Duration_from_ntp_time(&reliability->max_blocking_time,&time);

    return RTI_TRUE;
}

RTI_UINT32
DDS_CdrQosPolicy_getReliabilityMaxSerializedSize(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += DDS_CdrQosPolicy_get_parameter_header_max_size_serialized(size);
    size += 3 * CDR_get_max_size_serialized_long(0);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return size - orig_size;
}

/*----------------------------------------------------------------------------*/

RTI_BOOL
DDS_CdrQosPolicy_serializeLiveliness(struct CDR_Stream_t *stream,
        const struct DDS_LivelinessQosPolicy *liveliness,
        void *param)
{
    RTI_BOOL ok = RTI_TRUE;
    struct OSAPI_NtpTime serializeTime;
    UNUSED_ARG(param);

    ok = DDS_CdrQosPolicy_serializeParameter(stream,RTPS_PID_LIVELINESS,
            (DDS_UnsignedShort)(3 * CDR_get_max_size_serialized_long(0)));

    DDS_Duration_to_ntp_time(&liveliness->lease_duration,&serializeTime);

#if CDR_VARIABLE_ENUM_ENABLED
    ok = ok && CDR_Stream_serialize_enum_value(stream, liveliness->kind);
#else
    ok = ok && CDR_Stream_serialize_unsigned_long(stream,
                                            (RTI_UINT32 *)&liveliness->kind);
#endif
    ok = ok && CDR_Stream_serialize_unsigned_long(stream,
                             (RTI_UINT32 *)&serializeTime.sec);

    ok = ok && CDR_Stream_serialize_unsigned_long(stream,
                            (RTI_UINT32 *)&serializeTime.frac);

    return ok;
}

RTI_BOOL
DDS_CdrQosPolicy_deserializeLiveliness(struct CDR_Stream_t *stream,
        struct DDS_LivelinessQosPolicy *liveliness,
        void *param)
{
    struct OSAPI_NtpTime serializeTime;
    UNUSED_ARG(param);

#if CDR_VARIABLE_ENUM_ENABLED
    if (!CDR_Stream_deserialize_enum(
            stream,
            &liveliness->kind,
            sizeof(liveliness->kind)))
    {
        return RTI_FALSE;
    }
#else
    if (!CDR_Stream_deserialize_unsigned_long(stream,
                            (RTI_UINT32 *)&liveliness->kind))
    {
        return RTI_FALSE;
    }
#endif

    if (!RTPS_deserialize_ntp_time(stream, &serializeTime, NULL))
    {
        return RTI_FALSE;
    }

    DDS_Duration_from_ntp_time(&liveliness->lease_duration,&serializeTime);

    return RTI_TRUE;
}

RTI_UINT32
DDS_CdrQosPolicy_getLivelinessMaxSerializedSize(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += DDS_CdrQosPolicy_get_parameter_header_max_size_serialized(size);
    size += 3 * CDR_get_max_size_serialized_long(0);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return (size - orig_size);
}

/*----------------------------------------------------------------------------*/

RTI_BOOL
DDS_CdrQosPolicy_serializeKey(struct CDR_Stream_t *stream,
        const struct DDS_BuiltinTopicKey_t *key,
        DDS_UnsignedShort id)
{
    struct RTPS_Guid guid;

    guid.prefix.host_id = key->value[0];
    guid.prefix.app_id  = key->value[1];
    guid.prefix.instance_id  = key->value[2];
    guid.object_id = key->value[3];

    if (!DDS_CdrQosPolicy_serializeNonPrimitiveParameter
            (stream, &guid, (CDR_Stream_SerializeFunction) RTPS_Guid_serialize,
                    id, RTI_FALSE, RTI_TRUE))
    {
        DPDE_LOG_SERIALIZE_GUID(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
DDS_CdrQosPolicy_deserializeKey(struct CDR_Stream_t *stream,
        struct DDS_BuiltinTopicKey_t *key,
        void *param)
{
    struct RTPS_Guid guid;
    UNUSED_ARG(param);

    if (!RTPS_Guid_deserialize(stream, &guid, NULL))
    {
        return RTI_FALSE;
    }

    key->value[0] = guid.prefix.host_id;
    key->value[1] = guid.prefix.app_id ;
    key->value[2] = guid.prefix.instance_id ;
    key->value[3] = guid.object_id;

    return RTI_TRUE;
}

RTI_UINT32
DDS_CdrQosPolicy_getKeyMaxSerializedSize(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += DDS_CdrQosPolicy_get_parameter_header_max_size_serialized(size);
    size += (RTI_UINT32)sizeof(struct DDS_BuiltinTopicKey_t);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return size - orig_size;
}

/*----------------------------------------------------------------------------*/

RTI_BOOL
DDS_CdrQosPolicy_serializeLocator(struct CDR_Stream_t *stream,
        const struct DDS_LocatorSeq *locator,
        DDS_UnsignedShort id)
{
    struct DDS_Locator loc;
    RTI_INT32 i, l;

    l = DDS_LocatorSeq_get_length(locator);

    for (i = 0; i < l; ++i)
    {

        loc = *DDS_LocatorSeq_get_reference(locator, i);

        if (NETIO_Address_get_kind((struct NETIO_Address*)&loc) == RTPS_LOCATOR_KIND_UDPv4)
        {

            struct DDS_LocatorUdpv4_t *udp4_loc = NULL;
            OSAPI_Compiler_reinterpret_cast(udp4_loc,&loc,ptr_ptr);
            DDS_Cdr_ipv6_from_ipv4(&loc, udp4_loc);
        }

        if (!DDS_CdrQosPolicy_serializeNonPrimitiveParameter(stream, &loc,
                (CDR_Stream_SerializeFunction)
                RTPS_serialize_ipv6_locator,
                id, RTI_FALSE,
                RTI_TRUE))
        {
            DPDE_LOG_SERIALIZE_DEFAULT_UNICAST(OSAPI_LOGKIND_ERROR)
            return RTI_FALSE;
        }
    }

    return RTI_TRUE;
}

RTI_BOOL
DDS_CdrQosPolicy_deserializeLocator(struct CDR_Stream_t *stream,
                                    struct DDS_LocatorSeq *locator,
                                    DDS_UnsignedShort kind,
                                    void *param)
{
    struct DDS_Locator *loc;
    RTI_INT32 l, ml;
    struct DPDE_DiscoveryPlugin *plugin = (struct DPDE_DiscoveryPlugin *)param;
    struct RTPS_Locator_t discard_loc;
#if !OSAPI_ENABLE_LOG
    UNUSED_ARG(kind);
#endif

    l = DDS_LocatorSeq_get_length(locator);
    ml = DDS_LocatorSeq_get_maximum(locator);

    if (l == ml)
    {
        /* We need to de-serialize the locator to properly discard it.
         * NOTE: We could also increment the position, but then we'll not
         *       know if the locator was valid.
         */
        if (!RTPS_deserialize_ipv6_locator(stream, &discard_loc, NULL))
        {
            /* De-serialization error, return error to drop the sample. */
            DPDE_LOG_DESERIALIZE_LOCATOR(OSAPI_LOGKIND_ERROR,kind)
            return RTI_FALSE;
        }
        /* Not an error, but log that the locator sequence is full */
        DPDE_LOG_LOCATORS_FULL(OSAPI_LOGKIND_WARNING,kind)
        return RTI_TRUE;
    }

    if (!DDS_LocatorSeq_set_length(locator, l + 1))
    {
        return RTI_FALSE;
    }

    loc = DDS_LocatorSeq_get_reference(locator, l);
    if (loc == NULL)
    {
        return RTI_FALSE;
    }

    NETIO_Address_init((struct NETIO_Address*)loc, 0);

    if (!RTPS_deserialize_ipv6_locator(stream, loc, NULL))
    {
        /* De-serialization error, return error to drop the sample.
         */
        DPDE_LOG_DESERIALIZE_LOCATOR(OSAPI_LOGKIND_ERROR,kind)
        return RTI_FALSE;
    }

    /* Check if this is a supported locator, if not drop it altogether
     */
    if (!DDS_DomainParticipant_locator_is_supported(plugin->participant,loc))
    {
        /* Discard the locator */
        DPDE_LOG_UNSUPPORTED_LOCATOR(OSAPI_LOGKIND_WARNING,loc->kind)
        DDS_LocatorSeq_set_length(locator, l);
    }

    return RTI_TRUE;
}

RTI_UINT32
DDS_CdrQosPolicy_getLocatorMaxSerializedSize(struct DDS_LocatorSeq *locator,
                                             RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;
    RTI_UINT32 el_size;

    el_size = DDS_CdrQosPolicy_get_parameter_header_max_size_serialized(0) +
                        RTPS_get_ipv6_locator_max_size_serialized(0);

    el_size += CDR_align_upwards(el_size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    size += (RTI_UINT32)DDS_LocatorSeq_get_maximum(locator) * el_size;

    return (size - orig_size);
}

/*----------------------------------------------------------------------------*/

RTI_BOOL
DDS_CdrQosPolicy_serializeProtocolVersion(struct CDR_Stream_t *stream,
        const struct DDS_ProtocolVersion *protocol,
        void *param)
{
    RTI_UINT16 protocolVersion;
    UNUSED_ARG(param);

    protocolVersion = (RTI_UINT16)(((RTI_UINT16)protocol->major << 8) | protocol->minor);

    if (!DDS_CdrQosPolicy_serializeNonPrimitiveParameter
            (stream, &protocolVersion,
                    (CDR_Stream_SerializeFunction) RTPS_serialize_2_octets,
                    RTPS_PID_PROTOCOL_VERSION, RTI_FALSE, RTI_TRUE))
    {
        DPDE_LOG_SERIALIZE_PROTOCOL_VERSION(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
DDS_CdrQosPolicy_deserializeProtocolVersion(struct CDR_Stream_t *stream,
        struct DDS_ProtocolVersion *protocol,
        void *param)
{
    RTI_UINT16 protocolVersion;
    UNUSED_ARG(param);

    if (!RTPS_deserialize_2_octets(stream, &protocolVersion, NULL))
    {
        DPDE_LOG_DESERIALIZE_PROTOCOL_VERSION(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    protocol->major = (DDS_Octet)(protocolVersion >> 8);
    protocol->minor = (DDS_Octet)(protocolVersion & 0x00ff);

    return RTI_TRUE;
}

RTI_UINT32
DDS_CdrQosPolicy_getProtocolVersionMaxSerializedSize(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += DDS_CdrQosPolicy_get_parameter_header_max_size_serialized(size);

    /* Add 4 total bytes: 2 for fieldsd, 2 for alignment */
    size += 4;

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return (size - orig_size);
}

/*----------------------------------------------------------------------------*/

RTI_BOOL
DDS_CdrQosPolicy_serializeVendorId(struct CDR_Stream_t *stream,
        const struct DDS_VendorId *vendor,
        void *param)
{
    RTI_UINT16 vendorId;
    UNUSED_ARG(param);

    vendorId = (RTI_UINT16)(((RTI_UINT16)vendor->vendorId[0] << 8) |
                             vendor->vendorId[1]);

    if (!DDS_CdrQosPolicy_serializeNonPrimitiveParameter
            (stream, &vendorId,
                    (CDR_Stream_SerializeFunction) RTPS_serialize_2_octets,
                    RTPS_PID_VENDOR_ID, RTI_FALSE, RTI_TRUE))
    {
        DPDE_LOG_SERIALIZE_VENDOR_ID(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
DDS_CdrQosPolicy_deserializeVendorId(struct CDR_Stream_t *stream,
        struct DDS_VendorId *vendor,
        void *param)
{
    RTPS_VendorId vendorId;
    UNUSED_ARG(param);

    if (!RTPS_deserialize_2_octets(stream, &vendorId, NULL))
    {
        DPDE_LOG_DESERIALIZE_VENDOR_ID(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    vendor->vendorId[0] = RTPS_VendorId_get_major(&vendorId);
    vendor->vendorId[1] = RTPS_VendorId_get_minor(&vendorId);

    return RTI_TRUE;
}

RTI_UINT32
DDS_CdrQosPolicy_getVendorIdMaxSerializedSize(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += DDS_CdrQosPolicy_get_parameter_header_max_size_serialized(size);
    size += RTPS_get_2_octets_max_size_serialized(size);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return (size - orig_size);
}

/*----------------------------------------------------------------------------*/

RTI_BOOL
DDS_CdrQosPolicy_serializeDurability(struct CDR_Stream_t *stream,
        const struct DDS_DurabilityQosPolicy *durability,
        void *param)
{
    RTI_BOOL ok = RTI_TRUE;
    UNUSED_ARG(param);

    ok = DDS_CdrQosPolicy_serializeParameter(stream,RTPS_PID_DURABILITY,
                        (DDS_UnsignedShort)CDR_get_max_size_serialized_long(0));

#if CDR_VARIABLE_ENUM_ENABLED
    ok = ok && CDR_Stream_serialize_enum_value(stream, durability->kind);
#else
    ok = ok && CDR_Stream_serialize_unsigned_long(stream,
                                        (RTI_UINT32 *)&durability->kind);
#endif

    return ok;
}

RTI_BOOL
DDS_CdrQosPolicy_deserializeDurability(struct CDR_Stream_t *stream,
        struct DDS_DurabilityQosPolicy *durability,
        void *param)
{
    RTI_BOOL ok = RTI_TRUE;
    UNUSED_ARG(param);

#if CDR_VARIABLE_ENUM_ENABLED
    ok = ok && CDR_Stream_deserialize_enum(
                    stream,
                    &durability->kind,
                    sizeof(durability->kind));
#else
    ok = ok && CDR_Stream_deserialize_unsigned_long(stream,
                                      (RTI_UINT32 *) & durability->kind);
#endif

    return ok;
}

RTI_UINT32
DDS_CdrQosPolicy_getDurabilityMaxSerializedSize(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += DDS_CdrQosPolicy_get_parameter_header_max_size_serialized(size);
    size += CDR_get_max_size_serialized_long(0);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return (size - orig_size);
}

/*----------------------------------------------------------------------------*/

RTI_BOOL
DDS_CdrQosPolicy_serializeDestinationOrder(struct CDR_Stream_t *stream,
        const struct DDS_DestinationOrderQosPolicy *destination_order,
        void *param)
{
    RTI_BOOL ok = RTI_TRUE;
    UNUSED_ARG(param);

    ok = DDS_CdrQosPolicy_serializeParameter(stream,RTPS_PID_DESTINATION_ORDER,
                        (DDS_UnsignedShort)CDR_get_max_size_serialized_long(0));

#if CDR_VARIABLE_ENUM_ENABLED
    ok = ok && CDR_Stream_serialize_enum_value(stream, destination_order->kind);
#else
    ok = ok && CDR_Stream_serialize_unsigned_long(stream,
                    (RTI_UINT32 *)&destination_order->kind);
#endif
    return ok;
}

RTI_BOOL
DDS_CdrQosPolicy_deserializeDestinationOrder(struct CDR_Stream_t *stream,
        struct DDS_DestinationOrderQosPolicy *destination_order,
        void *param)
{
    UNUSED_ARG(param);

#if CDR_VARIABLE_ENUM_ENABLED
    return  CDR_Stream_deserialize_enum(
                stream,
                &destination_order->kind,
                sizeof(destination_order->kind));
#else
    return  CDR_Stream_deserialize_unsigned_long(stream,
                                (RTI_UINT32 *) & destination_order->kind);
#endif

}

RTI_UINT32
DDS_CdrQosPolicy_getDestinationOrderMaxSerializedSize(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += DDS_CdrQosPolicy_get_parameter_header_max_size_serialized(size);
    size += CDR_get_max_size_serialized_long(0);
    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return (size - orig_size);
}

/*----------------------------------------------------------------------------*/

RTI_PRIVATE RTI_BOOL
DDS_Cdr_serializePresentation(struct CDR_Stream_t *stream,
        const struct DDS_PresentationQosPolicy *presentation,
        void *param)
{
    RTI_BOOL ok;
    UNUSED_ARG(param);

#if CDR_VARIABLE_ENUM_ENABLED
    ok = CDR_Stream_serialize_enum_value(stream, presentation->access_scope);
#else
    ok = CDR_Stream_serialize_unsigned_long(stream,
                                    (RTI_UINT32*)&presentation->access_scope);
#endif
    ok = ok && CDR_Stream_serialize_boolean(stream, 
                                            &presentation->coherent_access);

    ok = ok && CDR_Stream_serialize_boolean(stream, 
                                            &presentation->ordered_access);

    return ok;
}

RTI_BOOL
DDS_CdrQosPolicy_serializePresentation(struct CDR_Stream_t *stream,
        const struct DDS_PresentationQosPolicy *presentation,
        void *param)
{
    UNUSED_ARG(presentation);
    UNUSED_ARG(param);

    if (!DDS_CdrQosPolicy_serializeNonPrimitiveParameter
            (stream, presentation,
                    (CDR_Stream_SerializeFunction) DDS_Cdr_serializePresentation,
                    RTPS_PID_PRESENTATION, RTI_FALSE, RTI_TRUE))
    {
        DPDE_LOG_SERIALIZE_PRESENTATION(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
DDS_CdrQosPolicy_deserializePresentation(struct CDR_Stream_t *stream,
                        const struct DDS_PresentationQosPolicy *presentation,
                        void *param)
{
    UNUSED_ARG(param);

#if CDR_VARIABLE_ENUM_ENABLED
    if (!CDR_Stream_deserialize_enum(
            stream, 
            (void *) &presentation->access_scope,
            sizeof(presentation->access_scope)))
    {
        return RTI_FALSE;
    }
#else
    if (!CDR_Stream_deserialize_unsigned_long(stream,
                                 (RTI_UINT32*)&presentation->access_scope))
    {
        return RTI_FALSE;
    }
#endif

    if (!CDR_Stream_deserialize_boolean(stream, &presentation->coherent_access))
    {
        return RTI_FALSE;
    }

    if (!CDR_Stream_deserialize_boolean(stream, &presentation->ordered_access))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_UINT32
DDS_CdrQosPolicy_getPresentationMaxSerializedSize(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += DDS_CdrQosPolicy_get_parameter_header_max_size_serialized(size);
    size += CDR_get_max_size_serialized_long(0);
    size += CDR_get_max_size_serialized_boolean(0);
    size += CDR_get_max_size_serialized_boolean(0);
    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return (size - orig_size);
}

/*----------------------------------------------------------------------------*/

RTI_BOOL
DDS_CdrQosPolicy_serializeProductVersion(struct CDR_Stream_t *stream,
        const struct DDS_ProductVersion *product,
        void *param)
{
    UNUSED_ARG(param);

    /* Product version */
    if (!DDS_CdrQosPolicy_serializeNonPrimitiveParameter
            (stream, product,
                    (CDR_Stream_SerializeFunction) DDS_Cdr_serializeFourOctets,
                    DISC_RTPS_PID_PRODUCT_VERSION, RTI_FALSE, RTI_TRUE))
    {
        DPDE_LOG_SERIALIZE_PRODUCT_VERSION(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
DDS_CdrQosPolicy_deserializeProductVersion(struct CDR_Stream_t *stream,
        struct DDS_ProductVersion *product,
        void *param)
{
    UNUSED_ARG(param);

    CDR_Stream_deserialize_1_byte(stream, &product->major);
    CDR_Stream_deserialize_1_byte(stream, &product->minor);
    CDR_Stream_deserialize_1_byte(stream, &product->release);
    CDR_Stream_deserialize_1_byte(stream, &product->revision);

    return RTI_TRUE;
}

RTI_UINT32
DDS_CdrQosPolicy_getProductVersionMaxSerializedSize(RTI_UINT32 size)
{
    RTI_UINT32 origSize = size;

    size += DDS_CdrQosPolicy_get_parameter_header_max_size_serialized(size);

    /* Add 4 total bytes, 1 for each field of product version */
    size += 4;

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return (size - origSize);
}

/*----------------------------------------------------------------------------*/

RTI_BOOL
DDS_CdrQosPolicy_serializeLeaseDuration(struct CDR_Stream_t *stream,
        const struct DDS_Duration_t *lease_duration,
        void *param)
{
    struct OSAPI_NtpTime serializeTime;
    UNUSED_ARG(param);

    DDS_Duration_to_ntp_time(lease_duration,&serializeTime);

    if (!DDS_CdrQosPolicy_serializeNonPrimitiveParameter
            (stream, &serializeTime,
                    (CDR_Stream_SerializeFunction) RTPS_serialize_ntp_time,
                    RTPS_PID_LEASE_DURATION, RTI_FALSE, RTI_TRUE))
    {
        DPDE_LOG_SERIALIZE_LEASE_DURATION(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
DDS_CdrQosPolicy_deserializeLeaseDuration(struct CDR_Stream_t *stream,
        struct DDS_Duration_t *lease_duration,
        void *param)
{
    struct OSAPI_NtpTime time;
    UNUSED_ARG(param);

    if (!RTPS_deserialize_ntp_time(stream, &time, NULL))
    {
        return RTI_FALSE;
    }

    DDS_Duration_from_ntp_time(lease_duration,&time);

    return RTI_TRUE;
}

RTI_UINT32
DDS_CdrQosPolicy_getLeaseDurationMaxSerializedSize(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += DDS_CdrQosPolicy_get_parameter_header_max_size_serialized(size);
    size += 3 * CDR_get_max_size_serialized_long(0);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return (size - orig_size);
}

/*----------------------------------------------------------------------------*/

/*ci \brief Serialize the CRC properties structure
 *
 * \param[in] stream The stream to serialize to
 * \param[in] crc_property The CRC property to serialize
 * \param[in] param Used defined parameter, not used
 *
 * \return TRUE if the CRC was successfully serialized, FALSE if not.
 */
RTI_PRIVATE RTI_BOOL
DDS_Cdr_serializeChecksum(struct CDR_Stream_t *stream,
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

/*ci \brief Serialize the CRC properties
 *
 * \param[in] stream The stream to serialize to
 * \param[in] crc_property The CRC property to serialize
 * \param[in] param Used defined parameter, not used
 *
 * \return TRUE if the CRC was successfully serialized, FALSE if not.
 */
RTI_BOOL
DDS_CdrQosPolicy_serializeChecksumProperty(struct CDR_Stream_t *stream,
                              const struct DDS_ChecksumProperty *checksum_property,
                              void *param)
{
    UNUSED_ARG(param);

    if ((checksum_property->allowed_crc_mask != 0) ||
        (checksum_property->computed_crc_kind != 0))
    {
        if (!DDS_CdrQosPolicy_serializeNonPrimitiveParameter
                (stream, checksum_property,
                 (CDR_Stream_SerializeFunction) DDS_Cdr_serializeChecksum,
                 RTPS_PID_CHECKSUM_PROPERTY, RTI_FALSE, RTI_TRUE))
        {
            DPDE_LOG_SERIALIZE_PRESENTATION(OSAPI_LOGKIND_ERROR)
                return RTI_FALSE;
        }
    }

    return RTI_TRUE;
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
DDS_CdrQosPolicy_deserializeChecksumProperty(struct CDR_Stream_t *stream,
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
DDS_CdrQosPolicy_getChecksumPropertyMaxSerializedSize(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += DDS_CdrQosPolicy_get_parameter_header_max_size_serialized(size);
    size += 2 * CDR_get_max_size_serialized_short(0) +
            CDR_get_max_size_serialized_boolean(0);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return (size - orig_size);
}

/*----------------------------------------------------------------------------*/

RTI_BOOL
DDS_CdrQosPolicy_serializeSendQueueSize(struct CDR_Stream_t *stream,
        DDS_Long send_queue_size,
        void *param)
{
    RTI_BOOL ok = RTI_TRUE;
    UNUSED_ARG(param);

    ok = DDS_CdrQosPolicy_serializeParameter(stream,RTPS_PID_SEND_QUEUE_SIZE_DEPRECATED,
            (DDS_UnsignedShort)CDR_get_max_size_serialized_long(0));

    ok = ok && CDR_Stream_serialize_long(stream, (RTI_INT32 *) & send_queue_size);

    return ok;
}
