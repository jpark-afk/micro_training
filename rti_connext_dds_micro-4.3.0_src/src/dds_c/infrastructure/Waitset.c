/*
 *(c) Copyright, Real-Time Innovations, 2012-2015.
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * modification history
 * --------------------
 * 09jul2015,tk  MICRO-1403/PR#15275 Fixed code comment errors
 * 28mar2015,tk  MICRO-1273/PR#14881 Added robustness check to wait()
 * 12mar2015,tk  MICRO-1093/PR#14109 Fixed comment for cond_map_exists
 *               MICRO-1094/PR#14110 Fixed comment for cond_map_wait_prescan
 * 02feb2015,tk  MICRO-976/PR#12939 Added logging to DDS_WaitSet_new
 * 05dec2014,as  Additional fixes for MICRO-969
 * 13nov2014,as  MICRO-969 Events may be lost by DDS_WaitSet_wait if
 *               they occur while no thread is blocked inside it
 * 20sep2014,as  MICRO-871 Verify that DDS_WaitSet_attach_condition may
 *               return OUT_OF_RESOURCES return code
 * 19may2014,as  MICRO-788 Implement DDS_WaitSet_get_conditions
 * 05may2014,as  MICRO-270 Always enable precondition
 *               checks for public API operations
 * 08nov2013,as  MICRO-681 Complete implementation of WaitSets
 *               and support for StatusConditions
 * 12/21/2012,tk Written
 */
/*/ci
 * \file
 * \brief This file implements the standard DDS Waitset API. The implementation
 *        complies with OMG DDS 1.2 07-01-01
 */
/*ci \addtogroup DDSInfrastructureModule
 * @{
 */
#ifndef dds_c_dll_h
#include "dds_c/dds_c_dll.h"
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
#ifndef reda_circularlist_h
#include "reda/reda_circularlist.h"
#endif
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif

#include "Conditions.h"
#include "Waitset.h"

/*ci
 * \brief Structure holding references to DDS conditions
 */
struct DDS_ConditionRef
{
    /*ci \dref__node
     * This structure can be directly added to/removed from a circular list
     */
    REDA_CircularListNode_T _node;

    /*ci
     * A reference to a condition
     */
    DDS_Condition *cond_ref;

    /*ci
     * Flag signaling that the condition
     * has triggered and the waitset hasn't
     * processed the event (i.e. returned the
     * condition in a call to DDS_WaitSet_wait()).
     *
     * Initialized to DDS_BOOLEAN_FALSE.
     */
    DDS_Boolean flag_pending_trigger;

    /*ci
     * Flag signaling whether the condition
     * should be returned by the next call to
     * DDS_WaitSet_wait().
     *
     * Initialized to DDS_BOOLEAN_FALSE.
     */
    DDS_Boolean flag_return_next;

    /*ci
     * Flag storing the last value
     * of the Condition's trigger_value seen
     * by the WaitSet.
     *
     * Initialized to the trigger_value found
     * at the time the condition is attached to
     * the WaitSet.
     */
    DDS_Boolean flag_last_trigger;
};

#define DDS_WaitSet_WakeUpState_INITIALIZER \
{ NULL, DDS_BOOLEAN_FALSE }

/*ci
 * \brief Convenience function to get the first node in a condition
 *        reference list
 */
#define DDS_WaitSetConditionRefList_get_first(l_) \
        (struct DDS_ConditionRef*)REDA_CircularList_get_first(l_)

/*ci
 * \brief Convenience function to get next node in a condition reference list
 */
#define DDS_WaitSetConditionRefNode_next(c_) \
        (struct DDS_ConditionRef*)REDA_CircularListNode_get_next((c_))

typedef DDS_ReturnCode_t
(*DDS_WaitSetMapFunction_T)(struct DDS_WaitSetImpl*,
                            struct DDS_ConditionRef*,void *p,RTI_BOOL *done);

/*** SOURCE_BEGIN ***/

#ifndef RTI_CERT
/*ci
 * \brief Remove all condition references in a waitset
 *
 * \details
 *
 * This function is only called from \ref DDS_WaitSetImpl_cond_map.
 *
 * \param[in]  self  Waitset to remove condition references from
 * \param[in]  cref  Condition reference to remove
 * \param[in]  param Opaque param passed from \ref DDS_WaitSetImpl_cond_map
 * \param[out] done  Whether this was the last node or not
 *
 * \return DDS_RETCODE_OK on success, one of the error return codes on failure
 *
 * \sa \ref DDS_WaitSetImpl_cond_map
 */
