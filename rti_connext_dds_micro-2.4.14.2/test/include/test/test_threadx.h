/*
 * FILE: test_threadx.h - Unit-test support for ThreadX
 *
 * (c) Copyright, Real-Time Innovations, 2017-2024.
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
 * 18apr2024,bgg        File reused for external symbol needs for ThreadX UTP
 * 20apr2017,francisco  ThreadX specific test settings.
 */
/*ce
 * \file
 * \brief Unit-test support for ThreadX
 */
#ifndef test_threadx_h
#define test_threadx_h

#include "stdarg.h"


/*e \brief
 * This function is to be defined by the UTP
 */
void standalonevprintf(const char *format, va_list ap);
            
#endif
