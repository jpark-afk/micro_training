/*
 * FILE: DomainParticipantChecksum.h - DomainParticipant checksum related functions
 *
 * (c) Copyright, Real-Time Innovations, 2020 - 2022.
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
 * 08sep2022,tk MICRO-3367/PR.30121
 * - Added missing modifiction history header
 */
#ifndef DomainParticipantChecksum_h
#define DomainParticipantChecksum_h

extern DDS_Boolean
DDS_DomainParticipant_checksum_configure(DDS_DomainParticipant *self,
                                         struct RT_ComponentFactory *rtps_factory,
                                         struct RTPS_InterfaceProperty *rtps_property);



#endif
