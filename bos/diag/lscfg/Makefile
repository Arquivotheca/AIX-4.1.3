# @(#)23	1.4  src/bos/diag/lscfg/Makefile, lscfg, bos41J, 9516A_all 4/18/95 14:20:05
#
#   COMPONENT_NAME: LSCFG
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992, 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH           = ${MAKETOP}/bos/usr/ccs/lib/libdiag:${MAKETOP}/bos/objclass/${TARGET_MACHINE}

PROGRAMS	= lscfg

MSGHDRS		= lscfg_msg.h
HFILES		= ${MSGHDRS}
CATFILES 	= lscfg.cat

LIBS		= -lodm -lcfg -lc

ILIST		= ${PROGRAMS}
IDIR		= /usr/sbin/

lscfg_OFILES	= lscfg.o ls_init_db.o gen_menu.o parsevpd.o diagodm.o

.include <${RULES_MK}>

diagodm.c:      diagodm.cre
	${RM} -f ${.TARGET} ${.TARGET:.c=.h}
	${CP} ${diagodm.cre:P} .
	${ODMCREATE} -h -p diagodm.cre
	${RM} -f diagodm.cre

