/*
 * FILE: DataWriterProperty.c - DataWriter Property implementation
 *
 * (c) Copyright 2024 - 2026 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef dds_c_common_impl
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_sequence_h
#include "dds_c/dds_c_sequence.h"
#endif
#ifndef dds_c_infrastructure_h
#include "dds_c/dds_c_infrastructure_h.h"
#endif

#include "DataWriterProperty.h"

DDS_Boolean
DDS_DataWriter_set_from_property(DDS_DataWriter *self,
                                 const struct DDS_PropertyQosPolicy *policy)
{

    struct DDS_DataWriterImpl *dw = (struct DDS_DataWriterImpl*)self;
    struct DDS_Property *search = NULL;

    search = DDS_PropertyQosPolicyHelper_lookup_property(
                    policy,
                    DDS_DATAWRITER_PROTOCOL_ZCV2_VERSION_PROPERTY);

    if (search != NULL)
    {
        unsigned int version = 0;
        if (!OSAPI_String_parse_unsigned_long(search->value, &version) ||
            (version > 0xFFFF))
        {
            return DDS_BOOLEAN_FALSE;
        }
        dw->zcv2_protocol_version = (DDS_UnsignedShort)version;
    }

#if DDS_XTYPES_IS_ENABLED
    search = DDS_PropertyQosPolicyHelper_lookup_property(
                    policy,
                    DDS_XTYPES_COMPLIANCE_MASK_PROPERTY);

    if (search == NULL)
    {
        return DDS_BOOLEAN_TRUE;
    }

    if (!OSAPI_String_parse_unsigned_long(search->value,
                                          &dw->xtypes_compliance_mask))
    {
        return DDS_BOOLEAN_FALSE;
    }

#endif
    return DDS_BOOLEAN_TRUE;
}
