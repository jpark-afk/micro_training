/*
 * FILE: cdr_serialize_impl.h - CDR serialization implementation
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
 * 23feb2015,eh   MICRO-1075: remove and replace macros
 * 15sep2014,eh   Updated documentation
 * 03jun2013 kaj MICRO-418 remove variant for (de)serialize long double macro
 * 24mar2012,kaj  Written
 */
/*ci
 * \file 
 * \defgroup CDRSerializeImplClass CDR Serialization Impl
 * \ingroup CDRModule 
 * \brief Serialization implementation 
 *  
 */

/*ci \addtogroup CDRSerializeImplClass
 *   @{
 */
#ifndef cdr_serialize_impl_h
#define cdr_serialize_impl_h

/*ci \brief Get size of serialized and aligned byte 
 * \param[in] currentSize Current position 
 *  
 * \return Size of serialized and aligned byte 
 */
#define CDR_get_1_byte_max_size_serialized(currentSize) (1)

#endif /* cdr_serialize_impl_h */

/*ci @} */
