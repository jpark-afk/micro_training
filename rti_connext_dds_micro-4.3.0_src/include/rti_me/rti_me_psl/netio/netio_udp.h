/*
 * FILE: netio_udp.h - UDP API
 *
 * Copyright (c) 2008-2026 Real-Time Innovations, Inc.
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
 * 28oct2021,tk MICRO-3322/PR.29864
 * - Conditionally include disable_auto_interface_config when
 *   NETIO_CONFIG_HAVE_IFCONF is set or RTI_WIN32 is defined.
 * 20sep2021,tk MICRO-3152/PR.29564
 * - Conditionally set thread options to SUSPEND_ENABLE only when
 *   OSAPI_THREAD_SEMAPHORE_ENABLED is TRUE.
 * 13apr2021,tk MICRO-3028/PR.28960
 * - Fixed documentation for UDP_InterfaceTableEntry
 * 09apr2021,tk MICRO-2942/PR.28898
 *   - Use external documentation for UDP_InterfaceFactoryProperty_initialize
 *     and UDP_InterfaceFactoryProperty_finalize.
 * 25feb2021,tk MICRO-2914/PR#28853
 *    - Use OSAPI_THREAD_PRIORITY_DEFAULT, not OSAPI_THREAD_PRIORITY_NORMAL
 * 14dec2020,tk  MICRO-2679/PR.28247 Only include max_send_message_size
 *               and enable_interface_bind when UDP_TRANSFORMS_ENABLED
 *               are enabled.
 * 07may2020,fmt MICRO-2366/PR.27591 Default send and receive socket size is
  *                                   too big for QNX
 * 19may2015,as  MICRO-1193 Refactoring of Sequence API levels
 * 13mar2013,eh  Fix MICRO-352 (max message/send/receive sizes)
 * 25apr2012,tk  Written
 *
 */
/*ci
 * \file
 * \defgroup NETIO_UDPInterfaceClass UDP Interface
 * \ingroup NETIOModule
 * \brief NETIO UDP Interface
 *
 * \details
 *
 * The UDP interface is implemented as a NETIO interface and NETIO interface
 * factory.
 */
/*ci \addtogroup NETIO_UDPInterfaceClass
 * @{
 */
#ifndef netio_udp_h
#define netio_udp_h

#ifndef RTI_EXPORT_REDA_SEQUENCE
#define RTI_EXPORT_REDA_SEQUENCE
#endif

#ifndef osapi_config_h
#include "osapi/osapi_config.h"
#endif
#ifndef rti_me_psl_dll_h
#include "rti_me_psl/rti_me_psl_dll.h"
#endif
#ifndef osapi_thread_h
#include "osapi/osapi_thread.h"
#endif
#ifndef rt_rt_h
#include "rt/rt_rt.h"
#endif
#ifndef reda_string_h
#include "reda/reda_string.h"
#endif
#ifndef reda_sequence_h
#include "reda/reda_sequence.h"
#endif
#ifndef netio_config_h
#include "netio/netio_config.h"
#endif
#ifndef netio_address_h
#include "netio/netio_address.h"
#endif
#ifndef netio_route_h
#include "netio/netio_route.h"
#endif
#include "netio/netio_interface.h"
#ifndef netio_dgram_h
#include "netio_dgram/netio_dgram.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*ci
 * \def NETIO_CONFIG_HAVE_IFCONF
 * \brief Set to 1 if the network stack supports reading the interface list
 */
#ifndef NETIO_CONFIG_HAVE_IFCONF
#if (defined(RTI_UNIX) || defined(RTI_VXWORKS)) && \
    !defined(RTI_CERT) && !defined(RTI_NO_IFCONFIG) && \
    (ENABLE_FACE_COMPLIANCE == FACE_COMPLIANCE_LEVEL_NONE)
#define NETIO_CONFIG_HAVE_IFCONF 1
#else
#define NETIO_CONFIG_HAVE_IFCONF 0
#endif
#endif /* NETIO_CONFIG_HAVE_IFCONF */

