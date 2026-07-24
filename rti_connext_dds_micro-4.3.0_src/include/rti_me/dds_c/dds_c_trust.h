/*
 * FILE: dds_c_trust.h - DDS core trust extensions
 *
 * (c) Copyright, Real-Time Innovations, 2012-2026.
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
 * 16mar2017,as  Created
 */


#ifndef dds_c_trust_h
#define dds_c_trust_h

#ifndef dds_c_dll_h
#include "dds_c/dds_c_dll.h"
#endif

#ifndef cdr_cdr_type_h
#include "cdr/cdr_cdr_type.h"
#endif

#ifndef dds_c_common_h
#include "dds_c/dds_c_common.h"
#endif

#ifndef dds_c_infrastructure_h
#include "dds_c/dds_c_infrastructure.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif



/* prefix for RTI specific properties*/
/* in 4.2.0 the following are supported with this prefix
 * prefix + cryptography.max_blocks_per_session
 * prefix + files_poll_interval
*/
#define DDS_TRUST_RTI_PROPERTY_PREFIX "com.rti.serv.secure."
/* prefix for RTI specific properties*/

#define DDS_TRUST_MAX_BLOCKS_PER_SESSION_PROPERTY DDS_TRUST_RTI_PROPERTY_PREFIX "cryptography.max_blocks_per_session"
#define DDS_TRUST_FILES_POLL_INTERVAL_PROPERTY DDS_TRUST_RTI_PROPERTY_PREFIX "files_poll_interval"

/* prefix for general propeties for the plugins */
/* in 4.2.0 the following properties are supported with this prefix
 * prefix + rtps_psk_symmetric_cipher_algorithm"
 * prefix + rtps_psk_secret_passphrase
*/
#define DDS_TRUST_PROPERTY_PREFIX "dds.sec.crypto"

#define DDS_TRUST_RTPS_PSK_SYMMETRIC_CIPHER_PROPERTY \
    DDS_TRUST_PROPERTY_PREFIX ".rtps_psk_symmetric_cipher_algorithm"

#define DDS_TRUST_RTPS_PSK_PASSPHRASE_PROPERTY \
    DDS_TRUST_PROPERTY_PREFIX ".rtps_psk_secret_passphrase"

#define DDS_TRUST_RTPS_PSK_PROTECTION_KIND_PROPERTY \
    "dds.sec.access.rtps_psk_protection_kind"

#define DDS_TRUST_RTPS_PSK_PASSTRACKER_ARG_PROPERTY \
    "psk_pass_tracker_arg"

/* max transform buffers that will be allocated by rtps
 * This value is hardcoded to 2 to hold the scenario
 * in which some receiving thread's listener sends a
 * message while processing a received one.
 */
#define DDS_TRUST_MAX_TRANSFORM_BUFFERS (2U)



#ifdef __cplusplus
}
#endif

#endif


