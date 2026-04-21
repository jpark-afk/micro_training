/*
 * FILE: netio_log.h - NETIO Log definitions
 *
 * Copyright (c) 2012-2024 Real-Time Innovations, Inc.
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
 * 30may2013,eh MICRO-415: resource limit msgs
 * 09mar2012,tk Written
 */
/*e
 * \file
 * \brief NETIO module log codes
 */
#ifndef netio_log_h
#define netio_log_h

#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif

/******************************************************************************
 *                                     COMMON
 ******************************************************************************/

/*e
 * \defgroup NETIOLogCodesClass NETIO 
 * \brief Network I/O. ModuleID =  4
 * \ingroup LoggingModule
 */

/*e
 * \brief Failed to get host address from a string representation
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_GETHOST_BYNAME_EC                         (NETIO_LOG_BASE + 1)
#define NETIO_LOG_GETHOST_BYNAME(level_) \
OSAPI_LOG_ENTRY_ADD((level_),NETIO_LOG_GETHOST_BYNAME_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to allocate memory for a bind-resolver
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_BIND_NEW_EC                               (NETIO_LOG_BASE + 2)
#define NETIO_LOG_BIND_NEW(level_) \
OSAPI_LOG_ENTRY_ADD((level_),NETIO_LOG_BIND_NEW_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create a database table for routes of a bind-resolver
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_BIND_NEW_RTABLE_EC                        (NETIO_LOG_BASE + 3)
#define NETIO_LOG_BIND_NEW_RTABLE(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),NETIO_LOG_BIND_NEW_RTABLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to create route record. 
 *  
 * \details May have exceeded 
 *        DomainParticipantQos.resource_limits.max_receive_ports or
 *        DomainParticipantQos.resource_limits.local_writer_allocation
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_BIND_CREATE_RTABLE_EC                     (NETIO_LOG_BASE + 4)
#define NETIO_LOG_BIND_CREATE_RTABLE(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),NETIO_LOG_BIND_CREATE_RTABLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to delete a database table for routes of a bind_resolver
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_BIND_DELETE_RTABLE_EC                     (NETIO_LOG_BASE + 5)
#define NETIO_LOG_BIND_DELETE_RTABLE(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),NETIO_LOG_BIND_DELETE_RTABLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to set the length for a sequence of resolved addresses
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_BIND_SET_LENGTH_EC                        (NETIO_LOG_BASE + 6)
#define NETIO_LOG_BIND_SET_LENGTH(level_,len_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),NETIO_LOG_BIND_SET_LENGTH_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"len",(len_))

/*e
 * \brief Failed to set the maximum length of an internal sequence of addresses
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_BIND_SET_MAX_EC                           (NETIO_LOG_BASE + 7)
#define NETIO_LOG_BIND_SET_MAX(level_,len_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),NETIO_LOG_BIND_SET_MAX_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"len",(len_))

/*e
 * \brief An interface failed on bind_external
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_BIND_EXTERNAL_FAILED_EC                   (NETIO_LOG_BASE + 8)
#define NETIO_LOG_BIND_EXTERNAL_FAILED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),NETIO_LOG_BIND_EXTERNAL_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief An interface failed on unbind_external
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_UNBIND_EXTERNAL_FAILED_EC                (NETIO_LOG_BASE + 10)
#define NETIO_LOG_UNBIND_EXTERNAL_FAILED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),NETIO_LOG_UNBIND_EXTERNAL_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to allocate memory for a route resolver
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_ROUTE_RTABLE_ALLOC_EC                    (NETIO_LOG_BASE + 11)
#define NETIO_LOG_ROUTE_RTABLE_ALLOC(level_) \
OSAPI_LOG_ENTRY_ADD((level_),NETIO_LOG_ROUTE_RTABLE_ALLOC_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create a database table for routes of a route-resolver
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_ROUTE_RTABLE_CREATE_EC                   (NETIO_LOG_BASE + 12)
#define NETIO_LOG_ROUTE_RTABLE_CREATE(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),NETIO_LOG_ROUTE_RTABLE_CREATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to delete a database table for routes of a route-resolver
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_ROUTE_RTABLE_DELETE_EC                   (NETIO_LOG_BASE + 13)
#define NETIO_LOG_ROUTE_RTABLE_DELETE(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),NETIO_LOG_ROUTE_RTABLE_DELETE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to create route resolver record. 
 *  
 * \details May have exceeded 
 *        DomainParticipantQos.resource_limits.max_destination_ports 
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_ROUTE_RTABLE_ADD_EC                      (NETIO_LOG_BASE + 14)
#define NETIO_LOG_ROUTE_RTABLE_ADD(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),NETIO_LOG_ROUTE_RTABLE_ADD_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to find routes to update for a route-resolver 
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_ROUTE_RTABLE_UPDATE_EC                   (NETIO_LOG_BASE + 15)
#define NETIO_LOG_ROUTE_RTABLE_UPDATE(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),NETIO_LOG_ROUTE_RTABLE_UPDATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to allocate memory for address resolver
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_AR_ALLOC_FAILED_EC                       (NETIO_LOG_BASE + 16)
#define NETIO_LOG_AR_ALLOC_FAILED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),NETIO_LOG_AR_ALLOC_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief An attempt was made to add an existing interface with a different
 *        port resolver to the address resolver
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_AR_ADD_INVALID_INTERFACE_EC              (NETIO_LOG_BASE + 17)
#define NETIO_LOG_AR_ADD_INVALID_INTERFACE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),NETIO_LOG_AR_ADD_INVALID_INTERFACE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief An interface failed to resolve an address.
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_AR_ADDRESS_RESOLVE_FAILED_EC             (NETIO_LOG_BASE + 18)
#define NETIO_LOG_AR_ADDRESS_RESOLVE_FAILED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),NETIO_LOG_AR_ADDRESS_RESOLVE_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief An interface failed to resolve a port.
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_AR_PORT_RESOLVE_FAILED_EC                (NETIO_LOG_BASE + 19)
#define NETIO_LOG_AR_PORT_RESOLVE_FAILED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),NETIO_LOG_AR_PORT_RESOLVE_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief NETIO_AddressResolver_resolve/get_next was called with an
 *        uninitialized context
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_AR_CONTEXT_UNINITIALIZED_EC              (NETIO_LOG_BASE + 21)
#define NETIO_LOG_AR_CONTEXT_UNINITIALIZED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),NETIO_LOG_AR_CONTEXT_UNINITIALIZED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief An invalid token was encountered in the address string
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_AR_INVALID_TOKEN_EC                      (NETIO_LOG_BASE + 22)
#define NETIO_LOG_AR_INVALID_TOKEN(level_) \
OSAPI_LOG_ENTRY_ADD((level_),NETIO_LOG_AR_INVALID_TOKEN_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief The length of the address exceeded the address buffer passed in
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_AR_ADDRESS_STRING_EXCEEDED_EC            (NETIO_LOG_BASE + 23)
#define NETIO_LOG_AR_ADDRESS_STRING_EXCEEDED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),NETIO_LOG_AR_ADDRESS_STRING_EXCEEDED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief An address index was not parsed correctly
 *
 * \details
 * When parsing an address string the index was not specified correctly.
 * Legal formats are:
 * N@
 * [,N]@
 * [N]@
 * [M,N]@
 *
 * Where N and M are integers >= 0
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_INVALID_ADDRESS_INDEX_EC                 (NETIO_LOG_BASE + 24)
#define NETIO_LOG_INVALID_ADDRESS_INDEX(level_,low_,high_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),NETIO_LOG_INVALID_ADDRESS_INDEX_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"low",(low_),"high",(high_))

/*e
 * \brief Illegal route operation attempted
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_ILLEGAL_ROUTE_OPERATION_EC               (NETIO_LOG_BASE + 25)
#define NETIO_LOG_ILLEGAL_ROUTE_OPERATION(level_,op_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),NETIO_LOG_ILLEGAL_ROUTE_OPERATION_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"op",(op_))

/*e
 * \brief Failed to set the name of a runtime component interface
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_SET_NAME_EC                                (UDP_LOG_BASE + 26)
#define NETIO_LOG_SET_NAME(level_,name_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),NETIO_LOG_SET_NAME_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"name",(name_))
/*e
 * \brief Failed to set packet's head
 *
 * \details NETIO_Packet_set_head() failed, due to an invalid delta that would
 * have moved the head position out-of-bounds of the packet.
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_PACKET_SET_HEAD_EC                         (UDP_LOG_BASE + 27)
#define NETIO_LOG_PACKET_SET_HEAD(level_,delta_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),NETIO_LOG_PACKET_SET_HEAD_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"delta",(delta_))

/*e
 * \brief Failed to set packet's tail
 *
 * \details NETIO_Packet_set_tail() failed, due to an invalid delta that would
 * have moved the tail position out-of-bounds of the packet.
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_PACKET_SET_TAIL_EC                         (UDP_LOG_BASE + 28)
#define NETIO_LOG_PACKET_SET_TAIL(level_) \
OSAPI_LOG_ENTRY_ADD((level_),NETIO_LOG_PACKET_SET_TAIL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to initialize packet
 *
 * \details NETIO_Packet_initialize() failed due to invalid arguments
 *
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_PACKET_INITIALIZE_EC                       (UDP_LOG_BASE + 29)
#define NETIO_LOG_PACKET_INITIALIZE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),NETIO_LOG_PACKET_INITIALIZE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/******************************************************************************
 *                                LOOPBACK
 ******************************************************************************/

