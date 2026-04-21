/*
 * FILE: dds_c_infrastructure.h - DDS infrastructure module definitions
 *
 * Copyright (c) 2012-2025 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 07apr2016,tk  MICRO-1541 Fixed assignment operator issues for C++
 * 10feb2016,tk  MICRO-1525 Added assignment operator to DDS_EntityNameQosPolicy
 * 04nov2015,tk  MICRO-1505 - Reduce memory footprint
 * 15jul2015,tk  MICRO-1426/PR#15358 Added Added DDS_Duration_delta_gt
 * 30jun2015,tk  MICRO-1323/PR#15020 Refactored definition of DDS_DURATION_INFINITE
 * 19may2015,as  MICRO-1193 Refactoring of Sequence API levels
 * 02apr2014,eh  MICRO-964/PR#12378 Remove from Cert unused locator defines
 * 16mar2015,tk  MICRO-1129/PR#14274 Increased size of entity_name to
 *                                   DDS_ENTITYNAME_QOS_NAME_MAX + 1
 * 25feb2015,eh  MICRO-1040/PR#13555 Remove unused DDS_Guid_equals/copy/compare
 * 09feb2015,tk  MICRO-1061/PR#13633 DDS_AUTOMATIC_LIVELINESS_QOS is now default
 * 08oct2014,tk  MICRO-919 Added SampleLostReason to SampleLost status
 *               MICRO-918 Removed TimeBasedFilter qos policy (unsupported)
 * 20sep2014,as  Make support functions for QosPolicy types private; remove macros DDSC_CPP_VALUE_TYPE_SUPPORT_FUNCTIONS
 *               and DDSC_CPP_SUPPORT_TYPE_SUPPORT_FUNCTIONS; update DDSC_CPP_VALUE_TYPE_SUPPORT_METHODS and
 *               DDSC_CPP_STATUS_TYPE_SUPPORT_METHODS to only define method signatures (no inline implementation)
 * 04aug2014,tk  MICRO-846/PR#10203 - Limit exposure of PropertySeq API
 *               MICRO-848/PR#10206 - Limit exposure of PropertySeq API
 * 29jul2014,tk  MICRO-864/PR#10245 Fixed C++ DDS_DataReaderResourceLimitsQosPolicy
 * 19jul2013,as  Added support for C++
 * 06feb2013,eh  MICRO-262: add max_remote_readers
 * 29jun2012,tk  Written
 */
/*ce
 * \file
 * \brief DDS Domain Module definitions
 */
/*e @addtogroup DDSInfrastructureModule Infrastructure Module

    @brief Defines the \dds infrastructure package
*/
#ifndef dds_c_infrastructure_h
#define dds_c_infrastructure_h

#ifndef dds_c_config
#include "dds_c/dds_c_config.h"
#endif
#ifndef dds_c_dll_h
#include "dds_c/dds_c_dll.h"
#endif
#ifndef cdr_cdr_type_h
#include "cdr/cdr_cdr_type.h"
#endif
#ifndef netio_rtps_h
#include "netio/netio_rtps.h"
#endif
#ifndef rtps_rtps_h
#include "rtps/rtps_rtps.h"
#endif
#ifndef dds_c_common_h
#include "dds_c/dds_c_common.h"
#endif
#ifndef dds_c_string_h
#include "dds_c/dds_c_string.h"
#endif
#ifndef dds_c_sequence_h
#include "dds_c/dds_c_sequence.h"
#endif
#ifndef dds_c_user_data_manager_h
#include "dds_c/dds_c_user_data_manager.h"
#endif
#ifndef dds_c_string_manager_h
#include "dds_c/dds_c_string_manager.h"
#endif
#if DDS_XTYPES_IS_ENABLED
#ifndef xcdr_interpreter_h
#include "xcdr/xcdr_interpreter.h"
#endif
#endif /* DDS_XTYPES_IS_ENABLED */
#ifndef rtps_trust_plugin_h
#include "rtps/rtps_trust_plugin.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef NULL
#define NULL 0
#endif


typedef DDS_Long DDS_AllocationSettings_t;

/* ================================================================= */
/*                        Micro DDS                                  */
/* ================================================================= */
#define RTI_MICRODDS
#define RTI_MICRODDS_MAJOR 2
#define RTI_MICRODDS_MINOR 0

/* ================================================================= */
/*            CPP VALUE-TYPE SUPPORT                                 */
/* ================================================================= */

#ifdef RTI_CERT
#define DDSC_CPP_CERT_PRIVATE_OPERATORS(T) \
    private:\
        T(const T& from);\
        T& operator=(const T& from);\
        bool operator==(const T& other);\
        bool operator!=(const T& other);

#define DDSC_CPP_QOS_OPERATORS(T) \
        DDSC_CPP_CERT_PRIVATE_OPERATORS(T)

#define DDSC_CPP_QOS_POLICY_OPERATORS(T) \
        DDSC_CPP_CERT_PRIVATE_OPERATORS(T)

#define DDSC_CPP_STATUS_OPERATORS(T) \
        DDSC_CPP_CERT_PRIVATE_OPERATORS(T)
#else
#define DDSC_CPP_QOS_OPERATORS(T) \
    public:\
        bool operator==(const T& other);\
        bool operator!=(const T& other);\
        T& operator=(const T& from);

#define DDSC_CPP_QOS_POLICY_OPERATORS(T)

#define DDSC_CPP_STATUS_OPERATORS(T)
#endif

#ifdef RTI_CPP
#define DDSC_CPP_QOS_METHODS(T) \
    public:\
        T(); \
        T(const T& from);\
        ~T();\
        DDS_ReturnCode_t copy(const T& from); \
        DDSC_CPP_QOS_OPERATORS(T)

#define DDSC_CPP_BUILTINTOPICDATA_METHODS(T) \
    public:\
        T(); \
        ~T();\
        DDSC_CPP_QOS_OPERATORS(T)

#define DDSC_CPP_QOS_POLICY_METHODS(T)

#define DDSC_CPP_STATUS_METHODS(T)

#else
#define DDSC_CPP_QOS_METHODS(T)
#define DDSC_CPP_QOS_POLICY_METHODS(T)
#define DDSC_CPP_STATUS_METHODS(T)
#define DDSC_CPP_BUILTINTOPICDATA_METHODS(T)
#endif

#define DDSC_QOS_POLICY_METHODS_DECL(T)

#if 0
DDSCDllExport void T##_initialize(struct T *policy);
#endif
/* ================================================================= */
/*                        Time Support                               */
/* ================================================================= */

/*e
 * \dref_TimeSupportGroupDocs
 */

/*e
 * \dref_Time_t
 */
struct DDSCPPDllExport DDS_Time_t
{
    /*e
     * \dref_TimeStamp_sec
     */
    DDS_LongLong sec;

    /*e
     * \dref_TimeStamp_nanosec
     */
    DDS_UnsignedLong nanosec;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_Time_t)

#ifdef RTI_CPP
public:
    bool greater_than(const DDS_Time_t& other);
    bool is_zero();
    bool equal(const DDS_Time_t &other);
    DDS_Time_t add(const DDS_Time_t &other);
#endif

};

/* This is only used by tests, keep as macro */
#define DDS_Time_t_greater_than(l, r) \
    (((l).sec > (r).sec) || \
     (((l).sec == (r).sec) && \
      ((l).nanosec > (r).nanosec)))

#ifdef DOXYGEN_DOCUMENTATION_ONLY
/*e
 * \dref_Time_t_greater_than
 */
DDS_Boolean DDS_Time_greater_than(
        const struct DDS_Time_t *l, const struct DDS_Time_t *r);

#endif  /*DOXYGEN_DOCUMENTATION_ONLY */

/*e
 * \dref_Time_t_ZERO
 */
#define DDS_TIME_ZERO    { 0L, 0UL }

/* This is only used by tests, keep as macro */
#define DDS_Time_is_zero(timePtr) \
        ((timePtr)->sec == 0L || (timePtr)->nanosec == 0UL)

#ifdef DOXYGEN_DOCUMENTATION_ONLY

/*e
 * \dref_Time_t_is_zero
 */
    DDS_Boolean DDS_Time_is_zero(const struct DDS_Time_t *time);

#endif  /*DOXYGEN_DOCUMENTATION_ONLY */

/*e
 * \dref_Time_t_INVALID_SEC
 */
extern DDSCDllVariable const DDS_Long DDS_TIME_INVALID_SEC;

/*e
 * \dref_Time_t_INVALID_NSEC
 */
extern DDSCDllVariable const DDS_UnsignedLong DDS_TIME_INVALID_NSEC;

/*e
 * \dref_Time_t_INVALID
 */
extern DDSCDllVariable const struct DDS_Time_t DDS_TIME_INVALID;

/* ================================================================= */
/*                             Duration                              */
/* ================================================================= */

/*i \brief Internal constant for infinite seconds
 */
#define DDS_DURATION_INFINITE_SEC_INITIALIZER  0x7fffffff

/*i \brief Internal constant for infinite nanoseconds
 */
#define DDS_DURATION_INFINITE_NSEC_INITIALIZER 0xffffffffUL

/*e \dref_Duration_t
 */
struct DDSCPPDllExport DDS_Duration_t
{
    /*e \dref_TimeStamp_sec
     */
    DDS_Long sec;

    /*e \dref_TimeStamp_nanosec
     */
    DDS_UnsignedLong nanosec;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_Duration_t)

#ifdef RTI_CPP
public:
    DDS_Boolean is_infinite();
    int compare(const DDS_Duration_t *other);
    DDS_Boolean equal(const DDS_Duration_t *other);
    DDS_Boolean is_zero();
    DDS_Duration_t subtract(const DDS_Duration_t &other);
#endif
};

/*i \brief A infinite duration initializer
 */
#define DDS_DURATION_INFINITE_INITIALIZER \
{ \
    DDS_DURATION_INFINITE_SEC_INITIALIZER, \
    DDS_DURATION_INFINITE_NSEC_INITIALIZER \
}

/*e \dref_Duration_t_INFINITE_SEC
 */
extern DDSCDllVariable const DDS_Long DDS_DURATION_INFINITE_SEC;

/*e \dref_Duration_t_INFINITE_NSEC
 */
extern DDSCDllVariable const DDS_UnsignedLong DDS_DURATION_INFINITE_NSEC;

/*e \dref_Duration_t_INFINITE
 */
extern DDSCDllVariable const struct DDS_Duration_t DDS_DURATION_INFINITE;

/*ci \brief Approximate duration of a year.
 *
 * \details
 * This value is an the number of seconds in a nominal non-leap
 * year with 365 days, each day has 24 hours, and each day has 3600 seconds.
 * It is only used as a chosen upper limit for certain Qos policies.
 * Specifically, this value does not represent the exact number of seconds
 * in any specific year.
 */
extern DDSCDllVariable const struct DDS_Duration_t DDS_DURATION_YEAR;

/*ci \brief 1 nanosecond represented as a duration structure
 */
extern DDSCDllVariable const struct DDS_Duration_t DDS_DURATION_NANOSEC;

/*e \dref_Duration_t_is_infinite
 */
DDSCDllExport DDS_Boolean
DDS_Duration_is_infinite(const struct DDS_Duration_t *duration);

/*ce \dref_Duration_t_compare
 */
DDSCDllExport int
DDS_Duration_compare(const struct DDS_Duration_t *left,
                     const struct DDS_Duration_t *right);

/*ci \dref_Duration_t_to_ntp_format
 */
DDSCDllExport void
DDS_Duration_to_ntp_format(const struct DDS_Duration_t *self,
    RTI_INT32 *sec_out, RTI_UINT32 *frac_out);

/*i
 *\brief Divide a duration
 *
 * \param[inout] duration The duration structure. If infinite an infinite
 *               duration is returned.
 * \param[in]    div      The number to divide by. If 0 an infinite duration is
 *                        is returned.
 */
void
DDS_Duration_div(struct DDS_Duration_t *duration,
                 DDS_UnsignedLong div);

/*e
 * \dref_Duration_t_equal
 */
DDSCDllExport DDS_Boolean
DDS_Duration_equal(const struct DDS_Duration_t *self,
                   const struct DDS_Duration_t *other);
/*i
 *\brief Set a duration structure's seconds and nanoseconds
 *
 * \param[inout] self     The duration structure
 * \param[in]    sec      The seconds part
 * \param[in]    nanosec  The nanoseconds part
 */
DDSCDllExport void
DDS_Duration_set(struct DDS_Duration_t *self,
                 DDS_Long sec,DDS_UnsignedLong nanosec);

/*e \dref_Duration_t_ZERO_SEC
 */
extern DDSCDllVariable const DDS_Long DDS_DURATION_ZERO_SEC;

/*e \dref_Duration_t_ZERO_NSEC
 */
extern DDSCDllVariable const DDS_UnsignedLong DDS_DURATION_ZERO_NSEC;

/*e \dref_Duration_t_ZERO
 */
extern DDSCDllVariable const struct DDS_Duration_t DDS_DURATION_ZERO;

/*e \dref_Duration_t_is_zero
 */
DDSCDllExport DDS_Boolean
DDS_Duration_is_zero(const struct DDS_Duration_t *duration);

/*ci \brief Check if the delta between two durations exceed the specified limit
 *
 * \details
 * This function checks if the time between two durations exceed the
 * specified delta. Note that the concept of time is not
 * relevant. The function only checks if (end - begin) > delta.
 *
 * This function does not perform any robustness checks and assumes that
 * end >= begin.
 *
 * NOTE: This function assumes the delta is normalized as defined by
 *      \ref DDS_Duration_is_normalized.
 *
 * \param[in] delta The maximum allowed delta between the two durations
 * \param[in] end   The duration to be subtracted from
 * \param[in] begin The duration to subtract
 *
 * \return RTI_TRUE if (end - begin) > delta, RTI_FALSE otherwise
 */
DDSCDllExport DDS_Boolean
DDS_Duration_delta_gt(const struct DDS_Duration_t *const delta,
                      const struct DDS_Duration_t *const end,
                      const struct DDS_Duration_t *const begin);


/*ci
 * \brief Check that a duration is normalized
 *
 * \details
 * A normalized duration is defined as an INFINITE duration, or a duration
 * were sec >= 0 and nanosec < 1000000000
 *
 * \param[in] self Duration to verify
 *
 * \return RTI_TRUE if the duration is normalized, RTI_FALSE if not
 */
DDSCDllExport RTI_BOOL
DDS_Duration_is_normalized(const struct DDS_Duration_t *const self);

/*ci
 * \brief Serialize duration to ntp formatted time
 *
 * \param[in] stream    Serialization stream
 * \param[in] duration  Duration to serialize
 * \param[in] param     Unused
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN DDSCDllExport RTI_BOOL
DDS_Duration_serialize(struct CDR_Stream_t *stream,
                       const struct DDS_Duration_t *duration,
                       void *param);

/*ci
 * \brief Deserialize ntp formatted time to duration
 *
 * \param[in]  stream    Deserialization stream
 * \param[out] duration  Deserialized duration
 * \param[in]  param     Unused
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN DDSCDllExport RTI_BOOL
DDS_Duration_deserialize(struct CDR_Stream_t *stream,
                         struct DDS_Duration_t *duration,
                         void *param);

/* ================================================================= */
/*                             Cookie                                */
/* ================================================================= */

struct DDSCPPDllExport DDS_Cookie_t
{
    /*e \dref_Cookie_t_value
     */
    struct DDS_OctetSeq value;

#ifdef RTI_CPP
public:
    void* to_pointer() const;
#endif

};

/*ci
 * \brief Convert a cookie to a pointer
 *
 * \details Only usable for flatdata and zero copy types
 * \param[in] cookie Cookie to convert
 */
void* DDS_Cookie_to_pointer(struct DDS_Cookie_t *cookie);

#define DDS_COOKIE_DEFAULT { DDS_SEQUENCE_INITIALIZER }

/* ================================================================= */
/*                        Instance Handle                            */
/* ================================================================= */

/*e \dref_InstanceHandle_t
 */
typedef DDS_HANDLE_TYPE_NATIVE DDS_InstanceHandle_t;

#define T DDS_InstanceHandle_t
#define TSeq DDS_InstanceHandleSeq
#include <reda/reda_sequence_decl.h>

#ifdef DOXYGEN_DOCUMENTATION_ONLY
/*e \dref_InstanceHandleSeq
 */
struct DDS_InstanceHandleSeq {};
#endif

/*e \dref_InstanceHandle_t_NIL
*/
extern DDSCDllVariable const DDS_InstanceHandle_t DDS_HANDLE_NIL;

/*e \dref_InstanceHandle_t_equals
 */
DDSCDllExport DDS_Boolean
DDS_InstanceHandle_equals(const DDS_InstanceHandle_t *self,
                          const DDS_InstanceHandle_t *other);

#define DDS_InstanceHandle_is_nil(handlePtr) \
        DDS_InstanceHandle_equals(handlePtr, &DDS_HANDLE_NIL)

#ifdef DOXYGEN_DOCUMENTATION_ONLY
/*e \dref_InstanceHandle_t_is_nil
 */
DDS_Boolean DDS_InstanceHandle_is_nil(const DDS_InstanceHandle_t * self);
#endif

/*ci
 * \brief Convert to a DDS_InstanceHandle_t from a RTPS_Guid
 *
 * \details
 * Convert a host endian RTPS_Guid to a big-endian DDS_InstanceHandle_t.
 *
 * \param[out] self  DDS_InstanceHandle_t to convert to
 * \param[in]  other Converted RTPS_Guid
 *
 * \sa \ref DDS_InstanceHandle_to_rtps
 */
