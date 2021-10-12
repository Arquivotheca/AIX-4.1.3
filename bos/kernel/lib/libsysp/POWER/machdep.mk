# @(#)19        1.4  src/bos/kernel/lib/libsysp/POWER/machdep.mk, libsysp, bos411, 9428A410j 4/21/94 11:08:06
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

POWER_OFILES	= \
	pioutil.o tlbi.o ioccdelay.o cacheinval.o

cacheinval.o_INCFLAGS	=-I${MAKETOP}/bos/kernel/ml/${TARGET_MACHINE}
