/*
 * FILE: dds_c_discovery_plugin.h - DDS discovery plugin interface
 *
 * (c) Copyright, Real-Time Innovations, 2012-2026.
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
 * 23jan2012,tk Updated
 * 30apr2008,tk Written
 */
/*ce
 * \file
 * \brief DDS discovery plugin interface
 */
/*ci \defgroup DDSDiscoveryModule DDS Discovery Module
 *   \ingroup DDSCModule
 */
/*e \addtogroup DDSDiscoveryModule
 * @{
 */
#ifndef dds_c_discovery_plugin_h
#define dds_c_discovery_plugin_h

#ifndef dds_c_dll_h
#include "dds_c/dds_c_dll.h"
#endif
#ifndef dds_c_common_h
#include "dds_c/dds_c_common.h"
#endif
#ifndef dds_c_domain_h
#include "dds_c/dds_c_domain.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*ci
 * \brief The standardized type name for the Participant discovery type
 */
extern DDSCDllVariable const char *const DDS_PARTICIPANT_BUILTIN_TOPIC_TYPE_NAME;

/*ci
 * \brief The standardized topic name for Participant discovery
 */
extern DDSCDllVariable const char *const DDS_PARTICIPANT_BUILTIN_TOPIC_NAME;

/*ci
 * \brief The standardized type name for the Publication discovery type
 */
extern DDSCDllVariable const char *const DDS_PUBLICATION_BUILTIN_TOPIC_TYPE_NAME;

/*ci
 * \brief The standardized topic name for Publication discovery
 */
extern DDSCDllVariable const char *const DDS_PUBLICATION_BUILTIN_TOPIC_NAME;

/*ci
 * \brief The standardized type name for the Subscription discovery type
 */
extern DDSCDllVariable const char *const DDS_SUBSCRIPTION_BUILTIN_TOPIC_TYPE_NAME;

/*ci
 * \brief The standardized topic name for Subscription discovery
 */
extern DDSCDllVariable const char *const DDS_SUBSCRIPTION_BUILTIN_TOPIC_NAME;

#ifdef DOXYGEN_DOCUMENTATION_ONLY
/*e \dref_PARTICIPANT_TOPIC_NAME */
const char* const DDS_PARTICIPANT_TOPIC_NAME;
/*e \dref_PUBLICATION_TOPIC_NAME */
const char* const DDS_PUBLICATION_TOPIC_NAME;
/*e \dref_SUBSCRIPTION_TOPIC_NAME */
const char* const DDS_SUBSCRIPTION_TOPIC_NAME;
#else
#define DDS_PARTICIPANT_TOPIC_NAME    DDS_PARTICIPANT_BUILTIN_TOPIC_NAME
#define DDS_PUBLICATION_TOPIC_NAME    DDS_PUBLICATION_BUILTIN_TOPIC_NAME
#define DDS_SUBSCRIPTION_TOPIC_NAME   DDS_SUBSCRIPTION_BUILTIN_TOPIC_NAME
#endif

/*ci
 * \brief Base-class for discovery plugins
 */
struct NDDS_Discovery_Plugin
{
    /*ci
     * \brief Inherit from RT base-class
     */
    struct RT_Component _parent;
};

/*ci
 * \brief Base-class for discovery plugins
 */
struct NDDS_Discovery_Listener
{
    /*ci
     * \brief Inherit from RT base-class listener
     */
    struct RT_ComponentListener _parent;
};

/*ci
 * \def NDDS_Discovery_Listener_INITIALIZE
 * \brief Constant to initialize a NDDS_Discovery_Listener
 */
#define NDDS_Discovery_Listener_INITIALIZE {\
  RT_ComponentListener_INITIALIZER\
}

/*ci
 * \def NDDSDiscovery_narrow
 * \brief Narrow a RT factory to a DDS discovery factory
 *
 * \param[in] factory_ Generic factory to specialize
 *
 * \return NDDSDiscovery factory
 */
#define NDDSDiscovery_narrow(factory_) \
        (((struct NDDS_Discovery_Plugin*)(factory_))->intf)