/*e
 * \brief Failed to delete a database table for a NETIO loopback interface
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_LOOP_DELETE_TABLE_EC                    (NETIO_LOG_BASE + 100)
#define NETIO_LOG_LOOP_DELETE_TABLE(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),NETIO_LOG_LOOP_DELETE_TABLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to create a database table for a NETIO loopback interface
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_LOOP_CREATE_TABLE_EC                    (NETIO_LOG_BASE + 101)
#define NETIO_LOG_LOOP_CREATE_TABLE(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),NETIO_LOG_LOOP_CREATE_TABLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to select a valid record when sending with the NETIO loopback 
 *        interface
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_LOOP_SELECT_TABLE_EC                    (NETIO_LOG_BASE + 102)
#define NETIO_LOG_LOOP_SELECT_TABLE(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),NETIO_LOG_LOOP_SELECT_TABLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to send forward with the NETIO loopback interface
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_LOOP_FWD_EC                             (NETIO_LOG_BASE + 103)
#define NETIO_LOG_LOOP_FWD(level_) \
OSAPI_LOG_ENTRY_ADD((level_),NETIO_LOG_LOOP_FWD_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to bind an external interface with the NETIO loopback interface
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_LOOP_BINDX_DB_EC                        (NETIO_LOG_BASE + 104)
#define NETIO_LOG_LOOP_BINDX_DB(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),NETIO_LOG_LOOP_BINDX_DB_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to unbind an external interface with the NETIO loopback 
 * interface
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_LOOP_UNBINDX_DB_EC                      (NETIO_LOG_BASE + 105)
#define NETIO_LOG_LOOP_UNBINDX_DB(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),NETIO_LOG_LOOP_UNBINDX_DB_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to set the length of an address sequence
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_LOOP_SET_LENGTH_EC                      (NETIO_LOG_BASE + 106)
#define NETIO_LOG_LOOP_SET_LENGTH(level_,len_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),NETIO_LOG_LOOP_SET_LENGTH_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"len",(len_))

/*e
 * \brief Invalid property passed in when creating a loopback interface
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_LOOP_INVALID_PROPERTY_EC                (NETIO_LOG_BASE + 107)
#define NETIO_LOG_LOOP_INVALID_PROPERTY(level_) \
OSAPI_LOG_ENTRY_ADD((level_),NETIO_LOG_LOOP_INVALID_PROPERTY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to initialize the loopback interface
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_LOOP_INITIALIZE_FAILED_EC               (NETIO_LOG_BASE + 108)
#define NETIO_LOG_LOOP_INITIALIZE_FAILED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),NETIO_LOG_LOOP_INITIALIZE_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief An error occurred with a cursor while iterating over a table
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_LOOP_CURSOR_ERROR_EC                    (NETIO_LOG_BASE + 109)
#define NETIO_LOG_LOOP_CURSOR_ERROR(level_) \
OSAPI_LOG_ENTRY_ADD((level_),NETIO_LOG_LOOP_CURSOR_ERROR_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief An invalid factory was passed in to create a loopback interface
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_LOOP_INVALID_FACTORY_EC                 (NETIO_LOG_BASE + 110)
#define NETIO_LOG_LOOP_INVALID_FACTORY(level_) \
OSAPI_LOG_ENTRY_ADD((level_),NETIO_LOG_LOOP_INVALID_FACTORY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)


/*e
 * \brief An invalid component was passed in to delete a loopback interface
 * \ingroup NETIOLogCodesClass
 */
