/*
 * FILE: SqlFilterParser.c - DDS SQL Content Filter parser implementation
 *
 * Copyright (c) 2025-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#include "SqlFilterParser.h"

/*** SOURCE_BEGIN ***/

/* Forward declarations */
#if OSAPI_ENABLE_LOG
RTI_PRIVATE RTI_SIZE_T
DDS_SqlParser_get_current_offset(const struct DDS_SqlParser *parser);
#endif /* OSAPI_ENABLE_LOG */

RTI_PRIVATE DDS_Boolean
DDS_SqlParser_peek_next_token(struct DDS_SqlParser *parser,
                              struct DDS_SqlParserToken *token);

RTI_PRIVATE DDS_Boolean
DDS_SqlParser_get_next_predicate(struct DDS_SqlParser *parser,
                                 struct DDS_SqlPredicate *predicate_out);

/* Per-call working state for DDS_SqlParser_get_next_token */
struct DDS_SqlParserTokenState
{
    RTI_SIZE_T string_index;
    RTI_SIZE_T exponent_start_index;
    DDS_UnsignedLongLong int_accumulator;
    RTI_SIZE_T digit_count;
    RTI_BOOL negative;
    RTI_BOOL int_overflowed;
    RTI_BOOL done;
    RTI_BOOL rval;
};

DDS_Boolean
DDS_SqlContentFilter_compile_filter_expression(
        struct DDS_SqlContentFilter *filter_instance,
        struct DDS_SqlCompiledContentFilter *compiled_filter_out,
        const char *expression,
        const struct DDS_StringSeq *parameters,
        const struct DDS_TypeCode *type_code)
{
    struct DDS_SqlParser parser;
    struct DDS_SqlParserToken token;
    DDS_Boolean prev_and = DDS_BOOLEAN_TRUE;
    struct DDS_SqlPredicate *predicate;
    DDS_UnsignedShort i, max_predicates, next_or;

    if (DDS_TypeCode_is_flat_data_language_binding(type_code, NULL)
            || (RTIXCdrTypeCode_getKind(&type_code->_data) != DDS_TK_STRUCT)
            || (type_code->_data._members == NULL)
            || (type_code->_data._sampleAccessInfo == NULL)
            || (type_code->_data._sampleAccessInfo->memberAccessInfos == NULL))
    {
        DDS_FILTER_LOG_SQL_UNSUPPORTED_TYPE_CODE(OSAPI_LOGKIND_WARNING, type_code->_data._name)
        return DDS_BOOLEAN_FALSE;
    }

    max_predicates = (DDS_UnsignedShort)
            filter_instance->property.predicate_max_count_per_expression;

    compiled_filter_out->predicate_count = 0;

    DDS_SqlParser_init(&parser, type_code, parameters, expression);

    for (i = 0; i < max_predicates; ++i)
    {
        predicate = &compiled_filter_out->predicates[i];

        if (!DDS_SqlParser_get_next_predicate(&parser, predicate))
        {
            return DDS_BOOLEAN_FALSE;
        }
        if (prev_and)
        {
            predicate->flags |= DDS_SQL_FLAG_AND;
        }

        compiled_filter_out->predicate_count++;

        if (!DDS_SqlParser_get_next_token(&parser, &token))
        {
            DDS_FILTER_LOG_SQL_INVALID_TOKEN(OSAPI_LOGKIND_WARNING,
                                             DDS_SqlParser_get_current_offset(&parser))
            return DDS_BOOLEAN_FALSE;
        }

        if (token.kind == DDS_SQL_TOKEN_EOF)
        {
            break;
        }
        else if (token.kind == DDS_SQL_TOKEN_AND)
        {
            prev_and = DDS_BOOLEAN_TRUE;
        }
        else if (token.kind == DDS_SQL_TOKEN_OR)
        {
            prev_and = DDS_BOOLEAN_FALSE;
        }
        else
        {
            DDS_FILTER_LOG_SQL_UNEXPECTED_TOKEN(OSAPI_LOGKIND_WARNING,
                                                DDS_SqlParser_get_current_offset(&parser))
            return DDS_BOOLEAN_FALSE;
        }
    }

    if (i == max_predicates)
    {
        DDS_FILTER_LOG_SQL_MAX_PREDICATES_EXCEEDED(OSAPI_LOGKIND_WARNING)
        return DDS_BOOLEAN_FALSE;
    }

    /* Fill next_or_index for AND short-circuit. */
    next_or = compiled_filter_out->predicate_count;
    for (i = compiled_filter_out->predicate_count; i > 0; --i)
    {
        compiled_filter_out->predicates[i - 1].next_or_index = next_or;
        if (!(compiled_filter_out->predicates[i - 1].flags & DDS_SQL_FLAG_AND))
        {
            next_or = (DDS_UnsignedShort)(i - 1);
        }
    }

    return DDS_BOOLEAN_TRUE;
}

/* Unwrap a chain of typedef aliases. Returns NULL if the chain hits a
 * pointer-laid-out alias, since the executor uses flat byte offsets and
 * cannot follow pointer indirection.
 */
RTI_PRIVATE const struct RTIXCdrTypeCode *
DDS_SqlParser_unwrap_alias(const struct RTIXCdrTypeCode *tc)
{
    while ((tc != NULL) && (RTIXCdrTypeCode_getKind(tc) == DDS_TK_ALIAS))
    {
        if (tc->_isPointer)
        {
            return NULL;
        }
        tc = tc->_typeCode;
    }
    return tc;
}

RTI_PRIVATE DDS_Boolean
DDS_SqlParser_resolve_member_path(const struct RTIXCdrTypeCode *tc_data,
                                  const char *dotted_name,
                                  struct RTIXCdrTypeCodeMember **member_out,
                                  DDS_UnsignedLong *offset_out)
{
    const struct RTIXCdrTypeCode *current_tc = tc_data;
    const struct RTIXCdrTypeCode *next_tc;
    const char *seg_start = dotted_name;
    const char *p;
    DDS_UnsignedLong accumulated_offset = 0;
    DDS_UnsignedLong i;
    struct RTIXCdrTypeCodeMember *member;
    RTI_SIZE_T seg_len;

    while (RTI_TRUE)
    {
        p = seg_start;
        while ((*p != '.') && (*p != '\0'))
        {
            ++p;
        }
        seg_len = (RTI_SIZE_T)(p - seg_start);

        member = NULL;
        for (i = 0; i < current_tc->_memberCount; ++i)
        {
            if ((current_tc->_members[i]._name != NULL)
                    && (OSAPI_String_length(current_tc->_members[i]._name) == seg_len)
                    && (OSAPI_Memory_compare(current_tc->_members[i]._name,
                                             seg_start,
                                             seg_len) == 0))
            {
                member = &current_tc->_members[i];
                accumulated_offset += current_tc->_sampleAccessInfo
                        ->memberAccessInfos[i].bindingMemberValueOffset[0];
                break;
            }
        }

        if (member == NULL)
        {
            return DDS_BOOLEAN_FALSE;
        }

        /* Optional or pointer-indirected members break flat-offset
         * accumulation used by the executor.
         */
        if (RTIXCdrTypeCodeMember_isOptional(member)
                || member->_representation._isPointer)
        {
            return DDS_BOOLEAN_FALSE;
        }

        if (*p == '\0')
        {
            break;
        }

        /* Intermediate segment must be a struct; unwrap aliases first. */
        next_tc = DDS_SqlParser_unwrap_alias(member->_representation._typeCode);
        if ((next_tc == NULL)
                || (RTIXCdrTypeCode_getKind(next_tc) != DDS_TK_STRUCT)
                || (next_tc->_memberCount == 0)
                || (next_tc->_members == NULL)
                || (next_tc->_sampleAccessInfo == NULL)
                || (next_tc->_sampleAccessInfo->memberAccessInfos == NULL))
        {
            return DDS_BOOLEAN_FALSE;
        }

        current_tc = next_tc;
        seg_start = p + 1;
    }

    *member_out = member;
    *offset_out = accumulated_offset;
    return DDS_BOOLEAN_TRUE;
}

