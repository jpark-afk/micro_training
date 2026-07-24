/*
 * FILE: SqlFilterParser.h - DDS SQL Content Filter parser definitions
 *
 * Copyright (c) 2025-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#ifndef SqlFilterParser_h
#define SqlFilterParser_h

#include "SqlContentFilter.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    /*i \brief No token */
    DDS_SQL_TOKEN_NULL,
    /*i \brief End of file token */
    DDS_SQL_TOKEN_EOF,
    /*i \brief Failed to find a valid token */
    DDS_SQL_TOKEN_UNKNOWN,

    /* Valid returned values */
    /*i \brief A keyword token */
    DDS_SQL_TOKEN_KEYWORD,
    /*i \brief String constant token */
    DDS_SQL_TOKEN_STRING,
    /*i \brief Integer constant token */
    DDS_SQL_TOKEN_INT32,
    DDS_SQL_TOKEN_INT64,
    /*i \brief Floating point constant token */
    DDS_SQL_TOKEN_FLOAT,
    DDS_SQL_TOKEN_DOUBLE,
    /*i \brief Parameter token where the int value is the parameter index */
    DDS_SQL_TOKEN_PARAMETER,
    /*i \brief Boolean constant token */
    DDS_SQL_TOKEN_BOOLEAN,

    /* Valid syntax tokens */
    DDS_SQL_TOKEN_EQUAL,
    DDS_SQL_TOKEN_GT,
    DDS_SQL_TOKEN_GE,
    DDS_SQL_TOKEN_LT,
    DDS_SQL_TOKEN_LE,
    DDS_SQL_TOKEN_NOT_EQUAL,

    DDS_SQL_TOKEN_NOT,
    DDS_SQL_TOKEN_AND,
    DDS_SQL_TOKEN_OR,
    DDS_SQL_TOKEN_BETWEEN,

    /* Intermediate parsing tokens, which are not expected to be returned */
    DDS_SQL_TOKEN_NUMBER,
    DDS_SQL_TOKEN_HEX,
    DDS_SQL_TOKEN_FLOAT_FRACTION,
    DDS_SQL_TOKEN_FLOAT_EXPONENT
} DDS_SqlParserTokenKind_T;

#define DDS_SqlParserTokenKind_is_floating_point(kind_) \
    (((kind_) == DDS_SQL_TOKEN_FLOAT) || ((kind_) == DDS_SQL_TOKEN_DOUBLE))

#define DDS_SqlParserTokenKind_is_integer(kind_) \
    (((kind_) == DDS_SQL_TOKEN_INT32) || ((kind_) == DDS_SQL_TOKEN_INT64))

#define DDS_SqlParserTokenKind_is_immediate(kind_) \
    (DDS_SqlParserTokenKind_is_floating_point(kind_) \
        || DDS_SqlParserTokenKind_is_integer(kind_) \
        || ((kind_) == DDS_SQL_TOKEN_BOOLEAN) \
        || ((kind_) == DDS_SQL_TOKEN_STRING))

#define DDS_SQL_PARSER_MAX_TOKEN_LENGTH 255

union DDS_SqlParserValue
{
    char str_value[DDS_SQL_PARSER_MAX_TOKEN_LENGTH + 1];
    DDS_LongLong int_value;
    DDS_Float float_value;
    DDS_Double double_value;
};

struct DDS_SqlParserToken
{
    DDS_SqlParserTokenKind_T kind;
    union DDS_SqlParserValue value;
};

struct DDS_SqlParser
{
    const struct DDS_TypeCode *type_code;
    const char *cur_ptr;
    const struct DDS_StringSeq *parameters;
#if OSAPI_ENABLE_LOG
    const char *start_ptr;
#endif /* OSAPI_ENABLE_LOG */
};

extern DDS_Boolean
DDS_SqlContentFilter_compile_filter_expression(
        struct DDS_SqlContentFilter *filter_instance,
        struct DDS_SqlCompiledContentFilter *compiled_filter_out,
        const char *expression,
        const struct DDS_StringSeq *parameters,
        const struct DDS_TypeCode *type_code);

extern void
DDS_SqlParser_init(struct DDS_SqlParser *parser,
                   const struct DDS_TypeCode *type_code,
                   const struct DDS_StringSeq *parameters,
                   const char *ptr);

extern DDS_Boolean
DDS_SqlParser_get_next_token(struct DDS_SqlParser *parser,
                             struct DDS_SqlParserToken *token);

#ifdef __cplusplus
}
#endif

#endif /* SqlFilterParser_h */
