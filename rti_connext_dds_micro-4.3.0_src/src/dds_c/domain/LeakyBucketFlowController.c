/*
 * FILE: LeakyBucketFlowController.c - The builtin leaky-bucket FlowController
 *
 * Copyright (c) 2018-2024. Real-Time Innovations, Inc.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#include "LeakyBucketFlowController.h"
#include "DomainParticipant.h"

#if DDS_FLOW_CONTROLLER_ENABLED

RTI_PRIVATE void
DDS_LeakyBucketFlowController_schedule_flow(
                            struct DDS_LeakyBucketFlowController *self,
                            struct DDS_LeakyBucketFlowControllerTask *task);

RTI_PRIVATE RTI_BOOL
DDS_LeakyBucketFlowController_reschedule_flow(NETIO_FlowController *fc,
                                          NETIO_FlowControllerFlowHandle handle);

/*** SOURCE_BEGIN */

RTI_PRIVATE RTI_INT32
DDS_LeakyBucketFlowController_compare_task_by_priority(
                const struct DDS_LeakyBucketFlowControllerTask *const ltask,
                const struct DDS_LeakyBucketFlowControllerTask *const rtask)
{
    return ltask->priority - rtask->priority;
}

RTI_PRIVATE RTI_INT32
DDS_LeakyBucketlowController_compare_task_by_latency(
                const struct DDS_LeakyBucketFlowControllerTask *const ltask,
                const struct DDS_LeakyBucketFlowControllerTask *const rtask)
{
    return DDS_Duration_compare(&ltask->max_latency,&rtask->max_latency);
}

RTI_PRIVATE void
DDS_LeakyBucketFlowController_set_task_state(
                                REDA_CircularList_T *task_list,
                                NETIO_FlowControllerFlowState_T state)
{
    struct DDS_LeakyBucketFlowControllerTask *task;

    task = (struct DDS_LeakyBucketFlowControllerTask*)
                     REDA_CircularList_get_first(task_list);

    while (!REDA_CircularList_node_at_head(task_list,task))
    {
        task->state = state;
        task = (struct DDS_LeakyBucketFlowControllerTask*)
                            REDA_CircularListNode_get_next(&task->_parent);
    }
}

RTI_PRIVATE void
DDS_LeakyBucketFlowController_remove_task(
                                struct DDS_LeakyBucketFlowController *fc,
                                REDA_CircularList_T *task_list,
                                struct DDS_LeakyBucketFlowControllerTask *task)
{
    struct DDS_LeakyBucketFlowControllerTask *a_task;

    /* Walk backwards and decrement group count for all tasks in the same group */
    a_task = (struct DDS_LeakyBucketFlowControllerTask*)
                        REDA_CircularListNode_get_prev(&task->_parent);
    while ((fc->task_cmp(a_task,task) == 0) &&
            !REDA_CircularList_node_at_head(task_list,&a_task->_parent))
    {
        --a_task->group_count;
        a_task = (struct DDS_LeakyBucketFlowControllerTask*)
                        REDA_CircularListNode_get_prev(&a_task->_parent);
    }

    /* Walk forwards and decrement group count for all tasks in the same group */
    a_task = (struct DDS_LeakyBucketFlowControllerTask*)
                        REDA_CircularListNode_get_next(&task->_parent);
    while ((fc->task_cmp(a_task,task) == 0) &&
            !REDA_CircularList_node_at_head(task_list,&a_task->_parent))
    {
        --a_task->group_count;
        a_task = (struct DDS_LeakyBucketFlowControllerTask*)
                        REDA_CircularListNode_get_next(&a_task->_parent);
    }

    /* Finally unlink the task */
    REDA_CircularList_unlink_node(&task->_parent);
    task->state = NETIO_FLOW_CONTROLLER_FLOW_STATE_INVALID;

    /* Don't leave next_task pointing at a removed task */
    if (fc->next_task == task)
    {
        fc->next_task = NULL;
    }
}

RTI_PRIVATE NETIO_FlowControllerFlowState_T
DDS_LeakyBucketFlowController_update_task(
                                struct DDS_LeakyBucketFlowController *fc,
                                struct DDS_LeakyBucketFlowControllerTask *task,
                                RTI_INT32 bits_received,
                                RTI_INT32 *bits_used)
{
    UNUSED_ARG(fc);

    *bits_used = 0;

    return task->task_entry(task->task_param,bits_received,bits_used);
}

