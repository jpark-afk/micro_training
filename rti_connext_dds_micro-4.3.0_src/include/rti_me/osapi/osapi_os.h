/*
 * FILE: osapi_config.h - OS configuration common for all OSs
 *
 * (c) Copyright, Real-Time Innovations, 2012-2024
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
 * 28jul2016,tk  Refactored from osapi_config.h
 */
/*ce \file
 *   \brief OS API Configuration
 */
#ifndef OSAPI_OS_H
#define OSAPI_OS_H

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
