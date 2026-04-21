/*
(c) Copyright, Real-Time Innovations, 2018.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
*/

#ifndef RTI_DDS_FLAT_BUILDERHELPER_HPP_
#define RTI_DDS_FLAT_BUILDERHELPER_HPP_

namespace rti { namespace flat { 

template <typename T>
class PrimitiveSequenceBuilder;

template <typename ElementBuilder>
class MutableSequenceBuilder;

template <typename ElementOffset>
class FinalSequenceBuilder;

template <typename ElementBuilder, unsigned int N>
class MutableArrayBuilder;

class StringBuilder;

namespace detail {

//
// lc_code:
//

template <size_t PrimitiveSize>
struct primitive_lc_code_helper;

template <>
struct primitive_lc_code_helper<1> {
    enum { 
        single = 0, // LC = 1 => member length is 1
        sequence = 5 // LC = 5 => member length is also NEXTINT
    };
};

template <>
struct primitive_lc_code_helper<2> {
    enum { 
        single = 1, // LC = 1 => member length is 2
        sequence = 4 // LC = 4 => member length is provided as an additional int
    };
};

template <>
struct primitive_lc_code_helper<4> {
    enum { 
        single = 2, // LC = 2 => member length is 4
        sequence = 6 // LC = 6 => member length is 4 * NEXTINT
    };
};

template <>
struct primitive_lc_code_helper<8> {
    enum { 
        single = 3, // LC = 3 => member length is 8
        sequence = 7 // LC = 7 => member length is 8 * NEXTINT
    };
};

template <typename T>
struct primitive_lc_code {
    enum { value = primitive_lc_code_helper<sizeof(T)>::single };
};

// In general, use LC = 5, for mutable aggregation types. Final aggregation
// types don't use this 'lc_code' trait type. The other cases are specialized 
// after this.
template <typename T, typename Enable = void>
struct lc_code {
    static int value()
    {
        return 5;  // LC = 5 => member length is provided in NEXTINT/
    }
};

// For collections of complex types and enums, if we serialize the DHeader
// (XCDR2 compliant behavior) we use LC = 5; if we don't (legacy RTI behavior)
// we use LC = 4, since we codify the length in the member header.
template <typename T>
struct lc_code<MutableSequenceBuilder<T> > {
    static int value()
    {
        return is_dheader_required_in_non_primitive_collections() ? 5 : 4;
    }
};

template <typename T>
struct lc_code<FinalSequenceBuilder<T> > {
    static int value()
    {
        return is_dheader_required_in_non_primitive_collections() ? 5 : 4;
    }
};

template <typename T, unsigned int N>
struct lc_code<MutableArrayBuilder<T, N> > {
    static int value()
    {
        return is_dheader_required_in_non_primitive_collections() ? 5 : 4;
    }
};

#if defined(RTI_FLAT_DATA_NO_EXCEPTIONS)

// Traditional C++: no support for std::is_enum<T>

template <typename T>
struct lc_code<PrimitiveSequenceBuilder<T> > {
    static int value()
    {
        return primitive_lc_code_helper<sizeof(T)>::sequence;
    }
};

#else

template <typename T>
struct lc_code<
        PrimitiveSequenceBuilder<T>,
        typename std::enable_if<!std::is_enum<T>::value>::type> {
    static int value()
    {
        return primitive_lc_code_helper<sizeof(T)>::sequence;
    }
};

template <typename T>
struct lc_code<
        PrimitiveSequenceBuilder<T>,
        typename std::enable_if<std::is_enum<T>::value>::type> {
    static int value()
    {
        return is_dheader_required_in_enum_collections()
                ? 5
                : primitive_lc_code_helper<sizeof(T)>::sequence;
    }
};

#endif


//
// Initialization of final types
//

template <typename Offset>
struct final_offset_initializer {

    static bool initialize(Offset& offset)
    {
        return RTI_XCDR_TRUE == RTIXCdrFlatData_initializeSample(
                (char *) offset.get_buffer(), 
                offset.get_buffer_size(), 
                rti::xcdr::type_programs<rti::flat::Sample<Offset> >::get());
    }

};

// This will be specialized for enums in generated code (traditional C++)
// and for enum classes in FlatDataTraits.hpp (modern C++)
template <typename T, typename Enable = void>
struct default_primitive_value {
    static T get() 
    {
        return (T) 0;
    }
};

template <typename T, unsigned int N>
struct final_offset_initializer<rti::flat::PrimitiveArrayOffset<T, N> > {

    static bool initialize(rti::flat::PrimitiveArrayOffset<T, N>& array)
    {
        T default_value = default_primitive_value<T>::get();
        if (default_value == (T) 0) { // don't need to deal with endianness
            memset(array.get_buffer(), 0, array.get_buffer_size());
        } else {
            for (unsigned int i = 0; i < array.element_count(); i++) {
                array.set_element(i, default_value);
            }
        }
        return true;
    }
};

template <typename T, unsigned int N>
struct final_offset_initializer<rti::flat::FinalAlignedArrayOffset<T, N> > {

    static bool initialize(rti::flat::FinalAlignedArrayOffset<T, N>& array)
    {
        for (unsigned int i = 0; i < N; i++) {
            T offset = array.get_element(i);
            if (offset.is_null()) {
                return false;
            }

            if (!final_offset_initializer<T>::initialize(offset)) {
                return false;
            }
        }

        return true;
    }
};

} } }

#endif // RTI_DDS_FLAT_BUILDERHELPER_HPP_

