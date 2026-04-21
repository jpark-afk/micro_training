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

QNX_TARGET=/opt/qnx660/target/qnx6
QNX_HOST=/opt/qnx660/host/linux/x86
QNX_CONFIGURATION=/opt/qnx660/.qnx
MAKEFLAGS=-I/opt/qnx660/target/qnx6/usr/include
PATH=/opt/qnx660/host/linux/x86/usr/bin:/opt/qnx660/.qnx/bin:/opt/qnx660/jre/bin:$PATH

export QNX_TARGET QNX_HOST QNX_CONFIGURATION MAKEFLAGS PATH
unset PYTHONPATH

# Include CAR2.1 Environments if present
qnxCarDeployment=/opt/qnx660/deployment/qnx-car
[ -x $qnxCarDeployment/qnxcar-env.sh ] && 
   . $qnxCarDeployment/qnxcar-env.sh
