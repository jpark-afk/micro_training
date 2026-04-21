/*
 * FILE: RemoteEntity.c - RemoteEntity implementation
 *
 * (c) Copyright, Real-Time Innovations, 2008-2015.
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
 * 17may2012,tk Updated
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief RemoteEntity implementation
 *
 * \details
 * This file implements helper functions for remote entities.
 */
/*ci
 * \addtogroup DDSDomainModule
 * @{
 */
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_discovery_plugin_h
#include "dds_c/dds_c_discovery_plugin.h"
#endif
#include "Entity.h"
#include "RemoteEntity.h"

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Check if a remote endpoint has been enabled
 *
 * \details
 *
 * A remote endpoint is enabled when it is discovered. Statically asserted
 * endpoints, for example when using static discovery, is not enabled until the
 * parent participant is discovered.
 *
 * \param[in] entity Remote endpoint to check
 *
 * \return DDS_BOOLEAN_TRUE if it is enabled, DDS_BOOLEAN false otherwise
 */
DDS_Boolean
NDDS_RemoteEntity_is_enabled(struct NDDS_RemoteEntityImpl *entity)
{
    return (entity->state == RTIDDS_ENTITY_STATE_ENABLED);
}

/*ci @} */
