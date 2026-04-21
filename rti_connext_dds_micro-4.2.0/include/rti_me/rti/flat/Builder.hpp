/*
(c) Copyright, Real-Time Innovations, 2018.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
*/

#ifndef RTI_DDS_FLAT_BUILDER_HPP_
#define RTI_DDS_FLAT_BUILDER_HPP_

#include "xcdr/xcdr_stream.h"
#include "xcdr/xcdr_stream_impl.h"
#include "xcdr/xcdr_interpreter.h"
#include "xcdr/xcdr_interpreter.h"

#include "rti/xcdr/Stream.hpp"
#include "rti/flat/ExceptionHelper.hpp"
#include "rti/flat/SequenceOffsets.hpp"

/** 
 * @defgroup RTIFlatBuildersModule  FlatData Builders
 * @ingroup RTIFlatDataModule
 * @brief A Builder allows creating and initializing variable-size data
 * 
 * Builders allow creating variable-size \ref RTIFlatSampleModule "FlatData samples",
 * as described in \ref PublishingFlatData.
 * 
 * There are the following Builder types:
 * 
 * <table>
 * <tr>
 *   <th>Category</th>
 *   <th>Builder type</th>
 * </tr>
 * <tr>
 *   <td>User types</td>
 *   <td> For example: <ul>
 *     <li> MyFlatMutableBuilder, a mutable struct </li>
 *     <li> MyFlatUnionBuilder, a union </li>
 *     <li> Final structs are fixed-size and do not have a Builder (see for 
 *          example MyFlatMutableBuilder::add_my_final()).
 *   </ul></td>
 * </tr>
 * <tr>
 *   <td>Arrays</td>
 *   <td><ul>
 *     <li> rti::flat::MutableArrayBuilder </li>
 *     <li>Arrays of final elements are fixed-size and do not have a Builder
 *      (see for example MyFlatMutableBuilder::add_my_final_array()). </li>
 *   </ul></td>
 * </tr>
 * <tr>
 *   <td>Sequences</td>
 *   <td><ul>
 *      <li>rti::flat::MutableSequenceBuilder for sequences of mutable elements</li>
 *      <li>rti::flat::FinalSequenceBuilder for sequences of final elements</li>
 *      <li>rti::flat::PrimitiveSequenceBuilder for sequences of primitive elements</li>
 *      <li>rti::flat::StringBuilder</li>
 *   </ul></td>
 * </tr>
 * <tr>
 *   <td>Primitive types</td>
 *   <td><ul>
 *      <li>Primitive members are added using one of the member functions of the
 *      Builder of the type that contains them. See for example 
 *      MyFlatMutableBuilder::add_my_primitive()</li><
 *   </ul></td>
 * </tr> 
 * </table>
 * 
 * Builder for user types, such as MyFlatMutableBuilder, can behave as 
 * <b>sample builders</b>, when they build a sample, or <b>member builders</b> 
 * when they build a member for another Builder (such as the Builder returned
 * by MyFlatMutableBuilder::build_my_mutable()). All the other Builder types 
 * (sequences, arrays) are always member builders (such as the Builder returned
 * by MyFlatMutableBuilder::build_my_final_seq()).
 * 
 * Builders are <b>move-only</b> types. They cannot be copied, only moved. When
 * you assign a Builder as the return value of a function, this happens
 * automatically.
 * 
 * \code
 * // the return value of 'build_data' is moved into 'builder'
 * MyFlatMutableBuilder builder = rti::flat::build_data(writer);
 * \endcode
 * 
 * To explicitly move a Builder variable \if CPP_LANGUAGE_ONLY in
 * the Traditional C++ API, call its \p move() member function\else use \p std::move()
 * in C++11 platforms or call the \p move() member function in pre-C++11 platforms.
 * \endif
 * 
 * @section BuilderErrorManagement Builder Error Management
 * \if CPP_LANGUAGE_ONLY
 * The Traditional C++ API reports Builder errors by setting a flag
 * that can be checked with the member function \ref rti::flat::AbstractBuilder::check_failure()
 * "check_failure()". After each Builder operation, \p check_failure() must be
 * called to check (and clear) the error flag. 
 * 
 * In addition, in case of error, operations that return another Builder, such 
 * as MyFlatMutableBuilder::build_my_mutable(), return an invalid Builder
 * (\ref rti::flat::AbstractBuilder::is_valid() "is_valid()" is \p false). 
 * Operations that return an 
 * Offset, such as MyFlatMutableBuilder::add_my_final() return a "null" Offset 
 * when they fail (see \ref OffsetErrorManagement).
 * \endif
 * \if CPP2_LANGUAGE_ONLY
 * In the Modern C++ API errors are notified with exceptions. Builder operations
 * may throw the following exceptions
 * - dds::core::PreconditionNotMetError, for any precondition failure.
 * - dds::core::OutOfResourcesError, when the Builder's underlying buffer runs
 *   out of space (this should not happen when the Builder has been created with
 *   rti::flat::build_data()).
 * 
 * See also \ref OffsetErrorManagement.
 * \endif
 * 
 * @see MyFlatMutableBuilder for more information specific to builders for user
 * types.
 */

