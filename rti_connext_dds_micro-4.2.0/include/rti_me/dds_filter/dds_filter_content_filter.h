/*
 * FILE: dds_filter_content_filter.h - DDS Content Filter definitions
 *
 * (c) Copyright, Real-Time Innovations, 2025-2025.
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
 * \brief DDS Content Filter definitions
 */

#ifndef dds_filter_content_filter_h
#define dds_filter_content_filter_h

#include "dds_filter/dds_filter.h"

#ifdef __cplusplus
extern "C"
{
#endif

FUNCTION_MUST_TYPEDEF(
void *
(*DDS_ContentFilter_create_instance)(
        const struct DDS_FilterResourceLimits *resource_limits,
        const void *property)
)

typedef DDS_Boolean
(*DDS_ContentFilter_delete_instance)(void *filter_data);

FUNCTION_MUST_TYPEDEF(
DDS_Boolean
(*DDS_ContentFilter_compileFunc)(
        void *filter_data, void **new_compile_data,
        const char *expression, const struct DDS_StringSeq *parameters,
        const struct DDS_TypeCode *type_code)
)

FUNCTION_MUST_TYPEDEF(
DDS_Boolean
(*DDS_ContentFilter_evaluateFunc)(
        void *filter_data,
        void *compile_data,
        const void *sample,
        DDS_Boolean *result_out)
)

typedef void
(*DDS_ContentFilter_finalizeFunc)(void *filter_data, void *compile_data);

struct DDS_ContentFilterI
{
    DDS_ContentFilter_create_instance create_instance;

    DDS_ContentFilter_delete_instance delete_instance;

    DDS_ContentFilter_compileFunc compile;

    DDS_ContentFilter_evaluateFunc evaluate;

    DDS_ContentFilter_finalizeFunc finalize;
};

#define DDS_ContentFilterI_INITIALIZER { NULL, NULL, NULL, NULL, NULL }


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* dds_filter_content_filter_h */