#define NETIO_LOG_LOOP_INVALID_COMPONENT_EC               (NETIO_LOG_BASE + 111)
#define NETIO_LOG_LOOP_INVALID_COMPONENT(level_) \
OSAPI_LOG_ENTRY_ADD((level_),NETIO_LOG_LOOP_INVALID_COMPONENT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/******************************************************************************
 *                                  UDP
 ******************************************************************************/

/*e
 * \brief Failed to initialize use of sockets
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_SOCKET_INIT_EC                              (UDP_LOG_BASE + 200)
#define UDP_LOG_SOCKET_INIT(level_,sysrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),UDP_LOG_SOCKET_INIT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"sysrc",(sysrc_))

/*e
 * \brief Failed to get the host address given the host name
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_GETHOSTNAME_EC                              (UDP_LOG_BASE + 201)
#define UDP_LOG_GETHOSTNAME(level_,host_,sysrc_) \
OSAPI_LOG_ENTRY_ADD_1STRING_1INT((level_),UDP_LOG_GETHOSTNAME_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "host",(host_),"sysrc",(sysrc_))

/*e
 * \brief Failed to set a socket for multicast loopback
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_SOCKET_SET_MCAST_EC                         (UDP_LOG_BASE + 202)
#define UDP_LOG_SOCKET_SET_MCAST(level_,sysrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),UDP_LOG_SOCKET_SET_MCAST_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"sysrc",(sysrc_))

/*e
 * \brief Failed to set a socket for multicast bound to a specific interface
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_SOCKET_SET_MCASTIF_EC                       (UDP_LOG_BASE + 203)
#define UDP_LOG_SOCKET_SET_MCASTIF(level_,sysrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),UDP_LOG_SOCKET_SET_MCASTIF_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"sysrc",(sysrc_))

/*e
 * \brief Failed to create a new socket, the sysrc code is the OS return
 *        code and reason for failure.
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_SOCKET_CREATE_EC                            (UDP_LOG_BASE + 204)
#define UDP_LOG_SOCKET_CREATE(level_,sysrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),UDP_LOG_SOCKET_CREATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"sysrc",(sysrc_))

/*e
 * \brief Failed to set the close-on-exec flag for a socket
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_SOCKET_SETFD_EC                             (UDP_LOG_BASE + 205)
#define UDP_LOG_SOCKET_SETFD(level_,sysrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),UDP_LOG_SOCKET_SETFD_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"sysrc",(sysrc_))

/*e
 * \brief Failed to set the socket send buffer size
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_SOCKET_SNDBUF_EC                            (UDP_LOG_BASE + 206)
#define UDP_LOG_SOCKET_SNDBUF(level_,size_,sysrc_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),UDP_LOG_SOCKET_SNDBUF_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "size",(size_),"sysrc",(sysrc_))

/*e
 * \brief Failed to set multicast TTL for a socket
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_SOCKET_TTL_EC                               (UDP_LOG_BASE + 207)
#define UDP_LOG_SOCKET_TTL(level_,ttl_,sysrc_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),UDP_LOG_SOCKET_TTL_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
        "ttl",(ttl_),"sysrc",(sysrc_))

/*e
 * \brief Failed to initialize a receive packet
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_PACKET_INIT_EC                              (UDP_LOG_BASE + 208)
#define UDP_LOG_PACKET_INIT(level_,packet_,buf_,len_) \
OSAPI_LOG_ENTRY_CREATE((level_),UDP_LOG_PACKET_INIT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_POINTER("pkt",(packet_),RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_POINTER("buf",(buf_),RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_INT("len",(len_),RTI_TRUE)

/*e
 * \brief Failed to set the head of a receive packet
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_PACKET_HEAD_EC                              (UDP_LOG_BASE + 209)
#define UDP_LOG_PACKET_HEAD(level_,packet_,adjust_) \
OSAPI_LOG_ENTRY_CREATE((level_),UDP_LOG_PACKET_HEAD_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_POINTER("packet",(packet_),RTI_FALSE)\
        OSAPI_LOG_ENTRY_ADD_INT("adjust",(adjust_),RTI_TRUE)

/*e
 * \brief Failed reception upstream for a received packet
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_PACKET_FWD_EC                               (UDP_LOG_BASE + 210)
#define UDP_LOG_PACKET_FWD(level_) \
OSAPI_LOG_ENTRY_ADD((level_),UDP_LOG_PACKET_FWD_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to delete a database record or index from an internal NAT 
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_NAT_DELETE_EC                               (UDP_LOG_BASE + 211)
#define UDP_LOG_NAT_DELETE(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),UDP_LOG_NAT_DELETE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to finalize the parent NETIO interface
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_FINALIZE_EC                                 (UDP_LOG_BASE + 212)
#define UDP_LOG_FINALIZE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),UDP_LOG_FINALIZE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to set socket to reuse a port or address
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_SOCKET_REUSE_PORT_EC                        (UDP_LOG_BASE + 213)
#define UDP_LOG_SOCKET_REUSE_PORT(level_,sysrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),UDP_LOG_SOCKET_REUSE_PORT_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"sysrc",(sysrc_))

/*e
 * \brief Failed a socket bind
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_SOCKET_BIND_EC                              (UDP_LOG_BASE + 214)
#define UDP_LOG_SOCKET_BIND(level_,addr_,port_,sysrc_) \
OSAPI_LOG_ENTRY_ADD_3INT((level_),UDP_LOG_SOCKET_BIND_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "addr",(RTI_INT32)(addr_),"port",(RTI_INT32)(port_),"sysrc",(RTI_INT32)(sysrc_))

/*e
 * \brief Failed to add membership to a multicast group for a socket
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_SOCKET_ADDGRP_EC                            (UDP_LOG_BASE + 215)
#define UDP_LOG_SOCKET_ADDGRP(level_,if_,addr_,sysrc_) \
OSAPI_LOG_ENTRY_ADD_3INT((level_),UDP_LOG_SOCKET_ADDGRP_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "if",(RTI_INT32)(if_),"addr",(RTI_INT32)(addr_),"sysrc",(RTI_INT32)(sysrc_))

/*e
 * \brief Failed to create a bind entry for a receive port
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_PORT_ENTRY_EC                               (UDP_LOG_BASE + 216)
#define UDP_LOG_PORT_ENTRY(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),UDP_LOG_PORT_ENTRY_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to allocate memory for a receive buffer of a specific port  
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_PORT_ALLOC_EC                               (UDP_LOG_BASE + 217)
#define UDP_LOG_PORT_ALLOC(level_) \
OSAPI_LOG_ENTRY_ADD((level_),UDP_LOG_PORT_ALLOC_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to set the receive buffer size of a socket
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_SOCKET_RXBUF_EC                             (UDP_LOG_BASE + 218)
#define UDP_LOG_SOCKET_RXBUF(level_,size_,sysrc_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),UDP_LOG_SOCKET_RXBUF_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "size",(size_),"sysrc",(sysrc_))

/*e
 * \brief Failed to add a port entry record to a database table
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_PORT_ADD_EC                                 (UDP_LOG_BASE + 219)
#define UDP_LOG_PORT_ADD(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),UDP_LOG_PORT_ADD_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to create a receive thread for a receive port
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_PORT_THREAD_EC                              (UDP_LOG_BASE + 220)
#define UDP_LOG_PORT_THREAD(level_) \
OSAPI_LOG_ENTRY_ADD((level_),UDP_LOG_PORT_THREAD_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to get a list of interface addresses 
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_SOCKET_IFLIST_EC                            (UDP_LOG_BASE + 221)
#define UDP_LOG_SOCKET_IFLIST(level_,sysrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),UDP_LOG_SOCKET_IFLIST_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"sysrc",(sysrc_))

/*e
 * \brief Failed to get the network interface flags or mask of a socket 
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_SOCKET_IFFLAGS_EC                           (UDP_LOG_BASE + 222)
#define UDP_LOG_SOCKET_IFFLAGS(level_,sysrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),UDP_LOG_SOCKET_IFFLAGS_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"sysrc",(sysrc_))

/*e
 * \brief Failed to close a socket
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_SOCKET_CLOSE_EC                             (UDP_LOG_BASE + 223)
#define UDP_LOG_SOCKET_CLOSE(level_,sysrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),UDP_LOG_SOCKET_CLOSE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"sysrc",(sysrc_))

/*e
 * \brief Failed to load a dynamic library
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_LOADLIB_EC                                  (UDP_LOG_BASE + 224)
#define UDP_LOG_LOADLIB(level_) \
OSAPI_LOG_ENTRY_ADD((level_),UDP_LOG_LOADLIB_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to get interface buffer size
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_GET_BUF_SIZE_EC                             (UDP_LOG_BASE + 225)
#define UDP_LOG_GET_BUF_SIZE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),UDP_LOG_GET_BUF_SIZE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Out of memory to allocate interface buffer
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_GET_BUF_ALLOC_EC                            (UDP_LOG_BASE + 226)
#define UDP_LOG_GET_BUF_ALLOC(level_,size_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),UDP_LOG_GET_BUF_ALLOC_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"size",(size_))

/*e
 * \brief Failed to get an interface list
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_GET_IFLIST_EC                               (UDP_LOG_BASE + 227)
#define UDP_LOG_GET_IFLIST(level_,sysrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),UDP_LOG_GET_IFLIST_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"sysrc",(sysrc_))

/*e
 * \brief Failed getnameinfo()
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_GET_NAMEINFO_EC                             (UDP_LOG_BASE + 228)
#define UDP_LOG_GET_NAMEINFO(level_,sysrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),UDP_LOG_GET_NAMEINFO_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"sysrc",(sysrc_))

/*e
 * \brief Failed to enable multicast loopback
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_MULTICAST_ENABLE_EC                         (UDP_LOG_BASE + 229)
#define UDP_LOG_MULTICAST_ENABLE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),UDP_LOG_MULTICAST_ENABLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to create a database table for a UDP interface
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_UDP_CREATE_TABLE_EC                         (UDP_LOG_BASE + 230)
#define UDP_LOG_UDP_CREATE_TABLE(level_,name_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1STRING_1INT((level_),UDP_LOG_UDP_CREATE_TABLE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,\
        "name",(name_),"dbrc",(dbrc_))

/*e
 * \brief Failed to create an index for a UDP interface
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_UDP_CREATE_INDEX_EC                         (UDP_LOG_BASE + 231)
#define UDP_LOG_UDP_CREATE_INDEX(level_,table_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1STRING_1INT((level_),UDP_LOG_UDP_CREATE_INDEX_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
        "table",(table_),"dbrc",(dbrc_))

/*e
 * \brief Failed to create or insert a database record for a UDP interface
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_RECORD_EC                                   (UDP_LOG_BASE + 232)
#define UDP_LOG_RECORD(level_,dbrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),UDP_LOG_RECORD_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"dbrc",(dbrc_))

/*e
 * \brief Failed to finalize UDP interface factory property
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_PROPERTY_FINALIZE_EC                        (UDP_LOG_BASE + 233)
#define UDP_LOG_PROPERTY_FINALIZE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),UDP_LOG_PROPERTY_FINALIZE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to allocate memory for a UDP interface
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_ALLOC_EC                                    (UDP_LOG_BASE + 234)
#define UDP_LOG_ALLOC(level_) \
OSAPI_LOG_ENTRY_ADD((level_),UDP_LOG_ALLOC_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed sending a ping message, upon adding a route or waking up a 
 * receive thread 
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_SEND_PING_EC                                (UDP_LOG_BASE + 235)
#define UDP_LOG_SEND_PING(level_) \
OSAPI_LOG_ENTRY_ADD((level_),UDP_LOG_SEND_PING_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to drop membership from a multicast group
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_DROPGRP_EC                                  (UDP_LOG_BASE + 236)
#define UDP_LOG_DROPGRP(level_,sysrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),UDP_LOG_DROPGRP_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"sysrc",(sysrc_))

/*e
 * \brief Length of address sequence exceeds its maximum 
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_GET_LENGTH_EC                               (UDP_LOG_BASE + 237)
#define UDP_LOG_GET_LENGTH(level_) \
OSAPI_LOG_ENTRY_ADD((level_),UDP_LOG_GET_LENGTH_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to set the length of an address sequence
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_SET_LENGTH_EC                               (UDP_LOG_BASE + 240)
#define UDP_LOG_SET_LENGTH(level_,len_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),UDP_LOG_SET_LENGTH_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"len",(len_))

/*e
 * \brief Failed WSAStartup()
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_WSA_STARTUP_EC                              (UDP_LOG_BASE + 241)
#define UDP_LOG_WSA_STARTUP(level_,rc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),UDP_LOG_WSA_STARTUP_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"sysrc",(rc_))

/*e
 * \brief Incompatible version of Winsock, must not be older than 2.0
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_WINSOCK_INCOMPATIBLE_EC                     (UDP_LOG_BASE + 242)
#define UDP_LOG_WINSOCK_INCOMPATIBLE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),UDP_LOG_WSA_STARTUP_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Inconsistent max_message_size 
 *  
 * \details UDP_InterfaceProperty.max_message_size is set to an inconsistent 
 * value, larger than UDP_InterfaceProperty.max_send_buffer_size and/or 
 * larger than UDP_InterfaceProperty.max_receive_buffer_size .
 *  
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_INCONSISTENT_MAX_MESSSAGE_SIZE_EC           (UDP_LOG_BASE + 243)
#define UDP_LOG_INCONSISTENT_MAX_MESSAGE_SIZE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),UDP_LOG_INCONSISTENT_MAX_MESSSAGE_SIZE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to initialized the loopback interface
 *
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_INITIALIZE_FAILED_EC                        (UDP_LOG_BASE + 244)
#define UDP_LOG_INITIALIZE_FAILED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),UDP_LOG_INITIALIZE_FAILED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Did not find the specified thread port
 *
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_PORT_NOT_FOUND_EC                           (UDP_LOG_BASE + 245)
#define UDP_LOG_PORT_NOT_FOUND(level_) \
OSAPI_LOG_ENTRY_ADD((level_),UDP_LOG_PORT_NOT_FOUND_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief An error occurred while iterating over a cursor
 *
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_CURSOR_ERROR_EC                             (UDP_LOG_BASE + 246)
#define UDP_LOG_CURSOR_ERROR(level_) \
OSAPI_LOG_ENTRY_ADD((level_),UDP_LOG_CURSOR_ERROR_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to send message
 *
 * \details A message could not be sent due to system error
 *
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_SEND_ERROR_EC                               (UDP_LOG_BASE + 247)
#define UDP_LOG_SEND_ERROR(level_,sysrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),UDP_LOG_SEND_ERROR_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"sysrc",(sysrc_))

/*e
 * \brief Multicast address ignored because multicast supported is not compiled in
 *
 * \details
 * A multicast address was specified as an address, but multicast support has
 * not been enabled with NETIO_CONFIG_ENABLE_MULTICAST in netio_config.h
 *
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_MULTICAST_NOT_ENABLED_EC                    (UDP_LOG_BASE + 248)
#define UDP_LOG_MULTICAST_NOT_ENABLED(level_) \
OSAPI_LOG_ENTRY_ADD((level_),UDP_LOG_MULTICAST_NOT_ENABLED_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Transport does not have any interfaces.
 *
 * \details UDP configuration might be wrong.
 *
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_TRANSPORT_NO_INTERFACES_EC                  (UDP_LOG_BASE + 249)
#define UDP_LOG_TRANSPORT_NO_INTERFACES(level_) \
OSAPI_LOG_ENTRY_ADD((level_),UDP_LOG_TRANSPORT_NO_INTERFACES_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Failed to receive message
 *
 * \details A message could not be received due to system error
 *
 * \ingroup NETIOLogCodesClass
 */
#define UDP_LOG_RECV_ERROR_EC                               (UDP_LOG_BASE + 250)
#define UDP_LOG_RECV_ERROR(level_,sysrc_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),UDP_LOG_RECV_ERROR_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM ,"sysrc",(sysrc_))

/*e
 * \brief Rule object
 */
#define UDP_TRANSFORM_LOG_KIND_RULE                                          (1)

/*e
 * \brief Entry object
 */
#define UDP_TRANSFORM_LOG_KIND_ENTRY                                         (2)

/*e
 * \brief Factory object
 */
#define UDP_TRANSFORM_LOG_KIND_FACTORY                                       (3)

/*e
 * \brief Factory object
 */
#define UDP_TRANSFORM_LOG_KIND_FACTORY_INDEX                                 (4)

/*e
 * \brief Factory object
 */
#define UDP_TRANSFORM_LOG_KIND_DST_INDEX                                     (5)

/*e
 * \brief Factory object
 */
#define UDP_TRANSFORM_LOG_KIND_SRC_INDEX                                     (6)

/*e
 * \brief Rule object
 */
#define UDP_TRANSFORM_LOG_KIND_TRANSFORM_SOURCE                              (7)

/*e
 * \brief Entry object
 */
#define UDP_TRANSFORM_LOG_KIND_TRANSFORM_DESTINATION                         (8)

/*e
 * \brief Mask object
 */
#define UDP_TRANSFORM_LOG_KIND_TRANSFORM_NETMASK_INDEX                       (9)

/*e
 * \brief The returned factory reference is NULL.
 *
 * \details
 * This error indicates that the system has run out of memory
 *
 * \ingroup NETIOLogCodesClass
 */
#define UDP_TRANSFORM_LOG_INVALID_FACTORY_REFERENCE_EC      (UDP_LOG_BASE + 300)
#define UDP_TRANSFORM_LOG_INVALID_FACTORY_REFERENCE(level_) \
OSAPI_LOG_ENTRY_ADD((level_),UDP_TRANSFORM_LOG_INVALID_FACTORY_REFERENCE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM)

/*e
 * \brief Could not find the factory used as part of a transform rule
 *
 * \details
 * This error indicates that a factory for the specified rule has not been
 * registered before the UDP transport is registered.
 *
 * \ingroup NETIOLogCodesClass
 */
#define UDP_TRANSFORM_LOG_FACTORY_NOT_FOUND_EC              (UDP_LOG_BASE + 301)
#define UDP_TRANSFORM_LOG_FACTORY_NOT_FOUND(level_,factory_) \
OSAPI_LOG_ENTRY_ADD_1STRING((level_),UDP_TRANSFORM_LOG_FACTORY_NOT_FOUND_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
        "name",(factory_))

/*e
 * \brief Failed to allocate entry of the specified kind
 *
 * \ingroup NETIOLogCodesClass
 */
#define UDP_TRANSFORM_LOG_ALLOC_EC                          (UDP_LOG_BASE + 302)
#define UDP_TRANSFORM_LOG_ALLOC(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),UDP_TRANSFORM_LOG_ALLOC_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
        "kind",(kind_))

