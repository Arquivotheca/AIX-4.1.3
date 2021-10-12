# @(#)31        1.12  src/bos/kernel/ml/POWER/scrs.m4, sysml, bos411, 9428A410j 4/29/94 03:17:27
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
# (C) COPYRIGHT International Business Machines Corp. 1989, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************

#-------------------------------------------------------------------#
#
#  special purpose register labels
#
#-------------------------------------------------------------------#

	.set    MQ, 0                   # multiply/quotient reg
	.set    XER, 1                  # fixed point exception reg
	.set    LR, 8                   # link reg
	.set    CTR, 9                  # count reg
	.set	IMR, 16			# instruction match reg
	.set    TID, 17                 # transaction ID reg
	.set    DSISR, 18               # data storage interrupt status
	.set    DAR, 19                 # data address reg
	.set    MT_RTCU, 20		# realtime clock upper (for mtspr)
	.set    MT_RTCL, 21		# realtime clock lower (for mtspr)
	.set    MT_DEC, 22              # decrementer (for mtspr)
	.set    MF_RTCU, 04             # realtime clock upper (for mtspr)
	.set    MF_RTCL, 05		# realtime clock lower (for mfspr)
	.set    MF_DEC_PWR, 06		# decrementer (for mfspr) Power
	.set	MF_DEC_PPC, 22		# decrementer (for mfspr) Power PC
	.set	DABR, 23		# data address breakpoint reg
	.set    SDR0, 24		# storage description reg 0
	.set    SDR1, 25		# storage description reg 1
	.set    SRR0, 26		# save/restore reg 0
	.set    SRR1, 27		# save/restore reg 1
	.set	DSAR, 28		# 
	.set	TSR, 29			# 
	.set	ILCR, 30		# Interrupt level control reg
