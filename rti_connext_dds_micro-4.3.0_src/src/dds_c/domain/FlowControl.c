/*
 * FILE: FlowController.c - FlowController Implementation
 *
 * (c) Copyright, Real-Time Innovations, 2018-2024.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#include "FlowControl.h"
#include "DomainParticipant.h"

#if DDS_FLOW_CONTROLLER_ENABLED

const char *const
DDS_DEFAULT_FLOW_CONTROLLER_NAME = "DDS_DEFAULT_FLOW_CONTROLLER_NAME";

const char *const
DDS_FIXED_RATE_FLOW_CONTROLLER_NAME = "DDS_FIXED_RATE_FLOW_CONTROLLER_NAME";

const char *const
DDS_ON_DEMAND_FLOW_CONTROLLER_NAME = "DDS_ON_DEMAND_FLOW_CONTROLLER_NAME";

const struct DDS_FlowControllerProperty_t DDS_FLOW_CONTROLLER_PROPERTY_DEFAULT =
                                                DDS_FlowControllerProperty_t_INITIALIZER;

/*** SOURCE_BEGIN */

void
DDS_FlowControllerProperty_t_initialize(struct DDS_FlowControllerProperty_t *p)
{
    struct DDS_FlowControllerProperty_t ip = DDS_FlowControllerProperty_t_INITIALIZER;

    *p = ip;
}

RTI_INT32
DDS_FlowControllerProperty_t_is_equal(const struct DDS_FlowControllerProperty_t *left,
                                      const struct DDS_FlowControllerProperty_t *right)
{
    return (left->token_bucket.bytes_per_token == right->token_bucket.bytes_per_token) &&
           (left->token_bucket.max_tokens == right->token_bucket.max_tokens) &&
           (left->token_bucket.tokens_added_per_period == right->token_bucket.tokens_added_per_period) &&
           (left->token_bucket.tokens_leaked_per_period == right->token_bucket.tokens_leaked_per_period) &&
           (DDS_Duration_equal(&left->token_bucket.period,&right->token_bucket.period));
}

DDS_ReturnCode_t
DDS_FlowControllerProperty_copy(struct DDS_FlowControllerProperty_t *out,
                                const struct DDS_FlowControllerProperty_t *in)
{
    *out = *in;

    return DDS_RETCODE_OK;
}

