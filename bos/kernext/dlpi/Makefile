# @(#)62	1.1  src/bos/kernext/dlpi/Makefile, sysxdlpi, bos41J, 9514A_all 4/4/95 13:46:12
#
#   COMPONENT_NAME: SYSXDLPI
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#
# Makefile for aix
#

.include <$(MAKETOP)bos/kernext/Kernext.mk>

LOCALCFLAGS	= -DDEBUG
LIBS		= $(KERNEXT_LIBS)

DLPI		= dlpi
KERNEL_EXT	= $(DLPI)
IDIR		= /usr/lib/drivers/pse/
ILIST		= $(DLPI) README.dlpi

$(DLPI)_ENTRYPOINT = dlpiconfig

$(DLPI)_EXPORTS	=
$(DLPI)_IMPORTS	= -bI:pse.exp -bI:pse_int.exp

OBJECTS		= \
		abind.o \
		connect.o \
		convert.o \
		data.o \
		disc.o \
		dlack.o \
		drd.o \
		driver.o \
		globals.o \
		info.o \
		intr.o \
		llcmisc.o \
		misc.o \
		multi.o \
		reset.o \
		rx.o \
		timer.o \
		tx.o \
		udata.o \
		xid.o

$(DLPI)_OFILES	= $(OBJECTS)

.include <$(RULES_MK)>
