/*
 * FILE: test_autosar.h - Unit-test support for Autosar
 *
 * (c) Copyright, Real-Time Innovations, 2011-2023.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * modification history
 * --------------------
 * 22Jul2023,bgg  File reused for external symbol needs for AUTOSAR testing
 * 07Jun2019,fmt  Autosar specific test settings.
 */
/*ce
 * \file
 * \brief Unit-test support for Autosar
 */
#ifndef test_autosar_h
#define test_autosar_h

#include "Os.h"
#include "stdarg.h"

unsigned int UTEST_autosar_set_system_properties(void);

extern TaskType NETIO_Autosar_udp_receive_task_id;

extern EventMaskType RTIME_UDP_Receive_Event_id;

#if defined(RTIME_AUTOSAR_MICROSAR) && defined(__TASKING__)
void standalonevprintf(const char *format, va_list ap);
#elif defined(RTIME_AUTOSAR_MICROSAR) && defined(_MSC_VER)
/* Will use regular vprintf */
#elif defined(RTIME_AUTOSAR_WINCORE)
/* Will use regular vprintf */
#else
#  error "vprintf must be defined"
#endif

#endif