#ifdef DOXYGEN_DOCUMENTATION_ONLY

/**
 * @ingroup RTIFlatBuildersModule
 * @brief Represents the Builder for an arbitrary user-defined mutable type
 * 
 * This example type represents the \ref RTIFlatBuildersModule "Builder" type
 * that \nddsgen would generate for \ref MyFlatMutable and allows creating a 
 * sample or a member of that type.
 * 
 * The most common way to create a Builder is rti::flat::build_data(), which
 * obtains a managed sample from a \idref_FooDataWriter as described in 
 * \ref PublishingFlatData. It is also possible to create a Builder with an
 * arbitrary buffer using the constructor that receives the buffer and its size.
 * 
 * When a Builder is created, the type is empty--it doesn't contain any members. 
 * The Builder provides functions to create each member. There are two kind of
 * functions: \p add functions and \p build functions.
 * 
 * Adding fixed-size members
 * -------------------------
 * Fixed-size members are "added", and the corresponding function is called 
 * \p <b>add_<member_name></b>. "Add" functions directly place the member in
 * the buffer, and return an \ref RTIFlatOffsetsModule "Offset" that allows
 * setting its values. 
 * 
 * By default, after calling \p add_<member_name> the
 * member values are uninitialized. This behavior can be changed by creating
 * the writer with 
 * \idref_DataWriterResourceLimitsQosPolicy_initialize_writer_loaned_sample; 
 * this is however not recommended in general because of the performance impact 
 * for large types.
 * 
 * \code 
 * MyFlatMutableBuilder builder = ...;
 * MyFinalFooOffset member_offset = builder.add_my_final();
 * member_offset.my_primitive(10);
 * // ... add/build more members
 * \endcode
 * 
 * 
 * 
 * Building variable-size members
 * ------------------------------
 * Variable-size members are "built", and the function is called
 * \p <b>build_<member_name></b>. "Build" functions return another 
 * \ref RTIFlatBuildersModule "Builder" to build the member. 
 * 
 * \code
 * MyFlatMutableBuilder builder = ...;
 * auto member_builder = builder.build_my_mutable();
 * // ... use member_builder
 * member_builder.finish();
 * // ... add/build more members
 * \endcode
 * 
 * While a member is being built, the parent Builder is a <b>bound</b> state,
 * which prevents any changes to it until the member has been finished by calling
 * \p member_builder.finish(). In particular, the member Builder must be finished
 * before adding or building any other member, or before finishing \p builder.
 * 
 * Choosing which members are included
 * -----------------------------------
 * The \p add/build function for each member can be called one or zero times. 
 * That is, all members are optional, even those without the \p \@optional IDL
 * annotation. However \p \@key member must be added or \idref_FooDataWriter_write
 * will fail.
 * 
 * FlatData samples received by \idref_DataReader will not
 * contain the members that were not added/built--the corresponding member getters 
 * return a null Offset. A \idref_DataReader for an equivalent non-FlatData 
 * (plain) definition of this type will assign default values to any non-optional 
 * members that were not present in the sample when written.
 * 
 * It is not permitted to add a member more than once, but builders don't enforce
 * this. Trying to build/add a member more than once may cause the Builder to
 * overflow (see \ref BuilderErrorManagement). If it doesn't, it may be 
 * possible to write and read the data samples, but the duplicate members will 
 * be ignored.
 * 
 * @see \ref RTIFlatBuildersModule
 */