RTI_PRIVATE void
DDS_LeakyBucketFlowController_run_to_completion(
                                    struct DDS_LeakyBucketFlowController *fc,
                                    REDA_CircularList_T *task_list)
{
    struct DDS_LeakyBucketFlowControllerTask *task;
    struct DDS_LeakyBucketFlowControllerTask *task_prev;
    struct DDS_LeakyBucketFlowControllerTask *grp_task;
    RTI_INT32 bits_used;
    RTI_INT32 total_bits_used;
    const RTI_INT32 bits_per_task = INT_MAX;
    NETIO_FlowControllerFlowState_T task_state;

    /* Unlimited rate, publish until complete */
    while (!REDA_CircularList_is_empty(task_list))
    {
        task = (struct DDS_LeakyBucketFlowControllerTask*)
                         REDA_CircularList_get_first(task_list);
        grp_task = task;

        OSAPI_Trace_write("Distribute %^d bits each to %^d tasks",
                          &bits_per_task,
                          &task->group_count,
                          NULL,NULL,NULL,NULL,
                          NULL,NULL,NULL,NULL);

        total_bits_used = 0;
        while (!REDA_CircularList_node_at_head(task_list,task) &&
               (fc->task_cmp(task,grp_task) == 0))
        {
            /* Let each priority complete round robin */
            task_prev =(struct DDS_LeakyBucketFlowControllerTask*)
                             REDA_CircularListNode_get_next(&task->_parent);

            /* As task cannot remove itself, only return its state */
            task_state = DDS_LeakyBucketFlowController_update_task(fc,task,
                                                                   bits_per_task,
                                                                   &bits_used);

            total_bits_used += bits_used;

            /* This function is called with flow_lock released, so the lock must
             * be taken here to protect against concurrent calls to reschedule_flow()
             * and remove_flow() that could change the task state.
             */
            if (!OSAPI_Mutex_take(fc->flow_lock))
            {
                return;
            }
            if (((task_state == NETIO_FLOW_CONTROLLER_FLOW_STATE_COMPLETE) &&
                (task->state == NETIO_FLOW_CONTROLLER_FLOW_STATE_RUNNING)) ||
                task->is_deleted)
            {
                OSAPI_Trace_write("remove flow task %p from run to completion",
                                  task,
                                  &task->min_bits_per_run,
                                  NULL,NULL,NULL,NULL,NULL,
                                  NULL,NULL,NULL);
                DDS_LeakyBucketFlowController_remove_task(fc,task_list,task);
            }
            if (!OSAPI_Mutex_give(fc->flow_lock))
            {
                return;
            }

            task = task_prev;
        } /* while () */

        if (total_bits_used == 0)
        {
            /* If no bits were used, stop */
            break;
        }
    }
}

