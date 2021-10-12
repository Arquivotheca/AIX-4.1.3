#! /usr/bin/ksh
#-----------------------------------------------------------------------
#
#  IBM 7135 RAIDiant Array Subsystem script for the Graphical
#       Disk Array Manager
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#   Invocation:
#      7135_device_config -l darname
#
#-----------------------------------------------------------------------

# First determine the dacs from the darname
b=`get_config -l $2 2> /dev/null | head -1`
dac1=`echo $b | cut -d" " -f1`
dac1state=`echo $b | cut -d" " -f2`
dac2=`echo $b | cut -d" " -f3`
dac2state=`echo $b | cut -d" " -f4`

echo Group:LUN:Status:RAID Level:Device Name:Space:Controller

# Dump devices on first dac, and save highest group number
maxgrp=0
if [ "$dac1state" = "ACTIVE" ]
then
   for dev in `raidmgr -T -l $dac1 2> /dev/null | tail +2 | tr -d ' '`; do
      echo $dev
      maxgrp=`echo $dev | cut -d: -f1`
      if [ "$maxgrp" != "#group" -a "$maxgrp" != "" ]; then
         let "maxgrp = maxgrp + 1"
      else
         maxgrp=0
      fi
   done
fi

# Dump devices on second dac, and add to group number (since it restarts at 0)
if [ "$dac2state" = "ACTIVE" ]
then
   ( echo $maxgrp ; raidmgr -T -l $dac2 2> /dev/null | tail +2 ) | awk -F: '{ if (NR == 1) {grp=$1}; if (NR>1 && NF!=0) {print $1+grp,":",$2,":",$3,":",$4,":",$5,":",$6,":",$7 } }' | tr -d ' '
fi
