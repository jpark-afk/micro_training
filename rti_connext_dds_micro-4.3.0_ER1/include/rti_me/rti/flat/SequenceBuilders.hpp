/*
(c) Copyright, Real-Time Innovations, 2018-2024.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
*/

#ifndef RTI_DDS_FLAT_SEQUENCEBUILDERS_HPP_
#define RTI_DDS_FLAT_SEQUENCEBUILDERS_HPP_

#include "rti/flat/Builder.hpp"

//
// Builders for sequences, strings, and arrays.
//
// Type hierarchy (- indicates an abstract builder; * a concrete, usable builder)
//
// - AbstractListBuilder
//     * MutableArrayBuilder
//     - AbstractSequenceBuilder 
//         * MutableSequenceBuilder
//         * FinalSequenceBuilder
//         * PrimitiveSequenceBuilder
//             * StringBuilder
//
// Notes: 
//   - there's no "FinalArrayBuilder" because that's a fixed-size type and
//     doesn't need a builder (it's constructed via an "add" operation, not a "build"
//     operation).
//   - MutableSequenceBuilder "builds" its elements
//   - FinalSequenceBuilder "adds" its elements
//

namespace rti { namespace flat {

namespace detail {

template <bool IsForEnum>
struct DHeaderGeneratorImpl {
    static bool reserve_dheader(rti::xcdr::Stream& stream)
    {
        if (collection_dheader_traits<IsForEnum>::dheader_required()) {
            return stream.serialize_dheader() != NULL;
        } else {
            return true;
        }
    }

    static bool finish_dheader(
            rti::xcdr::Stream& stream,
            unsigned char *dheader_position)
    {
        if (collection_dheader_traits<IsForEnum>::dheader_required()) {
            return stream.finish_dheader((char *) dheader_position);
        } else {
            return true;
        }
    }

    static bool move_after_dheader(
            rti::xcdr::Stream& stream,
            unsigned char *begin_position)
    {
        stream.current_position(begin_position);

        if (collection_dheader_traits<IsForEnum>::dheader_required()) {
            return stream.skip(RTI_XCDR_DHEADER_SIZE);
        } else {
            return true;
        }
    }
};

struct NoDHeaderGenerator {
    static bool reserve_dheader(rti::xcdr::Stream&)
    {
        return true;
    }

    static bool finish_dheader(rti::xcdr::Stream&, unsigned char *)
    {
        return true;
    }

    static bool move_after_dheader(
            rti::xcdr::Stream& stream,
            unsigned char *begin_position)
    {
        stream.current_position(begin_position);
        return true;
    }
};

typedef DHeaderGeneratorImpl<false> DHeaderGenerator;
typedef DHeaderGeneratorImpl<true> DHeaderGeneratorForEnum;

// Maybe specialized for enums to select DHeaderGenerator
template <typename T, typename Enable = void>
struct primitive_sequence_dheader_gen {
    typedef NoDHeaderGenerator type;
};

} // namespace detail

/*i
    * Extends and specializes AbstractBuilder to add or override the
    * functionality required to build a collection of consecutive elements
    * (sequence or array):
    *
    * - Add or build elements
    * - Keep track of the element count
    * - Handle element alignment
    *
    */
/**
    * @ingroup RTIFlatBuildersModule
    * @brief Base class of all array and sequence builders
    */
template <typename DHeaderGen>
class AbstractListBuilder : public AbstractBuilder {
protected:
    AbstractListBuilder() : element_count_(0)
    { }

    AbstractListBuilder(
            nested_tag_t,
            AbstractBuilder& parent,
            unsigned int alignment)
            : AbstractBuilder(nested_tag_t(), parent, alignment),
                element_count_(0)
    {
        DHeaderGen::reserve_dheader(stream());
    }

    template <typename ElementBuilder>
    ElementBuilder build_next()
    {
        rti::xcdr::Stream::Memento stream_memento(stream());
        return build_element<ElementBuilder>(stream_memento);
    }