DDSCDllExport void
DDS_InstanceHandle_from_rtps(DDS_InstanceHandle_t *self,
                             const struct RTPS_Guid *other);

/*ci
 * \brief Convert from a DDS_InstanceHandle_t to a RTPS_Guid
 *
 * \details
 * Convert a big-endian DDS_InstanceHandle_t to a host endian RTPS_Guid.
 *
 * \param[out]  other RTPS_Guid to convert to
 * \param[in]   self  Converted DDS_InstanceHandle_t
 *
 * \sa \ref DDS_InstanceHandle_from_rtps
 */
DDSCDllExport void
DDS_InstanceHandle_to_rtps(struct RTPS_Guid *other,
                           const DDS_InstanceHandle_t *self);

/*ci
 * \brief Convert from an NETIO_Address to a DDS DDS_InstanceHandle_t
 *
 * \details
 * Convert to a big-endian DDS_InstanceHandle_t from a host-endian
 * NETIO_Address.
 *
 * \param[out] self  DDS_InstanceHandle_t to convert to
 * \param[in]  other Converted NETIO_Address
 *
 * \sa \ref DDS_InstanceHandle_from_rtps
 */
DDSCDllExport void
DDS_InstanceHandle_from_netio_address(DDS_InstanceHandle_t *self,
                                      const struct NETIO_Address *other);

/*ci
 * \brief Compare two DDS_InstanceHandle_t structures for ordering
 *
 * \param[in] self  Left side of comparison
 * \param[in] other Right side of comparison
 *
 * \return positive integer if self is greater than other,
 *         negative integer if self is less than other
 *         zero if left is self to other
 */
DDSCDllExport DDS_Long
DDS_InstanceHandle_compare(const DDS_InstanceHandle_t *self,
                           const DDS_InstanceHandle_t *other);

/* ================================================================= */
/*                               GUID                                */
/* ================================================================= */

/*e
 * \dref_GUIDSupportGroupDocs
 */

#define DDS_GUID_LENGTH     16

/*e \dref_GUID_t
 */
typedef struct DDS_GUID_t
{
    /*e \dref_GUID_t_value
     */
    DDS_Octet value[DDS_GUID_LENGTH];
} DDS_GUID_t;

/*i \dref_GUID_t_INITIALIZER
 */
#define DDS_GUID_INITIALIZER {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}}

/*i \dref_GUID_t_AUTO
*/
extern DDSCDllVariable const struct DDS_GUID_t DDS_GUID_AUTO;

/*e \dref_GUID_t_UNKNOWN
*/
extern DDSCDllVariable const struct DDS_GUID_t DDS_GUID_UNKNOWN;

/*i \dref_GUID_t_PREFIX_UNKNOWN
*/
extern DDSCDllVariable const struct DDS_GUID_t DDS_GUID_PREFIX_UNKNOWN;

/*i \dref_GUID_t_PREFIX_UNKNOWN
 */
extern DDSCDllVariable const struct DDS_GUID_t DDS_GUID_PREFIX_AUTO;

/*i \dref_GUID_t_to_rtps
 * \brief Convert from a DDS_GUID_t to a RTPS_Guid
 *
 * \details
 * This function converts a DDS_GUID_t type in big-endian order to a
 * RTPS_Guid on host order.
 *
 * \param[out] other Output value
 * \param[in]  self  Value to convert from
 */
DDSCDllExport void
DDS_GUID_to_rtps(struct RTPS_Guid *other,
                 const struct DDS_GUID_t *self);

/*i \dref_GUID_t_from_rtps
 * \brief Convert from a RTPS GUID to a DDS_GUID_t
 *
 * \details
 * This function converts from a RTPS_Guid in host order to a DDS_GUID_t in
 * big-endian order.
 *
 * \param[out] self  Output value
 * \param[in]  other Value to convert from
 */
DDSCDllExport void
DDS_GUID_from_rtps(struct DDS_GUID_t *self,
                   const struct RTPS_Guid *other);

/*i \dref_GUID_t_set_suffix
 *
 * \brief Set the suffix, the last 4 bytes, in a DDS_GUID_t in big-endian
 *        order
 *
 * \param[inout] self      GUID to set suffix in
 * \param[in]    suffix    Suffix to set
 */
DDSCDllExport void
DDS_GUID_set_suffix(struct DDS_GUID_t *self,DDS_Long suffix);

/*i \dref_GUID_t_compare
 *
 * \brief Compare the two GUIDs passed
 *
 * \param[inout] self      GUID to compare with
 * \param[in]    other     Other GUID
 */
DDSCDllExport int
DDS_GUID_compare(const struct DDS_GUID_t *self,const struct DDS_GUID_t *other);

/*i \dref_GUID_t_copy
 *
 * \brief Copy the GUID from self to other
 *
 * \param[inout] self      GUID source
 * \param[in]    other     GUID destination
 */
DDSCDllExport void
DDS_GUID_copy(struct DDS_GUID_t *self, const struct DDS_GUID_t *other);

DDSCDllExport RTI_BOOL
DDS_GUID_initialize(DDS_GUID_t* sample);

DDSCDllExport RTI_BOOL
DDS_GUID_finalize(DDS_GUID_t* sample);

DDSCDllExport void
DDS_GUID_print_debug(DDS_GUID_t* sample, const char *prefix);

/* ================================================================= */
/*                        Sequence Number Support                    */
/* ================================================================= */

/*e
 * \dref_SequenceNumberSupportGroupDocs
 */

/*e \dref_SequenceNumber_t
 */
struct DDSCPPDllExport DDS_SequenceNumber_t
{
    /*e \dref_SequenceNumber_t_high
    */
    DDS_Long high;

    /*e \dref_SequenceNumber_t_low
    */
    DDS_UnsignedLong low;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_SequenceNumber_t)

#ifdef RTI_CPP
public:
    int compare(const DDS_SequenceNumber_t *other);
#endif
};

/*i \dref_SequenceNumber_NUMBER_UNKNOWN
 */
#define DDS_SEQUENCE_NUMBER_UNKNOWN    REDA_SEQUENCE_NUMBER_UNKNOWN

/*i \dref_SequenceNumber_NUMBER_ZERO
 */
#define DDS_SEQUENCE_NUMBER_ZERO    REDA_SEQUENCE_NUMBER_ZERO

/*i \dref_SequenceNumber_NUMBER_MAX
 */
#define DDS_SEQUENCE_NUMBER_MAX    REDA_SEQUENCE_NUMBER_MAX

/*e \dref_SequenceNumber_t_compare
 */
DDSCDllExport int
DDS_SequenceNumber_compare(const struct DDS_SequenceNumber_t *sn1,
                           const struct DDS_SequenceNumber_t *sn2);


/* ================================================================= */
/*                         Return Types                              */
/* ================================================================= */
/*e \dref_ReturnCodeGroupDocs
 */

/* ----------------------------------------------------------------- */
/*e \dref_ReturnCode_t
*/
typedef enum
{
    /*e \dref_ReturnCode_t_RETCODE_OK
     */
    DDS_RETCODE_OK = 0,

    /*e \dref_ReturnCode_t_RETCODE_ERROR
     */
    DDS_RETCODE_ERROR = 1,

    /*e \dref_ReturnCode_t_RETCODE_UNSUPPORTED
     */
    DDS_RETCODE_UNSUPPORTED = 2,

    /*e \dref_ReturnCode_t_RETCODE_BAD_PARAMETER
     */
    DDS_RETCODE_BAD_PARAMETER = 3,

    /*e \dref_ReturnCode_t_RETCODE_PRECONDITION_NOT_MET
     */
    DDS_RETCODE_PRECONDITION_NOT_MET = 4,

    /*e \dref_ReturnCode_t_RETCODE_OUT_OF_RESOURCES
     */
    DDS_RETCODE_OUT_OF_RESOURCES = 5,

    /*e \dref_ReturnCode_t_RETCODE_NOT_ENABLED
     */
    DDS_RETCODE_NOT_ENABLED = 6,

    /*e \dref_ReturnCode_t_RETCODE_IMMUTABLE_POLICY
     */
    DDS_RETCODE_IMMUTABLE_POLICY = 7,

    /*e \dref_ReturnCode_t_RETCODE_INCONSISTENT_POLICY
     */
    DDS_RETCODE_INCONSISTENT_POLICY = 8,

    /*e \dref_ReturnCode_t_RETCODE_ALREADY_DELETED
     */
    DDS_RETCODE_ALREADY_DELETED = 9,

    /*e \dref_ReturnCode_t_RETCODE_TIMEOUT
     */
    DDS_RETCODE_TIMEOUT = 10,

    /*e \dref_ReturnCode_t_RETCODE_NO_DATA
     */
    DDS_RETCODE_NO_DATA = 11,

    /*e \dref_ReturnCode_t_RETCODE_ILLEGAL_OPERATION
     */
    DDS_RETCODE_ILLEGAL_OPERATION = 12,

    /*e \dref_ReturnCode_t_RETCODE_NOT_ALLOWED_BY_SECURITY
     */
    DDS_RETCODE_NOT_ALLOWED_BY_SEC = 13
} DDS_ReturnCode_t;


/* ================================================================= */
/*            VARIABLE LENGTH TYPES SUPPORT                          */
/* ================================================================= */
/*i @defgroup DDSVarLenType Full Variable Length Type Support
  @ingroup DDSCommonModule
 */
#define DDSC_VARIABLE_LENGTH_VALUE_TYPE_SUPPORT_FULL(T) \
 struct T; \
 DDSCDllExport DDS_Boolean T ## _initialize(struct T* self); \
 DDSCDllExport DDS_Boolean T ## _finalize(struct T* self); \
 DDSCDllExport DDS_Boolean T ## _copy(struct T* self, const struct T* from);\
 MUST_CHECK_RETURN DDSCDllExport DDS_Boolean      T ## _is_equal(const struct T* self, const struct T* from)

/*i @defgroup DDSVarLenType Basic Variable Length Type Support
  @ingroup DDSCommonModule
 */
#define DDSC_VARIABLE_LENGTH_VALUE_TYPE_SUPPORT_BASIC(T) \
 struct T; \
 DDSCDllExport DDS_Boolean      T ## _is_equal(const struct T* self, const struct T* from)

/*  DDSCDllExport DDS_ReturnCode_t T ## _copy(struct T* self, const struct T* from); */

/* ================================================================= */
/*                    Status Types                                   */
/* ================================================================= */

/*e \dref_StatusKindGroupDocs
 */

/*e \dref_StatusMask
 */
typedef DDS_UnsignedLong DDS_StatusMask;

/*e \dref_STATUS_MASK_NONE
 */
#define DDS_STATUS_MASK_NONE   ((DDS_StatusMask) 0)

/*e \dref_STATUS_MASK_ALL
 */
#define DDS_STATUS_MASK_ALL    (~DDS_STATUS_MASK_NONE)

/* ----------------------------------------------------------------- */
/*e \dref_StatusKind
 */
typedef enum
{
    /*e \dref_StatusKind_INCONSISTENT_TOPIC_STATUS
     */
    DDS_INCONSISTENT_TOPIC_STATUS = 0x0001 << 0,

    /*e \dref_StatusKind_OFFERED_DEADLINE_MISSED_STATUS
     */
    DDS_OFFERED_DEADLINE_MISSED_STATUS = 0x0001 << 1,

    /*e \dref_StatusKind_REQUESTED_DEADLINE_MISSED_STATUS
     */
    DDS_REQUESTED_DEADLINE_MISSED_STATUS = 0x0001 << 2,

    /*e \dref_StatusKind_OFFERED_INCOMPATIBLE_QOS_STATUS
     */
    DDS_OFFERED_INCOMPATIBLE_QOS_STATUS = 0x0001 << 5,

    /*e \dref_StatusKind_REQUESTED_INCOMPATIBLE_QOS_STATUS
     */
    DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS = 0x0001 << 6,

    /*e \dref_StatusKind_SAMPLE_LOST_STATUS
     */
    DDS_SAMPLE_LOST_STATUS = 0x0001 << 7,

    /*e \dref_StatusKind_SAMPLE_REJECTED_STATUS
     */
    DDS_SAMPLE_REJECTED_STATUS = 0x0001 << 8,

    /*e \dref_StatusKind_DATA_ON_READERS_STATUS
     */
    DDS_DATA_ON_READERS_STATUS = 0x0001 << 9,

    /*e \dref_StatusKind_DATA_AVAILABLE_STATUS
     */
    DDS_DATA_AVAILABLE_STATUS = 0x0001 << 10,

    /*e \dref_StatusKind_LIVELINESS_LOST_STATUS
     */
    DDS_LIVELINESS_LOST_STATUS = 0x0001 << 11,

    /*e \dref_StatusKind_LIVELINESS_CHANGED_STATUS
     */
    DDS_LIVELINESS_CHANGED_STATUS = 0x0001 << 12,

    /*e \dref_StatusKind_PUBLICATION_MATCHED_STATUS
     */
    DDS_PUBLICATION_MATCHED_STATUS = 0x0001 << 13,

    /*e \dref_StatusKind_SUBSCRIPTION_MATCHED_STATUS
     */
    DDS_SUBSCRIPTION_MATCHED_STATUS = 0x0001 << 14,

    /* --- Begin extended statuses --- */
    /* The "right"-most 24 bits of the StatusMask are reserved
     * for standard statuses. The remaining 8 bits are for extended statuses.
     */

    /*e \dref_StatusKind_INSTANCE_REPLACED_STATUS
     */
    DDS_INSTANCE_REPLACED_STATUS = 0x0001 << 25,

    /*e \dref_StatusKind_RELIABLE_READER_ACTIVITY_CHANGED_STATUS
     */
    DDS_RELIABLE_READER_ACTIVITY_CHANGED_STATUS = 0x0001 << 26,

    /*e \dref_StatusKind_RELIABLE_READER_ACTIVITY_CHANGED_STATUS
     */
    DDS_DATA_WRITER_SAMPLE_REMOVED_STATUS = 0x0001 << 27

} DDS_StatusKind;

/* ================================================================= */
/*                QoS Types                                          */
/* ================================================================= */
/*e \dref_QosPoliciesGroupDocs
 */

/*e \dref_QosPolicyId_t
 *
 * Note that the value of these constants disagree with the values of
 * the corresponding parameter IDs in the RTPS protocol. This conflict
 * is unavoidable since these values are given in the DDS specification,
 * which is not tied to RTPS.
 */
typedef enum
{
    /*e \dref_QosPolicyId_t_INVALID_QOS_POLICY_ID
     */
    DDS_INVALID_QOS_POLICY_ID = 0,

    /*i \dref_QosPolicyId_t_USERDATA_QOS_POLICY_ID
     */
    DDS_USERDATA_QOS_POLICY_ID = 1,

    /*i \dref_QosPolicyId_t_DURABILITY_QOS_POLICY_ID
     */
    DDS_DURABILITY_QOS_POLICY_ID = 2,

    /*i \dref_QosPolicyId_t_PRESENTATION_QOS_POLICY_ID
     */
    DDS_PRESENTATION_QOS_POLICY_ID = 3,

    /*e \dref_QosPolicyId_t_DEADLINE_QOS_POLICY_ID
     */
    DDS_DEADLINE_QOS_POLICY_ID = 4,

    /*i \dref_QosPolicyId_t_LATENCYBUDGET_QOS_POLICY_ID
     */
    DDS_LATENCYBUDGET_QOS_POLICY_ID = 5,

    /*e \dref_QosPolicyId_t_OWNERSHIP_QOS_POLICY_ID
     */
    DDS_OWNERSHIP_QOS_POLICY_ID = 6,

    /*e \dref_QosPolicyId_t_OWNERSHIPSTRENGTH_QOS_POLICY_ID
     */
    DDS_OWNERSHIPSTRENGTH_QOS_POLICY_ID = 7,

    /*e \dref_QosPolicyId_t_LIVELINESS_QOS_POLICY_ID
     */
    DDS_LIVELINESS_QOS_POLICY_ID = 8,

    /*i \dref_QosPolicyId_t_TIMEBASEDFILTER_QOS_POLICY_ID
     */
    DDS_TIMEBASEDFILTER_QOS_POLICY_ID = 9,

    /*i \dref_QosPolicyId_t_PARTITION_QOS_POLICY_ID
     */
    DDS_PARTITION_QOS_POLICY_ID = 10,

    /*e \dref_QosPolicyId_t_RELIABILITY_QOS_POLICY_ID
     */
    DDS_RELIABILITY_QOS_POLICY_ID = 11,

    /*i \dref_QosPolicyId_t_DESTINATIONORDER_QOS_POLICY_ID
     */
    DDS_DESTINATIONORDER_QOS_POLICY_ID = 12,

    /*e \dref_QosPolicyId_t_HISTORY_QOS_POLICY_ID
     */
    DDS_HISTORY_QOS_POLICY_ID = 13,

    /*i \dref_QosPolicyId_t_RESOURCELIMITS_QOS_POLICY_ID
     */
    DDS_RESOURCELIMITS_QOS_POLICY_ID = 14,

    /*e \dref_QosPolicyId_t_ENTITYFACTORY_QOS_POLICY_ID
     */
    DDS_ENTITYFACTORY_QOS_POLICY_ID = 15,

    /*i \dref_QosPolicyId_t_WRITERDATALIFECYCLE_QOS_POLICY_ID
     */
    DDS_WRITERDATALIFECYCLE_QOS_POLICY_ID = 16,

    /*i \dref_QosPolicyId_t_READERDATALIFECYCLE_QOS_POLICY_ID
     */
    DDS_READERDATALIFECYCLE_QOS_POLICY_ID = 17,

    /*i \dref_QosPolicyId_t_TOPICDATA_QOS_POLICY_ID
     */
    DDS_TOPICDATA_QOS_POLICY_ID = 18,

    /*i \dref_QosPolicyId_t_GROUPDATA_QOS_POLICY_ID
     */
    DDS_GROUPDATA_QOS_POLICY_ID = 19,

    /*i \dref_QosPolicyId_t_TRANSPORTPRIORITY_QOS_POLICY_ID
     */
    DDS_TRANSPORTPRIORITY_QOS_POLICY_ID = 20,

    /*i \dref_QosPolicyId_t_LIFESPAN_QOS_POLICY_ID
     */
    DDS_LIFESPAN_QOS_POLICY_ID = 21,

    /*i \dref_QosPolicyId_t_DURABILITYSERVICE_QOS_POLICY_ID
     */
    DDS_DURABILITYSERVICE_QOS_POLICY_ID = 22,

    /*i \dref_QosPolicyId_t_DATA_REPRESENTATION_QOS_POLICY_ID
     */
    DDS_DATA_REPRESENTATION_QOS_POLICY_ID = 23,

    /*i \dref_QosPolicyId_t_PROPERTY_QOS_POLICY_ID
     */
    DDS_PROPERTY_QOS_POLICY_ID = 24
} DDS_QosPolicyId_t;

