/*
 * FILE: DataReaderQos.h - DataReaderQos implementation
 *
 * (c) Copyright 2008-2024 Real-Time Innovations,
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
 * 20sep2014,as Exposed DDS_DataReaderQos_is_equal within module dds_c
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief DataReaderQos implementation
 */
/*ci \addtogroup DDSSubscriptionModule
 * @{
 */
#ifndef DataReaderQos_h
#define DataReaderQos_h

extern DDS_ReturnCode_t
DDS_DataReaderQos_set_from(
        struct DDS_DataReaderQos *out,
        const struct DDS_DataReaderQos *in,
        DDS_Boolean shallow_copy,
        DDS_DomainParticipant *participant);

#ifndef RTI_CERT
extern DDS_ReturnCode_t
DDS_DataReaderQos_finalize_managed(
        struct DDS_DataReaderQos * self,
        DDS_DomainParticipant *participant);
#endif

MUST_CHECK_RETURN extern DDS_Boolean
DDS_DataReaderQos_immutable_is_equal(const struct DDS_DataReaderQos *left,
                                     const struct DDS_DataReaderQos *right);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_DataReaderQos_is_consistent(const struct DDS_DataReaderQos *self);

#endif

/*ci @} */

