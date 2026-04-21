/*
 * FILE: xcdr_log.h
 *
 * (c) Copyright 2018-2019 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*e
 * \file
 * \defgroup CDRLogCodesClass CDR
 * \brief CDR. ModuleID = 5 XCDR_LOG_BASE = 20000
 * \ingroup LoggingModule
 */
#ifndef xcdr_log_h
#define xcdr_log_h

#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif

/*e
 * \brief Unknown error
 * \ingroup CDRLogCodesClass
 *
 */
#define XCDR_LOG_UNKNOWN_FAILURE_ID                          (XCDR_LOG_BASE + 1)

/*e
 * \brief Precondition failure
 * \ingroup CDRLogCodesClass
 */
#define XCDR_LOG_PRECONDITION_FAILURE_MSG_ID_EC              (XCDR_LOG_BASE + 2)

/*e
 * \brief Unsupported feature, typecode is in CDR
 *        representation or invalid program
 * \ingroup CDRLogCodesClass
 */
#define XCDR_LOG_UNSUPPORTED_FAILURE_ID_EC                   (XCDR_LOG_BASE + 3)

/*e
 * \brief Failed to allocate sample buffer
 * \ingroup CDRLogCodesClass
 */
#define XCDR_LOG_ALLOCATE_BUFFER_FAILURE_MSG_ID_EC           (XCDR_LOG_BASE + 4)

/*e
 * \brief Failed to allocate structure
 * \ingroup CDRLogCodesClass
 */
#define XCDR_LOG_ALLOCATE_STRUCTURE_FAILURE_MSG_ID_EC        (XCDR_LOG_BASE + 5)

/*e
 * \brief Failed to allocate array
 * \ingroup CDRLogCodesClass
 */
#define XCDR_LOG_ALLOCATE_ARRAY_FAILURE_MSG_ID_EC            (XCDR_LOG_BASE + 6)

/*e
 * \brief Primitive sequence exceeds the maximum length allowed
 *        in IDL.
 * \ingroup CDRLogCodesClass
 */
#define XCDR_LOG_EXCEED_SEQ_MAX_LENGTH_ID_EC                 (XCDR_LOG_BASE + 7)

/*e
 * \brief Failed to create type-support
 * \ingroup CDRLogCodesClass
 */
#define XCDR_LOG_CREATE_FAILURE_ID_EC                        (XCDR_LOG_BASE + 8)

/*e
 * \brief Failed to create program
 * \ingroup CDRLogCodesClass
 */
#define XCDR_LOG_GENERATE_PROGRAM_FAILURE_ID_EC              (XCDR_LOG_BASE + 9)

/*e
 * \brief Failed to deserialize sample
 * \ingroup CDRLogCodesClass
 */
#define XCDR_LOG_CDR_DESERIALIZE_FAILURE_ID_EC               (XCDR_LOG_BASE + 10)

/*e
 * \brief Failed to skip sample content
 * \ingroup CDRLogCodesClass
 */
#define XCDR_LOG_CDR_SKIP_FAILURE_ID_EC                      (XCDR_LOG_BASE + 11)

/*e
 * \brief Failed to serialize sample
 * \ingroup CDRLogCodesClass
 */
#define XCDR_LOG_CDR_SERIALIZE_FAILURE_ID_EC                 (XCDR_LOG_BASE + 12)

/*e
 * \brief Find to retreive reference to a member
 * \ingroup CDRLogCodesClass
 */
#define XCDR_LOG_CDR_NULL_MEMBER_FAILURE_ID_EC               (XCDR_LOG_BASE + 13)

/*e
 * \brief Primitive type exceeds legal value
 * \ingroup CDRLogCodesClass
 */
#define XCDR_LOG_GENERATE_PROGRAM_OUT_OF_BOUNDS_PRIMITIVE_FAILURE_ID_EC (XCDR_LOG_BASE + 14)

/*e
 * \brief Program exceeded internal limits
 * \ingroup CDRLogCodesClass
 */
#define XCDR_LOG_GENERATE_PROGRAM_OUT_OF_BOUNDS_FAILURE_ID_EC (XCDR_LOG_BASE + 15)

/*e
 * \brief Failed to get sample access information
 * \ingroup CDRLogCodesClass
 */
#define XCDR_LOG_GET_FAILURE_ID_EC                          (XCDR_LOG_BASE + 16)

/*e
 * \brief Failed to initialize type-support
 * \ingroup CDRLogCodesClass
 */
