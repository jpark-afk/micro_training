/*
(c) Copyright, Real-Time Innovations, 2018.
All rights reserved.

No duplications, whole or partial, manual or electronic, may be made
without express written permission.  Any such copies, or
revisions thereof, must display this notice unaltered.
This code contains trade secrets of Real-Time Innovations, Inc.
*/

#ifndef RTI_DDS_FLAT_AGGREGATIONBUILDERS_HPP_
#define RTI_DDS_FLAT_AGGREGATIONBUILDERS_HPP_

#include "rti/flat/BuilderHelper.hpp"
#include "rti/flat/Builder.hpp"
#include "xcdr/xcdr_interpreter.h"

namespace rti { namespace flat {

/*
 * Parameters describing a member to be added or built.
 *
 * The information encoded by this structure is used to set the appropriate
 * bits in the header that precedes each member of a mutable type.
 */
struct MemberParams {
    /*
     * Indicates whether the member is part of the key.
     */
    bool key;
    /*
     * Indicates whether the member is the discriminator of a union.
     */
    bool discriminator;
    /*
     * Indicates whether the member has been marked as "must understand".
     */
    bool must_understand;

    MemberParams() : key(false), discriminator(false), must_understand(false)
    { }

    /*
     * Check whether the current XCDR2 compliance mask allows the
     * specified features.
     *
     * This function will cache the value of the compliance mask to avoid
     * repeated nested function calls at runtime.
     *
     * @param check_mask The feature bits to check in the compliance mask.
     * @param reload_mask If true, force reloading of the compliance mask
     * from the global configuration.
     * @return true if all the bits in check_mask are set in the compliance
     * mask.
     */
    static bool xcdr2_compliant(
            const RTIXCdrXTypesComplianceMask check_mask,
            const bool reload_mask)
    {
        static bool g_compliance_mask_set = false;
        static RTIXCdrXTypesComplianceMask g_compliance_mask = 0;

        // Initialize the compliance mask on first use
        // There could be a data race here but the worst that can happen is
        // that the compliance mask is read more than once.
        // The goal is to avoid nested function calls on each and every call.
        if (reload_mask || !g_compliance_mask_set) {
            g_compliance_mask =
                    RTIXCdrInterpreter_getGlobalXtypeComplianceMask();
            g_compliance_mask_set = true;
        }

        return (g_compliance_mask & check_mask);
    }

    /*
     * Check if the must_understand bit should be set in the header for this
     * member.
     *
     * The bit will always be set if the must_understand attribute is true.
     * Additionally, if the current XCDR2 compliance mask allows it, the bit
     * will also be set for key members and discriminators.
     *
     * @return true if the must_understand bit should be set.
     */
    bool has_must_understand_bit() const
    {
        return must_understand
                || (xcdr2_compliant(
                            RTI_XCDR_XTYPES_MUST_UNDERSTAND_IN_KEY_AND_DISCRIMINATOR_BIT,
                            false)
                    && (key || discriminator));
    }
};

/*
 * Provides the common functionality for builders of aggregated types
 * 
 * Extends and specializes AbstractBuilder to add or override the functionality
 * required to build an aggregated type:
 * 
 * - Add or build members, prepending the member header
 * - Add dheader at the beginning
 * - Add parameter-list-end header at the end
 */
/**
 * @ingroup RTIFlatBuildersModule
 * @brief Base class of struct and union builders.
 * 
 * This class contains implementation details and doesn't add any public function
 * to AbstractBuilder. See MyFlatMutableBuilder for a concrete example of a
 * struct builder.
 */
class AggregationBuilder : public AbstractBuilder {
protected:
    AggregationBuilder() :
            dheader_position_(NULL),
            emheader_position_(NULL)
    {
    }

