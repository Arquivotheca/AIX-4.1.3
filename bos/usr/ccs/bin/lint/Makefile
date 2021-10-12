# @(#)52        1.10  src/bos/usr/ccs/bin/lint/Makefile, cmdprog, bos411, 9428A410j 5/13/94 16:38:38
# COMPONENT_NAME: (CMDPROG) Programming Utilities
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1991
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

SUBDIRS			= pass2

VPATH			= ../common:${TARGET_MACHINE}

PROGRAMS		= lint1
SCRIPTS			= lint
MSGHDRS                 = ctools_msg.h
CATFILES                = ctools.cat
DATAFILES		= llib-lansi llib-lcrses llib-lm llib-port \
			  llib-lc

ANSI_LIST		= llib-lansi.ln
LLIB_LIST 		= llib-lc.ln llib-port.ln llib-lm.ln llib-lcrses.ln

OTHERS			= ${ANSI_LIST} ${LLIB_LIST}

ILIST			= lint1 lint ${DATAFILES} ${ANSI_LIST} ${LLIB_LIST}
IDIR			= /usr/ccs/lib/
lint_IDIR		= /usr/ccs/bin/

INCFLAGS		= -I../common ${INCDIRS}
CFLAGS			= -DNEWCALL -DHOSTIEEE -DSDBDEBUG -DSINGLE_PRECISION \
		          -DTWOPASS -D_COMPILER -DLINT
HFILES			= ${MSGHDRS}  

OFILES			= cgram.o code.o   comm1.o  local.o  messages.o  optim.o \
			  pftn.o   scan.o   tftn.o   trees.o     xdefs.o

LIBS			= 

.include <${RULES_MK}>

${ANSI_LIST}:	${.TARGET:.ln=}
	${CPP} -E -C -DLINT -Dlint -D_AIX -D_ANSI_C_SOURCE -D_POWER -qlanglvl=ansi ${_INCFLAGS_} ${${.TARGET:.ln=}:P} | ${LINT1} -wAP -MA -L${.TARGET} || { ${RM} -f ${.TARGET}; ${FALSE}; }

${LLIB_LIST}:	${.TARGET:.ln=}
	${CPP} -E -C -DLINT -Dlint -D_AIX -D_POWER -D_LONG_LONG ${_INCFLAGS_} ${${.TARGET:.ln=}:P} |  ${LINT1} -Nn2500 -Nd3000 -L${.TARGET} || { ${RM} -f ${.TARGET}; ${FALSE}; }
