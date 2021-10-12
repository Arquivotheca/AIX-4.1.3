# @(#)62        1.11  src/bos/kernel/ml/POWER/initutil.s, sysml, bos41J, 9508A 2/17/95 14:44:58

#*****************************************************************************
#
# COMPONENT_NAME: (SYSSI) Kernel Initialization Code
#
# FUNCTIONS: write_leds, read_iocc_info
#
# ORIGINS: 27, 83
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
#****************************************************************************
 
#
# LEVEL 1,  5 Years Bull Confidential Information
#

        .file "initutil.s"

include(iplcb.m4)
include(machine.m4)
include(flihs.m4)
include(seg.m4)
include(systemcfg.m4)	
include(scrs.m4)
	

#**********************************************************************
#
#  NAME:  write_leds
#
#  FUNCTION:  Write a value to the LEDs.
#
#  INPUT STATE:
#		R3 = value to write to LEDs.
#
#  OUTPUT STATE:
#		Segment Register 15 has been clobbered
#
#  EXECUTION ENVIRONMENT:
#		This routine assumes the ipl_cb is V=R
#
#**********************************************************************

.machine "com"

	S_PROLOG(write_leds)

	mfmsr	r10			# get current MSR contents
	rlinm	r6,r10,0,~(MSR_DR)	# turn off DR
	mtmsr	r6
	mfsr	r12, r_segr15 		# save off seg reg 15
	isync				# let it be noticed
        LTOC(r4,_system_configuration,data) # addr of system cfg struct
        l       r5,scfg_arch(r4)        # load processor architecture
	cmpli	cr0,r5,POWER_PC         # check for PPC processor
        bne     pwr_leds                # not a PPC processor
	
	l	r5,scfg_modarch(r4)     # model architecture
	cmpli	cr0,r5,RS6K		# check for RS6K model
	bne	init_ppcexit		# no LEDs on non-RS6K model

#       Begin PowerPC specific LED code
#       - Use BUID 0x7F for 601
#       - Use BATs for other PPC Chips and PEGASUS 601

        LTOC(r4,ipl_cb,data)            # address of IPL CB pointer
        l       r4,0(r4)                # address of IPL CB
	l	r5,PROCINFO(r4)		# offset of proc_info structure
	a	r5,r5,r4		# addr of proc_info structure
