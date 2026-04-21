/*
 * FILE: rtps_rtps.h 
 *
 * Copyright 2008-2021 Real-Time Innovations, Inc.
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
 * 17aug2022,tk MICRO-4115/PR.30818
 * - Changed count to RTI_UINT32 in RTPS_HEARTBEAT and RTPS_HEARTBEAT_BATCH
 *   for well defined behavior of rollover.
 * 21jul2021,tk MICRO-3045 Fixed filenames and dates in file header comments
 * 13apr2021,tk MICRO-3028/PR.28960
 * - Made documentation for RTPS_InterfaceFactory_get_interface public.
 * 20feb2021,tk MICRO-2866/PR#28967
 *   - Added RTPS_SUBMESSAGE_LENGTH_BYTE_ALIGN
 * 19oct2020,tk MICRO-2575/PR#28172 Use 0 as the submessage ID for the
 *                                  HeaderExtension.
 * 04nov2015,tk  MICRO-1505 Reduce memory footprint
 * 16jul2015,eh  MICRO-1439/PR#15427 Fix RTPS_LOCATOR_DEFAULT
 * 23feb2015,eh  MICRO-1081: fix function/var names to conform to coding std
 * 23feb2015,eh  MICRO-1075: remove and replace macros
 * 13oct2014,eh  MICRO-925: remove unused RTPS_DATAFLAGS_K
 * 09may2014,eh  MICRO-261 (Verocel PR#1421): remove RTPS_SubmessageId_toString
 *               for Cert
 * 07jan2014,eh  MICRO-733: update to Connext Micro vendor ID  
 * 15may2013,eh  MICRO-394: constant inline qos lengths, from CR-138 
 * 18may2011,eh  Created, based on Waveworks tree.
 */
/*ci \file
 * \defgroup NETIO_RTPSInterfaceClass RTPS Interface
 * \ingroup NETIOModule
 * \brief NETIO RTPS Interface
 *
 * \details
 *
 * The RTPS interface is implemented as a RTPS interface and RTPS interface
 * factory and implements the RTPS protocol in C.
 */
/*ci \addtogroup NETIO_RTPSInterfaceClass
 * @{
 */
#ifndef rtps_rtps_h
#define rtps_rtps_h

#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef reda_epoch_h
#include "reda/reda_epoch.h"
#endif
#ifndef reda_sequenceNumber_h
#include "reda/reda_sequenceNumber.h"
#endif
#ifndef osapi_time_h
#include "osapi/osapi_time.h"
#endif
#ifndef osapi_system_h
#include "osapi/osapi_system.h"
#endif
#ifndef rtps_dll_h
#include "rtps/rtps_dll.h"
#endif
#ifndef cdr_stream_h
#include "cdr/cdr_stream.h"
#endif
#ifndef netio_common_h
#include "netio/netio_common.h"
#endif
#ifndef netio_common_h
#include "netio/netio_common.h"
#endif
#ifndef netio_address_h
#include "netio/netio_address.h"
#endif
#ifndef netio_route_h
#include "netio/netio_route.h"
#endif
#ifndef netio_interface_h
#include "netio/netio_interface.h"
#endif
#ifndef rtps_config_h
#include "rtps/rtps_config.h"
#endif
#ifndef rtps_checksum_h
#include "rtps/rtps_checksum.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef RTI_CPP
#define RTPS_CPP_SUPPORT_METHODS_BASIC(T) \
    public:\
        T();\
        ~T();\
    private: \
        T(const T& from);\
        T& operator=(const T& from);\
        bool operator==(const T& other);\
        bool operator!=(const T& other);
#else /* ifdef RTI_CPP */
#define RTPS_CPP_SUPPORT_METHODS_BASIC(T)
#endif /* ifdef RTI_CPP */

/*ci \brief Maximum length of an RTPS serializable path name string */
#define RTPS_PATHNAME_LEN_MAX (255)


/*ci \ingroup RTPS_PidModule
 * \brief RTPS parameter ID, used to describe RTPS objects.
 */
typedef RTI_UINT16 RTPS_ParameterId;

/*ci \ingroup RTPS_PidModule
 * \brief Mask for ParameterId bit indicating subspace where 
 * unrecognized PIDs are either ignored or treated as 
 * incompatible QoS.
*/
#define RTPS_PID_INCOMPATIBLE_MASK              (0x4000)

/*ci \ingroup RTPS_PidModule
 * \brief Pad for parameter values in case they don't end on a 4-byte boundary
 */
#define RTPS_PID_PAD                             (0x0000)

/*ci \ingroup RTPS_PidModule
 * \brief Indicate end of parameter sequence.
 */
#define RTPS_PID_SENTINEL                        (0x0001)

/*ci \ingroup RTPS_PidModule
 * \brief Topic name parameter ID
 *  
 * \details 
 * Used for field Publication::topic: PathName, Subscription::topic : PathName
 */
#define RTPS_PID_TOPIC_NAME                      (0x0005)

/*ci \ingroup RTPS_PidModule
 * \brief Type name parameter ID
 *  
 * \details 
 * Used for field Publication::typeName : TypeName,
 * Subscription::typeName : TypeName
 */
#define RTPS_PID_TYPE_NAME                       (0x0007)

/*ci \ingroup RTPS_PidModule
 * \brief Durability Qos parameter ID
 *  
 * \details Used for field
 * Publication::durabilityQosPolicy : DurabilityQosPolicy, and
 * Subscription::durabilityQosPolicy : DurabilityQosPolicy
 */
#define RTPS_PID_DURABILITY         (0x001d)

/*ci \ingroup RTPS_PidModule
 * \brief DestinationOrder Qos parameter ID
 *
 * \details Used for field
 * Publication::DestinationOrderQosPolicy :  DurabilityQosPolicy, and
 * Subscription::DestinationOrderQosPolicy : DestinationOrderQosPolicy
 */
#define RTPS_PID_DESTINATION_ORDER         (0x0025)

/*ci \ingroup RTPS_PidModule 
 * \brief Deadline Qos parameter ID
 *
 * \details
 * Used for field Subscription::deadlineQosPolicy : DeadlineQosPolicy
 */
#define RTPS_PID_DEADLINE         (0x0023)

/*ci \ingroup RTPS_PidModule 
 * \brief Liveliness Qos parameter ID
 *
 * \details
 * Used for field Publication::livelinessQosPolicy : LivelinessQosPolicy, and
 * Subscription::livelinessQosPolicy : LivelinessQosPolicy
 */
#define RTPS_PID_LIVELINESS         (0x001b)

/*ci \ingroup RTPS_PidModule 
 * \brief Reliability Qos parameter ID
 *
 * \details
 * Used for field Publication::reliabilityOffered : RTI_UINT32, and
 * Subscription::reliabilityRequested : RTI_UINT32
 */
#define RTPS_PID_RELIABILITY        (0x001a)

/*ci \ingroup RTPS_PidModule 
 * \brief Ownership Qos parameter ID
 *
 * \details
 * Used for field Subscription::ownershipQosPolicy : OwnershipQosPolicy
 */
#define RTPS_PID_OWNERSHIP         (0x001f)

/*ci \ingroup RTPS_PidModule 
 * \brief Ownership Strength Qos parameter ID
 *
 * \details
 * Used for field Publication::strength : long
 */
#define RTPS_PID_OWNERSHIP_STRENGTH (0x0006)

/*ci \ingroup RTPS_PidModule
 * \brief Protocol Version paramater ID 
 *  
 * \details 
 * Used for field Application::protocolVersion : ProtocolVersion
 */
#define RTPS_PID_PROTOCOL_VERSION                (0x0015)

/*ci \ingroup RTPS_PidModule
 * \brief Vendor ID parameterID 
 *  
 * \details 
 * Used for field Application::vendorId : VendorId
 */
#define RTPS_PID_VENDOR_ID                       (0x0016)


/*ci \brief Maximum number of addresses per type, per entity */
#define RTPS_PID_USERDATA_IPADDRESS_COUNT_MAX   (4)

/*ci \ingroup RTPS_PidModule 
 * \brief Unicast locator parameter ID
 *
 * \details Array of max size RTPS_PID_USERDATA_UNICAST_IPADDRESS_COUNT_MAX
 */
