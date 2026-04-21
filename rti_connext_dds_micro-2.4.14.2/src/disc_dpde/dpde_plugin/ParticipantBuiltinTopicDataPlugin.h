/*
 * FILE: ParticipantBuiltinTopicDataPlugin.c -
 *                                      ParticipantBuiltinTopicDataPlugin API
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
 * \brief ParticipantBuiltinTopicDataPlugin API
 */
#ifndef ParticipantBuiltinTopicDataPlugin_h
#define ParticipantBuiltinTopicDataPlugin_h

MUST_CHECK_RETURN extern RTI_BOOL
DPDE_ParticipantBuiltinTopicDataTypePlugin_create_sample(
                                                struct NDDS_Type_Plugin* plugin,
                                                void **sample,
                                                void *param);

MUST_CHECK_RETURN extern RTI_BOOL
DPDE_ParticipantBuiltinTopicDataTypePlugin_delete_sample(
                                            struct NDDS_Type_Plugin *plugin,
                                            void *sample,
                                            void *param);

MUST_CHECK_RETURN struct NDDS_Type_Plugin*
DPDE_ParticipantBuiltinTopicDataTypePlugin_get(void);

#endif