/*ci
 * \def NETIO_CONFIG_ENABLE_MULTICAST
 * \brief Set to 1 if multicast should be enabled
 *
 * \details
 *
 * The configuration file is platform independent and may be overridden
 * by an implementation. That is, if multicast is disabled here it shall
 * _not_ be implemented by the transport. However, even if multicast is
 * enabled here it _may_ be disabled by the implementation if the implementation
 * detects it is not supported.
 */
#ifdef RTI_CERT
/* Multicast is disabled for Cert configuration  */
#ifndef NETIO_CONFIG_ENABLE_MULTICAST
#define NETIO_CONFIG_ENABLE_MULTICAST 0
#endif
#else
#ifndef NETIO_CONFIG_ENABLE_MULTICAST
/* Multicast is disabled for FACE Safety Base & Security profiles */
#if (ENABLE_FACE_COMPLIANCE > FACE_COMPLIANCE_LEVEL_NONE) && \
    (ENABLE_FACE_COMPLIANCE < FACE_COMPLIANCE_LEVEL_SAFETY_EXTENDED)
#define NETIO_CONFIG_ENABLE_MULTICAST 0
#else
#define NETIO_CONFIG_ENABLE_MULTICAST 1
#endif
#endif /* NETIO_CONFIG_ENABLE_MULTICAST */
#endif /* !RTI_CERT */

/*ci
 * \brief Enable IP aliases interfaces
 *
 * \details
 * Setting this option to 1 disables filtering out IP aliases. Note that
 * this currently only works on platforms where each IP alias has its own
 * interface name, such as eth0:1, eth1:2 etc.
 */
#ifndef UDP_ENABLE_IPALIASES
#define UDP_ENABLE_IPALIASES  0
#endif

/*ci
 * \brief Enable transformed UDP payloads
 *
 * \details
 * Setting this option to 0 disables the possibility of sending/receiving
 * transformed UDP payloads.
 */
#ifndef UDP_ENABLE_TRANSFORMS
#define UDP_ENABLE_TRANSFORMS  0
#elif UDP_ENABLE_TRANSFORMS
#ifndef UDP_ENABLE_INTERFACE_BIND
#define UDP_ENABLE_INTERFACE_BIND (1)
#endif
#endif

/*ci \brief Determine the default value for UDP_ENABLE_INTERFACE_BIND
 *          when not set.
 */
#ifndef UDP_ENABLE_INTERFACE_BIND
#ifndef RTI_CERT
#define UDP_ENABLE_INTERFACE_BIND (1)
#else
#define UDP_ENABLE_INTERFACE_BIND (0)
#endif
#endif

/*ci \brief Determine if interface bind should be enabled
 */
#if UDP_ENABLE_INTERFACE_BIND
#define UDP_INTERFACE_BIND_ENABLED (1)
#else
#define UDP_INTERFACE_BIND_ENABLED (0)
#endif

/*ci \brief Determine if UDP transformations should and can be enabled
 */
#if UDP_ENABLE_TRANSFORMS
#if !UDP_INTERFACE_BIND_ENABLED
#error "UDP Transformations require UDP_ENABLE_INTERFACE_BIND to be enabled"
#endif
#define UDP_TRANSFORMS_ENABLED UDP_ENABLE_TRANSFORMS
#endif

/*ci
 * \brief Removes the UDP transport from built
 *
 * \details
 * Setting this option to 1 removes UDP transport from being built. This can be
 * useful if communication is done only using shared memory, INTRA or a custom
 * UDP transport.
 */
#ifndef UDP_EXCLUDE_BUILTIN
#define UDP_EXCLUDE_BUILTIN  0
#endif

/*ci \brief Determine if transport priority should be enforced
 *
 * \details
 * Setting this option to 1 causes the UDP transport to fail sending messages if
 * it fails to set the socket priority to the expected value. Setting this to 0
 * allows the transport to continue sending messages even if it fails to set the
 * socket priority, which can be useful in environments where setting socket
 * priority is not supported or requires elevated privileges.
 */
#ifndef UDP_ENFORCE_TRANSPORT_PRIORITY
#define UDP_ENFORCE_TRANSPORT_PRIORITY 1
#endif