    // Create a new top-level builder
    AggregationBuilder(
            unsigned char *initial_buffer, 
            offset_t size,
            bool initialize_members) : 
        AbstractBuilder(initial_buffer, size, initialize_members),
        dheader_position_(NULL),
        emheader_position_(NULL)
    {
        dheader_position_ = stream().serialize_dheader();
        if (dheader_position_ == NULL) {
            RTI_FLAT_BUILDER_OUT_OF_RESOURCES_ERROR(invalidate());
        }
    }

public:
    // For internal use and testing
    //
    // Modifies the encapsulation set in the constructor with a custom value.
    // This function can only be called after the construction of a non-nested
    // Builder, before any member is built or added.
    //
    void set_encapsulation_impl(RTIXCdrEncapsulationId encapsulation_id)
    {
        // Only a top-level Builder can change the encapsulation
        RTI_FLAT_CHECK_PRECONDITION(!is_nested(), return);

        char *current_position = (char *) owned_stream_.current_position();
        // This operation is only valid right after construction, when no
        // member has been added yet
        RTI_FLAT_CHECK_PRECONDITION(
                current_position == dheader_position_ + RTI_XCDR_DHEADER_SIZE, 
                return);

        owned_stream_.skip_back(
                RTI_XCDR_DHEADER_SIZE + RTI_XCDR_ENCAPSULATION_HEADER_SIZE);

        if (!RTIXCdrFlatSample_initializeEncapsulationAndStream(
                (char *) owned_stream_.buffer(), 
                &owned_stream_.c_stream(),
                encapsulation_id, 
                owned_stream_.total_size())) {
            RTI_FLAT_BUILDER_OUT_OF_RESOURCES_ERROR(return);
        }

        stream().serialize_dheader();
    }
    
protected:

#if defined(RTI_FLAT_DATA_CXX11_RVALUE_REFERENCES)
    AggregationBuilder(AggregationBuilder&& other) = default;

    AggregationBuilder& operator=(AggregationBuilder&& other)
    {
        if (this == &other) {
            return *this;
        }

        finish_untyped_impl();

        AbstractBuilder::operator=(static_cast<AbstractBuilder&&>(other));

        dheader_position_ = other.dheader_position_;
        emheader_position_ = other.emheader_position_;

        return *this;
    }
#else
public:
    // Enable the safe-move-constructor idiom without C++11 move constructors
    struct UntypedAggregationBuilderMoveProxy : AbstractBuilderMoveProxy {
        char * dheader_position_;
        char * emheader_position_;
    };

    operator UntypedAggregationBuilderMoveProxy () throw() // move-constructor idiom
    {
        UntypedAggregationBuilderMoveProxy other;
        move_to(other);
        return other;
    }
    
protected:
    void move_from(UntypedAggregationBuilderMoveProxy& other)
    {
        AbstractBuilder::move_from(other);
        dheader_position_ = other.dheader_position_;
        emheader_position_ = other.emheader_position_;
    }

    void move_to(UntypedAggregationBuilderMoveProxy& other)
    {
        AbstractBuilder::move_to(other);
        other.dheader_position_ = dheader_position_;
        other.emheader_position_ = emheader_position_;
    }
#endif

protected:
    virtual ~AggregationBuilder()
    {
        // AggregationBuilder::finish_untyped_impl() is specialized
        // with respect to AbstractBuilder::finish_untyped_impl()
        finish_untyped_impl();
    }

    template <typename T>
    bool add_primitive_member(
            member_id_t id,
            T value,
            const MemberParams& params)
    {
        RTI_FLAT_BUILDER_CHECK_VALID(return false);

        rti::xcdr::Stream::Memento stream_memento(stream());

        if (!begin_emheader(id, detail::primitive_lc_code<T>::value, params)) {
            RTI_FLAT_BUILDER_OUT_OF_RESOURCES_ERROR(return false);
        }
        // no need to call finish_emheader()

        if (!stream().serialize_no_align(value)) {
            RTI_FLAT_BUILDER_OUT_OF_RESOURCES_ERROR(return false);
        }

        stream_memento.discard(); // success: do not restore the current position
        return true;
    }

    template <typename T>
    bool add_primitive_member(member_id_t id, T value)
    {
        static const MemberParams default_params;
        return add_primitive_member(id, value, default_params);
    }

    template <typename OffsetType>
    OffsetType add_member(member_id_t id, const MemberParams& params)
    {
        RTI_FLAT_BUILDER_CHECK_VALID(return OffsetType());

        // save state in case of error
        rti::xcdr::Stream::Memento stream_memento(stream());
        return add_member<OffsetType>(id, stream_memento, params);
    }

