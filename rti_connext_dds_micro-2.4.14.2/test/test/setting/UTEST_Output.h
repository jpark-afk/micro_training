/*
 * FILE: UTEST_Output.h - Unit-test output support
 *
 * (c) Copyright, Real-Time Innovations, 2013-2020.
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
 * 08nov2013,tk Written
 */
/*ce
 * \file
 * \brief Unit-test output support
 */
#ifndef UTEST_Output_h
#define UTEST_Output_h

#include "UTEST_Property.h"
#include "UTEST_Runner.h"
#include "UTEST_File.h"
#include "UTEST_System.h"
#include "UTEST_String.h"
#include "UTEST_Stdio.h"

struct UTEST_Output
{
    int lua_fd;
    int log_fd;
    int sql_fd;
};

#endif

