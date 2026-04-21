/*
 * FILE: UDP.cxx - UDP C++ API
 *
 * Copyright (c) 2024-2024 Real-Time Innovations, Inc.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef osapi_config_h
#include "osapi/osapi_config.h"
#endif

#ifndef dds_cpp_netio_hxx
#include "rti_me_psl/netio/netio_udp_cpp.hxx"
#endif

/*** SOURCE_BEGIN ***/

struct RT_ComponentFactoryI*
UDPInterfaceFactory::get_interface()
{
    return UDP_InterfaceFactory_get_interface();
}

bool
UDPInterfaceTable::add_entry(struct UDP_InterfaceTableEntrySeq *seq,
                             RTI_UINT32 address,
                             RTI_UINT32 netmask,
                             const char *ifname,
                             RTI_UINT32 flags)
{
    return (UDP_InterfaceTable_add_entry(
                seq, address, netmask, ifname, flags) == RTI_TRUE);
}
