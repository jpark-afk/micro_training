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
                                            void **sample,
                                            void *param);

MUST_CHECK_RETURN extern RTI_BOOL
DPDE_PublicationBuiltinTopicDataTypePlugin_delete_sample(
                                            struct NDDS_Type_Plugin *plugin,
                                            void *sample,
                                            void *param);

MUST_CHECK_RETURN extern RTI_BOOL
DPDE_PublicationBuiltinTopicDataTypePlugin_copy_sample(
        struct NDDS_Type_Plugin *type,
        void *dst,
        const void *src,
        void *param);

MUST_CHECK_RETURN extern struct NDDS_Type_Plugin*
DPDE_PublicationBuiltinTopicDataTypePlugin_get(void);

#endif

