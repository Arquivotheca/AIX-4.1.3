# @(#)50	1.5  src/bos/diag/da/sdisk/Makefile, dasdisk, bos411, 9428A410j 12/5/93 20:48:21
#
#   COMPONENT_NAME: DASDISK
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992, 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


VPATH		= ${MAKETOP}/bos/diag/da/disks:${MAKETOP}/bos/diag/tu/sdisk
PROGRAMS	= dh2a dh2cntrl dh2dasd

MSGHDRS		= dh2_msg.h dhflvm_msg.h
CATFILES 	= dh2.cat
HFILES		= ${MSGHDRS}

CFLAGS		= -D DIAG
LIBS		= -lasl -ldiag -lodm -lcfg -lc

ILIST		= ${PROGRAMS} dh2.cat
IDIR		= /usr/lpp/diagnostics/da/
dh2.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

DABASE		= dh2disp_mn.o dh2ela.o debug.o

dh2a_OFILES	= ${DABASE} dh2a.o har2_atu.o
dh2cntrl_OFILES	= ${DABASE} dh2cntrl.o har2_atu.o
dh2dasd_OFILES	= ${DABASE} dh2dasd.o har2_atu.o dhflvm.o

.include <${RULES_MK}>
