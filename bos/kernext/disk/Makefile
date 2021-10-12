# @(#)01        1.6.1.2  src/bos/kernext/disk/Makefile, sysxdisk, bos41J, 9513A_all 3/24/95 15:43:06
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

LOCALCFLAGS	=

SUBDIRS		= sd

#SC_TRACE	= 1

KERNEL_EXT	= badisk badiskpin scdisk scdiskpin
IDIR		= /usr/lib/drivers/
ILIST		= badisk badiskpin scdisk scdiskpin

badisk_ENTRYPOINT	= ba_config
badisk_IMPORTS		= -bI:badiskpin.exp
badisk_EXPORTS		= -bE:badisk.exp

scdisk_ENTRYPOINT	= scdisk_config

# Trace table debug requires different .exp file
.ifdef SC_TRACE
scdisk_IMPORTS		= -bI:scdiskpintr.exp
scdiskpin_EXPORTS	= -bE:scdiskpintr.exp
.else
scdisk_IMPORTS		= -bI:scdiskpin.exp
scdiskpin_EXPORTS	= -bE:scdiskpin.exp
.endif


scdisk_EXPORTS		= -bE:scdisk.exp
scdiskpin_IMPORTS	= -bI:scdisk.exp



badiskpin_IMPORTS	= -bI:badisk.exp
badiskpin_EXPORTS	= -bE:badiskpin.exp




LIBS		= ${KERNEXT_LIBS}

badisk_OFILES = badiskt.o

badiskpin_OFILES = badiskb.o

scdisk_OFILES = scdiskt.o

scdiskpin_OFILES = scdiskb.o

STRIP_FLAG ?= -s

LDFLAGS_KERNEXT += ${STRIP_FLAG}

.include <${RULES_MK}>
