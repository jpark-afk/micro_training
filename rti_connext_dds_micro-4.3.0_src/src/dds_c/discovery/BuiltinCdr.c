/*
 * FILE: BuiltinCdr.c - DPDE CDR API
 *
 * Copyright (c) 2011-2026 Real-Time Innovations, Inc. All rights reserved.
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
#include "dds_c/dds_c_log.h"
#include "BuiltinCdr.h"
#include "PartitionQosPolicy.h"
#include "DomainParticipantTrust.h"
#include "Duration.h"

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
DDS_Cdr_get_parameter_length(DDS_UnsignedShort *length,
                             DDS_UnsignedLong begin, DDS_UnsignedLong end)
{
    DDS_UnsignedLong diff;

    if (begin > end)
    {
        return RTI_FALSE;
    }

    diff = end - begin;

    if (diff > USHRT_MAX)
    {
        return RTI_FALSE;
    }

    *length = (DDS_UnsignedShort)diff;

    return RTI_TRUE;
}

RTI_PRIVATE DDS_Boolean
DDS_CdrStream_insert_parameter_length(struct CDR_Stream_t *stream,
                                      DDS_UnsignedLong parameterBeginPosition,
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
                DDSC_LOG_CDR_SET_OFFSET(OSAPI_LOGKIND_ERROR,currentPosition)
                return DDS_BOOLEAN_FALSE;
            }
            return DDS_BOOLEAN_TRUE;
        }
    }

    if (!CDR_Stream_set_current_position_offset(stream, parameterBeginPosition))
    {
        DDSC_LOG_CDR_SET_OFFSET(OSAPI_LOGKIND_ERROR,parameterBeginPosition)
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

RTI_BOOL
DDS_CdrStream_serialize_4_byte_parameter(struct CDR_Stream_t *stream,
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

    if (!DDS_Cdr_get_parameter_length(&parameterLength,
                                          valueBeginPosition,valueEndPosition))
    {
        return RTI_FALSE;
    }

    return DDS_CdrStream_insert_parameter_length(stream, parameterBeginPosition,
            parameterLength, ok);
}

RTI_PRIVATE RTI_BOOL
DDS_CdrStream_serialize_4_octets(struct CDR_Stream_t *stream,
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

RTI_PRIVATE RTI_BOOL
DDS_CdrStream_serialize_parameter(struct CDR_Stream_t *stream,
                                    DDS_UnsignedShort param_id,
                                    DDS_UnsignedShort param_length)
{
    RTI_BOOL ok = RTI_TRUE;

    ok = CDR_Stream_serialize_unsigned_short(stream, &param_id);
    ok = ok && CDR_Stream_serialize_unsigned_short(stream, &param_length);

    return ok;
}

RTI_BOOL
DDS_CdrStream_serialize_non_primitive_parameter(struct CDR_Stream_t *stream,
                                                const void *in,
                                                CDR_Stream_SerializeFunction
                                                serializeFunction,
                                                DDS_UnsignedShort parameterId,
                                                RTI_BOOL serializeEncapsulation,
                                                RTI_BOOL serializeSample)
{
    RTI_BOOL ok = RTI_TRUE;
    DDS_UnsignedShort parameterLength;
    DDS_UnsignedLong parameterBeginPosition, valueBeginPosition, valueEndPosition;
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
    ok = ok && serializeFunction(stream, in,NULL);
    CDR_Stream_align(stream, CDR_DEFAULT_PARAMETER_ALIGNMENT);
    valueEndPosition = CDR_Stream_get_current_position_offset(stream);

    if (!DDS_Cdr_get_parameter_length(&parameterLength,
                                          valueBeginPosition,valueEndPosition))
    {
        return RTI_FALSE;
    }

    return ok && DDS_CdrStream_insert_parameter_length(stream,
                parameterBeginPosition, parameterLength, ok);
}

DDS_UnsignedLong
DDS_Cdr_get_parameter_header_max_size_serialized(DDS_UnsignedLong size)
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
DDS_Cdr_get_header_max_size_serialized(DDS_UnsignedLong size)
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
    RTI_UINT32 tmp = ipv4Locator->address;

    OSAPI_Memory_zero(ipv6Locator->address, 12);

   OSAPI_Memory_copy(&ipv6Locator->address[12],&tmp,4);
}

RTI_PRIVATE RTI_BOOL
DDS_CdrStream_serialize_string(struct CDR_Stream_t *stream,
                               const char *in,
                               void *param)
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
DDS_CdrStream_deserialize_parameter_sequence(void *parameter,
                                             struct CDR_Stream_t *stream,
                                             DDS_Cdr_SetDefaultParameterValuesFunction
                                             setDefaultParameterValuesFnc,
                                             DDS_Cdr_DeserializeParameterValueFunction
                                             deserializeParameterValueFnc, void *param)
{
    RTPS_ParameterId parameterId = 0, parameterLength = 0;
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

        if (parameterLength > (stream->length - origPosition))
        {
            DDSC_LOG_DESERIALIZE_BAD_PID_LENGTH(OSAPI_LOGKIND_ERROR,
                                                 (RTI_INT32)parameterLength)
            ok = RTI_FALSE;
            break;
        }

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
DDS_CdrQosPolicy_serialize_entity_name(struct CDR_Stream_t *stream,
                                       const struct DDS_EntityNameQosPolicy *entityName,
                                       void *param)
{
    UNUSED_ARG(param);

    if (entityName != NULL)
    {
        if (!DDS_CdrStream_serialize_non_primitive_parameter
                (stream, entityName->name,
                        (CDR_Stream_SerializeFunction) DDS_CdrStream_serialize_string,
                        DISC_RTPS_PID_ENTITY_NAME, RTI_FALSE, RTI_TRUE))
        {
            DDSC_LOG_SERIALIZE_ENTITY_NAME(OSAPI_LOGKIND_ERROR)
            return RTI_FALSE;
        }
    }

    return RTI_TRUE;
}

RTI_BOOL
DDS_CdrQosPolicy_deserialize_entity_name(struct CDR_Stream_t *stream,
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
DDS_CdrQosPolicy_get_max_size_serialized_entity_name(RTI_UINT32 size)
{
    RTI_UINT32 origSize = size;

    size += DDS_Cdr_get_parameter_header_max_size_serialized(size);
    size += CDR_get_max_size_serialized_string(size, RTPS_PATHNAME_LEN_MAX + 1);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return (size - origSize);
}

/*----------------------------------------------------------------------------*/

