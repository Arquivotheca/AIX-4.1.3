# @(#)26	1.9  src/bos/kernel/ml/POWER/debug.s, sysml, bos411, 9428A410j 6/10/94 12:23:51
#*****************************************************************************
#
# COMPONENT_NAME: (SYSML) Kernel Assembler Code
#
# FUNCTIONS: debugger assembler functions
#
# ORIGINS: 27
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
#****************************************************************************

	.file	"debug.s"
	.machine "com"
	.using	low,r0

#*******************************************************************************
#
# NAME: realbyt, read_align
#
# FUNCTION:
#	read 1, 2, or 4 bytes of data from memory, both functions flush
#	and invalidate cache lines touched
#
#	realbyt does read in real mode
#	read_align read xlate on
#	NOTE: real mode reads do *not* ignore seg regs with T=1, so this routine
#		now saves the current sr and sets it unused before reading
#
#	input:	r3 = address of the data
#		r4 = number of bytes(1,2,or 4) to read
# 	output:	r3 = the data that was read
#
# EXECUTION ENVIRONMENT:
#	Only called from debugger
#
#*******************************************************************************

 	S_PROLOG(realbyt)
#
	.machine "com"
	.set	DRBIT, 0x10
	mr	r9, r3			# Save addr for segreg calculations
	mfmsr	r6			# get current msr
	rlinm	r7, r6, 0, ~DRBIT	# unset data relocate bit
	mtmsr	r7			# put it back in msr
	isync

# Figure out which architecture we're using, because the segreg instructions are
# different between POWER and PPC.
	l	r12, syscfg_arch	# get architecture value
	cmpi	cr1, r12, POWER_PC	# check for PPC, save comp in cr1
	beq	cr1, rb_ppcsavesr	# Do PPC save and set of segreg

	.machine "pwr"
	mfsri	r8, 0, r9		# Save segment register for addr
	rlinm	r5, r8, 0, 0x7fffffff	# Mask off T bit
	mtsri	r5, 0, r9		# Mark segreg for addr unused
	b	read_com		# Ready to start reading

rb_ppcsavesr:
	.machine "ppc"
	mfsrin	r8, r9			# Save segment register for addr
	rlwinm	r5, r8, 0, 0x7fffffff	# Mask off T bit
	mtsrin	r5, r9			# Mark segreg for addr unused
	isync
	b	read_com

	.machine "com"
ENTRY(read_align):
	.globl	ENTRY(read_align)
	mr	r9, r3			# copy address to r9 for segreg use
	mfmsr	r6

# Figure out which architecture we're using, because the segreg instructions are
# different between POWER and PPC.
	l	r12, syscfg_arch	# get architecture value
	cmpi	cr1, r12, POWER_PC	# check for PPC, save comp in cr1
	beq	cr1, ra_ppcsavesr	# Do PPC save of segreg

	.machine "pwr"
	mfsri	r8, 0, r9		# Save seg reg for addr
	b	read_com

ra_ppcsavesr:
	.machine "ppc"
	mfsrin	r8, r9			# Save seg reg for addr

	.machine "com"
read_com:
#					# check the length of data to read
 	cmpli	0,r4,2			# compare r4 with 2 and put results
 					# in bit field (BF) 0
	ai	r5,r3,0 		# Move address to r5 for clf later
 	bge	rb02			# branch if 2 or 4 bytes to read
	lbz	r3,0(r3)		# load byte into r3
	b	rbfini
rb02:
 	bgt	rb04			# branch if 4 bytes to read
	lhz	r3,0(r3)		# load half word into r3
	b	rbfini
rb04:
	l	r3,0(r3)		# load word into r3
rbfini:
	beq	cr1, rb_ppcflush	# branch if PPC

	.machine "pwr"
	clf	0,r5			# flush/invalidate cache
	dcs
	mtsri	r8, 0, r9		# Restore seg reg for addr
	b	rb_fldone

rb_ppcflush:
	.machine "ppc"
	dcbf	0, r5			# flush/invalidate cche
	sync
	icbi	0, r5
	mtsrin	r8, r9			# Restore seg reg for addr
		
rb_fldone:
	.machine "com"
	isync
	mtmsr	r6			# reset msr to the way it was
	isync

	br

#*******************************************************************************
#
# NAME: writebyt, write_align
# 
# FUNCTION:
# 	write 1, 2, or 4 bytes of data to memory.  Both functions also flush
#	and invalidate cache blocks written
#
# 	input:  r3=address of the data 
#         r4=the data to write.
#         r5=N of bytes to store, 1, 2, or 4
#                                           
#	output:  the memory is changed.
#
# EXECUTION ENVIRONMENT:
#	called only from the debugger
#
#*******************************************************************************

 	S_PROLOG(writbyt)
