/*
 * FILE: TcpIp_CfgTypes.h - Minimum needed AutoSAR 4.0.3 TCP/IP configuration
 *
 * (c) Copyright, Real-Time Innovations, 2020-2022
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * AUTOSAR_SWS_TcpIp.pdf
 */
#ifndef TCPIP_CFGTYPES_H
#define TCPIP_CFGTYPES_H

#include "Platform_Types.h"

#if defined(RTIME_AUTOSAR_OSEKCORE) || defined(RTIME_AUTOSAR_WINCORE)
/*ci \brief Local address type as defined in Standard
 */
typedef uint8 TcpIp_LocalAddrIdType;
#endif

/*ci \brief Dummy TcpIp_ConfigType struct
 */
typedef struct
{
    uint8 x;
} TcpIp_ConfigType;


#ifdef RTIME_AUTOSAR_WINCORE
typedef uint16 TcpIp_DomainType;
typedef struct /* TcpIp_SockAddrInetType */ {
    VAR( TcpIp_DomainType, TYPEDEF ) domain; /* This is the code for the address format of this address. */
    VAR( uint16, TYPEDEF ) port; /* port number. */
    VAR( uint32, TYPEDEF ) addr[1]; /* IPv4 address in network byte order. */
} TcpIp_SockAddrInetType;
#endif

#endif