    template <typename ElementOffset>
    ElementOffset add_next()
    {
        RTI_FLAT_BUILDER_CHECK_VALID(return ElementOffset());

        // save state in case of error
        rti::xcdr::Stream::Memento stream_memento(stream());

        if (!stream().align(ElementOffset::required_alignment)) {
            RTI_FLAT_BUILDER_OUT_OF_RESOURCES_ERROR(return ElementOffset());
        }

        ElementOffset element = add_element<ElementOffset>();
        if (element.is_null()) {
            return ElementOffset();
        }

        // Success: clear memento, increment element count
        stream_memento.discard();
        element_count_++;

        return element;
    }

    template <typename ElementOffset>
    void add_n(unsigned int count)
    {
        RTI_FLAT_BUILDER_CHECK_VALID(return);

        if (count == 0) {
            return;
        }

        if (!stream().align(ElementOffset::required_alignment)) {
            RTI_FLAT_BUILDER_OUT_OF_RESOURCES_ERROR(return);
        }

        offset_t elements_size = ElementOffset::serialized_size(0)
                + ElementOffset::serialized_size_w_padding() * (count - 1);
        if (!stream().skip(elements_size)) {
            RTI_FLAT_BUILDER_OUT_OF_RESOURCES_ERROR(return);
        }

        element_count_ += count;
    }

    /**
     * @brief Returns the current number of elements that have been added
     */
    unsigned int element_count() const
    {
        return element_count_;
    }

#if defined(RTI_FLAT_DATA_CXX11_RVALUE_REFERENCES)
    AbstractListBuilder(AbstractListBuilder&& other) = default;
    AbstractListBuilder& operator=(AbstractListBuilder&& other)
    {
        if (this == &other) {
            return *this;
        }

        finish_untyped_impl();

        AbstractBuilder::operator=(static_cast<AbstractBuilder&&>(other));

        element_count_ = other.element_count_;
        other.element_count_ = 0;

        return *this;
    }
  #else
public:
    // Enables the safe-move-constructor idiom without C++11 move constructors
    struct AbstractListBuilderMoveProxy : AbstractBuilderMoveProxy {
        unsigned int element_count_;
    };

    operator AbstractListBuilderMoveProxy () throw() // move-constructor idiom
    {
        AbstractListBuilderMoveProxy other;
        move_to(other);
        return other;
    } 

protected:
    void move_from(AbstractListBuilderMoveProxy& other)
    {
        AbstractBuilder::move_from(other);
        element_count_ = other.element_count_;
    }

    void move_to(AbstractListBuilderMoveProxy& other)
    {
        AbstractBuilder::move_to(other);
        other.element_count_ = element_count_;
    }

#endif

private:
    friend class AbstractBuilder;

    virtual void finish_member() // override
    {
        RTI_FLAT_BUILDER_CHECK_VALID(return);

        // Increment element count after the nested element  builder finishes,
        // instead of on build_next(). This makes it easier to roll back the
        // state if the element building doesn't complete.
        element_count_++;
        AbstractBuilder::finish_member();
    }

protected:
    bool finish_dheader()
    {
        return DHeaderGen::finish_dheader(stream(), begin_position());
    }

    unsigned int element_count_;
};

// --- Array: -----------------------------------------------------------------
//
// (Note: there is no FinalArrayBuilder, because a final array's size is fixed,
// and is handled directly returning its offset with add_*; on the other hand,
// sequences are never fixed-size.)

/**
 * @ingroup RTIFlatBuildersModule
 * @brief Builds an array member of variable-size elements
 * 
 * @tparam ElementBuilder The Builder type for the elements of the array
 * @tparam N The array bound; the exact number of elements this array builder
 * must build. For multidimensional arrays, \p N is the product
 * of each dimension bound.
 * 
 * Each element of this array needs to be built using the ElementBuilder returned
 * by build_next(). N elements exactly must be built. Unlike a sequence Builder,
 * it is not possible to finish an array with less than N elements. 
 * 
 * This example shows how to use a MutableArrayBuilder to build an array member
 * of MyFlatMutableBuilder:
 * 
 * \code
 *   MyFlatMutableBuilder builder = rti::flat::build_data(writer);
 *   auto array_builder = builder.build_my_mutable_array();
 *   for (int i = 0; i < 10; i++) {
 *       FlatMutableBarBuilder element_builder = array_builder.build_next();
 *       // ... build element
 *       element_builder.finish();
 *   }
 *   array_builder.finish();
 * \endcode
 * 
 * Note that Builder types are not necessary for arrays of fixed-size elements,
 * since they are added at once (see MyFlatMutableBuilder::add_my_final_array()).
 */
template <typename ElementBuilder, unsigned int N>
class MutableArrayBuilder : public AbstractListBuilder<detail::DHeaderGenerator> {
public:
    /**
     * @brief The related Offset type
     */
    typedef MutableArrayOffset<typename ElementBuilder::Offset, N> Offset;
    typedef AbstractListBuilder<detail::DHeaderGenerator> Base;

