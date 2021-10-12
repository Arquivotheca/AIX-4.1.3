# @(#)83        1.2  src/bos/usr/bin/man/Makefile, cmdman, bos411, 9428A410j 10/19/93 17:16:18
# COMPONENT_NAME: (CMDMAN) commands that allow users to read online
# documentation
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

PROGRAMS	= man
MSGHDRS		= man_msg.h
CATFILES	= man.cat
HFILES		= ${MSGHDRS}

OFILES		= man.o knowset.o
CFLAGS		= -DKRS_API -DMSG

LIBS		= -lori2

ILIST		= man
IDIR		= /usr/bin/

.include <${RULES_MK}>
