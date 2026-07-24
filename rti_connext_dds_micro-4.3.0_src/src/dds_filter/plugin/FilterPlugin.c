/*
 * FILE: FilterPlugin.c - Filter plugin implementation
 *
 * Copyright (c) 2025-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#include "FilterPlugin.h"
#include "FilterPluginReader.h"
#include "FilterPluginWriter.h"

#include "dds_c/dds_c_subscription.h"

/*** SOURCE_BEGIN ***/

RTI_PRIVATE const char *const DDS_FILTER_CLASS_NAME_TABLE = "fcn_table";
RTI_PRIVATE const char *const DDS_FILTER_EXPRESSION_TABLE = "fe_table";
RTI_PRIVATE const char *const DDS_FILTER_PARAMETER_TABLE = "fp_table";

RTI_PRIVATE const char *const DDS_FILTER_CLASS_TABLE = "fc_table";
RTI_PRIVATE const char *const DDS_COMPILED_FILTER_TABLE = "cf_table";

/* Forward-definition of component interface*/
RTI_PRIVATE struct DDS_FilterPluginI DDS_FilterPluginImpl_fv_Intf;

RTI_PRIVATE RTI_INT32
DDS_ContentFilterClass_compare(RTI_INT32 flags, const DB_Record_T op1, void *op2)
{
    const struct DDS_ContentFilterClass *record_left = (const struct DDS_ContentFilterClass *)op1;
    const char *name_right;

    if (DB_SELECT_OP2_IS_KEY(flags))
    {
        name_right = (const char *)op2;
    }
    else
    {
        name_right = ((const struct DDS_ContentFilterClass *)op2)->name;
    }

    return OSAPI_String_cmp(record_left->name, name_right);
}

RTI_PRIVATE RTI_INT32
DDS_ContentFilterCompiledFilter_compare(RTI_INT32 flags, const DB_Record_T op1, void *op2)
{
    const struct DDS_ContentFilterCompiledFilter *record_left = (const struct DDS_ContentFilterCompiledFilter *)op1;
    const struct DDS_FilterSignature *filter_right;

    if (DB_SELECT_OP2_IS_KEY(flags))
    {
        filter_right = (const struct DDS_FilterSignature *)op2;
    }
    else
    {
        filter_right = &((const struct DDS_ContentFilterCompiledFilter *)op2)->filter_signature;
    }

    return DDS_FilterSignature_compare(&record_left->filter_signature, filter_right);
}

