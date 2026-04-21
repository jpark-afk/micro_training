/* 

 (c) Copyright, Real-Time Innovations, 2006-2015.  All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.
*/
/*
modification history
--------------------
22as2015,as  Created
=========================================================================*/

#ifndef dds_cpp_netio_hxx
#define dds_cpp_netio_hxx

#ifndef dds_cpp_dll_hxx
#include "dds_cpp/dds_cpp_dll.hxx"
#endif
#ifndef dds_cpp_infrastructure_hxx
#include "dds_cpp/dds_cpp_infrastructure.hxx"
#endif

/*e
  @addtogroup UDPPluginModule
 */
#ifndef netio_udp_h
#include "netio/netio_udp.h"
#endif


/*e \dref_UDP_InterfaceFactory
 */
class DDSCPPDllExport UDPInterfaceFactory
{
public:
    /*e \dref_UDP_InterfaceFactory_get_interface()
     */
    static struct RT_ComponentFactoryI* get_interface();
};

/*e \dref_UDP_InterfaceTable
 */
class DDSCPPDllExport UDPInterfaceTable
{
public:
    /*e \dref_UDP_InterfaceTable_add_entry
     */
    static bool add_entry(struct UDP_InterfaceTableEntrySeq *seq,
                             RTI_UINT32 address,
                             RTI_UINT32 netmask,
                             const char *ifname,
                             RTI_UINT32 flags);
};

#endif /* dds_cpp_netio_hxx */
