/* $Id$

(c) Copyright, Real-Time Innovations, 2018.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.

============================================================================= */

#ifndef RTI_XCDR_STREAM_HPP_
#define RTI_XCDR_STREAM_HPP_

#include "xcdr/xcdr_stream.h"

namespace rti { namespace xcdr {


#define RTI_XCDR_SPECIALIZE_PRIMITIVE_TYPE_TRAITS_BODY(\
        TYPE,\
        CDR_TYPE,\
        SERIALIZE_MACRO)\
    static void serialize_fast(RTIXCdrStream *stream, const TYPE* v)\
    {\
        RTIXCdrStream_ ## SERIALIZE_MACRO ## ByteFast(stream, (CDR_TYPE *) v);\
    }\
    static void deserialize_fast(RTIXCdrStream *stream, TYPE* v)\
    {\
        RTIXCdrStream_de ## SERIALIZE_MACRO ## ByteFast(stream, (CDR_TYPE *) v);\
    }\
    static bool serialize(RTIXCdrStream *stream, const TYPE* v)\
    {\
        return RTIXCdrStream_ ## SERIALIZE_MACRO ## Byte(stream, (CDR_TYPE *) v, RTI_XCDR_TRUE);\
    }\
    static bool deserialize(RTIXCdrStream *stream, TYPE* v)\
    {\
        return RTIXCdrStream_de ## SERIALIZE_MACRO ## Byte(stream, (CDR_TYPE *) v, RTI_XCDR_TRUE);\
    }\

#define RTI_XCDR_SPECIALIZE_PRIMITIVE_TYPE_TRAITS(\
        TYPE,\
        CDR_TYPE,\
        SERIALIZE_MACRO)\
template <>\
struct primitive_type_traits<TYPE> {\
RTI_XCDR_SPECIALIZE_PRIMITIVE_TYPE_TRAITS_BODY(TYPE, CDR_TYPE, SERIALIZE_MACRO)\
}

#define RTI_XCDR_SPECIALIZE_PRIMITIVE_TYPE_TRAITS_W_ALIGNMENT_PARAM(\
        TYPE,\
        CDR_TYPE,\
        SERIALIZE_MACRO,\
        ALIGNMENT)\
template <>\
struct primitive_type_traits<TYPE> {\
    static void serialize_fast(RTIXCdrStream *stream, const TYPE* v)\
    {\
        RTIXCdrStream_ ## SERIALIZE_MACRO ## ByteFast(stream, (CDR_TYPE *) v);\
    }\
    static void deserialize_fast(RTIXCdrStream *stream, TYPE* v)\
    {\
        RTIXCdrStream_de ## SERIALIZE_MACRO ## ByteFast(stream, (CDR_TYPE *) v);\
    }\
    static bool serialize(RTIXCdrStream *stream, const TYPE* v)\
    {\
        return RTIXCdrStream_ ## SERIALIZE_MACRO ## Byte(stream, (CDR_TYPE *) v, RTI_XCDR_TRUE, ALIGNMENT);\
    }\
    static bool deserialize(RTIXCdrStream *stream, TYPE* v)\
    {\
        return RTIXCdrStream_de ## SERIALIZE_MACRO ## Byte(stream, (CDR_TYPE *) v, RTI_XCDR_TRUE, ALIGNMENT);\
    }\
}

#define RTI_XCDR_SPECIALIZE_PRIMITIVE_TYPE_TRAITS_NO_ALIGN_PARAM(\
        TYPE,\
        CDR_TYPE,\
        SERIALIZE_MACRO)\
template <>\
struct primitive_type_traits<TYPE> {\
    static void serialize_fast(RTIXCdrStream *stream, const TYPE* v)\
    {\
        RTIXCdrStream_ ## SERIALIZE_MACRO ## ByteFast(stream, (CDR_TYPE *) v);\
    }\
    static void deserialize_fast(RTIXCdrStream *stream, TYPE* v)\
    {\
        RTIXCdrStream_de ## SERIALIZE_MACRO ## ByteFast(stream, (CDR_TYPE *) v);\
    }\
    static bool serialize(RTIXCdrStream *stream, const TYPE* v)\
    {\
        return RTIXCdrStream_ ## SERIALIZE_MACRO ## Byte(stream, (CDR_TYPE *) v);\
    }\
    static bool deserialize(RTIXCdrStream *stream, TYPE* v)\
    {\
        return RTIXCdrStream_de ## SERIALIZE_MACRO ## Byte(stream, (CDR_TYPE *) v);\
    }\
}

// Base case handles T as an 4-byte integer (this is necessary for traditional
// C++ enums as well as C++11 enum classes)
template <typename T>
struct primitive_type_traits {
    RTI_XCDR_SPECIALIZE_PRIMITIVE_TYPE_TRAITS_BODY(T, RTIXCdr4Byte, serialize4)
};