    MutableArrayBuilder()
    { }

    // The destructor does the same as finish_impl() but doesn't need to return
    // an Offset
    ~MutableArrayBuilder()
    {
        finish_untyped_impl();
    }

#if defined(RTI_FLAT_DATA_CXX11_RVALUE_REFERENCES)
    MutableArrayBuilder(MutableArrayBuilder&& other) = default;

    MutableArrayBuilder& operator=(MutableArrayBuilder&& other)
    {
        finish_untyped_impl();
        Base::operator=(static_cast<Base&&>(other));

        return *this;
    }
#endif

    void finish_untyped_impl()
    {
        if (!Base::is_valid() || !Base::is_nested()) {
            return;
        }

        Base::finish_dheader();
        Base::finish_untyped_impl();
    }

private:
    friend class AbstractBuilder; // to allow access to the constructor

    MutableArrayBuilder(
            nested_tag_t, 
            AbstractBuilder& parent,
            unsigned int alignment = 0)
        : AbstractListBuilder(nested_tag_t(), parent, alignment)
    {
    }

public:
    /**
     * @brief Begins building the next element
     * 
     * Before calling build_next() to create a new element, the element Builder
     * returned by a previous call to build_next must have been finished.
     */
    ElementBuilder build_next()
    {
        if (element_count() == N) {
            RTI_FLAT_BUILDER_PRECONDITION_ERROR(
                "Array builder build_next: too many elements",
                return ElementBuilder());
        }

        return AbstractListBuilder::build_next<ElementBuilder>();
    }

    /**
     * @brief Finishes building the array
     * 
     * @pre build_next() must have been succesfully called \p N times exactly.
     * @return The Offset to the member that has been built.
     * @see discard()
     */
    Offset finish()
    {
        RTI_FLAT_BUILDER_CHECK_CAN_FINISH(return Offset());
        
        if (element_count() != N) {
            RTI_FLAT_BUILDER_PRECONDITION_ERROR(
                "Cannot finish array builder: too few elements",
                return Offset());                     
        }

        if (!finish_dheader()) {
            RTI_FLAT_BUILDER_OUT_OF_RESOURCES_ERROR(return Offset());
        }

        return finish_impl<Offset>();
    }

    RTI_FLAT_BUILDER_DEFINE_MOVE_OPERATIONS_IMPL(\
            MutableArrayBuilder, AbstractListBuilder, AbstractListBuilderMoveProxy)
    
};

// --- Sequences: -------------------------------------------------------------

// Specializes AbstractListBuilder to handle the length header serialization
// common to all sequences (which arrays don't have)
/**
 * @ingroup RTIFlatBuildersModule
 * @brief Base class of Builders for sequence members.
 *
 * This class contains only implementation details and doesn't add any public
 * function to AbstractListBuilder.
 */
template <typename DHeaderGen>
class AbstractSequenceBuilder : public AbstractListBuilder<DHeaderGen> {
public:
    typedef AbstractListBuilder<DHeaderGen> Base;
    using Base::element_count;
    using Base::is_nested;
    using Base::is_valid;
    using Base::stream;
    using typename Base::nested_tag_t;
  #if !defined(RTI_FLAT_DATA_CXX11_RVALUE_REFERENCES)
    using typename Base::AbstractListBuilderMoveProxy;
  #endif

protected:

