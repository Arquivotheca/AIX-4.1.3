# @(#)87	1.3  src/bos/diag/dutil/Makefile, dutil, bos411, 9428A410j 12/5/93 20:22:28
#
#   COMPONENT_NAME: DUTIL
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992, 1993.
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


PROGRAMS	= dutil

MSGHDRS		= dutil_msg.h
HFILES		= ${MSGHDRS}
CATFILES 	= dutil.cat
LIBS		= -lasl -ldiag -lodm -lcfg -lc
SCRIPTS		= CEREADME

ILIST		= ${PROGRAMS} dutil.cat CEREADME
IDIR		= /usr/lpp/diagnostics/bin/
dutil.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/
CEREADME_IDIR	= /usr/lpp/diagnostics/

dutil_OFILES	= dutil.o

.include <${RULES_MK}>
