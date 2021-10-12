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
#      7135_subsys_status
#
#-----------------------------------------------------------------------

echo statusname:color:description:icon:iconx:icony:conn1x:conn1y:conn2x:conn2y
echo A:green:Dual Active Ctrlr:/etc/array/bitmaps/7135_ctrlr.dualactive.pm:0:0:0:45:0:50
echo B:blue:Single Active Ctrlr:/etc/array/bitmaps/7135_ctrlr.active.pm:0:0:0:45:0:50
echo C:yellow:Passive Ctrlr:/etc/array/bitmaps/7135_ctrlr.passive.pm:0:0:0:45:0:50
echo D:black:Failed Ctrlr:/etc/array/bitmaps/7135_ctrlr.failed.pm:0:0:0:45:0:50
echo 1:blue:Single Active Ctrlr:/etc/array/bitmaps/7135_ctrlr.active.pm:0:0:0:45:0:50

