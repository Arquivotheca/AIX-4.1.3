# @(#)39	1.6  src/bos/diag/da/async/Makefile, daasync, bos411, 9428A410j 5/25/94 10:36:33
#
#   COMPONENT_NAME: DAASYNC
#
#   FUNCTIONS: 
#
#   ORIGINS: 27, 83
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
#   LEVEL 1, 5 Years Bull Confidential Information
#



VPATH		= ${MAKETOP}/bos/diag/tu/async
PROGRAMS	= dasync

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/async

MSGHDRS		= dasync_msg.h
CATFILES 	= dasync.cat
HFILES		= ${MSGHDRS}

CFLAGS		+= ${_AIX3CURFLAGS} ${_AIX3CURSESFLAGS}
LIBFLAGS	+= ${_AIX3CURLIBFLAG} ${_AIX3CURSESLIBFLAG}

LIBS		= -lasl -ldiag -lcur -lodm -lcfg -lc

ILIST		= ${PROGRAMS} dasync.cat
IDIR		= /usr/lpp/diagnostics/da/
dasync.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

dasync_OFILES	= dasync.o dasync_cfg.o dasync_tu.o dasync_asl.o \
			exectu.o crc.o dastreams.o

.include <${RULES_MK}>