/* ----------------------------------------------------------------- */
/*i \dref_QosPolicyCount
 */
struct DDS_QosPolicyCount
{
    /*e \dref_QosPolicyCount_policy_id
     */
    DDS_QosPolicyId_t policy_id;

    /*e \dref_QosPolicyCount_count
     */
    DDS_Long count;
};

#define T struct DDS_QosPolicyCount
#define TSeq DDS_QosPolicyCountSeq
#include <reda/reda_sequence_decl.h>

#ifdef DOXYGEN_DOCUMENTATION_ONLY
/*i \dref_QosPolicyCountSeq
 */
struct DDS_QosPolicyCountSeq
{
};
#endif

/* ================================================================= */
/*                         Entity Types                              */
/* ================================================================= */

/*e \dref_EntityKind_t
*/
typedef enum
{
    /*e \dref_EntityKind_t_UNKNOWN_ENTITY_KIND
     */
    DDS_UNKNOWN_ENTITY_KIND = 0,
    /*e \dref_EntityKind_t_PARTICIPANT_ENTITY_KIND
     */
    DDS_PARTICIPANT_ENTITY_KIND = 1,
    /*e \dref_EntityKind_t_PUBLISHER_ENTITY_KIND
     */
    DDS_PUBLISHER_ENTITY_KIND = 2,
    /*e \dref_EntityKind_t_SUBSCRIBER_ENTITY_KIND
     */
    DDS_SUBSCRIBER_ENTITY_KIND = 3,
    /*e \dref_EntityKind_t_TOPIC_ENTITY_KIND
     */
    DDS_TOPIC_ENTITY_KIND = 4,
    /*e \dref_EntityKind_t_DATAREADER_ENTITY_KIND
     */
    DDS_DATAREADER_ENTITY_KIND = 5,
    /*e \dref_EntityKind_t_DATAWRITER_ENTITY_KIND
     */
    DDS_DATAWRITER_ENTITY_KIND = 6
} DDS_EntityKind_t;

/* ----------------------------------------------------------------- */
/*                DEADLINE                                           */
/* ----------------------------------------------------------------- */
/*e \dref_DeadlineQosGroupDocs
 */

/* ----------------------------------------------------------------- */
/*e \dref_DeadlineQosPolicy
 */
struct DDSCPPDllExport DDS_DeadlineQosPolicy
{
    /*e \dref_DeadlineQosPolicy_period
     */
    struct DDS_Duration_t period;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_DeadlineQosPolicy)
};

DDSC_QOS_POLICY_METHODS_DECL(DDS_DeadlineQosPolicy)

/*ci
 * \brief Calculate the sampling frequency for the deadline Qos policy
 *
 * \details
 *
 * A deadline is measured between two sent or received samples. However,
 * when no more samples are received there is nothing to compare against. To
 * solve this problem a periodic check can be performed. Every period a check
 * is performed to check if a sample should have been received. If the time
 * since the last check exceeds the deadline period the deadline has been
 * missed. This function calculates a reasonable sampling frequency for
 * different deadlines. The sampling frequency is based on a reasonable effort
 * to detect a stale instance without overloading the CPU. The constants used
 * in this function are not used elsewhere and the method to calculate the
 * sampling frequency is considered a formula. Thus, the constants are not
 * considered magic.
 *
 * NOTE: This function assumes valid deadline and sample_freq inputs.
 *       Invalid arguments have undefined behavior.
 *
 * \param[in]  deadline    The deadline to calculate a sampling frequency for
 * \param[out] sample_freq A suitable sampling frequency for deadline
 */
DDSCDllExport void
DDS_DeadlineQosPolicy_get_sample_freq(
                            const struct DDS_DeadlineQosPolicy *const deadline,
                            struct DDS_Duration_t *const sample_freq);

/*i \dref_DeadlineQosPolicy_DEFAULT
 * Default is infinite
 */
#define DDS_DEADLINE_QOS_POLICY_DEFAULT  \
{DDS_DURATION_INFINITE_INITIALIZER}

/* -------------------------------------------------------------------------- */
/*                               LatencyBudget                                */
/* -------------------------------------------------------------------------- */
/*e \dref_LatencyBudgetQosGroupDocs
  */

/*e \dref_LatencyBudgetQosPolicy
 */
struct DDSCPPDllExport DDS_LatencyBudgetQosPolicy
{
    /*e \dref_LatencyBudgetQosPolicy_duration
     */
    struct DDS_Duration_t duration;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_LatencyBudgetQosPolicy)
};

/*i \dref_LatencyBudgetQosPolicy_DEFAULT
 */
#define DDS_LATENCY_BUDGET_QOS_POLICY_DEFAULT { {0L, 0UL} }

DDSC_QOS_POLICY_METHODS_DECL(DDS_LatencyBudgetQosPolicy)

/* ----------------------------------------------------------------- */
/*                OWNERSHIP                                          */
/* ----------------------------------------------------------------- */
/*e \dref_OwnershipQosGroupDocs
 */

/* ----------------------------------------------------------------- */
/*e \dref_OwnershipQosPolicyKind
 */
typedef enum
{
    /*e \dref_OwnershipQosPolicyKind_SHARED_OWNERSHIP_QOS
     */
    DDS_SHARED_OWNERSHIP_QOS,

    /*e \dref_OwnershipQosPolicyKind_EXCLUSIVE_OWNERSHIP_QOS
     */
    DDS_EXCLUSIVE_OWNERSHIP_QOS
} DDS_OwnershipQosPolicyKind;

/* ----------------------------------------------------------------- */
/*e \dref_OwnershipQosPolicy
 */
struct DDSCPPDllExport DDS_OwnershipQosPolicy
{
    /*e \dref_OwnershipQosPolicy_kind
     */
    DDS_OwnershipQosPolicyKind kind;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_OwnershipQosPolicy)
};

/*i \dref_OwnershipQosPolicy_DEFAULT
 */
#define DDS_OWNERSHIP_QOS_POLICY_DEFAULT { DDS_SHARED_OWNERSHIP_QOS }

DDSC_QOS_POLICY_METHODS_DECL(DDS_OwnershipQosPolicy)

/* ----------------------------------------------------------------- */
/*                OWNERSHIP_STRENGTH                                 */
/* ----------------------------------------------------------------- */
/*e \dref_OwnershipStrengthQosGroupDocs
 */

/* ----------------------------------------------------------------- */
/*e \dref_OwnershipStrengthQosPolicy
 */
struct DDSCPPDllExport DDS_OwnershipStrengthQosPolicy
{
    /*e \dref_OwnershipStrengthQosPolicy_value
     */
    DDS_Long value;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_OwnershipStrengthQosPolicy)
};

/*i \dref_OwnershipStrengthQosPolicy_DEFAULT
 */
#define DDS_OWNERSHIP_STRENGTH_QOS_POLICY_DEFAULT { 0L }

DDSC_QOS_POLICY_METHODS_DECL(DDS_OwnershipStrengthQosPolicy)

/* ----------------------------------------------------------------- */
/*                LIVELINESS                                         */
/* ----------------------------------------------------------------- */
/*e \dref_LivelinessQosGroupDocs
 */

/* ----------------------------------------------------------------- */
/*e \dref_LivelinessQosPolicyKind
 */
typedef enum
{
    /*e \dref_LivelinessQosPolicyKind_AUTOMATIC_LIVELINESS_QOS
     */
    DDS_AUTOMATIC_LIVELINESS_QOS,

    /*e \dref_LivelinessQosPolicyKind_MANUAL_BY_PARTICIPANT_LIVELINESS_QOS
     */
    DDS_MANUAL_BY_PARTICIPANT_LIVELINESS_QOS,

    /*e \dref_LivelinessQosPolicyKind_MANUAL_BY_TOPIC_LIVELINESS_QOS
     */
    DDS_MANUAL_BY_TOPIC_LIVELINESS_QOS
} DDS_LivelinessQosPolicyKind;

/* ----------------------------------------------------------------- */
/*e \dref_LivelinessQosPolicy
 */
struct DDSCPPDllExport DDS_LivelinessQosPolicy
{
    /*e \dref_LivelinessQosPolicy_kind
     */
    DDS_LivelinessQosPolicyKind kind;

    /*e \dref_LivelinessQosPolicy_lease_duration
     */
    struct DDS_Duration_t lease_duration;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_LivelinessQosPolicy)
};

/*i \dref_LivelinessQosPolicy_DEFAULT
 * Default least duration is infinite
 */
#define DDS_LIVELINESS_QOS_POLICY_DEFAULT \
{\
    DDS_AUTOMATIC_LIVELINESS_QOS,\
    DDS_DURATION_INFINITE_INITIALIZER \
}

DDSC_QOS_POLICY_METHODS_DECL(DDS_LivelinessQosPolicy)

/* ----------------------------------------------------------------- */
/*                RELIABILITY                                       */
/* ----------------------------------------------------------------- */
/*e \dref_ReliabilityQosGroupDocs
 */

/* ----------------------------------------------------------------- */
/*e \dref_ReliabilityQosPolicyKind
 */
typedef enum
{
    /*e \dref_ReliabilityQosPolicyKind_BEST_EFFORT_RELIABILITY_QOS
     */
    DDS_BEST_EFFORT_RELIABILITY_QOS = 0x01,

    /*e \dref_ReliabilityQosPolicyKind_RELIABLE_RELIABILITY_QOS
     * NOTE: The RTPS spec defines reliability as 0x03 to comply with RTPS spec.
     * However, RTI Connext Core uses 0x3, as well as others.
     */
    DDS_RELIABLE_RELIABILITY_QOS = 0x02
} DDS_ReliabilityQosPolicyKind;

/* ----------------------------------------------------------------- */
/*e \dref_ReliabilityQosPolicy
 */
struct DDSCPPDllExport DDS_ReliabilityQosPolicy
{
    /*e \dref_ReliabilityQosPolicy_kind
     */
    DDS_ReliabilityQosPolicyKind kind;

    /*e \dref_ReliabilityQosPolicy_max_blocking_time
     */
    struct DDS_Duration_t max_blocking_time;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_ReliabilityQosPolicy)
};

/*i \dref_ReliabilityQosPolicy_DEFAULT
 */
#define DDS_DATAREADER_RELIABILITY_QOS_POLICY_DEFAULT \
{ \
    DDS_BEST_EFFORT_RELIABILITY_QOS, \
    {0L, 0UL} \
}

/*i \dref_ReliabilityQosPolicy_DEFAULT
 */
#define DDS_DATAWRITER_RELIABILITY_QOS_POLICY_DEFAULT \
{ \
    DDS_RELIABLE_RELIABILITY_QOS, \
    {0L, 100000000UL} \
}

#define DDS_RELIABILITY_QOS_POLICY_DEFAULT \
{ \
    DDS_BEST_EFFORT_RELIABILITY_QOS, \
    {0L, 0UL} \
}

DDSC_QOS_POLICY_METHODS_DECL(DDS_ReliabilityQosPolicy)

/* ----------------------------------------------------------------- */
/*                HISTORY                                            */
/* ----------------------------------------------------------------- */
/*e \dref_HistoryQosGroupDocs
 */

/* ----------------------------------------------------------------- */
/*e \dref_HistoryQosPolicyKind
 */
typedef enum
{
    /*e \dref_HistoryQosPolicyKind_KEEP_LAST_HISTORY_QOS
     */
    DDS_KEEP_LAST_HISTORY_QOS,

    /*e \dref_HistoryQosPolicyKind_KEEP_ALL_HISTORY_QOS
     */
    DDS_KEEP_ALL_HISTORY_QOS
} DDS_HistoryQosPolicyKind;

/* ----------------------------------------------------------------- */
/*e \dref_HistoryQosPolicy
 */
struct DDSCPPDllExport DDS_HistoryQosPolicy
{
    /*e \dref_HistoryQosPolicy_kind
     */
    DDS_HistoryQosPolicyKind kind;

    /*e \dref_HistoryQosPolicy_depth
     */
    DDS_Long depth;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_HistoryQosPolicy)
};

/*i \dref_HistoryQosPolicy_DEFAULT
 */
#define DDS_HISTORY_QOS_POLICY_DEFAULT { DDS_KEEP_LAST_HISTORY_QOS, \
                                         1L /* depth */ }

DDSC_QOS_POLICY_METHODS_DECL(DDS_HistoryQosPolicy)

/* ----------------------------------------------------------------- */
/*                DURABILITY                                         */
/* ----------------------------------------------------------------- */
/*e \dref_DurabilityQosGroupDocs
*/

/* ----------------------------------------------------------------- */
/*e \dref_DurabilityQosPolicyKind
*/
typedef enum
{
    /*e \dref_DurabilityQosPolicyKind_VOLATILE_DURABILITY_QOS
     */
    DDS_VOLATILE_DURABILITY_QOS,

    /*e \dref_DurabilityQosPolicyKind_TRANSIENT_LOCAL_DURABILITY_QOS
     */
    DDS_TRANSIENT_LOCAL_DURABILITY_QOS,

    /*i \dref_DurabilityQosPolicyKind_TRANSIENT_DURABILITY_QOS
     */
    DDS_TRANSIENT_DURABILITY_QOS,

    /*i \dref_DurabilityQosPolicyKind_PERSISTENT_DURABILITY_QOS
     */
    DDS_PERSISTENT_DURABILITY_QOS
} DDS_DurabilityQosPolicyKind;

/* ----------------------------------------------------------------- */
/*e \dref_DurabilityQosPolicy
 */
struct DDSCPPDllExport DDS_DurabilityQosPolicy
{
    /*e \dref_DurabilityQosPolicy_kind
     */
    DDS_DurabilityQosPolicyKind kind;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_DurabilityQosPolicy)
};

/*i \dref_DurabilityQosPolicy_DEFAULT
 */
#define DDS_DURABILITY_QOS_POLICY_DEFAULT \
{\
    DDS_VOLATILE_DURABILITY_QOS\
}

DDSC_QOS_POLICY_METHODS_DECL(DDS_DurabilityQosPolicy)

/* ----------------------------------------------------------------- */
/*                TransportEncapsulationQosPolicy                    */
/* ----------------------------------------------------------------- */

typedef DDS_UnsignedShort DDS_EncapsulationId_t;

extern DDSCDllVariable const DDS_EncapsulationId_t DDS_ENCAPSULATION_ID_CDR_BE;
extern DDSCDllVariable const DDS_EncapsulationId_t DDS_ENCAPSULATION_ID_CDR_LE;
extern DDSCDllVariable const DDS_EncapsulationId_t DDS_ENCAPSULATION_ID_CDR_NATIVE;
extern DDSCDllVariable const DDS_EncapsulationId_t DDS_ENCAPSULATION_ID_CDR;
extern DDSCDllVariable const DDS_EncapsulationId_t DDS_ENCAPSULATION_ID_PL_CDR_BE;
extern DDSCDllVariable const DDS_EncapsulationId_t DDS_ENCAPSULATION_ID_PL_CDR_LE;
extern DDSCDllVariable const DDS_EncapsulationId_t DDS_ENCAPSULATION_ID_PL_CDR_NATIVE;
extern DDSCDllVariable const DDS_EncapsulationId_t DDS_ENCAPSULATION_ID_PL_CDR;

/* Definition of standard and RTI supported encapsulations (sent as part of the
 * RTPS header.
 */

/* XCDR, CDRv1
 * The default inband encapsulation
 */
#define DDS_ENCAPSULATION_ID_CDR_BE             RTI_CDR_ENCAPSULATION_ID_CDR_BE
#define DDS_ENCAPSULATION_ID_CDR_LE             RTI_CDR_ENCAPSULATION_ID_CDR_LE
#define DDS_ENCAPSULATION_ID_PL_CDR_BE          RTI_CDR_ENCAPSULATION_ID_PL_CDR_BE
#define DDS_ENCAPSULATION_ID_PL_CDR_LE          RTI_CDR_ENCAPSULATION_ID_PL_CDR_LE

#ifdef RTI_ENDIAN_LITTLE
  #define DDS_ENCAPSULATION_ID_CDR_NATIVE        DDS_ENCAPSULATION_ID_CDR_LE
  #define DDS_ENCAPSULATION_ID_PL_CDR_NATIVE     DDS_ENCAPSULATION_ID_PL_CDR_LE
#else
  #define DDS_ENCAPSULATION_ID_CDR_NATIVE        DDS_ENCAPSULATION_ID_CDR_BE
  #define DDS_ENCAPSULATION_ID_PL_CDR_NATIVE     DDS_ENCAPSULATION_ID_PL_CDR_BE
