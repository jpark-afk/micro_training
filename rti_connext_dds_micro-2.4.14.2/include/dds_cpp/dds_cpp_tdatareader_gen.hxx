/* 

 (c) Copyright, Real-Time Innovations, 2013-2015.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.

modification history
--------------------
06jun2014,as  Added compiler error on incorrect use of template header;
              source files do not need to undefine dds_c_tdatareader_gen_hxx
              before including this header.
19jul2013,as  Major C++ update
11jan2013,eh  Created
===================================================================== */

#ifndef dds_cpp_tdatareader_gen_hxx
#define dds_cpp_tdatareader_gen_hxx

#ifndef dds_cpp_subscription_hxx
#include "dds_cpp/dds_cpp_subscription.hxx"
#endif
#ifndef dds_c_common_h
#include "dds_c/dds_c_common.h"
#endif
#ifndef dds_c_infrastructure_h
#include "dds_c/dds_c_infrastructure.h"
#endif
#ifndef dds_c_subscription_h
#include "dds_c/dds_c_subscription.h"
#endif


/* ================================================================== */
/*i
   <<INTERFACE>>

   Defines:   TDataReader_impl, TDataReader, TData

   Organized using the well-documented "Generics Pattern" for
   implementing generics in C and C++.
*/

/* ================================================================== */
/*i
   <<IMPLEMENTATION>>

   Defines:   TDataReader_impl TData
*/
#if defined(TDataReader) && defined(TData) && defined(TDataSeq)


/*
 * Two step procedure needed for stringize the value of a macro
 */
#define _QUOTE(x) # x
#define QUOTE(x) _QUOTE(x)

// ---------------------------------------------------------------------
// Public Methods
// ---------------------------------------------------------------------


// ---------------------------------------------------------------------

TDataReader* TDataReader::narrow(DDSDataReader *reader) 
{
    TDataReader *typedReader = static_cast<TDataReader*>(reader);
    return typedReader;
}

DDSDataReader* TDataReader::as_datareader()
{
    return this;
}

DDS_ReturnCode_t TDataReader::read_next_sample(TData& received_data,
                                               DDS_SampleInfo& sample_info)
{
    return this->read_next_sample_untyped(
            (void *)&received_data, sample_info);
}

DDS_ReturnCode_t TDataReader::take_next_sample(TData& received_data,
                                               DDS_SampleInfo& sample_info)
{
    return this->take_next_sample_untyped(
            (void *)&received_data, sample_info);
}



DDS_ReturnCode_t TDataReader::read(
        TDataSeq& received_data,
        DDS_SampleInfoSeq& info_seq,
        DDS_Long max_samples,
        DDS_SampleStateMask sample_states,
        DDS_ViewStateMask view_states,
        DDS_InstanceStateMask instance_states)
{
    return this->read_untyped(
            (REDA_Sequence*) &received_data, info_seq, max_samples,
            sample_states, view_states, instance_states);
}

DDS_ReturnCode_t TDataReader::take(
        TDataSeq& received_data,
        DDS_SampleInfoSeq& info_seq,
        DDS_Long max_samples,
        DDS_SampleStateMask sample_states,
        DDS_ViewStateMask view_states,
        DDS_InstanceStateMask instance_states)
{
    return this->take_untyped(
        (REDA_Sequence*) &received_data, info_seq, max_samples,
        sample_states, view_states, instance_states);
}

DDS_ReturnCode_t TDataReader::read_instance(
        TDataSeq& received_data,
        DDS_SampleInfoSeq& info_seq,
        DDS_Long max_samples,
        const DDS_InstanceHandle_t& a_handle,
        DDS_SampleStateMask sample_states,
        DDS_ViewStateMask view_states,
        DDS_InstanceStateMask instance_states)
{
    return this->read_instance_untyped(
        (REDA_Sequence*) &received_data, info_seq, max_samples,
        a_handle, sample_states, view_states, instance_states);
}

DDS_ReturnCode_t TDataReader::take_instance(
        TDataSeq& received_data,
        DDS_SampleInfoSeq& info_seq,
        DDS_Long max_samples,
        const DDS_InstanceHandle_t& a_handle,
        DDS_SampleStateMask sample_states,
        DDS_ViewStateMask view_states,
        DDS_InstanceStateMask instance_states)
{
    return this->take_instance_untyped(
        (REDA_Sequence*) &received_data, info_seq, max_samples,
        a_handle, sample_states, view_states, instance_states);
}

DDS_InstanceHandle_t
TDataReader::lookup_instance(const TData& key_holder)
{
    return this->lookup_instance_untyped((const void*)&key_holder);
}

DDS_ReturnCode_t
TDataReader::return_loan(
        TDataSeq& received_data,
        DDS_SampleInfoSeq& info_seq)
{
    return this->return_loan_untyped(
            (REDA_Sequence*) &received_data, info_seq);
}


// ---------------------------------------------------------------------
// Protected Methods
// ---------------------------------------------------------------------

// --- Constructors & destructors: -------------------------------------

TDataReader::~TDataReader()
{
    // empty
}


TDataReader::TDataReader(DDS_DataReader *cDataReader) :
    DDSDataReader_impl(cDataReader)
{
    // empty
}


// ---------------------------------------------------------------------
// Private Methods
// ---------------------------------------------------------------------

#else
#error "Incorrect use of dds_cpp_tdatareader_gen.hxx: TDataReader, TData, and TDataSeq must be defined"
#endif /* defined(TDataReader) && defined(TData) && defined(TDataSeq) */

#undef dds_cpp_tdatareader_gen_hxx
#endif /* dds_cpp_tdatareader_gen_hxx */


