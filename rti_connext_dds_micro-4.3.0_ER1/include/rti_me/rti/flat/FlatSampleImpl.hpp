/*
(c) Copyright, Real-Time Innovations, 2018.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
*/

#ifndef RTI_DDS_FLAT_FLATSAMPLEIMPL_HPP_
#define RTI_DDS_FLAT_FLATSAMPLEIMPL_HPP_

// This file includes the implementation of some templates that need to
// appear after some types are defined in the generated code. This is a
// a restriction on some platforms such as pentiumInty11.pcx64 and other Intys,
// sparcSol2.10gcc3.4.2.

// For example, type_programs<T> needs to be defined (as an specialization 
// for T) before its used in create_data().

#include "rti/flat/FlatSample.hpp"

namespace rti { namespace flat {

namespace detail {

template <typename OffsetType>
Sample<OffsetType>* create_data_impl(fixed_size_type_tag_t)
{
    unsigned char *buffer = RTIXCdrFlatData_createSampleFinal(
            OffsetType::serialized_size(0),
            rti::xcdr::type_programs<Sample<OffsetType> >::get());

    if (buffer == NULL) {
        #ifndef RTI_FLAT_DATA_NO_EXCEPTIONS
        throw std::bad_alloc();
        #else
        return NULL;
        #endif
    }

    return reinterpret_cast<rti::flat::Sample<OffsetType> *>(buffer);
}

template <typename OffsetType>
Sample<OffsetType>* create_data_impl(variable_size_type_tag_t)
{
    unsigned char *buffer = RTIXCdrFlatData_createSampleMutable(
            rti::xcdr::type_programs<Sample<OffsetType> >::get());

    if (buffer == NULL) {
        #ifndef RTI_FLAT_DATA_NO_EXCEPTIONS
        throw std::bad_alloc();
        #else
        return NULL;
        #endif
    }

    return reinterpret_cast<rti::flat::Sample<OffsetType> *>(buffer);
}

}

// Declared in FlatSample.hpp
template <typename OffsetType>
Sample<OffsetType>* Sample<OffsetType>::create_data()
{
    return detail::create_data_impl<OffsetType>(
            typename OffsetType::offset_kind());
}

} }

#endif // RTI_DDS_FLAT_FLATSAMPLEIMPL_HPP_

