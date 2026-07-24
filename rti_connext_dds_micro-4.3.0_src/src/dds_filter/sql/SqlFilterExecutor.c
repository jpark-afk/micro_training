/*
 * FILE: SqlFilterExecutor.c - DDS SQL Content Filter evaluation
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

/*** SOURCE_BEGIN ***/

DDS_Boolean
DDS_SqlCompiledContentFilter_evaluate(
        const struct DDS_SqlCompiledContentFilter *compiled_filter,
        const void *sample,
        DDS_Boolean *result_out)
{
    DDS_Boolean prev_result = DDS_BOOLEAN_TRUE;
    DDS_UnsignedShort i;
    const struct DDS_SqlPredicate *predicate;

    for (i = 0; i < compiled_filter->predicate_count; ++i)
    {
        predicate = &compiled_filter->predicates[i];

        if (predicate->flags & DDS_SQL_FLAG_AND)
        {
            if (prev_result)
            {
                if (!DDS_SqlPredicate_evaluate(predicate, sample, &prev_result))
                {
                    return DDS_BOOLEAN_FALSE;
                }
            }
            else
            {
                /* AND chain failed; skip ahead to the next OR predicate. */
                i = (DDS_UnsignedShort)(predicate->next_or_index - 1);
                continue;
            }
        }
        else
        {
            /* Previous conditional operator was OR */
            if (prev_result)
            {
                /* If the previous result was true, we can stop evaluating
                 * further predicates since the overall result will be true.
                 */
                break;
            }
            else
            {
                if (!DDS_SqlPredicate_evaluate(predicate, sample, &prev_result))
                {
                    return DDS_BOOLEAN_FALSE;
                }
            }
        }
    }

    *result_out = prev_result;

    return DDS_BOOLEAN_TRUE;
}

DDS_Boolean
DDS_SqlPredicate_evaluate(
        const struct DDS_SqlPredicate *predicate,
        const void *sample,
        DDS_Boolean *result_out)
{
    union DDS_SqlImmediateValue left;
    union DDS_SqlImmediateValue right;

    if (predicate->flags & DDS_SQL_FLAG_A_FIELD)
    {
        if (!DDS_SqlFieldValue_get_value(&predicate->left.field,
                                         sample,
                                         predicate->comparison_type,
                                         &left))
        {
            return DDS_BOOLEAN_FALSE;
        }
    }
    else
    {
        left = predicate->left.immediate;
    }

    if (predicate->flags & DDS_SQL_FLAG_B_FIELD)
    {
        if (!DDS_SqlFieldValue_get_value(&predicate->right.field,
                                         sample,
                                         predicate->comparison_type,
                                         &right))
        {
            return DDS_BOOLEAN_FALSE;
        }
    }
    else
    {
        right = predicate->right.immediate;
    }

    if (!DDS_SqlFilter_apply_comparison(predicate, &left, &right, result_out))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (predicate->flags & DDS_SQL_FLAG_NOT)
    {
        *result_out = !(*result_out);
    }

    return DDS_BOOLEAN_TRUE;
}

DDS_Boolean
DDS_SqlFieldValue_get_value(
        const struct DDS_SqlFieldValue *field,
        const void *sample,
        enum DDS_SqlImmediateType value_type,
        union DDS_SqlImmediateValue *value_out)
{
    void *field_ptr = ((char *)sample) + field->offset;