RTI_PRIVATE DDS_Boolean
DDS_FlowControllerProperty_is_consistent(struct DDS_FlowControllerProperty_t *fc_property)
{
    RTI_INT32 overhead = 1024; /* 1K */

    /* period */
    if (((DDS_Duration_compare(&fc_property->token_bucket.period, &DDS_DURATION_NANOSEC) < 0) ||
        (DDS_Duration_compare(&fc_property->token_bucket.period, &DDS_DURATION_YEAR) > 0))  &&
        (DDS_Duration_compare(&fc_property->token_bucket.period, &DDS_DURATION_INFINITE) != 0))
    {
        DDSC_LOG_FLOWCONTROLLER_INCONSISTENT_PROP(OSAPI_LOGKIND_ERROR,"period")
        return RTI_FALSE;
    }

    /* valid range 1024 to UNLIMITED */
    if ((fc_property->token_bucket.bytes_per_token < overhead) &&
        (fc_property->token_bucket.bytes_per_token != DDS_LENGTH_UNLIMITED))
    {
        DDSC_LOG_FLOWCONTROLLER_INCONSISTENT_PROP(OSAPI_LOGKIND_ERROR,"bytes_per_token")
        return RTI_FALSE;
    }
     /* valid range is 1 to UNLIMITED */
    if ((fc_property->token_bucket.max_tokens <= 0) &&
        (fc_property->token_bucket.max_tokens != DDS_LENGTH_UNLIMITED))
    {
        DDSC_LOG_FLOWCONTROLLER_INCONSISTENT_PROP(OSAPI_LOGKIND_ERROR,"max_tokens")
        return RTI_FALSE;
    }
     /* valid range is 1 to UNLIMITED */
    if ((fc_property->token_bucket.tokens_added_per_period <= 0) &&
        (fc_property->token_bucket.tokens_added_per_period != DDS_LENGTH_UNLIMITED))
    {
        DDSC_LOG_FLOWCONTROLLER_INCONSISTENT_PROP(OSAPI_LOGKIND_ERROR,"tokens_added_per_period")
        return RTI_FALSE;
    }
     /* valid range is 0 to UNLIMITED */
    if ((fc_property->token_bucket.tokens_leaked_per_period < 0) &&
        (fc_property->token_bucket.tokens_leaked_per_period != DDS_LENGTH_UNLIMITED))
    {
        DDSC_LOG_FLOWCONTROLLER_INCONSISTENT_PROP(OSAPI_LOGKIND_ERROR,"tokens_added_per_period")
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

const char*
DDS_FlowController_get_name(DDS_FlowController *self)
{
    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                              return NULL,
                          OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return NETIO_FlowController_get_name(self);
}

DDS_DomainParticipant*
DDS_FlowController_get_participant(DDS_FlowController *self)
{
    return (DDS_DomainParticipant*)NETIO_FlowController_get_owner(self);
}

DDS_ReturnCode_t
DDS_FlowController_get_property(DDS_FlowController *self,
                                struct DDS_FlowControllerProperty_t *prop)
{
    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (prop == NULL),
                              return DDS_RETCODE_BAD_PARAMETER,
                              OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                              OSAPI_Log_entry_add_pointer("prop",prop,RTI_TRUE);)

    *prop = self->property;

    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t
DDS_FlowController_set_property(DDS_FlowController *self,
                                const struct DDS_FlowControllerProperty_t *prop)
{
    UNUSED_ARG(self);
    UNUSED_ARG(prop);

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (prop == NULL),
                              return DDS_RETCODE_BAD_PARAMETER,
                              OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                              OSAPI_Log_entry_add_pointer("prop",prop,RTI_TRUE);)

    return DDS_RETCODE_UNSUPPORTED;
}

DDS_ReturnCode_t
DDS_FlowController_trigger_flow(DDS_FlowController *self)
{
    UNUSED_ARG(self);

    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                              return DDS_RETCODE_BAD_PARAMETER,
                              OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return DDS_RETCODE_UNSUPPORTED;
}

DDS_Boolean
DDS_FlowController_initialize(struct DDS_FlowController *fc,
                              struct NETIO_FlowControllerI *intf,
                              struct DDS_FlowControllerProperty_t *prop)
{
    RT_Component_initialize(&fc->_parent._parent,&intf->_parent,0,NULL,NULL);

    fc->property._parent._parent = prop->_parent._parent;
    fc->property.is_vendor_specific = prop->is_vendor_specific;
    fc->property.scheduling_policy = prop->scheduling_policy;
    fc->property.token_bucket = prop->token_bucket;

    fc->_parent.name = DDS_String_dup(prop->_parent.name);
    if (fc->_parent.name == NULL)
    {
        return DDS_BOOLEAN_FALSE;
    }

    fc->_parent.owner = prop->_parent.owner;
    fc->wrapper = NULL;
    fc->ref_count = 0;

    return DDS_BOOLEAN_TRUE;
}

#ifndef RTI_CERT
void
DDS_FlowController_finalize(struct DDS_FlowController *fc)
{
    if (fc->_parent.name != NULL)
    {
        DDS_String_free((char*)fc->_parent.name);
        fc->_parent.name = NULL;
    }
}
#endif

void
DDS_FlowController_set_wrapper(struct DDS_FlowController *fc,
                               void *wrapper)
{
    fc->wrapper = wrapper;
}

void*
DDS_FlowController_get_wrapper(struct DDS_FlowController *fc)
{
    return fc->wrapper;
}

RTI_PRIVATE DDS_FlowController*
DDS_DomainParticipant_create_flowcontroller_w_property(
                            DDS_DomainParticipant *self,
                            struct DDS_FlowControllerProperty_t *prop)
{
    struct DDS_FlowController *fc = NULL;

    fc = (struct DDS_FlowController*)
            NETIO_FlowControllerFactory_create_flowcontroller(
                                    self->flow_control.default_factory,
                                    &prop->_parent._parent,NULL);
    if (fc == NULL)
    {
        return NULL;
    }

    if (!REDA_Indexer_add_entry(self->flow_control.index,fc))
    {
        NETIO_FlowControllerFactory_delete_flowcontroller(
                            self->flow_control.default_factory,fc);
        return NULL;
    }

    return fc;
}

DDS_FlowController*
DDS_DomainParticipant_create_flowcontroller(
                            DDS_DomainParticipant *self,
                            const char *name,
                            const struct DDS_FlowControllerProperty_t *prop)
{
    struct DDS_FlowControllerProperty_t prop_copy =
                                    DDS_FlowControllerProperty_t_INITIALIZER;

    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (name == NULL) || (prop == NULL),
                              return NULL,
                              OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                              OSAPI_Log_entry_add_pointer("name",name,RTI_FALSE);
                              OSAPI_Log_entry_add_pointer("prop",prop,RTI_TRUE);)

    /* To avoid multiple calls to DDS_FlowController_assert(), the assertion
     * is done only in DDS_DomainParticipant_lookup_flowcontroller.
     */
    if (DDS_DomainParticipant_lookup_flowcontroller(self,name) != NULL)
    {
        return NULL;
    }

    /* It is necessary to make a copy in order to pass the name and owner to
     * the FlowController component since the public API uses a const struct.
     */
    if (prop == &DDS_FLOW_CONTROLLER_PROPERTY_DEFAULT)
    {
        DDS_DomainParticipant_get_default_flowcontroller_property(self, &prop_copy);
    }
    else
    {
        prop_copy.is_vendor_specific = prop->is_vendor_specific;
        prop_copy.scheduling_policy = prop->scheduling_policy;
        prop_copy.token_bucket = prop->token_bucket;
    }

    prop_copy._parent._parent.db = self->database;
    prop_copy._parent._parent.timer = self->timer;
    prop_copy._parent.name = name;
    prop_copy._parent.owner = self;

    if (!DDS_FlowControllerProperty_is_consistent(&prop_copy))
    {
        return NULL;
    }

    return DDS_DomainParticipant_create_flowcontroller_w_property(self,&prop_copy);
}

