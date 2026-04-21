/*
 * FILE: dds_c_infrastructure.h - DDS infrastructure module definitions
 *
 * (c) Copyright, Real-Time Innovations, 2012-2016.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 05aug2022,tk MICRO-4102/PR.30495
 * - Use DDS_SYSTEM_RESOURCE_LIMITS_MAX_COMPONENTS_DEFAULT instead of literals
 * - Increased DDS_SYSTEM_RESOURCE_LIMITS_MAX_COMPONENTS_DEFAULT to 64 for
 *   max_components when not using UDP transformations.
 * 15apr2021,tk MICRO-2979/PR.27549
 * - Removed unused functions from CERT:
 *   DDS_Duration_from_ntp_time,DDS_GUID_to_rtps,
 *   DDS_BuiltinTopicKey_copy_prefix,DDS_Entity_get_wrapper
 *   DDS_DataWriter_remove_instance
 * 13apr2021,tk MICRO-3028/PR.28960
 * - Fixed external documentation for DDS_ChecksumProperty
 * - Fixed variable argument names in DDSC_VARIABLE_LENGTH_VALUE_TYPE_SUPPORT_FULL
 * - Removed unsupported CERT APIs from documentation
 * 06apr2021,tk MICRO-3023/PR.29032
 * - Changed DDS_Duration_compare to return DDS_Long instead of int.
 * 06apr2021,tk MICRO-2940/PR.28879
 * - Make the PresentationQos policy external for documentation purposes.
 * 24feb2021,tk MICRO-2915/PR#28857
 *  - Removed matching_reader_writer_pair_allocation from
 *    DDS_DomainParticipantResourceLimitsQosPolicy.
 *  - Removed DDS_DomainParticipantResourceLimitsQosPolicy_MATCH_INIT
 * 24feb2021,tk MICRO-2911/PR#28849
 *   - Changed the value of DDS_CHECKSUM_MASK_MAX to 0x7 as only 3
 *     checksum kinds are supported.
 * 24feb2021,tk MICRO-2901/PR#28785
 *    - Changed DDS_DataWriterProtocolQosPolicy.serialize_on_write
 *      to internal doc. to match HLR.
 *    - Changed RTI_ManagementQosPolicy.is_anonymous to external doc. to
 *      match HLR.
 * 10jan2021,tk MICRO-2807/PR#27549  Removed unused functions for CERT
 * 20oct2020,tk MICRO-2615/PR#28208
 *     - Added comment to disable_unregister_dispose_for_unpublished_instance
 * 19oct2020,tk MICRO-2575/PR#28172 Removed support for custom checksums.
 * 08sep2020,tk  MICRO-2506/PR#28039 Corrected internal/external doxygen tags
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
    @ingroup DDSCModule

    @brief Defines the \dds infrastructure package
*/
#ifndef dds_c_infrastructure_h
#define dds_c_infrastructure_h

#ifndef dds_c_dll_h
#include "dds_c/dds_c_dll.h"
#endif
#ifndef osapi_thread_h
#include "osapi/osapi_thread.h"
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

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef NULL
#define NULL 0
#endif

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
        bool operator!=(const T& other);\
        const T& operator=(const T& from ) const;

#define DDSC_CPP_QOS_OPERATORS(T) \
        DDSC_CPP_CERT_PRIVATE_OPERATORS(T)

#define DDSC_CPP_QOS_POLICY_OPERATORS(T) \
        DDSC_CPP_CERT_PRIVATE_OPERATORS(T)

#define DDSC_CPP_STATUS_OPERATORS(T) \
        DDSC_CPP_CERT_PRIVATE_OPERATORS(T)
#else
#define DDSC_CPP_QOS_OPERATORS(T) \
    public:\
        T(const T& from);\
        T& operator=(const T& from);\
        bool operator==(const T& other);\
        bool operator!=(const T& other);

#define DDSC_CPP_QOS_POLICY_OPERATORS(T)

#define DDSC_CPP_STATUS_OPERATORS(T)
#endif

#ifdef RTI_CPP
#define DDSC_CPP_QOS_METHODS(T) \
    public:\
        T(); \
        ~T(); \
        DDS_ReturnCode_t copy(const T& from); \
    DDSC_CPP_QOS_OPERATORS(T)

#define DDSC_CPP_QOS_POLICY_METHODS(T) \

