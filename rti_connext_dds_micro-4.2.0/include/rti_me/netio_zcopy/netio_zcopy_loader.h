
/*
 * FILE: netio_zcopy_loader.h - Zero Copy Loader API
 *
 * (c) Copyright 2024 Real-Time Innovations,
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
 * \brief Zero Copy Loader API
 */

#include "netio_zcopy/netio_zcopy_notif_interface.h"

/*ci \brief Zcopy Loader specific property */
struct ZCOPY_NotifLoaderFactoryProperty
{
    struct ZCOPY_NotifInterfaceFactoryProperty _parent;

    const char* notif_transport_name;
};


#define ZCOPY_NotifLoaderFactoryProperty_INITIALIZER \
{\
    ZCOPY_NotifInterfaceFactoryProperty_INITIALIZER,\
    NETIO_DEFAULT_NOTIF_NAME\
}
