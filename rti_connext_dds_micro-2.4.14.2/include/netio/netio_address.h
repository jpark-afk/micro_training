/*
 * FILE: netio_address.h - NETIO Address API
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
 * 27may2022,am MICRO-3537/PR.30409
 * - Removed declaration of NETIO_Address_is_udpv4_multicast 
 *   since its no longer used.
 * - Removed NETIO_ADDRESS_KIND_UDPv6 as it is no longer used.
 * - Conditionally define NETIO_ADDRESS_KIND_SHMEM 
 *   only when UDP transforms is enabled. 
 * - Removed the following constants when builtin UDP is disabled
 *   NETIO_ADDRESS_KIND_UDPv4
 *   NETIO_ADDRESS_KIND_USER
 *   NETIO_ADDRESS_KIND_TUDPv4
 * 11mar2021, am MICRO-3515
 * - Remove NETIO_Address_set_ipv4 when builtin udp is excluded.
 * 15apr2021,tk MICRO-2979/PR.27549
 * - Removed unused functions from CERT:
 *   NETIO_Address_set_multicast
 *   NETIO_Address_set_internal
 *   NETIO_Address_kind
 *   NETIO_Address_set_guid_from_int32
 * 30jun2015,tk MICRO-1378/PR#15203 Updated comments
 * 19may2015,as MICRO-1193 Refactoring of Sequence API levels
 * 05may2014,tk MICRO-72 - Updated based on CR-232
 * 25apr2012,tk Written
 */
/*ci
 * \file
 * \brief NETIO Address API
 * \defgroup NETIOAddressClass NETIO Address API
 * \ingroup NETIOCommon
 *
 * \details
 *
 * This file contains functions to manipulate NETIO addresses.
 */
/*ci \addtogroup NETIOAddressClass
 * @{
 */
#ifndef netio_address_h
#define netio_address_h

#ifndef netio_dll_h
#include "netio/netio_dll.h"
#endif
#ifndef osapi_config_h
#include "osapi/osapi_config.h"
#endif
#ifndef osapi_types_h
#include "osapi/osapi_types.h"
#endif
#ifndef reda_sequence_h
#include "reda/reda_sequence.h"
#endif
#ifndef db_db_h
#include "db/db_api.h"
#endif
#ifndef rt_rt_h
#include "rt/rt_rt.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*ci
 * \def NETIO_ADDRESS_PORT_INVALID
 * \brief Constant for an invalid port number
 */
#define NETIO_ADDRESS_PORT_INVALID   (0)

/*ci
 * \def NETIO_ADDRESS_KIND_INVALID
 * \brief Constant for an invalid address kind
 */
#define NETIO_ADDRESS_KIND_INVALID   (-1)

/*ci
 * \def NETIO_ADDRESS_KIND_RESERVED
 * \brief Constant reserved for internal address kinds.
 */
#define NETIO_ADDRESS_KIND_RESERVED  0

/*ci
 * \def NETIO_ADDRESS_KIND_UDPv4
 * \brief Constant reserved for UDPv4 address kinds
 */
#define NETIO_ADDRESS_KIND_UDPv4     1

/*ci
 * \def NETIO_ADDRESS_KIND_UDPv6
 * \brief Constant reserved for UDPv6 address kinds
 */
#define NETIO_ADDRESS_KIND_UDPv6     2

/*ci
 * \def NETIO_ADDRESS_KIND_USER
 * \brief User defined (not in spec) locator kind shall 
 * be greater or equal than this constant
 */
#define NETIO_ADDRESS_KIND_USER      0x01000000

/*ci
 * \def NETIO_ADDRESS_KIND_SHMEM
 * \brief Constant reserved for shared memory addresses
 */
#define NETIO_ADDRESS_KIND_SHMEM     0x01000000

/*ci
 * \def NETIO_ADDRESS_KIND_INTRA
 * \brief Constant reserved for INTRA addresses
 */
#define NETIO_ADDRESS_KIND_INTRA     0x01001000

/*ci
 * \def NETIO_ADDRESS_KIND_TUDPv4
 * \brief Constant reserved for transformed UDP payloads
 */
