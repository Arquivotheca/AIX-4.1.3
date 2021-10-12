# @(#)37	1.5  src/bos/diag/da/iopsl/Makefile, daiopsl, bos411, 9428A410j 2/21/94 08:39:15
#
#   COMPONENT_NAME: DAIOPSL
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}/bos/diag/da/iop
PROGRAMS	= diopsl

MSGHDRS		= diop_msg.h
CATFILES 	= diop.cat
HFILES		= ${MSGHDRS}

CFLAGS		+= ${_AIX3CURFLAGS}
LIBFLAGS	+= ${_AIX3CURLIBFLAG}

LIBS		= -lasl -lcur -ldiag -lodm -lcfg -lc -liopsltu

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/iopsl

ILIST		= ${PROGRAMS} diop.cat
IDIR		= /usr/lpp/diagnostics/da/
diop.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

diopsl_OFILES	= diopsl.o diopsl_mn.o

.include <${RULES_MK}>
