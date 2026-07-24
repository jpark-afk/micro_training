/*
 * FILE: PublisherEvent.h - PublisherEvent implementation
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
 * 23mar2013,tk Updated logging
 * 05jul2013,tk Updated
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief PublisherEvent implementation
 */
/*ci \addtogroup DDSPublicationModule
 * @{
 */
#ifndef PublisherEvent_h
#define PublisherEvent_h

MUST_CHECK_RETURN extern DDS_Boolean
DDS_PublisherEvent_on_offered_incompatible_qos(DDS_Publisher *self,
                    DDS_DataWriter *writer,
                    const struct DDS_OfferedIncompatibleQosStatus *status);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_PublisherEvent_on_publication_matched(DDS_Publisher *self,
                     DDS_DataWriter *writer,
                     const struct DDS_PublicationMatchedStatus *status);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_PublisherEvent_on_liveliness_lost(DDS_Publisher *self,
                                  DDS_DataWriter *writer,
                                  const struct DDS_LivelinessLostStatus *status);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_PublisherEvent_on_offered_deadline_missed(DDS_Publisher *self,
                    DDS_DataWriter *writer,
                    const struct DDS_OfferedDeadlineMissedStatus *status);

MUST_CHECK_RETURN extern DDS_Boolean
DDS_PublisherEvent_on_reliable_reader_activity_changed(DDS_Publisher *self,
                  DDS_DataWriter *writer,
                  const struct DDS_ReliableReaderActivityChangedStatus *status);

#endif

/*ci @} */