/* ------------------------------ Life-cycle -------------------------------- */
/*ci
 * \brief Definition of \ref RT_ComponentFactory delete_component method
 *
 * \param[in] self The discovery instance to delete
 *
 * \sa \ref RT_ComponentFactoryI
 */
typedef void
(*NDDSDiscovery_deleteFunc)(struct NDDS_Discovery_Plugin *self);

/*ci
 * \brief Definition of \ref RT_ComponentFactory create_component method
 *
 * \param[in] property  The component property
 * \param[in] listener  The component listener
 *
 * \return A reference to a discovery instance on success, NULL otherwise
 *
 * \sa \ref RT_ComponentFactoryI
 */
FUNCTION_MUST_TYPEDEF(
struct NDDS_Discovery_Plugin*
(*NDDS_Discovery_Plugin_createFunc) (
                        const struct NDDS_Discovery_Property *const property,
                        const struct NDDS_Discovery_Listener *const listener)
)

/*ci
 * \brief Definition of the \ref NDDS_DiscoveryI on_before_local_participant_created method
 *
 * \param[in]    discovery_plugin       The DPSE plugin.
 * \param[in]    participant            The participant creating the plugin.
 * \param[inout] participant_qos        The participant's qos policy.
 * \param[inout] local_builtin_data_out The participant's announcement data.
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure.
 *
 * \sa \ref NDDS_DiscoveryI,
 *     \ref NDDS_Discovery_Plugin_on_before_local_participant_created
 */
FUNCTION_MUST_TYPEDEF(
DDS_Boolean
(*NDDS_Discovery_Plugin_BeforeLocalParticipantCreatedCallback)(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        struct DDS_DomainParticipantQos *participant_qos,
        struct DDS_ParticipantBuiltinTopicData *local_builtin_data_out)
)

/*ci
 * \brief Definition of the \ref NDDS_DiscoveryI on_after_local_participant_created method
 *
 * \param[in] discovery_plugin       The DPSE plugin
 * \param[in] participant            The participant creating the plugin
 * \param[in] local_participant_data The participant's announcement data
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 *
 * \sa \ref NDDS_DiscoveryI,
 *     \ref NDDS_Discovery_Plugin_on_after_local_participant_created
 */
FUNCTION_MUST_TYPEDEF(
DDS_Boolean
(*NDDS_Discovery_Plugin_AfterLocalParticipantCreatedCallback)(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        struct DDS_ParticipantBuiltinTopicData *const local_participant_data)
)

/*ci
 * \brief Definition of the \ref NDDS_DiscoveryI on_after_local_participant_enabled method
 *
 * \param[in] discovery_plugin        The DPSE plugin
 * \param[in] participant             The participant creating the plugin
 * \param[in] local_participant_data  The participant's announcement data
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 *
 * \sa \ref NDDS_DiscoveryI,
 *     \ref NDDS_Discovery_Plugin_on_after_local_participant_enabled
 */
FUNCTION_MUST_TYPEDEF(
DDS_Boolean
(*NDDS_Discovery_Plugin_AfterLocalParticipantEnabledCallback)(
        struct NDDS_Discovery_Plugin* const discovery_plugin,
        DDS_DomainParticipant *const participant,
        struct DDS_ParticipantBuiltinTopicData *const local_participant_data)
)

/*ci
 * \brief Definition of the \ref NDDS_DiscoveryI on_dispose_local_participant method
 *
 * \param[in] discovery_plugin      The DPSE plugin
 * \param[in] participant           The participant being deleted
 * \param[in] local_participant_key The participant's key
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 *
 * \sa \ref NDDS_DiscoveryI,
 *     \ref NDDS_Discovery_Plugin_on_before_local_participant_deleted
 */
FUNCTION_MUST_TYPEDEF(
DDS_Boolean
(*NDDS_Discovery_Plugin_DisposeLocalParticipantCallback)(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        struct DDS_BuiltinTopicKey_t *const local_participant_key)
)

