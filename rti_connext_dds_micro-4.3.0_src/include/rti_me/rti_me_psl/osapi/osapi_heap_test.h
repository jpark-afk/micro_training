/*
 * FILE: ospsl_heap_test.h - Heap Test interface definition
 *
 * Copyright (c) 2024-2024 Real-Time Innovations, Inc.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 */
/*ce
 * \file 
 * \brief Heap Test interface definition
 */
#ifndef osapi_heap_test_h
#define osapi_heap_test_h

#ifndef osapi_dll_h
#include "osapi/osapi_dll.h"
#endif
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

OSAPIDllExport void
OSAPI_Heap_add_allocated_byte_count(RTI_SIZE_T count);

OSAPIDllExport void
OSAPI_Heap_disable_alloc(void);

OSAPIDllExport void
OSAPI_Heap_enable_alloc(void);

OSAPIDllExport void
OSAPI_Heap_enable_allocated_byte_count(void);

OSAPIDllExport void
OSAPI_Heap_disable_allocated_byte_count(void);

OSAPIDllExport RTI_SIZE_T
OSAPI_Heap_get_allocated_byte_count_v2(void);

OSAPIDllExport RTI_SIZE_T
OSAPI_Heap_get_allocated_byte_count(void);

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* ospsl_heap_test_h */