#endif
#define DDS_ENCAPSULATION_ID_CDR                 DDS_ENCAPSULATION_ID_CDR_NATIVE
#define DDS_ENCAPSULATION_ID_PL_CDR              DDS_ENCAPSULATION_ID_PL_CDR_NATIVE

/* XCDR2, CDRv2
 * From X-Types 1.2
 */
#define DDS_ENCAPSULATION_ID_XCDR2_F_BE         ((RTI_UINT16)0x0006)
#define DDS_ENCAPSULATION_ID_XCDR2_F_LE         ((RTI_UINT16)0x0007)
#define DDS_ENCAPSULATION_ID_XCDR2_A_BE         ((RTI_UINT16)0x0008)
#define DDS_ENCAPSULATION_ID_XCDR2_A_LE         ((RTI_UINT16)0x0009)
#define DDS_ENCAPSULATION_ID_XCDR2_M_BE         ((RTI_UINT16)0x000a)
#define DDS_ENCAPSULATION_ID_XCDR2_M_LE         ((RTI_UINT16)0x000b)
#define DDS_ENCAPSULATION_ID_XCDR2_XML          ((RTI_UINT16)0x0004)

#ifdef RTI_ENDIAN_LITTLE
#define DDS_ENCAPSULATION_ID_XCDR2_F_NATIVE     DDS_ENCAPSULATION_ID_XCDR2_F_LE
#define DDS_ENCAPSULATION_ID_XCDR2_A_NATIVE     DDS_ENCAPSULATION_ID_XCDR2_A_LE
#define DDS_ENCAPSULATION_ID_XCDR2_M_NATIVE     DDS_ENCAPSULATION_ID_XCDR2_M_LE
#else
#define DDS_ENCAPSULATION_ID_XCDR2_F_NATIVE     DDS_ENCAPSULATION_ID_XCDR2_F_BE
#define DDS_ENCAPSULATION_ID_XCDR2_A_NATIVE     DDS_ENCAPSULATION_ID_XCDR2_A_BE
#define DDS_ENCAPSULATION_ID_XCDR2_M_NATIVE     DDS_ENCAPSULATION_ID_XCDR2_M_BE
#endif
#define DDS_ENCAPSULATION_ID_XCDR2_XML_NATIVE   DDS_ENCAPSULATION_ID_XCDR2_XML

#define DDS_ENCAPSULATION_ID_XCDR2_F       DDS_ENCAPSULATION_ID_XCDR2_F_NATIVE
#define DDS_ENCAPSULATION_ID_XCDR2_A       DDS_ENCAPSULATION_ID_XCDR2_A_NATIVE
#define DDS_ENCAPSULATION_ID_XCDR2_M       DDS_ENCAPSULATION_ID_XCDR2_M_NATIVE

/* Shmem references - language binding plain
 * This representation sends a reference to a sample in a shared memory segment
 */
#define DDS_ENCAPSULATION_ID_SHMEM_REF_PLAIN_BE             ((RTI_UINT16)0xC000)
#define DDS_ENCAPSULATION_ID_SHMEM_REF_PLAIN_LE             ((RTI_UINT16)0xC000)
#ifdef RTI_ENDIAN_LITTLE
#define DDS_ENCAPSULATION_ID_SHMEM_REF_PLAIN_NATIVE           DDS_ENCAPSULATION_ID_SHMEM_REF_PLAIN_LE
#else
#define DDS_ENCAPSULATION_ID_SHMEM_REF_PLAIN_NATIVE           DDS_ENCAPSULATION_ID_SHMEM_REF_PLAIN_BE
#endif
#define DDS_ENCAPSULATION_ID_SHMEM_REF_PLAIN              DDS_ENCAPSULATION_ID_SHMEM_REF_PLAIN_NATIVE

#define DDS_ENCAPSULATION_ID_SHMEM_REF_FLAT_DATA_BE         ((RTI_UINT16)0xC001)
#define DDS_ENCAPSULATION_ID_SHMEM_REF_FLAT_DATA_LE         ((RTI_UINT16)0xC001)
#ifdef RTI_ENDIAN_LITTLE
#define DDS_ENCAPSULATION_ID_SHMEM_REF_FLAT_DATA_NATIVE  DDS_ENCAPSULATION_ID_SHMEM_REF_FLAT_DATA_LE
#else
#define DDS_ENCAPSULATION_ID_SHMEM_REF_FLAT_DATA_NATIVE  DDS_ENCAPSULATION_ID_SHMEM_REF_FLAT_DATA_BE
#endif
#define DDS_ENCAPSULATION_ID_SHMEM_REF_FLAT_DATA     DDS_ENCAPSULATION_ID_SHMEM_REF_FLAT_DATA_NATIVE

#define DDS_ENCAPSULATION_ID_SHMEM_V2                      ((RTI_UINT16)0xC003)

/* IB references
 * This representation sends a QP to RDMA data (not supported)
 */
#define DDS_ENCAPSULATION_ID_IB_RDMA_READ_BE  ((RTI_UINT16)0x0083)
#define DDS_ENCAPSULATION_ID_IB_RDMA_READ_LE  ((RTI_UINT16)0x0084)
#ifdef RTI_ENDIAN_LITTLE
#define DDS_ENCAPSULATION_ID_IB_RDMA_NATIVE   DDS_ENCAPSULATION_ID_IB_RDMA_READ_LE
#else
#define DDS_ENCAPSULATION_ID_IB_RDMA_NATIVE   DDS_ENCAPSULATION_ID_IB_RDMA_READ_BE
#endif
#define DDS_ENCAPSULATION_ID_IB_RDMA          DDS_ENCAPSULATION_ID_IB_RDMA_NATIVE

/* Memory references
 * This representation is used for the intra transport, the reference is
 * a pointer to the local memory. When sending a sample for encapsulation
 * not serialization shall be performed.
 */
#define DDS_ENCAPSULATION_ID_MEMORY           ((RTI_UINT16)0x0085)

#define DDS_ENCAPSULATION_ID_INVALID          ((RTI_UINT16)0xffff)

#define DDS_ENCAPSULATION_ID_RESOLVE          ((RTI_UINT16)0xffff)

#define T DDS_EncapsulationId_t
#define TSeq DDS_EncapsulationIdSeq
#define REDA_SEQUENCE_USER_API
#define TSeq_is_equal
#include "reda/reda_sequence_decl.h"

struct DDS_TransportEncapsulationSettings_t
{
    /*i \dref_TransportEncapsulationSettings_t_transports
    */
    struct DDS_StringSeq transports;

    /*i \dref_TransportEncapsulationSettings_t_encapsulations
    */
    struct DDS_EncapsulationIdSeq encapsulations;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_TransportEncapsulationSettings_t)
};

#define T struct DDS_TransportEncapsulationSettings_t
#define TSeq DDS_TransportEncapsulationSettingsSeq
#define REDA_SEQUENCE_USER_API
#define TSeq_is_equal
#include "reda/reda_sequence_decl.h"

struct DDSCPPDllExport DDS_TransportEncapsulationQosPolicy
{
    /*i \dref_TransportEncapsulationQosPolicy_value
     */
    struct DDS_TransportEncapsulationSettingsSeq value;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_TransportEncapsulationQosPolicy)
};

#define DDS_TRANSPORT_ENCAPSULATION_QOS_POLICY_DEFAULT \
{ \
    REDA_DEFINE_SEQUENCE_INITIALIZER(struct DDS_TransportEncapsulationSettings_t) \
}

DDSC_QOS_POLICY_METHODS_DECL(DDS_TransportEncapsulationQosPolicy)

/* ----------------------------------------------------------------- */
/*                DATA_REPRESENTATION_QOS_POLICY                     */
/* ----------------------------------------------------------------- */
/*e \dref_DataRepresentationQosGroupDocs */

/*e \dref_DataRepresentationId_t
 */
typedef DDS_Short DDS_DataRepresentationId_t;

/*e \dref_XCDR_DATA_REPRESENTATION
 */
#define DDS_XCDR_DATA_REPRESENTATION    ((DDS_DataRepresentationId_t)0)

/*e \dref_XML_DATA_REPRESENTATION
 */
#define DDS_XML_DATA_REPRESENTATION     ((DDS_DataRepresentationId_t)1)

/*e \dref_XCDR2_DATA_REPRESENTATION
 */
#define DDS_XCDR2_DATA_REPRESENTATION   ((DDS_DataRepresentationId_t)2)


/*i
 */
#define DDS_INVALID_DATA_REPRESENTATION ((DDS_DataRepresentationId_t)-2)

/*e \dref_AUTO_DATA_REPRESENTATION
 */
#define DDS_AUTO_DATA_REPRESENTATION    ((DDS_DataRepresentationId_t)-1)

/*i \dref_XML_DATA_REPRESENTATION
 */
#define DDS_DATA_REPRESENTATION_COUNT   (3)

/*e \dref_DataRepresentationId_t
 */
#define T DDS_DataRepresentationId_t
#define TSeq DDS_DataRepresentationIdSeq
#define REDA_SEQUENCE_USER_API
#define TSeq_is_equal
#include "reda/reda_sequence_decl.h"

/*e \dref_DataRepresentationQosPolicy
 */
struct DDSCPPDllExport DDS_DataRepresentationQosPolicy
{
    /*i \dref_DataRepresentationQosPolicy_value
     */
    struct DDS_DataRepresentationIdSeq value;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_DataRepresentationQosPolicy)
};

/*i
 */
#define DDS_DATA_REPRESENTATION_QOS_POLICY_DEFAULT \
{ \
    REDA_DEFINE_SEQUENCE_INITIALIZER(DDS_DataRepresentationId_t) \
}

/*ci
 * \def DDS_DataRepresentationIdSeq
 * \brief Initializer for DDS_DataRepresentationIdSeq variables with loaned buffer
 */
#define DDS_DataRepresentationIdSeq_INITIALIZER_W_LOAN(b_, m_, l_) \
REDA_DEFINE_SEQUENCE_INITIALIZER_W_LOAN(b_, m_, l_, DDS_DataRepresentationId_t)

DDSC_QOS_POLICY_METHODS_DECL(DDS_DataRepresentationQosPolicy)

extern DDSCDllVariable const struct DDS_DataRepresentationQosPolicy DDS_DataRepresentationQosPolicy_gv_V2;

extern DDSCDllVariable const struct DDS_DataRepresentationQosPolicy DDS_DataRepresentationQosPolicy_gv_V2_V1;

/* ----------------------------------------------------------------- */
/*                RESOURCE_LIMITS                                    */
/* ----------------------------------------------------------------- */
/*e \dref_ResourceLimitsQosGroupDocs
 */

/*e \dref_LENGTH_UNLIMITED
 */
extern DDSCDllVariable const DDS_Long DDS_LENGTH_UNLIMITED;

/*e \dref_LENGTH_AUTO
 */
extern DDSCDllVariable const DDS_Long DDS_LENGTH_AUTO;

/*e \dref_SIZE_AUTO
 */
extern DDSCDllVariable const DDS_Long DDS_SIZE_AUTO;

/* ----------------------------------------------------------------- */
/*e \dref_ResourceLimitsQosPolicy
 */
struct DDSCPPDllExport DDS_ResourceLimitsQosPolicy
{
    /*e \dref_ResourceLimitsQosPolicy_max_samples
     */
    DDS_Long max_samples;

    /*e \dref_ResourceLimitsQosPolicy_max_instances
     */
    DDS_Long max_instances;

    /*e \dref_ResourceLimitsQosPolicy_max_samples_per_instance
     */
    DDS_Long max_samples_per_instance;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_ResourceLimitsQosPolicy)
};

/*i \dref_ResourceLimitsQosPolicy_DEFAULT
 */
#define DDS_RESOURCE_LIMITS_QOS_POLICY_DEFAULT { \
    1L, /* max_samples */ \
    1L, /* max_instances */ \
    1L, /* max_samples_per_instance */ \
}

DDSC_QOS_POLICY_METHODS_DECL(DDS_ResourceLimitsQosPolicy)

/* ----------------------------------------------------------------- */
/*                PRESENTATION                                       */
/* ----------------------------------------------------------------- */
/*e \dref_PresentationQosGroupDocs
 */
/* ----------------------------------------------------------------- */
/*e \dref_PresentationQosPolicyAccessScopeKind
 */
typedef enum
{
    /*e \dref_PresentationQosPolicyAccessScopeKind_INSTANCE_PRESENTATION_QOS
     */
    DDS_INSTANCE_PRESENTATION_QOS,

    /*e \dref_PresentationQosPolicyAccessScopeKind_TOPIC_PRESENTATION_QOS
     */
    DDS_TOPIC_PRESENTATION_QOS,

    /*e \dref_PresentationQosPolicyAccessScopeKind_GROUP_PRESENTATION_QOS
     */
    DDS_GROUP_PRESENTATION_QOS,

    /*i \dref_PresentationQosPolicyAccessScopeKind_HIGHEST_OFFERED_PRESENTATION_QOS
     */
    DDS_HIGHEST_OFFERED_PRESENTATION_QOS
} DDS_PresentationQosPolicyAccessScopeKind;

/* ----------------------------------------------------------------- */
/*e \dref_PresentationQosPolicy
 */
struct DDS_PresentationQosPolicy
{
    /*e \dref_PresentationQosPolicy_access_scope
     */
    DDS_PresentationQosPolicyAccessScopeKind access_scope;

    /*e \dref_PresentationQosPolicy_coherent_access
     */
    DDS_Boolean coherent_access;

    /*e \dref_PresentationQosPolicy_ordered_access
     */
    DDS_Boolean ordered_access;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_PresentationQosPolicy)
};

/*i \dref_PresentationQosPolicy_DEFAULT
 */
#define DDS_PRESENTATION_QOS_POLICY_DEFAULT \
{ DDS_INSTANCE_PRESENTATION_QOS, DDS_BOOLEAN_FALSE, DDS_BOOLEAN_FALSE }

DDSC_QOS_POLICY_METHODS_DECL(DDS_PresentationQosPolicy)

/*ci
 */
extern DDSCDllVariable const struct DDS_PresentationQosPolicy DDS_PRESENTATION_QOS_PUBLICATION_DEFAULT;

/*ci
 */
#define DDS_PRESENTATION_QOS_POLICY_PUBLICATION_DEFAULT \
{ \
    DDS_TOPIC_PRESENTATION_QOS, /* access_scope */ \
    RTI_FALSE,                  /* coherent_access */ \
    RTI_TRUE                    /* ordered_access */ \
}

/* ----------------------------------------------------------------- */
/*                DESTINATION_ORDER                                  */
/* ----------------------------------------------------------------- */
/*e \dref_DestinationOrderQosGroupDocs
 */
/*e \dref_DestinationOrderPolicyKind
*/
typedef enum
{
    /*e \dref_DestinationOrderPolicyKind_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS
     */
    DDS_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS,

    /*e \dref_DestinationOrderPolicyKind_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS
     */
    DDS_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS
} DDS_DestinationOrderQosPolicyKind;

/*e \dref_DestinationOrderQosPolicy
 */
struct DDSCPPDllExport DDS_DestinationOrderQosPolicy
{
    /*e \dref_DestinationOrderQosPolicy_kind
     */
    DDS_DestinationOrderQosPolicyKind kind;

    /*e \dref_DestinationOrderQosPolicy_source_timestamp_tolerance
     */
    struct DDS_Duration_t source_timestamp_tolerance;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_DestinationOrderQosPolicy)
};

/*i \dref_DestinationOrderQosPolicy_DEFAULT
 */
#define DDS_DESTINATION_ORDER_QOS_POLICY_DEFAULT \
{ \
        DDS_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS, \
        {0,100000000} \
}

DDSC_QOS_POLICY_METHODS_DECL(DDS_DestinationOrderQosPolicy)

/* ----------------------------------------------------------------- */
/*e \dref_DataReaderResourceLimitsInstanceReplacementKind
*/
typedef enum
{
    /*e \dref_DataReaderResourceLimitsInstanceReplacementKind_NO_INSTANCE_REPLACEMENT_QOS
     */
    DDS_NO_INSTANCE_REPLACEMENT_QOS,

    /*e \dref_DataReaderResourceLimitsInstanceReplacementKind_REPLACE_OLDEST_INSTANCE_REPLACEMENT_QOS
     */
    DDS_REPLACE_OLDEST_INSTANCE_REPLACEMENT_QOS
} DDS_DataReaderResourceLimitsInstanceReplacementKind;

/* ----------------------------------------------------------------- */
/*                DATAREADER_RESOURCE_LIMITS                         */
/* ----------------------------------------------------------------- */
/*e \dref_DataReaderResourceLimitsQosGroupDocs
 */

/* ----------------------------------------------------------------- */
/*e \dref_DataReaderResourceLimitsQosPolicy
 */
struct DDSCPPDllExport DDS_DataReaderResourceLimitsQosPolicy
{
    /*e \dref_DataReaderResourceLimitsQosPolicy_max_remote_writers
     */
    DDS_Long max_remote_writers;

    /*e \dref_DataReaderResourceLimitsQosPolicy_max_remote_writers_per_instance
     */
    DDS_Long max_remote_writers_per_instance;

    /*e \dref_DataReaderResourceLimitsQosPolicy_max_samples_per_remote_writer
     */
    DDS_Long max_samples_per_remote_writer;

    /*e \dref_DataReaderResourceLimitsQosPolicy_max_outstanding_reads
     */
    DDS_Long max_outstanding_reads;

    /*e \dref_DataReaderResourceLimitsQosPolicy_instance_replacement
     */
    DDS_DataReaderResourceLimitsInstanceReplacementKind instance_replacement;

