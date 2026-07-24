/*
(c) Copyright, Real-Time Innovations, 2018.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
*/

#ifndef RTI_DDS_FLAT_FLATSAMPLE_HPP_
#define RTI_DDS_FLAT_FLATSAMPLE_HPP_

// Note: FlatSampleImpl.hpp needs to be included in addition to this header.

#include <cstdlib>
#include <memory>

#include "xcdr/xcdr_stream_impl.h"
#include "xcdr/xcdr_flat_data.h"
#include "xcdr/xcdr_interpreter.h"

#include "rti/xcdr/Interpreter.hpp" // for type_programs

/** 
 * @defgroup RTIFlatSampleModule  FlatData Samples
 * @ingroup RTIFlatDataModule
 * @brief A Sample represents an instance of the IDL topic-type and contains 
 * the data in serialized format.
 *  
 * All FlatData topic-types are instantiations of rti::flat::Sample. 
 * This documentation uses the following three example types to illustrate the
 * different ways FlatData IDL types map to C++:
 * - \ref MyFlatFinal, the type generated for a final IDL struct
 * - \ref MyFlatMutable, the type generated for a mutable IDL struct
 * - \ref MyFlatUnion, the type generated for an IDL union
 */

namespace rti { namespace flat {

struct fixed_size_type_tag_t {};
struct variable_size_type_tag_t {};

typedef RTIXCdrUnsignedLong offset_t;
typedef RTIXCdrUnsignedLong member_id_t;

// The untemplated base of Sample<T>
class SampleBase {
public:

    unsigned char* get_buffer()
    {
        return &data[0];
    }

    const unsigned char* get_buffer() const
    {
        return &data[0];
    }

    unsigned char* get_root_buffer()
    {
        return get_buffer() + RTI_XCDR_ENCAPSULATION_SIZE;
    }

    /*i
     * @brief Provides the address of the top-level type
     */
    const unsigned char* get_root_buffer() const
    {
        return get_buffer() + RTI_XCDR_ENCAPSULATION_SIZE;
    }

    void initialize_stream(
        RTIXCdrStream& stream, 
        offset_t offset, 
        offset_t serialized_size) const
    {
        RTIXCdrFlatData_initializeStream(
                &stream, 
                const_cast<unsigned char*>(get_buffer()),
                offset,
                serialized_size);
    }

protected: // xlc compiler fails when the default constructor is private
#if !defined(RTI_FLAT_DATA_CXX11_DELETED_FUNCTIONS)    
    SampleBase() {} // Samples can only be created via Sample::create_data
    SampleBase(const SampleBase&) {}
    SampleBase& operator=(const SampleBase&);
#else
    SampleBase() = delete;
    SampleBase(const SampleBase&) = delete;
    SampleBase& operator=(const SampleBase&) = delete;
#endif

    // The actual size of data (and therefore, the Sample object) is variable,    
    // and equal to the serialized size of the IDL Sample
    unsigned char data[1];
};

/**
 * @ingroup RTIFlatSampleModule
 * @brief The generic definition of FlatData topic-types
 * 
 * @tparam OffsetType The Offset to the beginning of the data Sample (the root), 
 * for example MyFlatFinalOffset, or MyFlatMutableOffset.
 * 
 * For a FlatData IDL type, \nddsgen generates an instantiation of this class,
 * such as MyFlatFinal or MyFlatMutable. That's the <b>topic-type</b> used to 
 * declare a \idref_Topic and write or read FlatData samples.
 * 
 * A FlatData Sample owns an inline buffer that contains the serialized data.
 * To access the Sample data members we use \ref RTIFlatOffsetsModule "Offsets", 
 * iterators that point to the position of a member in the Sample buffer. The 
 * root() is the Offset to the top-level element (for example MyFlatMutableOffset). 
 * From there MyFlatMutableOffset's member functions provide access to its 
 * members.
 *
 * The method to create a Sample varies depending on whether the type is final
 * or mutable. 
 * 
 * Samples of <b>final</b> FlatData types always have the same size, and
 * can be obtained directly with \idref_FooDataWriter_get_loan().
 *
 * Samples of <b>mutable</b> FlatData types need to be "built" using the Builder
 * returned by rti::flat::build_data() (MyFlatMutableBuilder in this example). 
 * A Builder allows adding each member individually in any order, and sizing 
 * sequences as needed.
 * 
 * See \ref PublishingFlatData for code snippets.
 * 
 * Samples created with \idref_FooDataWriter_get_loan() or rti::flat::build_data()
 * are DataWriter-managed and do not need to be explicitly deleted. See 
 * \idref_FooDataWriter_get_loan() for an explanation of the lifecycle of
 * DataWriter-managed samples.
 * 
 * Samples received with a \idref_FooDataReader can be examined by obtaining the
 * root(), but cannot be modified. See \ref SubscribingFlatData.
 * 
 * FlatData samples are not value types. They don't provide copy constructors 
 * or assignment operators, because of their definition as an inline buffer that
 * contains the serialized Sample at all times. A Sample can be cloned with 
 * the static function clone(). A cloned Sample is unmanaged and has to be 
 * deleted with the static function delete_data().
 */
template <typename OffsetType>
class Sample : public SampleBase {
public:
    /**
     * @brief The related Offset type
     * 
     * This is the template parameter, OffsetType
     */
    typedef OffsetType Offset;