#define RTPS_PID_UNICAST_LOCATOR6     (0x002f)

/*ci \ingroup RTPS_PidModule 
 * \brief Multicast locator parameter ID
 *
 * \details Array of max size RTPS_PID_USERDATA_UNICAST_IPADDRESS_COUNT_MAX
 */
#define RTPS_PID_MULTICAST_LOCATOR6     (0x0030)

/*ci \ingroup RTPS_PidModule 
 * \brief Default unicast locator parameter ID
 *
 * \details Array of max size RTPS_PID_USERDATA_UNICAST_IPADDRESS_COUNT_MAX
 */
#define RTPS_PID_DEFAULT_UNICAST_LOCATOR6     (0x0031)

/*ci \ingroup RTPS_PidModule 
 * \brief Discovery (meta-traffic) unicast locator parameter ID
 *
 * \details Array of max size RTPS_PID_USERDATA_UNICAST_IPADDRESS_COUNT_MAX
 */
#define RTPS_PID_METATRAFFIC_UNICAST_LOCATOR6 (0x0032)

/*ci \ingroup RTPS_PidModule 
 * \brief Discovery (meta-traffic) multicast locator parameter ID
 *
 * \details Array of max size RTPS_PID_USERDATA_UNICAST_IPADDRESS_COUNT_MAX
 */
#define RTPS_PID_METATRAFFIC_MULTICAST_LOCATOR6 (0x0033)

/*ci \ingroup RTPS_PidModule 
 * \brief Liveliness lease duration parameter ID
 */
#define RTPS_PID_LEASE_DURATION                (0x0002)

/*ci \ingroup RTPS_PidModule 
 * \brief Property Qos parameter ID 
 *  
 * \details Used for field Participant::propertyList
 */
#define RTPS_PID_PROPERTY_LIST                    (0x0059)

/*ci \ingroup RTPS_PidModule 
 * \brief Participant GUID parameter ID
 */
#define RTPS_PID_PARTICIPANT_GUID                 (0x0050)

/*ci \ingroup RTPS_PidModule
 * \brief Presentation Qos parameter ID
 */
#define RTPS_PID_PRESENTATION                     (0x0021)

/*ci \ingroup RTPS_ParameterSequenceClass 
 * \brief Builtin endpoint mask parameter ID 
 *  
 * \details Indicate builtin endpoint set inline Qos, 
 * PID_BUILTIN_ENDPOINT_SET from the RTPS spec
 */
#define RTPS_PID_BUILTIN_ENDPOINT_MASK     (0x0058)

/*ci \ingroup RTPS_ParameterSequenceClass 
 * \brief Endpoint GUID parameter ID
 */
#define RTPS_PID_ENDPOINT_GUID    (0x005A)

/*ci \ingroup RTPS_ParameterSequenceClass 
 * \brief Entity name parameter ID
 */
#define RTPS_PID_ENTITY_NAME     (0x0062)

/* Vendor specific
 */
/*ci \ingroup RTPS_ParameterSequenceClass
 * \brief Checksum Property parameter ID sent by participants
 */
#define RTPS_PID_CHECKSUM_PROPERTY    (0x9000)

/******************************************************************************/
/*ci \ingroup RTPS_ParameterSequenceClass 
 * \brief Key Hash parameter ID
 */
#define RTPS_PID_KEY_HASH     (0x0070)

/*ci \brief Key Hash parameter length */
#define RTPS_KEY_HASH_PARAM_LENGTH  (16)

/*ci \ingroup RTPS_ParameterSequenceClass
 * \brief Status info parameter ID
 */
#define RTPS_PID_STATUS_INFO    (0x0071)

/*ci \brief Status info parameter length */
#define RTPS_STATUS_INFO_PARAM_LENGTH (4)

/*ci \brief Send queue size parameter ID */
#define RTPS_PID_SEND_QUEUE_SIZE_DEPRECATED  (0x0013)

/******************************************************************************/
/*ci \ingroup RTPS_Class 
 * \brief Host ID of GUID
 */
typedef RTI_UINT32 RTPS_HostId;

/*ci \ingroup RTPS_Class 
 * \brief Unknown Host ID
 */
#define RTPS_HOST_ID_UNKNOWN (0x00000000)

/*ci \ingroup RTPS_Class 
 * \brief Application ID of GUID
 */
typedef RTI_UINT32 RTPS_AppId;

/*ci \ingroup RTPS_Class 
 * \brief Unknown Application ID
 */
#define RTPS_APP_ID_UNKNOWN (0x00000000)

/*ci \ingroup RTPS_Class 
 * \brief Instance ID of GUID
 */
typedef RTI_UINT32 RTPS_InstanceId;

/*ci \ingroup RTPS_Class 
 * \brief Unknown Instance ID
 */
#define RTPS_INSTANCE_ID_UNKNOWN (0x00000000)

/*ci \ingroup RTPS_Class 
 * \brief Object ID of GUID
 */
typedef RTI_UINT32 RTPS_ObjectId;

/*ci \ingroup RTPS_Class 
 * \brief Unknown Object ID
 */
#define RTPS_OBJECT_ID_UNKNOWN (0x00000000)

/*ci \ingroup RTPS_Class 
 * \brief Maximum Object ID
 */
#define RTPS_OBJECT_ID_MAX     (0xFFFFFFFF)

/******************************************************************************/
/*ci \ingroup RTPS_Class 
 * \brief Prefix portion of GUID. 
 *  
 * \details Uniquely identifies the DomainParticipant to which 
 * an RTPS's DDS entity belong
 */
struct RTPS_GuidPrefix
{
    /*ci \brief First field of GUID Prefix. 
     * \details Historically set to host IP address
     */
    RTPS_HostId host_id;

    /*ci \brief Second field of GUID Prefix. 
     * \details Historically set to process ID
     */
    RTPS_AppId app_id ;

    /*ci \brief Third field of GUID prefix. 
     * \details Historically set to instance counter
     */
    RTPS_InstanceId instance_id ;
};

/*ci \ingroup RTPS_Class 
 * \brief Unknonwn GUID prefix
 */
#define RTPS_GUID_PREFIX_UNKNOWN { \
    RTPS_HOST_ID_UNKNOWN,   \
    RTPS_APP_ID_UNKNOWN,    \
    RTPS_INSTANCE_ID_UNKNOWN}

/*ci \ingroup RTPS_Class 
 * \brief Length in bytes of GUID prefix
 */
#define RTPS_GUID_PREFIX_SIZE (12)

/*ci \ingroup RTPS_Class 
 * \brief GUID
 */
struct RTPS_Guid
{
    /*ci \brief GUID prefix */
    struct RTPS_GuidPrefix prefix;

    /*ci \brief Unique entity identifier */
    RTPS_ObjectId object_id;
};

/*ci \ingroup RTPS_Class 
 * \brief Unknown GUID
 */
#define RTPS_GUID_UNKNOWN {   \
    RTPS_GUID_PREFIX_UNKNOWN,   \
    RTPS_OBJECT_ID_UNKNOWN}

/******************************************************************************/
/*ci \brief Participant Object ID */
#define RTPS_OBJECT_ID_PARTICIPANT             (0x000001C1)

/*ci \brief Simple Discovery Protocol Participant Writer Object ID */
#define RTPS_OBJECT_ID_WRITER_SDP_PARTICIPANT  (0x000100C2)

/*ci \brief Simple Discovery Protocol Publication Writer Object ID */
#define RTPS_OBJECT_ID_WRITER_SDP_PUBLICATION  (0x000003C2)

/*ci \brief Simple Discovery Protocol Subscription Writer Object ID */
#define RTPS_OBJECT_ID_WRITER_SDP_SUBSCRIPTION (0x000004C2)

/*ci \brief Simple Discovery Protocol Participant Reader Object ID */
#define RTPS_OBJECT_ID_READER_SDP_PARTICIPANT  (0x000100C7)

/*ci \brief Simple Discovery Protocol Subscription Reader Object ID */
#define RTPS_OBJECT_ID_READER_SDP_PUBLICATION  (0x000003C7)