    /*e \dref_DataReaderResourceLimitsQosPolicy_max_routes_per_writer
     */
    DDS_Long max_routes_per_writer;

    /*e \dref_DataReaderResourceLimitsQosPolicy_max_fragmented_samples
     */
    DDS_Long max_fragmented_samples;

    /*e \dref_DataReaderResourceLimitsQosPolicy_max_fragmented_samples_per_remote_writer
     */
    DDS_Long max_fragmented_samples_per_remote_writer;

    /*e \dref_DataReaderResourceLimitsQosPolicy_shmem_ref_transfer_mode_attached_segment_allocation
     */
    DDS_Long shmem_ref_transfer_mode_attached_segment_allocation;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_DataReaderResourceLimitsQosPolicy)
};

/*i \dref_DataReaderResourceLimitsQosPolicy_DEFAULT
 */
#define DDS_DATAREADERRESOURCE_LIMITS_QOS_POLICY_DEFAULT { \
    1L, /* max_remote_writers */ \
    1L, /* max_remote_writers_per_instance */ \
    1L, /* max_samples_per_remote_writer */ \
    1L, /* max_outstanding_reads */ \
    DDS_NO_INSTANCE_REPLACEMENT_QOS, /* Default */ \
    4,\
    DDS_MAX_AUTO,\
    DDS_MAX_AUTO,\
    DDS_SIZE_AUTO \
}

DDSC_QOS_POLICY_METHODS_DECL(DDS_DataReaderResourceLimitsQosPolicy)

/* ----------------------------------------------------------------- */
/*                DATAWRITER_RESOURCE_LIMITS                         */
/* ----------------------------------------------------------------- */
/*e \dref_DataWriterResourceLimitsQosGroupDocs
 */

/* ----------------------------------------------------------------- */
/*e \dref_DataWriterResourceLimitsQosPolicy
 */
struct DDSCPPDllExport DDS_DataWriterResourceLimitsQosPolicy
{
    /*e \dref_DataWriterResourceLimitsQosPolicy_max_remote_readers
     */
    DDS_Long max_remote_readers;

    /*e \dref_DataWriterResourceLimitsQosPolicy_max_routes_per_reader
     */
    DDS_Long max_routes_per_reader;

    /*e \dref_DataWriterResourceLimitsQosPolicy_writer_loaned_sample_allocation
     */
    DDS_Long writer_loaned_sample_allocation;

    /*e \dref_DataWriterResourceLimitsQosPolicy_initialize_writer_loaned_sample
     */
    DDS_Boolean initialize_writer_loaned_sample;

#if DDS_FILTERING_ENABLED
    /*e \dref_DataWriterResourceLimitsQosPolicy_max_remote_reader_filters
     */
    DDS_Long max_remote_reader_filters;
#endif

    DDSC_CPP_QOS_POLICY_METHODS(DDS_DataWriterResourceLimitsQosPolicy)
};

#if DDS_FILTERING_ENABLED
#define DDS_DATAWRITERRESOURCE_LIMITS_MAX_REMOTE_READER_FILTERS_DEFAULT ,DDS_LENGTH_UNLIMITED
#else
#define DDS_DATAWRITERRESOURCE_LIMITS_MAX_REMOTE_READER_FILTERS_DEFAULT
#endif

/*i \dref_DataWriterResourceLimitsQosPolicy_DEFAULT
 */
#define DDS_DATAWRITERRESOURCE_LIMITS_QOS_POLICY_DEFAULT {  \
    16L,                           /* max_remote_readers */ \
    4,                          /* max_routes_per_reader */ \
    DDS_SIZE_AUTO,    /* writer_loaned_sample_allocation */ \
    DDS_BOOLEAN_FALSE /* initialize_writer_loaned_sample */ \
    DDS_DATAWRITERRESOURCE_LIMITS_MAX_REMOTE_READER_FILTERS_DEFAULT \
}

DDSC_QOS_POLICY_METHODS_DECL(DDS_DataWriterResourceLimitsQosPolicy)

/* ----------------------------------------------------------------- */
/*                ENTITY_FACTORY                                     */
/* ----------------------------------------------------------------- */
/*e \dref_EntityFactoryQosGroupDocs
 */

/* ----------------------------------------------------------------- */
/*e \dref_EntityFactoryQosPolicy
 */
struct DDSCPPDllExport DDS_EntityFactoryQosPolicy
{
    /*e \dref_EntityFactoryQosPolicy_autoenable_created_entities
     */
    DDS_Boolean autoenable_created_entities;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_EntityFactoryQosPolicy)
};

/*i \dref_EntityFactoryQosPolicy_DEFAULT
 */
#define DDS_ENTITY_FACTORY_QOS_POLICY_DEFAULT   { DDS_BOOLEAN_TRUE }

DDSC_QOS_POLICY_METHODS_DECL(DDS_EntityFactoryQosPolicy)

/* ----------------------------------------------------------------- */
/*                EXTENDED QOS SUPPORT                               */
/* ----------------------------------------------------------------- */

/* ----------------------------------------------------------------- */
/*e
 * \dref_ExtendedQosSupportGroupDocs
 */
#define DDS_LENGTH_UNLIMITED   (-1)

/*ci \brief Value to indicate that the length should be determined by the middleware, when
 *          supported
 */
#define DDS_LENGTH_AUTO        (-2)

/*ci \brief A value to indicate that the size of something should be determined by the
 *   middleware, if supported.
 */
#define DDS_SIZE_AUTO          (-2)

/*ci \brief A value to indicate that the maximum size of something should be determined
 *   by the middleware, if supported
 */
#define DDS_MAX_AUTO           (-2)

/*ci \brief A value to indicate that the maximum is unlimited as defined by the middleware,
 *          if supported.
 */
#define DDS_MAX_UNLIMITED      (-1)

/* ----------------------------------------------------------------- */
/*                DDS_DataWriterTransferModeQosPolicy                */
/* ----------------------------------------------------------------- */

/*e \dref_DataWriterShmemRefTransferModeSettings
 */
struct DDS_DataWriterShmemRefTransferModeSettings
{
    /*e \dref_DataWriterShmemRefTransferModeSettings_enable_data_consistency_check
     */
    DDS_Boolean enable_data_consistency_check;
};

#define DDS_DataWriterShmemRefTransferModeSettings_INITIALIZER \
{\
    DDS_BOOLEAN_TRUE\
}

/* ----------------------------------------------------------------- */
/*e \dref_DataWriterTransferModeQosPolicy
 */
struct DDS_DataWriterTransferModeQosPolicy
{
    /*e \dref_DataWriterTransferModeQosPolicy_shmem_ref_settings
     */
    struct DDS_DataWriterShmemRefTransferModeSettings shmem_ref_settings;
};

#define DDS_DataWriterTransferModeQosPolicy_INITIALIZER \
{\
    DDS_DataWriterShmemRefTransferModeSettings_INITIALIZER \
}


/* ----------------------------------------------------------------- */

/* ----------------------------------------------------------------- */
/*                TYPESUPPORT                                        */
/* ----------------------------------------------------------------- */
/*i \dref_TypeSupportQosGroupDocs
*/

/* ----------------------------------------------------------------- */
/*i \dref_TypeSupportQosPolicy
 */
struct DDSCPPDllExport DDS_TypeSupportQosPolicy
{
    /*e \dref_TypeSupportQosPolicy_plugin_data
     */
    void *plugin_data;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_TypeSupportQosPolicy)
};

/*i \dref_TypeSupportQosPolicy_DEFAULT
 */
#define DDS_TYPESUPPORT_QOS_POLICY_DEFAULT \
{\
    NULL \
}

DDSC_QOS_POLICY_METHODS_DECL(DDS_TypeSupportQosPolicy)

/* ----------------------------------------------------------------- */
/*             SYSTEM_RESOURCE_LIMITS_X (eXtension QoS)              */
/* ----------------------------------------------------------------- */
/*e \dref_SystemResourceLimitsQosGroupDocs
 */

/* ----------------------------------------------------------------- */

/*e \dref_SystemResourceLimitsQosPolicy
 */
struct DDSCPPDllExport DDS_SystemResourceLimitsQosPolicy
{
    /*e \dref_SystemResourceLimitsQosPolicy_max_participants
     */
    DDS_Long max_participants;

    /*e \dref_SystemResourceLimitsQosPolicy_max_components
     */
    DDS_Long max_components;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_SystemResourceLimitsQosPolicy)
};

/*i \dref_SystemResourceLimitsQosPolicy_DEFAULT
 */
#if UDP_TRANSFORMS_ENABLED
#define DDS_SYSTEM_RESOURCE_LIMITS_QOS_POLICY_DEFAULT \
{ \
    1L,\
    64L \
}
#else
#define DDS_SYSTEM_RESOURCE_LIMITS_QOS_POLICY_DEFAULT \
{ \
    1L,\
    16L\
}
#endif /* UDP_TRANSFORMS_ENABLED */

DDSC_QOS_POLICY_METHODS_DECL(DDS_SystemResourceLimitsQosPolicy)

struct DDS_ProfileQosPolicy
{
    /*\e
     */
    struct DDS_StringSeq    string_profile;

    /*\e
     */
    struct DDS_StringSeq    url_profile;

    /*\e
     */
    DDS_Boolean ignore_user_profile;

    /*\e
     */
    DDS_Boolean ignore_environment_profile;

    /*\e
     */
    DDS_Boolean ignore_resource_profile;
};

#define DDS_ProfileQosPolicy_INITIALIZER \
{\
    DDS_SEQUENCE_INITIALIZER, \
    DDS_SEQUENCE_INITIALIZER, \
    DDS_BOOLEAN_TRUE, \
    DDS_BOOLEAN_TRUE, \
    DDS_BOOLEAN_TRUE  \
}

#define DDS_PROFILE_QOS_POLICY_DEFAULT DDS_ProfileQosPolicy_INITIALIZER

DDSC_QOS_POLICY_METHODS_DECL(DDS_ProfileQosPolicy)

/* ----------------------------------------------------------------- */
/*                WIRE_PROTOCOL_X (eXtension QoS)                    */
/* ----------------------------------------------------------------- */
/*e \dref_WireProtocolQosGroupDocs
 */

#define DDS_RtpsWellKnownPorts NETIO_RtpsPortParam

/*e \dref_RTI_BACKWARDS_COMPATIBLE_RTPS_WELL_KNOWN_PORTS
 */
extern DDSCDllVariable struct DDS_RtpsWellKnownPorts_t
                            DDS_RTI_BACKWARDS_COMPATIBLE_RTPS_WELL_KNOWN_PORTS;

/*e \dref_INTEROPERABLE_RTPS_WELL_KNOWN_PORTS
 */
extern DDSCDllVariable struct DDS_RtpsWellKnownPorts_t
                                        DDS_INTEROPERABLE_RTPS_WELL_KNOWN_PORTS;

/*i @ingroup DDSWireProtocolQosModule
    these defaults are compatible with >= 4.2d
 */

#ifdef DOXYGEN_DOCUMENTATION_ONLY
/*e \dref_RtpsWellKnownPorts_t
 */
struct DDS_RtpsWellKnownPorts_t
{
    /*e \dref_RtpsWellKnownPorts_t_port_base
    */
    DDS_Long port_base;

    /*e \dref_RtpsWellKnownPorts_t_domain_id_gain
    */
    DDS_Long domain_id_gain;

    /*e \dref_RtpsWellKnownPorts_t_participant_id_gain
    */
    DDS_Long participant_id_gain;

    /*e \dref_RtpsWellKnownPorts_t_builtin_multicast_port_offset
    */
    DDS_Long builtin_multicast_port_offset;

    /*e \dref_RtpsWellKnownPorts_t_builtin_unicast_port_offset
    */
    DDS_Long builtin_unicast_port_offset;

    /*e \dref_RtpsWellKnownPorts_t_user_multicast_port_offset
    */
    DDS_Long user_multicast_port_offset;

    /*e \dref_RtpsWellKnownPorts_t_user_unicast_port_offset
    */
    DDS_Long user_unicast_port_offset;
};

#endif /* DOXYGEN_DOCUMENTATION_ONLY */

#define DDS_RtpsWellKnownPorts_t DDS_RtpsWellKnownPorts

/*i \dref_RtpsWellKnownPorts_DEFAULT
 */
#define DDS_RTPS_WELL_KNOWN_PORTS_DEFAULT \
{ \
    7400, /* port_base */ \
    250, /* domain_id_gain */ \
    2, /* participant_id_gain */ \
    0, /* builtin_multicast_port_offset */ \
    10, /* builtin_unicast_port_offset */ \
    1, /* user_multicast_port_offset */ \
    11 /* user_unicast_port_offset */ \
}

/*e \dref_ChecksumKind_t
 */
typedef DDS_UnsignedShort DDS_ChecksumKind_t;

/*e \dref_ChecksumKindMask_t
 */
typedef DDS_UnsignedShort DDS_ChecksumKindMask_t;

/*e \dref_DDS_CHECKSUM_NONE
 */
#define DDS_CHECKSUM_NONE   (0)

/*e \dref_DDS_CHECKSUM_AUTO
 */
#define DDS_CHECKSUM_AUTO       (0xffffU)

/*e \dref_DDS_CHECKSUM_BUILTIN32
 */
#define DDS_CHECKSUM_BUILTIN32  (0x1U)

/*e \dref_DDS_CHECKSUM_BUILTIN64
 */
#define DDS_CHECKSUM_BUILTIN64  (0x2U)

/*e \dref_DDS_CHECKSUM_BUILTIN128
 */
#define DDS_CHECKSUM_BUILTIN128 (0x4U)

/*i \dref_DDS_CHECKSUM_MASK_MAX
 */
#define DDS_CHECKSUM_MASK_MAX (0x7)

/*i \dref_DDS_CHECKSUM_BUILTIN_ANY
 */
#define DDS_CHECKSUM_BUILTIN_ANY \
    (DDS_CHECKSUM_BUILTIN128 | DDS_CHECKSUM_BUILTIN32 | DDS_CHECKSUM_BUILTIN64)

/* ----------------------------------------------------------------- */
/*e \dref_WireProtocolQosPolicy
 */
struct DDSCPPDllExport DDS_WireProtocolQosPolicy
{
    /*e \dref_WireProtocolQosPolicy_participant_id
     */
    DDS_Long participant_id;

    /*e \dref_WireProtocolQosPolicy_rtps_host_id
     */
    DDS_UnsignedLong rtps_host_id;

    /*e \dref_WireProtocolQosPolicy_rtps_app_id
     */
    DDS_UnsignedLong rtps_app_id;

    /*e \dref_WireProtocolQosPolicy_rtps_instance_id
     */
    DDS_UnsignedLong rtps_instance_id;

    /*e \dref_WireProtocolQosPolicy_rtps_well_known_ports
     */
    struct DDS_RtpsWellKnownPorts_t rtps_well_known_ports;

    /*e \dref_WireProtocolQosPolicy_compute_crc
     */
    DDS_Boolean compute_crc;

    /*e \dref_WireProtocolQosPolicy_check_crc
     */
    DDS_Boolean check_crc;

    /*e \dref_WireProtocolQosPolicy_require_crc
     */
    DDS_Boolean require_crc;

    /*e \dref_WireProtocolQosPolicy_computed_crc_kind
     */
    DDS_ChecksumKind_t computed_crc_kind;

    /*e \dref_WireProtocolQosPolicy_allowed_crc_mask
     */
    DDS_ChecksumKindMask_t allowed_crc_mask;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_WireProtocolQosPolicy)
};

/*e
 */
enum
{
    /*e \dref_WireProtocolQosPolicy_RTPS_AUTO_ID
     */
    DDS_RTPS_AUTO_ID = 0
};

/*i \dref_WireProtocolQosPolicy_DEFAULT
 */
#define DDS_WIRE_PROTOCOL_QOS_POLICY_DEFAULT {      \
-1 /* auto participant_id */,                       \
DDS_RTPS_AUTO_ID /* rtps_host_id */,                \
DDS_RTPS_AUTO_ID /* rtps_app_id */,                 \
DDS_RTPS_AUTO_ID /* rtps_instance_id */,            \
DDS_RTPS_WELL_KNOWN_PORTS_DEFAULT, /* rtps_well_known_ports */\
DDS_BOOLEAN_FALSE,\
DDS_BOOLEAN_FALSE,\
DDS_BOOLEAN_FALSE,\
DDS_CHECKSUM_AUTO,\
DDS_CHECKSUM_AUTO\
}

DDSC_QOS_POLICY_METHODS_DECL(DDS_WireProtocolQosPolicy)

/*e \dref_ChecksumProperty_t
 */
struct DDSCPPDllExport DDS_ChecksumProperty
{
    /*e \dref_ChecksumProperty_t_computed_crc_kind
     */
    DDS_ChecksumKind_t computed_crc_kind;

    /*e \dref_ChecksumProperty_t_allowed_crc_mask
     */
    DDS_ChecksumKindMask_t allowed_crc_mask;

    /*e \dref_ChecksumProperty_t_require_crc
     */
    DDS_Boolean require_crc;
};

#ifdef DOXYGEN_DOCUMENTATION_ONLY

/*e \dref_ChecksumProperty_t
 */
struct DDS_ChecksumProperty_t
{
    /*e \dref_ChecksumProperty_t_computed_crc_kind
     */
    DDS_ChecksumKind_t computed_crc_kind;

    /*e \dref_ChecksumProperty_t_allowed_crc_mask
     */
    DDS_ChecksumKindMask_t allowed_crc_mask;

