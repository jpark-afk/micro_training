/*
 * FILE: Cdr.c - RTPS bitmap handling functions
 *
 * Copyright 2008-2021 Real-Time Innovations, Inc.
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
 * 27may2022,am MICRO-3537/PR.30409
 * - Added check for multicast for udpv4 locators and adding multicast flags
 *   when multicast UDPv4 locators are detected. 
 * 30jun2015,eh  MICRO-1373/PR#15164 Fix magic numbers
 * 30jun2015,eh  MICRO-1374/PR#15168 Rename CDR_Stream_has_free_space to
 *               CDR_Stream_has_remaining_space
 * 11jun2015,eh  MICRO-1295/PR#14963 Document necessary conversion of
 *               UDPv4 into UDPv6 when serializing locator
 * 10jun2015,eh  MICRO-1276/PR#14897 Remove redundant zeroing of IPv4 locator
 *               address
 * 09jun2015,eh  MICRO-1296/PR#14964 Check stream free space for
 *              (de)serialize_ip6_locator
 * 23mar2015,eh  MICRO-314/PR#1440 fail on deserializing unsupported locator
 *               kind
 * 17mar2015,eh  MICRO-313/PR#1439 remove unnecessary stream align
 * 23feb2015,eh  MICRO-1081: fix function/var names to conform to coding std
 * 23feb2015,eh  MICRO-1075: remove and replace macros
 * 07may2014,eh  MICRO-313/VerocelPR 1439: CDR_Stream_Align()
 * 28dec2008,rmw Changing IPv6 deserialization to correctly deserialize
 *                        IPv4 addresses into our IPv4 locators.
 * 22oct2008,rmw Adding support for well known discovery typename and
 *                     topic name.
 * 13sep2008,rmw Removal of debug code
 * 03sep2008,rmw Added (de)serialization of time and GUIDs
 * 30apr2008,rmw Created
 */

/*ci 
 *  
 * \brief Serialization and deserialization operations for types used within 
 * RTPS messages and submessages 
 *  
 */

#ifndef osapi_time_h
#include "osapi/osapi_time.h"
#endif
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef osapi_string_h
#include "osapi/osapi_string.h"
#endif
#ifndef cdr_serialize_h
#include "cdr/cdr_serialize.h"
#endif
#ifndef cdr_stream_h
#include "cdr/cdr_stream.h"
#endif
#ifndef rtps_rtps_h
#include "rtps/rtps_rtps.h"
#endif
#ifndef rtps_log_h
#include "rtps/rtps_log.h"
#endif

/*** SOURCE_BEGIN ***/

/*ci
 * \brief
 * Serialize one field of GUID
 *  
 * \param[in] stream Serialization stream 
 * \param[in] field Field of GUID to serialize 
 * \param[in] param Unused 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_GuidField_serialize(struct CDR_Stream_t *stream,
                         const RTI_UINT32 *field,
                         void *param)
{
    UNUSED_ARG(param);

    /* HostId, AppId, instance_id , or object_id */
    if (!CDR_Stream_serialize_unsigned_long_to_big_endian(stream, field))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief
 * Deserialize one field of GUID
 *  
 * \param[in] stream Deserialization stream 
 * \param[out] field Deserialized field of GUID 
 * \param[in] param Unused 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
