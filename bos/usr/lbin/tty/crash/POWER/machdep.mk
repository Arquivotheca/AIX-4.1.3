# @(#)99        1.4  src/bos/usr/lbin/tty/crash/POWER/machdep.mk, cmdtty, bos41J, 9521A_all 5/23/95 16:35:28
#
# COMPONENT_NAME: CMDTTY terminal control commands
#
# FUNCTIONS:
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# LEVEL 1, 5 Years Bull Confidential Information
#

VPATH              +=:${TARGET_MACHINE}:${MAKETOP}/bos/kernext/tty:${MAKETOP}/bos/kernext/tty/${TARGET_MACHINE}:${MAKETOP}/bos/kernext/isa_async:${MAKETOP}/bos/kernext/isa_async/${TARGET_MACHINE}

INCFLAGS	= -I${MAKETOP}/bos/kernel -I${MAKETOP}/bos/kernext/tty \
			-I${MAKETOP}/bos/kernext/tty/${TARGET_MACHINE}

${TARGET_MACHINE}_PROGRAMS	= crash-rs crash-lion crash-cxma

crash-rs_OFILES		= crash-rs.o srs_db.o ${OBJECTS}
crash-lion_OFILES	= crash-lion.o slion_db.o ${OBJECTS}
crash-cxma_OFILES	= crash-cxma.o scxma_print.o ${OBJECTS}

LIBS		= -lIN -lc -lPW

.include <${RULES_MK}>
