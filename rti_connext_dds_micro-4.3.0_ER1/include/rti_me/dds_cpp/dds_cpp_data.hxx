/* 
 (c) Copyright, Real-Time Innovations, 2013-2015.  All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.
 
 modification history
 -------------------- 
 23jan2016,as  MICRO-1523 Replace RTI_BOOL with bool
 06jun2014,as  Made definition of NDDSUSERDllExport conditional;
               Added TTypeSupport:register_type/unregister_type
 02may2014,as  MICRO-331 Document renaming of TypeSupport's methods
               to avoid collisions with reserved C++ keywords.
 19jul2013,as  Changed typed DR and DW macros to use implementation
               classes, added additional methods that are now
               supported
 15feb2013,eh  Created
=========================================================================*/
/*i @file
 */
/*e
 * @addtogroup DDSUserDataModule
 * @brief Defines the \dds user data type support.
 */
#ifndef dds_cpp_data_hxx
#define dds_cpp_data_hxx

#ifndef dds_cpp_dll_hxx
  #include "dds_cpp/dds_cpp_dll.hxx"
#endif
#ifndef dds_cpp_infrastructure_hxx
  #include "dds_cpp/dds_cpp_infrastructure.hxx"
#endif
#ifndef dds_cpp_publication_hxx
  #include "dds_cpp/dds_cpp_publication.hxx"
#endif
#ifndef dds_cpp_subscription_hxx
  #include "dds_cpp/dds_cpp_subscription.hxx"
#endif
#ifndef dds_cpp_domain_hxx
  #include "dds_cpp/dds_cpp_domain.hxx"
#endif


/* Make sure that NDDSUSERDllExport expands to nothing by default */
#ifndef NDDSUSERDllExport
#define NDDSUSERDllExport
#endif

/* ----------------------------------------------------------------- */

/* ================================================================= */
/*                   DATA WRITER                                     */
/* ================================================================= */

/*e \dref_DATAWRITER
 */
#define DDS_DATAWRITER_CPP(TDataWriter, TData) \
 \
class NDDSUSERDllExport DDSCPPDllExport TDataWriter : \
    public DDSDataWriter_impl \
{ \
    friend void* TData##Plugin_create_typed_datawriter(void* c_writer);\
    friend void TData##Plugin_delete_typed_datawriter(void* wrapper);\
public: \
    static TDataWriter* narrow(DDSDataWriter *writer); \
    DDSDataWriter* as_datawriter(); \
    DDS_ReturnCode_t write(const TData& instance_data, \
                                   const DDS_InstanceHandle_t& handle); \
    DDS_InstanceHandle_t register_instance(const TData& instance_data);\
    DDS_InstanceHandle_t register_instance_w_timestamp(\
            const TData& instance_data,\
            const DDS_Time_t& source_timestamp);\
    DDS_ReturnCode_t unregister_instance(\
            const TData& instance_data,\
            const DDS_InstanceHandle_t& handle);\
    DDS_ReturnCode_t unregister_instance_w_timestamp(\
            const TData& instance_data,\
            const DDS_InstanceHandle_t& handle,\
            const DDS_Time_t& source_timestamp);\
    DDS_ReturnCode_t dispose(\
            const TData& instance_data,\
            const DDS_InstanceHandle_t& handle);\
    DDS_ReturnCode_t dispose_w_timestamp(\
            const TData& instance_data,\
            const DDS_InstanceHandle_t& handle,\
            const DDS_Time_t& source_timestamp);\
    DDS_ReturnCode_t write_w_timestamp(\
            const TData& instance_data,\
            const DDS_InstanceHandle_t& handle,\
            const DDS_Time_t& source_timestamp);\
    DDS_ReturnCode_t get_loan(\
            TData*& instance_data); \
    DDS_ReturnCode_t discard_loan(\
            TData& instance_data); \
    TData* create_data(); \
    bool delete_data(TData* sample); \
protected: \
    TDataWriter(DDS_DataWriter* cDataWriter); \
    virtual ~TDataWriter(); \
}


/* ----------------------------------------------------------------- */

/* ================================================================= */
/*                   DATA READER                                     */
/* ================================================================= */

/*e \dref_DATAREADER
 */
