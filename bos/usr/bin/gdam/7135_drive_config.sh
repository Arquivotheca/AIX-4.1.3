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
#      7135_drive_config -l darname
#
#-----------------------------------------------------------------------

# First determine the dacs from the darname
b=`get_config -l $2 | head -1`
dac1=`echo $b | cut -d" " -f1`
dac1state=`echo $b | cut -d" " -f2`
dac2=`echo $b | cut -d" " -f3`
dac2state=`echo $b | cut -d" " -f4`

if [ "$dac1state" = "ACTIVE" -a "$dac2state" = "ACTIVE" ] ; then
   raidmgr -B -l $dac1 > /tmp/$$7135.drv1 2> /dev/null
   if [ $? = 0 ]; then
      raidmgr -B -l $dac2 | cut -d: -f8 | paste -d'\0' /tmp/$$7135.drv1 - 2> /dev/null
   else
      raidmgr -B -l $dac2
   fi
   rm /tmp/$$7135.drv1
else
   if [ "$dac1state" = "ACTIVE" ]; then
      raidmgr -B -l $dac1
   else
      if [ "$dac2state" = "ACTIVE" ]; then
         raidmgr -B -l $dac2
      fi
   fi
fi