#define NETIO_ADDRESS_KIND_TUDPv4    0x01001001

/*ci
 * \def NETIO_ADDRESS_MAX_32BIT
 * \brief The maximum number of 32bit values in an address
 */
#define NETIO_ADDRESS_MAX_32BIT (4)

/*ci
 * \def NETIO_ADDRESS_MAX_8IT
 * \brief The maximum number of 8bit values in an address
 */
#define NETIO_ADDRESS_MAX_8IT   (16)

/*ci
 * \brief 12 byte GUID prefix, can used as part of a complete GUID address
 */
struct NETIO_GuidPrefix
{
    RTI_UINT8 value[12];
};

/*ci
 * \brief 4 byte GUID entity part, can used as part of a complete GUID address
 */
struct NETIO_GuidEntity
{
    RTI_UINT8 value[4];
};

/*ci
 * \brief Address in RTPS format, commonly used by RTI internally
 */
struct NETIO_AddressRtps
{
    RTI_UINT32 host_id;
    RTI_UINT32 app_id;
    RTI_UINT32 instance_id;
    RTI_UINT32 object_id;
};

/*ci
 * \brief Address in GUID format
 */
struct NETIO_Guid
{
    struct NETIO_GuidPrefix prefix;
    struct NETIO_GuidEntity entity;
};

/*ci
 * \brief Address in GUID format, but with individual bytes accessible
 */
struct NETIO_AddressGuidPrefix
{
    RTI_UINT8 prefix[12];
    RTI_UINT32 entity;
};

/*ci
 * \brief Address in UDPv4 format. By internal convention, only the 4 most
 *        significant bytes are used.
 */
struct NETIO_AddressIp4
{
    /*ci
     * \brief 32 bit IPv4 address in host order
     */
    RTI_UINT32 address;
    RTI_UINT8 _unused[12];
};

/*ci
 * \brief Address in UDPv6 format. By internal convention
 */
struct NETIO_AddressIpv6
{
    RTI_UINT8 octet[16];
};

/*ci
 * \brief Address in shared memory format. By internal convention
 */
struct NETIO_AddressShem
{
    /*ci
     * \brief shared memory key
     */
    RTI_UINT32 key;
    RTI_UINT8 guid[12];
};

/*ci
 * \brief Convenience structure to initialize an \ref NETIO_Address structure
 */
struct NETIO_AddressInit
{
    RTI_UINT32 val0;
    RTI_UINT32 val1;
    RTI_UINT32 val2;
    RTI_UINT32 val3;
};

/*ci
 * \brief Convenience structure to access \ref NETIO_Address as an array of 4
 *        signed integers
 */
struct NETIO_AddressInt32
{
    RTI_INT32 value[4];
};

/*ci
 * \brief Convenience structure to access \ref NETIO_Address as an array of 4
 *        unsigned integers
 */
struct NETIO_AddressUInt32
{
    RTI_UINT32 value[4];
};

/*ci
 * \brief All the different address formats
 */
union NETIO_AddressValue
{
    struct NETIO_AddressRtps rtps_guid;
    struct NETIO_Guid guid;
    struct NETIO_AddressGuidPrefix guid_prefix;
    struct NETIO_AddressIp4 ipv4;
    struct NETIO_AddressIpv6 ipv6;
    struct NETIO_AddressShem shmem;
    struct NETIO_AddressInit init;
    struct NETIO_AddressInt32 as_int32;
    struct NETIO_AddressUInt32 as_uint32;
};

/*ci
 * \brief The definition of the NETIO address structure
 */
struct NETIO_Address
{
    /*ci
     * \brief Address discriminator. Note that the 8 MSB of the kind
     *        is reserved for internal use
     */
    RTI_INT32 kind;

    /*ci
     * \brief Port number
     */
    RTI_UINT32 port;

    /*ci
     * \brief Address as defined by the discriminator
     */
    union NETIO_AddressValue value;
};

/*ci
 * \def NETIO_ADDRESS_FLAG_MULTICAST
 * \brief Flags to indicate is an address is multicast
 */
#define NETIO_ADDRESS_FLAG_MULTICAST     (0x80000000)

