/*
 * FILE: rtps_rtps_impl.h 
 *
 * Copyright 2004-2021 Real-Time Innovations, Inc.
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
 * 23feb2015,eh  MICRO-1081: fix function/var names to conform to coding std
 * 23feb2015,eh  MICRO-1075: remove and replace macros
 * 09may2014,eh  MICRO-261 (Verocel PR#1421): remove RTPS_SubmessageId_toString
 *               for Cert
 * 22oct2008,rmw Added well-known ports
 * 26jun2008,rmw Added new functions for RTPS 2.0 support
 * 01dec2004,cc  Created, based on Waveworks tree.
 */

/*e \file
 * \brief Implementation of RTPS interface functions and types 
 *  
 * \details 
 * RTPS protocol defined types, implemented in C. 
 *  
 *  
 */

#ifndef rtps_rtps_impl_h
#define rtps_rtps_impl_h


#ifndef reda_sequenceNumber_h
#include "reda/reda_sequenceNumber.h"
#endif

/* -------------------------------------------------------------------------- */
/*ci \brief Get major field of vendor ID */
#define RTPS_VendorId_get_major(vendor)         (DDS_Octet)(*(vendor) >> 8)

/*ci \brief Get minor field of vendor ID */
#define RTPS_VendorId_get_minor(vendor)         (DDS_Octet)(*(vendor) & 0x00ff)

/* -------------------------------------------------------------------------- */
/*ci \brief Deserialize epoch of HEARTBEAT or ACKNACK from stream buffer */
#define RTPS_Epoch_deserialize(me_, stream_, swap_) \
    CDR_deserialize_long((stream_), (me_), (swap_))

/*ci \brief Serialized size of 2 octets */
#define RTPS_get_2_octets_max_size_serialized(size_) (2)

#endif /* rtps_rtps_impl_h */
