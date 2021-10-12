# @(#)17        1.5  src/bos/kernext/lvm/Makefile, cmdlvm, bos411, 9428A410j 4/13/94 09:54:13

# COMPONENT_NAME: (SYSXLVM) Logical Volume Manager Device Driver - 17
#
# FUNCTIONS: Pseudo device driver for LVM
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

KERNEL_EXT	= hd_pin_bot hd_pin
IDIR		= /usr/lib/drivers/
ILIST		= hd_pin_bot hd_pin

hd_pin_ENTRYPOINT = hd_config

hd_pin_bot_EXPORTS = -bE:hdpin.exp
hd_pin_IMPORTS = -bI:hdpin.exp 
hd_pin_EXPORTS = -bE:hdpin.exp 

LIBS = $(KERNEXT_LIBS)

hd_pin_bot_OFILES = hd_bbdir.o \
                    hd_bbrel.o \
                    hd_mircach.o \
                    hd_phys.o \
                    hd_sched.o \
                    hd_strat.o \
                    hd_vgsa.o \
                    hd_dump.o 

hd_pin_OFILES = hd_top.o \
                hd_cfgvg.o \
                hd_cfglv.o \
                hd_cfgpv.o

.include <${RULES_MK}>
