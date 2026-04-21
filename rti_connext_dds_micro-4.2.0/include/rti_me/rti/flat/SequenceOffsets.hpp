/*
(c) Copyright, Real-Time Innovations, 2018.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
*/

#ifndef RTI_DDS_FLAT_SEQUENCEOFFSETS_HPP_
#define RTI_DDS_FLAT_SEQUENCEOFFSETS_HPP_

#include "rti/flat/Offset.hpp"
#include "rti/flat/SequenceIterator.hpp"

namespace rti { namespace flat {

/**
 * @ingroup RTIFlatOffsetsModule
 * @brief Base class for Offsets to sequences and arrays of primitive types
 *
 * @tparam T The primitive type
 *
 */
template <typename T>
class AbstractPrimitiveList : public OffsetBase {
public:
    typedef T value_type;

protected:
    AbstractPrimitiveList()
    {
    }

    AbstractPrimitiveList(
            rti::flat::SampleBase *sample,
            offset_t offset,
            offset_t sequence_size)
        : OffsetBase(sample, offset, sequence_size)
    {
    }

public:

    bool is_cpp_compatible() const
    {
        RTI_FLAT_OFFSET_CHECK_NOT_NULL(return false);
        return sizeof(T) == 1 || !stream_.needs_byte_swap();
    }


    /*@private
     * @brief Gets direct access to the elements
     *
     * Returns a pointer to the elements of this sequence.
     *
     * @note If the Sample that contains this PrimitiveSequenceOffset has an endianess
     * different from the native one, the element bytes may be swapped. This can
     * happen if this Sample was published by a DataWriter running on a platform
     * with a different endianess and received by a DataReader running locally.
     *
     * This function is not for public use. See rti::flat::plain_cast() instead.
     */
    T * get_elements()
    {
        return reinterpret_cast<T*>(this->get_buffer());
    }

    /*@private
     * @brief Gets direct access to the array elements
     *
     * (Const overload)
     */
    const T * get_elements() const
    {
        return reinterpret_cast<const T*>(this->get_buffer());
    }

    /**
     * @brief Returns an element by index
     *
     * @param i The zero-based index of the element
     *
     * @see rti::flat::plain_cast() for a method to access all the elements at
     * once
     */
    T get_element(unsigned int i) const
    {
        // OffsetBase::serialize only checks this as a debug assertion; in
        // a sequence or array we need to do it in release mode too
        if (!stream_.check_size((i + 1) * ((unsigned int) sizeof(T)))) {
            return T();
        }

        return OffsetBase::template deserialize<T>(i * (unsigned int) sizeof(T));
    }

    /**
     * @brief Sets an element by index
     *
     * @param i The zero-based index of the element to set
     * @param value The value to set
     *
     * @return true if it was possible to set the element, or false if this
     * collection has less than i - 1 elements.
     *
     * @see rti::flat::plain_cast() for a method to access all the elements at
     * once
     */
    bool set_element(unsigned int i, T value)
    {
        // OffsetBase::serialize only checks this as a debug assertion; in
        // a sequence or array we need to do it in release mode too
        if (!stream_.check_size((i + 1) * static_cast<unsigned int>(sizeof(T)))) {
            return false;
        }

        return OffsetBase::serialize(i * static_cast<unsigned int>(sizeof(T)), value);
    }
};

namespace detail {

template <typename T>
struct PrimitiveSequenceOffsetHelper;

template <typename T, typename Enable = void>
struct primitive_sequence_header_length {
    static size_t value()
    {
        return sizeof(rti::xcdr::length_t);
    }
};

} // namespace detail

/**
 * @ingroup RTIFlatOffsetsModule
 * @brief Offset to a sequence of primitive elements
 *
 * @tparam T The primitive type
 *
 */
template <typename T>
class PrimitiveSequenceOffset : public AbstractPrimitiveList<T> {
public:
    typedef variable_size_type_tag_t offset_kind;
    typedef detail::PrimitiveSequenceOffsetHelper<T> Helper;

    PrimitiveSequenceOffset() : element_count_(0)
    {
    }

