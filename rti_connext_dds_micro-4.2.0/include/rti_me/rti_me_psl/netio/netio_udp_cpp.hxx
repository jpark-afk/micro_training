/*
 * FILE: netio_udp_cpp.hxx - UDP C++ API
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
#ifndef netio_udp_cpp_hxx
#define netio_udp_cpp_hxx

#ifndef dds_cpp_dll_hxx
#include "dds_cpp/dds_cpp_dll.hxx"
#endif
#ifndef rti_me_psl_dll_h
#include "rti_me_psl/rti_me_psl_dll.h"
#endif

/*e
  @addtogroup UDPPluginModule
 */
#ifndef netio_udp_h
#include "netio/netio_udp.h"
#endif

/*e \dref_UDP_InterfaceFactory
 */
class NETIOPSL_CPPDllExport UDPInterfaceFactory
{
public:
    /*e \dref_UDP_InterfaceFactory_get_interface()
     */
    static struct RT_ComponentFactoryI* get_interface();
};

/*e \dref_UDP_InterfaceTable
 */
class NETIOPSL_CPPDllExport UDPInterfaceTable
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

namespace UDP 
{
    typedef UDPInterfaceFactory                 InterfaceFactory;
    typedef UDPInterfaceTable                   InterfaceTable;

    typedef struct UDP_InterfaceTableEntry      InterfaceTableEntry;
    typedef struct UDP_InterfaceFactoryProperty InterfaceFactoryProperty;
    typedef struct UDP_InterfaceTableEntrySeq   InterfaceTableEntrySeq;

#ifndef RTI_CERT
    typedef struct UDP_NatEntry                 NatEntry;
    typedef struct UDP_NatEntrySeq              NatEntrySeq;
#endif
}

#endif /* netio_cpp_hxx */
