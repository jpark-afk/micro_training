/*
 * FILE: SqlContentFilter.h - DDS SQL Content Filter definitions
 *
 * Copyright (c) 2025-2026 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#ifndef SqlContentFilter_h
#define SqlContentFilter_h

#include "dds_filter/dds_filter_content_filter.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*ci
 * \brief An instance of the DDS SQL content filter.
 *
 * \details Each participant which uses this filter class will create an
 *          instance of this object which will manage all of the resources
 *          needed for SQL filter expression on that participant.
 */
struct DDS_SqlContentFilter
{
    REDA_BufferPool_T compiled_filter_pool;

    struct DDS_SqlFilterProperty property;
};

/* DDS_Octet enumeration of valid comparison operators */
#define DDS_SQL_COMPARISON_EQUAL 0
#define DDS_SQL_COMPARISON_GREATER 1
#define DDS_SQL_COMPARISON_GREATER_EQUAL 2
#define DDS_SQL_COMPARISON_LESS 3
#define DDS_SQL_COMPARISON_LESS_EQUAL 4
#define DDS_SQL_COMPARISON_NOT_EQUAL 5
#define DDS_SQL_COMPARISON_BETWEEN 6
#define DDS_SQL_COMPARISON_NOT_BETWEEN 7

/*i \brief Supported SQL operand kinds. This is a superset of types which will
 *         actually be stored.
 */
enum DDS_SqlOperandType
{
    DDS_SQL_KIND_INT32,
    DDS_SQL_KIND_INT64,
    DDS_SQL_KIND_UINT32,
    DDS_SQL_KIND_UINT64,
    DDS_SQL_KIND_FLOAT,
    DDS_SQL_KIND_DOUBLE,
    DDS_SQL_KIND_BOOLEAN,
    DDS_SQL_KIND_CHAR,
    DDS_SQL_KIND_ENUM
};

/*i \brief Discriminator for DDS_SqlImmediateValue. Represents the memory representation
 *         of an immediate value and determines how the value is compared.
 *
 * \details UINT32 widens to INT64 for comparison; UINT64 has its own path
 *          since no wider signed type exists.
 */
enum DDS_SqlImmediateType
{
    DDS_SQL_TYPE_INT64,
    DDS_SQL_TYPE_UINT64,
    DDS_SQL_TYPE_FLOAT,
    DDS_SQL_TYPE_DOUBLE
};

union DDS_SqlImmediateValue
{
    DDS_LongLong int64_value;
    DDS_UnsignedLongLong uint64_value;
    DDS_Float float_value;
    DDS_Double double_value;
};

struct DDS_SqlFieldValue
{
    DDS_Long type;

    DDS_UnsignedLong offset;
};

union DDS_SqlPredicateValue
{
    union DDS_SqlImmediateValue immediate;
    struct DDS_SqlFieldValue field;
};

/*ci
 * \brief Flag to indicate that the predicate is prefixed with NOT
 */
#define DDS_SQL_FLAG_NOT 0x01

/*ci
 * \brief Flag to indicate that the conditional operator before this predicate
 *        is an AND. If not set, the conditional operator is an OR.
 */
#define DDS_SQL_FLAG_AND 0x02

#define DDS_SQL_FLAG_A_FIELD 0x04

#define DDS_SQL_FLAG_B_FIELD 0x08

/*ci \brief Operand A came from a signed source. Only meaningful when
 *         comparison_type is DDS_SQL_TYPE_UINT64.
 */
#define DDS_SQL_FLAG_A_SIGNED_SOURCE 0x10

/*ci \brief Like DDS_SQL_FLAG_A_SIGNED_SOURCE, for operand B (and the
 *         BETWEEN upper bound, which shares B's kind).
 */
#define DDS_SQL_FLAG_B_SIGNED_SOURCE 0x20

struct DDS_SqlPredicate
{
    union DDS_SqlPredicateValue left;
    union DDS_SqlPredicateValue right;
    union DDS_SqlPredicateValue upper_range;

    /*ci
     * \brief The type on which the comparison operation is performed. All
     *        immediate values are expected to be stored as this type. Any
     *        field value is converted to this type before the comparison
     *        operation is applied.
     */
    enum DDS_SqlImmediateType comparison_type;

    /*ci
     * \brief The type of the comparison operator used in this predicate.
     *        This is one of the DDS_SQL_COMPARISON_* constants.
     */
    DDS_Octet comparison_operator;

    /*ci
     * \brief Flags for this predicate.
     */
    DDS_Octet flags;

    /*ci \brief Index of the next OR predicate, or predicate_count if none.
     *         Width bounded by predicate_max_count_per_expression <= USHRT_MAX.
     */
    DDS_UnsignedShort next_or_index;
};

/*ci
 * \brief A compiled SQL content filter
 */
struct DDS_SqlCompiledContentFilter
{
    struct DDS_SqlPredicate *predicates;

    DDS_UnsignedShort predicate_count;
};

#define DDS_SqlCompiledContentFilter_INITIALIZER \
{ \
    NULL, \
    0 \
}

extern void *
DDS_SqlContentFilter_create_instance(
        const struct DDS_FilterResourceLimits *resource_limits,
        const void *property);

#ifndef RTI_CERT
extern void
DDS_SqlContentFilter_delete_instance(void *filter_instance);
#endif

extern DDS_Boolean
DDS_SqlContentFilter_compile(
        void *filter_instance,
        void **compiled_expression_out,
        const char *expression,
        const struct DDS_StringSeq *parameters,
        const struct DDS_TypeCode *type_code,
        const char *type_class_name);

extern DDS_Boolean
DDS_SqlContentFilter_evaluate(
        void *filter_instance,
        void *compiled_expression,
        const void *sample,
        DDS_Boolean *result_out);

extern void
DDS_SqlContentFilter_finalize(
        void *filter_instance,
        void *compiled_expression);

#ifdef __cplusplus
}
#endif

#endif /* SqlContentFilter_h */