    PrimitiveSequenceOffset(
            rti::flat::SampleBase *sample,
            offset_t offset,
            offset_t serialized_size)
        : AbstractPrimitiveList<T>(
                // passing NULL causes the base constructors to fail
                serialized_size >= static_cast<offset_t>(header_length()) ? sample : NULL,
                offset + static_cast<offset_t>(header_length()),
                serialized_size - static_cast<offset_t>(header_length())),
        element_count_(0)
    {
#ifdef RTI_FLAT_DATA_NO_EXCEPTIONS
        if (!this->is_null()) { // without exceptions, we need to check if the base ctor failed
#endif
        rti::xcdr::Stream::Memento stream_memento(this->stream_);
        this->stream_.skip_back(sizeof(rti::xcdr::length_t));
        element_count_ = this->stream_.template deserialize_fast<rti::xcdr::length_t>();
#ifdef RTI_FLAT_DATA_NO_EXCEPTIONS
        }
#endif

    }

    /**
     * @brief Returns the number of elements
     */
    unsigned int element_count() const
    {
        RTI_FLAT_OFFSET_CHECK_NOT_NULL(return 0);

        return element_count_;
    }

private:
    static size_t header_length()
    {
        return detail::primitive_sequence_header_length<T>::value();
    }

    unsigned int element_count_;

};

namespace detail {

template <typename T>
struct PrimitiveSequenceOffsetHelper {
    static offset_t calculate_serialized_size(
            rti::flat::SampleBase *sample,
            offset_t absolute_offset,
            offset_t max_size)
    {
        RTI_FLAT_ASSERT(sample != NULL, return 0);
        RTI_FLAT_ASSERT(max_size > sizeof(rti::xcdr::length_t), return 0);

        PrimitiveSequenceOffset<T> tmp(sample, absolute_offset, max_size);

        // elements + header
        if ((RTIXCdrUnsignedLong_MAX
                - static_cast<unsigned int>(sizeof(rti::xcdr::length_t)))
                / static_cast<unsigned int>(sizeof(T))
                < tmp.element_count()) {
            RTI_FLAT_OFFSET_PRECONDITION_ERROR(
                    "Serialized size of a sequence of primitive elements "
                    "exceeds maximum representable 32-bit unsigned integer.",
                    return 0);
        }
        return (tmp.element_count() * static_cast<unsigned int>(sizeof(T)))
                + static_cast<unsigned int>(sizeof(rti::xcdr::length_t));
    }
};

} // namespace detail

/**
 * @ingroup RTIFlatOffsetsModule
 * @brief Offset to an array of primitive elements
 *
 * @tparam T The primitive type
 * @tparam N The array bound. For multidimensional arrays, \p N is the product
 * of each dimension bound.
 *
 * A PrimitiveArrayOffset may meet the requirements to be cast to an array of
 * the equivalent plain C++ element type (see rti::flat::plain_cast()).
 */
template <typename T, unsigned int N>
class PrimitiveArrayOffset : public AbstractPrimitiveList<T> {
public:
    typedef fixed_size_type_tag_t offset_kind;

    PrimitiveArrayOffset()
    {
    }

    PrimitiveArrayOffset(rti::flat::SampleBase *sample, offset_t offset)
        : AbstractPrimitiveList<T>(
                sample,
                offset + static_cast<offset_t>(header_length()),
                serialized_size(0))
    {
    }

public:

    /**
     * @brief Returns the number of elements, \p N
     *
     */
    unsigned int element_count() const
    {
        return N;
    }

    static offset_t serialized_size(offset_t)
    {
        return sizeof(T) * N;
    }

private:
    static size_t header_length()
    {
        return detail::primitive_sequence_header_length<T>::value()
                - sizeof(rti::xcdr::length_t);
    }
};

// Note: the modern C++ API defines a PrimitiveArrayOffset specializations for
// safe_enum and wchar_t

struct StringOffsetHelper;

/**
 * @ingroup RTIFlatOffsetsModule
 * @brief Offset to a string
 *
 */
class StringOffset : public PrimitiveSequenceOffset<char> {
public:
    typedef variable_size_type_tag_t offset_kind;
    typedef StringOffsetHelper Helper;

