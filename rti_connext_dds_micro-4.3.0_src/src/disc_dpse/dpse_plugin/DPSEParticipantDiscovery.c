/*
 * FILE: DPSEParticipantDiscovery.c - DPSE Participant Discovery
 *
 * Copyright (c) 2011-2026 Real-Time Innovations,Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 28may2015,tk MICRO-1263/PR#14863 Refactored for clarity and consistency
 * 27may2015,tk MICRO-1256/PR#14848 Reset retcode to DDS_RETCODE_ERROR in
 *                                  schedule_fast_assertions. Added comments.
 *              MICRO-1257/PR#14849 (related to above)
 * 27may2015,tk MICRO-1241/PR#14824 Added comments explaining initial and
 *                                  regular announcement
 * 27may2015,tk MICRO-1240/PR#14823 Refactored use of participant timer
 * 27may2015,tk MICRO-1255/PR#14847 (related to above)
 * 20feb2015,eh MICRO-813/PR#9172 Fix Lint warnings
 * 05jan2014,tk   Updated log-codes
 * 03jun2008,rmw  Created.
 */
/*ce
 * \file
 * \brief DPSE Participant Discovery functionality
 *
 * \details
 * This file implements functionality for participant announcements and
 * discovery.
 */
/*ci \addtogroup DPSEModule
 * @{
 */
#ifndef osapi_time_h
#include "osapi/osapi_time.h"
#endif
#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif
#ifndef dds_c_discovery_plugin_h
#include "dds_c/dds_c_discovery_plugin.h"
#endif
#ifndef disc_dpse_dpsediscovery_h
#include "disc_dpse/disc_dpse_dpsediscovery.h"
#endif
#ifndef disc_dpse_log_h
#include "disc_dpse/disc_dpse_log.h"
#endif

#include "DPSECdr.h"
#include "DPSEParticipantBuiltinTopicData.h"
#include "DPSEParticipantListener.h"
#include "DPSEParticipantDiscovery.h"
#include "DPSEDiscoveryPlugin.h"

/*** SOURCE_BEGIN ***/

/*ci
 * \brief Callback called when it is time to assert the local participant
 *
 * \details
 * This function is a timer callback and is called asynchronously to any
 * user API. A user cannot call this function. Because of this the function
 * does not return an error. Any calls that fail will log an error. For
 * Cert, no error logging is supported and thus there is no indication of
 * an error. However, if this function fails to assert liveliness the
 * remote participants will detect this as a liveliness change.
 *
 * \param[in] storage Data passed back from the timeout event
 *
 * \return OSAPI_TIMEOUT_OP_AUTOMATIC after the initial assertions,
 *         OSAPI_TIMEOUT_OP_MANUAL before
 */
RTI_PRIVATE OSAPI_TimeoutOp_t
DPSE_ParticipantDiscovery_assert_participant(
                                        struct OSAPI_TimeoutUserData *storage)
{
    struct DPSE_DiscoveryPlugin *dpse_plugin =
                            (struct DPSE_DiscoveryPlugin *)storage->field[0];
    void *local_participant_data = storage->field[1];
    DDS_ReturnCode_t ddsrc;
    RTI_BOOL bretval;
    struct DDS_Duration_t next_duration;

    OSAPI_TRACE_DDS("[DPSE] announce self",RTI_TRUE)

    ddsrc = DDS_DataWriter_write(dpse_plugin->participant_writer,
                                 local_participant_data,&DDS_HANDLE_NIL);
#if OSAPI_ENABLE_LOG
    if (DDS_RETCODE_OK != ddsrc)
    {
        DPSE_LOG_ANNOUNCEMENT(OSAPI_LOGKIND_ERROR)
    }
#else
    /* Ignore the return value since we have to let the timer continue */
    IGNORE_RETVAL(ddsrc);
#endif

    /* Participant announcements are sent at two different intervals. The
     * initial announcements period is used when the participant is first
     * enabled. The normal participant_liveliness_assert_period is used
     * after the initial announcements have been sent. This is to speed
     * up the discovery process since the announcements are best-effort.
     * dpse_plugin->initial_announcement_count is used to choose which interval
     * to reschedule the event with. If dpse_plugin->initial_announcement_count
     * otherwise we are sending with the regular interval. We use the
     * OSAPI_TIMEOUT_OP_MANUAL return value since the timer switches between
     * initial and normal interval based on participant discovery.
     */
    if (dpse_plugin->initial_announcement_count > 1)
    {
        next_duration = dpse_plugin->properties.initial_participant_announcement_period;
    }
    else
    {
        next_duration = dpse_plugin->properties.participant_liveliness_assert_period;
    }

    if (dpse_plugin->initial_announcement_count > 0)
    {
        dpse_plugin->initial_announcement_count--;
    }

    bretval = OSAPI_Timer_update_timeout(dpse_plugin->loaned_timer,
                                     &dpse_plugin->announcement_event,
                                     next_duration.sec,
                                     (RTI_INT32)next_duration.nanosec);

#if OSAPI_ENABLE_LOG
    if (!bretval)
    {
        DPSE_LOG_UPDATE_PARTICIPANT_ASSERT_PERIOD(OSAPI_LOGKIND_ERROR)
    }
#else
    IGNORE_RETVAL(bretval);
#endif

