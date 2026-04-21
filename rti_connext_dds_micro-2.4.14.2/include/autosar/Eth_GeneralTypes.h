/*
 * FILE: Eth_GeneralTypes.h - Eth Mac types
 *
 * (c) Copyright, Real-Time Innovations, 2020-2020
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * AUTOSAR_SWS_EthernetInterface.pdf
 */

#ifndef ETH_GENERALTYPES_H
#define ETH_GENERALTYPES_H


/*
 * \brief         Frame type.
 * \details       This type is used to pass the value of type/length field in the
 *                Ethernet frame header. It is 16 bits long unsigned integer.
 *                - Values less than or equal to 1500 represent the length.
 *                - Values grater than 1500 represent the type (i.e. 0x800 = IP).
 */
typedef uint16 Eth_FrameType;


#endif /* ETH_GENERALTYPES_H */