class NDDSUSERDllExport MyFlatMutableBuilder : public rti::flat::AggregationBuilder {
  public:
    /**
     * @brief The related offset type
     */
    typedef MyFlatMutableOffset Offset;

    /**
     * @brief Creates an invalid Builder
     * 
     * @post !is_valid()
     * 
     * @note Top-level builders are created with rti::flat::build_data(), and
     * member builders are created with the corresponding \p build_<member>
     * function.
     */ 
    MyFlatMutableBuilder()
    {
    }

    /**
     * @brief Construct a Builder with an arbitrary buffer
     * 
     * @note The recommended way to create a Builder is rti::flat::build_data()
     * as described in \ref PublishingFlatData. This constructor provides an
     * alternative option to use an arbitrary data buffer.
     * 
     * @param buffer The buffer that will be used to build the data sample
     * @param size The size of the buffer
     * @param initialize_members Whether fixed-size elements added to this
     * Builder will be initialized to their default values or will be left
     * uninitialized (more efficient).
     * 
     * If the sample being built overflows the buffer size, the add/build
     * operations will fail. See \ref BuilderErrorManagement.
     */
    MyFlatMutableBuilder(
            unsigned char *buffer, 
            int32_t size,
            bool initialize_members = false)
    {
    }

    /**
     * @brief Finishes building a member
     * 
     * @return The Offset to the member (normally it can be ignored)
     * 
     * @pre \ref rti::flat::AbstractBuilder::is_nested() "is_nested()". That is, 
     * this object must be a member Builder, not a sample Builder.
     */ 
    Offset finish();

    /**
     * @brief Finishes building a sample
     * 
     * @pre \ref rti::flat::AbstractBuilder::is_nested() "!is_nested()". 
     * That is, this object must be a sample Builder, not a member Builder. 
     * @post \ref rti::flat::AbstractBuilder::is_valid "!is_valid()". 
     * That is, this Builder can no longer be used.
     * 
     * @return The sample, ready to be written.
     * 
     * This function completes and returns the sample built with this Builder.
     *
     * @see rti::flat::build_data(), the function to create a 
     * \idref_FooDataWriter-managed sample Builder.
     */
    MyFlatMutable * finish_sample();

    /**
     * @brief Adds a primitive member
     */
    bool add_my_primitive(int32_t value);

    /**
     * @brief Adds a primitive member
     */
    bool add_my_optional_primitive(int32_t value);

    /**
     * @brief Adds a primitive array member
     * 
     * @return The Offset to the array, which can be used to set its values.
     */
    rti::flat::PrimitiveArrayOffset<int32_t, 10> add_my_primitive_array();

    /**
     * @brief Begins building a primitive-sequence member
     */
    rti::flat::PrimitiveSequenceBuilder<int32_t> build_my_primitive_seq();

    /**
     * @brief Adds a final complex member
     * 
     * @return The Offset to the member, which can be used to set its values.
     */    
    MyFlatFinalOffset add_my_final();

    /**
     * @brief Adds an array member of final complex elements
     * 
     * @return The Offset to the array, which can be used to set its values.
     */ 
    rti::flat::FinalAlignedArrayOffset<MyFlatFinalOffset, 10> add_my_final_array();

    /**
     * @brief Begins building a sequence member of final complex elements
     */     
    rti::flat::FinalSequenceBuilder<MyFlatFinalOffset> build_my_final_seq();

    /**
     * @brief Begins building a mutable complex member
     * 
     * FlatMutableBar represents another arbitrary mutable FlatData type. 
     */     
    FlatMutableBarBuilder build_my_mutable();

    /**
     * @brief Begins building an array member of mutable complex elements
     */    
    rti::flat::MutableArrayBuilder<FlatMutableBarBuilder, 10> build_my_mutable_array();

    /**
     * @brief Begins building a sequence member of mutable complex elements
     */    
    rti::flat::MutableSequenceBuilder<FlatMutableBarBuilder > build_my_mutable_seq();

