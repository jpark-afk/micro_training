/*
 * FILE: RTPSChecksum.h - Standard checksum functions
 *
 * Copyright 2020-2021 Real-Time Innovations, Inc.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef RTPSChecksum_h
#define RTPSChecksum_h

extern RTI_BOOL
RTPS_BuiltinCrc32_checksum_calculate(void *context,
                   const struct REDA_Buffer *buf,
                   RTI_UINT32 buf_length,
                   RTPS_Checksum_T *checksum);

extern RTI_BOOL
RTPS_BuiltinCrc32Pro_checksum_calculate(void *context,
                                        const struct REDA_Buffer *buf,
                                        RTI_UINT32 buf_length,
                                        RTPS_Checksum_T *checksum);

extern RTI_BOOL
RTPS_BuiltinCrc64_checksum_calculate(void *context,
                   const struct REDA_Buffer *buf,
                   RTI_UINT32 buf_length,
                   RTPS_Checksum_T *checksum);



extern RTI_BOOL
RTPS_BuiltinMD5_checksum_calculate(void *context,
                           const struct REDA_Buffer *buf,
                           RTI_UINT32 buf_length,
                           RTPS_Checksum_T *checksum);

extern RTI_BOOL
RTPS_ChecksumClass_is_equal(const struct RTPS_ChecksumClass *left,
                            const struct RTPS_ChecksumClass *right);

#endif