#define XCDR_LOG_INITIALIZE_FAILURE_ID_EC                   (XCDR_LOG_BASE + 17)

/*e
 * \brief Failed to initialize type support
 * \ingroup CDRLogCodesClass
 */
#define XCDR_LOG_FINALIZE_FAILURE_ID_EC                     (XCDR_LOG_BASE + 18)

/*e
 * \brief Failed to copy sample
 * \ingroup CDRLogCodesClass
 */
#define XCDR_LOG_COPY_FAILURE_ID_EC                         (XCDR_LOG_BASE + 19)

/*e
 * \brief String field exceeds maximum allocated space
 * \ingroup CDRLogCodesClass
 */
#define XCDR_LOG_CDR_DESERIALIZE_OUT_OF_BOUNDS_STRING_FAILURE_ID_EC (XCDR_LOG_BASE + 20)

/*e
 * \brief Sequence exceeds maximum allocated space
 * \ingroup CDRLogCodesClass
 */
#define XCDR_LOG_CDR_DESERIALIZE_OUT_OF_BOUNDS_SEQUENCE_FAILURE_ID_EC (XCDR_LOG_BASE + 21)

/*e
 * \brief Failed to ignore string that exceeds maximum allowed length
 * \ingroup CDRLogCodesClass
 */
#define XCDR_LOG_CDR_SKIP_OUT_OF_BOUNDS_STRING_FAILURE_ID_EC   (XCDR_LOG_BASE + 22)

/*e
 * \brief Failed to ignore sequence that exceeds maximum allowed length
 * \ingroup CDRLogCodesClass
 */
#define XCDR_LOG_CDR_SKIP_OUT_OF_BOUNDS_SEQUENCE_FAILURE_ID_EC (XCDR_LOG_BASE + 23)

/*e
 * \brief String exceeds maximum allowed serialized length
 * \ingroup CDRLogCodesClass
 */
#define XCDR_LOG_CDR_SERIALIZE_OUT_OF_BOUNDS_STRING_FAILURE_ID_EC (XCDR_LOG_BASE + 24)

/*e
 * \brief Sequence exceeds maximum allowed serialized length
 * \ingroup CDRLogCodesClass
 */
#define XCDR_LOG_CDR_SERIALIZE_OUT_OF_BOUNDS_SEQUENCE_FAILURE_ID_EC (XCDR_LOG_BASE + 25)

/*e
 * \brief Failed to generate string support
 * \ingroup CDRLogCodesClass
 */
#define XCDR_LOG_GENERATE_PROGRAM_OUT_OF_BOUNDS_STRING_FAILURE_ID_EC  (XCDR_LOG_BASE + 26)

/*e
 * \brief Invalid enum value serialized
 * \ingroup CDRLogCodesClass
 */
#define XCDR_LOG_CDR_SERIALIZE_INVALID_ENUMERATOR_ID_EC     (XCDR_LOG_BASE + 27)

/*e
 * \brief Invalid enum value deserialized
 * \ingroup CDRLogCodesClass
 */
#define XCDR_LOG_CDR_DESERIALIZE_INVALID_ENUMERATOR_ID_EC   (XCDR_LOG_BASE + 28)

/*e
 * \brief Invalid union discriminator deserialized
 * \ingroup CDRLogCodesClass
 */
#define XCDR_LOG_CDR_DESERIALIZE_INVALID_UNION_DISC_EC      (XCDR_LOG_BASE + 29)

/*e
 * \brief Unknown parameter deserialized
 * \ingroup CDRLogCodesClass
 */
#define XCDR_LOG_CDR_DESERIALIZE_UNKNOWN_PARAMETER_ID_EC    (XCDR_LOG_BASE + 30)

/*e
 * \brief FlatData builder out of resources
 * \ingroup CDRLogCodesClass
 */
#define XCDR_LOG_BUILDER_OUT_OF_RESOURCES_FAILURE_ID_EC     (XCDR_LOG_BASE + 31)

/*e
 * \brief Serialized value out of range
 * \ingroup CDRLogCodesClass
 */
#define XCDR_LOG_SERIALIZE_FAILURE_OUT_OF_RANGE_ID_EC       (XCDR_LOG_BASE + 32)

/*e
 * \brief Deserialized value out of range
 * \ingroup CDRLogCodesClass
 */
#define XCDR_LOG_DESERIALIZE_FAILURE_OUT_OF_RANGE_ID_EC     (XCDR_LOG_BASE + 33)

#endif /* xcdr_log_h */