/*ci
 * \brief Definition of the \ref NDDS_DiscoveryI on_before_local_participant_deleted method
 *
 * \param[in] discovery_plugin      The DPSE plugin
 * \param[in] participant           The participant being deleted
 * \param[in] local_participant_key The participant's key
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 *
 * \sa \ref NDDS_DiscoveryI,
 *     \ref NDDS_Discovery_Plugin_on_before_local_participant_deleted
 */
FUNCTION_MUST_TYPEDEF(
DDS_Boolean
(*NDDS_Discovery_Plugin_BeforeLocalParticipantDeletedCallback)(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        struct DDS_BuiltinTopicKey_t *const local_participant_key)
)

/*ci
 * \brief Definition of the \ref NDDS_DiscoveryI on_before_local_datawriter_created method
 *
 * \param[in] discovery_plugin The discovery plugin
 * \param[in] participant      The participant
 * \param[in] dw_key           The key of the datawriter to be created
 * \param[in] reservation      DDS_BOOLEAN_TRUE if this is a reservation,
 *                             DDS_BOOLEAN_FALSE if this is releasing a
 *                             reservation.
 *
 * \return DDS_RETCODE_OK success, one of the standard return codes on failure
 */
FUNCTION_MUST_TYPEDEF(
DDS_ReturnCode_t
(*NDDS_Discovery_Plugin_BeforeLocalDataWriterCreatedCallback)(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        struct DDS_BuiltinTopicKey_t *const dw_key,
        DDS_Boolean reservation)
)

/*ci
 * \brief Definition of the \ref NDDS_DiscoveryI on_after_local_datawriter_enabled method
 *
 * \param[in] discovery_plugin The DPSE plugin
 * \param[in] participant      The participant
 * \param[in] data_writer      The datawriter being enabled
 * \param[in] qos              The datawriter's Qos
 *
 * \return DDS_BOOLEAN_TRUE
 *
 * \sa \ref NDDS_DiscoveryI,
 *     \ref NDDS_Discovery_Plugin_on_after_local_datawriter_enabled
 */
FUNCTION_MUST_TYPEDEF(
DDS_Boolean
(*NDDS_Discovery_Plugin_AfterLocalDataWriterEnabledCallback)(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        DDS_DataWriter *const data_writer,
        const struct DDS_DataWriterQos *const qos)
)

/*ci
 * \brief Definition of the \ref NDDS_DiscoveryI after_local_datawriter_deleted method
 *
 * \param[in] discovery_plugin     The DPSE plugin
 * \param[in] participant          The participant
 * \param[in] local_datawriter_key The key of the deleted datawriter
 *
 * \return DDS_BOOLEAN_TRUE
 *
 * \sa \ref NDDS_DiscoveryI,
 *     \ref NDDS_Discovery_Plugin_on_after_local_datawriter_deleted
 */
FUNCTION_MUST_TYPEDEF(
DDS_Boolean
(*NDDS_Discovery_Plugin_AfterLocalDataWriterDeletedCallback)(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        DDS_DataWriter *datawriter
        /*const struct DDS_BuiltinTopicKey_t *const local_datawriter_key*/)
)

/*ci
 * \brief Definition of the \ref NDDS_DiscoveryI on_before_local_datareader_created method
 *
 * \param[in] discovery_plugin The discovery plugin
 * \param[in] participant      The participant
 * \param[in] dr_key           The key of the datareader to be created
 * \param[in] reservation      DDS_BOOLEAN_TRUE if this is a reservation,
 *                             DDS_BOOLEAN_FALSE if this is releasing a
 *                             reservation.
 *
 * \return DDS_RETCODE_OK success, one of the standard return codes on failure
 */
FUNCTION_MUST_TYPEDEF(
DDS_ReturnCode_t
(*NDDS_Discovery_Plugin_BeforeLocalDataReaderCreatedCallback)(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        struct DDS_BuiltinTopicKey_t *const dr_key,
        DDS_Boolean reservation)
)

