# @(#)36        1.3  src/bos/usr/bin/panel20/Makefile, cmdhia, bos410, bos4.17293 4/26/93 12:21:14
#
#   COMPONENT_NAME: CMDHIA
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
 
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
 
PROGRAMS	= panel20
MSGHDRS		=
CATFILES	= panel20.cat
HFILES		= ${MSGHDRS}

CFLAGS		+= ${_AIX3CURFLAGS} ${_AIX3CURSESFLAGS}
LIBFLAGS	+= ${_AIX3CURLIBFLAG} ${_AIX3CURSESLIBFLAG}
 
OFILES		= w_msgtbl.o w_dismsg.o w_signals.o panel20.o p20_lib.o p20_update.o
 
LIBS		= -lodm -lcfg -lcur -lcurses 
 
ILIST		= panel20
IDIR		= /usr/bin/
 
 
.include <${RULES_MK}>

w_msgtbl.c:	panel20.msg
	${MAKEMSGS} ${panel20.msg:P} ${.TARGET}
