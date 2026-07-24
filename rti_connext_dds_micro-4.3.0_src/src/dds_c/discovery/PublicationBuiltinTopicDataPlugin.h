/*
 * FILE: PublicationBuiltinTopicDataPlugin.h -
 *                          PublicationBuiltinTopicDataPlugin API
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
 * \brief PublicationBuiltinTopicDataPlugin API
 */
#ifndef PublicationBuiltinTopicDataPlugin_h
#define PublicationBuiltinTopicDataPlugin_h

MUST_CHECK_RETURN extern RTI_BOOL
DPDE_PublicationBuiltinTopicDataTypePlugin_create_sample(
                                            struct NDDS_Type_Plugin *plugin,
                                            void **sample);

MUST_CHECK_RETURN extern RTI_BOOL
DPDE_PublicationBuiltinTopicDataTypePlugin_delete_sample(
                                            struct NDDS_Type_Plugin *plugin,
                                            void *sample);

MUST_CHECK_RETURN extern RTI_BOOL
DPDE_PublicationBuiltinTopicDataTypePlugin_copy_sample(
        struct NDDS_Type_Plugin *type,
        void *dst,
        const void *src);

MUST_CHECK_RETURN extern DDS_ReturnCode_t
DPDE_PublicationBuiltinTopicDataTypePlugin_register(DDS_DomainParticipant *participant);

extern struct DDS_TypePlugin*
DPDE_PublicationBuiltinTopicDataTypePlugin_create(
                                    DDS_DomainParticipant *participant,
                                    struct DDS_DomainParticipantQos *dp_qos,
                                    DDS_TypePluginMode_T endpoint_mode,
                                    DDS_TypePluginEndpoint *endpoint,
                                    DDS_TypePluginEndpointQos *qos,
                                    struct DDS_TypePluginProperty *property);

#endif

