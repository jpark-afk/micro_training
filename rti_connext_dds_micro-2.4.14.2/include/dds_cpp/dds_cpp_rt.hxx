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

#ifndef dds_cpp_rt_hxx
#define dds_cpp_rt_hxx

#ifndef dds_cpp_dll_hxx
#include "dds_cpp/dds_cpp_dll.hxx"
#endif
#ifndef dds_cpp_infrastructure_hxx
#include "dds_cpp/dds_cpp_infrastructure.hxx"
#endif

#ifndef rt_rt_h
#include "rt/rt_rt.h"
#endif

class DDSDomainParticipantFactory_impl;

/*e \dref_RT_Registry
 */
class DDSCPPDllExport RTRegistry
{
    friend class DDSDomainParticipantFactory_impl;

public:

    /*e
     * \dref_RT_Registry_register_component
     */
    bool register_component(const char *name,
                        RT_ComponentFactoryI *intf,
                        RT_ComponentFactoryProperty *property,
                        RT_ComponentFactoryListener *listener);

    /*e
     * \dref_RT_Registry_unregister
     */
    bool unregister(const char *name,
                        RT_ComponentFactoryProperty **property,
                        RT_ComponentFactoryListener **listener);

    RT_Registry_T* get_c_registry();

protected:
    RTRegistry();
    ~RTRegistry();

    /*i
     * \dref_RT_Registry_get_instance
     */
    static RTRegistry* get_instance();

    /*i
     * \dref_RT_Registry_delete_instance
     */
    static void delete_instance();

    static RTRegistry *registry;
    RT_Registry_T *c_registry;
};

#endif /* dds_cpp_rt_hxx */