RTI_XCDR_SPECIALIZE_PRIMITIVE_TYPE_TRAITS_NO_ALIGN_PARAM(\
        char, RTIXCdr1Byte, serialize1);

// The API can work with fixed-width integers (dds_cpp.2.0) or 
// RTI-defined integers (dds_cpp.1.0). We need to make this distinction because
// on some platforms these types may not be exactly the same as far as template 
// resolution. That is, compilers may or may not pick the template
// specialization for int32_t for an instantiation with int.
//
// If we define only one (e.g. int32_t) some compilers won't find a suitable
// definition for int. If we define both, some compilers will find duplicate 
// definitions for int/int32_t.
#if !defined(RTI_AVOID_FIXED_WIDTH_INTEGERS)
RTI_XCDR_SPECIALIZE_PRIMITIVE_TYPE_TRAITS_NO_ALIGN_PARAM(\
        uint8_t, RTIXCdr1Byte, serialize1);

RTI_XCDR_SPECIALIZE_PRIMITIVE_TYPE_TRAITS(\
        int16_t, RTIXCdr2Byte, serialize2);
RTI_XCDR_SPECIALIZE_PRIMITIVE_TYPE_TRAITS(\
        uint16_t, RTIXCdr2Byte, serialize2);

RTI_XCDR_SPECIALIZE_PRIMITIVE_TYPE_TRAITS(\
        int32_t, RTIXCdr4Byte, serialize4);
RTI_XCDR_SPECIALIZE_PRIMITIVE_TYPE_TRAITS(\
        uint32_t, RTIXCdr4Byte, serialize4);

typedef uint32_t length_t;        
#else
RTI_XCDR_SPECIALIZE_PRIMITIVE_TYPE_TRAITS_NO_ALIGN_PARAM(\
        unsigned char, RTIXCdr1Byte, serialize1);

RTI_XCDR_SPECIALIZE_PRIMITIVE_TYPE_TRAITS(\
        short, RTIXCdr2Byte, serialize2);
RTI_XCDR_SPECIALIZE_PRIMITIVE_TYPE_TRAITS(\
        unsigned short, RTIXCdr2Byte, serialize2);

// int and unsigned int, as well as enum are handled by the general case
    
typedef unsigned int length_t;
#endif

RTI_XCDR_SPECIALIZE_PRIMITIVE_TYPE_TRAITS(\
        float, RTIXCdr4Byte, serialize4);

RTI_XCDR_SPECIALIZE_PRIMITIVE_TYPE_TRAITS_W_ALIGNMENT_PARAM(\
        DDS_LongLong, RTIXCdr8Byte, serialize8, 4);

RTI_XCDR_SPECIALIZE_PRIMITIVE_TYPE_TRAITS_W_ALIGNMENT_PARAM(\
        DDS_UnsignedLongLong, RTIXCdr8Byte, serialize8, 4);
RTI_XCDR_SPECIALIZE_PRIMITIVE_TYPE_TRAITS_W_ALIGNMENT_PARAM(\
        double, RTIXCdr8Byte, serialize8, 4);

// RTI_XCDR_SPECIALIZE_PRIMITIVE_TYPE_TRAITS(
//         rti::core::LongDouble, RTI_CDR_LONG_DOUBLE_TYPE, RTIXCdr16Byte, serialize16Byte);

template <typename T>
void serialize_fast(RTIXCdrStream& stream, const T& value)
{
    primitive_type_traits<T>::serialize_fast(&stream, &value);
}

template <typename T>
void deserialize_fast(RTIXCdrStream& stream, T& value)
{
    primitive_type_traits<T>::deserialize_fast(&stream, &value);
}

class Stream {
public:
    Stream()
    {
        RTIXCdrStream_init(&stream_);
        RTIXCdrStream_set(&stream_, NULL, 0);

        stream_._tmpRelativeBuffer = NULL;
        stream_._needByteSwap = 0;
        stream_._endian = stream_._nativeEndian;
        stream_._encapsulationKind = 0;
        stream_._encapsulationOptions = 0;
    }

    explicit Stream(const RTIXCdrStream& c_stream)
    {
        stream_ = c_stream; // shallow assign
    }

    // Creates a new stream that shares the same buffer and current position 
    // as another Stream
    Stream(const Stream& other)
    {
        stream_ = other.stream_; // shallow assign
    }

    // Creates a new Stream from an existing already-initialized buffer, whose
    // encapsulation has already been set
    Stream(
            unsigned char *buffer,
            length_t size,
            length_t offset)
    {
        RTIXCdrFlatData_initializeStream(
                &stream_,
                buffer,
                offset,
                size);
    }

    Stream& operator=(const Stream& other)
    {
        stream_ = other.stream_; // shallow assign (see copy constructor)
        return *this;
    }

    bool skip(length_t size) 
    {
        if (!check_size(size)) {
            return false;
        }
        RTIXCdrStream_increaseCurrentPosition(&stream_, size);
        return true;
    }