    StringOffset()
    {
    }

    StringOffset(
            SampleBase *sample,
            offset_t absolute_offset,
            offset_t serialized_size)
        : PrimitiveSequenceOffset<char>(
                sample,
                absolute_offset,
                serialized_size)
    {
    }

    /**
     * @brief Gets the string
     *
     * The string returned can be modified as long as its size doesn't change.
     */
    char * get_string()
    {
        return reinterpret_cast<char*>(this->get_buffer());
    }

    /**
     * @brief Gets the string (const)
     *
     */
    const char * get_string() const
    {
        return reinterpret_cast<const char*>(this->get_buffer());
    }

    /**
     * @brief Returns the number of characters
     *
     * This number doesn't include the null terminating character.
     */
    unsigned int element_count() const
    {
        RTI_FLAT_CHECK_PRECONDITION(
                PrimitiveSequenceOffset<char>::element_count() > 0,
                return 0);

        return PrimitiveSequenceOffset<char>::element_count() - 1;
    }
};

struct StringOffsetHelper {
    static offset_t calculate_serialized_size(
            rti::flat::SampleBase *sample,
            offset_t absolute_offset,
            offset_t max_size)
    {
        StringOffset tmp(sample, absolute_offset, max_size);
        return tmp.element_count()
            + 1 // null terminator
            + (unsigned int) sizeof(rti::xcdr::length_t); // length header
    }
};

// Wide strings are treated as a sequence of octets
typedef PrimitiveSequenceOffset<unsigned char> WStringOffset;

// The base class of SequenceOffset, and FinalAlignedArrayOffset
//
// This encapsulates the functionality that allows iterating through a list
// of elements (a sequence or an array). The list must be always aligned to 4
// (that excludes FinalArrayOffset)
//
/**
 * @ingroup RTIFlatOffsetsModule
 * @brief Base class of Offsets to sequences and arrays of non-primitive members
 *
 * @tparam ElementOffset The Offset type of the elements
 */
template <typename ElementOffset>
class AbstractAlignedList : public OffsetBase {
public:

    /**
     * @brief The iterator type, SequenceIterator
     */
    typedef SequenceIterator<ElementOffset, typename ElementOffset::offset_kind>
            iterator;

protected:
    AbstractAlignedList()
    {
    }

    AbstractAlignedList(
            rti::flat::SampleBase *sample,
            offset_t offset,
            offset_t sequence_size)
        : OffsetBase(
                sample,
                offset,
                sequence_size)
    {
    }

public:
    bool is_cpp_compatible() const // override
    {
        RTI_FLAT_OFFSET_CHECK_NOT_NULL(return false);
        return !stream_.needs_byte_swap()
                && rti::xcdr::has_cpp_friendly_cdr_layout_collection<
                        typename rti::flat::flat_type_traits<ElementOffset>::flat_type>();
    }

    ElementOffset get_element(unsigned int i)
    {
        RTI_FLAT_OFFSET_CHECK_NOT_NULL(return ElementOffset());

        return get_element_impl<ElementOffset>(
                i,
                typename ElementOffset::offset_kind());
    }

    /**
     * @brief Gets an iterator to the first Offset
     *
     * begin() and end() enable the use of range-for loops, for example:
     * \code
     * SequenceOffset<MyFlatMutableOffset> sequence_offset = my_type_offset.my_sequence();
     * for (auto element : sequence_offset) {
     *     std::cout << element.x() << std::endl;
     * }
     * \endcode
     *
     */
    iterator begin()
    {
        RTI_FLAT_OFFSET_CHECK_NOT_NULL(return iterator(NULL, 0, 0));

        return iterator(
                sample_,
                absolute_offset_,
                absolute_offset_ + get_buffer_size());
    }

