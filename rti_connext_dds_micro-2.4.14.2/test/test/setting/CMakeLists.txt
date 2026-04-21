###############################################################################
# (c) Copyright, Real-Time Innovations 2020-2020
#
# All rights reserved.
#
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
# Description:
# ------------
# Top level test CMakeLists.txt for building Micro port validation
#
# Modification History
# --------------------
# 14oct20 fmt Copyright added
################################################################################

SET(RTI_SUBMODULE_SRC UTEST_System.c UTEST_String.c UTEST_Stdio.c 
                      UTEST_Runner.c UTEST_Property.c UTEST_File.c 
                      UTEST_Main.c UTEST_Output.c)

SET(RTI_SUBMODULE_HDR UTEST_System.h UTEST_String.h UTEST_Stdio.h 
                      UTEST_Runner.h UTEST_Property.h UTEST_File.h 
                      UTEST_Android.h UTEST_Output.h UTEST_iOS.h)

SET(RTI_MODULE_TESTLIB_SHARED ${RTI_LIB_PREFIX}_${RTI_MODULE_NAME}${RTI_LIB_SUFFIX_DYNAMIC})
SET(RTI_MODULE_TESTLIB_STATIC ${RTI_LIB_PREFIX}_${RTI_MODULE_NAME}${RTI_LIB_SUFFIX_STATIC})

IF (RTIME_ANDROID)
    LIST(APPEND RTI_SUBMODULE_SRC  UTEST_Android.c)
ENDIF()

INCLUDE (${RESOURCEHOME}/cmake/cmake_srcC_submodule.txt)