#define DDSC_CPP_STATUS_METHODS(T)

#else
#define DDSC_CPP_QOS_METHODS(T)
#define DDSC_CPP_QOS_POLICY_METHODS(T)
#define DDSC_CPP_STATUS_METHODS(T)
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
    DDS_Long sec;

    /*e
     * \dref_TimeStamp_nanosec
     */
    DDS_UnsignedLong nanosec;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_Time_t)

#ifdef RTI_CPP
public:
    bool greater_than(const DDS_Time_t& other);
    bool is_zero();
#endif

};

/* This is only used by tests, keep as macro */
#define DDS_Time_t_greater_than(l, r) \
    (((l).sec > (r).sec) || \
     (((l).sec == (r).sec) && \
      ((l).nanosec > (r).nanosec)))

#ifdef DOXYGEN_DOCUMENTATION_ONLY
/*i
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

/*i
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
    void to_ntp_time(OSAPI_NtpTime *dst);
    void from_ntp_time(const OSAPI_NtpTime *src);
    DDS_Boolean equal(const DDS_Duration_t *other);
    DDS_Boolean is_zero();
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

/*i \dref_Duration_t_is_infinite
 */
DDSCDllExport DDS_Boolean
DDS_Duration_is_infinite(const struct DDS_Duration_t *duration);

/*ci \dref_Duration_t_compare
 */
DDSCDllExport DDS_Long
DDS_Duration_compare(const struct DDS_Duration_t *left,
                     const struct DDS_Duration_t *right);

/*ci \dref_Duration_t_to_ntp_time
 */
DDSCDllExport void
DDS_Duration_to_ntp_time(const struct DDS_Duration_t *self,
                         struct OSAPI_NtpTime *dst);

#ifndef RTI_CERT
/*ci \dref_Duration_t_from_ntp_time
 */
DDSCDllExport void
DDS_Duration_from_ntp_time(struct DDS_Duration_t *self,
                           const struct OSAPI_NtpTime *src);
#endif

/*i
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

/*i \dref_Duration_t_is_zero
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
/*i \dref_InstanceHandleSeq
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

/*e \dref_GUID_t
 */
struct DDS_GUID_t
{
    /*e \dref_GUID_t_value
     */
    DDS_Octet value[16];
};

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

#ifndef RTI_CERT
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
#endif

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
    DDS_RETCODE_ILLEGAL_OPERATION = 12
} DDS_ReturnCode_t;


/* ================================================================= */
/*            VARIABLE LENGTH TYPES SUPPORT                          */
/* ================================================================= */
/*i @defgroup DDSVarLenType Full Variable Length Type Support
  @ingroup DDSCommonModule
 */
#define DDSC_VARIABLE_LENGTH_VALUE_TYPE_SUPPORT_FULL(T) \
 struct T; \
 DDSCDllExport DDS_Boolean T ## _initialize(struct T *self); \
 DDSCDllExport DDS_Boolean T ## _finalize(struct T *self); \
 DDSCDllExport DDS_Boolean T ## _copy(struct T *out, const struct T *in);\
 MUST_CHECK_RETURN DDSCDllExport DDS_Boolean      T ## _is_equal(const struct T *left, const struct T *right)

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
    DDS_RELIABLE_READER_ACTIVITY_CHANGED_STATUS = 0x0001 << 26

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

    /*e \dref_QosPolicyId_t_PRESENTATION_QOS_POLICY_ID
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
    DDS_DURABILITYSERVICE_QOS_POLICY_ID = 22

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

/*e \dref_DEADLINE_QOS_POLICY_NAME
 */
extern DDSCDllVariable const char *const DDS_DEADLINE_QOS_POLICY_NAME;

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

/*i
 * \brief Convert a duration to ms
 *
 * \details
 * This function converts from sec,nanosec to milliseconds. Note that
 * no check is performed for overflow.
 *
 * \param[in] self Duration structure to convert to ms
 */
DDSCDllExport DDS_Long
DDS_Duration_to_ms(const struct DDS_Duration_t *const self);

/*i \dref_DeadlineQosPolicy_DEFAULT
 * Default is infinite
 */
#define DDS_DEADLINE_QOS_POLICY_DEFAULT  \
{DDS_DURATION_INFINITE_INITIALIZER}

