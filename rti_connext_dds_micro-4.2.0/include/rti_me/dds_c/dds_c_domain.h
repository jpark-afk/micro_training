/*
 * FILE: dds_c_domain.h - DDS domain module definitions
 *
 * Copyright (c) 2012-2025 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 07apr2016,tk MICRO-1541 Fixed assignment operator issues for C++
 * 29jun2015,tk MICRO-1351/PR#15141 Removed prototypes for non-Cert function
 * 19may2015,as MICRO-1193 Refactoring of Sequence API levels
 * 02apr2015,as MICRO-372/PR#1528 Set space for discovery component to be
 *                                RT_MAX_FACTORY_NAME+1
 * 20sep2014,as Make support functions for QosPolicy types private and reduce public
 *              support functions for Qos types to initialize, finalize, copy only.
 * 29apr2014,as MICRO-372 (Verocel PR#1528) Documented maximum length
 *              of DDS_DiscoveryComponent::name
 * 19jul2013,as Added support for C++
 * 29jun2012,tk Written
 */
/*ce
 * \file
 * \brief DDS Domain Module definitions
 */
/*e
  @addtogroup DDSDomainModule Domain Module
  @brief Defines the \dds domain package
 */
#ifndef dds_c_domain_h
#define dds_c_domain_h

#ifndef NULL
#define NULL 0
#endif
#ifndef dds_c_dll_h
#include "dds_c/dds_c_dll.h"
#endif
#ifndef reda_string_h
#include "reda/reda_string.h"
#endif
#ifndef rt_rt_h
#include "rt/rt_rt.h"
#endif
#ifndef dds_c_infrastructure_h
#include "dds_c/dds_c_infrastructure.h"
#endif
#include "dds_c_sequence.h"
#ifndef dds_c_topic_h
#include "dds_c/dds_c_topic.h"
#endif
#ifndef dds_c_publication_h
#include "dds_c/dds_c_publication.h"
#endif
#ifndef dds_c_subscription_h
#include "dds_c/dds_c_subscription.h"
#endif
#ifndef dds_c_discovery_h
#include "dds_c/dds_c_discovery.h"
#endif
#ifndef rtps_trust_plugin_h
#include "rtps/rtps_trust_plugin.h"
#endif
#ifndef dds_c_flowcontroller_h
#include "dds_c/dds_c_flowcontroller.h"
#endif
#ifndef dds_c_user_data_qos_h
#include "dds_c/dds_c_user_data_qos.h"
#endif

#if DDS_FILTERING_ENABLED
#ifndef dds_c_filter_plugin_h
#include "dds_c/dds_c_filter_plugin.h"
#endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/* ================================================================= */
/*                       Basic Types                                 */
/* ================================================================= */

/* ----------------------------------------------------------------- */
/*                DISCOVERY_X (eXtension QoS)                        */
/* ----------------------------------------------------------------- */
/*e \dref_DiscoveryQosGroupDocs
 */

/*e \dref_DiscoveryComponentProperty
 */
struct NDDS_Discovery_Property
{
    struct RT_ComponentProperty _parent;
};

/*i \dref_DiscoveryComponentProperty_INITIALIZER
 */
#define NDDS_Discovery_Property_INITIALIZER \
{\
  RT_ComponentProperty_INITIALIZER\
}

/*e \dref_DiscoveryComponent
 */
struct DDS_DiscoveryComponent
{
    /*e \dref_DiscoveryComponent_name
     *
     * This attribute specifies the name of the plug-in to be
     * used for DDS Discovery. The maximum length of the name
     * is RT_MAX_FACTORY_NAME excluding the NUL termination.
     */
    RT_ComponentFactoryId_T name;

    struct NDDS_Discovery_Property property;
};

/*e \dref_DiscoveryComponent_INITIALIZER
 */
#define DDS_DiscoveryComponent_INITIALIZER \
{\
  RT_ComponentFactoryId_INITIALIZER,\
  NDDS_Discovery_Property_INITIALIZER\
}

#define T struct DDS_DiscoveryComponent
#define TSeq DDS_DiscoveryComponentSeq
#include <reda/reda_sequence_decl.h>

#ifdef DOXYGEN_DOCUMENTATION_ONLY
/*i \dref_DiscoveryComponentSeq
 */
struct DDS_DiscoveryComponentSeq {};
#endif

/*e \dref_DiscoveryQosPolicy
 */
struct DDSCPPDllExport DDS_DiscoveryQosPolicy
{
    /*e \dref_DiscoveryQosPolicy_initial_peers
     */
    struct DDS_StringSeq initial_peers;

    /*e \dref_DiscoveryQosPolicy_enabled_transports
     */
    struct DDS_StringSeq enabled_transports;

    /*e \dref_DiscoveryQosPolicy_discovery
     */
    struct DDS_DiscoveryComponent discovery;

