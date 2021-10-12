# @(#)01	1.2  src/bos/diag/util/udiagmon/Makefile, dsaudiagmon, bos411, 9428A410j 1/21/94 13:45:53
# COMPONENT_NAME: DSAUDIAGMON
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= udiagmon diagd
MSGHDRS		= udiagmon_msg.h
CATFILES	= udiagmon.cat
HFILES		= ${MSGHDRS}

ILIST		= udiagmon udiagmon.cat diagd
IDIR		= /usr/lpp/diagnostics/bin/
udiagmon.cat_IDIR = /usr/lpp/diagnostics/catalog/default/

udiagmon_OFILES	= udiagmon.o
diagd_OFILES	= diagd.o

LIBS		= -lc -lodm -lcfg -ldiag 

.include <${RULES_MK}>
