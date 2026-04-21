/*
 * FILE: TopicDescription.h - DDS TopicDescription implementation
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
 * \brief DDS TopicDescription implementation
 */
/*ci \addtogroup DDSDomainModule
 * @{
 */
#ifndef TopicDescription_h
#define TopicDescription_h

/*ci
 * \brief Base-class for all topic descriptions
 */
struct DDS_TopicDescriptionImpl
{
    /*ci
     * \brief Reference to a derived class' topic-name
     */
    const char *topic_name;

    /*ci
     * \brief Reference to a derived class' type-name
     */
    const char *type_name;

    /*ci
     * \brief Reference to a derived class' participant
     */
    DDS_DomainParticipant *participant;

    /*ci
     * \brief Pointer to the derived class the includes this structure
     */
    void *owner;
};

extern void
DDS_TopicDescriptionImpl_initialize(struct DDS_TopicDescriptionImpl *td,
                                    const char *topic_name,
                                    const char *type_name,
                                    void *topic,
                                    DDS_DomainParticipant *participant);

#ifndef RTI_CERT
extern void
DDS_TopicDescriptionImpl_finalize(struct DDS_TopicDescriptionImpl *td);
#endif /* !RTI_CERT */

#endif

/*ci @} */

