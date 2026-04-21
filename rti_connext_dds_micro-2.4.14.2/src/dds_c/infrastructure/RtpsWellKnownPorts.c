/*
 * FILE: RtpsWellKnownPorts.c - RtpsWellKnown ports implementation
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
 * 20sep2014,as Removed unused operations initialize, finalize, copy.
 * 04mar2014,tk MICRO-299: Simplified calculation of max participant id
 * 19jul2013,as  Added support for C++ (functions of QosPolicy type)
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief RtpsWellKnown ports implementation
 *
 * \details
 * This file implements function to use and validate the DDS_RtpsWellKnownPorts
 * Qos policy.
 */
/*ci \addtogroup DDSInfrastructureModule
 * @{
 */
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif

#include "RtpsWellKnownPorts.h"

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Test if two DDS_RtpsWellKnownPorts structures are equal
 *
 * \param[in] left side of comparison
 * \param[in] right side of comparison
 *
 * \return DDS_BOOLEAN_TRUE if left = right, DDS_BOOLEAN_FALSE otherwise
 */
DDS_Boolean
DDS_RtpsWellKnownPorts_is_equal(const struct DDS_RtpsWellKnownPorts *left,
                                const struct DDS_RtpsWellKnownPorts *right)
{
    OSAPI_PRECONDITION(left == NULL || right == NULL,
                            return DDS_BOOLEAN_FALSE,
                            OSAPI_Log_entry_add_pointer("left",left,RTI_FALSE);
                            OSAPI_Log_entry_add_pointer("right",right,RTI_TRUE);)

    return (left->builtin_multicast_port_offset == right->builtin_multicast_port_offset
            && left->builtin_unicast_port_offset == right->builtin_unicast_port_offset
            && left->domain_id_gain == right->domain_id_gain
            && left->participant_id_gain == right->participant_id_gain
            && left->port_base == right->port_base
            && left->user_multicast_port_offset == right->user_multicast_port_offset
            && left->user_unicast_port_offset == right->user_unicast_port_offset) ? 
                DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
}

/*ci
 * \brief Check that a DDS_RtpsWellKnownPorts_t policy has consistent,
 *        legal values
 *
 * \details
 *
 * This function checks if the parameters for a calculating well-known
 * ports are consistent. There are two cases:
 *
 * 1. domain_id_gain > participant_id_gain. In this case the max participant
 *    index must fall within:
 *
 *    - (port_base + (domain_id_gain * domain_id))
 *                          and
 *      (port_base + (domain_id_gain * (domain_id + 1)) - 1)
 *
 *    - In addition the following must be true:
 *      max_participant_id < (domain_id_gain / participant_id_gain)
 *
 * 2. If domain_id_gain <= participant_id_gain. In this case the participant
 *    indices spans the entire port range and the number of domains are limited
 *    by the domain_id_gain. In addition the following must be true, otherwise
 *    port aliasing will occur (port overlapping between participants in
 *    different domains).
 *
 *    - max_domain_id < (participant_id_gain / domain_id_gain)
 *    - domain_id_gain > abs(builtin_multicast_port_offset - user_multicast_port_offset)
 *    - domain_id_gain > abs(builtin_unicast_port_offset - user_unicast_port_offset)
 *
 * \param[in] self DDS_RtpsWellKnownPorts_t structure to be validated
 *
 * \return DDS_BOOLEAN_TRUE if the content is valid, DDS_BOOLEAN_FALSE if not
 */
