/*
 * FILE: cdr_log.h CDR Log definitions
 * 
 * (c) Copyright, Real-Time Innovations, 2012-2015.
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 10dec2020,tk
 *     - MICRO-2712/PR#28305
 *       - Consistent use of in,out, and inout for parameter designations.
 * 23feb2015,eh  MICRO-1075: remove and replace macros
 * 15sep2014,eh   Updated documentation
 * 24mar2012,tk   Cleanup
 * 14aug2012,kaj  Written
 */

 /*ci
 * \file 
 * \defgroup CDREncapsulationClass CDR Data Encapsulation 
 * \ingroup CDRModule 
 * \brief CDR Data Encapsulation 
 *  
 * \details 
 * Data encapsulation headers defined by and for RTPS data payloads.
 *  
 */

/*ci \addtogroup CDREncapsulationClass
 *   @{
 */
#ifndef cdr_encapsulation_h
#define cdr_encapsulation_h

#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif

#ifndef cdr_serialize_h
#include "cdr/cdr_serialize.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*ci \brief OMG CDR Big Endian encapsulation ID */
#define RTI_CDR_ENCAPSULATION_ID_CDR_BE     ((RTI_UINT16)0x0000)

/*ci \brief OMG CDR Little Endian encapsulation ID */
#define RTI_CDR_ENCAPSULATION_ID_CDR_LE     ((RTI_UINT16)0x0001)

/*ci \brief Parameter List OMG CDR Big Endian encapsulation ID */
#define RTI_CDR_ENCAPSULATION_ID_PL_CDR_BE  ((RTI_UINT16)0x0002)

/*ci \brief Parameter List OMG CDR Little Endian encapsulation ID */
#define RTI_CDR_ENCAPSULATION_ID_PL_CDR_LE  ((RTI_UINT16)0x0003)

/*ci \brief Empty encapsulation options */
#define RTI_CDR_ENCAPSULATION_OPTIONS_NONE  ((RTI_UINT16)0x0000)

/*ci \brief The padding bits based on X-Types as defined by and RTPS 2.5.
 * Note that when compression is enabled, the padding bits are in a different
 * location which is in not supported.
 */
#define  RTI_ENCAPSULATION_OPTION_PADDING(options_) \
    ((options_) & 0x3U)

/*ci \brief Unsupported options bit that are expected to be 0.
 */
#define  RTI_ENCAPSULATION_OPTION_UNSUPPORTED(options_) \
    (((options_) & 0xfffffffc) != 0)

/*ci \brief Encapsulation header size */
#define RTI_CDR_ENCAPSULATION_HEADER_SIZE   4

/*ci \brief Encapsulation ID */
typedef RTI_UINT16 NDDSCDREncapsulationId;

/*ci \brief Encapsulation options */
typedef RTI_UINT16 NDDSCDREncapsulationOptions;

/*ci \brief CDR Encapsulation */
typedef struct NDDSCDREncapsulation
{
    /*ci \brief Encapsulation ID */
    NDDSCDREncapsulationId identifier;

    /*ci \brief Encapsulation options */
    NDDSCDREncapsulationOptions options;
} NDDSCDREncapsulation;

/*ci 
 * \brief 
 * Deserialize a CDR data encapsulation header from a stream 
 *  
 * \param[in] stream Deserialization stream
 *  
 * \return RTI_TRUE on success with stream set according to deserialized 
 * data encapsulation header. RTI_FALSE on failure 
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_deserialize_header(struct CDR_Stream_t *stream);

/*ci 
 * \brief 
 * Serialize a CDR data encapsulation header into a stream 
 *  
 * \param[in] stream Serialization stream
 * \param[in] is_guid Flag whether to use parameter/GUID data encapsulation   
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_serialize_header(struct CDR_Stream_t *stream,RTI_BOOL is_guid);

#ifdef __cplusplus
}                               /* extern "C" */
#endif


#endif /* cdr_encapsulation_h */

/*ci @} */
