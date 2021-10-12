#!/bin/ksh
#
# @(#)02        1.2  src/bos/usr/lib/assist/check_cd.sh, cmdassist, bos41J, 9524A_all 6/12/95 09:33:21
#
#   COMPONENT_NAME:  cmdassist
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# NAME: check_cd
#
# FUNCTION:
#    This program will check if the type of the device is CD-ROM.
#
# EXECUTION ENVIRONMENT:
#
#
# INPUT VALUES:
#    The installation device ($1).
#
# OUTPUT VALUES:
#    1  The installation device is CD-ROM.
#    0  The installation device is not CD-ROM.
#
# EXIT VALUES:
#    0  Successful completion.
#    1  Error.
#


export ODMDIR=/etc/objrepos

# Check if there is a device with the specified device name.
DEVNAME=`basename $1`
DEV=`echo $DEVNAME | cut -f1 -d'.'`
lsdev -C -S a| fgrep $DEV >/dev/null 2>&1
if [ $? != 0 ]
then
   echo '0'
   exit 0
fi

# Get the type of the device.
DEVTYPE=`lsdev -C -S a -F 'name class' | grep "^$DEV " | awk ' { print $2 } '`

# If device is CD-ROM
if [ x$DEVTYPE = xcdrom ]
then
  # Set value for device is CD-ROM. 
  echo '1'
else
  # Set value for device is not CD-ROM. 
  echo '0'
fi

# Exit with good return code.
exit 0