/* ----------------------------------------------------------------- */
/*                OWNERSHIP                                          */
/* ----------------------------------------------------------------- */
/*e \dref_OwnershipQosGroupDocs
 */

/*e \dref_OWNERSHIP_QOS_POLICY_NAME
 */
extern DDSCDllVariable const char *const DDS_OWNERSHIP_QOS_POLICY_NAME;

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

/*e \dref_OWNERSHIPSTRENGTH_QOS_POLICY_NAME
 */
extern DDSCDllVariable const char *const DDS_OWNERSHIPSTRENGTH_QOS_POLICY_NAME;

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

/*e \dref_LIVELINESS_QOS_POLICY_NAME
 */
extern DDSCDllVariable const char *const DDS_LIVELINESS_QOS_POLICY_NAME;

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

/*e \dref_RELIABILITY_QOS_POLICY_NAME
 */
extern DDSCDllVariable const char *const DDS_RELIABILITY_QOS_POLICY_NAME;

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
    {0L, 0UL} \
}

DDSC_QOS_POLICY_METHODS_DECL(DDS_ReliabilityQosPolicy)

/* ----------------------------------------------------------------- */
/*                HISTORY                                            */
/* ----------------------------------------------------------------- */
/*e \dref_HistoryQosGroupDocs
 */

/*e \dref_HISTORY_QOS_POLICY_NAME
 */
extern DDSCDllVariable const char *const DDS_HISTORY_QOS_POLICY_NAME;

/* ----------------------------------------------------------------- */
/*e \dref_HistoryQosPolicyKind
 */
typedef enum
{
    /*e \dref_HistoryQosPolicyKind_KEEP_LAST_HISTORY_QOS
     */
    DDS_KEEP_LAST_HISTORY_QOS,

    /*i \dref_HistoryQosPolicyKind_KEEP_ALL_HISTORY_QOS
     */
    DDS_KEEP_ALL_HISTORY_QOS
} DDS_HistoryQosPolicyKind;

/*ci \brief The maximum allowed history depth.
 *
 * \details
 * The maximum depth is the maximum positive integer which can be
 * represented by a 32 bit signed integer.
 */
#define DDS_MAX_DEPTH_HISTORY_QOS (2147483647)

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

/*e \dref_DURABILITY_QOS_POLICY_NAME
 */
extern DDSCDllVariable const char *const DDS_DURABILITY_QOS_POLICY_NAME;

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
{ DDS_VOLATILE_DURABILITY_QOS }

DDSC_QOS_POLICY_METHODS_DECL(DDS_DurabilityQosPolicy)

/* ----------------------------------------------------------------- */
/*                RESOURCE_LIMITS                                    */
/* ----------------------------------------------------------------- */
/*e \dref_ResourceLimitsQosGroupDocs
 */

/*e \dref_RESOURCELIMITS_QOS_POLICY_NAME
 */
extern DDSCDllVariable const char *const DDS_RESOURCELIMITS_QOS_POLICY_NAME;

/*e \dref_LENGTH_UNLIMITED
 */
extern DDSCDllVariable const DDS_Long DDS_LENGTH_UNLIMITED;

/*e \dref_LENGTH_AUTO
 */
extern DDSCDllVariable const DDS_Long DDS_LENGTH_AUTO;

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

    /*e \dref_PresentationQosPolicyAccessScopeKind_HIGHEST_OFFERED_PRESENTATION_QOS
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

/*ci \brief Default value for the DDS_PresentationQosPolicy
 */
extern DDSCDllVariable const struct DDS_PresentationQosPolicy DDS_PRESENTATION_QOS_PUBLICATION_DEFAULT;

/*ci \brief Default presentation Qos policy for a publisher.
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

/*e \dref_DATAREADERRESOURCELIMITS_QOS_POLICY_NAME
 */
extern DDSCDllVariable const char *const
                                 DDS_DATAREADERRESOURCELIMITS_QOS_POLICY_NAME;

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
    4\
}

DDSC_QOS_POLICY_METHODS_DECL(DDS_DataReaderResourceLimitsQosPolicy)

/* ----------------------------------------------------------------- */
/*                DATAWRITER_RESOURCE_LIMITS                         */
/* ----------------------------------------------------------------- */
/*e \dref_DataWriterResourceLimitsQosGroupDocs
 */

/*e \dref_DATAWRITERRESOURCELIMITS_QOS_POLICY_NAME
 */
