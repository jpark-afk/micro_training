/*
 * FILE: LOOPInterface.h - LOOPBack interface implementation
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
 * 07jul2012,tk Updated
 * 07jul2012,tk Updated
 * 27apr2012,tk Written
 */
/*ci
 * \file
 * \brief LOOPBack interface implementation
 */
#ifndef LOOPInterface_h
#define LOOPInterface_h

/*ci \addtogroup NETIO_LoopbackInterfaceClass
 * @{
 */
/*ci
 * \brief A loopback route entry
 */
struct LOOP_RouteEntry
{
    /*ci
     * \brief Inherited route entry
     */
    struct NETIORouteEntry _parent;
};

/*ci
 * \brief Loopback interface class
 */
struct LOOP_Interface
{
    /*ci
     * \brief Inherited NETIO interface base-class
     */
    struct NETIO_Interface _parent;

    /*ci
     * \brief The properties that the loopback interface was created with
     */
    struct LOOP_InterfaceProperty property;

    /*ci
     * \brief The factory that created <em> this </em> interface
     */
    struct LOOP_InterfaceFactory *factory;

};

/*ci
 * \brief Loopback factory class
 */
struct LOOP_InterfaceFactory
{
    /*ci
     * \brief Inherited factory
     */
    struct RT_ComponentFactory _parent;

    /*ci
     * \brief counter used when instantiating interfaces
     */
    RTI_INT32 instance_counter;
};

#endif

/*ci @} */