    template <typename OffsetType>
    OffsetType add_member(member_id_t id)
    {
        RTI_FLAT_BUILDER_CHECK_VALID(return OffsetType());

        // save state in case of error
        rti::xcdr::Stream::Memento stream_memento(stream());
        static const MemberParams default_params;
        return add_member<OffsetType>(id, stream_memento, default_params);
    }

    template <typename OffsetType>
    OffsetType add_member(
            member_id_t id,
            rti::xcdr::Stream::Memento& stream_memento)
    {
        static const MemberParams default_params;
        return add_member<OffsetType>(id, stream_memento, default_params);
    }

    template <typename OffsetType>
    OffsetType add_member(
            member_id_t id,
            rti::xcdr::Stream::Memento& stream_memento,
            const MemberParams& params)
    {
        // fixed-size non-primitive members may have a dheader or not; when they
        // do the LC code is 5, otherwise it is 4.
        if (!begin_emheader(
                    id,
                    detail::is_fixed_type_w_dheader<OffsetType>::value() ? 5
                                                                         : 4,
                    params)) {
            return OffsetType();
        }

        OffsetType result = AbstractBuilder::add_element<OffsetType>();
        finish_emheader();

        stream_memento.discard(); // success - do not restore the previous state
        return result;
    }

    template <typename NestedBuilder>
    NestedBuilder build_member(member_id_t id, const MemberParams& params)
    {
        RTI_FLAT_BUILDER_CHECK_VALID(return NestedBuilder());

        // save state in case of error
        rti::xcdr::Stream::Memento stream_memento(stream());
        return build_member<NestedBuilder>(id, stream_memento, params);
    }

    template <typename NestedBuilder>
    NestedBuilder build_member(member_id_t id)
    {
        static const MemberParams default_params;
        return build_member<NestedBuilder>(id, default_params);
    }

    template <typename NestedBuilder>
    NestedBuilder build_member(
            member_id_t id, 
            rti::xcdr::Stream::Memento& stream_memento)
    {
        static const MemberParams default_params;
        return build_member<NestedBuilder>(id, stream_memento, default_params);
    }

    template <typename NestedBuilder>
    NestedBuilder build_member(
            member_id_t id,
            rti::xcdr::Stream::Memento& stream_memento,
            const MemberParams& params)
    {
        if (!begin_emheader(
                    id,
                    static_cast<RTIXCdrUnsignedLong>(
                            detail::lc_code<NestedBuilder>::value()),
                    params)) {
            return NestedBuilder();
        }

        return AbstractBuilder::build_element_no_align<NestedBuilder>(
                stream_memento);
    }

    unsigned char* finish_sample_impl()
    {
        RTI_FLAT_BUILDER_CHECK_VALID(return NULL);
        RTI_FLAT_BUILDER_CHECK_NOT_BOUND(return NULL);
        RTI_FLAT_BUILDER_CHECK_CAN_FINISH_SAMPLE(return NULL);

        if (!stream().finish_dheader(dheader_position_)) {
            RTI_FLAT_BUILDER_OUT_OF_RESOURCES_ERROR(return NULL);
        }

        unsigned char *return_sample = stream().buffer();
        invalidate();
        
        return return_sample;
    }    

    // Same as finish_impl() but it doesn't throw exceptions or return an Offset
    void finish_untyped_impl()
    {
        if (is_valid() && is_nested()) {
            stream().finish_dheader(dheader_position_);
            AbstractBuilder::finish_untyped_impl();
        }
    }

    template <typename OffsetType>
    OffsetType finish_impl()
    {
        RTI_FLAT_BUILDER_CHECK_VALID(return OffsetType());
        RTI_FLAT_BUILDER_CHECK_CAN_FINISH(return OffsetType());

        stream().finish_dheader(dheader_position_);
        return AbstractBuilder::finish_impl<OffsetType>();
    }

public:
    // For internal use; need to use rti::flat::discard_sample(writer, builder)
    unsigned char * discard_sample_impl()
    {
        RTI_FLAT_BUILDER_CHECK_VALID(return NULL);
        RTI_FLAT_BUILDER_CHECK_NOT_BOUND(return NULL);
        RTI_FLAT_BUILDER_CHECK_CAN_FINISH_SAMPLE(return NULL);
                
        unsigned char *buffer = this->buffer();
        invalidate();
        return buffer;
    }

private:
    void finish_member() // override noexcept
    {
        if (!is_valid()) {
            return;
        }

        finish_emheader();
        AbstractBuilder::finish_member();
    }

private:
    bool begin_emheader(
            member_id_t id,
            RTIXCdrUnsignedLong lc,
            const MemberParams& params)
    {
        RTI_FLAT_BUILDER_CHECK_NOT_BOUND(return false);

        if (!stream().serialize_emheader(
                    emheader_position_,
                    id,
                    lc,
                    params.has_must_understand_bit())) {
            RTI_FLAT_BUILDER_OUT_OF_RESOURCES_ERROR(return false);
        }

        return true;
    }

