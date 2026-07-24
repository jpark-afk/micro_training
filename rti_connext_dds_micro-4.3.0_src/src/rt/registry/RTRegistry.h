/*
 * FILE: RTRegistry.h - RT Implementation
 *
 * Copyright 2011-2025 Real-Time Innovations, Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 12dec2011,tk Written
 */
/*ce
 * \file
 */

#ifndef RTRegistry_h
#define RTRegistry_h

/*ci
 * \brief Implementation of RT_Registry
 */
struct RT_Registry
{
    /*ci
     * \brief true if the registry has been initialized
     */
    RTI_BOOL _is_initialized;

    /*ci
     * \brief Table with registered factories
     */
    DB_Table_T factory_table;

    /*ci
     * \brief Table with registered system factories
     */
    DB_Table_T system_factory_table;

    /*ci
     * \brief Current properties of the registry
     */
    struct RT_RegistryProperty property;
};

#endif
