/*
 * FILE: InstanceHandleSeq.c - InstanceHandle sequence implementation
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
 * 19may2015,as MICRO-1193 Refactoring of Sequence API levels
 * 24oct2008,tk Created
 */
/*ce
 * \file
 * \brief InstanceHandle sequence implementation
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

/*** SOURCE_BEGIN ***/

#define T DDS_EncapsulationId_t
#define TSeq DDS_EncapsulationIdSeq
#define REDA_SEQUENCE_USER_API
#define TSeq_is_equal
#include "reda/reda_sequence_defn.h"
#undef T
#undef TSeq

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_TransportEncapsulationSettings_element_init(
                            struct DDS_TransportEncapsulationSettings_t *e)
{
    if (!REDA_StringSeq_initialize(&e->transports))
    {
        return RTI_FALSE;
    }

    if (!DDS_EncapsulationIdSeq_initialize(&e->encapsulations))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

#ifndef RTI_CERT
MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_TransportEncapsulationSettings_element_finalize(
                           struct DDS_TransportEncapsulationSettings_t *e)
{
    if (!REDA_StringSeq_finalize(&e->transports))
    {
        return RTI_FALSE;
    }

    if (!DDS_EncapsulationIdSeq_finalize(&e->encapsulations))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}
#endif

MUST_CHECK_RETURN RTI_PRIVATE RTI_BOOL
DDS_TransportEncapsulationSettings_element_copy(
                            struct DDS_TransportEncapsulationSettings_t *dst,
                            const struct DDS_TransportEncapsulationSettings_t *src)
{
    if (!REDA_StringSeq_copy(&dst->transports,&src->transports))
    {
        return RTI_FALSE;
    }

    if (!DDS_EncapsulationIdSeq_copy(&dst->encapsulations,&src->encapsulations))
    {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

MUST_CHECK_RETURN RTI_PRIVATE RTI_INT32
DDS_TransportEncapsulationSettings_element_compare(
                    const struct DDS_TransportEncapsulationSettings_t *left,
                    const struct DDS_TransportEncapsulationSettings_t *right)
{
    UNUSED_ARG(left);
    UNUSED_ARG(right);

    return RTI_TRUE;
}

#define T struct DDS_TransportEncapsulationSettings_t
#define TSeq DDS_TransportEncapsulationSettingsSeq
#define REDA_SEQUENCE_USER_API
#define TSeq_is_equal
#define T_initialize DDS_TransportEncapsulationSettings_element_init
#ifndef RTI_CERT
#define T_finalize   DDS_TransportEncapsulationSettings_element_finalize
#endif
#define T_copy       DDS_TransportEncapsulationSettings_element_copy
#define T_is_equal   DDS_TransportEncapsulationSettings_element_compare
#define TSeq_is_equal
#include <reda/reda_sequence_defn.h>

#define T DDS_DataRepresentationId_t
#define TSeq DDS_DataRepresentationIdSeq
#define REDA_SEQUENCE_USER_API
#define TSeq_is_equal
#include <reda/reda_sequence_defn.h>

/*ci @} */

