/*
 * FILE: Conditions.c - Implementation of DDS Conditions.
 *
 * (c) Copyright, Real-Time Innovations, 2012-2021.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * modification history
 * --------------------
 * 13dec2021,tk MICRO-3226/PR.29479
 * - Added missing FILE to file header comment.
 * - Moved brace to it's own line in DDS_ConditionImpl_remove_waitset_ref
 * - Fixed return value comment for DDS_ConditionImpl_initialize
 * 20oct2021,tk MICRO-3308/PR.29809
 * - Fixed issue in set_enabled_status where the new set of enabled statuses
 *   did not update a StatusCondition's trigger value and potentially
 *   unblocked a WaitSet if the trigger value was TRUE.
 * - Set the default enabled_statues to DDS_STATUS_MASK_ALL since the standard
 *   says that if set_enabled_status is not called, the default list of
 *   enabled statuses are all statues.
 * 10jan2021,tk MICRO-2807/PR.27549  Removed unused functions for CERT
 * 14jul2015,eh MICRO-1429 Add notify_waitsets param to on_entity_event()
 * 08jun2015,tk MICRO-1294/PR#14962 Return DDS_BOOLEAN_FALSE, not
 *                                  DDS_STATUS_MASK_NONE, in get_trigger_value()
 * 19may2015,as MICRO-1193 Refactoring of Sequence API levels
 * 12mar2015,tk MICRO-1079/PR#14031 Removed redundant code
 * 05dec2014,as Additional fixes for MICRO-969
 * 13nov2014,as MICRO-969 Events may be lost by DDS_WaitSet_wait if
 *              they occur while no thread is blocked inside it
 * 05may2014,as MICRO-270 Always enable precondition
 *              checks for public API operations
 * 08nov2013,as MICRO-681 Complete implementation of WaitSets
 *              and support for StatusConditions
 * 19jul2013,as Added support for C++
 * 21dec2012,tk Written
 */
/*ci
 * \file
 *  @brief Implementation of DDS Conditions and Waitsets
 *
 *  \details
 *  This function implements the OMG DDS Condition API. The functions include
 *  both the public API as well as function to manage the internal
 *  data-structures and state to support the public API.
*/
/*ci \addtogroup DDSInfrastructureModule
 * @{
 */
#ifndef dds_c_dll_h
#include "dds_c/dds_c_dll.h"
#endif
#ifndef dds_c_infrastructure_h
#include "dds_c/dds_c_infrastructure.h"
#endif
#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef osapi_semaphore_h
#include "osapi/osapi_semaphore.h"
#endif
#ifndef osapi_mutex_h
#include "osapi/osapi_mutex.h"
#endif
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif

#include "Entity.h"
#include "Waitset.h"
#include "Conditions.h"

/*ci
 * \brief Structure encapsulating a reference to a waitset
 */
struct DDS_WaitSetRef
{
    /*ci
     * \brief A Conditions maintains a list of waitset references
     */
    REDA_CircularListNode_T _node;

    /*ci
     * \brief Reference
     */
    DDS_WaitSet *ws_ref;

    /*ci
     * \brief Opaque reference to the condition, returned by the waitset
     *
     * used for O(1) access on the condition's state by operations of the waitset
     */
    struct DDS_ConditionRef *self_ref;
};

/*ci
 * \def DDS_ConditionWSRefList_get_first
 * \brief Convenience function to get the first element in a circular list
 */
#define DDS_ConditionWSRefList_get_first(l_) \
   (struct DDS_WaitSetRef*)REDA_CircularList_get_first((l_))

/*ci
 * \def DDS_ConditionWSNode_get_next
 * \brief Convenience function to get the next element from a list node
 */
#define DDS_ConditionWSNode_get_next(c_) \
   (struct DDS_WaitSetRef*)REDA_CircularListNode_get_next((c_))

/*\ci
 * \brief Implementation of the GuardCondition
 */
struct DDS_GuardConditionImpl
{
    /*ci
     * \brief Inherited from the generic condition
     */
    struct DDS_ConditionImpl _parent;
};

