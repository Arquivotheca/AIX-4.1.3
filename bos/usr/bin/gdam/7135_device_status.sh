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

echo statusname:color:icon:description
echo A:green:/etc/array/bitmaps/lun.opt.pm:Optimal
echo B:green:/etc/array/bitmaps/lun.opt.pm:Optimal-Parameter Mismatch
echo C:green:/etc/array/bitmaps/lun.opt.pm:Optimal-Parity Scan in Progress
echo D:green:/etc/array/bitmaps/lun.opt.pm:Optimal-Parity Scan Mismatch
echo E:green:/etc/array/bitmaps/lun.opt.pm:Optimal-Formatting Replaced Drive
echo F:green:/etc/array/bitmaps/lun.opt.pm:Optimal-Parity Scan Aborted
echo G:yellow:/etc/array/bitmaps/lun.warn.pm:Degraded-Drive Failed
echo H:yellow:/etc/array/bitmaps/lun.warn.pm:Degraded-Formatting Replaced Drive
echo I:yellow:/etc/array/bitmaps/lun.warn.pm:Reconstructing
echo J:red:/etc/array/bitmaps/lun.dead.pm:Dead-Multiple Drives Failed
echo K:red:/etc/array/bitmaps/lun.dead.pm:Dead-Parameter Mismatch
echo L:red:/etc/array/bitmaps/lun.dead.pm:Dead-Channel Mismatch
echo M:red:/etc/array/bitmaps/lun.dead.pm:Dead-ID Mismatch
echo N:red:/etc/array/bitmaps/lun.dead.pm:Dead-Format in Progress
echo O:red:/etc/array/bitmaps/lun.dead.pm:Dead-Awaiting Format
echo P:red:/etc/array/bitmaps/lun.dead.pm:Dead-Replaced Wrong Drive
echo Q:red:/etc/array/bitmaps/lun.dead.pm:Dead-Component Failure
echo R:red:/etc/array/bitmaps/lun.dead.pm:Unknown Error
echo a:green:/etc/array/bitmaps/lun.opt.pm:Optimal-Reserved
echo b:green:/etc/array/bitmaps/lun.opt.pm:Optimal-Parameter Mismatch-Reserved
echo c:green:/etc/array/bitmaps/lun.opt.pm:Optimal-Parity Scan in Progress-Reserved
echo d:green:/etc/array/bitmaps/lun.opt.pm:Optimal-Parity Scan Mismatch-Reserved
echo e:green:/etc/array/bitmaps/lun.opt.pm:Optimal-Formatting Replaced Drive-Reserved
echo f:green:/etc/array/bitmaps/lun.opt.pm:Optimal-Parity Scan Aborted-Reserved
echo g:yellow:/etc/array/bitmaps/lun.warn.pm:Degraded-Drive Failed-Reserved
echo h:yellow:/etc/array/bitmaps/lun.warn.pm:Degraded-Formatting Replaced Drive-Reserved
echo i:yellow:/etc/array/bitmaps/lun.warn.pm:Reconstructing-Reserved
echo j:red:/etc/array/bitmaps/lun.dead.pm:Dead-Multiple Drives Failed-Reserved
echo k:red:/etc/array/bitmaps/lun.dead.pm:Dead-Parameter Mismatch-Reserved
echo l:red:/etc/array/bitmaps/lun.dead.pm:Dead-Channel Mismatch-Reserved
echo m:red:/etc/array/bitmaps/lun.dead.pm:Dead-ID Mismatch-Reserved
echo n:red:/etc/array/bitmaps/lun.dead.pm:Dead-Format in Progress-Reserved
echo o:red:/etc/array/bitmaps/lun.dead.pm:Dead-Awaiting Format-Reserved
echo p:red:/etc/array/bitmaps/lun.dead.pm:Dead-Replaced Wrong Drive-Reserved
echo q:red:/etc/array/bitmaps/lun.dead.pm:Dead-Component Failure-Reserved
echo r:red:/etc/array/bitmaps/lun.dead.pm:Unknown Error-Reserved
echo -:green::Free Space

