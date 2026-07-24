/*
 * FILE: Cookie.cxx - DDS Cookie utility functions
 *
 * Copyright 2018-2018 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef dds_cpp_infrastructure_hxx
#include "dds_cpp/dds_cpp_infrastructure.hxx"
#endif

void* DDS_Cookie_t::to_pointer() const
{
    return *OSAPI_Compiler_reinterpret_cast(void**,
                                DDS_OctetSeq_get_contiguous_buffer(&value));
}
