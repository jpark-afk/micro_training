/*
 * FILE: netio_common.h - NETIO Common API
 *
 * Copyright (c) 2012-2024 Real-Time Innovations, Inc.
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
 * 28jun2016,tk MICRO-1548 Removed reliance on system headers for UDP
 * 16mar2015,tk MICRO-1117 Added RTPS and UDP constants for UDP and RTPS
 *                         header and trailer space.
 * 25apr2012,tk Written
 */
/*ce
 * \file
 */
/*ci \defgroup NETIOCommon NETIO Common API
 *   \ingroup  NETIOModule
 *   \brief    NETIO APIs available to all NETIO interface
 */
/*ci \defgroup NETIOUtility NETIO Utility API
 *   \ingroup  NETIOCommon
 *   \brief   Utility function provided by the NETIO module
 */
/*ci \addtogroup NETIOUtility
 * @{
 */
#ifndef netio_common_h
#define netio_common_h

#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef reda_buffer_h
#include "reda/reda_buffer.h"
#endif
#ifndef netio_dll_h
#include "netio/netio_dll.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*ci
 * The number of tables created by NETIO common functionality cannot exceed
 * this number. This number includes tables needed by the NETIO_BindResolver,
 * METIO_RouteResolver and NETIO_AddressResolver
 */
#define NETIO_RESOURCE_COMMON_TABLES        (3U)

/*ci
 * A standard NETIO interface typically only uses 2 tables, one route and
 * one bind
 */
#define NETIO_RESOURCE_TABLES_PER_INTERFACE (2U)

/*ci
 * The number of tables needed by and external transport. Although it
 * is a NETIO interface, it often has additional functionality such as
 * NAT and thread tables.
 */
#define NETIO_RESOURCE_EXTERNAL_TABLE       (4U)

/*ci
 * \def NETIO_ntohs
 * \brief Convert a short integer (16bit) received from an IP network to a
 *        short in host order
 */
#ifdef RTI_ENDIAN_LITTLE
#define NETIO_ntohs(x_) (RTI_UINT16)(\
                         ((((RTI_UINT16)(x_))&0xff00)>>8) | \
                         ((((RTI_UINT16)(x_))&0x00ff)<<8))
#else
#define NETIO_ntohs(x_) (RTI_UINT16)(x_)
#endif

/*ci
 * \def NETIO_htons
 * \brief Convert a short integer (16bit) in host order a to short integer
 *        IP network order
 */
#ifdef RTI_ENDIAN_LITTLE
#define NETIO_htons(x_) (RTI_UINT16)( \
                         ((((RTI_UINT16)(x_))&0xff00)>>8) | \
                         ((((RTI_UINT16)(x_))&0x00ff)<<8))
#else
#define NETIO_htons(x_) (RTI_UINT16)(x_)
#endif

/*ci
 * \def NETIO_ntohl
 * \brief Convert a long integer (32bit) received from an IP network to a
 *        long in host order
 */
#ifdef RTI_ENDIAN_LITTLE
#define NETIO_ntohl(x_) (RTI_UINT32)(( \
                         ((((RTI_UINT32)(x_))&0xff000000U)>>24) | \
                         ((((RTI_UINT32)(x_))&0x00ff0000U)>>8)  | \
                         ((((RTI_UINT32)(x_))&0x000000ffU)<<24) | \
                         ((((RTI_UINT32)(x_))&0x0000ff00U)<<8)))
#else
#define NETIO_ntohl(x_) (RTI_UINT32)(x_)
#endif

/*ci
 * \def NETIO_htonl
 * \brief Convert a long integer (32bit) in host order a to long integer
 *        IP network order
 */
#ifdef RTI_ENDIAN_LITTLE
#define NETIO_htonl(x_) (((((RTI_UINT32)(x_))&0xff000000U)>>24) | \
                         ((((RTI_UINT32)(x_))&0x00ff0000U)>>8)  | \
                         ((((RTI_UINT32)(x_))&0x000000ffU)<<24) | \
                         ((((RTI_UINT32)(x_))&0x0000ff00U)<<8))
