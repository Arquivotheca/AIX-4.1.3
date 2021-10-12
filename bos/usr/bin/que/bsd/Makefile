# @(#)81	1.3  src/bos/usr/bin/que/bsd/Makefile, cmdque, bos411, 9428A410j 1/29/93 09:48:39
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

PROGRAMS	= lpq lpr lprm lptest
MSGHDRS		= lpq_msg.h lpr_msg.h lprm_msg.h
CATFILES	= lpq.cat lpr.cat lprm.cat
HFILES		= ${MSGHDRS}

ILIST		= ${PROGRAMS}
IDIR		= /usr/bin/
lptest_IDIR	= /usr/sbin/
INCFLAGS	= -I../

LIBFLAGS	= -L../libque

LIBS		= -lque -ls -lqb -lbsd

.include <${RULES_MK}>
