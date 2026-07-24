/*
(c) Copyright, Real-Time Innovations, 2018.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
*/

#ifndef RTI_DDS_FLAT_SEQUENCEITERATOR_HPP_
#define RTI_DDS_FLAT_SEQUENCEITERATOR_HPP_

#include <iterator>

#include "rti/flat/Offset.hpp"

namespace rti { namespace flat {

/**
 * @ingroup RTIFlatOffsetsModule
 * @brief Iterator for collections of Offsets
 * 
 * @tparam E The Offset type of the elements
 * @tparam OffsetKind (implementation detail)
 * 
 * @note This type should not be declared directly. For example, for a 
 * SequenceOffset, use SequenceOffset::iterator (or simply \p auto).
 * 
 * A SequenceIterator moves through the Offset to elements of a sequence or
 * array \ref RTIFlatOffsetsModule "Offset", and provides the functionality of
 * a standard \p forward_iterator.
 * 
 * Note that a dereferenced SequenceIterator returns <b>by value</b> the Offset 
 * to the current element; it doesn't return it by reference. 
 * 
 * \if CPP_LANGUAGE_ONLY
 * Since the Traditional C++ API doesn't use exceptions, it is recommended
 * to use the function advance() instead of operator++ to check for errors
 * \endif
 * 
 */
template <typename E, typename OffsetKind>
class SequenceIterator {
public:
    /**
     * @brief The iterator category
     */
    typedef std::forward_iterator_tag iterator_category;

    /**
     * @brief The element type
     */
    typedef E value_type;

    /**
     * @brief The reference type is the same as the value type, an Offset
     */
    typedef value_type reference;

    /**
     * @brief The pointer type is the same as the value type, an Offset
     */    
    typedef value_type pointer;

    /**
     * @brief The difference type
     */    
    typedef std::ptrdiff_t difference_type;

    /**
     * @brief Constructs an invalid iterator
     */
    SequenceIterator() : sample_(NULL), current_offset_(0), max_offset_(0)
    {
    }

    SequenceIterator(
            rti::flat::SampleBase *sample,  
            offset_t initial_offset,
            offset_t max_offset)
        : sample_(sample), 
          current_offset_(initial_offset), 
          max_offset_(max_offset)
    {
        RTI_FLAT_ASSERT(sample != NULL, return);
    }

    /**
     * @brief Returns whether the iterator is invalid
     */
    bool is_null() const
    {
        return sample_ == NULL;
    }

    /**
     * @brief Returns the Offset of the current element
     * 
     * @return The Offset to the current element. Note that this function returns
     * by value, not by reference. This Offset may be 
     * \ref rti::flat::OffsetBase::is_null "null" if the iterator has surpassed 
     * the length of the collection. See \ref OffsetErrorManagement.
     */
    value_type operator*() const 
    {
        return get_impl(OffsetKind());
    }

    /**
     * @brief Returns the Offset of the current element
     */
    value_type operator->() const 
    {
        return get_impl(OffsetKind());
    }

    /**
     * @brief Advances to the next element, reporting any errors by returning false
     * 
     * \if CPP_LANGUAGE_ONLY
     * Since the Traditional C++ API doesn't support exceptions, this function
     * is recommended instead of operator++.
     * \endif
     * \if CPP2_LANGUAGE_ONLY
     * Unlike operator++, which throws an exception in case of error, advance()
     * returns false
     * \endif
     * 
     * @return True if the function succeeds, or false if there was an error
     */ 
    bool advance()
    {
        // Unlike operator++, advance() allows checking for errors when
        // exceptions are disabled (traditional C++)
        return advance_impl(OffsetKind());
    }

    /**
     * @brief Advances to the next element
     * 
     * \if CPP_LANGUAGE_ONLY
     * @warning Since the Traditional C++ API doesn't use exceptions, use
     * advance() instead of this operator to check for errors.
     * \endif
     */
    SequenceIterator& operator++()
    {
        bool can_advance_iter = advance_impl(OffsetKind());
        RTI_FLAT_CHECK_PRECONDITION(can_advance_iter, return *this);
        return *this;
    }

    /**
     * @brief Advances to the next element
     * 
     * \if CPP_LANGUAGE_ONLY
     * @warning Since the Traditional C++ API doesn't use exceptions, use
     * advance() instead of this operator to check for errors.
     * \endif
     */
    SequenceIterator operator++(int) 
    {
        SequenceIterator tmp(*this);
        ++(*this);
        return tmp;
    }