/*e
 * \brief Failed to create a transformation of the specified kind
 *
 * \ingroup NETIOLogCodesClass
 */
#define UDP_TRANSFORM_LOG_CREATE_EC                         (UDP_LOG_BASE + 303)
#define UDP_TRANSFORM_LOG_CREATE(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),UDP_TRANSFORM_LOG_CREATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
        "kind",(kind_))

/*e
 * \brief Failed to add object of specified kind
 *
 * \ingroup NETIOLogCodesClass
 */
#define UDP_TRANSFORM_LOG_ADD_EC                            (UDP_LOG_BASE + 304)
#define UDP_TRANSFORM_LOG_ADD(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),UDP_TRANSFORM_LOG_ADD_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
        "kind",(kind_))

/*e
 * \brief Failed to find a transformation
 *
 * \ingroup NETIOLogCodesClass
 */
#define UDP_TRANSFORM_LOG_FIND_EC                           (UDP_LOG_BASE + 305)
#define UDP_TRANSFORM_LOG_FIND(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),UDP_TRANSFORM_LOG_FIND_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
        "kind",(kind_))

/*e
 * \brief Duplicate object
 *
 * \ingroup NETIOLogCodesClass
 */
#define UDP_TRANSFORM_LOG_DUPLICATE_EC                      (UDP_LOG_BASE + 306)
#define UDP_TRANSFORM_LOG_DUPLICATE(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),UDP_TRANSFORM_LOG_DUPLICATE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
        "kind",(kind_))

