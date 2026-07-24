/*
 * FILE: PublisherQos.h - Publisher Qos implementation
 *
 * Copyright (c) 2008-2025 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 20sep2014,as Made DDS_PublisherQos_finalize public
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief Publisher Qos implementation
 * \ingroup DDSDomainModule
 */
/*ci \addtogroup DDSPublicationModule
 * @{
 */
#ifndef PublisherQos_h
#define PublisherQos_h

extern DDS_ReturnCode_t
DDS_PublisherQos_set_from(
        struct DDS_PublisherQos *out,
        const struct DDS_PublisherQos *in,
        DDS_Boolean shallow_copy,
        DDS_DomainParticipant *participant);

#ifndef RTI_CERT
extern DDS_ReturnCode_t
DDS_PublisherQos_finalize_managed(
        struct DDS_PublisherQos *self,
        DDS_DomainParticipant *participant);
#endif

#endif

/*ci @} */