    /*e \dref_DiscoveryQosPolicy_accept_unknown_peers
     */
    DDS_Boolean accept_unknown_peers;

    /*e \dref_DiscoveryQosPolicy_enable_participant_discovery_by_name
     */
    DDS_Boolean enable_participant_discovery_by_name;

    /*e \dref_DiscoveryQosPolicy_enable_endpoint_discovery_queue
     */
    DDS_Boolean enable_endpoint_discovery_queue;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_DiscoveryQosPolicy)
};

/*i \dref_DiscoveryQosPolicy_DEFAULT
 */
#define DDS_DISCOVERY_QOS_POLICY_DEFAULT \
{ \
    DDS_SEQUENCE_INITIALIZER,\
    DDS_SEQUENCE_INITIALIZER,\
    DDS_DiscoveryComponent_INITIALIZER, \
    DDS_BOOLEAN_TRUE, \
    DDS_BOOLEAN_FALSE, \
    DDS_BOOLEAN_FALSE \
}

DDSC_QOS_POLICY_METHODS_DECL(DDS_DiscoveryQosPolicy)

/*e \dref_UserTrafficQosGroupDocs
 */

/*e \dref_UserTrafficQosPolicy
 *  \ingroup UserTrafficQosModule
 */
struct DDSCPPDllExport DDS_UserTrafficQosPolicy
{
    /*e \dref_UserTrafficQosPolicy_enabled_transports
     */
    struct DDS_StringSeq enabled_transports;

    DDSC_CPP_QOS_POLICY_METHODS(DDS_UserTrafficQosPolicy)
};

/*i \dref_UserTrafficQosPolicy_DEFAULT
 */
#define DDS_USERTRAFFIC_QOS_POLICY_DEFAULT \
{ \
    DDS_SEQUENCE_INITIALIZER\
}

DDSC_QOS_POLICY_METHODS_DECL(DDS_UserTrafficQosPolicy)

/*i \dref_TrustQosPolicyGroupDocs
 */

/*i \dref_TrustQosPolicy
 */
struct DDSCPPDllExport DDS_TrustQosPolicy
{
    RT_ComponentFactoryId_T suite;
    DDSC_CPP_QOS_POLICY_METHODS(DDS_TrustQosPolicy)
};

#define DDS_TRUST_QOS_POLICY_DEFAULT \
{ \
        RT_ComponentFactoryId_INITIALIZER \
}

DDSC_CPP_QOS_POLICY_METHODS(DDS_TrustQosPolicy)

/* ================================================================= */
/*                       Listeners                                   */
/* ================================================================= */

struct DDS_DomainParticipantQos;
struct DDS_PublisherQos;
struct DDS_SubscriberQos;
struct DDS_DataReaderQos;
struct DDS_DataWriterQos;
struct DDS_TopicQos;
struct DDS_DomainParticipantListener;
struct DDS_PublisherListener;
struct DDS_SubscriberListener;
struct DDS_DataReaderListener;
struct DDS_DataWriterListener;
struct DDS_TopicListener;

/* ----------------------------------------------------------------- */

/*ce \dref_DomainParticipantListener
 */
struct DDS_DomainParticipantListener
{
    /*ce \dref_DomainParticipantListener_as_topiclistener
     */
    struct DDS_TopicListener as_topiclistener;

    /*ce \dref_DomainParticipantListener_as_publisherlistener
     */
    struct DDS_PublisherListener as_publisherlistener;

    /*ce \dref_DomainParticipantListener_as_subscriberlistener
     */
    struct DDS_SubscriberListener as_subscriberlistener;
};

/*ce \dref_DomainParticipantListener_INITIALIZER
 */
#define DDS_DomainParticipantListener_INITIALIZER   \
{\
    DDS_TopicListener_INITIALIZER,    \
    DDS_PublisherListener_INITIALIZER, \
    DDS_SubscriberListener_INITIALIZER \
}


#ifdef __cplusplus
}                               /* extern "C" */
#endif

/* ================================================================= */
/*                          QoS                                      */
/* ================================================================= */

/* ====================== */
/*  DomainParticipantQos  */
/* ====================== */

/*e \dref_DomainParticipantQos
 */
struct DDSCPPDllExport DDS_DomainParticipantQos
{
    /*e \dref_DomainParticipantQos_entity_factory
     */
    struct DDS_EntityFactoryQosPolicy entity_factory;

    /*e \dref_DomainParticipantQos_discovery
     */
    struct DDS_DiscoveryQosPolicy discovery;

    /*e \dref_DomainParticipantQos_resource_limits
     */
    struct DDS_DomainParticipantResourceLimitsQosPolicy resource_limits;

    /*e \dref_DomainParticipantQos_participant_name
     */
    struct DDS_EntityNameQosPolicy participant_name;

    /*e \dref_DomainParticipantQos_wire_protocol
     */
    struct DDS_WireProtocolQosPolicy protocol;

