/*
 * FILE: dds_c_data.h - DDS typed interface definitions
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
 * 21spe2012,eh Written
 */

/*i @file
 */
/*ce
 * @addtogroup DDSUserDataModule
 * @brief Defines the \dds user data type support.
 */
#ifndef dds_c_data_h
#define dds_c_data_h

#ifndef dds_c_common_h
#include "dds_c/dds_c_common.h"
#endif
#ifndef dds_c_infrastructure_h
#include "dds_c/dds_c_infrastructure.h"
#endif
#ifndef dds_c_topic_h
#include "dds_c/dds_c_topic.h"
#endif
#ifndef dds_c_publication_h
#include "dds_c/dds_c_publication.h"
#endif
#ifndef dds_c_subscription_h
#include "dds_c/dds_c_subscription.h"
#endif
#ifndef dds_c_domain_h
#include "dds_c/dds_c_domain.h"
#endif

#ifdef __cplusplus
    extern "C" {
#endif

/* ================================================================= */
/*                   DATA TYPE                                       */
/* ================================================================= */


#ifdef DOXYGEN_DOCUMENTATION_ONLY
/*i \dref_Foo
 */
struct Foo {
    RTI_INT32 undocumented_dummy;
};
#endif


#ifndef NDDSUSERDllExport
#define NDDSUSERDllExport
#endif

/*ce \dref_DATAWRITER
 */
#define DDS_DATAWRITER_C(TDataWriter, TData)                              \
    typedef struct TDataWriter TDataWriter;                               \
                                                                          \
    NDDSUSERDllExport DDSCDllExport                                       \
    TDataWriter* TDataWriter ## _narrow(                                  \
        DDS_DataWriter* writer);                                          \
                                                                          \
    NDDSUSERDllExport DDSCDllExport                                       \
    DDS_DataWriter* TDataWriter ## _as_datawriter(                        \
        TDataWriter* writer);                                             \
                                                                          \
    NDDSUSERDllExport DDSCDllExport                                       \
    DDS_InstanceHandle_t TDataWriter ## _register_instance(               \
        TDataWriter* self, const TData* instance_data);                   \
                                                                          \
    NDDSUSERDllExport DDSCDllExport                                       \
    DDS_InstanceHandle_t TDataWriter ## _register_instance_w_timestamp(   \
        TDataWriter* self, const TData* instance_data,                    \
        const struct DDS_Time_t* source_timestamp);                       \
                                                                          \
    NDDSUSERDllExport DDSCDllExport                                       \
    DDS_ReturnCode_t TDataWriter ## _unregister_instance(                 \
        TDataWriter* self, const TData* instance_data,                    \
        const DDS_InstanceHandle_t* handle);                              \
                                                                          \
    NDDSUSERDllExport DDSCDllExport                                       \
    DDS_ReturnCode_t TDataWriter ## _unregister_instance_w_timestamp(     \
        TDataWriter* self, const TData* instance_data,                    \
        const DDS_InstanceHandle_t* handle,                               \
        const struct DDS_Time_t* source_timestamp);                       \
                                                                          \
    NDDSUSERDllExport DDSCDllExport                                       \
    DDS_ReturnCode_t TDataWriter ## _write(                               \
        TDataWriter* self, const TData* instance_data,                    \
        const DDS_InstanceHandle_t* handle);                              \
                                                                          \
    NDDSUSERDllExport DDSCDllExport                                       \
    DDS_ReturnCode_t TDataWriter ## _write_w_timestamp(                   \
        TDataWriter* self, const TData* instance_data,                    \
        const DDS_InstanceHandle_t* handle,                               \
        const struct DDS_Time_t* source_timestamp);                       \
                                                                          \
    NDDSUSERDllExport DDSCDllExport                                       \
    DDS_ReturnCode_t TDataWriter ## _write_w_params(                      \
        TDataWriter* self, const TData* instance_data,                    \
        struct DDS_WriteParams_t* params);                                \
                                                                          \
    NDDSUSERDllExport DDSCDllExport                                       \
    DDS_ReturnCode_t TDataWriter ## _dispose(                             \
        TDataWriter* self, const TData* instance_data,                    \
        const DDS_InstanceHandle_t* instance_handle);                     \
                                                                          \
    NDDSUSERDllExport DDSCDllExport                                       \
    DDS_ReturnCode_t TDataWriter ## _dispose_w_timestamp(                 \
        TDataWriter* self, const TData* instance_data,                    \
        const DDS_InstanceHandle_t* instance_handle,                      \
        const struct DDS_Time_t* source_timestamp);                       \
                                                                          \
    NDDSUSERDllExport DDSCDllExport                                       \
    DDS_ReturnCode_t TDataWriter ## _get_loan(                            \
        TDataWriter* self,                                                \
        TData** data);                                                    \
                                                                          \
    NDDSUSERDllExport DDSCDllExport                                       \
    DDS_ReturnCode_t TDataWriter ## _discard_loan(                        \
        TDataWriter* self,                                                \
        TData* data);                                                     \
                                                                          \
    NDDSUSERDllExport DDSCDllExport                                       \
    TData* TDataWriter ## _create_data(                                   \
        TDataWriter* self);                                               \
                                                                          \
    NDDSUSERDllExport DDSCDllExport                                       \
    DDS_Boolean TDataWriter ## _delete_data(                              \
            TDataWriter* self,                                            \
            TData* sample)