RTI_PRIVATE DDS_ReturnCode_t
DDS_WaitSetImpl_cond_map_delete(struct DDS_WaitSetImpl *self,
                                struct DDS_ConditionRef *cref,
                                void *param,
                                RTI_BOOL *done)
{
    DDS_ReturnCode_t rc = DDS_RETCODE_OK;

    *done = RTI_FALSE;
    if ((param == NULL) || (cref->cond_ref == (DDS_Condition *)param))
    {
        REDA_CircularList_unlink_node(&cref->_node);
        --self->_conditions_len;
        rc = DDS_RETCODE_OK;

        /* Tell Condition to remove ref to waitset if we are deleting
         * all conditions (i.e. param == NULL); otherwise, skip this
         * because it is already taken care of by the condition (in the case
         * of DDS_WaitSet_detach_condition)
         */
        if (param == NULL)
        {
            rc = DDS_ConditionImpl_remove_waitset_ref(
                                        cref->cond_ref,self, DDS_BOOLEAN_FALSE);
            if (rc != DDS_RETCODE_OK)
            {
                rc = DDS_RETCODE_ERROR;
                *done = RTI_TRUE;
            }
        }

        OSAPI_Heap_free_struct(cref);

        if (param != NULL)
        {
            *done = RTI_TRUE;
            rc = DDS_RETCODE_ILLEGAL_OPERATION;
        }
    }

    return rc;
}
#endif /* !RTI_CERT */

/*ci
 * \brief Check if a condition is already attached to a waitset
 *
 * \details
 *
 * This function is only called from \ref DDS_WaitSetImpl_cond_map.
 *
 * \param[in]  self  Waitset. This parameter is not used, but is present to
 *                   comply with the calling convention for
 *                   \ref DDS_WaitSetImpl_cond_map.
 * \param[in]  cref  An existing condition reference attached to the waitset
 * \param[in]  param Opaque param passed from \ref DDS_WaitSetImpl_cond_map.
 *                   For this function the param is the condition that is
 *                   being checked if it already exists (tested against cref)
 * \param[out] done  Whether this was the last node or not. For this function
 *                   it indicates that the traversal of nodes should stop,
 *                   a match was found.
 *
 * \return DDS_RETCODE_OK if cref does not exist, DDS_RETCODE_ILLEGAL_OPERATION
 *         if cref already exists
 *
 * \sa \ref DDS_WaitSetImpl_cond_map
 */
RTI_PRIVATE DDS_ReturnCode_t
DDS_WaitSetImpl_cond_map_exists(struct DDS_WaitSetImpl *self,
                            struct DDS_ConditionRef *cref,
                            void *param,
                            RTI_BOOL *done)
{
    DDS_ReturnCode_t rc = DDS_RETCODE_OK;
    UNUSED_ARG(self);

    *done = RTI_FALSE;

    if (cref->cond_ref == (DDS_Condition *)param)
    {
        *done = RTI_TRUE;
        rc = DDS_RETCODE_ILLEGAL_OPERATION;
    }

    return rc;
}

/*ci
 * \brief Wake up all active conditions for a waitset
 *
 * \details
 *
 * This function is only called from \ref DDS_WaitSetImpl_cond_map.
 *
 * A prescan is performed when a wait() is called on a waitset before the
 * waitset blocks on the condition semaphore. This ensures that an
 * active condition can unblock the waitset in case the condition became
 * active after a previous call to wait unblocked it, but before the
 * condition could unblock it.
 *
 * \param[in]  self  Waitset to unblock if an attached condition is true
 * \param[in]  cref  An attached condition, passed in from
 *                   \ref DDS_WaitSetImpl_cond_map
 * \param[in]  param Opaque param passed from \ref DDS_WaitSetImpl_cond_map.
 *                   For this function it is not used.
 * \param[out] done  Whether this was the last node or not. Not used in this
 *                   function as all conditions are iterated over.
 *
 * \return DDS_RETCODE_OK on success, one of the error return codes on failure
 *
 * \sa \ref DDS_WaitSetImpl_cond_map
 */
RTI_PRIVATE DDS_ReturnCode_t
DDS_WaitSetImpl_cond_map_wait_prescan(struct DDS_WaitSetImpl *self,
                                      struct DDS_ConditionRef *cref,
                                      void *param,
                                      RTI_BOOL *done)
{
    UNUSED_ARG(param);
    UNUSED_ARG(done);

    if (cref->flag_last_trigger || cref->flag_pending_trigger)
    {
        cref->flag_return_next = DDS_BOOLEAN_TRUE;
    }
    else
    {
        cref->flag_return_next = DDS_BOOLEAN_FALSE;
    }

    /* Coverity warns that _state_lock is not owned when accessing
     * _state_blocked. This function is only called from one location in
     * WaitSet_wait where _state_lock is owned.
     */
    /* coverity[missing_lock] */
    if (cref->flag_return_next && self->_state_blocked)
    {
        /* coverity[missing_lock] */
        self->_state_blocked = DDS_BOOLEAN_FALSE;
        if (!OSAPI_Semaphore_give(self->_condition_active))
        {
            return DDS_RETCODE_ERROR;
        }
    }

    return DDS_RETCODE_OK;
}