/*ci
 * \brief Definition of the \ref NDDS_DiscoveryI on_after_local_datareader_enabled method
 *
 * \param[in] discovery_plugin The DPSE plugin
 * \param[in] participant      The participant
 * \param[in] data_reader      The datareader being enabled
 * \param[in] qos              The datareader's Qos
 *
 * \return DDS_BOOLEAN_TRUE
 *
 * \sa \ref NDDS_DiscoveryI,
 *     \ref NDDS_Discovery_Plugin_on_after_local_datareader_enabled
 */
FUNCTION_MUST_TYPEDEF(
DDS_Boolean
(*NDDS_Discovery_Plugin_AfterLocalDataReaderEnabledCallback)(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        DDS_DataReader *const data_reader,
        const struct DDS_DataReaderQos *const qos)
)

/*ci
 * \brief Definition of the \ref NDDS_DiscoveryI after_local_datareader_deleted method
 *
 * \param[in] discovery_plugin     The DPSE plugin
 * \param[in] participant          The participant
 * \param[in] local_datareader_key The key of the deleted datareader
 *
 * \return DDS_BOOLEAN_TRUE
 *
 * \sa \ref NDDS_DiscoveryI,
 *     \ref NDDS_Discovery_Plugin_on_after_local_datareader_deleted
 */
FUNCTION_MUST_TYPEDEF(
DDS_Boolean
(*NDDS_Discovery_Plugin_AfterLocalDataReaderDeletedCallback)(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        DDS_DataReader *datareader
        /* struct DDS_BuiltinTopicKey_t *const local_datareader_key */)
)

/*ci
 * \brief Definition of the \ref NDDS_DiscoveryI on_before_remote_participant_deleted method
 *
 * \param[in] discovery_plugin        The DPSE plugin
 * \param[in] participant             The participant
 * \param[in] remote_participant_data The remote participant data
 *
 * \return DDS_BOOLEAN_TRUE on success or DDS_BOOLEAN_FALSE on failure
 *
 * \sa \ref NDDS_DiscoveryI,
 *      \ref NDDS_Discovery_Plugin_on_before_remote_participant_deleted
 */
FUNCTION_MUST_TYPEDEF(
DDS_Boolean
(*NDDS_Discovery_Plugin_BeforeRemoteParticipantDeletedCallback)(
        struct NDDS_Discovery_Plugin * const discovery_plugin,
        DDS_DomainParticipant * const participant,
        struct DDS_ParticipantBuiltinTopicData *const remote_participant_data,
        const DDS_RemoteParticipantStatusMask remote_participant_status)
)

/*ci
 * \brief Definition of the \ref NDDS_DiscoveryI add_peer method
 *
 * \param[in] discovery_plugin The DPSE plugin
 * \param[in] participant      The participant
 * \param[in] peer             A peer address string
 *
 * \return DDS_BOOLEAN_TRUE on success or DDS_BOOLEAN_FALSE on failure
 *
 * \sa \ref NDDS_DiscoveryI,
 *      \ref NDDS_Discovery_Plugin_add_peer
 */
FUNCTION_MUST_TYPEDEF(
DDS_Boolean
(*NDDS_Discovery_Plugin_AddPeerFunction)(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,const char *peer)
)

/*ci
 * \brief Definition of the \ref NDDS_DiscoveryI assert_remote_participant method
 *
 * \param[in] discovery_plugin  The DPSE plugin
 * \param[in] participant       The participant
 * \param[in] participant_name  The name of the remote participant
 *
 * \sa \ref NDDS_DiscoveryI,
 *      \ref NDDS_Discovery_Plugin_assert_remote_participant
 */
typedef void
(*NDDS_Discovery_Plugin_AssertRemoteParticipant)(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        const struct DDS_ParticipantBuiltinTopicData *const data);

/*ci
 * \brief Definition of the \ref NDDS_DiscoveryI on_publication_data_return_loan method
 *
 * \param[in] discovery_plugin  The discovery plugin
 * \param[in] participant       The participant
 * \param[in] data              Data to return
 */
typedef void
(*NDDS_Discovery_Plugin_PublicationBuiltinTopicDataReturnLoanCallback)(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        struct DDS_PublicationBuiltinTopicData *data,
        struct DDS_SampleInfo *info);

