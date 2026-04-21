/*
 * FILE: ospsl_os_freertos.h - OS configuration file for FreeRTOS OS
 *
 * (c) Copyright, Real-Time Innovations 2024-2025
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 */
/*ce \file
 *   \brief OS PSL support for FreeRTOS
 */
#ifndef ospsl_os_freertos_h
#define ospsl_os_freertos_h

#ifndef RTI_FREERTOS
#define RTI_FREERTOS
#endif

#define OSAPI_PSL 1
#define OSAPI_NO_THREADS 1
#define OSAPI_ENABLE_STDC_FREE 0
#define OSAPI_ENABLE_STDC_REALLOC 0
#define OSAPI_ENABLE_PVPORTREALLOC 0

/* Standard FREERTORS headers */
#include "FreeRTOS.h"
#include <string.h>
#include <errno.h>
#include <task.h>

/* Needed definitions */
#define OSAPI_PLATFORM_FREERTOS_HOSTNAME "FreeRTOS-host"

/* The number of words (not bytes!) to allocate for use as the task's stack. */
#define OSAPI_PLATFORM_FREERTOS_STACK_SIZE_DEFAULT   1024

#ifndef OSAPI_LOG_WRITE_BUFFER
#define OSAPI_LOG_WRITE_BUFFER(buf_,len_) if (write(1,buf_,len_)){}
#endif


/* If there is no support of stdc realloc or pvportrealloc
 * we use a custom implementation. 
 */
#if !OSAPI_ENABLE_STDC_REALLOC && !OSAPI_ENABLE_PVPORTREALLOC
/* If so, we only support heap 4 and dynamic allocation */
#if (configAPPLICATION_ALLOCATED_HEAP == 4) && \
    (configSUPPORT_DYNAMIC_ALLOCATION == 1)
/* For heap_4.c implementation */
#include "portable.h"

/* Heap block structure used by heap_4.c and heap_5.c 
 * This structure is used to maintain a linked list of free blocks.
 * Each block has a pointer to the next free block and its size.
 */
typedef struct A_BLOCK_LINK
{
    struct A_BLOCK_LINK *pxNextFreeBlock;
    size_t xBlockSize;
} BlockLink_t;

/* Constants used by heap implementation */
#define xHeapStructSize    ( ( sizeof( BlockLink_t ) + \
                               ( ( size_t ) ( portBYTE_ALIGNMENT - 1 ) ) ) & \
                             ~( ( size_t ) portBYTE_ALIGNMENT_MASK ) )
#define xBlockAllocatedBit ( ( ( size_t ) 1 ) << \
                             ( ( sizeof( size_t ) * heapBITS_PER_BYTE ) - 1 ) )
#define heapBITS_PER_BYTE  ( ( size_t ) 8 )

#else
#error "FreeRTOS without pvportrealloc support, custom implementation only " \
       "available for heap_4.c"
#endif /* (configAPPLICATION_ALLOCATED_HEAP == 4) && \
          (configSUPPORT_DYNAMIC_ALLOCATION == 1) */

#endif /* !OSAPI_ENABLE_STDC_REALLOC && !OSAPI_ENABLE_PVPORTREALLOC */
#endif /* ospsl_os_freertos_h */