    /**
     * @brief The related read-only Offset type
     * 
     * For example, MyFlatMutableOffset::ConstOffset.
     */    
    typedef typename OffsetType::ConstOffset ConstOffset;

    /**
     * @brief Provides the Offset to the top-level type
     * 
     * @return An Offset that allows reading and modifying the members of this 
     * Sample, for example MyFlatFinalOffset or MyFlatMutableOffset.
     * 
     * This overload allows modifying the Sample.
     * 
     * Example:
     * \code
     * MyFlatFinal *my_sample = ...; // for example, created with DataWriter::get_loan()
     * MyFlatFinalOffset my_sample_root = my_sample->root();
     * my_sample_root.my_primitive(33);
     * auto my_member_offset = my_sample_root.my_complex();
     * // ... modify my_member_offset
     * \endcode
     */
    Offset root()
    {
        return root_impl(typename Offset::offset_kind());
    }

    /**
     * @brief Provides the Offset to the top-level type
     * 
     * @return An Offset that allows reading the members of this Sample. 
     * For example, MyFlatFinalOffset::ConstOffset or MyFlatMutableOffset::ConstOffset.
     * 
     * This overload doesn't allow modifying the Sample.
     * 
     * Example:
     * \code
     * const MyFlatMutable *my_sample = ...; // for example, read from a DataReader
     * MyFlatMutableOffset::ConstOffset my_sample_root = my_sample->root();
     * std::cout << my_sample_root.my_primitive() << std::endl;
     * auto my_member_offset = my_sample_root.my_mutable_seq();
     * // ... read my_member_offset
     * \endcode 
     */
    ConstOffset root() const
    {
        return root_impl(typename ConstOffset::offset_kind());
    }

    /** @private
     * @brief Returns the endianness in which this Sample is serialized
     */
    RTIXCdrEndian endian() const
    {
        RTIXCdrStream tmp_stream;
        initialize_stream(tmp_stream, 0, RTI_XCDR_ENCAPSULATION_SIZE);
        return tmp_stream._endian;
    }

    /** @private
     * @brief Casts a byte buffer into a Sample of this type
     */
    static Sample<OffsetType>* from_buffer(unsigned char *buffer)
    {
        return reinterpret_cast<Sample<OffsetType>*>(buffer);
    }

    /** 
     * @brief Creates an unmanaged FlatData Sample
     * 
     * @note In general on the publication side it is recommended to create
     * DataWriter-managed samples using \idref_FooDataWriter_get_loan 
     * (for final types) and rti::flat::build_data() (for mutable types).
     * 
     * If the topic-type is final, the returned Sample can be initialized
     * by obtaining the root().
     * 
     * If the topic-type is mutable, the resulting Sample has no members, and
     * root() returns a \ref rti::flat::OffsetBase::is_null "null" Offset. 
     * The result of create_data() must be used to
     * create a Builder. This is a rare use case, and rti::flat::build_data() 
     * should be used in most situations.
     * 
     * In all cases, the returned Sample must be explicitly deleted with
     * delete_data().
     */
    static Sample<OffsetType>* create_data(); // impl in FlatSampleImpl.hpp

    /**
     * @brief Clones a Sample, creating an unmanaged Sample
     * 
     * @warning When this Sample has been created with \idref_FooDataWriter_get_loan
     * or rti::flat::build_data(), the clone this function creates cannot be
     * used in \idref_FooDataWriter_write, because the DataWriter is managing
     * the lifecycle of its samples. 
     * 
     * Creates a new Sample and copies the underlying serialized buffer.
     * 
     * delete_data() must be called to release the Sample.
     */
    Sample<OffsetType>* clone() const
    {
        unsigned char *buffer = RTIXCdrFlatData_cloneSample(
                get_buffer(),
                sample_size());

        if (buffer == NULL) {
          #ifndef RTI_FLAT_DATA_NO_EXCEPTIONS
            throw std::bad_alloc();
          #else
            return NULL;
          #endif
        }

        return reinterpret_cast<rti::flat::Sample<OffsetType> *>(buffer);
    }

    /** 
     * @brief Releases an unmanaged Sample
     * 
     * Unmanaged samples are those created with create_data() or clone().
     * 
     * @pre \p sample cannot have been created with \idref_FooDataWriter_get_loan
     * or rti::flat::build_data().
     * 
     * @post \p sample becomes invalid and shouldn't be used
     */
    static void delete_data(rti::flat::Sample<OffsetType>* sample)
    {
        RTIXCdrFlatData_deleteSample(sample);
    }

    /** @private
     */
    offset_t buffer_size() const
    {
        return sample_size() + RTI_XCDR_ENCAPSULATION_SIZE;
    }

    /** @private
     */
    offset_t sample_size() const
    {
        return sample_size_impl(typename OffsetType::offset_kind());
    }

private:
    offset_t sample_size_impl(fixed_size_type_tag_t) const
    {
        return OffsetType::serialized_size(0);
    }

