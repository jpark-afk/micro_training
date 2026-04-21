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

SET(RTI_MODULE_PUBLIC_HDR test_setting.h
                          test_dll.h 
                          test_setting_impl.h 
                          test_android.h 
                          test_autosar.h 
                          test_threadx.h 
                          test_arinc653.h 
                          test_vxworks653.h 
                          test_deos.h
                          test_freertos.h 
                          test_ios.h)
                 
