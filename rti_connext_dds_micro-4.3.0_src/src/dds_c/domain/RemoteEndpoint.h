/*
 * FILE: RemoteEndpoint.h - Remote endpoint implementation
 *
 * (c) Copyright, Real-Time Innovations, 2012-2015.
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
 * 17may2012,tk Written
 */
/*ce
 * \file
 * \brief Remote endpoint implementation
 */
/*ci \addtogroup DDSDomainModule
 * @{
 */
#ifndef RemoteEndpoint_h
#define RemoteEndpoint_h

MUST_CHECK_RETURN struct DDS_RemoteParticipantImpl*
NDDS_RemoteEndpoint_find_parent(DDS_DomainParticipant *self,
                   const DDS_BuiltinTopicKey_t *participant_key,
                   const char *participant_name,
                   const DDS_BuiltinTopicKey_t *endpoint_key);

#endif

/*ci @} */
