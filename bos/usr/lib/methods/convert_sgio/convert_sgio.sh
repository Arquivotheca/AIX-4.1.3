#!/bin/ksh
# @(#)49	1.2  src/bos/usr/lib/methods/convert_sgio/convert_sgio.sh, inputdd, bos41J, 9518A_all 5/1/95 16:03:15
#
#   COMPONENT_NAME: inputdd
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
#   This purpose of this script is to migrate serial Dials/LPFKeys
#   device support. This script only needs to be executed if
#   Dials/LPFKeys devices have previously been defined/configured
#   on the target machine. The script performs the following steps:
#
#     1. Determine whether any serially attached Dials/LPFKeys devices
#        require migration. If so, proceed to Step 2. Otherwise, exit.
#
#     2. Acquire device information necessary to perform migration. This
#        includes obtaining the device's logical name, parent adapter,
#        connection port, and device type.
#
#     3. Remove any serially attached Dials/LPFKeys devices requiring
#        migration.
#
#        NOTE: There will be at most 2 devices requiring migration.
#              Due to restrictions, all serially attached Dials/LPFKeys
#              devices must be unconfigured/undefined prior to completing 
#              the remaining steps.
#
#     4. Define an available TTY on the parent adapter and connection port
#        acquired in Step 2. The TTY device logical name is passed onto the
#        Step 5.
#
#     5. Define/configure a serial Dials/LPFKeys device with the TTY device
#        and Dials/LPFKeys logical names acquired in Steps 2 and 5,
#        respectively.
#
#     6. Repeat Steps 4 and 5 for second migrating device, if necessary.
#
#   If a failure occurs during Step 3, close all programs which access the
#   device and retry. If a failure occurs during Steps 4 or 5, then retry
#   these steps manually via SMIT, addressing problems accordingly.
#


rmdev_error ()
{
  dspmsg -s 1 sgio_convert.cat 1 'Could not remove %s. Please close all programs which\naccess this device (including X-server) before retrying.\n' $1
  exit 1
}

mktty_error ()
{
  dspmsg -s 1 sgio_convert.cat 2 'Unable to complete configuration for serial %s device. TTY\nconfiguration on serial port with parent %s and port %s failed.\n' $1 $2 $3
  dspmsg -s 1 sgio_convert.cat 4 'Please refer to the "Setting Up X11 Graphic Input Devices" section\nof your documentation for information on how to configure graphic\ninput devices.\n'
  exit 1
}

mksgio_error ()
{
  dspmsg -s 1 sgio_convert.cat 3 'Unable to complete configuration for serial %s device. Device\nconfiguration with device instance %s and TTY %s failed.\n' $1 $2 $3
  dspmsg -s 1 sgio_convert.cat 4 'Please refer to the "Setting Up X11 Graphic Input Devices" section\nof your documentation for information on how to configure graphic\ninput devices.\n'
  exit 1
}

#
# Locate native serial Dials/LPFKeys devices and get necessary
# device information.
#
index=1
odmget -q"ddins='giodd' AND parent LIKE 'sa?'" CuDv | sed -n '/name = /p' | sed 's/name = /name=/' | sed 's/"//g' |
(while read giodev
do
  stuff=`odmget -q $giodev CuDv | sed 's/"//g' |
  awk '
  {
    if ( $1 == "name" )
    {
      lname = $3
    }
    if ( $1 == "parent" )
    {
      parent = $3
    }
    if ( $1 == "connwhere" )
    {
      connwhere = $3
    }
    if ( $1 == "PdDvLn" )
    {
      PdDvLn = $3
    }
  }
  END { print lname " " parent " " connwhere " " PdDvLn }'`

  if [ index -eq 1 ]; then
    set -- $stuff
    lname1=$1
    parent1=$2
    connwhere1=$3
    IFSOLD=$IFS
    IFS="/"
    set -- $4
    type1=$3
    IFS=$IFSOLD
    index=2
  else
    set -- $stuff
    lname2=$1
    parent2=$2
    connwhere2=$3
    IFSOLD=$IFS
    IFS="/"
    set -- $4
    type2=$3
    IFS=$IFSOLD
  fi
done

#
# Remove all native serial Dials/LPFKeys devices (at most 2).
#
if [ "$lname1" != "" ]; then
  rmdev -d -l $lname1 > /dev/null 2>&1
  if [ $? -ne 0 ]; then
    rmdev_error $lname1
  fi
fi

if [ "$lname2" != "" ]; then
  rmdev -d -l $lname2 > /dev/null 2>&1
  if [ $? -ne 0 ]; then
    rmdev_error $lname2
  fi
fi

#
# Define/configure new native serial Dials/LPFKeys devices
# and underlying tty devices.
#
if [ "$lname1" != "" ]; then
  tty_dev=`mkdev -c tty -t 'tty' -s 'rs232' -p $parent1 -w $connwhere1 2> /dev/null`
  if [ $? -ne 0 ]; then
    mktty_error $type1 $parent1 $connwhere1
  else
    tty_dev=`echo $tty_dev |cut -f1 -d" "`
    mkdev -s sgio -t $type1 -l $lname1 -a ttydevice=$tty_dev 2> /dev/null
    if [ $? -ne 0 ]; then
      mksgio_error $type1 $lname1 $tty_dev
    fi
  fi
fi

if [ "$lname2" != "" ]; then
  tty_dev=`mkdev -c tty -t 'tty' -s 'rs232' -p $parent2 -w $connwhere2 2> /dev/null`
  if [ $? -ne 0 ]; then
    mktty_error $type2 $parent2 $connwhere2
  else
    tty_dev=`echo $tty_dev |cut -f1 -d" "`
    mkdev -s sgio -t $type2 -l $lname2 -a ttydevice=$tty_dev 2> /dev/null
    if [ $? -ne 0 ]; then
      mksgio_error $type2 $lname2 $tty_dev
    fi
  fi
fi)

exit 0
