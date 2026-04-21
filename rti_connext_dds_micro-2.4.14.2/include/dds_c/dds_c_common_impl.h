/*
 * FILE: dds_c_common_impl.h - DDS_C common definitions
 *
 * (c) Copyright, Real-Time Innovations, 2008-2015.
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
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief DDS_C common definitions
 */

/*i @addtogroup DDSCommonModule Common Module
 @brief Common facilities used by the \dds implementation. Defines
 DDS_DEBUG and so on to facilitate code organization
 and the development process.
 */
/* ----------------------------------------------------------------- */
/*i @file
 @ingroup DDSCommonModule

 @brief Defines the common facilities used by the \dds implementation.
 */

#ifndef dds_c_common_impl_h
#define dds_c_common_impl_h

#include "dds_c/dds_c_config.h"

#ifndef dds_c_dll_h
#include "dds_c/dds_c_dll.h"
#endif
#ifndef osapi_thread_h
#include "osapi/osapi_thread.h"
#endif
#ifndef osapi_process_h
#include "osapi/osapi_process.h"
#endif
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif
#ifndef osapi_mutex_h
#include "osapi/osapi_mutex.h"
#endif
#ifndef osapi_osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef osapi_timer_h
#include "osapi/osapi_timer.h"
#endif
#ifndef osapi_system_h
#include "osapi/osapi_system.h"
#endif
#ifndef osapi_string_h
#include "osapi/osapi_string.h"
#endif
#ifndef reda_string_h
#include "reda/reda_string.h"
#endif
#ifndef reda_sequence_h
#include "reda/reda_sequence.h"
#endif
#ifndef rt_rt_h
#include "rt/rt_rt.h"
#endif
#ifndef db_db_api_h
#include "db/db_api.h"
#endif
#ifndef netio_address_h
#include "netio/netio_address.h"
#endif
#ifndef rtps_rtps_h
#include "rtps/rtps_rtps.h"
#endif
#ifndef netio_route_h
#include "netio/netio_route.h"
#endif
#ifndef netio_interface_h
#include "netio/netio_interface.h"
#endif
#ifndef cdr_encapsulation_h
#include "cdr/cdr_encapsulation.h"
#endif
#ifndef cdr_serialize_h
#include "cdr/cdr_serialize.h"
#endif

#include "dds_c/dds_c_log.h"
#include "dds_c/dds_c_common.h"
#ifndef dds_c_infrastructure_h
#include "dds_c/dds_c_infrastructure.h"
#endif
#ifndef dds_c_discovery_h
#include "dds_c/dds_c_discovery.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif


#if defined(interface)
#undef interface
#endif

#ifdef __cplusplus
}                               /* extern "C" */
#endif


#endif /* dds_c_common_impl_h */
