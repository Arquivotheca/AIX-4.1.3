#!/bin/ksh
#
# @(#)06        1.12  src/bos/usr/lib/assist/install_pkg.sh, cmdassist, bos411, 9428A410j 5/30/94 08:19:48
#
#   COMPONENT_NAME:  cmdassist
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# NAME:  install_pkg
#
# FUNCTION:
#   This function checks to see if a specified package is installed on
#   the system.  If the package is not installed, then the install
#   device will be obtained from the /var/adm/sw/__assistinfo file,
#   and an attempt will be made to install the package from that device.
#
# EXECUTION ENVIRONMENT:
#
#
# INPUT VALUES:
#    The installation package to be installed if it is not already
#    installed on the system.
#
# OUTPUT VALUES:
#    None.
#
# RETURNS:
#    0       Successful completion.
#    1       Error.
#


# Set variables for message numbers.  These must match the values
# in the include file cmdassist_msg.h which is generated from the
# cmdassist.msg file.
#
MF_CMDASSIST=cmdassist.cat  # Message file
ASSIST_ERR_SET=1            # Message set number
ASSIST_INSTL_PKG_E=10       # Install failure on packages

# First parameter input is list of packages to install.
list=$1

# Initialize install list to null.
instl_list=""

# Set variable for name of file that contains installation information
# from BOS install.
ASSIST_FILE="/var/adm/sw/__assistinfo"

# Get the install input device from the /var/adm/sw/__assistinfo file.
input_device=`sed -n "s?.*INSTALL_DEVICE=[ ]*\(.*\)?\1?p"<$ASSIST_FILE`

# Set cd flag to false.
cd=0

# Check to see if the specified packages are installed.
for i in $list
do
  lslpp -l $i >/dev/null 2>&1
  if [[ $? != 0 ]]
  then
    instl_list="$instl_list $i"
  fi
done

# If any packages are not installed, then attempt to install them.
if [[ $instl_list != "" ]]
then

  # Check the input device for CD-ROM.
  rc=`/usr/lib/assist/check_cd $input_device`

  # If the input device is CD
  if [[ $rc = 1 ]]
  then

    # Mount the CD and set install device to mounted directory.
    install_device=`/usr/lib/assist/mount_cd $input_device`
    if [[ $? = 0 ]]
    then
      # Set cd flag to true.
      cd=1
    else
      # Exit with an error if a CD could not be mounted.
      dspmsg -s $ASSIST_ERR_SET $MF_CMDASSIST $ASSIST_INSTL_PKG_E \
'0851-005 install_assist:  The following packages \
could not be installed from device %s.\n' $input_device
      for i in $instl_list
      do
	echo "  " $i
      done
      echo " "
      exit 1
    fi

  else
    # Set install device to input device.
    base=`basename $input_device`
    if [[ $base = $input_device ]]
    then
      install_device="/dev/$input_device"
    else
      install_device=$input_device
    fi

  fi

  instl_lang=`/usr/lib/instl/automsg -d $install_device`

  # Install the specified packages.  Did not redirect the output to /dev/null
  # because smit will hang if you do.
  installp -aQgqXd $install_device $instl_list $instl_lang

  # Save return code from installp in order to check it after doing an
  # unmount if the input device was a CD.
  rc=$?

  # If input device was a CD, then unmount the CD.
  if [[ $cd = 1 ]]
  then
    /usr/lib/assist/unmount_cd $input_device >/dev/null 2>&1
  fi

  # If the installp failed, display an error message and do error exit.
  if [[ $rc != 0 ]]
  then
    dspmsg -s $ASSIST_ERR_SET $MF_CMDASSIST $ASSIST_INSTL_PKG_E \
'0851-005 install_assist:  The following packages \
could not be installed from device %s.\n' $input_device
    for i in $instl_list
    do
      echo "  " $i
    done
    echo " "
    exit 1
  fi

fi

# Exit with good return code.
exit 0