RTI_PRIVATE OSAPI_TaskState_T
DDS_LeakyBucketFlowController_run(struct OSAPI_Task *fc_task, RTI_BOOL period_expired)
{
    struct DDS_LeakyBucketFlowController *fc = OSAPI_BASE_FROM_MEMBER(fc_task,
                                  struct DDS_LeakyBucketFlowController,task);
    struct DDS_LeakyBucketFlowControllerTask *task;
    struct DDS_LeakyBucketFlowControllerTask *task_next;
    RTI_INT32 bits_used = 0, total_bits_used = INT_MAX;
    struct DDS_LeakyBucketFlowControllerTask *grp_task;
    RTI_INT32 bits_per_task;
    NETIO_FlowControllerFlowState_T task_state;
    OSAPI_TaskState_T os_task_state = OSAPI_TASK_STATE_READY;
    RTI_INT32 available_bits;

    if (!OSAPI_Mutex_take(fc->flow_lock))
    {
        return OSAPI_TASK_STATE_INVALID;
    }

    /* Move current list over to the run list and update the first and last
     * task to point to the run list
     */
    REDA_CircularList_init(&fc->run_list);
    if (!REDA_CircularList_is_empty(&fc->tasks))
    {
        fc->run_list = fc->tasks;
        fc->run_list._next->_prev = &fc->run_list;
        fc->run_list._prev->_next = &fc->run_list;
    }

    DDS_LeakyBucketFlowController_set_task_state(&fc->run_list,
                                NETIO_FLOW_CONTROLLER_FLOW_STATE_RUNNING);

    /* Empty current task list */
    fc->tasks._next = &fc->tasks;
    fc->tasks._prev = &fc->tasks;

    if (REDA_CircularList_is_empty(&fc->run_list))
    {
        if (!OSAPI_Mutex_give(fc->flow_lock))
        {
            return OSAPI_TASK_STATE_INVALID;
        }
        return OSAPI_TASK_STATE_COMPLETED;
    }

    /* Only update the accumulated bits if the period has expired */
    if (period_expired)
    {
        /* Subtract bits leaked during previous period */
        if (fc->leaked_bits_per_period == DDS_LENGTH_UNLIMITED)
        {
            fc->accumulated_bits = 0;
        }
        else if (fc->accumulated_bits != DDS_LENGTH_UNLIMITED)
        {
            /* leaked_bits_per_period is always >= 0 if not DDS_LENGTH_UNLIMITED */
            if (fc->leaked_bits_per_period >= fc->accumulated_bits)
            {
                fc->accumulated_bits = 0;
            }
            else
            {
                fc->accumulated_bits -= fc->leaked_bits_per_period;
            }
        }

        /* Add bits for the new period */
        if ((fc->added_bits_per_period != DDS_LENGTH_UNLIMITED) &&
            (INT_MAX - fc->accumulated_bits) > fc->added_bits_per_period)
        {
            fc->accumulated_bits += fc->added_bits_per_period;
        }
        else
        {
            fc->accumulated_bits = DDS_LENGTH_UNLIMITED;
        }

        /* If max_tokens is limited, limit the accumulated bits to max_tokens */
        if (fc->bits_per_max_token != DDS_LENGTH_UNLIMITED)
        {
            if ((fc->accumulated_bits == DDS_LENGTH_UNLIMITED) ||
                (fc->accumulated_bits > fc->bits_per_max_token))
            {
                fc->accumulated_bits = fc->bits_per_max_token;
            }
        }
    }

    available_bits = fc->accumulated_bits;
    if (!OSAPI_Mutex_give(fc->flow_lock))
    {
        return OSAPI_TASK_STATE_INVALID;
    }

    OSAPI_Trace_write("%s has accumulated %^d bits",
                      fc->_parent._parent.name,
                      &available_bits,
                      NULL,NULL,NULL,NULL,
                      NULL,NULL,NULL,NULL);

    if (available_bits == DDS_LENGTH_UNLIMITED)
    {
        DDS_LeakyBucketFlowController_run_to_completion(fc,&fc->run_list);
    }
    else
    {
        RTI_INT32 bits_per_token;
        REDA_CircularList_T c_group = REDA_CircularList_INITIALIZER;
        RTI_INT32 run_group = RTI_FALSE;

        /* Move incomplete tasks in the current group to this list temporarily
         * in case there are tokens left to redistribute or that a task could
         * not complete. Any leftover tasks are spliced back into the task
         * list.
         */
        REDA_CircularList_init(&c_group);

        if (fc->next_task == NULL)
        {
            task = (struct DDS_LeakyBucketFlowControllerTask*)
                                    REDA_CircularList_get_first(&fc->run_list);
        }
        else
        {
            task = fc->next_task;
        }

        OSAPI_Trace_write("task %p group count = %^d\n",
                          task,&task->group_count,NULL,NULL,NULL,
                          NULL,NULL,NULL,NULL,NULL);

        /* We know that bytes_per_token is not unlimited because available_bits
         * is not unlimited or we would have called run_to_completion.
         */
        bits_per_token = fc->_parent.property.token_bucket.bytes_per_token * DDS_BITS_PER_BYTE;

        /* Run at least one time */
        total_bits_used = INT_MAX;
        while ((available_bits >= bits_per_token) && (total_bits_used > 0))
        {
            grp_task = task;

            /* group_count is the number of tasks with equal priority, including
             * this task, and is therefore guaranteed to be greater than zero.
             */
            bits_per_task = available_bits / task->group_count;
            if (bits_per_task < bits_per_token)
            {
                /* Ensure that a task gets a whole token. Otherwise we could
                 * end up with never giving a task enough bits to make progress.
                 */
                bits_per_task = bits_per_token;
            }

            run_group = RTI_FALSE;
            total_bits_used = 0;

            while (!REDA_CircularList_node_at_head(&fc->run_list,task)
                    && (fc->task_cmp(task,grp_task) == 0)
                    && (available_bits >= bits_per_task))
            {
                task_next = (struct DDS_LeakyBucketFlowControllerTask*)
                                REDA_CircularListNode_get_next(&task->_parent);

                task_state = DDS_LeakyBucketFlowController_update_task(
                                    fc,task,bits_per_task,&bits_used);

                /* flow_lock was released before calling update_task above, so
                 * the lock must be taken here to protect against concurrent calls to
                 * reschedule_flow() and remove_flow() that could change the task state.
                 */
                if (!OSAPI_Mutex_take(fc->flow_lock))
                {
                    return OSAPI_TASK_STATE_INVALID;
                }
                if (((task_state == NETIO_FLOW_CONTROLLER_FLOW_STATE_COMPLETE) &&
                    (task->state == NETIO_FLOW_CONTROLLER_FLOW_STATE_RUNNING)) ||
                    task->is_deleted)
                {
                    OSAPI_Trace_write("remove flow task %p from run to distributed flow",
                                      task,
                                      &task->min_bits_per_run,
                                      NULL,NULL,NULL,NULL,NULL,
                                      NULL,NULL,NULL);

                    /* update function to remove from run list */
                    DDS_LeakyBucketFlowController_remove_task(fc,&fc->run_list,task);
                }
                else
                {
                    run_group = RTI_TRUE;
                }
                if (!OSAPI_Mutex_give(fc->flow_lock))
                {
                    return OSAPI_TASK_STATE_INVALID;
                }

                task = task_next;

                /* May be left over bits, only subtract what was used.
                 * Cannot be < 0
                 */
                available_bits -= bits_used;

                /* if a full round is made in the scheduler but no bits used
                 * then this scheduler will exit.
                 */
                total_bits_used += bits_used;
            } /* while () */

            if (run_group && (available_bits > 0))
            {
                /* More bits left at the same priority, start over */
                /* If there are leftover bits, run same group again */
                task = (struct DDS_LeakyBucketFlowControllerTask*)
                                        REDA_CircularList_get_first(&fc->run_list);
                fc->next_task = NULL;
            }
            else if (!REDA_CircularList_node_at_head(&fc->run_list,task))
            {
                /* Previous group done, task points to first in next group */
                fc->next_task = NULL;
            }
            else
            {
                /* Nothing more to do, out of bits */
                if (REDA_CircularList_node_at_head(&fc->run_list,task))
                {
                    fc->next_task = NULL;
                }
                else
                {
                    fc->next_task = task;
                }
                break;
            }
        }
    }

    if (!OSAPI_Mutex_take(fc->flow_lock))
    {
        return OSAPI_TASK_STATE_INVALID;
    }

    /* Merge the run_list with any new nodes */
    {
        struct DDS_LeakyBucketFlowControllerTask *m_task;
        REDA_CircularList_T new_list;

        REDA_CircularList_init(&new_list);

        /* Save new tasks added since the last run, if any */
        if (!REDA_CircularList_is_empty(&fc->tasks))
        {
            new_list = fc->tasks;
            new_list._next->_prev = &new_list;
            new_list._prev->_next = &new_list;
        }
        REDA_CircularList_init(&fc->tasks);
        /* Restore current task list from what is left */
        if (!REDA_CircularList_is_empty(&fc->run_list))
        {
            fc->tasks = fc->run_list;
            fc->tasks._next->_prev = &fc->tasks;
            fc->tasks._prev->_next = &fc->tasks;
        }
        REDA_CircularList_init(&fc->run_list);
        /* Merge any new tasks */
        while (!REDA_CircularList_is_empty(&new_list))
        {
            m_task = (struct DDS_LeakyBucketFlowControllerTask*)
                            REDA_CircularList_get_first(&new_list);

            REDA_CircularList_unlink_node(&m_task->_parent);
            DDS_LeakyBucketFlowController_schedule_flow(fc,m_task);

            /* New tasks were added, start over */
            fc->next_task = NULL;
        }

        DDS_LeakyBucketFlowController_set_task_state(
                                &fc->tasks,
                                NETIO_FLOW_CONTROLLER_FLOW_STATE_READY);
    }

    /* There are no more tasks, suspend the flow-controller */
    if (REDA_CircularList_is_empty(&fc->tasks))
    {
        os_task_state = OSAPI_TASK_STATE_COMPLETED;
    }

    if (!OSAPI_Mutex_give(fc->flow_lock))
    {
    }

    return os_task_state;
}

