/*
(c) Copyright, Real-Time Innovations, 2018.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
*/


#ifndef RTI_DDS_FLAT_EXCEPTIONHELPER_HPP_
#define RTI_DDS_FLAT_EXCEPTIONHELPER_HPP_

//
// Enable debug assertions (RTI_FLAT_ASSERT) in debug mode
//
#if !defined(NDEBUG)
#define RTI_FLAT_DATA_ENABLE_DEBUG_ASSERTIONS
#endif

//
// Error handling without C++ exceptions (traditional C++ and Micro C++ APIs)
//
#ifdef RTI_FLAT_DATA_NO_EXCEPTIONS
    // TODO: use MSG
    #ifdef RTI_FLAT_DATA_ENABLE_DEBUG_ASSERTIONS
        #define RTI_FLAT_ASSERT(COND, ACTION) \
            if (!(COND)) { \
                RTIXCdrFlatData_logPreconditionError(__FILE__, __LINE__, "Assertion failure: " #COND); \
                ACTION; \
            }
    #else
        #define RTI_FLAT_ASSERT(COND, ACTION)
    #endif

    // CR decission: Log error messages. Export function for each log kind (serialize, deserialize, precondition, skip)
    // pass type/field (if possible) to serialization errors
    #define RTI_FLAT_OFFSET_CHECK_NOT_NULL(ACTION) \
        if (this->is_null()) { \
            RTIXCdrFlatData_logPreconditionError(__FILE__, __LINE__, "null offset"); \
            ACTION; \
        }

    #define RTI_FLAT_OFFSET_PRECONDITION_ERROR(MSG, ACTION) \
            RTIXCdrFlatData_logPreconditionError(__FILE__, __LINE__, MSG); \
            ACTION

    #define RTI_FLAT_CHECK_PRECONDITION(COND, ACTION) \
        if (!(COND))  { \
            RTIXCdrFlatData_logPreconditionError(__FILE__, __LINE__, "Precondition failure: " #COND); \
            ACTION; \
        }

    #define RTI_FLAT_BUILDER_CHECK_NOT_BOUND(ACTION) \
        if (this->bind_position_)  { \
            RTIXCdrFlatData_logPreconditionError(__FILE__, __LINE__, "Builder is currently bound - call finish() first"); \
            ACTION; \
        }

    #define RTI_FLAT_BUILDER_CHECK_CAN_FINISH(ACTION) \
        if (!this->is_nested())  { \
            RTIXCdrFlatData_logPreconditionError(__FILE__, __LINE__, "This is not a member builder; call finish_sample() instead"); \
            ACTION; \
        }

    #define RTI_FLAT_BUILDER_CHECK_CAN_FINISH_SAMPLE(ACTION) \
        if (this->is_nested())  { \
            RTIXCdrFlatData_logPreconditionError(__FILE__, __LINE__, "This is a member builder; call finish() instead"); \
            ACTION; \
        }

    #define RTI_FLAT_BUILDER_CHECK_VALID(ACTION) \
        if (!this->is_valid()) { \
            RTIXCdrFlatData_logPreconditionError(__FILE__, __LINE__, "This Builder is not valid"); \
            ACTION; \
        }

    #define RTI_FLAT_BUILDER_CHECK_CREATE_BUILDER(BUILDER, ACTION) \
        if (!BUILDER.is_valid()) { \
            this->set_failure(); \
            RTIXCdrFlatData_logCreationError(__FILE__, __LINE__, "Builder"); \
            ACTION; \
        }

    #define RTI_FLAT_BUILDER_OUT_OF_RESOURCES_ERROR(ACTION) \
        this->set_failure(); \
        RTIXCdrFlatData_logBuilderOutOfResources(__FILE__, __LINE__); \
        ACTION

    #define RTI_FLAT_BUILDER_PRECONDITION_ERROR(MSG, ACTION) \
        this->set_failure(); \
        RTIXCdrFlatData_logPreconditionError(__FILE__, __LINE__, MSG); \
        ACTION

//
// Error handling with C++ exceptions (modern C++ API)
//
#else // !RTI_FLAT_DATA_NO_EXCEPTIONS

    #ifdef RTI_FLAT_DATA_ENABLE_DEBUG_ASSERTIONS
        #define RTI_FLAT_ASSERT(COND, ACTION) \
            if (!(COND)) throw dds::core::PreconditionNotMetError("Assertion failure: " #COND)
    #else
        #define RTI_FLAT_ASSERT(COND, ACTION)
    #endif

    #define RTI_FLAT_OFFSET_CHECK_NOT_NULL(ACTION) \
        if (this->is_null()) throw dds::core::NullReferenceError("null offset")

    #define RTI_FLAT_OFFSET_PRECONDITION_ERROR(MSG, ACTION) \
            throw dds::core::PreconditionNotMetError(MSG)

    #define RTI_FLAT_CHECK_PRECONDITION(COND, ACTION) \
        if (!(COND)) throw dds::core::PreconditionNotMetError("Precondition failure: " #COND)

    #define RTI_FLAT_BUILDER_CHECK_NOT_BOUND(ACTION) \
        if (bind_position_) \
            throw dds::core::PreconditionNotMetError( \
                "Builder is currently bound - call finish() first")

    #define RTI_FLAT_BUILDER_CHECK_CAN_FINISH(ACTION) \
        if (!is_nested()) \
            throw dds::core::PreconditionNotMetError( \
                "This is not a member builder; call finish_sample() instead")  

    #define RTI_FLAT_BUILDER_CHECK_CAN_FINISH_SAMPLE(ACTION) \
        if (is_nested()) \
            throw dds::core::PreconditionNotMetError( \
                "This is a member builder; call finish() instead")                    

    #define RTI_FLAT_BUILDER_CHECK_VALID(ACTION) \
        if (!is_valid()) \
            throw dds::core::PreconditionNotMetError("This Builder is not valid")  

    // With exceptions, nothing to check (exception would have been thrown)
    #define RTI_FLAT_BUILDER_CHECK_CREATE_BUILDER(BUILDER, ACTION)

    #define RTI_FLAT_BUILDER_OUT_OF_RESOURCES_ERROR(ACTION) \
        throw dds::core::OutOfResourcesError("builder reached end of buffer")

    #define RTI_FLAT_BUILDER_PRECONDITION_ERROR(MSG, ACTION) \
        throw dds::core::PreconditionNotMetError(MSG)


#endif // RTI_FLAT_DATA_NO_EXCEPTIONS

//
// Error checking (other than assertions) in release mode is enabled unless
// the sources are compiled with -DRTI_FLAT_DATA_DISABLE_ERROR_CHECKING
//
// In that case, most error checks that deal with preconditions due to incorrect
// usage of the API are disabled in release mode. This would lead to undefined
// behavior in case of bad usage, but can improve performance.
//
#if defined(RTI_FLAT_DATA_ENABLE_DEBUG_ASSERTIONS) \
    || !defined(RTI_FLAT_DATA_DISABLE_ERROR_CHECKING)
#define RTI_FLAT_DATA_ENABLE_ERROR_CHECKING
#endif

#ifndef RTI_FLAT_DATA_ENABLE_ERROR_CHECKING

    #define RTI_FLAT_OFFSET_CHECK_NOT_NULL(ACTION)
    #define RTI_FLAT_ASSERT(COND, ACTION)
    #define RTI_FLAT_CHECK_PRECONDITION(COND, ACTION)
    #define RTI_FLAT_BUILDER_CHECK_NOT_BOUND(ACTION)
    #define RTI_FLAT_BUILDER_CHECK_CAN_FINISH(ACTION)
    #define RTI_FLAT_BUILDER_CHECK_CAN_FINISH_SAMPLE(ACTION)
    #define RTI_FLAT_BUILDER_CHECK_VALID(ACTION)
    #define RTI_FLAT_BUILDER_CHECK_CREATE_BUILDER(BUILDER, ACTION)

#endif // RTI_FLAT_DATA_ENABLE_ERROR_CHECKING

namespace rti { namespace flat { namespace detail {

} } }

#endif // RTI_DDS_FLAT_EXCEPTIONHELPER_HPP_

