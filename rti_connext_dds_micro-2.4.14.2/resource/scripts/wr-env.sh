################################################################################
# (c) Copyright, Real-Time Innovations 2020-2020
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################

WIND_HOME=/local/VxWorks/GPP-3.9
WIND_BASE=${WIND_HOME}/vxworks-6.9
WIND_USR=${WIND_HOME}/vxworks-6.9/target/usr
WIND_GNU_PATH=${WIND_HOME}/gnu/4.3.3-vxworks-6.9
LD_LIBRARY_PATH=${WIND_HOME}/lmapi-5.0/x86-linux2/lib/:${LD_LIBRARY_PATH}
WIND_HOST_TYPE=x86-linux2
GPLUSPLUSLDFLAGS=-r

export WIND_HOME WIND_BASE WIND_USR 
export WIND_GNU_PATH LD_LIBRARY_PATH WIND_HOST_TYPE GPLUSPLUSLDFLAGS