/*ci
 * \brief Find all active conditions for a waitset
 *
 * \details
 *
 * This function is only called from \ref DDS_WaitSetImpl_cond_map.
 *
 * "Active conditions" are all those conditions that have triggered (i.e.
 * their trigger_value became TRUE) since the last time this operation
 * checked their status.
 *
 * These conditions must be returned by the following call to DDS_WaitSet_wait.
 * Note that it is not sufficient to check for their current trigger_value
 * because this might have transitioned back to FALSE or it may have never been
 * actually triggered to TRUE, as in the case of a condition triggered by an
 * event that has already been consumed by a listener.
 *
 * \param[in]  self  Waitset to find active condition references on. Not used
 *                   in this function.
 * \param[in]  cref  A condition reference to include in the output if it is
 *                   true
 * \param[in]  param Opaque param passed from \ref DDS_WaitSetImpl_cond_map.
 *                   In this function it is the output sequence of active
 *                   conditions
 * \param[out] done  Whether this was the last node or not. In this function
 *                   it is set to TRUE if the maximum length of the output
 *                   sequence is reached.
 *
 * \return DDS_RETCODE_OK on success, one of the error return codes on failure
 *
 * \sa \ref DDS_WaitSetImpl_cond_map
 */
RTI_PRIVATE DDS_ReturnCode_t
DDS_WaitSetImpl_cond_map_wait_postscan(
                            struct DDS_WaitSetImpl *self,
                            struct DDS_ConditionRef *cref,
                            void *param,
                            RTI_BOOL *done)
{

    struct DDS_ConditionSeq *active_conditions =
            (struct DDS_ConditionSeq*) param;
    DDS_Long ac_count = 0;
    DDS_Long max_ac_count;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    UNUSED_ARG(self);

    *done = RTI_FALSE;

    if (cref->flag_return_next || cref->flag_pending_trigger)
    {
        ac_count = DDS_ConditionSeq_get_length(active_conditions);
        max_ac_count = DDS_ConditionSeq_get_maximum(active_conditions);

        if ((ac_count < 0) || (max_ac_count < 0))
        {
            retcode = DDS_RETCODE_PRECONDITION_NOT_MET;
            goto done;
        }

        if (max_ac_count == ac_count)
        {
            retcode = DDS_RETCODE_OUT_OF_RESOURCES;
            goto done;
        }
        else
        {
            ac_count++;
            if (!DDS_ConditionSeq_set_length(active_conditions,ac_count))
            {
                retcode = DDS_RETCODE_ERROR;
                goto done;
            }
            /* Since set_length succeeded it is assumed that get_reference
             * returns a valid address.
             */
            /* coverity[dereference] */
            /* coverity[cert_exp34_c_violation] */
            *DDS_ConditionSeq_get_reference(active_conditions,ac_count-1) =
                                                                cref->cond_ref;
        }
    }

    retcode = DDS_RETCODE_OK;

done:

    cref->flag_pending_trigger = DDS_BOOLEAN_FALSE;
    cref->flag_return_next = DDS_BOOLEAN_FALSE;

    return retcode;
}

/*ci
 * \brief Iterate over all conditions attached to a DDS_WaitSet
 *
 * \details
 *
 *  This is a generic helper function that iterates over all conditions
 *  attached to a \ref DDS_WaitSet and applies the map function on the
 *  condition reference, passing in the caller supplied param
 *
 * \param[in] self  DDS_WaitSet to iterate over all conditions
 * \param[in] map   The function to call on each condition reference
 * \param[in] param The parameter to pass to the map function
 *
 * \return DDS_RETCODE_OK on success, one of the error return codes on failure
 *
 * \sa \ref DDS_WaitSetImpl_cond_map_get_active_conditions,
 *     \ref DDS_WaitSetImpl_cond_map_exists,
 *     \ref DDS_WaitSetImpl_cond_map_delete
 */
RTI_PRIVATE DDS_ReturnCode_t
DDS_WaitSetImpl_cond_map(struct DDS_WaitSetImpl *self,
                     DDS_WaitSetMapFunction_T map,
                     void *param)
{
    struct DDS_ConditionRef *cref,*cref_next;
    DDS_ReturnCode_t rc = DDS_RETCODE_OK;
    RTI_BOOL done = RTI_FALSE;

    cref = DDS_WaitSetConditionRefList_get_first(&self->_conditions);
    while (!REDA_CircularList_node_at_head(&self->_conditions,&cref->_node))
    {
        cref_next = DDS_WaitSetConditionRefNode_next(&cref->_node);
        rc = map(self,cref,param,&done);

        if (done || (rc != DDS_RETCODE_OK))
        {
            break;
        }

        cref = cref_next;
    }

    return rc;
}