    /**
     * @brief Begins building a string member
     */    
    rti::flat::StringBuilder build_my_string();

    /**
     * @brief Begins building a sequence member of string elements
     */    
    rti::flat::MutableSequenceBuilder<rti::flat::StringBuilder > build_my_string_seq();
};

/**
 * @ingroup RTIFlatBuildersModule
 * @brief Represents the Builder for an arbitrary user-defined mutable IDL union
 * 
 * This example type represents the \ref RTIFlatBuildersModule "Builder" type
 * that \nddsgen would generate for \ref MyFlatUnion and allows creating a 
 * sample or a member of that type.
 * 
 * Union builders are similar to struct builders (see MyFlatMutableBuilder),
 * except that they only allow adding/building <b>one member</b>.
 * 
 * "Add" and "build" functions automatically set the discriminator value that
 * in the IDL definition selects that member. If more than one discriminator
 * value selects a member, the add/build function allows picking one.
 *  
 */
class MyFlatUnionBuilder : public rti::flat::UnionBuilder<int32_t> {
public:
    /**
     * @brief The related offset type
     */
    typedef MyFlatUnionOffset Offset;
    

    /**
     * @brief Creates an invalid Builder
     * 
     * @post !is_valid()
     * 
     * @note Top-level builders are created with rti::flat::build_data(), and
     * member builders are created with the corresponding \p build_<member>
     * function.
     */ 
    MyFlatUnionBuilder()
    {
    }

    /**
     * @brief Finishes building a member
     * 
     * @see MyMutableBuilder::finish()
     */ 
    Offset finish();


    /**
     * @brief Finishes building a sample
     * 
     * @see MyMutableBuilder::finish_sample()
     */
    MyFlatUnion * finish_sample();

    /**
     * @brief Adds a primitive member
     * 
     * This function automatically selects the discriminator 0, which corresponds
     * to the member 'my_primitive' (see \ref MyFlatUnion)
     */
    bool add_my_primitive(int32_t value);

    /**
     * @brief Builds a mutable-struct member
     * 
     * @param discriminator Allows selecting one of the possible discriminator
     * values for this member: 1 or 2. This argument is optional: if not
     * specified it selects the first 'case' label in the IDL definition (1).
     * 
     * @return The Builder to build this member
     */    
    MyFlatMutableBuilder build_my_mutable(int32_t discriminator = 1);

    /**
     * @brief Adds a final-struct member
     * 
     * @return The Offset to the member, which can be used to set its values.
     * 
     * This function automatically selects the discriminator value 3, which 
     * corresponds to the member 'my_final' (see \ref MyFlatUnion).
     */    
    MyFlatFinal::Offset add_my_final();
};

#endif

