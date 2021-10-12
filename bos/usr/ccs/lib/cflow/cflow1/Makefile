# @(#)31        1.1  src/bos/usr/ccs/lib/cflow/cflow1/Makefile, cmdprog, bos411, 9428A410j 8/5/92 16:25:21
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}/bos/usr/ccs/bin/common

PROGRAMS	= cflow1
MSGHDRS		= ctools_msg.h
CATFILES	= ctools.cat
HFILES		= ${MSGHDRS} mfile1.h error.h common.h

ILIST		= cflow1
IDIR		= /usr/ccs/lib/

INCFLAGS	= -I${MAKETOP}/bos/usr/ccs/bin/common
CFLAGS		= -DCFLOW -DXCOFF -DNEWCALL -DHOSTIEEE -DSDBDEBUG \
		  -DTWOPASS -DSINGLE_PRECISION -D_COMPILER

OFILES		= cgram.o code.o comm1.o local.o messages.o optim.o pftn.o \
		  scan.o tftn.o trees.o xdefs.o

.include <${RULES_MK}>
