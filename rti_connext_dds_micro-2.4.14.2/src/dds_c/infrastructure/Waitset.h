/*
 * (c) Copyright 2012-2015 Real-Time Innovations,
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
 * 20oct2021,tk MICRO-3288/PR.29702
 * - Exclude _state_deleting flag and _wait_ended semaphore for CERT because
 *   WaitSet_delete is excluded from CERT.
 * 09jul2015,tk MICRO-1402/PR#15274 Removed unused _node variable
 */
/*/ci
 * \file
 * \brief This file implements the standard DDS Waitset API. The implementation
 *        complies with OMG DDS 1.2 07-01-01
 */
/*ci \addtogroup DDSInfrastructureModule
 * @{
 */
#ifndef Waitset_pkg_h
#define Waitset_pkg_h

#ifdef __cplusplus
    extern "C" {
#endif

/*ci
 *\brief DDS_WaitSet implementation structure
 */
struct DDS_WaitSetImpl
{
    /*ci \dref_condition_active
     * Semaphore to block until at least one condition is true. Note that
     * the semaphore is binary; This is per design and the specification. It
     * is sufficient that at least one condition is true.
     */
    OSAPI_Semaphore_T *_condition_active;

#ifndef RTI_CERT

    /*ci \dref_condition_active
     * Semaphore to block until an ongoing call to wait() has ended
     * when deleting the Waitset from a different thread.
     */
    OSAPI_Semaphore_T *_wait_ended;
#endif

    /*ci \dref_cond_list_block
     * Mutex to protect the manipulation of the Waitset's state. Because
     * a waitset does not have a factory, it is completely independent of
     * any other DDS entity and consequently not included in any other
     * critical section
     */
    OSAPI_Mutex_T *_state_lock;

    /*ci \dref_conditions
     * A list of _conditions attached to this waitset. A waitset only keeps
     * a reference to a condition and only watches its state.
     */
    REDA_CircularList_T _conditions;

    /*ci \dref_conditions_len
     * Current number of conditions attached to this waitset. This value
     * must be kept in sync with the state of the _conditions list.
     */
    RTI_INT32 _conditions_len;

    /*ci
     * \brief TRUE if the waitset is busy
     */
    DDS_Boolean _state_waiting;

    /*ci
     * \brief TRUE if the waitset is currently blocked
     */
    DDS_Boolean _state_blocked;

#ifndef RTI_CERT
    /*ci
     * \brief TRUE if the waitset is being deleted
     */
    DDS_Boolean _state_deleting;
#endif
};

struct DDS_ConditionRef;

extern void
DDS_WaitSetImpl_on_condition_triggered(
        struct DDS_WaitSetImpl *self,
        struct DDS_ConditionRef *condition_ref,
        DDS_Boolean trigger_value);

extern void
DDS_WaitSetImpl_on_condition_reset(struct DDS_WaitSetImpl *self,
                        struct DDS_ConditionRef *cref);

extern DDS_ReturnCode_t
DDS_WaitSetImpl_add_cond_ref(struct DDS_WaitSetImpl *self,DDS_Condition *cond,
                             DDS_Boolean is_active,
                             struct DDS_ConditionRef **cref_out);

#ifndef RTI_CERT
extern DDS_ReturnCode_t
DDS_WaitSetImpl_remove_cond_ref(struct DDS_WaitSetImpl *self,DDS_Condition *cond);
#endif

#ifdef __cplusplus
    }   /* extern "C" */
#endif

#endif

/*ci @} */
