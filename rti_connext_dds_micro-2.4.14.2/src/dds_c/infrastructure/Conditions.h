/*
 * (c) Copyright, Real-Time Innovations, 2013-2015.
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * modification history
 * --------------------
 * 14jul2015,eh  MICRO-1429 Add notify_waitsets param to on_entity_event()
 * 05dec2014,as  Additional fixes for MICRO-969
 * 13nov2014,as  MICRO-969 Events may be lost by DDS_WaitSet_wait if
 *               they occur while no thread is blocked inside it
 * 08nov2013,as  MICRO-681 Complete implementation of WaitSets
 *              and support for StatusConditions
 * 19jul2013,as  Added support for C++
 */

/*ci \addtogroup DDSInfrastructureModule
 * @{
 */
#ifndef Conditions_h
#define Conditions_h

#include "osapi/osapi_mutex.h"

#ifdef __cplusplus
    extern "C" {
#endif

/*ci
 * \brief Implementation of the DDS_Condition class
 */
struct DDS_ConditionImpl
{
    /*ci \dref__waitsets
     * A list of waitsets which this condition is attached to
     */
    REDA_CircularList_T _waitsets;

    /*ci \dref__waitsets
     * Mutex to protect addition and removal from the waitset list
     */
    OSAPI_Mutex_T *_state_lock;

    /*ci \dref__waitsets
     * The current trigger value of this condition
     */
    DDS_Boolean _trigger_value;

    /*ci \dref__waitsets
     */
    void *wrapper;
};

/*ci
 * \brief Implementation of the DDS_StatusCondition class
 */
struct DDS_StatusConditionImpl
{
    /*ci
     * \brief Inherited from \ref DDS_ConditionImpl
     */
    struct DDS_ConditionImpl _parent;

    /*ci
     * \brief The DDS entity that owns this status condition, only one
     *        per DDS entity
     */
    struct DDS_EntityImpl *_entity;

    /*ci
     * \brief The currenly enabled status as specified by the user
     */
    DDS_StatusMask _enabled_statuses;
};

extern DDS_Boolean
DDS_ConditionImpl_notify_waitsets(struct DDS_ConditionImpl *self,
                DDS_Boolean triggered, DDS_Boolean trigger_value);

extern DDS_ReturnCode_t
DDS_ConditionImpl_add_waitset_ref(struct DDS_ConditionImpl *self,DDS_WaitSet *ws);

extern DDS_ReturnCode_t
DDS_ConditionImpl_remove_waitset_ref(struct DDS_ConditionImpl *self,
            DDS_WaitSet *ws, DDS_Boolean remove_from_ws);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_StatusConditionImpl_initialize(
        struct DDS_StatusConditionImpl *s_cond, struct DDS_EntityImpl *entity);

#ifndef RTI_CERT
extern DDS_Boolean
DDS_StatusConditionImpl_finalize(struct DDS_StatusConditionImpl *self);
#endif

extern DDS_Boolean
DDS_StatusConditionImpl_on_entity_event(
        struct DDS_StatusConditionImpl *self,
        DDS_Boolean triggered,
        DDS_StatusMask event_status,
        DDS_StatusMask active_statuses,
        DDS_Boolean notify_waitsets);


#ifdef __cplusplus
    } /* extern "C" */
#endif

#endif

/*ci @} */
