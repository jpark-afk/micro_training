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
 * 14aug2012,kaj Written
 */
/*e
 * \file
 * \defgroup CDRLogCodesClass CDR
 * \brief CDR. ModuleID = 5
 * \ingroup LoggingModule
 */
#ifndef cdr_log_h
#define cdr_log_h

#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif

/*e
 * \brief Failed to allocate stream or stream buffer
 * \ingroup CDRLogCodesClass
 */
#define CDR_LOG_STREAM_ALLOC_EC                               (CDR_LOG_BASE + 1)
#define CDR_LOG_STREAM_ALLOC(level_,size_) \
OSAPI_LOG_ENTRY_ADD_1UINT((level_),CDR_LOG_STREAM_ALLOC_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"size",(size_))

/*e
 * \brief Failed to set stream's current offset
 * \ingroup CDRLogCodesClass
 */
#define CDR_LOG_SET_OFFSET_EC                                 (CDR_LOG_BASE + 2)
#define CDR_LOG_SET_OFFSET(level_,offset_) \
OSAPI_LOG_ENTRY_ADD_1UINT((level_),CDR_LOG_SET_OFFSET_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"offset",(offset_))


/*e
 * \brief Failed to increament stream's current offset
 * \ingroup CDRLogCodesClass
 */
#define CDR_LOG_INCR_OFFSET_EC                                (CDR_LOG_BASE + 3)
#define CDR_LOG_INCR_OFFSET(level_,offset_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),CDR_LOG_INCR_OFFSET_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"offset",(offset_))

/*e
 * \brief Unsupported CDR encapsulations were received. The sample
 * is dropped.
 *
 * \ingroup CDRLogCodesClass
 */
#define CDR_LOG_UNSUPPORTED_OPTIONS_EC                        (CDR_LOG_BASE + 4)
#define CDR_LOG_UNSUPPORTED_OPTIONS(level_,options_) \
OSAPI_LOG_ENTRY_ADD_1UINT((level_),CDR_LOG_UNSUPPORTED_OPTIONS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"options",(options_))

#endif /* cdr_log_h */
