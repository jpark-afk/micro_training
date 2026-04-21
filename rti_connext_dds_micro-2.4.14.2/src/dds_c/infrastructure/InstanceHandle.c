/*
 * FILE: InstanceHandle.c - InstanceHandle implementation
 *
 * (c) Copyright 2008-2015 Real-Time Innovations,
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
 * 13apr2021,tk MICRO-3028/PR.28960
 * - Added robustness check to DDS_InstanceHandle_equals
 * 23feb2015,eh MICRO-1081: fix function/var names to conform to coding std
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \ingroup DDSDomainModule
 * \brief InstanceHandle implementation
 *
 * \details
 * This file contains functions to manage and manipulate the InstanceHandle
 * data type.
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

#include "InstanceHandle.h"

/* ------------------------------------------------------------------------
 * Public Constants
 * ------------------------------------------------------------------------ */
const DDS_InstanceHandle_t DDS_HANDLE_NIL = DDS_HANDLE_NIL_NATIVE;

/*** SOURCE_BEGIN ***/
void
DDS_InstanceHandle_to_rtps(struct RTPS_Guid *other,
                           const DDS_InstanceHandle_t *self)
{

#if RTI_ENDIAN_BIG
    OSAPI_Memory_copy(other, self->octet, 16);
#else
    struct RTPS_Guid rtps_tmp;

    OSAPI_Memory_copy(&rtps_tmp, self->octet, 16);

    other->prefix.host_id = NETIO_ntohl(rtps_tmp.prefix.host_id);
    other->prefix.app_id  = NETIO_ntohl(rtps_tmp.prefix.app_id );
    other->prefix.instance_id  = NETIO_ntohl(rtps_tmp.prefix.instance_id );
    other->object_id = NETIO_ntohl(rtps_tmp.object_id);
#endif
}

void
DDS_InstanceHandle_from_rtps(DDS_InstanceHandle_t *self,
                             const struct RTPS_Guid *other)
{
#if RTI_ENDIAN_BIG
    OSAPI_Memory_copy(self->octet, other, 16);
#else
    struct RTPS_Guid rtps_tmp;

    rtps_tmp.prefix.host_id = NETIO_htonl(other->prefix.host_id);
    rtps_tmp.prefix.app_id  = NETIO_htonl(other->prefix.app_id );
    rtps_tmp.prefix.instance_id  = NETIO_htonl(other->prefix.instance_id );
    rtps_tmp.object_id = NETIO_htonl(other->object_id);

    OSAPI_Memory_copy(self->octet, &rtps_tmp, 16);
#endif
    self->is_valid = DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Set the suffix of an instance handle
 *
 * \param[in] self   Instance handle to set suffix in
 * \param[in] suffix Suffix to set on instance handle
 *
 * \sa \ref DDS_InstanceHandle_to_rtps
 */
void
DDS_InstanceHandle_set_suffix(DDS_InstanceHandle_t *self,DDS_UnsignedLong suffix)
{
    suffix = NETIO_htonl(suffix);
    OSAPI_Memory_copy((void*)&self->octet[12],&suffix,(RTI_SIZE_T)sizeof(DDS_Long));
}

/* ------------------------------------------------------------------------
 * Public Methods
 * ------------------------------------------------------------------------ */
DDS_Boolean
DDS_InstanceHandle_equals(const DDS_InstanceHandle_t *self,
                          const DDS_InstanceHandle_t *other)
{
    OSAPI_PRECONDITION_ALWAYS(self == NULL || other == NULL,
                              return DDS_BOOLEAN_FALSE,
                              OSAPI_Log_entry_add_pointer("self",self,RTI_FALSE);
                              OSAPI_Log_entry_add_pointer("other",other,RTI_TRUE);)

    if (self->is_valid != other->is_valid)
    {
        return DDS_BOOLEAN_FALSE;
    }

    return OSAPI_Memory_compare(self->octet, other->octet, 16) == 0 ?
                        DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

DDS_Long
DDS_InstanceHandle_compare(const DDS_InstanceHandle_t * self,
                           const DDS_InstanceHandle_t * other)
{
    return OSAPI_Memory_compare(self->octet, other->octet, sizeof(self->octet));
}

void 
DDS_InstanceHandle_from_netio_address(DDS_InstanceHandle_t *self,
                                      const struct NETIO_Address *other)
{
#if RTI_ENDIAN_BIG
    OSAPI_Memory_copy(&self->octet, &other->value.guid, 16);
#else
    struct RTPS_Guid rtps_tmp;
    rtps_tmp.prefix.host_id = NETIO_htonl(other->value.rtps_guid.host_id);
    rtps_tmp.prefix.app_id  = NETIO_htonl(other->value.rtps_guid.app_id);
    rtps_tmp.prefix.instance_id  = 
        NETIO_htonl(other->value.rtps_guid.instance_id);
    rtps_tmp.object_id = NETIO_htonl(other->value.rtps_guid.object_id);
    OSAPI_Memory_copy(&self->octet, &rtps_tmp, 16);
#endif
    self->is_valid = DDS_BOOLEAN_TRUE;
}

/*ci @} */