/*ci
 * \brief Definition of the \ref NDDS_DiscoveryI on_subscription_data_return_loan method
 *
 * \param[in] discovery_plugin  The discovery plugin
 * \param[in] participant       The participant
 * \param[in] data              Data to return
 */
typedef void
(*NDDS_Discovery_Plugin_SubscriptionBuiltinTopicDataReturnLoanCallback)(
        struct NDDS_Discovery_Plugin *const discovery_plugin,
        DDS_DomainParticipant *const participant,
        struct DDS_SubscriptionBuiltinTopicData *data,
        struct DDS_SampleInfo *info);

/*ci
 * \brief Definition of the NDDS_Discovery interface
 */
struct NDDS_DiscoveryI
{
    /*ci
     * \brief Inherit from base-class
     */
    struct RT_ComponentI _parent;

    /*ci
     * \brief Method to signal a remote participant's lease expired
     */
    NDDS_Discovery_Plugin_BeforeRemoteParticipantDeletedCallback
    on_before_remote_participant_deleted;

    /*ci
     * \brief Method called when the participant is created, but not yet
     *        initialized
     */
    NDDS_Discovery_Plugin_BeforeLocalParticipantCreatedCallback
    on_before_local_participant_created;

    /*ci
     * \brief Method called when the participant has been created but it not
     *        yet enabled
     */
    NDDS_Discovery_Plugin_AfterLocalParticipantCreatedCallback
    on_after_local_participant_created;

    /*ci
     * \brief Method called when the participant is enabled
     */
    NDDS_Discovery_Plugin_AfterLocalParticipantEnabledCallback
    on_after_local_participant_enabled;

    /*ci
     * \brief Method called when the participant is being deleted
     */
    NDDS_Discovery_Plugin_BeforeLocalParticipantDeletedCallback
    on_before_local_participant_deleted;

    /*ci
     * \brief Method called when a local datawriter is enabled
     */
    NDDS_Discovery_Plugin_AfterLocalDataWriterEnabledCallback
    on_after_local_datawriter_enabled;

    /*ci
     * \brief Method called when a local datawriter is deleted
     */
    NDDS_Discovery_Plugin_AfterLocalDataWriterDeletedCallback
    on_after_local_datawriter_deleted;

    /*ci
     * \brief Method called when a local datareader is enabled
     */
    NDDS_Discovery_Plugin_AfterLocalDataReaderEnabledCallback
    on_after_local_datareader_enabled;

    /*ci
     * \brief Method called when a local datareader is deleted
     */
    NDDS_Discovery_Plugin_AfterLocalDataReaderDeletedCallback
    on_after_local_datareader_deleted;

    /*ci
     * \brief Method called when a peer is added to the participant
     */
    NDDS_Discovery_Plugin_AddPeerFunction add_peer;

    /*ci
     * \brief Method called when a remote participant is asserted in the
     *        participant
     */
    NDDS_Discovery_Plugin_AssertRemoteParticipant on_assert_remote_participant;

    /*ci
     * \brief Method called before a datareader is created
     */
    NDDS_Discovery_Plugin_BeforeLocalDataReaderCreatedCallback
    on_before_local_datareader_created;

    /*ci
     * \brief Method called before a datawriter is created
     */
    NDDS_Discovery_Plugin_BeforeLocalDataWriterCreatedCallback
    on_before_local_datawriter_created;

    /*ci
     * \brief Method called when the local participant is disposed
     *        of.
     *
     * \details This method is called when the local participant is
     *          no longer available to other participants in the
     *          domain.
     */
    NDDS_Discovery_Plugin_DisposeLocalParticipantCallback
    on_dispose_local_participant;

    /*ci
     * \brief Method to call if returns DDS_RETCODE_OUT_OF_RESOURCES
     */
    NDDS_Discovery_Plugin_PublicationBuiltinTopicDataReturnLoanCallback
    on_publication_data_return_loan;

