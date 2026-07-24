/*
 * FILE: appgen.h - Application Generator interface
 *
 * (c) Copyright, Real-Time Innovations, 2017-2018.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 */
/*ci
 * \file
 * \brief Application Generator Interface
 */
/*ci \defgroup AppGenModule Application Generation API
 *   \ingroup DDSAPPGENModule
 */
/*ci \addtogroup AppGenModule
 * @{
 */
#ifndef appgen_h
#define appgen_h

#ifndef rti_me_c_h
#include "rti_me_c.h"
#endif
#ifndef dds_c_type_h
#include "dds_c/dds_c_type.h"
#endif
#ifndef dds_c_publication_h
#include "dds_c/dds_c_publication.h"
#endif
#ifndef dds_c_subscription_h
#include "dds_c/dds_c_subscription.h"
#endif
#include "dds_c/dds_c_domain.h"
#ifndef RTI_CERT
#ifndef disc_dpde_discovery_plugin_h
#include "disc_dpde/disc_dpde_discovery_plugin.h"
#endif
#endif
#ifndef disc_dpse_dpsediscovery_h
#include "disc_dpse/disc_dpse_dpsediscovery.h"
#endif
#ifndef reda_sequence_h
#include "reda/reda_sequence.h"
#endif

#ifndef appgen_dll_h
#include "app_gen/app_gen_dll.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*ci
 * \brief Model for a Topic
 */
struct APPGENDllExport APPGEN_TopicModel
{
    /*ci
     * \brief Name of the topic
     */
    const char *name;

    /*ci
     * \brief The type name to which the new \ref DDS_Topic will be bound
     */
    const char *type_name;

    /*ci
     * \brief The topic QoS that shall be used to create the topic
     */
    const struct DDS_TopicQos topic_qos;
};

/*ci
 * \brief Model for a registered type
 */
struct APPGENDllExport APPGEN_TypeRegistrationModel
{
    /*ci
     * \brief The type name under with the data type is registered with the
     * participant. This type name is used when creating a new \ref DDS_Topic.
     * (See \ref APPGEN_TopicModel.)
     */
    const char *type_name;

    /*ci
     * \brief Function pointer which returns the type plugin to register
     * the data type with
     */
    DDS_TypePluginI_get_interface_T get_type_plugin;
};

/*ci
 * \brief Model for a DataWriter
 */
struct APPGENDllExport APPGEN_DataWriterModel
{
    /*ci
     * \brief Data writer name
     */
    const char *name;

    /*ci
     * \brief Data writer multiplicity. Cannot be 0. As many DDS_DataWriter as
     * indicated by this parameter will be created.
     */
    const DDS_UnsignedLong multiplicity;

    /*ci
     * \brief The \ref DDS_Topic that the DDS_DataWriter will be
     * associated with
     */
    const char *topic_name;

    /*ci
     * \brief The \ref DDS_DataWriterQos that the DDS_DataWriter will be
     * created with
     */
    const struct DDS_DataWriterQos writer_qos;
};

/*ci
 * \brief Model for a Publisher
 */
struct APPGENDllExport APPGEN_PublisherModel
{
    /*ci
     * \brief Publisher name
     */
    const char *name;

    /*ci
     * \brief Publisher multiplicity. Cannot be 0. As many DDS_Publisher as
     * indicated by this parameter will be created.
     */
    const DDS_UnsignedLong multiplicity;

    /*ci
     * \brief The \ref DDS_PublisherQos that the DDS_Publisher will be
     * created with
     */
    const struct DDS_PublisherQos publisher_qos;

    /*ci
     * \brief Number of data writers in the publisher
     */
    const DDS_UnsignedLong writer_count;

    /*ci
     * \brief Pointer to an array of datawriter models. This array shall have
     * as many models as indicated in field writer_count. Can be NULL only if
     * writer_count is 0.
     */
    const struct APPGEN_DataWriterModel *data_writers;
};

/*ci
 * \brief Model for a DataReader
 */
struct APPGENDllExport APPGEN_DataReaderModel
{
    /*ci
     * \brief Data reader name
     */
    const char *name;

    /*ci
     * \brief Data reader multiplicity. Cannot be 0. As many DDS_DataReader as
     * indicated by this parameter will be created.
     */
    const DDS_UnsignedLong multiplicity;

    /*ci
     * \brief The \ref DDS_TopicDescription that the DDS_DataReader will be
     * associated with.
     */
    const char *topic_name;

    /*ci
     * \brief The \ref DDS_DataReaderQos that the DDS_DataReader will be
     * created with
     */
    const struct DDS_DataReaderQos reader_qos;
};

/*ci
 * \brief Model for a Subscriber
 */
struct APPGENDllExport APPGEN_SubscriberModel
{
    /*ci
     * \brief Subscriber name
     */
    const char *name;

    /*ci
     * \brief Subscriber multiplicity. Cannot be 0. As many DDS_Subscriber as
     * indicated by this parameter will be created.
     */
    const DDS_UnsignedLong multiplicity;

    /*ci
     * \brief The \ref DDS_SubscriberQos that the DDS_Subscriber will be
     * created with
     */
    const struct DDS_SubscriberQos subscriber_qos;

