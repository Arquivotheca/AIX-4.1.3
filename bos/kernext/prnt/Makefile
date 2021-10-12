# @(#)20        1.3  src/bos/kernext/prnt/Makefile, sysxprnt, bos411, 9428A410j 8/3/93 08:39:07
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

.include <${MAKETOP}bos/kernext/Kernext.mk>

VPATH	+=:${TARGET_MACHINE}

KERNEL_EXT	= ppdd ppddpin
IDIR		= /usr/lib/drivers/
ILIST		= ppdd ppddpin

ppdd_ENTRYPOINT = ppconfig
ppdd_IMPORTS	= -bI:ppddpin.exp
ppdd_EXPORTS 	= -bE:ppdd.exp

ppddpin_IMPORTS	= -bI:ppdd.exp
ppddpin_EXPORTS	= -bE:ppddpin.exp

LIBS = $(KERNEXT_LIBS)

ppdd_OFILES = \
	ppconfig.o ppopen.o ppclose.o ppread.o \
	ppwrite.o ppioctl.o ppddt.o ppddm_top.o \
	prnformat.o

ppddpin_OFILES = \
	ppddm_bot.o pppio_bot.o ppddb.o

.include <${RULES_MK}>
