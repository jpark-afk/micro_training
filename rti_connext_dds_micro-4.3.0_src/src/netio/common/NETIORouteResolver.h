/*
 * FILE: NETIORouteResolver.h - NETIO Route implementation
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
 * 05may2014,tk MICRO-72 - Updated based on CR-232
 * 16sep2012,tk Written
 */
/*ci
 * \file
 * \brief NETIO Route implementation
 */
/*ci \addtogroup NETIO_RouteClass
 *  @{
 */
#ifndef NETIORouteResolver_h
#define NETIORouteResolver_h

/*ci
 * \brief Route table entry
 */
struct NETIO_RouteResolverRecord
{
    /*ci
     * \brief The address of the interface
     */
    struct NETIO_Address address;

    /*ci
     * \brief The netmask to apply to an address before comparison
     */
    struct NETIO_Netmask netmask;

    /*ci
     * \brief The interface which can reach the address
     */
    NETIO_Interface_T *intf;

    RT_ComponentFactoryId_T id;
};

/*ci
 * \brief Constant to initialize \ref NETIO_RouteResolverRecord
 */
#define NETIO_RouteResolverRecord_INITIALIZER \
{\
    NETIO_Address_INITIALIZER,\
    NETIO_Netmask_INITIALIZER, \
    NULL,\
    RT_ComponentFactoryId_INITIALIZER\
}

/*ci
 * \brief The NETIO_RouteResolver resolver class
 */
struct NETIO_RouteResolver
{
    /*ci
     * \brief The properties the route resolver was created with
     */
    struct NETIO_RouteResolverProperty property;

    /*ci
     * \brief Table with route entries
     */
    DB_Table_T route_table;

    /*ci
     * \brief The database to create the route table in
     */
    DB_Database_T db;

    /*ci
     * \brief The address resolver to parse address strings
     */
    NETIO_AddressResolver_T *nar;

    /*ci
     * \brief Index of supported locator kinds
     */
    DB_Index_T kind_index;
};

#endif

/*ci @} */