/*ce \dref_TYPESUPPORT
 */
#define DDS_TYPESUPPORT_C(TTypeSupport, TData)                \
    NDDSUSERDllExport DDSCDllExport                           \
    DDS_ReturnCode_t TTypeSupport ## _register_type(          \
        DDS_DomainParticipant* participant,                   \
        const char* type_name);                               \
                                                              \
    NDDSUSERDllExport DDSCDllExport                           \
    DDS_ReturnCode_t TTypeSupport ## _unregister_type(        \
        DDS_DomainParticipant* participant,                   \
        const char* type_name);                               \
                                                              \
    NDDSUSERDllExport DDSCDllExport                           \
    const char* TTypeSupport ## _get_type_name(void);         \
                                                              \
    NDDSUSERDllExport DDSCDllExport                           \
    TData* TTypeSupport ## _create_data(void);                \
                                                              \
    NDDSUSERDllExport DDSCDllExport                           \
    DDS_ReturnCode_t TTypeSupport ## _delete_data(TData*);                \
                                                              \
    NDDSUSERDllExport DDSCDllExport                           \
    DDS_ReturnCode_t TTypeSupport ## _serialize_data_to_cdr_buffer_ex(   \
                            char *buffer,                                \
                            unsigned int *length,                        \
                            const TData *a_data,                         \
                            DDS_DataRepresentationId_t  representation); \
                                                                         \
    NDDSUSERDllExport DDSCDllExport                                      \
    DDS_ReturnCode_t TTypeSupport ## _serialize_data_to_cdr_buffer(      \
                                         char *buffer,                   \
                                         unsigned int *length,           \
                                         const TData *a_data);           \
                                                                         \
    NDDSUSERDllExport DDSCDllExport                                      \
    DDS_ReturnCode_t TTypeSupport ## _deserialize_data_from_cdr_buffer(  \
                                       TData *sample,                    \
                                       const char *buffer,               \
                                       unsigned int length)
#ifdef DOXYGEN_DOCUMENTATION_ONLY

/*ce \dref_FooTypeSupport
 */
struct FooTypeSupport { int x; };


/*ce \dref_FooTypeSupport_create_data
 * @ingroup DDSUserDataModule
 */
Foo*
FooTypeSupport_create_data(void);

/*ce \dref_FooTypeSupport_delete_data
 * @ingroup DDSUserDataModule
 */
DDS_ReturnCode_t
FooTypeSupport_delete_data(Foo *sample);

/*ce \dref_FooTypeSupport_register_type
 * @ingroup DDSUserDataModule
 */
DDS_ReturnCode_t
FooTypeSupport_register_type(DDS_DomainParticipant* participant,
                             const char* type_name);

/*ce \dref_FooTypeSupport_unregister_type
 * @ingroup DDSUserDataModule
 */
