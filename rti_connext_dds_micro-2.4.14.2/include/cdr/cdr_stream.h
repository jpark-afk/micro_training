/*
 * FILE: cdr_stream.h - CDR stream API
 *
 * (c) Copyright, Real-Time Innovations, 2012-2022.
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * -------------------- 
 * 08sep2022,tk MICRO-3367/PR.30121
 * - Changed little_endian to endian and updated function prototype
 *   for CDR_Stream_endian_and_byteswap_get and
 *   CDR_Stream_endian_and_byteswap_set to match implementation.
 * 10jan2021,tk   MICRO-2807 / PR.27549  Removed unused functions for CERT
 * 13dec2020,tk
 *     - MICRO-2710/PR.28397
 *         - Changed littleEndian to little_endian in the function header
 *           comment for CDR_Stream_byteswap_set()
 * 23feb2015,eh   MICRO-1075: remove and replace macros
 * 03jun2013,kaj  MICRO-193 (verification cdrs != NULL made resp. of caller)
 * 24mar2012,kaj  Written
 */
/*ci
 * \file 
 * \defgroup CDRStreamClass CDR Stream 
 * 
 * \brief CDR Stream  
 *  
 * \details 
 * An abstraction for serializing and deserializing CDR types to and from a 
 * buffer.  Enforces alignment and endianness. 
 *  
 */
/*ci \addtogroup CDRStreamClass
 *   @{
 */
#ifndef cdr_stream_h
#define cdr_stream_h

#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif

#ifndef cdr_dll_h
#include "cdr/cdr_dll.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#define CDR_MAX_ALIGNMENT     8

/*i \ingroup RTICdrStreamModule
 * Big endian in CDR.
 */
#define RTI_CDR_ENDIAN_BIG (0)

/*i \ingroup RTICdrStreamModule
 * Little endian in CDR
 */
#define RTI_CDR_ENDIAN_LITTLE (1)

/*i \ingroup RTICdrStreamModule
 * Byteswap has not been set
 */
#define CDR_BYTESWAP_INVALID  -1

/*ci \brief Major version of RTI vendor ID */
#define CDR_VENDOR_ID_MAJOR_RTI (0x01)

/*ci \brief Minor version of RTI Connext DDS Micro and Cert vendor ID */
#define CDR_VENDOR_ID_MINOR_MICRO (0x0A)

/*ci \brief Minor version of RTI Connext DDS Core vendor ID */
#define CDR_VENDOR_ID_MINOR_CORE (0x01)

/*ci \brief The stream was not received with a checksum
 */
#define CDR_STREAM_CHECKSUM_NONE       (0)

/*ci \brief The stream was validated with a builtinCRC2 checksum
 */
#define CDR_STREAM_CHECKSUM_BUILTIN32  (2)

/*ci \brief If the 3 LSB are non-zero, the stream was validated with a checksum.
 */
#define CDR_STREAM_CHECKSUM_MASK       (0x7)

/*ci \brief The stream was received with CRC32 submsg. If a checksum
 *          was validated on the stream the 3 LSB indicates the checksum
 *          function as defined by DDS and RTPS.
 */
#define CDR_STREAM_CHECKSUM_CRC32      (0x8)

/*ci \brief The stream was received with a RTI Header Extension submsg.
 */
#define CDR_STREAM_CHECKSUM_HDREXT     (0x10)

/* -------------------------------------------------------------------------- */
/*ci \brief Abstraction for serializing to and deserializing from a buffer */
struct CDR_Stream_t
{
    /*ci \brief Allocated buffer */
    char *real_buff;           

    /*ci \brief Buffer aligned on 8-byte boundary */
    char *buffer;            

    /*ci \brief Address used for alignment calculations */
    char *align_base;    
    
    /*ci \brief Current location in buffer */        
    char *buff_ptr;      
    
    /*ci \brief Length of the aligned buffer */        
    RTI_UINT32 length;   
    
    /*ci \brief Whether to byte swap during serialization/deserialization to 
     *   be consistent with stream's endianness
     */
    RTI_BOOL need_byte_swap;

    /*ci \brief Endianness of stream, either RTI_CDR_ENDIAN_LITTLE or 
     * RTI_CDR_ENDIAN_BIG
     */
    RTI_UINT16 endian;

    /*ci \brief The major vendor ID for this stream
     */
    RTI_UINT8 vendor_id_major;

    /*ci \brief The minor vendor ID for this stream
     */
    RTI_UINT8 vendor_id_minor;

    /*ci \brief The stream has been validated with a CRC
     */
    RTI_UINT32 checksum_info;
};

/*ci \brief Set stream to specified alignment 
 * 
 * \param[in] stream Stream to set, must be valid pointer
 * \param[in] alignment Alignment to set
 */
CDRDllExport void
CDR_Stream_align(struct CDR_Stream_t *stream, 
                 RTI_UINT8 alignment);


#ifndef RTI_CERT
/*ci 
 * \brief 
 * Free resources of a stream 
 *  
 * \param[in] cdrs Stream to free 
 *   
 */
CDRDllExport void
CDR_Stream_free(struct CDR_Stream_t *cdrs);
#endif

/*ci 
 * \brief 
 * Allocate and return a new stream 
 *  
 * \param[in] buffsize Size in bytes of stream buffer
 *  
 * \return Pointer to allocated stream, is NULL if allocation failed 
 */
MUST_CHECK_RETURN CDRDllExport struct CDR_Stream_t*
CDR_Stream_alloc(RTI_UINT32 buffsize);

/*ci 
 * \brief 
 * Reset stream pointer to start of buffer 
 *  
 * \param[in] cdrs Stream to reset 
 *   
 */