/*ci
 * \def NETIO_ADDRESS_FLAG_INTERNAL
 * \brief Flags to indicate is an address is an internal address
 */
#define NETIO_ADDRESS_FLAG_INTERNAL      (0x40000000)

/*ci
 * \brief Check if a \ref NETIO_Address is a multicast address
 *
 * \param[in] addr Address to check
 *
 * \return RTI_TRUE is the address is multicast, RTI_FALSE if not
 */
NETIODllExport RTI_BOOL
NETIO_Address_is_multicast(const struct NETIO_Address *const addr);

/*ci
 * \brief Check if a \ref NETIO_Address is an internal address
 *
 * \param[in] addr Address to check
 *
 * \return RTI_TRUE is the address is internal, RTI_FALSE if not
 */
NETIODllExport RTI_BOOL
NETIO_Address_is_internal(const struct NETIO_Address *const addr);

#ifndef RTI_CERT
/*ci
 * \brief Mark a \ref NETIO_Address as a multicast address
 *
 * \param[in] addr Address to mark
 */
NETIODllExport void
NETIO_Address_set_multicast(struct NETIO_Address *const addr);
#endif

#ifndef RTI_CERT
/*ci
 * \brief Mark a \ref NETIO_Address as an internal address
 *
 * \param[in] addr Address to mark
 */
NETIODllExport void
NETIO_Address_set_internal(struct NETIO_Address *const addr);
#endif

/*ci
 * \brief Set the \ref NETIO_Address kind
 *
 * \param[in] addr  Address to set kind of
 * \param[in] kind  Address kind
 * \param[in] flags Address flags such as internal or multicast
 */
NETIODllExport void
NETIO_Address_set_kind(struct NETIO_Address *const addr,RTI_UINT32 kind,RTI_UINT32 flags);

/*ci
 * \brief Get the \ref NETIO_Address kind
 *
 * \param[in] addr  Address to get kind of
 *
 * \return The address kind
 */
NETIODllExport RTI_INT32
NETIO_Address_get_kind(const struct NETIO_Address *const addr);

#ifndef RTI_CERT
/*ci
 * \brief Convert a \ref NETIO_Address::kind field to a valid kind
 *
 * \param[in] kind Kind to mask
 *
 * \return The kind
 */
NETIODllExport RTI_INT32
NETIO_Address_kind(RTI_INT32 kind);
#endif

/*ci
 * \def NETIO_NETMASK_MASK_LENGTH
 * \brief The maximum number of elements in a netmask
 */
#define NETIO_NETMASK_MASK_LENGTH        (4U)

/*ci
 * \def NETIO_NETMASK_MASK_BITS_PER_UNIT
 * \brief The number of bits per netmask element
 */
#define NETIO_NETMASK_MASK_BITS_PER_UNIT ((RTI_UINT32)(sizeof(RTI_INT32) * 8U))

/*ci
 * \brief The maximum number of bits in a netmask
 */
#define NETIO_NETMASK_MASK_MAX_BITS \
                (NETIO_NETMASK_MASK_LENGTH * NETIO_NETMASK_MASK_BITS_PER_UNIT)

/*ci
 * \brief Representation of a netmask
 */
struct NETIO_Netmask
{
    /*ci
     * \brief The number of valid bits in the netmask
     */
    RTI_UINT32 bits;

    /*ci
     * \brief The netmask
     */
    RTI_UINT32 mask[NETIO_NETMASK_MASK_LENGTH];
};

/*ci
 * \brief \ref NETIO_Netmask initializer
 */
#define NETIO_Netmask_INITIALIZER \
{\
    0,\
    {0,0,0,0}\
}

/* The NETIO API uses both individual NETIO_Addresses and NETIO_Netmask
 * field, as well as sequences of these. Thi declares the sequence
 * of the respective types.
 */
#define T struct NETIO_Address
#define TSeq NETIO_AddressSeq
#include <reda/reda_sequence_decl.h>

#define T struct NETIO_Netmask
#define TSeq NETIO_NetmaskSeq
#include <reda/reda_sequence_decl.h>

/*ci
 * \def NETIO_AddressSeq_INITIALIZER
 * \brief Macro to initialize a NETIO_Address sequence
 */