/*ci
 * \brief Wake-up all waitsets attached to a condition that was triggered
 *
 * \details
 * This function is used to unblock a waitset when a condition attached to the
 * waitset is triggered. Note that the trigger value may be false. If the
 * waitset is already unblocked it is not unblocked again. This case is handled
 * by scanning the state of the condition when the waitset blocks again.
 *
 * \param[in] self          The DDS_WaitSet to wakeup
 * \param[in] condition     The condition that triggered
 * \param[in] trigger_value The trigger_value of the condition
 */
void
DDS_WaitSetImpl_on_condition_triggered(struct DDS_WaitSetImpl *self,
                        struct DDS_ConditionRef *cref,
                        DDS_Boolean trigger_value)
{
    RTI_BOOL bretval;

    if (!OSAPI_Mutex_take(self->_state_lock))
    {
        OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
        return;
    }

    cref->flag_pending_trigger = DDS_BOOLEAN_TRUE;
    cref->flag_last_trigger = trigger_value;
    /* if the waitset is in BLOCKED state, transition it to UNBLOCKED */
    if (self->_state_blocked)
    {
        self->_state_blocked = DDS_BOOLEAN_FALSE;
        if (!OSAPI_Semaphore_give(self->_condition_active))
        {
            OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
            goto done;
        }
    }

done:

    bretval = OSAPI_Mutex_give(self->_state_lock);

#if OSAPI_ENABLE_LOG
    if (!bretval)
    {
        OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
    }
#else
    IGNORE_RETVAL(bretval);
#endif

    return;
}

/*ci
 * \brief Set the current trigger value of a condition to FALSE
 *
 * \details
 * This function is used to prevent a condition from triggering a waitset
 * if the waitset has already processed this condition as triggered.
 *
 * \param[in] self Waitset the condition is attached to
 * \param[in] cref Condition to reset
 */
void
DDS_WaitSetImpl_on_condition_reset(struct DDS_WaitSetImpl *self,
                        struct DDS_ConditionRef *cref)
{
    UNUSED_ARG(self);
    cref->flag_last_trigger = DDS_BOOLEAN_FALSE;
}

/*ce \dref_WaitSet_add_cond_ref
 *   \brief Add a reference to a condition
 *
 *   \details
 *   This function adds a reference to a condition to this waitset. A
 *   condition that has is attached to a waitset is allowed to wake-up
 *   the waitset.
 *
 *   It is allowed to attached a condition to a blocked waitset
 *
 *   If the condition is already attached to the waitset, then just return
 *   (See DDS standard, section 7.1.2.1.6.1)
 *
 *   If the condition has a trigger value of TRUE, then then condition
 *   will unblock the waitset (See DDS standard, section 7.1.2.1.6.1)
 *
 *   \return - DDS_RETCODE_OK is the reference was successfully added,
 *             DDS_RETCODE_ERROR if the addition failed
 *
 *   \mtsafety This function is thread safe
 *
 *   \sa DDS_WaitSet_remove_cond_ref
 */
DDS_ReturnCode_t
DDS_WaitSetImpl_add_cond_ref(struct DDS_WaitSetImpl *self, DDS_Condition *cond,
        DDS_Boolean is_active, struct DDS_ConditionRef **cref_out)
{
    struct DDS_ConditionRef *cref = NULL;
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;
    RTI_BOOL locked = RTI_FALSE;


    if (!OSAPI_Mutex_take(self->_state_lock))
    {
        retcode = DDS_RETCODE_ERROR;
        goto done;
    }
    locked = RTI_TRUE;

    retcode = DDS_WaitSetImpl_cond_map(self,
                DDS_WaitSetImpl_cond_map_exists,cond);

    /* OK means the condition wasn't found so it should be added,
     * ILLEGAL_OPERATION means that the condition was found,
     * ERROR means an error occurred while performing the operation;
     */
    if (retcode != DDS_RETCODE_OK)
    {
        goto done;
    }

    OSAPI_Heap_allocate_struct(&cref,struct DDS_ConditionRef);
    if (cref == NULL)
    {
        retcode = DDS_RETCODE_OUT_OF_RESOURCES;
        goto done;
    }

    REDA_CircularListNode_init(&cref->_node);
    cref->cond_ref = cond;
    cref->flag_pending_trigger = DDS_BOOLEAN_FALSE;
    cref->flag_return_next = DDS_BOOLEAN_FALSE;
    cref->flag_last_trigger = DDS_BOOLEAN_FALSE;

    REDA_CircularList_append(&self->_conditions,&cref->_node);
    ++self->_conditions_len;

    if (is_active)
    {
        DDS_WaitSetImpl_on_condition_triggered(self, cref, is_active);
    }

    locked = RTI_FALSE;
    if (!OSAPI_Mutex_give(self->_state_lock))
    {
        retcode = DDS_RETCODE_ERROR;
        goto done;
    }

done:
    if (locked)
    {
        if (!OSAPI_Mutex_give(self->_state_lock))
        {
            retcode = DDS_RETCODE_ERROR;
        }
    }

    if (retcode == DDS_RETCODE_OK)
    {
        *cref_out = cref;
    }
#if OSAPI_ENABLE_LOG
    else if (retcode != DDS_RETCODE_ILLEGAL_OPERATION)
    {
        OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
    }
#endif

    return retcode;
}

