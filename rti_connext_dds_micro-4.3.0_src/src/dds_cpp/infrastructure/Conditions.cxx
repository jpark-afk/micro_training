/* 

 (c) Copyright, Real-Time Innovations, 2013-2024.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.

 modification history
---------------------
19may2015,as  Removed unsupported functions copy() and is_equal()
16may2014,as  MICRO-794 Remove C++ TODO and commented out code
08nov2013,as  MICRO-681 Complete implementation of WaitSets
              and support for StatusConditions
19jul2013,as  Major C++ update
11jan2013,eh  Created
===================================================================== */

#ifndef dds_cpp_infrastructure_hxx
  #include "dds_cpp/dds_cpp_infrastructure.hxx"
#endif

#include "Conditions.hxx"

/*** SOURCE_BEGIN ***/

// ---------------------------------------------------------------------
// Public Methods
// ---------------------------------------------------------------------

DDSConditionSeq::DDSConditionSeq()
{
    DDS_ConditionSeq_initialize(&this->_c_seq);
}

DDSConditionSeq::~DDSConditionSeq()
{
#ifndef RTI_CERT
    DDS_ConditionSeq_finalize(&this->_c_seq);
#endif /* !RTI_CERT */
}

DDSConditionSeq::DDSConditionSeq(const DDSConditionSeq&)
{

}

RTI_INT32
DDSConditionSeq::maximum() const
{
    return DDS_ConditionSeq_get_maximum(&this->_c_seq);
}

bool
DDSConditionSeq::maximum(RTI_INT32 new_max)
{
    return (DDS_ConditionSeq_set_maximum(&this->_c_seq, new_max) == RTI_TRUE);
}

RTI_INT32
DDSConditionSeq::length() const
{
    return DDS_ConditionSeq_get_length(&this->_c_seq);
}

bool
DDSConditionSeq::length(RTI_INT32 new_length)
{
    return (DDS_ConditionSeq_set_length(&this->_c_seq, new_length) == RTI_TRUE);
}

bool
DDSConditionSeq::copy(const DDSConditionSeq& src_seq)
{
#ifndef RTI_CERT
    DDS_ConditionSeq *result =
            DDS_ConditionSeq_copy(&this->_c_seq, &src_seq._c_seq);
    return (result != NULL);
#else
    UNUSED_ARG(src_seq);
    return false;
#endif
}

DDSConditionSeq&
DDSConditionSeq::operator=(const DDSConditionSeq& src_seq)
{
#ifdef RTI_CERT
    UNUSED_ARG(src_seq);
#else
    this->copy(src_seq);
#endif
    return *this;
}

bool
DDSConditionSeq::is_equal(const DDSConditionSeq& src_seq) const
{
#ifndef RTI_CERT
    RTI_BOOL result =
            DDS_ConditionSeq_is_equal(&this->_c_seq, &src_seq._c_seq);
    return (result == RTI_TRUE);
#else
    UNUSED_ARG(src_seq);
    return false;
#endif
}

bool
DDSConditionSeq::operator==(const DDSConditionSeq& other) const
{
    return this->is_equal(other);
}

bool
DDSConditionSeq::operator!=(const DDSConditionSeq& other) const
{
    return !(this->is_equal(other));
}

DDSCondition**
DDSConditionSeq::get_reference(RTI_INT32 i)
{
    DDS_Condition** c_cond_ref =
            DDS_ConditionSeq_get_reference(&this->_c_seq,i);

    /* If we were returned a NULL ref from C,
     * let's return that.
     */
    if (c_cond_ref == NULL)
    {
        return NULL;
    }

    return (DDSCondition**)DDS_ConditionImpl_get_wrapper_ref(*c_cond_ref);
}

DDSCondition*&
DDSConditionSeq::operator[](RTI_INT32 i)
{
    return *(this->get_reference(i));
}

DDSCondition *const &
DDSConditionSeq::operator[](RTI_INT32 i) const
{
    DDS_Condition** c_cond_ref =
            DDS_ConditionSeq_get_reference(&this->_c_seq,i);

    static DDSCondition* null_condition = NULL;

    if (c_cond_ref == NULL)
    {
        return null_condition;
    }

    return *(DDSCondition **)DDS_ConditionImpl_get_wrapper_ref(*c_cond_ref);
}

