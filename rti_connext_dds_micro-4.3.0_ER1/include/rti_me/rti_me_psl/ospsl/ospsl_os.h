/*
 * FILE: ospsl_os.h - Specific OS related definitions
 *
 * Copyright (c) 2024-2024 Real-Time Innovations, Inc.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */
#ifndef ospsl_os_h
#define ospsl_os_h

/*ci \brief Macro to stringify a macro using the C preprocessor
 */
#define OSPSL_CC_STRINGIFY_DEFINE(v_) OSPSL_CC_STRINGIFY_VALUE(v_)

/*ci \brief Macro to stringify a value using the C preprocessor
 */
#define OSPSL_CC_STRINGIFY_VALUE(v_) #v_

#ifndef OSPSL_OS_DEF_H
/* If the platform has not been specified, attempt to determine it.
 */
#if defined(__APPLE__) && defined(__MACH__)
#define OSPSL_OS_DEF_H rti_me_psl/ospsl/ospsl_os_posix.h
#elif defined(__linux__) || defined(RTI_DEOS_RTEMS)
#define OSPSL_OS_DEF_H rti_me_psl/ospsl/ospsl_os_posix.h
#elif defined(__VOS__)
#define OSPSL_OS_DEF_H rti_me_psl/ospsl/ospsl_os_posix.h
#elif defined(_MSC_VER) || defined(WIN32)
#define OSPSL_OS_DEF_H rti_me_psl/ospsl/ospsl_os_windows.h
#elif defined(__vxworks)
#if defined(RTI_ARINC653)
#define OSPSL_OS_DEF_H rti_me_psl/ospsl/ospsl_os_vxworks653.h
#else
#if (ENABLE_FACE_COMPLIANCE > FACE_COMPLIANCE_LEVEL_SECURITY) && \
    (ENABLE_FACE_COMPLIANCE < FACE_COMPLIANCE_LEVEL_GENERAL)
#define OSPSL_OS_DEF_H rti_me_psl/ospsl/ospsl_os_posix.h
#else
#define OSPSL_OS_DEF_H rti_me_psl/ospsl/ospsl_os_vxworks.h
#endif /* ENABLE_FACE_COMPLIANCE */
#endif /* defined(RTI_VX653) */
#elif defined(__QNXNTO__)
#define OSPSL_OS_DEF_H rti_me_psl/ospsl/ospsl_os_posix.h
#elif defined(RTI_DEOS) && defined(RTI_ARINC653)
#define OSPSL_OS_DEF_H rti_me_psl/ospsl/ospsl_os_deos653.h
#elif defined(__autosar__)
#define OSPSL_OS_DEF_H rti_me_psl/ospsl/ospsl_os_autosar.h
#elif defined(__freertos__)
#define OSPSL_OS_DEF_H rti_me_psl/ospsl/ospsl_os_freertos.h
#else
#error "Unable to detect OS. Please define OSPSL_OS_DEF_H."
#endif
#endif

#include OSPSL_CC_STRINGIFY_DEFINE(OSPSL_OS_DEF_H)

#endif
