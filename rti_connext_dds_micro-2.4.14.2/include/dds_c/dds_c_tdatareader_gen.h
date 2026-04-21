/*
 * FILE: dds_c_tdatareader_gen.h - DDS typed datareader template
 *
 * (c) Copyright, Real-Time Innovations, 2012-2015.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 06jun2014,as  Added compiler error on incorrect use of template header;
 *               source files do not need to undefine dds_c_tdatareader_gen_h
 *               before including this header.
 * 16sep2012,eh  Created
 */
/*ci
 * \file
 * \brief DDS subscription module
 * @ingroup DDSSubscriptionModule
 */
#ifndef dds_c_tdatareader_gen_h
#define dds_c_tdatareader_gen_h

#ifndef dds_c_common_h
#include "dds_c/dds_c_common.h"
#endif
#ifndef dds_c_infrastructure_h
#include "dds_c/dds_c_infrastructure.h"
#endif
#ifndef dds_c_subscription_h
#include "dds_c/dds_c_subscription.h"
#endif


/*** SOURCE_BEGIN ***/

#if defined(TDataReader) && defined(TDataSeq ) && defined(TData)

#define concatenate(A, B)   A ## B

#define TDataReader_narrow_c(TDataReader) concatenate(TDataReader, _narrow)
#define TDataReader_narrow TDataReader_narrow_c(TDataReader)

#define TDataReader_as_datareader_c(TDataReader) concatenate(TDataReader, _as_datareader)
#define TDataReader_as_datareader TDataReader_as_datareader_c(TDataReader)

#define TDataReader_read_c(TDataReader) concatenate(TDataReader, _read)
#define TDataReader_read TDataReader_read_c(TDataReader)

#define TDataReader_take_c(TDataReader) concatenate(TDataReader, _take)
#define TDataReader_take TDataReader_take_c(TDataReader)

#define TDataReader_read_next_sample_c(TDataReader) concatenate(TDataReader, _read_next_sample)
#define TDataReader_read_next_sample TDataReader_read_next_sample_c(TDataReader)

#define TDataReader_take_next_sample_c(TDataReader) concatenate(TDataReader, _take_next_sample)
#define TDataReader_take_next_sample TDataReader_take_next_sample_c(TDataReader)

#define TDataReader_read_instance_c(TDataReader) concatenate(TDataReader, _read_instance)
#define TDataReader_read_instance TDataReader_read_instance_c(TDataReader)

#define TDataReader_take_instance_c(TDataReader) concatenate(TDataReader, _take_instance)
#define TDataReader_take_instance TDataReader_take_instance_c(TDataReader)

#define TDataReader_return_loan_c(TDataReader) concatenate(TDataReader, _return_loan)
#define TDataReader_return_loan TDataReader_return_loan_c(TDataReader)

#define TDataReader_lookup_instance_c(TDataReader) concatenate(TDataReader, _lookup_instance)
#define TDataReader_lookup_instance TDataReader_lookup_instance_c(TDataReader)

/* ---------------------------------------------------------------------
   Public Methods
 --------------------------------------------------------------------- */ 

TDataReader* 
TDataReader_narrow(DDS_DataReader* reader) 
{
    return (TDataReader *)reader;
}

DDS_DataReader* 
TDataReader_as_datareader(TDataReader* reader) 
{
    return (DDS_DataReader *)reader;
}

DDS_ReturnCode_t 
TDataReader_read(TDataReader* self,
                 struct TDataSeq * received_data,
                 struct DDS_SampleInfoSeq* info_seq,
                 DDS_Long max_samples,
                 DDS_SampleStateMask sample_states,
                 DDS_ViewStateMask view_states,
                 DDS_InstanceStateMask instance_states) 
{
    return DDS_DataReader_read((DDS_DataReader *)self, 
                               (struct DDS_UntypedSampleSeq *)received_data, 
                               info_seq, max_samples, 
                               sample_states, view_states, instance_states);
}

