/*
 * FILE: dds_filter_log.h - DDS Filter module log codes
 *
 * (c) Copyright 2022-2024 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*e
 * \file dds_filter_log.h
 * \brief DDS Filter module log codes
 */
#ifndef dds_filter_log_h
#define dds_filter_log_h

#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif

/*e
 * \defgroup DDSFilterLogCodesClass DDS_FILTER
 * \brief DDS Filter. ModuleID = 16
 * \ingroup LoggingModule
 */

/*e
 * \brief Database record for a Filter class.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_FILTER_CLASS_RECORD                1

/*e
 * \brief Database record for a compiled filter.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_COMPILED_FILTER_RECORD             2

/*e
 * \brief Database record for a reader entry.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_READER_RECORD                      3

/*e
 * \brief Database record for a route entry.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_ROUTE_RECORD                       4

/*e
 * \brief A database select on the specified record kind failed.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_RECORD_SELECT_EC                (DDS_FILTER_LOG_BASE + 1)
#define DDS_FILTER_LOG_RECORD_SELECT(level_,kind_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDS_FILTER_LOG_RECORD_SELECT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_),"dbrc",(dbrc_))

/*e
 * \brief Failed to create a database record of the specified kind.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_RECORD_CREATE_EC                (DDS_FILTER_LOG_BASE + 2)
#define DDS_FILTER_LOG_RECORD_CREATE(level_,kind_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDS_FILTER_LOG_RECORD_CREATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_),"dbrc",(dbrc_))

/*e
 * \brief Failed to delete a database record of the specified kind.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_RECORD_DELETE_EC                (DDS_FILTER_LOG_BASE + 3)
#define DDS_FILTER_LOG_RECORD_DELETE(level_,kind_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDS_FILTER_LOG_RECORD_DELETE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_),"dbrc",(dbrc_))

/*e
 * \brief Failed to insert a database record of the specified kind.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_RECORD_INSERT_EC                (DDS_FILTER_LOG_BASE + 4)
#define DDS_FILTER_LOG_RECORD_INSERT(level_,kind_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDS_FILTER_LOG_RECORD_INSERT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_),"dbrc",(dbrc_))

/*e
 * \brief Failed to remove a database record of the specified kind.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_RECORD_REMOVE_EC                (DDS_FILTER_LOG_BASE + 5)
#define DDS_FILTER_LOG_RECORD_REMOVE(level_,kind_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDS_FILTER_LOG_RECORD_REMOVE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_),"dbrc",(dbrc_))

/*e
 * \brief Failed to create a database table of the specified kind.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_TABLE_CREATE_EC                 (DDS_FILTER_LOG_BASE + 6)
#define DDS_FILTER_LOG_TABLE_CREATE(level_,kind_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDS_FILTER_LOG_TABLE_CREATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_),"dbrc",(dbrc_))

/*e
 * \brief Failed to delete a database table of the specified kind.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_TABLE_DELETE_EC                 (DDS_FILTER_LOG_BASE + 7)
#define DDS_FILTER_LOG_TABLE_DELETE(level_,kind_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDS_FILTER_LOG_TABLE_DELETE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"kind",(kind_),"dbrc",(dbrc_))



/*e
 * \brief Failed to create an instance of the specified filter class.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_FILTER_CREATE_INSTANCE_EC      (DDS_FILTER_LOG_BASE + 10)
#define DDS_FILTER_LOG_FILTER_CREATE_INSTANCE(level_,class_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDS_FILTER_LOG_FILTER_CREATE_INSTANCE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"class",(class_))

/*e
 * \brief Failed to delete an instance of the specified filter class.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_FILTER_DELETE_INSTANCE_EC      (DDS_FILTER_LOG_BASE + 11)
#define DDS_FILTER_LOG_FILTER_DELETE_INSTANCE(level_,class_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDS_FILTER_LOG_FILTER_DELETE_INSTANCE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"class",(class_))

/*e
 * \brief Failed to compile a filter of the specified class.
 *
 * \details
 * For a reader, it is an error to not be able to compile a filter which was
 * explicitly configured for it. For a writer performing writer-side filtering,
 * it is only a warning that it will not be able to filter the data for the
 * reader because it was not able to compile the filter.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_FILTER_COMPILE_EC              (DDS_FILTER_LOG_BASE + 12)
#define DDS_FILTER_LOG_FILTER_COMPILE(level_,class_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDS_FILTER_LOG_FILTER_COMPILE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"class",(class_))

/*e
 * \brief Failed to evaluate a filter of the specified class.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_FILTER_EVALUATE_EC             (DDS_FILTER_LOG_BASE + 13)
#define DDS_FILTER_LOG_FILTER_EVALUATE(level_,class_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDS_FILTER_LOG_FILTER_EVALUATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"class",(class_))

/*e
 * \brief Failed to detach a writer because it still has remote readers attached
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_WRITER_DETACH_EC               (DDS_FILTER_LOG_BASE + 14)
#define DDS_FILTER_LOG_WRITER_DETACH(level_,reader_count_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDS_FILTER_LOG_WRITER_DETACH_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"reader_count",(reader_count_))

/*e
 * \brief No more filters can be compiled because the configured maximum
 *        number of filter expressions has been reached.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_FILTER_EXPRESSION_MAX_COUNT_EC (DDS_FILTER_LOG_BASE + 15)
#define DDS_FILTER_LOG_FILTER_EXPRESSION_MAX_COUNT(level_,max_count_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDS_FILTER_LOG_FILTER_EXPRESSION_MAX_COUNT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"max_count",(max_count_))


/*e
 * \brief Failed to register a filter class because one with the same name
 *        has already been registered.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_FILTER_CLASS_REGISTER_EC       (DDS_FILTER_LOG_BASE + 20)
#define DDS_FILTER_LOG_FILTER_CLASS_REGISTER(level_,name_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDS_FILTER_LOG_FILTER_CLASS_REGISTER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"name",(name_))

/*e
 * \brief Failed to find a filter class registered with the specified name.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_FILTER_CLASS_NOT_FOUND_EC      (DDS_FILTER_LOG_BASE + 21)
#define DDS_FILTER_LOG_FILTER_CLASS_NOT_FOUND(level_,name_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDS_FILTER_LOG_FILTER_CLASS_NOT_FOUND_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"name",(name_))

/*e
 * \brief Failed to unregister a filter class because it is still in use.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_FILTER_CLASS_UNREGISTER_EC     (DDS_FILTER_LOG_BASE + 22)
#define DDS_FILTER_LOG_FILTER_CLASS_UNREGISTER(level_,name_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDS_FILTER_LOG_FILTER_CLASS_UNREGISTER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"name",(name_))



/*e
 * \brief Content filter topic name string.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_CONTENT_FILTERED_TOPIC_NAME_STRING               1

/*e
 * \brief Content filter related topic name string.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_RELATED_TOPIC_NAME_STRING                        2

/*e
 * \brief Filter class name string.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_FILTER_CLASS_NAME_STRING                         3

/*e
 * \brief Filter expression string.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_FILTER_EXPRESSION_STRING                         4

/*e
 * \brief Filter expression parameter string.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_FILTER_EXPRESSION_PARAMETER_STRING               5

/*e
 * \brief Content filter QoS expression name string.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_CONTENT_FILTER_NAME_STRING                       6

/*e
 * \brief Failed to assert a string in the string manager of a specified kind.
 *
 * \details
 * For string that are configured by the user in their QoS, this is an error
 * that indicates that the resource limits configured were not sufficient to
 * hold the string. For content filter's received through discovery, this is
 * a warning that the content filter will be dropped because of inadequate
 * resources. This could either be because the string is too long or because
 * too many strings of that kind have been configured.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_STRING_ASSERT_EC               (DDS_FILTER_LOG_BASE + 30)
#define DDS_FILTER_LOG_STRING_ASSERT(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDS_FILTER_LOG_STRING_ASSERT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"kind",(kind_))

/*e
 * \brief Failed to delete a string in the string manager of a specified kind.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_STRING_DELETE_EC               (DDS_FILTER_LOG_BASE + 31)
#define DDS_FILTER_LOG_STRING_DELETE(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDS_FILTER_LOG_STRING_DELETE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"kind",(kind_))

/*e
 * \brief Failed to construct a content filter topic name from the combination
 *        of the related topic name and the filter name because it exceeded
 *        the maximum topic name length.
 *
 * \details
 * To maintain compatibility with the RTPS specification, an internal name
 * is constructed to represent a content filter on the wire. This name is
 * constructed by concatenating the related topic name, two colons, and the
 * filter name. If the resulting name exceeds the maximum topic name length,
 * then either the related topic name or the filter name must be shortened.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_CONTENT_FILTER_NAME_TOO_LONG_EC (DDS_FILTER_LOG_BASE + 32)
#define DDS_FILTER_LOG_CONTENT_FILTER_NAME_TOO_LONG(level_,related_topic_name_,filter_name_) \
OSAPI_LOG_ENTRY_ADD_2STRING((level_),DDS_FILTER_LOG_CONTENT_FILTER_NAME_TOO_LONG_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"related_topic_name",(related_topic_name_),\
        "filter_name",(filter_name_))

/*e
 * \brief Failed to create a string manager for a specified string kind,
 *        max_string_size, and memory_allocation.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_STRING_MANAGER_CREATE_EC       (DDS_FILTER_LOG_BASE + 33)
#define DDS_FILTER_LOG_STRING_MANAGER_CREATE(level_,kind_,max_string_size_,memory_allocation_) \
OSAPI_LOG_ENTRY_ADD_3INT((level_),DDS_FILTER_LOG_STRING_MANAGER_CREATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"kind",(kind_),\
        "max_string_size",(max_string_size_),\
        "memory_allocation",(memory_allocation_))

/*e
 * \brief Failed to deserialize a content filter property because it contained
 *        to many expression parameters.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_FILTER_PARAMETER_COUNT_EC      (DDS_FILTER_LOG_BASE + 34)
#define DDS_FILTER_LOG_FILTER_PARAMETER_COUNT(level_,count_,max_count_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),DDS_FILTER_LOG_FILTER_PARAMETER_COUNT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"count",(count_),"max_count",(max_count_))


/*e
 * \brief Content filtering is not supported on this data type because it's
 *        type support does not include a type code.
 *
 * \details
 * Content filtering requires the type code to be able to evaluate the
 * filter expression. The type code can be automatically generated by rtiddsgen
 * either by annotating the type in its IDL with \@interpreted(true) or by
 * using the "-interpreted 1" flag.
 */
