# @(#)77        1.5  src/bos/diag/util/umlc/Makefile, dsaumlc, bos411, 9428A410j 3/14/94 21:01:24
# COMPONENT_NAME: DSAUMLC
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1992, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS        = umlc

MSGHDRS         = umlc_msg.h
CATFILES        = umlc.cat
HFILES          = ${MSGHDRS}

LIBS            = -lasl -ldiag -lodm -lcfg -lc -lrts

ILIST           = ${PROGRAMS} umlc.cat
IDIR            = /usr/lpp/diagnostics/bin/
umlc.cat_IDIR = /usr/lpp/diagnostics/catalog/default/
umlc_OFILES     = umlc.o ubuff.o umlcproc.o umlcio.o umlcdisp.o umlcglobal.o

.include <${RULES_MK}>
