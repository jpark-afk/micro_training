/*
 * FILE: SqlFilterExecutor.h - DDS SQL Content Filter Executor definitions
 *
 * Copyright (c) 2025-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#ifndef SqlFilterExecutor_h
#define SqlFilterExecutor_h

#include "SqlContentFilter.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern DDS_Boolean
DDS_SqlCompiledContentFilter_evaluate(
        const struct DDS_SqlCompiledContentFilter *compiled_filter,
        const void *sample,
        DDS_Boolean *result_out);

extern DDS_Boolean
DDS_SqlPredicate_evaluate(
        const struct DDS_SqlPredicate *predicate,
        const void *sample,
        DDS_Boolean *result_out);

extern DDS_Boolean
DDS_SqlFieldValue_get_value(
        const struct DDS_SqlFieldValue *field,
        const void *sample,
        enum DDS_SqlImmediateType value_type,
        union DDS_SqlImmediateValue *value_out);

extern DDS_Boolean
DDS_SqlFilter_apply_comparison(
        const struct DDS_SqlPredicate *predicate,
        const union DDS_SqlImmediateValue *a,
        const union DDS_SqlImmediateValue *b,
        DDS_Boolean *result_out);

#ifdef __cplusplus
}
#endif

#endif /* SqlFilterExecutor_h */