#define DDS_FILTER_LOG_TYPE_CODE_NOT_FOUND_EC         (DDS_FILTER_LOG_BASE + 40)
#define DDS_FILTER_LOG_TYPE_CODE_NOT_FOUND(level_,type_name_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDS_FILTER_LOG_TYPE_CODE_NOT_FOUND_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"type_name",(type_name_))



/*e
 * \brief Heap memory for a filter plugin factory instance.
 */
#define DDS_FILTER_LOG_FILTER_PLUGIN_FACTORY_BUFFER                      1

/*e
 * \brief Heap memory for a filter plugin instance.
 */
#define DDS_FILTER_LOG_FILTER_PLUGIN_BUFFER                              2

/*e
 * \brief Heap memory for a writer filter instance.
 */
#define DDS_FILTER_LOG_WRITER_FILTER_BUFFER                              3

/*e
 * \brief Heap memory for a writer to serialize a sample's in-line QoS
 */
#define DDS_FILTER_LOG_INLINE_QOS_BUFFER                                 4

/*e
 * \brief Failed to allocate heap memory for a buffer of a given type.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_HEAP_ALLOC_EC                  (DDS_FILTER_LOG_BASE + 50)
#define DDS_FILTER_LOG_HEAP_ALLOC(level_,type_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDS_FILTER_LOG_HEAP_ALLOC_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"type",(type_))

/*e
 * \brief Failed to initialize a filter plugin
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_FILTER_PLUGIN_INIT_EC          (DDS_FILTER_LOG_BASE + 51)
#define DDS_FILTER_LOG_FILTER_PLUGIN_INIT(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDS_FILTER_LOG_FILTER_PLUGIN_INIT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to finalize a filter plugin
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_FILTER_PLUGIN_FINALIZE_EC      (DDS_FILTER_LOG_BASE + 52)
#define DDS_FILTER_LOG_FILTER_PLUGIN_FINALIZE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDS_FILTER_LOG_FILTER_PLUGIN_FINALIZE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to finalize a filter plugin factory because it is still in use
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_FILTER_PLUGIN_FACTORY_IN_USE_EC (DDS_FILTER_LOG_BASE + 53)
#define DDS_FILTER_LOG_FILTER_PLUGIN_FACTORY_IN_USE(level_,instance_count_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDS_FILTER_LOG_FILTER_PLUGIN_FACTORY_IN_USE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"instance_count",(instance_count_))


/*e
 * \brief A writer filter failed to compile a filter for a discovered reader.
 *
 * \details
 * This is a warning that the writer will not be able to filter the data for
 * the reader because it was not able to compile the filter.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_WRITER_FILTER_COMPILE_EC       (DDS_FILTER_LOG_BASE + 61)
#define DDS_FILTER_LOG_WRITER_FILTER_COMPILE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDS_FILTER_LOG_WRITER_FILTER_COMPILE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to remove a reader from a writer filter because it still has active routes.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_WRITER_FILTER_REMOVE_READER_EC (DDS_FILTER_LOG_BASE + 62)
#define DDS_FILTER_LOG_WRITER_FILTER_REMOVE_READER(level_,route_count_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDS_FILTER_LOG_WRITER_FILTER_REMOVE_READER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"route_count",(route_count_))

/*e
 * \brief Writer failed to send a reader a GAP to indicate filtered data.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_WRITER_FILTER_SEND_GAP_EC      (DDS_FILTER_LOG_BASE + 63)
#define DDS_FILTER_LOG_WRITER_FILTER_SEND_GAP(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDS_FILTER_LOG_WRITER_FILTER_SEND_GAP_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Writer failed to serialize the content filter info into the in-line QoS.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_WRITER_SERIALIZE_INLINE_QOS_EC (DDS_FILTER_LOG_BASE + 64)
#define DDS_FILTER_LOG_WRITER_SERIALIZE_INLINE_QOS(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDS_FILTER_LOG_WRITER_SERIALIZE_INLINE_QOS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Writer filter was evaluated out of order which requires invalidating
 *        a previously results.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_WRITER_FILTER_EVALUATE_OUT_OF_ORDER_EC (DDS_FILTER_LOG_BASE + 65)
#define DDS_FILTER_LOG_WRITER_FILTER_EVALUATE_OUT_OF_ORDER(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDS_FILTER_LOG_WRITER_FILTER_EVALUATE_OUT_OF_ORDER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Writer filter unexpectedly failed to store the result of a filter evaluation.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_WRITER_FILTER_STORE_RESULT_EC  (DDS_FILTER_LOG_BASE + 66)
#define DDS_FILTER_LOG_WRITER_FILTER_STORE_RESULT(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDS_FILTER_LOG_WRITER_FILTER_STORE_RESULT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)



/*e
 * \brief Unsupported type code for SQL filtering
 *
 * \details
 * The DDS SQL filter only supports filtering on structs with the C or C++
 * type binding. The flat data language binding is not supported.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_SQL_UNSUPPORTED_TYPE_CODE_EC  (DDS_FILTER_LOG_BASE + 100)
#define DDS_FILTER_LOG_SQL_UNSUPPORTED_TYPE_CODE(level_,type_name_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDS_FILTER_LOG_SQL_UNSUPPORTED_TYPE_CODE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"type_name",(type_name_))

/*e
 * \brief Found an unexpected token while parsing an SQL filter expression
 *
 * \details
 * This error indicate that the SQL parser found a token that it did not
 * expect at the given index in the expression. This could occur for various
 * syntax errors in the expression. Check that the expression follows the
 * supported SQL syntax.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_SQL_UNEXPECTED_TOKEN_EC       (DDS_FILTER_LOG_BASE + 101)
#define DDS_FILTER_LOG_SQL_UNEXPECTED_TOKEN(level_,index_) \
OSAPI_LOG_ENTRY_ADD_1UINT((level_),DDS_FILTER_LOG_SQL_UNEXPECTED_TOKEN_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"index",(index_))

/*e
 * \brief Failed to parse an SQL filter expression because the number of predicates
 *        in the expression exceeded the configured maximum.
 *
 * \details
 * The maximum number of predicates in an SQL filter expression is configured
 * with max_predicates_per_expression in the DDS_SqlFilterProperty when
 * registering SQL content filter support with DDS_SqlContentFilter_register().
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_SQL_MAX_PREDICATES_EXCEEDED_EC (DDS_FILTER_LOG_BASE + 102)
#define DDS_FILTER_LOG_SQL_MAX_PREDICATES_EXCEEDED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),DDS_FILTER_LOG_SQL_MAX_PREDICATES_EXCEEDED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Found an invalid token while parsing an SQL filter expression
 *
 * \details
 * This error indicates that the SQL parser found a token that it did not
 * recognize at the given index in the expression. This could occur for various
 * syntax errors in the expression. Check that the expression follows the
 * supported SQL syntax.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_SQL_INVALID_TOKEN_EC          (DDS_FILTER_LOG_BASE + 103)
#define DDS_FILTER_LOG_SQL_INVALID_TOKEN(level_,index_) \
OSAPI_LOG_ENTRY_ADD_1UINT((level_),DDS_FILTER_LOG_SQL_INVALID_TOKEN_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"index",(index_))

/*e
 * \brief Failed to parse a parameter in an SQL filter expression
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_SQL_PARSE_PARAMETER_EC        (DDS_FILTER_LOG_BASE + 104)
#define DDS_FILTER_LOG_SQL_PARSE_PARAMETER(level_,param_num_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),DDS_FILTER_LOG_SQL_PARSE_PARAMETER_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"param_num",(param_num_))

/*e
 * \brief Failed to parse an SQL filter expression because a given field name
 *        did not match any field in the type code of the data being filtered.
 *
 * \details
 * This error indicates a field name in an SQL filter expression did not correspond
 * to any field in the type code of the data being filtered. This could also occur
 * if the specified field is not a field which can be used in an SQL filter. Only
 * primitive, top-level, non-optional fields can be used in an SQL expression.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_SQL_FIELD_NOT_FOUND_EC        (DDS_FILTER_LOG_BASE + 105)
#define DDS_FILTER_LOG_SQL_FIELD_NOT_FOUND(level_,type_name_,field_name_) \
OSAPI_LOG_ENTRY_ADD_2STRING((level_),DDS_FILTER_LOG_SQL_FIELD_NOT_FOUND_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
        "type_name",(type_name_),\
        "field_name",(field_name_))

/*e
 * \brief A field specified in an SQL filter expression is not a supported type
 *        for use in an SQL filter.
 *
 * \details
 * This error indicates that a field in an SQL filter expression is not a
 * supported type for use in an SQL filter. Only primitive fields can be used in
 * an SQL filter expression and strings are not supported.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_SQL_UNSUPPORTED_FIELD_TYPE_EC (DDS_FILTER_LOG_BASE + 106)
#define DDS_FILTER_LOG_SQL_UNSUPPORTED_FIELD_TYPE(level_,type_name_,field_name_) \
OSAPI_LOG_ENTRY_ADD_2STRING((level_),DDS_FILTER_LOG_SQL_UNSUPPORTED_FIELD_TYPE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
        "type_name",(type_name_),\
        "field_name",(field_name_))

/*e
 * \brief Failed to find a given enum label in an SQL filter expression
 *
 * \details
 * This error indicates that an enum label specified in an SQL filter expression
 * did not correspond to any label in type code of the enum type.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_SQL_ENUM_LABEL_NOT_FOUND_EC   (DDS_FILTER_LOG_BASE + 107)
#define DDS_FILTER_LOG_SQL_ENUM_LABEL_NOT_FOUND(level_,label_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),DDS_FILTER_LOG_SQL_ENUM_LABEL_NOT_FOUND_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"label",(label_))

/*e
 * \brief Comparison between two incompatible types in an SQL filter expression
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_SQL_INCOMPATIBLE_TYPES_EC     (DDS_FILTER_LOG_BASE + 108)
#define DDS_FILTER_LOG_SQL_INCOMPATIBLE_TYPES(level_,index_) \
OSAPI_LOG_ENTRY_ADD_1UINT((level_),DDS_FILTER_LOG_SQL_INCOMPATIBLE_TYPES_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"index",(index_))

/*e
 * \brief Integer constant in an SQL expression is out of range for the type
 *
 * \details
 * The default integer type in an SQL expression is 32-bit signed integer. If
 * additional range is needed, then the constant can be suffixed with "L" to
 * indicate that it is a 64-bit signed integer.
 *
 * \ingroup DDSFilterLogCodesClass
 */
#define DDS_FILTER_LOG_SQL_INTEGER_OUT_OF_RANGE_EC (DDS_FILTER_LOG_BASE + 109)
#define DDS_FILTER_LOG_SQL_INTEGER_OUT_OF_RANGE(level_,index_) \
OSAPI_LOG_ENTRY_ADD_1UINT((level_),DDS_FILTER_LOG_SQL_INTEGER_OUT_OF_RANGE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,"index",(index_))
#endif  /* dds_filter_log_h */