    void skip_fast(length_t size) 
    {
        RTIXCdrStream_increaseCurrentPosition(&stream_, size);
    }

    void skip_back(length_t bytes)
    {
        RTIXCdrStream_increaseCurrentPosition(
                &stream_, 
                -static_cast<int>(bytes));
    }

    unsigned char * buffer()
    {
        return reinterpret_cast<unsigned char*>(stream_._buffer);
    }

    const unsigned char * buffer() const
    {
        return reinterpret_cast<unsigned char*>(stream_._buffer);
    }    

    unsigned char * current_position()
    {
        return reinterpret_cast<unsigned char*>(
                RTIXCdrStream_getCurrentPosition(&stream_));
    }

    void current_position(unsigned char * pos)
    {
        RTIXCdrStream_setCurrentPosition(
                &stream_, 
                reinterpret_cast<char*>(pos));
    }

    length_t used_size() const
    {
        return static_cast<length_t>(
                RTIXCdrStream_getCurrentPosition(&stream_) - stream_._buffer);
    }

    length_t remaining_size() const
    {
        return total_size() - used_size();
    }

    length_t total_size() const
    {
        return stream_._bufferLength;
    }

    bool needs_byte_swap() const
    {
        return stream_._needByteSwap == RTI_XCDR_TRUE;
    }

    template <typename T>
    bool serialize(T value)
    {
        return primitive_type_traits<T>::serialize(&stream_, &value);
    }

    template <typename T>
    void serialize_fast(T value)
    {
        primitive_type_traits<T>::serialize_fast(&stream_, &value);
    }

    template <typename T>
    bool serialize_no_align(T value)
    {
        if (!check_size(sizeof(T))) {
            return false;
        }
        primitive_type_traits<T>::serialize_fast(&stream_, &value);
        return true;
    }    

    template <typename T>
    T deserialize_fast()
    {
        T value;
        primitive_type_traits<T>::deserialize_fast(&stream_, &value);
        return value;
    }

    void serialize_fast(const void *array, length_t bytes)
    {
        RTIXCdrStream_serializeNByteFast(&stream_, array, bytes);
    }

    char * serialize_dheader()
    {
        return RTIXCdrStream_serializeDHeader(&stream_);
    }

    bool finish_dheader(char *dheader_position)
    {
        return RTIXCdrStream_serializeDHeaderLength(&stream_, dheader_position)
                == RTI_XCDR_TRUE;
    }

    bool serialize_emheader(
            char*& position,
            RTIXCdrUnsignedLong parameter_id,
            RTIXCdrUnsignedLong lc,
            bool must_understand = false)
    {
        RTIXCdrBoolean failure = 0;
        position = RTIXCdrStream_serializeV2ParameterHeader(
                &stream_,
                &failure,
                parameter_id,
                must_understand ? RTI_XCDR_TRUE : RTI_XCDR_FALSE,
                lc);

        return failure == RTI_XCDR_FALSE;
    }

    void finish_emheader(char * emheader_position)
    {
        RTIXCdrStream_finishV2ParameterHeader(&stream_, emheader_position);
    }

    bool align(unsigned int alignment)
    {
        return RTIXCdrStream_align(&stream_, (RTIXCdrAlignment) alignment)
                == RTI_XCDR_TRUE;
    }

    bool check_size(length_t size) const
    {
        bool result = RTIXCdrStream_checkSize(&stream_, size) == RTI_XCDR_TRUE;
        return result;
    }

    bool need_byte_swap() const
    {
        return stream_._needByteSwap != 0;
    }

    RTIXCdrStream& c_stream()
    {
        return stream_;
    }


    // Saves the current position of a stream so it can be restored later on if
    // needed. This utility has two main uses:
    //  - For Offsets: move the offset to a member, then go back
    //  - For Builders: allow rolling back to the state before starting building
    //      a member, in case of an error or a call to the builder's discard()
    // 
    // For maintainability we also save the relativeBuffer, but FlatData doesn't
    // use it.
    class Memento {
    public:
        explicit Memento(Stream& stream) :
                stream_(stream),
                original_position_(stream.current_position()),
                relative_buffer_(stream.c_stream()._relativeBuffer)
        {
        }

        // Restores the saved position unless discard() has been called
        ~Memento()
        {
            restore();
        }

        // Restores the saved position unless discard() has been called
        void restore()
        {
            if (original_position_ != NULL) {
                stream_.current_position(original_position_);
                stream_.c_stream()._relativeBuffer = relative_buffer_;
            }            
        }

        // Call this to no longer restore the original position
        unsigned char * discard()
        {
            unsigned char *pos = original_position_;
            original_position_ = NULL;
            return pos;
        }

    private:
        Stream& stream_;
        unsigned char *original_position_;
        char *relative_buffer_;
    };


private:
    RTIXCdrStream stream_;
};

} } // namespace rti::xcdr

#endif // RTI_XCDR_STREAM_HPP_
