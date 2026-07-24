/*
 QosPolicyCountSeq.c
 
 (c) Copyright, Real-Time Innovations, Sep 19, 2014-2015.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.

 modification history
---------------------
19may2015,as MICRO-1193 Refactoring of Sequence API levels
20sep2014,as Created
===================================================================== */

/*ci
 * \file
 * \brief Implementation of DDS_QosPolicyCountSeq
 */
/*ci \addtogroup DDSInfrastructureModule
 * @{
 */
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif

/*** SOURCE_BEGIN ***/

#define T struct DDS_QosPolicyCount
#define TSeq DDS_QosPolicyCountSeq
#include "reda/reda_sequence_defn.h"

/*ci
 * @}
 */
