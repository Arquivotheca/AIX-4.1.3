#! /usr/bin/ksh
#-----------------------------------------------------------------------
#
#  IBM 7135 RAIDiant Array Subsystem script for the Graphical
#       Disk Array Manager
#
#  (C) COPYRIGHT International Business Machines Corp. 1994
#  All Rights Reserved
#
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#  Invocation:
#
#  7135_set_subsys_info -health_check -l <darname> -a health_check = <health_freq>
#                       -load_balance -l <darname> -a load_balance = <yes/no> -a bal_freq = <bal_freq>
#                       -aen <darname> <aen_freq>
#
#-----------------------------------------------------------------------

case "$1" in
   "-health_check")
         if [ $7 -gt 9999 -o $7 -lt 1 ]; then
            echo The health check frequency must be in the range of 1 to 9999 seconds.
            exit 1
         fi
         echo chdev -l $3 -a hlthchk_freq=$7
         chdev -l $3 -a hlthchk_freq=$7
         break;;

   "-load_balance")
         if [ ${11} -gt 9999 -o ${11} -lt 1 ]; then
            echo The load balancing frequency must be in the range of 1 to 9999 seconds.
            exit 1
         fi
         echo chdev -l $3 -a load_balancing=$7
         chdev -l $3 -a load_balancing=$7
         echo chdev -l $3 -a balance_freq=${11}
         chdev -l $3 -a balance_freq=${11}
         break;;

   "-aen")
         if [ $3 -lt 1 -o $3 -gt 9999 ]; then
            echo The polled AEN frequency must be in the range of 1 to 9999 seconds.
            exit 1
         fi
         echo chdev -l $2 -a aen_freq=$3
         chdev -l $2 -a aen_freq=$3
         break;;

   "-spindle_sync")
         echo raidmgr -N -l $3
         raidmgr -N -l $3
         break;;

   *)    break;;
esac