DDS_ReturnCode_t
DDS_FilterPluginImpl_register_filter_class(
        struct DDS_FilterPlugin *self,
        const char *name,
        const struct DDS_ContentFilterI *filter_intf,
        const void *filter_property)
{
    struct DDS_ContentFilterClass *record = NULL;
    struct DDS_FilterResourceLimits resource_limits;
    void *instance = NULL;
    DB_ReturnCode_T dbrc;
    RTI_BOOL retval;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    OSAPI_PRECONDITION(
            (self == NULL) || (name == NULL) || (filter_intf == NULL),
            return DDS_RETCODE_BAD_PARAMETER,
            OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
            OSAPI_Log_entry_add_string("name", name, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("filter_intf", filter_intf, RTI_TRUE);)

    dbrc = DB_Table_select_match(self->filter_class_table,
                                DB_TABLE_DEFAULT_INDEX,
                                (DB_Record_T)&record,
                                (DB_Key_T)name);
    if (dbrc == DB_RETCODE_OK)
    {
        DDS_FILTER_LOG_FILTER_CLASS_REGISTER(OSAPI_LOGKIND_ERROR, name)
        return DDS_RETCODE_PRECONDITION_NOT_MET;
    }
    else if (dbrc != DB_RETCODE_NO_DATA)
    {
        DDS_FILTER_LOG_RECORD_SELECT(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_FILTER_CLASS_RECORD,
                                     dbrc)
        goto done;
    }

    if ((filter_intf->create_instance == NULL)
#ifndef RTI_CERT
        || (filter_intf->delete_instance == NULL)
#endif /* !RTI_CERT */
        || (filter_intf->compile == NULL)
        || (filter_intf->evaluate == NULL)
        || (filter_intf->finalize == NULL))
    {
        DDS_FILTER_LOG_FILTER_CLASS_REGISTER(OSAPI_LOGKIND_ERROR, name)
        retcode = DDS_RETCODE_PRECONDITION_NOT_MET;
        goto done;
    }

    /* Add one filter expression for when a filter is updated */
    resource_limits = self->property.resource_limits;
    resource_limits.filter_expression_max_count += 1;

    instance = filter_intf->create_instance(&resource_limits, filter_property);
    if (instance == NULL)
    {
        DDS_FILTER_LOG_FILTER_CREATE_INSTANCE(OSAPI_LOGKIND_ERROR, name)
        goto done;
    }

    dbrc = DB_Table_create_record(self->filter_class_table,
                                  (DB_Record_T)&record);
    if (dbrc != DB_RETCODE_OK)
    {
        DDS_FILTER_LOG_RECORD_CREATE(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_FILTER_CLASS_RECORD,
                                     dbrc)
        goto done;
    }

    record->name = DDS_StringManager_assert_string(self->filter_class_string_manager, name);
    if (record->name == NULL)
    {
        DDS_FILTER_LOG_STRING_ASSERT(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_FILTER_CLASS_NAME_STRING)
        goto done;
    }
    record->intf = filter_intf;
    record->instance = instance;
    record->ref_count = 0;

    dbrc = DB_Table_insert_record(self->filter_class_table, (DB_Record_T)record);
    if (dbrc != DB_RETCODE_OK)
    {
        DDS_FILTER_LOG_RECORD_INSERT(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_FILTER_CLASS_RECORD,
                                     dbrc)
        goto done;
    }

    retcode = DDS_RETCODE_OK;
done:
    if (retcode != DDS_RETCODE_OK)
    {
        if (record != NULL)
        {
            if (record->name != NULL)
            {
                retval = DDS_StringManager_delete_string(
                                            self->filter_class_string_manager,
                                            record->name);
#if OSAPI_ENABLE_LOG
                if (!retval)
                {
                    DDS_FILTER_LOG_STRING_DELETE(OSAPI_LOGKIND_ERROR,
                                                 DDS_FILTER_LOG_FILTER_CLASS_NAME_STRING)
                }
#else
                IGNORE_RETVAL(retval);
#endif
            }
            dbrc = DB_Table_delete_record(self->filter_class_table, (DB_Record_T)record);
#if OSAPI_ENABLE_LOG
            if (dbrc != DB_RETCODE_OK)
            {
                DDS_FILTER_LOG_RECORD_DELETE(OSAPI_LOGKIND_ERROR,
                                             DDS_FILTER_LOG_FILTER_CLASS_RECORD,
                                             dbrc)
            }
#else
            IGNORE_RETVAL(dbrc);
#endif
        }
#ifndef RTI_CERT
        if (instance != NULL)
        {
            filter_intf->delete_instance(instance);
        }
#endif /* !RTI_CERT */
    }

    return retcode;
}


RTI_PRIVATE DDS_ReturnCode_t
DDS_FilterPluginImpl_unregister_filter_class(
        struct DDS_FilterPlugin *self,
        const char *name)
{
    struct DDS_ContentFilterClass *record = NULL;
    DB_ReturnCode_T dbrc;
    RTI_BOOL retval = RTI_TRUE;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    dbrc = DB_Table_select_match(self->filter_class_table,
                                DB_TABLE_DEFAULT_INDEX,
                                (DB_Record_T)&record,
                                (DB_Key_T)name);
    if (dbrc == DB_RETCODE_NO_DATA)
    {
        DDS_FILTER_LOG_FILTER_CLASS_NOT_FOUND(OSAPI_LOGKIND_ERROR, name)
        goto done;
    }
    else if (dbrc != DB_RETCODE_OK)
    {
        DDS_FILTER_LOG_RECORD_SELECT(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_FILTER_CLASS_RECORD,
                                     dbrc)
        goto done;
    }

    if (record->ref_count > 0)
    {
        /* Filter is still in use */
        DDS_FILTER_LOG_FILTER_CLASS_UNREGISTER(OSAPI_LOGKIND_ERROR, name)
        goto done;
    }

    record = NULL;
    dbrc = DB_Table_remove_record(self->filter_class_table,
                                (DB_Record_T)&record,
                                (DB_Key_T)name);
    if (dbrc != DB_RETCODE_OK)
    {
        DDS_FILTER_LOG_RECORD_REMOVE(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_FILTER_CLASS_RECORD,
                                     dbrc)
        goto done;
    }

#ifndef RTI_CERT
    record->intf->delete_instance(record->instance);
#endif /* !RTI_CERT */

    retval = DDS_StringManager_delete_string(
                                    self->filter_class_string_manager,
                                    record->name);
#if OSAPI_ENABLE_LOG
    if (!retval)
    {
        DDS_FILTER_LOG_STRING_DELETE(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_FILTER_CLASS_NAME_STRING)
    }
#endif

    record->name = NULL;
    record->intf = NULL;
    record->instance = NULL;

    dbrc = DB_Table_delete_record(self->filter_class_table, (DB_Record_T)record);
    if (dbrc != DB_RETCODE_OK)
    {
        DDS_FILTER_LOG_RECORD_DELETE(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_FILTER_CLASS_RECORD,
                                     dbrc)
        goto done;
    }

    if (retval)
    {
        retcode = DDS_RETCODE_OK;
    }
done:
    return retcode;
}

RTI_BOOL
DDS_FilterPluginImpl_initialize(
        struct DDS_FilterPlugin *self,
        struct DDS_FilterPluginFactory *factory,
        const struct DDS_FilterPluginProperty *const property)
{
    RTI_BOOL result = RTI_FALSE;
    struct DDS_StringManagerProperty string_manager_property =
            DDS_StringManagerProperty_INITIALIZER;
    struct DB_TableProperty tbl_prop = DB_TableProperty_INITIALIZER;
    DDS_Long filter_expression_max_count;
    DDS_LongLong memory_allocation_ll;
    DB_ReturnCode_T dbrc;

    OSAPI_PRECONDITION(
            (self == NULL) || (factory == NULL) || (property == NULL),
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("factory", factory, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("property", property, RTI_TRUE);)

    if (property->topic_string_manager == NULL)
    {
        return RTI_FALSE;
    }

    /* Reserve space for one additional filter expression to allow space for
     * updated filter expressions to be compiled before the old one is removed.
     */
    filter_expression_max_count = property->resource_limits.filter_expression_max_count;
    if ((filter_expression_max_count < 0) || (filter_expression_max_count == INT_MAX)
            || (property->resource_limits.filter_class_max_count < 0)
            || (property->resource_limits.filter_class_max_length == INT_MAX)
            || (property->resource_limits.filter_expression_max_length == INT_MAX)
            || (property->resource_limits.filter_parameter_max_length == INT_MAX))
    {
        return RTI_FALSE;
    }
    filter_expression_max_count += 1;

    RT_Component_initialize(
            &self->_parent,
            &DDS_FilterPluginImpl_fv_Intf._parent._parent,
            factory->instance_counter,
            &property->_parent,
            NULL);

    self->factory = factory;
    self->property = *property;
#ifndef RTI_CERT
    self->writer_count = 0;
#endif /* !RTI_CERT */
    self->compiled_filter_count = 0;
    self->filter_class_string_manager = NULL;
    self->filter_expression_string_manager = NULL;
    self->filter_parameter_string_manager = NULL;
    self->filter_class_table = NULL;
    self->compiled_filter_table = NULL;

    string_manager_property.database = property->_parent.db;

    /* For all of the string managers:
     * - Add one to the max_string_size to account for the null terminator
     * - Add one to the number of strings to account for the empty string
     */

    /* filter_class_string_manager */
    string_manager_property.max_string_size = property->resource_limits.filter_class_max_length + 1;
    memory_allocation_ll =
            ((DDS_LongLong)property->resource_limits.filter_class_max_count + 1)
                * string_manager_property.max_string_size;
    if (memory_allocation_ll > INT_MAX)
    {
        DDS_FILTER_LOG_STRING_MANAGER_CREATE(OSAPI_LOGKIND_ERROR,
                                             DDS_FILTER_LOG_FILTER_CLASS_NAME_STRING,
                                             string_manager_property.max_string_size,
                                             INT_MAX)
        goto done;
    }
    string_manager_property.memory_allocation = (RTI_INT32)memory_allocation_ll;
    if (!DDS_StringManager_create(
            &self->filter_class_string_manager,
            &string_manager_property,
            DDS_FILTER_CLASS_NAME_TABLE))
    {
        DDS_FILTER_LOG_STRING_MANAGER_CREATE(OSAPI_LOGKIND_ERROR,
                                             DDS_FILTER_LOG_FILTER_CLASS_NAME_STRING,
                                             string_manager_property.max_string_size,
                                             string_manager_property.memory_allocation)
        goto done;
    }

    /* filter_expression_string_manager */
    string_manager_property.max_string_size = property->resource_limits.filter_expression_max_length + 1;
    memory_allocation_ll =
            ((DDS_LongLong)filter_expression_max_count + 1) * string_manager_property.max_string_size;
    if (memory_allocation_ll > INT_MAX)
    {
        DDS_FILTER_LOG_STRING_MANAGER_CREATE(OSAPI_LOGKIND_ERROR,
                                             DDS_FILTER_LOG_FILTER_EXPRESSION_STRING,
                                             string_manager_property.max_string_size,
                                             INT_MAX)
        goto done;
    }
    string_manager_property.memory_allocation = (RTI_INT32)memory_allocation_ll;
    if (!DDS_StringManager_create(
            &self->filter_expression_string_manager,
            &string_manager_property,
            DDS_FILTER_EXPRESSION_TABLE))
    {
        DDS_FILTER_LOG_STRING_MANAGER_CREATE(OSAPI_LOGKIND_ERROR,
                                             DDS_FILTER_LOG_FILTER_EXPRESSION_STRING,
                                             string_manager_property.max_string_size,
                                             string_manager_property.memory_allocation)
        goto done;
    }

    /* filter_parameter_string_manager */
    if (property->resource_limits.filter_parameter_max_count_per_expression > 0)
    {
        string_manager_property.max_string_size = property->resource_limits.filter_parameter_max_length + 1;
        memory_allocation_ll =
                ((DDS_LongLong)filter_expression_max_count
                    * property->resource_limits.filter_parameter_max_count_per_expression + 1)
                    * string_manager_property.max_string_size;
        if (memory_allocation_ll > INT_MAX)
        {
            DDS_FILTER_LOG_STRING_MANAGER_CREATE(OSAPI_LOGKIND_ERROR,
                                                 DDS_FILTER_LOG_FILTER_EXPRESSION_PARAMETER_STRING,
                                                 string_manager_property.max_string_size,
                                                 INT_MAX)
            goto done;
        }
        string_manager_property.memory_allocation = (RTI_INT32)memory_allocation_ll;
        if (!DDS_StringManager_create(
                &self->filter_parameter_string_manager,
                &string_manager_property,
                DDS_FILTER_PARAMETER_TABLE))
        {
            DDS_FILTER_LOG_STRING_MANAGER_CREATE(OSAPI_LOGKIND_ERROR,
                                                DDS_FILTER_LOG_FILTER_EXPRESSION_PARAMETER_STRING,
                                                string_manager_property.max_string_size,
                                                string_manager_property.memory_allocation)
            goto done;
        }
    }
    else
    {
        self->filter_parameter_string_manager = NULL;
    }

    /* filter_class_table */
    tbl_prop.max_records = (RTI_SIZE_T)property->resource_limits.filter_class_max_count;
    dbrc = DB_Database_create_table(
                    &self->filter_class_table,
                    self->property._parent.db,
                    DDS_FILTER_CLASS_TABLE,
                    sizeof(struct DDS_ContentFilterClass),
                    DDS_ContentFilterClass_compare,
                    &tbl_prop);
    if (dbrc != DB_RETCODE_OK)
    {
        DDS_FILTER_LOG_TABLE_CREATE(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_FILTER_CLASS_RECORD,
                                     dbrc)
        goto done;
    }

    /* compiled_filter_table */
    tbl_prop.max_records = (RTI_SIZE_T)filter_expression_max_count;
    dbrc = DB_Database_create_table(
                    &self->compiled_filter_table,
                    self->property._parent.db,
                    DDS_COMPILED_FILTER_TABLE,
                    sizeof(struct DDS_ContentFilterCompiledFilter),
                    DDS_ContentFilterCompiledFilter_compare,
                    &tbl_prop);
    if (dbrc != DB_RETCODE_OK)
    {
        DDS_FILTER_LOG_TABLE_CREATE(OSAPI_LOGKIND_ERROR,
                                    DDS_FILTER_LOG_COMPILED_FILTER_RECORD,
                                    dbrc)
        goto done;
    }

    result = RTI_TRUE;
done:
#ifndef RTI_CERT
    if (!result)
    {
        DDS_FilterPluginImpl_finalize(self);
    }
#endif
    return result;
}

#ifndef RTI_CERT
RTI_BOOL
DDS_FilterPluginImpl_finalize(struct DDS_FilterPlugin *self)
{
    DB_Cursor_T cursor;
    struct DDS_ContentFilterClass *filter_class = NULL;
    DB_ReturnCode_T dbrc;
    RTI_BOOL result = RTI_TRUE;

    OSAPI_PRECONDITION(
            (self == NULL),
            return RTI_FALSE,
            OSAPI_Log_entry_add_pointer("self", self, RTI_TRUE);)

    if ((self->writer_count != 0) || (self->compiled_filter_count != 0))
    {
        /* Error: there are still writers using the filter plugin */
        result = RTI_FALSE;
        goto done;
    }

    /* Delete tables */
    if (self->compiled_filter_table != NULL)
    {
        dbrc = DB_Database_delete_table(self->property._parent.db, self->compiled_filter_table);
        if (dbrc != DB_RETCODE_OK)
        {
            DDS_FILTER_LOG_TABLE_DELETE(OSAPI_LOGKIND_ERROR,
                                        DDS_FILTER_LOG_COMPILED_FILTER_RECORD,
                                        dbrc)
            result = RTI_FALSE;
        }
        else
        {
            self->compiled_filter_table = NULL;
        }
    }
    if (self->filter_class_table != NULL)
    {
        /* Automatically unregister filter classes which are not in use */
        dbrc = DB_Table_select_all(self->filter_class_table,
                                    DB_TABLE_DEFAULT_INDEX,
                                    &cursor);
        if (dbrc != DB_RETCODE_OK)
        {
            DDS_FILTER_LOG_RECORD_SELECT(OSAPI_LOGKIND_ERROR,
                                         DDS_FILTER_LOG_FILTER_CLASS_RECORD,
                                         dbrc)
            result = RTI_FALSE;
        }
        else
        {
            dbrc = DB_Cursor_get_next(cursor, (DB_Record_T *)&filter_class);
            while (dbrc == DB_RETCODE_OK)
            {
                if (DDS_FilterPluginImpl_unregister_filter_class(self, filter_class->name) != DDS_RETCODE_OK)
                {
                    result = RTI_FALSE;
                }

                dbrc = DB_Cursor_get_next(cursor, (DB_Record_T *)&filter_class);
            }
            DB_Cursor_finish(self->filter_class_table, cursor);
            if (dbrc != DB_RETCODE_NO_DATA)
            {
                DDS_FILTER_LOG_RECORD_SELECT(OSAPI_LOGKIND_ERROR,
                                            DDS_FILTER_LOG_FILTER_CLASS_RECORD,
                                            dbrc)
                result = RTI_FALSE;
            }
        }

        dbrc = DB_Database_delete_table(self->property._parent.db, self->filter_class_table);
        if (dbrc != DB_RETCODE_OK)
        {
            DDS_FILTER_LOG_TABLE_DELETE(OSAPI_LOGKIND_ERROR,
                                        DDS_FILTER_LOG_FILTER_CLASS_RECORD,
                                        dbrc)
            result = RTI_FALSE;
        }
        else
        {
            self->filter_class_table = NULL;
        }
    }

    /* Delete string managers */
    if (self->filter_class_string_manager != NULL)
    {
        if (!DDS_StringManager_delete(&self->filter_class_string_manager))
        {
            result = RTI_FALSE;
        }
        self->filter_class_string_manager = NULL;
    }
    if (self->filter_expression_string_manager != NULL)
    {
        if (!DDS_StringManager_delete(&self->filter_expression_string_manager))
        {
            result = RTI_FALSE;
        }
        self->filter_expression_string_manager = NULL;
    }
    if (self->filter_parameter_string_manager != NULL)
    {
        if (!DDS_StringManager_delete(&self->filter_parameter_string_manager))
        {
            result = RTI_FALSE;
        }
        self->filter_parameter_string_manager = NULL;
    }

    RT_Component_finalize(&self->_parent);

done:
    return result;
}
#endif

RTI_PRIVATE void
DDS_FilterPluginImpl_filter_qos_policy_finalize_shallow_copy(
        struct DDS_FilterPlugin *self,
        struct DDS_ContentFilterQosPolicy *policy);

RTI_PRIVATE DDS_ReturnCode_t
DDS_FilterPluginImpl_filter_qos_policy_shallow_copy(
        struct DDS_FilterPlugin *self,
        struct DDS_ContentFilterQosPolicy *out,
        const struct DDS_ContentFilterQosPolicy *in)
{
    DDS_ReturnCode_t result = DDS_RETCODE_ERROR;
    RTI_INT32 parameter_count;
    RTI_INT32 i;

    OSAPI_PRECONDITION(
            (self == NULL) || (out == NULL) || (in == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("out", out, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("in", in, RTI_TRUE);)

    parameter_count = DDS_StringSeq_get_length(&in->expression_parameters);
    if (parameter_count > self->property.resource_limits.filter_parameter_max_count_per_expression)
    {
        /* Error: too many parameters */
        goto done;
    }

    /* filter_name */
    if (out->filter_name != NULL)
    {
        /* Error destination contains data. Do not copy to avoid memory leak. */
        goto done;
    }
    out->filter_name = DDS_StringManager_assert_string(
            self->property.topic_string_manager,
            in->filter_name != NULL ? in->filter_name : "");
    if (out->filter_name == NULL)
    {
        DDS_FILTER_LOG_STRING_ASSERT(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_CONTENT_FILTER_NAME_STRING)
        goto done;
    }

    /* filter_class_name */
    if (out->filter_class_name != NULL)
    {
        goto done;
    }
    out->filter_class_name = DDS_StringManager_assert_string(
            self->filter_class_string_manager,
            in->filter_class_name != NULL ? in->filter_class_name : "");
    if (out->filter_class_name == NULL)
    {
        DDS_FILTER_LOG_STRING_ASSERT(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_FILTER_CLASS_NAME_STRING)
        goto done;
    }

    /* filter_expression */
    if (out->filter_expression != NULL)
    {
        goto done;
    }
    out->filter_expression = DDS_StringManager_assert_string(
            self->filter_expression_string_manager,
            in->filter_expression != NULL ? in->filter_expression : "");
    if (out->filter_expression == NULL)
    {
        DDS_FILTER_LOG_STRING_ASSERT(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_FILTER_EXPRESSION_STRING)
        goto done;
    }

    /* expression_parameters */
    if (DDS_StringSeq_get_length(&out->expression_parameters) != 0)
    {
        goto done;
    }
    if (parameter_count >
        DDS_StringSeq_get_maximum(&out->expression_parameters))
    {
        if (!DDS_StringSeq_set_maximum(
                    &out->expression_parameters,
                    self->property.resource_limits
                        .filter_parameter_max_count_per_expression))
        {
            goto done;
        }
    }
    if (!DDS_StringSeq_set_length(
                &out->expression_parameters,
                parameter_count))
    {
        goto done;
    }
    for (i = 0; i < parameter_count; ++i)
    {
        DDS_String *parameter = DDS_StringSeq_get_reference(&out->expression_parameters, i);
        if (parameter == NULL)
        {
            goto done;
        }
        *parameter = DDS_StringManager_assert_string(
                self->filter_parameter_string_manager,
                *DDS_StringSeq_get_reference(&in->expression_parameters, i));
        if (*parameter == NULL)
        {
             DDS_FILTER_LOG_STRING_ASSERT(OSAPI_LOGKIND_ERROR,
                                          DDS_FILTER_LOG_FILTER_EXPRESSION_PARAMETER_STRING)
            goto done;
        }
    }

    result = DDS_RETCODE_OK;
done:
    if (result != DDS_RETCODE_OK)
    {
        DDS_FilterPluginImpl_filter_qos_policy_finalize_shallow_copy(
                self, out);
    }
    return result;
}

RTI_PRIVATE void
DDS_FilterPluginImpl_filter_qos_policy_finalize_shallow_copy(
        struct DDS_FilterPlugin *self,
        struct DDS_ContentFilterQosPolicy *policy)
{
    RTI_INT32 i;
    RTI_BOOL retval;

    OSAPI_PRECONDITION((self == NULL) || (policy == NULL),
                        return,
                        OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("policy", policy, RTI_TRUE);)

    /* filter_name */
    if (policy->filter_name != NULL)
    {
        retval = DDS_StringManager_delete_string(self->property.topic_string_manager,
                                                 policy->filter_name);
#if OSAPI_ENABLE_LOG
        if (!retval)
        {
            DDS_FILTER_LOG_STRING_DELETE(OSAPI_LOGKIND_ERROR,
                                         DDS_FILTER_LOG_CONTENT_FILTER_NAME_STRING)
        }
#else
        IGNORE_RETVAL(retval);
#endif
        policy->filter_name = NULL;
    }

    /* filter_class_name */
    if (policy->filter_class_name != NULL)
    {
        retval = DDS_StringManager_delete_string(self->filter_class_string_manager,
                                                 policy->filter_class_name);
#if OSAPI_ENABLE_LOG
        if (!retval)
        {
            DDS_FILTER_LOG_STRING_DELETE(OSAPI_LOGKIND_ERROR,
                                         DDS_FILTER_LOG_FILTER_CLASS_NAME_STRING)
        }
#else
        IGNORE_RETVAL(retval);
#endif
        policy->filter_class_name = NULL;
    }

    /* filter_expression */
    if (policy->filter_expression != NULL)
    {
        retval = DDS_StringManager_delete_string(self->filter_expression_string_manager,
                                                 policy->filter_expression);
#if OSAPI_ENABLE_LOG
        if (!retval)
        {
            DDS_FILTER_LOG_STRING_DELETE(OSAPI_LOGKIND_ERROR,
                                         DDS_FILTER_LOG_FILTER_EXPRESSION_STRING)
        }
#else
        IGNORE_RETVAL(retval);
#endif
        policy->filter_expression = NULL;
    }

    /* expression_parameters */
    for (i = 0; i < DDS_StringSeq_get_length(&policy->expression_parameters); ++i)
    {
        DDS_String *parameter = DDS_StringSeq_get_reference(&policy->expression_parameters, i);
        if ((parameter == NULL) || (*parameter == NULL))
        {
            continue;
        }
        if (self->filter_parameter_string_manager != NULL)
        {
            retval = DDS_StringManager_delete_string(self->filter_parameter_string_manager,
                                                     *parameter);
#if OSAPI_ENABLE_LOG
            if (!retval)
            {
                DDS_FILTER_LOG_STRING_DELETE(OSAPI_LOGKIND_ERROR,
                                            DDS_FILTER_LOG_FILTER_EXPRESSION_PARAMETER_STRING)
            }
#else
            IGNORE_RETVAL(retval);
#endif
        }
        *parameter = NULL;
    }
    retval = DDS_StringSeq_set_length(&policy->expression_parameters, 0);
    IGNORE_RETVAL(retval);
}

RTI_PRIVATE void
DDS_FilterPluginImpl_filter_property_finalize_shallow_copy(
        struct DDS_FilterPlugin *self,
        struct DDS_ContentFilterProperty *property);

RTI_PRIVATE DDS_Boolean
DDS_FilterPluginImpl_filter_property_shallow_copy(
        struct DDS_FilterPlugin *self,
        struct DDS_ContentFilterProperty *out,
        const struct DDS_ContentFilterProperty *in)
{
    RTI_INT32 i;
    RTI_INT32 parameter_count;
    DDS_Boolean result = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION(
            (self == NULL) || (out == NULL) || (in == NULL),
            goto done,
            OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("out", out, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("in", in, RTI_TRUE);)

    parameter_count = DDS_StringSeq_get_length(&in->expression_parameters);
    if (parameter_count > self->property.resource_limits.filter_parameter_max_count_per_expression)
    {
        /* Error: too many parameters */
        goto done;
    }

    /* content_filtered_topic_name */
    if (out->content_filtered_topic_name != NULL)
    {
        /* Error destination contains data. Do not copy to avoid memory leak. */
        goto done;
    }
    out->content_filtered_topic_name = DDS_StringManager_assert_string(
            self->property.topic_string_manager,
            in->content_filtered_topic_name != NULL ? in->content_filtered_topic_name : "");
    if (out->content_filtered_topic_name == NULL)
    {
        DDS_FILTER_LOG_STRING_ASSERT(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_CONTENT_FILTERED_TOPIC_NAME_STRING)
        goto done;
    }

    /* related_topic_name */
    if (out->related_topic_name != NULL)
    {
        goto done;
    }
    out->related_topic_name = DDS_StringManager_assert_string(
            self->property.topic_string_manager,
            in->related_topic_name != NULL ? in->related_topic_name : "");
    if (out->related_topic_name == NULL)
    {
        DDS_FILTER_LOG_STRING_ASSERT(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_RELATED_TOPIC_NAME_STRING)
        goto done;
    }

    /* filter_class_name */
    if (out->filter_class_name != NULL)
    {
        goto done;
    }
    out->filter_class_name = DDS_StringManager_assert_string(
            self->filter_class_string_manager,
            in->filter_class_name != NULL ? in->filter_class_name : "");
    if (out->filter_class_name == NULL)
    {
        DDS_FILTER_LOG_STRING_ASSERT(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_FILTER_CLASS_NAME_STRING)
        goto done;
    }

    /* filter_expression */
    if (out->filter_expression != NULL)
    {
        goto done;
    }
    out->filter_expression = DDS_StringManager_assert_string(
            self->filter_expression_string_manager,
            in->filter_expression != NULL ? in->filter_expression : "");
    if (out->filter_expression == NULL)
    {
        DDS_FILTER_LOG_STRING_ASSERT(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_FILTER_EXPRESSION_STRING)
        goto done;
    }

    /* expression_parameters */
    if (DDS_StringSeq_get_length(&out->expression_parameters) != 0)
    {
        goto done;
    }
    if (parameter_count >
        DDS_StringSeq_get_maximum(&out->expression_parameters))
    {
        if (!DDS_StringSeq_set_maximum(
                    &out->expression_parameters,
                    self->property.resource_limits
                        .filter_parameter_max_count_per_expression))
        {
            goto done;
        }
    }
    if (!DDS_StringSeq_set_length(
                &out->expression_parameters,
                parameter_count))
    {
        goto done;
    }
    for (i = 0; i < parameter_count; ++i)
    {
        DDS_String *parameter = DDS_StringSeq_get_reference(&out->expression_parameters, i);
        if (parameter == NULL)
        {
            goto done;
        }
        *parameter = DDS_StringManager_assert_string(
                self->filter_parameter_string_manager,
                *DDS_StringSeq_get_reference(&in->expression_parameters, i));
        if (*parameter == NULL)
        {
            DDS_FILTER_LOG_STRING_ASSERT(OSAPI_LOGKIND_ERROR,
                                         DDS_FILTER_LOG_FILTER_EXPRESSION_PARAMETER_STRING)
            goto done;
        }
    }

    result = DDS_BOOLEAN_TRUE;
done:
    if (!result)
    {
        DDS_FilterPluginImpl_filter_property_finalize_shallow_copy(
                self, out);
    }
    return result;
}

RTI_PRIVATE void
DDS_FilterPluginImpl_filter_property_finalize_shallow_copy(
        struct DDS_FilterPlugin *self,
        struct DDS_ContentFilterProperty *property)
{
    RTI_INT32 i;
    RTI_INT32 parameter_count;
    RTI_BOOL retval;

    OSAPI_PRECONDITION((self == NULL) || (property == NULL),
                        return,
                        OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
                        OSAPI_Log_entry_add_pointer("property", property, RTI_TRUE);)

    /* content_filtered_topic_name */
    if (property->content_filtered_topic_name != NULL)
    {
        retval = DDS_StringManager_delete_string(self->property.topic_string_manager,
                                                 property->content_filtered_topic_name);
#if OSAPI_ENABLE_LOG
        if (!retval)
        {
            DDS_FILTER_LOG_STRING_DELETE(OSAPI_LOGKIND_ERROR,
                                         DDS_FILTER_LOG_CONTENT_FILTERED_TOPIC_NAME_STRING)
        }
#else
        IGNORE_RETVAL(retval);
#endif
        property->content_filtered_topic_name = NULL;
    }

    /* related_topic_name */
    if (property->related_topic_name != NULL)
    {
        retval = DDS_StringManager_delete_string(self->property.topic_string_manager,
                                                 property->related_topic_name);
#if OSAPI_ENABLE_LOG
        if (!retval)
        {
            DDS_FILTER_LOG_STRING_DELETE(OSAPI_LOGKIND_ERROR,
                                         DDS_FILTER_LOG_RELATED_TOPIC_NAME_STRING)
        }
#else
        IGNORE_RETVAL(retval);
#endif
        property->related_topic_name = NULL;
    }

    /* filter_class_name */
    if (property->filter_class_name != NULL)
    {
        retval = DDS_StringManager_delete_string(self->filter_class_string_manager,
                                                 property->filter_class_name);
#if OSAPI_ENABLE_LOG
        if (!retval)
        {
            DDS_FILTER_LOG_STRING_DELETE(OSAPI_LOGKIND_ERROR,
                                         DDS_FILTER_LOG_FILTER_CLASS_NAME_STRING)
        }
#else
        IGNORE_RETVAL(retval);
#endif
        property->filter_class_name = NULL;
    }

    /* filter_expression */
    if (property->filter_expression != NULL)
    {
        retval = DDS_StringManager_delete_string(self->filter_expression_string_manager,
                                                 property->filter_expression);
#if OSAPI_ENABLE_LOG
        if (!retval)
        {
            DDS_FILTER_LOG_STRING_DELETE(OSAPI_LOGKIND_ERROR,
                                         DDS_FILTER_LOG_FILTER_EXPRESSION_STRING)
        }
#else
        IGNORE_RETVAL(retval);
#endif
        property->filter_expression = NULL;
    }

    /* expression_parameters */
    parameter_count = DDS_StringSeq_get_length(&property->expression_parameters);
    for (i = 0; i < parameter_count; ++i)
    {
        DDS_String *parameter = DDS_StringSeq_get_reference(&property->expression_parameters, i);
        if ((parameter == NULL) || (*parameter == NULL))
        {
            continue;
        }
        if (self->filter_parameter_string_manager != NULL)
        {
            retval = DDS_StringManager_delete_string(self->filter_parameter_string_manager,
                                                     *parameter);
#if OSAPI_ENABLE_LOG
            if (!retval)
            {
                DDS_FILTER_LOG_STRING_DELETE(OSAPI_LOGKIND_ERROR,
                                            DDS_FILTER_LOG_FILTER_EXPRESSION_PARAMETER_STRING)
            }
#else
            IGNORE_RETVAL(retval);
#endif
        }
        *parameter = NULL;
    }
    retval = DDS_StringSeq_set_length(&property->expression_parameters, 0);
    IGNORE_RETVAL(retval);
}

RTI_PRIVATE DDS_Boolean
DDS_FilterPluginImpl_filter_property_copy_from_filter(
        struct DDS_FilterPlugin *self,
        DDS_DataReader *reader,
        struct DDS_ContentFilterCompiledFilter *compiled_filter,
        struct DDS_ContentFilterProperty *out)
{
    const struct DDS_ContentFilterQosPolicy *dr_qos;
    struct DDS_DomainParticipantQos *dp_qos;

    const char *content_filtered_topic_name;
    const char *related_topic_name;
    const char *filter_class_name;
    const char *filter_expression;

    DDS_Boolean result = DDS_BOOLEAN_FALSE;

    OSAPI_PRECONDITION(
            (self == NULL) || (reader == NULL) || (out == NULL),
            return DDS_BOOLEAN_FALSE,
            OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("reader", reader, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("compiled_filter", compiled_filter, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("out", out, RTI_TRUE);)

    PRECOND_ARG(self)

    dr_qos = DDS_DataReader_get_content_filter_qos_ref(reader);
    dp_qos = DDS_DomainParticipant_get_qos_ref(
                    DDS_Subscriber_get_participant(
                            DDS_DataReader_get_subscriber(reader)));

    if (compiled_filter == NULL)
    {
        /* If a reader does not have a filter configured, copy out empty strings
         * so that the memory for the strings remains allocated or is allocated
         * and can be reused.
         */
        content_filtered_topic_name = "";
        related_topic_name = "";
        filter_class_name = "";
        filter_expression = "";
    }
    else
    {
        content_filtered_topic_name = compiled_filter->cft_name;
        related_topic_name = DDS_TopicDescription_get_name(DDS_DataReader_get_topicdescription(reader));
        filter_class_name = compiled_filter->filter_class->name;
        filter_expression = dr_qos->filter_expression;
    }

    if (!REDA_String_copy_w_max(
            &out->content_filtered_topic_name,
            content_filtered_topic_name,
            RTPS_PATHNAME_LEN_MAX))
    {
        goto done;
    }

    if (!REDA_String_copy_w_max(
            &out->related_topic_name,
            related_topic_name,
            RTPS_PATHNAME_LEN_MAX))
    {
        goto done;
    }

    if (!REDA_String_copy_w_max(
            &out->filter_class_name,
            filter_class_name,
            (RTI_UINT32)dp_qos->filter.resource_limits.filter_class_max_length))
    {
        goto done;
    }

    if (!REDA_String_copy_w_max(
            &out->filter_expression,
            filter_expression,
            (RTI_UINT32)dp_qos->filter.resource_limits.filter_expression_max_length))
    {
        goto done;
    }

    if (!DDS_StringSeq_copy(
            &out->expression_parameters,
            &dr_qos->expression_parameters))
    {
        goto done;
    }

    result = DDS_BOOLEAN_TRUE;
done:
    return result;
}

RTI_PRIVATE RTI_UINT32
DDS_FilterPluginImpl_filter_property_get_max_serialized_size(
        struct DDS_FilterPlugin *plugin,
        struct DDS_DomainParticipantQos *dp_qos,
        RTI_UINT32 size)
{
    RTI_UINT32 orig_size = size;

    OSAPI_PRECONDITION(
            (plugin == NULL) || (dp_qos == NULL),
            return 0,
            OSAPI_Log_entry_add_pointer("plugin", plugin, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("dp_qos", dp_qos, RTI_TRUE);)

    PRECOND_ARG(plugin)

    size += DDS_Cdr_get_parameter_header_max_size_serialized(size);
    size += CDR_get_max_size_serialized_string(size, RTPS_PATHNAME_LEN_MAX + 1);
    size += CDR_get_max_size_serialized_string(size, RTPS_PATHNAME_LEN_MAX + 1);
    size += CDR_get_max_size_serialized_string(size, (RTI_UINT32)dp_qos->filter.resource_limits.filter_class_max_length + 1);
    size += CDR_get_max_size_serialized_string(size, (RTI_UINT32)dp_qos->filter.resource_limits.filter_expression_max_length + 1);
    size += CDR_get_max_size_serialized_string_sequence(
                size,
                (RTI_UINT32)dp_qos->filter.resource_limits.filter_parameter_max_count_per_expression,
                (RTI_UINT32)dp_qos->filter.resource_limits.filter_parameter_max_length,
                CDR_CHAR_TYPE);

    return size - orig_size;
}

struct DDS_FilterPluginSerializeContext
{
    const char *content_filtered_topic_name;
    const char *related_topic_name;
    const struct DDS_ContentFilterQosPolicy *qos;
    const struct DDS_FilterResourceLimits *resource_limits;
};

RTI_PRIVATE RTI_BOOL
DDS_FilterPluginImpl_serialize_content_filter_property_parameter(struct CDR_Stream_t *stream,
                                                              const void *data,
                                                              void *param)
{
    struct DDS_FilterPluginSerializeContext *ctx =
            (struct DDS_FilterPluginSerializeContext *)data;

    UNUSED_ARG(param);

    return CDR_Stream_serialize_string(stream, ctx->content_filtered_topic_name, RTPS_PATHNAME_LEN_MAX)
            && CDR_Stream_serialize_string(stream, ctx->related_topic_name, RTPS_PATHNAME_LEN_MAX)
            && CDR_Stream_serialize_string(stream, ctx->qos->filter_class_name, (RTI_UINT32)ctx->resource_limits->filter_class_max_length)
            && CDR_Stream_serialize_string(stream, ctx->qos->filter_expression, (RTI_UINT32)ctx->resource_limits->filter_expression_max_length)
            && CDR_Stream_serialize_string_sequence(stream,
                    (struct REDA_Sequence *)&ctx->qos->expression_parameters,
                    (RTI_UINT32)ctx->resource_limits->filter_parameter_max_length,
                    CDR_CHAR_TYPE);
}

RTI_PRIVATE DDS_Boolean
DDS_FilterPluginImpl_filter_property_serialize(
        const struct DDS_FilterPlugin *self,
        struct CDR_Stream_t *stream,
        const DDS_DataReader *reader,
        const struct DDS_ContentFilterCompiledFilter *compiled_filter)
{
    struct DDS_FilterPluginSerializeContext ctx;

    OSAPI_PRECONDITION(
            (self == NULL) || (stream == NULL) || (reader == NULL) || (compiled_filter == NULL),
            return DDS_BOOLEAN_FALSE,
            OSAPI_Log_entry_add_pointer("self", self, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("stream", stream, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("reader", reader, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("compiled_filter", compiled_filter, RTI_TRUE);)

    ctx.content_filtered_topic_name = compiled_filter->cft_name;
    ctx.related_topic_name = DDS_TopicDescription_get_name(
                                    DDS_DataReader_get_topicdescription((DDS_DataReader *)reader));
    ctx.qos = DDS_DataReader_get_content_filter_qos_ref(reader);
    ctx.resource_limits = &self->property.resource_limits;

    if (!DDS_CdrStream_serialize_non_primitive_parameter(
            stream,
            &ctx,
            DDS_FilterPluginImpl_serialize_content_filter_property_parameter,
            RTPS_PID_CONTENT_FILTER_PROPERTY, RTI_FALSE, RTI_TRUE))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

RTI_PRIVATE DDS_Boolean
DDS_FilterPluginImpl_filter_property_deserialize(
        struct DDS_FilterPlugin *self,
        struct CDR_Stream_t *stream,
        struct DDS_ContentFilterProperty *prop)
{
    char *string;
    RTI_UINT32 string_length;
    RTI_INT32 i, seq_length;
    DDS_Boolean ok = DDS_BOOLEAN_FALSE;

    /* Release any prior string-manager refs in case a non-conformant reader
     * sends multiple filter properties in a single announcement.
     */
    DDS_FilterPluginImpl_filter_property_finalize_shallow_copy(self, prop);

    /* Deserialize content_filtered_topic_name */
    if (!CDR_Stream_deserialize_get_string(stream, &string, &string_length))
    {
        goto done;
    }
    if (string_length > 0)
    {
        prop->content_filtered_topic_name = DDS_StringManager_assert_string(
                self->property.topic_string_manager,
                string);
        if (prop->content_filtered_topic_name == NULL)
        {
            DDS_FILTER_LOG_STRING_ASSERT(OSAPI_LOGKIND_WARNING,
                                         DDS_FILTER_LOG_CONTENT_FILTERED_TOPIC_NAME_STRING)
            goto done;
        }
    }
    else
    {
        prop->content_filtered_topic_name = NULL;
    }

    /* Deserialize related_topic_name */
    if (!CDR_Stream_deserialize_get_string(stream, &string, &string_length))
    {
        goto done;
    }
    if (string_length > 0)
    {
        prop->related_topic_name = DDS_StringManager_assert_string(
                self->property.topic_string_manager,
                string);
        if (prop->related_topic_name == NULL)
        {
            DDS_FILTER_LOG_STRING_ASSERT(OSAPI_LOGKIND_WARNING,
                                         DDS_FILTER_LOG_RELATED_TOPIC_NAME_STRING)
            goto done;
        }
    }
    else
    {
        prop->related_topic_name = NULL;
    }

    /* Deserialize filter_class_name */
    if (!CDR_Stream_deserialize_get_string(stream, &string, &string_length))
    {
        goto done;
    }
    if (string_length > 0)
    {
        prop->filter_class_name = DDS_StringManager_assert_string(
                self->filter_class_string_manager,
                string);
        if (prop->filter_class_name == NULL)
        {
            DDS_FILTER_LOG_STRING_ASSERT(OSAPI_LOGKIND_WARNING,
                                         DDS_FILTER_LOG_FILTER_CLASS_NAME_STRING)
            goto done;
        }
    }
    else
    {
        prop->filter_class_name = NULL;
    }

    /* Deserialize filter_expression */
    if (!CDR_Stream_deserialize_get_string(stream, &string, &string_length))
    {
        goto done;
    }
    if (string_length > 0)
    {
        prop->filter_expression = DDS_StringManager_assert_string(
                self->filter_expression_string_manager,
                string);
        if (prop->filter_expression == NULL)
        {
            DDS_FILTER_LOG_STRING_ASSERT(OSAPI_LOGKIND_WARNING,
                                         DDS_FILTER_LOG_FILTER_EXPRESSION_STRING)
            goto done;
        }
    }
    else
    {
        prop->filter_expression = NULL;
    }

    /* Deserialize expression_parameters */
    if (!CDR_Stream_deserialize_long(stream, &seq_length))
    {
        goto done;
    }
    if (seq_length > DDS_StringSeq_get_maximum(&prop->expression_parameters))
    {
        DDS_FILTER_LOG_FILTER_PARAMETER_COUNT(
                OSAPI_LOGKIND_WARNING,
                seq_length,
                DDS_StringSeq_get_maximum(&prop->expression_parameters))
        goto done;
    }
    if (!DDS_StringSeq_set_length(&prop->expression_parameters, seq_length))
    {
        goto done;
    }
    for (i = 0; i < seq_length; ++i)
    {
        DDS_String *parameter = DDS_StringSeq_get_reference(&prop->expression_parameters, i);
        if (parameter == NULL)
        {
            goto done;
        }

        if (!CDR_Stream_deserialize_get_string(stream, &string, &string_length))
        {
            goto done;
        }
        *parameter = DDS_StringManager_assert_string(
                self->filter_parameter_string_manager,
                string);
        if (*parameter == NULL)
        {
            DDS_FILTER_LOG_STRING_ASSERT(OSAPI_LOGKIND_WARNING,
                                         DDS_FILTER_LOG_FILTER_EXPRESSION_PARAMETER_STRING)
            goto done;
        }
    }
    ok = DDS_BOOLEAN_TRUE;
done:
    if (!ok)
    {
        /* If an error occurs, finalize the property to avoid leaked resources */
        DDS_FilterPluginImpl_filter_property_finalize_shallow_copy(self, prop);
    }
    return ok;
}

DDS_Boolean
DDS_FilterPluginImpl_compile_filter(
        struct DDS_FilterPlugin *self,
        struct DDS_FilterSignature *signature,
        DDS_String cft_name,
        DDS_String filter_class_name,
        DDS_String filter_expression,
        const struct DDS_StringSeq *expression_parameters,
        struct DDS_TypeCode *type_code,
        const char *type_class_name,
        struct DDS_ContentFilterCompiledFilter **filter_inout)
{
    struct DDS_ContentFilterCompiledFilter *record = NULL;
    struct DDS_ContentFilterClass *filter_class = NULL;
    DDS_String cft_name_copy = NULL;
    void *compiled_filter_instance = NULL;
    DB_ReturnCode_T dbrc;
    RTI_BOOL retval;
    DDS_Boolean ok = DDS_BOOLEAN_FALSE;
    DDS_Boolean is_cached = DDS_BOOLEAN_FALSE;

    /* Check if filter is already compiled */
    dbrc = DB_Table_select_match(self->compiled_filter_table,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T)&record,
                                 (DB_Key_T)signature);
    if (dbrc == DB_RETCODE_OK)
    {
        /* Filter already exists */
        record->ref_count++;
        is_cached = DDS_BOOLEAN_TRUE;
        ok = DDS_BOOLEAN_TRUE;
        goto done;
    }
    else if (dbrc != DB_RETCODE_NO_DATA)
    {
        /* Unknown error */
        DDS_FILTER_LOG_RECORD_SELECT(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_COMPILED_FILTER_RECORD,
                                     dbrc)
        goto done;
    }

    if (self->compiled_filter_count >=
            (RTI_UINT32)self->property.resource_limits.filter_expression_max_count)
    {
        /* If the previous filter will be released after this compilation,
         * then we can safely compile this filter and then release it
         * because additional space was allocated in the table for one
         * additional filter beyond the resource limit.
         */
        if ((*filter_inout == NULL) || ((*filter_inout)->ref_count > 1))
        {
            DDS_FILTER_LOG_FILTER_EXPRESSION_MAX_COUNT(
                    OSAPI_LOGKIND_WARNING,
                    self->property.resource_limits.filter_expression_max_count)
            goto done;
        }
    }

    /* Find the filter class */
    dbrc = DB_Table_select_match(self->filter_class_table,
                                 DB_TABLE_DEFAULT_INDEX,
                                 (DB_Record_T)&filter_class,
                                 (DB_Key_T)filter_class_name);
    if (dbrc == DB_RETCODE_NO_DATA)
    {
        DDS_FILTER_LOG_FILTER_CLASS_NOT_FOUND(OSAPI_LOGKIND_WARNING, filter_class_name)
        goto done;
    }
    else if (dbrc != DB_RETCODE_OK)
    {
        DDS_FILTER_LOG_RECORD_SELECT(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_FILTER_CLASS_RECORD,
                                     dbrc)
        goto done;
    }

    cft_name_copy = DDS_StringManager_assert_string(
                            self->property.topic_string_manager,
                            cft_name);
    if (cft_name_copy == NULL)
    {
        DDS_FILTER_LOG_STRING_ASSERT(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_CONTENT_FILTERED_TOPIC_NAME_STRING)
        goto done;
    }

    /* Compile the filter using the filter class */
    if (!filter_class->intf->compile(
            filter_class->instance,
            &compiled_filter_instance,
            filter_expression,
            expression_parameters,
            type_code,
            type_class_name))
    {
        DDS_FILTER_LOG_FILTER_COMPILE(OSAPI_LOGKIND_WARNING, filter_class_name)
        goto done;
    }

    /* Create a record for the compiled filter */
    dbrc = DB_Table_create_record(self->compiled_filter_table,
                                  (DB_Record_T)&record);
    if (dbrc != DB_RETCODE_OK)
    {
        DDS_FILTER_LOG_RECORD_CREATE(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_COMPILED_FILTER_RECORD,
                                     dbrc)
        goto done;
    }

    record->filter_signature = *signature;
    record->cft_name = cft_name_copy;
    record->filter_class = filter_class;
    record->compiled_filter_instance = compiled_filter_instance;
    record->ref_count = 1;

    /* Insert the record into the compiled filter table */
    dbrc = DB_Table_insert_record(self->compiled_filter_table,
                                  (DB_Record_T)record);
    if (dbrc != DB_RETCODE_OK)
    {
        DDS_FILTER_LOG_RECORD_INSERT(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_COMPILED_FILTER_RECORD,
                                     dbrc)
        goto done;
    }

    /* Increase the reference count of the filter class */
    filter_class->ref_count++;

    ok = DDS_BOOLEAN_TRUE;
done:
    if (!ok)
    {
        if (cft_name_copy != NULL)
        {
            retval = DDS_StringManager_delete_string(
                                            self->property.topic_string_manager,
                                            cft_name_copy);
#if OSAPI_ENABLE_LOG
            if (!retval)
            {
                DDS_FILTER_LOG_STRING_DELETE(OSAPI_LOGKIND_WARNING,
                                             DDS_FILTER_LOG_CONTENT_FILTERED_TOPIC_NAME_STRING)
            }
#else
            IGNORE_RETVAL(retval);
#endif
        }
        if (record != NULL)
        {
            dbrc = DB_Table_delete_record(self->compiled_filter_table, (DB_Record_T)record);
#if OSAPI_ENABLE_LOG
            if (dbrc != DB_RETCODE_OK)
            {
                DDS_FILTER_LOG_RECORD_DELETE(OSAPI_LOGKIND_ERROR,
                                             DDS_FILTER_LOG_COMPILED_FILTER_RECORD,
                                             dbrc)
            }
#else
            IGNORE_RETVAL(dbrc);
#endif
        }
        if (compiled_filter_instance != NULL)
        {
            filter_class->intf->finalize(filter_class->instance, compiled_filter_instance);

        }
    }
    else
    {
        if (*filter_inout != NULL)
        {
            DDS_FilterPluginImpl_finalize_filter(self, *filter_inout);
        }

        if (!is_cached)
        {
            self->compiled_filter_count++;
        }
        *filter_inout = record;
    }

    return ok;
}

void
DDS_FilterPluginImpl_finalize_filter(
        struct DDS_FilterPlugin *plugin,
        struct DDS_ContentFilterCompiledFilter *compiled_filter)
{
    DB_ReturnCode_T dbrc;
    struct DDS_ContentFilterCompiledFilter *record = NULL;
    RTI_BOOL retval;

    OSAPI_PRECONDITION(
            (plugin == NULL) || (compiled_filter == NULL),
            return,
            OSAPI_Log_entry_add_pointer("plugin", plugin, RTI_FALSE);
            OSAPI_Log_entry_add_pointer("compiled_filter", compiled_filter, RTI_TRUE);)

    /* Decrease the reference count of the compiled filter */
    compiled_filter->ref_count--;
    if (compiled_filter->ref_count > 0)
    {
        /* Filter is still in use */
        return;
    }

    /* Remove the compiled filter from the table */
    dbrc = DB_Table_remove_record(plugin->compiled_filter_table,
                                 (DB_Record_T)&record,
                                 (DB_Key_T)&compiled_filter->filter_signature);
    if (dbrc != DB_RETCODE_OK)
    {
        DDS_FILTER_LOG_RECORD_REMOVE(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_COMPILED_FILTER_RECORD,
                                     dbrc)
        return;
    }

    /* Delete the compiled filter instance */
    record->filter_class->intf->finalize(record->filter_class->instance,
                                         record->compiled_filter_instance);
    record->filter_class->ref_count--;
    record->filter_class = NULL;
    record->compiled_filter_instance = NULL;

    retval = DDS_StringManager_delete_string(plugin->property.topic_string_manager,
                                             record->cft_name);
#if OSAPI_ENABLE_LOG
    if (!retval)
    {
        DDS_FILTER_LOG_STRING_DELETE(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_CONTENT_FILTERED_TOPIC_NAME_STRING)
    }
#else
    IGNORE_RETVAL(retval);
#endif
    record->cft_name = NULL;

    /* Delete the compiled filter record */
    dbrc = DB_Table_delete_record(plugin->compiled_filter_table, (DB_Record_T)record);
#if OSAPI_ENABLE_LOG
    if (dbrc != DB_RETCODE_OK)
    {
        DDS_FILTER_LOG_RECORD_DELETE(OSAPI_LOGKIND_ERROR,
                                     DDS_FILTER_LOG_COMPILED_FILTER_RECORD,
                                     dbrc)
    }
#else
    IGNORE_RETVAL(dbrc);
#endif

    plugin->compiled_filter_count--;
}


RTI_PRIVATE struct DDS_FilterPluginI DDS_FilterPluginImpl_fv_Intf =
{
    {
        RT_COMPONENTI_BASE,
        DDS_FilterPluginImpl_writer_add_route,
        DDS_FilterPluginImpl_writer_delete_route,
        DDS_FilterPluginImpl_writer_apply_filter,
        DDS_FilterPluginImpl_for_each_reliable_peer
    },
    DDS_FilterPluginImpl_filter_qos_policy_shallow_copy,
    DDS_FilterPluginImpl_filter_qos_policy_finalize_shallow_copy,
    DDS_FilterPluginImpl_filter_property_shallow_copy,
    DDS_FilterPluginImpl_filter_property_finalize_shallow_copy,
    DDS_FilterPluginImpl_filter_property_copy_from_filter,
    DDS_FilterPluginImpl_filter_property_get_max_serialized_size,
    DDS_FilterPluginImpl_filter_property_serialize,
    DDS_FilterPluginImpl_filter_property_deserialize,
    DDS_FilterPluginImpl_register_filter_class,
    DDS_FilterPluginImpl_unregister_filter_class,
    DDS_FilterPluginImpl_reader_compile,
    DDS_FilterPluginImpl_finalize_filter,
    DDS_FilterPluginImpl_reader_process_filter_info,
    DDS_FilterPluginImpl_reader_evaluate,
    DDS_FilterPluginImpl_writer_attach,
#ifndef RTI_CERT
    DDS_FilterPluginImpl_writer_detach,
#else
    NULL,
#endif /* !RTI_CERT */
    DDS_FilterPluginImpl_writer_add_remote_reader,
    DDS_FilterPluginImpl_writer_add_local_reader,
    DDS_FilterPluginImpl_writer_remove_reader,
    DDS_FilterPluginImpl_writer_evaluate,
    DDS_FilterPluginImpl_writer_apply_reader_filter
};
