# @(#)30        1.1  src/bos/usr/ccs/lib/cflow/Makefile, cmdprog, bos411, 9428A410j 8/5/92 16:25:15
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}/bos/usr/ccs/bin/common:${MAKETOP}/bos/usr/ccs/bin/cflow

SUBDIRS		= cflow1

PROGRAMS	= dag flip lpfx nmf
MSGHDRS		= cflow_msg.h ctools_msg.h
CATFILES	= cflow.cat ctools.cat
HFILES		= ${MSGHDRS}

ILIST		= dag flip lpfx nmf
IDIR		= /usr/ccs/lib/

INCFLAGS	= -I${MAKETOP}/bos/usr/ccs/bin/common -I${MAKETOP}/bos/usr/ccs/bin/cflow
CFLAGS		= -DHOSTIEEE -DCFLOW2

lpfx_OFILES	= lpfx.o tftn.o xdefs.o

.include <${RULES_MK}>
