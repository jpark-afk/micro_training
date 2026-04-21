/*
 * FILE: TopicQos.h - DDS TopicQos implementation
 *
 * (c) Copyright 2008-2015 Real-Time Innovations,
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
 * 06may2012,tk Major update
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \ingroup DDSDomainModule
 * \brief DDS TopicQos implementation
 */
/*ci \addtogroup DDSDomainModule
 * @{
 */
#ifndef TopicQos_h
#define TopicQos_h

#ifndef RTI_CERT
MUST_CHECK_RETURN extern DDS_Boolean
DDS_TopicQos_immutable_is_equal(const struct DDS_TopicQos *left,
                                const struct DDS_TopicQos *right);
#endif

#endif

/*ci @} */