/*ci \brief Simple Discovery Protocol Subscription Reader Object ID */
#define RTPS_OBJECT_ID_READER_SDP_SUBSCRIPTION (0x000004C7)

/******************************************************************************/
/*ci \brief Key Hash max length */
#define RTPS_KEY_HASH_MAX_LENGTH 16

/*ci \brief Hash of instance key */
struct RTPS_KeyHash
{
    /*ci \brief Key hash value */
    RTI_UINT8 value[RTPS_KEY_HASH_MAX_LENGTH];

    /*ci \brief Key hash length */
    RTI_UINT32 length;
};

/*ci \brief Nil Key Hash value */
#define RTPS_KEY_HASH_NIL {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}

/*ci \brief Default Key Hash Initializer */
#define RTPS_KEY_HASH_DEFAULT {     \
RTPS_KEY_HASH_NIL,  /* value */\
RTPS_KEY_HASH_MAX_LENGTH/* length */\
}

/******************************************************************************/
/*ci \ingroup RTPS_Class
 * \brief Returns RTI_TRUE if GUIDs are equal 
 *  
 * \param[in] a First GUID 
 * \param[in] b Second GUID 
 *  
 * \return RTI_TRUE if GUIDs are equal, RTI_FALSE otherwise 
 */
MUST_CHECK_RETURN RTPSDllExport RTI_BOOL
RTPS_Guid_equals(struct RTPS_Guid *a, struct RTPS_Guid *b);

/*ci \ingroup RTPS_Class
 * \brief Returns RTI_TRUE if GUID prefixes are equal 
 *  
 * \param[in] a First GUID 
 * \param[in] b Second GUID 
 *  
 * \return RTI_TRUE if prefixes of GUIDs are equal, RTI_FALSE otherwise 
 */
MUST_CHECK_RETURN RTPSDllExport RTI_BOOL
RTPS_Guid_prefix_equals(
        struct RTPS_Guid *a,
        struct RTPS_Guid *b);

/*ci \ingroup RTPS_Class
 * \brief Returns RTI_TRUE if GUID suffixes are equal 
 *  
 * \param[in] a First GUID 
 * \param[in] b Second GUID 
 *  
 * \return RTI_TRUE if suffixes of GUIDs are equal, RTI_FALSE otherwise 
 */
MUST_CHECK_RETURN RTPSDllExport RTI_BOOL
RTPS_Guid_suffix_equals(
        struct RTPS_Guid *a,
        struct RTPS_Guid *b);

/******************************************************************************/
/*ci \brief RTPS Protocol Version type */
typedef RTI_UINT16 RTPS_ProtocolVersion_T;

/*ci \brief Major version of supported RTPS protocol */
#define RTPS_PROTOCOL_VERSION_MAJOR (0x02)

/*ci \brief Minor version of supported RTPS protocol */
#define RTPS_PROTOCOL_VERSION_MINOR (0x01)  

/*ci \brief RTPS Vendor ID type */
typedef RTI_UINT16 RTPS_VendorId;

/*ci \brief Major version of vendor ID */
#define RTPS_VENDOR_ID_MAJOR (0x01)

/*ci \brief Minor version of vendor ID */
#define RTPS_VENDOR_ID_MINOR (0x0A)

/*ci \brief Connext DDS Vendor ID */
#define RTPS_VENDOR_ID_RTI_DDS (0x0101)

/*ci \brief Connext Micro Vendor ID */
#define RTPS_VENDOR_ID_RTI_MICRO   (0x010A)

/*ci 
 * \brief Get major number of vendor ID
 *  
 * \param[in] me Vendor ID 
 *  
 * \return Major number of vendor ID 
 */
RTPS_VendorId
RTPS_VendorId_get_major(RTPS_VendorId *me);

/*ci 
 * \brief Get minor number of vendor ID
 *  
 * \param[in] me Vendor ID 
 *  
 * \return Minor number of vendor ID 
 */
RTPS_VendorId
RTPS_VendorId_get_minor(RTPS_VendorId *me);

/*****************************************************************************/
/*ci \brief RTPS Status Info type */
typedef RTI_UINT32 RTPS_StatusInfo;

/*ci \brief Default empty status info */
#define RTPS_NO_STATUS_INFO (0x00000000)

/*ci \brief Dispose status info bit */
#define RTPS_DISPOSE_STATUS_INFO (0x00000001)

/*ci \brief Unregister status info bit */
#define RTPS_UNREGISTER_STATUS_INFO (0X00000002)

/*ci \brief RTPS submessage header endian flag */
#define RTPS_ENDIAN_FLAG (0x01)

/*****************************************************************************/

/*ci \ingroup RTPS_Class
 * \brief Denotes the built-in object types
 *
 * \details
 * RTPSObject is categorized into (normal/reserved) x (user/meta) quadrant.
 * Within each quadrant, there are several built-in (provided for at the RTPS
 *  protocol level) objects, such as application, publication/subscription,
 * CST (keyed) writer/reader.
 */
typedef enum
{
    RTPS_OBJECT_NORMAL_USER_UNKNOWN = 0x00,
    RTPS_OBJECT_NORMAL_USER_APPLICATION = 0x01,
    /*ci \brief Use this suffix for writers of data with a key. */
    RTPS_OBJECT_NORMAL_USER_CST_WRITER = 0x02,
    /*ci \brief Use this suffix for writers of data with no key. */
    RTPS_OBJECT_NORMAL_USER_PUBLICATION = 0x03,
    /*ci \brief Use this suffix for readers of data with no key. */
    RTPS_OBJECT_NORMAL_USER_SUBSCRIPTION = 0x04,
    /*ci \brief Use this suffix for readers of data with a key. */
    RTPS_OBJECT_NORMAL_USER_CST_READER = 0x07,
    RTPS_OBJECT_NORMAL_USER_VIRTUAL_SUBSCRIPTION = 0x3C,
    RTPS_OBJECT_NORMAL_USER_VIRTUAL_CST_READER = 0x3D,
    RTPS_OBJECT_RESERVED_USER_UNKNOWN = 0x40,
    RTPS_OBJECT_RESERVED_USER_APPLICATION = 0x41,
    RTPS_OBJECT_RESERVED_USER_CST_WRITER = 0x42,
    RTPS_OBJECT_RESERVED_USER_PUBLICATION = 0x43,
    RTPS_OBJECT_RESERVED_USER_SUBSCRIPTION = 0x44,
    RTPS_OBJECT_RESERVED_USER_CST_READER = 0x47,
    RTPS_OBJECT_NORMAL_META_UNKNOWN = 0x80,
    RTPS_OBJECT_NORMAL_META_APPLICATION = 0x81,
    RTPS_OBJECT_NORMAL_META_CST_WRITER = 0x82,
    RTPS_OBJECT_NORMAL_META_PUBLICATION = 0x83,
    RTPS_OBJECT_NORMAL_META_SUBSCRIPTION = 0x84,
    RTPS_OBJECT_NORMAL_META_CST_READER = 0x87,
    RTPS_OBJECT_RESERVED_META_UNKNOWN = 0xc0,
    RTPS_OBJECT_RESERVED_META_APPLICATION = 0xc1,
    RTPS_OBJECT_RESERVED_META_CST_WRITER = 0xc2,
    RTPS_OBJECT_RESERVED_META_PUBLICATION = 0xc3,
    RTPS_OBJECT_RESERVED_META_SUBSCRIPTION = 0xc4,
    RTPS_OBJECT_RESERVED_META_CST_READER = 0xc7
} RTPS_ObjectSuffix;


/* ===========================================================================*/
/*                              RTPS types                                    */
/* ===========================================================================*/
/*ci \brief RTPS locator address max length */
#define RTPS_LOCATOR_ADDRESS_LENGTH_MAX 16

/*ci \brief 
 * RTPS locator address offset to UDPv4 address (elements 12-15 of array)
 */
#define RTPS_LOCATOR_ADDRESS_UDPV4_OFFSET 12

/*ci \brief RTPS Locator type */
struct RTPS_Locator_t
{
    /*ci \brief Locator kind */
    RTI_INT32 kind;

    /*ci \brief Locator port */
    RTI_UINT32 port;

    /*ci \brief Locator address */
    RTI_UINT8 address[RTPS_LOCATOR_ADDRESS_LENGTH_MAX];
};