    /**
     * @brief Gets an iterator to the past-the-end element
     */
    iterator end()
    {
        RTI_FLAT_OFFSET_CHECK_NOT_NULL(return iterator(NULL, 0, 0));

        return iterator(
                sample_,
                absolute_offset_ + get_buffer_size(),
                absolute_offset_ + get_buffer_size());
    }

private:
    // for variable size types we skip element by element
    template <typename E>
    E get_element_impl(unsigned int i, variable_size_type_tag_t)
    {
        iterator it = begin();
        while (it != end() && i > 0) {
            if (!it.advance()) {
                return E();
            }
            i--;
        }

        return *it;
    }

    // for fixed-size types we support random access
    template <typename E>
    E get_element_impl(unsigned int i, fixed_size_type_tag_t)
    {
        offset_t size = i * E::serialized_size_w_padding();

        // Ensure that the stream has space for i + 1 elements
        if (!stream_.check_size(size + E::serialized_size(0))) {
            return E();
        }

        return E(this->sample_, this->absolute_offset_ + size);
    }
};

template <typename ElementOffset>
struct SequenceOffsetHelper;

/**
 * @ingroup RTIFlatOffsetsModule
 * @brief Offset to a sequence of non-primitive elements
 *
 * @tparam ElementOffset The Offset type, for example MyFlatMutableOffset,
 * MyFlatFinalOffset, or StringOffset.
 *
 * Represents an Offset to a sequence member and allows getting an Offset to
 * each of its elements.
 *
 * A SequenceOffset may meet the requirements to be cast to an array of
 * the equivalent plain C++ element type (see rti::flat::plain_cast), if the
 * \p ElementOffset is a final type.
 */
template <typename ElementOffset>
class SequenceOffset : public AbstractAlignedList<ElementOffset> {
public:
    typedef variable_size_type_tag_t offset_kind;
    typedef SequenceOffsetHelper<ElementOffset> Helper;

    SequenceOffset() : element_count_(0)
    {
    }

    SequenceOffset(
            rti::flat::SampleBase *sample,
            offset_t offset,
            offset_t sequence_size)
            : AbstractAlignedList<ElementOffset>(
                    // Member size in bytes must be > 4, otherwise let base ctor
                    // fail
                    sequence_size >= header_length() ? sample : NULL,
                    // The offset begins after any padding to align to 4;
                    // padding may be needed for example for an array of
                    // sequence of final types; the end position of the previous
                    // sequence may not be aligned
                    RTIXCdrAlignment_alignSizeUp(
                            offset,
                            RTI_XCDR_SEQ_LENGTH_ALIGNMENT)
                            + header_length(),
                    sequence_size - header_length()),
              element_count_(0)
    {
#ifdef RTI_FLAT_DATA_NO_EXCEPTIONS
        if (!this->is_null()) { // without exceptions, we need to check if the base ctor failed
#endif
        rti::xcdr::Stream::Memento stream_memento(this->stream_);
        this->stream_.skip_back(sizeof(rti::xcdr::length_t));
        element_count_ = this->stream_.template deserialize_fast<rti::xcdr::length_t>();
#ifdef RTI_FLAT_DATA_NO_EXCEPTIONS
        }
#endif
    }

    /**
     * @brief Gets the Offset to an element
     *
     * @param i The zero-based index to the element
     * @return The Offset to the element in the i-th position
     */
    ElementOffset get_element(unsigned int i)
    {
        RTI_FLAT_OFFSET_CHECK_NOT_NULL(return ElementOffset());

        if (i >= element_count_) {
            return ElementOffset();
        }
        return AbstractAlignedList<ElementOffset>::get_element(i);
    }

    /**
     * @brief The number of elements
     */
    unsigned int element_count() const
    {
        RTI_FLAT_OFFSET_CHECK_NOT_NULL(return 0);

        return element_count_;
    }

private:
    static offset_t header_length()
    {
        return static_cast<offset_t>(
                detail::collection_dheader_traits<false>::dheader_size()
                + sizeof(rti::xcdr::length_t));
    }


