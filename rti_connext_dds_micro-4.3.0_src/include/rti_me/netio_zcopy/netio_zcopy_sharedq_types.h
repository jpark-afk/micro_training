/*
 * FILE: netio_zcopy_sharedq_types.h - Common definitions for Zero Copy sample queue
 *
 * (c) Copyright 2022-2026 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*ci
 * \addtogroup NETIO_ZCOPY_SharedQGroup
 * @{
 */
/*ci
 * \file
 * \brief Zero Copy Shared queue types
 */
#ifndef NETIOZCOPYSharedQTypes_h
#define NETIOZCOPYSharedQTypes_h

/* Include for common types used by multiple SQ definitions */

#include "osapi/osapi_config.h"
#include "osapi/osapi_time.h" /* for OSAPI_SystemTime */
#include "netio_zcopy/netio_zcopy_guid.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * Basic types
 */

 /*ci
  * \brief Version number for the Shared Queue
  */
typedef RTI_UINT16 SQ_VersionNumber;

/*ci
 * \brief Special value of SQ_VersionNumber, indicating "no value"
 */
#define SQ_VERSION_NUMBER_NONE ((SQ_VersionNumber)-1)

/*ci
 * \brief Type used for values representing an index in an array
 */
typedef RTI_UINT32 SQ_Index;

/*ci
 * \brief Special value of SQ_Index, indicating "no value"
 *
 * \details Because an index of 0 is a valid index, possibly containing a value,
 *          we will use the maximum value that an SQ_Index type can hold by
 *          casting a -1 to the (unsigned) SQ_Index type.
 */
#define SQ_INDEX_NONE ((SQ_Index)-1)

/*ci
 * \brief Type for holding values containing sizes (in bytes)
 */
typedef RTI_UINT64 SQ_Length;

/*ci
 * \brief Opaque pointer to a blob of bytes not understood by SharedQ
 */
typedef unsigned char *SQ_MemPtr;

/*ci
 * \brief Opaque const pointer to a blob of bytes not understood by SharedQ
 */
typedef const unsigned char *SQ_MemPtr_Const;

/*ci
 * \brief Observation modes with increasing consistency guarantees
 */
typedef enum
{
    /*ci
     * \brief No mode has been set
     */
    SQ_CSTY_MODE_NONE = 0,

    /*ci
     * \brief No protection against Samples being overwritten while being read
     */
    SQ_CSTY_MODE_WEAK = 1,

    /*ci
     * \brief Samples cannot be overwritten by a writer while being accessed by
     *        a reader
     */
    SQ_CSTY_MODE_STRONG = 2,

    /*ci
     * \brief Strong consistency mode with additional recovery mechanism from
     *        dead readers
     */
    SQ_CSTY_MODE_ROBUST = 3
} SQ_ConsistencyMode;

/*
 * Definitions for SampleInfo
 */

/*ci
 * \brief 64-bit sequence number of a DDS sample
 */
struct SQ_SequenceNumber
{
    /*ci
     * \brief First half 32 bits of the sequence number
     */
    RTI_INT32 high;

    /*ci
     * \brief Last half 32 bits of the sequence number
     */
    RTI_UINT32 low;
};

/*ci
 * \brief What kind of Sample depends on how it was committed
 */
typedef enum
{
    /*ci
     * \brief Empty sample uses for registering
     */
    SQ_SAMPLE_KIND_NONE = 1 << 0,

    /*ci
     * \brief User data sample
     */
    SQ_SAMPLE_KIND_WRITE = 1 << 1,

    /*ci
     * \brief The sample is the result of a dispose operation
     */
    SQ_SAMPLE_KIND_DISPOSE = 1 << 2,

    /*ci
     * \brief The sample is the result of an unregister operation
     */
    SQ_SAMPLE_KIND_UNREGISTER = 1 << 3,

    /*ci
     * \brief The sample is the result of a pulse operation
     */
    SQ_SAMPLE_KIND_PULSE = 1 << 4
} SQ_SampleKind_t;

/*ci
 * \brief Contain DDS-related data
 */
struct SQ_SampleInfo
{
    /*ci
     * \brief Globally unique identifier for the DDS instance
     *        associated with this sample
     */
    struct ZCOPY_Guid key;

    /*ci
     * \brief Sequence number representing where this sample lives in
     *        the order of all known samples written by a writer
     */
    struct SQ_SequenceNumber seq_nr;

    /*ci
     * \brief Time when the sample was committed by the writer
     */
    OSAPI_SystemTime timestamp;

    /*ci
     * \brief Sample kind
     */
    SQ_SampleKind_t kind;
};

/*ci
 * \brief Return code for SharedQueue module
 */
typedef enum
{
    /*ci
     * \brief Generic catch all return code in case of failure
     */
    SQ_RETCODE_ERROR = 0,

    /*ci
     * \brief Function call was successful
     */
    SQ_RETCODE_OK,

    /*ci
     * \brief No pre-allocated resources are available
     */
    SQ_RETCODE_OUT_OF_RESOURCES
} SQ_ReturnCode_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NETIOZCOPYSharedQTypes_h */

/*ci @} */
