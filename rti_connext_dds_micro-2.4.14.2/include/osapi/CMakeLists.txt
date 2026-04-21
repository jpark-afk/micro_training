###############################################################################
# (c) Copyright, Real-Time Innovations 2018-2018
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
# Modification History
# --------------------
# 09sep2020,tk MICRO-2420 / PR#27261 Exclude unused header files
#
################################################################################

SET(RTI_MODULE_PUBLIC_HDR
    osapi_config.h
    osapi_dll.h
    osapi_types.h
    osapi_heap.h
    osapi_string.h
    osapi_mutex.h
    osapi_semaphore.h
    osapi_process.h
    osapi_thread.h
    osapi_time.h
    osapi_timer.h
    osapi_system.h
    osapi_heap_impl.h
    osapi_log_impl.h
    osapi_log.h
    osapi_cc.h
    osapi_os.h
)

IF ((${_RTIME_OSAPI_PLATFORM} STREQUAL ${RTIME_OSAPI_AUTO}) OR (RTIME_SHIP_SOURCE))
    LIST(APPEND RTI_MODULE_PUBLIC_HDR
         osapi_os_autosar.h
         osapi_cc_autosar.h
         osapi_os_vxworks.h
         osapi_os_posix.h
    )
    IF (NOT RTIME_CERT_SOURCE)
        LIST(APPEND RTI_MODULE_PUBLIC_HDR
             osapi_os_windows.h
             osapi_os_freertos.h
             osapi_os_threadx.h
        )
    ENDIF()
ELSEIF (${_RTIME_OSAPI_PLATFORM} STREQUAL ${RTIME_OSAPI_POSIX})
    LIST(APPEND RTI_MODULE_PUBLIC_HDR
         osapi_os_posix.h
    )
ELSEIF (${_RTIME_OSAPI_PLATFORM} STREQUAL ${RTIME_OSAPI_VXWORKS})
    LIST(APPEND RTI_MODULE_PUBLIC_HDR
         osapi_os_vxworks.h
    )
ELSEIF (${_RTIME_OSAPI_PLATFORM} STREQUAL ${RTIME_OSAPI_AUTOSAR})
    LIST(APPEND RTI_MODULE_PUBLIC_HDR
         osapi_cc_autosar.h
         osapi_os_autosar.h
    )
ELSEIF (${_RTIME_OSAPI_PLATFORM} STREQUAL ${RTIME_OSAPI_WINDOWS})
    LIST(APPEND RTI_MODULE_PUBLIC_HDR
         osapi_os_windows.h
    )
ELSEIF (${_RTIME_OSAPI_PLATFORM} STREQUAL ${RTIME_OSAPI_FREERTOS})
    LIST(APPEND RTI_MODULE_PUBLIC_HDR
         osapi_os_freertos.h
    )
ELSEIF (${_RTIME_OSAPI_PLATFORM} STREQUAL ${RTIME_OSAPI_THREADX})
    LIST(APPEND RTI_MODULE_PUBLIC_HDR
         osapi_os_threadx.h
    )
ENDIF()

IF ((NOT ${_RTIME_OSAPI_PLATFORM} STREQUAL ${RTIME_OSAPI_AUTOSAR}) OR (RTIME_SHIP_SOURCE))
    LIST(APPEND RTI_MODULE_PUBLIC_HDR
         osapi_cc_stdc.h
    )
ENDIF()
