# @(#)88	1.3  src/bos/kernext/lft/root/devices.graphics.com.trc.S, lftdd, bos411, 9435D411a 9/1/94 19:14:41
#
#   COMPONENT_NAME: LFTDD
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
#----------------------------------------------------------------------
#
#  LFT trace format template
#
#----------------------------------------------------------------------
#
* /etc/trcfmt
+707 1.0 "LFTDD:  " \
        {{ $a =  $HD & 0xfc00 }} \
        {{ $b =  $a / 1024 }} \
        {{ $func =  $b & 0x3f }} \
        {{ $c  = $HD - $a }} \
        {{ $routine  = $c & 0x03ff }} \
        $routine, \
        100  { "lftcfg  " }, \
        120  { "lftinit " }, \
        140  { "lftterm " }, \
        160  { "lftfont " }, \
        161  { "fntinit " }, \
        162  { "ldfnts  " }, \
        163  { "fntterm " }, \
        170  { "lftswkb " }, \
        200  { "lftopen " }, \
        201  { "lftclse " }, \
        202  { "strinit " }, \
        203  { "strterm " }, \
        204  { "lftout  " }, \
        205  { "vtmupd  " }, \
        206  { "vtmupd3 " }, \
        207  { "KiSak   " }, \
        208  { "KiCb    " }, \
        209  { "KiOffl  " }, \
        210  { "KiInit  " }, \
        211  { "KiTerm  " }, \
        212  { "lftst   " }, \
        300  { "fkproc  " }, \
        301  { "crfkp   " }, \
        302  { "killfkp " }, \
        303  { "pinfont " }, \
        304  { "fontadr " }, \
        305  { "att_shm " }, \
        306  { "det_shm " }, \
        307  { "partfnt " }, \
        308  { "dd_fun  " }, \
        320  { "fsqueue " }, \
        321  { "fsp_enq " }, \
        322  { "fsp_deq " }, \
        340  { "k_ftok  " }, \
        400  { "lft_dpms" }, \
        \*  { $routine%X2"    " } \
        $func%D2, \
        0  { " [ent ]" }, \
        32 { " [exit]" }, \
        \* { " ["$func%X2"]"  } \
        HT & 0x07, \
        02 {O2 "  d0=" X4 }, \
        06 {O2 "  d0=" X4 " d1=" X4 " d2=" X4 " d3=" X4 " d4=" X4 }