#ifndef RTI_CERT
/*ce \dref_WaitSet_remove_cond_ref
 *   \brief Remove a reference to a condition
 *
 *   \details
 *   This function removes a reference to a condition from this waitset. If the
 *   condition cannot be found that has is attached to a waitset is allowed to
 *   wake-up waitset.
 *
 *   \return - DDS_RETCODE_OK if the reference was successfully removed
 *             DDS_RETCODE_ENTITY_NOT_EXIST if the condition did not exist
 *
 *   \mtsafety This function is thread safe
 *
 *   \sa DDS_WaitSet_remove_cond_ref
 */
DDS_ReturnCode_t
DDS_WaitSetImpl_remove_cond_ref(struct DDS_WaitSetImpl *self,
            DDS_Condition *cond)
{
    DDS_ReturnCode_t rc = DDS_RETCODE_ERROR;

    if (!OSAPI_Mutex_take(self->_state_lock))
    {
        rc = DDS_RETCODE_ERROR;
        goto done;
    }

    rc = DDS_WaitSetImpl_cond_map(self,DDS_WaitSetImpl_cond_map_delete,cond);
    switch (rc)
    {
        case DDS_RETCODE_ILLEGAL_OPERATION:
            /* the condition was found and removed */
            rc = DDS_RETCODE_OK;
            break;
        case DDS_RETCODE_OK:
            /* map() reached the end without finding the condition */
            rc = DDS_RETCODE_BAD_PARAMETER;
            break;
        default:
            /* an error occurred */
            rc = DDS_RETCODE_ERROR;
            DDSC_LOG_WS_REMOVE_COND_REFERENCE(OSAPI_LOGKIND_ERROR)
            break;
    }

    if (!OSAPI_Mutex_give(self->_state_lock))
    {
        rc = DDS_RETCODE_ERROR;
    }

done:

    return rc;
}
#endif /* !RTI_CERT */

/*ce \dref_WaitSet_new
 *   \brief Create a new, empty waitset.
 *
 *   \details
 *   Create a new waitset. The waitset does not have a factory, so for this
 *   reason the waitset is allocated from the heap. A waitset maintains a list
 *   of conditions that have been attached with DDS_WaitSet_attach.
 *
 *   \return - A new, empty waitset on success or NULL on failure.
 *
 *   \mtsafety This function is thread safe
 *
 *   \sa DDS_WaitSet_delete
 */
DDS_WaitSet*
DDS_WaitSet_new(void)
{
    struct DDS_WaitSetImpl *retval = NULL;
    struct DDS_WaitSetImpl *awaitset = NULL;

    /* Try to get a waitset form the pool */
    OSAPI_Heap_allocate_struct(&awaitset,struct DDS_WaitSetImpl);
    if (awaitset == NULL)
    {
        DDSC_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_WAITSET_OBJECT)
        return NULL;
    }

    OSAPI_Memory_zero(awaitset,sizeof(struct DDS_WaitSetImpl));

    awaitset->_condition_active = OSAPI_Semaphore_new();
    if (awaitset->_condition_active == NULL)
    {
        goto done;
    }

    awaitset->_wait_ended = OSAPI_Semaphore_new();
    if (awaitset->_wait_ended == NULL)
    {
        goto done;
    }

    awaitset->_state_lock = OSAPI_Mutex_new();
    if (awaitset->_state_lock == NULL)
    {
        DDSC_LOG_OBJECT_ALLOCATE(OSAPI_LOGKIND_ERROR,DDSC_LOG_MUTEX_OBJECT)
        goto done;
    }

    REDA_CircularList_init(&awaitset->_conditions);

    /* Take the lock in order to suppress coverity warnings */
    if (!OSAPI_Mutex_take(awaitset->_state_lock ))
    {
        OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    awaitset->_conditions_len = 0;
    awaitset->_state_deleting = DDS_BOOLEAN_FALSE;
    awaitset->_state_waiting = DDS_BOOLEAN_FALSE;
    awaitset->_state_blocked = DDS_BOOLEAN_TRUE;

    if (!OSAPI_Mutex_give(awaitset->_state_lock ))
    {
        OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
        goto done;
    }

    retval = awaitset;

done:
#ifndef RTI_CERT
    if ((retval == NULL) && (awaitset != NULL))
    {
        DDS_WaitSet_delete(awaitset);
    }
#endif /* !RTI_CERT */

#if OSAPI_ENABLE_LOG
    if (retval == NULL)
    {
        OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
    }
#endif

    return retval;
}

