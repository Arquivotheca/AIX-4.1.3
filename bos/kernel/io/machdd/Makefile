# @(#)38      1.10  src/bos/kernel/io/machdd/Makefile, machdd, bos41J, bai15 3/17/95 12:07:59
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27, 83
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
#
# LEVEL 1,  5 Years Bull Confidential Information
#

.include <${MAKETOP}bos/kernel/Kernel.mk>

PINNED_OFILES	= \
	crc.o hand.o nvdd.o lgbus.o pcibus.o rspc.o

PPC_OFILES	= \
	pgs_bump_pin.o pgs_bump_pag.o

PAGED_OFILES	= \
	md.o

INIT_OFILES	= \
	init.o base.o pgs_bump_init.o rspcinit.o

pgs_bump_pin.o_INCFLAGS     = -I${MAKETOP}bos/kernel/ios/${TARGET_MACHINE}
pgs_bump_pag.o_INCFLAGS     = -I${MAKETOP}bos/kernel/ios/${TARGET_MACHINE}

.include <${RULES_MK}>
