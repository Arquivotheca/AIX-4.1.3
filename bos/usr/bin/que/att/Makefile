# @(#)82	1.2  src/bos/usr/bin/que/att/Makefile, cmdque, bos411, 9428A410j 1/28/93 13:00:03
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

PROGRAMS	= lp lpstat enable disable cancel
MSGHDRS		= lp_msg.h lpstat_msg.h enable_msg.h disabl_msg.h cancel_msg.h
CATFILES	= lp.cat lpstat.cat enable.cat disabl.cat cancel.cat
HFILES		= ${MSGHDRS}

ILIST		= ${PROGRAMS}
IDIR		= /usr/bin/
INCFLAGS	= -I../

LIBFLAGS	= -L../libque

LIBS		= -lque -ls -lqb -lbsd

.include <${RULES_MK}>
