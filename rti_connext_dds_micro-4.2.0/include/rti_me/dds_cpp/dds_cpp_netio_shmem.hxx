/* 
 (c) Copyright, Real-Time Innovations, 2017-2018.  All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.
*/

#ifndef dds_cpp_netio_shmem_hxx
#define dds_cpp_netio_shmem_hxx

#ifndef dds_cpp_dll_hxx
#include "dds_cpp/dds_cpp_dll.hxx"
#endif
#ifndef dds_cpp_infrastructure_hxx
#include "dds_cpp/dds_cpp_infrastructure.hxx"
#endif

/*e \addtogroup SHMEMPluginModule
 */
#ifndef netio_shmem_h
#include "netio_shmem/netio_shmem.h"
#endif

/*e \dref_SHMEM_InterfaceFactory
 */
class DDSCPPDllExport SHMEMInterfaceFactory
{
public:
    /*e \dref_SHMEM_InterfaceFactory_get_interface
     */
    static struct RT_ComponentFactoryI* get_interface();
};

#endif /* dds_cpp_netio_shmem_hxx */