RTI_BOOL
DDS_CdrQosPolicy_serialize_topic_name(struct CDR_Stream_t *stream,
                                      const char *topic_name,
                                      void * param)
{
    RTI_BOOL retval = RTI_FALSE;
    UNUSED_ARG(param);

    if (!DDS_CdrStream_serialize_non_primitive_parameter(
            stream, topic_name,
            (CDR_Stream_SerializeFunction) DDS_CdrStream_serialize_string,
            RTPS_PID_TOPIC_NAME, RTI_FALSE, RTI_TRUE))
    {
        DDSC_LOG_SERIALIZE_TOPIC_NAME(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    retval = RTI_TRUE;

    done:
    return retval;
}

RTI_BOOL
DDS_CdrQosPolicy_deserialize_topic_name(struct CDR_Stream_t *stream,
                                        char **topic_name,
                                        DDS_StringManager_T *string_manager)
{
    char *string;
    RTI_UINT32 length;

    if (!CDR_Stream_deserialize_get_string(stream, &string, &length))
    {
        return RTI_FALSE;
    }

    if (length > RTPS_PATHNAME_LEN_MAX)
    {
        return RTI_FALSE;
    }

    *topic_name = DDS_StringManager_assert_string(string_manager, string);

    if (*topic_name == NULL)
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_topic_name(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += CDR_get_max_size_serialized_string(size, RTPS_PATHNAME_LEN_MAX + 1);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return size - orig_size;
}

/*----------------------------------------------------------------------------*/

RTI_BOOL
DDS_CdrQosPolicy_serialize_type_name(struct CDR_Stream_t *stream,
                                     const char *type_name,
                                     void *param)
{
    RTI_BOOL retval = RTI_FALSE;
    UNUSED_ARG(param);

    if (!DDS_CdrStream_serialize_non_primitive_parameter
            (stream, type_name,
                    (CDR_Stream_SerializeFunction) DDS_CdrStream_serialize_string,
                    RTPS_PID_TYPE_NAME, RTI_FALSE, RTI_TRUE))
    {
        DDSC_LOG_SERIALIZE_TYPE_NAME(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    retval = RTI_TRUE;

    done:
    return retval;
}

RTI_BOOL
DDS_CdrQosPolicy_deserialize_type_name(struct CDR_Stream_t *stream,
                                        char **type_name,
                                        DDS_StringManager_T *string_manager)
{
    char *string;
    RTI_UINT32 length;

    if (!CDR_Stream_deserialize_get_string(stream, &string, &length))
    {
        return RTI_FALSE;
    }

    if (length > RTPS_PATHNAME_LEN_MAX)
    {
        return RTI_FALSE;
    }

    *type_name = DDS_StringManager_assert_string(string_manager, string);

    if (*type_name == NULL)
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_type_name(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += CDR_get_max_size_serialized_string(size, RTPS_PATHNAME_LEN_MAX + 1);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return size - orig_size;
}

/*----------------------------------------------------------------------------*/

/*ci
 * \brief Serialize a Partition qos policy
 *
 * \param[inout] stream  The serialization stream
 * \param[in] data       The partition string sequence
 * \param[in] param      Unused
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
DDS_CdrQosPolicy_serialize_partition_string_seq(struct CDR_Stream_t *stream,
                                     const void *data,
                                    void *param)
{
    struct CDR_StringSeq *string_seq = (struct CDR_StringSeq *) data;
    UNUSED_ARG(param);
    return CDR_Stream_serialize_string_sequence(stream,
                            (struct REDA_Sequence*) string_seq,
                            DDS_PARTITIONQOSPOLICY_MAX_PARTITION_CHARACTERS - 1,
                            CDR_CHAR_TYPE);
}

/*ci
 * \brief
 *
 * \param[inout] stream  The serialization stream
 * \param[in] partition  The Partition Qos to serialize
 * \param[in] param      Unused
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
DDS_CdrQosPolicy_serialize_partition(struct CDR_Stream_t *stream,
                                const struct DDS_PartitionQosPolicy *partition,
                                void *param)
{
    RTI_BOOL ok = RTI_TRUE;
    UNUSED_ARG(param);

    OSAPI_PRECONDITION_ALWAYS(partition == NULL,
                return RTI_FALSE,
                OSAPI_Log_entry_add_pointer("partition",partition,RTI_TRUE);)

    if (DDS_StringSeq_get_length(&partition->name) == 0)
    {
        return RTI_TRUE;
    }

    if (!DDS_CdrStream_serialize_non_primitive_parameter(stream,
                                                        &partition->name,
        (CDR_Stream_SerializeFunction)DDS_CdrQosPolicy_serialize_partition_string_seq,
                                                        RTPS_PID_PARTITION,
                                                        RTI_FALSE,
                                                        RTI_TRUE))
    {
        return RTI_FALSE;
    }
    return ok;
}

/*ci
 * \brief Deserialize the partition QoS policy
 *
 * \param[inout] stream            The deserialization stream
 * \param[out] partition           The partition Qos Policy
 * \param[in] string_manager       The string manger to use to assert the
 *                                 partition strings
 * \param[in] max_cumulative_chars The max characters to deserialize
 * \param[in] max_string_size      The max string size to deserialize
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
RTI_BOOL
DDS_CdrQosPolicy_deserialize_partition(struct CDR_Stream_t *stream,
                                        struct DDS_PartitionQosPolicy *partition,
                                        DDS_StringManager_T *string_manager,
                                        RTI_INT32 max_cumulative_chars,
                                        RTI_INT32 max_string_size)
{
    RTI_BOOL ok = RTI_FALSE;
    RTI_BOOL retval;
    RTI_INT32 i, seq_length;
    RTI_UINT32 string_length = 0;
    RTI_UINT32 cumulative_chars = 0;
    char *out_str;

    OSAPI_PRECONDITION(partition == NULL,
        goto done,
        OSAPI_Log_entry_add_pointer("partition",partition,RTI_TRUE);)

    OSAPI_PRECONDITION(DDS_StringSeq_get_length(&partition->name) != 0,
        goto done,
        OSAPI_Log_entry_add_int("partition->name.length",DDS_StringSeq_get_length(&partition->name),RTI_TRUE);)

    /* Deserialize sequence length */
    retval = CDR_Stream_deserialize_long(stream, &seq_length);
    if ((!retval) || (seq_length < 0))
    {
        goto done;
    }

    if (seq_length == 0)
    {
        ok = RTI_TRUE;
        goto done;
    }

    /* The string manager can be NULL if max_partition is 0 and can
     * receive a partition seq of length 0. However, don't deserialize a
     * partition seq of length > 0 with a NULL string manager.
     */
    if (string_manager == NULL)
    {
        goto done;
    }

    if ((seq_length > DDS_StringSeq_get_maximum(&partition->name)))
    {
        goto done;
    }

    if (!REDA_StringSeq_set_length(&partition->name,seq_length))
    {
        goto done;
    }

    /* Get the total string length and max cumulative characters to validate
     * that the partition policy does not exceed limits.
     */
    for (i=0; i<seq_length; i++)
    {
        if (!CDR_Stream_deserialize_get_string(stream, &out_str, &string_length))
        {
            break;
        }

        if (string_length >= INT_MAX)
        {
            break;
        }

        string_length++;

        if ((max_string_size < (RTI_INT32)string_length) &&
            (max_string_size != DDS_LENGTH_UNLIMITED))
        {
            break;
        }

        /* If we are using fixed string sizes then the length is always
         * max_partition_string_size
         */
        if (max_string_size != DDS_LENGTH_UNLIMITED)
        {
            string_length = (RTI_UINT32)max_string_size;
        }

        cumulative_chars += string_length;

        if (cumulative_chars > (RTI_UINT32)max_cumulative_chars)
        {
            break;
        }

        *DDS_StringSeq_get_reference(&partition->name, i) =
            DDS_StringManager_assert_string(string_manager, out_str);

        if (*DDS_StringSeq_get_reference(&partition->name,i) == NULL)
        {
            break;
        }
    }

    /* If we didn't assert all of the strings it means we encountered an error.
     * clear the resources and reset the output partition policy length
     */
    if (i < seq_length)
    {
        retval = DDS_PartitionQosPolicy_clear_strings(string_manager, partition);
        IGNORE_RETVAL(retval);
    }
    else
    {
        ok = RTI_TRUE;
    }

done:
#if OSAPI_ENABLE_LOG
    if (!ok)
    {
        DDSC_LOG_DESERIALIZE_PARTITION_SEQ(OSAPI_LOGKIND_INFO)
    }
#endif
    return ok;
}

/*ci
 * \brief Returns the max partition serialize size.
 *
 * \param[inout] size     The serialization size
 * \param[in] max_length  The max partition sequence length
 *
 * \return The max partition serialize size
 */
RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_partition(RTI_UINT32 size,
                                                   RTI_UINT32 max_length)
{
    RTI_UINT32 orig_size = size;

    size += DDS_Cdr_get_parameter_header_max_size_serialized(size);

    size += max_length * CDR_get_max_size_serialized_char(0);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return size - orig_size;
}

/*----------------------------------------------------------------------------*/
RTI_BOOL
DDS_CdrQosPolicy_serialize_deadline(struct CDR_Stream_t *stream,
                                    const struct DDS_DeadlineQosPolicy *deadline,
                                    void *param)
{
    RTI_BOOL ok = RTI_TRUE;

    ok = DDS_CdrStream_serialize_parameter(stream,RTPS_PID_DEADLINE,
                (DDS_UnsignedShort)(2 * CDR_get_max_size_serialized_long(0)));

    ok = ok && DDS_Duration_serialize(stream, &deadline->period, param);

    return ok;
}

RTI_BOOL
DDS_CdrQosPolicy_deserialize_deadline(struct CDR_Stream_t *stream,
                                      struct DDS_DeadlineQosPolicy *deadline,
                                      void *param)
{
    return DDS_Duration_deserialize(stream, &deadline->period, param);
}

RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_deadline(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += DDS_Cdr_get_parameter_header_max_size_serialized(size);
    size += CDR_get_max_size_serialized_long(0);
    size += CDR_get_max_size_serialized_long(0);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return size - orig_size;
}

/*----------------------------------------------------------------------------*/

RTI_BOOL
DDS_CdrQosPolicy_serialize_ownership(struct CDR_Stream_t *stream,
                                     const struct DDS_OwnershipQosPolicy
                                     *ownership,
                                     void *param)
{
    RTI_BOOL ok = RTI_TRUE;
    UNUSED_ARG(param);

    ok = DDS_CdrStream_serialize_parameter(stream,RTPS_PID_OWNERSHIP,
                    (DDS_UnsignedShort)CDR_get_max_size_serialized_long(0));

    ok = ok && CDR_Stream_serialize_unsigned_long(stream,
                                              (RTI_UINT32 *)&ownership->kind);
    return ok;
}

RTI_BOOL
DDS_CdrQosPolicy_deserialize_ownership(struct CDR_Stream_t *stream,
                                       struct DDS_OwnershipQosPolicy *ownership,
                                       void *param)
{
    UNUSED_ARG(param);
    return CDR_Stream_deserialize_unsigned_long(stream,
                                               (RTI_UINT32*)&ownership->kind);
}

RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_ownership(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += DDS_Cdr_get_parameter_header_max_size_serialized(size);
    size += CDR_get_max_size_serialized_long(0);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return size - orig_size;
}

/*----------------------------------------------------------------------------*/

RTI_BOOL
DDS_CdrQosPolicy_serialize_ownership_strength(struct CDR_Stream_t *stream,
              const struct DDS_OwnershipStrengthQosPolicy *ownership_strength,
              void *param)
{
    RTI_BOOL ok = RTI_TRUE;
    UNUSED_ARG(param);

    ok = DDS_CdrStream_serialize_parameter(stream,RTPS_PID_OWNERSHIP_STRENGTH,
            (DDS_UnsignedShort)CDR_get_max_size_serialized_long(0));

    ok = ok && CDR_Stream_serialize_unsigned_long(stream,
                                    (RTI_UINT32*)&ownership_strength->value);
    return ok;
}

RTI_BOOL
DDS_CdrQosPolicy_deserialize_ownership_strength(struct CDR_Stream_t *stream,
                    struct DDS_OwnershipStrengthQosPolicy *ownership_strength,
                    void *param)
{
    UNUSED_ARG(param);
    return CDR_Stream_deserialize_unsigned_long(stream,
                                   (RTI_UINT32 *)&ownership_strength->value);
}

RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_ownership_strength(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += DDS_Cdr_get_parameter_header_max_size_serialized(size);
    size += CDR_get_max_size_serialized_long(0);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return size - orig_size;
}

/*----------------------------------------------------------------------------*/


RTI_BOOL
DDS_CdrQosPolicy_serialize_reliability(struct CDR_Stream_t *stream,
                                       const struct DDS_ReliabilityQosPolicy *reliability,
                                       void *param)
{
    RTI_BOOL ok = RTI_TRUE;

    ok = DDS_CdrStream_serialize_parameter(stream,RTPS_PID_RELIABILITY,
            (DDS_UnsignedShort)(3 * CDR_get_max_size_serialized_long(0)));

    ok = ok && CDR_Stream_serialize_unsigned_long(stream,
                                (RTI_UINT32*)&reliability->kind);

    ok = ok && DDS_Duration_serialize(stream,
                                      &reliability->max_blocking_time,
                                      param);

    return ok;
}

RTI_BOOL
DDS_CdrQosPolicy_deserialize_reliability(struct CDR_Stream_t *stream,
                                 struct DDS_ReliabilityQosPolicy *reliability,
                                 void *param)
{
    UNUSED_ARG(param);

    if (!CDR_Stream_deserialize_unsigned_long(stream,
                                            (RTI_UINT32*)&reliability->kind))
    {
        return RTI_FALSE;
    }

    if (!DDS_Duration_deserialize(stream,
                                  &reliability->max_blocking_time,
                                  param))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_reliability(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += DDS_Cdr_get_parameter_header_max_size_serialized(size);
    size += 3 * CDR_get_max_size_serialized_long(0);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return size - orig_size;
}

/*----------------------------------------------------------------------------*/

RTI_BOOL
DDS_CdrQosPolicy_serialize_liveliness(struct CDR_Stream_t *stream,
                                      const struct DDS_LivelinessQosPolicy *liveliness,
                                      void *param)
{
    RTI_BOOL ok = RTI_TRUE;

    ok = DDS_CdrStream_serialize_parameter(stream,RTPS_PID_LIVELINESS,
            (DDS_UnsignedShort)(3 * CDR_get_max_size_serialized_long(0)));

    ok = ok && CDR_Stream_serialize_unsigned_long(stream,
                                            (RTI_UINT32 *)&liveliness->kind);

    ok = ok && DDS_Duration_serialize(stream,
                                      &liveliness->lease_duration,
                                      param);

    return ok;
}

RTI_BOOL
DDS_CdrQosPolicy_deserialize_liveliness(struct CDR_Stream_t *stream,
                                        struct DDS_LivelinessQosPolicy *liveliness,
                                        void *param)
{
    UNUSED_ARG(param);

    if (!CDR_Stream_deserialize_unsigned_long(stream,
                            (RTI_UINT32 *)&liveliness->kind))
    {
        return RTI_FALSE;
    }

    if (!DDS_Duration_deserialize(stream,
                                  &liveliness->lease_duration,
                                  param))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_liveliness(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += DDS_Cdr_get_parameter_header_max_size_serialized(size);
    size += 3 * CDR_get_max_size_serialized_long(0);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return (size - orig_size);
}

/*----------------------------------------------------------------------------*/

RTI_BOOL
DDS_CdrQosPolicy_serialize_key(struct CDR_Stream_t *stream,
                               const struct DDS_BuiltinTopicKey_t *key,
                               DDS_UnsignedShort id)
{
    struct RTPS_Guid guid;

    guid.prefix.host_id = key->value[0];
    guid.prefix.app_id  = key->value[1];
    guid.prefix.instance_id  = key->value[2];
    guid.object_id = key->value[3];

    if (!DDS_CdrStream_serialize_non_primitive_parameter
            (stream, &guid, (CDR_Stream_SerializeFunction) RTPS_Guid_serialize,
                    id, RTI_FALSE, RTI_TRUE))
    {
        DDSC_LOG_SERIALIZE_GUID(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
DDS_CdrQosPolicy_deserialize_key(struct CDR_Stream_t *stream,
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
DDS_CdrQosPolicy_get_max_size_serialized_key(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += DDS_Cdr_get_parameter_header_max_size_serialized(size);
    size += (RTI_UINT32)sizeof(struct DDS_BuiltinTopicKey_t);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return size - orig_size;
}

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/

RTI_PRIVATE RTI_BOOL
DDS_CdrQosPolicy_locator_has_encapsulations(NETIO_BindResolver_T* bresolver,
                                            const struct DDS_Locator *locator,
                                            struct DDS_TransportEncapsulationSettings_t *ts)
{
    RTI_INT32 length,index;
    NETIO_Interface_T *intf;
    RT_ComponentFactoryId_T id = RT_ComponentFactoryId_INITIALIZER;
    UNUSED_ARG(locator);

    /* 1. Use NETIO_BindResolver_lookup_by_address to determine if an
     *    address was created by any of the transports listed.
     *    - Yes -> Add encapsulations for this locator
     *    - No  -> Move to next. If no transports in the policy owns it,
     *             then no
     *
     * Given the locator, first find the interface that reserved it.
     */
    length = DDS_StringSeq_get_length(&ts->transports);
    for (index = 0; index < length; ++index)
    {
        if (!RT_ComponentFactoryId_set_name(&id,
                        *DDS_StringSeq_get_reference(&ts->transports,index)))
        {

        }

        if (NETIO_BindResolver_lookup_by_address(bresolver,
                                         NETIO_ROUTEKIND_USER,
                                         &id,
                                         (struct NETIO_Address*)locator,&intf))
        {
            return DDS_BOOLEAN_TRUE;
        }
    }

    return DDS_BOOLEAN_FALSE;
}

RTI_PRIVATE RTI_BOOL
DDS_CdrQosPolicy_serialize_locator(struct CDR_Stream_t *stream,
                                   NETIO_BindResolver_T* bresolver,
                                   const struct DDS_Locator *locator,
                                   RTI_UINT16 locator_id,
                                   RTI_UINT16 locator_id_ex,
                                   struct DDS_TransportEncapsulationQosPolicy *policy)
{
    RTI_INT32 length,index;
    struct DDS_TransportEncapsulationSettings_t *ts = NULL;
    RTI_UINT16 pid_kind;
    RTI_UINT16 pid_length;
    DDS_UnsignedLong hdr_off;
    DDS_UnsignedLong seq_off = 0;
    DDS_UnsignedLong save_off = 0;
    RTI_INT32 seq_length = 0;
    RTI_INT32 kind;
    struct NETIO_Address *address = (struct NETIO_Address *)locator;
    struct DDS_Locator a_locator = *locator;

    RTI_BOOL has_enc = RTI_FALSE;

    UNUSED_ARG(locator);

    CDR_Stream_align(stream, CDR_DEFAULT_PARAMETER_ALIGNMENT);

   /* +--------+-------+
    * | PID    | PL    |
    * +--------+-------+
    * | Transport Kind |
    * +----------------+
    * | Transport Port |
    * +----------------+
    * | Transport Addr |
    * ~                ~
    * |                |
    * +----------------+
    * |   Enc. Count   |
    * +----------------+
    * |    Enc[0]      |
    * +----------------+
    * |    Enc[1]      |
    * +----------------+
    * | Enc[Count-1]   |
    * +----------------+
    */

    /* 1. Save position for PID and PID length and allocate space */
    hdr_off = CDR_Stream_get_current_position_offset(stream);

    if (!CDR_Stream_increment_current_position(stream,2*CDR_SHORT_SIZE))
    {
        return RTI_FALSE;
    }

    /* 2. Serialize kind & port */
    kind = NETIO_Address_get_kind(address);
    if (!CDR_Stream_serialize_long(stream, &kind))
    {
        return RTI_FALSE;
    }

    if (!CDR_Stream_serialize_long(stream,&address->port))
    {
        return RTI_FALSE;
    }

    if (!CDR_Stream_has_remaining_space(stream, RTPS_LOCATOR_ADDRESS_LENGTH_MAX))
    {
        return RTI_FALSE;
    }

    if (kind == RTPS_LOCATOR_KIND_UDPv4)
    {
        struct DDS_LocatorUdpv4_t udp4_loc = *(struct DDS_LocatorUdpv4_t*)locator;

        DDS_Cdr_ipv6_from_ipv4(&a_locator,&udp4_loc);
    }

    for (index = 0; index < RTPS_LOCATOR_ADDRESS_LENGTH_MAX; ++index)
    {
        *(RTI_UINT8*)(stream->buff_ptr++) = a_locator.address[index];
    }

    if (policy != NULL)
    {
        /* 3. Serialize possible encapsulations */
        length = DDS_TransportEncapsulationSettingsSeq_get_length(&policy->value);
        for (index = 0; index < length; index++)
        {
            ts = DDS_TransportEncapsulationSettingsSeq_get_reference(&policy->value,index);
            if (ts == NULL)
            {
                return RTI_FALSE;
            }

            if (DDS_CdrQosPolicy_locator_has_encapsulations(bresolver,locator,ts))
            {
                if (seq_off == 0)
                {
                    seq_off = CDR_Stream_get_current_position_offset(stream);
                    if (!CDR_Stream_increment_current_position(stream,CDR_LONG_SIZE))
                    {
                        return RTI_FALSE;
                    }
                }

                /* Serialize encapsulations
                 * - Save offset of length, increment for each element
                 */
                for (seq_length = 0; seq_length <
                        DDS_EncapsulationIdSeq_get_length(&ts->encapsulations);
                        seq_length++)
                {
                    DDS_EncapsulationId_t *enc_ref =
                        DDS_EncapsulationIdSeq_get_reference(
                                             &ts->encapsulations,seq_length);
                    if ((enc_ref != NULL) &&
                        (!CDR_Stream_serialize_unsigned_short(stream,enc_ref)))
                    {
                        return RTI_FALSE;
                    }
                }

                has_enc = RTI_TRUE;
            }
        }
    }

    /* Serialization complete, align to make the correct pid lenght
     * is calculated, update the sequence length for
     * the encapsulations, the pid kind and the pid length.
     */
    CDR_Stream_align(stream, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    save_off = CDR_Stream_get_current_position_offset(stream);

    /* 4. If there was encapsulations use EX locator, otherwise regular
     * locator
     */
    if (has_enc)
    {
        if (!CDR_Stream_set_current_position_offset(stream,seq_off))
        {
            return RTI_FALSE;
        }

        if (!CDR_Stream_serialize_long(stream,&seq_length))
        {
            return RTI_FALSE;
        }

        pid_kind = locator_id_ex;
    }
    else
    {
        pid_kind = locator_id;
    }

    /* Does not include header in length */
    pid_length = (RTI_UINT16)(save_off - hdr_off - (2 + CDR_SHORT_SIZE));

    if (!CDR_Stream_set_current_position_offset(stream,hdr_off))
    {
        return RTI_FALSE;
    }

    if (!CDR_Stream_serialize_unsigned_short(stream,&pid_kind))
    {
        return RTI_FALSE;
    }

    if (!CDR_Stream_serialize_unsigned_short(stream,&pid_length))
    {
        return RTI_FALSE;
    }

    if (!CDR_Stream_set_current_position_offset(stream,save_off))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*
NETIO_AddressResolver_lookup_name
*/
RTI_BOOL
DDS_CdrQosPolicy_serialize_locator_sequence(struct CDR_Stream_t *stream,
                                            const struct DDS_LocatorSeq *locator,
                                            RTI_UINT16 locator_id,
                                            RTI_UINT16 locator_id_ex,
                                            struct DDS_TransportEncapsulationQosPolicy *policy,
                                            NETIO_BindResolver_T* bresolver)
{
    RTI_INT32 length;
    RTI_INT32 index;
    struct DDS_Locator *a_locator;

    if (locator == NULL)
    {
        return RTI_TRUE;
    }

    length = DDS_LocatorSeq_get_length(locator);

    for (index = 0; index < length; ++index)
    {
        a_locator = DDS_LocatorSeq_get_reference(locator,index);
        if (!DDS_CdrQosPolicy_serialize_locator(stream,bresolver,
                                                a_locator,
                                           locator_id,
                                           locator_id_ex,
                                           policy))
        {
            return RTI_FALSE;
        }
    }

    return RTI_TRUE;
}

RTI_BOOL
DDS_CdrQosPolicy_deserialize_locator_sequence(struct CDR_Stream_t *stream,
                                              struct DDS_LocatorSeq *locator,
                                              DDS_UnsignedShort kind,
                                              void *param)
{
    struct DDS_Locator *loc;
    RTI_INT32 l, ml;
    DDS_DomainParticipant *dp = (DDS_DomainParticipant*) param;
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
            DDSC_LOG_DESERIALIZE_LOCATOR(OSAPI_LOGKIND_ERROR,kind)
            return RTI_FALSE;
        }
        /* Not an error, but log that the locator sequence is full */
        DDSC_LOG_LOCATORS_FULL(OSAPI_LOGKIND_WARNING,kind)
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

    NETIO_Address_init(DDS_Locator_cast(struct NETIO_Address,loc),0);

    if (!RTPS_deserialize_ipv6_locator(stream, loc, NULL))
    {
        /* De-serialization error, return error to drop the sample.
         */
        DDSC_LOG_DESERIALIZE_LOCATOR(OSAPI_LOGKIND_ERROR,kind)
        return RTI_FALSE;
    }

    if ((kind == RTPS_PID_MULTICAST_LOCATOR6) ||
        (kind == RTPS_PID_METATRAFFIC_MULTICAST_LOCATOR6) ||
        (kind == RTPS_PID_DEFAULT_MULTICAST_LOCATOR6))
    {
        /* Do not call set_multicast since this assignment cannot fail
         */
        loc->kind |= (RTI_INT32)NETIO_ADDRESS_FLAG_MULTICAST;
    }

    /* Check if this is a supported locator, if not drop it altogether
     */
    if (!DDS_DomainParticipant_locator_is_supported(dp,loc))
    {
        /* Discard the locator */
        DDSC_LOG_UNSUPPORTED_LOCATOR(OSAPI_LOGKIND_WARNING,loc->kind)
        DDS_LocatorSeq_set_length(locator, l);
    }

    return RTI_TRUE;
}

RTI_BOOL
DDS_CdrQosPolicy_deserialize_locator_ex_sequence(struct CDR_Stream_t *stream,
                                                 struct DDS_LocatorExSeq *locator,
                                                 DDS_UnsignedShort kind,
                                                 void *param)
{
    struct DDS_LocatorEx *loc;
    RTI_INT32 l, ml;
    DDS_DomainParticipant *dp = (DDS_DomainParticipant*) param;
    struct RTPS_Locator_t discard_loc;
    RTI_INT32 seq_length = 0;
    RTI_INT32 seq_index = 0;
    DDS_UnsignedShort enc = 0;
    struct DDS_Locator basic_loc = DDS_LOCATOR_INVALID;

    l = DDS_LocatorExSeq_get_length(locator);
    ml = DDS_LocatorExSeq_get_maximum(locator);

    if (l == ml)
    {
        /* We need to de-serialize the locator to properly discard it.
         * NOTE: We could also increment the position, but then we'll not
         *       know if the locator was valid.
         */
        if (!RTPS_deserialize_ipv6_locator(stream, &discard_loc, NULL))
        {
            /* De-serialization error, return error to drop the sample. */
            DDSC_LOG_DESERIALIZE_LOCATOR(OSAPI_LOGKIND_ERROR,kind)
            return RTI_FALSE;
        }
        /* Not an error, but log that the locator sequence is full */
        DDSC_LOG_LOCATORS_FULL(OSAPI_LOGKIND_WARNING,kind)
        return RTI_TRUE;
    }

    if (!DDS_LocatorExSeq_set_length(locator, l + 1))
    {
        return RTI_FALSE;
    }

    loc = DDS_LocatorExSeq_get_reference(locator, l);
    if (loc == NULL)
    {
        return RTI_FALSE;
    }

    /* deserialize common content into the correct type */
    if (!RTPS_deserialize_ipv6_locator(stream, &basic_loc, NULL))
    {
        /* De-serialization error, return error to drop the sample.
         */
        DDSC_LOG_DESERIALIZE_LOCATOR(OSAPI_LOGKIND_ERROR,kind)
        return RTI_FALSE;
    }

    /* Check if this is a supported locator, if not drop it altogether
     */
    if (!DDS_DomainParticipant_locator_is_supported(dp,&basic_loc))
    {
        /* Discard the locator */
        DDSC_LOG_UNSUPPORTED_LOCATOR(OSAPI_LOGKIND_WARNING,basic_loc.kind)
        DDS_LocatorExSeq_set_length(locator, l);

        return RTI_TRUE;
    }

    DDS_LocatorEx_from(loc, &basic_loc);

    /* If this is a regular locator, return */
    if (kind == RTPS_PID_UNICAST_LOCATOR6)
    {
        return RTI_TRUE;
    }

    /* Asssign the extended locator content otherwise. This is an extended
     * locator de-serialize the supported encapsulations for this transport.
     */
    if (!CDR_Stream_deserialize_long(stream,&seq_length))
    {
        return RTI_FALSE;
    }

    if (seq_length > MAX_LOCATOR_ENCAPSULATIONS)
    {
        return RTI_FALSE;
    }

    for (seq_index = 0; seq_index < seq_length; seq_index++)
    {
        if (!CDR_Stream_deserialize_unsigned_short(stream,&enc))
        {
            return RTI_FALSE;
        }

        loc->encapsulations[loc->length] = enc;
        loc->length++;
    }

    return RTI_TRUE;
}

RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_ex_locator(struct DDS_LocatorExSeq *locator,
                                                    RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;
    RTI_UINT32 el_size;

    el_size = DDS_Cdr_get_parameter_header_max_size_serialized(0) +
                        RTPS_get_ipv6_locator_max_size_serialized(0) +
                        CDR_LONG_SIZE + (7 * CDR_SHORT_SIZE);

    el_size = CDR_align_upwards(el_size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    size += (RTI_UINT32)DDS_LocatorExSeq_get_maximum(locator) * el_size;

    return (size - orig_size);
}

RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_locator(struct DDS_LocatorSeq *locator,
                                                 RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;
    RTI_UINT32 el_size;

    el_size = DDS_Cdr_get_parameter_header_max_size_serialized(0) +
                        RTPS_get_ipv6_locator_max_size_serialized(0);

    el_size = CDR_align_upwards(el_size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    size += (RTI_UINT32)DDS_LocatorSeq_get_maximum(locator) * el_size;

    return (size - orig_size);
}

/*----------------------------------------------------------------------------*/

RTI_BOOL
DDS_CdrQosPolicy_serialize_protocol_version(struct CDR_Stream_t *stream,
                                            const struct DDS_ProtocolVersion *protocol,
                                            void *param)
{
    CDR_Octet octet_version[2] = {0,0};
    UNUSED_ARG(param);

    octet_version[0] = (CDR_Octet)protocol->major;
    octet_version[1] = (CDR_Octet)protocol->minor;


    if (!DDS_CdrStream_serialize_non_primitive_parameter
            (stream, octet_version,
                    (CDR_Stream_SerializeFunction) RTPS_serialize_2_octets,
                    RTPS_PID_PROTOCOL_VERSION, RTI_FALSE, RTI_TRUE))
    {
        DDSC_LOG_SERIALIZE_PROTOCOL_VERSION(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
DDS_CdrQosPolicy_deserialize_protocol_version(struct CDR_Stream_t *stream,
                                              struct DDS_ProtocolVersion *protocol,
                                              void *param)
{
    CDR_Octet octet_version[2] = {0,0};
    UNUSED_ARG(param);

    if (!RTPS_deserialize_2_octets(stream, octet_version, NULL))
    {
        DDSC_LOG_DESERIALIZE_PROTOCOL_VERSION(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    protocol->major = (DDS_Octet)octet_version[0];
    protocol->minor = (DDS_Octet)octet_version[1];

    return RTI_TRUE;
}

RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_protocol_version(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += DDS_Cdr_get_parameter_header_max_size_serialized(size);

    /* Add 4 total bytes: 2 for fieldsd, 2 for alignment */
    size += 4;

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return (size - orig_size);
}

/*----------------------------------------------------------------------------*/

RTI_BOOL
DDS_CdrQosPolicy_serialize_vendor_id(struct CDR_Stream_t *stream,
                                     const struct DDS_VendorId *vendor,
                                     void *param)
{
    CDR_Octet vendor_octets[2] = {0,0};
    UNUSED_ARG(param);

    vendor_octets[0] = (CDR_Octet)vendor->vendorId[0];
    vendor_octets[1] = (CDR_Octet)vendor->vendorId[1];
    if (!DDS_CdrStream_serialize_non_primitive_parameter
            (stream, vendor_octets,
                    (CDR_Stream_SerializeFunction) RTPS_serialize_2_octets,
                    RTPS_PID_VENDOR_ID, RTI_FALSE, RTI_TRUE))
    {
        DDSC_LOG_SERIALIZE_VENDOR_ID(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
DDS_CdrQosPolicy_deserialize_vendor_id(struct CDR_Stream_t *stream,
                                       struct DDS_VendorId *vendor,
                                       void *param)
{
    CDR_Octet vendor_octets[2];
    UNUSED_ARG(param);

    if (!RTPS_deserialize_2_octets(stream, vendor_octets, NULL))
    {
        DDSC_LOG_DESERIALIZE_VENDOR_ID(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    vendor->vendorId[0] = (DDS_Octet)vendor_octets[0];
    vendor->vendorId[1] = (DDS_Octet)vendor_octets[1];

    return RTI_TRUE;
}

RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_vendor_id(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += DDS_Cdr_get_parameter_header_max_size_serialized(size);
    size += RTPS_get_2_octets_max_size_serialized(size);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return (size - orig_size);
}

/*----------------------------------------------------------------------------*/

RTI_PRIVATE RTI_BOOL
DDS_Cdr_serialize_presentation(struct CDR_Stream_t *stream,
                       const void *sample,
                       void *param)
{
    RTI_BOOL ok;
    const struct DDS_PresentationQosPolicy *presentation =
                           (const struct DDS_PresentationQosPolicy*)sample;

    UNUSED_ARG(param);

    ok = CDR_Stream_serialize_unsigned_long(stream,
                                    (RTI_UINT32*)&presentation->access_scope);

    ok = ok && CDR_Stream_serialize_boolean(stream,
                                    &presentation->coherent_access);

    ok = ok && CDR_Stream_serialize_boolean(stream,
                                    &presentation->ordered_access);

    return ok;
}

RTI_BOOL
DDS_CdrQosPolicy_serialize_presentation(struct CDR_Stream_t *stream,
                        const struct DDS_PresentationQosPolicy *presentation,
                        void *param)
{
    UNUSED_ARG(presentation);
    UNUSED_ARG(param);

    if (!DDS_CdrStream_serialize_non_primitive_parameter(stream,
                    presentation,
                    (CDR_Stream_SerializeFunction)DDS_Cdr_serialize_presentation,
                    RTPS_PID_PRESENTATION, RTI_FALSE, RTI_TRUE))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
DDS_CdrQosPolicy_deserialize_presentation(struct CDR_Stream_t *stream,
                        struct DDS_PresentationQosPolicy *presentation,
                        void *param)
{
    UNUSED_ARG(param);

    if (!CDR_Stream_deserialize_unsigned_long(stream,
                                     (RTI_UINT32*)&presentation->access_scope))
    {
        return RTI_FALSE;
    }

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
DDS_CdrQosPolicy_get_max_size_serialized_presentation(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += DDS_Cdr_get_parameter_header_max_size_serialized(size);
    size += CDR_get_max_size_serialized_long(0);
    size += CDR_get_max_size_serialized_boolean(0);
    size += CDR_get_max_size_serialized_boolean(0);
    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return (size - orig_size);
}


/*----------------------------------------------------------------------------*/

RTI_BOOL
DDS_CdrQosPolicy_serialize_durability(struct CDR_Stream_t *stream,
                                      const struct DDS_DurabilityQosPolicy *durability,
                                      void *param)
{
    RTI_BOOL ok = RTI_TRUE;
    UNUSED_ARG(param);

    ok = DDS_CdrStream_serialize_parameter(stream,RTPS_PID_DURABILITY,
                        (DDS_UnsignedShort)CDR_get_max_size_serialized_long(0));

    ok = ok
            && CDR_Stream_serialize_unsigned_long(stream,
                    (RTI_UINT32 *)&durability->kind);

    return ok;
}

RTI_BOOL
DDS_CdrQosPolicy_deserialize_durability(struct CDR_Stream_t *stream,
                                        struct DDS_DurabilityQosPolicy *durability,
                                        void *param)
{
    RTI_BOOL ok = RTI_TRUE;
    UNUSED_ARG(param);

    ok = ok
            && CDR_Stream_deserialize_unsigned_long(stream,
                    (RTI_UINT32 *) & durability->kind);

    return ok;
}

RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_durability(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += DDS_Cdr_get_parameter_header_max_size_serialized(size);
    size += CDR_get_max_size_serialized_long(0);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return (size - orig_size);
}

/*----------------------------------------------------------------------------*/

RTI_BOOL
DDS_CdrQosPolicy_serialize_destination_order(struct CDR_Stream_t *stream,
        const struct DDS_DestinationOrderQosPolicy *destination_order,
        void *param)
{
    RTI_BOOL ok = RTI_TRUE;
    UNUSED_ARG(param);

    ok = DDS_CdrStream_serialize_parameter(stream,RTPS_PID_DESTINATION_ORDER,
                        (DDS_UnsignedShort)CDR_get_max_size_serialized_long(0));

    ok = ok && CDR_Stream_serialize_unsigned_long(stream,
                    (RTI_UINT32 *)&destination_order->kind);

    return ok;
}

RTI_BOOL
DDS_CdrQosPolicy_deserialize_destination_order(struct CDR_Stream_t *stream,
        struct DDS_DestinationOrderQosPolicy *destination_order,
        void *param)
{
    UNUSED_ARG(param);

    return  CDR_Stream_deserialize_unsigned_long(stream,
                                (RTI_UINT32 *) & destination_order->kind);
}

RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_destination_order(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += DDS_Cdr_get_parameter_header_max_size_serialized(size);
    size += CDR_get_max_size_serialized_long(0);
    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return (size - orig_size);
}

/*----------------------------------------------------------------------------*/

RTI_BOOL
DDS_CdrQosPolicy_serialize_product_version(struct CDR_Stream_t *stream,
                                           const struct DDS_ProductVersion *product,
                                           void *param)
{
    UNUSED_ARG(param);

    /* Product version */
    if (!DDS_CdrStream_serialize_non_primitive_parameter
            (stream, product,
                    (CDR_Stream_SerializeFunction) DDS_CdrStream_serialize_4_octets,
                    DISC_RTPS_PID_PRODUCT_VERSION, RTI_FALSE, RTI_TRUE))
    {
        DDSC_LOG_SERIALIZE_PRODUCT_VERSION(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
DDS_CdrQosPolicy_deserialize_product_version(struct CDR_Stream_t *stream,
        struct DDS_ProductVersion *product,
        void *param)
{
    RTI_BOOL ok = RTI_FALSE;
    UNUSED_ARG(param);

    ok = CDR_Stream_deserialize_char(stream, (CDR_Char *)&product->major);
    ok = ok && CDR_Stream_deserialize_char(stream, (CDR_Char *)&product->minor);
    ok = ok && CDR_Stream_deserialize_char(stream, (CDR_Char *)&product->release);
    ok = ok && CDR_Stream_deserialize_char(stream, (CDR_Char *)&product->revision);

    return ok;
}

RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_product_version(RTI_UINT32 size)
{
    RTI_UINT32 origSize = size;

    size += DDS_Cdr_get_parameter_header_max_size_serialized(size);

    /* Add 4 total bytes, 1 for each field of product version */
    size += 4;

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return (size - origSize);
}

/*----------------------------------------------------------------------------*/

RTI_BOOL
DDS_CdrQosPolicy_serialize_lease_duration(struct CDR_Stream_t *stream,
        const struct DDS_Duration_t *lease_duration,
        void *param)
{
    UNUSED_ARG(param);

    if (!DDS_CdrStream_serialize_non_primitive_parameter
            (stream, lease_duration,
            (CDR_Stream_SerializeFunction) DDS_Duration_serialize,
            RTPS_PID_LEASE_DURATION, RTI_FALSE, RTI_TRUE))
    {
        DDSC_LOG_SERIALIZE_LEASE_DURATION(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
DDS_CdrQosPolicy_deserialize_lease_duration(struct CDR_Stream_t *stream,
        struct DDS_Duration_t *lease_duration,
        void *param)
{
    UNUSED_ARG(param);

    if (!DDS_Duration_deserialize(stream,
                                  lease_duration,
                                  param))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_lease_duration(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += DDS_Cdr_get_parameter_header_max_size_serialized(size);
    size += 3 * CDR_get_max_size_serialized_long(0);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return (size - orig_size);
}

/*----------------------------------------------------------------------------*/

RTI_BOOL
DDS_CdrQosPolicy_serialize_latency_budget(struct CDR_Stream_t *stream,
                  const struct DDS_LatencyBudgetQosPolicy *latency_budget,
                  void *param)
{
    UNUSED_ARG(param);

    if (!DDS_CdrStream_serialize_non_primitive_parameter
            (stream, &latency_budget->duration,
            (CDR_Stream_SerializeFunction) DDS_Duration_serialize,
            RTPS_PID_LATENCY_BUDGET, RTI_FALSE, RTI_TRUE))
    {
        DDSC_LOG_SERIALIZE_LEASE_DURATION(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_BOOL
DDS_CdrQosPolicy_deserialize_latency_budget(struct CDR_Stream_t *stream,
                      struct DDS_LatencyBudgetQosPolicy *latency_budget,
                      void *param)
{
    UNUSED_ARG(param);

    if (!DDS_Duration_deserialize(stream,
                                  &latency_budget->duration,
                                  param))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_latency_budget(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += DDS_Cdr_get_parameter_header_max_size_serialized(size);
    size += 2 * CDR_get_max_size_serialized_long(0);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return (size - orig_size);
}

/*----------------------------------------------------------------------------*/

RTI_PRIVATE RTI_BOOL
DDS_CdrStream_serialize_user_data_parameter(struct CDR_Stream_t *stream,
                                           const void *data,
                                           void *param)
{
    const struct DDS_UserDataQosPolicy *policy =
            (const struct DDS_UserDataQosPolicy *)data;

    UNUSED_ARG(param);

    return CDR_Stream_serialize_primitive_sequence(stream,
            (const struct REDA_Sequence*) &policy->value,
            CDR_OCTET_TYPE);
}

RTI_BOOL
DDS_CdrQosPolicy_serialize_user_data(struct CDR_Stream_t *stream,
                                     const struct DDS_UserDataQosPolicy *user_data)
{
    if (DDS_OctetSeq_get_length(&user_data->value) == 0)
    {
        /* Don't serialize empty user data */
        return RTI_TRUE;
    }

    return DDS_CdrStream_serialize_non_primitive_parameter(stream,
            user_data,
            (CDR_Stream_SerializeFunction)
            DDS_CdrStream_serialize_user_data_parameter,
            RTPS_PID_USER_DATA, RTI_FALSE, RTI_TRUE);
}

RTI_BOOL
DDS_CdrQosPolicy_deserialize_user_data(struct CDR_Stream_t *stream,
                                       struct DDS_UserDataQosPolicy *user_data,
                                       DDS_UserDataManager_T *user_data_manager,
                                       DDS_UserDataType user_data_type)
{
    struct DDS_OctetSeq stream_data = DDS_SEQUENCE_INITIALIZER;
    RTI_BOOL ok = RTI_FALSE;

    /* Get deserialization stream as an octet sequence */
    if (!CDR_Stream_deserialize_byte_sequence_without_copy(
            stream, (struct REDA_Sequence *)&stream_data, CDR_OCTET_TYPE))
    {
        goto done;
    }

    /* Assert stream_data into user data */
    if (!DDS_UserDataManager_assert_user_data(
            user_data_manager, user_data_type, &stream_data, &user_data->value))
    {
        goto done;
    }
    ok = RTI_TRUE;

done:
    return ok;
}

RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_user_data(RTI_UINT32 size,
                                                   RTI_UINT32 max_length)
{
    RTI_UINT32 orig_size = size;

    size += DDS_Cdr_get_parameter_header_max_size_serialized(size);
    size += CDR_get_max_size_serialized_primitive_sequence(
            size, max_length, CDR_OCTET_TYPE);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return size - orig_size;
}

/*----------------------------------------------------------------------------*/

RTI_PRIVATE RTI_BOOL
DDS_CdrStream_serialize_group_data_parameter(struct CDR_Stream_t *stream,
                                             const void *data,
                                             void *param)
{
    const struct DDS_GroupDataQosPolicy *policy =
            (const struct DDS_GroupDataQosPolicy *)data;

    UNUSED_ARG(param);

    return CDR_Stream_serialize_primitive_sequence(stream,
            (const struct REDA_Sequence*) &policy->value,
            CDR_OCTET_TYPE);
}

RTI_BOOL
DDS_CdrQosPolicy_serialize_group_data(struct CDR_Stream_t *stream,
                                      const struct DDS_GroupDataQosPolicy *group_data)
{
    if (DDS_OctetSeq_get_length(&group_data->value) == 0)
    {
        /* Don't serialize empty group data */
        return RTI_TRUE;
    }

    return DDS_CdrStream_serialize_non_primitive_parameter(stream,
            group_data,
            (CDR_Stream_SerializeFunction)
            DDS_CdrStream_serialize_group_data_parameter,
            RTPS_PID_GROUP_DATA, RTI_FALSE, RTI_TRUE);
}

RTI_BOOL
DDS_CdrQosPolicy_deserialize_group_data(struct CDR_Stream_t *stream,
                                       struct DDS_GroupDataQosPolicy *group_data,
                                       DDS_UserDataManager_T *user_data_manager,
                                       DDS_UserDataType group_data_type)
{
    struct DDS_OctetSeq stream_data = DDS_SEQUENCE_INITIALIZER;
    RTI_BOOL ok = RTI_FALSE;

    /* Get deserialization stream as an octet sequence */
    if (!CDR_Stream_deserialize_byte_sequence_without_copy(
            stream, (struct REDA_Sequence *)&stream_data, CDR_OCTET_TYPE))
    {
        goto done;
    }

    /* Assert stream_data into user data */
    if (!DDS_UserDataManager_assert_user_data(
            user_data_manager, group_data_type, &stream_data, &group_data->value))
    {
        goto done;
    }
    ok = RTI_TRUE;

done:
    return ok;
}

RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_group_data(RTI_UINT32 size,
                                                    RTI_UINT32 max_length)
{
    RTI_UINT32 orig_size = size;

    size += DDS_Cdr_get_parameter_header_max_size_serialized(size);
    size += CDR_get_max_size_serialized_primitive_sequence(
            size, max_length, CDR_OCTET_TYPE);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return size - orig_size;
}

/*----------------------------------------------------------------------------*/

RTI_PRIVATE RTI_BOOL
DDS_CdrStream_serialize_topic_data_parameter(struct CDR_Stream_t *stream,
                                             const void *data,
                                             void *param)
{
    const struct DDS_TopicDataQosPolicy *policy =
            (const struct DDS_TopicDataQosPolicy *)data;

    UNUSED_ARG(param);

    return CDR_Stream_serialize_primitive_sequence(stream,
            (const struct REDA_Sequence*) &policy->value,
            CDR_OCTET_TYPE);
}

RTI_BOOL
DDS_CdrQosPolicy_serialize_topic_data(struct CDR_Stream_t *stream,
                                      const struct DDS_TopicDataQosPolicy *topic_data)
{
    if (DDS_OctetSeq_get_length(&topic_data->value) == 0)
    {
        /* Don't serialize empty topic data */
        return RTI_TRUE;
    }

    return DDS_CdrStream_serialize_non_primitive_parameter(stream,
            topic_data,
            (CDR_Stream_SerializeFunction)
            DDS_CdrStream_serialize_topic_data_parameter,
            RTPS_PID_TOPIC_DATA, RTI_FALSE, RTI_TRUE);
}

RTI_BOOL
DDS_CdrQosPolicy_deserialize_topic_data(struct CDR_Stream_t *stream,
                                        struct DDS_TopicDataQosPolicy *topic_data,
                                        DDS_UserDataManager_T *user_data_manager,
                                        DDS_UserDataType topic_data_type)
{
    struct DDS_OctetSeq stream_data = DDS_SEQUENCE_INITIALIZER;
    RTI_BOOL ok = RTI_FALSE;

    /* Get deserialization stream as an octet sequence */
    if (!CDR_Stream_deserialize_byte_sequence_without_copy(
            stream, (struct REDA_Sequence *)&stream_data, CDR_OCTET_TYPE))
    {
        goto done;
    }

    /* Assert stream_data into user data */
    if (!DDS_UserDataManager_assert_user_data(
            user_data_manager, topic_data_type, &stream_data, &topic_data->value))
    {
        goto done;
    }
    ok = RTI_TRUE;

done:
    return ok;
}

RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_topic_data(RTI_UINT32 size,
                                                    RTI_UINT32 max_length)
{
    RTI_UINT32 orig_size = size;

    size += DDS_Cdr_get_parameter_header_max_size_serialized(size);
    size += CDR_get_max_size_serialized_primitive_sequence(
            size, max_length, CDR_OCTET_TYPE);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return size - orig_size;
}

/*----------------------------------------------------------------------------*/

RTI_BOOL
DDS_CdrQosPolicy_serialize_send_queue_size(struct CDR_Stream_t *stream,
        DDS_Long send_queue_size,
        void *param)
{
    RTI_BOOL ok = RTI_TRUE;
    UNUSED_ARG(param);

    ok = DDS_CdrStream_serialize_parameter(stream,RTPS_PID_SEND_QUEUE_SIZE_DEPRECATED,
            (DDS_UnsignedShort)CDR_get_max_size_serialized_long(0));

    ok = ok && CDR_Stream_serialize_long(stream, (RTI_INT32 *) & send_queue_size);

    return ok;
}

RTI_PRIVATE RTI_BOOL
DDS_CdrStream_serialize_property_parameter(struct CDR_Stream_t *stream,
                                           const void *data,
                                           void *param)
{
    RTI_BOOL ok = RTI_TRUE;
    const struct DDS_PropertyQosPolicy *const properties =
            (const struct DDS_PropertyQosPolicy *const) data;

    UNUSED_ARG(param);

    ok = ok && CDR_Stream_serialize_property_sequence(
                    stream,&properties->value,NULL);

    return ok;
}

RTI_BOOL
DDS_CdrQosPolicy_serialize_property(struct CDR_Stream_t *stream,
                                    const struct DDS_PropertyQosPolicy *properties)
{
    RTI_BOOL ok = RTI_TRUE;

    ok = ok && DDS_CdrStream_serialize_non_primitive_parameter(stream,
            properties,
            (CDR_Stream_SerializeFunction)
            DDS_CdrStream_serialize_property_parameter,
            RTPS_PID_PROPERTY_LIST, RTI_FALSE,RTI_TRUE);

    return ok;
}

/*ci \brief Get the maximum serialized size of a property sequence
 *
 * \details
 *
 * The maximum serialized size of a property sequence is calculated based on
 * serializing the folowing properties.
 *
 * The following properties are serialized:
 * DDS_PROPERTY_HOSTNAME_NAME = "dds.sys_info.hostname" (64)
 * DDS_PROPERTY_PROCESSID_NAME = "dds.sys_info.process_id" (16)
 * DDS_PROPERTY_TARGET_NAME = "dds.sys_info.target" (64)
 * DDS_PROPERTY_VERSION_NAME = "rti.service.version (9)";
 */
RTI_UINT32
DDS_CdrQosPolicy_get_max_size_serialized_property(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    size += CDR_get_max_size_serialized_non_primitive_sequence(
                    NULL,size,
                    DDSC_PARTICIPANT_MAX_PROPERTIES,
                    CDR_get_max_size_serialized_property);

    return (size - orig_size);
}

RTI_UINT32
DDS_CdrQosPolicy_get_max_serialized_size_data_representation(
                                struct DDS_DataRepresentationQosPolicy *policy,
                                RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += DDS_Cdr_get_parameter_header_max_size_serialized(0);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    /* Sequence length */
    size += CDR_get_max_size_serialized_long(0);

    /* Times number of elements */
    size += (RTI_UINT32)DDS_DataRepresentationIdSeq_get_maximum(&policy->value) *
                        CDR_get_max_size_serialized_short(0);

    /* Align next pid, if any */
    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return (size - orig_size);
}


RTI_BOOL
DDS_CdrQosPolicy_deserialize_data_representation(
                                struct CDR_Stream_t *stream,
                                struct DDS_DataRepresentationQosPolicy *policy,
                                void *param)
{
    struct DDS_DataRepresentationIdSeq *drep;
    RTI_INT32 index,length;
    RTI_BOOL retval = RTI_FALSE;
    UNUSED_ARG(param);

    if (!CDR_Stream_deserialize_long(stream,&length))
    {
        goto done;
    }

    drep = &policy->value;

    if (!DDS_DataRepresentationIdSeq_set_maximum(drep,length))
    {
        goto done;
    }

    if (!DDS_DataRepresentationIdSeq_set_length(drep,length))
    {
        goto done;
    }

    for (index = 0; index < length; ++index)
    {
        if (!CDR_Stream_deserialize_short(stream,
                         DDS_DataRepresentationIdSeq_get_reference(drep,index)))
        {
            goto done;
        }
    }

    retval = RTI_TRUE;

done:

    return retval;
}

RTI_PRIVATE RTI_BOOL
DDS_CdrStream_serialize_data_representation(
                                struct CDR_Stream_t *stream,
                                struct DDS_DataRepresentationQosPolicy *policy,
                                void *param)
{
    struct DDS_DataRepresentationIdSeq *drep;
    RTI_INT32 index,max_index;
    RTI_UINT32 offset;
    RTI_BOOL retval = RTI_FALSE;
    UNUSED_ARG(param);

    offset = CDR_Stream_get_current_position_offset(stream);
    drep = &policy->value;
    max_index = DDS_DataRepresentationIdSeq_get_length(drep);

    if (!CDR_Stream_serialize_long(stream,&max_index))
    {
        goto done;
    }

    for (index = 0; index < max_index; ++index)
    {
        if (!CDR_Stream_serialize_short(stream,
                         DDS_DataRepresentationIdSeq_get_reference(drep,index)))
        {
            goto done;
        }
    }

    retval = RTI_TRUE;

done:

    if (!retval)
    {
        if (!CDR_Stream_set_current_position_offset(stream,offset))
        {
            return RTI_FALSE;
        }
    }

    return retval;
}

RTI_BOOL
DDS_CdrQosPolicy_serialize_data_representation(
                                struct CDR_Stream_t *stream,
                                const struct DDS_DataRepresentationQosPolicy *policy)
{

    if (!DDS_CdrStream_serialize_non_primitive_parameter(stream,policy,
                    (CDR_Stream_SerializeFunction)
                    DDS_CdrStream_serialize_data_representation,
                    RTPS_PID_DATA_REPRESENTATION, RTI_FALSE, RTI_TRUE))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
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
        if (!DDS_CdrStream_serialize_non_primitive_parameter
                (stream, checksum_property,
                 (CDR_Stream_SerializeFunction) DDS_Cdr_serializeChecksum,
                 RTPS_PID_CHECKSUM_PROPERTY, RTI_FALSE, RTI_TRUE))
        {
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

    size += DDS_Cdr_get_parameter_header_max_size_serialized(size);
    size += 2 * CDR_get_max_size_serialized_short(0) +
            CDR_get_max_size_serialized_boolean(0);

    size = CDR_align_upwards(size, CDR_DEFAULT_PARAMETER_ALIGNMENT);

    return (size - orig_size);
}
