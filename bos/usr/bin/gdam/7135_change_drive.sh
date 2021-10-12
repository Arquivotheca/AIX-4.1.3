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
#       7135_change_drive <status> <darname> <drvlist>
#
#-----------------------------------------------------------------------

for drv in $4; do
   for drvline in `7135_drive_config -l $2 | tail +2 | tr -d ' '`; do
      d=`echo $drvline | cut -d: -f1`
      h=`echo $drvline | cut -d: -f8 | cut -d, -f1`
      if [ $drv = $d ]; then
         if [ "$h" != "" ]; then
            echo raidmgr -F -l $h -d $1 -y $drv
            raidmgr -F -l $h -d $1 -y $drv 2> /tmp/gdam.err
            cat /tmp/gdam.err
         else
            dac=`lsattr -E -l $2 | grep act_controller | awk '{ print $2}' | cut -c1-4`
            echo raidmgr -F -l $dac -d $1 -y $drv
            raidmgr -F -l $dac -d $1 -y $drv 2> /tmp/gdam.err
            cat /tmp/gdam.err
         fi
      fi
   done
done
