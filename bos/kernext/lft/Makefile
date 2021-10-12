# @(#)41	1.8  src/bos/kernext/lft/Makefile, lftdd, bos41J, 9513A_all 3/16/95 16:42:18
#
#   COMPONENT_NAME: LFTDD
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

EXPINC_SUBDIRS  = inc
VPATH		+=:./config:./fonts:./swkbd:./streams:./utils:./pwr_mgr:../exp

LIBRARIES       = 
EXPLIB_TARGETS  =

KERNEL_EXT	= lft_loadpin
IDIR		= /usr/lib/drivers/
ILIST		= lft_loadpin

LIBS		= ${KERNEXT_LIBS}

lft_loadpin_ENTRYPOINT	= lftconfig

lft_loadpin_EXPORTS	= -bE:lft_loadpin.exp

LOCALCFLAGS 	=  -D_KERNEL  -D_AIXV3_POSTWAIT

INCFLAGS	=-I./inc -I./inc/${TARGET_MACHINE}

KERNEXT_IMPORTS	= sysoverlay.exp kernex.exp syscalls.exp -bI:pse.exp \
		-bI:aio.exp -bI:dead.exp sockets.exp -bI:pse_int.exp  \
		-bI:dbtty.exp -bI:tty.exp

lft_loadpin_LDFLAGS_KERNEXT= ${LDFLAGS_KERNEXT}

lft_loadpin_OFILES	=	\
	lftconfig.o lftinit.o lftterm.o  lftsicfg.o   \
	fkproc.o kernel_ftok.o lftfonts.o lftswkbd.o  \
	diac850.o diac88591.o diac88592.o diac88593.o \
	diac88594.o diac88597.o diac88599.o control.o \
	function.o fsqueue.o  lfterr.o lftsi.o lftte.o \
	lftvi.o lftst.o lftki.o lft_trace.o pwr_mgr.o

.include <${RULES_MK}>
