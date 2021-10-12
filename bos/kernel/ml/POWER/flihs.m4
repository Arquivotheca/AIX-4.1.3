# @(#)36	1.2  src/bos/kernel/ml/POWER/flihs.m4, sysml, bos411, 9428A410j 4/15/94 17:06:22
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: defines for flihs
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************

# Registers containing mst addresses

	.set   	NEW_MST, r13		# newly-acquired mstsave
	.set   	INTR_MST, r14		# mstsave of interrupted program
	.set	PPDA_ADDR, r15		# address of PPDA for current processor

	.set	NEW_ADSPACE, 0xE002	# New address space allocation value

# Segment register privileged access bit

	.set     sr_k_bit, 0x40000000	# Privileged access to segment
					# Power Only

# Segment reg value to address non-volatile RAM 
# For POWER Architecture

       .set     nv_ram_seg_pwr, 0x82040080
       .set     u.nv_ram_seg_pwr, 0x8204
       .set     l.nv_ram_seg_pwr, 0x0080

# Segment reg value to address non-volatile RAM 
# For Rainbow Box....uses special segment BUID 0x7F 

       .set     nv_ram_seg_601, 0x87F0000F
       .set     u.nv_ram_seg_601, 0x87F0
       .set     l.nv_ram_seg_601, 0x000F

# Address value for LEDs within NVRAM

       .set     l.leds_addr, 0x0300

# For POWER Architecture (uses sreg 15, T=1 space)

       .set     leds_addr_pwr, 0xF0A00300
       .set     u.leds_addr_pwr, 0xF0A0

# For PowerPC Architecture (T=0 space)

       .set     leds_addr_ppc, 0xFF600300
       .set     u.leds_addr_ppc, 0xFF60

# NVRAM data

        .set    l.nvram_addr, 0x0000
        .set    l.nvram_addr_max, 0x7FFF  # limit of implemented NVRAM

# For POWER Architecture (T=1 space)

        .set    nvram_addr_pwr, 0x00A00000
        .set    u.nvram_addr_pwr, 0x00A0

# For PowerPC Architecture (T=0 space)

        .set    nvram_addr_ppc, 0xFF600000
        .set    u.nvram_addr_ppc, 0xFF60

# Value used to blank the displayed level number

	.set     led_blank, 0xF          # Display understands 0-9, and F

# Trace parameter to begin_interrupt
	.set	SR_LEVEL, 0x01		# system reset
	.set	MC_LEVEL, 0x02		# machine check
	.set	DS_LEVEL, 0x03		# data storage interrupt
	.set	IS_LEVEL, 0x04		# instruction storage interrupt
	.set	EX_LEVEL, 0x05		# external interrupt
	.set	PR_LEVEL, 0x07		# program interrupt
	.set	FP_LEVEL, 0x08		# floating un available level
	.set	DSE_LEVEL, 0x09		# 601 direct store interrupt
	.set	FI_LEVEL, 0x0a		# RS2 floating point imprecise
	.set	RUN_LEVEL, 0x20		# 601 run-mode interrupt
	.set	DC_LEVEL, 0x31		# PPC decrementer
	.set	SOFT_LEVEL, 0x32	# software interrupt model - i_dosoft

# Address range of cs() on Power
	.set	cslo_pwr, 0x1020	# start of Power cs routine
	.set	cshi_pwr, 0x102f	# end of Power cs routine