RTI_PRIVATE struct NETIO_FlowControllerI DDS_LeakyBucketFlowControllerI;

RTI_PRIVATE DDS_FlowController*
DDS_LeakyBucketFlowController_create(
                                struct NETIO_FlowControllerFactory *factory,
                                struct NETIO_FlowControllerProperty *property)
{
    struct DDS_LeakyBucketFlowController *fc = NULL;
    struct DDS_LeakyBucketFlowController *retval = NULL;
    struct REDA_BufferPoolProperty bp = REDA_BufferPoolProperty_INITIALIZER;
    struct DDS_FlowControllerProperty_t *prop =
                                (struct DDS_FlowControllerProperty_t*)property;
    DDS_DomainParticipant *dp;
    struct DDS_DomainParticipantQos *dp_qos;

    UNUSED_ARG(factory);

    OSAPI_Heap_allocate_struct(&fc,struct DDS_LeakyBucketFlowController);
    if (fc == NULL)
    {
        return NULL;
    }

    if (!DDS_FlowController_initialize(&fc->_parent,
                                       &DDS_LeakyBucketFlowControllerI,
                                       prop))
    {
#ifndef RTI_CERT
        OSAPI_Heap_free_struct(fc);
#endif
        return NULL;
    }

    fc->flow_lock = OSAPI_Mutex_new();
    if (fc->flow_lock == NULL)
    {
        goto done;
    }

    if (fc->_parent.property.scheduling_policy != DDS_EDF_FLOW_CONTROLLER_SCHED_POLICY)
    {
        fc->task_cmp = DDS_LeakyBucketFlowController_compare_task_by_priority;
    }
    else
    {
        fc->task_cmp = DDS_LeakyBucketlowController_compare_task_by_latency;
    }

    fc->task_pool = NULL;
    REDA_CircularList_init(&fc->tasks);
    REDA_CircularList_init(&fc->run_list);

    /* Take the lock in order to suppress coverity warnings */
    if (!OSAPI_Mutex_take(fc->flow_lock))
    {
        goto done;
    }
    fc->accumulated_bits = 0;

    if ((fc->_parent.property.token_bucket.tokens_added_per_period == DDS_LENGTH_UNLIMITED) ||
        (fc->_parent.property.token_bucket.bytes_per_token == DDS_LENGTH_UNLIMITED))
    {
        fc->added_bits_per_period = DDS_LENGTH_UNLIMITED;
    }
    else
    {
        fc->added_bits_per_period = fc->_parent.property.token_bucket.tokens_added_per_period *
                                    fc->_parent.property.token_bucket.bytes_per_token *
                                    DDS_BITS_PER_BYTE;
    }

    if (fc->_parent.property.token_bucket.tokens_leaked_per_period == 0)
    {
        fc->leaked_bits_per_period = 0;
    }
    else if ((fc->_parent.property.token_bucket.tokens_leaked_per_period == DDS_LENGTH_UNLIMITED) ||
             (fc->_parent.property.token_bucket.bytes_per_token == DDS_LENGTH_UNLIMITED))
    {
        fc->leaked_bits_per_period = DDS_LENGTH_UNLIMITED;
    }
    else
    {
        fc->leaked_bits_per_period = fc->_parent.property.token_bucket.tokens_leaked_per_period *
                                     fc->_parent.property.token_bucket.bytes_per_token *
                                     DDS_BITS_PER_BYTE;
    }

    if ((fc->_parent.property.token_bucket.max_tokens == DDS_LENGTH_UNLIMITED) ||
        (fc->_parent.property.token_bucket.bytes_per_token == DDS_LENGTH_UNLIMITED))
    {
        fc->bits_per_max_token = DDS_LENGTH_UNLIMITED;
    }
    else
    {
        fc->bits_per_max_token = fc->_parent.property.token_bucket.max_tokens *
                                 DDS_BITS_PER_BYTE *
                                 fc->_parent.property.token_bucket.bytes_per_token;
    }

    fc->next_task = NULL;

    dp = (DDS_DomainParticipant*)fc->_parent._parent.owner;
    dp_qos = DDS_DomainParticipant_get_qos_ref(dp);