namespace rti { namespace flat {

// Support for move constructor and assignment operator in C++98
//
// All concrete subclasses of AbstractBuilder must use this macro to implement
// move semantics
//
#if defined(RTI_FLAT_DATA_CXX11_RVALUE_REFERENCES)
#define RTI_FLAT_BUILDER_DEFINE_MOVE_OPERATIONS_IMPL(TYPE, BASE, PROXY)
#define RTI_FLAT_BUILDER_DEFINE_MOVE_OPERATIONS(TYPE, BASE)
#define RTI_FLAT_MOVE_BUILDER(BUILDER) BUILDER
#else
#define RTI_FLAT_BUILDER_DEFINE_MOVE_OPERATIONS_IMPL(TYPE, BASE, PROXY) \
public: \
    TYPE(PROXY other) throw() \
    { \
        this->move_from(other); \
    } \
    TYPE& operator=(PROXY other) throw() \
    { \
        this->finish_untyped_impl(); \
        this->move_from(other); \
        return *this; \
    } \
    TYPE move() \
    { \
        return TYPE(PROXY(*this)); \
    } \
private: \
    TYPE(TYPE&); \
    TYPE& operator=(TYPE&);

// Shortcut for generated IDL types
#define RTI_FLAT_BUILDER_DEFINE_MOVE_OPERATIONS(TYPE, BASE) \
    RTI_FLAT_BUILDER_DEFINE_MOVE_OPERATIONS_IMPL( \
        TYPE, BASE, UntypedAggregationBuilderMoveProxy)

#define RTI_FLAT_MOVE_BUILDER(BUILDER) (BUILDER).move()
#endif

namespace detail {
    // Template parameters can be AbstractBuilder or AbstractBuilderMoveProxy
    template <typename Builder1, typename Builder2>
    void move_abstract_builder(Builder1& to, Builder2& from)
    {
        if (from.parent_builder_ == NULL) {
            to.owned_stream_ = from.owned_stream_;
        }

        to.parent_stream_ = from.parent_stream_;
        to.parent_builder_ = from.parent_builder_;
        to.begin_position_ = from.begin_position_;
        to.bind_position_ = from.bind_position_;
        to.initialize_on_add_ = from.initialize_on_add_;
#ifdef RTI_FLAT_DATA_NO_EXCEPTIONS
        to.failure_ = from.failure_;
#endif

        from.parent_stream_ = NULL;
        from.parent_builder_ = NULL;
        from.bind_position_ = NULL;
        from.begin_position_ = NULL;
    }
}

/*
 * @brief Provides the functionality common to all builders
 * 
 * This class is the base of all builders and provides the following 
 * functionality:
 * 
 * - Get the buffer and its size
 * - Add final elements (base case)
 * - Build operations--create nested builders, binding (base case)
 * - Finish operation--unbind (base case)
 * - Discard operation--throw away a nested Builder and roll back the parent
 * - Error management when exceptions are not supported (traditional C++)
 * - Move semantics (base case)
 */
/**
 * @ingroup RTIFlatBuildersModule
 * @brief Base class of all \ref RTIFlatBuildersModule "Builders"
 */
class AbstractBuilder {
protected:
    AbstractBuilder()
        : parent_stream_(NULL),
          parent_builder_(NULL),
          begin_position_(NULL),
          bind_position_(NULL),
          initialize_on_add_(false)
#ifdef RTI_FLAT_DATA_NO_EXCEPTIONS
          , failure_(false)
#endif          
    {
    }

    /*i
     * @brief Create a new top-level Builder
     */
    AbstractBuilder(unsigned char *buffer, offset_t size, bool initialize_members)
        : parent_stream_(NULL),
          parent_builder_(NULL),
          bind_position_(NULL),
          initialize_on_add_(initialize_members)
#ifdef RTI_FLAT_DATA_NO_EXCEPTIONS
          , failure_(false)
#endif
    {
        // Serializes the encapsulation into the buffer and initializes the
        // stream with that buffer and that encapsulation
        if (!RTIXCdrFlatSample_initializeEncapsulationAndStream(
                (char *) buffer, 
                &owned_stream_.c_stream(),
                RTIXCdrEncapsulationId_getNativePlCdr2(), 
                size)) {
            RTI_FLAT_BUILDER_OUT_OF_RESOURCES_ERROR(return);
        }

        begin_position_ = owned_stream_.current_position();
    }

protected:
    template <typename Builder1, typename Builder2>
    friend void detail::move_abstract_builder(Builder1& to, Builder2& from);

#if defined(RTI_FLAT_DATA_CXX11_RVALUE_REFERENCES)
    AbstractBuilder(AbstractBuilder&& other)
    {
        detail::move_abstract_builder(*this, other);
    }

    AbstractBuilder& operator=(AbstractBuilder&& other)
    {
        if (this == &other) {
            return *this;
        }

        finish_untyped_impl();

        detail::move_abstract_builder(*this, other);

        return *this;
    }
#else
    // Enables the safe-move-constructor idiom without C++11 move constructors
    struct AbstractBuilderMoveProxy {
        rti::xcdr::Stream owned_stream_;
        rti::xcdr::Stream *parent_stream_;
        AbstractBuilder *parent_builder_;
        unsigned char *begin_position_;
        unsigned char *bind_position_;
        bool initialize_on_add_;
#ifdef RTI_FLAT_DATA_NO_EXCEPTIONS
        bool failure_;
#endif
    };

    void move_from(AbstractBuilderMoveProxy& other)
    {
        detail::move_abstract_builder(*this, other);
    }

    void move_to(AbstractBuilderMoveProxy& other)
    {
        detail::move_abstract_builder(other, *this);
    }