DDS_ReturnCode_t
DDS_DomainParticipant_delete_flowcontroller(DDS_DomainParticipant *self,
                                            DDS_FlowController *fc)
{
    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (fc == NULL),
                              return DDS_RETCODE_BAD_PARAMETER,
                              OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                              OSAPI_Log_entry_add_pointer("fc",fc,RTI_TRUE);)

    if (self != fc->_parent.owner)
    {
        return DDS_RETCODE_BAD_PARAMETER;
    }

    if (fc->ref_count > 0)
    {
        /* The flow controller is still in use by one or more DataWriters, so
         * it cannot be deleted.
         */
        DDSC_LOG_DELETE_FLOWCONTROLLER(OSAPI_LOGKIND_ERROR,fc->_parent.name)
        return DDS_RETCODE_PRECONDITION_NOT_MET;
    }

    if (REDA_Indexer_remove_entry(self->flow_control.index,fc->_parent.name) != fc)
    {
        return DDS_RETCODE_ERROR;
    }

    if (self->flow_control.wrapper_finalizer != NULL)
    {
        self->flow_control.wrapper_finalizer(fc->wrapper);
    }

    NETIO_FlowControllerFactory_delete_flowcontroller(
                                        self->flow_control.default_factory,fc);


    return DDS_RETCODE_OK;
}

