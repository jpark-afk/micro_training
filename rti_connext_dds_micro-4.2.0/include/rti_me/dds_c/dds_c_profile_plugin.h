/*
 * FILE: dds_c_profile_plugin.h - DDS profile plugin interface
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

#ifndef dds_c_profile_plugin_h
#define dds_c_profile_plugin_h

#ifndef dds_c_dll_h
#include "dds_c/dds_c_dll.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/* ================================================================= */
/*                 QoS Profile interface                             */
/* ================================================================= */

typedef struct DDS_QosProfilePlugin
{
    struct RT_Component _parent;
} DDS_QosProfilePlugin_T;

typedef DDS_Boolean
(*DDS_QosProfileI_get_domain_participant_factory_qos_T)(
                              struct DDS_QosProfilePlugin *self,
                              struct DDS_DomainParticipantFactoryQos *qos,
                              const char *library_name,
                              const char *profile_name);

typedef DDS_Boolean
(*DDS_QosProfileI_get_domain_participant_qos_T)(
                                      struct DDS_QosProfilePlugin *self,
                                      struct DDS_DomainParticipantQos *qos,
                                      const char *library_name,
                                      const char *profile_name);

typedef DDS_Boolean
(*DDS_QosProfileI_get_publisher_qos_T)(struct DDS_QosProfilePlugin *self,
                                       struct DDS_PublisherQos *qos,
                                       const char *library_name,
                                       const char *profile_name);

typedef DDS_Boolean
(*DDS_QosProfileI_get_subscriber_qos_T)(struct DDS_QosProfilePlugin *self,
                                        struct DDS_SubscriberQos *qos,
                                        const char *library_name,
                                        const char *profile_name);

typedef DDS_Boolean
(*DDS_QosProfileI_get_datareader_qos_T)(struct DDS_QosProfilePlugin *self,
                                        struct DDS_DataReaderQos *qos,
                                        const char *library_name,
                                        const char *profile_name);

typedef DDS_Boolean
(*DDS_QosProfileI_get_datawriter_qos_T)(struct DDS_QosProfilePlugin *self,
                                        struct DDS_DataWriterQos *qos,
                                        const char *library_name,
                                        const char *profile_name);

typedef DDS_Boolean
(*DDS_QosProfileI_get_topic_qos_T)(struct DDS_QosProfilePlugin *self,
                                   struct DDS_TopicQos *qos,
                                   const char *library_name,
                                   const char *profile_name);

typedef DDS_Boolean
(*DDS_QosProfileI_load_profiles_T)(struct DDS_QosProfilePlugin *self);

typedef DDS_Boolean
(*DDS_QosProfileI_unload_profiles_T)(struct DDS_QosProfilePlugin *self);

typedef DDS_Boolean
(*DDS_QosProfileI_reload_profiles_T)(struct DDS_QosProfilePlugin *self);

typedef struct DDS_QosProfileI
{
    struct RT_ComponentI _parent;

    DDS_QosProfileI_get_domain_participant_factory_qos_T
        get_domain_participant_factory_qos;

    DDS_QosProfileI_get_domain_participant_qos_T
        get_domain_participant_qos;

    DDS_QosProfileI_get_publisher_qos_T
        get_publisher_qos;

    DDS_QosProfileI_get_subscriber_qos_T
        get_subscriber_qos;

    DDS_QosProfileI_get_datareader_qos_T
        get_datareader_qos;

    DDS_QosProfileI_get_datawriter_qos_T
        get_datawriter_qos;

    DDS_QosProfileI_get_topic_qos_T
        get_topic_qos;

    DDS_QosProfileI_load_profiles_T
        load_profile;

    DDS_QosProfileI_unload_profiles_T
        unload_profile;

    DDS_QosProfileI_reload_profiles_T
        reload_profiles;

} DDS_QosProfileI_T;

#define DDS_QosProfileI_INITIALIZER \
{\
    RT_COMPONENTI_BASE,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
    NULL,\
}

#define DDS_QosProfile_get_domain_participant_factory_qos(self_,qos_,l_,p_) \
((struct DDS_QosProfilePlugin*)(\
    (self_)->_parent._intf))->get_domain_participant_factory_qos(self_,qos_,l_,p_)

