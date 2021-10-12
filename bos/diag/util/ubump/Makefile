# @(#)50 1.2 src/bos/diag/util/ubump/Makefile, dsaubump, bos411, 9428A410j 2/21/94 08:40:19

# COMPONENT_NAME: DSAUBUMP
# 
# FUNCTIONS: Diagnostic Service Aid - UBUMP (makefile)
# 
# ORIGINS: 83 
# 
# (C) COPYRIGHT International Business Machines Corp.1993
# All Rights Reserved
#
# LEVEL 1, 5 Years Bull Confidential Information
#

PROGRAMS	= ubump

MSGHDRS		= ubump_msg.h
CATFILES	= ubump.cat
HFILES		= ${MSGHDRS}
CFLAGS		+= ${_AIX3CURFLAGS} ${_AIX3CURSESFLAGS}
LIBFLAGS	+= ${_AIX3CURLIBFLAG} ${_AIX3CURSESLIBFLAG}


LIBS		=  -lasl -ldiag -lodm -lcfg -lc -lcur -lcurses

INCFLAGS	= -I${MAKETOP}/bos/usr/include/diag -I${MAKETOP}/bos/usr/include

ILIST		= ${PROGRAMS} ubump.cat
IDIR		= /usr/lpp/diagnostics/bin/
ubump.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/
ubump_OFILES	= ubump.o

.include <${RULES_MK}>