MUST_CHECK_RETURN NETIOPSLDllExport RTI_BOOL
UDP_Interface_leak_NETIO_DGRAM_InterfaceI(struct NETIO_DGRAM_InterfaceI **user_intfI);

#ifndef RTI_CERT

/*e \dref_UDP_NatEntry
 */
struct DDSCPPDllExport UDP_NatEntry
{
    /*ce \dref_UDP_NatEntry_local_address
     */
    struct NETIO_Address local_address;

    /*ce \dref_UDP_NatEntry_public_address
     */
    struct NETIO_Address public_address;
};

/*ce \dref_UDP_NatEntry_INITIALIZER
 */
#define UDP_NatEntry_INITIALIZER \
{\
    NETIO_Address_INITIALIZER,\
    NETIO_Address_INITIALIZER\
}

#define T struct UDP_NatEntry
#define TSeq UDP_NatEntrySeq
#include <reda/reda_sequence_decl.h>
#ifdef DOXYGEN_DOCUMENTATION_ONLY
/*e
 * \dref_UDP_NatEntrySeq
 */
struct UDP_NatEntrySeq {};
#endif

/*ce \dref_UDP_NatEntrySeq_INITIALIZER
 */
#define UDP_NatEntrySeq_INITIALIZER \
                        REDA_DEFINE_SEQUENCE_INITIALIZER(struct UDP_NatEntry)

#define UDP_NAT_INITIALIZER UDP_NatEntrySeq_INITIALIZER,

#else /* !RTI_CERT */
#define UDP_NAT_INITIALIZER
#endif

/*e \dref_UDP_INTERFACE_MAX_IFNAME
 */
#define UDP_INTERFACE_MAX_IFNAME 64

/*e \dref_UDP_INTERFACE_INTERFACE_UP_FLAG
 */
#define UDP_INTERFACE_INTERFACE_UP_FLAG        0x1U

/*e \dref_UDP_INTERFACE_INTERFACE_MULTICAST_FLAG
 */
#define UDP_INTERFACE_INTERFACE_MULTICAST_FLAG 0x2U

/*e \dref_UDP_INTERFACE_MAX_NETMASK_BITS
 */
#define UDP_INTERFACE_MAX_NETMASK_BITS (32U)

/*e \dref_UDP_InterfaceTableEntry
 */
struct DDSCPPDllExport UDP_InterfaceTableEntry
{
    /*e \dref_UDP_InterfaceTableEntry_flags
     */
    RTI_UINT32 flags;

    /*e \dref_UDP_InterfaceTableEntry_address
     */
    RTI_UINT32 address;

    /*e \dref_UDP_InterfaceTableEntry_netmask
     */
    RTI_UINT32 netmask;

    /*e \dref_UDP_InterfaceTableEntry_ifname
     */
    char ifname[UDP_INTERFACE_MAX_IFNAME];
};

/*ce
 * \def UDP_InterfaceTableEntry_INITIALIZER
 */
#define UDP_InterfaceTableEntry_INITIALIZER \
{\
    0,\
    0,\
    0,\
    {0}\
}

#define T struct UDP_InterfaceTableEntry
#define TSeq UDP_InterfaceTableEntrySeq
#include <reda/reda_sequence_decl.h>
#ifdef DOXYGEN_DOCUMENTATION_ONLY
/*e
 * \dref_UDP_InterfaceTableEntrySeq
 */
struct UDP_InterfaceTableEntrySeq {};
#endif

#define UDP_InterfaceTableEntrySeq_INITIALIZER \
               REDA_DEFINE_SEQUENCE_INITIALIZER(struct UDP_InterfaceTableEntry)

#define UDP_InterfaceTableEntrySeq_INITIALIZER_W_LOAN(b_, m_, l_) \
REDA_DEFINE_SEQUENCE_INITIALIZER_W_LOAN(b_, m_, l_, struct UDP_InterfaceTableEntry*)

/*ce \dref_UDP_InterfaceTable_add_entry
 */
