/*
 * FILE: TopicDescription.c - DDS TopicDescription implementation
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
 *
 * \details
 * This file implements the public TopicDescription API.
 */
/*ci \addtogroup DDSDomainModule
 * @{
 */
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif

#include "TopicDescription.h"

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Initialize a topic-description
 *
 * \param[in] td          A topic description to initialize
 * \param[in] topic_name  A reference to the topic-descriptions name
 * \param[in] type_name   A reference to the topic-descriptions type-name
 * \param[in] topic       A reference to the topic to support conversion from
 *                        a topic-description to a topic
 * \param[in] participant Participant the topic-description belongs to
 */
void
DDS_TopicDescriptionImpl_initialize(struct DDS_TopicDescriptionImpl *td,
                                    const char *topic_name,
                                    const char *type_name,
                                    void *topic,
                                    DDS_DomainParticipant *participant)
{
    td->topic_name = topic_name;
    td->type_name = type_name;
    td->owner = topic;
    td->participant = participant;
}

#ifndef RTI_CERT
/*ci
 * \brief Finalize a Topic-description
 *
 * \param[in] td topic-description to finalize
 */
void
DDS_TopicDescriptionImpl_finalize(struct DDS_TopicDescriptionImpl *td)
{
    UNUSED_ARG(td);
}
#endif /* !RTI_CERT */

/*******************************************************************************
 *                              Public API
 ******************************************************************************/
const char*
DDS_TopicDescription_get_type_name(DDS_TopicDescription *self)
{
    struct DDS_TopicDescriptionImpl *t = (struct DDS_TopicDescriptionImpl*)self;

    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                          return NULL,
                          OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)
    return t->type_name;
}

const char*
DDS_TopicDescription_get_name(DDS_TopicDescription *self)
{
    struct DDS_TopicDescriptionImpl *t = (struct DDS_TopicDescriptionImpl*)self;

    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                          return NULL,
                          OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return t->topic_name;
}

DDS_DomainParticipant*
DDS_TopicDescription_get_participant(DDS_TopicDescription *self)
{
    struct DDS_TopicDescriptionImpl *t = (struct DDS_TopicDescriptionImpl*)self;

    OSAPI_PRECONDITION_ALWAYS(self == NULL,
                          return NULL,
                          OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    return t->participant;
}

/*ci @} */
