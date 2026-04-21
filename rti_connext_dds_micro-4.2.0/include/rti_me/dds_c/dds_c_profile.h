/*
 * FILE: dds_c_qos_profile.h - DDS Qos Profile API
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
/*ce
 * \file
 * \brief DDS Qos Profile APIs
 */
#ifndef dds_c_qos_profile_h
#define dds_c_qos_profile_h

#include "dds_c_config.h"

#include "dds_c/dds_c_dll.h"

#ifndef dds_c_profile_plugin_h
#include "dds_c/dds_c_profile_plugin.h"
#endif

#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * DDS APIs
 */

DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipantFactory_set_default_participant_qos_with_profile(
                DDS_DomainParticipantFactory *self,
                const char *library_name,
                const char *profile_name);

DDSCDllExport DDS_DomainParticipant*
DDS_DomainParticipantFactory_create_participant_with_profile(
                        DDS_DomainParticipantFactory *self,
                        DDS_DomainId_t  domainId,
                        const char *library_name,
                        const char *profile_name,
                        const struct DDS_DomainParticipantListener *listener,
                        DDS_StatusMask  mask);

#if DDS_ENABLE_APPGEN

/*ce \dref_DomainParticipantFactory_create_participant_from_config
 */
DDSCDllExport DDS_DomainParticipant*
DDS_DomainParticipantFactory_create_participant_from_config(
                                DDS_DomainParticipantFactory *self,
                                const char *configuration_name);

/*ci
 * \brief Creates a DomainParticipant given its configuration name.
 *
 * \param[in] self DomainParticipantFactory used to delete the application
 * \param[in] configuration_name The configuration name is the fully qualified
 * name of the participant, consisting of the name of the application library
 * plus the name of participant configuration. For example the name
 * "MyApplicationLibrary::PublicationParticipant" can be used to create the
 * participant from the configuration with library name "MyApplicationLibrary"
 * and participant name "PublicationParticipant"
 * \param[in] entity_manager Pointer to structure with function pointers which will
 * be used to delete DDS entities.
 *
 * \return Pointer to DDS_DomainParticipant if participant was created correctly
 * or NULL in case of error.
 */
DDSCDllExport DDS_DomainParticipant*
DDS_DomainParticipantFactory_create_participant_from_config_w_entity_manager(
                              DDS_DomainParticipantFactory *self,
                              const char *configuration_name,
                              const struct DDS_AppGen_EntityManager *entity_manager);

#endif /* DDS_ENABLE_APPGEN */

DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipantFactory_load_profiles(DDS_DomainParticipantFactory *self);

DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipantFactory_reload_profiles(DDS_DomainParticipantFactory *self);

DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipantFactory_unload_profiles(DDS_DomainParticipantFactory *self);

DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipantFactory_set_default_library(
                                            DDS_DomainParticipantFactory *self,
                                            const char *library_name);

DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipantFactory_set_default_profile(
                                            DDS_DomainParticipantFactory *self,
                                            const char *library_name,
                                            const char *profile_name);

DDSCDllExport const char*
DDS_DomainParticipantFactory_get_default_library(
                                DDS_DomainParticipantFactory *self);

DDSCDllExport const char*
DDS_DomainParticipantFactory_get_default_profile(
                                DDS_DomainParticipantFactory *self);

DDSCDllExport const char*
DDS_DomainParticipantFactory_get_default_profile_library(
                                DDS_DomainParticipantFactory *self);

DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipantFactory_get_participant_factory_qos_from_profile(
                                DDS_DomainParticipantFactory *self,
                                struct DDS_DomainParticipantFactoryQos *qos,
                                const char *library_name,
                                const char *profile_name);

DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipantFactory_get_participant_qos_from_profile(
                                DDS_DomainParticipantFactory *self,
                                struct DDS_DomainParticipantQos *qos,
                                const char *library_name,
                                const char *profile_name);

DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipantFactory_get_publisher_qos_from_profile(
                                DDS_DomainParticipantFactory *self,
                                struct DDS_PublisherQos *qos,
                                const char *library_name,
                                const char *profile_name);

DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipantFactory_get_subscriber_qos_from_profile(
                                DDS_DomainParticipantFactory *self,
                                struct DDS_SubscriberQos *qos,
                                const char *library_name,
                                const char *profile_name);

DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipantFactory_get_datareader_qos_from_profile(
                                DDS_DomainParticipantFactory *self,
                                struct DDS_DataReaderQos *qos,
                                const char *library_name,
                                const char *profile_name);

DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipantFactory_get_datareader_qos_from_profile_w_topic_name(
                                DDS_DomainParticipantFactory *self,
                                struct DDS_DataReaderQos *qos,
                                const char *library_name,
                                const char *profile_name,
                                const char *topic_name);

DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipantFactory_get_datawriter_qos_from_profile(
                                DDS_DomainParticipantFactory *self,
                                struct DDS_DataWriterQos *qos,
                                const char *library_name,
                                const char *profile_name);

DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipantFactory_get_datawriter_qos_from_profile_w_topic_name(
                                DDS_DomainParticipantFactory *self,
                                struct DDS_DataWriterQos *qos,
                                const char *library_name,
                                const char *profile_name,
                                const char *topic_name);

DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipantFactory_get_topic_qos_from_profile(
                                DDS_DomainParticipantFactory *self,
                                struct DDS_TopicQos *qos,
                                const char *library_name,
                                const char *profile_name);

DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipantFactory_get_topic_qos_from_profile_w_topic_name(
                                DDS_DomainParticipantFactory *self,
                                struct DDS_TopicQos *qos,
                                const char *library_name,
                                const char *profile_name,
                                const char *topic_name);

DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipantFactory_get_qos_profile_libraries(
                                DDS_DomainParticipantFactory *self,
                                struct DDS_StringSeq *library_names);

DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipantFactory_get_qos_profiles(
                                DDS_DomainParticipantFactory *self,
                                struct DDS_StringSeq *profile_names,
                                const char *library_name);

DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_set_default_topic_qos_with_profile(
                                DDS_DomainParticipant *self,
                                const char *library_name,
                                const char *profile_name);

DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_set_default_publisher_qos_with_profile(
                                DDS_DomainParticipant *self,
                                const char *library_name,
                                const char *profile_name);

DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_set_default_datawriter_qos_with_profile(
                                DDS_DomainParticipant *self,
                                const char *library_name,
                                const char *profile_name);

DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_set_default_subscriber_qos_with_profile(
                                DDS_DomainParticipant *self,
                                const char *library_name,
                                const char *profile_name);

DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_set_default_datareader_qos_with_profile(
                                DDS_DomainParticipant *self,
                                const char *library_name,
                                const char *profile_name);

DDSCDllExport const char*
DDS_DomainParticipant_get_default_library(DDS_DomainParticipant *self);

DDSCDllExport const char*
DDS_DomainParticipant_get_default_profile(DDS_DomainParticipant *self);

DDSCDllExport const char*
DDS_DomainParticipant_get_default_profile_library(DDS_DomainParticipant *self);

DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_set_default_library(DDS_DomainParticipant *self,
                                          const char *library_name);

DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_set_default_profile(DDS_DomainParticipant *self,
                                          const char *library_name,
                                          const char *profile_name);

DDSCDllExport DDS_Publisher*
DDS_DomainParticipant_create_publisher_with_profile(
                                DDS_DomainParticipant *self,
                                const char *library_name,
                                const char *profile_name,
                                const struct DDS_PublisherListener *listener,
                                DDS_StatusMask  mask);

DDSCDllExport DDS_Subscriber*
DDS_DomainParticipant_create_subscriber_with_profile(
                                DDS_DomainParticipant *self,
                                const char *library_name,
                                const char *profile_name,
                                const struct DDS_SubscriberListener *listener,
                                DDS_StatusMask  mask);

DDSCDllExport DDS_DataWriter*
DDS_DomainParticipant_create_datawriter_with_profile(
                                DDS_DomainParticipant *self,
                                DDS_Topic *topic,
                                const char *library_name,
                                const char *profile_name,
                                const struct DDS_DataWriterListener *listener,
                                DDS_StatusMask  mask);