#define NETIO_AddressSeq_INITIALIZER REDA_DEFINE_SEQUENCE_INITIALIZER(struct NETIO_Address)

/*ci
 * \def NETIO_NetmaskSeq_INITIALIZER
 * \brief Macro to initialize a NETIO_Netmask sequence
 */
#define NETIO_NetmaskSeq_INITIALIZER REDA_DEFINE_SEQUENCE_INITIALIZER(struct NETIO_Netmask)

/*ci
 * \def NETIO_ADDRESS_GUID_UNKNOWN
 * \brief Constant used to initialize an NETIO_Address of the GUID kind to
 *        unknown.
 */
#define NETIO_ADDRESS_GUID_UNKNOWN {{{0,0,0,0,0,0,0,0,0,0,0,0}},{{0,0,0,0}}}

/*ci
 * \brief Compare two NETIO_Addresses
 *
 * \details
 *
 * This function compares two NETIO_Address structures.
 *
 * \param[in] left_addr   Left side of the comparison
 * \param[in] right_addr  Right side of the comparison
 *
 * \return positive integer if left_addr is greater than right_addr,
 *         negative integer if left_addr is less than right_addr,
 *         zero if left_addr is equal to right_addr
 */
NETIODllExport RTI_INT32
NETIO_Address_compare(const struct NETIO_Address *left_addr,
                      const struct NETIO_Address *right_addr);

/*ci
 * \def NETIO_Address_INITIALIZER
 * Constant to initialize a \ref NETIO_Address
 */
#define NETIO_Address_INITIALIZER \
{\
    NETIO_ADDRESS_KIND_RESERVED, /* kind */ \
    0, /* port */ \
    {{0,0,0,0}} /* value */ \
}

/*ci
 *  \brief Initialize a \ref NETIO_Address to a known state
 *
 *  \param[inout] addr Address to initialize
 *  \param[in]    kind Address kind
 */
NETIODllExport void
NETIO_Address_init(struct NETIO_Address *addr,RTI_INT32 kind);

#if !UDP_EXCLUDE_BUILTIN
/*ci
 *  \brief Set the IPv4 address part of a \ref NETIO_Address
 *
 *  \param[inout] addr    Address to initialize
 *  \param[in]    port    Address port
 *  \param[in]    address IPv4 address in host order
 */
NETIODllExport void
NETIO_Address_set_ipv4(struct NETIO_Address *addr,RTI_UINT32 port,RTI_UINT32 address);
#endif

/*ci
 *  \brief Set the GUID address part of a \ref NETIO_Address
 *
 *  \param[inout] addr  Address to initialize
 *  \param[in]    port  Address port
 *  \param[in]    guid  GUID to set
 */
NETIODllExport void
NETIO_Address_set_guid(struct NETIO_Address *addr,RTI_UINT32 port,struct NETIO_Guid *guid);

/*ci
 *  \brief Set the GUID address part of a \ref NETIO_Address using an array
 *
 *  \param[inout] addr  Address to initialize
 *  \param[in]    port  Address port
 *  \param[in]    octet_array octet_array to be used as guid
 */
NETIODllExport void 
NETIO_Address_set_guid_from_array(
        struct NETIO_Address* addr, RTI_UINT32 port, RTI_UINT8 *octet_array);



/*ci
 *  \brief Set the GUID address part of a \ref NETIO_Address from a DDS Key.
 *
 *  \details
 *  DDS keys are received over the arrays as an array of 4 integers
 *  as defined by the DDS specification. Because the key is represented as
 *  4 integers in the <em> host order </em> of the sender but are internally
 *  represented as 16 octets (endianess is not an issue), care must be
 *  taken to perform endianness conversion.
 *
 *  \param[inout] addr Address to initialize
 *  \param[in]    port Address port
 *  \param[in]    key  Key to set
 */
NETIODllExport void
NETIO_Address_set_guid_from_key(struct NETIO_Address *addr,RTI_UINT32 port,
                                struct NETIO_AddressInt32 *key);

