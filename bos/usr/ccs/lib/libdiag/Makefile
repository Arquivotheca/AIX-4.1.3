# @(#)27        1.14  src/bos/usr/ccs/lib/libdiag/Makefile, libdiag, bos41J, 9516A_all 4/18/95 13:12:45
# COMPONENT_NAME: LIBDIAG
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}/bos/kernel/exp:${MAKETOP}/bos/objclass/${TARGET_MACHINE}

SHARED_LIBRARIES	= libdiag.a
MSGHDRS         = dcda_msg.h
CATFILES        = dcda.cat
HFILES          = ${MSGHDRS}

CFLAGS          += ${_AIX3CURFLAGS} ${_AIX3CURSESFLAGS}
LIBFLAGS        += ${_AIX3CURLIBFLAG} ${_AIX3CURSESLIBFLAG}

SUBDIRS		= min
EXPLIB_TARGETS  = export_libdiag.a

ILIST           = libdiag.a dcda_msg.h dcda.cat
IDIR            = /usr/lib/
dcda_msg.h_IDIR = /usr/include/diag/
dcda.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/
INCLUDES	= dcda_msg.h

LIBS		= libc.a libodm.a libcfg.a libasl.a libcur.a libcurses.a libm.a
IMPORTS		= -bI:syscalls.exp
EXPORTS		= -bE:libdiag.exp

SHARED_OFILES	= 	\
	addfrub.o	\
	cfgdev.o	\
	diagodm.o	\
	diag_errlg.o	\
	diag_sdm.o	\
	diag_support.o	\
	diag_trace.o	\
	dmenu.o		\
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
	parsevpd.o	\
	scsi.o

NOUPDATE_LIST = /usr/include/diag/dcda_msg.h

.include <${RULES_MK}>

diagodm.c:	diagodm.cre
	${RM} -f ${.TARGET} ${.TARGET:.c=.h}
	${CP} ${diagodm.cre:P} .
	${ODMCREATE} -h -p diagodm.cre
	${RM} -f diagodm.cre

dmenu.o:	${HFILES}