RTI_PRIVATE DDS_Boolean
DDS_SqlParser_find_field_by_name(const struct DDS_SqlParser *parser,
                                 const char *field_name,
                                 struct DDS_SqlFieldValue *field_out)
{
    struct RTIXCdrTypeCodeMember *member = NULL;
    const struct RTIXCdrTypeCode *leaf_tc;
    DDS_UnsignedLong offset = 0;

    if (!DDS_SqlParser_resolve_member_path(&parser->type_code->_data,
                                           field_name,
                                           &member,
                                           &offset))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (member->_representation._typeCode == NULL)
    {
        return DDS_BOOLEAN_FALSE;
    }

    /* Resolve typedef aliases on the leaf to its underlying primitive. */
    leaf_tc = DDS_SqlParser_unwrap_alias(member->_representation._typeCode);
    if (leaf_tc == NULL)
    {
        return DDS_BOOLEAN_FALSE;
    }

    field_out->type = (DDS_Long)RTIXCdrTypeCode_getKind(leaf_tc);
    field_out->offset = offset;

    return DDS_BOOLEAN_TRUE;
}

RTI_PRIVATE DDS_Boolean
DDS_SqlParser_find_enum_by_name(const struct DDS_SqlParser *parser,
                                const char *enum_field_name,
                                const char *enum_label_name,
                                DDS_LongLong *ordinal_out)
{
    struct RTIXCdrTypeCodeMember *member = NULL;
    const struct RTIXCdrTypeCode *enum_tc;
    DDS_UnsignedLong offset = 0;
    DDS_UnsignedLong enum_label_count;
    DDS_UnsignedLong i;
    struct RTIXCdrTypeCodeMember *enum_label;

    if (!DDS_SqlParser_resolve_member_path(&parser->type_code->_data,
                                           enum_field_name,
                                           &member,
                                           &offset))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (member->_representation._typeCode == NULL)
    {
        return DDS_BOOLEAN_FALSE;
    }

    /* Resolve typedef aliases on the leaf to its underlying enum. */
    enum_tc = DDS_SqlParser_unwrap_alias(member->_representation._typeCode);
    if ((enum_tc == NULL)
            || (RTIXCdrTypeCode_getKind(enum_tc) != DDS_TK_ENUM))
    {
        return DDS_BOOLEAN_FALSE;
    }

    enum_label_count = enum_tc->_memberCount;
    for (i = 0; i < enum_label_count; ++i)
    {
        enum_label = &enum_tc->_members[i];
        if ((enum_label->_name != NULL)
                && (OSAPI_String_cmp(enum_label->_name, enum_label_name) == 0))
        {
            *ordinal_out = (DDS_LongLong)enum_label->_ordinal;
            return DDS_BOOLEAN_TRUE;
        }
    }

    return DDS_BOOLEAN_FALSE;
}

RTI_PRIVATE DDS_Boolean
DDS_SqlFieldValue_get_operand_type(const struct DDS_SqlFieldValue *field,
                                   enum DDS_SqlOperandType *operand_type_out)
{
    switch (field->type)
    {
        case DDS_TK_OCTET:
        case DDS_TK_SHORT:
        case DDS_TK_USHORT:
        case DDS_TK_LONG:
            *operand_type_out = DDS_SQL_KIND_INT32;
            return DDS_BOOLEAN_TRUE;
        case DDS_TK_ULONG:
            *operand_type_out = DDS_SQL_KIND_UINT32;
            return DDS_BOOLEAN_TRUE;
        case DDS_TK_LONGLONG:
            *operand_type_out = DDS_SQL_KIND_INT64;
            return DDS_BOOLEAN_TRUE;
        case DDS_TK_ULONGLONG:
            *operand_type_out = DDS_SQL_KIND_UINT64;
            return DDS_BOOLEAN_TRUE;
        case DDS_TK_FLOAT:
            *operand_type_out = DDS_SQL_KIND_FLOAT;
            return DDS_BOOLEAN_TRUE;
        case DDS_TK_DOUBLE:
            *operand_type_out = DDS_SQL_KIND_DOUBLE;
            return DDS_BOOLEAN_TRUE;
        case DDS_TK_CHAR:
            *operand_type_out = DDS_SQL_KIND_CHAR;
            return DDS_BOOLEAN_TRUE;
        case DDS_TK_ENUM:
            *operand_type_out = DDS_SQL_KIND_ENUM;
            return DDS_BOOLEAN_TRUE;
        case DDS_TK_BOOLEAN:
            *operand_type_out = DDS_SQL_KIND_BOOLEAN;
            return DDS_BOOLEAN_TRUE;
        default:
            return DDS_BOOLEAN_FALSE;
    }
}

RTI_PRIVATE DDS_Boolean
DDS_SqlParser_get_parameter(const struct DDS_SqlParser *parser,
                            RTI_INT32 parameter_index,
                            struct DDS_SqlParserToken *token_out)
{
    struct DDS_SqlParser tmp_parser;
    DDS_String *parameter;
    struct DDS_SqlParserToken trailing_token;

    if ((parser->parameters == NULL)
        || (parameter_index >= DDS_StringSeq_get_length(parser->parameters)))
    {
        return DDS_BOOLEAN_FALSE;
    }

    parameter = DDS_StringSeq_get_reference(parser->parameters, parameter_index);
    if ((parameter == NULL) || (*parameter == NULL))
    {
        return DDS_BOOLEAN_FALSE;
    }

    DDS_SqlParser_init(&tmp_parser, parser->type_code, NULL, *parameter);
    if (!DDS_SqlParser_get_next_token(&tmp_parser, token_out))
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (!DDS_SqlParserTokenKind_is_immediate(token_out->kind))
    {
        return DDS_BOOLEAN_FALSE;
    }

    /* Reject trailing tokens so "123 garbage" does not parse as 123. */
    if (!DDS_SqlParser_get_next_token(&tmp_parser, &trailing_token)
            || (trailing_token.kind != DDS_SQL_TOKEN_EOF))
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

RTI_PRIVATE DDS_Boolean
DDS_SqlParser_get_operand(struct DDS_SqlParser *parser,
                          struct DDS_SqlParserToken *operand_out,
                          struct DDS_SqlFieldValue *field_out)
{
    if (!DDS_SqlParser_get_next_token(parser, operand_out))
    {
        DDS_FILTER_LOG_SQL_INVALID_TOKEN(OSAPI_LOGKIND_WARNING,
                                         DDS_SqlParser_get_current_offset(parser))
        return DDS_BOOLEAN_FALSE;
    }

    if ((field_out != NULL) && (operand_out->kind == DDS_SQL_TOKEN_KEYWORD))
    {
        if (!DDS_SqlParser_find_field_by_name(parser,
                                              operand_out->value.str_value,
                                              field_out))
        {
            DDS_FILTER_LOG_SQL_FIELD_NOT_FOUND(OSAPI_LOGKIND_WARNING,
                                               parser->type_code->_data._name,
                                               operand_out->value.str_value)
            return DDS_BOOLEAN_FALSE;
        }

        return DDS_BOOLEAN_TRUE;
    }

    if (operand_out->kind == DDS_SQL_TOKEN_PARAMETER)
    {
        /* Replace operand with the value of parameter */
        if (!DDS_SqlParser_get_parameter(parser,
                                         (RTI_INT32)operand_out->value.int_value,
                                         operand_out))
        {
            DDS_FILTER_LOG_SQL_PARSE_PARAMETER(OSAPI_LOGKIND_WARNING,
                                               (RTI_INT32)operand_out->value.int_value)
            return DDS_BOOLEAN_FALSE;
        }
    }