    operator AbstractBuilderMoveProxy () throw() // move-constructor idiom
    {
        AbstractBuilderMoveProxy other;
        move_to(other);
        return other;
    }
private:
    AbstractBuilder(AbstractBuilder& other);
    AbstractBuilder& operator=(AbstractBuilder& other);    
#endif

protected:
    /**
     * @brief If this is a member Builder, it calls finish().
     * 
     * If this Builder is building a member (that is, is_nested() is true), and
     * the object goes out of scope before finish() has been called, its
     * destructor calls finish(). Note, however, that it won't report any error.
     * 
     * If this Builder is building a sample (!is_nested()), its destructor doesn't
     * do anything.
     * 
     */ 
    virtual ~AbstractBuilder()
    {
        finish_untyped_impl();
    }

    struct nested_tag_t {}; // disambiguate copy constructor
    
    /*i
     * @brief Creates a nested Builder to build a member or element
     * 
     * @param parent The Builder for the type that contains the member or element
     * this Builder builds.
     * 
     * @param alignment Specifies the alignment required by the concrete Builder.
     * Namely, aggregation and sequence builders require alignment of 4 for the 
     * DHeader and the sequence length, respectively. This alignment doesn't
     * reflect the alignment requirement of the elements, that's why arrays do
     * not require an alignment here. If no alignment is required, this parameter
     * must be zero. Concrete classes must provide a default value for alignment,
     * that indicates their requirement. This parameter can be overridden by
     * build_element_no_align to when we know we don't need to align.
     */
    AbstractBuilder(
            nested_tag_t, 
            AbstractBuilder& parent, 
            unsigned int alignment) 
        : /* owned_stream_ empty and unused */
          parent_stream_(&parent.stream()), /* work on the parent's stream */
          parent_builder_(&parent), 
          begin_position_(NULL),
          bind_position_(NULL),
          initialize_on_add_(parent.initialize_on_add_)
#ifdef RTI_FLAT_DATA_NO_EXCEPTIONS
          , failure_(false)
#endif          
    {
        if (alignment != 0) {
            if (!stream().align(alignment)) {
                RTI_FLAT_BUILDER_OUT_OF_RESOURCES_ERROR(return);
            }
        }
        begin_position_ = stream().current_position();
    }

    unsigned char * buffer()
    {
        return stream().buffer();
    }

    unsigned char * begin_position()
    {
        return begin_position_;
    }

    /*i
     * @brief Adds a fixed-size member or element
     * 
     * @return The offset to the member or element that was added. This offset
     * can be used to initialize it.
     */
    template <typename OffsetType>
    OffsetType add_element()
    {
        RTI_FLAT_BUILDER_CHECK_VALID(return OffsetType());
        RTI_FLAT_BUILDER_CHECK_NOT_BOUND(return OffsetType());

        offset_t member_size = OffsetType::serialized_size(0);
        unsigned char *pos = stream().current_position();
        if (detail::is_fixed_type_w_dheader<OffsetType>::value()) {
            pos = (unsigned char *) stream().serialize_dheader();
            if (pos == NULL) {
                RTI_FLAT_BUILDER_OUT_OF_RESOURCES_ERROR(return OffsetType());
            }
        }
        if (!stream().skip(member_size)) {
            RTI_FLAT_BUILDER_OUT_OF_RESOURCES_ERROR(return OffsetType());
        }
        if (detail::is_fixed_type_w_dheader<OffsetType>::value()) {
            stream().finish_dheader((char *) pos);
        }

        if (initialize_on_add_) {
            OffsetType offset(
                (SampleBase *) stream().buffer(), 
                detail::ptrdiff(pos, stream().buffer()));

            detail::final_offset_initializer<OffsetType>::initialize(offset);

            return offset;
        } else {
            return OffsetType(
                (SampleBase *) stream().buffer(), 
                detail::ptrdiff(pos, stream().buffer()));
        }
    }