    bp.buffer_size = sizeof(struct DDS_LeakyBucketFlowControllerTask);
    bp.max_buffers = (RTI_SIZE_T)dp_qos->resource_limits.local_writer_allocation;

    bp.flags |= REDA_BUFFERPOOL_FLAG_ARRAY_ALLOC;
    fc->task_pool = REDA_BufferPool_new("ddsfc_taskpool",
                                        &bp,NULL,NULL,NULL,NULL);

    if (!OSAPI_Mutex_give(fc->flow_lock))
    {
        goto done;
    }

    if (fc->task_pool == NULL)
    {
        goto done;
    }

    if (!OSAPI_Task_initialize(&fc->task,DDS_LeakyBucketFlowController_run))
    {
        goto done;
    }

    OSAPI_Trace_write("Successfully created flow-controller %s",
                      prop->_parent.name,NULL,NULL,NULL,NULL,
                      NULL,NULL,NULL,NULL,NULL);

    retval = fc;

done:

#ifndef RTI_CERT
    if (retval == NULL)
    {
        RTI_BOOL bretval;

        if (fc->task_pool != NULL)
        {
            bretval = REDA_BufferPool_delete(fc->task_pool);
            IGNORE_RETVAL(bretval);

        }

        if (fc->flow_lock != NULL)
        {
            bretval = OSAPI_Mutex_delete(fc->flow_lock);
            IGNORE_RETVAL(bretval);
        }

        DDS_FlowController_finalize(&fc->_parent);

        OSAPI_Heap_free_struct(fc);
    }
#endif

    return &retval->_parent;
}

#ifndef RTI_CERT
RTI_PRIVATE void
DDS_LeakyBucketFlowController_delete(
                    struct NETIO_FlowControllerFactory *factory,
                    DDS_FlowController *self)
{
    struct DDS_LeakyBucketFlowController *fc =
                                (struct DDS_LeakyBucketFlowController*)self;
    struct DDS_LeakyBucketFlowControllerTask *task;
    struct DDS_LeakyBucketFlowControllerTask *task_next;
    DDS_DomainParticipant *dp = (DDS_DomainParticipant*)fc->_parent._parent.owner;

    UNUSED_ARG(factory);
    UNUSED_ARG(fc);

    OSAPI_Task_deschedule(&dp->flow_control.task_scheduler,&fc->task);

    if (fc->_parent._parent.name != NULL)
    {
        DDS_String_free((char*)fc->_parent._parent.name);
        fc->_parent._parent.name = NULL;
    }

    fc->_parent._parent.owner = NULL;

    task = (struct DDS_LeakyBucketFlowControllerTask*)
                            REDA_CircularList_get_first(&fc->tasks);

    while (!REDA_CircularList_node_at_head(&fc->tasks,task))
    {
        task_next = (struct DDS_LeakyBucketFlowControllerTask*)
                                REDA_CircularListNode_get_next(&task->_parent);

        if (task->task_finalize != NULL)
        {
            task->task_finalize(task->task_param);
        }
        REDA_CircularList_unlink_node(&task->_parent);
        REDA_BufferPool_return_buffer(fc->task_pool,task);
        task = task_next;
    }

    task = (struct DDS_LeakyBucketFlowControllerTask*)
                            REDA_CircularList_get_first(&fc->run_list);

    while (!REDA_CircularList_node_at_head(&fc->run_list,task))
    {
        task_next = (struct DDS_LeakyBucketFlowControllerTask*)
                                REDA_CircularListNode_get_next(&task->_parent);

        if (task->task_finalize != NULL)
        {
            task->task_finalize(task->task_param);
        }
        REDA_CircularList_unlink_node(&task->_parent);
        REDA_BufferPool_return_buffer(fc->task_pool,task);
        task = task_next;
    }

    if (!REDA_BufferPool_delete(fc->task_pool))
    {
    }

    if (fc->flow_lock != NULL)
    {
        OSAPI_Mutex_delete(fc->flow_lock);
    }

    DDS_FlowController_finalize(&fc->_parent);

    OSAPI_Heap_free_struct(fc);
}
#endif

RTI_PRIVATE void
DDS_LeakyBucketFlowController_send(NETIO_FlowController *fc,
                                struct OSAPI_SystemTime *time)
{
    UNUSED_ARG(fc);
    UNUSED_ARG(time);
}

RTI_PRIVATE void
DDS_LeakyBucketFlowController_schedule_flow(
                            struct DDS_LeakyBucketFlowController *self,
                            struct DDS_LeakyBucketFlowControllerTask *task)
{
    struct DDS_LeakyBucketFlowControllerTask *a_task;
    struct DDS_LeakyBucketFlowControllerTask *first_equal = NULL;
    struct DDS_LeakyBucketFlowControllerTask *last_equal = NULL;
    RTI_INT32 group_count = 1;

    task->state = NETIO_FLOW_CONTROLLER_FLOW_STATE_READY;

    /* Task list is empty, just add the task */
    if (REDA_CircularList_is_empty(&self->tasks))
    {
        task->group_count = 1;
        REDA_CircularList_append(&self->tasks,&task->_parent);
        return;
    }