MUST_CHECK_RETURN NETIOPSLDllExport RTI_BOOL
UDP_InterfaceTable_add_entry(struct UDP_InterfaceTableEntrySeq *seq,
                             RTI_UINT32 address,
                             RTI_UINT32 netmask,
                             const char *ifname,
                             RTI_UINT32 flags);

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#ifdef __cplusplus
extern "C"
{
#endif

struct UDP_InterfaceFactoryProperty;

/*e \dref_UDP_InterfaceFactoryProperty_initialize
 */
NETIOPSLDllExport RTI_BOOL
UDP_InterfaceFactoryProperty_initialize(
            struct UDP_InterfaceFactoryProperty *self);

#ifndef RTI_CERT
/*e \dref_UDP_InterfaceFactoryProperty_finalize
 */
NETIOPSLDllExport RTI_BOOL
UDP_InterfaceFactoryProperty_finalize(
        struct UDP_InterfaceFactoryProperty *self);
#endif /* !RTI_CERT */

#ifdef __cplusplus
}
#endif

#if UDP_TRANSFORMS_ENABLED

#ifdef __cplusplus
extern "C"
{
#endif

/*ce
 * \dref_UDP_Transform
 */
typedef struct UDP_Transform
{
    /*ci
     * \brief Component base-class
     */
    struct RT_Component _parent;
} UDP_Transform_T;

/*ce \dref_UDP_TransformProperty
 */
struct UDP_TransformProperty
{
    /*ci
     * \brief Component base-class
     */
    struct RT_ComponentProperty _parent;

    /*ce \dref_UDP_TransformProperty_max_send_message_size
     */
    RTI_INT32 max_send_message_size;

    /*ce \dref_UDP_TransformProperty_max_receive_message_size
     */
    RTI_INT32 max_receive_message_size;
};

/*ce \dref_UDP_TransformProperty_INITIALIZER
 */
#define UDP_TransformProperty_INITIALIZER \
{ \
    RT_ComponentProperty_INITIALIZER,\
    -1,\
    -1 \
}

/*ci \dref_UDP_TransformI_create_destination_transform
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*UDP_TransformI_create_destination_transform)(
                                     UDP_Transform_T *const self,
                                     void **const context,
                                     const struct NETIO_Address *const destination,
                                     const struct NETIO_Netmask *const netmask,
                                     void *user_data,
                                     const struct UDP_TransformProperty *const property,
                                     RTI_INT32 *const ec)
)

/*ci \dref_UDP_TransformI_create_source_transform
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*UDP_TransformI_create_source_transform)(UDP_Transform_T *const self,
                                    void **const context,
                                    const struct NETIO_Address *const source,
                                    const struct NETIO_Netmask *const netmask,
                                    void *user_data,
                                    const struct UDP_TransformProperty *const property,
                                    RTI_INT32 *const ec)
)

/*ci \dref_UDP_TransformI_delete_destination_transform
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*UDP_TransformI_delete_destination_transform)(UDP_Transform_T *const self,
                               void *context,
                               const struct NETIO_Address *const destination,
                               const struct NETIO_Netmask *const netmask,
                               RTI_INT32 *const ec)
)

/*ci \dref_UDP_TransformI_delete_source_transform
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*UDP_TransformI_delete_source_transform)(UDP_Transform_T *const self,
                                      void *context,
                                      const struct NETIO_Address *const source,
                                      const struct NETIO_Netmask *const netmask,
                                      RTI_INT32 *const ec)
)

/*ci \dref_UDP_TransformI_transform_source
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*UDP_TransformI_transform_source)(UDP_Transform_T *const self,
                                   void *context,
                                   const struct NETIO_Address *const source,
                                   const NETIO_Packet_T *const in_packet,
                                   NETIO_Packet_T **out_packet,
                                   RTI_INT32 *const ec)
)

/*ci \dref_UDP_TransformI_transform_destination
 */
FUNCTION_MUST_TYPEDEF(
RTI_BOOL
(*UDP_TransformI_transform_destination)(UDP_Transform_T *const self,
                                        void *context,
                                        const struct NETIO_Address *const destination,
                                        const NETIO_Packet_T *const in_packet,
                                        NETIO_Packet_T **out_packet,
                                        RTI_INT32 *const ec)
)

/*ce
 * \ingroup UDPTransformModule
 */
struct UDP_TransformI
{
    /*ci
     * \brief Base-class interface
     */
    struct RT_ComponentI _parent;

    /*ce \dref_UDP_TransformI_create_destination_transform
     */
    UDP_TransformI_create_destination_transform create_destination_transform;

    /*ce \dref_UDP_TransformI_create_source_transform
     */
    UDP_TransformI_create_source_transform create_source_transform;

    /*ce \dref_UDP_TransformI_transform_source
     */
    UDP_TransformI_transform_source transform_source;

    /*ce \dref_UDP_TransformI_transform_destination
     */
    UDP_TransformI_transform_destination transform_destination;

    /*ce \dref_UDP_TransformI_delete_destination_transform
     */
    UDP_TransformI_delete_destination_transform delete_destination_transform;

    /*ce \dref_UDP_TransformI_delete_source_transform
     */
    UDP_TransformI_delete_source_transform delete_source_transform;
};

#define UDP_Transform_create_destination_transform(\
       self_,context_,destination_,netmask_,user_data_,property_,ec_) \