DDSCDllExport DDS_DataReader*
DDS_DomainParticipant_create_datareader_with_profile(
                                DDS_DomainParticipant *self,
                                DDS_TopicDescription *topic,
                                const char *library_name,
                                const char *profile_name,
                                const struct DDS_DataReaderListener *listener,
                                DDS_StatusMask  mask);

DDSCDllExport DDS_Topic*
DDS_DomainParticipant_create_topic_with_profile(
                                DDS_DomainParticipant *self,
                                const char *topic_name,
                                const char *type_name,
                                const char *library_name,
                                const char *profile_name,
                                const struct DDS_TopicListener *listener,
                                DDS_StatusMask  mask);

DDSCDllExport DDS_ReturnCode_t
DDS_DomainParticipant_set_qos_with_profile(DDS_DomainParticipant *self,
                                           const char *library_name,
                                           const char *profile_name);

DDSCDllExport DDS_ReturnCode_t
DDS_Topic_set_qos_with_profile (DDS_Topic *self,
                                const char *library_name,
                                const char *profile_name);

DDSCDllExport DDS_ReturnCode_t
DDS_Publisher_set_default_datawriter_qos_with_profile(
                                DDS_Publisher *self,
                                const char *library_name,
                                const char *profile_name);

DDSCDllExport DDS_ReturnCode_t
DDS_Publisher_set_default_profile(DDS_Publisher *self,
                                  const char *library_name,
                                  const char *profile_name);

DDSCDllExport DDS_ReturnCode_t
DDS_Publisher_set_default_library(DDS_Publisher *self,
                                  const char *library_name);


DDSCDllExport DDS_DataWriter*
DDS_Publisher_create_datawriter_with_profile(DDS_Publisher *self,
                        DDS_Topic *topic,
                        const char *library_name,
                        const char *profile_name,
                        const struct DDS_DataWriterListener *listener,
                        DDS_StatusMask  mask);

DDSCDllExport DDS_ReturnCode_t
DDS_Publisher_set_qos_with_profile(DDS_Publisher *self,
                                   const char *library_name,
                                   const char *profile_name);

DDSCDllExport const char*
DDS_Publisher_get_default_library(DDS_Publisher *self);

DDSCDllExport const char*
DDS_Publisher_get_default_profile(DDS_Publisher *self);

DDSCDllExport const char*
DDS_Publisher_get_default_profile_library(DDS_Publisher *self);

DDSCDllExport DDS_ReturnCode_t
DDS_DataWriter_set_qos_with_profile(DDS_DataWriter *self,
                                    const char *library_name,
                                    const char *profile_name);

DDSCDllExport DDS_ReturnCode_t
DDS_Subscriber_set_default_datareader_qos_with_profile(
                                    DDS_Subscriber *self,
                                    const char *library_name,
                                    const char *profile_name);

DDSCDllExport DDS_DataReader*
DDS_Subscriber_create_datareader_with_profile(
                        DDS_Subscriber *self,
                        DDS_TopicDescription *topic,
                        const char *library_name,
                        const char *profile_name,
                        const struct DDS_DataReaderListener *listener,
                        DDS_StatusMask  mask);

DDSCDllExport DDS_ReturnCode_t
DDS_Subscriber_set_qos_with_profile(DDS_Subscriber *self,
                                    const char *library_name,
                                    const char *profile_name);

DDSCDllExport DDS_ReturnCode_t
DDS_Subscriber_set_default_profile(DDS_Subscriber *self,
                                    const char *library_name,
                                    const char *profile_name);

DDSCDllExport const char*
DDS_Subscriber_get_default_profile(DDS_Subscriber * self);

DDSCDllExport const char*
DDS_Subscriber_get_default_profile_library(DDS_Subscriber *self);

DDSCDllExport DDS_ReturnCode_t
DDS_Subscriber_set_default_library(DDS_Subscriber *self,
                                   const char *library_name);

DDSCDllExport const char*
DDS_Subscriber_get_default_library(DDS_Subscriber *self);

DDSCDllExport DDS_ReturnCode_t
DDS_DataReader_set_qos_with_profile(DDS_DataReader *self,
                                    const char *library_name,
                                    const char *profile_name);


#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif
