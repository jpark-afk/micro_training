/*
 * FILE: DPSEParticipantDiscovery.h - DPSE Participant Discovery
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
 * 20feb2015,eh MICRO-813/PR#9172 Fix Lint warnings
 * 03jun2008,rmw  Created.
 */
/*ce
 * \file
 * \brief DPSE Participant Discovery
 */
/*ci \addtogroup DPSEModule
 * @{
 */
#ifndef DPSEParticipantDiscovery_h
#define DPSEParticipantDiscovery_h

MUST_CHECK_RETURN extern DDS_ReturnCode_t
DPSE_ParticipantDiscovery_schedule_fast_assertions(
        struct NDDS_Discovery_Plugin *discovery_plugin,
        const struct DDS_ParticipantBuiltinTopicData *local_participant_data,
        DDS_Boolean new_event);

#endif

/*ci @} */

