# @(#)45	1.4  src/bos/diag/da/msla/Makefile, damsla, bos411, 9428A410j 1/7/94 13:15:37
#
#   COMPONENT_NAME: DAMSLA
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
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

PROGRAMS	= dsla

MSGHDRS		= dsla_msg.h
CATFILES 	= dsla.cat
HFILES		= ${MSGHDRS}

LIBS		= -lasl -ldiag -lodm -lcfg -lc -lmslatu

ILIST		= ${PROGRAMS} dsla.cat 8787dd1.00.01 8787dd2.00.01 \
		   8787d50.00.01 8787dmod.00.01 8787dp.00.01 \
		   8787dpl2.00.01 8787dpl3.00.01 8787dwrp.00.01

IDIR		= /usr/lpp/diagnostics/da/

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/msla

dsla.cat_IDIR = /usr/lpp/diagnostics/catalog/default/
8787dd1.00.01_IDIR = /usr/lib/microcode/
8787dd2.00.01_IDIR = /usr/lib/microcode/
8787d50.00.01_IDIR = /usr/lib/microcode/
8787dmod.00.01_IDIR = /usr/lib/microcode/
8787dp.00.01_IDIR = /usr/lib/microcode/
8787dpl2.00.01_IDIR = /usr/lib/microcode/
8787dpl3.00.01_IDIR = /usr/lib/microcode/
8787dwrp.00.01_IDIR = /usr/lib/microcode/

dsla_OFILES	= dsla.o dslacfg.o

OTHERS=

OTHERS+=8787dd1.00.01
8787dd1.00.01:8787dd1.00.01.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS+=8787dd2.00.01
8787dd2.00.01:8787dd2.00.01.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS+=8787d50.00.01
8787d50.00.01:8787d50.00.01.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS+=8787dmod.00.01
8787dmod.00.01:8787dmod.00.01.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS+=8787dp.00.01
8787dp.00.01:8787dp.00.01.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS+=8787dpl2.00.01
8787dpl2.00.01:8787dpl2.00.01.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS+=8787dpl3.00.01
8787dpl3.00.01:8787dpl3.00.01.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS+=8787dwrp.00.01
8787dwrp.00.01:8787dwrp.00.01.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}


.include <${RULES_MK}>