ifdef(`_POWER_MP',`
led_next_proc:	
	l	r8,PROC_PRESENT(r5)	# processor status
	cmpi	cr0, r8, RUNNING_AIX	# processor running AIX
	beq+	led_test_proc_601	# 
	l	r8, PROC_SIZE(r5)	# proc_info structure size
	add	r5,r5,r8		# next proc_info entry
	b	led_next_proc
led_test_proc_601:
')
	l	r8,IMPLEMENT(r5)	# cpu implementation field
	andil.  r5,r8,POWER_601		# check for 601
	bne     p601_leds		# jump if 601

#	POWER-PC (non-601 Path) for LED Init.
#	Use BATs. 

.machine "ppc"
	mfdbatl r11, BAT0		# save off BAT
	mfdbatu r12, BAT0		# "     "
	
	liu	r8, u.leds_addr_ppc	# Real and Effective address, upper
	oril    r7, r8, 0x2A		# wimg='0101'b, pp=2 (r/w)

	rlinm	r6, r6, 0, ~(MSR_EE)	# Turn off external interrupts
        oril    r6, r6, MSR_DR          # Set Data Relocate bit = 1
        mtmsr   r6
	isync				# make sure DR is noticed
	
	mtdbatl BAT0, r7		# set lower batreg
	oril    r7, r8, 0x2		# bl=128K, v{s}=1, v{p}=0
	mtdbatu BAT0, r7		# validate the BATs
	isync				# make sure BAT load noticed
	
        st      r3, l.leds_addr(r8)     # Write blanks to LEDs
	eieio				# Defensive code..order the store
	isync 				# Make sure store gets done

	mtdbatu	BAT0, r12		# restore BATs
	mtdbatl BAT0, r11		# restore BATs
.machine "com"
init_ppcexit:	
        mtmsr   r10                     # restore MSR to original
	isync				# let DR be noticed
	br				# return
	
#	601 LED initialization. 
#	Use BUID 0x7F.
	
p601_leds:

ifdef(`_RS6K_SMP_MCA',`

#	Begin PowerPC-601 specific LED code using 601 BATS
#	NB: Pegasus has no buid 0x7f.

	.machine "ppc"
	# Set BAT 0 to access NVRAM (V=R, uncached)
	# Note: we assume other BATs do not conflict with this ...
	mfspr	r11, BAT0L	 	# First, save BAT0
	mfspr	r12, BAT0U

	liu	r4, u.leds_addr_ppc	# Effective address, upper
	oril    r8, r4, 0x2A		# wim=010 ks=1 kp=0 pp=2

	rlinm	r6, r6, 0, ~(MSR_EE)	# Turn off external interrupts
        oril    r6, r6, MSR_DR          # Set Data Relocate bit = 1
        mtmsr   r6

	mtspr	BAT0U, r8
	oril    r8, r4, 0x40		# s=0, v=1, size = 128 KBytes
	mtspr	BAT0L, r8

	isync				# let DR/EE and BATs be noticed

        st      r3, l.leds_addr(r4)     # Write to LEDs

	mtspr	BAT0L, r11		# restore BAT0
	mtspr	BAT0U, r12
        mtmsr   r10                     # restore MSR to original

	isync				# let DR be noticed
	.machine "com"
	br				# return
	
',` #else _RS6K_SMP_MCA
	
#       Begin PowerPC specific LED code
	
        cau     r5, 0, u.nv_ram_seg_601 # Set up to use BUID 7F
        oril    r5, r5, l.nv_ram_seg_601
        mtsr    r_segr15, r5            # Load segment register 15
        cau     r4, 0, u.leds_addr_ppc  # Construct address of LEDs

        b       do_write
') #endif _RS6K_SMP_MCA

#       Begin POWER specific LED code
pwr_leds:
.machine "pwr"
	clf	0,r4			# flush what we pulled into the cache
.machine "com"
        cau     r5, 0, u.nv_ram_seg_pwr # Get value for seg reg
        oril    r5, r5, l.nv_ram_seg_pwr
        mtsr    r_segr15, r5            # Load segment register 15
        cau     r4, 0, u.leds_addr_pwr  # Construct address of LEDs

do_write:
        oril    r6, r6, MSR_DR          # Set Data Relocate bit = 1
        mtmsr   r6                      # Turn on Data Relocate
	isync				# let DR be noticed
        st      r3, l.leds_addr(r4)     # Write to LEDs
        mtmsr   r10                     # restore MSR to original
	mtsr	r_segr15, r12		# restore seg reg 15
	isync				# let DR be noticed
	br				# return
	FCNDES(write_leds)

                .toc
                TOCE(_system_configuration,data)
                TOCE(ipl_cb,data)


#**********************************************************************
#
#  NAME:  read_iocc_info
#
#  FUNCTION:  Reads the contents of the IOCC TCE Anchor Register
#
#  INPUT STATE:
#		DR=off upon entry	
#		R3 = I/O Segment register value
#		R4 = address to store IOCC TCE Anchor Register Contents
#
#  OUTPUT STATE:
#		DR=off
#		Segment Register 15 has been clobbered
#
#  EXECUTION ENVIRONMENT:
#		Called from init_sys_ranges() of hardinit 
#
#**********************************************************************
#
#	void read_iocc_info (
#				uint	seg_reg_val,
#				uint	*tce_reg)
#

	.set	u.iocc_addr, 0xF001	#upper address of IOCC address
	.set	l.tce_reg, 0x009C	#offset of TCE Anchor Reg

	S_PROLOG(read_iocc_info)
        mtsr    r_segr15, r3            # Load segment register 15
        mfmsr   r3                      # Get current (non-reloc) MSR
        oril    r6, r3, MSR_DR          # Set Data Relocate bit = 1
        mtmsr   r6                      # Turn on Data Relocate
	isync				# Make sure DR is noticed

        cau     r6, 0, u.iocc_addr	# Set up address of IOCC
	l	r8, l.tce_reg(r6)	# load tce anchor register
	mtmsr	r3			# turn off DR
	isync				# Make sure DR off is noticed

	st	r8, 0(r4)		# save anchor reg contents

	br				# return
	FCNDES(read_iocc_info)

ifdef(`_RS6K_SMP_MCA',`
	.machine "ppc"
#**********************************************************************
#
#  NAME:  nvram_cpy
#
#  FUNCTION: copy bytes from nvram (similar to memcpy)
#
#  INPUT STATE:
#		DR = off
#		r3 = buffer address
#		r4 = NVRAM physical address
#		r5 = number of bytes ( must be > 0 )
#
#  OUTPUT STATE:
#		DR = unchanged
#		r3 = buffer address
#
#  EXECUTION ENVIRONMENT:
#		Called by system init.
#		Runs with xlate off, and all interrupts disabled.
#		Overlap of input and ouput not allowed.
#
#**********************************************************************
#
#	char *nvram_cpy(char *s1, const char *s2, int n)
#
	S_PROLOG(nvram_cpy)

        LTOC(r6,ipl_cb,data)            # address of IPL CB pointer
        l       r6,0(r6)                # address of IPL CB

	l	r7,PROCINFO(r6)		# offset of proc_info structure
	a	r7,r7,r6		# addr of proc_info structure
ifdef(`_POWER_MP',`
nvram_next_proc:	
	l	r8,PROC_PRESENT(r7)	# processor status
	cmpi	cr0, r8, RUNNING_AIX	# processor running AIX
	beq+	nvram_test_proc_601	# 
	l	r8, PROC_SIZE(r7)	# proc_info structure size
	add	r7,r7,r8		# next proc_info entry
	b	nvram_next_proc
nvram_test_proc_601:
')
	l	r6,IMPLEMENT(r7)	# cpu implementation field
	andi.	r6,r6,POWER_601		# check for 601
	cmpi	cr1,r6,0

	rlinm	r8, r4, 0, 0xFFFE0000	# keep high bits of target address
	oril    r7, r8, 0x2A		# wim=010 pp=2 (r/w)

	bne     cr1, nv_sav_601		# jump if 601

#	PowerPC (non-601 Path) for nvram copy.
#	Use PowerPC BATs.

	mfdbatl	r11, BAT0		# First, save BAT0
	mfdbatu	r12, BAT0

	mtdbatl	BAT0, r7
	oril    r7, r8, 0x2		# bl=128K, v{s}=1, v{p}=0
	mtdbatu BAT0, r7

	b	nv_move

#	PowerPC 601 Path for nvram copy.
#	Use PowerPC 601 BATs.
nv_sav_601:
	mfspr	r11, BAT0L		# First, save BAT0
	mfspr	r12, BAT0U

	mtspr	BAT0U, r7
	oril    r7, r8, 0x40		# s=0, v=1, size = 128 KBytes
	mtspr	BAT0L, r7

nv_move:
	mfmsr   r6                      # Get current (non-reloc) MSR
	oril    r7, r6, MSR_DR          # Set Data Relocate bit = 1

	addi	r4, r4, -1		# pre-set origin address
	addi	r10, r3, -1		# pre-set destination address
	mtctr	r5			# set count
next_byte:
	mtmsr   r7                      # Turn on Data Relocate
	isync				# make sure DR is noticed
	lbzu	r9, 1(r4)		# read one byte in the NVRAM
	mtmsr	r6                      # Turn off Data Relocate again
	isync				# make sure DR is noticed
	stbu	r9, 1(r10)		# store in destination
	bdnz	next_byte

	bne     cr1, nv_rest_601	# jump if 601

	mtdbatl	BAT0, r12		# Restore PowerPC BAT0
	mtdbatu BAT0, r11
	isync				# make sure BAT is notified
	br

nv_rest_601:

	mtspr	BAT0U, r12		# Restore 601 BAT0
	mtspr	BAT0L, r11

	br
	FCNDES(nvram_cpy)

	.machine "com"
') #endif _RS6K_SMP_MCA
