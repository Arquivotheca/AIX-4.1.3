# @(#)18        1.2  src/bos/kernel/lib/libsys/POWER/machdep.mk, libsys, bos411, 9428A410j 1/26/93 09:34:21
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

INCFLAGS     = -I${MAKETOP}bos/kernel/ml/${TARGET_MACHINE}

POWER_OFILES	= \
	d_align.o d_roundup.o

