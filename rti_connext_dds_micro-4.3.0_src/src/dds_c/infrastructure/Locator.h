/*
 * FILE: Locator.h - Locator functions
 *
 * (c) Copyright 2018-2018 Real-Time Innovations,
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
 * \ingroup DDSDomainModule
 * \brief Locator functions
 */
/*ci \addtogroup DDSInfrastructureModule
 * @{
 */

#ifndef Locator_h
#define Locator_h
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef osapi_timer_h
#include "osapi/osapi_timer.h"
#endif
#ifndef reda_bufferpool_h
#include "reda/reda_bufferpool.h"
#endif
#ifndef netio_address_h
#include "netio/netio_address.h"
#endif
#ifndef netio_route_h
#include "netio/netio_route.h"
#endif
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_discovery_plugin_h
#include "dds_c/dds_c_discovery_plugin.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern RTI_BOOL
DDS_Locator_append_locator_kind(const struct DDS_LocatorSeq *in_seq,
                                struct DDS_LocatorSeq *reslvd_seq,
                                RTI_INT32 kind);

extern RTI_BOOL
DDS_Locator_get_interface(const struct DDS_Locator *locator,
                          RT_ComponentFactoryId_T *name,
                          NETIO_RouteResolver_T *r_table,
                          NETIO_AddressResolver_T *nar);

extern void
DDS_Locator_to_netio_address(const struct DDS_Locator *loc,
                             struct NETIO_Address *addr_out);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*ci @} */