DDS_Boolean
DDS_RtpsWellKnownPorts_is_consistent(
                                 const struct DDS_RtpsWellKnownPorts_t *self)
{
    RTI_INT32 diff;

    OSAPI_PRECONDITION(self == NULL, return DDS_BOOLEAN_FALSE,
                           OSAPI_Log_entry_add_pointer("self",self,RTI_TRUE);)

    /* Micro can handle same offset for some UDP ports. The same ports can be 
     * used for :
     *  - builtin multicast and user data multicast.
     *  - builtin unicast and user data unicast.
     */
    if ((self->port_base < 1) ||
        (self->domain_id_gain < 1) ||
        (self->participant_id_gain < 1) ||
        (self->builtin_multicast_port_offset < 0) ||
        (self->builtin_unicast_port_offset < 0) ||
        (self->user_multicast_port_offset < 0) ||
        (self->user_unicast_port_offset < 0) ||
        (self->builtin_multicast_port_offset == self->builtin_unicast_port_offset) ||
        (self->builtin_multicast_port_offset == self->user_unicast_port_offset) ||
        (self->builtin_unicast_port_offset == self->user_multicast_port_offset) ||
        (self->user_multicast_port_offset == self->user_unicast_port_offset))
    {
        return DDS_BOOLEAN_FALSE;
    }

    /* Check that the offsets are large enough
     * participant_id_gain > abs(builtin_unicast_port_offset - user_unicast_port_offset)
     */
    if (self->builtin_unicast_port_offset >= self->user_unicast_port_offset)
    {
        diff = self->builtin_unicast_port_offset - self->user_unicast_port_offset;
    }
    else
    {
        diff = self->user_unicast_port_offset - self->builtin_unicast_port_offset;
    }

    if (self->participant_id_gain <= diff)
    {
        return DDS_BOOLEAN_FALSE;
    }

    if (self->domain_id_gain > self->participant_id_gain)
    {
        return DDS_BOOLEAN_TRUE;
    }

    /* if self->domain_id_gain <= self->participant_id_gain, perform
     * additional checks to ensure port-aliasing cannot happen
     */

    /* Check that:
     * domain_id_gain > abs(builtin_multicast_port_offset - user_multicast_port_offset)
     */
    if (self->builtin_multicast_port_offset >= self->user_multicast_port_offset)
    {
        diff = self->builtin_multicast_port_offset - self->user_multicast_port_offset;
    }
    else
    {
        diff = self->user_multicast_port_offset - self->builtin_multicast_port_offset;
    }

    if (self->domain_id_gain <= diff)
    {
        return DDS_BOOLEAN_FALSE;
    }

    /* Check that:
     * domain_id_gain > abs(builtin_unicast_port_offset - user_unicast_port_offset)
     */
    if (self->builtin_unicast_port_offset >= self->user_unicast_port_offset)
    {
        diff = self->builtin_unicast_port_offset - self->user_unicast_port_offset;
    }
    else
    {
        diff = self->user_unicast_port_offset - self->builtin_unicast_port_offset;
    }

    if (self->domain_id_gain <= diff)
    {
        return DDS_BOOLEAN_FALSE;
    }

    return DDS_BOOLEAN_TRUE;
}

/*ci
 * \brief Calculate the maximum legal participant index based on the port
 *        calculation parameters
 *
 * \details
 *
 * Calculate the maximum participant index based on the parameters specified
 * in the input. This function shall only be called if the parameters passes
 * the consistency check in DDS_RtpsWellKnownPorts_is_consistent().
 *
 * \param[in] self Port parameters
 *
 * \return Maximum legal participant index
 *
 * \sa DDS_RtpsWellKnownPorts_is_consistent
 */
DDS_Long
DDS_RtpsWellKnownPorts_get_max_participant_index(
                                    const struct DDS_RtpsWellKnownPorts_t *self)
{
    DDS_Long max_index = 0;
    const DDS_Long PORT_MAX = 65535;

    if (self->domain_id_gain > self->participant_id_gain)
    {
        /* The maximum index is determined solely by the domain_id_gain,
         * all participants will have ports within on domain_id_gain
         * group.
         */
        max_index = self->domain_id_gain / self->participant_id_gain;
    }
    else
    {
        /* Use the entire range of valid ports, participant ports are spread
         * across the entire port range.
         */
        max_index = (PORT_MAX - self->port_base) / self->participant_id_gain;
    }

    /* The above calculation would cause port aliasing because of the
     * port offsets. Since each port has an offset, use the largest offset to
     * calculate how many participant indices that correspond to and subtract
     * it from the current max_index.
     */
    if (self->builtin_unicast_port_offset > self->user_unicast_port_offset)
    {
        max_index -= self->builtin_unicast_port_offset /
                     self->participant_id_gain;
    }
    else
    {
        max_index -= self->user_unicast_port_offset /
                     self->participant_id_gain;
    }

    return max_index;
}
/*ci @} */