#ifndef RTI_CERT
/*ci
 *  \brief Set the GUID address part of a \ref NETIO_Address from 4 integers.
 *
 *  \details
 *  A GUID is an array of 16 octets and it is necessary
 *  to perform endianess conversion based on the host's endianess
 *
 *  \param[inout] addr Address to initialize
 *  \param[in]    port Address port
 *  \param[in]    int0  Byte 15-12
 *  \param[in]    int1  Byte 11-8
 *  \param[in]    int2  Byte 7-4
 *  \param[in]    int3  Byte 3-0
 */
NETIODllExport void
NETIO_Address_set_guid_from_int32(struct NETIO_Address *addr,RTI_UINT32 port,
                                  RTI_UINT32 int0,RTI_UINT32 int1,
                                  RTI_UINT32 int2,RTI_UINT32 int3);
#endif

/*ci
 * \brief Parse an address string
 *
 * \details
 *
 * This function parses an NETIO address in stringa format and breaks
 * it into the individual parts.
 *
 * Algorithm:
 *
 * An address is on the form:
 *
 * [index @][<interface>://][<address>]
 *
 * - break address string into 3 parts:
 *   - An optional index. Three index  formats are supported
 *     N@     - In this type the low_index = 0 and high_index is N
 *     [N-M]@ - In this type the low_index = N and high_index is M
 *     M@     - In this type the low_index = high_index = M
 * - An optional interface id (out_id)
 * - An optional address string (address_string)
 *
 * The parse allows an empty string. If an index is not specified -1 is
 * returned. If a prefix is not specified an empty prefix is returned.
 *
 * \param[in]  name               The address to be parsed
 * \param[out] base_port          The base port in used in port calculation
 * \param[out] low_index          The lowest index parsed from the address
 * \param[out] high_index         The highest index parsed from the address
 * \param[out] out_id             The ComponentFactoryId that successfully parsed it
 * \param[out] address_string     The address part of the NETIO address string
 * \param[out] address_max_string The maximum length of the address part
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
MUST_CHECK_RETURN NETIODllExport RTI_BOOL
NETIO_Address_parse(const char *name,
                    RTI_UINT32 *base_port,
                    RTI_INT32 *low_index,
                    RTI_INT32 *high_index,
                    RT_ComponentFactoryId_T *out_id,
                    char *address_string,
                    RTI_SIZE_T address_max_string);

/*ci
 * \brief The maximum size of an individual address part.
 */
#define NETIO_ADDRESS_TOKEN_MAX_SIZE 256

/*ci
 * \brief The maximum size of an an address prefix. The address prefix
 * is limited by the the run-time can handle, which is 7 octets, excluding the
 * NULL termination.
 */
#define NETIO_PREFIX_TOKEN_MAX_SIZE  (RT_MAX_FACTORY_NAME + 1)

/*ci
 * \brief Enumeration of what type of address is being resolved. NETIO
 *        supports 2 types of address, meta and user. It is up to the
 *        caller to determine what the definition of user and meta means.
 */
typedef enum
{
    /*ci
     * \brief The address if for meta traffic
     */
    NETIO_ROUTEKIND_META,

    /*ci
     * \brief The address if for user traffic
     */
    NETIO_ROUTEKIND_USER
} NETIO_RouteKind_T;

/*ci
 * \brief Calculate a port number based on user provided information
 *
 * \details
 *
 * NETIO does not enforce the user of a port calculation function. However,
 * some layers such as DDS defines a port mapping to enable discovery on
 * well-known ports. Based on information provided by the caller
 * the user can calculate specific port numbers which will be used as
 * part of a \ref NETIO_Address.
 *
 * \param[in] param     User provided parameter passed transparently
 * \param[in] kind      The type of address the port is calculated for
 * \param[in] port_base The base-port number, typically used together with an
 *                      index to calculate the final port
 * \param[in] index     The index extracted from an address string
 * \param[out] address  The NETIO_Address structure to update the port for
 */
typedef RTI_BOOL
(*NETIO_PortCalculateFunc_T)(void *param,NETIO_RouteKind_T kind,
                             RTI_UINT32 port_base,RTI_INT32 index,
                             struct NETIO_Address *address);

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif /* netio_address_h */

/*ci @} */

