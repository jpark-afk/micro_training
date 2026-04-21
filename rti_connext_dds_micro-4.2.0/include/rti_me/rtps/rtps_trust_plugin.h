/*
 * FILE: rtps_psk_plugin.h - RTPS Trust plugin Interface
 *
 * (c) Copyright, Real-Time Innovations, 2025-2025.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 */

#ifndef rtps_trust_plugin_h
#define rtps_trust_plugin_h


#ifndef cdr_cdr_type_h
#include "cdr/cdr_cdr_type.h"
#endif
#ifndef reda_buffer_h
#include "reda/reda_buffer.h"
#endif
#ifndef rt_rt_h
#include "rt/rt_rt.h"
#endif
#ifndef cdr_stream_h
#include "cdr/cdr_stream.h"
#endif
#ifndef netio_interface_h
#include "netio/netio_interface.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif


#define DDS_SampleHash      OSAPI_Hash

#define DDS_HASH_NIL        OSAPI_HASH_NIL

/*ci
* \brief Trust plugin state.
*
* This structure is used to store the state of the trust plugin returned by the
* trust plugin. This allows Micro to make a decision on the features available
* for the participant.
*/
typedef struct RTPS_TrustPluginState
{
    RTI_BOOL is_aad_enabled;
    RTI_BOOL is_rtps_transform_enabled;
} RTPS_TrustPluginState;

#define RTPS_TrustPluginState_INITIALIZER \
{\
    RTI_FALSE, /* is_aad_enabled */\
    RTI_FALSE  /* is_rtps_transform_enabled */\
}

/* forward declarations */
struct DDS_DomainParticipantQos;
struct DDS_GUID_t;


/*******************************************************************************
 *
 *                            The Service Module
 *
 ******************************************************************************/
typedef struct RTPS_TrustPluginProperty
{
    struct RT_ComponentProperty _parent;
    const struct DDS_DomainParticipantQos *qos;
}RTPS_TrustPluginProperty;

#define RTPS_TrustPluginProperty_INITIALIZER \
{\
    RT_ComponentProperty_INITIALIZER, /*parent*/\
    NULL /*qos*/\
}

typedef struct RTPS_TrustPlugin
{
    struct RT_Component _parent;
}RTPS_TrustPlugin;

 /*ci
 *
 * /brief Function pointer type for retrieving the maximum serialized size for Participant Builtin Topic Data
 *
 *
 * This function pointer is used to define a method that calculates the maximum
 * serialized size of a all trust specific parameters for the Participant BuiltinTopic Data
 *
 * \param self A pointer to the RTPS_TrustPlugin instance.
 * \param size The current size to be considered for the calculation.
 * \return The maximum serialized size as an RTI_UINT32 value.
 */
 typedef RTI_UINT32
(*RTPS_TrustPlugin_get_max_serialized_sizeFunc_T)(
    RTPS_TrustPlugin *self, RTI_UINT32 size);

/*ci
 * \brief Function pointer type for serializing all the trust specific parameters.
 *
 * \param self Pointer to the RTPS_TrustPlugin instance.
 * \param stream Pointer to the CDR_Stream_t structure for serialization.
 * \return RTI_BOOL indicating success (RTI_TRUE) or failure (RTI_FALSE).
 *  */
typedef RTI_BOOL
(*RTPS_TrustPlugin_serialize_parameterFunc_T)(
    RTPS_TrustPlugin *self,
    struct CDR_Stream_t *stream);

/*ci
 * \brief Function pointer type for validating participant trust.
 *
 * \param [in] self Pointer to the RTPS_TrustPlugin instance.
 * \param [in] domain_id The domain ID of the participant.
 * \param [in] qos Pointer to the DDS_DomainParticipantQos structure containing QoS settings.
 * \param [inout] candidate_guid_inout Pointer to the DDS_GUID_t structure for the candidate GUID (input/output).
 * \param [inout] state A state returned by the plugin that allows the caller to make decision on the features available
 * \return RTI_BOOL indicating whether the participant trust is valid (RTI_TRUE) or not (RTI_FALSE).
 */
typedef RTI_BOOL
(*RTPS_TrustPlugin_validate_participant_trustFunc_T)(
    RTPS_TrustPlugin *self,
    RTI_INT32 domain_id,
    const struct DDS_DomainParticipantQos *qos,
    struct DDS_GUID_t *candidate_guid_inout,
    RTPS_TrustPluginState *state);


/*ci
 * \brief Function pointer type for transforming RTPS messages.
 *
 * \param self Pointer to the RTPS_TrustPlugin instance.
 * \param packet Pointer to the NETIO_Packet_T structure containing the packet data.
 * \param transformed_buf Pointer to the REDA_Buffer structure for storing the transformed data.
 * \return RTI_BOOL indicating success (RTI_TRUE) or failure (RTI_FALSE).
 */
typedef RTI_BOOL
(*RTPS_TrustPlugin_transform_rtps_messageFunc_T)(
    RTPS_TrustPlugin *self,
    NETIO_Packet_T *packet,
    struct REDA_Buffer *transformed_buf);

/*ci
 *
 * \brief Function pointer type for invalidating participant trust.
 * \param self Pointer to the RTPS_TrustPlugin instance.
 */
typedef void
(*RTPS_TrustPlugin_invalidate_participant_trustFunc_T)(
        RTPS_TrustPlugin *self);