#ifndef RTI_CERT
/*ce \dref_WaitSet_delete
 *   \brief Delete a waitset.
 *
 *   \details
 *   Calling this method deletes a waitset. A waitset can only be deleted from
 *   the thread that is blocking on it. Thus, either use timeouts are
 *   a guard condition to onblock a waitset that i
 *
 *   \return - A new, empty waitset on success or NULL on failure.
 *
 *   \mtsafety This function is thread safe
 *
 *   \sa DDS_WaitSet_new
 */
DDS_ReturnCode_t
DDS_WaitSet_delete(struct DDS_WaitSetImpl *self)
{
    RTI_INT32 ms = OSAPI_SEMAPHORE_TIMEOUT_INFINITE;
    RTI_INT32 fc = 0;
    DDS_ReturnCode_t rc = DDS_RETCODE_ERROR;
    DDS_Boolean waiting = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION((self == NULL),
                            return DDS_RETCODE_BAD_PARAMETER,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    if (!OSAPI_Mutex_take(self->_state_lock))
    {
        rc = DDS_RETCODE_ERROR;
        goto done;
    }

    self->_state_deleting = DDS_BOOLEAN_TRUE;
    waiting = self->_state_waiting;

    if (!OSAPI_Mutex_give(self->_state_lock))
    {
        rc = DDS_RETCODE_ERROR;
        goto done;
    }

    if (waiting)
    {
        if (!OSAPI_Semaphore_give(self->_condition_active))
        {
            rc = DDS_RETCODE_ERROR;
            goto done;
        }

        if (!OSAPI_Semaphore_take(self->_wait_ended,ms,&fc))
        {
            rc = DDS_RETCODE_ERROR;
            goto done;
        }

    }

    rc = DDS_WaitSetImpl_cond_map(self,DDS_WaitSetImpl_cond_map_delete,NULL);

    if (rc != DDS_RETCODE_OK)
    {
        goto done;
    }

    if (self->_condition_active != NULL)
    {
        if (!OSAPI_Semaphore_delete(self->_condition_active))
        {
            rc = DDS_RETCODE_ERROR;
            goto done;
        }

        self->_condition_active = NULL;
    }

    if (self->_wait_ended != NULL)
    {
        if (!OSAPI_Semaphore_delete(self->_wait_ended))
        {
            rc = DDS_RETCODE_ERROR;
            goto done;
        }

        self->_wait_ended = NULL;
    }

    if (self->_state_lock != NULL)
    {
        if (!OSAPI_Mutex_delete(self->_state_lock))
        {
            rc = DDS_RETCODE_ERROR;
            goto done;
        }

        self->_state_lock = NULL;
    }

    OSAPI_Heap_free_struct(self);

    rc = DDS_RETCODE_OK;

    done:

#if OSAPI_ENABLE_LOG
    if (rc != DDS_RETCODE_OK)
    {
        OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
    }
#endif

    return rc;
}
#endif /* !RTI_CERT */

/*ce \dref_WaitSet_wait
 *
 *
 */
/*ce \dref_WaitSet_wait
 * \brief Wakeup all waitsets attached to a condition that was triggered
 *
 * \details
 *
 * The caller must guarantee that the passed sequence has
 * sufficient memory to store the result of the operation
 * (at most all conditions attached to the WaitSet).
 *
 * \param[in]    self              The DDS_WaitSet to wait in
 * \param[inout] active_conditions On return a sequence of all active conditions
 * \param[in]    timeout           The maximum wait time
 *
 * \return DDS_RETCODE_OK on success, one of the standard DDS_ReturnCode_t on
 *        failure
 */
DDS_ReturnCode_t
DDS_WaitSet_wait(struct DDS_WaitSetImpl *self,
                 struct DDS_ConditionSeq *active_conditions,
                 const struct DDS_Duration_t *timeout)
{
    RTI_INT32 wait_sec = 0;
    RTI_UINT32 wait_nanosec = 0;
    RTI_INT32 fc = 0;
    DDS_ReturnCode_t rc = DDS_RETCODE_ERROR;
    RTI_BOOL locked = RTI_FALSE;

    OSAPI_PRECONDITION_ALWAYS(
                  (self == NULL) || (active_conditions == NULL) ||
                  (timeout == NULL),
                   return DDS_RETCODE_BAD_PARAMETER,
               OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("active_conditions",
                                           active_conditions,RTI_FALSE);
               OSAPI_Log_entry_add_pointer("timeout",timeout,RTI_TRUE);)


    if (DDS_Duration_is_infinite(timeout))
    {
        wait_sec = OSAPI_SEMAPHORE_TIMEOUT_INFINITE_SEC;
        wait_nanosec = OSAPI_SEMAPHORE_TIMEOUT_INFINITE_NANOSEC;
    }
    else
    {
        wait_sec = timeout->sec;
        wait_nanosec = timeout->nanosec;
    }

    if (!DDS_ConditionSeq_set_length(active_conditions, 0))
    {
        /* Cannot clear active_conditions list*/
        rc = DDS_RETCODE_ERROR;
        goto done;
    }

    if (!OSAPI_Mutex_take(self->_state_lock))
    {
        rc = DDS_RETCODE_ERROR;
        goto done;
    }
    locked = RTI_TRUE;

    /* if the waitset is already in WAITING state, it means that another thread
     * is currently blocked, waiting on its semaphore. Only one thread at a time
     * can be blocked on a waitset, and wait() must return
     * RETCODE_PRECONDITION_NOT_MET, if called on an already "occupied" waitset.
     */
    if (self->_state_waiting)
    {
        rc = DDS_RETCODE_PRECONDITION_NOT_MET;
        goto done;
    }

    /* scan the list of attached conditions to retrieve the "active" ones (i.e.
     * those whose trigger_value transitioned from FALSE to TRUE - possibly
     * multiple times and with possibly current value FALSE - and those whose
     * trigger_value remained TRUE - e.g. a guard_condition - between the last
     * time wait() returned and the time of this invocation) and clear their
     * "triggered" flag.
     */
    rc = DDS_WaitSetImpl_cond_map(self,DDS_WaitSetImpl_cond_map_wait_prescan,NULL);
    if (rc != DDS_RETCODE_OK)
    {
        goto done;
    }

    /* we will wait until the specified timeout for any of the condition
     * attached to the waitset to trigger or for a new, active, condition
     * to be attached to the waitset, so set the waitset also in WAITING state.
     */
    self->_state_waiting = DDS_BOOLEAN_TRUE;
    locked = RTI_FALSE;

    if (!OSAPI_Mutex_give(self->_state_lock))
    {
        rc = DDS_RETCODE_ERROR;
        goto done;
    }

    if (!OSAPI_Semaphore_take_sec_nanosec(self->_condition_active,
                                          wait_sec,wait_nanosec,&fc))
    {
        /* set rc so that we return after locking */
        rc = DDS_RETCODE_ERROR;
    }

    if (!OSAPI_Mutex_take(self->_state_lock))
    {
        self->_state_waiting = DDS_BOOLEAN_FALSE;
        rc = DDS_RETCODE_ERROR;
        goto done;
    }
    locked = RTI_TRUE;

    /* we are no longer waiting on the semaphore (either a condition triggered
     * or the specified timeout was reached), so set the waitset in NOT_WAITING
     * state. We might still be in BLOCKED state (i.e. if the timeout expired)
     */
    self->_state_waiting = DDS_BOOLEAN_FALSE;

    /* the OSAPI_Semaphore_take failed */
    if (rc != DDS_RETCODE_OK)
    {
        goto done;
    }

    self->_state_blocked = DDS_BOOLEAN_TRUE;

    /* if, while we were WAITING, state DELETING became active (i.e. application
     * called delete()), wait() must return RETCODE_ALREADY_DELETED, the content
     * of the returned sequence is undefined.
     * The _wait_ended semaphore must be signaled so that the thread performing
     * the delete() operation may be waken up and finish deleting the waitset.
     */
    if (self->_state_deleting)
    {
        rc = DDS_RETCODE_ALREADY_DELETED;
        goto done;
    }

    /* if take() on the semaphore timed out, wait() must return RETCODE_TIMEOUT,
     * the contents of the returned sequence are undefined.
     */
    if (fc == OSAPI_SEMAPHORE_RESULT_TIMEOUT)
    {
        rc = DDS_RETCODE_TIMEOUT;
        goto done;
    }

    /* scan the list conditions to retrieve the "active" ones (i.e. conditions
     * that triggered before wait() woke up from the semaphore, independently
     * of their current trigger_value) and reset their "triggered" flag
     */
    rc = DDS_WaitSetImpl_cond_map(self,DDS_WaitSetImpl_cond_map_wait_postscan,
                                  active_conditions);
    if (rc != DDS_RETCODE_OK)
    {
        goto done;
    }

    locked = RTI_FALSE;
    if (!OSAPI_Mutex_give(self->_state_lock))
    {
        rc = DDS_RETCODE_ERROR;
        goto done;
    }

done:

    if (locked)
    {
        if (!OSAPI_Mutex_give(self->_state_lock))
        {
            rc = DDS_RETCODE_ERROR;
        }
    }

    if (rc == DDS_RETCODE_ALREADY_DELETED)
    {
        if (!OSAPI_Semaphore_give(self->_wait_ended))
        {
            rc = DDS_RETCODE_ERROR;
        }
    }

#if OSAPI_ENABLE_LOG
    if (!(rc == DDS_RETCODE_OK || rc == DDS_RETCODE_TIMEOUT ||
                rc == DDS_RETCODE_ALREADY_DELETED))
    {
        OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
    }
#endif

    return rc;
}

/*ce \dref_WaitSet_attach_condition
 */
DDS_ReturnCode_t
DDS_WaitSet_attach_condition(DDS_WaitSet *self,DDS_Condition *cond)
{
    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (cond == NULL),
                                    return DDS_RETCODE_BAD_PARAMETER,
                     OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                     OSAPI_Log_entry_add_pointer("cond",cond,RTI_TRUE);)

    return DDS_ConditionImpl_add_waitset_ref(cond,
                (struct DDS_WaitSetImpl*)self);
}