RTPS_GuidField_deserialize(struct CDR_Stream_t *stream,
                           RTI_UINT32 *field,
                           void *param)
{
    UNUSED_ARG(param);

    /* HostId, AppId, instance_id , or object_id */
    if (!CDR_Stream_deserialize_unsigned_long_from_big_endian(stream, field))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief
 * Serialize a GUID
 *  
 * \param[in] stream Serialization stream 
 * \param[in] guid GUID to serialize 
 * \param[in] param Unused 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
RTI_BOOL
RTPS_Guid_serialize(struct CDR_Stream_t *stream,
                    const struct RTPS_Guid *guid,
                    void *param)
{
    UNUSED_ARG(param);

    if (!RTPS_GuidField_serialize(stream, &guid->prefix.host_id, NULL))
    {
        RTPS_LOG_SERIALIZE_HOST_ID(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (!RTPS_GuidField_serialize(stream, &guid->prefix.app_id , NULL))
    {
        RTPS_LOG_SERIALIZE_APP_ID(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (!RTPS_GuidField_serialize(stream, &guid->prefix.instance_id , NULL))
    {
        RTPS_LOG_SERIALIZE_INSTANCE_ID(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (!RTPS_GuidField_serialize(stream, &guid->object_id, NULL))
    {
        RTPS_LOG_SERIALIZE_OBJECT_ID(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief
 * Deserialize a GUID
 *  
 * \param[in] stream Deserialization stream 
 * \param[out] guid Deserialized GUID 
 * \param[in] param Unused 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
RTI_BOOL
RTPS_Guid_deserialize(struct CDR_Stream_t *stream,
                      struct RTPS_Guid *guid,
                      void *param)
{
    UNUSED_ARG(param);

    if (!RTPS_GuidField_deserialize(stream, &guid->prefix.host_id, NULL))
    {
        RTPS_LOG_DESERIALIZE_HOST_ID(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (!RTPS_GuidField_deserialize(stream, &guid->prefix.app_id , NULL))
    {
        RTPS_LOG_DESERIALIZE_APP_ID(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (!RTPS_GuidField_deserialize(stream, &guid->prefix.instance_id , NULL))
    {
        RTPS_LOG_DESERIALIZE_INSTANCE_ID(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    if (!RTPS_GuidField_deserialize(stream, &guid->object_id, NULL))
    {
        RTPS_LOG_DESERIALIZE_OBJECT_ID(OSAPI_LOGKIND_ERROR)
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief
 * Return number of bytes to serialize a GUID
 *  
 * \param[in] size Current size of serialized stream 
 *  
 * \return Number of additional bytes to serialize a GUID
 */
RTI_UINT32
RTPS_Guid_get_max_size_serialized(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    /* Host Id */
    size += CDR_get_4_byte_max_size_serialized(size);

    /* App Id */
    size += CDR_get_4_byte_max_size_serialized(size);

    /* Instance Id */
    size += CDR_get_4_byte_max_size_serialized(size);

    /* Object Id */
    size += CDR_get_4_byte_max_size_serialized(size);

    return (size - orig_size);
}

/* ----------------------------------------------------------------- */

/*ci
 * \brief
 * Serialize NTP time
 *  
 * \param[in] stream Serialization stream 
 * \param[in] time NTP time to serialize
 * \param[in] param Unused 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
RTI_BOOL
RTPS_serialize_ntp_time(struct CDR_Stream_t *stream,
                        const OSAPI_NtpTime *time,
                        void *param)
{
    UNUSED_ARG(param);

    if (!CDR_Stream_serialize_long(stream, &time->sec))
    {
        return RTI_FALSE;
    }

    if (!CDR_Stream_serialize_unsigned_long(stream, &time->frac))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief
 * Deserialize NTP time
 *  
 * \param[in] stream Deserialization stream 
 * \param[out] time Deserialized NTP time 
 * \param[in] param Unused 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
RTI_BOOL
RTPS_deserialize_ntp_time(struct CDR_Stream_t *stream,
                          OSAPI_NtpTime *time,
                          void *param)
{
    UNUSED_ARG(param);

    if (!CDR_Stream_deserialize_long(stream, &time->sec))
    {
        return RTI_FALSE;
    }

    if (!CDR_Stream_deserialize_unsigned_long(stream, &time->frac))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*
 * \brief
 * Return number of bytes to serialize an NTP time type
 *  
 * \param[in] size Current size of serialized stream 
 *  
 * \return Number of additional bytes to serialize an NTP time type
 */
RTI_UINT32
RTPS_get_ntp_time_max_size_serialized(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    size += CDR_get_max_size_serialized_long(size);
    size += CDR_get_max_size_serialized_unsigned_long(size);

    return (size - orig_size);
}

/* ----------------------------------------------------------------- */

/*ci
 * \brief
 * Serialize unsigned short, of length 2 octets
 *  
 * \param[in] stream Serialization stream 
 * \param[in] in Unsigned short to serialize
 * \param[in] param Unused 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
RTI_BOOL
RTPS_serialize_2_octets(struct CDR_Stream_t *stream,
                        const RTI_UINT16 *in,
                        void *param)
{
    UNUSED_ARG(param);

    if (!CDR_Stream_check_size(stream, CDR_UNSIGNED_SHORT_SIZE))
    {
        return RTI_FALSE;
    }

    if (!CDR_Stream_serialize_unsigned_short_to_big_endian(stream, in))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci
 * \brief
 * Deserialize unsigned short, of length 2 octets
 *  
 * \param[in] stream Deserialization stream 
 * \param[out] out Deserialized unsigned short 
 * \param[in] param Unused 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
RTI_BOOL
RTPS_deserialize_2_octets(struct CDR_Stream_t *stream,
                          RTI_UINT16 *out,
                          void *param)
{
    UNUSED_ARG(param);

    if (!CDR_Stream_check_size(stream, CDR_UNSIGNED_SHORT_SIZE))
    {
        return RTI_FALSE;
    }

    return CDR_Stream_deserialize_unsigned_short_from_big_endian(stream, out);
}


/* ----------------------------------------------------------------- */

/*ci
 * \brief
 * Serialize IPv6 locator 
 *  
 * \details 
 * Locator may contain IPv6 address or IPv4 address (i.e. of 16-byte address, 
 * first 12 bytes are zeros, last 4 bytes contain IPv4 address) 
 *  
 * \param[in] stream Serialization stream 
 * \param[in] loc Locator to serialize
 * \param[in] param Unused 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
RTI_BOOL
RTPS_serialize_ipv6_locator(struct CDR_Stream_t *stream,
                            const struct RTPS_Locator_t *loc,
                            void *param)
{
    RTI_INT32 i;
    RTI_INT32 kind;
    UNUSED_ARG(param);

    kind = NETIO_Address_get_kind((struct NETIO_Address*)loc);
    if (!CDR_Stream_serialize_long(stream, &kind))
    {
        return RTI_FALSE;
    }

    if (!CDR_Stream_serialize_long(stream, &loc->port))
    {
        return RTI_FALSE;
    }

    if (!CDR_Stream_has_remaining_space(stream, RTPS_LOCATOR_ADDRESS_LENGTH_MAX))
    {
        return RTI_FALSE;
    }

    for (i = 0; i < RTPS_LOCATOR_ADDRESS_LENGTH_MAX; ++i)
    {
        *(stream->buff_ptr++) = (char)loc->address[i];
    }

    return RTI_TRUE;
}

/*ci
 * \brief
 * Deserialize IPv6 locator
 *  
 * \details 
 * The locator being deserialized may be of kind UDPv4, which has its 
 * 4-byte UDPv4 address in the last 4-bytes of the 16-byte address array. 
 *  
 * A UDPv6 locator with UDPv4 address is deserialized into a UDPv4 locator. 
 *  
 * Subsequent serialization of the UDPv4 using the UDPv6 serialization 
 * function must first convert the UDPv4 locator into a UDPv6 locator.  
 *  
 * \param[in] stream Deserialization stream 
 * \param[out] loc Deserialized locator
 * \param[in] param Unused 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
RTI_BOOL
RTPS_deserialize_ipv6_locator(struct CDR_Stream_t *stream,
                              struct RTPS_Locator_t *loc,
                              void *param)
{
    RTI_INT32 i;
    struct RTPS_LocatorUdpv4_t *udpv4_loc;
    UNUSED_ARG(param);

    if (!CDR_Stream_deserialize_long(stream, &loc->kind))
    {
        return RTI_FALSE;
    }

    if (!CDR_Stream_deserialize_unsigned_long(stream, &loc->port))
    {
        return RTI_FALSE;
    }

    if (!CDR_Stream_has_remaining_space(stream, RTPS_LOCATOR_ADDRESS_LENGTH_MAX))
    {
        return RTI_FALSE;
    }

    if (NETIO_Address_get_kind((struct NETIO_Address*)loc) != RTPS_LOCATOR_KIND_UDPv4)
    {
        for (i = 0; i < RTPS_LOCATOR_ADDRESS_LENGTH_MAX; ++i)
        {
            loc->address[i] = (RTI_UINT8)(*(stream->buff_ptr++));
        }
    }
    else
    {
        stream->buff_ptr = stream->buff_ptr + RTPS_LOCATOR_ADDRESS_UDPV4_OFFSET;
        udpv4_loc = (struct RTPS_LocatorUdpv4_t *)loc;

        if (!CDR_Stream_deserialize_unsigned_long_from_big_endian(
                                                  stream, &udpv4_loc->address))
        {
            return RTI_FALSE;
        }

        /*check if the address is udpv4 multicast address*/
        /* IPv4 addresses 224.0.0.0 to 239.255.255.255 are reserved for multicast*/
        if ((udpv4_loc->address >= 0xe0000000) && (udpv4_loc->address <= 0xefffffff))
        {
            udpv4_loc->kind |= (RTI_INT32)NETIO_ADDRESS_FLAG_MULTICAST;
        }
    }

    return RTI_TRUE;
}

/*ci 
 * \brief
 * Return number of bytes to serialize an IPv6 locator
 *  
 * \param[in] size Current size of serialized stream 
 *  
 * \return Number of additional bytes to serialize an IPv6 locator
 */
RTI_UINT32
RTPS_get_ipv6_locator_max_size_serialized(RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    /* NDDS_Transport_ClassId_t */
    size += CDR_get_max_size_serialized_long(size);

    /* NDDS_Transport_Port_t */
    size += CDR_get_max_size_serialized_unsigned_long(size);

    /* NDDS_Transport_Address_t */
    size += RTPS_LOCATOR_ADDRESS_LENGTH_MAX;

    return (size - orig_size);
}
