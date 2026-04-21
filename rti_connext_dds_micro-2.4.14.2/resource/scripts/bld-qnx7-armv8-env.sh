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

QNX_INSTALL_DIR=/opt/toolchains/qnx700
QNX_TARGET=$QNX_INSTALL_DIR/target/qnx7
QNX_HOST=$QNX_INSTALL_DIR/host/linux/x86_64
QNX_CONFIGURATION=$QNX_INSTALL_DIR/../.qnx
MAKEFLAGS=-I$QNX_INSTALL_DIR/target/qnx7/usr/include
# path
PATH=$QNX_INSTALL_DIR/host/linux/x86_64/usr/bin:$PATH

export QNX_TARGET QNX_HOST QNX_CONFIGURATION MAKEFLAGS PATH
unset PYTHONPATH

echo QNX_HOST=$QNX_HOST
echo QNX_TARGET=$QNX_TARGET
echo MAKEFLAGS=$MAKEFLAGS