/* ci \brief Default locator kind */
#define RTPS_LOCATOR_DEFAULT_KIND (-1)

/*ci \brief Default locator initializer */
#define RTPS_LOCATOR_DEFAULT {\
    RTPS_LOCATOR_DEFAULT_KIND,        /* kind    */\
    0,                                /* port    */\
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} /* address */\
}

/*ci \brief RTPS Locator type */
#define RTPS_Locator RTPS_Locator_t

/*ci \brief UDPv4 locator type */
struct RTPS_LocatorUdpv4_t
{
    /*ci \brief Locator kind */
    RTI_INT32 kind;

    /*ci \brief Locator port */
    RTI_UINT32 port;

    /*ci \brief Locator address */
    RTI_UINT32 address;
};

/*ci \brief Default UDPv4 locator initializer */
#define RTPS_LOCATOR_UDPV4_DEFAULT {\
    RTPS_LOCATOR_DEFAULT_KIND,        /* kind    */\
    0,                                /* port    */\
    0                                 /* address */\
}

/*ci \brief Invalid Locator */
extern RTPSDllVariable const struct RTPS_Locator_t RTPS_LOCATOR_INVALID;

/*ci \brief Invalid Locator kind */
extern RTPSDllVariable const RTI_INT32 RTPS_LOCATOR_KIND_INVALID;

/*ci \brief Invalid Locator port */
extern RTPSDllVariable const RTI_UINT32 RTPS_LOCATOR_PORT_INVALID;

/*ci \brief Invalid Locator address */
extern RTPSDllVariable const RTI_UINT8
    RTPS_LOCATOR_ADDRESS_INVALID[RTPS_LOCATOR_ADDRESS_LENGTH_MAX];

/*ci \brief UDPv4 Locator kind */
extern RTPSDllVariable const RTI_INT32 RTPS_LOCATOR_KIND_UDPv4;

/*ci \brief UDPv6 Locator kind */
extern RTPSDllVariable const RTI_INT32 RTPS_LOCATOR_KIND_UDPv6;

/*ci \brief Reserved Locator kind */
extern RTPSDllVariable const RTI_INT32 RTPS_LOCATOR_KIND_RESERVED;

/*ci \brief Shared Memory Locator kind */
extern RTPSDllVariable const RTI_INT32 RTPS_LOCATOR_KIND_SHMEM;


/* ===========================================================================*/
/*                           RTPS Sequence Number                             */
/* ===========================================================================*/


/*ci \ingroup RTPS_SequenceNumberClass
 * \brief Deserialize sequence number from stream buffer
 * 
 * \details
 * Post-condition: sequenceNumber is packed and stream pointer is moved forward.
 * 
 * \param[inout] src_buffer Deserialization stream buffer
 * \param[out] instance Deserialized instance
 * \param[in] byte_swap RTI_TRUE if byte swapping is necessary,
 * RTI_FALSE otherwise.
 *
 */
RTPSDllExport void
RTPS_SequenceNumber_deserialize(char **src_buffer,
                                struct REDA_SequenceNumber *instance,
                                RTI_BOOL byte_swap);


/*ci \ingroup RTPS_SequenceNumberClass
 * \brief Calculate the distance between two sequence numbers.
 *
 * \param[in] s1 First sequence number
 * \param[in] s2 Second sequence number
 *
 * \return The distance, possibly saturated to RTPS_BITMAP_DISTANCE_MAX.
 * -1 on failure.
 */
MUST_CHECK_RETURN RTPSDllExport RTI_INT32
RTPS_SequenceNumber_get_distance(
        const struct REDA_SequenceNumber *s1,
        const struct REDA_SequenceNumber *s2);

/*ci \brief Bitmap array max size 
 * \details Bitmap has maximum 256 bits, using (256/32 =) 8 ints 
 */
#define RTPS_BITMAP_32BITS_ARRAY_SIZE_MAX (8)       

/*ci 
 * \brief Bitmap of sequence numbers received 
 *  
 * \details 
 * Tracks sequence numbers of samples sent or received.  Used in send and 
 * window implementations. 
 */
struct RTPS_Bitmap
{
    /*ci \brief Starting sequence number of bitmap */
    struct REDA_SequenceNumber lead;

    /*ci \brief Number of valid bits or sequence numbers, starting from lead */
    RTI_INT32 bit_count;

    /*ci \brief The bitmap stored as an array of integers.
     *  
     * \details 
     * Maximum of 256 (=8*32) bits or sequence numbers. 
     * This must be unsigned in order to apply the >> operator.
     * There are two approaches to handle bits outside bitCount:
     * 1) They must always be zeroed.
     * 2) They can be anything.
     * The first approach puts the burden on otherwise simple
     * methods such as truncate to zero out truncated bits, whereas
     * the second approach puts the burden on methods such as shift
     * to make sure bits beyond the bitCount are not shifted in.
     * In our implementation, we have opted for the second approach. 
     */ 
    RTI_UINT32 bits[RTPS_BITMAP_32BITS_ARRAY_SIZE_MAX];
};

/*ci \brief Maximum number of bits in bitmap */
#define RTPS_BITMAP_SIZE_MAX              (256)

/*ci \brief Maximum bitmap sequence number distance */
#define RTPS_BITMAP_DISTANCE_MAX          (0x7FFFFFFF)

/*ci \brief Reset bitmap to specified lead and bitCount 
 *
 * \details Post condition: bitmap has its lead and bit count assigned, and
 * all bits are reset to zero.
 *
 * \param[in] me Bitmap
 * \param[in] sn New lead sequence number
 * \param[in] bit_count New bit count. Must be <= RTPS_BITMAP_SIZE_MAX.
 * 
 */
RTPSDllExport void
RTPS_Bitmap_reset(
        struct RTPS_Bitmap *me,
        const struct REDA_SequenceNumber *sn,
        RTI_INT32 bit_count);

/*ci \ingroup RTPS_BitmapClass
 * \brief Set specified bit in bitmap.
 *
 * \details
 * The bit corresponding to the input sequence number is set to either
 * 0 or 1 as specified in input param.
 *
 * \param[in] me Bitmap
 * \param[out] existed RTI_TRUE if bit is already set, RTI_FALSE otherwise.
 * \param[in] num The sequence number of interest.
 * \param[in] bit RTI_TRUE if bit is to be turned on, RTI_FALSE otherwise.
 *
 * \return RTI_FALSE on failure, which includes the sequence not in bitmap.
 * RTI_TRUE on success.
 */
MUST_CHECK_RETURN RTPSDllExport RTI_BOOL
RTPS_Bitmap_set_bit(struct RTPS_Bitmap *me,
                    RTI_BOOL *existed,
                    const struct REDA_SequenceNumber *num,
                    RTI_BOOL bit);

/*ci \ingroup RTPS_BitmapClass
 * \brief Get specified bit in bitmap.
 *
 * \details
 * The bit corresponding to the input sequence number is set to either
 * 0 or 1 as specified in input param.
 *
 * \param[in] me Bitmap
 * \param[in] bit Pointer to the answer, whose value will be RTI_TRUE
  if bit is on and RTI_FALSE otherwise.
 * \param[in] num The sequence number of interest.
 *
 * \return RTI_FALSE on failure, which includes the sequence not in bitmap.
 * RTI_TRUE on success.
 */
MUST_CHECK_RETURN RTPSDllExport RTI_BOOL
RTPS_Bitmap_get_bit(
        const struct RTPS_Bitmap *me,
        RTI_BOOL *bit,
        const struct REDA_SequenceNumber *num);

/*ci
 * \brief
 * Get the sequence number of the first bit in bitmap that matches the input 
 * value 
 * 
 * \param[in] me Bitmap
 * \param[out] position Sequence number of first bit matching searchBit.  If 
 * no match found, position is one greater than last valid bit of bitmap. 
 * \param[in] search_bit Value of bit for which to search 
 * 
 * \return RTI_TRUE on successfully finding matching bit, RTI_FALSE on failure
 */
SHOULD_CHECK_RETURN RTPSDllExport RTI_BOOL
RTPS_Bitmap_get_first_bit(
        const struct RTPS_Bitmap *me,
        struct REDA_SequenceNumber *position,
        RTI_BOOL search_bit);