/*** SOURCE_BEGIN ***/
#define T struct DDS_ConditionImpl*
#ifndef RTI_CERT
#define TSeq_ensure_length
#define TSeq_has_ownership
#endif
#define TSeq DDS_ConditionSeq
#include "reda/reda_sequence_defn.h"


/*ci \dref_Condition_initialize
 *   \brief Initialize a Condition
 *
 *   \details
 *   This function initializes the base-class DDS_Condition. It should be
 *   called by all conditions derived from DDS_Condition before the object
 *   is used.
 *
 *   \return - DDS_BOOLEAN_TRUE if the condition was successfully initialized
 *             DDS_BOOLEAN_FALSE if the initialization failed
 *
 *   \mtsafety This function is thread safe
 *
 *   \sa Condition_finalize
 */
RTI_PRIVATE DDS_Boolean
DDS_ConditionImpl_initialize(struct DDS_ConditionImpl *cond)
{
    REDA_CircularList_init(&cond->_waitsets);

    cond->_state_lock = OSAPI_Mutex_new();
    if (cond->_state_lock == NULL)
    {
        DDSC_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_MUTEX_OBJECT)
        return DDS_BOOLEAN_FALSE;
    }

    cond->_trigger_value = DDS_BOOLEAN_FALSE;

    return DDS_BOOLEAN_TRUE;
}

#ifndef RTI_CERT
/*ci \dref_Condition_finalize
 *   \brief Finalize a condition
 *
 *   \details
 *   This function finalizes the base-class DDS_Condition. It should be
 *   called by all conditions derived from DDS_Condition before a concrete
 *   condition is being deleted.
 *
 *   \return - DDS_RETCODE_OK is the condition was successfully finalized
 *             DDS_RETCODE_ERROR if the finalization failed
 *
 *   \mtsafety This function is thread safe
 *
 *   \sa Condition_initialize
 */
RTI_PRIVATE DDS_Boolean
DDS_ConditionImpl_finalize(struct DDS_ConditionImpl *self)
{
    struct DDS_WaitSetRef *wsref,*wsref_next;
    DDS_Boolean rc  = DDS_BOOLEAN_FALSE;

    if (self->_state_lock == NULL)
    {
        return DDS_BOOLEAN_TRUE;
    }

    if (!OSAPI_Mutex_take(self->_state_lock))
    {
        return DDS_BOOLEAN_FALSE;
    }

    wsref = DDS_ConditionWSRefList_get_first(&self->_waitsets);
    while (!REDA_CircularList_node_at_head(&self->_waitsets,&wsref->_node))
    {
        wsref_next = DDS_ConditionWSNode_get_next(&wsref->_node);
        REDA_CircularList_unlink_node(&wsref->_node);
        if (DDS_WaitSetImpl_remove_cond_ref(wsref->ws_ref,self)
            != DDS_RETCODE_OK)
        {
            goto done;
        }
        OSAPI_Heap_free_struct(wsref);
        wsref = wsref_next;
    }

    rc = DDS_BOOLEAN_TRUE;

done:

    if (!OSAPI_Mutex_give(self->_state_lock))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (rc)
    {
        if (!OSAPI_Mutex_delete(self->_state_lock))
        {
            return DDS_BOOLEAN_FALSE;
        }

        self->_state_lock = NULL;
    }

    return rc;
}
#endif /* !RTI_CERT */

