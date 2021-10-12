# @(#)12	1.6  src/bos/usr/ccs/lib/libdiag/min/Makefile, libdiag, bos41J, 9516A_all 4/18/95 13:12:38
# COMPONENT_NAME: LIBDIAG
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}/bos/kernel/exp:${MAKETOP}/bos/objclass/${TARGET_MACHINE}:${MAKETOP}/bos/usr/ccs/lib/libdiag

SHARED_LIBRARIES	= libdiag.a
MSGHDRS         = dcda_msg.h
CATFILES        = dcda.cat
HFILES          = ${MSGHDRS}

ILIST           = libdiag.a
IDIR            = /usr/lpp/diagnostics/lib/

LIBS		= libc.a libodm.a libcfg.a libm.a
IMPORTS		= -bI:syscalls.exp
EXPORTS		= -bE:libdiag.exp

SHARED_OFILES	= 	\
	addfrub.o	\
	cfgdev.o	\
	diagodm.o	\
	diag_errlg.o	\
	diag_sdm.o	\
	diag_support.o	\
	dmenustub.o	\
	exitstruct.o	\
	frus.o		\
	gen_menu.o	\
	getdainput.o	\
	getdate.o	\
	getdavar.o	\
	getmode.o	\
	init_diag.o	\
	init_index.o	\
	ipl_mode.o	\
	led.o		\
	menugoal.o	\
	Perror.o	\
	putdavar.o	\
	save_cdiag.o	\
	stack_dev.o	\
	scsi.o		\
	parsevpd.o	\
	diag_trace.o


.include <${RULES_MK}>

diagodm.c:	diagodm.cre
	${RM} -f ${.TARGET} ${.TARGET:.c=.h}
	${CP} ${diagodm.cre:P} .
	${ODMCREATE} -h -p diagodm.cre
	${RM} -f diagodm.cre

dmenustub.o:	${HFILES}

