/*
 * FILE: osapi_config.h - OS configuration common for all OSs
 *
 * (c) Copyright, Real-Time Innovations, 2012-2016
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 15feb2022,tk MICRO-3455
 * - Use OSAPI_CC_STRINGIFY_DEFINE instead of " to include header files
 *   from a preprocessor define
 * 28jul2016,tk  Refactored from osapi_config.h
 */
/*ce \file
 *   \brief OS API Configuration
 */
#ifndef OSAPI_OS_H
#define OSAPI_OS_H

#ifndef OSAPI_OS_DEF_H
/* If the platform has not been specified, attempt to determine it.
 */
#if defined(__APPLE__) && defined(__MACH__)
#define OSAPI_OS_DEF_H osapi_os_posix.h
#elif defined(__linux__) || defined(RTI_DEOS_RTEMS)
#define OSAPI_OS_DEF_H osapi_os_posix.h
#elif defined(__VOS__)
#define OSAPI_OS_DEF_H osapi_os_posix.h
#elif defined(__autosar__)
#define OSAPI_OS_DEF_H osapi_os_autosar.h
#elif defined(_MSC_VER) || defined(WIN32)
#define OSAPI_OS_DEF_H osapi_os_windows.h
#elif defined(__vxworks)
#if defined(RTI_ARINC653)
#define OSAPI_OS_DEF_H osapi_os_vxworks653.h
#else
#if (ENABLE_FACE_COMPLIANCE > FACE_COMPLIANCE_LEVEL_SECURITY) && \
    (ENABLE_FACE_COMPLIANCE < FACE_COMPLIANCE_LEVEL_GENERAL)
#define OSAPI_OS_DEF_H osapi_os_posix.h
#else
#define OSAPI_OS_DEF_H osapi_os_vxworks.h
#endif /* ENABLE_FACE_COMPLIANCE */
#endif /* defined(RTI_VX653) */
#elif defined(__QNXNTO__)
#define OSAPI_OS_DEF_H osapi_os_posix.h
#elif defined(RTI_DEOS) && defined(RTI_ARINC653)
#define OSAPI_OS_DEF_H osapi_os_deos653.h
#else
#error "Unable to detect OS. Please define OSAPI_OS_DEF_H."
#endif
#endif

#include OSAPI_CC_STRINGIFY_DEFINE(OSAPI_OS_DEF_H)

#if !defined(OSAPI_ThreadId) && !defined(OSAPI_ThreadHandle)
#error "OSAPI_ThreadId and/or OSAPI_ThreadHandle not defined in "##OSAPI_CC_STRINGIFY_DEFINE(OSAPI_OS_DEF_H)
#endif

/* Linker section definitions:
 * -Any data in bss_sdram will be initially zeroed. 
 * -Any data in data_sdram will be initialized if an initializer is provided. 
 */
#ifndef LINK_SECTION_DATA_SDRAM
#define LINK_SECTION_DATA_SDRAM
#endif
#ifndef LINK_SECTION_BSS_SDRAM
#define LINK_SECTION_BSS_SDRAM
#endif

/* This is to maintain backwards compatability with Micro <= 2.4.8 */
#ifndef OSAPI_ProcessId
#define OSAPI_ProcessId RTI_UINT32
#endif

/* Default (noop) memory barriers */
/* Read memory barrier */
#ifndef OSAPI_Memory_read_barrier
#define OSAPI_Memory_read_barrier()
#endif

/* Write memory barrier */
#ifndef OSAPI_Memory_write_barrier
#define OSAPI_Memory_write_barrier()
#endif

/* Read/Write memory barrier */
#ifndef OSAPI_Memory_read_write_barrier
#define OSAPI_Memory_read_write_barrier()
#endif

#endif /* osapi_os_h */