    /* Walk the list until we find the insertion point which is either before
     * the first task with lower priority or at the end of the list.
     * While walking, keep track of the range of tasks with equal priority.
     */
    a_task = (struct DDS_LeakyBucketFlowControllerTask*)
                                REDA_CircularList_get_first(&self->tasks);
    while (!REDA_CircularList_node_at_head(&self->tasks,&a_task->_parent))
    {
        const RTI_INT32 cmp = self->task_cmp(task,a_task);

        if (cmp < 0)
        {
            /* Found the insertion point */
            break;
        }

        if (cmp == 0)
        {
            if (first_equal == NULL)
            {
                first_equal = a_task;
            }
            last_equal = a_task;
            group_count++;
        }

        a_task = (struct DDS_LeakyBucketFlowControllerTask*)
                            REDA_CircularListNode_get_next(&a_task->_parent);
    }

    if (first_equal == NULL)
    {
        /* No equal priority found, just insert the task */
        task->group_count = 1;
        REDA_CircularList_link_node_after(a_task->_parent._prev,&task->_parent);
        return;
    }

    /* Update the group count for all equal priority tasks */
    a_task = first_equal;
    while (!REDA_CircularList_node_at_head(&self->tasks,&a_task->_parent))
    {
        a_task->group_count = group_count;
        if (a_task == last_equal)
        {
            break;
        }
        a_task = (struct DDS_LeakyBucketFlowControllerTask*)
                    REDA_CircularListNode_get_next(&a_task->_parent);
    }

    task->group_count = group_count;
    REDA_CircularList_link_node_after(&last_equal->_parent,&task->_parent);
}

RTI_PRIVATE NETIO_FlowControllerFlowHandle
DDS_LeakyBucketFlowController_add_flow(NETIO_FlowController *fc,
                                       struct NETIO_Guid *netio_guid,
                                       NETIO_FlowControllerFlowSendData_T send_data,
                                       NETIO_FlowControllerFlowFinalize_T flow_finalize,
                                       void *flow_param,
                                       struct NETIO_FlowProperty *property)
{
    struct DDS_LeakyBucketFlowController *self = (struct DDS_LeakyBucketFlowController*)fc;
    struct DDS_LeakyBucketFlowControllerTask *task;
    struct DDS_LeakyBucketFlowControllerTask task_init = DDS_LeakyBucketFlowControllerTask_INITIALIZER;
    DDS_DomainParticipant *dp = (DDS_DomainParticipant*)self->_parent._parent.owner;
    DDS_Entity *entity = NULL;
    RTI_BOOL retval;

    entity = DDS_DomainParticipant_lookup_entity(dp,netio_guid);
    if (entity == NULL)
    {
        return NULL;
    }

    if (!DDS_ObjectId_is_writer(entity->entity_id))
    {
        return NULL;
    }

    if (!OSAPI_Mutex_take(self->flow_lock))
    {
        return NULL;
    }

    task = (struct DDS_LeakyBucketFlowControllerTask*)
                                REDA_BufferPool_get_buffer(self->task_pool);
    if (task == NULL)
    {
        /* The function is returning regardless, ignore retval */
        retval = OSAPI_Mutex_give(self->flow_lock);
        IGNORE_RETVAL(retval);
        return NULL;
    }

    OSAPI_Trace_write("Added flow %G to %s",
                      netio_guid,fc->name,
                      NULL,NULL,NULL,NULL,
                      NULL,NULL,NULL,NULL);

    *task = task_init;
    task->task_entry  = send_data;
    task->task_finalize = flow_finalize;
    task->task_param = flow_param;
    task->fc = self;
    task->min_bits_per_run = property->min_bits_required;
    task->group_count = 1;

    if (self->_parent.property.scheduling_policy ==
                                        DDS_HPF_FLOW_CONTROLLER_SCHED_POLICY)
    {
        task->priority = property->priority;
    }
    else if (self->_parent.property.scheduling_policy ==
                                        DDS_EDF_FLOW_CONTROLLER_SCHED_POLICY)
    {
        DDS_DataWriter *dw = (DDS_DataWriter*)entity;
        const struct DDS_LatencyBudgetQosPolicy *latency_budget;

        latency_budget = DDS_DataWriter_get_latency_budget_ref(dw);

        task->priority = 0;
        task->max_latency = latency_budget->duration;
    }
    else
    {
        /* Round robin */
    }

    DDS_LeakyBucketFlowController_schedule_flow(self,task);

    OSAPI_Trace_write("Added task %p "
                      "min_per_run=%^d at priority=%^d"
                      "group_count=%^d",
                      task,
                      &task->min_bits_per_run,
                      &task->priority,
                      &task->group_count,
                      NULL,NULL,NULL,NULL,NULL,NULL);

    if (!OSAPI_Mutex_give(self->flow_lock))
    {
        return NULL;
    }

    if (!OSAPI_Task_schedule_periodic(&dp->flow_control.task_scheduler,
                &self->task,
                self->_parent.property.token_bucket.period.sec,
                self->_parent.property.token_bucket.period.nanosec,
                0,
                RTI_FALSE))
    {
    }

    return (NETIO_FlowControllerFlowHandle)task;
}

