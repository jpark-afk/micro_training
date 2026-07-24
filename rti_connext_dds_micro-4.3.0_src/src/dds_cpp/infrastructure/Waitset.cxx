/* 

 (c) Copyright, Real-Time Innovations, 2013-2015.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.

 modification history
---------------------

===================================================================== */

#ifndef dds_cpp_infrastructure_hxx
  #include "dds_cpp/dds_cpp_infrastructure.hxx"
#endif

/*** SOURCE_BEGIN ***/

DDSWaitSet::DDSWaitSet()
{
    _c_impl = DDS_WaitSet_new();
}

DDSWaitSet::~DDSWaitSet()
{
#ifndef RTI_CERT
    DDS_WaitSet_delete(_c_impl);
#endif /* !RTI_CERT */
}

DDS_ReturnCode_t
DDSWaitSet::wait(DDSConditionSeq& active_conditions,
                 const DDS_Duration_t& timeout)
{
    return DDS_WaitSet_wait(
                this->_c_impl,&active_conditions._c_seq,&timeout);
}

DDS_ReturnCode_t
DDSWaitSet::attach_condition(DDSCondition *cond)
{
    return DDS_WaitSet_attach_condition(this->_c_impl,cond->_c_cond);
}


DDS_ReturnCode_t
DDSWaitSet::detach_condition(DDSCondition* cond)
{
#ifndef RTI_CERT
    return DDS_WaitSet_detach_condition(this->_c_impl,cond->_c_cond);
#else
    UNUSED_ARG(cond);
    return DDS_RETCODE_UNSUPPORTED;
#endif /* !RTI_CERT */
}


DDS_ReturnCode_t
DDSWaitSet::get_conditions(DDSConditionSeq& attached_cond)
{
    return DDS_WaitSet_get_conditions(this->_c_impl,&attached_cond._c_seq);
}