#
	.machine "com"
					# DRBIT, 0x10
	mr	r9, r3			# copy address to r9 for segreg use
	mfmsr	r6			# get current msr
	rlinm	r7, r6, 0, ~DRBIT	# unset data relocate bit
	mtmsr	r7			# put it back in msr
	isync

# Figure out which architecture we're using, because the segreg instructions are
# different between POWER and PPC.
	l	r12, syscfg_arch	# get architecture value
	cmpi	cr1, r12, POWER_PC	# check for PPC, save comp in cr1
	beq	cr1, wb_ppcsavesr	# Do PPC save and set of segreg

	.machine "pwr"
	mfsri	r8, 0, r9		# Save segment register for addr
	rlinm	r10, r8, 0, 0x7fffffff	# Mask off T bit
	mtsri	r10, 0, r9		# Mark segreg for addr unused
	b	write_com		# Ready to write

wb_ppcsavesr:
	.machine "ppc"
	mfsrin	r8, r9			# Save segment register for addr
	rlwinm	r10, r8, 0, 0x7fffffff	# Mask off T bit
	mtsrin	r10, r9			# Mark segreg for addr unused
	isync
	b	write_com

	.machine "com"
ENTRY(write_align):
	.globl	ENTRY(write_align)
	mr	r9, r3			# copy address to r9 for segreg use
	mfmsr	r6

# Figure out which architecture we're using, because the segreg instructions are
# different between POWER and PPC.
	l	r12, syscfg_arch	# get architecture value
	cmpi	cr1, r12, POWER_PC	# check for PPC, save comp in cr1
	beq	cr1, wa_ppcsavesr	# Do PPC save of segreg

	.machine "pwr"
	mfsri	r8, 0, r9		# Save seg reg for addr
	b	write_com

wa_ppcsavesr:
	.machine "ppc"
	mfsrin	r8, r9			# Save seg reg for addr

	.machine "com"
write_com:
#					# check the length of data to write
 	cmpli	0,r5,2			# compare r5 with 2 and put results
 					# in bit field (BF) 0
 	bge	wb02			# branch if 2 or 4 bytes to write
	stb     r4,0(r3)                # store the byte
	b	wbfini
wb02:
 	bgt	wb04			# branch if 4 bytes to write
	sth	r4,0(r3)		# store half word
	b	wbfini
wb04:
	st	r4,0(r3)		# store word
wbfini:
	beq	cr1, wb_ppcflush	# branch if PPC

	.machine "pwr"
	clf	0,r3			# flush/invalidate cache
	dcs
	mtsri	r8, 0, r9		# Restore seg reg for addr
	b	wb_fdone

wb_ppcflush:
	.machine "ppc"
	dcbf	0, r3			# flush/invalidate cache
	sync
	icbi	0, r3
	mtsrin	r8, r9			# Restore seg reg for addr

wb_fdone:
	.machine "com"
	isync
	mtmsr	r6			# reset msr to the way it was
	isync

	br

	.machine "any"
	S_PROLOG(cache_sync)
	dcs
	ics
	S_EPILOG

#*******************************************************************************
#
# NAME: Move to bpr - breakpoint register
#
# FUNCTION:
#	input:	r3 = address to put in breakpoint register
# 	output:	bpr updated
#
# EXECUTION ENVIRONMENT:
#	Only called from debugger
#
	S_PROLOG(mtbpr)
	mtspr 	0x10,r3
	S_EPILOG