DDS_ReturnCode_t
FooTypeSupport_unregister_type(DDS_DomainParticipant* participant,
                               const char* type_name);

/*ce \dref_FooTypeSupport_serialize_data_to_cdr_buffer_ex
 * @ingroup DDSUserDataModule
 */
DDS_ReturnCode_t
FooTypeSupport_serialize_data_to_cdr_buffer_ex(
                                   char *buffer,
                                   unsigned int *length,
                                   const TData *a_data,
                                   DDS_DataRepresentationId_t  representation);

/*ce \dref_FooTypeSupport_serialize_data_to_cdr_buffer
 * @ingroup DDSUserDataModule
 */
DDS_ReturnCode_t
FooTypeSupport_serialize_data_to_cdr_buffer(char *buffer,
                                            unsigned int *length,
                                            const Foo *a_data);

/*ce \dref_FooTypeSupport_deserialize_data_from_cdr_buffer
 * @ingroup DDSUserDataModule
 */
DDS_ReturnCode_t
FooTypeSupport_deserialize_data_from_cdr_buffer(Foo *a_data,
                                                const char *buffer,
                                                unsigned int length);

/****************** Typed DataWriter APIs **************************/

/* Needed so DOXYGEN generates the hyperlinks to TSeq */
typedef TSeq FooSeq;

/*ce \dref_FooDataWriter
 */
struct FooDataWriter {  };

/*ce \dref_FooDataWriter_narrow
 */
FooDataWriter* 
FooDataWriter_narrow(DDS_DataWriter* writer);

/*ce \dref_FooDataWriter_as_datawriter
 */
DDS_DataWriter* 
FooDataWriter_as_datawriter(FooDataWriter* writer);

/*ce \dref_FooDataWriter_register_instance
 */
DDS_InstanceHandle_t
FooDataWriter_register_instance(
        FooDataWriter * self,
        const void *instance_data);

/*ce \dref_FooDataWriter_register_instance_w_timestamp
 */
DDS_InstanceHandle_t
FooDataWriter_register_instance_w_timestamp(
        FooDataWriter * self,
        const void *instance_data,
        const struct DDS_Time_t *source_timestamp);

/*ce \dref_FooDataWriter_unregister_instance
 */
DDS_ReturnCode_t
FooDataWriter_unregister_instance(
        FooDataWriter *self,
        const void *instance_data,
        const DDS_InstanceHandle_t *handle);

/*ce \dref_FooDataWriter_unregister_instance_w_timestamp
 */
DDS_ReturnCode_t
FooDataWriter_unregister_instance_w_timestamp(
        FooDataWriter *self,
        const void *instance_data,
        const DDS_InstanceHandle_t *handle,
        const struct DDS_Time_t *source_timestamp);

/*ce \dref_FooDataWriter_dispose
 */
DDS_ReturnCode_t
FooDataWriter_dispose(
        FooDataWriter *self,
        const void *instance_data,
        const DDS_InstanceHandle_t *handle);

/*ce \dref_FooDataWriter_dispose_w_timestamp
 */
DDS_ReturnCode_t
FooDataWriter_dispose_w_timestamp(
        FooDataWriter *self,
        const void *instance_data,
        const DDS_InstanceHandle_t *handle,
        const struct DDS_Time_t *source_timestamp);

/*ce \dref_FooDataWriter_write
 */
DDS_ReturnCode_t
FooDataWriter_write(
        FooDataWriter *self,
        const void *instance_data,
        const DDS_InstanceHandle_t * handle);

/*ce \dref_FooDataWriter_write_w_timestamp
 */
DDS_ReturnCode_t
FooDataWriter_write_w_timestamp(
        FooDataWriter *self,
        const void *instance_data,
        const DDS_InstanceHandle_t *handle,
        const struct DDS_Time_t *source_timestamp);

/*ce \dref_FooDataWriter_write_w_params
 */
DDS_ReturnCode_t
FooDataWriter_write_w_params(
        FooDataWriter *self,
        const void *instance_data,
        struct DDS_WriteParams_t *params);

/*ce \dref_FooDataWriter_create_data
 */
