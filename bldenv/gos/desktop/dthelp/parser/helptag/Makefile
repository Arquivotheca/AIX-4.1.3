# @(#)43	1.2  src/bldenv/gos/desktop/dthelp/parser/helptag/Makefile, desktop, bos412, GOLDA411a 4/21/94 17:21:53
#
#   COMPONENT_NAME: DESKTOP
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH 		= ${MAKETOP}/gos/desktop/cde1/dthelp/parser/helptag

OTHERS		= dthelptag

CFLAGS 		= -DSYSV -DAIXV3 -D_ALL_SOURCE -D_AIX -D__AIX -D__aix -D_aix \
		  -Daix -DMULTIBYTE -DNLS16 -DMESSAGE_CAT -DAIX_ILS -DR5TREE \
		  -DVALUE_ADDED -qMAXMEM=50000

LOCAL_CPP_DEFINES = -DReleaseTree

ILIST 		= dthelptag
IDIR 		= /opt/dt/etc/

.include <${RULES_MK}>

dthelptag: helptag.src
	${RM} -f ${.TARGET}
	${SED} 's/^\\#/\\!#/' ${.ALLSRC} | \
	${CPP} -P -C -D_AIX -D__aix ${LOCAL_CPP_DEFINES} | \
	${SED} 's/^\\!#/#/' > ${.TARGET}
	${ECHO} "PATH=\$$ODE_TOOLS/opt/dt/etc:/usr/bin:/bin" >tmp.file
	${ECHO} "export PATH" >>tmp.file
	${CAT} ${.TARGET} >>tmp.file
	${MV} tmp.file ${.TARGET}

