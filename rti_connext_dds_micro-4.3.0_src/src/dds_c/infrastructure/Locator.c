/*
 * FILE: Locator.c - Locator functions
 *
 * (c) Copyright 2018-2024 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
/*ce
 * \file
 * \brief Locator functions
 */
/*ci \addtogroup DDSInfrastructureModule
 * @{
 */
#include "Locator.h"
#include "Transport.h"

/*ci
 * \brief Copy a locator to a Netio Address
 *
 * \param[in]    loc        DDS Locator passed in
 * \param[inout] addr_out   Netio Address out
 */
void
DDS_Locator_to_netio_address(const struct DDS_Locator *loc,
                             struct NETIO_Address *addr_out)
{
    if (loc != NULL)
    {
        *addr_out = (struct NETIO_Address)NETIO_Address_INITIALIZER;
        NETIO_Address_set_guid_from_array(addr_out, loc->port, loc->address);
        addr_out->kind = loc->kind;
    }
}

/*ci \brief Copy an extended locator to a locator, ignoring the extended fields
 *
 * \param[out]   dst_loc    The locator to copy into
 * \param[in]    src_loc    The extended locator to copy from
 */
void
DDS_Locator_from(struct DDS_Locator *dst_loc,
                 const struct DDS_LocatorEx *src_loc)
{
    dst_loc->kind = src_loc->kind;
    dst_loc->port = src_loc->port;
    OSAPI_Memory_copy(dst_loc->address, src_loc->address,
                      DDS_LOCATOR_ADDRESS_LENGTH_MAX);
}

/*ci \brief Copy an locator to an extended locator, initializing the extended
 *   fields to 0 or NULL.
 *
 * \param[out]   dst_loc    The extened locator to copy into
 * \param[in]    src_loc    The  locator to copy from
 */
void
DDS_LocatorEx_from(struct DDS_LocatorEx *dst_loc,
                   const struct DDS_Locator *src_loc)
{
    dst_loc->kind = src_loc->kind;
    dst_loc->port = src_loc->port;
    OSAPI_Memory_copy(dst_loc->address, src_loc->address,
                      DDS_LOCATOR_ADDRESS_LENGTH_MAX);
    dst_loc->length = 0;
    OSAPI_Memory_zero(dst_loc->encapsulations,
                      sizeof(dst_loc->encapsulations));
}

/*ci \brief Copy a locator to a Netio AddressEx, initializing the extended
 *   fields to 0 or NULL.
 *
 * \param[out]   dst_addr   The Netio AddressEx to copy into
 * \param[in]    src_loc    The  locator to copy from
 */
void
NETIO_AddressEx_from(struct NETIO_AddressEx *dst_addr,
                     const struct DDS_Locator *src_loc)
{
    dst_addr->kind = src_loc->kind;
    dst_addr->port = src_loc->port;
    OSAPI_Memory_copy(&dst_addr->value, src_loc->address, sizeof(dst_addr->value));
    OSAPI_Memory_zero(dst_addr->data, sizeof(dst_addr->data));
}

RTI_BOOL
DDS_Locator_append_locator_kind(const struct DDS_LocatorSeq *in_seq,
                                struct DDS_LocatorSeq *reslvd_seq,
                                RTI_INT32 kind)
{
    RTI_INT32 reslv_index;
    RTI_INT32 loc_length;
    RTI_INT32 loc_index;
    struct DDS_Locator copy_loc = DDS_LOCATOR_INVALID;
    RTI_INT32 loc_kind;

    /* Return all locators of the same kind */
    reslv_index = 0;
    loc_length = DDS_LocatorSeq_get_length(in_seq);

    for (loc_index = 0; loc_index < loc_length; ++loc_index)
    {
        const struct DDS_Locator *a_loc = NULL;
        const struct DDS_LocatorEx *a_ex_loc = NULL;

        if (DDS_LocatorSeq_is_extended(in_seq))
        {
            struct DDS_LocatorExSeq *extended_loc_seq =
                            OSAPI_Compiler_reinterpret_cast(
                                                struct DDS_LocatorExSeq*,
                                                in_seq);
            a_ex_loc = DDS_LocatorExSeq_get_reference(extended_loc_seq,
                                                     loc_index);
            if (a_ex_loc == NULL)
            {
                continue;
            }
            loc_kind = NETIO_Address_kind(a_ex_loc->kind);
        }
        else
        {
            a_loc = DDS_LocatorSeq_get_reference(in_seq,loc_index);
            if (a_loc == NULL)
            {
                continue;
            }
            loc_kind = NETIO_Address_kind(a_loc->kind);
        }

        /* either (a_ex_loc != NULL) OR (a_loc != NULL) here */
        if (loc_kind != kind)
        {
            continue;
        }

        if (!DDS_LocatorSeq_set_length(reslvd_seq,reslv_index + 1))
        {
            return RTI_FALSE;
        }

        if (a_ex_loc != NULL)
        {
            DDS_Locator_from(&copy_loc, a_ex_loc);
        }
        else
        {
            copy_loc = *a_loc;
        }

        *DDS_LocatorSeq_get_reference(reslvd_seq,reslv_index) = copy_loc;
        ++reslv_index;
    }

    return RTI_TRUE;
}

/*ci
 * \brief Resolve which unicast and multicast locators should be used to reach
 *        a remote subscription
 *
 * \details
 *
 * If a subscription specifies its own locators those are used instead of the
 * default locators specified on the participant.
 *
 * \param[in]  parent_data    The remote participant qos
 * \param[in]  data           The remote subscription qos
 * \param[out] uc_locator_seq The unicast locators to use for the subscription
 * \param[out] mc_locator_seq The multicast locators to use for the subscription
 */
RTI_BOOL
DDS_Locator_get_interface(const struct DDS_Locator *locator,
                          RT_ComponentFactoryId_T *name,
                          NETIO_RouteResolver_T *r_table,
                          NETIO_AddressResolver_T *nar)
{
    NETIO_Interface_T *route_intf;

    if (!NETIO_RouteResolver_find_interface(r_table,
                                            &route_intf,
                                            DDS_Locator_as_netioaddress(locator)))
    {
        return RTI_FALSE;
    }

    /* Change to address resolver */
    if (!NETIO_AddressResolver_lookup_name(nar,route_intf,name))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

/*ci @} */