    /*ci
     * \brief Method to call if returns DDS_RETCODE_OUT_OF_RESOURCES
     */
    NDDS_Discovery_Plugin_SubscriptionBuiltinTopicDataReturnLoanCallback
    on_subscription_data_return_loan;
};

/*ci
 * \brief Wrapper to call of the DiscoveryComponentFactory->create_component
 *
 * \param[in] f_ The discovery factory
 * \param[in] p_ The property
 * \param[in] l_ The listener
 *
 * \return A reference to new discovery instance on success, NULL otherwise
 */
#define DiscoveryComponentFactory_create_component(f_,p_,l_) \
    (struct NDDS_Discovery_Plugin*)((f_)->intf)->create_component(f_,p_,l_)

/*ci
 * \brief Wrapper to call of the DiscoveryComponentFactory->delete_component
 *
 * \param[in] f_ The discovery factory
 * \param[in] c_ The discovery instance to delete
 */
#define DiscoveryComponentFactory_delete_component(f_,c_) \
    ((f_)->intf)->delete_component(f_,(RT_Component_T*)(c_))

/*ci
 * \brief Wrapper to call \ref NDDS_DiscoveryI::on_before_local_participant_created
 *
 * \details
 * When a participant creates an instance of the discovery plugin it calls
 * this function before allocating any resources. The plugin is allowed to
 * modify the participant's qos to take its own resource needs into
 * account.
 *
 * \param[in]    self_ The discovery plugin
 * \param[in]    p_    The participant creating the plugin
 * \param[inout] q_    The participant's qos policy
 * \param[inout] b_    The participant's announcement data
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
#define NDDS_Discovery_Plugin_on_before_local_participant_created(\
        self_,p_,q_,b_) \
((struct NDDS_DiscoveryI*)((self_)->_parent._intf))->\
  on_before_local_participant_created(self_,p_,q_,b_)

/*ci
 * \brief Wrapper to call \ref NDDS_DiscoveryI::on_after_local_participant_created
 *
 * \details
 * When a participant is created it is ready for creating user entities. In
 * this callback the discovery plugin creates the discovery endpoints. The qos
 * for the discovery endpoints are pre-defined by the DDS specification.
 *
 * \param[in] self_ The discovery plugin
 * \param[in] p_    The participant creating the plugin
 * \param[in] data_ The participant's announcement data
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
#define NDDS_Discovery_Plugin_on_after_local_participant_created(\
        self_,p_,data_) \
((struct NDDS_DiscoveryI*)((self_)->_parent._intf))->\
  on_after_local_participant_created(self_,p_,data_)

/*ci
 * \brief Wrapper to call \ref NDDS_DiscoveryI::on_after_local_participant_enabled
 *
 * \details
 * When a participant is enabled it is ready to announce discovery data. In
 * this callback the discovery plugin enables its own discovery endpoints and
 * starts announcing the participant.
 *
 * \param[in] self_ The discovery plugin
 * \param[in] p_    The participant creating the plugin
 * \param[in] data_ The participant's announcement data
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
#define NDDS_Discovery_Plugin_on_after_local_participant_enabled(\
        self_,p_,data_) \
((struct NDDS_DiscoveryI*)((self_)->_parent._intf))->\
  on_after_local_participant_enabled(self_,p_,data_)

/*ci
 * \brief Wrapper to call \ref NDDS_DiscoveryI::on_dispose_local_participant
 *
 * \details
 * When a participant is deleted the plugin is called when the local participant
 * should be signaled as no longer being available in a domain and other
 * participants may free up resources. Typically the discovery plugin uses
 * the DDS dispose method to dispose of the participant. This method is called
 * during the deletion of the local participant.
 *
 * \param[in] self_ The discovery plugin
 * \param[in] p_    The participant being disposed of.
 * \param[in] key_  The participant's key
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
#define NDDS_Discovery_Plugin_on_dispose_local_participant(\
        self_,p_,key_) \
((struct NDDS_DiscoveryI*)((self_)->_parent._intf))->\
  on_dispose_local_participant(self_,p_,key_)

/*ci
 * \brief Wrapper to call \ref NDDS_DiscoveryI::on_before_local_participant_deleted
 *
 * \details
 * When a participant is deleted the plugin must delete all its internal
 * resources.
 *
 * \param[in] self_ The discovery plugin
 * \param[in] p_    The participant being deleted
 * \param[in] key_  The participant's key
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
#define NDDS_Discovery_Plugin_on_before_local_participant_deleted(\
        self_,p_,key_) \
((struct NDDS_DiscoveryI*)((self_)->_parent._intf))->\
  on_before_local_participant_deleted(self_,p_,key_)

/*ci
 * \brief Wrapper to call \ref NDDS_DiscoveryI::on_after_local_datawriter_enabled
 *
 * \details
 * This function is called when a local datawriter is enabled.
 *
 * \param[in] self_ The discovery plugin
 * \param[in] p_    The participant
 * \param[in] dw_   The datawriter being enabled
 * \param[in] dq_   The datawriters Qos
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
#define NDDS_Discovery_Plugin_on_after_local_datawriter_enabled(\
        self_,p_,dw_,dq_) \
((struct NDDS_DiscoveryI*)((self_)->_parent._intf))->\
  on_after_local_datawriter_enabled(self_,p_,dw_,dq_)

/*ci
 * \brief Wrapper to call \ref NDDS_DiscoveryI::on_after_local_datawriter_deleted
 *
 * \details
 * This function is called when a local datawriter is deleted.
 *
 * \param[in] self_  The discovery plugin
 * \param[in] p_     The participant
 * \param[in] dwkey_ The key of the deleted datawriter
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
#define NDDS_Discovery_Plugin_on_after_local_datawriter_deleted(\
        self_,p_,dwkey_) \
((struct NDDS_DiscoveryI*)((self_)->_parent._intf))->\
  on_after_local_datawriter_deleted(self_,p_,dwkey_)

/*ci
 * \brief Wrapper to call \ref NDDS_DiscoveryI::on_after_local_datareader_enabled
 *
 * \details
 * This function is called when a local datareader is enabled.
 *
 * \param[in] self_ The discovery plugin
 * \param[in] p_    The participant
 * \param[in] dr_   The datareader being enabled
 * \param[in] dq_   The datareaders Qos
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
#define NDDS_Discovery_Plugin_on_after_local_datareader_enabled(\
        self_,p_,dr_,dq_) \
((struct NDDS_DiscoveryI*)((self_)->_parent._intf))->\
  on_after_local_datareader_enabled(self_,p_,dr_,dq_)

/*ci
 * \brief Wrapper to call \ref NDDS_DiscoveryI::on_after_local_datareader_deleted
 *
 * \details
 * This function is called when a local datareader is deleted.
 *
 * \param[in] self_  The discovery plugin
 * \param[in] p_     The participant
 * \param[in] drkey_ The datareader being enabled
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
#define NDDS_Discovery_Plugin_on_after_local_datareader_deleted(self_,p_,drkey_)\
((struct NDDS_DiscoveryI*)((self_)->_parent._intf))->\
  on_after_local_datareader_deleted(self_,p_,drkey_)

/*ci
 * \brief Wrapper to call \ref NDDS_DiscoveryI::on_before_remote_participant_deleted
 *
 * \details
 * The participant calls this interface when it has decided to delete a
 * remote participant record, before the record is deleted (and remote endpoints
 * unmatched). This could happen when the participant detects that the remote
 * participant has failed to maintain its lease_duration commitment, or, in
 * case trust extensions are enabled, if the remote participant failed to be
 * authenticated.
 *
 * \param[in] self_ The discovery plugin
 * \param[in] p_    The participant
 * \param[in] data_ The remote participant data
 * \param[in] status_ The remote participant's status flags
 *
 * \return DDS_BOOLEAN_TRUE on success or DDS_BOOLEAN_FALSE on failure
 */