extern DDSCDllVariable const char *const
                                 DDS_DATAWRITERRESOURCELIMITS_QOS_POLICY_NAME;

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

    DDSC_CPP_QOS_POLICY_METHODS(DDS_DataWriterResourceLimitsQosPolicy)
};

/*i \dref_DataWriterResourceLimitsQosPolicy_DEFAULT
 */
#define DDS_DATAWRITERRESOURCE_LIMITS_QOS_POLICY_DEFAULT { \
    16L, /* max_remote_readers */ \
    4\
}

DDSC_QOS_POLICY_METHODS_DECL(DDS_DataWriterResourceLimitsQosPolicy)

/* ----------------------------------------------------------------- */
/*                ENTITY_FACTORY                                     */
/* ----------------------------------------------------------------- */
/*e \dref_EntityFactoryQosGroupDocs
 */

/*e \dref_ENTITYFACTORY_QOS_POLICY_NAME
 */
extern DDSCDllVariable const char *const DDS_ENTITYFACTORY_QOS_POLICY_NAME;

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
/*                TYPESUPPORT                                        */
/* ----------------------------------------------------------------- */
/*i \dref_TypeSupportQosGroupDocs
*/

/*i \dref_TYPESUPPORT_QOS_POLICY_NAME
 */
extern DDSCDllVariable const char *const DDS_TYPESUPPORT_QOS_POLICY_NAME;

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
{ NULL }

DDSC_QOS_POLICY_METHODS_DECL(DDS_TypeSupportQosPolicy)

/* ----------------------------------------------------------------- */
/*             SYSTEM_RESOURCE_LIMITS_X (eXtension QoS)              */
/* ----------------------------------------------------------------- */
/*e \dref_SystemResourceLimitsQosGroupDocs
 */

/*e \dref_SYSTEMRESOURCELIMITS_QOS_POLICY_NAME
 */
extern DDSCDllVariable const char *const DDS_SYSTEMRESOURCELIMITS_QOS_POLICY_NAME;

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

/*ci
 * The default maximum number of components which can be registered
 */
#if UDP_TRANSFORMS_ENABLED
#define DDS_SYSTEM_RESOURCE_LIMITS_MAX_COMPONENTS_DEFAULT 64L
#else
#define DDS_SYSTEM_RESOURCE_LIMITS_MAX_COMPONENTS_DEFAULT 64L
#endif /* UDP_TRANSFORMS_ENABLED */

/*i \dref_SystemResourceLimitsQosPolicy_DEFAULT
 */
#define DDS_SYSTEM_RESOURCE_LIMITS_QOS_POLICY_DEFAULT \
{\
    1L,\
    DDS_SYSTEM_RESOURCE_LIMITS_MAX_COMPONENTS_DEFAULT \
}

DDSC_QOS_POLICY_METHODS_DECL(DDS_SystemResourceLimitsQosPolicy)

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

/*e \dref_WIREPROTOCOL_QOS_POLICY_NAME
 */
extern DDSCDllVariable const char *const DDS_WIREPROTOCOL_QOS_POLICY_NAME;

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
#define DDS_PROTOCOLVERSION { 2, 1 }

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
DDSC_VARIABLE_LENGTH_VALUE_TYPE_SUPPORT_BASIC(DDS_ProductVersion);

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


/*ci \brief The default NACK period for RTPS
*/
#define DDS_RTPS_NACK_PERIOD_DEFAULT \
{\
  0,50 * 1000000 /* 50ms nack_period */ \
}

/*ci \brief The default reliability settings for a DataReader
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

/*e \dref_DATAREADERPROTOCOL_QOS_POLICY_NAME
 */
extern DDSCDllVariable const char *const DDS_DATAREADERPROTOCOL_QOS_POLICY_NAME;

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

/*e \dref_DATAWRITERPROTOCOL_QOS_POLICY_NAME
 */
extern DDSCDllVariable const char *const DDS_DATAWRITERPROTOCOL_QOS_POLICY_NAME;

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

    /*i \dref_DataWriterProtocolQosPolicy_serialize_on_write
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

/*e \dref_TRANSPORT_QOS_POLICY_NAME
 */
extern DDSCDllVariable const char *const DDS_TRANSPORT_QOS_POLICY_NAME;

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

