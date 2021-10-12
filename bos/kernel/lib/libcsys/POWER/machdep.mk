# @(#)17        1.3  src/bos/kernel/lib/libcsys/POWER/machdep.mk, Makefiles, bos411, 9428A410j 12/12/93 10:50:51
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 10,27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

INCFLAGS = -I${MAKETOP}bos/kernel/ml/${TARGET_MACHINE}

POWER_SYS_OFILES	= \
	udiv.o compi64.o compu64.o divi64.o divu64.o maxi64.o multi64.o
