/*
 * FILE: SubscriptionListener.h - Subscription listener API
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
 * \brief Subscription listener API
 */
#ifndef SubscriptionListener_h
#define SubscriptionListener_h

extern void
DPDE_SubscriptionBuiltinDataReaderListener_initialize(
                                       struct DPDE_DiscoveryPlugin *plugin,
                                       struct DDS_DataReaderListener *listener);

extern void
DPDE_SubscriptionBuiltinDataWriterListener_initialize(
                                    struct DPDE_DiscoveryPlugin *plugin,
                                    struct DDS_DataWriterListener *listener);

#endif

