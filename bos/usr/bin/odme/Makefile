# @(#)95        1.4  src/bos/usr/bin/odme/Makefile, odme, bos411, 9428A410j 2/21/94 08:41:04
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

PROGRAMS	= odme
MSGHDRS		= 
CATFILES	= odme.cat
HFILES		= ${MSGHDRS}
OTHERS		= odme.cat

ILIST		= odme
IDIR		= /usr/bin/

CFLAGS		+= -DD_SIZE=4096 ${_AIX3CURFLAGS} ${_AIX3CURSESFLAGS}
LIBFLAGS	+= ${_AIX3CURLIBFLAG} ${_AIX3CURSESLIBFLAG}
		

OFILES		= odme.o odmecreate.o odmecriteria.o odmeobjdisp.o \
		  odmeobjstore.o odmehft.o odmerelgraph.o odmecontrol.o \
		  odmewindow.o

LIBS		= -lodm -lcur -lcurses 

.include <${RULES_MK}>