#define DDS_DATAREADER_CPP(TDataReader, TData) \
 \
class NDDSUSERDllExport DDSCPPDllExport TDataReader : \
    public DDSDataReader_impl \
{ \
    friend void* TData##Plugin_create_typed_datareader(void* c_reader);\
    friend void TData##Plugin_delete_typed_datareader(void* wrapper);\
  public: \
      DDSDataReader* as_datareader(); \
    static TDataReader* narrow(DDSDataReader *reader); \
  public: \
    DDS_ReturnCode_t read_next_sample(TData& received_data, \
                                              DDS_SampleInfo& sample_info); \
    \
    DDS_ReturnCode_t take_next_sample(TData& received_data, \
                                              DDS_SampleInfo& sample_info); \
    \
    DDS_ReturnCode_t read(\
        TData##Seq& received_data,\
        DDS_SampleInfoSeq& info_seq,\
        DDS_Long max_samples,\
        DDS_SampleStateMask sample_states,\
        DDS_ViewStateMask view_states,\
        DDS_InstanceStateMask instance_states);\
    \
    DDS_ReturnCode_t take(\
        TData##Seq& received_data,\
        DDS_SampleInfoSeq& info_seq,\
        DDS_Long max_samples,\
        DDS_SampleStateMask sample_states,\
        DDS_ViewStateMask view_states,\
        DDS_InstanceStateMask instance_states);\
    \
    DDS_ReturnCode_t read_instance(\
        TData##Seq& received_data,\
        DDS_SampleInfoSeq& info_seq,\
        DDS_Long max_samples,\
        const DDS_InstanceHandle_t& a_handle,\
        DDS_SampleStateMask sample_states,\
        DDS_ViewStateMask view_states,\
        DDS_InstanceStateMask instance_states);\
    \
    DDS_ReturnCode_t take_instance(\
        TData##Seq& received_data,\
        DDS_SampleInfoSeq& info_seq,\
        DDS_Long max_samples,\
        const DDS_InstanceHandle_t& a_handle,\
        DDS_SampleStateMask sample_states,\
        DDS_ViewStateMask view_states,\
        DDS_InstanceStateMask instance_states);\
    \
    DDS_ReturnCode_t \
        is_data_consistent( \
        DDS_Boolean &is_data_consistent, \
        const TData& sample,\
        const DDS_SampleInfo &sample_info); \
    \
    DDS_InstanceHandle_t\
    lookup_instance(const TData& key_holder);\
    \
    DDS_ReturnCode_t return_loan(\
        TData##Seq& received_data,\
        DDS_SampleInfoSeq& info_seq);\
  protected: \
    TDataReader(DDS_DataReader *cDataReader); \
    virtual ~TDataReader(); \
}

/* ================================================================= */
/*                   TYPE SUPPORT                                    */
/* ================================================================= */
/*e \dref_TYPESUPPORT
 */
#define DDS_TYPESUPPORT_CPP(TTypeSupport, TData)                         \
                                                                         \
    NDDSUSERDllExport DDSCDllExport                                      \
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
                                       unsigned int length);             \
                                                                         \
class NDDSUSERDllExport DDSCPPDllExport TTypeSupport                     \
{ \
  public: \
    static bool initialize_data(TData* a_data); \
    static bool finalize_data(TData* a_data); \
    static TData* create_data(); \
    static DDS_ReturnCode_t delete_data(TData* a_data); \
    static bool copy_data(TData* a_dst, const TData* a_src); \
    static const char* get_type_name();\
    static DDS_ReturnCode_t register_type(\
                    DDSDomainParticipant* participant,\
                    const char* type_name);\
    static DDS_ReturnCode_t unregister_type(\
                    DDSDomainParticipant* participant,\
                    const char* type_name);\
    static DDS_ReturnCode_t serialize_data_to_cdr_buffer_ex(   \
                            char *buffer,                                \
                            unsigned int &length,                        \
                            const TData *a_data,                         \
                            DDS_DataRepresentationId_t  representation); \
    static DDS_ReturnCode_t serialize_data_to_cdr_buffer(                \
                                         char *buffer,                   \
                                         unsigned int &length,           \
                                         const TData *a_data);           \
    static DDS_ReturnCode_t deserialize_data_from_cdr_buffer(            \
                                       TData *sample,                    \
                                       const char *buffer,               \
                                       unsigned int length);             \
                                                                         \
private: \
  TTypeSupport(); \
  ~TTypeSupport(); \
}