/*ci
 *   \brief Wake up all waitsets that this condition is attached to.
 *
 *   \details
 *   A condition can trigger a waitset to wake up. Normally a condition
 *   becomes true and wakes up a waitset. However, in some cases a condition
 *   may change value to false after it became true and woke up the waitset.
 *   Also, a condition may trigger after a wakeset was woken up. The flags
 *   triggered and trigger_value is used to indicate whether this condition
 *   should wakeup a waitset and with which value. Note that it is legal to
 *   wakeup a waitset with a value of false.
 *
 *   \param[in] self          The condition that is triggered
 *   \param[in] triggered     TRUE if the condition triggered the waitset or not
 *   \param[in] trigger_value The current trigger value of the condition
 *
 *   \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
DDS_Boolean
DDS_ConditionImpl_notify_waitsets(struct DDS_ConditionImpl *self,
                DDS_Boolean triggered, DDS_Boolean trigger_value)
{
    struct DDS_WaitSetRef *wsref;
    DDS_Boolean rc  = DDS_BOOLEAN_TRUE;

    if (!OSAPI_Mutex_take(self->_state_lock))
    {
        return DDS_BOOLEAN_FALSE;
    }

    wsref = DDS_ConditionWSRefList_get_first(&self->_waitsets);
    while (!REDA_CircularList_node_at_head(&self->_waitsets,&wsref->_node))
    {
        if (triggered)
        {
            DDS_WaitSetImpl_on_condition_triggered(
                    wsref->ws_ref, wsref->self_ref, trigger_value);
        }
        else
        {
            DDS_WaitSetImpl_on_condition_reset(wsref->ws_ref, wsref->self_ref);
        }
        wsref = DDS_ConditionWSNode_get_next(&wsref->_node);
    }

    if (!OSAPI_Mutex_give(self->_state_lock))
    {
        rc = DDS_BOOLEAN_FALSE;
    }

    return rc;
}

/*ci \dref_Condition_add_waitset_ref
 *   \brief Add a reference to a waitset
 *
 *   \details
 *   When a condition is attached to a waitset a reference to that waitset
 *   is needed by a condition to be able to wakeup the waitset when the
 *   condition is triggered. This function adds a reference to a waitset. A
 *   condition can be attached to multiple waitsets.
 *
 *   References are allocated from/deallocated to the heap. This is because
 *   waitsets and conditions do not have a factory. Since waitsets never, or
 *   rarely, are deleted, this is not considered a problem
 *
 *  \param [in]  self - Condition to attach waitset to
 *  \param [in]  ws   - Waitset that is referenced
 *
 *   \return DDS_RETCODE_OK if the reference was successfully added,
 *           DDS_RETCODE_OUT_OF_RESOURCES if the operation failed to allocate
 *           a new reference,
 *           DDS_RETCODE_ERROR if an error occurred to prevent the reference from
 *           being added.
 *
 *   \mtsafety This function is thread safe
 *
 *   \sa Condition_remove_waitset_ref
 */
