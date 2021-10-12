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
#      7135_get_drive_info -blksize raidlevel drivelist
#                          -freespace dacname raidlevel drivelist
#                          -segsize raidlevel
#
#-----------------------------------------------------------------------

case "$1" in
   "-blksize") if [ "$2" = "3" ]; then
                  drive_count=`echo $3 | wc -w`
                  if [ $drive_count = 3 ]; then
                     echo 1024
                  else
                     if [ $drive_count = 5 ]; then
                        echo 2048
                     else
                        echo -Invalid number of drives-
                     fi
                  fi
               else
                  echo 512
               fi
               break;;

   "-freespace") if [ "$4" = "" ]; then
                    exit 0
                 fi
                 if [ "$3" = "3" ]; then
                    segsz=1
                    drive_count=`echo $4 | wc -w`
                    if [ $drive_count = 3 ]; then
                       blksz=1024
                    else
                       if [ $drive_count = 5 ]; then
                          blksz=2048
                       else
                          blksz=512
                       fi
                    fi
                 else
                    segsz=512
                    blksz=512
                 fi
                 raidmgr -H -l $2 -r $3 -b $blksz -n 1 -s $segsz -0 0 -t 1 -R 256 -e "$4" 2> /dev/null
                 if [ $? != 0 ]; then
                    echo ERROR: Invalid drives selected, or error accessing drives.
                    exit 1
                 fi
                 break;;

   "-segsize") if [ "$2" = "3" ]; then
                  echo 1
               else
                  echo 512
               fi
               break;;

   *) echo ERROR: Invalid command option: $1
      exit 1
      break;;
esac
