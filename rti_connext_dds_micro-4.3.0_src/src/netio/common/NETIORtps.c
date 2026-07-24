/*
 * FILE: NETIORtps.c - NETIO RTPS implementation
 *
 * Copyright (c) 2012-2024 Real-Time Innovations, Inc.
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
 * 31jul2015,tk MICRO-1479/PR#15696 Check maximum participant_index when a
 *                                  port is calculated
 * 09jul2015,tk MICRO-1401/PR#15273 Fixed comment error for calculate_port()
 * 16sep2012,tk Written
 */
/*ci
 * \file
 * \brief NETIO RTPS implementation
 *
 * \details
 * This file implements functions to help route resolvers resolve RTPS specific
 * port numbers as defined by the DDS and RTPS specification.
 */
/*ci \addtogroup NETIOUtility
 * @{
 */
#include "osapi/osapi_config.h"

#ifndef osapi_string_h
#include "osapi/osapi_string.h"
#endif
#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef reda_string_h
#include "reda/reda_string.h"
#endif
#ifndef db_api_h
#include "db/db_api.h"
#endif
#ifndef rt_rt_h
#include "rt/rt_rt.h"
#endif
#ifndef netio_log_h
#include "netio/netio_log.h"
#endif
#ifndef netio_address_h
#include "netio/netio_address.h"
#endif
#ifndef netio_route_h
#include "netio/netio_route.h"
#endif

#ifndef netio_rtps_h
#include "netio/netio_rtps.h"
#endif

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Calculate well-known RTPS multicast ports
 *
 * \details
 * This function calculates the well-known multicast port based on the
 * input arguments. Well-known means the ports are calculated as defined
 * in the RTPS and DDS specifications.
 *
 * \param[in] domain_id      The domain id the port is calculated for
 * \param[in] port_base      The base port to use in the calculation
 * \param[in] domain_id_gain The domain gain parameter, how many domain id's
 *                           can exist in the port range
 * \param[in] port_offset    The port offset, the number of ports reserved for
 *                           each index
 *
 * \return port number
 */
RTI_PRIVATE RTI_UINT32
NETIO_RTPS_get_wellknown_multicastport(RTI_INT32 domain_id,
                                     RTI_UINT32 port_base,
                                     RTI_INT32 domain_id_gain,
                                     RTI_INT32 port_offset)
{
        return (port_base + ((RTI_UINT32)domain_id_gain *(RTI_UINT32)domain_id) +
                (RTI_UINT32)port_offset);
}

/*ci
 * \brief Calculate well-known RTPS unicast ports
 *
 * \details
 * This function calculates the well-known unicast port based on the
 * input arguments. Well-known means the ports are calculated as defined
 * in the RTPS and DDS specifications.
 *
 * \param[in] domain_id           The domain id the port is calculated for
 * \param[in] participant_id      The participant id to use in the calculation
 * \param[in] port_base           The base port to use in the calculation
 * \param[in] domain_id_gain      The domain gain parameter How many domain id
 *                                can exist in the port range
 * \param[in] participant_id_gain The participant gain parameter, how many
 *                                participants can exist per domain
 * \param[in] port_offset         The port offset, the number of ports reserved
 *                                for each index
 *
 * \return port number
 */
RTI_PRIVATE RTI_UINT32
NETIO_RTPS_get_wellknown_unicastport(RTI_INT32 domain_id,
                                   RTI_INT32  participant_id,
                                   RTI_UINT32 port_base,
                                   RTI_INT32  domain_id_gain,
                                   RTI_INT32  participant_id_gain,
                                   RTI_INT32  port_offset)
{
    return (port_base + ((RTI_UINT32)domain_id_gain*(RTI_UINT32)domain_id) +
           ((RTI_UINT32)participant_id_gain*(RTI_UINT32)participant_id) +
           (RTI_UINT32)port_offset);
}

RTI_BOOL
NETIO_rtps_calculate_port(void *param,NETIO_RouteKind_T kind,
                          RTI_UINT32 port_base,RTI_INT32 index,
                          struct NETIO_Address *address)
{
    struct NETIO_RtpsPortData *port_data = (struct NETIO_RtpsPortData *)param;
    RTI_UINT32 actual_port;

    if (index > port_data->max_participant_id)
    {
        return RTI_FALSE;
    }

    if (port_base == 0)
    {
        /* Rtps used unsigned long for ports, but the DDS APIs uses signed */
        actual_port = (RTI_UINT32)port_data->port_param.port_base;
    }
    else
    {
        actual_port = port_base;
    }

    if (NETIO_Address_is_multicast(address))
    {
        if (kind == NETIO_ROUTEKIND_USER)
        {
            address->port = NETIO_RTPS_get_wellknown_multicastport(
                    port_data->domain_id,actual_port,
                    port_data->port_param.domain_id_gain,
                    port_data->port_param.user_multicast_port_offset);
            return RTI_TRUE;
        }

        address->port = NETIO_RTPS_get_wellknown_multicastport(
                port_data->domain_id,actual_port,
                port_data->port_param.domain_id_gain,
                port_data->port_param.builtin_multicast_port_offset);
        return RTI_TRUE;
    }

    if (kind == NETIO_ROUTEKIND_USER)
    {
        address->port = NETIO_RTPS_get_wellknown_unicastport(
                port_data->domain_id,index,actual_port,
                port_data->port_param.domain_id_gain,
                port_data->port_param.participant_id_gain,
                port_data->port_param.user_unicast_port_offset);
        return RTI_TRUE;
    }

    address->port = NETIO_RTPS_get_wellknown_unicastport(
            port_data->domain_id,index,actual_port,
            port_data->port_param.domain_id_gain,
            port_data->port_param.participant_id_gain,
            port_data->port_param.builtin_unicast_port_offset);

    return RTI_TRUE;
}

/*ci @} */

