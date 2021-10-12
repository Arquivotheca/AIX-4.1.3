# @(#)03	1.43.1.2  src/bos/kernel/ml/POWER/misc.s, sysml, bos41J, 9518A_all 5/2/95 08:52:26
#
#   COMPONENT_NAME: SYSML
#
#   FUNCTIONS: NONE
#
#   ORIGINS: 27, 83
#
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1989, 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
#
# LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************


	.file	"misc.s"
	.machine "com"
	.using	low,0

include(systemcfg.m4)
include(macros.m4)

ifdef(`_SLICER',`
include(proc.m4)

# Slicer-only function
#
# Restore user private segment register
# input: r3 = curproc
#
        S_PROLOG(rupsr)
        l       r3, p_adspace(r3)        # get users sr2 value
        rlinm   r3, r3, 0, ~sr_k_bit     # clear the "k" bit
        mtsr    r_user, r3               # restore it to seg reg
        br                               # return
        FCNDES(rupsr)

')

#*********************************************************
# usermode()
# sets machine state to user (problem) state. caller must
# be in supervisor (kernel) state.
#							 *
#********************************************************
#
 	S_PROLOG(usermode)
	mfmsr	r3		# get current msr
	oril	r3,r3,pr	# set problem state
	mtmsr   r3		# set machine state
	isync
	br
 	FCNDES(usermode)

#
#  static breakpoint or trap
#
        S_PROLOG(brkpoint)		# Formerly misnamed brkpt()
	TRAP
	br
        FCNDES(brkpoint)
#
#  move from msr register
#  output: contents of msr into register 3
        S_PROLOG(mfmsr)
	include(scrs.m4)
	mfmsr	r3			# value to set to in r3
	br
        FCNDES(mfmsr)
#
#  move to msr register
#  input: value to or into msr in reg 3
#  output: msr updated
#
        S_PROLOG(mtmsr)
	mtmsr	r3			# value to set to in r3
	isync
	br
        FCNDES(mtmsr)

#
# Move from tid
#
# input:	none
#
# output:	r3=tid value
#
	S_PROLOG(mftid)
	MFSPR(TID)			# get the tid
	br
	FCNDES(mftid)

#
# move to dsisr
#
# input:	r3 = value of dsisr to write
#
# output:	dsisr updated
#
	S_PROLOG(mtdsisr)
	MTSPR(DSISR)			# set the dsisr
	br
	FCNDES(mtdsisr)

#
# Move from dsisr
#
# input:	none
#
# output:	r3=dsisr value
#
	S_PROLOG(mfdsisr)
	MFSPR(DSISR)			# get the dsisr
	br
	FCNDES(mfdsisr)

#
# Move to sdr0
#
# input:	r3 = value to write to sdr0
#
# output:	sdr0 updated
#
	S_PROLOG(mtsdr0)
	MTSPR(SDR0)			# set the sdr0
	br
	FCNDES(mtsdr0)
#
# Move from sdr0
#
# input:	none
#
# output:	r3=sdr0 value
#
	S_PROLOG(mfsdr0)
	MFSPR(SDR0)			# get the sdr0
	br
	FCNDES(mfsdr0)
#
# Move to sdr1
#
# input:	r3 = value to write to sdr1
#
# output:	sdr1 updated
#
	S_PROLOG(mtsdr1)
	MTSPR(SDR1)			# set the sdr1
	br
	FCNDES(mtsdr1)
#
# Move from sdr1
#
# input:	none
#
# output:	r3=sdr1 value
#
	S_PROLOG(mfsdr1)
	MFSPR(SDR1)			# get the sdr1
	br
	FCNDES(mfsdr1)
#
# Move to rtcl
#
# input:	r3 = value to write to rtcl
#
# output:	rtcl updated
#
	S_PROLOG(mtrtcl)
	MTSPR(MT_RTCL)			# set the rtcl
	br
	FCNDES(mtrtcl)
#
# Move from rtcl
#
# input:	none
#
# output:	r3=rtcl value
#
	S_PROLOG(mfrtcl)
	MFSPR(MF_RTCL)			# get the rtcl
	br
	FCNDES(mfrtcl)
#
# Move to rtcu
#
# input:	r3 = value to write to rtcu
#
# output:	rtcu updated
#
	S_PROLOG(mtrtcu)
	MTSPR(MT_RTCU)			# set the rtcu
	br
	FCNDES(mtrtcu)
#
# Move from rtcu
#
# input:	none
#
# output:	r3=rtcu value
#
	S_PROLOG(mfrtcu)
	MFSPR(MF_RTCU)			# get the rtcu
	br
	FCNDES(mfrtcu)
#
# Move to dec
#
# input:	r3 = value to write to dec
#
# output:	dec updated
#
	S_PROLOG(mtdec)
	MTSPR(MT_DEC)			# set the dec
	br
	FCNDES(mtdec)

#
# Move to dar
#
# input:	r3 = value to write to dar
#
# output:	dar updated
#
	S_PROLOG(mtdar)
	MTSPR(DAR)			# get the dar
	br
	FCNDES(mtdar)
#
# Move from dar
#
# input:	none
#
# output:	r3=dar value
#
	S_PROLOG(mfdar)
	MFSPR(DAR)			# get the dar
	br
	FCNDES(mfdar)

#*********************************************************
# scan memory for a non-zero bit and return its bit index*
#							 *
# input:  r3=address of start of scan                    *
#                                                        *
# output: r3=zero based index of the first one bit       *
# 					                 *
# N.B. This routine assumes that the input pointer is    *
#      word aligned and that a one bit will be found     *
#********************************************************
#
 	S_PROLOG(bitindex)
	l	r4,0(r3)        #Load first word - assume r3 word aligned
	ai	r6,r3,0 	#Move address to r6 - free instruction under the load
	cmpli	0,r4,0          #Check for zero - cntlz doesn't do that
	cntlz	r3,r4           #Free inst. waiting for cmpli to get to branch unit
	bner			#Return if first word is it
bitlp:
     	lu	r4,4(r6)        #Load next word
     	cmpli	0,r4,0		#Compare first since cntlz doesn't do the right thing
     	cntlz	r5,r4           #Free while waiting for compare
	a	r3,r3,r5        #Free - add number of bits to 1 or 32 if word is zero
	beq     bitlp		#This word zero - try again
	br
 	FCNDES(bitindex)

#*********************************************************
# get_stkp()
# return stack pointer					 *
#							 *
# output: r3 = stack pointer = r1			 *
# 					                 *
#********************************************************
#
 	S_PROLOG(get_stkp)
	mr	r3,r1		# get stack pointer
	br
 	FCNDES(get_stkp)

#
#
# enable()
# enable interrupts
# caller must be in supervisor (kernel) state.
#							 *
#
 	S_PROLOG(enable)
	mfmsr	r3		# get current msr
	oril	r3,r3,ee	# set enable bit
	mtmsr   r3		# set machine state
	br
 	FCNDES(enable)
#
#
# disable()
# disable interrupts
# returns msr on entry in r3
# caller must be in supervisor (kernel) state.
#							 *
#
 	S_PROLOG(disable)
	mfmsr	r3		# get current msr
	CLRBIT(r4,r3,16)	# clear enable bit
	mtmsr   r4		# set machine state
	br
 	FCNDES(disable)

#
# addtomem (ptr, val1)
# int *ptr (passed in r3)
# int val1 (passed in r4)
#
# adds atomically val1 to the word at address ptr.
#
 	S_PROLOG(addtomem)
	mfmsr	r0		# get current msr
	CLRBIT(r8,r0,16)	# clear enable bit
	mtmsr   r8		# set machine state to disabled
	l	r9,0(r3)	# get word
	a	r9,r9,r4	# add val1
	st	r9,0(r3)	# put in memory
	mtmsr	r0		# restore state
	br
 	FCNDES(addtomem)

#
# addtohmem (ptr, val1)
# short *ptr (passed in r3)
# short val1 (passed in r4)
#
# adds atomically val1 to the short at address ptr.
#
 	S_PROLOG(addtohmem)
	mfmsr	r0		# get current msr
	CLRBIT(r8,r0,16)	# clear enable bit
	mtmsr   r8		# set machine state to disabled
	lhz	r9,0(r3)	# get short
	a	r9,r9,r4	# add val1
	sth	r9,0(r3)	# put in memory
	mtmsr	r0		# restore state
	br
 	FCNDES(addtohmem)

#*****************************************************************************
#
# FUNCTION: switch from si stack to scheduler's kernel stack
#
#*****************************************************************************

	S_PROLOG(runsched)
	LTOC(r1, __ublock, data)# get address of top of kernel stack (1st thrd)
	cal	r1, -stkmin(r1)	# adjust into stack frame
	rlinm	r1, r1, 0 , ~7	# double word allign
	LTOC(r3, ret_trap, data)
	mtlr	r3		# set return address to breakpoint
	.extern	ENTRY(sched)
	ba	ENTRY(sched)
ret_trap:
	TRAP			# We should not return from sched


#********************************************************************
#
#  FUNCTION: xbcopy
#
#
#************************************************************************
#
#  NAME: xbcopy  move (overlapped ok) and check for errors
#
#  FUNCTION: Equal length character string move
#
#  EXECUTION ENVIRONMENT:
#  Must be in the kernel with write access to the current mst
#  Standard register usage and linkage convention.
#  Registers used r0,r3-r12
#  Condition registers used: 0,1
#  No stack requirements.
#
#  NOTES:
#  The source string represented by R3 is moved to the target string
#  area represented by R4 and R5.
#
#  The strings may be on any address boundary and may be of any length from 0
#  through (2**31)-1 inclusive.  The strings may overlap.  The move is performed
#  in a nondestructive manner (backwards if necessary).
#
#  The addresses are treated as unsigned quantities,
#  i.e., max is 2**32-1.
#
#  The lengths are treated as unsigned quantities,
#  i.e., max is 2**31-1.
#
#  Use of this logic assumes string lengths <= 2**31-1 - Signed arithmetic
#
#  RETURN VALUE DESCRIPTION: Target string modified.
#			     returns 0 unless exception occurs
#			     in which case exception value is returned.
#
#       xpcopy( char * source, char * target, int length)
#
#  Calling sequence: xbcopy 
#       R3   Address of source string
#       R4   Address of target string
#       R5   Length of target and source string
#
#
          S_PROLOG(xbcopy)

          sri.   r0,r5,5     #1 Number of 32-byte chunks to move (L/32),
	  mflr   r7	      #1 Get return address
	  GET_CSA(cr1, r9, r6) # address of current mst

#                                 CR0 = short/long move switch.
          bgt    cr0,movelong #0/0 Branch if > 32 bytes to move.
#
# Short move (0 to 32 bytes).
#
          mtxer  r5          #1 Set move length (presuming short).
	  .using  mstsave,r6
          st	 r7,excbranch #1 On exception, branch to this location with
                             # error code in R3.
          		     # We set this to the return address!
          lsx    r5,0,r3     #v Get source string (kills R5 - R12).
          stsx   r5,0,r4     #v Store it.
	  GET_CSA(cr1, r7, r6) # address of current mst
          cal	 r3,0(r0)    #1 Zero return code for successful move
          st	 r3,excbranch #1 Reset special exception handler cell
          br                 #0 Return.
#
# Here we check if we have overlapping strings.
# overlap if: source < target && target < source + length
#		r3   <   r4   &&   r4   <  r3    +   r5
#
movelong: cmpl   cr1,r3,r4   #1 CR1 = forward/backward switch.
          st	 r7,excbranch #1 On exception, branch to this location with
                             # error code in R3.
          		     # We set this to the return address!
	  bge	 cr1,nobackward #skip if r3 >= r4
	  a	 r10,r3,r5   # r10 = r3 + r5 (source + length)
	  cmpl	 cr1,r4,r10  # Test if long fwd move might destruct
nobackward:
          mtctr  r0          #1 CTR = num chunks to move.
          lil    r0,32       #1
          mtxer  r0          #1 XER = 32 (move length per iteration).
          rlinm. r0,r5,0,0x1f #1 R0 = remainder length.
          sf     r4,r3,r4    #1 r4 = targ addr - source addr.
          blt    cr1,backward #0 B If A(source) < A(target) logically.
#
#
forward:  lsx    r5,0,r3     #8 Get 32 bytes of source.
          stsx   r5,r4,r3    #8 Store it.
          ai     r3,r3,32    #1 Increment source address.
          bdn    forward     #0 Decr count, Br if chunk(s) left to do.
#
          mtxer  r0          #1 XER = remainder length.
          lsx    r5,0,r3     #v Get the remainder string.
          stsx   r5,r4,r3    #v Store it.
	  GET_CSA(cr0, r7, r6) # address of current mst
          cal	 r3,0(r0)    #1 Zero return code for successful move
          st	 r3,excbranch #1 Reset special exception handler cell
          br                 #0 Return.
#
backward: a      r3,r3,r5    #1 r3 = source addr + len
#
loopb:    ai     r3,r3,-32   #1 Decrement source address.
          lsx    r5,0,r3     #8 Get 32 bytes of source.
          stsx   r5,r4,r3    #8 Store it.
          bdn    loopb       #0 Decr count, Br if chunk(s) left to do.
#
          sf     r3,r0,r3    #1 Decrement source address.
          mtxer  r0          #1 XER = remainder length.
          lsx    r5,0,r3     #v Get the remainder string.
          stsx   r5,r4,r3    #v Store it.
	  GET_CSA(cr0, r7, r6) # 1 address of current mst
          cal	 r3,0(r0)    #1 Zero return code for successful move
          st	 r3,excbranch #1 Reset special exception handler cell
	  .drop	 r6
	  br
          FCNDES(xbcopy)      #Function Descriptors

#********************************************************************
#
#  FUNCTION: exbcopy_pwr
#
#
#************************************************************************
#
#  NAME: exbcopy_pwr  move (overlapped NOT ok) and check for errors
#		  and don't perform load from source across page boundary
#
#  FUNCTION: Equal length character string move
#
#  EXECUTION ENVIRONMENT:
#  Must be in the kernel with write access to the current mst
#  Standard register usage and linkage convention.
#  Registers used r0,r3-r12
#  Condition registers used: 0,1
#  No stack requirements.
#
#  NOTES:
#  The source string represented by R3 is moved to the target string
#  area represented by R4 and R5.
#
#  The strings may be on any address boundary and may be of any length from 0
#  through (2**31)-1 inclusive.  The strings may NOT overlap.
#
#  The addresses are treated as unsigned quantities,
#  i.e., max is 2**32-1.
#
#  The lengths are treated as unsigned quantities,
#  i.e., max is 2**31-1.
#
#  Use of this logic assumes string lengths <= 2**31-1 - Signed arithmetic
#
#  RETURN VALUE DESCRIPTION: Target string modified.
#			     returns 0 unless exception occurs
#			     in which case exception value is returned.
#
#       exbcopy_pwr( char * source, char * target, int length)
#
#  Calling sequence: exbcopy_pwr 
#       R3   Address of source string
#       R4   Address of target string
#       R5   Length of target and source string
#
#

	  .csect ENTRY(exbcopy_pwr[PR]),5
	  .globl ENTRY(exbcopy_pwr[PR])
.exbcopy_pwr:
#
# We must avoid the situation where a load-string operation crosses
# a page boundary since if this operation faults it is impossible
# to determine how much of the move was completed (and we must know
# this in order to update file size correctly, etc.)
# We accomplish this by ensuring that the source address we load from
# is aligned on a 32-byte boundary.
#
	andil.	r0,r3,31	# See if source aligned on 32-byte boundary
	beq	cr0,align	# Branch if already aligned
	sfi	r0,r0,32	# Number of bytes to move to align source
	cmp	cr1,r5,r0	# See if total move smaller than align move
	ble	cr1,align	# Branch if smaller

	mflr	r7		# Get return address
	GET_CSA(cr0, r8, r6)	# address of current mst
	mtxer	r0		# Set length for move
	sf	r0,r0,r5	# Subtract align move from total move
	.using	mstsave,r6
	st	r7,excbranch	# On exception, branch to this location with
				# error code in R3.
				# We set this to the return address!
	lsx	r5,0,r3		# Get source string (could kill R5 - R12).
	stsx	r5,0,r4		# Store it.
	mr	r5,r0		# Set remaining move length
	mfxer	r0		# Get align move length from XER
	andil.	r0,r0,31	# Only want bits 25-31
	a	r3,r3,r0	# Update source address.
	a	r4,r4,r0	# Update target address.

align:
          sri.   r0,r5,5     #1 Number of 32-byte chunks to move (L/32),
	  mflr   r7	      #1 Get return address
	GET_CSA(cr1, r8, r6)	# address of current mst

#                                 CR0 = short/long move switch.
          bgt    cr0,emovelong #0/0 Branch if > 32 bytes to move.
#
# Short move (0 to 32 bytes).
#
          mtxer  r5          #1 Set move length (presuming short).
          st	 r7,excbranch #1 On exception, branch to this location with
                             # error code in R3.
          		     # We set this to the return address!
          lsx    r5,0,r3     #v Get source string (kills R5 - R12).
          stsx   r5,0,r4     #v Store it.
	  GET_CSA(cr0, r7, r6) # get current mst
          cal	 r3,0(r0)    #1 Zero return code for successful move
          st	 r3,excbranch #1 Reset special exception handler cell
          br                 #0 Return.
#
emovelong:
          st	 r7,excbranch #1 On exception, branch to this location with
                             # error code in R3.
          		     # We set this to the return address!
          mtctr  r0          #1 CTR = num chunks to move.
          lil    r0,32       #1
          mtxer  r0          #1 XER = 32 (move length per iteration).
          rlinm. r0,r5,0,0x1f #1 R0 = remainder length.
          sf     r4,r3,r4    #1 r4 = targ addr - source addr.
#
#
	  .align	5
eforward: lsx    r5,0,r3     #8 Get 32 bytes of source.
          stsx   r5,r4,r3    #8 Store it.
          ai     r3,r3,32    #1 Increment source address.
          bdn    eforward    #0 Decr count, Br if chunk(s) left to do.
#
          mtxer  r0          #1 XER = remainder length.
          lsx    r5,0,r3     #v Get the remainder string.
          stsx   r5,r4,r3    #v Store it.
	  GET_CSA(cr0, r7, r6)   # address of current mst
          cal	 r3,0(r0)    #1 Zero return code for successful move
          st	 r3,excbranch #1 Reset special exception handler cell
#
	  .drop	 r6
	  br
          FCNDES(exbcopy_pwr)      #Function Descriptors

#********************************************************************
#
#  FUNCTION: touchrc
#
#
#************************************************************************
#
#  NAME: touchrc
#
#  FUNCTION: Load from source byte
#
#  EXECUTION ENVIRONMENT:
#  Must be in the kernel with write access to the current mst
#  Standard register usage and linkage convention.
#  Registers used r3-r6
#  Condition registers used: None
#  No stack requirements.
#
#  RETURN VALUE DESCRIPTION: Load from source byte
#			     returns 0 unless exception occurs
#			     in which case exception value is returned.
#
#       touchrc(char * source)
#
#  Calling sequence: touchrc 
#       R3   Address of byte
#
#
          S_PROLOG(touchrc)

	  mflr   r6		# get return address
	  GET_CSA(cr0, r7, r5)	# address of current mst
	  .using  mstsave,r5
          st	 r6,excbranch	# on exception, branch to this location with
                             	# error code in R3.
          lbz    r4,0(r3)    	# get source byte
          cal	 r3,0(r0)    	# zero return code
          st	 r3,excbranch 	# reset special exception handler cell
	  .drop  r5
          br                 	# return
          FCNDES(touchrc)      	

#
#  NAME: xmemccpy_pwr
#                                                                    
#  FUNCTION:	Copy source to target, stopping if character c is copied.
# 		Copy no more than n bytes.
# 
#  RETURN VALUE DESCRIPTION: returns 0 unless an exception 
#		occurs. sets the target pointer to the character
#		following c in the copy or to NULL if c is not found 
#		in the first n bytes.
# 
#  xmemccpy(void **target, void *source, int c, size_t n)
#  {
#	char *t = *target, *s = source;
#	while (n-- > 0)
#		if ((*t++ = *s++) == c)
#		{
#			*target = t;
#			return (0);
#		}
#	*target = NULL;
#	return (0);
#  }	
#
#	r3 = target, r4 = source, r5 = c , r6 = n
#
#       uses r0, r3-r12
#
# NOTE:  This must be in misc.s because it is used by 601!

        S_PROLOG(xmemccpy_pwr)

	mflr	r0		# get return address
	.using  mstsave,r7
	GET_CSA(cr0, r8, r7)	# address of current mst
        st	r0,excbranch    # On exception, branch to this location with
				# r3 set to exception value.
	l	r12, 0(r3)	# r12 =  pointer to target
xmloop:	cmpi	cr0,r6,0	# test for no more bytes
	.machine "any"
	dozi	r7,r6,16	# r7 = max (0, 16-r6)
	.machine "com"
 	sfi	r7,r7,16	# r7 = 16 - r7 = min(16,r6)  
	ble	nomatch		# branch if no more bytes        
	rlimi   r7,r5,8,16,23   # insert byte c into bits 16-23
	mtxer	r7		# set up xer for lscbx 
	.machine "any"
	lscbx.	r8,r0,r4	# load bytes from source (up to 16)
	.machine "com"
	stsx	r8,r0,r12	# store bytes to target   
	bne	notyet 		# branch if no match found
	mfxer	r8		# get xer
	rlinm   r8,r8,0,25,31   # get count of bytes moved
	a	r12,r12,r8	# set pointer to 1 after last stored
	st	r12,0(r3)       # store pointer 
	b	xmreturn	# goto common return
notyet: ai	r12,r12,16	# increment target by 16
	ai	r4,r4,16	# increment source by 16
	ai	r6,r6,-16	# decrement count by 16
	b	xmloop		# loop
nomatch:
	cal	r4,0(r0)	# r4 = 0
	st	r4,0(r3)	# set pointer to null
xmreturn:
	GET_CSA(cr0, r0, r7)	# address of current mst
	cal	r3,0(r0)	# set return value to zero
        st	r3,excbranch    # clear exception branch 
	.drop	r7
	br
        FCNDES(xmemccpy_pwr)        # Function Descriptors




#
#  NAME: call_debugger
#                                                                    
#  FUNCTION: Calls the debugger 
#
#  NOTES:
#
#	call_debugger( struct mstsave *db1, int dbg2, int db3);
#
# 	Buys an MST;
#	Calls the debugger with the specified parameters: db1, db2, db3;
#	Returns MST;
#	Returns rc from debugger;
# 
#  RETURN VALUE DESCRIPTION: returns rc from debugger; 
# 

        S_PROLOG(call_debugger)

	st 	r31, -4(r1)		# save r31 on user stack
	mflr	r0			# save link register - return address 
	st	r0, stklink(r1)		# store it on the user stack
	stu	r1, -stkmin-4(r1)	# buy a stack frame on the user stack

	mr	r31, r1			# keep stack pointer in non-volatile reg

        .extern ENTRY(buy_mstsave)
	bl	ENTRY(buy_mstsave)	# buy a mst and stack for debugger

	.extern	ENTRY(debugger)		
	bl	ENTRY(debugger)		# call debugger

	st	NEW_MST, ppda_mstack(PPDA_ADDR)	# Give back mst
	st 	INTR_MST, ppda_csa(PPDA_ADDR)	# Restore current save area 
	CSA_UPDATED(cr0, r3, INTR_MST)

	cal	r1, stkmin+4(r31)	# restore user stack pointer
	l	r31, -4(r1)		# restore r31 from user stack
	l	r0, stklink(r1)		# restore return address
	mtlr	r0			# move to link register for br
	br				# return

#
#  NAME: buy_mstsave
#                                                                    
#  FUNCTION: Buy a new mstsave area and stack frame pointer
# 
#  RETURN VALUE DESCRIPTION: returns zero (0) upon completion
# 

        S_PROLOG(buy_mstsave)

	mfmsr	r11				# Get current interrupt levels 

#  We need to ensure that address translation is turned on because the
# current csa may point to the save area in u-block, which is addressed
# by virtual address. If we're running on an interrupt level, the csa 
# points to save area in mst stack, pinned in kernel. 
	cal	r1, DISABLED_MSR(0)		# disable interrupts
	mtmsr	r1			
	GET_PPDA(cr0, PPDA_ADDR)		# load ppda pointer

	l 	INTR_MST, ppda_csa(PPDA_ADDR)	# move old csa to INTR_MST
	l	NEW_MST, ppda_mstack(PPDA_ADDR)	# get new mst from mst stack
	cal	r0,0(0)				# get a zero
	st	r0, mstkjmpbuf(NEW_MST) 	# clear jmpbuf pointer
	st	r0, excbranch(NEW_MST)		# clear jmpbuf pointer
	sth	r0, mstintpri(NEW_MST)		# clear priority and backtrack
	oriu	r0,r0,NEW_ADSPACE		# mark segs 0,1,2,14 allocated
	st 	r0, mstsralloc(NEW_MST) 	# set allocation vector
	cal	r0, -framesize(NEW_MST)
	st	r0, ppda_mstack(PPDA_ADDR)
	st	NEW_MST, ppda_csa(PPDA_ADDR)	# store new csa
	cal	r1, -stkmin(NEW_MST)		# point to real stack frame
	CSA_UPDATED(cr0, r0, NEW_MST)

	mtmsr	r11				# restore interrupt level
	br

#****************************************************************************
#
#  NAME:  ffs
#
#  FUNCTION:  Finds First Set (ffs), the index of the first bit set in a mask 
#               word, left to right, 1-32, or 0 for a NULL mask.
#
#       ffs(mask)               rc = index of the first bit set
#
#  INPUT STATE:
#       r3 = mask, word of bits 
#
#  OUTPUT STATE:
#       r3 = index of the first bit found rigth to left, 1-32, or 0
#
#  EXECUTION ENVIRONMENT:
#       Supervisor state  : No
#
#*****************************************************************************

        S_PROLOG(ffs)

	#  y = ffs(x) = 32 - cntlz(x & -x);

        neg     4,3             # r4 = -x.
        and     4,3,4           # r4 = (x & -x).
        cntlz   3,4             # leading zeros
        sfi     3,3,0x20        # result =  32 - leading zeros
        br

#****************************************************************************
#
#  NAME:  fsig
#
#  FUNCTION:  Same as ffs(), except it works on a mask of 2 words (sigset_t)
#               uint mask[0] == bits  1-32,     (SIGMASKLO)
#               uint mask[1] == bits 33-64      (SIGMASKHI)
#
#       fsig(sigset)            rc = index of the first bit set
#       sigset_t sigset
#
#  INPUT STATE:
#       sigset_t:
#       r3 = SIGMASKLO
#       r4 = SIGMASKHI
#
#  OUTPUT STATE:
#       r3 = index of the first bit found rigth to left, 1-64, or 0
#
#  EXECUTION ENVIRONMENT:
#       Supervisor state  : No
#
#*****************************************************************************
        S_PROLOG(fsig)

SIGMASKLO:
        neg.    r5, r3          # r5 = -x.
        beq     cr0, SIGMASKHI  # (SIGMASKLO == 0) => try (SIGMASKHI)
        and     r5, r3, r5      # r5 = (x & -x).
        cntlz   r3, r5          # leading zeros
        sfi     r3, r3, 0x20    # result =  32 - leading zeros
        br
SIGMASKHI:
        neg.    r5, r4          # r5 = -x.
        beqr    cr0             # (SIGMASKHI == 0) => return 0 to caller
        and     r5, r4, r5      # r5 = (x & -x).
        cntlz   r3, r5          # leading zeros
        sfi     r3, r3, 0x40    # result =  32 - leading zeros
        br

#
# Name: sig_slih_resume
#
# Inputs: none
#
# Returns: does not return, branches to resume.
#
# Synopsis:
#    Due to the way sigreturn is coded, it is necessary to have a function
#    that clears ppda->stackfix and csa->stackfix before calling resume.
#    This function provides that function (see Defect 125828)
#
# Environment:
#    This function should only be called by sigreturn.  As such, called
#    with interrupts disabled to INTMAX.
#
	S_PROLOG(sig_slih_resume)

	GET_PPDA(cr0,r3)		# get ppda into r3
	li	r4,0			# use r4 to clear
	lwz	r5,ppda_csa(r3)		# get pointer to csa in r5
	stb	r4,ppda_stackfix(r3)	# clear ppda->stackfix
	st	r4,mststackfix(r5)	# clear csa->stackfix

	.extern ENTRY(resume)
	bl	ENTRY(resume)		# branch to resume

	FCNDES(sig_slih_resume)

	.toc
	TOCE(__ublock, data)
	TOCL(ret_trap, data)

include(flihs.m4)
include(machine.m4)
include(seg.m4)
include(m_types.m4)
include(mstsave.m4)
include(low_dsect.m4)
