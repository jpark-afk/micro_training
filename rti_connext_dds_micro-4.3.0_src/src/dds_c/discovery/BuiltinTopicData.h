/*
 * FILE: BuiltinTopicData.h - BuiltinTopicData API
 *
 * (c) Copyright 2024-2024 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*ce
 * \file
 * \brief BuiltinTopicData API
 */
#ifndef BuiltinTopicData_h
#define BuiltinTopicData_h

#ifndef dds_c_discovery_h
#include "dds_c/dds_c_discovery.h"
#endif

MUST_CHECK_RETURN extern DDS_Boolean
DDS_ParticipantBuiltinTopicData_set_from(
                            struct DDS_ParticipantBuiltinTopicData *out,
                            const struct DDS_ParticipantBuiltinTopicData *in,
                            DDS_Boolean shallow_copy,
                            DDS_DomainParticipant *participant);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_PublicationBuiltinTopicData_set_from(
                                    struct DDS_PublicationBuiltinTopicData *out,
                                    const struct DDS_PublicationBuiltinTopicData *in,
                                    DDS_Boolean shallow_copy,
                                    DDS_DomainParticipant *participant);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_SubscriptionBuiltinTopicData_set_from(
                            struct DDS_SubscriptionBuiltinTopicData *out,
                            const struct DDS_SubscriptionBuiltinTopicData *in,
                            DDS_Boolean shallow_copy,
                            DDS_DomainParticipant *participant);

#endif /* BuiltinTopicData_h */