DDS_ReturnCode_t
DDS_ConditionImpl_add_waitset_ref(struct DDS_ConditionImpl *self,
                                  DDS_WaitSet *ws)
{
    struct DDS_WaitSetRef *wsref = NULL;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    RTI_BOOL locked = DDS_BOOLEAN_FALSE;

    OSAPI_Heap_allocate_struct(&wsref,struct DDS_WaitSetRef);
    if (wsref == NULL)
    {
        DDSC_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_WSREF_OBJECT)
        retcode = DDS_RETCODE_OUT_OF_RESOURCES;
        goto done;
    }

    REDA_CircularListNode_init(&wsref->_node);
    wsref->ws_ref = ws;

    if (!OSAPI_Mutex_take(self->_state_lock))
    {
        retcode = DDS_RETCODE_ERROR;
        goto done;
    }
    locked = RTI_TRUE;

    retcode = DDS_WaitSetImpl_add_cond_ref(ws, self, self->_trigger_value,
                                           &wsref->self_ref);
    if (retcode != DDS_RETCODE_OK)
    {
        DDSC_LOG_WS_ADD_COND_REFERENCE(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    REDA_CircularList_append(&self->_waitsets,&wsref->_node);

    locked = RTI_FALSE;
    if (!OSAPI_Mutex_give(self->_state_lock))
    {
        retcode = DDS_RETCODE_ERROR;
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    /* at this point retcode can have one of 3 value:
     * - OK         -> everything ok, the waitset added the condition, a new
     *                 reference was added to the condition, and the condition
     *                 was unlocked.
     * - ERROR      -> either DDS_WaitSetImpl_add_cond_ref, OSAPI_Mutex_take,
     *                 OSAPI_Mutex_give failed.
     * - OUT_OF_RES -> A new reference could not be allocated, either by this
     *                 operation or DDS_WaitSetImpl_add_cond_ref.
     * - ILLEGAL_OP -> returned only by DDS_WaitSetImpl_add_cond_ref, the
     *                 condition is already attached to the waitset.
     */

    if (locked)
    {
        if (!OSAPI_Mutex_give(self->_state_lock))
        {
            retcode = DDS_RETCODE_ERROR;
        }
    }

#ifndef RTI_CERT
    /* the condition was already attached to the waitset or the waitset failed
     * to allocate a new reference to add (hence a new one wasn't added to
     * the condition either): the newly allocated reference must be freed.
     */
    if ((retcode == DDS_RETCODE_ILLEGAL_OPERATION ||
            retcode == DDS_RETCODE_OUT_OF_RESOURCES) && wsref != NULL)
    {
          OSAPI_Heap_free(wsref);
    }
#endif

    if (retcode == DDS_RETCODE_ILLEGAL_OPERATION)
    {
        /* ILLEGAL_OPERATION is an internal retcode signaling the
         * condition is already attached. In this case, OK must be
         * returned to the caller, since DDS_WaitSet_attach_condition
         * is idempotent
         */
        retcode = DDS_RETCODE_OK;
    }

#if OSAPI_ENABLE_LOG
    if (retcode != DDS_RETCODE_OK)
    {
        OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
    }
#endif
    return retcode;
}

#ifndef RTI_CERT
/*ci \dref_Condition_remove_waitset_ref
 *   \brief Remove a reference to a waitset
 *
 *   \details
 *   When a condition is attached to a waitset, a reference to that waitset
 *   is needed by a condition to be able to wakeup the waitset. Conversely,
 *   when a waitset is deleted any reference to it must be removed from
 *   a condition.
 *
 *   References are allocated from/de-allocated to the heap. This is because
 *   waitsets and conditions  do not have any resource-limits and thus
 *   it is not trivial to use buffer-pools. Since waitsets never, or rarely,
 *   get deleted, this is not considered a problem
 *
 *  \param [in] self            Condition to remove waitsets from
 *  \param [in] ws              Waitset that is referenced
 *  \param [in] remove_from_ws  TRUE if the condition reference should be
 *                              removed (if it exists)
 *
 *   \return - DDS_BOOLEAN_TRUE if a reference was successfully added
 *             DDS_BOOLEAN_FALSE if a reference could not be added
 *
 *   \mtsafety This function is thread safe
 *
 *   \sa Condition_remove_waitset_ref
 */
DDS_ReturnCode_t
DDS_ConditionImpl_remove_waitset_ref(struct DDS_ConditionImpl *self,
                                     DDS_WaitSet *ws,
                                     DDS_Boolean remove_from_ws)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    RTI_BOOL removed = RTI_FALSE;
    struct DDS_WaitSetRef *wsref = NULL, *next_wsref = NULL;

    if (!OSAPI_Mutex_take(self->_state_lock))
    {
        return DDS_RETCODE_ERROR;
    }

    wsref = DDS_ConditionWSRefList_get_first(&self->_waitsets);
    while (!REDA_CircularList_node_at_head(&self->_waitsets,&wsref->_node) &&
            !removed)
    {
        next_wsref = DDS_ConditionWSNode_get_next(&wsref->_node);
        if (wsref->ws_ref == ws)
        {
            if (remove_from_ws)
            {
                retcode = DDS_WaitSetImpl_remove_cond_ref(
                                (struct DDS_WaitSetImpl*)ws,self);
                switch (retcode)
                {
                    case DDS_RETCODE_BAD_PARAMETER:
                        goto done;
                    case DDS_RETCODE_OK:
                        break;
                    default:
                        retcode = DDS_RETCODE_ERROR;
                        DDSC_LOG_WS_REMOVE_COND_REFERENCE(OSAPI_LOGKIND_ERROR)
                        goto done;
                }
            }
            REDA_CircularList_unlink_node(&wsref->_node);
            OSAPI_Heap_free_struct(wsref);
            removed = RTI_TRUE;
        }
        wsref = next_wsref;
    }

    if (removed)
    {
        retcode = DDS_RETCODE_OK;
    }
    else
    {
        retcode = DDS_RETCODE_BAD_PARAMETER;
        DDSC_LOG_WS_REMOVE_COND_REFERENCE(OSAPI_LOGKIND_ERROR)
    }

done:
    if (!OSAPI_Mutex_give(self->_state_lock))
    {
        retcode = DDS_RETCODE_ERROR;
    }

    return retcode;
}
#endif

/*ce \dref_Condition_add_waitset_ref
 *  \brief Get the current trigger value of condition
 *
 *  \param[in] self Condition to return the current trigger value of. Cannot
 *                  be NULL.
 *
 *  \return  The current trigger value is returned
 *
 *  \mtsafety This function is thread safe
 */
DDS_Boolean
DDS_Condition_get_trigger_value(struct DDS_ConditionImpl *self)
{
    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                       return DDS_BOOLEAN_FALSE,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE))

    return self->_trigger_value;
}