    /*ci
     * \brief Number of data readers in the subscriber
     */
    const DDS_UnsignedLong reader_count;

    /*ci
     * \brief Pointer to an array of datareader models. This array shall have
     * as many models as indicated in field reader_count. Can be NULL only if
     * reader_count is 0.
     */
    const struct APPGEN_DataReaderModel *data_readers;
};

/*ci
 * \brief Model with information for remote Publications/DataWriters
 */
struct APPGENDllExport APPGEN_RemotePublicationModel
{
    /*ci
     * \brief Remote publication builtin data
     */
    const struct DDS_PublicationBuiltinTopicData publication_data;

    /*ci
     * \brief Function pointer which returns the type plugin which can return
     * the key kind use
     */
    DDS_TypePluginI_get_interface_T get_type_plugin;
};

/*ci
 * \brief Model with information for remote Subscriptions/DataReaders
 */
struct APPGENDllExport APPGEN_RemoteSubscriptionModel
{
    /*ci
     * \brief Remote subscription builtin data
     */
    const struct DDS_SubscriptionBuiltinTopicData subscription_data;

    /*ci
     * \brief Function pointer which returns the type plugin which can return
     * the key kind use
     */
    DDS_TypePluginI_get_interface_T get_type_plugin;
};

/*ci
 * \brief Model with information about remote Domain Participant. Used to
 * assert remote participants when DPSE discovery is used.
 */
struct APPGENDllExport APPGEN_RemoteParticipantModel
{
    /*ci
     * \brief Remote domain participant name
     */
    const char *name;

    /*ci
     * \brief Number of remote publishers
     */
    const DDS_UnsignedLong remote_publisher_count;

    /*ci
     * \brief Pointer to an array of remote publication models. This array
     * shall have as many models as indicated in field remote_publisher_count.
     * Can be NULL only if remote_publisher_count is 0.
     */
    const struct APPGEN_RemotePublicationModel *remote_publishers;

    /*ci
     * \brief Number of remote subscribers
     */
    const DDS_UnsignedLong remote_subscriber_count;

    /*ci
     * \brief Pointer to an array of remote subscription models. This array
     * shall have as many models as indicated in field remote_subscriber_count.
     * Can be NULL only if remote_subscriber_count is 0.
     */
    const struct APPGEN_RemoteSubscriptionModel *remote_subscribers;
};

/*ci
 * \brief Model for a ComponentFactoryUnregisterModel
 */
struct APPGENDllExport ComponentFactoryUnregisterModel
{
    /*ci
     * \brief Pointer to the component factory model.
     */
    const char *unregister_name;

    /*ci
     * \brief Pointer to the component factory properties used to unregister
     * the component.
     */
    const struct RT_ComponentFactoryProperty **unregister_property;

    /*ci
     * \brief Pointer to the component factory listener used to unregister
     * the component.
     */
    const struct RT_ComponentFactoryListener **unregister_listener;
};

/*ci
 * \brief Model for a ComponentFactoryRegisterModel
 */
struct APPGENDllExport ComponentFactoryRegisterModel
{
    /*ci
     * \brief Pointer to the component factory model.
     */
    const char *register_name;

    /*ci
     * \brief Pointer to the component factory interface used to register
     * the component.
     */
    const RT_ComponentFactoryI_get_interface_T register_intf;

    /*ci
     * \brief Pointer to the component factory properties used to register
     * the component.
     */
    const struct RT_ComponentFactoryProperty *register_property;

    /*ci
     * \brief Pointer to the component factory listener used to register
     * the component.
     */
    const struct RT_ComponentFactoryListener *register_listener;
};

/*ci
 * \brief Model for a DomainParticipantFactory
 */
struct APPGENDllExport APPGEN_DomainParticipantFactoryModel
{
    /*ci
     * Number of components that are going to be unregistered before register
     * the new ones; it is only used when we find an UDP component, otherwise
     * this will be empty. \rtime registers a UDP transport with default
     * properties, so in case it is needed to change those properties it needs
     * to be unregistered and registered again.
     */
    const DDS_UnsignedLong unregister_count;

    /*ci
     * \brief Pointer to an array of components factory to unregister. This array
     * shall have as many elements as indicated in field register_count. Can be
     * NULL only if register_count is 0.
     */
    const struct ComponentFactoryUnregisterModel *unregister_components;

    /*ci
     * Number of components that are going to be registered before the
     * application is created and unregistered when the application is deleted.
     */
    const DDS_UnsignedLong register_count;

    /*ci
     * \brief Pointer to an array of components factory to register. This array
     * shall have as many elements as indicated in field unregister_count. Can be
     * NULL only if register_count is 0.
     */
    const struct ComponentFactoryRegisterModel *register_components;

    /*ci
     * \brief \ref DDS_DomainParticipantFactoryQos to set before instantiating
     * the \ref DDS_DomainParticipantFactory.
     */
    const struct DDS_DomainParticipantFactoryQos factory_qos;
};

/*ci
 * \brief Model for a FlowController
 */
