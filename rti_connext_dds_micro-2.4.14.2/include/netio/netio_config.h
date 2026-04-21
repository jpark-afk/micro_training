/*
 * FILE: netio_config.h - NETIO Configuration API
 *
 * Copyright 2012-2021 Real-Time Innovations, Inc.
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
 * MICRO-2679/PR.28247 Do not enable max_send_message_size and
 *                     enable_interface_bind for RTI_CERT
 * 22jan2014,eh MICRO-1027: disable multicast for RTI_CERT
 * 23feb2012,tk Written
 */
/*ce
 * \file
 */
/*ci
 * \defgroup NETIOConfig NETIO Configuration
 * \ingroup NETIOModule
 * \brief NETIO Configuration API
 *
 * \details
 *
 * This file is the single point where NETIO behavior can be configured.
 * All optional and configurable features shall be added here.
 */
/*ci \addtogroup NETIOConfig
 * @{
 */
#ifndef netio_config_h
#define netio_config_h

/* The global configuration is read first. Local definitions must
 * check if the global configuration has set an option or not. Note that
 * a module cannot disable a feature.
 */
#ifdef HAVE_GLOBAL_CONFIG
#include "rti_me_config.h"
#endif
  
/*ci
 * \def NETIO_CONFIG_INTERFACE
 * \brief Set NETIO_CONFIG_INTERFACE to the in-use interface address
 */  
#ifndef NETIO_CONFIG_INTERFACE
#define NETIO_CONFIG_INTERFACE "127.0.0.1"
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

#endif /* netio_config_h */

/*ci @} */
