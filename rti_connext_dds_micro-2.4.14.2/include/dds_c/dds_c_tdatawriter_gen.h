/*
 * FILE: dds_c_tdatawriter_gen.h - DDS typed datawriter template
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
 *               source files do not need to undefine dds_c_tdatawriter_gen_h
 *               before including this header.
 * 16sep2012,eh  Created
 */
/*i @file
 * @ingroup DDSPublicationModule
 */
#ifndef dds_c_tdatawriter_gen_h
#define dds_c_tdatawriter_gen_h

#ifndef dds_c_common_h
#include "dds_c/dds_c_common.h"
#endif
#ifndef dds_c_infrastructure_h
#include "dds_c/dds_c_infrastructure.h"
#endif
#ifndef dds_c_publication_h
#include "dds_c/dds_c_publication.h"
#endif

#include "dds_c/dds_c_config.h"

/*** SOURCE_BEGIN ***/

#if defined(TDataWriter) && defined(TData)

#define concatenate(A, B)   A ## B

#define TDataWriter_narrow_c(TDataWriter) concatenate(TDataWriter, _narrow)
#define TDataWriter_narrow TDataWriter_narrow_c(TDataWriter)

#define TDataWriter_as_datawriter_c(TDataWriter) concatenate(TDataWriter, _as_datawriter)
#define TDataWriter_as_datawriter TDataWriter_as_datawriter_c(TDataWriter)

#define TDataWriter_register_instance_c(TDataWriter) concatenate(TDataWriter, _register_instance)
#define TDataWriter_register_instance TDataWriter_register_instance_c(TDataWriter)

#define TDataWriter_register_instance_w_timestamp_c(TDataWriter) concatenate(TDataWriter, _register_instance_w_timestamp)
#define TDataWriter_register_instance_w_timestamp TDataWriter_register_instance_w_timestamp_c(TDataWriter)

#define TDataWriter_unregister_instance_c(TDataWriter) concatenate(TDataWriter, _unregister_instance)
#define TDataWriter_unregister_instance TDataWriter_unregister_instance_c(TDataWriter)

#define TDataWriter_unregister_instance_w_timestamp_c(TDataWriter) concatenate(TDataWriter, _unregister_instance_w_timestamp)
#define TDataWriter_unregister_instance_w_timestamp TDataWriter_unregister_instance_w_timestamp_c(TDataWriter)

#define TDataWriter_write_c(TDataWriter) concatenate(TDataWriter, _write)
#define TDataWriter_write TDataWriter_write_c(TDataWriter)

#define TDataWriter_write_w_timestamp_c(TDataWriter) concatenate(TDataWriter, _write_w_timestamp)
#define TDataWriter_write_w_timestamp TDataWriter_write_w_timestamp_c(TDataWriter)

#define TDataWriter_write_w_params_c(TDataWriter) concatenate(TDataWriter, _write_w_params)
#define TDataWriter_write_w_params TDataWriter_write_w_params_c(TDataWriter)

#define TDataWriter_dispose_c(TDataWriter) concatenate(TDataWriter, _dispose)
#define TDataWriter_dispose TDataWriter_dispose_c(TDataWriter)

#define TDataWriter_dispose_w_timestamp_c(TDataWriter) concatenate(TDataWriter, _dispose_w_timestamp)
#define TDataWriter_dispose_w_timestamp TDataWriter_dispose_w_timestamp_c(TDataWriter)



/* ---------------------------------------------------------------------
   Public Methods
 --------------------------------------------------------------------- */ 

TDataWriter* 
TDataWriter_narrow(DDS_DataWriter* writer) 
{
    return (TDataWriter *)writer;
}

DDS_DataWriter* 
TDataWriter_as_datawriter(TDataWriter* writer) 
{
    return (DDS_DataWriter *)writer;
}

#ifdef INCLUDE_API_INSTANCE
DDS_InstanceHandle_t 
TDataWriter_register_instance(TDataWriter* self,
                              const TData* instance_data) 
{
    return DDS_DataWriter_register_instance((DDS_DataWriter *)self,
                                            instance_data);
}
#endif