#define NDDS_Discovery_Plugin_on_before_remote_participant_deleted(\
        self_,p_,data_,status_) \
((struct NDDS_DiscoveryI*)((self_)->_parent._intf))->\
  on_before_remote_participant_deleted(self_,p_,data_,status_)

/*ci
 * \brief Wrapper to call \ref NDDS_DiscoveryI::add_peer
 *
 * \details
 * This API enables peer addresses to be dynamically added to a participants
 * peer list after it has been created.
 *
 * \param[in] self_ The discovery plugin
 * \param[in] p_    The participant
 * \param[in] peer_ A peer address string
 *
 * \return DDS_BOOLEAN_TRUE on success or DDS_BOOLEAN_FALSE on failure
 */
#define NDDS_Discovery_Plugin_add_peer(\
        self_,p_,peer_) \
((struct NDDS_DiscoveryI*)((self_)->_parent._intf))->add_peer(self_,p_,peer_)

/*ci
 * \brief Wrapper to call \ref NDDS_DiscoveryI::on_assert_remote_participant
 *
 * \details
 * When a remote participant is added to a participant the discovery plugin
 * is notified. The discovery plugin keeps track of which participants have
 * been statically asserted. If a participant is discovered, but has not
 * been statically asserted it is ignored.
 *
 * \param[in] self_ The discovery plugin
 * \param[in] p_    The participant
 * \param[in] name_ The name of the remote participant
 */
