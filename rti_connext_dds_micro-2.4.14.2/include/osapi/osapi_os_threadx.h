/*
 * FILE: osapi_os_threadx.h - OS configuration file for ThreadX
 *
 * (c) Copyright, Real-Time Innovations, 2012-2021
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.    Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 21jul2021,tk MICRO-3045 Fixed filenames and dates in file header comments
 * 13dec2016,francisco    File created
 *
 */
/*ce \file
 *     \brief OS API Configuration
 */
#ifndef osapi_os_threadx_h
#define osapi_os_threadx_h

#ifndef RTI_THREADX
#define RTI_THREADX
#endif

/* This is needed to compile the ThreadX source files */
#define OSAPI_INCLUDE_THREADX 1

#define OSAPI_DONT_HAVE_REALLOC 1

/* Linker section definitions:
 * -Any data in bss_sdram will be initially zeroed. 
 * -Any data in data_sdram will be initialized if an initializer is provided. 
 */
#if defined(RX63N)
#define LINK_SECTION_DATA_SDRAM LINK_SECTION(data_sdram)
#define LINK_SECTION_BSS_SDRAM  LINK_SECTION(bss_sdram)
#elif defined(MPC5125)
#define LINK_SECTION_DATA_SDRAM
#define LINK_SECTION_BSS_SDRAM
#elif defined(SYNERGY_S7G2)
#define LINK_SECTION_DATA_SDRAM
#define LINK_SECTION_BSS_SDRAM
#else 
#error "Platform not defined"
#endif

/* Standard ThreadX headers */
#if !defined(SYNERGY_S7G2)
#include "types.h"
#endif
#include "tx_api.h"
#include "nx_api.h"

/* NetX variables */
#if defined(MPC5125)

extern NX_IP            bsp_ip_system_bus;
extern NX_PACKET_POOL   bsp_pool_system_bus;

#define bsp_ip_bus      bsp_ip_system_bus
#define bsp_pool_bus    bsp_pool_system_bus

#elif defined (RX63N)
extern NX_IP            bsp_ip_local_bus;
extern NX_PACKET_POOL   bsp_pool_local_bus;

#define bsp_ip_bus      bsp_ip_local_bus
#define bsp_pool_bus    bsp_pool_local_bus

#elif defined(SYNERGY_S7G2)

extern NX_IP            g_ip0;
extern NX_PACKET_POOL   g_packet_pool0;

#define bsp_ip_bus      g_ip0
#define bsp_pool_bus    g_packet_pool0

#else

#error "Unknown platform"

#endif

#include <errno.h>

/* Needed definitions */
#define OSAPI_PLATFORM_THREADX_HOSTNAME "ThreadX-host"

#define OSAPI_PLATFORM_THREADX_STACK_SIZE_DEFAULT    (6 * 1024)

#ifndef OSAPI_PLATFORM_THREADX_MAX_THREADS
#ifdef RTI_USE_NONBLOCKING_SOCKET
#define OSAPI_PLATFORM_THREADX_MAX_THREADS 2
#else
#define OSAPI_PLATFORM_THREADX_MAX_THREADS 4
#endif /* RTI_USE_NONBLOCKING_SOCKET */
#endif /* !OSAPI_PLATFORM_THREADX_MAX_THREADS */

#ifndef OSAPI_PLATFORM_THREADX_HEAP_SIZE
#define OSAPI_PLATFORM_THREADX_HEAP_SIZE (1 * 1024 * 1024)
#endif

/* There are 100 timer ticks in a second, so 10ms in a tick */
#define OSAPI_MS_TIMER_TICK    10

/* Socket definitions */
#if defined(SYNERGY_S7G2)
typedef uint32_t socklen_t;
#elif  defined(RX63N) || defined(MPC5125)
typedef uint32 socklen_t;
#else
#error "Platform does not define integer"
#endif

struct in_addr 
{
    unsigned long s_addr;
};

struct sockaddr_in 
{
    short            sin_family;
    unsigned short   sin_port;
    struct in_addr   sin_addr;
    char             sin_zero[8];
};

struct sockaddr 
{
    unsigned short sa_family;   // address family, AF_xxx
    char           sa_data[14]; // 14 bytes of protocol address
};

#define SOCK_DGRAM 0
#define AF_INET    0
#define INADDR_ANY 0

#define IPPROTO_IP  0
#define SOL_SOCKET  1

#define IP_MULTICAST_LOOP  1
#define IP_MULTICAST_IF    2
#define IP_MULTICAST_TTL   3
#define IP_ADD_MEMBERSHIP  4
#define IP_DROP_MEMBERSHIP 5
#define SO_SNDBUF          6
#define SO_RCVBUF          7
#define SO_REUSEPORT       8
#define SO_REUSEADDR       9

struct ip_mreq
{
    struct in_addr imr_multiaddr;  /* IP multicast group address */
    struct in_addr imr_interface;  /* IP address of local interface */
    struct in_addr imr_sourceaddr; /* IP address of multicast source */
};

/* Define the thread handle */
struct OSAPI_ThreadHandle
{
    TX_THREAD thread;
    void      *stack;
};
typedef struct OSAPI_ThreadHandle OSAPI_ThreadHandle;

#define OSAPI_ThreadId     RTI_UINT32

#define OSAPI_ProcessId    RTI_UINT32

#define HAVE_SOCKET_API


#ifndef OSAPI_LOG_WRITE_BUFFER
#if defined(RX63N) || defined(MPC5125)
void bsp_debug_printf(const char* fmt, ...);
#define OSAPI_LOG_WRITE_BUFFER(buf_,len_) do \
                                          { \
                                              UNUSED_ARG(len_); \
                                              bsp_debug_printf("%s", buf_);\
                                          } while (0)
#else
#define OSAPI_LOG_WRITE_BUFFER(buf_,len_) do \
                                          { \
                                              UNUSED_ARG(len_); \
                                              UNUSED_ARG(buf_); \
                                              /*printf("%s", buf_);*/ \
                                          } while (0)
#endif /* defined(RX63N) || defined(MPC5125) */
#endif /* OSAPI_LOG_WRITE_BUFFER */

#endif /* osapi_os_threadx_h */
