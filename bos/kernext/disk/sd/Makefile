# @(#)00        1.5  src/bos/kernext/disk/sd/Makefile, sysxdisk, bos411, 9428A410j 3/16/94 09:31:04
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

.include <${MAKETOP}/bos/kernext/Kernext.mk>

LOCALCFLAGS     = -qmaxmem=7000

KERNEL_EXT	= sd sdpin
IDIR		= /usr/lib/drivers/
ILIST		= sd sdpin

sd_ENTRYPOINT	= sd_config

sd_EXPORTS	= -bE:sd.exp

# Trace table debug requires different .exp file
.ifdef SD_TRACE
sd_IMPORTS	= -bI:sdpintr.exp
sdpin_EXPORTS	= -bE:sdpintr.exp
.else
sd_IMPORTS	= -bI:sdpin.exp
sdpin_EXPORTS	= -bE:sdpin.exp
.endif


sdpin_IMPORTS	= -bI:sd.exp

LIBS		= ${KERNEXT_LIBS}

sd_OFILES = sdconfig.o sdopen.o \
	 sdclose.o sdioctl.o \
	 sdrdwr.o sdutilt.o 

sdpin_OFILES = sdintr.o sdstrategy.o \
	    sdstart.o sdutilb.o \
	    sddump.o sdactivity.o \
	    sdgfree.o sdconc.o

STRIP_FLAG ?= -s

LDFLAGS_KERNEXT += ${STRIP_FLAG}

.include <${RULES_MK}>