    /*e \dref_ChecksumProperty_t_require_crc
     */
    DDS_Boolean require_crc;
};
#endif

#define DDS_ChecksumPropertyWireProtocol_INITIALIZER \
{\
    DDS_CHECKSUM_NONE,\
    DDS_CHECKSUM_NONE,\
    DDS_BOOLEAN_FALSE\
}

#define DDS_ChecksumProperty_t DDS_ChecksumProperty

/* ----------------------------------------------------------------- */

/*i
 * Alias for DDS_Locator
 */
#define DDS_Locator RTPS_Locator

/*i
 * Alias for DDS_Locator_t
 */
#define DDS_Locator_t RTPS_Locator_t

#define DDS_Locator_as_netioaddress RTPS_Locator_as_netioaddress
#define DDS_Locator_const_cast(t_,v_) (const t_ *const) v_
#define DDS_Locator_cast(t_,v_) (t_ *) v_

/*i
 * Alias for DDS_LocatorUdpv4_t
 */
#define DDS_LocatorUdpv4_t RTPS_LocatorUdpv4_t

#ifdef DOXYGEN_DOCUMENTATION_ONLY

/*e \dref_Locator_t
 */
struct DDS_Locator_t
{
    /*e \dref_Locator_t_kind
     */
    DDS_Long kind;

    /*e \dref_Locator_t_port
     */
    DDS_UnsignedLong port;

    /*e \dref_Locator_t_address
     */
    DDS_Octet address[RTPS_LOCATOR_ADDRESS_LENGTH_MAX];
};

#endif                          /*DOXYGEN_DOCUMENTATION_ONLY */

#define T struct RTPS_Locator
#define TSeq DDS_LocatorSeq
#define REDA_SEQUENCE_USER_API
#define TSeq_is_equal
#include <reda/reda_sequence_decl.h>

/*ci
 * \def_DDS_LocatorSeq_INITIALIZER
 * \brief Initializer for DDS_LocatorSeq variables
 */
#define DDS_LocatorSeq_INITIALIZER \
REDA_DEFINE_SEQUENCE_INITIALIZER(struct RTPS_Locator*)

/*ci
 * \def_DDS_LocatorSeq_INITIALIZER_W_LOAN
 * \brief Initializer for DDS_LocatorSeq variables with loaned buffer
 */
#define DDS_LocatorSeq_INITIALIZER_W_LOAN(b_, m_, l_) \
REDA_DEFINE_SEQUENCE_INITIALIZER_W_LOAN(b_, m_, l_, struct RTPS_Locator*)

#ifdef DOXYGEN_DOCUMENTATION_ONLY
/*e \dref_LocatorSeq
 */
struct DDS_LocatorSeq {};
#endif

/*e
 * \dref_Locator_t_INVALID
 */
#define DDS_LOCATOR_INVALID RTPS_LOCATOR_INVALID

#ifndef RTI_CERT
/*e
 * \dref_Locator_t_DEFAULT
 */
#define DDS_LOCATOR_DEFAULT RTPS_LOCATOR_DEFAULT

/*e
 * \dref_Locator_t_KIND_INVALID
 */
#define DDS_LOCATOR_KIND_INVALID RTPS_LOCATOR_KIND_INVALID

/*e
 * \dref_Locator_t_PORT_INVALID
 */
#define DDS_LOCATOR_PORT_INVALID RTPS_LOCATOR_PORT_INVALID

/*e
 * \dref_Locator_t_ADDRESS_INVALID
 */
#define DDS_LOCATOR_ADDRESS_INVALID RTPS_LOCATOR_ADDRESS_INVALID

#endif /* !RTI_CERT */


/*e
 * \dref_Locator_t_KIND_UDPv4
 */
#define DDS_LOCATOR_KIND_UDPv4 RTPS_LOCATOR_KIND_UDPv4

/*e
 * \dref_Locator_t_KIND_UDPv6
 */
#define DDS_LOCATOR_KIND_UDPv6 RTPS_LOCATOR_KIND_UDPv6

/*e
 * \dref_Locator_t_KIND_RESERVED
 */
#define DDS_LOCATOR_KIND_RESERVED RTPS_LOCATOR_KIND_RESERVED

/*e
 * \dref_Locator_t_KIND_SHMEM
 */
#define DDS_LOCATOR_KIND_SHMEM RTPS_LOCATOR_KIND_SHMEM

/* ----------------------------------------------------------------- */

/*e \dref_ProtocolVersion_t
 */
typedef struct DDS_ProtocolVersion
{
    /*e \dref_ProtocolVersion_t_major
     */
    DDS_Octet major;

    /*e \dref_ProtocolVersion_t_minor
     */
    DDS_Octet minor;
} DDS_ProtocolVersion_t;

DDSC_VARIABLE_LENGTH_VALUE_TYPE_SUPPORT_BASIC(DDS_ProtocolVersion);

/*i \dref_ProtocolVersion_DEFAULT
 */
#define DDS_PROTOCOL_VERSION_DEFAULT { 0, 0 }

/*e \dref_ProtocolVersion_t_PROTOCOLVERSION_1_0
 */
#define DDS_PROTOCOLVERSION_1_0 { 1, 0 }
/*e \dref_ProtocolVersion_t_PROTOCOLVERSION_1_1
 */
#define DDS_PROTOCOLVERSION_1_1 { 1, 1 }
/*e \dref_ProtocolVersion_t_PROTOCOLVERSION_1_2
 */
#define DDS_PROTOCOLVERSION_1_2 { 1, 2 }
/*e \dref_ProtocolVersion_t_PROTOCOLVERSION
 */
#define DDS_PROTOCOLVERSION_2_0 { 2, 0 }
/*e \dref_ProtocolVersion_t_PROTOCOLVERSION_2_1
 */
#define DDS_PROTOCOLVERSION_2_1 { 2, 1 }
/*e \dref_ProtocolVersion_t_PROTOCOLVERSION
 */
#define DDS_PROTOCOLVERSION { 2, 5 }

/*e \dref_VendorId_t_LENGTH_MAX
 */
#define DDS_VENDOR_ID_LENGTH_MAX 2

/*e \dref_VendorId_t
 */
struct DDS_VendorId
{
    /*e \dref_VendorId_t_vendorId
     */
    DDS_Octet vendorId[DDS_VENDOR_ID_LENGTH_MAX];
};

#ifdef DOXYGEN_DOCUMENTATION_ONLY
/*e \dref_VendorId_t
 */
struct DDS_VendorId_t
{
    /*e \dref_VendorId_t_vendorId
     */
    DDS_Octet vendorId[DDS_VENDOR_ID_LENGTH_MAX];
};
#endif                          /*DOXYGEN_DOCUMENTATION_ONLY */


DDSC_VARIABLE_LENGTH_VALUE_TYPE_SUPPORT_BASIC(DDS_VendorId);
#define DDS_VendorId_t DDS_VendorId

/*i \dref_VendorId_dEFAULT
 */
#define DDS_VENDOR_ID_DEFAULT { {0, 0} }

/*i \dref_VendorId_t_VENDORID_UNKNOWN
 */
#define DDS_VENDORID_UNKNOWN { {0, 0} }

/* ----------------------------------------------------------------- */

/*e \dref_ProductVersion_t
 */
struct DDS_ProductVersion
{
    /*e \dref_ProductVersion_t_major
     */
    DDS_Char major;

    /*e \dref_ProductVersion_t_minor
     */
    DDS_Char minor;

    /*e \dref_ProductVersion_t_release
     */
    DDS_Char release;

    /*e \dref_ProductVersion_t_revision
     */
    DDS_Char revision;
};


#ifdef DOXYGEN_DOCUMENTATION_ONLY
/*e \dref_ProductVersion_t
 */
struct DDS_ProductVersion_t
{
    /*e \dref_ProductVersion_t_major
     */
    DDS_Char major;
    /*e \dref_ProductVersion_t_minor
     */
    DDS_Char minor;
    /*e \dref_ProductVersion_t_release
     */
    DDS_Char release;
    /*e \dref_ProductVersion_t_revision
     */
    DDS_Char revision;
};
#endif                          /*DOXYGEN_DOCUMENTATION_ONLY */

#define DDS_ProductVersion_t DDS_ProductVersion

DDSCDllExport DDS_Long
DDS_ProductVersion_compare(const struct DDS_ProductVersion* left,
                           const struct DDS_ProductVersion* right);

/*e \dref_ProductVersion_t_UNKNOWN
 */
#define DDS_PRODUCTVERSION_UNKNOWN { 0, 0, '0', 0 }

/*i \dref_ProductVersion_DEFAULT
 */
#define DDS_PRODUCTVERSION_DEFAULT { 2, 0, 'c', 1 }

#ifndef RTI_CERT

/*e \dref_DomainParticipant_C_Library_get_version
 */
DDSCDllExport const char*
DDSC_Library_get_version(void);
#endif

/* ----------------------------------------------------------------- */

/*e \dref_RtpsReliableReaderProtocol_t
 */
 struct DDSCPPDllExport DDS_RtpsReliableReaderProtocol_t
 {
    /*e \dref_RtpsReliableReaderProtocol_t_nack_period
     */
    struct DDS_Duration_t nack_period;
 };


/*i \dref_RtpsReliableWriterProtocol_DEFAULT
*/
#define DDS_RTPS_NACK_PERIOD_DEFAULT \
{\
  0,50 * 1000000 /* 50ms nack_period */ \
}

/*i \dref_RtpsReliableWriterProtocol_DEFAULT
 */
#define DDS_RTPS_RELIABLE_READER_PROTOCOL_DEFAULT \
{ \
    DDS_RTPS_NACK_PERIOD_DEFAULT /* nack_period */ \
}

/* ----------------------------------------------------------------- */
/*               DATA_READER_PROTOCOL_X (eXtension QoS)              */
/* ----------------------------------------------------------------- */
/*e \dref_DataReaderProtocolQosGroupDocs
 */

/* ----------------------------------------------------------------- */
/*e \dref_DataReaderProtocolQosPolicy
 */
struct DDSCPPDllExport DDS_DataReaderProtocolQosPolicy
{
    /*e \dref_DataReaderProtocolQosPolicy_rtps_object_id
     */
    DDS_UnsignedLong rtps_object_id;

    /*e \dref_DataReaderProtocolQosPolicy_rtps_reliable_reader
     */
    struct DDS_RtpsReliableReaderProtocol_t rtps_reliable_reader;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_DataReaderProtocolQosPolicy)
};

/*i \dref_DataReaderProtocolQosPolicy_DEFAULT
 */
#define DDS_DATA_READER_PROTOCOL_QOS_POLICY_DEFAULT \
{ \
  DDS_RTPS_AUTO_ID, \
  DDS_RTPS_RELIABLE_READER_PROTOCOL_DEFAULT \
}

DDSC_QOS_POLICY_METHODS_DECL(DDS_DataReaderProtocolQosPolicy)

/* ----------------------------------------------------------------- */

/*e \dref_RtpsReliableWriterProtocol_t
 */
 struct DDSCPPDllExport DDS_RtpsReliableWriterProtocol_t
 {
    /*e \dref_RtpsReliableWriterProtocol_t_heartbeat_period
     */
    struct DDS_Duration_t heartbeat_period;

    /*e \dref_RtpsReliableWriterProtocol_t_heartbeats_per_max_samples
     */
    DDS_Long heartbeats_per_max_samples;

    /*e \dref_RtpsReliableWriterProtocol_t_max_send_window
     */
    DDS_Long max_send_window;

    /*e \dref_RtpsReliableWriterProtocol_t_max_heartbeat_retries
     */
    DDS_Long max_heartbeat_retries;

    /*i \dref_RtpsReliableWriterProtocol_t_first_write_sequence_number
     */
    struct DDS_SequenceNumber_t first_write_sequence_number;
 };

#define DDS_RTPSRELIABLEWRITER_DEFAULT_SEND_WINDOW (DDS_LENGTH_UNLIMITED)

/*i \dref_RtpsReliableWriterProtocol_DEFAULT
 */
#define DDS_RTPS_RELIABLE_WRITER_PROTOCOL_DEFAULT { \
    {3,0}, /* hb_period */ \
    1, /* hb_per_max_samples */ \
    DDS_RTPSRELIABLEWRITER_DEFAULT_SEND_WINDOW, /* max_send_window */ \
    DDS_LENGTH_UNLIMITED, /* max_heartbeat_retries */ \
    {0,1} /* first_write_sequence_number */\
}

/* ----------------------------------------------------------------- */
/*               DATA_WRITER_PROTOCOL_X (eXtension QoS)              */
/* ----------------------------------------------------------------- */
/*e \dref_DataWriterProtocolQosGroupDocs
 */

/* ----------------------------------------------------------------- */
/*e \dref_DataWriterProtocolQosPolicy
 */
struct DDSCPPDllExport DDS_DataWriterProtocolQosPolicy
{
    /*e \dref_DataWriterProtocolQosPolicy_rtps_object_id
     */
    DDS_UnsignedLong rtps_object_id;

    /*e \dref_DataWriterProtocolQosPolicy_rtps_reliable_writer
     */
    struct DDS_RtpsReliableWriterProtocol_t rtps_reliable_writer;

    /*e \dref_DataWriterProtocolQosPolicy_serialize_on_write
     */
    DDS_Boolean serialize_on_write;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_DataWriterProtocolQosPolicy)
};

/*i \dref_DataWriterProtocolQosPolicy_DEFAULT
 */
#define DDS_DATA_WRITER_PROTOCOL_QOS_POLICY_DEFAULT \
{ DDS_RTPS_AUTO_ID, \
  DDS_RTPS_RELIABLE_WRITER_PROTOCOL_DEFAULT,\
  RTI_TRUE \
}

DDSC_QOS_POLICY_METHODS_DECL(DDS_DataWriterProtocolQosPolicy)

/* ----------------------------------------------------------------- */
/*                    TRANSPORT_QOS_POLICY (eXtension QoS)           */
/* ----------------------------------------------------------------- */
/*e \dref_TransportQosGroupDocs
 */

/* ----------------------------------------------------------------- */
#ifdef DOXYGEN_DOCUMENTATION_ONLY
/*i \dref_StringSeq
 */
struct DDS_StringSeq {};
#endif


/*e \dref_TransportQosPolicy
 */
struct DDSCPPDllExport DDS_TransportQosPolicy
{
    /*e \dref_TransportQosPolicy_enabled_transports
     */
    struct DDS_StringSeq enabled_transports;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_TransportQosPolicy)
};

/*i \dref_TransportQosPolicy_DEFAULT
 */
#define DDS_TRANSPORT_QOS_POLICY_DEFAULT \
{\
    DDS_SEQUENCE_INITIALIZER \
}

DDSC_QOS_POLICY_METHODS_DECL(DDS_TransportQosPolicy)

/* ----------------------------------------------------------------- */
/*     DOMAIN_PARTICIPANT_RESOURCE_LIMITS_X (eXtension QoS)          */
/* ----------------------------------------------------------------- */
/*e \dref_DomainParticipantResourceLimitsQosGroupDocs
 */

/* ----------------------------------------------------------------- */

/*e \dref_DomainParticipantResourceLimitsQosPolicy
 */
struct DDSCPPDllExport DDS_DomainParticipantResourceLimitsQosPolicy
{
    /*e \dref_DomainParticipantResourceLimitsQosPolicy_local_writer_allocation
     */
    DDS_Long local_writer_allocation;

    /*e \dref_DomainParticipantResourceLimitsQosPolicy_local_reader_allocation
     */
    DDS_Long local_reader_allocation;

    /*e \dref_DomainParticipantResourceLimitsQosPolicy_local_publisher_allocation
     */
    DDS_Long local_publisher_allocation;

    /*e \dref_DomainParticipantResourceLimitsQosPolicy_local_subscriber_allocation
     */
    DDS_Long local_subscriber_allocation;

    /*e \dref_DomainParticipantResourceLimitsQosPolicy_local_topic_allocation
     */
    DDS_Long local_topic_allocation;

    /*e \dref_DomainParticipantResourceLimitsQosPolicy_local_type_allocation
     */
    DDS_Long local_type_allocation;

    /*e \dref_DomainParticipantResourceLimitsQosPolicy_remote_participant_allocation
     */
    DDS_Long remote_participant_allocation;

    /*e \dref_DomainParticipantResourceLimitsQosPolicy_remote_writer_allocation
     */
    DDS_Long remote_writer_allocation;

    /*e \dref_DomainParticipantResourceLimitsQosPolicy_remote_reader_allocation
     */
    DDS_Long remote_reader_allocation;

    /*e \dref_DomainParticipantResourceLimitsQosPolicy_matching_writer_reader_pair_allocation
     */
    DDS_Long matching_writer_reader_pair_allocation;

    /*e \dref_DomainParticipantResourceLimitsQosPolicy_matching_reader_writer_pair_allocation
     */
    DDS_Long matching_reader_writer_pair_allocation;

    /*e \dref_DomainParticipantResourceLimitsQosPolicy_max_receive_ports
     */
    DDS_Long max_receive_ports;

    /*e \dref_DomainParticipantResourceLimitsQosPolicy_max_destination_ports
     */
    DDS_Long max_destination_ports;

#ifndef DOXYGEN_DOCUMENTATION_ONLY
    /*i
     */
    DDS_Long unbound_data_buffer_size;
#endif

    /*e \dref_DomainParticipantResourceLimitsQosPolicy_shmem_ref_transfer_mode_max_segments
     */
    DDS_UnsignedLong shmem_ref_transfer_mode_max_segments;

    /*e \dref_DomainParticipantResourceLimitsQosPolicy_participant_user_data_max_length
     */
    DDS_Long participant_user_data_max_length;