    switch (value_type)
    {
        case DDS_SQL_TYPE_INT64:
            switch (field->type)
            {
                case DDS_TK_BOOLEAN:
                    value_out->int64_value = (DDS_LongLong)*(DDS_Boolean *)field_ptr;
                    break;
                case DDS_TK_ENUM:
                    value_out->int64_value = (DDS_LongLong)*(DDS_Long *)field_ptr;
                    break;
                case DDS_TK_CHAR:
                    value_out->int64_value = (DDS_LongLong)(unsigned char)*(DDS_Char *)field_ptr;
                    break;
                case DDS_TK_OCTET:
                    value_out->int64_value = (DDS_LongLong)*(DDS_Octet *)field_ptr;
                    break;
                case DDS_TK_SHORT:
                    value_out->int64_value = (DDS_LongLong)*(DDS_Short *)field_ptr;
                    break;
                case DDS_TK_LONG:
                    value_out->int64_value = (DDS_LongLong)*(DDS_Long *)field_ptr;
                    break;
                case DDS_TK_LONGLONG:
                    value_out->int64_value = (DDS_LongLong)*(DDS_LongLong *)field_ptr;
                    break;
                case DDS_TK_USHORT:
                    value_out->int64_value = (DDS_LongLong)*(DDS_UnsignedShort *)field_ptr;
                    break;
                case DDS_TK_ULONG:
                    value_out->int64_value = (DDS_LongLong)*(DDS_UnsignedLong *)field_ptr;
                    break;
                default:
                    return DDS_BOOLEAN_FALSE;
            }
            break;
        case DDS_SQL_TYPE_UINT64:
            switch (field->type)
            {
                case DDS_TK_ULONGLONG:
                    value_out->uint64_value = *(DDS_UnsignedLongLong *)field_ptr;
                    break;
                case DDS_TK_ULONG:
                    value_out->uint64_value = (DDS_UnsignedLongLong)*(DDS_UnsignedLong *)field_ptr;
                    break;
                case DDS_TK_USHORT:
                    value_out->uint64_value = (DDS_UnsignedLongLong)*(DDS_UnsignedShort *)field_ptr;
                    break;
                case DDS_TK_OCTET:
                    value_out->uint64_value = (DDS_UnsignedLongLong)*(DDS_Octet *)field_ptr;
                    break;
                case DDS_TK_LONGLONG:
                    value_out->uint64_value = (DDS_UnsignedLongLong)*(DDS_LongLong *)field_ptr;
                    break;
                case DDS_TK_LONG:
                    value_out->uint64_value = (DDS_UnsignedLongLong)*(DDS_Long *)field_ptr;
                    break;
                case DDS_TK_SHORT:
                    value_out->uint64_value = (DDS_UnsignedLongLong)*(DDS_Short *)field_ptr;
                    break;
                case DDS_TK_ENUM:
                    value_out->uint64_value = (DDS_UnsignedLongLong)*(DDS_Long *)field_ptr;
                    break;
                default:
                    return DDS_BOOLEAN_FALSE;
            }
            break;
        case DDS_SQL_TYPE_FLOAT:
            switch (field->type)
            {
                case DDS_TK_OCTET:
                    value_out->float_value = (DDS_Float)*(DDS_Octet *)field_ptr;
                    break;
                case DDS_TK_SHORT:
                    value_out->float_value = (DDS_Float)*(DDS_Short *)field_ptr;
                    break;
                case DDS_TK_LONG:
                    value_out->float_value = (DDS_Float)*(DDS_Long *)field_ptr;
                    break;
                case DDS_TK_LONGLONG:
                    value_out->float_value = (DDS_Float)*(DDS_LongLong *)field_ptr;
                    break;
                case DDS_TK_USHORT:
                    value_out->float_value = (DDS_Float)*(DDS_UnsignedShort *)field_ptr;
                    break;
                case DDS_TK_ULONG:
                    value_out->float_value = (DDS_Float)*(DDS_UnsignedLong *)field_ptr;
                    break;
                case DDS_TK_ULONGLONG:
                    value_out->float_value = (DDS_Float)*(DDS_UnsignedLongLong *)field_ptr;
                    break;
                case DDS_TK_FLOAT:
                    value_out->float_value = *(DDS_Float *)field_ptr;
                    break;
                default:
                    return DDS_BOOLEAN_FALSE;
            }
            break;
        case DDS_SQL_TYPE_DOUBLE:
            switch (field->type)
            {
                case DDS_TK_OCTET:
                    value_out->double_value = (DDS_Double)*(DDS_Octet *)field_ptr;
                    break;
                case DDS_TK_SHORT:
                    value_out->double_value = (DDS_Double)*(DDS_Short *)field_ptr;
                    break;
                case DDS_TK_LONG:
                    value_out->double_value = (DDS_Double)*(DDS_Long *)field_ptr;
                    break;
                case DDS_TK_LONGLONG:
                    value_out->double_value = (DDS_Double)*(DDS_LongLong *)field_ptr;
                    break;
                case DDS_TK_USHORT:
                    value_out->double_value = (DDS_Double)*(DDS_UnsignedShort *)field_ptr;
                    break;
                case DDS_TK_ULONG:
                    value_out->double_value = (DDS_Double)*(DDS_UnsignedLong *)field_ptr;
                    break;
                case DDS_TK_ULONGLONG:
                    value_out->double_value = (DDS_Double)*(DDS_UnsignedLongLong *)field_ptr;
                    break;
                case DDS_TK_FLOAT:
                    value_out->double_value = (DDS_Double)*(DDS_Float *)field_ptr;
                    break;
                case DDS_TK_DOUBLE:
                    value_out->double_value = *(DDS_Double *)field_ptr;
                    break;
                default:
                    return DDS_BOOLEAN_FALSE;
            }
            break;
        default:
            return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

/* Three-way compare for UINT64 with signed-source flags. The signed bit
 * pattern survives the cast into uint64_value and is recovered by
 * reinterpreting as DDS_LongLong. Returns -1, 0, or +1.
 */
RTI_PRIVATE RTI_INT32
DDS_SqlFilter_compare_uint64_signaware(DDS_UnsignedLongLong a,
                                       DDS_UnsignedLongLong b,
                                       DDS_Octet flags)
{
    DDS_Boolean a_is_signed = ((flags & DDS_SQL_FLAG_A_SIGNED_SOURCE) != 0);
    DDS_Boolean b_is_signed = ((flags & DDS_SQL_FLAG_B_SIGNED_SOURCE) != 0);
    DDS_LongLong a_signed = (DDS_LongLong)a;
    DDS_LongLong b_signed = (DDS_LongLong)b;

    if (a_is_signed && (a_signed < 0))
    {
        if (b_is_signed && (b_signed < 0))
        {
            return (a_signed < b_signed) ? -1 : ((a_signed > b_signed) ? 1 : 0);
        }
        return -1;
    }
    if (b_is_signed && (b_signed < 0))
    {
        return 1;
    }
    return (a < b) ? -1 : ((a > b) ? 1 : 0);
}

DDS_Boolean
DDS_SqlFilter_apply_comparison(
        const struct DDS_SqlPredicate *predicate,
        const union DDS_SqlImmediateValue *a,
        const union DDS_SqlImmediateValue *b,
        DDS_Boolean *result_out)
{
    switch (predicate->comparison_type)
    {
        case DDS_SQL_TYPE_INT64:
            switch (predicate->comparison_operator)
            {
                case DDS_SQL_COMPARISON_EQUAL:
                    *result_out = (a->int64_value == b->int64_value);
                    break;
                case DDS_SQL_COMPARISON_GREATER:
                    *result_out = (a->int64_value > b->int64_value);
                    break;
                case DDS_SQL_COMPARISON_GREATER_EQUAL:
                    *result_out = (a->int64_value >= b->int64_value);
                    break;
                case DDS_SQL_COMPARISON_LESS:
                    *result_out = (a->int64_value < b->int64_value);
                    break;
                case DDS_SQL_COMPARISON_LESS_EQUAL:
                    *result_out = (a->int64_value <= b->int64_value);
                    break;
                case DDS_SQL_COMPARISON_NOT_EQUAL:
                    *result_out = (a->int64_value != b->int64_value);
                    break;
                case DDS_SQL_COMPARISON_BETWEEN:
                    *result_out = (a->int64_value >= b->int64_value)
                                && (a->int64_value <= predicate->upper_range.immediate.int64_value);
                    break;
                case DDS_SQL_COMPARISON_NOT_BETWEEN:
                    *result_out = (a->int64_value < b->int64_value)
                                || (a->int64_value > predicate->upper_range.immediate.int64_value);
                    break;
                default:
                    return DDS_BOOLEAN_FALSE;
            }
            break;
        case DDS_SQL_TYPE_UINT64:
            if (predicate->flags & (DDS_SQL_FLAG_A_SIGNED_SOURCE | DDS_SQL_FLAG_B_SIGNED_SOURCE))
            {
                RTI_INT32 cmp = DDS_SqlFilter_compare_uint64_signaware(
                        a->uint64_value,
                        b->uint64_value,
                        predicate->flags);
                switch (predicate->comparison_operator)
                {
                    case DDS_SQL_COMPARISON_EQUAL:
                        *result_out = (cmp == 0);
                        break;
                    case DDS_SQL_COMPARISON_GREATER:
                        *result_out = (cmp > 0);
                        break;
                    case DDS_SQL_COMPARISON_GREATER_EQUAL:
                        *result_out = (cmp >= 0);
                        break;
                    case DDS_SQL_COMPARISON_LESS:
                        *result_out = (cmp < 0);
                        break;
                    case DDS_SQL_COMPARISON_LESS_EQUAL:
                        *result_out = (cmp <= 0);
                        break;
                    case DDS_SQL_COMPARISON_NOT_EQUAL:
                        *result_out = (cmp != 0);
                        break;
                    case DDS_SQL_COMPARISON_BETWEEN:
                    case DDS_SQL_COMPARISON_NOT_BETWEEN:
                    {
                        RTI_INT32 cmp_upper = DDS_SqlFilter_compare_uint64_signaware(
                                a->uint64_value,
                                predicate->upper_range.immediate.uint64_value,
                                predicate->flags);
                        if (predicate->comparison_operator == DDS_SQL_COMPARISON_BETWEEN)
                        {
                            *result_out = ((cmp >= 0) && (cmp_upper <= 0));
                        }
                        else
                        {
                            *result_out = ((cmp < 0) || (cmp_upper > 0));
                        }
                        break;
                    }
                    default:
                        return DDS_BOOLEAN_FALSE;
                }
                break;
            }
            switch (predicate->comparison_operator)
            {
                case DDS_SQL_COMPARISON_EQUAL:
                    *result_out = (a->uint64_value == b->uint64_value);
                    break;
                case DDS_SQL_COMPARISON_GREATER:
                    *result_out = (a->uint64_value > b->uint64_value);
                    break;
                case DDS_SQL_COMPARISON_GREATER_EQUAL:
                    *result_out = (a->uint64_value >= b->uint64_value);
                    break;
                case DDS_SQL_COMPARISON_LESS:
                    *result_out = (a->uint64_value < b->uint64_value);
                    break;
                case DDS_SQL_COMPARISON_LESS_EQUAL:
                    *result_out = (a->uint64_value <= b->uint64_value);
                    break;
                case DDS_SQL_COMPARISON_NOT_EQUAL:
                    *result_out = (a->uint64_value != b->uint64_value);
                    break;
                case DDS_SQL_COMPARISON_BETWEEN:
                    *result_out = (a->uint64_value >= b->uint64_value)
                                && (a->uint64_value <= predicate->upper_range.immediate.uint64_value);
                    break;
                case DDS_SQL_COMPARISON_NOT_BETWEEN:
                    *result_out = (a->uint64_value < b->uint64_value)
                                || (a->uint64_value > predicate->upper_range.immediate.uint64_value);
                    break;
                default:
                    return DDS_BOOLEAN_FALSE;
            }
            break;
        case DDS_SQL_TYPE_FLOAT:
            switch (predicate->comparison_operator)
            {
                case DDS_SQL_COMPARISON_EQUAL:
                    *result_out = (a->float_value == b->float_value);
                    break;
                case DDS_SQL_COMPARISON_GREATER:
                    *result_out = (a->float_value > b->float_value);
                    break;
                case DDS_SQL_COMPARISON_GREATER_EQUAL:
                    *result_out = (a->float_value >= b->float_value);
                    break;
                case DDS_SQL_COMPARISON_LESS:
                    *result_out = (a->float_value < b->float_value);
                    break;
                case DDS_SQL_COMPARISON_LESS_EQUAL:
                    *result_out = (a->float_value <= b->float_value);
                    break;
                case DDS_SQL_COMPARISON_NOT_EQUAL:
                    *result_out = (a->float_value != b->float_value);
                    break;
                case DDS_SQL_COMPARISON_BETWEEN:
                    *result_out = (a->float_value >= b->float_value)
                                && (a->float_value <= predicate->upper_range.immediate.float_value);
                    break;
                case DDS_SQL_COMPARISON_NOT_BETWEEN:
                    *result_out = (a->float_value < b->float_value)
                                || (a->float_value > predicate->upper_range.immediate.float_value);
                    break;
                default:
                    return DDS_BOOLEAN_FALSE;
            }
            break;
        case DDS_SQL_TYPE_DOUBLE:
            switch (predicate->comparison_operator)
            {
                case DDS_SQL_COMPARISON_EQUAL:
                    *result_out = (a->double_value == b->double_value);
                    break;
                case DDS_SQL_COMPARISON_GREATER:
                    *result_out = (a->double_value > b->double_value);
                    break;
                case DDS_SQL_COMPARISON_GREATER_EQUAL:
                    *result_out = (a->double_value >= b->double_value);
                    break;
                case DDS_SQL_COMPARISON_LESS:
                    *result_out = (a->double_value < b->double_value);
                    break;
                case DDS_SQL_COMPARISON_LESS_EQUAL:
                    *result_out = (a->double_value <= b->double_value);
                    break;
                case DDS_SQL_COMPARISON_NOT_EQUAL:
                    *result_out = (a->double_value != b->double_value);
                    break;
                case DDS_SQL_COMPARISON_BETWEEN:
                    *result_out = (a->double_value >= b->double_value)
                                && (a->double_value <= predicate->upper_range.immediate.double_value);
                    break;
                case DDS_SQL_COMPARISON_NOT_BETWEEN:
                    *result_out = (a->double_value < b->double_value)
                                || (a->double_value > predicate->upper_range.immediate.double_value);
                    break;
                default:
                    return DDS_BOOLEAN_FALSE;
            }
            break;
        default:
            return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}