    /*e \dref_DomainParticipantQos_transports
     */
    struct DDS_TransportQosPolicy transports;

    /*e \dref_DomainParticipantQos_user_traffic
     */
    struct DDS_UserTrafficQosPolicy user_traffic;

    /*e \dref_DomainParticipantQos_trust
     */
    struct DDS_TrustQosPolicy trust;

    /*e \dref_DomainParticipantQos_property
     */
    struct DDS_PropertyQosPolicy property;

    /*e \dref_DomainParticipantQos_user_data
     */
    struct DDS_UserDataQosPolicy user_data;

#if DDS_FILTERING_ENABLED
    /*e \dref_DomainParticipantQos_filter
     */
    struct DDS_FilterQosPolicy filter;
#endif /* DDS_FILTERING_ENABLED */

    DDSC_CPP_QOS_METHODS(DDS_DomainParticipantQos)
};

#ifdef __cplusplus
extern "C"
{
#endif

#if DDS_FILTERING_ENABLED
#define DDS_FILTER_QOS_POLICY_PARTICIPANT_DEFAULT \
    ,DDS_FILTER_QOS_POLICY_DEFAULT
#else
#define DDS_FILTER_QOS_POLICY_PARTICIPANT_DEFAULT
#endif

/*ce \dref_DomainParticipantQos_INITIALIZER
 */
#define DDS_DomainParticipantQos_INITIALIZER                \
{                                                           \
  DDS_ENTITY_FACTORY_QOS_POLICY_DEFAULT,                    \
  DDS_DISCOVERY_QOS_POLICY_DEFAULT,                         \
  DDS_DOMAIN_PARTICIPANT_RESOURCE_LIMITS_QOS_POLICY_DEFAULT,\
  DDS_ENTITY_NAME_QOS_POLICY_DEFAULT,                       \
  DDS_WIRE_PROTOCOL_QOS_POLICY_DEFAULT,                     \
  DDS_TRANSPORT_QOS_POLICY_DEFAULT,                         \
  DDS_USERTRAFFIC_QOS_POLICY_DEFAULT,                       \
  DDS_TRUST_QOS_POLICY_DEFAULT,                             \
  DDS_PROPERTY_QOS_POLICY_DEFAULT,                          \
  DDS_USER_DATA_QOS_POLICY_DEFAULT                          \
  DDS_FILTER_QOS_POLICY_PARTICIPANT_DEFAULT                 \
}


/*ce \dref_DomainParticipantQos_initialize
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipantQos_initialize(struct DDS_DomainParticipantQos *self);

/*ce \dref_DomainParticipantQos_copy
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipantQos_copy(struct DDS_DomainParticipantQos *self,
                              const struct DDS_DomainParticipantQos *source);

/*ci
 * \brief Compare two DDS_DomainParticipantQos policies for equality
 *
 * \param[in] left  The left side of the comparison
 * \param[in] right The right side of the comparison
 *
 * \return DDS_BOOLEAN_TRUE if the structures are equal,
 *         DDS_BOOLEAN_FALSE otherwise
 */
MUST_CHECK_RETURN DDSCDllExport DDS_Boolean
DDS_DomainParticipantQos_is_equal(const struct DDS_DomainParticipantQos *left,
                                  const struct DDS_DomainParticipantQos *right);

#ifndef RTI_CERT
/*ce \dref_DomainParticipantQos_finalize
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipantQos_finalize(struct DDS_DomainParticipantQos *self);
#endif /* !RTI_CERT */

#ifdef __cplusplus
}                               /* extern "C" */
#endif

/* ============================== */
/*  DomainParticipantFactoryQos  */
/* ============================== */

/*e \dref_DomainParticipantFactoryQos
 */
struct DDSCPPDllExport DDS_DomainParticipantFactoryQos
{
    /*e \dref_DomainParticipantFactoryQos_entity_factory
     */
    struct DDS_EntityFactoryQosPolicy entity_factory;

    /*e \dref_DomainParticipantFactoryQos_resource_limits
     */
    struct DDS_SystemResourceLimitsQosPolicy resource_limits;

    DDSC_CPP_QOS_METHODS(DDS_DomainParticipantFactoryQos)
};

