/*
 * FILE: GUID.c - GUID implementation
 *
 * (c) Copyright 2008-2026 Real-Time Innovations,
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
#ifndef osapi_string_h
#include "osapi/osapi_string.h"
#endif
#ifndef dds_c_infrastructure_h
#include "dds_c/dds_c_infrastructure.h"
#endif

/* ------------------------------------------------------------------
 * Public Constants
 * ------------------------------------------------------------------ */

/* exported in infrastructure.ifc */
const struct DDS_GUID_t DDS_GUID_UNKNOWN =
    { {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} };

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

void
DDS_GUID_to_rtps(struct RTPS_Guid *other, const struct DDS_GUID_t *self)
{
#if RTI_ENDIAN_BIG
    OSAPI_Memory_copy(other, self->value, DDS_GUID_LENGTH);
#else
    struct RTPS_Guid rtps_tmp = RTPS_GUID_UNKNOWN;

    OSAPI_Memory_copy(&rtps_tmp, self->value, DDS_GUID_LENGTH);

    other->prefix.host_id = NETIO_ntohl(rtps_tmp.prefix.host_id);
    other->prefix.app_id  = NETIO_ntohl(rtps_tmp.prefix.app_id );
    other->prefix.instance_id  = NETIO_ntohl(rtps_tmp.prefix.instance_id );
    other->object_id = NETIO_ntohl(rtps_tmp.object_id);
#endif
}

void
DDS_GUID_from_rtps(struct DDS_GUID_t *self, const struct RTPS_Guid *other)
{
#if RTI_ENDIAN_BIG
    OSAPI_Memory_copy(self->value, other, DDS_GUID_LENGTH);
#else
    struct RTPS_Guid rtps_tmp;

    rtps_tmp.prefix.host_id = NETIO_htonl(other->prefix.host_id);
    rtps_tmp.prefix.app_id  = NETIO_htonl(other->prefix.app_id );
    rtps_tmp.prefix.instance_id  = NETIO_htonl(other->prefix.instance_id );
    rtps_tmp.object_id = NETIO_htonl(other->object_id);

    OSAPI_Memory_copy(self->value, &rtps_tmp, DDS_GUID_LENGTH);
#endif
}

int
DDS_GUID_compare(const struct DDS_GUID_t *self,const struct DDS_GUID_t *other) {
    return OSAPI_Memory_compare(self->value,other->value,DDS_GUID_LENGTH);
}

void
DDS_GUID_copy(struct DDS_GUID_t *self, const struct DDS_GUID_t *other) {
	OSAPI_Memory_copy(self->value, other->value, DDS_GUID_LENGTH);
}


RTI_BOOL
DDS_GUID_initialize(DDS_GUID_t* sample)
{
    const DDS_GUID_t def_guid =
            { {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} };

    if (sample == NULL)
    {
        return RTI_FALSE;
    }

    *sample = def_guid;

    return RTI_TRUE;
}

RTI_BOOL
DDS_GUID_finalize(DDS_GUID_t* sample)
{
    if (sample == NULL)
    {
        return RTI_FALSE;
    }

    /* NO-OP for now */

    return RTI_TRUE;
}

/*ci @} */


