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
#      7135_drive_status
#
#-----------------------------------------------------------------------

echo statusname:color:icon:description
echo S:green:/etc/array/bitmaps/scsipv.spare.pm:Spare
echo A:black:/etc/array/bitmaps/scsipv.dead.pm:Failed
echo R:green:/etc/array/bitmaps/scsipv.warn.pm:Replaced
echo W:green:/etc/array/bitmaps/scsipv.warn.pm:Warning
echo P:black:/etc/array/bitmaps/scsipv.warn.pm:Parm Mismatch
echo C:black:/etc/array/bitmaps/scsipv.warn.pm:Controller
echo F:black:/etc/array/bitmaps/scsipv.warn.pm:Formatting
echo H:green:/etc/array/bitmaps/scsipv.spare.pm:Hot Spare
echo G:black:/etc/array/bitmaps/scsipv.warn.pm:Wrong Drive
echo O:green:/etc/array/bitmaps/scsipv.opt.pm:Optimal
echo N:white:/etc/array/bitmaps/scsipv.none.pm:Non-existent

