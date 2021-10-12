# @(#)49 1.2 src/bos/diag/util/ump/Makefile, dsaump, bos411, 9428A410j 2/21/94 08:40:25

# COMPONENT_NAME: DSAUMP
# 
# FUNCTIONS: Diagnostic Service Aid - UMP (makefile)
# 
# ORIGINS: 83 
# 
# (C) COPYRIGHT International Business Machines Corp.1993
# All Rights Reserved
#
# LEVEL 1, 5 Years Bull Confidential Information
# 

PROGRAMS	= ump

MSGHDRS		= ump_msg.h
CATFILES	= ump.cat
HFILES		= ${MSGHDRS}
CFLAGS		+= ${_AIX3CURFLAGS} ${_AIX3CURSESFLAGS}
LIBFLAGS	+= ${_AIX3CURLIBFLAG} ${_AIX3CURSESLIBFLAG}

LIBS		=  -lasl -ldiag -lodm -lcfg -lc -lcur -lcurses

INCFLAGS	= -I${MAKETOP}/bos/usr/include/diag -I${MAKETOP}/bos/usr/include

ILIST		= ${PROGRAMS} ump.cat
IDIR		= /usr/lpp/diagnostics/bin/
ump.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/
ump_OFILES	= ump.o

.include <${RULES_MK}>
