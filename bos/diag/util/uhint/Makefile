# @(#)17	1.4  src/bos/diag/util/uhint/Makefile, dutil, bos411, 9428A410j 2/21/94 08:40:22
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
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


PROGRAMS	= uhint

MSGHDRS		= uhint_msg.h
CATFILES 	= uhint.cat
HFILES		= ${MSGHDRS}
CFLAGS		+= ${_AIX3CURFLAGS} ${_AIX3CURSESFLAGS}
LIBFLAGS	+= ${_AIX3CURLIBFLAG} ${_AIX3CURSESLIBFLAG}

LIBS		= -lasl -ldiag -lcur -lcurses 

INCFLAGS	= -I${MAKETOP}/bos/usr/include

ILIST		= ${PROGRAMS} uhint.cat
IDIR		= /usr/lpp/diagnostics/bin/

uhint.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

uhint_OFILES	= uhint.o

.include <${RULES_MK}>
