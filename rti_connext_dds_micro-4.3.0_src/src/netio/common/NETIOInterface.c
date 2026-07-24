/*
 * FILE: NETIOInterface.c - NETIO interface implementation
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
 * 04nov2015,tk MICRO-1505 Reduce memory footprint
 * 18feb2015,tk MICRO-1070/PR#13963 Fixed hex conversion in name_from_id
 * 31jul2014,tk MICRO-842/PR#9683 - Removed superfluous paramaters in DB
 *                                  compare function
 * 05may2014,tk MICRO-72 - Updated based on CR-232
 * 27apr2012,tk Written
 */
/*ce
 * \file
 * \brief NETIO interface implementation
 */
#include "osapi/osapi_config.h"

#ifndef osapi_string_h
#include "osapi/osapi_string.h"
#endif
#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef db_api_h
#include "db/db_api.h"
#endif
#ifndef rt_rt_h
#include "rt/rt_rt.h"
#endif
#ifndef netio_log_h
#include "netio/netio_log.h"
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

const char* const NETIO_DEFAULT_UDP_NAME = "_udp";

const char* const NETIO_DEFAULT_INTRA_NAME = "_intra";

const char* const NETIO_DEFAULT_SHMEM_NAME = "_shmem";

const char* const NETIO_DEFAULT_RTPS_NAME = "rtps";

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Compare entries in the database of route entries. The function is
 *        compatible with \ref DB_IndexCompare_T
 *
 * \param[in] flags Used to distinguish between op2 being a key or record
 * \param[in] op1   A NETIORouteEntryKey already in the database
 * \param[in] op2   Either a NETIORouteEntryKey being added or a key
 *                  being searched for
 *
 * \return positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2,
 *         zero if op1 is equal to op2
 */
RTI_INT32
NETIO_Interface_compare_route(RTI_INT32 flags,const DB_Record_T op1, void *op2)
{
    RTI_INT32 diff;
    struct NETIORouteEntryKey *lkey = (struct NETIORouteEntryKey*)op1;
    struct NETIORouteEntryKey *rkey = (struct NETIORouteEntryKey*)op2;
    UNUSED_ARG(flags);

    diff = NETIO_Address_compare(&lkey->destination,&rkey->destination);
    if (diff != 0)
    {
        return diff;
    }

    if (lkey->intf > rkey->intf)
    {
        return 1;
    }

    if (lkey->intf < rkey->intf)
    {
        return -1;
    }

    return  NETIO_Address_compare(&lkey->intf_address,&rkey->intf_address);
}

/*ci
 * \brief Compare entries in the database of bind entries. The function is
 *        compatible with \ref DB_IndexCompare_T
 *
 * \param[in] flags Used to distinguish between op2 being a key or record
 * \param[in] op1   A NETIOBindEntryKey already in the database
 * \param[in] op2   Either a NETIOBindEntryKey being added or a key
 *                  being searched for
 *
 * \return positive integer if op1 is greater than op2,
 *         negative integer if op1 is less than op2,
 *         zero if op1 is equal to op2
 */
RTI_INT32
NETIO_Interface_compare_bind(RTI_INT32 flags,const DB_Record_T op1, void *op2)
{
    RTI_INT32 diff;
    struct NETIOBindEntryKey *lkey = (struct NETIOBindEntryKey*)op1;
    struct NETIOBindEntryKey *rkey = (struct NETIOBindEntryKey*)op2;
    UNUSED_ARG(flags);

    diff = NETIO_Address_compare(&lkey->source,&rkey->source);
    if (diff != 0)
    {
        return diff;
    }

    return  NETIO_Address_compare(&lkey->destination,&rkey->destination);
}

void
NETIO_Interface_Table_name_from_id(char *tbl_name,
                                   union RT_ComponentFactoryId *id,
                                   char suffix,RTI_INT32 instance)
{
    RTI_INT32 i,j,d;
    static const char hex[] = "0123456789abcdef";

    i = 0;
    while (id->_name._name[i])
    {
        tbl_name[i] = id->_name._name[i];
        ++i;
    }

    tbl_name[i++] = suffix;
    for (j = 28; (j >= 0); j -= 4, ++i)
    {
        d = ((instance >> j) & 0xf);

        /* Since  0 <= d <= 15 it cannot exceed the table of 17 entries
         */
         /* coverity[cert_str31_c_violation] */
        tbl_name[i] = hex[d];
    }
    tbl_name[i] = 0;
}

RTI_BOOL
NETIO_Interface_initialize(struct NETIO_Interface *netio,
                          struct NETIO_InterfaceI *netio_intf,
                          const struct NETIO_InterfaceProperty *const property,
                          const struct NETIO_InterfaceListener *const listener)
{
    RT_Component_initialize(&netio->_parent,
                           &netio_intf->_parent,
                           0,
                           (property ? &property->_parent : NULL),
                           (listener ? &listener->_parent : NULL));

    netio->_rtable = NULL;
    netio->_btable = NULL;

    return RTI_TRUE;
}

#ifndef RTI_CERT
RTI_BOOL
NETIO_Interface_finalize(struct NETIO_Interface *netio)
{
    RT_Component_finalize(&netio->_parent);

    return RTI_TRUE;
}
#endif /* !RTI_CERT */


