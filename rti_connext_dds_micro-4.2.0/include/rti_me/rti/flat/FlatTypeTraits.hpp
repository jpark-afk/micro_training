/*
(c) Copyright, Real-Time Innovations, 2018.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
*/

#ifndef RTI_DDS_FLAT_FLATTYPETRAITS_HPP_
#define RTI_DDS_FLAT_FLATTYPETRAITS_HPP_

#include "rti/flat/Offset.hpp"
#include "rti/flat/SequenceOffsets.hpp"
#include "rti/flat/AggregationBuilders.hpp"

namespace rti { namespace flat {


/**
 * @ingroup RTIFlatSampleModule
 * @brief Given a Sample, an Offset or a Builder, it allows obtaining the other types.
 *
 * @tparam T One of the following:
 * \li A Sample type, such as \ref MyFlatMutable
 * \li An Offset type, such as MyFlatMutableOffset
 * \li A Builder type, such as MyFlatMutableBuilder
 *
 * <p>Given \p T, this type provides the following typedefs:
 * \li \p flat_type_traits<T>::offset, T's related offset type
 * (undefined if T itself is an Offset)
 * \li \p flat_type_traits<T>::builder, T's related builder type
 * (undefined if  T itself is a Builder, or the topic-type is not mutable)
 * \li \p flat_type_traits<T>::flat_type, T's related Sample type
 * (undefined if  T itself is a Sample type)
 * \li \p flat_type_traits<T>::plain_type, T's equivalent definition as a plain
 * (non-FlatData) type.
 *
 * For example, for \p T = \ref MyFlatMutable, flat_type_traits is defined as follows:
 *
 * \code
 *      template <>
 *       struct flat_type_traits<MyFlatMutable> {
 *           typedef MyFlatMutablePlainHelper plain_type;
 *           typedef MyFlatMutableOffset offset;
 *           typedef MyFlatMutableBuilder builder;
 *       };
 * \endcode
 *
 * Or if \p T = MyFlatMutableOffset:
 *
 * \code
 *     template <>
 *     struct flat_type_traits<MyFlatMutableOffset> {
 *           typedef MyFlatMutable flat_type;
 *           typedef MyFlatMutablePlainHelper plain_type;
 *           typedef MyFlatMutableBuilder builder;
 *     };
 * \endcode
 *
 * @see rti::flat::plain_cast()
 */
// For aggregation types, this template is to be specialized in generated code
template <typename T>
struct flat_type_traits;

template <typename T, unsigned int N>
struct flat_type_traits<FinalArrayOffset<T, N> > {
    typedef typename flat_type_traits<T>::plain_type plain_type;
};

template <typename T, unsigned int N>
struct flat_type_traits<PrimitiveArrayOffset<T, N> > {
    typedef T plain_type;
};

template <typename T>
struct flat_type_traits<PrimitiveSequenceOffset<T> > {
    typedef T plain_type;
};

template <typename T, unsigned int N>
struct flat_type_traits<FinalAlignedArrayOffset<T, N> > {
    typedef typename flat_type_traits<T>::plain_type plain_type;
};

template <typename T, unsigned int N>
struct flat_type_traits<MutableArrayOffset<T, N> > {
    typedef typename flat_type_traits<T>::plain_type plain_type;
};

template <typename T>
struct flat_type_traits<SequenceOffset<T> > {
    typedef typename flat_type_traits<T>::plain_type plain_type;
};

template <>
struct flat_type_traits<StringOffset > :
        flat_type_traits<PrimitiveSequenceOffset<char> > {
};

template <typename T>
struct flat_type_traits<PrimitiveOffset<T> > {
    typedef T plain_type;
};

/**
 * @ingroup RTIFlatOffsetsModule
 * @brief Casts into an equivalent plain C++ type
 *
 * Some FlatData types can be cast to their equivalent <b>plain</b> definition as
 * a regular non-FlatData C++ type. This is a more efficient way to access
 * the data. This function casts, if possible, the member pointed to by the
 * offset argument to an equivalent plain C++ type. Any changes made through
 * the plain type are reflected in the FlatData sample.
 *
 * @tparam OffsetType The Offset type
 *
 * @pre plain_cast requires the type to meet the following restrictions:
 * - The type must be a final struct, or an array or sequence of members of a
 *   type that meets these restrictions (including primitive types)
 * - The type must be defined in a way such that the packing of the plain C++ type
 *   doesn't differ from the padding in XCDR2.
 * - The type may not inherit from another type.
 * - In addition, the sample must be serialized in the native endianness.
 *   For example, if a subscribing application on a little-endian platform
 *   receives a sample published from a big-endian system, it is not possible
 *   to plain_cast the sample or any of its members, except if the member is a
 *   primitive array or sequence of 1-byte elements.
 *
 * \ref rti::flat::OffsetBase::is_cpp_compatible "offset.is_cpp_compatible()"
 * indicates if the member meets the requirements. If the type doesn't, this
 * function \if CPP2_LANGUAGE_ONLY throws dds::core::PreconditionNotMetError.
 * \else returns NULL. \endif
 *
 * @param offset The offset to the member to cast.
 * @return The data that \p offset referred to, cast as a plain C++ type with the
 * same definition. If \p offset is an array or sequence member, the returned pointer
 * represents a C++ array with the same number of elements.
 *
 * The following table summarizes the possible parameters to this function (assuming
 * that they meet the previous requirements), and the return type in each case:
 *
 * | \p OffsetType | return type
 * | ------------- | -----------
 * | Offset to a final struct, such as `MyFlatFinalOffset` | `MyFlatFinalPlainHelper*` (pointer to a single element). This is a type with the same IDL definition, but without `@language_binding(FLAT_DATA)`.
 * | Offset to an array of final structs, such as `FinalArrayOffset<MyFlatFinalOffset, N>` | `MyFlatFinalPlainHelper*` (array of `N` elements)
 * | Offset to a sequence of final structs, such as `SequenceOffset<MyFlatFinalOffset>` | `MyFlatFinalPlainHelper*` (array of `offset.element_count()` elements)
 * | Offset to an array of primitive types, such as `PrimitiveArrayOffset<int32_t, N>` | `int32_t*` (array of `N` elements)
 * | Offset to a sequence of primitive types, such as `PrimitiveSequenceOffset<int32_t>` | `int32_t*` (array of `offset.element_count()` elements)
 *
 *
 * Due to the performance advantages that \p plain_cast offers, it is recommended
 * to define FlatData types in a way that their largest member(s) can be
 * \p plain_cast.
 *
 * The following example shows how to use \p plain_cast to cast a MyFlatFinalOffset
 * into the type with the same IDL definition as \ref MyFlatFinal, but without
 * the \p \@language_binding(FLAT_DATA) annotation:
 *
 * \code
 *  MyFlatMutable *my_sample = ...;
 *  auto my_root = my_sample->root();
 *  auto my_final = my_root.my_final();
 *
 *  // Get the plain C++ type and modify an array
 *  auto my_final_plain = rti::flat::plain_cast(my_final);
 *  my_final_plain->my_primitive(3); // this is using the plain C++ setter
 *  my_final_plain->my_complex_array()[1].x(20); // this is a std::array
 *
 *  // The change to my_complex_array is reflected in the FlatData sample
 *  std::cout << my_final.my_primitive() << std::endl; // 3
 *  std::cout << my_final.my_complex_array().get_element(1).x() << std::endl; // 20
 * \endcode
 *
 * This example shows how to use plain_cast to efficiently build a sequence
 * of final elements and a sequence of integers, both members of a mutable type:
 *
 * \code
 *   MyFlatMutableBuilder builder = rti::flat::build_data(writer);
 *
 *   // 1)
 *   //
 *   // Build a sequence of 5 elements with add_n() instead of 5 calls to
 *   // add_next()
 *   rti::flat::FinalSequenceBuilder<MyFlatFinalOffset> seq_builder =
 *               builder.build_my_final_seq();
 *   seq_builder.add_n(5);
 *   // Finish the member, getting the offset to the member
 *   rti::flat::SequenceOffset<MyFlatFinalOffset> seq_offset = seq_builder.finish();
 *
 *   // Cast it to an array of plain C++ type and initialize it.
 *   // This way to initialize it is more efficient than add_next().
 *   auto seq_elements = rti::flat::plain_cast(seq_offset);
 *   for (int i = 0; i < 5; i++) {
 *       seq_elements[i].my_primitive(i);
 *       // ...
 *   }
 *
 *   // 2)
 *   //
 *   // Build a sequence of 1000 integers
 *   auto seq_builder = builder.build_my_primitive_seq();
 *   // make space for 1000 elements, but leave them uninitialized
 *   seq_builder.add_n(1000);
 *   auto seq_offset = seq_builder.finish();
 *
 *   // Initialize the elements:
 *   int32_t *elements = rti::flat::plain_cast(seq_offset);
 *   for (int i = 0; i < 1000; i++) {
 *       elements[i] = ...;
 *   }
 *
 *   // ... continue building the sample using 'builder'
 *  \endcode
 *
 */
template <typename OffsetType>
typename flat_type_traits<OffsetType>::plain_type* plain_cast(OffsetType& offset)
{
    RTI_FLAT_CHECK_PRECONDITION(offset.is_cpp_compatible(), return NULL);
    return reinterpret_cast<typename flat_type_traits<OffsetType>::plain_type*>(
            offset.get_buffer());
}

/**
 * @ingroup RTIFlatOffsetsModule
 * @brief Const version of plain_cast
 */
template <typename OffsetType>
const typename flat_type_traits<OffsetType>::plain_type* plain_cast(const OffsetType& offset)
{
    RTI_FLAT_CHECK_PRECONDITION(offset.is_cpp_compatible(), return NULL);
    return reinterpret_cast<const typename flat_type_traits<OffsetType>::plain_type*>(
            offset.get_buffer());
}

namespace detail {

template <typename OffsetKind>
struct offset_kind_is_fixed_size {
    enum { value = 0 };
};

template <>
struct offset_kind_is_fixed_size<rti::flat::fixed_size_type_tag_t> {
    enum { value = 1 };
};

}

// Utility to determine if a Flat Data type is fixed size (final) or variable
// size (mutable)
template <typename T>
struct is_fixed_size_type
        : detail::offset_kind_is_fixed_size<typename T::Offset::offset_kind>
{
};

} }

#endif // RTI_DDS_FLAT_FLATSAMPLE_HPP_