DDS_ReturnCode_t 
TDataReader_take(TDataReader* self,
                 struct TDataSeq * received_data,
                 struct DDS_SampleInfoSeq* info_seq,
                 DDS_Long max_samples,
                 DDS_SampleStateMask sample_states,
                 DDS_ViewStateMask view_states,
                 DDS_InstanceStateMask instance_states) 
{
    return DDS_DataReader_take((DDS_DataReader *)self, 
                               (struct DDS_UntypedSampleSeq *)received_data, 
                               info_seq, max_samples, 
                               sample_states, view_states, instance_states);
}

DDS_ReturnCode_t 
TDataReader_read_next_sample(TDataReader* self,
                             TData* received_data,
                             struct DDS_SampleInfo* sample_info) 
{
    return DDS_DataReader_read_next_sample((DDS_DataReader *)self, 
                                           received_data, sample_info);
}

DDS_ReturnCode_t 
TDataReader_take_next_sample(TDataReader* self,
                             TData* received_data,
                             struct DDS_SampleInfo* sample_info) 
{
    return DDS_DataReader_take_next_sample((DDS_DataReader *)self, 
                                           received_data, sample_info);
}

DDS_ReturnCode_t 
TDataReader_read_instance(TDataReader* self,
                          struct TDataSeq * received_data,
                          struct DDS_SampleInfoSeq* info_seq,
                          DDS_Long max_samples,
                          const DDS_InstanceHandle_t* a_handle,
                          DDS_SampleStateMask sample_states,
                          DDS_ViewStateMask view_states,
                          DDS_InstanceStateMask instance_states) 
{
    return DDS_DataReader_read_instance((DDS_DataReader *)self,
                                (struct DDS_UntypedSampleSeq  *)received_data, 
                                info_seq, max_samples, a_handle,
                                sample_states, view_states, instance_states);
}

DDS_ReturnCode_t 
TDataReader_take_instance(TDataReader* self,
                          struct TDataSeq * received_data,
                          struct DDS_SampleInfoSeq* info_seq,
                          DDS_Long max_samples,
                          const DDS_InstanceHandle_t* a_handle,
                          DDS_SampleStateMask sample_states,
                          DDS_ViewStateMask view_states,
                          DDS_InstanceStateMask instance_states) 
{
    return DDS_DataReader_take_instance((DDS_DataReader *)self,
                                (struct DDS_UntypedSampleSeq  *)received_data, 
                                info_seq, max_samples, a_handle,
                                sample_states, view_states, instance_states);
}

DDS_ReturnCode_t 
TDataReader_return_loan(TDataReader* self,
                        struct TDataSeq * received_data,
                        struct DDS_SampleInfoSeq* info_seq) 
{
    return DDS_DataReader_return_loan((DDS_DataReader *)self,
                                (struct DDS_UntypedSampleSeq  *)received_data,
                                info_seq);
}

DDS_InstanceHandle_t 
TDataReader_lookup_instance(TDataReader* self,
                            const TData *key_holder)
{
    return DDS_DataReader_lookup_instance((DDS_DataReader *)self,
                                          (const void *)key_holder);
}

#undef concatenate

#undef TDataReader_narrow_c
#undef TDataReader_narrow

#undef TDataReader_as_datareader_c
#undef TDataReader_as_datareader

#undef TDataReader_read_c
#undef TDataReader_read

#undef TDataReader_take_c
#undef TDataReader_take

#undef TDataReader_read_next_sample_c
#undef TDataReader_read_next_sample

#undef TDataReader_take_next_sample_c
#undef TDataReader_take_next_sample

#undef TDataReader_read_instance_c
#undef TDataReader_read_instance

#undef TDataReader_take_instance_c
#undef TDataReader_take_instance

#undef TDataReader_return_loan_c
#undef TDataReader_return_loan

#undef TDataReader_lookup_instance_c
#undef TDataReader_lookup_instance

#else
#error "Incorrect use of dds_c_tdatareader_gen.h: TDataReader, TData, and TDataSeq must be defined"
#endif /* defined(TDataReader) && defined(TDataSeq ) && defined(TData) */

#undef dds_c_tdatareader_gen_h

#endif /* dds_c_tdatareader_gen_h */
