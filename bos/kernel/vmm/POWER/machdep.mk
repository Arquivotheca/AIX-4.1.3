# @(#)22	1.11  src/bos/kernel/vmm/POWER/machdep.mk, sysvmm, bos41J, 9513A_all 3/17/95 14:05:43
#****************************************************************************
#
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
#****************************************************************************
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************


POWER_PINNED_OFILES	= \
	v_alloc.o	\
	v_cpfsubs.o	\
	v_compsubs.o	\
	v_exception.o	\
	v_disksubs.o	\
	v_fragsubs.o	\
	v_fssubs.o	\
	v_getsubs.o	\
	v_getsubs1.o	\
	v_interrupt.o	\
	v_lists.o	\
	v_map.o		\
	v_mapsubs.o	\
	v_mpsubs.o	\
	v_mvfork.o	\
	v_pdtsubs.o	\
	v_pfend.o	\
	v_pinsubs.o	\
	v_pre.o		\
	v_protsubs.o	\
	v_putsubs.o	\
	v_relsubs.o	\
	v_segsubs.o	\
	v_xptsubs.o	\
	vmcleardata.o	\
	vmmove.o	\
	vmpinsubs.o	\
	vmmisc.o	\
	vmdump.o	\
	vmusage.o	\
	userio.o	\
	xmem.o		\
	vmperf.o	\
	vmpm.o

POWER_PAGED_OFILES_PRIM	= \
	vmcreate.o	\
	vmdelete.o	\
	vmdevices.o	\
	vmforkcopy.o	\
	vmlimits.o

POWER_PAGED_OFILES	= \
	vm_map.o	\
	vm_mmap.o	\
	vmmap.o 	\
	vmpspace.o 	\
	vmprotect.o	\
	vmrelease.o	\
	vmqmodify.o	\
	vmiowait.o	\
	vmmakep.o	\
	vmwrite.o

POWER_INIT_OFILES	= \
	vmsi.o		\
	vmhwsi.o

PWR_OFILES		= \
	xmem_pwr.o	\
	v_power_rs1.o	\
	v_power_rs2.o

PPC_OFILES		= \
	xmem_ppc.o	\
	v_power_pc.o