    void finish_emheader() // noexcept
    {
        if (emheader_position_ != NULL) {
            stream().finish_emheader(emheader_position_);
            emheader_position_ = NULL;
        }
    }

protected:
    // Create a nested builder
    AggregationBuilder(nested_tag_t, AbstractBuilder& parent, unsigned int alignment)
        : AbstractBuilder(nested_tag_t(), parent, alignment),
          dheader_position_(NULL),
          emheader_position_(NULL)
    {
        dheader_position_ = stream().serialize_dheader();
        if (dheader_position_ == NULL) {
            RTI_FLAT_BUILDER_OUT_OF_RESOURCES_ERROR(invalidate());            
        }
    }

private:
    char * dheader_position_;
    char * emheader_position_;
};

// Specializes an AggregationBuilder to enforce that only one member can
// be set at a time
/**
 * @ingroup RTIFlatBuildersModule
 * @brief Base class of builders for user-defined mutable unions.
 * 
 * Union builders can only add or build a single member.
 * 
 * This class contains implementation details and doesn't add any public function
 * to AbstractBuilder.
 */
template <typename Discriminator>
class UnionBuilder : public AggregationBuilder {
protected:
    UnionBuilder()
    {
    }

    // Create a new top-level builder
    UnionBuilder(
            unsigned char *initial_buffer, 
            offset_t size, 
            bool initialize_members) : 
        AggregationBuilder(initial_buffer, size, initialize_members)
    {
    }

    template <typename T>
    bool add_primitive_member(member_id_t id, Discriminator disc, T value)
    {
        RTI_FLAT_BUILDER_CHECK_VALID(return false);

        // Always start at the beginning and override any member or discriminator
        // that may have been set
        move_to_discriminator();
        rti::xcdr::Stream::Memento stream_memento(stream());

        if (!add_discriminator(disc)) {
            return false;
        }

        if (!AggregationBuilder::add_primitive_member(id, value)) {
            return false;
        }

        stream_memento.discard(); // success: do not restore the current position
        return true;
    }

    template <typename OffsetType>
    OffsetType add_member(member_id_t id, Discriminator disc)
    {
        RTI_FLAT_BUILDER_CHECK_VALID(return OffsetType());

        move_to_discriminator();
        rti::xcdr::Stream::Memento stream_memento(stream());
        if (!add_discriminator(disc)) {
            RTI_FLAT_BUILDER_OUT_OF_RESOURCES_ERROR(return OffsetType());
        }

        return AggregationBuilder::add_member<OffsetType>(id, stream_memento);
    }

    template <typename NestedBuilder>
    NestedBuilder build_member(member_id_t id, Discriminator disc)
    {
        RTI_FLAT_BUILDER_CHECK_VALID(return NestedBuilder());

        move_to_discriminator();
        rti::xcdr::Stream::Memento stream_memento(stream());
        add_discriminator(disc);

        return AggregationBuilder::build_member<NestedBuilder>(id, stream_memento);
    }

    // Create a nested builder
    UnionBuilder(nested_tag_t, AbstractBuilder& parent, unsigned int alignment)
        : AggregationBuilder(nested_tag_t(), parent, alignment)
    {
    }

private:
    void move_to_discriminator()
    {
        stream().current_position(begin_position() + RTI_XCDR_DHEADER_SIZE);
    }

    bool add_discriminator(Discriminator disc)
    {
        MemberParams params;
        params.discriminator = true;
        return AggregationBuilder::add_primitive_member(0, disc, params);
    }
};


} }

#endif  // RTI_DDS_FLAT_AGGREGATIONBUILDERS_HPP_