DDS_FlowController*
DDS_DomainParticipant_lookup_flowcontroller(DDS_DomainParticipant *self,
                                            const char *name)
{
    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (name == NULL),
                              return NULL,
                              OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                              OSAPI_Log_entry_add_pointer("name",name,RTI_TRUE);)

    /* The flow-controller is initialized only when used. Thus, assert the
     * flow-controller to make sure it is properly initialized.
     */
    if (!DDS_FlowControl_assert(&self->flow_control,self))
    {
        return NULL;
    }

    return (struct DDS_FlowController*)REDA_Indexer_find_entry(
                                            self->flow_control.index,name);
}

DDS_FlowController*
DDS_DomainParticipant_acquire_flowcontroller(DDS_DomainParticipant *self,
                                            const char *name)
{
    struct DDS_FlowController *fc = NULL;
    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (name == NULL),
                              return NULL,
                              OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                              OSAPI_Log_entry_add_pointer("name",name,RTI_TRUE);)

    fc = DDS_DomainParticipant_lookup_flowcontroller(self,name);
    if (fc != NULL)
    {
        /* Check for overflow before incrementing */
        if (fc->ref_count == UINT_MAX)
        {
            fc = NULL;
        }
        else
        {
            fc->ref_count++;
        }
    }
    return fc;
}

DDS_Boolean
DDS_DomainParticipant_release_flowcontroller(DDS_DomainParticipant *self,
                                            DDS_FlowController *fc)
{
    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (fc == NULL),
                              return DDS_BOOLEAN_FALSE,
                              OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                              OSAPI_Log_entry_add_pointer("fc",fc,RTI_TRUE);)

    /* Check for underflow before decrementing */
    if (fc->ref_count == 0)
    {
        /* This should not happen since the flow controller should have
         * been acquired before being released
         */
        return DDS_BOOLEAN_FALSE;
    }

    fc->ref_count--;
    return DDS_BOOLEAN_TRUE;
}

DDS_ReturnCode_t
DDS_DomainParticipant_get_default_flowcontroller_property(
                                    DDS_DomainParticipant *self,
                                    struct DDS_FlowControllerProperty_t *prop)
{
    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (prop == NULL),
                              return DDS_RETCODE_BAD_PARAMETER,
                              OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                              OSAPI_Log_entry_add_pointer("prop",prop,RTI_TRUE);)

    *prop = self->flow_control.default_property;

    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t
DDS_DomainParticipant_set_default_flowcontroller_property(
                            DDS_DomainParticipant *self,
                            const struct DDS_FlowControllerProperty_t *prop)
{
    OSAPI_PRECONDITION_ALWAYS((self == NULL) || (prop == NULL),
                              return DDS_RETCODE_BAD_PARAMETER,
                              OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                              OSAPI_Log_entry_add_pointer("prop",prop,RTI_TRUE);)

    self->flow_control.default_property = *prop;

    return DDS_RETCODE_OK;
}

void
DDS_DomainParticipant_set_flowcontroller_finalizer(DDS_DomainParticipant *self,
                                   DDS_FlowController_Finalizer_T finalizer)
{
    self->flow_control.wrapper_finalizer = finalizer;
}

RTI_PRIVATE RTI_INT32
DDS_FlowController_compare(const void *const record,
                           RTI_BOOL key_is_record,
                           const void *const key)
{
    struct DDS_FlowController *lval = (struct DDS_FlowController*)record;
    const char *rname;

    if (key_is_record)
    {
        rname = ((struct DDS_FlowController*)key)->_parent.name;
    }
    else
    {
        rname = (char*)key;
    }

    return OSAPI_String_cmp(lval->_parent.name,rname);
}

void
DDS_FlowControl_initialize(struct DDS_FlowControl *self,
                           struct DDS_DomainParticipantImpl *dp)
{
    struct DDS_FlowControllerProperty_t df =
                                DDS_FlowControllerProperty_t_INITIALIZER;
    UNUSED_ARG(dp);

    OSAPI_Memory_zero(self,sizeof(struct DDS_FlowControl));

    /* NOTE: The default properties does not require the flow-controller
     * running
     */
    self->default_property = df;

