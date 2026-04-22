/*
 * FILE: xcd_infrastructure_psm.h
 *
 * (c) Copyright 2018-2018 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#ifndef dds_cpp_flat_data_h
#define dds_cpp_flat_data_h

// The traditional C++ API disables exceptions in rtiflat.hpp
#define RTI_FLAT_DATA_NO_EXCEPTIONS
// Serialization functions use, for example, int instead of int32_t
#define RTI_AVOID_FIXED_WIDTH_INTEGERS

#include "rti/flat/rtiflat.hpp" // the actual Flat Data support

#undef RTI_AVOID_FIXED_WIDTH_INTEGERS
#undef RTI_FLAT_DATA_NO_EXCEPTIONS

// Used by rtiddsgen generated code to specialize FlatData implementation traits
// for enums. In the modern C++ this is done using std::enable_if and
// std::is_enum.
#define RTI_FLAT_DATA_DEFINE_ENUM_TRAITS(__EnumType)                           \
  template <unsigned int N>                                                    \
  struct final_offset_initializer<                                             \
      rti::flat::PrimitiveArrayOffset<__EnumType, N> > {                       \
    static bool                                                                \
    initialize(rti::flat::PrimitiveArrayOffset<__EnumType, N> &array) {        \
      __EnumType default_value = default_primitive_value<__EnumType>::get();   \
      if (static_cast<int>(default_value) == 0) {                              \
        memset(array.get_buffer(), 0, array.get_buffer_size());                \
      } else {                                                                 \
        for (unsigned int i = 0; i < array.element_count(); i++) {             \
          array.set_element(i, default_value);                                 \
        }                                                                      \
      }                                                                        \
      return true;                                                             \
    }                                                                          \
  };                                                                           \
                                                                               \
  template <> struct primitive_sequence_dheader_gen<__EnumType> {              \
    typedef DHeaderGeneratorForEnum type;                                      \
  };                                                                           \
                                                                               \
  template <> struct primitive_sequence_header_length<__EnumType> {            \
    static size_t value() {                                                    \
      return sizeof(rti::xcdr::length_t) +                                     \
             (is_dheader_required_in_enum_collections()                        \
                  ? RTI_XCDR_DHEADER_SIZE                                      \
                  : 0);                                                        \
    }                                                                          \
  };                                                                           \
                                                                               \
  template <unsigned int N>                                                    \
  struct is_fixed_type_w_dheader<                                              \
      rti::flat::PrimitiveArrayOffset<__EnumType, N> > {                       \
    static bool value() { return is_dheader_required_in_enum_collections(); }  \
  };                                                                           \
                                                                               \
  template <> struct lc_code<PrimitiveSequenceBuilder<__EnumType> > {          \
    static int value() {                                                       \
      return is_dheader_required_in_enum_collections()                         \
                 ? 5                                                           \
                 : primitive_lc_code_helper<sizeof(__EnumType)>::sequence;     \
    }                                                                          \
  };                                                                           \

namespace rti { namespace flat {

template <typename TopicType>
typename rti::flat::flat_type_traits<TopicType>::builder build_data(
        typename TopicType::DataWriter *writer)
{
    typedef typename rti::flat::flat_type_traits<TopicType>::builder Builder;
    const struct DDS_DataWriterResourceLimitsQosPolicy *policy;

    DDS_TypePlugin* type_plugin = DDS_DataWriter_get_type_plugin(
                              (DDS_DataWriter*) writer->get_c_entity());
    unsigned int max_size = DDS_TypePlugin_get_sample_size(type_plugin);
    TopicType *type;
    DDS_ReturnCode_t retcode = writer->get_loan(type);
    if (retcode != DDS_RETCODE_OK)
    {
        return Builder();
    }

    policy = DDS_DataWriter_get_writer_resource_limits_ref((DDS_DataWriter*) writer->get_c_entity());

    DDS_Boolean initialize_data = policy->initialize_writer_loaned_sample;

    return Builder(
            reinterpret_cast<unsigned char *>(type),
            max_size,
            initialize_data == DDS_BOOLEAN_TRUE);
}

template <typename BuilderType>
void discard_builder(
        typename rti::flat::flat_type_traits<BuilderType>::flat_type::DataWriter *writer,
        BuilderType& builder)
{
    typedef typename rti::flat::flat_type_traits<BuilderType>::flat_type TopicType;
    unsigned char *buffer = builder.discard_sample_impl();
    if (buffer != NULL) {
        writer->discard_loan(*reinterpret_cast<TopicType *>(buffer));
    }
}

} }


#endif /* dds_cpp_flat_data_h */


