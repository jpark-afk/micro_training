/*
 * FILE: Transport.h - Transport functions
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
 * \ingroup DDSInfrastructureModule
 * \brief Transport functions
 */
/*ci \addtogroup DDSInfrastructureModule
 * @{
 */

#ifndef Transport_h
#define Transport_h

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

#include "Locator.h"
#include "QosPolicy.h"

#define NETIO_DEFAULT_PRIORITY (3)

extern RTI_INT32
DDS_Transport_get_locator_priority(const struct DDS_Locator *loc);

extern void
DDS_Transport_set_encapsulation_policy(
                            struct DDS_TransportEncapsulationQosPolicy *policy,
                            struct DDS_TypePlugin *plugin,
                            struct DDS_Locator *a_loc,
                            NETIO_AddressResolver_T *ar,
                            NETIO_RouteResolver_T *rr);

extern DDS_Boolean
DDS_Transport_is_encapsulation_policy_valid(
                            struct DDS_TransportEncapsulationQosPolicy *policy,
                            struct DDS_TypePlugin *plugin,
                            NETIO_AddressResolver_T *ar,
                            NETIO_RouteResolver_T *rr);
#endif

/*ci @} */