/*ci
 * \brief
 * Get the sequence number of the last bit in bitmap that matches the input 
 * value 
 * 
 * \param[in] me Bitmap
 * \param[out] position Sequence number of last bit matching searchBit.  If 
 * no match found, position is zero. 
 * \param[in] search_bit Value of bit for which to search 
 * 
 * \return RTI_TRUE on successfully finding matching bit, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTPSDllExport RTI_BOOL
RTPS_Bitmap_get_last_bit(
        const struct RTPS_Bitmap *me,
        struct REDA_SequenceNumber *position,
        RTI_BOOL search_bit);

/*ci
 * \brief
 * Fill a range of bits in bitmap with specified value
 * 
 * \param[in] me Bitmap
 * \param[in] first_seq_num  First sequence number in range to fill
 * \param[in] last_seq_num Last sequence number in range to fill 
 * \param[in] bit Value to set bits in range 
 * 
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTPSDllExport RTI_BOOL
RTPS_Bitmap_fill(
        struct RTPS_Bitmap *me,
        const struct REDA_SequenceNumber *first_seq_num,
        const struct REDA_SequenceNumber *last_seq_num,
        RTI_BOOL bit);


/*ci
 * \brief
 * Merge another bitmap with self bitmap 
 *  
 * \details 
 * Copies '1' bits from source bitmap whose sequence numbers overlap with self 
 * bitmap.  Bitcount and lead SN of self bitmap are preserved. 
 * 
 * \param[in] me Self bitmap
 * \param[in] source Bitmap to merge with self
 * 
 */
RTPSDllExport void
RTPS_Bitmap_merge(
        struct RTPS_Bitmap *me,
        const struct RTPS_Bitmap *source);

/*ci
 * \brief
 * Shift lead sequence number of bitmap to new value
 *  
 * \details 
 * Preserves bitcount of bitmap, setting new shifted-in bits to zero.
 * 
 * \param[in] me Self bitmap
 * \param[in] seq_num New lead sequence number
 * 
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN RTPSDllExport RTI_BOOL
RTPS_Bitmap_shift(struct RTPS_Bitmap *me,
                  const struct REDA_SequenceNumber *seq_num);

/*ci
 * \brief Return the number of unreserved samples in the receive window.
 *
 * \param[in] me      The bitmap to shift
 * \param[in] seq_num New lead sequence number
 *
 * \return The number of unreserved samples
 */
MUST_CHECK_RETURN RTPSDllExport RTI_INT32
RTPS_Bitmap_get_unreserved_count(struct RTPS_Bitmap *me,
                                 const struct REDA_SequenceNumber *seq_num);

/*ci
 * \brief
 * Truncates bitmap to end at specified sequence number
 *  
 * \details 
 * Updates bitcount of bitmap to reflect number of valid bits up to truncated 
 * sequence number.  If truncation sequence number is greater than current last 
 * sequence number of bitmap, no truncation is done and bitcount stays the same.
 * 
 * \param[in] me Self bitmap
 * \param[in] seq_num Sequence number after which bitmap is truncated. 
 * 
 */
RTPSDllExport void
RTPS_Bitmap_truncate(
        struct RTPS_Bitmap *me,
        const struct REDA_SequenceNumber *seq_num);

/*ci
 * \brief
 * Toggle bits of bitmap 
 *  
 * \details 
 * Change '1' to '0' and vice versa. 
 *  
 * \param[inout] me Self bitmap
 * 
 */
RTPSDllExport void
RTPS_Bitmap_invert(struct RTPS_Bitmap *me);

/*ci
 * \brief
 * Deserialize from buffer a bitmap 
 *  
 * \details 
 * Change '1' to '0' and vice versa. 
 *  
 * \param[in] me Self bitmap 
 * \param[in] stream_ptr Pointer to serialized buffer 
 * \param[in] max_bits_len Remaining space of submessage to deserialize 
 * the variable length bits of bitmap 
 * \param[in] need_byte_swap Flag whether byte swap is necessary for 
 * deserialization 
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 * 
 */
MUST_CHECK_RETURN RTPSDllExport RTI_BOOL
RTPS_Bitmap_deserialize(
        struct RTPS_Bitmap *me,
        const char **stream_ptr,
        RTI_UINT32 max_bits_len,
        RTI_BOOL need_byte_swap);

/******************************************************************************/
/*ci
 * \brief
 * Serialize GUID to stream buffer 
 *  
 * \param[inout] stream Serialization stream 
 * \param[in] guid GUID to serialize 
 * \param[in] param Unused
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
MUST_CHECK_RETURN RTPSDllExport RTI_BOOL
RTPS_Guid_serialize(
    struct CDR_Stream_t *stream,
    const struct RTPS_Guid *guid,
    void * param);

/*ci
 * \brief
 * Deserialize GUID from stream buffer 
 *  
 * \param[in] stream Deserialization stream 
 * \param[inout] guid Deserialized GUID 
 * \param[in] param Unused
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
MUST_CHECK_RETURN RTPSDllExport RTI_BOOL
RTPS_Guid_deserialize(
    struct CDR_Stream_t *stream,
    struct RTPS_Guid *guid,
    void * param);

/*ci
 * \brief
 * Length in bytes of serialized GUID
 *  
 * \param[in] size Current length of serialized buffer
 *  
 * \return Number of bytes of serialized GUID
 */
MUST_CHECK_RETURN RTPSDllExport RTI_UINT32
RTPS_Guid_get_max_size_serialized(RTI_UINT32 size);

/* -------------------------------------------------------------------------- */
/*ci
 * \brief
 * Serialize NTP time to stream buffer 
 *  
 * \param[inout] stream Serialization stream 
 * \param[in] time NTP time to serialize 
 * \param[in] param Unused
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
MUST_CHECK_RETURN RTPSDllExport RTI_BOOL
RTPS_serialize_ntp_time(
    struct CDR_Stream_t *stream,
    const OSAPI_NtpTime *time,
    void * param);

/*ci
 * \brief
 * Deserialize NTP time from stream buffer 
 *  
 * \param[in] stream Deserialization stream 
 * \param[inout] time Deserialized NTP time 
 * \param[in] param Unused
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
MUST_CHECK_RETURN RTPSDllExport RTI_BOOL
RTPS_deserialize_ntp_time(
    struct CDR_Stream_t *stream,
    OSAPI_NtpTime *time,
    void * param);

/*ci
 * \brief
 * Length in bytes of serialized NTP time
 *  
 * \param[in] size Current length of serialized buffer
 *  
 * \return Number of bytes of serialized NTP time
 */
MUST_CHECK_RETURN RTPSDllExport RTI_UINT32
RTPS_get_ntp_time_max_size_serialized(RTI_UINT32 size);

/* -------------------------------------------------------------------------- */
/*ci
 * \brief
 * Serialize unsigned short to stream buffer 
 *  
 * \param[inout] stream Serialization stream 
 * \param[in] in Value to serialize 
 * \param[in] param Unused
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
MUST_CHECK_RETURN RTPSDllExport RTI_BOOL
RTPS_serialize_2_octets(
    struct CDR_Stream_t *stream,
    const RTI_UINT16 * in,
    void * param);

/*ci
 * \brief
 * Deserialize unsigned short from stream buffer 
 *  
 * \param[in] stream Deserialization stream 
 * \param[inout] out Deserialized value 
 * \param[in] param Unused
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
MUST_CHECK_RETURN RTPSDllExport RTI_BOOL
RTPS_deserialize_2_octets(
    struct CDR_Stream_t *stream,
    RTI_UINT16 *out,
    void * param);

/*ci
 * \brief
 * Length in bytes of serialized unsigned short
 *  
 * \param[in] size Current length of serialized buffer
 *  
 * \return Number of bytes of serialized unsigned short
 */
MUST_CHECK_RETURN RTPSDllExport RTI_UINT32
RTPS_get_2_octets_max_size_serialized(RTI_UINT32 size);