ifdef(`_POWER_PC',`
#******************************************************************************
#
# NAME: Move from time base upper 
#
# CALL: unsigned int db_mftbu
#
# FUNCTION: Reads time base upper on PowerPC
#
# RETURNS: raw contents of register
# 	output:	R3 = contents of time base upper
#
# EXECUTION ENVIRONMENT:
#	Only called from debugger
#

	S_PROLOG(db_mftbu)
	.machine "ppc"
	mftbu	r3		# read high part of time base register
	.machine "com"
	br

#******************************************************************************
#
# NAME: Move from time base upper 
#
# CALL: unsigned int db_mftbl
#
# FUNCTION: Reads time base lower on PowerPC
#
# RETURNS: raw contents of register
# 	output:	R3 = contents of time base lower
#
# EXECUTION ENVIRONMENT:
#	Only called from debugger
#

	S_PROLOG(db_mftbl)
	.machine "ppc"
	mftb	r3		# read low part of time base register
	.machine "com"
	br

#******************************************************************************
#
# NAME: Move to time base upper 
#
# CALL: void db_mttbu(unsigned int)
#
# FUNCTION: Writes time base upper on PowerPC
# 	input:	R3 = contents to put in  time base upper
#
# RETURNS: nothing (void)
#
# EXECUTION ENVIRONMENT:
#	Only called from debugger
#

	S_PROLOG(db_mttbu)
	.machine "ppc"
	mttbu	r3
	.machine "com"
	br

#******************************************************************************
#
# NAME: Move to time base lower
#
# CALL: void db_mttbl(unsigned int)
#
# FUNCTION: Writes time base lower on PowerPC
# 	input:	R3 = contents to put in  time base lower
#
# RETURNS: nothing (void)
#
# EXECUTION ENVIRONMENT:
#	Only called from debugger
#

	S_PROLOG(db_mttbl)
	.machine "ppc"
	mttb	r3         # documentation says should be mttbl
	.machine "com"
	br
',)

ifdef(`_POWER_RS2',`

#
#
# FUNCTION: read peis registers
#
# CALL:	void db_mfpeis(peis0, peis1)
#	int *peis0;		/* return value of peis0 here */
#	int *peis1;		/* reture value of peis1 here */
#
# EXECUTION ENVIORNMET:
#	Interrupt level, and process level
#
# RETURNS: None
#
#

	S_PROLOG(db_mfpeis)
	cau	r6, 0, BUID0		# value for i/o seg reg
	mfsr	r5, sr15		# save seg reg 15
	mtsr	sr15, r6		# to seg reg 15 for i/o
	cau	r6, 0, 0xf000		# set r6 to access i/o segment
	cal	r6, PEIS0(r6)		# add offset to PEISs
	lsi	r7, r6, 8		# load PEIS0 and PEIS1
	mtsr	sr15, r5		# restore seg reg 15
	st	r7, 0(r3)		# set PEIS0
	st	r8, 0(r4)		# set PEIS1
	br
	FCNDES(db_mfpeis)


#
#
# FUNCTION: Set bit in PEIS0/1 for RS2
#
# CALL:	void db_rs2peis_set(level)
#	int level;			/* 0 - 63.  Bit to set in PEIS0/1 */
#
# RETURNS:
#	None
#
#

	S_PROLOG(db_rs2peis_set)
	cau	r3, r3, ICO_SLI		# add in ICO cmd to set level
	mtspr	ILCR, r3                # set the level
	br

#
#
# FUNCTION: Clear level in PEIS0/1 for RS2
#
# CALL: void db_rs2peis_reset(level)
#	int level;			/* 0-63.  level to clear */
#
# RETURNS:
#	None
#
#

	S_PROLOG(db_rs2peis_reset)
	cau	r3, r3, ICO_CLI		# add in ICO cmd to clear level
	mtspr	ILCR, r3                # clear the level
	br


#
#
# FUNCTION: Get contents of ILCR without any side effects
#
# CALL: void db_mfilcr()
#
# RETURNS:
#	contents of ILCR
#
#

	S_PROLOG(db_mfilcr)
	mfspr	r3, ILCR                # get ILCR
	rlinm   r4, r3, 0, 0xFF         # get PIL
	cmpi	cr0, r4, 0x3F           # if PIL <= 64 set the PEIS bit
	bgt     cr0, ilcr_rd1           # that was cleared by the read of ILCR

	cau	r4, r4, ICO_SLI		# add in ICO cmd to set level
	mtspr	ILCR, r4                # set the level

ilcr_rd1:	
	br

#
# Move to dabr
#
# input:        r3 = value to write to dabr
#
# output:       dabr updated
#
        S_PROLOG(mtdabr)
        MTSPR(DABR)                     # get the dar
        br
        FCNDES(mtdabr)

#
# Move from dabr
#
# input:        none
#
# output:       r3=dabr value
#
        S_PROLOG(mfdabr)
        MFSPR(DABR)                      # get the dar
        br
        FCNDES(mfdabr)


	.machine "601"
        .set    HID1, 1009		#
        .set    HID2, 1010		#
        .set    HID5, 1013		#DABR
#
# Move to dabr - 601
#
# input:        r3 = value to write to dabr
#
# output:       dabr (hid5)  updated
#
        S_PROLOG(mtdabr601)
	mtspr	HID5,r3
        br
        FCNDES(mtdabr601)


# Move to hid1 - 601
#
# input:        r3 = value to write to hid1
#
# output:        hid1 updated
#
        S_PROLOG(mthid1)
	mtspr	HID1,r3
        br
        FCNDES(mthid1)


# Move to hid2 - 601
#
# input:        r3 = value to write to hid2
#
# output:        hid2 updated
#
        S_PROLOG(mthid2)
	mtspr	HID2,r3
        br
        FCNDES(mthid2)


# Move from hid1 - 601
#
# input:        none
#
# output:        r3 = value of hid1
#
	S_PROLOG(mfhid1)
	mfspr   r3,HID1
	br
	FCNDES(mfhid1)



',)

include(low_dsect.m4)
include(systemcfg.m4)
include(scrs.m4)

