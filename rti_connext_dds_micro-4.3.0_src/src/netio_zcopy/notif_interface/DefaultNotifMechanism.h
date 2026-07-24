/*
 * FILE: DefaultNotifMechanism.h - Default Notif Mechanism private definitions
 *
 * Copyright 2024-2024 Real-Time Innovations, Inc.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*ci
 * \file
 * \brief Default Notif Mechanism private definitions
 *
 * \addtogroup ZCOPY_NotifMechanismClass
 * @{
 */
#ifndef DefaultNotifMechanism_h
#define DefaultNotifMechanism_h

#include "netio_zcopy/netio_zcopy_default_notif_mech.h"
#include "netio/netio_common.h"
#include "reda/reda_bufferpool.h"
#include "netio_zcopy/netio_zcopy_shm_segment.h"
#include "netio_zcopy/netio_zcopy_shm_monitor.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*ci The max rate at which the receive thread can run in ns */
#define ZCOPY_RECEIVE_THREAD_RATE 100

struct ZCOPY_NotifMechanism
{
    struct NETIO_Interface _parent;

    struct NETIO_Interface *upstream;

    struct ZCOPY_NotifMechanismProperty property;

    struct REDA_BufferPool *port_pool;

    struct REDA_BufferPool *route_pool;
};

struct ZCOPY_NotifMechanism_PI
{
    RTI_UINT32 monitor_offset;

    RTI_BOOL notified;

    /* In shared memory, the monitor is stored at the end of this struct. */
    /* struct OSAPI_SharedMemoryMonitor monitor; */
};

struct ZCOPY_NotifMechanismHandle
{
    struct OSAPI_SharedMemorySegmentHandle *shm_handle;

    struct ZCOPY_NotifMechanism_PI *notif_pi;

    struct OSAPI_SharedMemoryMonitor *monitor;
};

typedef struct ZCOPY_NotifMechanismPortEntry
{
    struct ZCOPY_NotifMechanismHandle handle;

    struct NETIO_Address port;

    RTI_UINT32 ref_count;

    struct OSAPI_Thread *rx_thread;

    struct NETIO_Interface *upstream;
} ZCOPY_NotifMechanismPortEntry;

typedef struct ZCOPY_NotifMechanismHandle ZCOPY_NotifMechanismRouteEntry;

/*ci
 * \brief Convert an unsigned integer to a string in decimal format
 * *
 * \param[inout] buffer      Buffer to store result in
 * \param[in]    max_length  Maximum length of the buffer
 * \param[in]    d           The digit to convert
 *
 * \return The number of characters placed in the buffer excluding the
 *         NULL termination. If there is insufficient space max_length
 *         max_length is returned.
 */
extern RTI_SIZE_T
ZCOPY_NotifMechanism_addr_uint_to_string(char *buffer, RTI_SIZE_T max_length, RTI_UINT32 u);

/*ci
 * \brief Convert an address to a string in the format "NM.<addr>_<port>"
 *
 * \param[in] addr     Address value
 * \param[in] port     Port value
 * \param[out] buf     Buffer to store the string
 * \param[in] buf_len  Length of the buffer
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
extern RTI_BOOL
ZCOPY_NotifMechanism_addr_to_string(
        RTI_UINT32 addr,
        RTI_UINT32 port,
        char *buf,
        RTI_UINT32 buf_len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DefaultNotifMechanism_h */

/*ci @} */
