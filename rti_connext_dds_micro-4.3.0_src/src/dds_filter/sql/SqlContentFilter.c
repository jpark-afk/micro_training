/*
 * FILE: SqlContentFilter.c - DDS SQL Content Filter implementation
 *
 * Copyright (c) 2025-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#include "SqlContentFilter.h"
#include "SqlFilterExecutor.h"
#include "SqlFilterParser.h"
#include "dds_c/dds_c_domain.h"
#include "dds_filter/dds_filter.h"

/*** SOURCE_BEGIN ***/

RTI_PRIVATE RTI_BOOL
DDS_SqlCompiledContentFilter_initialize(void *initialize_param, void *buffer)
{
    struct DDS_SqlContentFilter *filter_instance = (struct DDS_SqlContentFilter *)initialize_param;
    struct DDS_SqlCompiledContentFilter *compiled_filter =
            (struct DDS_SqlCompiledContentFilter *)buffer;

    *compiled_filter = (struct DDS_SqlCompiledContentFilter)DDS_SqlCompiledContentFilter_INITIALIZER;

    OSAPI_Heap_allocate_array(&compiled_filter->predicates,
                              (RTI_SIZE_T)filter_instance->property.predicate_max_count_per_expression,
                              struct DDS_SqlPredicate);
    if (compiled_filter->predicates == NULL)
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

#ifndef RTI_CERT
RTI_PRIVATE RTI_BOOL
DDS_SqlCompiledContentFilter_finalize(void *finalize_param, void *buffer)
{
    struct DDS_SqlCompiledContentFilter *compiled_filter =
            (struct DDS_SqlCompiledContentFilter *)buffer;

    UNUSED_ARG(finalize_param);

    if (compiled_filter->predicates != NULL)
    {
        OSAPI_Heap_free_array(compiled_filter->predicates);
    }

    return RTI_TRUE;
}
#endif

void *
DDS_SqlContentFilter_create_instance(
        const struct DDS_FilterResourceLimits *resource_limits,
        const void *property)
{
    struct DDS_SqlContentFilter *filter_instance = NULL;
    struct REDA_BufferPoolProperty pool_prop = REDA_BufferPoolProperty_INITIALIZER;
    const struct DDS_SqlFilterProperty *sql_property = (const struct DDS_SqlFilterProperty *)property;

    OSAPI_PRECONDITION((resource_limits == NULL),
                            return NULL,
                            OSAPI_Log_entry_add_pointer("resource_limits", resource_limits, RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("property", property, RTI_TRUE);)

    if (property != NULL)
    {
        /* Limit to the maximum value of an unsigned short to allow
         * DDS_SqlPredicate.next_or_index to use a short for indexing.
         */
        if ((sql_property->predicate_max_count_per_expression <= 0)
            || (sql_property->predicate_max_count_per_expression > USHRT_MAX))
        {
            DDS_FILTER_LOG_SQL_INVALID_PROPERTY(
                    OSAPI_LOGKIND_ERROR,
                    sql_property->predicate_max_count_per_expression)
            return NULL;
        }
    }

    OSAPI_Heap_allocate_struct(&filter_instance, struct DDS_SqlContentFilter);
    if (filter_instance == NULL)
    {
        return NULL;
    }

    if (property != NULL)
    {
        filter_instance->property = *(const struct DDS_SqlFilterProperty *)property;
    }
    else
    {
        filter_instance->property =
                (struct DDS_SqlFilterProperty)DDS_SqlFilterProperty_INITIALIZER;
    }

    pool_prop.buffer_size = sizeof(struct DDS_SqlCompiledContentFilter);
    pool_prop.max_buffers = (RTI_SIZE_T)resource_limits->filter_expression_max_count;

    filter_instance->compiled_filter_pool = REDA_BufferPool_new(
                                        "sql_filter",
                                        &pool_prop,
                                        DDS_SqlCompiledContentFilter_initialize,
                                        filter_instance,
#ifndef RTI_CERT
                                        DDS_SqlCompiledContentFilter_finalize,
                                        filter_instance);
#else
                                        NULL,
                                        NULL);
#endif
    if (filter_instance->compiled_filter_pool == NULL)
    {
#ifndef RTI_CERT
        OSAPI_Heap_free_struct(filter_instance);
#endif
        return NULL;
    }

    return filter_instance;
}

#ifndef RTI_CERT
void
DDS_SqlContentFilter_delete_instance(void *filter_instance)
{
    struct DDS_SqlContentFilter *sql_filter_instance = (struct DDS_SqlContentFilter *)filter_instance;
    RTI_BOOL bretval;

    OSAPI_PRECONDITION((filter_instance == NULL),
                        return,
                        OSAPI_Log_entry_add_pointer("filter_instance", filter_instance, RTI_TRUE);)

    if (sql_filter_instance->compiled_filter_pool != NULL)
    {
        bretval = REDA_BufferPool_delete(sql_filter_instance->compiled_filter_pool);
        IGNORE_RETVAL(bretval);

        sql_filter_instance->compiled_filter_pool = NULL;
    }

    OSAPI_Heap_free_struct(sql_filter_instance);
}
#endif

DDS_Boolean
DDS_SqlContentFilter_compile(
        void *filter_instance,
        void **compiled_expression_out,
        const char *expression,
        const struct DDS_StringSeq *parameters,
        const struct DDS_TypeCode *type_code,
        const char *type_class_name)
{
    struct DDS_SqlContentFilter *sql_filter_instance =
            (struct DDS_SqlContentFilter *)filter_instance;
    struct DDS_SqlCompiledContentFilter *compiled_filter = NULL;

    UNUSED_ARG(type_class_name);

    OSAPI_PRECONDITION((filter_instance == NULL) || (compiled_expression_out == NULL)
                                  || (expression == NULL) || (parameters == NULL)
                                  || (type_code == NULL),
                            return DDS_BOOLEAN_FALSE,
                            OSAPI_Log_entry_add_pointer("filter_instance", filter_instance, RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("compiled_expression_out", compiled_expression_out, RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("expression", expression, RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("parameters", parameters, RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("type_code", type_code, RTI_TRUE);)

    compiled_filter = REDA_BufferPool_get_buffer(sql_filter_instance->compiled_filter_pool);
    if (compiled_filter == NULL)
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_SqlContentFilter_compile_filter_expression(sql_filter_instance,
                                                        compiled_filter,
                                                        expression,
                                                        parameters,
                                                        type_code))
    {
        REDA_BufferPool_return_buffer(sql_filter_instance->compiled_filter_pool, compiled_filter);
        return DDS_BOOLEAN_FALSE;
    }

    *compiled_expression_out = compiled_filter;

    return DDS_BOOLEAN_TRUE;
}

DDS_Boolean
DDS_SqlContentFilter_evaluate(
        void *filter_instance,
        void *compiled_expression,
        const void *sample,
        DDS_Boolean *result_out)
{
    struct DDS_SqlCompiledContentFilter *compiled_filter =
            (struct DDS_SqlCompiledContentFilter *)compiled_expression;

    OSAPI_PRECONDITION((filter_instance == NULL) || (compiled_expression == NULL)
                                  || (sample == NULL) || (result_out == NULL),
                            return DDS_BOOLEAN_FALSE,
                            OSAPI_Log_entry_add_pointer("filter_instance", filter_instance, RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("compiled_expression", compiled_expression, RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("sample", sample, RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("result_out", result_out, RTI_TRUE);)

    UNUSED_ARG(filter_instance);

    return DDS_SqlCompiledContentFilter_evaluate(compiled_filter,
                                                 sample,
                                                 result_out);
}

void
DDS_SqlContentFilter_finalize(
        void *filter_instance,
        void *compiled_expression)
{
    struct DDS_SqlContentFilter *sql_filter_instance =
            (struct DDS_SqlContentFilter *)filter_instance;
    struct DDS_SqlCompiledContentFilter *compiled_filter =
            (struct DDS_SqlCompiledContentFilter *)compiled_expression;

    OSAPI_PRECONDITION_ALWAYS((filter_instance == NULL) || (compiled_expression == NULL),
                            return,
                            OSAPI_Log_entry_add_pointer("filter_instance", filter_instance, RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("compiled_expression", compiled_expression, RTI_TRUE);)

    compiled_filter->predicate_count = 0;

    REDA_BufferPool_return_buffer(sql_filter_instance->compiled_filter_pool, compiled_filter);
}

RTI_PRIVATE const struct DDS_ContentFilterI DDS_SqlContentFilter_fv_interface =
{
    DDS_SqlContentFilter_create_instance,
#ifndef RTI_CERT
    DDS_SqlContentFilter_delete_instance,
#endif
    DDS_SqlContentFilter_compile,
    DDS_SqlContentFilter_evaluate,
    DDS_SqlContentFilter_finalize
};

const struct DDS_ContentFilterI *
DDS_SqlContentFilter_get_interface(void)
{
    return &DDS_SqlContentFilter_fv_interface;
}