#ifndef RTI_CERT
/*ci
 *  \brief Attach an opaque object to the condition
 *
 *  \details
 *
 *  The DDS C API enables other language bindings on-top of the C language
 *  by allowing opaque pointers to be attached to the C object. This makes
 *  it possible to retrieve language specific wrapper objects.
 *
 *  \param[in] self    Condition to store the pointer in
 *  \param[in] wrapper An opaque pointer store in the condition object
 *
 *  \sa DDS_ConditionImpl_get_wrapper_ref
 */
void
DDS_ConditionImpl_set_wrapper(struct DDS_ConditionImpl *self, void *wrapper)
{
    self->wrapper = wrapper;
}

/*ci
 *  \brief Return the address of the wrapper object
 *
 *  \details
 *
 *  Language bindings built on-top the C API may use native objects to
 *  represent the DDS_Condition type. This method enables such a binding
 *  to store a reference to the native object inside the C object.
 *
 *  \param[in] self    Condition to get the pointer from
 *
 *  \return The address of the pointer holding the wrapper object.
 *
 *  \sa DDS_ConditionImpl_set_wrapper
 */
void**
DDS_ConditionImpl_get_wrapper_ref(struct DDS_ConditionImpl *self)
{
    return &self->wrapper;
}
#endif

/*ce \dref_GuardCondition_new
 *   \brief Create a new guard condition
 *
 *   \return New GuardCondition on success, NULL on failure
 *
 *   \mtsafety This function is thread safe
 *
 *   \sa \ref GuardCondition_delete
 */
DDS_GuardCondition*
DDS_GuardCondition_new(void)
{
    struct DDS_GuardConditionImpl *retval = NULL;
    struct DDS_GuardConditionImpl *acond = NULL;

    OSAPI_Heap_allocate_struct(&acond,struct DDS_GuardConditionImpl);
    if (acond == NULL)
    {
        DDSC_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_CONDITION_OBJECT)
        goto done;
    }

    if (!DDS_ConditionImpl_initialize(&acond->_parent))
    {
        goto done;
    }

    retval = acond;

done:
#ifndef RTI_CERT
    if ((retval == NULL) && (acond != NULL))
    {
        (void)DDS_GuardCondition_delete(acond);
    }
#endif
    return retval;
}

#ifndef RTI_CERT
/*ce \dref_GuardCondition_delete
 *  \brief Delete a guard condition
 *
 *  \param [in] self GuardCondition to delete
 *
 *  \return  DDS_RETCODE_BAD_PARAMETER - Illegal input argument,
 *           DDS_RETCODE_OK - The guard condition was successfully deleted
 *
 *  \mtsafety This function is thread safe
 *
 *  \sa \ref GuardCondition_new
 */