    return OSAPI_TIMEOUT_OP_MANUAL;
}

/*ci
 * \brief Announce the participant and start a timer to announce the participant
 *        periodically
 *
 * \details
 * When the participant is enabled a number of initial announcements are sent
 * out (configurable). The time between the initial announcements are less
 * than the normal announcements. The timer is thus updated when the initial
 * number of announcements have been sent. This function assumes that
 * the DPSE plugin instance has been successfully initialized.
 *
 * \param[in] discovery_plugin       The DPSE plugin
 * \param[in] local_participant_data The participant data to announce
 * \param[in] new_event              Whether the announcement data has
 *                                   changed or not
 *
 * \return DDS_RETCODE_OK on success, one of the other \ref DDS_ReturnCode_t
 *         on error
 */
MUST_CHECK_RETURN DDS_ReturnCode_t
DPSE_ParticipantDiscovery_schedule_fast_assertions(
        struct NDDS_Discovery_Plugin *discovery_plugin,
        const struct DDS_ParticipantBuiltinTopicData *local_participant_data,
        DDS_Boolean new_event)
{
    struct DPSE_DiscoveryPlugin *dpse_plugin =
                        (struct DPSE_DiscoveryPlugin *)discovery_plugin;
    struct OSAPI_TimeoutUserData storage = OSAPI_TimeoutUserData_INITIALIZER;
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    struct DDS_Duration_t next_duration;

    OSAPI_TRACE_DDS("[DPSE] schedule participant assertions",RTI_TRUE)

    if (new_event)
    {
        /* Advance the sequence number the first time. The sequence is
         * manually updated to prevent participants who already have seen
         * this participant from processing the announcement again. The
         * announcement does not change (it is immutable).
         */
        retcode = DDS_DataWriter_advance_sn(dpse_plugin->participant_writer);
        if (retcode != DDS_RETCODE_OK)
        {
            DPSE_LOG_ADVANCE_SN(OSAPI_LOGKIND_ERROR)
            return retcode;
        }
    }

    /* Send out the first participant announcement */
    retcode = DDS_DataWriter_write(dpse_plugin->participant_writer,
                                   (void *)local_participant_data,
                                   &DDS_HANDLE_NIL);
    if (retcode != DDS_RETCODE_OK)
    {
        DPSE_LOG_ANNOUNCE_WRITE(OSAPI_LOGKIND_ERROR,retcode)
            return retcode;
    }

    /* Reset retcode to DDS_RETCODE_ERROR since at this point it must be
     * DDS_RETCODE_OK
     */
    retcode = DDS_RETCODE_ERROR;

    storage.field[0] = (void*)dpse_plugin;
    storage.field[1] = (void*)local_participant_data;

    /* If initial_participant_announcements > 0 one announcement has already
     * been sent above. If more then 1 is requested schedule them
     * at the initial_participant_announcement_period. The number of initial
     * announcements left to send is stored in the
     * dpse_plugin->initial_announcement_count variable. Schedule sending the
     * initial announcements at the initial announcement period. Since we
     * have already sent one announcement, subtract one. Otherwise schedule at
     * the regular interval.
     */
    if (dpse_plugin->properties.initial_participant_announcements > 1)
    {
        dpse_plugin->initial_announcement_count =
                  dpse_plugin->properties.initial_participant_announcements - 1;
        next_duration = dpse_plugin->properties.initial_participant_announcement_period;
    }
    else
    {
        dpse_plugin->initial_announcement_count = 0;
        next_duration = dpse_plugin->properties.participant_liveliness_assert_period;
    }

    if (new_event)
    {
        /* This is the first time we announce this participant. Create the
         * timer. The timer may be started with either initial or regular
         * period.
         */
        if (!OSAPI_Timer_create_timeout(dpse_plugin->loaned_timer,
                                &dpse_plugin->announcement_event,
                                next_duration.sec,
                                (RTI_INT32)next_duration.nanosec,
                                OSAPI_TIMER_PERIODIC,
                                DPSE_ParticipantDiscovery_assert_participant,
                                &storage))
        {
            DPSE_LOG_SCHEDULE_FAST_ASSERTION(OSAPI_LOGKIND_ERROR)
            goto finally;
        }
        dpse_plugin->timer_is_created = RTI_TRUE;
    }
    else
    {
        if (!OSAPI_Timer_update_timeout(dpse_plugin->loaned_timer,
                                        &dpse_plugin->announcement_event,
                                        next_duration.sec,
                                        (RTI_INT32)next_duration.nanosec))
        {
            DPSE_LOG_UPDATE_PARTICIPANT_ASSERT_PERIOD(OSAPI_LOGKIND_ERROR)
            goto finally;
        }
    }

    retcode = DDS_RETCODE_OK;

finally:
    return retcode;
}

/*ci @} */