    unsigned int element_count_;
};

template <typename E>
struct SequenceOffsetHelper {
    // Calculates the serialized size of a Sequence by skipping each element
    static offset_t calculate_serialized_size(
            rti::flat::SampleBase *sample,
            offset_t absolute_offset,
            offset_t max_offset)
    {
        // Create a SequenceOffset beginning at sample + absolute_offset,
        // with a size we don't know, but no greater than
        // max_offset - absolute_offset
        SequenceOffset<E> tmp(
                sample,
                absolute_offset,
                max_offset - absolute_offset);
        unsigned int count = tmp.element_count();
        typename SequenceOffset<E>::iterator it = tmp.begin();
        for (unsigned int i = 0; i < count; i++) {
            if (!it.advance()) {
                return 0; // error
            }
        }

        return detail::ptrdiff(it.get_position(), sample->get_buffer())
                - absolute_offset;
    }
};

/**
 * @ingroup RTIFlatOffsetsModule
 * @brief Offset to an array of variable-size elements
 *
 * @tparam ElementOffset An Offset for a type of variable size, such as a
 * mutable struct (MyFlatMutableOffset), union, or StringOffset.
 * @tparam N The array bound. For multidimensional arrays, \p N is the product
 * of each dimension bound.
 *
 * Represents an Offset to an array member and allows getting an Offset to
 * each of its elements.
 *
 * @see FinalArrayOffset encapsulates arrays of fixed-size elements
 */
template <typename ElementOffset, unsigned int N>
class MutableArrayOffset : public AbstractAlignedList<ElementOffset> {
public:
    typedef variable_size_type_tag_t offset_kind;

    MutableArrayOffset()
    {
    }

    MutableArrayOffset(
            rti::flat::SampleBase *sample,
            offset_t offset,
            offset_t sequence_size)
            : AbstractAlignedList<ElementOffset>(
                    sequence_size >= header_length() ? sample : NULL,
                    offset + header_length(),
                    sequence_size - header_length())
    {
    }

    /**
     * @brief Gets the Offset to an element
     *
     * @param i The zero-based index to the element
     * @return The Offset to the element in the i-th position
     */
    ElementOffset get_element(unsigned int i)
    {
        if (i >= N) {
            return ElementOffset();
        }

        return AbstractAlignedList<ElementOffset>::get_element(i);
    }

private:
    static offset_t header_length()
    {
        return static_cast<offset_t>(
                detail::collection_dheader_traits<false>::dheader_size());
    }
};

/**
 * @ingroup RTIFlatOffsetsModule
 * @brief Offset to an array of final elements
 *
 * @tparam ElementOffset A final struct Offset, such as MyFlatFinalOffset.
 * @tparam N The array bound. For multidimensional arrays, \p N is the product
 * of each dimension bound.
 *
 * Represents an Offset to an array member and allows getting an Offset to
 * each of its elements.
 *
 * FinalArrayOffset and FinalAlignedArrayOffset provide the same
 * interface, but have different implementation details. FinalArrayOffset is
 * used when the array member is part of a final type too, whereas
 * FinalAlignedArrayOffset corresponds to an array inside a mutable type.
 *
 * A FinalArrayOffset may meet the requirements to be cast to an array of
 * the equivalent plain C++ element type (see rti::flat::plain_cast()).
 */
template <typename ElementOffset, unsigned int N>
class FinalArrayOffset : public OffsetBase {
public:
    typedef fixed_size_type_tag_t offset_kind;

    FinalArrayOffset()
    {
    }

    // Constructor used when this array is part of a fixed-size type, and
    // therefore it's initial alignment is not known ahead of time
    FinalArrayOffset(
        rti::flat::SampleBase *sample,
        offset_t absolute_offset,
        offset_t first_element_size,
        offset_t element_size)
        : OffsetBase(
                sample,
                absolute_offset,
                first_element_size + element_size * (N - 1)),
          first_element_size_(first_element_size),
          element_size_(element_size)
    {
    }