#ifndef RTI_CERT
/*ce \dref_WaitSet_detach_condition
 */
DDS_ReturnCode_t
DDS_WaitSet_detach_condition(DDS_WaitSet *self,DDS_Condition *cond)
{
    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (cond == NULL),
                                return DDS_RETCODE_BAD_PARAMETER,
                            OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("cond",cond,RTI_TRUE);)

    return DDS_ConditionImpl_remove_waitset_ref(cond,self, DDS_BOOLEAN_TRUE);
}
#endif /* !RTI_CERT */

/*ce \dref_WaitSet_get_conditions
 *
 * The caller must guarantee that the passed sequence has
 * sufficient memory to store the result of the operation
 * (all conditions attached to the WaitSet).
 *
 */
DDS_ReturnCode_t
DDS_WaitSet_get_conditions(struct DDS_WaitSetImpl *self,
                           struct DDS_ConditionSeq *attached_conditions)
{
    DDS_ReturnCode_t rc = DDS_RETCODE_ERROR;
    RTI_INT32 seq_size = 0;
    RTI_INT32 i = 0;
    struct DDS_ConditionRef *cref = NULL;
    RTI_BOOL locked = RTI_FALSE;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (attached_conditions==NULL),
                           return DDS_RETCODE_BAD_PARAMETER,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                           OSAPI_Log_entry_add_pointer("attached_conditions",
                                               attached_conditions,RTI_TRUE);)

    if (!DDS_ConditionSeq_set_length(attached_conditions, 0))
    {
        rc = DDS_RETCODE_ERROR;
        goto done;
    }

    seq_size = DDS_ConditionSeq_get_maximum(attached_conditions);

    if (!OSAPI_Mutex_take(self->_state_lock))
    {
        rc = DDS_RETCODE_ERROR;
        goto done;
    }
    locked = RTI_TRUE;

    if (seq_size < self->_conditions_len)
    {
        rc = DDS_RETCODE_OUT_OF_RESOURCES;
        goto done;
    }

    if (!DDS_ConditionSeq_set_length(
            attached_conditions, self->_conditions_len))
    {
        rc = DDS_RETCODE_ERROR;
        goto done;
    }

    cref = DDS_WaitSetConditionRefList_get_first(&self->_conditions);
    while (!REDA_CircularList_node_at_head(&self->_conditions,&cref->_node))
    {
        /* Since set_length succeeded it is assumed that get_reference
         * returns a valid address.
         */
        /* coverity[dereference] */
        /* coverity[cert_exp34_c_violation] */
        *DDS_ConditionSeq_get_reference(attached_conditions,i) = cref->cond_ref;

        ++i;
        cref = DDS_WaitSetConditionRefNode_next(&cref->_node);
    }

    locked = RTI_FALSE;
    if (!OSAPI_Mutex_give(self->_state_lock))
    {
        rc = DDS_RETCODE_ERROR;
        goto done;
    }

    rc = DDS_RETCODE_OK;

    done:

    if (locked)
    {
        if (!OSAPI_Mutex_give(self->_state_lock))
        {
            rc = DDS_RETCODE_ERROR;
        }
    }

#if OSAPI_ENABLE_LOG
    if (rc != DDS_RETCODE_OK)
    {
        OSAPI_LOG_LAST_RECORDED_ERROR(OSAPI_LOGKIND_ERROR)
    }
#endif

    return rc;
}

/*ci @} */
