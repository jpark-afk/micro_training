###############################################################################
# (c) Copyright, Real-Time Innovations 2018-2024
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################

SET(RTI_MODULE_PUBLIC_HDR
    osapi_config.h
    osapi_features.h
    osapi_dll.h
    osapi_types.h
    osapi_heap.h
    osapi_hash.h
    osapi_string.h
    osapi_mutex.h
    osapi_semaphore.h
    osapi_process.h
    osapi_task.h
    osapi_time.h
    osapi_timer.h
    osapi_system.h
    osapi_heap_impl.h
    osapi_log_impl.h
    osapi_log.h
    osapi_cc.h
    osapi_cc_stdc.h
    osapi_os.h
    osapi_thread.h
)

IF (RTIME_SHIP_SOURCE)
    LIST(APPEND RTI_MODULE_PUBLIC_HDR
        osapi_cc_clang.h
        osapi_cc_qcc.h
        osapi_cc_gcc.h
        osapi_cc_msvsc.h
    )
ENDIF()

IF (DEFINED RTIME_OSAPI_COMPILER
    AND (NOT ${RTIME_OSAPI_COMPILER} STREQUAL ${RTIME_CC_AUTO}))
    LIST(APPEND RTI_MODULE_PUBLIC_HDR
         ${RTIME_OSAPI_COMPILER}
    )
ENDIF()

IF (RTIME_PIL AND (NOT RTIME_SHIP_SOURCE) AND (NOT RTIME_GENERATE))
    RETURN()
ENDIF()

IF (DEFINED RTIME_TARGET)
    IF ("${RTIME_TARGET}" STREQUAL "self")
        RETURN()
    ENDIF()
ENDIF()
