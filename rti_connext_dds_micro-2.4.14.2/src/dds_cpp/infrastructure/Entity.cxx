/* 

 (c) Copyright, Real-Time Innovations, 2013-2015.
 All rights reserved.
 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.

 modification history
---------------------
19jul2013,as  Major C++ update
11jan2013,eh  Created
===================================================================== */

#ifndef dds_cpp_infrastructure_hxx
#include "dds_cpp/dds_cpp_infrastructure.hxx"
#endif

#include "Entity.hxx"
#include "Conditions.hxx"
/*** SOURCE_BEGIN ***/

DDS_ReturnCode_t
DDSEntity::enable()
{
    return DDS_Entity_enable(this->_c_entity);
}

DDSStatusCondition*
DDSEntity::get_statuscondition()
{
    DDSCondition** cpp_cond_ref = NULL;
    DDS_StatusCondition *c_cond = NULL;
    c_cond = DDS_Entity_get_statuscondition(this->_c_entity);
    cpp_cond_ref = (DDSCondition**) DDS_ConditionImpl_get_wrapper_ref(
                                DDS_StatusCondition_as_condition(c_cond));
    return (DDSStatusCondition*) *cpp_cond_ref;
}

DDS_StatusMask
DDSEntity::get_status_changes()
{
    return DDS_Entity_get_status_changes(this->_c_entity);
}

DDS_InstanceHandle_t
DDSEntity::get_instance_handle()
{
    return DDS_Entity_get_instance_handle(this->_c_entity);
}


DDS_Entity*
DDSEntity::get_c_entity()
{
    return this->_c_entity;
}

DDSEntity::DDSEntity(DDS_Entity *c_entity) :
        _c_entity(c_entity)
{
    DDS_StatusCondition *c_cond =
            DDS_Entity_get_statuscondition(this->_c_entity);
    DDSStatusCondition *cpp_cond = NULL;

    cpp_cond = new DDSStatusCondition(c_cond);

    DDS_ConditionImpl_set_wrapper(
            DDS_StatusCondition_as_condition(c_cond), cpp_cond);
}

DDSEntity::~DDSEntity()
{
    DDS_StatusCondition *c_cond =
            DDS_Entity_get_statuscondition(this->_c_entity);
    DDSStatusCondition *cpp_cond = NULL;

    cpp_cond = (DDSStatusCondition*)
         (*DDS_ConditionImpl_get_wrapper_ref(
            DDS_StatusCondition_as_condition(c_cond)));

    delete cpp_cond;

    DDS_ConditionImpl_set_wrapper(
            DDS_StatusCondition_as_condition(c_cond), NULL);

    this->_c_entity = NULL;
}
