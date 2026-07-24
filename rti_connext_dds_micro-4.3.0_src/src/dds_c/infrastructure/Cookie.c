/*
 * FILE: Cookie.c - DDS Cookie utility functions
 *
 * Copyright 2018-2018 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef dds_cpp_infrastructure_h
#include "dds_c/dds_c_infrastructure.h"
#endif

void*
DDS_Cookie_to_pointer(struct DDS_Cookie_t *cookie)
{
    return *OSAPI_Compiler_reinterpret_cast(void**,
                        DDS_OctetSeq_get_contiguous_buffer(&cookie->value));
}