    offset_t sample_size_impl(variable_size_type_tag_t) const
    {
        return RTIXCdrFlatSample_getMutableSampleSize(
                get_buffer(),
                RTI_XCDR_ENCAPSULATION_SIZE);
    }

    Offset root_impl(fixed_size_type_tag_t)
    {
        return Offset(this, RTI_XCDR_ENCAPSULATION_SIZE); 
    }

    Offset root_impl(variable_size_type_tag_t)
    {
        return Offset(this, RTI_XCDR_ENCAPSULATION_SIZE, sample_size()); 
    }

    ConstOffset root_impl(fixed_size_type_tag_t) const
    {
        return ConstOffset(this, RTI_XCDR_ENCAPSULATION_SIZE); 
    }

    ConstOffset root_impl(variable_size_type_tag_t) const
    {
        return ConstOffset(this, RTI_XCDR_ENCAPSULATION_SIZE, sample_size()); 
    }
};

template <typename T>
struct flat_type_traits;

} }

#ifdef DOXYGEN_DOCUMENTATION_ONLY

/**
 * @ingroup RTIFlatSampleModule
 * @brief Represents an arbitrary user-defined FlatData final IDL struct
 * 
 * This documentation uses the following example IDL definition of MyFlatFinal:
 * 
 * \code
 *   @language_binding(FLAT_DATA)
 *   @final
 *   struct MyFlatFinal {
 *       long my_primitive;
 *       FlatFinalBar my_complex; // Another arbitrary final FlatData type
 *       long my_primitive_array[10];
 *       FlatFinalBar my_complex_array[10];
 *   };
 *   \endcode
 *
 * For this type, \nddsgen generates:
 * - MyFlatFinal (this instantiation of Sample), 
 * - MyFlatFinalOffset.
 * 
 * Note that, as a final FlatData type, MyFlatFinal can only contain 
 * fixed-size types, such as primitives, other final FlatData structs, and arrays
 * of fixed-size types. 
 * 
 * Samples of MyFlatFinal are created with \idref_FooDataWriter_get_loan.
 * After creating a final Sample, modify its values using the MyFlatFinalOffset
 * returned by \ref rti::flat::Sample::root "root()".
 */
typedef rti::flat::Sample<MyFlatFinalOffset> MyFlatFinal;

/**
 * @ingroup RTIFlatSampleModule
 * @brief Represents an arbitrary user-defined flat mutable IDL struct
 * 
 * This documentation uses the following example IDL definition of MyFlatMutable:
 * 
 * \code
 *   @language_binding(FLAT_DATA)
 *   @mutable
 *   struct MyFlatMutable {
 *       long my_primitive;
 *       @optional long my_optional_primitive;
 *       long my_primitive_array[10];
 *       sequence<long, 10> my_primitive_seq;
 * 
 *       MyFlatFinal my_final;
 *       MyFlatFinal my_final_array[10];
 *       sequence<MyFlatFinal, 10> my_final_seq;
 *
 *       FlatMutableBar my_mutable;
 *       FlatMutableBar my_mutable_array[10];
 *       sequence<FlatMutableBar, 10> my_mutable_seq;
 *
 *       string<255> my_string;
 *       sequence<string<255>, 10> my_string_seq;
 *   };
 * \endcode
 * 
 * For this type, \nddsgen generates:
 * - MyFlatMutable (this instantiation of Sample), 
 * - MyFlatMutableOffset,
 * - MyFlatMutableBuilder.
 * 
 * As a mutable FlatData type, MyFlatMutable is not restricted to fixed-size
 * members. Samples of MyFlatMutable are created with rti::flat::build_data(), which
 * returns a MyFlatMutableBuilder. Once built, a mutable Sample can't change 
 * in size, but the value of members that already exist can be changed using
 * the MyFlatMutableOffset returned by \ref rti::flat::Sample::root "root()".
 */
typedef rti::flat::Sample<MyFlatMutableOffset> MyFlatMutable;

/**
 * @ingroup RTIFlatSampleModule
 * @brief Represents an arbitrary user-defined flat mutable IDL union
 * 
 * This documentation uses the following example IDL definition of MyFlatUnion:
 * 
 * \code
 *   @language_binding(FLAT_DATA)
 *   @mutable
 *   union MyFlatUnion switch (long) {
 *       case 0:
 *           long my_primitive;
 *       case 1:
 *       case 2:
 *           MyFlatMutable my_mutable;
 *       case 3:
 *           MyFlatFinal my_final;
 *   };
 * \endcode
 * 
 * @note FlatData unions can only be <b>mutable</b> since unions are, 
 * by definition, variable-size types.
 * 
 * For this type, \nddsgen generates:
 * - MyFlatUnion (this instantiation of Sample), 
 * - MyFlatUnionOffset,
 * - MyFlatUnionBuilder.
 * 
 */
typedef rti::flat::Sample<MyFlatUnionOffset> MyFlatUnion;
#endif

#endif // RTI_DDS_FLAT_FLATSAMPLE_HPP_