void*
FooDataWriter_create_data(DDS_DataWriter *self);

/*ce \dref_FooDataWriter_delete_data
 */
DDS_Boolean
FooDataWriter_delete_data(FooDataWriter *self, void *sample);

/*ce \dref_FooDataWriter_get_loan
 */
DDS_ReturnCode_t
FooDataWriter_get_loan(TDataWriter *self, Foo **sample);

/*ce \dref_FooDataWriter_discard_loan
 */
DDS_ReturnCode_t
FooDataWriter_discard_loan(TDataWriter *self, Foo *sample);

#endif

/* ----------------------------------------------------------------- */

/*ce \dref_DATAREADER
 */
#define DDS_DATAREADER_C(TDataReader, TDataSeq, TData)                       \
    typedef struct TDataReader TDataReader;                                  \
                                                                             \
    NDDSUSERDllExport DDSCDllExport TDataReader* TDataReader ## _narrow(     \
        DDS_DataReader* reader);                                             \
    NDDSUSERDllExport DDSCDllExport DDS_DataReader* TDataReader ## _as_datareader(   \
        TDataReader* reader);                                                \
    NDDSUSERDllExport DDSCDllExport                                          \
    DDS_ReturnCode_t TDataReader ## _read(TDataReader* self,                 \
        struct TDataSeq* received_data, struct DDS_SampleInfoSeq* info_seq,  \
        DDS_Long max_samples,                                                    \
        DDS_SampleStateMask sample_states, DDS_ViewStateMask view_states,    \
        DDS_InstanceStateMask instance_states);                              \
                                                                             \
    NDDSUSERDllExport DDSCDllExport                                          \
    DDS_ReturnCode_t TDataReader ## _take(TDataReader* self,                 \
        struct TDataSeq* received_data, struct DDS_SampleInfoSeq* info_seq,  \
        DDS_Long max_samples,                                                    \
        DDS_SampleStateMask sample_mask, DDS_ViewStateMask view_mask,        \
        DDS_InstanceStateMask instance_mask);                                \
                                                                             \
    NDDSUSERDllExport DDSCDllExport                                          \
    DDS_ReturnCode_t TDataReader ## _read_next_sample(                       \
        TDataReader* self, TData* received_data,                             \
        struct DDS_SampleInfo* sample_info);                                 \
                                                                             \
    NDDSUSERDllExport DDSCDllExport                                          \
    DDS_ReturnCode_t TDataReader ## _take_next_sample(                       \
        TDataReader* self, TData* received_data,                             \
        struct DDS_SampleInfo* sample_info);                                 \
                                                                             \
    NDDSUSERDllExport DDSCDllExport                                          \
    DDS_ReturnCode_t TDataReader ## _read_instance(                          \
        TDataReader* self,                                                   \
        struct TDataSeq* received_data, struct DDS_SampleInfoSeq* info_seq,  \
        DDS_Long max_samples,                                                    \
        const DDS_InstanceHandle_t* a_handle,                                \
        DDS_SampleStateMask sample_states, DDS_ViewStateMask view_states,    \
        DDS_InstanceStateMask instance_states);                              \
                                                                             \
    NDDSUSERDllExport DDSCDllExport                                          \
    DDS_ReturnCode_t TDataReader ## _take_instance(                          \
        TDataReader* self,                                                   \
        struct TDataSeq* received_data, struct DDS_SampleInfoSeq* info_seq,  \
        DDS_Long max_samples,                                                    \
        const DDS_InstanceHandle_t* a_handle,                                \
        DDS_SampleStateMask sample_states, DDS_ViewStateMask view_states,    \
        DDS_InstanceStateMask instance_states);                              \
                                                                             \
    NDDSUSERDllExport DDSCDllExport                                          \
    DDS_ReturnCode_t TDataReader ## _return_loan(                            \
        TDataReader* self,                                                   \
        struct TDataSeq* received_data, struct DDS_SampleInfoSeq* info_seq); \
                                                                             \
    NDDSUSERDllExport DDSCDllExport                                          \
    DDS_InstanceHandle_t TDataReader ## _lookup_instance(                    \
        TDataReader* self,                                                   \
        const TData* key_holder);                                            \
                                                                             \
    NDDSUSERDllExport DDSCDllExport DDS_ReturnCode_t                         \
    TDataReader ## _is_data_consistent(                                      \
        TDataReader *self,                                                   \
        DDS_Boolean *is_data_consistent,                                     \
        const TData *sample,                                                 \
        const struct DDS_SampleInfo *sample_info)                            \
        
