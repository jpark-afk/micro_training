/*
 * FILE: Entity.c - Entity implementation
 *
 * (c) Copyright 2008-2024 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 14jul2015,eh MICRO-1429 Add enable_status_only()
 * 16sep2014,tk MICRO-896/PR#10780 Added extra robustness checks
 * 29jul2014,tk MICRO-856/PR#10218 Added precond check to get_instance_handle()
 *              MICRO-857/PR#10219 Removed unused function get_entity_kind()
 *              MICRO-858/PR#10220 Added precond check to get_status_changes()
 * 05may2014,as MICRO-270 Always enable precondition
 *              checks for public API operations
 * 08nov2013,as MICRO-681 Complete implementation of WaitSets
 *              and support for StatusConditions
 * 19jul2013,as Added support for C++
 * 06may2012,tk Updated
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief Entity implementation
 *
 * \details
 * This function implements functions to support a DDS Entity.
 */

/*ci \addtogroup DDSInfrastructureModule
 * @{
 */
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif

#include "Entity.h"

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Initialize a entity base-class
 *
 * \details
 *
 * All entities derived from this class must initialize the base-class
 * by calling this function first.
 *
 * \param[in] entity              The base-class to initialize
 * \param[in] kind                The kind of entity
 * \param[in] entity_id           The entity_id of if the entity
 * \param[in] enable              Overloaded entity enable function
 * \param[in] get_instance_handle Overloaded entity get_instance_handle function
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 *
 * \sa DDS_EntityImpl_finalize
 */
DDS_Boolean
DDS_EntityImpl_initialize(struct DDS_EntityImpl *entity,
                     DDS_EntityKind_t kind,
                     DDS_UnsignedLong entity_id,
                     RTIDDS_EntityEnableFunction enable,
                     RTIDDS_EntityGetInstanceHandleFunction get_instance_handle)
{
    UNUSED_ARG(kind);
    OSAPI_PRECONDITION_ALWAYS((enable == NULL) ||
                                  (get_instance_handle == NULL),
              return DDS_BOOLEAN_FALSE,
              OSAPI_Log_entry_add_pointer("enable",
                                  enable != NULL ? (void*)1 : NULL,RTI_FALSE);
              OSAPI_Log_entry_add_pointer("get_instance_handle",
                                  get_instance_handle != NULL ? (void*)1 : NULL,RTI_TRUE);)

    entity->enable = enable;
    entity->get_instance_handle = get_instance_handle;
    entity->entity_id = entity_id;
    entity->state = RTIDDS_ENTITY_STATE_CREATED;
    entity->wrapper = NULL;
    entity->current_statuses = DDS_STATUS_MASK_NONE;
    return DDS_StatusConditionImpl_initialize(&entity->status_condition, entity);
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize an entity base-class
 *
 * \details
 *
 * All entities derived from this class must finalize the base-class
 * by calling this function.
 *
 * \param[in] entity The base-class to finalize
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 *
 * \sa DDS_EntityImpl_initialize
 */
DDS_Boolean
DDS_EntityImpl_finalize(struct DDS_EntityImpl *entity)
{
    return DDS_StatusConditionImpl_finalize(&entity->status_condition);
}
#endif

/*******************************************************************************
 *
 *                            Public API
 *
 ******************************************************************************/
DDS_ReturnCode_t
DDS_Entity_enable(DDS_Entity *self)
{
    struct DDS_EntityImpl *entity = (struct DDS_EntityImpl *)self;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) ||
                                  (self->enable == NULL),
                   return DDS_RETCODE_PRECONDITION_NOT_MET,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("self->enable",
                                         self ? (self->enable != NULL ?
                                             (void*)11 : NULL): NULL,RTI_TRUE);)

    return entity->enable(self);
}

