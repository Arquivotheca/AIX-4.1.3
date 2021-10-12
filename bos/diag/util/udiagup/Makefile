# @(#)76	1.3  src/bos/diag/util/udiagup/Makefile, dsaudiagup, bos411, 9428A410j 4/7/93 14:31:06
# COMPONENT_NAME: DSAUDIAGUP
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1992, 1993.
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= udiagup

MSGHDRS		= udiagup_msg.h
CATFILES 	= udiagup.cat
HFILES		= ${MSGHDRS}

LIBS		= -lasl -ldiag -lodm -lcfg -lc -lswvpd

ILIST		= ${PROGRAMS} udiagup.cat
IDIR		= /usr/lpp/diagnostics/bin/
udiagup.cat_IDIR = /usr/lpp/diagnostics/catalog/default/
udiagup_OFILES	= udiagup.o

.include <${RULES_MK}>