    AbstractSequenceBuilder()
    {
    }

    AbstractSequenceBuilder(
            nested_tag_t,
            AbstractBuilder& parent,
            unsigned int alignment)
            : Base(nested_tag_t(), parent, alignment)
    {
        // leave space for sequence length
        if (!stream().check_size(sizeof(rti::xcdr::length_t))) {
            RTI_FLAT_BUILDER_OUT_OF_RESOURCES_ERROR(this->invalidate());
        } else {
            // No need to align; the base constructor already aligned to 4 the
            // begin_position
            stream().template serialize_fast<rti::xcdr::length_t>(0);
        }
    }

    // The destructor does the same as finish_impl() but doesn't need to return
    // an Offset
    ~AbstractSequenceBuilder()
    {
        finish_untyped_impl();
    }

#if defined(RTI_FLAT_DATA_CXX11_RVALUE_REFERENCES)
    AbstractSequenceBuilder(AbstractSequenceBuilder&& other) = default;
    AbstractSequenceBuilder& operator=(AbstractSequenceBuilder&& other)
    {
        finish_untyped_impl();
        Base::operator=(static_cast<Base&&>(other));

        return *this;
    }
#endif

    void finish_untyped_impl()
    {
        if (!Base::is_valid() || !Base::is_nested()) {
            return;
        }

        Base::finish_dheader();
        finish_length();
        Base::finish_untyped_impl();
    }

