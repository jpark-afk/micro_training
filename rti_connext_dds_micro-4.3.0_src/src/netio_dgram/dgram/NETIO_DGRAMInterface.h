/*
 * FILE: NETIO_DGRAM_Interface.h - NETIO_DGRAM_Interface interface
 *
 * Copyright (c) 2022-2024 Real-Time Innovations, Inc.
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
 * 17jul2023,tk MICRO-5364/PR.32033
 * - Added macro NETIO_DGRAM_InterfaceEntry_resolve_address
 * 27may2023,tk MICRO-4824/PR.31538
 * - Added is_req_shared_shared instead of using a bit in the locator
 *   kind to avoid overlap with standard ranges.
 * MICRO-4890/PR.31651 Address interface changes
 * - Added inclusion of netio_route.h
 * 24apr2023,ad MICRO-4815/PR.31519
 * - Removed UDP external constants as they are no longer needed.
 * 26oct2022,tk Written
 */
#ifndef NETIO_DGRAMInterface_h
#define NETIO_DGRAMInterface_h

#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef osapi_time_h
#include "osapi/osapi_time.h"
#endif
#ifndef osapi_string_h
#include "osapi/osapi_string.h"
#endif
#ifndef osapi_log_h
#include "osapi/osapi_log.h"
#endif
#ifndef reda_buffer_h
#include "reda/reda_buffer.h"
#endif
#ifndef reda_indexer_h
#include "reda/reda_indexer.h"
#endif
#ifndef reda_sequenceNumber_h
#include "reda/reda_sequenceNumber.h"
#endif
#ifndef rt_rt_h
#include "rt/rt_rt.h"
#endif
#include "netio/netio_common.h"
#include "netio/netio_route.h"
#include "netio/netio_interface.h"
#include "netio/netio_log.h"

#include  "netio_dgram/netio_dgram.h"

/*i \brief Instance data for the NETIO_DGRAM Interface
 */
struct NETIO_DGRAM_Interface
{
    /*i
     * \brief Inherited NETIO interface base-class
     */
    struct NETIO_Interface _parent;

    /*i
     * \brief The properties the NETIO_DGRAM interface was created with
     */
    struct NETIO_InterfaceProperty property;

    /*i
     * \brief The factory that created the DGRAM interface interface
     */
    struct NETIO_DGRAM_InterfaceFactory *factory;

    /*i
     * \brief Table with DGRAM port entries, once for each unique port listened to
     */
    DB_Table_T rx_port_table;
    
    /*i
     * \brief pointer to downstream interface integrating with a network stack
     */
    struct NETIO_Interface *user_netio;
    
    /*ci \brief This is a copy of the user interface pointers, used by user_netio
     */
    struct NETIO_InterfaceI _user_netio_intf;

    /*ci
     * \brief List of interfaces from the underlying transport.
     */
    struct NETIO_DGRAM_InterfaceTableEntrySeq if_table;

    /*ci
     * \brief List of requested addressed passed to the underlying transport.
     */
    struct NETIO_AddressSeq req_addr;

    /*ci
     * \brief bitmap to keep track of if an address is shared or not, where bit
     * 0 is index 0 in req_addr, bit 1 index 1 in reg_addr etc.
     */
     RTI_UINT32 is_req_addr_shared;
};

/*ci
 * \brief Implementation of the NETIO_DGRAM Interface factory derived from the
 *        \ref RT_ComponentFactory
 */
struct NETIO_DGRAM_InterfaceFactory
{
    /*ci
     * \brief Base-class
     */
    struct RT_ComponentFactory _parent;

    /*ci
     * \brief Counter used to differentiate between different DGRAM interfaces
     *        created from the same factory.
     */
    RTI_INT32 instance_counter;

    /*ci
     * \brief Copy of the downstream transport interface structure.
     */
    NETIO_DGRAM_InterfaceI user_intf;

    /*ci
     * \brief Pass through pointer to the downstream transport. Must
     *        be valid for the life-cycle of the transport.
     */
    void *user_property;
};

/*ci \brief Convenience macro to call downstream interface
 */
#define NETIO_DGRAM_Interface_create_instance(self_) \
    (self_)->factory->user_intf.create_instance(\
            (struct NETIO_Interface*)(self_),\
            (self_)->factory->user_property)

#define NETIO_DGRAM_InterfaceEntry_resolve_address(self_,\
    if_entry_,addr_string_,addr_val_,is_invalid_) \
    (self_)->factory->user_intf.resolve_address((self_)->user_netio,\
            (if_entry_),(addr_string_),(addr_val_),(is_invalid_))

/*ci \brief Convenience macro to call downstream interface
 */
#define NETIO_DGRAM_Interface_get_interface_list(self_,seq_) \
(self_)->factory->user_intf.get_interface_list(\
        (self_)->user_netio,(seq_))

/*ci \brief Convenience macro to call downstream interface
 */
#define NETIO_DGRAM_Interface_bind_address(self_,addr_) \
(self_)->factory->user_intf.bind_address(\
        (self_)->user_netio,(addr_))
        
/*ci \brief Convenience macro to call downstream interface
 */
#define NETIO_DGRAM_Interface_delete_instance(self_,instance_) \
    (self_)->factory->user_intf.delete_instance(instance_)

/*ci
 * \brief NETIO_DGRAM bind entry
 */
struct NETIO_DGRAM_BindEntry
{
    /*ci
     * \brief base-class
     */
    struct NETIOBindEntry _parent;

    /*ci
     * \brief The number of binds to the port
     */
    RTI_INT32 ref_count;
    
    /*ci
     * \brief If TRUE the locator port is shared, do not compare addresses
     */
    RTI_BOOL is_shared;
};

/*ci
 * \brief NETIO_DGRAM port entry
 */
struct NETIO_DGRAM_PortEntry
{
    /*ci
     * \brief The address to bind to and receive data from
     */
    struct NETIO_Address source;

    /*ci
     * \brief The number of listeners to this port
     */
    RTI_UINT32 _ref_count;

    /*ci
     * \brief Back reference to the NETIO_DGRAM interface that created this port
     */
    struct NETIO_DGRAM_Interface *_dgram_intf;
    
    /*ci
     * \brief If TRUE, the locator port is shared, do not compare addresses
     */
    RTI_BOOL is_shared;
};

#endif