RTI_PRIVATE RTI_BOOL
DDS_LeakyBucketFlowController_remove_flow(NETIO_FlowController *fc,
                                 NETIO_FlowControllerFlowHandle handle)
{
    struct DDS_LeakyBucketFlowController *self =
                            (struct DDS_LeakyBucketFlowController*)fc;
    struct DDS_LeakyBucketFlowControllerTask *task =
                            (struct DDS_LeakyBucketFlowControllerTask*)handle;
    RTI_UINT32 wait_count = 0;
    const RTI_UINT32 max_wait_count = 100;

    OSAPI_Trace_write("remove flow task %p ",
                      task,
                      &task->min_bits_per_run,
                      NULL,NULL,NULL,NULL,NULL,
                      NULL,NULL,NULL);

    if (!OSAPI_Mutex_take(self->flow_lock))
    {
        return RTI_FALSE;
    }

    task->is_deleted = RTI_TRUE;

    /* Wait only while still linked and owned by a run; an unlinked task can no
     * longer be reached by run(), so it is safe to free. Sleep with the lock
     * released so run() can progress and transition the task.
     */
    while (REDA_CircularListNode_is_linked(&task->_parent) &&
           ((task->state == NETIO_FLOW_CONTROLLER_FLOW_STATE_RUNNING) ||
            (task->state == NETIO_FLOW_CONTROLLER_FLOW_STATE_RESCHEDULED)))
    {
        if (!OSAPI_Mutex_give(self->flow_lock))
        {
            return RTI_FALSE;
        }

        /* Bound the wait time to avoid hanging indefinitely */
        ++wait_count;
        if (wait_count > max_wait_count)
        {
            return RTI_FALSE;
        }

        OSAPI_Thread_nanosleep(1000000);

        if (!OSAPI_Mutex_take(self->flow_lock))
        {
            return RTI_FALSE;
        }
    }

    /* run() may already have unlinked it; unlink_node is idempotent */
    REDA_CircularList_unlink_node(&task->_parent);
    if (self->next_task == task)
    {
        self->next_task = NULL;
    }
    if (task->task_finalize != NULL)
    {
        task->task_finalize(task->task_param);
    }

    REDA_BufferPool_return_buffer(self->task_pool,task);

    if (!OSAPI_Mutex_give(self->flow_lock))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

RTI_PRIVATE RTI_BOOL
DDS_LeakyBucketFlowController_reschedule_flow(NETIO_FlowController *fc,
                                          NETIO_FlowControllerFlowHandle handle)
{
    struct DDS_LeakyBucketFlowController *self =
                                    (struct DDS_LeakyBucketFlowController*)fc;
    struct DDS_LeakyBucketFlowControllerTask *task = NULL;
    struct DDS_LeakyBucketFlowControllerTask *found_task = NULL;
    DDS_DomainParticipant *dp = (DDS_DomainParticipant*)self->_parent._parent.owner;
    RTI_BOOL have_bits;

    if (!OSAPI_Mutex_take(self->flow_lock))
    {
        return RTI_FALSE;
    }

    task = (struct DDS_LeakyBucketFlowControllerTask*)handle;
    if ((task->state == NETIO_FLOW_CONTROLLER_FLOW_STATE_RUNNING) ||
        (task->state == NETIO_FLOW_CONTROLLER_FLOW_STATE_RESCHEDULED))
    {
        task->state = NETIO_FLOW_CONTROLLER_FLOW_STATE_RESCHEDULED;
        goto done;
    }

    task = (struct DDS_LeakyBucketFlowControllerTask*)
                            REDA_CircularList_get_first(&self->tasks);

    while (!REDA_CircularList_node_at_head(&self->tasks,task))
    {
        if (task == handle)
        {
            found_task = task;
            break;
        }
        task = (struct DDS_LeakyBucketFlowControllerTask*)
                    REDA_CircularListNode_get_next(&task->_parent);
    }

    if (found_task == NULL)
    {
        /* Not found, append it, reset iterator task */
        task = (struct DDS_LeakyBucketFlowControllerTask*)handle;
        DDS_LeakyBucketFlowController_schedule_flow(self,task);
    }

    OSAPI_Trace_write("rescheduled task (1) %p  "
                      "min_per_run=%^d",
                      task,
                      &task->min_bits_per_run,
                      NULL,NULL,NULL,NULL,NULL,
                      NULL,NULL,NULL);

done:
    /* Wake up the task before the next period expires if there are still more
     * bits available in the current period, unless if leaked_bits_per_period
     * is unlimited which implies that the task is only allowed to run once
     * per period.
     */
    have_bits = ((self->accumulated_bits > 0) || (self->accumulated_bits == DDS_LENGTH_UNLIMITED))
                && (self->leaked_bits_per_period != DDS_LENGTH_UNLIMITED);

    if (!OSAPI_Mutex_give(self->flow_lock))
    {
        return RTI_FALSE;
    }

    if (!OSAPI_Task_schedule_periodic(&dp->flow_control.task_scheduler,
                                      &self->task,
                                      self->_parent.property.token_bucket.period.sec,
                                      self->_parent.property.token_bucket.period.nanosec,
                                      0,
                                      have_bits))
    {
    }

    return RTI_TRUE;
}

RTI_PRIVATE struct NETIO_FlowControllerI DDS_LeakyBucketFlowControllerI =
{
        RT_COMPONENTI_BASE,
        DDS_LeakyBucketFlowController_send,
        DDS_LeakyBucketFlowController_add_flow,
        DDS_LeakyBucketFlowController_remove_flow,
        DDS_LeakyBucketFlowController_reschedule_flow,
        NULL
};

MUST_CHECK_RETURN RTI_PRIVATE struct RT_ComponentFactory*
DDS_LeakyBucketFlowControllerFactory_initialize(
                                struct RT_ComponentFactoryProperty* property,
                                struct RT_ComponentFactoryListener *listener);

#ifndef RTI_CERT
RTI_PRIVATE void
DDS_LeakyBucketFlowControllerFactory_finalize(struct RT_ComponentFactory *factory,
                                   struct RT_ComponentFactoryProperty **property,
                                   struct RT_ComponentFactoryListener **listener);
#endif

MUST_CHECK_RETURN RTI_PRIVATE RT_Component_T*
DDS_LeakyBucketFlowControllerFactory_create_component(
                                   struct RT_ComponentFactory *factory,
                                   struct RT_ComponentProperty *property,
                                   struct RT_ComponentListener *listener)
{
    UNUSED_ARG(listener);

    return (RT_Component_T*)DDS_LeakyBucketFlowController_create(
                        (struct NETIO_FlowControllerFactory*)factory,
                        (struct NETIO_FlowControllerProperty*)property);
}

#ifndef RTI_CERT
/*ci
 * \brief Delete an instance of the UDP interface
 *
 * \details
 * Implementation of the RT ComponentFactory delete method. This method is not
 * called directly, only via the factory interface.
 *
 * \param[in] factory   The factory that created the component
 * \param[in] property  The component to be deleted
 *
 * \sa \ref UDP_InterfaceFactory_create_component
 */
RTI_PRIVATE void
DDS_LeakyBucketFlowControllerFactory_delete_component(
                                        struct RT_ComponentFactory *factory,
                                           RT_Component_T *component)
{
    DDS_LeakyBucketFlowController_delete(
                        (struct NETIO_FlowControllerFactory*)factory,
                        (DDS_FlowController*)component);
}
#endif /* !RTI_CERT */

#ifndef RTI_CERT
#define DDS_LeakyBucketFlowControllerFactory_finalize_impl \
        DDS_LeakyBucketFlowControllerFactory_finalize
#define DDS_LeakyBucketFlowControllerFactory_delete_component_impl \
        DDS_LeakyBucketFlowControllerFactory_delete_component
#else
#define DDS_LeakyBucketFlowControllerFactory_finalize_impl NULL
#define DDS_LeakyBucketFlowControllerFactory_delete_component_impl  NULL
#endif

RTI_PRIVATE struct RT_ComponentFactoryI DDS_FlowControllerFactory_fv_Intf =
{
        RT_MKINTERFACEID(RT_COMPONENT_CLASS_NETIO_FLOWCONTROLLER,
                         RT_COMPONENT_INSTANCE_DDS_FLOWCONTROLLER),
        DDS_LeakyBucketFlowControllerFactory_initialize,
        DDS_LeakyBucketFlowControllerFactory_finalize_impl,
        DDS_LeakyBucketFlowControllerFactory_create_component,
        DDS_LeakyBucketFlowControllerFactory_delete_component_impl,
        NULL,
        NULL
};

RTI_PRIVATE struct RT_ComponentFactory DDS_FlowControllerFactory_fv_Factory =
{
    &DDS_FlowControllerFactory_fv_Intf,
    NULL,
    {{{0,0}}}
};

MUST_CHECK_RETURN RTI_PRIVATE struct RT_ComponentFactory*
DDS_LeakyBucketFlowControllerFactory_initialize(
                                struct RT_ComponentFactoryProperty* property,
                                struct RT_ComponentFactoryListener *listener)
{
    UNUSED_ARG(property);
    UNUSED_ARG(listener);

    DDS_FlowControllerFactory_fv_Factory._factory = &DDS_FlowControllerFactory_fv_Factory;

    return &DDS_FlowControllerFactory_fv_Factory;
}

#ifndef RTI_CERT
RTI_PRIVATE void
DDS_LeakyBucketFlowControllerFactory_finalize(struct RT_ComponentFactory *factory,
                               struct RT_ComponentFactoryProperty **property,
                               struct RT_ComponentFactoryListener **listener)
{
    UNUSED_ARG(factory);
    UNUSED_ARG(property);
    UNUSED_ARG(listener);
}
#endif

DDS_ReturnCode_t
DDS_LeakyBucketFlowControllerFactory_register(RT_Registry_T *registry,
                                              const char* const name)
{
    RTI_BOOL brc = RTI_FALSE;

    if (name == NULL)
    {
        brc = RT_Registry_system_register(registry,"ddsfc",
                                   &DDS_FlowControllerFactory_fv_Intf,
                                   NULL,NULL);
    }
    else
    {
        brc = RT_Registry_system_register(registry,name,
                                   &DDS_FlowControllerFactory_fv_Intf,
                                   NULL,NULL);
    }

    if (!brc)
    {
        return DDS_RETCODE_ERROR;
    }

    OSAPI_Trace_write("Successfully registered default flow-controller",
                      NULL,NULL,NULL,NULL,NULL,
                      NULL,NULL,NULL,NULL,NULL);

    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t
DDS_LeakyBucketFlowControllerFactory_unregister(RT_Registry_T *registry,
                                                const char* const name)
{
    RTI_BOOL brc = RTI_FALSE;

    if (name == NULL)
    {
        brc = RT_Registry_unregister(registry,"ddsfc",
                                   NULL,NULL);
    }
    else
    {
        brc = RT_Registry_unregister(registry,name,
                                     NULL,NULL);
    }

    if (!brc)
    {
        return DDS_RETCODE_ERROR;
    }

    OSAPI_Trace_write("Successfully unregistered default flow-controller",
                      NULL,NULL,NULL,NULL,NULL,
                      NULL,NULL,NULL,NULL,NULL);

    return DDS_RETCODE_OK;
}

#endif