    /**
     * @brief Gets the Offset to an element
     *
     * @param i The zero-based index to the element
     * @return The Offset to the element in the i-th position
     */
    ElementOffset get_element(unsigned int i)
    {
        RTI_FLAT_OFFSET_CHECK_NOT_NULL(return ElementOffset());

        if (i >= N) {
            return ElementOffset(); // null offset
        }

        offset_t element_offset = this->absolute_offset_;
        if (i > 0) {
            element_offset += this->first_element_size_
                + (i - 1) * this->element_size_;
        }

        return ElementOffset(this->sample_, element_offset);
    }

private:
    offset_t first_element_size_;
    offset_t element_size_;
};

/**
 * @ingroup RTIFlatOffsetsModule
 * @brief Offset to an array of final elements
 *
 * @tparam ElementOffset A final struct Offset, such as MyFlatFinalOffset.
 * @tparam N The array bound. For multidimensional arrays, \p N is the product
 * of each dimension bound.
 *
 * Represents an Offset to an array member and allows getting an Offset to
 * each of its elements.
 *
 * The following code shows how to use a FinalAlignedArrayOffset to initialize
 * an array member with MyFlatMutableBuilder:
 *
 * \code
 *  MyFlatMutableBuilder builder = rti::flat::build_data(writer);
 *  auto array_offset = builder.add_my_final_array();
 *  for (MyFlatFinalOffset element_offset : array_offset) {
 *      element_offset.my_primitive(3);
 *      // ...
 *  }
 *
 *  // Or access an element directly:
 *  auto element_offset = array_offset.get_element(3);
 *  element_offset.my_primitive(3);
 * \endcode
 *
 * A more efficient way to access a final array, provided it complies with
 * the required preconditions, is through rti::flat::plain_cast().
 *
 * FinalArrayOffset and FinalAlignedArrayOffset provide the same
 * interface, but have different implementation details. FinalArrayOffset is
 * used when the array member is part of a final type too, whereas
 * FinalAlignedArrayOffset corresponds to an array inside a mutable type.
 *
 * A FinalAlignedArrayOffset may meet the requirements to be cast to an array of
 * the equivalent plain C++ element type (see rti::flat::plain_cast()).
 *
 * @see MutableArrayOffset encapsulates arrays of variable-size elements
 */
template <typename ElementOffset, unsigned int N>
class FinalAlignedArrayOffset : public AbstractAlignedList<ElementOffset> {
public:
    typedef fixed_size_type_tag_t offset_kind;

    FinalAlignedArrayOffset()
    {
    }

    // Constructor used when this array is part of a mutable type, and
    // therefore it's aligned to 4 always
    FinalAlignedArrayOffset(
            rti::flat::SampleBase *sample,
            offset_t absolute_offset)
            : AbstractAlignedList<ElementOffset>(
                    sample,
                    absolute_offset
                            + detail::collection_dheader_traits<
                                    false>::dheader_size(),
                    serialized_size(0))
    { }

    // serialized_size() only works when this array is part of a mutable type
    // and we know it's aligned to 4
    static offset_t serialized_size(offset_t)
    {
        // [element1][pad][element2][pad]...[elementN]
        return ElementOffset::serialized_size_w_padding() * (N - 1)
                + ElementOffset::serialized_size(0);
    }

    /**
     * @brief Gets the Offset to an element
     *
     * @param i The zero-based index to the element
     * @return The Offset to the element in the i-th position
     *
     * @see rti::flat::plain_cast() for a method to access all the elements at
     * once
     */
    ElementOffset get_element(unsigned int i)
    {
        RTI_FLAT_OFFSET_CHECK_NOT_NULL(return ElementOffset());

        if (i >= N) {
            return ElementOffset();
        }

        return AbstractAlignedList<ElementOffset>::get_element(i);
    }
};

namespace detail {

// Arrays of complex elements may need a DHEADER
template <typename T, unsigned int N>
struct is_fixed_type_w_dheader<FinalAlignedArrayOffset<T, N> > {
    static bool value()
    {
        return is_dheader_required_in_non_primitive_collections();
    }
};


} // namespace detail

}}  // namespace rti::flat

#endif // RTI_DDS_FLAT_SEQUENCEOFFSETS_HPP_

