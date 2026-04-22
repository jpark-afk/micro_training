/*
 * FILE: xcdr_infrastructure_psm_impl.h
 *
 * (c) Copyright 2018-2024 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef xcdr_infrastructure_psm_impl_h
#define xcdr_infrastructure_psm_impl_h

#include "osapi/osapi_string.h"
#include "xcdr/xcdr_dll.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define RTIXCdrHeap_allocateStruct(structStoragePointer, Type) \
    OSAPI_Heap_allocate_struct(structStoragePointer, Type)

#ifndef RTI_CERT
#define RTIXCdrHeap_freeStruct(structStorage) \
    OSAPI_Heap_free_struct(structStorage)
#else
#define RTIXCdrHeap_freeStruct(structStorage) \
    UNUSED_ARG(structStorage)
#endif

#define RTIXCdrString_getLengthWithMax(str__, maxLength__) \
    (RTIXCdrUnsignedLong)OSAPI_String_length_w_max((str__),(maxLength__))

#define RTIXCdrUtility_ntohs(s_)  NETIO_ntohs((s_))

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif /* xcdr_infrastructure_psm_impl_h */
