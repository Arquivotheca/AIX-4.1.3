# @(#)75        1.5  src/bos/usr/bin/mh/zotnet/Makefile, cmdmh, bos41J, 9516A_all 4/17/95 14:12:56
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

LIBRARIES	= libzot.a
MSGHDRS		= 
CATFILES	= 
HFILES		= ${MSGHDRS}

CFLAGS		= -D_BSD -DBSD42 -DMHE -DMHRC -DSENDMTS
INCFLAGS	= -I.. -I../h

OFILES		= getbbent.o mf.o mts.o lock.o client.o dtime.o \
		  dtimep.o Lexstring.o

SCRIPTS		= mtstailor
ILIST		= ${SCRIPTS}
IDIR		= /etc/mh/
NOUPDATE_LIST 	= /etc/mh/mtstailor

.include <${RULES_MK}>


Lexstring.c: lexstring.c dtimep.c
	@${RM} -f ${.TARGET}
	{ \
	    ${GREP} '^[ 	]*#[ 	]*define[ 	]YYTYPE' ${dtimep.c:P} && \
	    ${CAT} ${lexstring.c:P} ; \
	} > ${.TARGET} || { ${RM} -f ${.TARGET}; exit 1; }

dtimep.c: dtimep.l lexedit.sed
	${RM} -f dtimep.c lex.yy.c
	${LEX} ${dtimep.l:P} 
	${SED} -f ${lexedit.sed:P} lex.yy.c > dtimep.c

lexedit.sed : lexedit.sh
	@${RM} -f lexedit.sed
	${CP} ${lexedit.sh:P} .
	${CHMOD} +x lexedit.sh
	lexedit.sh >lexedit.sed
	${RM} -f lexedit.sh