    /*e \dref_DomainParticipantResourceLimitsQosPolicy_participant_user_data_max_count
     */
    DDS_Long participant_user_data_max_count;

    /*e \dref_DomainParticipantResourceLimitsQosPolicy_topic_data_max_length
     */
    DDS_Long topic_data_max_length;

    /*e \dref_DomainParticipantResourceLimitsQosPolicy_topic_data_max_count
     */
    DDS_Long topic_data_max_count;

    /*e \dref_DomainParticipantResourceLimitsQosPolicy_publisher_group_data_max_length
     */
    DDS_Long publisher_group_data_max_length;

    /*e \dref_DomainParticipantResourceLimitsQosPolicy_publisher_group_data_max_count
     */
    DDS_Long publisher_group_data_max_count;

    /*e \dref_DomainParticipantResourceLimitsQosPolicy_subscriber_group_data_max_length
     */
    DDS_Long subscriber_group_data_max_length;

    /*e \dref_DomainParticipantResourceLimitsQosPolicy_subscriber_group_data_max_count
     */
    DDS_Long subscriber_group_data_max_count;

    /*e \dref_DomainParticipantResourceLimitsQosPolicy_writer_user_data_max_length
     */
    DDS_Long writer_user_data_max_length;

    /*e \dref_DomainParticipantResourceLimitsQosPolicy_writer_user_data_max_count
     */
    DDS_Long writer_user_data_max_count;

    /*e \dref_DomainParticipantResourceLimitsQosPolicy_reader_user_data_max_length
     */
    DDS_Long reader_user_data_max_length;

    /*e \dref_DomainParticipantResourceLimitsQosPolicy_reader_user_data_max_count
     */
    DDS_Long reader_user_data_max_count;

    /*e \dref_DomainParticipantResourceLimitsQosPolicy_max_partitions
     */
    DDS_Long max_partitions;

    /*e \dref_DomainParticipantResourceLimitsQosPolicy_max_partition_cumulative_characters
     */
    DDS_Long max_partition_cumulative_characters;

    /*e \dref_DomainParticipantResourceLimitsQosPolicy_max_partition_string_size
     */
    DDS_Long max_partition_string_size;

    /*e \dref_DomainParticipantResourceLimitsQosPolicy_max_partition_string_allocation
     */
    DDS_Long max_partition_string_allocation;

    /*i \dref_DomainParticipantResourceLimitsQosPolicy_participant_property_list_max_length
     */
    DDS_Long participant_property_list_max_length;

    /*i \dref_DomainParticipantResourceLimitsQosPolicy_participant_property_string_max_length
     */
    DDS_Long participant_property_string_max_length;

    /*i \dref_DomainParticipantResourceLimitsQosPolicy_writer_property_list_max_length
     */
    DDS_Long writer_property_list_max_length;

    /*i \dref_DomainParticipantResourceLimitsQosPolicy_writer_property_string_max_length
     */
    DDS_Long writer_property_string_max_length;

    /*i \dref_DomainParticipantResourceLimitsQosPolicy_reader_property_list_max_length
     */
    DDS_Long reader_property_list_max_length;

    /*i \dref_DomainParticipantResourceLimitsQosPolicy_reader_property_string_max_length
     */
    DDS_Long reader_property_string_max_length;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_DomainParticipantResourceLimitsQosPolicy)
};

/*i @ingroup DDSDomainParticipantResourceLimitsQosModule
 *
 * This constant is used below in
 * DDS_DOMAIN_PARTICIPANT_RESOURCE_LIMITS_QOS_POLICY_DEFAULT; other types
 * should not use it alone.
 */
#define DDS_DomainParticipantResourceLimitsQosPolicy_MATCH_INIT (32L)


/*i @ingroup DDSDomainParticipantResourceLimitsQosModule
 *
 * Several fields in this structure are actually never used. These fields
 * should be explicitly initialized when this structure is copied
 * from a lower layer structure to prevent the values from being left in
 * an uninitialized state. The unused fields include:
 *
 * - local_publisher_allocation.max_count
 * - local_subscriber_allocation.max_count
 * - local_topic_allocation.max_count
 * - matching_writer_reader_pair_allocation.initial_count
 * - matching_writer_reader_pair_allocation.max_count
 * - matching_reader_writer_pair_allocation.initial_count
 * - matching_reader_writer_pair_allocation.max_count
 */
/*i \dref_DomainParticipantReasourceLimitsQosPolicy_DEFAULT
 */

#define DDS_DOMAIN_PARTICIPANT_RESOURCE_LIMITS_PARTITION_DEFAULT \
    64,  /* max_partitions */                       \
    256, /* max_partition_cumulative_characters */  \
    DDS_LENGTH_UNLIMITED,  /* max_partition_string_size */ \
    DDS_LENGTH_UNLIMITED /* max_partition_string_allocation */


/*ci \brief Default value for property string length. Note, these
 * values are not used.
 */
#define DDS_PROPERTY_MAX_STRING_LENGTH_DEFAULT 32

#define DDS_DOMAIN_PARTICIPANT_RESOURCE_LIMITS_QOS_POLICY_DEFAULT \
{1L, /*local_writer_allocation*/       \
 1L, /*local_reader_allocation*/       \
 1L, /*local_publisher_allocation*/    \
 1L, /*local_subscriber_allocation*/   \
 1L, /*local_topic_allocation*/        \
 1L, /*local_type_allocation*/         \
 1L, /*remote_participant_allocation*/ \
 1L, /*remote_writer_allocation*/      \
 1L, /*remote_reader_allocation*/      \
 DDS_DomainParticipantResourceLimitsQosPolicy_MATCH_INIT, \
 DDS_DomainParticipantResourceLimitsQosPolicy_MATCH_INIT, \
 8L, /* max_receive_ports */           \
 8L, /* max_destination_ports */       \
 65536L, /*unbound_data_buffer_size */ \
 500UL, /* shmem_ref_transfer_mode_max_segments */ \
 0L,            /* participant_user_data_max_length */ \
 DDS_SIZE_AUTO, /* participant_user_data_max_count */  \
 0L,            /* topic_data_max_length */            \
 DDS_SIZE_AUTO, /* topic_data_max_count */             \
 0L,            /* publisher_group_data_max_length */  \
 DDS_SIZE_AUTO, /* publisher_group_data_max_count */   \
 0L,            /* subscriber_group_data_max_length */ \
 DDS_SIZE_AUTO, /* subscriber_group_data_max_count */  \
 0L,            /* writer_user_data_max_length */      \
 DDS_SIZE_AUTO, /* writer_user_data_max_count */       \
 0L,            /* reader_user_data_max_length */      \
 DDS_SIZE_AUTO, /* reader_user_data_max_count */       \
 DDS_DOMAIN_PARTICIPANT_RESOURCE_LIMITS_PARTITION_DEFAULT, \
 1, /* participant_property_list_max_length */ \
 32,\
 1 ,/* writer_property_list_max_length */ \
 32,\
 0,\
 0\
}

DDSC_QOS_POLICY_METHODS_DECL(DDS_DomainParticipantResourceLimitsQosPolicy)

/* ----------------------------------------------------------------- */

/*i \dref_BUILTIN_TOPIC_KEY_TYPE_NATIVE_LENGTH
 */
#define DDS_BUILTIN_TOPIC_KEY_TYPE_NATIVE_LENGTH (4)

/*e \dref_BuiltinTopicKey_t
 */
typedef struct DDS_BuiltinTopicKey_t
{
  /*e \dref_BuiltinTopicKey_t_value
   */
  DDS_BUILTIN_TOPIC_KEY_TYPE_NATIVE value[DDS_BUILTIN_TOPIC_KEY_TYPE_NATIVE_LENGTH];
} DDS_BuiltinTopicKey_t;

/*i @ingroup BuiltinTopicGroupDocs
 */
#define DDS_BuiltinTopicKey_t_INITIALIZER { {0, 0, 0, 0} }

/*e \dref_BuiltinTopicKey_t_AUTO
 */
extern DDSCDllVariable const struct DDS_BuiltinTopicKey_t DDS_BUILTINTOPICKEY_AUTO;

/*e \dref_BuiltinTopicKey_t_UNKNOWN
 */
extern DDSCDllVariable const struct DDS_BuiltinTopicKey_t DDS_BUILTINTOPICKEY_UNKNOWN;

/*i \dref_BuiltinTopicKey_t_PREFIX_UNKNOWN
 */
extern DDSCDllVariable const struct DDS_BuiltinTopicKey_t DDS_BUILTINTOPICKEY_PREFIX_UNKNOWN;

/*i \dref_BuiltinTopicKey_t_PREFIX_UNKNOWN
 */
extern DDSCDllVariable const struct DDS_BuiltinTopicKey_t DDS_BUILTINTOPICKEY_PREFIX_AUTO;

/*i \ingroup BuiltinTopicGroupDocs_equals
*/
DDSCDllExport DDS_Boolean
DDS_BuiltinTopicKey_equals(const DDS_BuiltinTopicKey_t *a,
                           const DDS_BuiltinTopicKey_t *b);

/*i \ingroup BuiltinTopicGroupDocs_prefix_equals
 */
DDSCDllExport DDS_Boolean
DDS_BuiltinTopicKey_prefix_equals(const DDS_BuiltinTopicKey_t *a,
                                  const DDS_BuiltinTopicKey_t *b);

/*i \ingroup BuiltinTopicGroupDocs_suffix_equals
 */
DDSCDllExport DDS_Boolean
DDS_BuiltinTopicKey_suffix_equals(const DDS_BuiltinTopicKey_t *a,
                                  const DDS_BuiltinTopicKey_t *b);

/*i \ingroup BuiltinTopicGroupDocs_copy_prefix
 */
DDSCDllExport void
DDS_BuiltinTopicKey_copy_prefix(DDS_BuiltinTopicKey_t *a,
                                const DDS_BuiltinTopicKey_t *b);

/*i \ingroup BuiltinTopicGroupDocs_copy_suffix
 */
DDSCDllExport void
DDS_BuiltinTopicKey_copy_suffix(DDS_BuiltinTopicKey_t *a,
                                const DDS_BuiltinTopicKey_t *b);

/*i \ingroup BuiltinTopicGroupDocs_from_guid
 */
DDSCDllExport void
DDS_BuiltinTopicKey_from_guid(DDS_BuiltinTopicKey_t *in,
                              const DDS_InstanceHandle_t *out);
/*ci
 * \brief Compare two DDS_BuiltinTopicKey_t structure for ordering
 *
 * \param[in] left  Left side of comparison
 * \param[in] right Right side of comparison
 *
 * \return positive integer if left is greater than right,
 *         negative integer if left is less than right,
 *         zero if left is equal to right
 */
DDSCDllExport DDS_Long
DDS_BuiltinTopicKey_compare(const DDS_BuiltinTopicKey_t *left,
                            const DDS_BuiltinTopicKey_t *right);



DDSCDllExport void
DDS_BuiltinTopicKey_from_instance_handle(
        DDS_BuiltinTopicKey_t *self,
        const DDS_InstanceHandle_t *instance_handle);

/* ================================================================= */
/*                 Condition and Waitsets                            */
/* ================================================================= */

struct DDS_EntityImpl;
/*ce \dref_Entity
 */
typedef struct DDS_EntityImpl DDS_Entity;

/* ----------------------------------------------------------------- */
/*e \dref_ConditionsAndWaitsetsModuleDocs
 */

struct DDS_ConditionImpl;
/*ce \dref_Condition
 */
typedef struct DDS_ConditionImpl DDS_Condition;

/*ci @ingroup DDSConditionsModule
  @brief Pointer to DDS_Condition.
 */
typedef struct DDS_ConditionImpl *DDS_Condition_ptr;

/*ce \dref_ConditionSeq
 */
#define T struct DDS_ConditionImpl*
#ifndef RTI_CERT
#define TSeq_ensure_length
#define TSeq_has_ownership
#endif
#define TSeq DDS_ConditionSeq
#include <reda/reda_sequence_decl.h>

#ifdef DOXYGEN_DOCUMENTATION_ONLY
/*ce \dref_ConditionSeq
 */
struct DDS_ConditionSeq {};
#endif

/*ce \dref_Condition_get_trigger_value
 */
DDSCDllExport DDS_Boolean
DDS_Condition_get_trigger_value(DDS_Condition *self);

/*i \dref_Condition_set_wrapper
 */
DDSCDllExport void
DDS_ConditionImpl_set_wrapper(DDS_Condition *self, void *wrapper);

/*i \dref_Condition_get_wrapper
 */
DDSCDllExport void**
DDS_ConditionImpl_get_wrapper_ref(DDS_Condition *self);

/* ----------------------------------------------------------------- */

/*ce \dref_GuardCondition
 */
typedef struct DDS_GuardConditionImpl DDS_GuardCondition;

#define DDS_GuardCondition_as_condition(guard_condition_ptr_) \
                                        ((DDS_Condition*) guard_condition_ptr_)

#ifdef DOXYGEN_DOCUMENTATION_ONLY
/*ce \dref_GuardCondition_as_condition
 */
DDS_Condition* DDS_GuardCondition_as_condition(DDS_GuardCondition* guard_cond);
#endif /*DOXYGEN_DOCUMENTATION_ONLY*/

/*ce \dref_GuardCondition_new
 */
DDSCDllExport DDS_GuardCondition*
DDS_GuardCondition_new(void);

#ifndef RTI_CERT
/*ce \dref_GuardCondition_delete
 */
DDSCDllExport DDS_ReturnCode_t
DDS_GuardCondition_delete(DDS_GuardCondition *self);
#endif

/*ce \dref_GuardCondition_set_trigger_value
 */
DDSCDllExport DDS_ReturnCode_t
DDS_GuardCondition_set_trigger_value(DDS_GuardCondition *self,
                                     DDS_Boolean value);

/* ----------------------------------------------------------------- */

/*ce \dref_StatusCondition
 */
typedef struct DDS_StatusConditionImpl DDS_StatusCondition;

#define DDS_StatusCondition_as_condition(status_cond_ptr_) \
                                    ((DDS_Condition*) status_cond_ptr_)

#ifdef DOXYGEN_DOCUMENTATION_ONLY
/*ce \dref_StatusCondition_as_condition
 */
DDS_Condition* DDS_StatusCondition_as_condition(
                                    DDS_StatusCondition *statusCondition);
#endif /*DOXYGEN_DOCUMENTATION_ONLY*/


/*ce \dref_StatusCondition_get_enabled_statuses
 */
DDSCDllExport DDS_StatusMask
DDS_StatusCondition_get_enabled_statuses(DDS_StatusCondition *self);

/*ce \dref_StatusCondition_set_enabled_statuses
 */
DDSCDllExport DDS_ReturnCode_t
DDS_StatusCondition_set_enabled_statuses(DDS_StatusCondition *self,
                                         DDS_StatusMask mask);

/*ce \dref_StatusCondition_get_entity
 */
DDSCDllExport DDS_Entity*
DDS_StatusCondition_get_entity(DDS_StatusCondition *self);

/*ce \dref_WaitSet
 */
typedef struct DDS_WaitSetImpl DDS_WaitSet;

/*ce \dref_WaitSet_new
 */
DDSCDllExport DDS_WaitSet*
DDS_WaitSet_new(void);

/*ce \dref_WaitSet_wait
 */
DDSCDllExport DDS_ReturnCode_t
DDS_WaitSet_wait(DDS_WaitSet *self,
                 struct DDS_ConditionSeq *active_conditions,
                 const struct DDS_Duration_t *timeout);

/*ce \dref_WaitSet_attach_condition
 */
DDSCDllExport DDS_ReturnCode_t
DDS_WaitSet_attach_condition(DDS_WaitSet *self,DDS_Condition *cond);

#ifndef RTI_CERT
/*ce \dref_WaitSet_delete
 */
DDSCDllExport DDS_ReturnCode_t
DDS_WaitSet_delete(DDS_WaitSet *self);
#endif /* !RTI_CERT */

#ifndef RTI_CERT
/*ce \dref_WaitSet_detach_condition
 */
DDSCDllExport DDS_ReturnCode_t
DDS_WaitSet_detach_condition(DDS_WaitSet *self,DDS_Condition *cond);
#endif /* !RTI_CERT */


/*ce \dref_WaitSet_get_conditions
 */
DDSCDllExport DDS_ReturnCode_t
DDS_WaitSet_get_conditions(DDS_WaitSet *self,
                           struct DDS_ConditionSeq *attached_conditions);

/* ================================================================= */
/*                 Listeners                                         */
/* ================================================================= */
/*e \dref_EntityModuleDocs
 */

/*ce \dref_Listener
 */
struct DDS_Listener
{
    /*ce \dref_Listener_listener_data
     */
    void *listener_data;
};

/*ce \dref_Listener_INITIALIZER
 */
#define DDS_Listener_INITIALIZER    { NULL }

/* ================================================================= */
/*                         Entity typedef                            */
/* ================================================================= */

#ifdef DOXYGEN_DOCUMENTATION_ONLY
/*ce \dref_DomainEntity
 */
typedef struct DDS_DomainEntityImpl DDS_DomainEntity;
#endif                          /*DOXYGEN_DOCUMENTATION_ONLY */

/* ================================================================= */
/*                            DDS_Entity                             */
/* ================================================================= */

/*ce \dref_Entity_enable
 */
DDSCDllExport DDS_ReturnCode_t
DDS_Entity_enable(DDS_Entity * self);

/*ce \dref_Entity_is_enabled
 */
DDSCDllExport DDS_Boolean
DDS_Entity_is_enabled(DDS_Entity * self);

/*ce \dref_Entity_get_instance_handle
 */
