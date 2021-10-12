# @(#)68	1.7  src/bos/kernel/ml/POWER/system_rspc.m4, sysml, bos41J, 9517A_all 4/25/95 17:47:42
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: 
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# NOTES:
#
#       The values in this file have corresponding names in various
#	system_rspc.h.
#
# INTERRUPTS:
#       The PC I/O subsystem on Dakotas use two 8259 equavilant chips to
#       control external hardware interrupts.  This results in 16 separate
#       interrupt levels that must be allocated between the NIO, ISA bus,
#       PCI bus, and potentially PCMCIA bus.

############################################################################
# 8259 Controller internal locations.
#

# Interrupt controller addresses  (Byte addressable)
        .set    INTA00, 0x20
        .set    INTA01, 0x21
        .set    INTB00, 0xA0
        .set    INTB01, 0xA1
	.set	ELCR0,  0x04D0       # IRQ 0-7 Edge/Level control register
	.set	ELCR1,  0x04D1       # IRQ 8-15 Edge/Level control register

############################################################################
# MPIC Controller internal locations.
#

# Locations within the Per Processor Registers
	.set	MPIC_INTACK,	0xA0		# Interrupt Acknowledge Reg.
	.set	MPIC_EOI,	0xB0		# EOI Reg.  Always write 0.

############################################################################

# Current interrupt level register (byte addressable)
        .set    RSPCINT_VECT,0xBFFFFFF0		# Real memory address - IVR
	.set	rspcint_vect, 0xfffffff0	# Effective address - IVR
        .set    u.rspcint_vect, 0xffff
        .set    l.rspcint_vect, 0xfff0
        .set    u.rspcint_seg, 0x87f0		# 601 only
        .set    l.rspcint_seg, 0x000b		# 601 only

# BAT 0&1 values for 603/604
#
#	Planar registers
#	BAT 0 - 256MB      EA	F0000000 - FFFFFFFF
#			 REAL	B0000000 - BFFFFFFF
	.set	uu0.bat_rspcint_seg, 0xF000
	.set	ul0.bat_rspcint_seg, 0x1FFE
	.set	lu0.bat_rspcint_seg, 0xB000
	.set	ll0.bat_rspcint_seg, 0x002A

#	I/O segments - ISA Ports
#	BAT 1 - 256MB      EA	C0000000 - CFFFFFFF
#			 REAL	80000000 - 8FFFFFFF
	.set	uu1.bat_rspcio_seg, 0xC000
	.set	ul1.bat_rspcio_seg, 0x1FFE
	.set	lu1.bat_rspcio_seg, 0x8000
	.set	ll1.bat_rspcio_seg, 0x002A

# I/O space segment mapper (0x80000000 - 0x8fffffff)
        .set    rspcio_seg,   0x87f00008
        .set    u.rspcio_seg, 0x87f0
        .set    l.rspcio_seg, 0x0008

# 	Generic values for low 32 bits of u&l batregs
# 	Maps 256M, etc

	.set	ul.bat_gen, 0x1FFE
	.set	ll.bat_gen, 0x002A
	.set	u.buid7f_gen, 0x87F0
#
#	MPIC Eaddr - BATU value & virt@
#
	.set    uu.bat_mpic_seg, 0xB000
	.set	MPIC_VSEG, 0xB000