#ifdef DOXYGEN_DOCUMENTATION_ONLY

/*ce \dref_FooDataReader
 */
struct FooDataReader {};

/*ce \dref_FooDataReader_narrow
 */
FooDataReader* 
FooDataReader_narrow(DDS_DataReader* reader);

/*ce \dref_FooDataReader_as_datareader
 */
DDS_DataReader* 
FooDataReader_as_datareader(FooDataReader* reader);
            
/*ce \dref_FooDataReader_read
 */
DDS_ReturnCode_t 
FooDataReader_read(FooDataReader* self,
                   struct FooSeq* received_data, 
                   struct DDS_SampleInfoSeq* info_seq,
                   DDS_Long max_samples,
                   DDS_SampleStateMask sample_states, 
                   DDS_ViewStateMask view_states,
                   DDS_InstanceStateMask instance_states);

/*ce \dref_FooDataReader_take
 */
DDS_ReturnCode_t 
FooDataReader_take(FooDataReader* self,
                   struct FooSeq* received_data, 
                   struct DDS_SampleInfoSeq* info_seq,
                   DDS_Long max_samples,
                   DDS_SampleStateMask sample_states, 
                   DDS_ViewStateMask view_states,
                   DDS_InstanceStateMask instance_states);

/*ce \dref_FooDataReader_read_next_sample
 */
DDS_ReturnCode_t 
FooDataReader_read_next_sample(FooDataReader* self,
                               struct Foo* received_data, 
                               struct DDS_SampleInfo* sample_info);

/*ce \dref_FooDataReader_take_next_sample
 */
DDS_ReturnCode_t 
FooDataReader_take_next_sample(FooDataReader* self,
                               struct Foo* received_data, 
                               struct DDS_SampleInfo* sample_info);

/*ce \dref_FooDataReader_read_instance
 */
DDS_ReturnCode_t 
FooDataReader_read_instance(FooDataReader* self,
                            struct FooSeq* received_data, 
                            struct DDS_SampleInfoSeq* info_seq,
                            DDS_Long max_samples,
                            const DDS_InstanceHandle_t* a_handle,
                            DDS_SampleStateMask sample_states, 
                            DDS_ViewStateMask view_states,
                            DDS_InstanceStateMask instance_states);

/*ce \dref_FooDataReader_take_instance
 */
DDS_ReturnCode_t 
FooDataReader_take_instance(FooDataReader* self,
                            struct FooSeq* received_data, 
                            struct DDS_SampleInfoSeq* info_seq,
                            DDS_Long max_samples,
                            const DDS_InstanceHandle_t* a_handle,
                            DDS_SampleStateMask sample_states, 
                            DDS_ViewStateMask view_states,
                            DDS_InstanceStateMask instance_states);

/*ce \dref_FooDataReader_return_loan
 */
DDS_ReturnCode_t 
FooDataReader_return_loan(FooDataReader* self, 
                          FooSeq* received_data,
                          struct DDS_SampleInfoSeq* info_seq);

/*ce \dref_FooDataReader_lookup_instance
 */
DDS_InstanceHandle_t 
FooDataReader_lookup_instance(FooDataReader* self, 
                              const Foo* key_holder);

/*ce \dref_FooDataReader_is_data_consistent
 */
DDS_ReturnCode_t
FooDataReader_is_data_consistent(DDS_DataReader *self,
                               DDS_Boolean *is_data_consistent,
                               const Foo *sample,
                               const struct DDS_SampleInfo *sample_info);

#endif /* DOXYGEN_DOCUMENTATION_ONLY */


#ifdef __cplusplus
    } /* extern "C" */
#endif

#endif /* dds_c_data_h */
