################################################################################
#
# Copyright (c) 2019-2024 Real-Time Innovations, Inc. All rights reserved.
#
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################
QNX_TARGET=/opt/qnx700/target/qnx7
QNX_HOST=/opt/qnx700/host/linux/x86_64
QNX_CONFIGURATION=/opt/qnx700/.qnx
MAKEFLAGS=-I/opt/qnx700/target/qnx7/usr/include
PATH=/opt/qnx700/host/linux/x86_64/usr/bin:$QNX_CONFIGURATION/bin:/opt/qnx700/jre/bin:$PATH

export QNX_TARGET QNX_HOST QNX_CONFIGURATION MAKEFLAGS PATH
unset PYTHONPATH

echo QNX_HOST=$QNX_HOST
echo QNX_TARGET=$QNX_TARGET
echo MAKEFLAGS=$MAKEFLAGS

