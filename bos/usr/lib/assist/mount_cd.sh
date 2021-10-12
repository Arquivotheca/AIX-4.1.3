#!/bin/ksh
#
# @(#)03        1.4  src/bos/usr/lib/assist/mount_cd.sh, cmdassist, bos411, 9428A410j 4/1/94 08:58:11
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

# NAME: mount_cd
#
# FUNCTION:
#    This program is called if the installation device is CD-ROM.
#
# EXECUTION ENVIRONMENT:
#
#
# INPUT VALUES:
#    The installation device ($1).
#
# OUTPUT VALUES:
#    The directory containing the install images.
#
# EXIT VALUES:
#    0       Successful completion.
#    1       Error.
#


# Set variables for message numbers.  These must match the values
# in the include file cmdassist_msg.h which is generated from the
# cmdassist.msg file.
#
MF_CMDASSIST=cmdassist.cat  # Message file
ASSIST_ERR_SET=1            # Message set number
ASSIST_CDRFS_E=11           # CD-ROM file system creation error
ASSIST_MNT_CD_E=12          # CD mount error
ASSIST_INVALID_CD_E=13      # Invalid installation CD

# Set CD mount point.
#
MNT_POINT=/__instmnt__

DEVFILE=/dev/`basename $1`

# Attempt to unmount the CD in case it was previously mounted.
#
/usr/lib/assist/unmount_cd $DEVFILE >/dev/null 2>&1

# Make the mount directory and create the CD-ROM file system.
#
mkdir $MNT_POINT >/dev/null 2>&1
crfs -v cdrfs -p ro -d $DEVFILE -m $MNT_POINT -A no >/dev/null 2>&1
if [[ $? != 0 ]]
then
  dspmsg -s $ASSIST_ERR_SET $MF_CMDASSIST $ASSIST_CDRFS_E \
'0851-006 mount_cd:  Cannot create a \
CD_ROM file system for device %s \
with mount point %s.\n' $DEVFILE $MNT_POINT
  /usr/lib/assist/unmount_cd $DEVFILE >/dev/null 2>&1
  exit 1
fi

# Mount the CD.
#
mount $MNT_POINT >/dev/null 2>&1
if [[ $? != 0 ]]
then
  dspmsg -s $ASSIST_ERR_SET $MF_CMDASSIST $ASSIST_MNT_CD_E \
'0851-007 mount_cd:  Cannot mount the \
CD_ROM file system for device %s \
on mount point %s.\n' $DEVFILE $MNT_POINT
  /usr/lib/assist/unmount_cd $DEVFILE >/dev/null 2>&1
  exit 1
fi

# Return directory where install images are located if the
# CD is a valid installation CD.
#
if [[ -f $MNT_POINT/usr/sys/inst.images/.toc ]]
then
  # Location of install images for 4.1 installation CD.
  echo "$MNT_POINT/usr/sys/inst.images"
else
  if [[ -f $MNT_POINT/.toc ]]
  then
    # Location of install images for release 3.2 CD.
    echo "$MNT_POINT"
  else
    /usr/lib/assist/unmount_cd $DEVFILE >/dev/null 2>&1
  dspmsg -s $ASSIST_ERR_SET $MF_CMDASSIST $ASSIST_INVALID_CD_E \
'0851-008 mount_cd:  The CD is not a valid \
installation CD.\n'
    exit 1
  fi
fi
exit 0