/* -------------------------------------------------------------------------- */
/*ci
 * \brief
 * Serialize locator to stream buffer 
 *  
 * \param[inout] stream Serialization stream 
 * \param[in] loc Locator to serialize 
 * \param[in] param Unused
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
MUST_CHECK_RETURN RTPSDllExport RTI_BOOL
RTPS_serialize_ipv6_locator(
    struct CDR_Stream_t *stream,
    const struct RTPS_Locator_t *loc,
    void * param);

/*ci
 * \brief
 * Deserialize locator from stream buffer 
 *  
 * \param[in] stream Deserialization stream 
 * \param[inout] loc Deserialized locator 
 * \param[in] param Unused
 *  
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
MUST_CHECK_RETURN RTPSDllExport RTI_BOOL
RTPS_deserialize_ipv6_locator(
    struct CDR_Stream_t *stream,
    struct RTPS_Locator_t *loc,
    void * param);

/*ci
 * \brief
 * Length in bytes of serialized locator
 *  
 * \param[in] size Current length of serialized buffer
 *  
 * \return Number of bytes of serialized locator
 */
MUST_CHECK_RETURN RTPSDllExport RTI_UINT32
RTPS_get_ipv6_locator_max_size_serialized(RTI_UINT32 size);

/* -------------------------------------------------------------------------- */
/*ci \brief Deserialize epoch from stream buffer 
 *
 * \param[inout] me Deserialized epoch
 * \param[in] stream Deserialization stream
 * \param[in] need_byte_swap Flags whether to byte swap byte order on
 * deserialization
 * 
 * \return RTI_TRUE on success, RTI_FALSE on failure 
 */
RTPSDllExport void
RTPS_Epoch_deserialize(REDA_Epoch_T * me,
                       const char **stream,
                       RTI_BOOL need_byte_swap);


/*ci \brief RTPS GUID type */
typedef struct NETIO_Guid RTPS_GUID; 

/*ci \brief RTPS GuidPrefix type */
typedef struct NETIO_GuidPrefix RTPS_GuidPrefix_T;

/*ci \brief RTPS Entity type */
typedef struct NETIO_GuidEntity RTPS_Entity_T;

/*ci \brief Default unknown GUID prefix initializer */
#define RTPS_GUIDPREFIX_UNKNOWN {{0,0,0,0,0,0,0,0,0,0,0,0}}

/*ci \brief Default unknown Entity ID initializer */
#define RTPS_ENTITY_UNKNOWN {{0,0,0,0}}

/* -------------------------------------------------------------------------- */
/*ci \brief RTPS Timestamp type */
struct RTPS_Time 
{
    /*ci \brief Seconds */
    RTI_INT32 seconds;

    /*ci \brief Fractional part */
    RTI_UINT32 fractions;
};

/*ci \brief RTPS Vendor ID type */
struct RTPS_Vendor 
{
    /*ci \brief Vendor ID value */
    RTI_UINT8 value[2];
};

/*ci \brief RTPS Protocol Version type */
struct RTPS_ProtocolVersion 
{
    /*ci \brief Major */
    RTI_UINT8 major;

    /*ci \brief Minor */
    RTI_UINT8 minor;
};

/*ci \brief RTPS message header 
 * \details 
 * Fields are ordered exactly as defined by RTPS specification 
 */
struct RTPS_Header 
{
    /*ci \brief 'R''T''P''S' protocol identifier */
    RTI_UINT32 rtps;

    /*ci \brief Protocol version */
    struct RTPS_ProtocolVersion protocol_version;

    /*ci \brief Vendor ID */
    struct RTPS_Vendor vendor_id;

    /*ci \brief Sender's GUID Prefix*/
    RTPS_GuidPrefix_T guid_prefix;
};

/*ci \brief RTPS message header's RTPS identifier, in host order */
#ifdef RTI_ENDIAN_LITTLE
#define VALID_RTPS_HEADER 0x53505452    /* RTPS backwards */
#else
#define VALID_RTPS_HEADER 0x52545053    /* RTPS in big endian */
#endif /* RTI_ENDIAN_LITTLE */

/******************************************************************************/

/* RTPS Submessage IDs */
/*ci \brief PAD submessage ID */
#define RTPS_PAD_KIND            (0x01)

/*ci \brief ACKNACK submessage ID */
#define RTPS_ACKNACK_KIND        (0x06)

/*ci \brief HEARTBEAT submessage ID */
#define RTPS_HEARTBEAT_KIND      (0x07)

/*ci \brief GAP submessage ID */
#define RTPS_GAP_KIND            (0x08)

/*ci \brief INFO_TS submessage ID */
#define RTPS_INFO_TS_KIND        (0x09)

/*ci \brief INFO_SRC submessage ID */
#define RTPS_INFO_SRC_KIND       (0x0c)

/*ci \brief INFO_REPLY_IP4 submessage ID */
#define RTPS_INFO_REPLY_IP4_KIND (0x0d)

/*ci \brief INFO_DST submessage ID */
#define RTPS_INFO_DST_KIND       (0x0e)

/*ci \brief INFO_REPLY submessage ID */
#define RTPS_INFO_REPLY_KIND     (0x0f)

/*ci \brief DATA submessage ID */
#define RTPS_DATA_KIND           (0x15)

/*ci \brief BATCH DATA submessage ID */
#define RTPS_DATA_BATCH_KIND     (0x18)

/*ci \brief BATCH HEARTBEAT submessage ID */
#define RTPS_HEARTBEAT_BATCH_KIND (0x19)

/*ci \brief Core CRC32   */
#define RTPS_RTI_CRC32            (0x80)

/*ci \brief HEADER EXTENSION submessage ID */
#define RTPS_HEADER_EXTN_KIND     (0x00)

/*ci \brief Length of RTPS submessage header */
#define RTPS_SUBMESSAGE_HEADER_LENGTH   4

/*ci \brief Return TRUE if the submessage kind is vendor specific
 */
#define RTPS_Submsg_is_kind_vendor(kind_) ((kind_) & 0x80)

/*ci \brief Alignment requirement for a submessage length except for the last
 *          submessage.
 */
#define RTPS_SUBMESSAGE_LENGTH_BYTE_ALIGN (4)

/*ci \brief RTPS submessage header 
 * 
 * \details 
 * Fields ordered exactly as defined by RTPS specification 
  */
struct RTPS_SubmsgHdr
{
    /*ci \brief Submessage kind ID */
    RTI_UINT8 kind;

    /*ci \brief Submessage flags */
    RTI_UINT8 flags;

    /*ci \brief Submessage length */
    RTI_UINT16 length;
};

/*ci \brief Submessage little endianness flag */
#define RTPS_SUBMSG_FLAG_E 0x1

/******************************************************************************/
/*ci \brief ACKNACK submessage, over-the-wire layout */
struct RTPS_ACKNACK 
{
    /*ci \brief Submesage header */
    struct RTPS_SubmsgHdr hdr;

    /*ci \brief Source reader's entity ID */
    RTPS_Entity_T reader;

    /*ci \brief Destination writer's entity ID */
    RTPS_Entity_T writer;

    /*ci \brief Bitmap (variable length) */
    struct RTPS_Bitmap bitmap;

    /*ci \brief Epoch count */
    RTI_UINT32 count;
};

/*ci \brief ACKNACK Final flag */
#define RTPS_ACKNACKFLAGS_F 0x2

/******************************************************************************/
/*ci \brief RTPS DATA submessage 
 * 
 * \details 
 * Fixed length fields declared in type.  Variable length fields (inline QoS, 
 * DATA payload) accounted for later when setting DATA submessage. 
  */
struct RTPS_DATA 
{
    /*ci \brief Submessage header */
    struct RTPS_SubmsgHdr hdr;

    /*ci \brief Submessage flags */
    RTI_UINT16 flags;

    /*ci \brief Inline Qos offset */
    RTI_UINT16 qos_offset;

    /*ci \brief Destination reader entity ID */
    RTPS_Entity_T reader;

    /*ci \brief Source writer entity ID */
    RTPS_Entity_T writer;

    /*ci \brief Sequence number */
    struct REDA_SequenceNumber sn; 
};

/*ci \brief Default empty DATA submessage flag */
#define RTPS_FLAGS_NONE  0x0

/*ci \brief Inline Qos Q-flag for DATA submessage */
#define RTPS_DATAFLAGS_Q 0x2