#ifdef DOXYGEN_DOCUMENTATION_ONLY
/* ----------------------------------------------------------------- */

 /*e @brief \st_interface \st_generic User data type specific interface.
   * @ingroup DDSUserDataModule
   */
class FooTypeSupport
{
  public:

    /*e \dref_FooTypeSupport_initialize_data
     */
    static bool initialize_data(Foo* sample);

    /*e \dref_FooTypeSupport_finalize_data
     */
    static bool finalize_data(Foo* sample);

    /*e \dref_FooTypeSupport_create_data
     */
    static Foo* create_data();

    /*e \dref_FooTypeSupport_delete_data
     */
    static DDS_ReturnCode_t delete_data(Foo* sample);

    /*e \dref_FooTypeSupport_copy_data
     */
    static bool copy_data(Foo* dst_data, Foo* src_data);

    /*e \dref_FooTypeSupport_register_type
     */
    static DDS_ReturnCode_t register_type(
                    DDSDomainParticipant* participant,
                    const char* type_name);

    /*e \dref_FooTypeSupport_unregister_type
     */
    static DDS_ReturnCode_t unregister_type(
                    DDSDomainParticipant* participant,
                    const char* type_name);

    /*e \dref_FooTypeSupport_serialize_data_to_cdr_buffer_ex
     */
    static DDS_ReturnCode_t serialize_data_to_cdr_buffer_ex(
                            char *buffer,
                            unsigned int &length,
                            const TData *a_data,
                            DDS_DataRepresentationId_t  representation);

    /*e \dref_FooTypeSupport_serialize_data_to_cdr_buffer
     */
    static DDS_ReturnCode_t serialize_data_to_cdr_buffer(
                                         char *buffer,
                                         unsigned int &length,
                                         const TData *a_data);

    /*e \dref_FooTypeSupport_deserialize_data_from_cdr_buffer
     */
    static DDS_ReturnCode_t deserialize_data_from_cdr_buffer(
                                       TData *a_data,
                                       const char *buffer,
                                       unsigned int length);

  private:
  FooTypeSupport();
  ~FooTypeSupport();
};

/*e
 *  @brief Declares the interface required to support a user data
 *        type-specific data reader.
 *  @ingroup DDSReaderModule
 */
class FooDataReader : public DDSDataReader_impl
{
    friend void* FooPlugin_create_typed_datareader(void* c_reader);
    friend void FooPlugin_delete_typed_datareader(void* wrapper);

  public:
    DDSDataReader* as_datareader();

     /*e \dref_FooDataReader_narrow
     */
    static FooDataReader* narrow(DDSDataReader *reader);

    /*e \dref_FooDataReader_return_loan
     */
    DDS_ReturnCode_t return_loan(FooSeq& received_data,
                     DDS_SampleInfoSeq& info_seq);

    /*e \dref_FooDataReader_read_next_sample
     */
    DDS_ReturnCode_t read_next_sample(Foo& received_data,
                                              DDS_SampleInfo& sample_info);

    /*e \dref_FooDataReader_take_next_sample
     */
    DDS_ReturnCode_t take_next_sample(Foo& received_data,
                                              DDS_SampleInfo& sample_info);

    /*e \dref_FooDataReader_read
     */
    DDS_ReturnCode_t read(
        FooSeq& received_data,
        DDS_SampleInfoSeq& info_seq,
        DDS_Long max_samples,
        DDS_SampleStateMask sample_states,
        DDS_ViewStateMask view_states,
        DDS_InstanceStateMask instance_states);

    /*e \dref_FooDataReader_take
     */
    DDS_ReturnCode_t take(
        FooSeq& received_data,
        DDS_SampleInfoSeq& info_seq,
        DDS_Long max_samples,
        DDS_SampleStateMask sample_states,
        DDS_ViewStateMask view_states,
        DDS_InstanceStateMask instance_states);

