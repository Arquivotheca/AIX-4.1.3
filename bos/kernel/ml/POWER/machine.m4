# @(#)07	1.11.1.3  src/bos/kernel/ml/POWER/machine.m4, sysml, bos41J, 9508A 2/3/95 10:02:01
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Defines
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************

#****************************************************************************
#
#  NAME:    machine.m4
#
#           This is the assembly language version of "<sys/machine.h>",
#           which defines various machine-dependent registers and fields
#           within the registers.
#
#****************************************************************************

#
#       Value to write into GPR when nothing useful should be there
#

       .set     DEFAULT_GPR, 0xDEADBEEF
       .set     u.DEFAULT_GPR, 0xDEAD
       .set     l.DEFAULT_GPR, 0xBEEF

#
#	Value of a NULL pointer
#

       .set	nullA, 0

#
#       Machine Status Register (MSR)
#

       .set     MSR_EE,  0x8000           # External interrupt enable
       .set     MSR_PR,  0x4000           # Problem state
       .set     MSR_FP,  0x2000           # Floating point available
       .set     MSR_ME,  0x1000           # Machine check enable
       .set     MSR_FE,  0x0800           # Floating point exception enable (PWR)
       .set     MSR_FE0, 0x0800           # Floating point xcp mode bit0    (PPC)
       .set     MSR_SE,  0x0400           # Single Step Trace Enable    (RS2/PPC)
       .set     MSR_BE,  0x0200           # Branch Trace Enable         (RS2/PPC)
       .set     MSR_IE,  0x0100           # Flt-pt imprecise int enable     (RS2)
       .set     MSR_FE1, 0x0100           # Floating point xcp mode bit1    (PPC)
       .set     MSR_AL,  0x0080           # Alignment check enable          (PWR)
       .set     MSR_IP,  0x0040           # Interrupt prefix active
       .set     MSR_IR,  0x0020           # Instruction relocate on
       .set     MSR_DR,  0x0010	          # Data relocate on
       .set     MSR_PM,  0x0004	          # Performance Monitoring          (RS2)

       .set     DISABLED_MSR, 0x10B0	  #    |    | ME | AL | IR | DR
       .set	DISABLED_REAL_MSR, 0x1080 #    |    | ME | AL |    |
       .set	DEFAULT_MSR, 0x90B0       # EE |    | ME | AL | IR | DR
       .set     DEFAULT_USER_MSR, 0xD0B0  # EE | PR | ME | AL | IR | DR

       .set	SRR1_MSR_BITS, 0x87C0FFFF # bits in SRR1 that are from MSR
       .set	u.SRR1_MSR_BITS, 0x87C0
       .set	l.SRR1_MSR_BITS, 0xFFFF

#
#       Condition Register (CR)
#

       .set     CR_LT,  0x80000000      # Less Than,        field 0
       .set     CR_GT,  0x40000000      # Greater Than,     field 0
       .set     CR_EQ,  0x20000000      # Equal,            field 0
       .set     CR_SO,  0x10000000      # Summary Overflow, field 0
       .set     CR_FX,  0x08000000      # Floating point exception
       .set     CR_FEX, 0x04000000      # Floating point enabled exception
       .set     CR_VX,  0x02000000      # Floating point invalid operation
       .set     CR_OX,  0x01000000      # Copy of FPSCR(OX)


#
#       Fixed Point Exception Register (XER)
#

       .set     XER_SO, 0x80000000      # Summary overflow
       .set     XER_OV, 0x40000000      # Overflow
       .set     XER_CA, 0x20000000      # Carry


#
#       Data Storage Interrupt Status Register (DSISR)
#

       .set     DSISR_IO,   0x80000000  # I/O exception
       .set     DSISR_PFT,  0x40000000  # No valid PFT for page
       .set     DSISR_LOCK, 0x20000000  # Access prohibited by data locking (PWR)
       .set     DSISR_FPIO, 0x10000000  # FP load/store to I/O space        (PWR)
       .set     DSISR_PROT, 0x08000000  # Protection exception
       .set     DSISR_LOOP, 0x04000000  # PFT search > 127 entries          (RS1)
       .set     DSISR_DRST, 0x04000000  # Direct Store Access by lwarx, etc (PPC)
       .set     DSISR_ST,   0x02000000  # 1 => store, 0 => load
       .set     DSISR_SEGB, 0x01000000  # Crosses segment boundary, from    (PWR)
                                        #   T=0 to T=1
       .set	DSISR_DABR, 0x00400000  # DABR exception                (RS2/PPC)
       .set	DSISR_EAR,  0x00100000  # eciwx with EAR enable = 0         (PPC)

#
#       System Recall Register 1 (SRR1)
#
#       Note:  This register is used for various purposes, depending on the
#         type of interrupt, thus the prefix of the name will vary.
#

       .set     SRR_IS_PFT,   0x40000000   # No valid PFT for page
       .set     SRR_IS_ISPEC, 0x20000000   # I-fetch from special segment   (PWR)
       .set     SRR_IS_IIO,   0x10000000   # I-fetch from I/O space
       .set	SRR_IS_GUARD, 0x10000000   # I-fetch from guarded storage
       .set     SRR_IS_PROT,  0x08000000   # Protection exception
       .set     SRR_IS_LOOP,  0x04000000   # PFT search > 127 entries       (RS1)

       .set     SRR_PR_FPEN,  0x00100000   # FP Enabled Interrupt exception
       .set     SRR_PR_INVAL, 0x00080000   # Invalid operation
       .set     SRR_PR_PRIV,  0x00040000   # Privileged instruction
       .set     SRR_PR_TRAP,  0x00020000   # Trap instruction
       .set	SRR_PR_IMPRE, 0x00010000   # Imprecise PI                   (PPC)

	.set	INVCSR15.h,   0x000F	   # invalid csr15
	.set	INVCSR15.l,   0x00FF
	.set	INVBUSLIMT.h, 0xFFFF       # invalid limit register 
	.set	INVBUSLIMT.l, 0x0000
	.set	CSR15.h,      0x004F       # address of csr15 in IOCC
	.set	CSR15.l,      0x0060	   # add. space
	.set	BUSLIMT.h,    0x0040       # address of limit reg in IOCC 
	.set	BUSLIMT.l,    0x0040	   # add. space
	
	# seg reg. value that ensures exceptions for PPC processor bus
	# displays (T=0, Ks=Kp=1, SID=7FFFFE)
	.set	INV_PB_SEGREG_PPC.h, 0x607F 
	.set	INV_PB_SEGREG_PPC.l, 0xFFFE 
	.set	KPBIT,	      0x2000	   # Kp bit