/*e
 * \brief Source or destination transformation function failed
 *
 * \ingroup NETIOLogCodesClass
 */
#define UDP_TRANSFORM_LOG_TRANSFORM_EC                      (UDP_LOG_BASE + 307)
#define UDP_TRANSFORM_LOG_TRANSFORM(level_,kind_,ec_) \
OSAPI_LOG_ENTRY_ADD_2INT((level_),UDP_TRANSFORM_LOG_TRANSFORM_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
        "kind",(kind_),"error",(ec_))

/*e
 * \brief Invalid UDP mode specified
 *
 * \ingroup NETIOLogCodesClass
 */
#define UDP_TRANSFORM_LOG_INVALID_UDP_MODE_EC               (UDP_LOG_BASE + 308)
#define UDP_TRANSFORM_LOG_INVALID_UDP_MODE(level_,mode_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),UDP_TRANSFORM_LOG_INVALID_UDP_MODE_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
        "mode",(mode_))

/*e
 * \brief Invalid transformation UDP locator kind specified. 
 *
 * \details \idref_UDP_InterfaceFactoryProperty_transform_locator_kind
 * must be greater or equal than NETIO_ADDRESS_KIND_USER and not
 * equal to NETIO_ADDRESS_KIND_SHMEM or NETIO_ADDRESS_KIND_INTRA.
 *
 * \ingroup NETIOLogCodesClass
 */
#define UDP_TRANSFORM_LOG_INVALID_TUDP_LOCATOR_KIND_EC        (UDP_LOG_BASE + 309)
#define UDP_TRANSFORM_LOG_INVALID_TUDP_LOCATOR_KIND(level_,kind_) \
OSAPI_LOG_ENTRY_ADD_1INT((level_),UDP_TRANSFORM_LOG_INVALID_TUDP_LOCATOR_KIND_EC,\
        OSAPI_LOG_MSG_PN_X2_STD_PARAM,\
        "kind",(kind_))

#endif /* netio_log_h */