    /*i
     * @brief Creates a nested Builder to build a variable-size member or element
     * 
     * @param stream_memento Subclasses using build_element() to implement
     * their own build method are required to provide a stream memento that
     * contains the position in the stream before any bytes related to this
     * member were added. Any error before the nested Builder is created will
     * cause the memento's destructor to roll back the Builder to its state
     * before this member was added. If the Builder can be created without errors
     * stream_mement.discard() is called, and its position is kept in case
     * the nested Builder is discarded (instead of finished), which also rolls
     * back the parent Builder to its previous state.
     * 
     * @post This Builder becomes "bound" until the returned Builder is finished.
     * While bound, this Builder can't be used to add or build more elements.
     * 
     * @return The Builder that allows building this member or element
     */
    template <typename NestedBuilder>
    NestedBuilder build_element(rti::xcdr::Stream::Memento& stream_memento)
    {
        RTI_FLAT_BUILDER_CHECK_VALID(return NestedBuilder());
        RTI_FLAT_BUILDER_CHECK_NOT_BOUND(return NestedBuilder());

        NestedBuilder nested_builder(nested_tag_t(), *this);
        RTI_FLAT_BUILDER_CHECK_CREATE_BUILDER(
                nested_builder, 
                return NestedBuilder());

        bind_position_ = stream_memento.discard();

        // uninit_use_in_call is not possible given how the constructor for
        // the NestedBuilder type is generated by rtiddsgen. The warning is
        // suppressed for performance reasons.
        /* coverity[uninit_use_in_call : FALSE] */
        return RTI_FLAT_MOVE_BUILDER(nested_builder);
    }

    template <typename NestedBuilder>
    NestedBuilder build_element_no_align(rti::xcdr::Stream::Memento& stream_memento)
    {
        RTI_FLAT_BUILDER_CHECK_VALID(return NestedBuilder());
        RTI_FLAT_BUILDER_CHECK_NOT_BOUND(return NestedBuilder());

        // By passing '0' to the builder constructor we override its default
        // argument which indicates its required alignment
        NestedBuilder nested_builder(nested_tag_t(), *this, 0);
        RTI_FLAT_BUILDER_CHECK_CREATE_BUILDER(
                nested_builder, 
                return NestedBuilder());

        bind_position_ = stream_memento.discard();

        // uninit_use_in_call is not possible given how the constructor for
        // the NestedBuilder type is generated by rtiddsgen. The warning is
        // suppressed for performance reasons.
        /* coverity[uninit_use_in_call : FALSE] */
        return RTI_FLAT_MOVE_BUILDER(nested_builder);
    }  

    /*i
     * @brief Returns the currently used number of bytes
     * 
     * The current size is the number of bytes that have been used to create
     * this sample by adding or building members.
     * 
     * When current_size() reaches capacity(), the Builder will fail to add
     * or build any additional member.
     * 
     * @see capacity().
     */
    unsigned int current_size()
    {
        return detail::ptrdiff(stream().current_position(), begin_position_);
    }    


    friend class AggregationBuilder; // to access finish_member()

    // Finishes this nested builder, updating the parent builder.
    //
    // This function doesn't throw exceptions and doesn't return a typed Offset
    // This function can be used in destructors to "silently" finish the builder
    void finish_untyped_impl()
    {
        if (!is_valid() || !is_nested()) {
            return;
        }

        parent_builder_->finish_member();
        invalidate();
    }

protected:
    /*i
     * @brief Complete the building of a nested member
     * 
     * Concrete Builders must implement a finish() function with the following body:
     * 
     * RTI_FLAT_BUILDER_CHECK_CAN_FINISH(...);
     * // optionally, additonal work
     * return finish_impl<OffsetType>();
     * 
     * Destructors can directly call finish_untyped_impl(); 
     * 
     * When this is a nested Builder, this function indicates that this element
     * or member is complete.
     * 
     * This function is automatically called by the Builder destructor.
     * 
     * Concrete builders must override this function to return a typed Offset to
     * the element that was built.
     * 
     * @post This Builder is in an empty state and cannot be used further
     * 
     * @return The position in the buffer where this element ended.
     */
    template <typename OffsetType>
    OffsetType finish_impl()
    {
        RTI_FLAT_BUILDER_CHECK_VALID(return OffsetType());

        unsigned char *begin_pos = this->begin_position_;
        unsigned char *sample_base = stream().buffer();
        unsigned char *current_pos = stream().current_position();
        finish_untyped_impl();
    
        return OffsetType(
                reinterpret_cast<SampleBase *>(sample_base), // start of the top-level sample
                detail::ptrdiff(begin_pos, sample_base), // absoute offset to the member
                detail::ptrdiff(current_pos, begin_pos)); // size of the member
    }

public:
    /**
     * @brief Discards a member in process of being built
     * 
     * This function ends the creation of a member, returning the Builder of
     * the type that contains the member to its previous state, as if this
     * member had never been built.
     * 
     * @pre This object must be a member Builder, not a sample Builder.
     * 
     * This method is useful when during the building of a member an error
     * occurs and the application wants to roll back, instead of finishing an
     * incomplete member.
     */
    void discard()
    {
        RTI_FLAT_BUILDER_CHECK_VALID(return);
        RTI_FLAT_BUILDER_CHECK_CAN_FINISH(return);

        parent_builder_->discard_member();
        invalidate();
    }