    /* self->wrapper_finalizer is initialized to NULL
     */
}

DDS_Boolean
DDS_FlowControl_assert(struct DDS_FlowControl *self,
                       struct DDS_DomainParticipantImpl *dp)
{
    struct REDA_IndexerProperty ip = REDA_IndexerProperty_INITIALIZER;
    struct DDS_FlowControllerProperty_t df =
                                    DDS_FlowControllerProperty_t_INITIALIZER;
    struct DDS_FlowController *fc1 = NULL,*fc2 = NULL,*fc3 = NULL;
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;

    /* This value is non-NULL after a successful initialization
     */
    if (self->index != NULL)
    {
        return DDS_BOOLEAN_TRUE;
    }

    self->sched_lock = OSAPI_Mutex_new();
    if (self->sched_lock == NULL)
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!OSAPI_TaskScheduler_initialize(&self->task_scheduler,self->sched_lock))
    {
        goto done;
    }

    /*
     * Connext Pro has:
     * DDS_DomainParticipantResourceLimitsQosPolicy::flow_controller_allocation
     *
     * However, it did not make it into Micro for 3.0.0.0 and an upper limit
     * is hard-coded.
     */
    ip.max_entries = DDS_DOMAINPARTICIPANTRESOURCELIMITSQSPOLICY_FLOW_CONTROLLER_ALLOCATION;

    self->index = REDA_Indexer_new(DDS_FlowController_compare,&ip);
    if (self->index == NULL)
    {
        goto done;
    }

    /* The fc_factory is used by the public flow-controller APIs and is the
     * flow-controller that matches Connext Pro leaky bucket algorithm.
     */
    self->default_factory = RT_Registry_lookup_by_class(dp->config.registry,
                              RT_MKINTERFACEID(
                                  RT_COMPONENT_CLASS_NETIO_FLOWCONTROLLER,
                                  RT_COMPONENT_INSTANCE_DDS_FLOWCONTROLLER));
    if (self->default_factory == NULL)
    {
        goto done;
    }

    /* Common settings */
    df._parent._parent.db = dp->database;
    df._parent._parent.timer = dp->timer;
    df._parent.owner = dp;
    df.is_vendor_specific = DDS_BOOLEAN_FALSE;

    /* per flow-controller settings */
    df._parent.name = DDS_DEFAULT_FLOW_CONTROLLER_NAME;
    df.scheduling_policy = DDS_EDF_FLOW_CONTROLLER_SCHED_POLICY;
    df.token_bucket.max_tokens = DDS_LENGTH_UNLIMITED;
    df.token_bucket.tokens_added_per_period = DDS_LENGTH_UNLIMITED;
    df.token_bucket.tokens_leaked_per_period = 0;
    df.token_bucket.period.sec = 60;
    df.token_bucket.period.nanosec = 0;
    df.token_bucket.bytes_per_token =  DDS_LENGTH_UNLIMITED;

    fc1 = DDS_DomainParticipant_create_flowcontroller_w_property(dp,&df);
    if (fc1 == NULL)
    {
        DDSC_LOG_FLOW_CONTROLLER_CREATE(OSAPI_LOGKIND_ERROR,
                                        DDS_DEFAULT_FLOW_CONTROLLER_NAME)
        goto done;
    }

    df._parent.name = DDS_FIXED_RATE_FLOW_CONTROLLER_NAME;
    df.scheduling_policy = DDS_EDF_FLOW_CONTROLLER_SCHED_POLICY;
    df.token_bucket.max_tokens = DDS_LENGTH_UNLIMITED;
    df.token_bucket.tokens_added_per_period = DDS_LENGTH_UNLIMITED;
    df.token_bucket.tokens_leaked_per_period = DDS_LENGTH_UNLIMITED;
    df.token_bucket.period.sec = 1;
    df.token_bucket.period.nanosec = 0;
    df.token_bucket.bytes_per_token = DDS_LENGTH_UNLIMITED;

    fc2 = DDS_DomainParticipant_create_flowcontroller_w_property(dp,&df);
    if (fc2 == NULL)
    {
        DDSC_LOG_FLOW_CONTROLLER_CREATE(OSAPI_LOGKIND_ERROR,
                                        DDS_FIXED_RATE_FLOW_CONTROLLER_NAME)
        goto done;
    }

    df._parent.name = DDS_ON_DEMAND_FLOW_CONTROLLER_NAME;
    df.scheduling_policy = DDS_EDF_FLOW_CONTROLLER_SCHED_POLICY;
    df.token_bucket.max_tokens = DDS_LENGTH_UNLIMITED;
    df.token_bucket.tokens_added_per_period = DDS_LENGTH_UNLIMITED;
    df.token_bucket.tokens_leaked_per_period = DDS_LENGTH_UNLIMITED;
    df.token_bucket.period = DDS_DURATION_INFINITE;
    df.token_bucket.bytes_per_token = DDS_LENGTH_UNLIMITED;

    fc3 = DDS_DomainParticipant_create_flowcontroller_w_property(dp,&df);
    if (fc3 == NULL)
    {
        DDSC_LOG_FLOW_CONTROLLER_CREATE(OSAPI_LOGKIND_ERROR,
                                        DDS_ON_DEMAND_FLOW_CONTROLLER_NAME)
        goto done;
    }

    OSAPI_Trace_write("Successfully initialized flow-control state",
                      NULL,NULL,NULL,NULL,NULL,
                      NULL,NULL,NULL,NULL,NULL);



    retval = DDS_BOOLEAN_TRUE;

