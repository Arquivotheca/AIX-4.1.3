# @(#)84	1.2  src/bos/diag/da/niokta/Makefile, dakbd, bos411, 9428A410j 3/4/94 16:41:42
#
#   COMPONENT_NAME: DAKBD
#
#   FUNCTIONS:
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH           = ${MAKETOP}/bos/diag/da/siosl/kmta

PROGRAMS        = dkeytaba

MSGHDRS         = tab_a_msg.h
HFILES          = ${MSGHDRS}

CFLAGS          += ${_AIX3CURFLAGS} ${_AIX3CURSESFLAGS}
LIBFLAGS	+= ${_AIX3CURLIBFLAG} ${_AIX3CURSESLIBFLAG}

LIBS            = -lasl -ldiag -lcur -lcurses -lodm -lcfg -lc -lakbdtu

ILIST           = ${PROGRAMS}
IDIR            = /usr/lpp/diagnostics/da/

dkeytaba_OFILES = dkeytaba.o

.include <${RULES_MK}>

