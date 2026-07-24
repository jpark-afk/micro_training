/*
 * FILE: FilterSignature.h - Filter signature definitions
 *
 * Copyright (c) 2025-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*ce
 * \file
 * \brief Filter signature definitions
 */
#ifndef FilterSignature_h
#define FilterSignature_h

#include "dds_c/dds_c_content_filter.h"

#ifdef __cplusplus
extern "C"
{
#endif

struct DDS_FilterSignature
{
    DDS_UnsignedLong value[4];
};

#define DDS_FilterSignature_INITIALIZER \
{\
    {0, 0, 0, 0}\
}

#define DDS_FilterSignature_t struct DDS_FilterSignature

#define T struct DDS_FilterSignature
#define TSeq DDS_FilterSignatureSeq
#define TSeq_loan_contiguous
#define REDA_SEQUENCE_USER_API
#include <reda/reda_sequence_decl.h>

extern RTI_INT32
DDS_FilterSignature_compare(const struct DDS_FilterSignature *left,
                            const struct DDS_FilterSignature *right);

extern RTI_BOOL
DDS_FilterSignature_calculate(struct DDS_FilterSignature *signature,
                              DDS_String content_filtered_topic_name,
                              DDS_String related_topic_name,
                              DDS_String filter_class_name,
                              DDS_String filter_expression,
                              const struct DDS_StringSeq *expression_parameters);

extern RTI_BOOL
DDS_FilterSignature_calculate_from_property(
        struct DDS_FilterSignature *signature,
        struct DDS_ContentFilterProperty *property);

#ifdef __cplusplus
}
#endif

#endif /* FilterSignature_h */
