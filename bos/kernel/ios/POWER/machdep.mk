# @(#)16	1.10  src/bos/kernel/ios/POWER/machdep.mk, sysios, bos41J, 9515A_all 4/3/95 09:41:06
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


POWER_PINNED_OFILES	= \
	pio_assist.o d_protect.o ctlaltnum.o i_misc.o sys_resource.o \
	dispauth_pin.o u_iomem.o

POWER_PAGED_OFILES      = \
	dispauth_paged.o \
	busreg.o

POWER_INIT_OFILES	= \
	initiocc.o init_misc.o rminit.o hdlight_init.o

PWR_OFILES	= \
	dma_pwr.o \
	intr_pwr.o \
	i_misc_pwr.o

PPC_OFILES	= \
	rmalloc.o \
	dma_ppc.o \
	hdlight.o \
	intr_ppc.o \
	i_misc_ppc.o \
	pio_ppc.o \
	dse601.o \
	intr_rspc.o \
	disable_io_rspc.o

