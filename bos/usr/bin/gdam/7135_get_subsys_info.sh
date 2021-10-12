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
#      7135_get_subsys_info -hlthfreq -l darname
#                           -balance_freq -l darname
#                           -aen_freq -l darname
#                           -c darname
#
#-----------------------------------------------------------------------

case "$1" in
   "-c") b=`lsattr -E -l $3 | grep all_controller | awk '{ print $2 }'`
         echo $b | cut -c1-4
         echo $b | cut -c6-9
         break;;

   "-hlthfreq")
         lsattr -E -l $3 -a hlthchk_freq | cut -d" " -f2
         break;;

   "-balance_freq")
         lsattr -E -l $3 -a balance_freq | cut -d" " -f2
         break;;

   "-aen_freq")
         lsattr -E -l $3 -a aen_freq | cut -d" " -f2
         break;;

   *) break;;
esac
