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

#ifndef dds_cpp_rh_sm_hx
#define dds_cpp_rh_sm_hx

#ifndef dds_cpp_dll_hxx
#include "dds_cpp/dds_cpp_dll.hxx"
#endif
#ifndef dds_cpp_infrastructure_hxx
#include "dds_cpp/dds_cpp_infrastructure.hxx"
#endif
#ifndef dds_cpp_subscription_hxx
#include "dds_cpp/dds_cpp_subscription.hxx"
#endif
#ifndef rt_rt_h
#include "rt/rt_rt.h"
#endif

#ifndef rh_sm_history_h
#include "rh_sm/rh_sm_history.h"
#endif

/*e \dref_RHSM_ComponentFactory
 */
class DDSCPPDllExport RHSMHistoryFactory
{
public:

    /*e \dref_RHSM_ComponentFactory_get_interface
     */
    static struct RT_ComponentFactoryI* get_interface();
};


#endif /* dds_cpp_rh_sm_hx */
