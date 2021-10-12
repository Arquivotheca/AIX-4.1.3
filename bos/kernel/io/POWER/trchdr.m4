# @(#)51	1.7  src/bos/kernel/io/POWER/trchdr.m4, sysio, bos411, 9428A410j 6/15/90 18:09:16

# COMPONENT_NAME: SYSTRACE  /dev/systrace pseudo-device driver
#
# FUNCTIONS: program generated .s version of trchdr structure in trchdr.h
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

	.set	trc_state,0x0
	.set	trc_msr,0x4
	.set	trc_lockword,0x8
	.set	trc_sleepword,0xc
	.set	trc_mode,0x10
	.set	trc_fsm,0x12
	.set	trc_channel,0x14
	.set	trc_wrapcount,0x16
	.set	trc_ovfcount,0x18
	.set	trc_start,0x20
	.set	trc_end,0x24
	.set	trc_inptr,0x28
	.set	trc_size,0x2c
	.set	trc_startA,0x30
	.set	trc_endA,0x34
	.set	trc_inptrA,0x38
	.set	trc_sizeA,0x3c
	.set	trc_startB,0x40
	.set	trc_endB,0x44
	.set	trc_inptrB,0x48
	.set	trc_sizeB,0x4c
	.set	trc_events,0x54
	.set	HKTY_TMASK,0x0008
	.set	HKTY_Sr,0x1
	.set	HKTY_STr,0x9
	.set	HKTY_Lr,0x2
	.set	HKTY_LTr,0xA
	.set	HKTY_Gr,0x6
	.set	HKTY_GTr,0xE
	.set	HKTY_Vr,0x0
	.set	HKTY_VTr,0x8
	.set	ST_ISOPEN,0x1
	.set	ST_TRCON,0x2
	.set	ST_TRCSTOP,0x10