    /**
     * @brief Returns whether this is a member Builder
     * 
     * A member Builder is a Builder that has been created 
     * by calling a \p "build_<member>" function on another Builder (for 
     * example, MyFlatMutableBuilder::build_my_mutable()).
     * 
     * @return True if this is a member Builder, or false if this is a sample
     * Builder.
     */
    bool is_nested() const
    {
        return parent_builder_ != NULL;
    }

    /**
     * @brief Whether this Builder is valid
     * 
     * A Builder is not valid when it is default-constructed, or after any
     * of these functions is called: finish(), finish_sample(), discard().
     * 
     * \if CPP_LANGUAGE_ONLY
     * Since the Traditional C++ API doesn't use exceptions, certain function
     * failures due to the Builder running out resources are also reported by
     * invalidating the Builder.
     * \endif
     */
    bool is_valid() const
    {
        return begin_position_ != NULL;
    }

    /**
     * @brief Returns the total capacity in bytes
     * 
     * The capacity is the total number of bytes this Builder can contain. For
     * a sample Builder (that is, one such that is_nested() returns false), 
     * rti::flat::build_data reserves enough bytes to accommodate any sample of 
     * a given type.
     */
    rti::xcdr::length_t capacity() const
    {
        return stream().total_size();
    }

protected:
    // Makes this Builder invalid after finish(); discard(); finish_sample();
    // and, when exceptions are not enabled (traditional C++ API), after an
    // error during construction
    void invalidate()
    {
        parent_stream_ = NULL;
        parent_builder_ = NULL;
        bind_position_ = NULL;
        begin_position_ = NULL;
    }

    // This function is called by a nested Builder when it has finished building
    // an element
    virtual void finish_member() // noexcept
    {
        if (!is_valid()) {
            return;
        }

        bind_position_ = NULL;
    }

    void discard_member()
    {
        RTI_FLAT_BUILDER_CHECK_VALID(return);

        stream().current_position(bind_position_);
        bind_position_ = NULL;
    }

    rti::xcdr::Stream& stream()
    {
        if (parent_stream_ != NULL) {
            return *parent_stream_;
        } else {
            return owned_stream_;
        }
    }

    const rti::xcdr::Stream& stream() const
    {
        if (parent_stream_ != NULL) {
            return *parent_stream_;
        } else {
            return owned_stream_;
        }
    }

private:
    rti::xcdr::Stream owned_stream_;
    rti::xcdr::Stream *parent_stream_;
    AbstractBuilder *parent_builder_;
    unsigned char *begin_position_;
    unsigned char *bind_position_;
    bool initialize_on_add_;

#ifdef RTI_FLAT_DATA_NO_EXCEPTIONS
public:
    /**
     * @brief Checks if the previous operation failed and resets the failure flag
     * 
     * This function must be called after each Builder operation to check if
     * it succeeded. Note that after calling check_failure() the error flag is
     * reset, so a subsequent call will always return false.
     * 
     * @return True if there was a failure in the previous operation, false
     * if there was no failure since the last call to check_failure.
     */
    bool check_failure()
    {
        bool failure = failure_;
        failure_ = false;
        return failure;
    }
protected:
    void set_failure()
    {
        failure_ = true;
    }
private:
    bool failure_;
#endif


};

} }

#endif // RTI_DDS_FLAT_BUILDER_HPP_

