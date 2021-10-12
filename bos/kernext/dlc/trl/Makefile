# @(#)81	1.5  src/bos/kernext/dlc/trl/Makefile, sysxdlct, bos411, 9428A410j 4/1/93 15:03:33
#
# COMPONENT_NAME: (SYSXDLCT) Token Ring Data Link Controls
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#----------------------------------------------------------------------
#  Makefile for dlctoken
#-----------------------------------------------------------------------

.include <${MAKETOP}bos/kernext/Kernext.mk>

KERNEL_EXT = TRL.ext
IDIR = /usr/lib/drivers/
ILIST = TRL.ext trlextcb.h

trlextcb.h_IDIR = /usr/include/sys/

LIBS=  ${KERNEXT_LIBS} libdlch.a

VPATH := ${VPATH}:../lan

INCLUDES= trlextcb.h

EXPDIR = /kernel/include/sys/


CFLAGS =  -DTRL -D_KERNEL

TRL.ext_ENTRYPOINT = dlcconfig

TRL.ext_OFILES = lan.o lankproc.o landiscv.o lanhash.o lanmisc.o lansta1.o \
	lansta2.o lansta3.o trlmod.o lanerr.o dump.o


INCFLAGS =  -I${MAKETOP}/bos/kernext/dlc/include -I${MAKETOP}/bos/kernext/dlc/head -I${MAKETOP}/bos/kernext/dlc/lan

.include <${RULES_MK}>

