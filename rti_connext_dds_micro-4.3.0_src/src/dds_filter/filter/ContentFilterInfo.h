/*
 * FILE: ContentFilterInfo.h - Content filter info definitions
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
 * \brief Content filter info definitions
 */
#ifndef ContentFilterInfo_h
#define ContentFilterInfo_h

#include "dds_c/dds_c_content_filter.h"

#include "FilterSignature.h"

#ifdef __cplusplus
extern "C"
{
#endif

struct DDS_ContentFilterInfo
{
    struct DDS_UnsignedLongSeq filter_result;
    struct DDS_FilterSignatureSeq filter_signatures;
};

#define DDS_ContentFilterInfo_INITIALIZER \
{\
    DDS_SEQUENCE_INITIALIZER,\
    DDS_SEQUENCE_INITIALIZER\
}

#define DDS_ContentFilterInfo_t struct DDS_ContentFilterInfo

#ifdef __cplusplus
}
#endif

#endif /* ContentFilterInfo_h */