    // Concrete sequences must call this method in their own finish() function
    template <typename Offset>
    Offset finish_impl()
    {
        RTI_FLAT_BUILDER_CHECK_VALID(return Offset());
        RTI_FLAT_BUILDER_CHECK_CAN_FINISH(return Offset());

        Base::finish_dheader();
        finish_length(); // serialize the length header
        return Base::template finish_impl<Offset>();
    }

private:
    // Serializes the sequence length when finishing the builder.
    void finish_length()
    {
        rti::xcdr::Stream::Memento stream_memento(stream());
        DHeaderGen::move_after_dheader(stream(), Base::begin_position());

        // Coverity found examples where element_count() is checked, but
        // that's only done in Array builders, which are unrelated to this
        // class, AbstractSequenceBuilder.
        // coverity[CHECKED_RETURN : FALSE]
        stream().template serialize_fast<rti::xcdr::length_t>(element_count());
    }
};

/**
 * @ingroup RTIFlatBuildersModule
 * @brief Builds a sequence member of variable-size elements
 * 
 * @tparam ElementBuilder The Builder type for the elements of the sequence
 * 
 * To build the elements use the ElementBuilder returned
 * by build_next(). An empty sequence can be built by calling finish() without
 * any call to build_next().
 * 
 * This class doesn't enforce the sequence bound set in IDL.
 * 
 * The following example uses a MutableSequenceBuilder to initialize a sequence
 * member of MyFlatMutableBuilder with two elements:
 * 
 * \code
 *  MyFlatMutableBuilder builder = rti::flat::build_data(writer);
 *  auto seq_builder = builder.build_my_mutable_seq();
 *  auto element_builder = seq_builder.build_next();
 *  // ... build the first element
 *  element_builder.finish();
 *  element_builder = seq_builder.build_next();
 *  // ... build the second element
 *  element_builder.finish();
 *   
 *  seq_builder.finish();
 * \endcode 
 */
template <typename ElementBuilder>
class MutableSequenceBuilder
        : public AbstractSequenceBuilder<detail::DHeaderGenerator> {
public:
    /**
     * @brief The related Offset type
     */
    typedef SequenceOffset<typename ElementBuilder::Offset> Offset;

    MutableSequenceBuilder()
    {
    }

private:
    typedef AbstractSequenceBuilder<detail::DHeaderGenerator> Base;

    friend class AbstractBuilder; // to allow access to the constructor

    MutableSequenceBuilder(
            nested_tag_t, 
            AbstractBuilder& parent,
            unsigned int alignment = RTI_XCDR_DHEADER_ALIGNMENT)
        : Base(nested_tag_t(), parent, alignment)
    {
    }

public:
    /**
     * @brief Begins building the next element
     * 
     * Before calling build_next() to create a new element, the application must
     * have called finish() on the previous element Builder.
     */
    ElementBuilder build_next()
    {
        return AbstractListBuilder::build_next<ElementBuilder>();
    }

    /**
     * @brief Finishes building the sequence
     * 
     * @return An Offset to the member that has been built.
     * 
     * @see discard()
     */
    Offset finish()
    {
        return finish_impl<Offset>();
    }

    RTI_FLAT_BUILDER_DEFINE_MOVE_OPERATIONS_IMPL(
            MutableSequenceBuilder,
            Base,
            AbstractListBuilderMoveProxy)
};

/**
 * @ingroup RTIFlatBuildersModule
 * @brief Builds a sequence member of fixed-size elements
 * 
 * @tparam ElementOffset The Offset type for the elements of the sequence
 * 
 * To add an element, call add_next() and use the ElementOffset it returns to
 * initialize the element's values. An empty sequence can be built by 
 * calling finish() without any call to add_next().
 * 
 * This class doesn't enforce the sequence bound set in IDL.
 * 
 * The following example uses a FinalSequenceBuilder to initialize a sequence
 * member of MyFlatMutableBuilder with two elements:
 * 
 * \code
 *  MyFlatMutableBuilder builder = rti::flat::build_data(writer);
 *  auto seq_builder = builder.build_my_final_seq();
 *  MyFlatFinalOffset element = seq_builder.add_next();
 *  element.my_primitive(1);
 *  // ... continue initializing the first element
 *  
 *  element = seq_builder.add_next();
 *  element.my_primitive(2);
 *  // ... continue initializing the second element
 *   
 *  seq_builder.finish();
 * \endcode
 * 
 * If the element type meets certain requirements, rti::flat::plain_cast()
 * provides a more efficient way to initialize a sequence of final elements.
 */
template <typename ElementOffset>
class FinalSequenceBuilder
        : public AbstractSequenceBuilder<detail::DHeaderGenerator> {
public:
    typedef SequenceOffset<ElementOffset> Offset;

    FinalSequenceBuilder()
    {
    }

private:
    friend class AbstractBuilder; // to allow access to the constructor

    FinalSequenceBuilder(
            nested_tag_t, 
            AbstractBuilder& parent,
            unsigned int alignment = RTI_XCDR_SEQ_LENGTH_ALIGNMENT)
        : AbstractSequenceBuilder(nested_tag_t(), parent, alignment)
    {
    }

public:
    /**
     * @brief Adds the next element
     * 
     * @return The Offset that can be used to set the element values
     */
    ElementOffset add_next()
    {
        return AbstractListBuilder::add_next<ElementOffset>();
    }

    /**
     * @brief Adds a number of elements at once
     * 
     * This is an alternative to add_next().
     * 
     * To initialize the elements, call finish() and use the Offset it returns
     * to access the elements.
     */
    FinalSequenceBuilder& add_n(unsigned int count)
    {
        AbstractListBuilder::add_n<ElementOffset>(count);
        return *this;
    }

    /**
     * @brief Finishes building the sequence
     * 
     * @return An Offset to the member that has been built.
     * @see discard()
     */
    Offset finish()
    {
        return finish_impl<Offset>();
    }

    RTI_FLAT_BUILDER_DEFINE_MOVE_OPERATIONS_IMPL(\
            FinalSequenceBuilder, AbstractSequenceBuilder, AbstractListBuilderMoveProxy)
};

/**
 * @ingroup RTIFlatBuildersModule
 * @brief Builds a sequence of primitive members
 * 
 * The elements can be added one by one with add_next() or all at once with
 * add_n().
 */
template <typename T>
class PrimitiveSequenceBuilder
        : public AbstractSequenceBuilder<
                  typename detail::primitive_sequence_dheader_gen<T>::type> {
public:
    typedef PrimitiveSequenceOffset<T> Offset;
    typedef AbstractSequenceBuilder<
            typename detail::primitive_sequence_dheader_gen<T>::type>
            Base;
    using Base::is_valid;
    using Base::stream;
    using typename Base::nested_tag_t;
#if !defined(RTI_FLAT_DATA_CXX11_RVALUE_REFERENCES)
    using typename Base::AbstractListBuilderMoveProxy;
#endif

    PrimitiveSequenceBuilder()
    {
    }

protected:
    friend class AbstractBuilder;  // to allow access to the constructor

    PrimitiveSequenceBuilder(
            nested_tag_t,
            AbstractBuilder& parent,
            unsigned int alignment = RTI_XCDR_SEQ_LENGTH_ALIGNMENT)
            : Base(nested_tag_t(), parent, alignment)
    {
    }

public:
    /**
     * @brief Adds the next element
     * 
     * @param value The primitive element to add
     */
    PrimitiveSequenceBuilder& add_next(T value)
    {
        RTI_FLAT_BUILDER_CHECK_VALID(return *this);

        if (!stream().template serialize<T>(value)) {
            RTI_FLAT_BUILDER_OUT_OF_RESOURCES_ERROR(return *this);
        }
        
        Base::element_count_++;
        return *this;
    }

    /**
     * @brief Adds all the elements in an array
     *
     * @param array The array containing the values to add
     * @param count The size of the array
     *
     * @pre array must not be NULL
     */
    PrimitiveSequenceBuilder& add_n(const T *array, unsigned int count)
    {
        RTI_FLAT_BUILDER_CHECK_VALID(return *this);
        RTI_FLAT_ASSERT(array != NULL, return *this);

        if (RTIXCdrUnsignedLong_MAX / static_cast<unsigned int>(sizeof(T))
                < count) {
            RTI_FLAT_BUILDER_OUT_OF_RESOURCES_ERROR(return *this);
        }

        if (!stream().check_size(static_cast<unsigned int>(sizeof(T)) * count)) {
            RTI_FLAT_BUILDER_OUT_OF_RESOURCES_ERROR(return *this);
        }

        if (stream().needs_byte_swap() && sizeof(T) > 1) {
            for (unsigned int i = 0; i < count; i++) {
                stream().template serialize_fast<T>(array[i]);
            }
        } else {
            stream().serialize_fast((void *) array, count * static_cast<unsigned int>(sizeof(T)));
        }

        Base::element_count_ += count;
        return *this;
    }

    /**
     * @brief Adds a number of elements with the same value
     * 
     * This overload initializes all the elements to a value.
     * 
     * @note This operation is O(\p count). The other overloads, 
     * add_n(unsigned int) and add_n(const T*, unsigned int), are more 
     * efficient 
     * 
     * 
     * @param count The number of elements to add
     * @param value The value to set for each element
     */
    PrimitiveSequenceBuilder& add_n(unsigned int count, T value)
    {
        RTI_FLAT_BUILDER_CHECK_VALID(return *this);

        if (RTIXCdrUnsignedLong_MAX / static_cast<unsigned int>(sizeof(T))
                < count) {
            RTI_FLAT_BUILDER_OUT_OF_RESOURCES_ERROR(return *this);
        }

        if (!stream().check_size(static_cast<unsigned int>(sizeof(T)) * count)) {
            RTI_FLAT_BUILDER_OUT_OF_RESOURCES_ERROR(return *this);
        }

        for (unsigned int i = 0; i < count; i++) {
            stream().template serialize_fast<T>(value);
        }
        Base::element_count_ += count;
        return *this;
    }

    /**
     * @brief Adds a number of uninitialized elements
     * 
     * This operation is O(1), since it leaves the elements uninitialized.
     * They can be initialized using the Offset returned by finish() and
     * rti::flat::plain_cast(). For example:
     *
     * \code
     * MyFlatMutableBuilder builder = ...;
     * auto seq_builder = builder.build_my_primitive_seq();
     * seq_builder.add_n(1000);
     * auto seq_offset = seq_builder.finish();
     * int32_t *elements = rti::flat::plain_cast(seq_offset);
     * for (int i = 0; i < 1000; i++) {
     *     elements[i] = ...;
     * }
     * \endcode
     * 
     * @param count The number of elements to add
     */
    PrimitiveSequenceBuilder& add_n(unsigned int count)
    {
        RTI_FLAT_BUILDER_CHECK_VALID(return *this);

        if (RTIXCdrUnsignedLong_MAX / static_cast<unsigned int>(sizeof(T))
                < count) {
            RTI_FLAT_BUILDER_OUT_OF_RESOURCES_ERROR(return *this);
        }

        if (!stream().skip(static_cast<unsigned int>(sizeof(T)) * count)) {
            RTI_FLAT_BUILDER_OUT_OF_RESOURCES_ERROR(return *this);
        }

        Base::element_count_ += count;
        return *this;
    }


    /**
     * @brief Finishes building the sequence
     * 
     * @return An Offset to the member that has been built.
     * @see discard()
     */
    Offset finish()
    {
        this->finish_dheader();
        return Base::template finish_impl<Offset>();
    }

private:
    RTI_FLAT_BUILDER_DEFINE_MOVE_OPERATIONS_IMPL(\
            PrimitiveSequenceBuilder, Base, AbstractListBuilderMoveProxy)
};

/**
 * @ingroup RTIFlatBuildersModule
 * @brief Builds a string
 * 
 * A StringBuilder only provides one method, set_string(), so it can be 
 * typically used as follows:
 * 
 * \code
 * MyFlatMutableBuilder my_builder = ...;
 * my_builder.build_my_string().set_string("Hello!");
 * \endcode
 * 
 * Note that by relying on the builder destructor, there is no need to call
 * finish() on the object returned by build_my_string().
 */
class StringBuilder : public PrimitiveSequenceBuilder<char> {
public:
    typedef StringOffset Offset;
    typedef PrimitiveSequenceBuilder<char> Base;
    using Base::is_valid;
    using Base::stream;
    using typename Base::nested_tag_t;

