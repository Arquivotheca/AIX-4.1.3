# @(#)75	1.1  src/bos/usr/ccs/lib/libc/POWER/pgcrt0.s, libcgen, bos411, 9428A410j 1/18/90 10:09:29
#########################################################################
#
#  COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
#
#  FUNCTIONS: pcrt0
#
#  ORIGINS: 3 10 27
#
#  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#  combined with the aggregated modules for this product)
#                   SOURCE MATERIALS
#  (C) COPYRIGHT International Business Machines Corp. 1985, 1990
#  All Rights Reserved
#
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
##########################################################################
# Profiling setup for BSD profiling

define(profiling,`yes')
define(profispg,`yes')
include(crt0main.s)