struct APPGENDllExport APPGEN_CustomFlowControllerModel
{
    /*ci
     * \brief Flow Controller name
     */
    const char *name;

    /*ci
     * \brief The \ref DDS_FlowControllerProperty_t that the DDS_FlowController
     * will be created with
     */
    const struct DDS_FlowControllerProperty_t flow_controller_property;
};

/*ci
 * \brief Function pointer type that returns a content
 * filter interface
 */
typedef const struct DDS_ContentFilterI *(*APPGEN_GetContentFilterIntfFn)(void);

/*ci
 * \brief Model for a content filter registration to
 * perform after participant creation
 */
struct APPGENDllExport APPGEN_ContentFilterRegistration
{
    /*ci \brief Filter class name (e.g. "DDSSQL") */
    const char *filter_class_name;

    /*ci \brief Function returning the filter interface */
    APPGEN_GetContentFilterIntfFn get_filter_intf;

    /*ci
     * \brief Filter-specific property (may be NULL)
     */
    const void *filter_property;
};

/*ci
 * \brief Model for a DomainParticipant
 */
struct APPGENDllExport APPGEN_DomainParticipantModel
{
    /*ci
     * \brief Domain participant name
     */
    const char *name;

    /*ci
     * \brief Domain participant factory. Contains information about
     * \ref DDS_DomainParticipantFactoryQos and factories to register.
     */
    const struct APPGEN_DomainParticipantFactoryModel domain_participant_factory;

    /*ci
     * \brief The \ref DDS_DomainParticipantQos that the DDS_DomainParticipant
     *  will be created with
     */
    const struct DDS_DomainParticipantQos participant_qos;

    /*ci \dref_RtpsWellKnownPorts_t_domain_id
     */
    const DDS_DomainId_t domain_id;

    /*ci Number of registered types
     */
    const DDS_UnsignedLong type_registration_count;

    /*ci
     * \brief Pointer to an array of type registration models. This array
     * shall have as many models as indicated in field type_registration_count.
     * Can be NULL only if type_registration_count is 0.
     */
    const struct APPGEN_TypeRegistrationModel *type_registrations;

    /*ci Number of topics
     */
    const DDS_UnsignedLong topic_count;

    /*ci
     * \brief Pointer to an array of topic models. This array
     * shall have as many models as indicated in field topic_count. Can be NULL
     * only if topic_count is 0.
     */
    const struct APPGEN_TopicModel *topics;

    /*ci Number of publishers
     */
    const DDS_UnsignedLong publisher_count;

    /*ci
     * \brief Pointer to an array of publisher models. This array
     * shall have as many models as indicated in field publisher_count. Can be
     * NULL only if publisher_count is 0.
     */
    const struct APPGEN_PublisherModel *publishers;

    /*ci Number of subscribers
     */
    const DDS_UnsignedLong subscriber_count;

    /*ci
     * \brief Pointer to an array of subscriber models. This array
     * shall have as many models as indicated in field subscriber_count. Can be
     * NULL only if subscriber_count is 0.
     */
    const struct APPGEN_SubscriberModel *subscribers;

    /*ci Number of remote participants
     */
    const DDS_UnsignedLong remote_participant_count;

    /*ci
     * \brief Pointer to an array of remote participants. This array
     * shall have as many models as indicated in field remote_participant_count.
     * Can be NULL only if remote_participant_count is 0.
     */
    const struct APPGEN_RemoteParticipantModel *remote_participants;

    /*ci Number of custom flow controllers
     */
    const DDS_UnsignedLong custom_flow_controller_count;

    /*ci
     * \brief Pointer to an array of custom flow controllers. This array shall
     * have as many models as indicated in field custom_flow_controller_count.
     * Can be NULL only if custom_flow_controller_count is 0.
     */
    const struct APPGEN_CustomFlowControllerModel *custom_flow_controllers;

    /*ci Number of content filter registrations
     */
    const DDS_UnsignedLong content_filter_registration_count;

    /*ci
     * \brief Pointer to an array of content filter
     * registrations. NULL when count is 0.
     */
    const struct APPGEN_ContentFilterRegistration *content_filter_registrations;
};

/*ci
 * \brief Model for a Library
 */
struct APPGENDllExport APPGEN_LibraryModel
{
    /*ci
     * \brief Library name
     */
    const char *library_name;

    /*ci
     * \brief Number of participants in the application
     */
    const DDS_UnsignedLong participant_count;

    /*ci
     * \brief Pointer to an array of domain participant models contained in the
     * application. This array shall have as many elements as indicated in
     * field participant_count.
     */
    const struct APPGEN_DomainParticipantModel *participants;
};

#define T struct APPGEN_LibraryModel
#define TSeq APPGEN_LibraryModelSeq
#include <reda/reda_sequence_decl.h>

/*ci
 * \brief Return a sequence with all Application Generator Libraries
 * avaialble
 *
 * \return A reference to \ref APPGEN_LibraryModelSeq.
 */
APPGENDllExport const struct APPGEN_LibraryModelSeq*
APPGEN_get_library_seq(void);

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif /* appgen_h */

/*ci @} */
