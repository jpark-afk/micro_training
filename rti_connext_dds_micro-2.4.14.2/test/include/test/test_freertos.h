/*
 * FILE: test_freertos.h - Unit-test support for FreeRTOS
 *
 * (c) Copyright, Real-Time Innovations, 2020-2020.
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
 * 11Nov2020,fmt  FreeRTOS specific test settings.
 */
/*ce
 * \file
 * \brief Unit-test support for FreeRTOS
 */
#ifndef test_freertos_h
#define test_freertos_h

#include "FreeRTOS.h"
#include "task.h"

/*e \brief
 * UTEST uses malloc(). Ensure that the FreeRTOS memory allocation
 * function is used instead.
 */
#define malloc(x)         pvPortMalloc(x)

/*e \brief
 * UTEST uses free(). Ensure that the FreeRTOS memory free
 * function is used instead.
 */
#define free(x)           vPortFree(x)

#endif /* test_freertos_h */
