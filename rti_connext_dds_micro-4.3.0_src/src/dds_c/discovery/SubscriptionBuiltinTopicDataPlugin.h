/*
 * FILE: SubscriptionBuiltinTopicDataPlugin.h -
 *                          SubscriptionBuiltinTopicDataPlugin API
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
 * \brief SubscriptionBuiltinTopicDataPlugin API
 */
#ifndef SubscriptionBuiltinTopicDataPlugin_h
#define SubscriptionBuiltinTopicDataPlugin_h

MUST_CHECK_RETURN extern DDS_ReturnCode_t
DPDE_SubscriptionBuiltinTopicDataTypePlugin_register(DDS_DomainParticipant *participant);

MUST_CHECK_RETURN struct NDDS_Type_Plugin*
DPDE_SubscriptionBuiltinTopicDataTypePlugin_get(void);

struct DDS_TypePlugin*
DPDE_SubscriptionBuiltinTopicDataTypePlugin_create(
                                    DDS_DomainParticipant *participant,
                                    struct DDS_DomainParticipantQos *dp_qos,
                                    DDS_TypePluginMode_T endpoint_mode,
                                    DDS_TypePluginEndpoint *endpoint,
                                    DDS_TypePluginEndpointQos *qos,
                                    struct DDS_TypePluginProperty *property);

#endif

