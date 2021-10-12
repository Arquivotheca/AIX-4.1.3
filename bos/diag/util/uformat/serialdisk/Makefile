# @(#)16	1.3  src/bos/diag/util/uformat/serialdisk/Makefile, dsauformat, bos411, 9428A410j 12/5/93 20:40:34
#
#   COMPONENT_NAME: DSAUFORMAT
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992, 1993.
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


VPATH		= ${MAKETOP}/bos/diag/util/uformat ${MAKETOP}/bos/diag/tu/sdisk
PROGRAMS	= srwfor
MSGHDRS		= ufmt_msg.h
HFILES		= ${MSGHDRS}

LIBS		= -lasl -ldiag -lodm -lcfg -lc

CFLAGS		= -D DIAG
INCFLAGS	= -I${MAKETOP}/bos/diag/util/uformat

ILIST		= ${PROGRAMS}
IDIR		= /usr/lpp/diagnostics/bin/

srwfor_OFILES	= disp_mn.o  srwfor.o har2_atu.o

.include <${RULES_MK}>
