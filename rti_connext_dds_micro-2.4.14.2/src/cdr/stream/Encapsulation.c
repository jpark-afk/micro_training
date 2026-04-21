/*
 * FILE: Encapsulation.h - CDR encapsulation API
 *
 * (c) Copyright, Real-Time Innovations, 2012-2021.
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
 * 10dec2020,tk
 *     - MICRO-2712/PR#28305
 *       - Removed duplicate function header comments already in the public
 *         header-files.
 * 23feb2015,eh  MICRO-1075: remove and replace macros
 * 29jul2014,tk  MICRO-859/PR#10221 - Fixed Source code comment
 *               MICRO-860/PR#10222 - Check CDR options
 *               MICRO-861/PR#10224 - Removed redundant checks
 *               MICRO-862/PR#10226 - Do not realign buffer if not enough space
 * 09may2014,eh  MICRO-334 (Verocel PR#1462): commentary on
 *               (de)serialize_header() as internal APIs
 * 06jun2013,kaj MICRO-183: CDR stream alignment reset moved to (de)ser_header 
 * 14aug2012,kaj Written
 */
/*ci
 * \file @ingroup CDRModule
 * 
 * \brief CDR Data Encapsulation
 *
 * \details
 * Operations for serializing and deserializing CDR data encapsulation headers
 *
 * \details
 * This file implements miscellaneous function to related to the CDR
 * encapsulation header, such as setting kind of header.
 */
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef cdr_stream_h
#include "cdr/cdr_stream.h"
#endif
#ifndef cdr_serialize_h
#include "cdr/cdr_serialize.h"
#endif
#ifndef cdr_encapsulation_h
#include "cdr/cdr_encapsulation.h"
#endif
#ifndef cdr_log_h
#include "cdr/cdr_log.h"
#endif

/*** SOURCE_BEGIN ***/

/*
    Section 15.3.1.1 of the OMG specification for Common Data Representation:
 
    "Alignment is defined above as being relative to the beginning of an octet
    stream. ... Such octet streams begin at the start of a GIOP message header
    ... and at the beginning of an encapsulation, even if the encapsulation
    itself is nested in another encapsulation."
 
    The header announces the start of an encapsulation following, so the
    alignment base is reset after the header.
*/

/* CDR_Stream_deserialize_header() and CDR_Stream_serialize_header() are
 * internal APIs, whose * parameters are always valid (non-NULL) and thus 
 * do not require non-debug precondition checks. 
 */


RTI_BOOL
CDR_Stream_deserialize_header(struct CDR_Stream_t *cdrs)
{
    RTI_UINT16 kind;
    RTI_UINT16 options;
    RTI_BOOL retval = RTI_TRUE;

    OSAPI_PRECONDITION(cdrs == NULL,
                           return RTI_FALSE,
                           OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_TRUE);)

    if (!CDR_Stream_deserialize_unsigned_short_from_big_endian(cdrs, &kind))
    {
        return RTI_FALSE;
    }

    if (!CDR_Stream_deserialize_unsigned_short_from_big_endian(cdrs, &options))
    {
        return RTI_FALSE;
    }

    /* Reset stream's alignment to immediately following the option bits */
    (cdrs)->align_base = (cdrs)->buff_ptr;

    /* No option bits are defined for RTPS < 2.5. RTPS < 2.5 states that the
     * option bits shall be ignored (and 2.3 states they shall be set to 0).
     * However, the options bits may add meaning for how to interpret the
     * data, such as compression.
     *
     * Since it is not clear what the 14 MSB could mean not valid for
     * discard the sample.
     */
    if (RTI_ENCAPSULATION_OPTION_UNSUPPORTED(options))
    {
        CDR_LOG_UNSUPPORTED_OPTIONS(OSAPI_LOGKIND_ERROR,options)
        return RTI_FALSE;
    }

    /* No option bits are defined for RTPS < 2.5. For RTPS 2.5 and higher
     * the options bits are used by the X-Types specification. The
     * 2 LSB are the number of bits added to align the serialized data
     * to the next 4 bytes. The stream length is initialized to the length of the
     * DATA, DATA_BATCH, or DATA_FRAG submessage. It is not important that the
     * length is the length of serialied data, only that the length does not
     * go beyond the serialized data. Thus, substract the paddig from the
     * stream to get the actual length of the payload message (not the length
     * of the serialized data though.
     */
    cdrs->length -= RTI_ENCAPSULATION_OPTION_PADDING(options);

    switch (kind)
    {
        case RTI_CDR_ENCAPSULATION_ID_PL_CDR_BE:
        case RTI_CDR_ENCAPSULATION_ID_CDR_BE:
            if (cdrs->endian != RTI_CDR_ENDIAN_BIG)
            {
                cdrs->endian = RTI_CDR_ENDIAN_BIG;
                cdrs->need_byte_swap =
                    (cdrs->need_byte_swap == RTI_TRUE) ? RTI_FALSE : RTI_TRUE;
            }
            break;
        case RTI_CDR_ENCAPSULATION_ID_PL_CDR_LE:
        case RTI_CDR_ENCAPSULATION_ID_CDR_LE:
            if (cdrs->endian != RTI_CDR_ENDIAN_LITTLE)
            {
                cdrs->endian = RTI_CDR_ENDIAN_LITTLE;
                cdrs->need_byte_swap =
                    (cdrs->need_byte_swap == RTI_TRUE) ? RTI_FALSE : RTI_TRUE;
            }
            break;
        default:
            retval = RTI_FALSE;
            break;
    }

    return retval;
}

RTI_BOOL
CDR_Stream_serialize_header(struct CDR_Stream_t * cdrs, RTI_BOOL is_guid)
{
    RTI_UINT16 kind;
    RTI_UINT16 options;

    OSAPI_PRECONDITION(cdrs == NULL,
                           return RTI_FALSE,
                           OSAPI_Log_entry_add_pointer("cdrs",cdrs,RTI_TRUE);)

    if (is_guid)
    {
        kind = ((cdrs->endian == RTI_CDR_ENDIAN_BIG) ?
                RTI_CDR_ENCAPSULATION_ID_PL_CDR_BE :
                RTI_CDR_ENCAPSULATION_ID_PL_CDR_LE);
    }
    else
    {
        kind = ((cdrs->endian == RTI_CDR_ENDIAN_BIG) ?
                RTI_CDR_ENCAPSULATION_ID_CDR_BE :
                RTI_CDR_ENCAPSULATION_ID_CDR_LE);
    }

    options = RTI_CDR_ENCAPSULATION_OPTIONS_NONE;

    /* Make sure the stream is aligned before de-serializing the header */
    CDR_Stream_align(cdrs, CDR_SHORT_ALIGN);

    if (!CDR_Stream_serialize_unsigned_short_to_big_endian(cdrs, &kind))
    {
        return RTI_FALSE;
    }

    if (!CDR_Stream_serialize_unsigned_short_to_big_endian(cdrs, &options))
    {
        return RTI_FALSE;
    }

    /* Reset stream's alignment */
    (cdrs)->align_base = (cdrs)->buff_ptr;

    return RTI_TRUE;
}

/*ci \brief Get serialized size of stream with encapsulation header 
 * 
 * \param[in] size Current stream size 
 *  
 * \return Size in bytes with encapsulation header serialized  
 */
RTI_UINT32
CDR_Stream_get_encapsulation_size(RTI_UINT32 size)             
{
    size += CDR_get_max_size_serialized_unsigned_short(size);  
    size += CDR_get_max_size_serialized_unsigned_short(size);
    return size;
}
