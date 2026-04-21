/* 

 (c) Copyright, Real-Time Innovations, 2013-2015.
 All rights reserved.

 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.

modification history
-------------------- 
22dec2015,as  Created
===================================================================== */


#ifndef dds_cpp_netio_hxx
#include "dds_cpp/dds_cpp_netio.hxx"
#endif

#ifndef netio_udp_h
#include "netio/netio_udp.h"
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