    /*e \dref_FooDataReader_read_instance
     */
    DDS_ReturnCode_t read_instance(
        FooSeq& received_data,
        DDS_SampleInfoSeq& info_seq,
        DDS_Long max_samples,
        const DDS_InstanceHandle_t& a_handle,
        DDS_SampleStateMask sample_states,
        DDS_ViewStateMask view_states,
        DDS_InstanceStateMask instance_states);

    /*e \dref_FooDataReader_take_instance
     */
    DDS_ReturnCode_t take_instance(
        FooSeq& received_data,
        DDS_SampleInfoSeq& info_seq,
        DDS_Long max_samples,
        const DDS_InstanceHandle_t& a_handle,
        DDS_SampleStateMask sample_states,
        DDS_ViewStateMask view_states,
        DDS_InstanceStateMask instance_states);

    /*e \dref_FooDataReader_lookup_instance
     */
    DDS_InstanceHandle_t
    lookup_instance(const Foo& key_holder);

    /*e \dref_FooDataReader_is_data_consistent
     */
    DDS_ReturnCode_t
    is_data_consistent(DDS_Boolean& is_data_consistent,
                       const Foo& sample,
                       const struct DDS_SampleInfo& sample_info);

  protected:
    FooDataReader(DDS_DataReader *cDataReader);
    virtual ~FooDataReader();
};


/*e
 *  @brief Declares the interface required to support a user data
 *         type-specific data writer.
 *  @ingroup DDSWriterModule
 */
class FooDataWriter : public DDSDataWriter_impl
{
    friend void* FooPlugin_create_typed_datawriter(void* c_writer);
    friend void FooPlugin_delete_typed_datawriter(void* wrapper);

public:
    /*e \dref_FooDataWriter_narrow
     */
    static FooDataWriter* narrow(DDSDataWriter *writer);

    /*e \dref_FooDataWriter_as_datawriter
     */
    DDSDataWriter* as_datawriter();

    /*e \dref_FooDataWriter_write
     */
    DDS_ReturnCode_t write(const Foo& instance_data,
                           const DDS_InstanceHandle_t& handle);

    /*e \dref_FooDataWriter_register_instance
     */
    DDS_InstanceHandle_t register_instance(const Foo& instance_data);

    /*e \dref_FooDataWriter_register_instance_w_timestamp
     */
    DDS_InstanceHandle_t register_instance_w_timestamp(
            const Foo& instance_data,
            const DDS_Time_t& source_timestamp);

    /*e \dref_FooDataWriter_unregister_instance
     */
    DDS_ReturnCode_t unregister_instance(
            const Foo& instance_data,
            const DDS_InstanceHandle_t& handle);

    /*e \dref_FooDataWriter_unregister_instance_w_timestamp
     */
    DDS_ReturnCode_t unregister_instance_w_timestamp(
            const Foo& instance_data,
            const DDS_InstanceHandle_t& handle,
            const DDS_Time_t& source_timestamp);

     /*e \dref_FooDataWriter_dispose
     */
    DDS_ReturnCode_t dispose(
            const Foo& instance_data,
            const DDS_InstanceHandle_t& handle);

    /*e \dref_FooDataWriter_dispose_w_timestamp
     */
    DDS_ReturnCode_t dispose_w_timestamp(
            const Foo& instance_data,
            const DDS_InstanceHandle_t& handle,
            const DDS_Time_t& source_timestamp);

    /*e \dref_FooDataWriter_write_w_timestamp
     */
    DDS_ReturnCode_t write_w_timestamp(
            const Foo& instance_data,
            const DDS_InstanceHandle_t& handle,
            const DDS_Time_t& source_timestamp);

    /*e \dref_FooDataWriter_get_loan
     */
    DDS_ReturnCode_t get_loan(Foo*& sample);

    /*e \dref_FooDataWriter_discard_loan
     */
    DDS_ReturnCode_t discard_loan(Foo& sample);

    /*e \dref_FooDataWriter_create_data
     */
    Foo* create_data();

    /*e \dref_FooDataWriter_delete_data
     */
    bool delete_data(Foo* sample);

protected:
    FooDataWriter(DDS_DataWriter* cDataWriter);
    virtual ~FooDataWriter();
};

#endif /* DOXYGEN_DOCUMENTATION_ONLY */

#endif /* dds_cpp_data_hxx */