/*e \dref_DOMAINPARTICIPANTRESOURCELIMITS_QOS_POLICY_NAME
 */
extern DDSCDllVariable const char *const
                            DDS_DOMAINPARTICIPANTRESOURCELIMITS_QOS_POLICY_NAME;

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

    /*e \dref_DomainParticipantResourceLimitsQosPolicy_max_receive_ports
     */
    DDS_Long max_receive_ports;

    /*e \dref_DomainParticipantResourceLimitsQosPolicy_max_destination_ports
     */
    DDS_Long max_destination_ports;

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
 */
/*i \dref_DomainParticipantReasourceLimitsQosPolicy_DEFAULT
 */
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
 8L,\
 8L \
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

#ifndef RTI_CERT
/*i \ingroup BuiltinTopicGroupDocs_copy_prefix  
 */
DDSCDllExport void
DDS_BuiltinTopicKey_copy_prefix(DDS_BuiltinTopicKey_t *a,
                                const DDS_BuiltinTopicKey_t *b);
#endif

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

/* ================================================================= */
/*                 Condition and Waitsets                            */
/* ================================================================= */

/*ce \dref_Entity
 */
typedef struct DDS_EntityImpl DDS_Entity;

/* ----------------------------------------------------------------- */
/*e \dref_ConditionsAndWaitsetsModuleDocs
 */

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

#ifndef RTI_CERT
/*i \dref_Condition_set_wrapper
 */
DDSCDllExport void
DDS_ConditionImpl_set_wrapper(DDS_Condition *self, void *wrapper);

/*i \dref_Condition_get_wrapper
 */
DDSCDllExport void**
DDS_ConditionImpl_get_wrapper_ref(DDS_Condition *self);
#endif

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

/*ci \dref_Entity_is_enabled
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

#ifndef RTI_CERT
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
#endif

/* ----------------------------------------------------------------- */
/*                ENTITY_NAME                                        */
/* ----------------------------------------------------------------- */
/*e \dref_EntityNameQosGroupDocs
 */

/*e \dref_ENTITYNAME_QOS_POLICY_NAME
 */
extern DDSCDllVariable const char *const DDS_ENTITYNAME_QOS_POLICY_NAME;

/*e \dref_AUTO_NAME_ENTITY
 */
extern DDSCDllVariable const char *const DDS_AUTO_NAME_ENTITY;

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
    DDS_EntityNameQosPolicy() {}
    bool set_name(const char *const name);
    DDS_ReturnCode_t copy(const DDS_EntityNameQosPolicy& from);
#ifdef RTI_CERT
    private:
#else
    public:
#endif
    DDS_EntityNameQosPolicy(const DDS_EntityNameQosPolicy& from);
    DDS_EntityNameQosPolicy& operator=(const char *const name);
    DDS_EntityNameQosPolicy& operator=(const DDS_EntityNameQosPolicy& from);
    const DDS_EntityNameQosPolicy& operator=(const DDS_EntityNameQosPolicy& from) const;
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

/*e \dref_ManagementQosPolicy
 */
struct DDSCPPDllExport RTI_ManagementQosPolicy
{
    /*i \brief The entity being created is hidden and ignored by public APIs
     *         that applies to multiple entities, such as the
     *         delete_contained_entities(). Entities marked as hidden must
     *         always be deleted and managed individually.
     */
    DDS_Boolean is_hidden;

    /*e \dref_ManagementQosPolicy_is_anonymous
     */
    DDS_Boolean is_anonymous;

    /*i \brief If TRUE do not send a DISPOSE & UNREGISTER sample for unpublished
     *         instances, otherwise send DISPOSE & UNREGISTER samples even if
     *         no data was published for an instance.
     *
     *  \details
     *
     *  The flag disable_unregister_dispose_for_unpublished_instance
     *  is used to prevent UNREGISTER & DISPOSE samples from being sent if
     *  the instance was never published. The typical use is by a
     *  discovery plugin. If an endpoint was created and deleted, instance
     *  resources may have been reserved, but if the endpoint was never
     *  enabled, and thus the instance never published, there is no reason
     *  to send UNREGISTER & DISPOSE samples.
     *
     *  A value of TRUE means the feature is enabled.
     */
    DDS_Boolean disable_unregister_dispose_for_unpublished_instance;

