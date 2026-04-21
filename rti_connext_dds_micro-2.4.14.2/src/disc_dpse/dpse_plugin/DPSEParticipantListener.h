/*
 * FILE: DPSEParticipantListener.h - DPSE Participant Listener
 *
 * (c) Copyright 2008-2015 Real-Time Innovations,
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
 * 03jun2008,rmw  Created.
 */
/*ce
 * \file
 * \brief DPSE Participant Listener
 */
/*ci \addtogroup DPSEModule
 * @{
 */
#ifndef DPSEParticipantListener_h
#define DPSEParticipantListener_h

extern void
DPSE_ParticipantBuiltinDataReaderListener_initialize(
                                       struct NDDS_Discovery_Plugin *plugin,
                                       struct DDS_DataReaderListener *listener);

#endif

/*ci @} */
