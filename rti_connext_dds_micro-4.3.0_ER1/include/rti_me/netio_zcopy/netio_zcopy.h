/*
 * FILE: netio_zcopy.h - Zero Copy transport API
 *
 * (c) Copyright 2022-2024 Real-Time Innovations,
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
 * \brief Zero Copy transport API
 */
/*ci \addtogroup ZeroCopyModule
 * @{
 */
#ifndef netio_zcopy_h
#define netio_zcopy_h

#include "netio_zcopy/netio_zcopy_dll.h"
#include "netio_zcopy/netio_zcopy_notif_interface.h"
#include "netio_zcopy/netio_zcopy_notif_mechanism_intf.h"
#include "netio_zcopy/netio_zcopy_whsq.h"
#include "netio_zcopy/netio_zcopy_loader.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*e
 * \ingroup ZCOPY_NotifInterfaceClass
 *
 * \brief Default name for notification interface
 */
NETIO_ZCOPYDllExport extern const char *const NETIO_DEFAULT_NOTIF_NAME;

/*e
 * \brief Default writer history name when it is wrapped by another history.
 */
NETIO_ZCOPYDllExport extern const char* const DDSHST_WRITER_WRAPPED_HISTORY_NAME;

/*e \dref_NDDS_Transport_ZeroCopy_initialize
 */
NETIO_ZCOPYDllExport RTI_BOOL
NDDS_Transport_ZeroCopy_initialize(
    RT_Registry_T *registry,
    const char *transport_name,
    struct ZCOPY_NotifInterfaceFactoryProperty *property);

/*e \dref_NDDS_Transport_ZeroCopy_finalize
 */
NETIO_ZCOPYDllExport RTI_BOOL
NDDS_Transport_ZeroCopy_finalize(
    RT_Registry_T *registry,
    const char *transport_name);

/*e \dref_ZCOPY_NotifInterfaceFactory_get_interface
 */
MUST_CHECK_RETURN NETIO_ZCOPYDllExport struct RT_ComponentFactoryI*
ZCOPY_NotifInterfaceFactory_get_interface(void);

/*i \dref_ZCOPY_Loader_get_interface
 */
MUST_CHECK_RETURN NETIO_ZCOPYDllExport struct RT_ComponentFactoryI*
ZCOPY_Loader_get_interface(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

/*ci @} */
