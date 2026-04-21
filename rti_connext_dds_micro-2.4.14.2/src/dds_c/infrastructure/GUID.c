/*
 * FILE: GUID.c - GUID implementation
 *
 * (c) Copyright 2008-2021 Real-Time Innovations,
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
 * 13dec2021,tk MICRO-3226/PR.29479
 * - Place braces on separate lines for the global constants.
 * 15apr2021,tk MICRO-2979/PR.27549
 * - Removed unused functions from CERT:
 *   DDS_GUID_to_rtps
 * 25feb2015,eh  MICRO-1040/PR#13555 Remove unused DDS_Guid_equals/copy/compare
 * 23feb2015,eh MICRO-1081: fix function/var names to conform to coding std
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief GUID implementation
 *
 * \details
 * This file implements functions to manipulate the GUID data-type.
 */
/*ci \addtogroup DDSInfrastructureModule
 * @{
 */
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_common_h
#include "dds_c/dds_c_common.h"
#endif
#ifndef dds_c_infrastructure_h
#include "dds_c/dds_c_infrastructure.h"
#endif
#ifndef dds_c_log_h
#include "dds_c/dds_c_log.h"
#endif

/* ------------------------------------------------------------------
 * Public Constants
 * ------------------------------------------------------------------ */

/* exported in infrastructure.ifc */
const struct DDS_GUID_t DDS_GUID_UNKNOWN =
{
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }
};

const struct DDS_GUID_t DDS_GUID_AUTO =
{
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }
};

const struct DDS_GUID_t DDS_GUID_PREFIX_UNKNOWN =
{
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }
};

const struct DDS_GUID_t DDS_GUID_PREFIX_AUTO =
{
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }
};

/*** SOURCE_BEGIN ***/

/* ------------------------------------------------------------------------
 * Public Methods
 * ------------------------------------------------------------------------ */

void
DDS_GUID_set_suffix(struct DDS_GUID_t *self,DDS_Long suffix)
{
    suffix = (DDS_Long)NETIO_htonl(suffix);
    OSAPI_Memory_copy((void*)&self->value[12],&suffix,(RTI_SIZE_T)sizeof(DDS_Long));
}

#ifndef RTI_CERT
void
DDS_GUID_to_rtps(struct RTPS_Guid *other, const struct DDS_GUID_t *self)
{
#if RTI_ENDIAN_BIG
    OSAPI_Memory_copy(other, self->value, 16);
#else
    struct RTPS_Guid rtps_tmp;

    OSAPI_Memory_copy(&rtps_tmp, self->value, 16);

    other->prefix.host_id = NETIO_ntohl(rtps_tmp.prefix.host_id);
    other->prefix.app_id  = NETIO_ntohl(rtps_tmp.prefix.app_id );
    other->prefix.instance_id  = NETIO_ntohl(rtps_tmp.prefix.instance_id );
    other->object_id = NETIO_ntohl(rtps_tmp.object_id);
#endif
}
#endif

void
DDS_GUID_from_rtps(struct DDS_GUID_t *self, const struct RTPS_Guid *other)
{
#if RTI_ENDIAN_BIG
    OSAPI_Memory_copy(self->value, other, 16);
#else
    struct RTPS_Guid rtps_tmp;

    rtps_tmp.prefix.host_id = NETIO_htonl(other->prefix.host_id);
    rtps_tmp.prefix.app_id  = NETIO_htonl(other->prefix.app_id );
    rtps_tmp.prefix.instance_id  = NETIO_htonl(other->prefix.instance_id );
    rtps_tmp.object_id = NETIO_htonl(other->object_id);

    OSAPI_Memory_copy(self->value, &rtps_tmp, 16);
#endif
}

/*ci @} */