DDS_ReturnCode_t
DDS_GuardCondition_delete(struct DDS_GuardConditionImpl *self)
{

    OSAPI_PRECONDITION_ALWAYS((self == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (!DDS_ConditionImpl_finalize(&self->_parent))
    {
        return DDS_RETCODE_BAD_PARAMETER;
    }

    OSAPI_Heap_free_struct(self);

    return DDS_RETCODE_OK;
}
#endif /* !RTI_CERT */


/*ce \dref_GuardCondition_set_trigger_value
 *  \brief Set the trigger value of the guard condition
 *
 *  \details
 *
 *  \param [in] self  GuardCondition to set trigger value on. Cannot be NULL.
 *  \param [in] value New trigger value
 *
 *  \return - DDS_RETCODE_BAD_PARAMETER - Illegal input argument
 *            DDS_RETCODE_OK - The trigger value was successfully set
 *
 *  \mtsafety This function is thread safe
 *
 *  \sa \ref Condition_get_trigger_value
 *
 */
DDS_ReturnCode_t
DDS_GuardCondition_set_trigger_value(struct DDS_GuardConditionImpl *self,
                                     DDS_Boolean value)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION_ALWAYS((self == NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (!OSAPI_Mutex_take(self->_parent._state_lock))
    {
        return DDS_RETCODE_ERROR;
    }

    self->_parent._trigger_value = value;

    if (!DDS_ConditionImpl_notify_waitsets(&self->_parent,
                value, self->_parent._trigger_value))
    {
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:

    if (!OSAPI_Mutex_give(self->_parent._state_lock))
    {
        OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
        return DDS_RETCODE_ERROR;
    }

    return retcode;
}



/*ce \dref_StatusCondition
 */

/*ce \dref_StatusCondition_initialize
 *   \brief Initialize a status condition
 *
 *   \return - DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 *
 *   \mtsafety This function is thread safe
 *
 *   \sa \ref StatusCondition_finalize
 */
DDS_Boolean
DDS_StatusConditionImpl_initialize(struct DDS_StatusConditionImpl *self,
                                   struct DDS_EntityImpl *entity)
{
    DDS_Boolean result = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION((self == NULL || entity == NULL),
                       return DDS_RETCODE_BAD_PARAMETER,
                       OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                       OSAPI_Log_entry_add_pointer("entity",entity,RTI_TRUE);)

    if (!DDS_ConditionImpl_initialize(&self->_parent))
    {
        goto done;
    }

    self->_entity = entity;
    self->_enabled_statuses = DDS_STATUS_MASK_ALL;

    result = DDS_BOOLEAN_TRUE;

done:
#ifndef RTI_CERT
    if (!result)
    {
        DDS_ConditionImpl_finalize(&self->_parent);
    }
#endif

    return result;
}

#ifndef RTI_CERT
/*ce \dref_StatusCondition_finalize
 *  \brief Finalize a status condition
 *
 *  \param [in] self  StatusCondition to finalize
 *
 *  \return  RTI_TRUE on success, RTI_FALSE on failure
 *
 *  \mtsafety This function is thread safe
 *
 *  \sa \ref GuardCondition_new
 */
DDS_Boolean
DDS_StatusConditionImpl_finalize(struct DDS_StatusConditionImpl *self)
{
    if (!DDS_ConditionImpl_finalize(&self->_parent))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}
#endif

/*ce \dref_StatusCondition_get_enabled_statuses
 */
DDS_StatusMask
DDS_StatusCondition_get_enabled_statuses(DDS_StatusCondition *self)
{
    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                           return DDS_STATUS_MASK_NONE,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return self->_enabled_statuses;
}

/*ce \dref_StatusCondition_set_enabled_statuses
 */
DDS_ReturnCode_t
DDS_StatusCondition_set_enabled_statuses(DDS_StatusCondition *self,
                                         DDS_StatusMask mask)
{
    DDS_Boolean result;

    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                           return DDS_RETCODE_ERROR,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (!OSAPI_Mutex_take(self->_parent._state_lock))
    {
        return DDS_RETCODE_ERROR;
    }

    self->_enabled_statuses = mask;

    if (self->_enabled_statuses & self->_entity->current_statuses)
    {
        /* If any of the currently active status conditions are enabled,
         * set the trigger value to TRUE. DDS_ConditionImpl_notify_waitsets
         * calls DDS_WaitSetImpl_on_condition_triggered which handles the case
         * where if a WaitSet is already unblocked an already active
         * condition does not unblock it again, but the condition's trigger
         * value is updated to TRUE.
         */
        self->_parent._trigger_value = DDS_BOOLEAN_TRUE;
    }
    else
    {
        /* If none of the enabled statuses are active, the condition will be
         * reset to not triggered and will not unblock a WaitSet and the status
         * condition will be marked as inactive. If a WaitSet is already
         * unblocked then it remains unblocked. However, the status condition
         * will not be triggered.
         */
        self->_parent._trigger_value = DDS_BOOLEAN_FALSE;
    }

    /* Notify all WaitSets with the change in enabled statuses. If the
     * trigger value is TRUE, the WaitSet is unblocked (if not already
     * unblocked) and the condition is marked as active, otherwise the
     * trigger value is FALSE and the condition is marked as inactive, and
     * the WaitSet is not unblocked.
     */
    result = DDS_ConditionImpl_notify_waitsets(&self->_parent,
                                               self->_parent._trigger_value,
                                               self->_parent._trigger_value);

    if (!OSAPI_Mutex_give(self->_parent._state_lock))
    {
        return DDS_RETCODE_ERROR;
    }

    if (!result)
    {
        return DDS_RETCODE_ERROR;
    }

    return DDS_RETCODE_OK;
}

/*ci \dref_StatusCondition_on_statuses_changed
 *
 * \brief Notification that the entity statuses have changed
 *
 * \details
 *
 * Notifies a StatusCondition that the active statuses
 * of its Entity have changed. The StatusCondition must
 * update its trigger_value based on the status mask
 * passed as input which contains all active statuses
 * in the Entity and its enabled_statuses mask.
 *
 * \param[in] self            The status condition with changed statuses
 * \param[in] triggered       The trigger value of the status
 * \param[in] event_status    The event status that triggered
 * \param[in] active_statuses The currently active statuses on the entity
 * \param[in] notify_waitsets Whether to notify waitsets of this event
 *
 * \return  DDS_BOOLEAN_FALSE on failure, DDS_BOOLEAN_TRUE on success
 */
DDS_Boolean
DDS_StatusConditionImpl_on_entity_event(struct DDS_StatusConditionImpl *self,
                                        DDS_Boolean triggered,
                                        DDS_StatusMask event_status,
                                        DDS_StatusMask active_statuses,
                                        DDS_Boolean notify_waitsets)
{
    DDS_Boolean result = DDS_BOOLEAN_TRUE;
    DDS_StatusMask enabled_statuses;

    if (!OSAPI_Mutex_take(self->_parent._state_lock))
    {
        return DDS_BOOLEAN_FALSE;
    }

    enabled_statuses = self->_enabled_statuses;

    if (enabled_statuses & event_status)
    {
        if (enabled_statuses & active_statuses)
        {
            self->_parent._trigger_value = DDS_BOOLEAN_TRUE;
        }
        else
        {
            self->_parent._trigger_value = DDS_BOOLEAN_FALSE;
        }

        if (notify_waitsets)
        {
            result = DDS_ConditionImpl_notify_waitsets(&self->_parent,
                            triggered, self->_parent._trigger_value);
        }
    }

    if (!OSAPI_Mutex_give(self->_parent._state_lock))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return result;
}

/*ce \dref_StatusCondition_get_entity
 */
DDS_Entity*
DDS_StatusCondition_get_entity(DDS_StatusCondition *self)
{
    OSAPI_PRECONDITION_ALWAYS((self == NULL),
                           return NULL,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return self->_entity;
}

/*ci @} */