    /* Verify that the operand is an immediate value */
    if (!DDS_SqlParserTokenKind_is_immediate(operand_out->kind))
    {
        DDS_FILTER_LOG_SQL_UNEXPECTED_TOKEN(OSAPI_LOGKIND_WARNING,
                                             DDS_SqlParser_get_current_offset(parser))
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

RTI_PRIVATE DDS_Boolean
DDS_SqlParser_get_comparison_operator(struct DDS_SqlParser *parser,
                                      DDS_Octet *comparison_operator_out)
{
    struct DDS_SqlParserToken token;

    if (!DDS_SqlParser_get_next_token(parser, &token))
    {
        DDS_FILTER_LOG_SQL_INVALID_TOKEN(OSAPI_LOGKIND_WARNING,
                                         DDS_SqlParser_get_current_offset(parser))
        return DDS_BOOLEAN_FALSE;
    }

    switch (token.kind)
    {
        case DDS_SQL_TOKEN_EQUAL:
            *comparison_operator_out = DDS_SQL_COMPARISON_EQUAL;
            break;
        case DDS_SQL_TOKEN_GT:
            *comparison_operator_out = DDS_SQL_COMPARISON_GREATER;
            break;
        case DDS_SQL_TOKEN_GE:
            *comparison_operator_out = DDS_SQL_COMPARISON_GREATER_EQUAL;
            break;
        case DDS_SQL_TOKEN_LT:
            *comparison_operator_out = DDS_SQL_COMPARISON_LESS;
            break;
        case DDS_SQL_TOKEN_LE:
            *comparison_operator_out = DDS_SQL_COMPARISON_LESS_EQUAL;
            break;
        case DDS_SQL_TOKEN_NOT_EQUAL:
            *comparison_operator_out = DDS_SQL_COMPARISON_NOT_EQUAL;
            break;
        case DDS_SQL_TOKEN_BETWEEN:
            *comparison_operator_out = DDS_SQL_COMPARISON_BETWEEN;
            break;
        case DDS_SQL_TOKEN_NOT:
            *comparison_operator_out = DDS_SQL_COMPARISON_NOT_BETWEEN;
            break;
        default:
            DDS_FILTER_LOG_SQL_UNEXPECTED_TOKEN(OSAPI_LOGKIND_WARNING,
                                                DDS_SqlParser_get_current_offset(parser))
            return DDS_BOOLEAN_FALSE;
    }

    if (*comparison_operator_out == DDS_SQL_COMPARISON_NOT_BETWEEN)
    {
        /* Next token must be BETWEEN */
        if (!DDS_SqlParser_get_next_token(parser, &token))
        {
            DDS_FILTER_LOG_SQL_INVALID_TOKEN(OSAPI_LOGKIND_WARNING,
                                             DDS_SqlParser_get_current_offset(parser))
            return DDS_BOOLEAN_FALSE;
        }
        if (token.kind != DDS_SQL_TOKEN_BETWEEN)
        {
            DDS_FILTER_LOG_SQL_UNEXPECTED_TOKEN(OSAPI_LOGKIND_WARNING,
                                                DDS_SqlParser_get_current_offset(parser))
            return DDS_BOOLEAN_FALSE;
        }
    }

    return DDS_BOOLEAN_TRUE;
}

RTI_PRIVATE DDS_Boolean
DDS_SqlParser_get_comparison_type(enum DDS_SqlOperandType variable_type,
                                  enum DDS_SqlOperandType immediate_type,
                                  union DDS_SqlImmediateValue *immediate_value_out,
                                  enum DDS_SqlImmediateType *comparison_type_out)
{
    switch (variable_type)
    {
        case DDS_SQL_KIND_CHAR:
            switch (immediate_type)
            {
                case DDS_SQL_KIND_CHAR:
                    *comparison_type_out = DDS_SQL_TYPE_INT64;
                    break;
                default:
                    return DDS_BOOLEAN_FALSE;
            }
            break;
        case DDS_SQL_KIND_BOOLEAN:
            switch (immediate_type)
            {
                case DDS_SQL_KIND_BOOLEAN:
                    *comparison_type_out = DDS_SQL_TYPE_INT64;
                    break;
                default:
                    return DDS_BOOLEAN_FALSE;
            }
            break;
        case DDS_SQL_KIND_INT32:
        case DDS_SQL_KIND_UINT32:
            switch (immediate_type)
            {
                case DDS_SQL_KIND_INT32:
                case DDS_SQL_KIND_INT64:
                case DDS_SQL_KIND_UINT32:
                case DDS_SQL_KIND_ENUM:
                    *comparison_type_out = DDS_SQL_TYPE_INT64;
                    break;
                case DDS_SQL_KIND_UINT64:
                    *comparison_type_out = DDS_SQL_TYPE_UINT64;
                    break;
                case DDS_SQL_KIND_FLOAT:
                    *comparison_type_out = DDS_SQL_TYPE_FLOAT;
                    break;
                case DDS_SQL_KIND_DOUBLE:
                    *comparison_type_out = DDS_SQL_TYPE_DOUBLE;
                    break;
                default:
                    return DDS_BOOLEAN_FALSE;
            }
            break;
        case DDS_SQL_KIND_INT64:
            switch (immediate_type)
            {
                case DDS_SQL_KIND_INT32:
                case DDS_SQL_KIND_INT64:
                case DDS_SQL_KIND_UINT32:
                case DDS_SQL_KIND_ENUM:
                    *comparison_type_out = DDS_SQL_TYPE_INT64;
                    break;
                case DDS_SQL_KIND_UINT64:
                    *comparison_type_out = DDS_SQL_TYPE_UINT64;
                    break;
                case DDS_SQL_KIND_FLOAT:
                    *comparison_type_out = DDS_SQL_TYPE_DOUBLE;
                    if (immediate_value_out != NULL)
                    {
                        DDS_Float temp_float = immediate_value_out->float_value;
                        immediate_value_out->double_value = (DDS_Double)temp_float;
                    }
                    break;
                case DDS_SQL_KIND_DOUBLE:
                    *comparison_type_out = DDS_SQL_TYPE_DOUBLE;
                    break;
                default:
                    return DDS_BOOLEAN_FALSE;
            }
            break;
        case DDS_SQL_KIND_UINT64:
            switch (immediate_type)
            {
                case DDS_SQL_KIND_INT32:
                case DDS_SQL_KIND_INT64:
                case DDS_SQL_KIND_ENUM:
                    *comparison_type_out = DDS_SQL_TYPE_UINT64;
                    if (immediate_value_out != NULL)
                    {
                        DDS_LongLong temp_int64 = immediate_value_out->int64_value;
                        immediate_value_out->uint64_value =
                                (DDS_UnsignedLongLong)temp_int64;
                    }
                    break;
                case DDS_SQL_KIND_FLOAT:
                    *comparison_type_out = DDS_SQL_TYPE_DOUBLE;
                    if (immediate_value_out != NULL)
                    {
                        DDS_Float temp_float = immediate_value_out->float_value;
                        immediate_value_out->double_value = (DDS_Double)temp_float;
                    }
                    break;
                case DDS_SQL_KIND_DOUBLE:
                    *comparison_type_out = DDS_SQL_TYPE_DOUBLE;
                    break;
                case DDS_SQL_KIND_UINT32:
                case DDS_SQL_KIND_UINT64:
                    *comparison_type_out = DDS_SQL_TYPE_UINT64;
                    break;
                default:
                    return DDS_BOOLEAN_FALSE;
            }
            break;
        case DDS_SQL_KIND_FLOAT:
            switch (immediate_type)
            {
                case DDS_SQL_KIND_INT32:
                    *comparison_type_out = DDS_SQL_TYPE_FLOAT;
                    if (immediate_value_out != NULL)
                    {
                        DDS_LongLong temp_int64 = immediate_value_out->int64_value;
                        immediate_value_out->float_value = (DDS_Float)temp_int64;
                    }
                    break;
                case DDS_SQL_KIND_INT64:
                    *comparison_type_out = DDS_SQL_TYPE_DOUBLE;
                    if (immediate_value_out != NULL)
                    {
                        DDS_LongLong temp_int64 = immediate_value_out->int64_value;
                        immediate_value_out->double_value = (DDS_Double)temp_int64;
                    }
                    break;
                case DDS_SQL_KIND_FLOAT:
                case DDS_SQL_KIND_UINT32:
                    *comparison_type_out = DDS_SQL_TYPE_FLOAT;
                    break;
                case DDS_SQL_KIND_UINT64:
                    *comparison_type_out = DDS_SQL_TYPE_DOUBLE;
                    if (immediate_value_out != NULL)
                    {
                        DDS_UnsignedLongLong temp_uint64 = immediate_value_out->uint64_value;
                        immediate_value_out->double_value = (DDS_Double)temp_uint64;
                    }
                    break;
                case DDS_SQL_KIND_DOUBLE:
                    *comparison_type_out = DDS_SQL_TYPE_DOUBLE;
                    break;
                default:
                    return DDS_BOOLEAN_FALSE;
            }
            break;
        case DDS_SQL_KIND_DOUBLE:
            switch (immediate_type)
            {
                case DDS_SQL_KIND_INT32:
                case DDS_SQL_KIND_INT64:
                    *comparison_type_out = DDS_SQL_TYPE_DOUBLE;
                    if (immediate_value_out != NULL)
                    {
                        DDS_LongLong temp_int64 = immediate_value_out->int64_value;
                        immediate_value_out->double_value = (DDS_Double)temp_int64;
                    }
                    break;
                case DDS_SQL_KIND_FLOAT:
                    *comparison_type_out = DDS_SQL_TYPE_DOUBLE;
                    if (immediate_value_out != NULL)
                    {
                        DDS_Float temp_float = immediate_value_out->float_value;
                        immediate_value_out->double_value = (DDS_Double)temp_float;
                    }
                    break;
                case DDS_SQL_KIND_DOUBLE:
                case DDS_SQL_KIND_UINT32:
                case DDS_SQL_KIND_UINT64:
                    *comparison_type_out = DDS_SQL_TYPE_DOUBLE;
                    break;
                default:
                    return DDS_BOOLEAN_FALSE;
            }
            break;
        case DDS_SQL_KIND_ENUM:
            switch (immediate_type)
            {
                case DDS_SQL_KIND_INT32:
                case DDS_SQL_KIND_INT64:
                case DDS_SQL_KIND_UINT32:
                case DDS_SQL_KIND_ENUM:
                    *comparison_type_out = DDS_SQL_TYPE_INT64;
                    break;
                case DDS_SQL_KIND_UINT64:
                    *comparison_type_out = DDS_SQL_TYPE_UINT64;
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

RTI_PRIVATE DDS_Boolean
DDS_SqlParser_get_upper_range_token(struct DDS_SqlParser *parser,
                                    struct DDS_SqlParserToken *lower_range,
                                    struct DDS_SqlParserToken *upper_range_out)
{
    struct DDS_SqlParserToken token;

    /* In a range, the two values must both be immediate values */
    if (!DDS_SqlParserTokenKind_is_immediate(lower_range->kind))
    {
        DDS_FILTER_LOG_SQL_UNEXPECTED_TOKEN(OSAPI_LOGKIND_WARNING,
                                            DDS_SqlParser_get_current_offset(parser))
        return DDS_BOOLEAN_FALSE;
    }

    /* Next token must be AND */
    if (!DDS_SqlParser_get_next_token(parser, &token))
    {
        DDS_FILTER_LOG_SQL_INVALID_TOKEN(OSAPI_LOGKIND_WARNING,
                                            DDS_SqlParser_get_current_offset(parser))
        return DDS_BOOLEAN_FALSE;
    }
    if (token.kind != DDS_SQL_TOKEN_AND)
    {
        DDS_FILTER_LOG_SQL_UNEXPECTED_TOKEN(OSAPI_LOGKIND_WARNING,
                                            DDS_SqlParser_get_current_offset(parser))
        return DDS_BOOLEAN_FALSE;
    }

    /* Get the value. This is necessary if the upper range is a parameter */
    if (!DDS_SqlParser_get_operand(parser, upper_range_out, NULL))
    {
        return DDS_BOOLEAN_FALSE;
    }

    /* The upper range must be the same type as the right operand which has
     * already been asserted to be an immediate value.
     */
    if (upper_range_out->kind != lower_range->kind)
    {
        DDS_FILTER_LOG_SQL_UNEXPECTED_TOKEN(OSAPI_LOGKIND_WARNING,
                                            DDS_SqlParser_get_current_offset(parser))
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Get an immediate value from a token.
 *
 * \details
 * For most tokens they can be directly be converted into an immediate value.
 * However, specific types like strings can be different types. In that case,
 * the type of the immediate is determined by the type of the variable which
 * it is being compared to.
 *
 * \param[in] parser The SQL parser instance
 * \param[in] immediate The token to get the immediate value from
 * \param[in] variable_type The type of the variable that the immediate value
 *                          is being compared to.
 * \param[out] type_out The type of the immediate value
 * \param[out] value_out The immediate value to fill in
 *
 * \return DDS_BOOLEAN_TRUE if the immediate value was successfully
 *         retrieved, DDS_BOOLEAN_FALSE otherwise.
 */
RTI_PRIVATE DDS_Boolean
DDS_SqlParser_get_immediate_value(struct DDS_SqlParser *parser,
                                  struct DDS_SqlParserToken *token,
                                  enum DDS_SqlOperandType variable_type,
                                  const struct DDS_SqlParserToken *variable,
                                  enum DDS_SqlOperandType *type_out,
                                  union DDS_SqlImmediateValue *value_out)
{
    switch (token->kind)
    {
        case DDS_SQL_TOKEN_INT32:
            *type_out = DDS_SQL_KIND_INT32;
            value_out->int64_value = token->value.int_value;
            break;
        case DDS_SQL_TOKEN_INT64:
            *type_out = DDS_SQL_KIND_INT64;
            value_out->int64_value = token->value.int_value;
            break;
        case DDS_SQL_TOKEN_FLOAT:
            *type_out = DDS_SQL_KIND_FLOAT;
            value_out->float_value = token->value.float_value;
            break;
        case DDS_SQL_TOKEN_DOUBLE:
            *type_out = DDS_SQL_KIND_DOUBLE;
            value_out->double_value = token->value.double_value;
            break;
        case DDS_SQL_TOKEN_BOOLEAN:
            *type_out = DDS_SQL_KIND_BOOLEAN;
            value_out->int64_value = token->value.int_value;
            break;
        case DDS_SQL_TOKEN_STRING:
            /* Must either be a char or an enum depending on the variable */
            if (variable_type == DDS_SQL_KIND_CHAR)
            {
                if (OSAPI_String_length(token->value.str_value) != 1)
                {
                    DDS_FILTER_LOG_SQL_UNEXPECTED_TOKEN(OSAPI_LOGKIND_WARNING,
                                                        DDS_SqlParser_get_current_offset(parser))
                    return DDS_BOOLEAN_FALSE;
                }
                *type_out = DDS_SQL_KIND_CHAR;
                value_out->int64_value = (DDS_LongLong)(unsigned char)token->value.str_value[0];
            }
            else if (variable_type == DDS_SQL_KIND_ENUM)
            {
                if (!DDS_SqlParser_find_enum_by_name(parser,
                                                    variable->value.str_value,
                                                    token->value.str_value,
                                                    &value_out->int64_value))
                {
                    DDS_FILTER_LOG_SQL_ENUM_LABEL_NOT_FOUND(OSAPI_LOGKIND_WARNING,
                                                            token->value.str_value)
                    return DDS_BOOLEAN_FALSE;
                }
                *type_out = DDS_SQL_KIND_ENUM;
            }
            else
            {
                DDS_FILTER_LOG_SQL_UNEXPECTED_TOKEN(OSAPI_LOGKIND_WARNING,
                                                    DDS_SqlParser_get_current_offset(parser))
                return DDS_BOOLEAN_FALSE;
            }
            break;
        default:
            DDS_FILTER_LOG_SQL_UNEXPECTED_TOKEN(OSAPI_LOGKIND_WARNING,
                                                DDS_SqlParser_get_current_offset(parser))
            return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

RTI_PRIVATE DDS_Boolean
DDS_SqlParser_get_next_predicate(struct DDS_SqlParser *parser,
                                 struct DDS_SqlPredicate *predicate_out)
{
    struct DDS_SqlParserToken left;
    struct DDS_SqlParserToken right;
    struct DDS_SqlParserToken upper_range;

    struct DDS_SqlParserToken *variable = NULL;
    enum DDS_SqlOperandType variable_type;
    struct DDS_SqlParserToken *immediate = NULL;
    enum DDS_SqlOperandType immediate_type;
    union DDS_SqlImmediateValue *immediate_value = NULL;
    union DDS_SqlImmediateValue *upper_range_value = NULL;

    predicate_out->flags = 0;

    /* Check if predicate has a NOT prefix */
    if (!DDS_SqlParser_peek_next_token(parser, &left))
    {
        DDS_FILTER_LOG_SQL_INVALID_TOKEN(OSAPI_LOGKIND_WARNING,
                                         DDS_SqlParser_get_current_offset(parser))
        return DDS_BOOLEAN_FALSE;
    }
    if (left.kind == DDS_SQL_TOKEN_NOT)
    {
        predicate_out->flags |= DDS_SQL_FLAG_NOT;

        /* Consume the NOT token */
        if (!DDS_SqlParser_get_next_token(parser, &left))
        {
            return DDS_BOOLEAN_FALSE;
        }
    }

    /* Get left-side of the predicate */
    if (!DDS_SqlParser_get_operand(parser,
                                   &left,
                                   &predicate_out->left.field))
    {
        return DDS_BOOLEAN_FALSE;
    }

    /* Get comparison operator */
    if (!DDS_SqlParser_get_comparison_operator(parser,
                                               &predicate_out->comparison_operator))
    {
        return DDS_BOOLEAN_FALSE;
    }

    /* Get right-side of the predicate */
    if (!DDS_SqlParser_get_operand(parser,
                                   &right,
                                   &predicate_out->right.field))
    {
        return DDS_BOOLEAN_FALSE;
    }

    /* If this is a BETWEEN predicate, then get the upper range */
    if ((predicate_out->comparison_operator == DDS_SQL_COMPARISON_BETWEEN)
            || (predicate_out->comparison_operator == DDS_SQL_COMPARISON_NOT_BETWEEN))
    {
        if (!DDS_SqlParser_get_upper_range_token(parser, &right, &upper_range))
        {
            return DDS_BOOLEAN_FALSE;
        }
        upper_range_value = &predicate_out->upper_range.immediate;
    }

    /* Determine which side is the variable and which is the immediate. */
    if (left.kind == DDS_SQL_TOKEN_KEYWORD)
    {
        predicate_out->flags |= DDS_SQL_FLAG_A_FIELD;

        if (!DDS_SqlFieldValue_get_operand_type(&predicate_out->left.field,
                                                &variable_type))
        {
            DDS_FILTER_LOG_SQL_UNSUPPORTED_FIELD_TYPE(OSAPI_LOGKIND_WARNING,
                                                      parser->type_code->_data._name,
                                                      left.value.str_value)
            return DDS_BOOLEAN_FALSE;
        }

        if (right.kind != DDS_SQL_TOKEN_KEYWORD)
        {
            variable = &left;
            immediate = &right;
            immediate_value = &predicate_out->right.immediate;
        }
        else
        {
            predicate_out->flags |= DDS_SQL_FLAG_B_FIELD;

            /* Both sides are variables. The type of the right side is put
             * into immediate_type only for the purpose of determining the
             * comparison type.
             */
            if (!DDS_SqlFieldValue_get_operand_type(&predicate_out->right.field,
                                                    &immediate_type))
            {
                DDS_FILTER_LOG_SQL_UNSUPPORTED_FIELD_TYPE(OSAPI_LOGKIND_WARNING,
                                                          parser->type_code->_data._name,
                                                          right.value.str_value)
                return DDS_BOOLEAN_FALSE;
            }
        }
    }
    else
    {
        if (right.kind == DDS_SQL_TOKEN_KEYWORD)
        {
            predicate_out->flags |= DDS_SQL_FLAG_B_FIELD;

            if (!DDS_SqlFieldValue_get_operand_type(&predicate_out->right.field,
                                                    &variable_type))
            {
                DDS_FILTER_LOG_SQL_UNSUPPORTED_FIELD_TYPE(OSAPI_LOGKIND_WARNING,
                                                          parser->type_code->_data._name,
                                                          right.value.str_value)
                return DDS_BOOLEAN_FALSE;
            }

            variable = &right;
            immediate = &left;
            immediate_value = &predicate_out->left.immediate;
        }
        else
        {
            /* Both sides are immediate values which is not supported */
            DDS_FILTER_LOG_SQL_UNEXPECTED_TOKEN(OSAPI_LOGKIND_WARNING,
                                                DDS_SqlParser_get_current_offset(parser))
            return DDS_BOOLEAN_FALSE;
        }
    }

    /* Determine the type of the immediate based on the variable */
    if (immediate != NULL)
    {
        if (!DDS_SqlParser_get_immediate_value(parser,
                                               immediate,
                                               variable_type,
                                               variable,
                                               &immediate_type,
                                               immediate_value))
        {
            return DDS_BOOLEAN_FALSE;
        }
    }

    /* Determine comparison type from variable and immediate types. If the
     * immediate value is not the same type as the comparison type, then it will
     * be coerced into the comparison type.
     */
    if (!DDS_SqlParser_get_comparison_type(variable_type,
                                           immediate_type,
                                           immediate_value,
                                           &predicate_out->comparison_type))
    {
        DDS_FILTER_LOG_SQL_INCOMPATIBLE_TYPES(OSAPI_LOGKIND_WARNING,
                                              DDS_SqlParser_get_current_offset(parser))
        return DDS_BOOLEAN_FALSE;
    }

    /* Boolean comparisons only support = and <>. */
    if ((variable_type == DDS_SQL_KIND_BOOLEAN)
            && (predicate_out->comparison_operator != DDS_SQL_COMPARISON_EQUAL)
            && (predicate_out->comparison_operator != DDS_SQL_COMPARISON_NOT_EQUAL))
    {
        DDS_FILTER_LOG_SQL_INCOMPATIBLE_TYPES(OSAPI_LOGKIND_WARNING,
                                              DDS_SqlParser_get_current_offset(parser))
        return DDS_BOOLEAN_FALSE;
    }

    /* Tag signed operands for sign-aware UINT64 comparison in the executor. */
    if (predicate_out->comparison_type == DDS_SQL_TYPE_UINT64)
    {
        enum DDS_SqlOperandType a_kind, b_kind;
        if (predicate_out->flags & DDS_SQL_FLAG_A_FIELD)
        {
            a_kind = variable_type;
            b_kind = immediate_type;
        }
        else
        {
            a_kind = immediate_type;
            b_kind = variable_type;
        }
        if ((a_kind == DDS_SQL_KIND_INT32) || (a_kind == DDS_SQL_KIND_INT64)
                || (a_kind == DDS_SQL_KIND_ENUM))
        {
            predicate_out->flags |= DDS_SQL_FLAG_A_SIGNED_SOURCE;
        }
        if ((b_kind == DDS_SQL_KIND_INT32) || (b_kind == DDS_SQL_KIND_INT64)
                || (b_kind == DDS_SQL_KIND_ENUM))
        {
            predicate_out->flags |= DDS_SQL_FLAG_B_SIGNED_SOURCE;
        }
    }

    /* If there is an upper range value, then also coerce it into the comparison type */
    if (upper_range_value != NULL)
    {
        enum DDS_SqlImmediateType temp_comparison_type;
        RTI_BOOL retval;

        /* Get the immediate value of the upper range from the token */
        if (!DDS_SqlParser_get_immediate_value(parser,
                                               &upper_range,
                                               variable_type,
                                               variable,
                                               &immediate_type,
                                               upper_range_value))
        {
            return DDS_BOOLEAN_FALSE;
        }

        /* Coerce the upper range value to the comparison type. In a BETWEEN
         * predicate, both the right and the upper range must the same type of
         * token, so the variable_type and immediate_type are the same as when
         * the right value was coerced. We do not need to check the result
         * because the types are the same as when the right value was coerced.
         */
        retval = DDS_SqlParser_get_comparison_type(variable_type,
                                                   immediate_type,
                                                   upper_range_value,
                                                   &temp_comparison_type);
        IGNORE_RETVAL(retval);
    }

    return DDS_BOOLEAN_TRUE;
}

/* Case-insensitive match between input and a lowercase keyword. */
RTI_PRIVATE RTI_BOOL
DDS_SqlParser_keyword_match(const char *input,
                            RTI_SIZE_T input_len,
                            const char *keyword,
                            RTI_SIZE_T keyword_len)
{
    RTI_SIZE_T i;
    char c;

    if (input_len != keyword_len)
    {
        return RTI_FALSE;
    }

    for (i = 0; i < input_len; ++i)
    {
        c = input[i];
        if ((c >= 'A') && (c <= 'Z'))
        {
            c = (char)(c + ('a' - 'A'));
        }
        if (c != keyword[i])
        {
            return RTI_FALSE;
        }
    }

    return RTI_TRUE;
}

/* True if c can begin a keyword (alpha or underscore). */
RTI_PRIVATE RTI_BOOL
DDS_SqlParser_is_keyword_start(char c)
{
    return ((c >= 'a') && (c <= 'z'))
            || ((c >= 'A') && (c <= 'Z'))
            || (c == '_');
}

/* True if c is a decimal digit. */
RTI_PRIVATE RTI_BOOL
DDS_SqlParser_is_digit(char c)
{
    return (c >= '0') && (c <= '9');
}

/* True if c can continue a keyword (alpha, digit, or underscore). */
RTI_PRIVATE RTI_BOOL
DDS_SqlParser_is_keyword_continue(char c)
{
    return DDS_SqlParser_is_keyword_start(c) || DDS_SqlParser_is_digit(c);
}

/* Append the current parser char to the token buffer and advance the parser. */
RTI_PRIVATE RTI_BOOL
DDS_SqlParser_token_append_char(struct DDS_SqlParser *parser,
                                struct DDS_SqlParserToken *token,
                                RTI_SIZE_T *string_index)
{
    RTI_BOOL result = RTI_FALSE;

    if (*string_index < DDS_SQL_PARSER_MAX_TOKEN_LENGTH)
    {
        token->value.str_value[*string_index] = *parser->cur_ptr;
        ++(*string_index);
        token->value.str_value[*string_index] = 0;
        (parser->cur_ptr)++;
        result = RTI_TRUE;
    }
    return result;
}

/* Decode an ASCII hex digit. Returns 0..15 on success, -1 if c is not hex. */
RTI_PRIVATE RTI_INT32
DDS_SqlParser_decode_hex_digit(char c)
{
    RTI_INT32 result = -1;

    if ((c >= '0') && (c <= '9'))
    {
        result = c - '0';
    }
    else if ((c >= 'a') && (c <= 'f'))
    {
        result = (c - 'a') + 10;
    }
    else if ((c >= 'A') && (c <= 'F'))
    {
        result = (c - 'A') + 10;
    }
    return result;
}

/* Finalize an integer literal as INT64 (l/L suffix) or INT32 (default). */
RTI_PRIVATE void
DDS_SqlParser_finalize_integer_token(struct DDS_SqlParser *parser,
                                     struct DDS_SqlParserToken *token,
                                     struct DDS_SqlParserTokenState *state)
{
    RTI_BOOL valid = (state->digit_count > 0);
    DDS_UnsignedLongLong max_magnitude;

    if ((*parser->cur_ptr == 'l') || (*parser->cur_ptr == 'L'))
    {
        token->kind = DDS_SQL_TOKEN_INT64;
        (parser->cur_ptr)++;
        max_magnitude = state->negative
                ? ((DDS_UnsignedLongLong)LLONG_MAX + 1ULL)
                : (DDS_UnsignedLongLong)LLONG_MAX;
    }
    else
    {
        /* int32 is default without a suffix */
        token->kind = DDS_SQL_TOKEN_INT32;
        max_magnitude = state->negative
                ? ((DDS_UnsignedLongLong)INT_MAX + 1ULL)
                : (DDS_UnsignedLongLong)INT_MAX;
    }

    if (state->int_overflowed || (state->int_accumulator > max_magnitude))
    {
        DDS_FILTER_LOG_SQL_INTEGER_OUT_OF_RANGE(OSAPI_LOGKIND_WARNING,
                                                DDS_SqlParser_get_current_offset(parser))
        valid = RTI_FALSE;
    }

    if (valid)
    {
        token->value.int_value = state->negative
                ? (DDS_LongLong)(0ULL - state->int_accumulator)
                : (DDS_LongLong)state->int_accumulator;
        state->rval = DDS_BOOLEAN_TRUE;
    }
    state->done = RTI_TRUE;
}

/* Reclassify a keyword as one of the reserved keywords if it matches */
RTI_PRIVATE void
DDS_SqlParser_classify_keyword(struct DDS_SqlParserToken *token,
                               RTI_SIZE_T string_index)
{
    if (DDS_SqlParser_keyword_match(token->value.str_value, string_index, "not", 3))
    {
        token->kind = DDS_SQL_TOKEN_NOT;
    }
    else if (DDS_SqlParser_keyword_match(token->value.str_value, string_index, "and", 3))
    {
        token->kind = DDS_SQL_TOKEN_AND;
    }
    else if (DDS_SqlParser_keyword_match(token->value.str_value, string_index, "or", 2))
    {
        token->kind = DDS_SQL_TOKEN_OR;
    }
    else if (DDS_SqlParser_keyword_match(token->value.str_value, string_index, "between", 7))
    {
        token->kind = DDS_SQL_TOKEN_BETWEEN;
    }
    else if (DDS_SqlParser_keyword_match(token->value.str_value, string_index, "true", 4))
    {
        token->kind = DDS_SQL_TOKEN_BOOLEAN;
        token->value.int_value = 1;
    }
    else if (DDS_SqlParser_keyword_match(token->value.str_value, string_index, "false", 5))
    {
        token->kind = DDS_SQL_TOKEN_BOOLEAN;
        token->value.int_value = 0;
    }
}

RTI_PRIVATE RTI_BOOL
DDS_SqlParser_start_number_token(struct DDS_SqlParser *parser,
                                 struct DDS_SqlParserToken *token,
                                 struct DDS_SqlParserTokenState *state)
{
    char c = *parser->cur_ptr;

    token->kind = DDS_SQL_TOKEN_NUMBER;
    state->string_index = 0;
    state->int_overflowed = RTI_FALSE;
    state->negative = (c == '-');

    if (DDS_SqlParser_is_digit(c))
    {
        state->digit_count = 1;
        state->int_accumulator = (DDS_UnsignedLongLong)(c - '0');
    }
    else
    {
        /* '+' or '-' prefix; no digit yet */
        state->digit_count = 0;
        state->int_accumulator = 0ULL;
    }

    return DDS_SqlParser_token_append_char(parser, token, &state->string_index);
}

/* Finalize a float literal as FLOAT (f/F suffix) or DOUBLE (default) */
RTI_PRIVATE void
DDS_SqlParser_finalize_float_token(struct DDS_SqlParser *parser,
                                   struct DDS_SqlParserToken *token,
                                   struct DDS_SqlParserTokenState *state)
{
    if ((*parser->cur_ptr == 'f') || (*parser->cur_ptr == 'F'))
    {
        token->kind = DDS_SQL_TOKEN_FLOAT;
        (parser->cur_ptr)++;
        state->rval = OSAPI_String_parse_float(token->value.str_value,
                                               &token->value.float_value);
    }
    else
    {
        /* double is default without a suffix */
        token->kind = DDS_SQL_TOKEN_DOUBLE;
        state->rval = OSAPI_String_parse_double(token->value.str_value,
                                                &token->value.double_value);
    }
    state->done = RTI_TRUE;
}

RTI_PRIVATE RTI_BOOL
DDS_SqlParser_transition_to_exponent(struct DDS_SqlParser *parser,
                                     struct DDS_SqlParserToken *token,
                                     struct DDS_SqlParserTokenState *state)
{
    token->kind = DDS_SQL_TOKEN_FLOAT_EXPONENT;
    if (!DDS_SqlParser_token_append_char(parser, token, &state->string_index))
    {
        return RTI_FALSE;
    }
    /* +/- is only allowed at the beginning of the exponent */
    if ((*parser->cur_ptr == '+') || (*parser->cur_ptr == '-'))
    {
        if (!DDS_SqlParser_token_append_char(parser, token, &state->string_index))
        {
            return RTI_FALSE;
        }
    }
    state->exponent_start_index = state->string_index;
    return RTI_TRUE;
}

void
DDS_SqlParser_init(struct DDS_SqlParser *parser,
                   const struct DDS_TypeCode *type_code,
                   const struct DDS_StringSeq *parameters,
                   const char *ptr)
{
    parser->type_code = type_code;
    parser->cur_ptr = ptr;
    parser->parameters = parameters;
#if OSAPI_ENABLE_LOG
    parser->start_ptr = ptr;
#endif /* OSAPI_ENABLE_LOG */
}

#if OSAPI_ENABLE_LOG
RTI_PRIVATE RTI_SIZE_T
DDS_SqlParser_get_current_offset(const struct DDS_SqlParser *parser)
{
    return (RTI_SIZE_T)(parser->cur_ptr - parser->start_ptr);
}
#endif /* OSAPI_ENABLE_LOG */

RTI_PRIVATE DDS_Boolean
DDS_SqlParser_peek_next_token(struct DDS_SqlParser *parser,
                              struct DDS_SqlParserToken *token)
{
    struct DDS_SqlParser tmp_parser = *parser;

    return DDS_SqlParser_get_next_token(&tmp_parser, token);
}

RTI_PRIVATE void
DDS_SqlParser_parse_null_state(struct DDS_SqlParser *parser,
                               struct DDS_SqlParserToken *token,
                               struct DDS_SqlParserTokenState *state)
{
    if (*parser->cur_ptr == '\0')
    {
        token->kind = DDS_SQL_TOKEN_EOF;
        state->done = RTI_TRUE;
        state->rval = RTI_TRUE;
    }
    else if (DDS_SqlParser_is_keyword_start(*parser->cur_ptr))
    {
        token->kind = DDS_SQL_TOKEN_KEYWORD;
        state->string_index = 0;
        if (!DDS_SqlParser_token_append_char(parser, token, &state->string_index))
        {
            state->done = RTI_TRUE;
        }
    }
    else if (*parser->cur_ptr == '\'')
    {
        token->kind = DDS_SQL_TOKEN_STRING;
        state->string_index = 0;
        token->value.str_value[state->string_index] = 0;
        (parser->cur_ptr)++;
    }
    else if (DDS_SqlParser_is_digit(*parser->cur_ptr)
             || (*parser->cur_ptr == '+')
             || (*parser->cur_ptr == '-'))
    {
        /* '+' and '-' are only valid as the start of a number */
        if (!DDS_SqlParser_start_number_token(parser, token, state))
        {
            state->done = RTI_TRUE;
        }
    }
    else if (*parser->cur_ptr == '=')
    {
        token->kind = DDS_SQL_TOKEN_EQUAL;
        (parser->cur_ptr)++;
        state->done = RTI_TRUE;
        state->rval = RTI_TRUE;
    }
    else if (*parser->cur_ptr == '<')
    {
        (parser->cur_ptr)++;
        if (*parser->cur_ptr == '>')
        {
            token->kind = DDS_SQL_TOKEN_NOT_EQUAL;
            (parser->cur_ptr)++;
        }
        else if (*parser->cur_ptr == '=')
        {
            token->kind = DDS_SQL_TOKEN_LE;
            (parser->cur_ptr)++;
        }
        else
        {
            token->kind = DDS_SQL_TOKEN_LT;
        }
        state->done = RTI_TRUE;
        state->rval = RTI_TRUE;
    }
    else if (*parser->cur_ptr == '>')
    {
        (parser->cur_ptr)++;
        if (*parser->cur_ptr == '=')
        {
            token->kind = DDS_SQL_TOKEN_GE;
            (parser->cur_ptr)++;
        }
        else
        {
            token->kind = DDS_SQL_TOKEN_GT;
        }
        state->done = RTI_TRUE;
        state->rval = RTI_TRUE;
    }
    else if (*parser->cur_ptr == '%')
    {
        token->kind = DDS_SQL_TOKEN_PARAMETER;
        state->digit_count = 0;
        token->value.int_value = 0;
        (parser->cur_ptr)++;
    }
    else if ((*parser->cur_ptr == ' ')
             || (*parser->cur_ptr == '\t')
             || (*parser->cur_ptr == '\n'))
    {
        /* Skip over whitespace */
        (parser->cur_ptr)++;
    }
    else
    {
        token->kind = DDS_SQL_TOKEN_UNKNOWN;
        state->done = RTI_TRUE;
    }
}

RTI_PRIVATE void
DDS_SqlParser_parse_keyword_state(struct DDS_SqlParser *parser,
                                  struct DDS_SqlParserToken *token,
                                  struct DDS_SqlParserTokenState *state)
{
    if (DDS_SqlParser_is_keyword_continue(*parser->cur_ptr))
    {
        if (!DDS_SqlParser_token_append_char(parser, token, &state->string_index))
        {
            state->done = RTI_TRUE;
        }
    }
    else if ((*parser->cur_ptr == '.')
            && DDS_SqlParser_is_keyword_start(*(parser->cur_ptr + 1)))
    {
        if (!DDS_SqlParser_token_append_char(parser, token, &state->string_index))
        {
            state->done = RTI_TRUE;
        }
    }
    else
    {
        DDS_SqlParser_classify_keyword(token, state->string_index);
        state->done = RTI_TRUE;
        state->rval = RTI_TRUE;
    }
}

RTI_PRIVATE void
DDS_SqlParser_parse_string_state(struct DDS_SqlParser *parser,
                                 struct DDS_SqlParserToken *token,
                                 struct DDS_SqlParserTokenState *state)
{
    if (*parser->cur_ptr == '\'')
    {
        (parser->cur_ptr)++;
        state->done = RTI_TRUE;
        state->rval = RTI_TRUE;
    }
    else if (*parser->cur_ptr == '\0')
    {
        /* Missing closing quote */
        state->done = RTI_TRUE;
    }
    else if (!DDS_SqlParser_token_append_char(parser, token, &state->string_index))
    {
        /* Exceeded max string length */
        state->done = RTI_TRUE;
    }
}

RTI_PRIVATE void
DDS_SqlParser_parse_number_state(struct DDS_SqlParser *parser,
                                 struct DDS_SqlParserToken *token,
                                 struct DDS_SqlParserTokenState *state)
{
    DDS_UnsignedLongLong digit;

    if (DDS_SqlParser_is_digit(*parser->cur_ptr))
    {
        digit = (DDS_UnsignedLongLong)(*parser->cur_ptr - '0');
        if (state->int_accumulator > ((ULLONG_MAX - digit) / 10ULL))
        {
            state->int_overflowed = RTI_TRUE;
        }
        state->int_accumulator = (state->int_accumulator * 10ULL) + digit;
        ++state->digit_count;
        if (!DDS_SqlParser_token_append_char(parser, token, &state->string_index))
        {
            state->done = RTI_TRUE;
        }
    }
    else if ((*parser->cur_ptr == '.') && (state->digit_count > 0))
    {
        token->kind = DDS_SQL_TOKEN_FLOAT_FRACTION;
        if (!DDS_SqlParser_token_append_char(parser, token, &state->string_index))
        {
            state->done = RTI_TRUE;
        }
    }
    else if (((*parser->cur_ptr == 'e') || (*parser->cur_ptr == 'E'))
             && (state->digit_count > 0))
    {
        if (!DDS_SqlParser_transition_to_exponent(parser, token, state))
        {
            state->done = RTI_TRUE;
        }
    }
    else if (((*parser->cur_ptr == 'x') || (*parser->cur_ptr == 'X'))
             && (state->digit_count > 0))
    {
        if ((state->digit_count != 1) || (state->int_accumulator != 0ULL))
        {
            state->done = RTI_TRUE;
        }
        else
        {
            token->kind = DDS_SQL_TOKEN_HEX;
            state->digit_count = 0;
            (parser->cur_ptr)++;
        }
    }
    else
    {
        DDS_SqlParser_finalize_integer_token(parser, token, state);
    }
}

RTI_PRIVATE void
DDS_SqlParser_parse_float_fraction_state(struct DDS_SqlParser *parser,
                                         struct DDS_SqlParserToken *token,
                                         struct DDS_SqlParserTokenState *state)
{
    if (DDS_SqlParser_is_digit(*parser->cur_ptr))
    {
        if (!DDS_SqlParser_token_append_char(parser, token, &state->string_index))
        {
            state->done = RTI_TRUE;
        }
    }
    else if ((*parser->cur_ptr == 'e') || (*parser->cur_ptr == 'E'))
    {
        if (!DDS_SqlParser_transition_to_exponent(parser, token, state))
        {
            state->done = RTI_TRUE;
        }
    }
    else
    {
        DDS_SqlParser_finalize_float_token(parser, token, state);
    }
}

RTI_PRIVATE void
DDS_SqlParser_parse_float_exponent_state(struct DDS_SqlParser *parser,
                                         struct DDS_SqlParserToken *token,
                                         struct DDS_SqlParserTokenState *state)
{
    if (DDS_SqlParser_is_digit(*parser->cur_ptr))
    {
        if (!DDS_SqlParser_token_append_char(parser, token, &state->string_index))
        {
            state->done = RTI_TRUE;
        }
    }
    else if (state->string_index == state->exponent_start_index)
    {
        /* Error, there were no digits in the exponent */
        state->done = RTI_TRUE;
    }
    else
    {
        DDS_SqlParser_finalize_float_token(parser, token, state);
    }
}

RTI_PRIVATE void
DDS_SqlParser_parse_hex_state(struct DDS_SqlParser *parser,
                              struct DDS_SqlParserToken *token,
                              struct DDS_SqlParserTokenState *state)
{
    RTI_INT32 decoded = DDS_SqlParser_decode_hex_digit(*parser->cur_ptr);

    if (decoded >= 0)
    {
        DDS_UnsignedLongLong digit = (DDS_UnsignedLongLong)decoded;
        if (state->int_accumulator > ((ULLONG_MAX - digit) / 16ULL))
        {
            state->int_overflowed = RTI_TRUE;
        }
        state->int_accumulator = (state->int_accumulator * 16ULL) + digit;
        ++state->digit_count;
        /* No str_value append: hex literals can't become floats, so the
         * textual form isn't needed at finalization. */
        (parser->cur_ptr)++;
    }
    else
    {
        DDS_SqlParser_finalize_integer_token(parser, token, state);
    }
}

RTI_PRIVATE void
DDS_SqlParser_parse_parameter_state(struct DDS_SqlParser *parser,
                                    struct DDS_SqlParserToken *token,
                                    struct DDS_SqlParserTokenState *state)
{
    DDS_LongLong digit;

    if (DDS_SqlParser_is_digit(*parser->cur_ptr))
    {
        /* Require that a parameter fits in INT32 */
        digit = *parser->cur_ptr - '0';
        if (token->value.int_value > ((INT_MAX - digit) / 10))
        {
            /* Integer overflow */
            DDS_FILTER_LOG_SQL_INTEGER_OUT_OF_RANGE(OSAPI_LOGKIND_WARNING,
                                                    DDS_SqlParser_get_current_offset(parser))
            state->done = RTI_TRUE;
        }
        else
        {
            ++state->digit_count;
            token->value.int_value *= 10;
            token->value.int_value += digit;
            (parser->cur_ptr)++;
        }
    }
    else
    {
        state->done = RTI_TRUE;
        if (state->digit_count > 0)
        {
            /* Found at least one number after % */
            state->rval = RTI_TRUE;
        }
    }
}

DDS_Boolean
DDS_SqlParser_get_next_token(struct DDS_SqlParser *parser,
                             struct DDS_SqlParserToken *token)
{
    struct DDS_SqlParserTokenState state = {
        .string_index = 0,
        .exponent_start_index = 0,
        .int_accumulator = 0ULL,
        .digit_count = 0,
        .negative = RTI_FALSE,
        .int_overflowed = RTI_FALSE,
        .done = RTI_FALSE,
        .rval = RTI_FALSE
    };

    token->kind = DDS_SQL_TOKEN_NULL;
    token->value.int_value = 0;

    while (!state.done)
    {
        switch (token->kind)
        {
            case DDS_SQL_TOKEN_NULL:
                DDS_SqlParser_parse_null_state(parser, token, &state);
                break;
            case DDS_SQL_TOKEN_KEYWORD:
                DDS_SqlParser_parse_keyword_state(parser, token, &state);
                break;
            case DDS_SQL_TOKEN_STRING:
                DDS_SqlParser_parse_string_state(parser, token, &state);
                break;
            case DDS_SQL_TOKEN_NUMBER:
                DDS_SqlParser_parse_number_state(parser, token, &state);
                break;
            case DDS_SQL_TOKEN_FLOAT_FRACTION:
                DDS_SqlParser_parse_float_fraction_state(parser, token, &state);
                break;
            case DDS_SQL_TOKEN_FLOAT_EXPONENT:
                DDS_SqlParser_parse_float_exponent_state(parser, token, &state);
                break;
            case DDS_SQL_TOKEN_HEX:
                DDS_SqlParser_parse_hex_state(parser, token, &state);
                break;
            case DDS_SQL_TOKEN_PARAMETER:
                DDS_SqlParser_parse_parameter_state(parser, token, &state);
                break;
            default:
                /* Error, unexpected token kind */
                state.done = RTI_TRUE;
                break;
        }
    }

    return (DDS_Boolean)state.rval;
}
