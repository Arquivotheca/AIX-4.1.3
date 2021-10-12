# @(#)85        1.8  src/bos/diag/tu/siosl/kbda/Makefile, tu_siosl, bos411, 9428A410j 2/21/94 08:40:05
#
#   COMPONENT_NAME: TU_SIOSL
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992,1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

SUBDIRS      =  slih

EXPINC_SUBDIRS 	= slih
EXPLIB_SUBDIRS 	= slih

LIBRARIES	= libkbdatu.a

EXPLIB_TARGETS  = export_libkbdatu.a

EXPDIR		= /usr/lib/

CFLAGS          += -DSALMON -DDIAGNOSTICS ${_AIX3CURFLAGS} ${_AIX3CURSESFLAGS}
LIBFLAGS	+= ${_AIX3CURLIBFLAG} ${_AIX3CURSESLIBFLAG}

LIBS            = -lcur -lcurses

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/siosl

OFILES		= kbda.o

.include <${RULES_MK}>