((struct UDP_TransformI*)self_->_parent._intf)->\
    create_destination_transform(self_,context_,\
                                 destination_,netmask_,user_data_,property_,ec_)

#define UDP_Transform_create_source_transform(\
       self_,context_,source_,netmask_,user_data_,property_,ec_) \
((struct UDP_TransformI*)self_->_parent._intf)->\
    create_source_transform(self_,context_,\
                                 source_,netmask_,user_data_,property_,ec_)

#define UDP_Transform_delete_destination_transform(\
       self_,context_,destination_,netmask_,ec_) \
((struct UDP_TransformI*)self_->_parent._intf)->\
    delete_destination_transform(self_,context_,destination_,netmask_,ec_)

#define UDP_Transform_delete_source_transform(\
       self_,context_,source_,netmask_,ec_) \
((struct UDP_TransformI*)self_->_parent._intf)->\
    delete_source_transform(self_,context_,source_,netmask_,ec_)

#define UDP_Transform_transform_source(\
       self_,context_,source_,in_packet_,out_packet_,ec_) \
((struct UDP_TransformI*)self_->_parent._intf)->\
        transform_source(self_,context_,source_,\
                              in_packet_,out_packet_,ec_)

#define UDP_Transform_transform_destination(\
       self_,context_,destination_,in_packet_,out_packet_,ec_) \
((struct UDP_TransformI*)self_->_parent._intf)->\
        transform_destination(self_,context_,destination_,\
                              in_packet_,out_packet_,ec_)
/*ce \dref_UDP_TransformRule
 */
struct UDP_TransformRule
{
    /*ce \dref_UDP_TransformRule_address
     */
    struct NETIO_Address address;

    /*ce \dref_UDP_TransformRule_netmask
     */
    struct NETIO_Netmask netmask;

    /*ce \dref_UDP_TransformRule_transformation
     */
    RT_ComponentFactoryId_T transformation;

    /*ce \dref_UDP_TransformRule_user_data
     */
    void *user_data;
};

#define T struct UDP_TransformRule
#define TSeq UDP_TransformRuleSeq
#include <reda/reda_sequence_decl.h>
#ifdef DOXYGEN_DOCUMENTATION_ONLY
/*ce \dref_UDP_TransformRuleSeq
 */
struct UDP_TransformRuleSeq {};
#endif

/*ce \dref_UDP_TransformRuleSeq_INITIALIZER
 */
#define UDP_TransformRuleSeq_INITIALIZER \
               REDA_DEFINE_SEQUENCE_INITIALIZER(struct UDP_TransformRule)

/*ce \dref_UDP_TransformRules_assert_source_rule
 */
MUST_CHECK_RETURN NETIOPSLDllExport RTI_BOOL
UDP_TransformRules_assert_source_rule(
                                 struct UDP_TransformRuleSeq *src_rules,
                                 RTI_UINT32 ipv4_address,
                                 RTI_UINT32 ipv4_netmask,
                                 const char *transform_name,
                                 void *user_data);

