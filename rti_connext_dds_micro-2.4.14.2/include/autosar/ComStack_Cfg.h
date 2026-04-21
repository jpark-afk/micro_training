/*
 * FILE: ComStack_Cfg.h - Basic communication stack types
 *
 * (c) Copyright, Real-Time Innovations, 2020-2020
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

/* This file is required as it is included by AutoSAR headers when
 * compiling Micro. However, none of the contents are required by Micro.
 */

/*
 * Specification of Platform Types AUTOSAR CP Release 4.0.3
 * AUTOSAR_SWS_CommunicationStackTypes.pdf
 */
#ifndef COMSTACK_CFG_H
#define COMSTACK_CFG_H

/*ci \brief PduType
 *
 * \details
 *
 *  Unique identifier for a PDU within a software module. Per the spec,
 *  if _no_ software module deals with more then 256 PDUs, this value can
 *  be set to uint8. If at least one software module deals with _more_ than
 *  256 PDUs, this type must be set to uint16.
 */
#ifndef RTIME_AUTOSAR_PDUID_TYPE
#define RTIME_AUTOSAR_PDUID_TYPE uint16
#endif

typedef RTIME_AUTOSAR_PDUID_TYPE PduIdType;

/*ci \brief Max PDU length
 *
 * \details
 *
 * The maximum PDU length that can be sent by the software. Legal types are
 * uint8, uint16, or uint32.
 *
 */
#ifndef RTIME_AUTOSAR_PDU_LENGTH_TYPE
#define RTIME_AUTOSAR_PDU_LENGTH_TYPE uint16
#endif
typedef RTIME_AUTOSAR_PDU_LENGTH_TYPE PduLengthType;

#endif
