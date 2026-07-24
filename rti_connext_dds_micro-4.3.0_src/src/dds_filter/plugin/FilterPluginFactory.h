/*
 * FILE: FilterPluginFactory.h - Filter plugin factory definitions
 *
 * Copyright (c) 2025-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#ifndef FilterPluginFactory_h
#define FilterPluginFactory_h

#include "dds_filter/dds_filter.h"
#include "rt/rt_rt.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define RT_COMPONENT_INSTANCE_DDS_FILTER (1)

#define DDS_FILTER_PLUGIN_INTERFACE_ID \
    RT_MKINTERFACEID(RT_COMPONENT_CLASS_FILTER_PLUGIN, \
                     RT_COMPONENT_INSTANCE_DDS_FILTER)

struct DDS_FilterPluginFactory
{
    struct RT_ComponentFactory _parent;

    RTI_UINT32 instance_counter;
};

#ifdef __cplusplus
}
#endif

#endif /* FilterPluginFactory_h */