    /**
     * @brief Compares two iterators
     */
    friend bool operator<(
            const SequenceIterator & s1,
            const SequenceIterator & s2) 
    {
        return s1.get_position() < s2.get_position();
    }

    /**
     * @brief Compares two iterators
     */
    friend bool operator > (
            const SequenceIterator & s1,
            const SequenceIterator & s2) 
    {
        return s1.get_position() > s2.get_position();
    }

    /**
     * @brief Compares two iterators
     */
    friend bool operator <= (
            const SequenceIterator & s1,
            const SequenceIterator & s2) 
    {
        return s1.get_position() <= s2.get_position();
    }

    /**
     * @brief Compares two iterators
     */
    friend bool operator >= (
            const SequenceIterator & s1,
            const SequenceIterator & s2) 
    {
        return s1.get_position() >= s2.get_position();
    }

    /**
     * @brief Compares two iterators
     */
    friend bool operator == (
            const SequenceIterator & s1,
            const SequenceIterator & s2) 
    {
        return s1.get_position() == s2.get_position();
    }

    /**
     * @brief Compares two iterators
     */
    friend bool operator != (
            const SequenceIterator & s1,
            const SequenceIterator & s2) 
    {
        return !(s1 == s2);
    }

    unsigned char * get_position() const
    {
        if (is_null()) {
            return NULL;
        } else {
            return sample_->get_buffer() + current_offset_;
        }
    }    

private:
    E get_impl(variable_size_type_tag_t) const
    {   
        if (is_null()) {
            return E();
        }

        // The iterator points to the end of the previous element. If elements
        // need padding, we have to add it here (required alignment is 4 because
        // mutable types (variable_size_type_tag_t) are always aligned to 4)
        offset_t offset = RTIXCdrAlignment_alignSizeUp(current_offset_, 4);

        if (offset >= max_offset_) {
            return E();
        }

        // Calculate the size of the current element; if E is a struct type,
        // this will be quick (just deserialize the DHeader); if we're itearating
        // over an array of sequences (E is a sequence), this will require
        // skipping over the current sequence.
        offset_t size = E::Helper::calculate_serialized_size(
                sample_, 
                offset,
                max_offset_);
        // size 0 indicates an error; a variable-size type E cannot have size
        // zero because it will always have a header (DHeader for mutable structs,
        // length for sequences)
        if (size == 0) {
            return E();
        }

        return E(sample_, offset, size);
    }

    E get_impl(fixed_size_type_tag_t) const
    {
        if (is_null()) {
            return E();
        }

        // The iterator points to the end of the previous element. If elements
        // need padding, we have to add it here.
        offset_t offset = RTIXCdrAlignment_alignSizeUp(
                current_offset_, 
                E::required_alignment);
        if (offset >= max_offset_) {
            return E();
        }

        return E(sample_, offset);        
    }

    bool advance_impl(variable_size_type_tag_t)
    {
        if (is_null()) {
            return false;
        }

        current_offset_ = RTIXCdrAlignment_alignSizeUp(current_offset_, 4);
        offset_t size = E::Helper::calculate_serialized_size(
                sample_, // beginning of the buffer
                current_offset_, // absolute offset to the current element
                max_offset_); // maximum offset in the list
        if (size == 0) {
            return false;
        }

        current_offset_ += size;

        if (current_offset_ > max_offset_) {
            current_offset_ = max_offset_;
            return false;
        }

        return true;
    }

    bool advance_impl(fixed_size_type_tag_t)
    {
        if (is_null()) {
            return false;
        }

        // The iterator points to the end of the previous element. If elements
        // need padding, we have to add it here. We do that to keep the end()
        // iterator consistent, becuase the last element may not need padding.
        current_offset_ = RTIXCdrAlignment_alignSizeUp(
                current_offset_, 
                E::required_alignment);        
        current_offset_ += E::serialized_size(0);

        if (current_offset_ > max_offset_) {
            current_offset_ = max_offset_;
            return false;
        }

        return true;
    }

    rti::flat::SampleBase *sample_;
    offset_t current_offset_;
    offset_t max_offset_;
};

} }

#endif // RTI_DDS_FLAT_SEQUENCEITERATOR_HPP_

