# @(#)15	1.3  src/bos/kernel/io/POWER/machdep.mk, Makefiles, bos411, 9428A410j 11/8/93 08:08:39
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

.include <${MAKETOP}bos/kernel/Kernel.mk>

PGSDIR =
SNOOPYDIR =

.if defined(SNOOPY) && ${SNOOPY} == "-D_SNOOPY"
SNOOPYDIR = snoopy
PGSDIR = pgsdb
.endif

.if defined(PGSDB) && ${PGSDB} == "-D_PEGASUS"
PGSDIR = pgsdb
.endif

POWER_SUBDIRS	= machdd ${SNOOPYDIR} ${PGSDIR}

POWER_PINNED_OFILES	= \
	trchka.o

trchka.o_INCFLAGS	= -I${TARGET_MACHINE} -I${MAKETOP}/bos/kernel/ml -I${MAKETOP}/bos/kernel/ml/${TARGET_MACHINE}