DDSCDllExport DDS_InstanceHandle_t
DDS_Entity_get_instance_handle(DDS_Entity * self);

/*ce \dref_Entity_get_entity_kind
 */
DDSCDllExport DDS_EntityKind_t
DDS_Entity_get_entity_kind(DDS_Entity *self);

/*ce \dref_Entity_get_statuscondition
 */
DDSCDllExport DDS_StatusCondition*
DDS_Entity_get_statuscondition(DDS_Entity *self);

/*ce \dref_Entity_get_status_changes
 */
DDSCDllExport DDS_StatusMask
DDS_Entity_get_status_changes(DDS_Entity *self);

/*ci
 * \brief Associate a language-dependent wrapper object with a DDS_Entity.
 *
 * \details
 * This operation is used to support access to the functionality of
 * the C implementation from other compatible programming languages, such as C++.
 * Implementations of the Micro API in these programming languages may use
 * this facility to associate objects created in their run-time environments
 * with DDS_Entity instances by means of this operation, and then access
 * them using DDS_Entity_get_wrapper, typically to support the correct
 * propagation of events from the C core to the wrapping programming language's
 * layer.
 *
 * \param[in] self a non NULL DDS_Entity
 * \param[in] wrapper pointer to a wrapper instance or NULL to delete an
 * existing association.
 */
DDSCDllExport void
DDS_Entity_set_wrapper(DDS_Entity *self, void *wrapper);

/*ci
 * \brief Access a language-dependent wrapper object attached to the DDS_Entity.
 *
 * \long This operation is used to support access to the functionality of
 * the C implementation from other compatible programming languages, such as C++.
 * Implementations of the Micro API in these programming languages may use
 * this facility to associate objects created in their run-time environments
 * with DDS_Entity instances by means of DDS_Entity_set_wrapper, typically
 * to support the correct propagation of events from the C core to the wrapping
 * programming language's layer.
 *
 * \param[in] self a non NULL DDS_Entity
 *
 * \return a pointer to a wrapper object previously or NULL if none had been
 * previously set using DDS_Entity_set_wrapper.
 */
DDSCDllExport void*
DDS_Entity_get_wrapper(DDS_Entity *self);

/* ----------------------------------------------------------------- */
/*                ENTITY_NAME                                        */
/* ----------------------------------------------------------------- */
/*e \dref_EntityNameQosGroupDocs
 */

/* ----------------------------------------------------------------- */
/*e \dref_EntityNameQosPolicy_NAME_MAX
 */
#define DDS_ENTITYNAME_QOS_NAME_MAX 255

#ifdef RTI_WIN32
#pragma warning(push)
#pragma warning(disable: 4522)
#endif

/*e \dref_EntityNameQosPolicy
 */
struct DDSCPPDllExport DDS_EntityNameQosPolicy
{
    /*e \dref_EntityNameQosPolicy_name
     */
    char name[DDS_ENTITYNAME_QOS_NAME_MAX + 1];

    DDSC_CPP_QOS_POLICY_METHODS(DDS_EntityNameQosPolicy)

#ifdef RTI_CPP
public:
    /*e \dref_EntityNameQosPolicy_set_name
     */
    bool set_name(const char *const name);
#ifdef RTI_CERT
    private:
#else
    public:
#endif
    DDS_EntityNameQosPolicy();
    ~DDS_EntityNameQosPolicy();
    DDS_EntityNameQosPolicy(const DDS_EntityNameQosPolicy& from);
    DDS_EntityNameQosPolicy& operator=(const char *const name);
    DDS_EntityNameQosPolicy& operator=(const DDS_EntityNameQosPolicy& from);
    bool operator==(const DDS_EntityNameQosPolicy& other) const;
    bool operator==(const char *const name) const;
    bool operator!=(const DDS_EntityNameQosPolicy& other) const;
    bool operator!=(const char *const name) const;
#endif
};

#ifdef RTI_WIN32
#pragma warning(pop)
#endif

/*i \dref_EntityNameQosPolicy_DEFAULT
 */
#define DDS_ENTITY_NAME_QOS_POLICY_DEFAULT   { {'\0'} }

DDSC_QOS_POLICY_METHODS_DECL(DDS_EntityNameQosPolicy)

/*ce \dref_EntityNameQosPolicy_set_name
 */
DDSCDllExport DDS_Boolean
DDS_EntityNameQosPolicy_set_name(struct DDS_EntityNameQosPolicy *const self,
                                 const char *const name);

/* ================================================================= */
/*                            RTI_Management                         */
/* ================================================================= */

/*i \dref_ManagementQosPolicy
 */
struct DDSCPPDllExport RTI_ManagementQosPolicy
{
    DDS_Boolean is_hidden;
    DDS_Boolean is_anonymous;
    DDS_Boolean disable_unregister_dispose_for_unpublished_instance;
    DDS_Boolean is_announced;

    DDSC_CPP_QOS_POLICY_METHODS(RTI_ManagementQosPolicy)
};

/*i \dref_ManagementQosPolicy_DEFAULT
 */
#define RTI_MANAGEMENT_QOS_POLICY_DEFAULT { \
    DDS_BOOLEAN_FALSE,\
    DDS_BOOLEAN_FALSE,\
    DDS_BOOLEAN_FALSE,\
    DDS_BOOLEAN_TRUE  \
}

DDSC_QOS_POLICY_METHODS_DECL(RTI_ManagementQosPolicy)

/* ================================================================= */
/*                            DDS_DomainEntity                       */
/* ================================================================= */

/*ce \dref_DomainParticipant
 */
typedef struct DDS_DomainParticipantImpl DDS_DomainParticipant;

struct DDS_DomainParticipantQos;





/* ================================================================= */
/*       DDS_DataWriter and DDS_DataReader forward declarations      */
/* ================================================================= */

/*ce \dref_DataReader
 */
typedef struct DDS_DataReaderImpl DDS_DataReader;

/*ce \dref_DataWriter
 */
typedef struct DDS_DataWriterImpl DDS_DataWriter;


/* ----------------------------------------------------------------- */

/* ================================================================= */
/*                            DDSHST_History                           */
/* ================================================================= */
/*i \dref_DDSHST_ReturnCode_T
 */
typedef enum
{
    DDSHST_RETCODE_ERROR = -1000,
    DDSHST_RETCODE_NOSPACE,
    DDSHST_RETCODE_EXISTS,
    DDSHST_RETCODE_NOT_EXISTS,
    DDSHST_RETCODE_INVALID_PROPERTY,
    DDSHST_RETCODE_INVALID_ENTRY_REQUEST,
    DDSHST_RETCODE_NOT_SUPPORTED,
    DDSHST_RETCODE_SUCCESS = 0
} DDSHST_ReturnCode_T;

/*i \dref_ReplacePolicyKind_T
 */
typedef enum
{
    DDSHST_REPLACE_POLICY_KIND_OLDEST,
    DDSHST_REPLACE_POLICY_KIND_NONE
} DDSHST_ReplacePolicyKind_T;


/* ----------------------------------------------------------------- */
/*                WRITE_PARAMS                                       */
/* ----------------------------------------------------------------- */

/*e \dref_SampleIdentity_t
 */
struct DDSCPPDllExport DDS_SampleIdentity_t
{
    /*e \dref_SampleIdentity_t_writer_guid */
    struct DDS_GUID_t writer_guid;
    /*e \dref_SampleIdentity_t_sequence_number */
    struct DDS_SequenceNumber_t sequence_number;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_SampleIdentity_t)
};


/*ci \dref_SampleIdentity_t
 */
#define DDS_SAMPLE_IDENTITY_UNKNOWN \
{ \
    DDS_GUID_INITIALIZER, \
    DDS_SEQUENCE_NUMBER_UNKNOWN \
}

/*e \dref_WriteParams_t
 */
struct DDSCPPDllExport DDS_WriteParams_t
{
    /*i \dref_WriteParams_t_identity
    */
    struct DDS_SampleIdentity_t identity;

    /*i \dref_WriteParams_t_related_sample_identity
    */
    struct DDS_SampleIdentity_t related_sample_identity;

    /*e \dref_WriteParams_t_source_timestamp
     */
    struct DDS_Time_t source_timestamp;

    /*e \dref_WriteParams_t_handle
     */
    DDS_InstanceHandle_t handle;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_WriteParams_t)
};

/*e \dref_WriteParams_t_DEFAULT
 */
#define DDS_WRITEPARAMS_DEFAULT \
{ \
    DDS_SAMPLE_IDENTITY_UNKNOWN, \
    DDS_SAMPLE_IDENTITY_UNKNOWN, \
    DDS_TIME_ZERO, \
    DDS_HANDLE_NIL_NATIVE \
}

#include "dds_c/dds_c_property_qos.h"

/* ================================================================= */
/*                       DataTag Qos Policy                          */
/* ================================================================= */






















































typedef enum NDDS_ParticipantMemPoolId
{
    NDDS_PARTICIPANT_MEMPOOL_UNKNOWN = 0x00,
    NDDS_PARTICIPANT_MEMPOOL_UNBOUND = 0x01
} NDDS_ParticipantMemPoolId_T;


/* ================================================================= */
/*                       PublishMode Qos Policy                     */
/* ================================================================= */
/*e \dref_PublishModeQosGroupDocs */

/*e \dref_PublishModeQosPolicyKind
 */
typedef enum
{
    /*e \dref_PublishModeQosPolicyKind_SYNCHRONOUS_PUBLISH_MODE_QOS
     */
    DDS_SYNCHRONOUS_PUBLISH_MODE_QOS,

    /*e \dref_PublishModeQosPolicyKind_ASYNCHRONOUS_PUBLISH_MODE_QOS
     */
    DDS_ASYNCHRONOUS_PUBLISH_MODE_QOS,

    /*e \dref_PublishModeQosPolicyKind_AUTOMATIC_PUBLISH_MODE_QOS
     */
    DDS_AUTOMATIC_PUBLISH_MODE_QOS
} DDS_PublishModeQosPolicyKind;

/*e \dref_PUBLICATION_PRIORITY_UNDEFINED
 */
#define DDS_PUBLICATION_PRIORITY_UNDEFINED (0)

/*e \dref_PUBLICATION_PRIORITY_AUTOMATIC
 */
#define DDS_PUBLICATION_PRIORITY_AUTOMATIC (-1)

#define DDS_DEFAULT_PUBLISH_MODE_QOS DDS_AUTOMATIC_PUBLISH_MODE_QOS

/*e \dref_PublishModeQosPolicy
 */
struct DDSCPPDllExport DDS_PublishModeQosPolicy
{
    /*e \dref_PublishModeQosPolicy_kind
     */
    DDS_PublishModeQosPolicyKind kind;

    /*e \dref_PublishModeQosPolicy_flow_controller_name
     */
    const char *flow_controller_name;

    /*e \dref_PublishModeQosPolicy_priority
     */
    DDS_Long priority;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_PublishModeQosPolicy)
};

DDSC_QOS_POLICY_METHODS_DECL(DDS_PublishModeQosPolicy)

#define DDS_PUBLISH_MODE_QOS_POLICY_DEFAULT \
{ \
    DDS_DEFAULT_PUBLISH_MODE_QOS,\
    NULL, \
    DDS_PUBLICATION_PRIORITY_UNDEFINED \
}

struct DDSCPPDllExport DDS_TypeAllocationParams_t
{
    /*i \dref_TypeAllocationParams_allocate_pointers
     */
    DDS_Boolean allocate_pointers;

    /*i \dref_TypeAllocationParams_allocate_optional_members
     */
    DDS_Boolean allocate_optional_members;

    /*i
     * \dref_TypeAllocationParams_allocate_memory
     */
    DDS_Boolean allocate_memory;

#if 0
    DDSCPP_VARIABLE_LENGTH_VALUE_TYPE_SUPPORT(DDS_TypeAllocationParams_t)
#ifdef RTI_CPP
    /*e
     * Whether to allocate pointer members or not
     */
    struct DDS_TypeAllocationParams_t & set_allocate_pointers(DDS_Boolean allocate)
    {
        this->allocate_pointers = allocate;
        return *this;
    }
    /*e
     * Whether to allocate optional members or not
     */
    struct DDS_TypeAllocationParams_t & set_allocate_optional_members(DDS_Boolean allocate) {
        this->allocate_optional_members = allocate;
        return *this;
    }
#endif
#endif
};



struct DDSCPPDllExport DDS_TypeDeallocationParams_t {
    /*e
     * \dref_TypeDeallocationParams_delete_pointers
     */
    DDS_Boolean delete_pointers;
    /*e
     * \dref_TypeDeallocationParams_delete_optional_members
     */
    DDS_Boolean delete_optional_members;
#if 0
    DDSCPP_VARIABLE_LENGTH_VALUE_TYPE_SUPPORT(DDS_TypeDeallocationParams_t)
#ifdef RTI_CPP
    /*e
     * Whether to delete pointer members or not
     */
    struct DDS_TypeDeallocationParams_t & set_delete_pointers(DDS_Boolean do_delete) {
        this->delete_pointers = do_delete;
        return *this;
    }
    /*e
     * Whether to delete optional members or not
     */
    struct DDS_TypeDeallocationParams_t & set_delete_optional_members(DDS_Boolean do_delete) {
        this->delete_optional_members = do_delete;
        return *this;
    }
#endif
#endif
};

/*ce \dref_ExceptionCode_t
 */
typedef enum
{
    /*e \dref_ExceptionCode_t_NO_EXCEPTION_CODE
     */
    DDS_NO_EXCEPTION_CODE              = 0,

    /*e \dref_ExceptionCode_t_USER_EXCEPTION_CODE
     */
    DDS_USER_EXCEPTION_CODE            = 1,

    /*e \dref_ExceptionCode_t_SYSTEM_EXCEPTION_CODE
     */
    DDS_SYSTEM_EXCEPTION_CODE          = 2,

    /*e \dref_ExceptionCode_t_BAD_PARAM_SYSTEM_EXCEPTION_CODE
     */
    DDS_BAD_PARAM_SYSTEM_EXCEPTION_CODE = 3,

    /*e \dref_ExceptionCode_t_NO_MEMORY_SYSTEM_EXCEPTION_CODE
     */
    DDS_NO_MEMORY_SYSTEM_EXCEPTION_CODE = 4,

    /*e \dref_ExceptionCode_t_BAD_TYPECODE_SYSTEM_EXCEPTION_CODE
     */
    DDS_BAD_TYPECODE_SYSTEM_EXCEPTION_CODE = 5,

    /*e \dref_ExceptionCode_t_BADKIND_USER_EXCEPTION_CODE
     */
    DDS_BADKIND_USER_EXCEPTION_CODE    = 6,

    /*e \dref_ExceptionCode_t_BOUNDS_USER_EXCEPTION_CODE
     */
    DDS_BOUNDS_USER_EXCEPTION_CODE     = 7,

    /*e \dref_ExceptionCode_t_IMMUTABLE_TYPECODE_SYSTEM_EXCEPTION_CODE
     */
    DDS_IMMUTABLE_TYPECODE_SYSTEM_EXCEPTION_CODE = 8,

    /*e \dref_ExceptionCode_t_BAD_MEMBER_NAME_USER_EXCEPTION_CODE
     */
    DDS_BAD_MEMBER_NAME_USER_EXCEPTION_CODE  = 9,

    /*e \dref_ExceptionCode_t_BAD_MEMBER_ID_USER_EXCEPTION_CODE
     */
    DDS_BAD_MEMBER_ID_USER_EXCEPTION_CODE  = 10

} DDS_ExceptionCode_t;


#if DDS_XTYPES_IS_ENABLED
/*e \dref_GlobalConfigurationGroupDocs
 */

/*e \dref_XTypesComplianceMaskBits
 */
typedef enum
{
    /*i \dref_XTypesComplianceMaskBits_NDDS_CONFIG_XTYPES_DHEADER_IN_NON_PRIMITIVE_COLLECTIONS_BIT
     */
    NDDS_CONFIG_XTYPES_DHEADER_IN_NON_PRIMITIVE_COLLECTIONS_BIT =
            RTI_XCDR_XTYPES_DHEADER_IN_NON_PRIMITIVE_COLLECTIONS_BIT,

    /*i \dref_XTypesComplianceMaskBits_NDDS_CONFIG_XTYPES_ENUM_AS_PRIMITIVE_IN_COLLECTIONS_BIT
     */
    NDDS_CONFIG_XTYPES_ENUM_AS_PRIMITIVE_IN_COLLECTIONS_BIT =
            RTI_XCDR_XTYPES_ENUM_AS_PRIMITIVE_IN_COLLECTIONS_BIT,

    /*i \dref_XTypesComplianceMaskBits_NDDS_CONFIG_XTYPES_PARAMETER_LENGTH_WITH_PADDING_BIT
     */
    NDDS_CONFIG_XTYPES_PARAMETER_LENGTH_WITH_PADDING_BIT =
            RTI_XCDR_XTYPES_PARAMETER_LENGTH_WITH_PADDING_BIT,

    /*e \dref_XTypesComplianceMaskBits_NDDS_CONFIG_XTYPES_ENCAPSULATION_OPTIONS_WITH_PADDING_BIT
     */
    NDDS_CONFIG_XTYPES_ENCAPSULATION_OPTIONS_WITH_PADDING_BIT =
            RTI_XCDR_XTYPES_ENCAPSULATION_OPTIONS_WITH_PADDING_BIT
} NDDS_Config_XTypesComplianceMaskBits;

/*e \dref_XTypesComplianceMask
 */
typedef DDS_UnsignedLong NDDS_Config_XTypesComplianceMask;
#endif

#ifdef __cplusplus
}
#endif

#endif /* dds_c_infrastructure_h */