#ifdef __cplusplus
extern "C"
{
#endif

/*ci
 * \brief Initialize a DDS_DomainParticipantFactoryQos policy
 *
 * \param[in] self DDS_DomainParticipantFactoryQos policy to initialize
 *
 * \return DDS_RETCODE_OK on success, one of the standard error codes on failure
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipantFactoryQos_initialize(
            struct DDS_DomainParticipantFactoryQos* self);

#ifndef RTI_CERT
/*ci
 * \brief Initialize a DDS_DomainParticipantFactoryQos policy
 *
 * \param[in] self DDS_DomainParticipantFactoryQos policy to finalize
 *
 * \return DDS_RETCODE_OK on success, one of the standard error codes on failure
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipantFactoryQos_finalize(
            struct DDS_DomainParticipantFactoryQos* self);
#endif /*RTI_CERT*/

/*ci
 * \brief Copy a DDS_DomainParticipantFactoryQos policy
 *
 * \details
 * Copy the contents of the source structure to the destination structure. The
 * destination structure must be preallocated and initialized.
 *
 * \param[inout] out The destination DDS_DomainParticipantFactoryQos policy
 * \param[in]    in  The source DDS_DomainParticipantFactoryQos policy
 *
 * \return DDS_RETCODE_OK on success, one of the standard error codes on failure
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipantFactoryQos_copy(
            struct DDS_DomainParticipantFactoryQos* self,
            const struct DDS_DomainParticipantFactoryQos* from);

/*ci
 * \brief Test if two DDS_DomainParticipantFactoryQos are equal
 *
 * \param[in] left  The left side of the comparison
 * \param[in] right The right side of the comparison
 *
 * \return DDS_BOOLEAN_TRUE if left is equal to right,
 *         DDS_BOOLEAN_FALSE if left is not equal to right
 */
MUST_CHECK_RETURN DDSCDllExport DDS_Boolean
DDS_DomainParticipantFactoryQos_is_equal(
                        const struct DDS_DomainParticipantFactoryQos *left,
                        const struct DDS_DomainParticipantFactoryQos *right);

/*ce \dref_DomainParticipantFactoryQos_INITIALIZER
 */
#define DDS_DomainParticipantFactoryQos_INITIALIZER     \
{\
  DDS_ENTITY_FACTORY_QOS_POLICY_DEFAULT,                \
  DDS_SYSTEM_RESOURCE_LIMITS_QOS_POLICY_DEFAULT         \
}


/* ================================================================= */
/*                        DomainParticipant                          */
/* ================================================================= */

/*e \dref_TOPIC_QOS_DEFAULT
 */
extern DDSCDllVariable const struct DDS_TopicQos
    DDS_TOPIC_QOS_DEFAULT;

/*e \dref_PUBLISHER_QOS_DEFAULT
 */
extern DDSCDllVariable const struct DDS_PublisherQos
    DDS_PUBLISHER_QOS_DEFAULT;

/*e \dref_SUBSCRIBER_QOS_DEFAULT
 */
extern DDSCDllVariable const struct DDS_SubscriberQos
    DDS_SUBSCRIBER_QOS_DEFAULT;

/* ----------------------------------------------------------------- */
/*      LANGUAGE-DEPENDENDENT WRAPPER-ENTITIES FINALIZERS            */
/* ----------------------------------------------------------------- */
/*i \dref_DomainParticipant_finalize_publisher_fn
 */
typedef void
(*DDS_DomainParticipant_finalize_publisher_fn)(DDS_Publisher *publisher);

/*i \dref_DomainParticipant_finalize_subscriber_fn
 */
typedef void
(*DDS_DomainParticipant_finalize_subscriber_fn)(DDS_Subscriber *subscriber);

/*i \dref_DomainParticipant_finalize_topic_fn
 */
typedef void
(*DDS_DomainParticipant_finalize_topic_fn)(DDS_Topic *topic);

/*i \dref_DomainParticipant_EntityFinalizer
 */
typedef struct DDS_DomainParticipant_EntityFinalizer
{
    DDS_DomainParticipant_finalize_publisher_fn finalize_publisher;
    DDS_DomainParticipant_finalize_subscriber_fn finalize_subscriber;
    DDS_DomainParticipant_finalize_topic_fn finalize_topic;
} DDS_DomainParticipant_EntityFinalizer_t;

/*i \dref_DomainParticipant_EntityFinalizer_INITIALIZER
 */
#define DDS_DomainParticipant_EntityFinalizer_INITIALIZER \
            { NULL, NULL, NULL}

/* ----------------------------------------------------------------- */

#define DDS_DomainParticipant_as_entity(self) \
            ((DDS_Entity*) self)

#ifdef DOXYGEN_DOCUMENTATION_ONLY

/*ce \dref_DomainParticipant_as_entity
 */
DDS_Entity*
DDS_DomainParticipant_as_entity(DDS_DomainParticipant *self);

#endif /*DOXYGEN_DOCUMENTATION_ONLY */

/* ================================== */
/*  Get and Set Default QoS routines  */
/* ================================== */

#if INCLUDE_API_QOS
/*ce \dref_DomainParticipant_get_default_publisher_qos
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_get_default_publisher_qos(
        DDS_DomainParticipant *self,
        struct DDS_PublisherQos *qos);

/*ce \dref_DomainParticipant_set_default_publisher_qos
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_set_default_publisher_qos(
                    DDS_DomainParticipant *self,
                    const struct DDS_PublisherQos *qos);

/*ce \dref_DomainParticipant_get_default_subscriber_qos
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_get_default_subscriber_qos(
        DDS_DomainParticipant *self,
        struct DDS_SubscriberQos *qos);

/*ce \dref_DomainParticipant_set_default_subscriber_qos
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_set_default_subscriber_qos(
        DDS_DomainParticipant *self,
        const struct DDS_SubscriberQos *qos);

/*ce \dref_DomainParticipant_get_default_topic_qos
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_get_default_topic_qos(
        DDS_DomainParticipant *self,
        struct DDS_TopicQos *qos);

/*ce \dref_DomainParticipant_set_default_topic_qos
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_set_default_topic_qos(
        DDS_DomainParticipant *self,
        const struct DDS_TopicQos *qos);
#endif

/* ============================ */
/*  Create and Delete routines  */
/* ============================ */

/*ce \dref_DomainParticipant_create_publisher
 */
DDSCDllExport DDS_Publisher*
DDS_DomainParticipant_create_publisher(
        DDS_DomainParticipant * self,
        const struct DDS_PublisherQos *qos,
        const struct DDS_PublisherListener *listener,
        DDS_StatusMask mask);

#ifndef RTI_CERT
/*ce \dref_DomainParticipant_delete_publisher
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_delete_publisher(
        DDS_DomainParticipant *self,
        DDS_Publisher *p);
#endif

/*ce \dref_DomainParticipant_create_subscriber
 */
DDSCDllExport DDS_Subscriber*
DDS_DomainParticipant_create_subscriber(
        DDS_DomainParticipant *self,
        const struct DDS_SubscriberQos *qos,
        const struct DDS_SubscriberListener *listener,
        DDS_StatusMask mask);

#ifndef RTI_CERT
/*ce \dref_DomainParticipant_delete_subscriber
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_delete_subscriber(
        DDS_DomainParticipant *self,
        DDS_Subscriber * s);
#endif

/*ce \dref_DomainParticipant_create_topic
 */
DDSCDllExport DDS_Topic*
DDS_DomainParticipant_create_topic(
        DDS_DomainParticipant * self,
        const char *topic_name,
        const char *type_name,
        const struct DDS_TopicQos *qos,
        const struct DDS_TopicListener *listener,
        DDS_StatusMask mask);

#ifndef RTI_CERT
/*ci \dref_DomainParticipant_delete_topic_w_finalizer
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_delete_topic_w_finalizer(
        DDS_DomainParticipant *self,
        DDS_Topic *topic,
        DDS_DomainParticipant_finalize_topic_fn finalizer);

/*ce \dref_DomainParticipant_delete_topic
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_delete_topic(
        DDS_DomainParticipant * self,
        DDS_Topic * topic);
#endif

/*ce \dref_DomainParticipant_find_topic
 */
#if INCLUDE_API_LOOKUP
DDSCDllExport DDS_Topic*
DDS_DomainParticipant_find_topic(DDS_DomainParticipant *self,
                                 const char *topic_name,
                                 const struct DDS_Duration_t *timeout);
#endif

#if DDS_FILTERING_ENABLED
/*ci
 * \brief Register a content filter with the DomainParticipant to allow it
 *        to be used by its child DataReaders and DataWriters.
 *
 * \details This function requires that content filtering is enabled on the
 *          DomainParticipant. If it is not, this function will return
 *          DDS_RETCODE_PRECONDITION_NOT_MET.
 *
 * \param[in] self The DomainParticipant to register the content filter with
 * \param[in] filter_name The name of the content filter class
 * \param[in] filter_intf The content filter interface
 * \param[in] filter_property The content filter property
 *
 * \return DDS_RETCODE_OK on success, one of the standard error codes on failure
 */
MUST_CHECK_RETURN DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_register_contentfilter(
        DDS_DomainParticipant *self,
        const char *filter_name,
        const struct DDS_ContentFilterI *filter_intf,
        const void *filter_property);

/*ci
 * \brief Unregister a content filter from the DomainParticipant
 *
 * \details This function requires that content filtering is enabled on the
 *          DomainParticipant. If it is not, this function will return
 *          DDS_RETCODE_PRECONDITION_NOT_MET.
 *
 * \param[in] self The DomainParticipant to unregister the content filter from
 * \param[in] filter_name The name of the content filter class
 *
 * \return DDS_RETCODE_OK on success, one of the standard error codes on failure
 */
MUST_CHECK_RETURN DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_unregister_contentfilter(
        DDS_DomainParticipant *self,
        const char *filter_name);
#endif /* DDS_FILTERING_ENABLED */

/* ================= */
/*  Lookup routines  */
/* ================= */

/*ce \dref_DomainParticipant_lookup_topicdescription
 */
DDSCDllExport DDS_TopicDescription*
DDS_DomainParticipant_lookup_topicdescription(
        DDS_DomainParticipant *self,
        const char *topic_name);

#if DDS_ENABLE_APPGEN
/*ce \dref_DomainParticipant_lookup_publisher_by_name
 */
DDSCDllExport DDS_Publisher*
DDS_DomainParticipant_lookup_publisher_by_name(
            DDS_DomainParticipant *self,
            const char *publisher_name);

/*ce \dref_DomainParticipant_lookup_subscriber_by_name
 */
DDSCDllExport DDS_Subscriber*
DDS_DomainParticipant_lookup_subscriber_by_name(
            DDS_DomainParticipant *self,
            const char *subscriber_name);

/*ce \dref_DomainParticipant_lookup_datawriter_by_name
 */
DDSCDllExport DDS_DataWriter*
DDS_DomainParticipant_lookup_datawriter_by_name(
            DDS_DomainParticipant *self,
            const char *datawriter_full_name);

/*ce \dref_DomainParticipant_lookup_datareader_by_name
 */
DDSCDllExport DDS_DataReader*
DDS_DomainParticipant_lookup_datareader_by_name(
            DDS_DomainParticipant *self,
            const char *datareader_full_name);
#endif /* DDS_ENABLE_APPGEN */

/* ================ */
/*  Misc. routines  */
/* ================ */

/*ce \dref_DomainParticipant_add_peer
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_add_peer(DDS_DomainParticipant *self,const char* peer);

/*ce \dref_DomainParticipant_get_domain_id
 */
DDSCDllExport DDS_DomainId_t
DDS_DomainParticipant_get_domain_id(DDS_DomainParticipant *self);

#if DDS_LIVELINESS_CHANNEL_ENABLED
/*ce \dref_DomainParticipant_assert_liveliness
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_assert_liveliness(DDS_DomainParticipant *self);
#endif /* DDS_LIVELINESS_CHANNEL_ENABLED */

#ifndef RTI_CERT
/*ce \dref_DomainParticipant_delete_contained_entities
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_delete_contained_entities(DDS_DomainParticipant *self);
#endif

#ifndef RTI_CERT

/*ci \dref_DomainParticipant_delete_contained_entities_w_finalizerI
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_delete_contained_entities_w_finalizerI(
        DDS_DomainParticipant *self,
        struct DDS_DomainParticipant_EntityFinalizer *finalizer);
#endif

/*ce \dref_DomainParticipant_get_current_time
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_get_current_time(
        DDS_DomainParticipant * self,
        struct DDS_Time_t *current_time);

struct DDS_ParticipantBuiltinTopicData;

#if INCLUDE_API_LOOKUP
/*ce \dref_DomainParticipant_get_discovered_participants
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_get_discovered_participants(
        DDS_DomainParticipant *self,
        struct DDS_InstanceHandleSeq *participant_handles);

/*ce \dref_DomainParticipant_get_discovered_participant_data
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_get_discovered_participant_data(
        DDS_DomainParticipant *self,
        struct DDS_ParticipantBuiltinTopicData *participant_data,
        const DDS_InstanceHandle_t *participant_handle);
#endif /* INCLUDE_API_LOOKUP */

/*ci \brief Determine if a remote participant has compatible checksum
 */
DDSCDllExport DDS_Boolean
DDS_DomainParticipant_is_checksum_compatible(DDS_DomainParticipant *self,
                const struct DDS_ParticipantBuiltinTopicData *remote);

/*ci
 * \brief Get pointer to the DDS participant properties
 *
 * \details
 * The DDS domain participant may send additional information as properties
 * in the participant announcement. Typically this is used by discovery plugins
 * to serialize the information.
 *
 * \param[in] self Participant to get properties for
 *
 * \return A sequence with 0 or more properties on success, NULL if no
 *        sequence exists.
 */
DDSCDllExport struct DDS_PropertySeq*
DDS_DomainParticipant_get_dds_properties(DDS_DomainParticipant *self);


/*ci
 * \brief Query the domain-participant if the specified locator is supported
 *
 * \param[in] self Participant to query
 *
 * \return DDS_BOOLEAN_TRUE if the locator is supported, DDS_BOOLEEAN_FALSE
 *         otherwise.
 */
DDSCDllExport DDS_Boolean
DDS_DomainParticipant_locator_is_supported(DDS_DomainParticipant *self,
                                           struct DDS_Locator *locator);

/*ci
 * \details
 *
 * The user data allows for storing Domain Participant specific
 * information. For instance, for the Zero Copy Shared Data feature, the
 * netio_sdm module will place a bitmap responsible for maintaining which
 * shared memory keys are used by DataWriters
 */
typedef enum
{
    DDS_PARTICIPANT_USER_DATA_ZERO_COPY_SHARED_DATA,

    DDS_PARTICIPANT_USER_DATA_MAX_ELEMENT

} DDS_DomainParticipantUserDataId;

/*ci \dref_DomainParticipant_set_user_data
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_set_user_data(
        DDS_DomainParticipant *self,
        DDS_DomainParticipantUserDataId user_data_id,
        void *data);

/*ci \dref_DomainParticipant_get_user_data
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_get_user_data(
        DDS_DomainParticipant *self,
        DDS_DomainParticipantUserDataId user_data_id,
        void **user_data_out);

/*ci
 * \dref_DomainParticipant_get_announced_datawriters
 */
DDSCDllExport RTI_BOOL
DDS_DomainParticipant_get_announced_datawriters(
        DDS_DomainParticipant *self,
        DDS_Long *announced_out,
        DDS_Long *unannounced_out);

/*ci
 * \dref_DomainParticipant_get_announced_datareaders
 */
DDSCDllExport RTI_BOOL
DDS_DomainParticipant_get_announced_datareaders(
        DDS_DomainParticipant *self,
        DDS_Long *announced_out,
        DDS_Long *unannounced_out);

/* ================================================================= */
/*   From DDS_Entity                                                 */
/* ================================================================= */

/* ========================== */
/*  Get and Set QoS routines  */
/* ========================== */

#if INCLUDE_API_QOS

/*ce \dref_DomainParticipant_set_qos
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_set_qos(
        DDS_DomainParticipant *self,
        const struct DDS_DomainParticipantQos *qos);

/*ce \dref_DomainParticipant_get_qos
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_get_qos(
        DDS_DomainParticipant *self,
        struct DDS_DomainParticipantQos *qos);
#endif

/*ci
 * \brief Get a reference to a participant's Qos policy
 *
 * \details
 * There are special cases when a participant's Qos policy may be accessed,
 * such as when a discovery plugin needs to adjust resource limits. This
 * function returns a pointer to the Qos policy. The pointer is only valid
 * when called from a discovery plugin related function and should not be
 * accessed outside of such a calling context.
 *
 * \param[in] self The participant to get the Qos policy reference for
 *
 * \return A pointer to the participant Qos policy on success, NULL on failure
 */
DDSCDllExport struct DDS_DomainParticipantQos*
DDS_DomainParticipant_get_qos_ref(DDS_DomainParticipant * self);

DDSCDllExport DDS_UserDataManager_T*
DDS_DomainParticipant_get_user_data_manager(DDS_DomainParticipant *self);

DDSCDllExport struct NDDS_ParticipantConfig*
DDS_DomainParticipant_get_cfg_ref(DDS_DomainParticipant * self);

DDSCDllExport DDS_StringManager_T*
DDS_DomainParticipant_get_partition_string_manager(DDS_DomainParticipant *self);

DDSCDllExport RTI_BOOL
DDS_DomainParticipant_is_mtu_greater_than_builtindata(
        DDS_DomainParticipant *const participant,
        DDS_DataWriter *dw,
        struct DDS_ParticipantBuiltinTopicData *local_participant_data);

DDSCDllExport DDS_Boolean
DDS_DomainParticipant_is_discovery_by_name_enabled(
                                const DDS_DomainParticipant *const participant);

/* =============================== */
/*  Get and Set Listener routines  */
/* ============================== */

#ifndef RTI_CERT
/*ce \dref_DomainParticipant_set_listener
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_set_listener(
        DDS_DomainParticipant * self,
        const struct DDS_DomainParticipantListener *listener,
        DDS_StatusMask mask);
#endif

#ifndef RTI_CERT
/*ce \dref_DomainParticipant_get_listener
 */
DDSCDllExport struct DDS_DomainParticipantListener
DDS_DomainParticipant_get_listener(DDS_DomainParticipant *self);
#endif

/*ci
 * \brief Get pointer to a DomainParticipant timer
 *
 * \details
 * Some plugins may need a timer and it is legal to share the timer with a
 * participant as long as a plugin/component belongs to the participant.
 * Note that this is not checked and it is up to the caller to use this
 * function correctly.
 *
 * \param[in] self The participant to get the timer object for
 *
 * \return The participants timer on success, NULL on failure
 *
 */
DDSCDllExport OSAPI_Timer_T
DDS_DomainParticipant_get_timer(DDS_DomainParticipant *const self);

/*e \dref_DomainParticipant_register_type
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_register_type(DDS_DomainParticipant *self,
                                    const char *type_name,
                                    struct DDS_TypePluginI *plugin);

#ifndef RTI_CERT
/*e \dref_DomainParticipant_unregister_type
 */
DDSCDllExport struct DDS_TypePluginI*
DDS_DomainParticipant_unregister_type(DDS_DomainParticipant *self,
                                      const char *type_name);
#endif

DDSCDllExport DDS_Publisher*
DDS_DomainParticipant_get_builtin_publisher(DDS_DomainParticipant *participant);

DDSCDllExport DDS_Subscriber*
DDS_DomainParticipant_get_builtin_subscriber(DDS_DomainParticipant *participant);

DDSCDllExport struct DDS_ParticipantBuiltinTopicData*
DDS_DomainParticipant_get_builtin_data(DDS_DomainParticipant *participant);

/* ================================================================= */
/*                       Factory                                     */
/* ================================================================= */

/*e \dref_PARTICIPANT_QOS_DEFAULT
 */
extern DDSCDllVariable const struct DDS_DomainParticipantFactoryQos
DDS_PARTICIPANT_FACTORY_QOS_DEFAULT;

/*e \dref_PARTICIPANT_QOS_DEFAULT
 */
extern DDSCDllVariable const struct DDS_DomainParticipantQos
DDS_PARTICIPANT_QOS_DEFAULT;

/*ce \dref_DomainParticipantFactory
 */
typedef struct DDS_DomainParticipantFactoryImpl DDS_DomainParticipantFactory;

/*ce \dref_DomainParticipantFactory_get_instance
 */
DDSCDllExport DDS_DomainParticipantFactory*
DDS_DomainParticipantFactory_get_instance(void);

/*ce \dref_DomainParticipantFactory_TheParticipantFactory
  */
#define DDS_TheParticipantFactory DDS_DomainParticipantFactory_get_instance()

#ifndef RTI_CERT
/*ce \dref_DomainParticipantFactory_finalize_instance
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipantFactory_finalize_instance(void);
#endif

/* ================================= */
/*  Set and Get Default QoS routines */
/* ================================= */

#if INCLUDE_API_QOS
/*ce \dref_DomainParticipantFactory_set_default_participant_qos
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipantFactory_set_default_participant_qos(
        DDS_DomainParticipantFactory * self,
         const struct DDS_DomainParticipantQos *qos);

/*ce \dref_DomainParticipantFactory_get_default_participant_qos
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipantFactory_get_default_participant_qos(
        DDS_DomainParticipantFactory * self,
        struct DDS_DomainParticipantQos *qos);
#endif

/* =========================== */
/*  Create and Delete routines */
/* =========================== */

/*ce \dref_DomainParticipantFactory_create_participant
 */
DDSCDllExport DDS_DomainParticipant*
DDS_DomainParticipantFactory_create_participant(
        DDS_DomainParticipantFactory * self,
        DDS_DomainId_t domainId,
        const struct DDS_DomainParticipantQos *qos,
        const struct DDS_DomainParticipantListener *listener,
        DDS_StatusMask mask);

#ifndef RTI_CERT
/*ce \dref_DomainParticipantFactory_delete_participant
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipantFactory_delete_participant(
        DDS_DomainParticipantFactory * self,
         DDS_DomainParticipant * a_participant);
#endif

/* =============== */
/*  Lookup routine */
/* =============== */

/*ce \dref_DomainParticipantFactory_lookup_participant
 */
DDSCDllExport DDS_DomainParticipant*
DDS_DomainParticipantFactory_lookup_participant(
        DDS_DomainParticipantFactory *self,
        DDS_DomainId_t domainId);

#if DDS_ENABLE_APPGEN
/*ce \dref_DomainParticipantFactory_lookup_participant_by_name
 */
DDSCDllExport DDS_DomainParticipant*
DDS_DomainParticipantFactory_lookup_participant_by_name(
        DDS_DomainParticipantFactory *self,
        const char *participant_name);
#endif /* DDS_ENABLE_APPGEN */

#if DDS_XTYPES_IS_ENABLED
/*ci \dref_DomainParticipantFactory_assert_program
 */
DDSCDllExport const struct RTIXCdrInterpreterPrograms*
DDS_DomainParticipantFactory_assert_program(
                                    DDS_DomainParticipantFactory *self,
                                    const struct DDS_TypePluginI *intf,
                                    DDS_TypeProgramNode *program,
                                    const DDS_TypeCode *const tc);
#endif

DDSCDllExport DDS_Entity*
DDS_DomainParticipant_lookup_entity(DDS_DomainParticipant *self,
                                    const struct NETIO_Guid *const guid);


/* ========================= */
/*  Get and Set Qos routines */
/* ========================= */

/*ce \dref_DomainParticipantFactory_get_qos
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipantFactory_get_qos(
        DDS_DomainParticipantFactory *self,
        struct DDS_DomainParticipantFactoryQos *qos);

/*ce \dref_DomainParticipantFactory_set_qos
 */
DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipantFactory_set_qos(
        DDS_DomainParticipantFactory *self,
        const struct DDS_DomainParticipantFactoryQos *qos);

/*ce \dref_DomainParticipantFactory_get_registry
 */
DDSCDllExport RT_Registry_T*
DDS_DomainParticipantFactory_get_registry(DDS_DomainParticipantFactory *self);

#ifdef __cplusplus
}                               /* extern "C" */
#endif


#endif /* dds_c_domain_h */
