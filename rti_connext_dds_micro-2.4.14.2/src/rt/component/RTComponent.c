/*
 * FILE: RTComponent.c - RT Component implementation
 *
 * Copyright 2012-2021 Real-Time Innovations, Inc.
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
 * 22apr2014,tk MICRO-82   Updated after code review.
 * 02aug2013,tk MICRO-245/PR#1415 Initialize all fields in component
 * 28may2012,tk Written
 */
/*ce
 * \file
 * \brief Implementation of the RT_Component API
 */
#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif
#ifndef rt_rt_h
#include "rt/rt_rt.h"
#endif

/*** SOURCE_BEGIN ***/

void
RT_Component_initialize(RT_Component_T *c,
                       struct RT_ComponentI *intf,
                       RTI_UINT32 id,
                       const struct RT_ComponentProperty *const property,
                       const struct RT_ComponentListener *const listener)
{
    UNUSED_ARG(property);
    UNUSED_ARG(listener);

    c->id = id;
    c->_intf = intf;
}

#ifndef RTI_CERT
void
RT_Component_finalize(RT_Component_T *self)
{
    UNUSED_ARG(self);
}
#endif /* !RTI_CERT */
