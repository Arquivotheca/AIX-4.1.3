# @(#) 94 1.6 src/bos/usr/lpp/bosinst/BosMenus/Makefile, bosinst, bos41J, 9518A_all 4/27/95 12:16:33
# COMPONENT_NAME: bosinst
#
# FUNCTIONS: Makefile for BosMenus and CheckSize
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

VPATH		= ${MAKETOP}/bos/objclass

PROGRAMS	= BosMenus CheckSize bidata datadaemon

MSGHDRS		= BosMenus_msg.h
INCLUDES	= Menus.h ${MSGHDRS}
CATFILES	= BosMenus.cat
HFILES		= ${MSGHDRS}

BosMenus_OFILES		= BosMenus.o ILS.o Help.o MenuHand.o Sizes.o parse.o \
			readBos.o readImage.o readTargetVGS.o readfunc.o

CheckSize_OFILES	= CheckSize.o Sizes.o readBos.o readImage.o \
			readTargetVGS.o

bidata_OFILES		= bidata.o

datadaemon_OFILES	= datadaemon.o readBos.o readImage.o

LIBS		= -lodm -llvm -lm
# INCFLAGS	= -I${MAKETOP}/bos/usr/lpp/bosinst/Ils

ILIST		= ${PROGRAMS}
IDIR		= /usr/lpp/bosinst/

EXPDIR		= /usr/include/

.include <${RULES_MK}>