    DDSC_CPP_QOS_POLICY_METHODS(RTI_ManagementQosPolicy)
};

/*i \dref_ManagementQosPolicy_DEFAULT
 */
#define RTI_MANAGEMENT_QOS_POLICY_DEFAULT { \
    DDS_BOOLEAN_FALSE,\
    DDS_BOOLEAN_FALSE, \
    DDS_BOOLEAN_FALSE \
}

DDSC_QOS_POLICY_METHODS_DECL(RTI_ManagementQosPolicy)

/* ================================================================= */
/*                            DDS_DomainEntity                       */
/* ================================================================= */

/*ce \dref_DomainParticipant
 */
typedef struct DDS_DomainParticipantImpl DDS_DomainParticipant;

/* ----------------------------------------------------------------- */

/* ================================================================= */
/*                            DDSHST_History                           */
/* ================================================================= */
/*i \dref_DDSHST_ReturnCode_T
 */
typedef enum
{
    /*ci \brief Generic error code 
     */
    DDSHST_RETCODE_ERROR = -1000,
    
    /*ci \brief The history cache is full
     */
    DDSHST_RETCODE_NOSPACE,
    
    /*ci \brief A history entry already exist
     */
    DDSHST_RETCODE_EXISTS,
    
    /*ci \brief The history entry does not exist
     */
    DDSHST_RETCODE_NOT_EXISTS,
    
    /*ci \brief The history cacehe was created with an invalid proprty
     */
    DDSHST_RETCODE_INVALID_PROPERTY,
    
    /*ci \brief A request for a invalid cache entry was made
     */
    DDSHST_RETCODE_INVALID_ENTRY_REQUEST,
    
    /*ci \brief An API/Qos/operation was not supported 
     */     
    DDSHST_RETCODE_NOT_SUPPORTED,
    
    /*ci \brief The requested API succeeded
     */
    DDSHST_RETCODE_SUCCESS = 0
} DDSHST_ReturnCode_T;

/* ----------------------------------------------------------------- */
/*                WRITE_PARAMS                                       */
/* ----------------------------------------------------------------- */

/*i \dref_SampleIdentity_t
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

/*i \dref_WriteParams_t
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

/*i \dref_WriteParams_t_DEFAULT
 */
#define DDS_WRITEPARAMS_DEFAULT \
{ \
    DDS_SAMPLE_IDENTITY_UNKNOWN, \
    DDS_SAMPLE_IDENTITY_UNKNOWN, \
    DDS_TIME_ZERO, \
    DDS_HANDLE_NIL_NATIVE \
}

/* ================================================================= */
/*                            DDS Properties                         */
/* ================================================================= */
/* NOTE:
 * DDS_PropertySeq is not exposed as a general purpose property
 * sequence. It is only used to send required properties for RTI Tools.
 * Thus, although the generic sequence type is used to implement the property
 * sequence, its internal use is highly specialized and only a limited
 * number of functions are exposed.
 */
#define DDS_PropertySeq                 CDR_PropertySeq
#define DDS_Property                    CDR_Property
#define DDS_PropertySeq_initialize      CDR_PropertySeq_initialize
#define DDS_PropertySeq_set_length      CDR_PropertySeq_set_length
#define DDS_PropertySeq_get_length      CDR_PropertySeq_get_length
#define DDS_PropertySeq_get_reference   CDR_PropertySeq_get_reference

/*ci
 * \brief Get pointer to the DDS participant properties
 *
 * \details
 * The DDS domain participant may send additional information as properties
 * in the participant announcement. Typically this is used by discovery plugins
 * to serialize the information.
 *
 * \param[in] self Participant to get properties for
 *
 * \return A sequence with 0 or more properties on success, NULL if no
 *        sequence exists.
 */
DDSCDllExport struct DDS_PropertySeq*
DDS_DomainParticipant_get_dds_properties(DDS_DomainParticipant *self);


/*ci
 * \brief Query the domain-participant if the specified locator is supported
 *
 * \param[in] self Participant to query
 *
 * \return DDS_BOOLEAN_TRUE if the locator is supported, DDS_BOOLEEAN_FALSE
 *         otherwise.
 */
DDSCDllExport DDS_Boolean
DDS_DomainParticipant_locator_is_supported(DDS_DomainParticipant *self,
                                           struct DDS_Locator *locator);

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif /* dds_c_infrastructure_h */