CDRDllExport void
CDR_Stream_reset(struct CDR_Stream_t *cdrs);

/*ci
 * \brief 
 * Set byte swap state of stream 
 *  
 * \param[in] cdrs Stream to set 
 * \param[in] little_endian Flag to set stream for little endian byte order
 *  
 */
CDRDllExport void
CDR_Stream_byteswap_set(struct CDR_Stream_t *cdrs, RTI_BOOL little_endian);

/*ci 
 * \brief 
 * Get endian and byte swap state of stream 
 *  
 * \param[in] cdrs Stream to set 
 * \param[out] need_byte_swap TRUE if byte-swapped is needed
 * \param[out] endian Set to TRUE If the endianess is little endian
 *  
 */
CDRDllExport void
CDR_Stream_endian_and_byteswap_get(struct CDR_Stream_t *cdrs, 
                                   RTI_BOOL *need_byte_swap, 
                                   RTI_UINT16 *endian);

/*ci 
 * \brief 
 * Get endian and byte swap state of stream 
 *  
 * \param[in] cdrs Stream to set
 * \param[in] need_byte_swap TRUE if byte-swapped is needed
 * \param[in] endian If TRUE, set stream for little endian byte order
 *  
 */
CDRDllExport void
CDR_Stream_endian_and_byteswap_set(struct CDR_Stream_t *cdrs, 
                                   RTI_BOOL need_byte_swap, 
                                   RTI_UINT16 endian);

/*e \dref_CDR_Stream_get_current_position_offset
 */
MUST_CHECK_RETURN CDRDllExport RTI_UINT32
CDR_Stream_get_current_position_offset(struct CDR_Stream_t *cdrs);

/*e \dref_CDR_Stream_set_current_position_offset
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_set_current_position_offset(struct CDR_Stream_t *cdrs,RTI_UINT32 num);

/*e \dref_CDR_Stream_increment_current_position
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_increment_current_position(struct CDR_Stream_t *me,RTI_INT32 amount);

/*ci 
 * \brief 
 * Assign buffer to stream 
 *  
 * \param[in] me Stream 
 * \param[in] buf Pointer of new stream buffer 
 * \param[in] length Length in bytes of new stream buffer 
 *  
 * \return RTI_TRUE on success with new stream buffer assigned.  RTI_FALSE on 
 * failure.
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_set_buffer(struct CDR_Stream_t *me,char *buf, RTI_UINT32 length);


/*e \dref_CDR_Stream_check_size
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_check_size(struct CDR_Stream_t *me, RTI_UINT32 size); 

/*e \dref_CDR_Stream_get_current_position_ptr
 */
MUST_CHECK_RETURN CDRDllExport char*
CDR_Stream_get_current_position_ptr(struct CDR_Stream_t *me);

/*ci
 * \brief
 * Return the size of the encapsulation header based on the logical size of a
 * stream.
 *
 * \param[in] size Current size of a stream
 *
 * \return Return the size of the encapsulation header
 */
MUST_CHECK_RETURN CDRDllExport RTI_UINT32
CDR_Stream_get_encapsulation_size(RTI_UINT32 size);

/*e \dref_CDR_Stream_is_byte_swapped
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_is_byte_swapped(struct CDR_Stream_t *me);

/*ci
 * \brief Set the vendor ID for the stream
 *
 * \param[in] me Stream
 * \param[in] vendor_id_major The major vendor ID
 * \param[in] vendor_id_minor The minor vendor ID
 *
 * \details
 * When deserializing data it is sometimes necessary to know if the data comes
 * from RTI or not due to RTI specific content, such as vendor specific
 * PIDs. This function sets the source of the stream.
 *
 */
CDRDllExport void
CDR_Stream_set_vendor(struct CDR_Stream_t *me,
                      RTI_UINT8 vendor_id_major,
                      RTI_UINT8 vendor_id_minor);

/*ci
 * \brief Check if the stream contains serialized data from RTI.
 *
 * \details
 * When deserializing data it is sometimes necessary to know if the data comes
 * from RTI or not due to RTI specific content, such as vendor specific
 * PIDs.
 *
 * \param[in] me Stream
 *
 * \return TRUE if the stream is from RTI, FALSE if not
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_is_vendor_rti(struct CDR_Stream_t *me);

/*ci
 * \brief Set checksum info
 *
 * \param[in] me Stream
 * \param[in] checksum_info Checksum flags
 *
 */
CDRDllExport void
CDR_Stream_set_checksum_info(struct CDR_Stream_t *me,RTI_UINT32 checksum_info);

#ifndef RTI_CERT
/*ci
 * \brief Check if a stream was validated with a checksum.
 *
 * \param[in] me      Stream
 *
 * \return Returns TRUE if the stream was validated with a checksum, FALSE if not.
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_is_checksum_protected(struct CDR_Stream_t *me);
#endif

/*ci
 * \brief Check if the stream was received with a CRC32 submsg
 *
 * \param[in] me Stream
 *
 * \return TRUE if the stream was received with a CRC32 submsg, FALSE if not.
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_is_checksum_crc32(struct CDR_Stream_t *me);

#ifndef RTI_CERT
/*ci
 * \brief Check if the stream was received with a Header Extension submsg
 *
 * \param[in] me Stream
 *
 * \return TRUE if the stream was received with a Header Extension submsg,
 *         FALSE if not.
 */
MUST_CHECK_RETURN CDRDllExport RTI_BOOL
CDR_Stream_is_checksum_hdrext(struct CDR_Stream_t *me);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* cdr_stream_h */

/*ci @} */