#define DDS_QosProfile_get_domain_participant_qos(self_,qos_,l_,p_) \
((struct DDS_QosProfilePlugin*)(\
    (self_)->_parent._intf))->get_domain_participant_qos(self_,qos_,l_,p_)

#define DDS_QosProfile_get_publisher_qos(self_,qos_,l_,p_) \
((struct DDS_QosProfilePlugin*)(\
    (self_)->_parent._intf))->get_publisher_qos(self_,qos_,l_,p_)

#define DDS_QosProfile_get_subscriber_qos(self_,qos_,l_,p_) \
((struct DDS_QosProfilePlugin*)(\
    (self_)->_parent._intf))->get_subscriber_qos(self_,qos_,l_,p_)

#define DDS_QosProfile_get_datareader_qos(self_,qos_,l_,p_) \
((struct DDS_QosProfilePlugin*)(\
    (self_)->_parent._intf))->get_datareader_qos(self_,qos_,l_,p_)

#define DDS_QosProfile_get_datawriter_qos(self_,qos_,l_,p_) \
((struct DDS_QosProfilePlugin*)(\
    (self_)->_parent._intf))->get_datawriter_qos(self_,qos_,l_,p_)

#define DDS_QosProfile_get_topic_qos(self_,qos_,l_,p_) \
((struct DDS_QosProfilePlugin*)(\
    (self_)->_parent._intf))->get_topic_qos(self_,qos_,l_,p_)

#define DDS_QosProfile_load_profiles(self_) \
((struct DDS_QosProfilePlugin*)(\
    (self_)->_parent._intf))->load_profiles(self_)

#define DDS_QosProfile_unload_profiles(self_) \
((struct DDS_QosProfilePlugin*)(\
    (self_)->_parent._intf))->unload_profiles(self_)

#define DDS_QosProfile_reload_profiles(self_) \
((struct DDS_QosProfilePlugin*)(\
    (self_)->_parent._intf))->reload_profiles(self_)

/* ================================================================= */
/*          Application Generation interface                         */
/* ================================================================= */
#if DDS_ENABLE_APPGEN

/*e \dref_PROFILE_DEFAULT_APPGEN_NAME
 */
DDSCDllVariable extern const char* const PROFILE_DEFAULT_APPGEN_NAME;

typedef struct DDS_AppGenPlugin
{
    struct RT_Component _parent;
} DDS_AppGenPlugin_T;

typedef DDS_DomainParticipant* (*DDS_AppGenI_create_participant_T)(
                        DDS_DomainParticipantFactory *self,
                        DDS_DomainId_t domain_id,
                        const struct DDS_DomainParticipantQos *qos,
                        const struct DDS_DomainParticipantListener *listener,
                        DDS_StatusMask mask);

typedef DDS_Publisher* (*DDS_AppGenI_create_publisher_T)(
                        DDS_DomainParticipant *self,
                        const struct DDS_PublisherQos *qos,
                        const struct DDS_PublisherListener *listener,
                        DDS_StatusMask mask);

typedef DDS_Subscriber* (*DDS_AppGenI_create_subscriber_T)(
                        DDS_DomainParticipant *self,
                        const struct DDS_SubscriberQos *qos,
                        const struct DDS_SubscriberListener *listener,
                        DDS_StatusMask mask);

typedef DDS_DataWriter* (*DDS_AppGenI_create_datawriter_T)(
                        DDS_Publisher *self,
                        DDS_Topic *topic,
                        const struct DDS_DataWriterQos *qos,
                        const struct DDS_DataWriterListener *listener,
                        DDS_StatusMask mask);

typedef DDS_DataReader* (*DDS_AppGenI_create_datareader_T)(
                        DDS_Subscriber *self,
                        DDS_TopicDescription *topic_desc,
                        const struct DDS_DataReaderQos *qos,
                        const struct DDS_DataReaderListener *listener,
                        DDS_StatusMask mask);

