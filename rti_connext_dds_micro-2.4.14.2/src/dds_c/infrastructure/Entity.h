/*
 * FILE: Entity.h - Entity implementation
 *
 * (c) Copyright 2008-2020 Real-Time Innovations,
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
 * 05dec2014,as Additional fixes for MICRO-969
 * 20sep2014,as Entity.c - DDS_EntityImpl_initialize() - uninitialized
 *              and unused components
 * 08nov2013,as MICRO-681 Complete implementation of WaitSets
 *              and support for StatusConditions
 * 19jul2013,as Added support for C++
 * 06may2012,tk Updated
 * 30apr2008,tk Created
 */
/*ci
 * \file
 * \brief Entity implementation
 */
/*ci \addtogroup DDSInfrastructureModule
 * @{
 */
#ifndef Entity_pkg_h
#define Entity_pkg_h

#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif

#include "Conditions.h"

#ifdef __cplusplus
extern "C"
{
#endif


/*ci \brief Typedefinition for the internal enable function
 * 
 * \param[in] self The entity that is enabled
 */
typedef DDS_ReturnCode_t
(*RTIDDS_EntityEnableFunction)(DDS_Entity *self);

/*ci \brief Typedefinition for the internal get instance handle function
 * 
 * \param[in] self The entity to retrieve the instance-handle for
 */
typedef DDS_InstanceHandle_t
(*RTIDDS_EntityGetInstanceHandleFunction)(DDS_Entity *self);

/*ci \brief Typedefinition for the internal object id generator
 * 
 * \param[in] self The entity generating the object id
 */
typedef DDS_UnsignedLong
(*RTIDDS_ObjectIdGenerator)(void *self);

typedef enum
{
    RTIDDS_ENTITY_STATE_CREATED,
    RTIDDS_ENTITY_STATE_ENABLED
} RTIDDS_EntityState;

/*ci
 * \brief Implementation of the entity base-class for all DDS entities
 */
struct DDS_EntityImpl
{
    /*ci
     * \brief The unique entity id within a participant
     */
    DDS_UnsignedLong entity_id;

    /*ci
     * \brief "virtual" function to enable any entity
     */
    RTIDDS_EntityEnableFunction enable;

    /*ci
     * \brief "virtual" function to get the instance handle for any entity
     */
    RTIDDS_EntityGetInstanceHandleFunction get_instance_handle;

    /*ci
     * \brief The current state of an entity
     */
    RTIDDS_EntityState state;

    /*ci
     * \brief The kind of DDS entity
     */
    DDS_EntityKind_t kind;

    /*ci
     * \brief The status condition for an entity. All entities have exactly
     *        one status condition and the memory of it is allocated as part
     *        of the Entity itself
     */
    struct DDS_StatusConditionImpl status_condition;

    /*ci
     * \brief The set of DDS_Status flags currently
     * active in an entity.
     */
    DDS_StatusMask current_statuses;

    /*ci
     * \brief Opaque pointer to an optional object associated with this
     *        entity
     */
    void *wrapper;
};

MUST_CHECK_RETURN extern DDS_Boolean
DDS_EntityImpl_initialize(struct DDS_EntityImpl *entity,
        DDS_EntityKind_t kind,
        DDS_UnsignedLong entity_id,
        RTIDDS_EntityEnableFunction enable,
        RTIDDS_EntityGetInstanceHandleFunction get_instance_handle);

#ifndef RTI_CERT
MUST_CHECK_RETURN extern DDS_Boolean
DDS_EntityImpl_finalize(struct DDS_EntityImpl *entity);
#endif

extern DDS_Boolean
DDS_EntityImpl_enable_status(
        DDS_Entity *self, DDS_StatusMask statusmask, DDS_Boolean consumed);

extern DDS_Boolean
DDS_EntityImpl_disable_status(
        DDS_Entity *self, DDS_StatusMask statusmask);

extern DDS_Boolean
DDS_EntityImpl_enable_status_only(
        DDS_Entity *self, DDS_StatusMask statusmask);

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif /* Entity_pkg_h */

/*ci @} */

