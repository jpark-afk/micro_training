/*
 * FILE: ParticipantListener.h - Participant listener API
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
 * 25jul2011,tk Written.
 */
/*ce
 * \file
 * \brief Participant listener API
 */
#ifndef ParticipantListener_h
#define ParticipantListener_h

extern void
DPDE_ParticipantBuiltinDataReaderListener_initialize(
                        struct NDDS_Discovery_Plugin *plugin,
                        struct DDS_DataReaderListener *listener);

#endif

