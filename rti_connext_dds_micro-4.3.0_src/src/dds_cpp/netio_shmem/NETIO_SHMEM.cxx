/* 

 (c) Copyright, Real-Time Innovations, 2017-2018.
 All rights reserved.

 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.
===================================================================== */


#ifndef dds_cpp_netio_shmem_hxx
#include "dds_cpp/dds_cpp_netio_shmem.hxx"
#endif

#ifndef netio_shmem_h
#include "netio_shmem/netio_shmem.h"
#endif
/*** SOURCE_BEGIN ***/

struct RT_ComponentFactoryI*
SHMEMInterfaceFactory::get_interface()
{
    return NETIO_SHMEMInterfaceFactory_get_interface();
}