/*ce \dref_UDP_TransformRules_assert_destination_rule
 */
MUST_CHECK_RETURN NETIOPSLDllExport RTI_BOOL
UDP_TransformRules_assert_destination_rule(
                                 struct UDP_TransformRuleSeq *dst_rules,
                                 RTI_UINT32 ipv4_address,
                                 RTI_UINT32 ipv4_netmask,
                                 const char *transform_name,
                                 void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* UDP_TRANSFORMS_ENABLED */

#if UDP_TRANSFORMS_ENABLED

/*ce \dref_UDP_TransformUdpMode_T
 */
typedef enum
{
    /*ce \dref_UDP_TransformUdpMode_T_UDP_TRANSFORM_UDP_MODE_DISABLED
     */
    UDP_TRANSFORM_UDP_MODE_DISABLED,

    /*ce \dref_UDP_TransformUdpMode_T_UDP_TRANSFORM_UDP_MODE_ENABLED
     */
    UDP_TRANSFORM_UDP_MODE_ENABLED
} UDP_TransformUdpMode_T;

#endif

/*e \dref_UDP_InterfaceFactoryProperty
 */
struct DDSCPPDllExport UDP_InterfaceFactoryProperty
{
    /*ci
     * \brief Inherited property struct
     */
    struct NETIO_InterfaceFactoryProperty _parent;

    /*e \dref_UDP_InterfaceFactoryProperty_allow_interface
     */
    struct REDA_StringSeq allow_interface;

    /*e \dref_UDP_InterfaceFactoryProperty_deny_interface
     */
    struct REDA_StringSeq deny_interface;

    /*e \dref_UDP_InterfaceFactoryProperty_max_send_buffer_size
     */
    RTI_INT32 max_send_buffer_size;

    /*e \dref_UDP_InterfaceFactoryProperty_max_receive_buffer_size
     */
    RTI_INT32 max_receive_buffer_size;

    /*e \dref_UDP_InterfaceFactoryProperty_max_message_size
     */
    RTI_INT32 max_message_size;

    /*e \dref_UDP_InterfaceFactoryProperty_max_send_message_size
     */
    RTI_INT32 max_send_message_size;

    /*e \dref_UDP_InterfaceFactoryProperty_multicast_ttl
     */
    RTI_INT32 multicast_ttl;

#ifndef RTI_CERT
    /*e \dref_UDP_InterfaceFactoryProperty_nat
     */
    struct UDP_NatEntrySeq nat;
#endif /* !RTI_CERT */

    /*e \dref_UDP_InterfaceFactoryProperty_if_table
     */
    struct UDP_InterfaceTableEntrySeq if_table;

    /*e \dref_UDP_InterfaceFactoryProperty_multicast_interface
     */
    REDA_String_T multicast_interface;

    /*e \dref_UDP_InterfaceFactoryProperty_is_default_interface
     */
    RTI_BOOL is_default_interface;

    /*e \dref_UDP_InterfaceFactoryProperty_disable_auto_interface_config
     */
    RTI_BOOL disable_auto_interface_config;

    /*e \dref_UDP_InterfaceFactoryProperty_multicast_loopback_disabled
     */
    RTI_BOOL multicast_loopback_disabled;

    /*e \dref_UDP_InterfaceFactoryProperty_recv_thread
     */
    struct OSAPI_ThreadProperty recv_thread;

    /*e \dref_UDP_InterfaceFactoryProperty_enable_interface_bind
     */
    RTI_BOOL enable_interface_bind;

    /*e \dref_UDP_InterfaceFactoryProperty_disable_multicast_bind
     */
    RTI_BOOL disable_multicast_bind;

    /*e \dref_UDP_InterfaceFactoryProperty_disable_multicast_interface_select
     */
    RTI_BOOL disable_multicast_interface_select;

#if UDP_TRANSFORMS_ENABLED
    /*e \dref_UDP_InterfaceFactoryProperty_source_rules
     */
    struct UDP_TransformRuleSeq source_rules;

    /*e \dref_UDP_InterfaceFactoryProperty_destination_rules
     */
    struct UDP_TransformRuleSeq destination_rules;

    /*e \dref_UDP_InterfaceFactoryProperty_transform_udp_mode
     */
    UDP_TransformUdpMode_T transform_udp_mode;

    /*e \dref_UDP_InterfaceFactoryProperty_transform_locator_kind
     */
    RTI_INT32 transform_locator_kind;
#endif

    /*e \dref_UDP_InterfaceFactoryProperty_transport_priority_mapping_low
     */
    RTI_INT32 transport_priority_mapping_low;
    /*e \dref_UDP_InterfaceFactoryProperty_transport_priority_mapping_high
     */
    RTI_INT32 transport_priority_mapping_high;
    /*e \dref_UDP_InterfaceFactoryProperty_transport_priority_mask
     */
    RTI_UINT32 transport_priority_mask;
    /*e \dref_UDP_InterfaceFactoryProperty_max_unicast_send_sockets
     */
    RTI_UINT32 max_unicast_send_sockets;


#ifdef RTI_CPP
    public:
        UDP_InterfaceFactoryProperty()
        {
            UDP_InterfaceFactoryProperty_initialize(this);
        }

        ~UDP_InterfaceFactoryProperty()
        {
#ifndef RTI_CERT
            UDP_InterfaceFactoryProperty_finalize(this);
#endif /* !RTI_CERT */
        }
#endif /* RTI_CPP */
};

#ifdef __cplusplus
extern "C"
{
#endif

#if UDP_TRANSFORMS_ENABLED
#define UDP_TRANSFORMS_INITIALIZER \
        UDP_TransformRuleSeq_INITIALIZER,\
        UDP_TransformRuleSeq_INITIALIZER,\
        UDP_TRANSFORM_UDP_MODE_DISABLED,\
        NETIO_ADDRESS_KIND_TUDPv4,
#else
#define UDP_TRANSFORMS_INITIALIZER
#endif

#if OSAPI_THREAD_SEMAPHORE_ENABLED
#define UDP_THREAD_DEFAULT_OPTIONS OSAPI_THREAD_SUSPEND_ENABLE
#else
#define UDP_THREAD_DEFAULT_OPTIONS OSAPI_THREAD_DEFAULT_OPTIONS
#endif

/*ce \dref_UDP_THREAD_PROPERTY_DEFAULT
 */
#define UDP_THREAD_PROPERTY_DEFAULT \
{ \
    OSAPI_THREAD_USE_OSDEFAULT_STACKSIZE, \
    OSAPI_THREAD_PRIORITY_DEFAULT, \
    UDP_THREAD_DEFAULT_OPTIONS \
}

#ifdef RTI_QNX
#define RTI_SEND_SOCKET_BUFFER_SIZE (65535)
#define RTI_RECV_SOCKET_BUFFER_SIZE (65535)
#define RTI_SOCKET_BUFFER_SIZE_OS_DEFAULT (65535)
#define RTI_SOCKET_BUFFER_SIZE_OS_MAX (65535)
#else
#define RTI_SEND_SOCKET_BUFFER_SIZE (256*1024)
#define RTI_RECV_SOCKET_BUFFER_SIZE (256*1024)
#define RTI_SOCKET_BUFFER_SIZE_OS_DEFAULT (256*1024)
#define RTI_SOCKET_BUFFER_SIZE_OS_MAX (256*1024)
#endif

/*ce \dref_UDP_MAX_MESSAGE_SIZE_UNLIMITED
 */
#define UDP_MAX_MESSAGE_SIZE_UNLIMITED (-1)

#define UDP_MAX_MULTICAST_INTERFACES (16)

/*ce \dref_UDP_MAX_PACKET_SIZE
 */
#define UDP_MAX_PACKET_SIZE (65507)

#if UDP_TRANSFORMS_ENABLED
#define UDP_MAX_SEND_MESSAGE_SIZE_DEFAULT (UDP_MAX_MESSAGE_SIZE_UNLIMITED),
#else
#define UDP_MAX_SEND_MESSAGE_SIZE_DEFAULT (UDP_MAX_MESSAGE_SIZE_UNLIMITED),
#endif

#if UDP_INTERFACE_BIND_ENABLED
#define UDP_INTERFACE_BIND_DEFAULT ,RTI_FALSE
#else
#define UDP_INTERFACE_BIND_DEFAULT , RTI_FALSE
#endif

#if NETIO_CONFIG_HAVE_IFCONF || defined(RTI_WIN32)
#define NETIO_CONFIG_HAVE_IFCONF_DEFAULT RTI_FALSE,
#else
#define NETIO_CONFIG_HAVE_IFCONF_DEFAULT RTI_FALSE,
#endif

#define UDP_TRANSPORT_PRIORITY_MAPPING_LOW_DEFAULT 0
#define UDP_TRANSPORT_PRIORITY_MAPPING_HIGH_DEFAULT 0xff
#define UDP_TRANSPORT_PRIORITY_MASK_DEFAULT 0

#define UDP_TRANSPORT_PRIORITY_MAPPING_DEFAULT \
    UDP_TRANSPORT_PRIORITY_MAPPING_LOW_DEFAULT, /* transport_priority_mapping_low */ \
    UDP_TRANSPORT_PRIORITY_MAPPING_HIGH_DEFAULT, /* transport_priority_mapping_high */ \
    UDP_TRANSPORT_PRIORITY_MASK_DEFAULT /* transport_priority_mask */ \

/*ci \brief Default number of unicast send sockets */
#define UDP_MAX_UNICAST_SEND_SOCKETS_DEFAULT 1

/*ci
 * \dref_UDP_InterfaceFactoryProperty_INITIALIZER
 *
 * NOTE: Do not add a comma (,) after UDP_NAT_INITIALIZER, it is
 * part of the definition based on the RTI_CERT definition.
 */
#define UDP_InterfaceFactoryProperty_INITIALIZER \
{\
    NETIO_InterfaceFactoryProperty_INITIALIZER,\
    REDA_StringSeq_INITIALIZER,\
    REDA_StringSeq_INITIALIZER,\
    RTI_SEND_SOCKET_BUFFER_SIZE,\
    RTI_RECV_SOCKET_BUFFER_SIZE,\
    (8*1024),\
    UDP_MAX_SEND_MESSAGE_SIZE_DEFAULT \
    1,\
    UDP_NAT_INITIALIZER \
    UDP_InterfaceTableEntrySeq_INITIALIZER,\
    NULL,\
    RTI_TRUE,\
    NETIO_CONFIG_HAVE_IFCONF_DEFAULT \
    RTI_FALSE,\
    UDP_THREAD_PROPERTY_DEFAULT \
    UDP_INTERFACE_BIND_DEFAULT, \
    RTI_FALSE, \
    RTI_FALSE, \
    UDP_TRANSFORMS_INITIALIZER \
    UDP_TRANSPORT_PRIORITY_MAPPING_DEFAULT, \
    UDP_MAX_UNICAST_SEND_SOCKETS_DEFAULT /* max_unicast_send_sockets */\
}

#define UDP_INTERFACE_INTERFACE_ID RT_MKINTERFACEID(\
            RT_COMPONENT_CLASS_NETIO,RT_COMPONENT_INSTANCE_UDP)

/*ce \dref_UDP_INTERFACE_FACTORY_PROPERTY_DEFAULT
 */
NETIOPSLDllVariable extern
struct UDP_InterfaceFactoryProperty UDP_INTERFACE_FACTORY_PROPERTY_DEFAULT;

/*ce \dref_UDP_InterfaceFactory_get_interface
 */
MUST_CHECK_RETURN NETIOPSLDllExport struct RT_ComponentFactoryI*
UDP_InterfaceFactory_get_interface(void);

/*ce \dref_UDP_Interface_register
 */
MUST_CHECK_RETURN NETIOPSLDllExport RTI_BOOL
UDP_Interface_register(RT_Registry_T *registry,
                       const char *name,
                       struct UDP_InterfaceFactoryProperty *property);

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#undef RTI_EXPORT_REDA_SEQUENCE

#endif /* netio_udp_h */

/*ci @} */