#else
#define NETIO_htonl(x_) (RTI_UINT32)(x_)
#endif

/*ci
 * \def NETIO_PING_MSG
 * \brief Internal RTPS ping message which serves prime ARP table when a
 *        destination address is added.
 */
#define NETIO_PING_MSG      "RTIPING"

/*ci
 * \def NETIO_PING_MSG_SIZE
 * \brief The length in bytes, excluding the NUL character, of internal
 *        ping message
 */
#define NETIO_PING_MSG_SIZE (7)

#if RTI_ENDIAN_LITTLE
#define NETIO_INFO_TS_PING_MSG "RTPS\2\1\1\1\0\0\0\0\0\0\0\0\0\0\0\0\x9\x3\0\0"
#else
#define NETIO_INFO_TS_PING_MSG "RTPS\2\1\1\1\0\0\0\0\0\0\0\0\0\0\0\0\x9\x2\0\0"
#endif
#define NETIO_INFO_TS_PING_MSG_SIZE (24)


#define NETIO_CONNEXT_PING_MSG_SIZE (16)

/*ci
 * \def RTPS_PACKET_HEADER_MAX_LENGTH
 * \brief The maximum amount of header space needed to send an RTPS data-sample
 * \details
 *
 * The RTPS header space includes space of the RTPS header and a single
 * TS and DATA sub message. NOTE: The space for the DATA payload is _not_
 * included in this constant.
 *
 * - This number is calculated using the following layout
 *
 *   +-----------------------------+
 *   |           HEADER            | 16 bytes (always)
 *   +-----------------------------+
 *   |          INFO_TS            | 12 bytes (no inline Qos)
 *   +-----------------------------+
 *   |       DATA SUBMESSAGE       | 24 bytes (no inline Qos)
 *   +-----------------------------+
 *
 *   +-----------------------------+
 *   |           HEADER            | 16 bytes (always)
 *   +-----------------------------+
 *   |          INFO_TS            | 12 bytes (no inline Qos)
 *   +-----------------------------+
 *   |      DATA SUBMESSAGE        | 52 bytes (with status info (8)
 *   |                             |           and keyhash     (20))
 *   +-----------------------------+
 *
 *   The max size is  80
 *
 *   However, because the max header size is hard-coded, 256 was chosen as a
 *   reasonably big header size for other RTPS or downstream interfaces.
 */
#define RTPS_PACKET_HEADER_MAX_LENGTH  (256)

/*ci
 * \def RTPS_PACKET_TRAILER_MAX_LENGTH
 * \brief The maximum amount of trailer space needed to send an RTPS data-sample
 *
 * \details
 * The RTPS trailer includes space for an optional RTPS piggyback heart-beat
 *
 * Although the size of a piggyback HB is only 32 byes, 64 bytes was chosen
 * in case there are implementations that may need more and this is currently
 * hard-coded.
 */
#define RTPS_PACKET_TRAILER_MAX_LENGTH (128)

/*ci
 * \def UDP_PACKET_HEADER_MAX_LENGTH
 * \brief The maximum amount of header space needed to send a UDP data-sample
 *
 * \details
 * The UDP transport is using an existing UDP stack and does not add any
 * UDP information. However, size is chosen based on a fictional
 * minimum UDP/IPv4 implementation with UDP(8) + IP (20) = 28 padded up to 32
 */
#define UDP_PACKET_HEADER_MAX_LENGTH   (32)

/*ci
 * \def UDP_PACKET_TRAILER_MAX_LENGTH
 * \brief The maximum amount of trailer space needed to send a UDP data-sample
 *
 * \details
 * The UDP transport is using an existing UDP stack and does not add any
 * UDP information. A fictional implementation does not add any header or
 * trailer thus it is set to 0
 */
#define UDP_PACKET_TRAILER_MAX_LENGTH  (0)

/*ci
 * 
 * \brief The maximum size of an IP packet
 */
#define NETIO_IP_PACKET_MAX_SIZE (65535)

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif /* netio_common_h */

/*ci @} */