ifdef(`_POWER_PC',`
	.set	SPRG0, 272		# software use SPR0
	.set	SPRG1, 273		# software use SPR1
	.set	SPRG2, 274		# software use SPR2
	.set	SPRG3, 275		# software use SPR3

 	.set	MT_TBL,   284		 # time base lower (post-601)
 	.set	MT_TBU,   285		 # time base upper (post-601)

	.set    PID, 1023		# processor identification register
	.set	PID_MASK, 0xf		# 4 bits only in this register

ifdef(`_POWER_601',`
	.set    BAT0U, 528		 # Instruction block addr translation
	.set    BAT0L, 529		 #
	.set    BAT1U, 530		 #
	.set    BAT1L, 531		 #
	.set    BAT2U, 532		 #
	.set    BAT2L, 533		 #
	.set    BAT3U, 534		 #
	.set    BAT3L, 535		 #
') #endif _POWER_601

	.set    IBAT0U, 528		 # Instruction block addr translation
	.set    IBAT0L, 529		 #
	.set    IBAT1U, 530		 #
	.set    IBAT1L, 531		 #
	.set    IBAT2U, 532		 #
	.set    IBAT2L, 533		 #
	.set    IBAT3U, 534		 #
	.set    IBAT3L, 535		 #

	.set    DBAT0U, 536		 # Data block address translation upper
	.set    DBAT0L, 537		 # Data block address translation lower
	.set    DBAT1U, 538		 #
	.set    DBAT1L, 539		 #
	.set    DBAT2U, 540		 #
	.set    DBAT2L, 541		 #
	.set    DBAT3U, 542		 #
	.set    DBAT3L, 543		 #

	.set    BAT0,   0                # Extended mnemonic codes post-601 BATs
	.set    BAT1,   1                #   "
	.set    BAT2,   2                #   "
	.set    BAT3,   3                #   "

ifdef(`_POWER_603',`
	.set    DMISS,  976              # Data tlb miss address
	.set    DCMP,   977              # Data tlb miss compare
	.set    HASH1,  978              # Pteg1 address
	.set    HASH2,  979              # Pteg2 address
	.set    IMISS,  980              # Instruction tlb miss address
	.set    ICMP,   981              # Instruction tlb miss compare
	.set    RPA,    982              # tlb replacement entry
	.set    HID0603, 1008            # checkstop and misc. enables
	.set    IABR603, 1010            # Instruction breakpoint register
') #endif _POWER_603

ifdef(`_POWER_604',`
	.set    HID0604, 1008            # checkstop and misc enables
') #endif _POWER_604

') #endif _POWER_PC
	
#
#       Machine State Register (MSR) bit definitions
#
	.set    EE, 16			# 1 enables external interrupts
	.set    PR, 17			# 1 sets problem state
	.set    FP, 18                  # 1 implies FP registers available
	.set    ME, 19                  # 1 enables machine check interrupts
	.set    FE, 20                  # 1 enables FP enabled exceptions
	.set    SE, 21                  # 1 enables single step
	.set    BE, 22                  # 1 enables branch & trap
	.set    IE, 23                  # 1 enables FP imprecise
	.set    AL, 24                  # 1 enables alignment checking
	.set    IP, 25                  # 1 sets interrupt prefix to ROS
	.set    IR, 26                  # 1 enables instruction relocate
	.set    DR, 27                  # 1 enables data relocate
	.set    PM, 29                  # 1 enables performance monitoring
#
#       BUID 0 layout
#
	.set	BUID0, 0x8000		# access to Bus Unit ID 0 by:
					#   cau   <reg>, 0, BUID0
					#   mtsr  sr15, <reg>

	.set    EIM0, 0                 # external interrupt mask 0-31
	.set    EIM1, 4                 # external interrupt mask 32-63
	.set    EIS0, 0x10              # external interrupt source 0-31
	.set    EIS1, 0x14              # external interrupt source 32-63
	.set    PEIS0, 0x10             # external interrupt source 0-31 RS2
	.set    PEIS1, 0x14             # external interrupt source 32-63 RS2
	.set    DEB, 0x20               # decrementer EIS BID
	.set    MEEB, 0x24              # memory error EIS BID
	.set    LED, 0x300              # LED address
	.set    BSCR0, 0x1000		# bit-steering config reg 0
	.set    BSCR1, 0x1004		# bit-steering config reg 1
	.set    BSCR2, 0x1008		# bit-steering config reg 2
	.set    BSCR3, 0x100c		# bit-steering config reg 3
	.set    BSCR4, 0x1010		# bit-steering config reg 4
	.set    BSCR5, 0x1014		# bit-steering config reg 5
	.set    BSCR6, 0x1018		# bit-steering config reg 6
	.set    BSCR7, 0x101c		# bit-steering config reg 7
	.set    BSCR8, 0x1020		# bit-steering config reg 8
	.set    BSCR9, 0x1024		# bit-steering config reg 9
	.set    BSCRA, 0x1028		# bit-steering config reg 10
	.set    BSCRB, 0x102c		# bit-steering config reg 11
	.set    BSCRC, 0x1030		# bit-steering config reg 12
	.set    BSCRD, 0x1034		# bit-steering config reg 13
	.set    BSCRE, 0x1038		# bit-steering config reg 14
	.set    BSCRF, 0x103c		# bit-steering config reg 15
	.set    CRE0, 0x1040		# configuration reg extent 0
	.set    CRE1, 0x1044		# configuration reg extent 1
	.set    CRE2, 0x1048		# configuration reg extent 2
	.set    CRE3, 0x104c		# configuration reg extent 3
	.set    CRE4, 0x1050		# configuration reg extent 4
	.set    CRE5, 0x1054		# configuration reg extent 5
	.set    CRE6, 0x1058		# configuration reg extent 6
	.set    CRE7, 0x105c		# configuration reg extent 7
	.set    CRE8, 0x1060		# configuration reg extent 8
	.set    CRE9, 0x1064		# configuration reg extent 9
	.set    CREA, 0x1068		# configuration reg extent 10
	.set    CREB, 0x106c		# configuration reg extent 11
	.set    CREC, 0x1070		# configuration reg extent 12
	.set    CRED, 0x1074		# configuration reg extent 13
	.set    CREE, 0x1078		# configuration reg extent 14
	.set    CREF, 0x107c		# configuration reg extent 15
	.set    EESR, 0x1080		# external error status reg
	.set    EASR, 0x1084		# external error address reg
	.set    EECAR, 0x1088		# external error corr addr reg
	.set    ECCSR, 0x108c		# ECC signature/syndrome reg
	.set    MESR, 0x1090		# machine check error status reg
	.set    MEAR, 0x1094		# machine check error address reg
	.set    DSIRR, 0x1098		# exception reason for data stor int
	.set    SBSSR, 0x10a0		# single bit signature/syndrome reg
	.set    SBSR, 0x10a4		# single bit status register
	.set    SBAR, 0x10a8		# single bit address register
	.set    MTOR, 0x10b0		# memory timeout value
	.set    STVR, 0x10b4		# scrubbing timer value register
	.set    SSAR, 0x10b8		# scrub start address register
	.set    SEAR, 0x10bc		# scrub end address register
	.set    SCCR, 0x10c0		# storage control unit control reg
	.set    DCCR, 0x10d0		# data cache control register
#
#       IOCC base offsets
#
	.set    IOCC_ENAB, 0x80		# IOCC interrupt enable mask address
	.set    IOCC_VEC, 0x90		# IOCC bus vector table start addr
#
#	RS2 Interrupt subsystem commands
#
	.set	ICO_UCIL, 0x0000	# Update Current Interrupt Level
	.set	ICO_CLI, 0x0100		# Clear Interrupt Level
	.set	ICO_SLI, 0x0200		# Set Interrupt Level

#
#	Segment register defines
#
	.set	SR_KsKp, 0x60000000	# Ks and Kp bits
