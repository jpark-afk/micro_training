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


#ifndef dds_cpp_wh_sm_hxx
#include "dds_cpp/dds_cpp_wh_sm.hxx"
#endif
/*** SOURCE_BEGIN ***/

struct RT_ComponentFactoryI*
WHSMHistoryFactory::get_interface()
{
    return WHSM_HistoryFactory_get_interface();
}


