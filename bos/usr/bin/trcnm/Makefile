# @(#)95        1.5  src/bos/usr/bin/trcnm/Makefile, cmdtrace, bos411, 9428A410j 12/7/93 13:26:39
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# LEVEL 1, 5 Years Bull Confidential Information
# 

VPATH		= ../trcrpt

PROGRAMS	= trcnm
MSGHDRS		= 
CATFILES	= 
HFILES		= ${MSGHDRS}

ILIST		= trcnm
IDIR		= /usr/bin/

OFILES		= trcnm_main.o nm.o

CFLAGS		= -D_JNLS -DTRCRPT
INCFLAGS	= -I../trcrpt/trcrpt -I${MAKETOP}/bos/sbin/trace

LIBS		= -lras

.include <${RULES_MK}>
