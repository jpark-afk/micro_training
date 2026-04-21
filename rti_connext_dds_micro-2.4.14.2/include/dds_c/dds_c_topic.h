/*
 * FILE: dds_c_topic.h - DDS topic module
 *
 * (c) Copyright, Real-Time Innovations, 2008-2020.
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
 * 13apr2021,tk MICRO-3028/PR.28960
 * - Removed DDS_TopicQos_initialize,DDS_TopicQos_copy,
 *   DDS_TopicQos_is_equal from CERT
 * 07apr2016,tk MICRO-1541 Fixed assignment operator issues for C++
 * 29jun2015,tk MICRO-1351/PR#15141 Removed prototypes for non-Cert function
 * 20sep2014,as Explicitly define public support functions for Status types
 * 07may2014,as MICRO-784 Expose get_X_status API in C++
 * 19jul2013,as Added support for C++
 * 30apr2008    Created
 */
/*ce
 * \file
 * \brief DDS topic module
 * @addtogroup DDSDomainModule
 * @ingroup DDSCModule
 */
#ifndef dds_c_topic_h
#define dds_c_topic_h

#ifndef dds_c_dll_h
#include "dds_c/dds_c_dll.h"
#endif
#include "dds_c_config.h"
#ifndef dds_c_common_h
#include "dds_c/dds_c_common.h"
#endif
#ifndef dds_c_sequence_h
#include "dds_c/dds_c_sequence.h"
#endif
#ifndef dds_c_infrastructure_h
#include "dds_c/dds_c_infrastructure.h"
#endif

/* ================================================================= */
/*                       Basic Types                                 */
/* ================================================================= */

/*e \dref_InstanceId_t
 */
typedef struct DDS_BuiltinTopicKey_t DDS_InstanceId_t;


/* ================================================================= */
/*                       Status                                      */
/* ================================================================= */

/*e \dref_InconsistentTopicStatus
 */
struct DDSCPPDllExport DDS_InconsistentTopicStatus
{
    /*i \dref_InconsistentTopicStatus_total_count
     */
    DDS_Long total_count;

    /*i \dref_InconsistentTopicStatus_total_count_change
     */
    DDS_Long total_count_change;
    
    DDSC_CPP_STATUS_METHODS(DDS_InconsistentTopicStatus)
};

#ifdef __cplusplus
extern "C"
{
#endif

/*ci
 * \brief Initialize a DDS_InconsistentTopicStatus structure
 *
 * \param[in] self DDS_InconsistentTopicStatus to initialize.
 *
 * \return DDS_RETCODE_OK on success, one of the standard error codes on
 *         failure
 */
DDSCDllExport DDS_ReturnCode_t
DDS_InconsistentTopicStatus_initialize(
            struct DDS_InconsistentTopicStatus *self);

/*ce \dref_InconsistentTopicStatus_INITIALIZER
 */
#define DDS_InconsistentTopicStatus_INITIALIZER { 0L, 0L }

/*ci
 * \brief Reset the changed counters in DDS_InconsistentTopicStatus
 *
 * \param[in] s Structure to clear
 */
DDSCDllExport void
DDS_InconsistentTopicStatus_reset(struct DDS_InconsistentTopicStatus *s);

#ifdef __cplusplus
}                               /* extern "C" */
#endif

/* ================================================================= */
/*                          QoS                                      */
/* ================================================================= */

#ifdef RTI_WIN32
#pragma warning(push)
#pragma warning(disable: 4522)
#endif

/*i \dref_DDSTopicBuiltInTopicModule
 */

/*e \dref_TopicQos
 */
struct DDSCPPDllExport DDS_TopicQos
{
    /*i \dref_TopicQos_management
     */
    struct RTI_ManagementQosPolicy management;

    DDSC_CPP_QOS_METHODS(DDS_TopicQos)
};

#ifdef RTI_WIN32
#pragma warning(pop)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RTI_CERT
/*ci \dref_TopicQos_initialize
 */
DDSCDllExport DDS_ReturnCode_t
DDS_TopicQos_initialize(struct DDS_TopicQos *self);

/*ci \dref_TopicQos_copy
 */
DDSCDllExport DDS_ReturnCode_t
DDS_TopicQos_copy(struct DDS_TopicQos *out,
                  const struct DDS_TopicQos *in);

/*ci \dref_TopicQos_finalize
 */
