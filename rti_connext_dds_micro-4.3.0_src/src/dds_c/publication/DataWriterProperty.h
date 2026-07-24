/*
 * FILE: DataWriterProperty.c - DataWriter Property API definition
 *
 * (c) Copyright 2024 - 2024 Real-Time Innovations,
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef DataWriterProperty_h
#define DataWriterProperty_h

#ifndef dds_c_config_h
#include "dds_c/dds_c_config.h"
#endif
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif

#include "DataWriterImpl.h"

/*ci \brief Set DataWriter policies from the property qos policy
 *
 * \param[in] self - The DataWriter to set the policy on
 * \param[in] policy - A valid policy with properties to set
 *
 * \return DDS_BOOLEAN_TRUE on success, DDS_BOOLEAN_FALSE on failure
 */
extern DDS_Boolean
DDS_DataWriter_set_from_property(DDS_DataWriter *self,
                                 const struct DDS_PropertyQosPolicy *policy);
#endif