#define NDDS_Discovery_Plugin_assert_remote_participant(self_,p_,data_) \
((struct NDDS_DiscoveryI*)((self_)->_parent._intf))->on_assert_remote_participant(self_,p_,data_)

/*ci
 * \brief Wrapper to call \ref NDDS_DiscoveryI::on_before_local_datareader_created method
 *
 * \param[in] self_   The discovery plugin
 * \param[in] p_      The participant
 * \param[in] dr_key_ The key of the datareader to be created
 * \param[in] res_    DDS_BOOLEAN_TRUE if this is a reservation,
 *                    DDS_BOOLEAN_FALSE if this is releasing a
 *                    reservation.
 *
 * \return DDS_RETCODE_OK success, one of the standard return codes on failure
 */
#define NDDS_Discovery_Plugin_on_before_local_datareader_created(self_,p_,dr_key_,res_) \
((struct NDDS_DiscoveryI*)((self_)->_parent._intf))->on_before_local_datareader_created(self_,p_,dr_key_,res_)

/*ci
 * \brief Wrapper to call \ref NDDS_DiscoveryI::on_before_local_datawriter_created method
 *
 * \param[in] self_   The discovery plugin
 * \param[in] p_      The participant
 * \param[in] dw_key_ The key of the datawriter to be created
 * \param[in] res_    DDS_BOOLEAN_TRUE if this is a reservation,
 *                    DDS_BOOLEAN_FALSE if this is releasing a
 *                    reservation.
 *
 * \return DDS_RETCODE_OK success, one of the standard return codes on failure
 */
#define NDDS_Discovery_Plugin_on_before_local_datawriter_created(self_,p_,dw_key_,res_) \
((struct NDDS_DiscoveryI*)((self_)->_parent._intf))->on_before_local_datawriter_created(self_,p_,dw_key_,res_)

#define NDDS_Discovery_Plugin_on_publication_data_return_loan(self_,p_,data_,info_) \
((struct NDDS_DiscoveryI*)((self_)->_parent._intf))->on_publication_data_return_loan(self_,p_,data_,info_)

#define NDDS_Discovery_Plugin_has_on_publication_data_return_loan(self_) \
(((struct NDDS_DiscoveryI*)(self_)->_parent._intf)->on_publication_data_return_loan != NULL)

#define NDDS_Discovery_Plugin_on_subscription_data_return_loan(self_,p_,data_,info_) \
((struct NDDS_DiscoveryI*)((self_)->_parent._intf))->on_subscription_data_return_loan(self_,p_,data_,info_)

#define NDDS_Discovery_Plugin_has_on_subscription_data_return_loan(self_) \
(((struct NDDS_DiscoveryI*)(self_)->_parent._intf)->on_subscription_data_return_loan != NULL)

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif /* dds_c_discovery_plugin_h */

/*e @} */
