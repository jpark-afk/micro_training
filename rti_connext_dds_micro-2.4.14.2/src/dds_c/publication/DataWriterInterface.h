/*
 * FILE: DataWriterInterface.h - DataWriterInterface implementation
 *
 * (c) Copyright 2012-2015 Real-Time Innovations,
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
 * 06jun2016,tk MICRO-1543 Reference count shared resources for matched entities (needed
 *                         after changes to related to MICRO-1505
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 * 30jun2015,tk MICRO-1378/PR#15203 Updated comments
 * 23mar2013,tk Major update
 * 14may2012,tk Written
 */
/*ce
 * \file
 * \brief DataWriterInterface implementation
 */
/*ci \addtogroup DDSPublicationModule
 * @{
 */
#ifndef DataWriterInterface_h
#define DataWriterInterface_h

/*ci
 * \brief Specialized properties for the DataWriter NETIO interface
 */
struct DDS_DataWriterInterfaceProperty
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
    struct DDS_DataWriterImpl *datawriter;

    /*ci
     * \brief The maximum number of destinations the interface can send a
     *        packet to in one send()
     */
    RTI_UINT32 max_send_fanout;
};

/*ci
 * \def DDS_DataWriterInterfaceProperty_INITIALIZER
 * \brief Constant to initialize \ref  DDS_DataWriterInterfaceProperty
 */
#define DDS_DataWriterInterfaceProperty_INITIALIZER \
{\
    NETIO_InterfaceProperty_INITIALIZER,\
    NETIO_Address_INITIALIZER, \
    NULL, \
    1  \
}

/*ci
 * \brief Datawriter NETIO interface bind properties
 */
struct DDS_DataWriterBindProperty
{
    /*ci
     * \brief Inherited bind-properties
     */
    struct NETIOBindProperty _parent;

    /*ci
     * \brief The lowest SN applicable to the peer
     */
    struct REDA_SequenceNumber low_history_sn;

    /*ci
     * \brief The highest SN applicable to the peer
     */
    struct REDA_SequenceNumber high_history_sn;

    /*ci
     * \brief The next SN applicable to the peer
     */
    struct REDA_SequenceNumber next_new_sn;

    /*ci
     * \brief Whether the peer interface is reliable or not
     */
    RTI_BOOL is_reliable;
};

/*ci
 * \def DDS_DataWriterBindProperty_INITIALIZER
 * \brief Constant to initialize \ref DDS_DataWriterBindProperty
 */
#define DDS_DataWriterBindProperty_INITIALIZER \
{\
    NETIOBindProperty_INITIALIZER, \
    REDA_SEQUENCE_NUMBER_ZERO, \
    REDA_SEQUENCE_NUMBER_ZERO, \
    REDA_SEQUENCE_NUMBER_ZERO, \
    RTI_FALSE\
}

/*ci
 * \brief Entry in the DataWriter NETIO interface route table
 */
struct DDS_DataWriterRouteEntry
{
     /*ci
      * \brief The interface which can reach the address
      */
     NETIO_Interface_T *intf;

     /*ci
      * \brief Reference counter when a writer is matched with multiple
      *        readers using the same downstream route.
      */
     DDS_Long ref_count;
};

/*ci
 * \brief Key for entries in the DataWriter NETIO interface route table
 */
struct DDS_DataWriterRouteEntryKey
{
    /*ci
     * \brief The interface which can reach the address
     */
    NETIO_Interface_T *intf;
};

/*ci
 * \brief ACTIVE state bit for DDS_DataWriterBindEntry.state
 */
#define DDS_DATAWRITER_BINDENTRY_STATE_ACTIVE   0x1U

/*ci
 * \brief RELIABLE state bit for DDS_DataWriterBindEntry.state
 */
#define DDS_DATAWRITER_BINDENTRY_STATE_RELIABLE 0x2U

/*ci
 * \brief Entry in the DataWriter NETIO interface bind table
 */
struct DDS_DataWriterBindEntry
{
    /*ci
     * \brief The destination address listening to the source
     */
    struct NETIO_Guid source;

    /*ci
     * \brief The interface to use to listen to the source
     */
    NETIO_Interface_T *intf;

    /*ci
     * \brief The highest SN applicable to the peer
     */
    struct REDA_SequenceNumber high_history_sn;

    /*ci
     * \brief The next SN applicable to the peer
     */
    struct REDA_SequenceNumber next_new_sn;

    /*ci
     * \brief The state of the peer, such as ACTIVE and whether it is reliable
     */
    RTI_UINT32 state;
};

/*ci
 * \brief Key for entries in the DataWriter NETIO interface bind table
 */
struct DDS_DataWriterBindEntryKey
{
    /*ci
     * \brief The destination address listening to the source
     */
    struct NETIO_Guid source;
};

/*ci
 * \brief Implementation of the DataWriter NETIO interface
 */
struct DDS_DataWriterInterface
{
    /*ci
     * \brief Base-class
     */
    struct NETIO_Interface _parent;

    /*ci
      * \brief Reference to the owner of the NETIO interface
      */
     struct DDS_DataWriterImpl *datawriter;

    /*ci
     * \brief Reference to the interface-factory
     */
    struct DDS_DataWriterInterfaceFactory *factory;

    /*ci
     * \brief Sequence with destination address, re-used for each send
     */
    struct NETIO_AddressSeq send_dests_seq;

    /*ci
     * \brief The number of acknowledgments expected for a written sample
     */
    RTI_INT32 active_acking_readers;
};

/*ci \def DDSWI_INTERFACE_INTERFACE_ID
 *   \brief DataWriter NETIO interface ID
 */
#define DDSWI_INTERFACE_INTERFACE_ID RT_MKINTERFACEID(\
            RT_COMPONENT_CLASS_NETIO,RT_COMPONENT_INSTANCE_DDSWI)

/*ci
 * \brief Function to retrieve the concrete implementation of the
 *        NETIO DataWriter interface factory
 *
 * \return Pointer to NETIO DataWriter factory implementation
 */
MUST_CHECK_RETURN struct RT_ComponentFactoryI*
DDS_DataWriterInterfaceFactory_get_interface(void);

#endif

/*ci @} */
