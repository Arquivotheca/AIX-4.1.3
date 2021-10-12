#!/bin/ksh
#
# @(#)04        1.2  src/bos/usr/lib/assist/unmount_cd.sh, cmdassist, bos411, 9428A410j 2/10/94 08:27:01
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

# NAME:  unmount_cd
#
# FUNCTION:
#    If the installation device is a CD-ROM, this program will be called 
#    after each install task where a CD was mounted.
#
# EXECUTION ENVIRONMENT:
#
#
# INPUT VALUES:
#    The installation device ($1).
#
# OUTPUT VALUES:
#    None.
#
# EXIT VALUES:
#    0  Successful completion.
#


#----------------------------------------------------------------
# Check to see if the CD is mounted and, if so, unmount it.
# Remove the file system, and remove the mount directory.
# Ignore all errors.
#----------------------------------------------------------------

DEVFILE=/dev/`basename $1`
mount | fgrep $DEVFILE > /dev/null 2>&1
if [ $? = 0 ]
then
  # Unmount the CD.
  #
  unmount /__instmnt__ >/dev/null 2>&1
fi
  
# Remove the CD-ROM file system.
#
rmfs /__instmnt__ >/dev/null 2>&1

# Remove the directory where the CD was mounted.
#
rmdir /__instmnt__ >/dev/null 2>&1


# Exit with a good return code.
#
exit 0
