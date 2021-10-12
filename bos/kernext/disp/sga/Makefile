# @(#)88	1.6  src/bos/kernext/disp/sga/Makefile, sgadd, bos411, 9432A411a 8/4/94 15:12:52
#   COMPONENT_NAME: SGADD
#
# FUNCTIONS:
#
#   ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}/bos/kernext/Kernext.mk>

EXPINC_SUBDIRS  = inc

KERNEL_EXT	= sgadd
IDIR		= /usr/lib/drivers/
ILIST		= sgadd
LIBS            = ${KERNEXT_LIBS}

INCFLAGS        = -I./inc

sgadd_ENTRYPOINT	= sga_config

sgadd_OFILES	= moveit.o load_pal.o stct.o reset.o open.o cfl.o \
		  text.o intr.o diag.o draw.o act.o copyps.o font.o \
		  makegp.o init.o setm.o config.o define.o term.o \
		  ioctl.o rds.o cpl.o dact.o defc.o close.o \
		  scr.o clr.o sga_err.o dpm.o

.include <${RULES_MK}>