/*ci \brief Data D-flag for DATA submessage */
#define RTPS_DATAFLAGS_D 0x4

/*ci \brief Data K-flag for DATA submessage */
#define RTPS_DATAFLAGS_K 0x8

/*ci \brief Default Inline Qos offset */
#define RTPS_DATA_INLINEQOS_OFFSET 16

#if RTPS_DATA_BATCH_ENABLED
/******************************************************************************/
/*ci \brief RTPS DATA_BATCH submessage
 *
 * \details
 * Fixed length fields declared in type.  Variable length fields (inline QoS,
 * DATA payload) accounted for later when setting DATA_BATCH submessage.
 */
struct RTPS_DATA_BATCH
{
    /*ci \brief Submessage header */
    struct RTPS_SubmsgHdr hdr;

    /*ci \brief Submessage flags */
    RTI_UINT16 flags;

    /*ci \brief Inline Qos offset */
    RTI_UINT16 qos_offset;

    /*ci \brief Destination reader entity ID */
    RTPS_Entity_T reader;

    /*ci \brief Source writer entity ID */
    RTPS_Entity_T writer;

    /*ci \brief Batch sequence number */
    struct REDA_SequenceNumber batch_sn;

    /*ci \brief First sample sequence number */
    struct REDA_SequenceNumber first_sample_sn;

    /*ci \brief Offset to last sample sn */
    RTI_UINT32 offset_last_sn;

    /*ci \brief Batch sample count */
    RTI_UINT32 batch_sample_count;

    /*ci \brief Octets to SL Encapsulation Id */
    RTI_UINT32 encapsulation_offset;
};

/*ci \brief Timestamp T-flag for DATA_BATCH submessage */
#define RTPS_DATABATCHFLAGS_T 0x1

/*ci \brief Inline Qos Q-flag for DATA_BATCH submessage */
#define RTPS_DATABATCHFLAGS_Q 0x2

/*ci \brief OffsetSN O-flag for DATA_BATCH submessage */
#define RTPS_DATABATCHFLAGS_O 0x4

/*ci \brief Data D-flag for DATA_BATCH submessage */
#define RTPS_DATABATCHFLAGS_D 0x8

/*ci \brief Invalid I-flag for DATA_BATCH submessage */
#define RTPS_DATABATCHFLAGS_I 0x10

/*ci \brief Seralized Key K-flag for DATA_BATCH submessage */
#define RTPS_DATABATCHFLAGS_K 0x20
#endif /* RTPS_DATA_BATCH_ENABLED */

/******************************************************************************/
/*ci \brief GAP submessage */
struct RTPS_GAP 
{
    /*ci \brief Submessage header */
    struct RTPS_SubmsgHdr hdr;

    /*ci \brief Destination reader entity ID */
    RTPS_Entity_T reader;

    /*ci \brief Source writer entity ID */
    RTPS_Entity_T writer;

    /*ci \brief Sequence number of start of GAP */
    struct REDA_SequenceNumber sn_start;

    /*ci \brief Bitmap */
    struct RTPS_Bitmap bitmap; /* variable length */
};

/******************************************************************************/
/*ci \brief HEARTBEAT submessage */
struct RTPS_HEARTBEAT 
{
    /*ci \brief Submessage header */
    struct RTPS_SubmsgHdr hdr;

    /*ci \brief Destination reader entity ID */
    RTPS_Entity_T reader;

    /*ci \brief Source writer entity ID */
    RTPS_Entity_T writer;

    /*ci \brief First sequence number */
    struct REDA_SequenceNumber sn_first;
    
    /*ci \brief Last sequence number */ 
    struct REDA_SequenceNumber sn_last; 

    /*ci \brief Epoch count */
    RTI_UINT32 count;
};

/******************************************************************************/
/*ci \brief HEARTBEAT_BATCH submessage */
struct RTPS_HEARTBEAT_BATCH
{
    /*ci \brief Submessage header */
    struct RTPS_SubmsgHdr hdr;

    /*ci \brief Destination reader entity ID */
    RTPS_Entity_T reader;

    /*ci \brief Source writer entity ID */
    RTPS_Entity_T writer;

    /*ci \brief First batch sequence number */
    struct REDA_SequenceNumber sn_batch_first;

    /*ci \brief Last batch sequence number */
    struct REDA_SequenceNumber sn_batch_last;

    /*ci \brief First sequence number */
    struct REDA_SequenceNumber sn_first;

    /*ci \brief Last sequence number */
    struct REDA_SequenceNumber sn_last;

    /*ci \brief Epoch count */
    RTI_UINT32 count;
};

/*ci \brief Final HEARTBEAT submesasge flag */
#define RTPS_HBFLAGS_F 0x02

/*ci \brief Liveliness HEARTBEAT submessage flag */
#define RTPS_HBFLAGS_L 0x04

/******************************************************************************/
/*ci \brief INFO_DST submessage */
struct RTPS_INFO_DST 
{
    /*ci \brief Submessage header */
    struct RTPS_SubmsgHdr hdr;

    /*ci \brief Destination GUID prefix */
    RTPS_GuidPrefix_T guid_prefix;
};

/******************************************************************************/
/*ci \brief INFO_TS submessage */
struct RTPS_INFO_TS 
{
    /*ci \brief Submessage header */
    struct RTPS_SubmsgHdr hdr;

    /*ci \brief Source timestamp */
    struct RTPS_Time timestamp;
};

/*ci \brief Invalid Timestamp INFO_TS flag */
#define RTPS_INFO_TSFLAGS_I (0x2)

/******************************************************************************/
/*ci \brief CRC32 submessage */
struct RTPS_CRC32
{
    /*ci \brief Submessage header */
    struct RTPS_SubmsgHdr hdr;

    /*ci \brief Message length */
    RTI_UINT32  length;

    /*ci \brief 32 bit CRC */
    RTI_UINT32  checksum;
};

/*ci \brief Length in bytes of RTPS CRC32 submessage */
#define RTPS_RTI_CRC32_LENGTH  8


/******************************************************************************/
/*ci \brief CRC32 submessage */
struct RTPS_HEADER_EXT
{
    /*ci \brief Submessage header */
    struct RTPS_SubmsgHdr hdr;

    /*ci \brief Message length */
    RTI_UINT32  length;

    /*ci \brief upto 16 byte checksum. NOTE that it is not
     *   allowed to use sizeof(struct RTPS_HEADER_EXT) anywhere because
     *   the actual size depends on the bits.
     */
    RTI_UINT32  checksum[4];
};

/*ci \brief The HeaderExtension MessageLength flag, L-bit
 */
#define RTPS_HEADER_EXTFLAG_L (0x2U)

/*ci \brief The HeaderExtension Timestamp flag, T-bit
 */
#define RTPS_HEADER_EXTFLAG_T (0x4U)

/*ci \brief The HeaderExtension UExtension4 flag, U-bit
 */
#define RTPS_HEADER_EXTFLAG_U (0x8U)

/*ci \brief The HeaderExtension WExtension4 flag, W-bit
 */
#define RTPS_HEADER_EXTFLAG_W (0x10U)

/*ci \brief The HeaderExtension Checksum flag (2), C-bits
 */
#define RTPS_HEADER_EXTFLAG_C (0x60U)

/*ci \brief The position of the first C-bit in the submessage header flags
 */
#define RTPS_HEADER_EXTFLAG_C_POS (5U)

/*ci \brief The HeaderExtension ParameterList flag, P-bit
 */
#define RTPS_HEADER_EXTFLAG_P (0x80U)

/*ci \brief The length of the PID header in the HeaderExtension
 */
#define RTPS_HEADER_EXTPID_HDR_SIZE (4)

/******************************************************************************/
/*ci \brief RTPS headers and submessages 
 * 
 * \details 
 * Used to typecast serialized/deserialized stream buffer 
 */
union RTPS_MESSAGES
{
    /*ci \brief Submessage header */
    struct RTPS_SubmsgHdr submsg;

    /*ci \brief Message header */
    struct RTPS_Header header;

