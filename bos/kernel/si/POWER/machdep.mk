# @(#)21        1.3  src/bos/kernel/si/POWER/machdep.mk, Makefiles, bos411, 9428A410j 2/16/94 16:34:58
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

INCFLAGS     += -I${MAKETOP}bos/kernel/ios/${TARGET_MACHINE}

POWER_INIT_OFILES	= \
	hardinit.o initmname.o

