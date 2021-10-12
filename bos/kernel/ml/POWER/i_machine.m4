# @(#)37        1.9  src/bos/kernel/ml/POWER/i_machine.m4, sysml, bos41J, 9513A_all 3/23/95 17:39:50

#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Defines
#
# FUNCTIONS:
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989, 1995
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
#  IMPORTANT NOTE:  This file should match sys/intr.h.
#****************************************************************************

#
#	 Interrupt priority values;  See i_enable() and i_disable 
#
	.set	INTMAX,         0       # All interrupts disabled     
	.set	INTCLASS0,      1       # Class 0 devices            
	.set	INTCLASS1,      2       # Class 1 devices              
	.set	INTCLASS2,      3       # Class 2 devices             
	.set	INTCLASS3,      4       # Class 3 devices            
	.set	INTTIMER,       5       # Timer interrupt priority  
	.set	INTOFFL0,       6       # Off-level for class 0 device 
	.set	INTOFFL1,       7       # Off-level for class 1 device
	.set	INTOFFL2,       8       # Off-level for class 2 device
	.set	INTOFFL3,       9       # Off-level for class 3 device
	.set	INTIODONE,      10      # For use by IODONE routines 
	.set	INTPAGER,       10      # Pager interrupt priority 
	.set	INTOFFLVL,	10	# Offlevel MFRR value
	.set	INTBASE,        11      # All interrupts enabled   
	.set	ORGPFAULT,      0x0003  # Original page fault
	.set	BKTPFAULT,      0x0001  # Backtracking page fault
ifdef(`_POWER_RS',`
	.set    INT_OFFLVL,     63      # HW assist level(bit)
	.set	INT_OFFLVLWD,	0x1	# Bit 31 in eis1
',)
ifdef(`_POWER_PC',`
	.set    MFRR_XISR,      2       # HW assist level(bit)
	.set    XISR_MASK,      0x1FFF  # Mask to get BUID/SRC from XISR
	.set    EMPTY_MFRR,     0xFF    # Value for no MFRR interrupt
',)
	.set	pgbacktorg,INTPAGER*256+ORGPFAULT
	.set	pgbackt,INTPAGER*256+BKTPFAULT

	# The following have corresponding #defines in sys/POWER/m_intr.h 
	.set	MAX_BUC_POLL_GRP, 10	# Maximum number of BUC groupings
					# in the poll array.
	.set	NUM_BUS_SOURCE, 16	# Micro Channel Bus Levels

					# Max number of interrupt sources
	.set    NUM_INTR_SOURCE, NUM_BUS_SOURCE * MAX_BUC_POLL_GRP

	.set    NUM_INTR_PRIORITY, 12   # Number of interrupt priorities

	.dsect	ppcint
xirr_poll:	.long		0	# XIRR with no side effects
xirr:					# XIRR with side effects
cppr:		.byte		0	# CPPR		
xisr:		.byte		0,0,0	# XISR
dsier:		.long		0	# DSIER
mfrr:		.long		0	# local MFRR

#
# Two low-memory variables, intctl_pri and intctl_sec, were added to
# control operation of the external flih on RSPC platforms.  Possible
# values are below.  You must have at least a primary interrupt controller.
# These values shadow the file interrupt.h
#
# extern unsigned short      intctl_pri;
# extern unsigned short      intctl_sec;
#
	.set	INT_CTL_NONE,	0x0000	# No interrupt controller present
	.set	INT_CTL_RS6K,	0x0001	# PowerPC Compliant controller
	.set	INT_CTL_8259,	0x0002	# Cascaded 8259s or compatible
	.set	INT_CTL_MPIC,	0x0004	# MPIC model