    /*ci \brief DATA submessage */
    struct RTPS_DATA data;

#if RTPS_DATA_BATCH_ENABLED
    /*ci \brief DATA_BATCH submessage */
    struct RTPS_DATA_BATCH data_batch;
#endif /* RTPS_DATA_BATCH_ENABLED */

    /*ci \brief GAP submessage */
    struct RTPS_GAP gap;

    /*ci \brief HEARTBEAT submessage */
    struct RTPS_HEARTBEAT hb;

    /*ci \brief HEARTBEAT_BATCH submessage */
    struct RTPS_HEARTBEAT_BATCH hb_batch;

    /*ci \brief ACKNACK submessage */
    struct RTPS_ACKNACK acknack;
  
    /*ci \brief INFO_TS submessage */
    struct RTPS_INFO_TS info_ts;

    /*ci \brief INFO_DST submessage */
    struct RTPS_INFO_DST info_dst;

    /*ci \brief Core 32bit CRC submessage */
    struct RTPS_CRC32 crc32;

    /*ci \brief header ext submessage, maximum size */
    struct RTPS_HEADER_EXT header_ext;
};

/******************************************************************************/
/*ci \brief RTPS interface mode */
typedef enum
{
    /*ci \brief Default undefined mode */
    RTPS_INTERFACEMODE_UNDEFINED,

    /*ci \brief Writer mode */
    RTPS_INTERFACEMODE_WRITER,

    /*ci \brief Reader mode */
    RTPS_INTERFACEMODE_READER,

    /*ci \brief External receiver mode */
    RTPS_INTERFACEMODE_EXTERNAL_RECEIVER
} RTPS_InterfaceMode_t;

/*ci \brief RTPS interface property */
struct RTPS_InterfaceProperty
{
    /*ci \brief Base NETIO interface property */
    struct NETIO_InterfaceProperty _parent;

    /*ci \brief RTPS interface mode */
    RTPS_InterfaceMode_t mode;

    /*ci \brief RTPS interface GUID address */
    struct NETIO_Address intf_address;

    /*ci \brief Maximum number of peers */
    RTI_INT32 max_peer_count;

    /*ci \brief RTI_TRUE for reliable writer or reader */
    RTI_BOOL reliable;

    /*ci \brief RTI_TRUE for anonymous writer or reader */
    RTI_BOOL anonymous;

    /*ci \brief HEARTBEAT period for reliable writer */
    struct OSAPI_NtpTime hb_period; 

    /*ci \brief NACK period for reliable reader. Currently only used for the
     *   preemptive NACK period.
     */
    struct OSAPI_NtpTime nack_period;

    /*ci \brief Piggyback HEARTBEAT rate for reliable writer 
     * 
     * \details 
     * A HEARTBEAT submessage is appended to every Nth new DATA submessage 
     * sent, where N is sample_per_hb. 
      */
    RTI_SIZE_T samples_per_hb;

    /*ci \brief Maximum window size for writer or reader */
    RTI_UINT32 max_window_size;

    /*ci \brief Maximum HEARTBEAT retries for reliable writer 
     * 
     * \details 
     * A peer reader must respond to a reliable writer's HEARTBEATs to be 
     * considered active. If N periodic HEARTBEATs have been sent without 
     * reciving an ACKNACK, where N is maxhb_retries, the peer reader is 
     * then considered inactive and will stay inactive until an ACKNACK is 
     * received.
     */
    RTI_INT32 max_hb_retries;

    /*ci \brief Maximum number of samples of upstream interface */
    RTI_INT32 max_samples;

    /*ci \brief checksum to send data with
     */
    RTI_UINT16 computed_crc_kind;

    /*ci \brief checksums that are supported. This may be a subset of the
     *          supported checksums.
     */
    RTI_UINT16 allowed_crc_mask;

    /*ci \brief Whether to check to received or not if present. If not
     *          present, pass upstream.
     */
    RTI_BOOL check_crc;

    /*ci \brief Whether to drop messages without a CRC or not.
     */
    RTI_BOOL require_crc;

#if OSAPI_ENABLE_TRACE
    const char *session_name;
#endif
};

#if OSAPI_ENABLE_TRACE
#define RTPS_SESSION_NAME_INIT ,NULL
#else
#define RTPS_SESSION_NAME_INIT
#endif

/*ci \brief Unlimited max HEARTBEAT retries */
#define RTPS_RETRIES_UNLIMITED (-1)

/*ci \brief Maximum receive window size */
#define RTPS_RECEIVE_WINDOW_MAX_SIZE (256)

/*ci \brief Default number of maximum peers */
#define DEFAULT_MAX_PEER_COUNT 4

/*ci \brief Default hearbeat period */
#define DEFAULT_HB_PERIOD {3,0}

/*ci \brief Default NACK period in NTP time (sec,fractions of a nanosecond)
 */
#define DEFAULT_NACK_PERIOD {0,4 * 50 * 1000000}

/*ci \brief Default window_size */
#define DEFAULT_WINDOW_SIZE 256

/*ci \brief Default max samples */
#define DEFAULT_MAX_SAMPLES 256

/*ci \brief Default RTPS interface property initializer */
#define RTPS_InterfaceProperty_INITIALIZER \
{\
    NETIO_InterfaceProperty_INITIALIZER,\
    RTPS_INTERFACEMODE_UNDEFINED,\
    NETIO_Address_INITIALIZER, \
    4, /* max_peer_count */ \
    RTI_FALSE, /* reliable */ \
    RTI_FALSE, /* anonymous */ \
    DEFAULT_HB_PERIOD, /* hb_period */ \
    DEFAULT_NACK_PERIOD, /* nack_period */ \
    8, /* samples_per_hb */ \
    256, /* window_size */ \
    RTPS_RETRIES_UNLIMITED, /* max_hb_retries */\
    256, /* max_samples */ \
    0U, \
    0U, \
    RTI_FALSE, \
    RTI_FALSE \
    RTPS_SESSION_NAME_INIT \
}

/*i \brief Property for an RTPS route */
struct RTPS_RouteProperty 
{
    /*ci \brief Base NETIO route property */
    struct NETIORouteProperty _parent; 

    /*ci \brief Reliable flag of peer reachable over route */
    RTI_BOOL reliable;

    /*ci \brief First sequence number of upstream interface creating the 
     *   route
     */
    struct REDA_SequenceNumber first_sn;

    /*ci \brief Last sequence number of upstream interface creating the 
     *   route
     */
    struct REDA_SequenceNumber last_sn;

    /*ci \brief Last acknowledged sequence number of upstream interface 
     *   creating the route
     */
    struct REDA_SequenceNumber last_acked_sn;
};

/*ci \brief Default RTPS route property initializer */
#define RTPSRouteProperty_INITIALIZER \
{\
    NETIORouteProperty_INITIALIZER, \
    RTI_FALSE, /* reliable */ \
    REDA_SEQUENCE_NUMBER_ZERO,\
    REDA_SEQUENCE_NUMBER_ZERO, \
    REDA_SEQUENCE_NUMBER_ZERO \
}

/*e \dref_RTPS_InterfaceFactoryProperty
 */
typedef struct RTPS_InterfaceFactoryProperty
{
    /*ci \brief Base-class property
     */
    struct NETIO_InterfaceFactoryProperty _parent;

    /*e \dref_RTPS_InterfaceFactoryProperty_checksum
     */
    struct RTPS_ChecksumProperty checksum;
} RTPS_InterfaceFactoryProperty_T;

/*ci \brief Initializer for a RTPS InterfaceFactoryProperty
 */
#define RTPS_InterfaceFactoryProperty_INITIALIZER \
{\
    NETIO_InterfaceFactoryProperty_INITIALIZER,\
    RTPS_ChecksumProperty_INITIALIZER \
}

/*e \dref_RTPS_INTERFACE_FACTORY_DEFAULT
 */
extern RTPSDllVariable
const struct RTPS_InterfaceFactoryProperty RTPS_INTERFACE_FACTORY_DEFAULT;

/*ce \dref_RTPS_InterfaceFactory_get_interface
 */
RTPSDllExport struct RT_ComponentFactoryI*
RTPS_InterfaceFactory_get_interface(void);

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#include "rtps/rtps_rtps_impl.h"

#endif /* rtps_rtps_h */

/*ci @} */

