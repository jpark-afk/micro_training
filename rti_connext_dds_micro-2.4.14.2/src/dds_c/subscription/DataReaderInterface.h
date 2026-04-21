/*
 * FILE: DataReaderInterface.h - NETIO interface for DDS DataReader
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
 * 05mar2013,eh MICRO-350: unlock DB when rcv liveliness HB
 * 06feb2013,eh MICRO-216: receive liveliness msg
 * 06feb2013,eh MICRO-210: fix inlineQoS deserialization's endianness
 * 14may2012,tk Written
 */
/*ce
 * \file
 * \brief NETIO interface for DDS DataReader
 */
/*ci \addtogroup DDSSubscriptionModule
 * @{
 */
#ifndef DataReaderInterface_h
#define DataReaderInterface_h

/*ci
 * \brief Specialized properties for the DataReader NETIO interface
 */
struct DDS_DataReaderInterfaceProperty
{
    /*ci
     * \brief Inherited properties
     */
    struct NETIO_InterfaceProperty _parent;

    /*ci
     * \brief The NETIO address for the interface
     */
    struct NETIO_Address intf_address;

    /*ci
     * \brief Reference to the owner of the NETIO interface
     */
    struct DDS_DataReaderImpl *datareader;
};

/*ci
 * \def DDS_DataReaderInterfaceProperty_INITIALIZER
 * \brief Constant to initialize \ref  DDS_DataReaderInterfaceProperty
 */
#define DDS_DataReaderInterfaceProperty_INITIALIZER \
{\
    NETIO_InterfaceProperty_INITIALIZER,\
    NETIO_Address_INITIALIZER, \
    NULL\
}

/*ci
 * \brief The datareader interface local state of a peer datawriter interface
 */
typedef enum
{
    /*ci
     * \brief The peer datawriter interface is not alive, e.g. liveliness lost
     */
    REMOTE_WRITERSTATE_NOT_ALIVE,

    /*ci
     * \brief The peer datawriter interface is alive, e.g. samples are received
     *        timely
     */
    REMOTE_WRITERSTATE_ALIVE
} RemoteWriterState_t;


/*ci
 * \brief Entry in the DataReader NETIO interface route table
 */
struct DataReaderRouteEntry
{
    /*ci
     * \brief Inherited entry
     */
    struct NETIORouteEntry _parent;
};

/*ci
 * \brief Entry in the DataReader NETIO interface bind table
 */
struct DataReaderBindEntry
{
    /*ci
     * \brief Inherited bind entry
     */
    struct NETIOBindEntry _parent;

    /*ci
     * \brief The state of the peer entry
     */
    RemoteWriterState_t writer_state;

    /*ci
     * \brief The number of times there has been a change in activity
     */
    RTI_UINT32 activity_count;

    /*ci
     * \brief The number of times there has been a change in activity since
     *        last time the counter was read
     */
    RTI_UINT32 last_activity_count;

    /*ci
     * \brief The strength of the entry, used to determine which peer to
     *        accept data from in case of exclusive ownership
     */
    RTI_INT32 strength;
};

struct DDS_DataReaderInterfaceFactory;

/*ci
 * \brief Implementation of the DataReader NETIO interface
 */
struct DDS_DataReaderInterface
{
    /*ci
     * \brief Base-class
     */
    struct NETIO_Interface _parent;

    /*ci
     * \brief The properties the interface was created with
     */
    struct DDS_DataReaderInterfaceProperty property;

    /*ci
     * \brief The listener the interface was created with
     */
    struct NETIO_InterfaceListener listener;

    /*ci
     * \brief Reference to the interface factory
     */
    struct DDS_DataReaderInterfaceFactory *factory;

    /*ci
     * \brief The interface state, determines whether the interface will
     *        accept data or not
     */
    NETIO_InterfaceState_T state;
};

/*ci \def DDSRI_INTERFACE_INTERFACE_ID
 *   \brief DataReader NETIO interface ID
 */
#define DDSRI_INTERFACE_INTERFACE_ID RT_MKINTERFACEID(\
            RT_COMPONENT_CLASS_NETIO,RT_COMPONENT_INSTANCE_DDSRI)

/*ci
 * \brief Function to get the concrete implementation of the
 *        NETIO DataReader interface factory
 *
 * \return Pointer to NETIO DataReader factory implementation
 */
MUST_CHECK_RETURN struct RT_ComponentFactoryI*
DDS_DataReaderInterfaceFactory_get_interface(void);

#endif

/*ci @} */