done:

#ifndef RTI_CERT
    if (!retval)
    {
        if (fc1 != NULL)
        {
            DDS_DomainParticipant_delete_flowcontroller(dp,fc1);
        }

        if (fc2 != NULL)
        {
            DDS_DomainParticipant_delete_flowcontroller(dp,fc2);
        }

#ifndef RTI_CERT
        if (self->index != NULL)
        {
            REDA_Indexer_delete(self->index);
        }

        if (self->sched_lock != NULL)
        {
            OSAPI_Mutex_delete(self->sched_lock);
        }
#endif

        DDS_FlowControl_initialize(self,dp);
    }
#endif

    return retval;
}

#ifndef RTI_CERT
DDS_Boolean
DDS_FlowControl_finalize(struct DDS_FlowControl *self,
                         DDS_Boolean is_delete_contained)
{
    REDA_IndexIterator_T *it;
    struct DDS_FlowController *fc;
    UNUSED_ARG(is_delete_contained);

    if (!OSAPI_TaskScheduler_finalize(&self->task_scheduler))
    {
        return DDS_BOOLEAN_TRUE;
    }

    if (self->index == NULL)
    {
        return DDS_BOOLEAN_TRUE;
    }

    it = REDA_Indexer_iterator_begin(self->index);

    fc = (struct DDS_FlowController*)REDA_Indexer_iterator_next(it);
    while (fc != NULL)
    {
        if (!fc->property.is_vendor_specific)
        {
            if (self->wrapper_finalizer != NULL)
            {
                self->wrapper_finalizer(fc->wrapper);
            }

            NETIO_FlowControllerFactory_delete_flowcontroller(self->
                                                          default_factory,fc);
        }
        fc = (struct DDS_FlowController *)REDA_Indexer_iterator_next(it);
    }

    if (self->sched_lock != NULL)
    {
        OSAPI_Mutex_delete(self->sched_lock);
        self->sched_lock = NULL;
    }

    if (!REDA_Indexer_delete(self->index))
    {
        return DDS_BOOLEAN_FALSE;
    }

    OSAPI_Trace_write("Successfully finalized flow-control state",
                      NULL,NULL,NULL,NULL,NULL,
                      NULL,NULL,NULL,NULL,NULL);

    return DDS_BOOLEAN_TRUE;
}
#endif

#endif /* DDS_FLOW_CONTROLLER_ENABLED */
