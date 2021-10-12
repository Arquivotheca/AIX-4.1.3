# @(#)39        1.2  src/bos/kernext/disp/ped/pedmacro/hw_timer.s, pedmacro, bos411, 9428A410j 3/17/93 20:07:46
#
#   COMPONENT_NAME: PEDMACRO
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1991,1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#**********************************************************************

.rtc_upper:
        .globl  .rtc_upper
        mfspr   3,4
        br

.rtc_lower:
        .globl  .rtc_lower
        mfspr   3,5
        br