typedef DDS_Topic* (*DDS_AppGenI_create_topic_T)(
                        DDS_DomainParticipant *self,
                        const char *topic_name,
                        const char *type_name,
                        const struct DDS_TopicQos *qos,
                        const struct DDS_TopicListener *listener,
                        DDS_StatusMask mask);

#ifndef RTI_CERT
typedef DDS_ReturnCode_t (*DDS_AppGenI_delete_participant_T)(
                        DDS_DomainParticipantFactory *self,
                        DDS_DomainParticipant *a_participant);

typedef DDS_ReturnCode_t (*DDS_AppGenI_delete_contained_entities_T)
                       (DDS_DomainParticipant *self);
#endif

/*ci
 * \brief Interface to create and delete DDS entities.
 */
struct DDS_AppGen_EntityManager
{
    DDS_AppGenI_create_participant_T create_participant;

    DDS_AppGenI_create_publisher_T create_publisher;

    DDS_AppGenI_create_subscriber_T create_subscriber;

    DDS_AppGenI_create_datawriter_T create_datawriter;

    DDS_AppGenI_create_datareader_T create_datareader;

    DDS_AppGenI_create_topic_T create_topic;

#ifndef RTI_CERT
    DDS_AppGenI_delete_participant_T delete_participant;

    DDS_AppGenI_delete_contained_entities_T delete_contained_entities;
#endif
};

/*ci
 * \def DDS_AppGen_EntityManager_INITIALIZER
 * \brief Constant to initialize \ref AppGen_Property
 */
#ifndef RTI_CERT
#define DDS_AppGen_EntityManager_DELETE_INITIALIZER \
    , NULL                                          \
    , NULL
#else
#define DDS_AppGen_EntityManager_DELETE_INITIALIZER
#endif

/*ci
 * \def DDS_AppGen_EntityManager_INITIALIZER
 * \brief Constant to initialize \ref AppGen_Property
 */
#define DDS_AppGen_EntityManager_INITIALIZER    \
{                                               \
    NULL,                                       \
    NULL,                                       \
    NULL,                                       \
    NULL,                                       \
    NULL,                                       \
    NULL                                        \
    DDS_AppGen_EntityManager_DELETE_INITIALIZER \
}

/*ci
 * \brief Application generation properties
 */
struct DDS_AppGen_ComponentProperty
{
    /*ci
     * \brief Inherit from a component
     */
    struct RT_ComponentProperty _parent;

    struct DDS_AppGen_EntityManager entity_manager;
};

/*ci
 * \def DDS_AppGen_ComponentProperty_INITIALIZER
 * \brief Constant to initialize \ref DDS_AppGen_Properties
 */
#define DDS_AppGen_ComponentProperty_INITIALIZER \
{\
    RT_ComponentProperty_INITIALIZER,\
    DDS_AppGen_EntityManager_INITIALIZER \
}

typedef DDS_DomainParticipant*
(*DDS_AppGenI_create_participant_from_config_T)(
                                struct DDS_AppGenPlugin * self,
                                const char * configuration_name);

typedef struct DDS_AppGenI
{
    struct RT_ComponentI _parent;

    DDS_AppGenI_create_participant_from_config_T create_participant_from_config;
} DDS_AppGenI_T;

#define DDS_AppGenI_INITIALIZER \
{\
    RT_COMPONENTI_BASE,\
    NULL,\
}

/*ci
 * \brief Create a new instance of the application generation
 */
#define DDS_AppGenFactory_create_component(f_,p_,l_) \
    (struct DDS_AppGenPlugin*)((f_)->intf)->create_component(f_,p_,l_)

/*ci
 * \brief Delete an instance of the application generation
 */
#define DDS_AppGenFactory_delete_component(f_,c_) \
    ((f_)->intf)->delete_component(f_,(RT_Component_T*)c_)

/*ci
 * \brief Create a Domain Participant.
 */
#define DDS_AppGen_create_participant_from_config(self_,n_) \
    ((struct DDS_AppGenI*)(\
        (self_)->_parent._intf))->create_participant_from_config(self_,n_)

#endif /* DDS_ENABLE_APPGEN */

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif /* dds_c_profile_plugin_h */