    StringBuilder()
    {
    }
    
private:
    friend class AbstractBuilder; // to allow access to the constructor

    StringBuilder(
            nested_tag_t,
            AbstractBuilder& parent,
            unsigned int alignment = RTI_XCDR_SEQ_LENGTH_ALIGNMENT)
            : Base(nested_tag_t(), parent, alignment)
    {
    }

public:
    /**
     * @brief Sets the string value
     *
     * @param value The string value to set.
     *
     * @pre value must not be NULL.
     */
    StringBuilder& set_string(const char *value)
    {
        RTI_FLAT_BUILDER_CHECK_VALID(return *this);
        RTI_FLAT_ASSERT(value != NULL, return *this);

        // if set_string is called more than once we override the string
        // that was set before
        if (Base::element_count_ != 0) {
            stream().current_position(begin_position());
            stream().serialize_fast<rti::xcdr::length_t>(0);
            Base::element_count_ = 0;
        }

        unsigned int length =
                static_cast<unsigned int>(RTIXCdrString_getLength(value)) + 1;
        add_n(value, length);  // if this fail, error has been reported
        return *this;
    }

    /**
     * @brief Finishes building the string
     * 
     * @return An Offset to the member that has been built.
     * @see discard()
     */
    Offset finish()
    {
        RTI_FLAT_BUILDER_CHECK_VALID(return Offset());

        if (element_count_ == 0) {
            add_next('\0'); // build empty string if no string was built
        }

        Base::finish_dheader();
        return finish_impl<Offset>();
    }

    RTI_FLAT_BUILDER_DEFINE_MOVE_OPERATIONS_IMPL(\
            StringBuilder, Base, AbstractListBuilderMoveProxy)
};

// Wide strings are treated as a sequence of octets
typedef PrimitiveSequenceBuilder<unsigned char> WStringBuilder;

} }

#endif // RTI_DDS_FLAT_SEQUENCEBUILDERS_HPP_