#ifdef INCLUDE_API_INSTANCE
DDS_InstanceHandle_t 
TDataWriter_register_instance_w_timestamp(TDataWriter* self,
                                   const TData* instance_data,
                                   const struct DDS_Time_t* source_timestamp) 
{
    return DDS_DataWriter_register_instance_w_timestamp(
       (DDS_DataWriter *)self, instance_data, source_timestamp);
}
#endif

#ifdef INCLUDE_API_INSTANCE
DDS_ReturnCode_t 
TDataWriter_unregister_instance(TDataWriter*  self,
                                const TData* instance_data, 
                                const DDS_InstanceHandle_t* handle) 
{
    return DDS_DataWriter_unregister_instance(
       (DDS_DataWriter *)self, instance_data, handle);
}
#endif

#ifdef INCLUDE_API_INSTANCE
DDS_ReturnCode_t 
TDataWriter_unregister_instance_w_timestamp(TDataWriter* self,
                                     const TData* instance_data, 
                                     const DDS_InstanceHandle_t* handle,
                                     const struct DDS_Time_t* source_timestamp) 
{
    return DDS_DataWriter_unregister_instance_w_timestamp(
       (DDS_DataWriter *)self, instance_data, handle, source_timestamp);
}
#endif

#ifdef INCLUDE_API_INSTANCE
DDS_ReturnCode_t 
TDataWriter_dispose(TDataWriter* self,
                    const TData* instance_data,
                    const DDS_InstanceHandle_t* instance_handle) 
{
    return DDS_DataWriter_dispose(
       (DDS_DataWriter *)self, instance_data, instance_handle);
}
#endif

#ifdef INCLUDE_API_INSTANCE
DDS_ReturnCode_t 
TDataWriter_dispose_w_timestamp(TDataWriter* self,
                                const TData* instance_data, 
                                const DDS_InstanceHandle_t* instance_handle,
                                const struct DDS_Time_t* source_timestamp) 
{
    return DDS_DataWriter_dispose_w_timestamp(
        (DDS_DataWriter *)self,
        instance_data, instance_handle, source_timestamp);
}
#endif

DDS_ReturnCode_t 
TDataWriter_write(TDataWriter* self,
                  const TData* instance_data,
                  const DDS_InstanceHandle_t* handle) 
{
    return DDS_DataWriter_write((DDS_DataWriter *)self, instance_data, handle);
}

DDS_ReturnCode_t 
TDataWriter_write_w_timestamp(TDataWriter* self,
                              const TData* instance_data,
                              const DDS_InstanceHandle_t* handle,
                              const struct DDS_Time_t* source_timestamp) 
{
    return DDS_DataWriter_write_w_timestamp(
       (DDS_DataWriter *)self, instance_data, handle, source_timestamp);
}

DDS_ReturnCode_t 
TDataWriter_write_w_params(TDataWriter* self,
                           const TData* instance_data,
                           struct DDS_WriteParams_t* params) 
{
    return DDS_DataWriter_write_w_params(
       (DDS_DataWriter *)self, instance_data, params);
}

#undef concatenate

#undef TDataWriter_narrow_c
#undef TDataWriter_narrow

#undef TDataWriter_as_datawriter_c
#undef TDataWriter_as_datawriter

#undef TDataWriter_register_instance_c
#undef TDataWriter_register_instance

#undef TDataWriter_register_instance_w_timestamp_c
#undef TDataWriter_register_instance_w_timestamp

#undef TDataWriter_unregister_instance_c
#undef TDataWriter_unregister_instance

#undef TDataWriter_unregister_instance_w_timestamp_c
#undef TDataWriter_unregister_instance_w_timestamp

#undef TDataWriter_write_c
#undef TDataWriter_write

#undef TDataWriter_write_w_timestamp_c
#undef TDataWriter_write_w_timestamp

#undef TDataWriter_write_w_params_c
#undef TDataWriter_write_w_params

#undef TDataWriter_dispose_c
#undef TDataWriter_dispose

#undef TDataWriter_dispose_w_timestamp_c
#undef TDataWriter_dispose_w_timestamp

#else
#error "Incorrect use of dds_cpp_tdatawriter_gen.hxx: TDataWriter, and TData must be defined"
#endif /* defined(TDataWriter) && defined(TData) */

#undef dds_c_tdatawriter_gen_h
#endif /* dds_c_tdatawriter_gen_h */