DDSCDllExport DDS_ReturnCode_t
DDS_TopicQos_finalize(struct DDS_TopicQos *self);

/*ci \dref_TopicQos_is_equal
 */
DDSCDllExport DDS_Boolean
DDS_TopicQos_is_equal(const struct DDS_TopicQos *left,
                      const struct DDS_TopicQos *right);
#endif /* !RTI_CERT */

/*ci \dref_TopicQos_INITIALIZER
 */
#define DDS_TopicQos_INITIALIZER \
{ \
    RTI_MANAGEMENT_QOS_POLICY_DEFAULT  \
}

/* ================================================================= */
/*                   DDS_TopicDescription                            */
/* ================================================================= */

/*ce \dref_TopicDescription
 */
typedef struct DDS_TopicDescriptionImpl DDS_TopicDescription;

/*ce \dref_TopicDescription_get_type_name
 */
DDSCDllExport const char*
DDS_TopicDescription_get_type_name(DDS_TopicDescription * self);

/*ce \dref_TopicDescription_get_name
 */
DDSCDllExport const char*
DDS_TopicDescription_get_name(DDS_TopicDescription * self);

/*ce \dref_TopicDescription_get_participant
 */
DDSCDllExport DDS_DomainParticipant*
DDS_TopicDescription_get_participant(DDS_TopicDescription * self);

/*ce \dref_Topic
 */
typedef struct DDS_TopicImpl DDS_Topic;

/* ================================================================= */
/*                       Listeners                                   */
/* ================================================================= */

/*ce \dref_TopicListener_InconsistentTopicCallback
 */
typedef void
(*DDS_TopicListener_InconsistentTopicCallback)(
        void *listener_data,
        DDS_Topic *topic,
        const struct DDS_InconsistentTopicStatus *status);

/*ce \dref_TopicListener
 */
struct DDS_TopicListener
{
    /*ce \dref_TopicListener_as_listener
     */
    struct DDS_Listener as_listener;

    /*ce \dref_TopicListener_on_inconsistent_topic
     */
    DDS_TopicListener_InconsistentTopicCallback on_inconsistent_topic;
};

/*ce \dref_TopicListener_INITIALIZER
 */
#define DDS_TopicListener_INITIALIZER \
{ DDS_Listener_INITIALIZER, (DDS_TopicListener_InconsistentTopicCallback)NULL}

/* ================================================================= */
/*                           Topic                                   */
/* ================================================================= */
/*ce \dref_Topic_as_entity
 */
DDSCDllExport DDS_Entity*
DDS_Topic_as_entity(DDS_Topic * self);

/*ce \dref_Topic_as_topicdescription
 */
DDSCDllExport DDS_TopicDescription*
DDS_Topic_as_topicdescription(DDS_Topic * self);

/*ce \dref_Topic_narrow
 */
DDSCDllExport DDS_Topic*
DDS_Topic_narrow(DDS_TopicDescription *self);

/*ce \dref_Topic_get_inconsistent_topic_status
*/
DDSCDllExport DDS_ReturnCode_t
DDS_Topic_get_inconsistent_topic_status(
        DDS_Topic *self,
        struct DDS_InconsistentTopicStatus *status);

#if INCLUDE_API_QOS
/*ce \dref_Topic_set_qos
 */
DDSCDllExport DDS_ReturnCode_t
DDS_Topic_set_qos(
        DDS_Topic *self,
        const struct DDS_TopicQos *qos);

/*ce \dref_Topic_get_qos
 */
DDSCDllExport DDS_ReturnCode_t
DDS_Topic_get_qos(DDS_Topic *self,struct DDS_TopicQos *qos);
#endif

#ifndef RTI_CERT 
/*ce \dref_Topic_set_listener
 */
DDSCDllExport DDS_ReturnCode_t
DDS_Topic_set_listener(
        DDS_Topic *self,
        const struct DDS_TopicListener *listener,
        DDS_StatusMask mask);
#endif 

#ifndef RTI_CERT 
/*ce \dref_Topic_get_listener
 */
DDSCDllExport struct DDS_TopicListener
DDS_Topic_get_listener(DDS_Topic *self);
#endif 

/* ----------------------------------------------------------------- */

#ifdef __cplusplus
}                               /* extern "C" */
#endif


#endif /* dds_c_topic_h */