DDS_Boolean
DDS_Entity_is_enabled(DDS_Entity *self)
{
    struct DDS_EntityImpl *entity = (struct DDS_EntityImpl *)self;

    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                            return DDS_BOOLEAN_FALSE,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return (entity->state == RTIDDS_ENTITY_STATE_ENABLED);
}

DDS_InstanceHandle_t
DDS_Entity_get_instance_handle(DDS_Entity *self)
{
    struct DDS_EntityImpl *entity = (struct DDS_EntityImpl *)self;
    DDS_InstanceHandle_t retval = DDS_HANDLE_NIL;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) ||
                            (self->get_instance_handle == NULL),
                   return retval,
                OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                OSAPI_Log_entry_add_pointer("self->get_instance_handle",
                (self == NULL ? NULL : (self->get_instance_handle != NULL ?
                        (void*)1 : NULL)),RTI_TRUE);)

    retval = entity->get_instance_handle(self);

    return retval;
}

DDS_StatusCondition*
DDS_Entity_get_statuscondition(DDS_Entity *self)
{
    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                           return NULL,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return &self->status_condition;
}

DDS_StatusMask
DDS_Entity_get_status_changes(DDS_Entity *self)
{
    OSAPI_PRECONDITION_ALWAYS((self == NULL),
                           return DDS_STATUS_MASK_NONE,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return DDS_StatusConditionImpl_get_entity_status_changes(
                                                &self->status_condition);
}

void
DDS_Entity_set_wrapper(DDS_Entity *self, void *wrapper)
{
    struct DDS_EntityImpl *entity = (struct DDS_EntityImpl *)self;
    entity->wrapper = wrapper;
}

void*
DDS_Entity_get_wrapper(DDS_Entity *self)
{
    struct DDS_EntityImpl *entity = (struct DDS_EntityImpl *)self;
    return entity->wrapper;
}

/*ci
 * \brief Disable a specific status in a DDS_Entity and
 *        update its DDS_StatusCondition's trigger value.
 *
 * \param[in] self   DDS_Entity on which the status must be disabled
 * \param[in] status The status to disable on the DDS_Entity
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DDS_EntityImpl_disable_status(DDS_Entity *self, DDS_StatusMask status)
{
    return DDS_StatusConditionImpl_on_entity_event(&self->status_condition,
                                                    DDS_BOOLEAN_FALSE,
                                                    status,
                                                    &self->current_statuses,
                                                    DDS_BOOLEAN_TRUE,
                                                    DDS_BOOLEAN_FALSE);
}

/*ci
 * \brief Enable a specific status in a DDS_Entity and
 *        update its DDS_StatusCondition's trigger value.
 *
 * \param[in] self       DDS_Entity on which the status must be enabled
 * \param[in] status     The status to enable on the DDS_Entity
 * \param[in] consumed   Whether the status has been consumed or not,
 *                       used as part of internal state update
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DDS_EntityImpl_enable_status(DDS_Entity *self,
                             DDS_StatusMask status,
                             DDS_Boolean consumed)
{
    return DDS_StatusConditionImpl_on_entity_event(&self->status_condition,
                                                    DDS_BOOLEAN_TRUE,
                                                    status,
                                                    &self->current_statuses,
                                                    DDS_BOOLEAN_TRUE,
                                                    consumed);
}

/*ci
 * \brief Enable a specific status in a DDS_Entity and
 *        update its DDS_StatusCondition's trigger value.
 *
 * \param[in] self       DDS_Entity on which the status must be enabled
 * \param[in] status     The status to enable on the DDS_Entity
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DDS_EntityImpl_enable_status_only(DDS_Entity *self,
                             DDS_StatusMask status)
{
    return DDS_StatusConditionImpl_on_entity_event(&self->status_condition,
                                                    DDS_BOOLEAN_TRUE,
                                                    status,
                                                    &self->current_statuses,
                                                    DDS_BOOLEAN_FALSE,
                                                    DDS_BOOLEAN_FALSE);
}

/*ci @} */
