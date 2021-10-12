# @(#)74	1.19  src/bos/diag/diags/Makefile, diagsup, bos41J, 9512A_all 3/17/95 13:19:34
#
#   COMPONENT_NAME: CMDDIAG
#
#   FUNCTIONS: Makefile
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


VPATH		= ${MAKETOP}/bos/objclass/${TARGET_MACHINE}:${MAKETOP}/bos/objclass
PROGRAMS	= diags 
SCRIPTS		= diag diagpt diagipl diagela fastdiag \
		  dskpt.dep diagdskpt get_diag_stanzas bos.diag.rte.err

MSGHDRS		= diags_msg.h diagcd_msg.h
HFILES		= ${MSGHDRS}
CATFILES 	= diags.cat diagcd.cat
INCLUDES	= diag_errids.h
EXPDIR		= /usr/include/diag/

LIBS		= -lasl -ldiag -lodm -lcfg -lc

ILIST		= ${PROGRAMS} diags.cat diag diagpt diagipl \
		  diagdskpt get_diag_stanzas diagela fastdiag dskpt.dep \
		  PDiagDev PDiagAtt PDiagDev.vc PDiagAtt.vc DSMenu \
		  DSMOptions DSMOptions.vc CDiagDev MenuGoal FRUB FRUs \
		  TMInput DAVars diagcd.cat diag_errids.h

IDIR		= /usr/lpp/diagnostics/bin/
diag_IDIR	= /usr/sbin/
diag_errids.h_IDIR = /usr/include/diag/
diags.cat_IDIR  = /usr/lpp/diagnostics/catalog/default/
fastdiag_IDIR   = /etc/lpp/diagnostics/data/
CDiagDev_IDIR	= /usr/lpp/diagnostics/obj/
MenuGoal_IDIR	= /usr/lpp/diagnostics/obj/
FRUB_IDIR	= /usr/lpp/diagnostics/obj/
FRUs_IDIR	= /usr/lpp/diagnostics/obj/
TMInput_IDIR	= /usr/lpp/diagnostics/obj/
DAVars_IDIR	= /usr/lpp/diagnostics/obj/
PDiagAtt_IDIR	= /usr/lpp/diagnostics/obj/
PDiagAtt.vc_IDIR	= /usr/lpp/diagnostics/obj/
PDiagDev_IDIR	= /usr/lpp/diagnostics/obj/
PDiagDev.vc_IDIR	= /usr/lpp/diagnostics/obj/
DSMenu_IDIR	= /usr/lpp/diagnostics/obj/
DSMOptions_IDIR	= /usr/lpp/diagnostics/obj/
DSMOptions.vc_IDIR	= /usr/lpp/diagnostics/obj/

diags_OFILES	= diags.o

OTHERS		= tempdiag diag_errids.h


.include <${RULES_MK}>

tempdiag : diagodm.cre
	${RM} -f ${.TARGET}
	ODMDIR=. ${ODMCREATE} -c -p ${diagodm.cre:P}
	${TOUCH} tempdiag

diag_errids.h:  bos.diag.rte.err.S
	${RM} -f ${.TARGET}
	${ERRTMPLTBLD} -d ${bos.diag.rte.err.S:P} -o ${.TARGET}