/*ci
 * \brief Sets the (QoS) parameters for the  trust plugin.
 *
 * \param self Pointer to the RTPS_Trust plugin
 * \param qos Pointer to the QoS structure containing the desired settings.
 * \return RTI_BOOL Returns RTI_TRUE on success, or a RTI_FALSE  failure.
 */
typedef RTI_BOOL
(*RTPS_TrustPlugin_set_qos)
    (RTPS_TrustPlugin *self,
     const struct DDS_DomainParticipantQos *qos);

/*ci
 * \brief updates the trust plugin with information about the matched remote participant
 *
 * \param self Pointer to the RTPS_Trust plugin
 * \param self Pointer to remote participants guid.
 * \return RTI_BOOL Returns RTI_TRUE on success, or a RTI_FALSE  failure.
 */
typedef RTI_BOOL
(*RTPS_TrustPlugin_matched_remote_participantFunc_T)
    (RTPS_TrustPlugin *self,
     struct DDS_GUID_t* remote_participant_guid);



/* ci
 * \brief RTPS_Trust_ServiceI structure
 *
 * This structure defines the interface for the RTPS Trust service.
 * It inherits from the base class RT_ComponentI and contains function pointers
 * for various operations related to the Trust service.
 */
typedef struct RTPS_Trust_ServiceI
{
    /*ci
     * \brief Inherit from base-class
     */
    struct RT_ComponentI _parent;
    RTPS_TrustPlugin_get_max_serialized_sizeFunc_T
        get_max_serialized_size;
    RTPS_TrustPlugin_serialize_parameterFunc_T
        serialize_trust_parameter;
    RTPS_TrustPlugin_validate_participant_trustFunc_T
        validate_participant_trust;
    RTPS_TrustPlugin_invalidate_participant_trustFunc_T
        invalidate_participant_trust;
    RTPS_TrustPlugin_transform_rtps_messageFunc_T
        outgoing_transform;
    RTPS_TrustPlugin_transform_rtps_messageFunc_T
        incoming_transform;
    RTPS_TrustPlugin_set_qos
        set_qos;
    RTPS_TrustPlugin_matched_remote_participantFunc_T
        register_matched_remote_participant;
    RTPS_TrustPlugin_matched_remote_participantFunc_T
        unregister_matched_remote_participant;

}RTPS_Trust_ServiceI;

#define RTPS_Trust_ServiceI_INITIALIZER \
{\
    RT_COMPONENTI_BASE,\
    NULL,  /* get_max_serialized_size */\
    NULL,  /* serialize */\
    NULL,  /* validate_ParticipantTrust */\
    NULL,  /* invalidate_ParticipantTrust */\
    NULL,  /* outgoing_transform */\
    NULL,  /* incoming_transform */\
    NULL,  /* set_qos */\
    NULL, /* register_matched_remote_participant */\
    NULL, /* unregister matched remote participant */\
}

#define RTPS_TrustPlugin_create_component(f_,p_,l_) \
    (struct RTPS_TrustPlugin*)((f_)->intf)->create_component(f_,p_,l_)

#define RTPS_TrustPlugin_delete_component(f_,c_) \
    ((f_)->intf)->delete_component(f_,(RT_Component_T*)(c_))

#define RTPS_TrustPlugin_validate_participant_trust(self_, domainid_, qos_, guid_, state_) \
    ((struct RTPS_Trust_ServiceI*)((self_)->_parent._intf))-> \
    validate_participant_trust(self_, domainid_, qos_, guid_, state_)

#define RTPS_TrustPlugin_max_serialized_size(self_, sz_) \
    ((struct RTPS_Trust_ServiceI*)((self_)->_parent._intf))-> \
        get_max_serialized_size(self_, sz_)

#define RTPS_TrustPlugin_serialize(self_, str_) \
    ((struct RTPS_Trust_ServiceI*)((self_)->_parent._intf))-> \
        serialize_trust_parameter(self_, str_)

#define RTPS_TrustPlugin_invalidate_participant_trust(self_) \
    ((struct RTPS_Trust_ServiceI*)((self_)->_parent._intf))-> \
        invalidate_participant_trust(self_)

#define RTPS_TrustPlugin_outgoing_transform(self_, pkt_, t_buf_)\
    ((struct RTPS_Trust_ServiceI*)((self_)->_parent._intf))-> \
        outgoing_transform(self_, pkt_, t_buf_)

#define RTPS_TrustPlugin_incoming_transform(self_, pkt_, t_buf_)\
        ((struct RTPS_Trust_ServiceI*)((self_)->_parent._intf))-> \
            incoming_transform(self_, pkt_, t_buf_)

#define RTPS_TrustPlugin_set_qos(self_, prop_)\
        ((struct RTPS_Trust_ServiceI*)((self_)->_parent._intf))-> \
            set_qos(self_, prop_)

#define RTPS_TrustPlugin_register_matched_remote_participant(self_, guid_)\
        ((struct RTPS_Trust_ServiceI*)((self_)->_parent._intf))-> \
            register_matched_remote_participant(self_, guid_)

#define RTPS_TrustPlugin_unregister_matched_remote_participant(self_, guid_)\
        ((struct RTPS_Trust_ServiceI*)((self_)->_parent._intf))-> \
            unregister_matched_remote_participant(self_, guid_)

#ifdef __cplusplus
}                               /* extern "C" */
#endif
#endif