bool
DDSConditionSeq::ensure_length(RTI_INT32 len, RTI_INT32 max)
{
#ifndef RTI_CERT
    return (RTI_TRUE == DDS_ConditionSeq_ensure_length(&this->_c_seq,len,max));
#else
    UNUSED_ARG(len);
    UNUSED_ARG(max);
    return false;
#endif
}

DDSCondition::DDSCondition(DDS_Condition *c_cond) :
        _c_cond(c_cond)
{

}

DDSCondition::~DDSCondition()
{
    this->_c_cond = NULL;
}

bool
DDSCondition::get_trigger_value()
{
    return DDS_Condition_get_trigger_value(this->_c_cond) == DDS_BOOLEAN_TRUE;
}

DDSGuardCondition::DDSGuardCondition() :
        DDSCondition(NULL)
{
    this->_c_cond = (DDS_Condition*) DDS_GuardCondition_new();
    if (this->_c_cond != NULL)
    {
        DDS_ConditionImpl_set_wrapper(this->_c_cond, this);
    }

}

DDSGuardCondition::~DDSGuardCondition()
{
#ifndef RTI_CERT
    DDS_GuardCondition_delete((DDS_GuardCondition*)this->_c_cond);
#endif
}

DDS_ReturnCode_t
DDSGuardCondition::set_trigger_value(bool value)
{
    return DDS_GuardCondition_set_trigger_value(
                (DDS_GuardCondition*)this->_c_cond,value?DDS_BOOLEAN_TRUE:DDS_BOOLEAN_FALSE);
}

DDSStatusCondition::DDSStatusCondition(DDS_StatusCondition *c_cond) :
        DDSCondition(DDS_StatusCondition_as_condition(c_cond))
{

}

DDSStatusCondition::~DDSStatusCondition()
{

}

DDS_StatusMask
DDSStatusCondition::get_enabled_statuses()
{
    return DDS_StatusCondition_get_enabled_statuses(
                    (DDS_StatusCondition*)this->_c_cond);
}

DDS_ReturnCode_t
DDSStatusCondition::set_enabled_statuses(DDS_StatusMask mask)
{
    return DDS_StatusCondition_set_enabled_statuses(
                    (DDS_StatusCondition*)this->_c_cond,mask);
}

DDSEntity*
DDSStatusCondition::get_entity()
{
    DDS_Entity *c_entity = DDS_StatusCondition_get_entity(
                                    (DDS_StatusCondition*)this->_c_cond);
    return (DDSEntity*)DDS_Entity_get_wrapper(c_entity);
}


/* We must implement the default constructors of any C sequences that
   do not have a similarly named C++ sequence so that it can be
   dllexport'ed for Windows even though never used.  This is because
   we are using the same DDS_SEQUENCE #define macro to define both the
   C and C++ sequences.  By doing so, when including the definition of
   a C sequence from C++, we are declaring methods in the sequence
   struct to be dllexport'ed that are not implemented, of which the
   default constructor is an issue as the implementation is required.
   If we can refactor the sequence so that the macro definition is
   different for C and C++, this will no longer be needed. */

DDS_ConditionSeq::DDS_ConditionSeq() :
     _contiguous_buffer(NULL),
    _maximum(0),
    _length(0),
    _element_size(0),
    _token1(NULL),
    _token2(NULL),
    _flags(0)
{
   
}

DDS_ConditionSeq::DDS_ConditionSeq(const DDS_ConditionSeq&) :
    _contiguous_buffer(NULL),
    _maximum(0),
    _length(0),
    _element_size(0),
    _token1(NULL),
    _token2(NULL),
    _flags(0)
{
    
}

DDS_ConditionSeq::~DDS_ConditionSeq()
{
}



#undef DDS_CURRENT_SUBMODULE    /* DDS_SUBMODULE_MASK_INFRASTRUCTURE */


